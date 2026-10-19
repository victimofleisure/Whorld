// Video export: encodes canvas frames into a real movie container (.avi,
// .mp4, .mov, .mkv) with ffmpeg, the upstream V1 "export movie" feature.
// Built only when ffmpeg dev libraries are found; otherwise open() fails with
// a clear message and the rest of Whorld is unaffected (WHM2 recording in
// movie.cpp always works, it is self-contained).
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include <string>

namespace whorld {

// Encodes a sequence of full-size frames. write() takes one BGRA, top-down
// frame of exactly the open() width x height (w*h*4 bytes). Frames get
// consecutive timestamps at the open() frame rate. close() finalizes the
// container; frameCount() reports the number of accepted frames after close.
class MovieExporter {
public:
    MovieExporter() = default;
    ~MovieExporter();
    MovieExporter(const MovieExporter&) = delete;
    MovieExporter& operator=(const MovieExporter&) = delete;

    bool open(const std::string& path, double fps, int width, int height, std::string& error);
    bool write(const unsigned char* bgra, std::string& error);
    bool close(std::string& error);
    int frameCount() const { return frames_; }

private:
    struct Impl;
    Impl* impl_ = nullptr;
    int frames_ = 0;
    std::string path_;
};

} // namespace whorld
