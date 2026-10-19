#include "video_layer.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <condition_variable>
#include <cstring>
#include <deque>
#include <limits>
#include <mutex>
#include <string>
#include <thread>

#ifdef WHORLD_HAVE_FFMPEG
extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}
#endif

namespace whorld {

#ifndef WHORLD_HAVE_FFMPEG

// Stub build: no ffmpeg dev libraries on this system.
bool decodeVideoFrame(const std::string&, int, int, int, VideoFrame&, std::string& error) {
    error = "whorld was built without ffmpeg; video overlay is unavailable";
    return false;
}

VideoLayer::VideoLayer() = default;
VideoLayer::~VideoLayer() = default;
bool VideoLayer::open(const std::string&, int, int, std::string& error) {
    error = "whorld was built without ffmpeg; video overlay is unavailable";
    return false;
}
void VideoLayer::close() {}
void VideoLayer::setPaused(bool) {}
void VideoLayer::setLooping(bool) {}
void VideoLayer::poll() {}
std::string VideoLayer::lastError() const { return {}; }

} // namespace whorld

#else // WHORLD_HAVE_FFMPEG

namespace {

// One ffmpeg decode session over an input file. Each successful `decodeOne`
// hands back exactly one frame in `bg` as BGRA; a restart rewinds the input so
// loops continue from frame 0.
struct Session {
    AVFormatContext* fmt = nullptr;
    AVCodecContext* codec = nullptr;
    AVFrame* raw = nullptr;
    AVPacket* pkt = nullptr;
    int stream = -1;
    int videoW = 0, videoH = 0;
    double fps = 25;
    std::vector<unsigned char> bg;

    ~Session() { close(); }
    void close() {
        if (pkt) av_packet_free(&pkt);
        if (raw) av_frame_free(&raw);
        if (codec) avcodec_free_context(&codec);
        if (fmt) avformat_close_input(&fmt);
        pkt = nullptr;
        raw = nullptr;
        codec = nullptr;
        fmt = nullptr;
        stream = -1;
        bg.clear();
        bg.shrink_to_fit();
    }

    // Opens `path`; on failure the session resets and `error` describes it.
    bool open(const std::string& path, std::string& error) {
        close();
        if (avformat_open_input(&fmt, path.c_str(), nullptr, nullptr) != 0) {
            error = "cannot open video: " + path;
            return false;
        }
        if (avformat_find_stream_info(fmt, nullptr) < 0) {
            error = "cannot read video stream info: " + path;
            close();
            return false;
        }
        const AVCodec* decoder = nullptr;
        stream = av_find_best_stream(fmt, AVMEDIA_TYPE_VIDEO, -1, -1, &decoder, 0);
        if (stream < 0 || !decoder) {
            error = "no video stream in " + path;
            close();
            return false;
        }
        codec = avcodec_alloc_context3(decoder);
        if (!codec || avcodec_parameters_to_context(codec, fmt->streams[stream]->codecpar) < 0 ||
            avcodec_open2(codec, decoder, nullptr) < 0) {
            error = "cannot open video decoder for " + path;
            close();
            return false;
        }
        raw = av_frame_alloc();
        pkt = av_packet_alloc();
        if (!raw || !pkt) {
            error = "out of memory";
            close();
            return false;
        }
        videoW = codec->width;
        videoH = codec->height;
        if (videoW < 2 || videoH < 2 || videoW > 8192 || videoH > 8192) {
            error = "unsupported video size: " + path;
            close();
            return false;
        }
        const AVRational fr = fmt->streams[stream]->avg_frame_rate.num > 0
                                  ? fmt->streams[stream]->avg_frame_rate
                                  : fmt->streams[stream]->r_frame_rate;
        fps = fr.num > 0 && fr.den > 0 && double(fr.num) / double(fr.den) > 0.01
                  ? double(fr.num) / double(fr.den)
                  : 25.0;
        bg.resize(std::size_t(videoW) * videoH * 4);
        return true;
    }

