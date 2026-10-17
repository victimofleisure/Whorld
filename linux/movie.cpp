// WHM2 movie format implementation.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "movie.h"
#include <cstring>

namespace whorld {
namespace {

constexpr size_t HEADER = 40;
constexpr uint32_t FILE_ID = uint32_t('W') | (uint32_t('H') << 8) | (uint32_t('M') << 16) |
                             (uint32_t('2') << 24);
constexpr uint16_t FILE_VERSION = 1;

uint32_t u32(const uint8_t* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}
uint64_t u64(const uint8_t* p) { return uint64_t(u32(p)) | (uint64_t(u32(p + 4)) << 32); }
void put16(std::vector<uint8_t>& b, uint16_t x) {
    b.push_back(uint8_t(x));
    b.push_back(uint8_t(x >> 8));
}
void put32(std::vector<uint8_t>& b, uint32_t x) {
    for (int i = 0; i < 4; ++i) b.push_back(uint8_t(x >> (8 * i)));
}
void put64(std::vector<uint8_t>& b, uint64_t x) {
    put32(b, uint32_t(x));
    put32(b, uint32_t(x >> 32));
}
void putFloat(std::vector<uint8_t>& b, double x) {
    float f = float(x);
    uint32_t n;
    std::memcpy(&n, &f, 4);
    put32(b, n);
}
float f32(const uint8_t* p) {
    auto n = u32(p);
    float f;
    std::memcpy(&f, &n, 4);
    return f;
}
std::vector<uint8_t> makeHeader(uint64_t frameCount, uint64_t indexOffset, double frameRate,
                                double width, double height) {
    std::vector<uint8_t> h;
    h.reserve(HEADER);
    put32(h, FILE_ID);
    put16(h, FILE_VERSION);
    put16(h, uint16_t(SNAPSHOT_STATE_SIZE));
    put16(h, uint16_t(SNAPSHOT_GLOBAL_SIZE));
    put16(h, uint16_t(SNAPSHOT_RING_SIZE));
    putFloat(h, frameRate);
    putFloat(h, width);
    putFloat(h, height);
    put64(h, frameCount);
    put64(h, indexOffset);
    return h;
}

bool fail(std::string& error, const char* msg) {
    error = msg;
    return false;
}

} // namespace

bool MovieWriter::open(const std::string& path, double fps, double w, double h, std::string& error) {
    file.open(path, std::ios::binary | std::ios::trunc);
    if (!file) return fail(error, "Cannot open movie for writing");
    frameRate = fps;
    width = w;
    height = h;
    bytes = 0;
    index.clear();
    const std::vector<uint8_t> header = makeHeader(0, 0, frameRate, width, height);
    file.write(reinterpret_cast<const char*>(header.data()), std::streamsize(header.size()));
    if (!file) return fail(error, "Cannot write movie header");
    bytes = header.size();
    error.clear();
    return true;
}

bool MovieWriter::write(const Snapshot& snapshot, std::string& error) {
    if (!file) return fail(error, "Movie is not open");
    const std::vector<uint8_t> body = encodeSnapshotBody(snapshot);
    index.push_back(bytes);
    file.write(reinterpret_cast<const char*>(body.data()), std::streamsize(body.size()));
    if (!file) return fail(error, "Cannot write movie frame");
    bytes += body.size();
    error.clear();
    return true;
}

bool MovieWriter::close(std::string& error) {
    if (!file) return fail(error, "Movie is not open");
    const uint64_t indexOffset = bytes;
    // Frame index: one 64-bit file offset per frame, immediately after frames.
    for (uint64_t offset : index) {
        uint8_t raw[8];
        for (int i = 0; i < 8; ++i) raw[i] = uint8_t(offset >> (8 * i));
        file.write(reinterpret_cast<const char*>(raw), 8);
    }
    if (!file) return fail(error, "Cannot write movie frame index");
    // Rewrite the header now that the frame count and index offset are known.
    const std::vector<uint8_t> header = makeHeader(index.size(), indexOffset, frameRate, width, height);
    file.seekp(0);
    file.write(reinterpret_cast<const char*>(header.data()), std::streamsize(header.size()));
    file.close();
    if (!file) return fail(error, "Error finalizing movie");
    error.clear();
    return true;
}

bool MovieReader::open(const std::string& path, std::string& error) {
    file.open(path, std::ios::binary);
    if (!file) return fail(error, "Cannot open movie");
    file.seekg(0, std::ios::end);
    const std::streamoff len = file.tellg();
    if (len < std::streamoff(HEADER)) return fail(error, "Invalid movie file size");
    uint8_t header[HEADER];
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(header), HEADER)) return fail(error, "Truncated movie");
    const uint8_t* b = header;
    if (u32(b) != FILE_ID) return fail(error, "Unsupported movie format");
    if (b[4] != FILE_VERSION || b[5] != 0) return fail(error, "Unsupported movie version");
    const uint16_t stateSize = uint16_t(b[6] | (b[7] << 8));
    const uint16_t globalSize = uint16_t(b[8] | (b[9] << 8));
    const uint16_t ringSize = uint16_t(b[10] | (b[11] << 8));
    if (stateSize != SNAPSHOT_STATE_SIZE || globalSize != SNAPSHOT_GLOBAL_SIZE ||
        ringSize != SNAPSHOT_RING_SIZE)
        return fail(error, "Unsupported movie snapshot layout");
    rate = f32(b + 12);
    sizeW = f32(b + 16);
    sizeH = f32(b + 20);
    const uint64_t frameCount = u64(b + 24);
    indexOffset = u64(b + 32);
    if (frameCount > MAX_MOVIE_FRAMES || indexOffset > uint64_t(len) ||
        indexOffset + frameCount * 8 > uint64_t(len))
        return fail(error, "Invalid movie frame count or index offset");
    std::vector<uint8_t> raw(size_t(frameCount) * 8);
    file.seekg(std::streamoff(indexOffset));
    if (!raw.empty() && !file.read(reinterpret_cast<char*>(raw.data()), std::streamsize(raw.size())))
        return fail(error, "Truncated movie index");
    index.resize(size_t(frameCount));
    for (size_t i = 0; i < index.size(); ++i) {
        const uint64_t offset = u64(raw.data() + i * 8);
        if (offset < HEADER || offset >= indexOffset) return fail(error, "Invalid movie frame offset");
        index[i] = offset;
    }
    error.clear();
    return true;
}

bool MovieReader::frame(size_t i, Snapshot& out, std::string& error) {
    if (!file) return fail(error, "Movie is not open");
    if (i >= index.size()) return fail(error, "Movie frame index out of range");
    const uint64_t start = index[i];
    const uint64_t end = (i + 1 < index.size()) ? index[i + 1] : indexOffset;
    if (end <= start) return fail(error, "Invalid movie frame");
    const uint64_t bodySize = end - start;
    if (bodySize < SNAPSHOT_BODY_SIZE || (bodySize - SNAPSHOT_BODY_SIZE) % SNAPSHOT_RING_SIZE)
        return fail(error, "Invalid movie frame size");
    const size_t count = size_t((bodySize - SNAPSHOT_BODY_SIZE) / SNAPSHOT_RING_SIZE);
    std::vector<uint8_t> body(static_cast<std::size_t>(bodySize));
    file.seekg(std::streamoff(start));
    if (!file.read(reinterpret_cast<char*>(body.data()), std::streamsize(body.size())))
        return fail(error, "Truncated movie frame");
    return decodeSnapshotBody(body.data(), count, out, error);
}

} // namespace whorld
