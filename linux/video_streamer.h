// VideoStreamer: outputs raw frames to a V4L2 virtual camera (/dev/videoX)
// or to a pipe/stdout for real-time video streaming and inter-service routing.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <cstddef>
#include <string>
#include <vector>

namespace whorld {

class VideoStreamer {
public:
    VideoStreamer() = default;
    ~VideoStreamer();

    bool openV4L2(const std::string& device, int width, int height, std::string& error);
    bool openPipe(const std::string& path, int width, int height, std::string& error);
    bool active() const { return fd >= 0; }

    // Accepts row-major top-down RGB24 image
    bool writeFrame(const unsigned char* rgbData, std::size_t size);
    void close();

    int width() const { return w; }
    int height() const { return h; }
    bool isV4L2Device() const { return isV4L2; }

private:
    int fd = -1;
    int w = 0, h = 0;
    bool isV4L2 = false;
};

} // namespace whorld
