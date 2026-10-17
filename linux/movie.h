// WHM2 movie format: a 40-byte header, a run of snapshot payloads, and a
// trailing array of 64-bit frame offsets. The payloads are the same bytes as
// the body of a WHS2 snapshot file, so movies and snapshots share code.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "snapshot.h"
#include <cstdint>
#include <fstream>
#include <string>
#include <vector>

namespace whorld {

// Streams snapshots to a .whm file. The header is finalized on close, once the
// frame count and index offset are known.
class MovieWriter {
public:
    bool open(const std::string& path, double frameRate, double width, double height,
              std::string& error);
    bool write(const Snapshot& snapshot, std::string& error);
    bool close(std::string& error);
    size_t frameCount() const { return index.size(); }

private:
    std::ofstream file;
    std::vector<uint64_t> index;
    uint64_t bytes = 0; // current write offset, also the next frame's offset
    double frameRate = 60, width = 0, height = 0;
};

// Reads a .whm file and decodes individual frames on demand. The header and
// frame index are loaded up front; frame payloads are streamed from the file.
class MovieReader {
public:
    bool open(const std::string& path, std::string& error);
    size_t frameCount() const { return index.size(); }
    double frameRate() const { return rate; }
    double width() const { return sizeW; }
    double height() const { return sizeH; }
    bool frame(size_t i, Snapshot& out, std::string& error);

private:
    std::ifstream file;
    std::vector<uint64_t> index;
    uint64_t indexOffset = 0;
    double rate = 60, sizeW = 0, sizeH = 0;
};

} // namespace whorld
