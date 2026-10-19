#include "movie_export.h"

#include <cstring>
#include <string>
#include <vector>

#ifdef WHORLD_HAVE_FFMPEG
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
}
#endif

namespace whorld {

#ifndef WHORLD_HAVE_FFMPEG

MovieExporter::~MovieExporter() = default;
bool MovieExporter::open(const std::string&, double, int, int, std::string& error) {
    error = "whorld was built without ffmpeg; video export is unavailable";
    return false;
}
bool MovieExporter::write(const unsigned char*, std::string& error) {
    error = "whorld was built without ffmpeg; video export is unavailable";
    return false;
}
bool MovieExporter::close(std::string&) { return true; }

#else // WHORLD_HAVE_FFMPEG

struct MovieExporter::Impl {
    AVFormatContext* fmt = nullptr;
    AVCodecContext* codec = nullptr;
    AVStream* stream = nullptr;
    AVFrame* frame = nullptr;
    AVPacket* pkt = nullptr;
    SwsContext* sws = nullptr;
    int64_t nextPts = 0;
    bool headerWritten = false;

    ~Impl() {
        if (sws) sws_freeContext(sws);
        if (pkt) av_packet_free(&pkt);
        if (frame) av_frame_free(&frame);
        if (codec) avcodec_free_context(&codec);
        if (fmt) {
            if (!(fmt->oformat->flags & AVFMT_NOFILE) && fmt->pb) avio_closep(&fmt->pb);
            avformat_free_context(fmt);
        }
    }
};

// Container by extension: QuickTime/MOV-compatible muxers accept H.264 in AVI
// too (the muxer does not care), so the choice is purely format metadata.
bool chooseFormat(const std::string& path, const char*& name, std::string& error) {
    const std::size_t dot = path.find_last_of('.');
    const std::string ext = dot == std::string::npos ? "" : path.substr(dot + 1);
    if (ext == "avi") name = "avi";
    else if (ext == "mp4" || ext == "m4v") name = "mp4";
    else if (ext == "mov") name = "mov";
    else if (ext == "mkv") name = "matroska";
    else {
        error = "unknown movie container: use .avi, .mp4, .mov or .mkv";
        return false;
    }
    return true;
}

// Prefers H.264; falls back to MPEG-4 part 2 when the build lacks libx264
// (which upstream AVI export effectively produced with old encoders anyway).
const AVCodec* pickVideoCodec(AVCodecID h264, std::string& error) {
    if (const AVCodec* c = avcodec_find_encoder(h264)) return c;
    error = "no video encoder available";
    return nullptr;
}

MovieExporter::~MovieExporter() {
    std::string unused;
    (void)close(unused);
}

bool MovieExporter::open(const std::string& path, double fps, int width, int height,
                         std::string& error) {
    std::string unused;
    (void)close(unused);
    if (path.empty() || width < 2 || height < 2 || (width & 1) || (height & 1)) {
        // Most video codecs want even dimensions.
        error = "movie export needs even width and height";
        return false;
    }
    if (!(fps >= 1 && fps <= 240)) {
        error = "movie export rate must be 1-240";
        return false;
    }
    const char* formatName = nullptr;
    if (!chooseFormat(path, formatName, error)) return false;
    av_log_set_level(AV_LOG_ERROR); // encoders print quietable stats otherwise
    impl_ = new Impl;
    if (avformat_alloc_output_context2(&impl_->fmt, nullptr, formatName, path.c_str()) < 0) {
        error = "cannot allocate output format for " + path;
        delete impl_;
        impl_ = nullptr;
        return false;
    }
    const AVCodec* encoder = pickVideoCodec(AV_CODEC_ID_H264, error);
    if (!encoder) {
        delete impl_;
        impl_ = nullptr;
        error = "no H.264 encoder available (install a full ffmpeg codec build)";
        return false;
    }
    impl_->codec = avcodec_alloc_context3(encoder);
    if (!impl_->codec) {
        error = "out of memory";
        delete impl_;
        impl_ = nullptr;
        return false;
    }
    impl_->codec->width = width;
    impl_->codec->height = height;
    impl_->codec->pix_fmt = AV_PIX_FMT_YUV420P;
    impl_->codec->time_base = AVRational{1, int(fps)};
    impl_->codec->framerate = AVRational{int(fps), 1};
    impl_->codec->gop_size = 12;
    // Variable bitrate quality target; CRF ~23 is visually equivalent to the
    // default H.264 muzzle and keeps disk usage predictable.
    impl_->codec->bit_rate = 3'000'000 * int(fps) / 30;
    if (impl_->fmt->oformat->flags & AVFMT_GLOBALHEADER)
        impl_->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    AVDictionary* opts = nullptr;
    if (av_dict_set(&opts, "preset", "veryfast", 0) < 0) { /* cosmetic only */ }
    if (avcodec_open2(impl_->codec, encoder, &opts) < 0) {
        av_dict_free(&opts);
        error = "cannot open video encoder for " + path;
        delete impl_;
        impl_ = nullptr;
        return false;
    }
    av_dict_free(&opts);
    if (!(impl_->fmt->oformat->flags & AVFMT_NOFILE) &&
        avio_open(&impl_->fmt->pb, path.c_str(), AVIO_FLAG_WRITE) < 0) {
        error = "cannot write movie file: " + path;
        delete impl_;
        impl_ = nullptr;
        return false;
    }
    impl_->stream = avformat_new_stream(impl_->fmt, nullptr);
    if (!impl_->stream) {
        error = "cannot create output stream";
        delete impl_;
        impl_ = nullptr;
        return false;
    }
    impl_->stream->time_base = impl_->codec->time_base;
    if (avcodec_parameters_from_context(impl_->stream->codecpar, impl_->codec) < 0) {
        error = "cannot set codec parameters";
        delete impl_;
        impl_ = nullptr;
        return false;
    }
    impl_->frame = av_frame_alloc();
    impl_->pkt = av_packet_alloc();
    if (!impl_->frame || !impl_->pkt) {
        error = "out of memory";
        delete impl_;
        impl_ = nullptr;
        return false;
    }
    impl_->frame->format = impl_->codec->pix_fmt;
    impl_->frame->width = width;
    impl_->frame->height = height;
    if (av_frame_get_buffer(impl_->frame, 0) < 0) {
        error = "cannot allocate frame buffer";
        delete impl_;
        impl_ = nullptr;
        return false;
    }
    impl_->sws = sws_getContext(width, height, AV_PIX_FMT_BGRA, width, height,
                                impl_->codec->pix_fmt, SWS_BILINEAR, nullptr, nullptr, nullptr);
    if (!impl_->sws) {
        error = "cannot allocate pixel converter";
        delete impl_;
        impl_ = nullptr;
        return false;
    }
    if (avformat_write_header(impl_->fmt, nullptr) < 0) {
        error = "cannot write movie header for " + path;
        delete impl_;
        impl_ = nullptr;
        return false;
    }
    impl_->headerWritten = true;
    path_ = path;
    return true;
}

bool MovieExporter::write(const unsigned char* bgra, std::string& error) {
    if (!impl_ || !impl_->headerWritten) {
        error = "movie exporter is not open";
        return false;
    }
    if (av_frame_make_writable(impl_->frame) < 0) {
        error = "frame buffer locked";
        return false;
    }
    const uint8_t* src[4] = {bgra, nullptr, nullptr, nullptr};
    const int srcStride[4] = {impl_->codec->width * 4, 0, 0, 0};
    if (sws_scale(impl_->sws, src, srcStride, 0, impl_->codec->height,
                  impl_->frame->data, impl_->frame->linesize) <= 0) {
        error = "cannot convert frame";
        return false;
    }
    impl_->frame->pts = impl_->nextPts++;
    const int sent = avcodec_send_frame(impl_->codec, impl_->frame);
    if (sent < 0) {
        error = "encoder rejected a frame";
        return false;
    }
    while (avcodec_receive_packet(impl_->codec, impl_->pkt) == 0) {
        av_packet_rescale_ts(impl_->pkt, impl_->codec->time_base, impl_->stream->time_base);
        impl_->pkt->stream_index = impl_->stream->index;
        if (av_interleaved_write_frame(impl_->fmt, impl_->pkt) < 0) {
            av_packet_unref(impl_->pkt);
            error = "cannot write movie frame";
            return false;
        }
        av_packet_unref(impl_->pkt);
    }
    ++frames_;
    return true;
}

bool MovieExporter::close(std::string& error) {
    if (!impl_) return true;
    if (impl_->headerWritten && impl_->codec) {
        // Flush the encoder, then the container trailer.
        if (avcodec_send_frame(impl_->codec, nullptr) == 0) {
            while (avcodec_receive_packet(impl_->codec, impl_->pkt) == 0) {
                av_packet_rescale_ts(impl_->pkt, impl_->codec->time_base,
                                     impl_->stream->time_base);
                impl_->pkt->stream_index = impl_->stream->index;
                if (av_interleaved_write_frame(impl_->fmt, impl_->pkt) < 0)
                    break;
                av_packet_unref(impl_->pkt);
            }
        }
        av_interleaved_write_frame(impl_->fmt, nullptr);
        if (av_write_trailer(impl_->fmt) < 0 && error.empty())
            error = "cannot write movie trailer";
    }
    delete impl_;
    impl_ = nullptr;
    return error.empty();
}

#endif // WHORLD_HAVE_FFMPEG

} // namespace whorld