    // Returns 1 when a frame converted ok, 0 at end of input, or -1 after
    // decoding trouble (`error` carries details; the session may continue).
    int decodeOne(std::string& error) {
        for (;;) {
            const int rc = avcodec_receive_frame(codec, raw);
            if (rc == 0) break;                 // a frame is ready
            if (rc == AVERROR(EAGAIN)) {        // feed the decoder more input
                if (av_read_frame(fmt, pkt) < 0) return 0;
                if (pkt->stream_index != stream) {
                    av_packet_unref(pkt);
                    continue;
                }
                if (avcodec_send_packet(codec, pkt) < 0) {
                    av_packet_unref(pkt);
                    error = "video decoder rejected a packet";
                    return -1;
                }
                av_packet_unref(pkt);
                continue;
            }
            if (rc == AVERROR_EOF) return 0;    // input fully drained
            error = "video frame decode failed";
            return -1;
        }
        SwsContext* sws = sws_getContext(videoW, videoH,
                                         static_cast<AVPixelFormat>(raw->format),
                                         videoW, videoH, AV_PIX_FMT_BGRA,
                                         SWS_BILINEAR, nullptr, nullptr, nullptr);
        if (!sws) {
            error = "cannot allocate a pixel scaler";
            av_frame_unref(raw);
            return -1;
        }
        unsigned char* dst[4] = { bg.data(), nullptr, nullptr, nullptr };
        const int dstStride[4] = { videoW * 4, 0, 0, 0 };
        const int wrote = sws_scale(sws, raw->data, raw->linesize, 0, videoH, dst, dstStride);
        sws_freeContext(sws);
        av_frame_unref(raw);
        if (wrote <= 0) {
            error = "cannot convert a video frame";
            return -1;
        }
        return 1;
    }

    // Restarts the stream at frame 0 by reopening the file, which also rules
    // out stale decoder state for looping playback.
    bool restart(const std::string& path, std::string& error) {
        const double keptFps = fps;
        if (!open(path, error)) return false;
        fps = keptFps;
        return true;
    }
};

// Aspect-preserving letterbox of one BGRA frame into the target size; alpha
// stays 255 inside the video rectangle and 0 in the bars.
void letterboxInto(std::vector<unsigned char>& out, int outW, int outH,
                   const std::vector<unsigned char>& in, int inW, int inH) {
    out.assign(std::size_t(outW) * outH * 4, 0);
    if (inW <= 0 || inH <= 0 || outW <= 0 || outH <= 0) return;
    const double scale = std::min(double(outW) / inW, double(outH) / inH);
    const int w = std::max(1, int(std::lround(inW * scale)));
    const int h = std::max(1, int(std::lround(inH * scale)));
    const int x0 = (outW - w) / 2, y0 = (outH - h) / 2;
    std::vector<unsigned char> scaled(std::size_t(w) * h * 4);
    SwsContext* sws = sws_getContext(inW, inH, AV_PIX_FMT_BGRA, w, h, AV_PIX_FMT_BGRA,
                                    SWS_AREA, nullptr, nullptr, nullptr);
    if (sws) {
        const unsigned char* src[4] = { in.data(), nullptr, nullptr, nullptr };
        unsigned char* dst[4] = { scaled.data(), nullptr, nullptr, nullptr };
        const int srcStride[4] = { inW * 4, 0, 0, 0 };
        const int dstStride[4] = { w * 4, 0, 0, 0 };
        sws_scale(sws, src, srcStride, 0, inH, dst, dstStride);
        sws_freeContext(sws);
    } else {
        for (int y = 0; y < h; ++y) for (int x = 0; x < w; ++x) {
            const int sx = std::min(inW - 1, x * inW / w);
            const int sy = std::min(inH - 1, y * inH / h);
            std::memcpy(&scaled[(std::size_t(y) * w + x) * 4],
                        &in[(std::size_t(sy) * inW + sx) * 4], 4);
        }
    }
    for (int y = 0; y < h; ++y)
        std::memcpy(&out[(std::size_t(y0 + y) * outW + x0) * 4],
                    &scaled[std::size_t(y) * w * 4], std::size_t(w) * 4);
}

} // namespace

bool decodeVideoFrame(const std::string& path, int index, int targetW, int targetH,
                      VideoFrame& out, std::string& error) {
    if (targetW < 1 || targetH < 1 || targetW > 8192 || targetH > 8192 || index < 0) {
        error = "invalid frame or frame size request";
        return false;
    }
    Session session;
    if (!session.open(path, error)) return false;
    for (int i = 0; i <= index; ++i) {
        const int rc = session.decodeOne(error);
        if (rc != 1) {
            if (rc == 0)
                error = "video is shorter than " + std::to_string(index) + " frames: " + path;
            return false;
        }
    }
    letterboxInto(out.bgra, targetW, targetH, session.bg, session.videoW, session.videoH);
    out.width = targetW;
    out.height = targetH;
    return true;
}

// VideoLayer internals --------------------------------------------------------

struct VideoLayer::Impl {
    Session session;
    std::thread worker;
    std::mutex mutex;
    std::condition_variable cv;

    bool running = false;         // worker lives while true
    bool paused = false;
    bool looping = true;
    bool completed = false;       // non-looping stream played through
    int errors = 0;               // decode error hits for a backoff count
    std::string lastError;

    std::chrono::steady_clock::time_point origin;    // presentation clock origin
    std::chrono::steady_clock::time_point pauseBegan;
    unsigned long produced = 0;
    int targetW = 0, targetH = 0;

    struct Entry {
        unsigned long index = 0;
        VideoFrame data;
    };
    std::deque<Entry> queue;
};

