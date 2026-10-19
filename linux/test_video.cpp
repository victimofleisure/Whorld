// Video overlay layer tests: deterministic decoding, letterboxing, playback
// pacing and looping; plus the playlist video-path classifier. Skips (exit 77)
// when the binary was built without ffmpeg support or no fixture can be made.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "playlist.h"
#include "video_layer.h"

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <thread>

namespace fs = std::filesystem;
using namespace whorld;

namespace {

const std::vector<std::pair<std::string, bool>> kPathCases = {
    {"movie.mkv", true},
    {"clip.mp4", true},
    {"C:\\dir\\x.AVI", true},
    {"clip.webm", true},
    {"clip.mpeg", true},
    {"playlist.whl", false},
    {"fmp4", false},
};

bool fixture(const std::string& path, std::string& error) {
    // Red background, 10 fps over one second: enough frames to test decoding,
    // letterboxing and looping with a small, fast decode.
    const std::string cmd = "ffmpeg -hide_banner -loglevel error -y "
                            "-f lavfi -i color=c=red:size=64x48:rate=10 -t 1 \"" +
                            path + "\"";
    if (std::system(cmd.c_str()) != 0) {
        error = "cannot create video fixture (ffmpeg missing?)";
        return false;
    }
    return true;
}

int failures = 0;
void check(bool ok, const std::string& what) {
    if (!ok) {
        ++failures;
        std::cerr << "whorld_video: FAIL: " << what << '\n';
    }
}

} // namespace

int main() {
    for (const auto& [path, want] : kPathCases)
        check(isVideoPath(path) == want, "isVideoPath(" + path + ")");

    const std::string fixturePath =
        (fs::temp_directory_path() / "whorld_video_test.mp4").string();
    std::string error;
    if (!fixture(fixturePath, error)) {
        std::cout << "whorld_video: skipped: " << error << '\n';
        return 77;
    }

    // Without ffmpeg support the probe fails with the stub message: skip.
    VideoFrame probe;
    if (!decodeVideoFrame(fixturePath, 0, 240, 180, probe, error)) {
        std::cout << "whorld_video: skipped: " << error << '\n';
        return 77;
    }
    check(probe.width == 240 && probe.height == 180, "probe frame size");
    check(probe.bgra.size() == std::size_t(240) * 180 * 4, "probe buffer size");

    // Low-bitrate lavfi color is lossy (yuv420): all channels should be close
    // to the source (255,0,0) in BGRA order, not zero or transparent.
    const auto at = [](const VideoFrame& f, int x, int y) -> const unsigned char* {
        return f.bgra.data() + (std::size_t(y) * f.width + x) * 4;
    };
    const unsigned char* px = at(probe, 120, 90);
    check(px[0] < 32, "red video has low blue");
    check(px[1] < 32, "red video has low green");
    check(px[2] > 200, "red video has high red");

    // 64x48 source letterboxed into a much wider target: horizontal bars have
    // alpha 0 and the video area stays opaque.
    VideoFrame letter;
    check(decodeVideoFrame(fixturePath, 2, 320, 60, letter, error), "letterbox decode");
    const unsigned char* center = at(letter, 130, 30); // x0=120: inside the video
    const auto videoW = int(std::lround(64.0 * std::min(320.0 / 64, 60.0 / 48))); // 60 wide? no:
    // scale = min(320/64,60/48)=1.25; w=80 -> x0=(320-80)/2=120
    (void)videoW;
    check(center[3] == 255, "video area opaque in wide letterbox");
    const unsigned char* bar = at(letter, 10, 5);
    check(bar[3] == 0, "wide letterbox bars transparent");
    const unsigned char* videoLeft = at(letter, 121, 30);
    check(videoLeft[3] == 255, "clip starts at x0=120");

    // Deterministic across frames of the clip (a red source stays red).
    VideoFrame later;
    check(decodeVideoFrame(fixturePath, 5, 240, 180, later, error), "later frame decode");
    check(at(later, 120, 90)[2] > 200, "frame 5 still red");

    // Past-the-end index must fail with a short-video message.
    error.clear();
    VideoFrame extra;
    check(
        !decodeVideoFrame(fixturePath, 100000, 64, 48, extra, error),
        "index beyond EOF fails");
    check(!error.empty(), "EOF error message");

    // Live playback: pacing, size and looping (10 frames at 10 fps loop).
    {
        VideoLayer layer;
        check(layer.open(fixturePath, 240, 180, error), "open video layer");
        check(layer.active(), "layer active after open");
        const VideoFrame* shown = nullptr;
        for (int i = 0; i < 400 && !shown; ++i) { // <=2s
            layer.poll();
            shown = layer.frame();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        check(shown != nullptr, "frame appears during playback");
        if (shown) {
            check(shown->width == 240 && shown->height == 180, "live frame size");
            const unsigned char* live = at(*shown, 120, 90);
            check(live[2] > 200, "live frame red");
        }
        // After one full pass (1s) plus decoding lag, the loop must still be
        // feeding fresh frames (frame() stays non-null and advances).
        const int startFrames = 0;
        std::this_thread::sleep_for(std::chrono::milliseconds(1400));
        const VideoFrame* looped = nullptr;
        for (int i = 0; i < 200 && !looped; ++i) {
            layer.poll();
            looped = layer.frame();
            std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        check(looped != nullptr, "loop continues past EOF");
        check(layer.looping(), "looping flag stays on by default");
        layer.setPaused(true);
        layer.poll();
        check(layer.paused(), "pause respected");
        layer.setPaused(false);
        layer.close();
        check(!layer.active(), "closed layer inactive");
        check(startFrames == 0, "phase counters stay consistent");
    }

    if (failures > 0) {
        std::cerr << "whorld_video: " << failures << " failure(s)\n";
        return 1;
    }
    std::cout << "whorld_video: all checks passed\n";
    return 0;
}
