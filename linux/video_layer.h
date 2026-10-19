// Video overlay: ffmpeg decode of a file, paced and blended over the canvas
// (GPL-3.0-or-later). Built only when ffmpeg dev libraries are found; without
// them open() fails with a clear message and the rest of Whorld is unaffected.
#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace whorld {

// Compositing modes for the overlay (a subset of the upstream video blending
// table). Premiered as fixed-function blends on the GPU path and cairo
// operators on the fallback:
//   0 Normal    alpha over
//   1 Add       linear add (an additive glow, like --blend add)
//   2 Screen    brightened wash (like --blend screen)
//   3 Multiply  darkening wash
constexpr int VIDEO_BLEND_NORMAL = 0;
constexpr int VIDEO_BLEND_ADD = 1;
constexpr int VIDEO_BLEND_SCREEN = 2;
constexpr int VIDEO_BLEND_MULTIPLY = 3;

// A decoded frame: BGRA, top-down, one row targetWidth*4 bytes, non-premultiplied
// alpha (255 inside the letterboxed video, 0 in the unused bars).
struct VideoFrame {
    std::vector<unsigned char> bgra;
    int width = 0, height = 0; // target frame size
};

// Decodes a video: open once, poll every display frame; frame() is updated to
// new frames as their presentation time arrives. Playback time tracks the wall
// clock, so frames decoded too late or too early are dropped to keep the pace.
class VideoLayer {
public:
    VideoLayer();
    ~VideoLayer();
    VideoLayer(const VideoLayer&) = delete;
    VideoLayer& operator=(const VideoLayer&) = delete;

    // Opens `path` and decodes into targetW x targetH frames. Uses the video's
    // frame rate for pacing, loops forever, and starts a decode thread. FINALED:
    // false with an `error` when the file cannot be opened/decoded (or ffmpeg
    // support was not built).
    // Opens `path` and decodes into targetW x targetH frames. Uses the video's
    // frame rate for pacing, loops forever, and starts a decode thread. Returns
    // false with an `error` when the file cannot be opened/decoded (or ffmpeg
    // support was not built).
    bool open(const std::string& path, int targetW, int targetH, std::string& error);
    void close();
    bool active() const { return running_; }
    bool paused() const { return paused_; }
    void setPaused(bool on); // pauses the presentation clock, not the stream
    void setLooping(bool on); // default: loop
    bool looping() const { return looping_; }

    // Drains the decode queue up to the presentation clock. Call once per
    // frame; then frame() returns the latest frame (nullptr early on).
    void poll();
    const VideoFrame* frame() const { return current_; }
    std::string path() const { return path_; }
    int videoStreamIndex() const { return streamIndex_; }
    double fps() const { return fps_; }
    std::string lastError() const; // latest decode error, when any

private:
    struct Impl;
    Impl* impl_ = nullptr;
    VideoFrame* current_ = nullptr;
    std::string path_;
    int targetW_ = 0, targetH_ = 0;
    bool running_ = false;
    bool paused_ = false;
    bool looping_ = true;
    int streamIndex_ = -1;
    double fps_ = 0;
};

// Decodes one specific frame of a video into `targetW x targetH
// letterboxed BGRA pixels sequentially (frame 0..index). Deterministic, so it
// backs `--render`/`--movie-out` overlay passes and tests. Bigger indexes are
// O(n) frame decodes. Returns false with an `error` when the video is shorter
// than `index` or cannot be opened.
bool decodeVideoFrame(const std::string& path, int index, int targetW, int targetH,
                      VideoFrame& frame, std::string& error);

} // namespace whorld