VideoLayer::VideoLayer() = default;
VideoLayer::~VideoLayer() { close(); }

void VideoLayer::setLooping(bool on) {
    looping_ = on;
    if (impl_) impl_->looping = on;
}

void VideoLayer::setPaused(bool on) {
    if (impl_ && running_) {
        if (on == impl_->paused) return;
        const auto now = std::chrono::steady_clock::now();
        if (on) {
            impl_->pauseBegan = now;
        } else {
            impl_->origin += now - impl_->pauseBegan; // presentation clock stops here
        }
        impl_->paused = on;
    }
    paused_ = on;
}

void VideoLayer::close() {
    if (impl_) {
        {
            std::lock_guard<std::mutex> lock(impl_->mutex);
            impl_->running = false;
        }
        impl_->cv.notify_all();
        if (impl_->worker.joinable()) impl_->worker.join();
        delete impl_;
        impl_ = nullptr;
    }
    if (current_) {
        delete current_;
        current_ = nullptr;
    }
    running_ = false;
    paused_ = false;
    path_.clear();
    fps_ = 0;
}

bool VideoLayer::open(const std::string& path, int targetW, int targetH, std::string& error) {
    close();
    if (targetW < 4 || targetH < 4 || targetW > 8192 || targetH > 8192) {
        error = "invalid video target size";
        return false;
    }
    targetW_ = targetW;
    targetH_ = targetH;
    path_ = path;
    impl_ = new Impl;
    if (!impl_->session.open(path, error)) {
        const std::string reason = error;
        close();
        error = reason;
        return false;
    }
    fps_ = impl_->session.fps;
    streamIndex_ = impl_->session.stream;
    impl_->targetW = targetW;
    impl_->targetH = targetH;
    impl_->origin = std::chrono::steady_clock::now();
    impl_->running = true;
    impl_->worker = std::thread([this]() {
        for (;;) {
            std::unique_lock<std::mutex> lock(impl_->mutex);
            impl_->cv.wait_for(lock, std::chrono::milliseconds(60), [&] {
                return !impl_->running || (!impl_->paused && impl_->queue.size() < 6);
            });
            if (!impl_->running) return;
            lock.unlock();

            std::string failure;
            const int rc = impl_->session.decodeOne(failure);
            if (rc == 1) {
                VideoLayer::Impl::Entry entry;
                entry.index = impl_->produced++;
                letterboxInto(entry.data.bgra, impl_->targetW, impl_->targetH,
                              impl_->session.bg, impl_->session.videoW, impl_->session.videoH);
                entry.data.width = impl_->targetW;
                entry.data.height = impl_->targetH;
                {
                    std::lock_guard<std::mutex> lock2(impl_->mutex);
                    impl_->queue.push_back(std::move(entry));
                    impl_->cv.notify_all();       // poll() may be waiting on space
                }
                continue;
            }
            if (rc == 0) {
                // End of input: loop back to frame 0 (looping) or mark done.
                bool restarted = false;
                if (impl_->looping && impl_->session.restart(path_, failure)) restarted = true;
                std::lock_guard<std::mutex> lock2(impl_->mutex);
                if (restarted) {
                    if (!impl_->paused)
                        impl_->origin = std::chrono::steady_clock::now();
                    impl_->produced = 0;
                    continue;
                }
                impl_->completed = true;
                continue;
            }
            {
                // A decode error: record it, back off briefly, keep trying for a
                // few packets; after a string of failures give up quietly.
                std::lock_guard<std::mutex> lock2(impl_->mutex);
                impl_->lastError = failure;
                if (++impl_->errors > 10)
                    impl_->lastError = "video decode failed repeatedly";
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(
                impl_->errors > 10 ? 1000 : 40));
        }
    });
    running_ = true;
    return true;
}

void VideoLayer::poll() {
    if (!running_ || !impl_) return;
    // Presentation clock: while paused nothing is consumed (the visible frame
    // freezes); on resume the origin shift keeps the existing display time. No
    // locked cv wait here: poll runs every display frame in the main loop.
    if (impl_->paused) return;
    const auto now = std::chrono::steady_clock::now();
    const double elapsed = std::chrono::duration<double>(now - impl_->origin).count();
    const unsigned long target = static_cast<unsigned long>(
        std::max(0.0, elapsed * std::max(0.01, fps_)));

    {
        std::lock_guard<std::mutex> lock(impl_->mutex);
        while (!impl_->queue.empty() && impl_->queue.front().index <= target) {
            if (current_) delete current_;
            current_ = new VideoFrame(std::move(impl_->queue.front().data));
            impl_->queue.pop_front();
        }
    }
    impl_->cv.notify_all(); // wake the producer for more queue space
}

std::string VideoLayer::lastError() const {
    return impl_ ? impl_->lastError : std::string();
}

} // namespace whorld

#endif // WHORLD_HAVE_FFMPEG
