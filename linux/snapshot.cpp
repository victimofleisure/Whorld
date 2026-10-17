// WHS2 v1 binary layout (Windows x64 D2D structs), without C++ struct dumps.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "snapshot.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring>
#include <fstream>
#include <limits>

namespace whorld {
namespace {
constexpr size_t HEADER = 12, STATE = 40, GLOBAL = 104, RING = 192;
constexpr size_t BASE = HEADER + STATE + GLOBAL;
constexpr size_t BODY = STATE + GLOBAL;
uint32_t u32(const uint8_t* p) {
    return uint32_t(p[0]) | (uint32_t(p[1]) << 8) | (uint32_t(p[2]) << 16) | (uint32_t(p[3]) << 24);
}
uint64_t u64(const uint8_t* p) { return uint64_t(u32(p)) | (uint64_t(u32(p + 4)) << 32); }
void put32(uint8_t* p, uint32_t x) { for (int i = 0; i < 4; ++i) p[i] = uint8_t(x >> (8 * i)); }
void put64(uint8_t* p, uint64_t x) { put32(p, uint32_t(x)); put32(p + 4, uint32_t(x >> 32)); }
float f32(const uint8_t* p) { auto n = u32(p); float f; std::memcpy(&f, &n, 4); return f; }
double f64(const uint8_t* p) { auto n = u64(p); double f; std::memcpy(&f, &n, 8); return f; }
void set32(uint8_t* p, double x) { float f = float(x); uint32_t n; std::memcpy(&n, &f, 4); put32(p, n); }
void set64(uint8_t* p, double x) { uint64_t n; std::memcpy(&n, &x, 8); put64(p, n); }
Point point(const uint8_t* p) { return {f64(p), f64(p + 8)}; }
void setPoint(uint8_t* p, Point q) { set64(p, q.x); set64(p + 8, q.y); }
Color color(const uint8_t* p) { return {f32(p), f32(p + 4), f32(p + 8), f32(p + 12)}; }
void setColor(uint8_t* p, Color c) { set32(p, c.r); set32(p + 4, c.g); set32(p + 8, c.b); set32(p + 12, c.a); }
bool bounded(double x) { return std::isfinite(x) && std::abs(x) <= 1e9; }
bool ok(Point p) { return bounded(p.x) && bounded(p.y); }
bool ok(Color c) { return bounded(c.r) && bounded(c.g) && bounded(c.b) && bounded(c.a); }
bool fail(std::string& error, const char* message) { error = message; return false; }
}
bool Snapshot::valid(std::string& error) const {
    if (!bounded(width) || !bounded(height) || width < 1 || height < 1 || width > 1e6 || height > 1e6 ||
        !bounded(zoom) || zoom <= 0 || zoom > 1e6 || !ok(background) || flags != 0 || rings.size() > MAX_RINGS)
        return fail(error, "Invalid snapshot drawing state");
    const auto& g = global;
    if (!bounded(g.rot) || !bounded(g.starRatio) || !bounded(g.pinwheel) || !ok(g.scale) || !ok(g.shift) ||
        !bounded(g.evenCurve) || !bounded(g.oddCurve) || !bounded(g.evenShear) || !bounded(g.oddShear) ||
        !bounded(g.lineWidth) || g.scale.x <= 0 || g.scale.y <= 0 || g.starRatio <= 0 ||
        g.sides < -MAX_RINGS || g.sides > MAX_RINGS)
        return fail(error, "Invalid snapshot global ring");
    for (const auto& r : rings) {
        if (!ok(r.shiftDelta) || !bounded(r.rotDelta) || !bounded(r.hue) || !bounded(r.lightness) ||
            !bounded(r.saturation) || !bounded(r.rot) || !bounded(r.radius) || !ok(r.scale) || !ok(r.shift) ||
            !bounded(r.starRatio) || !ok(r.color) || !bounded(r.pinwheel) || !bounded(r.lineWidth) ||
            !ok(r.origin) || !bounded(r.evenCurve) || !bounded(r.oddCurve) || !bounded(r.evenShear) ||
            !bounded(r.oddShear) || r.scale.x <= 0 || r.scale.y <= 0 || r.starRatio <= 0 ||
            r.sides < 1 || r.sides > 50 || r.mode < 0 || r.mode > 7)
            return fail(error, "Invalid snapshot ring");
    }
    error.clear();
    return true;
}
std::vector<uint8_t> encodeSnapshotBody(const Snapshot& snap) {
    std::vector<uint8_t> bytes(BODY + snap.rings.size() * RING, 0);
    uint8_t* s = bytes.data();
    set32(s, snap.width); set32(s + 4, snap.height); setColor(s + 8, snap.background);
    set64(s + 24, snap.zoom); put32(s + 32, uint32_t(snap.rings.size()));
    s[36] = snap.convex; s[37] = snap.flags;
    uint8_t* g = s + STATE;
    set64(g, snap.global.rot); set64(g + 8, snap.global.starRatio); set64(g + 16, snap.global.pinwheel);
    setPoint(g + 24, snap.global.scale); setPoint(g + 40, snap.global.shift);
    set64(g + 56, snap.global.evenCurve); set64(g + 64, snap.global.oddCurve);
    set64(g + 72, snap.global.evenShear); set64(g + 80, snap.global.oddShear);
    set64(g + 88, snap.global.lineWidth); put32(g + 96, uint32_t(snap.global.sides));
    for (size_t i = 0; i < snap.rings.size(); ++i) {
        uint8_t* p = g + GLOBAL + i * RING;
        const auto& r = snap.rings[i];
        setPoint(p, r.shiftDelta); set64(p + 16, r.rotDelta); set64(p + 24, r.hue);
        set64(p + 32, r.lightness); set64(p + 40, r.saturation);
        set64(p + 48, r.rot); set64(p + 56, r.radius); setPoint(p + 64, r.scale);
        setPoint(p + 80, r.shift); set64(p + 96, r.starRatio);
        p[104] = uint8_t(r.sides); p[105] = uint8_t(r.sides >> 8);
        p[106] = uint8_t(r.mode); p[107] = uint8_t(r.mode >> 8);
        p[108] = r.deleted; p[109] = r.skipFill;
        setColor(p + 112, r.color); set64(p + 128, r.pinwheel); set64(p + 136, r.lineWidth);
        setPoint(p + 144, r.origin); set64(p + 160, r.evenCurve); set64(p + 168, r.oddCurve);
        set64(p + 176, r.evenShear); set64(p + 184, r.oddShear);
    }
    return bytes;
}

bool decodeSnapshotBody(const uint8_t* s, size_t count, Snapshot& out, std::string& error) {
    if (count > MAX_RINGS) return fail(error, "Invalid snapshot ring count");
    Snapshot next;
    next.width = f32(s); next.height = f32(s + 4); next.background = color(s + 8);
    next.zoom = f64(s + 24); next.convex = s[36] != 0; next.flags = s[37];
    if (s[36] > 1 || s[38] || s[39]) return fail(error, "Invalid snapshot state flags");
    const uint8_t* g = s + STATE;
    next.global = {f64(g), f64(g + 8), f64(g + 16), point(g + 24), point(g + 40),
        f64(g + 56), f64(g + 64), f64(g + 72), f64(g + 80), f64(g + 88), int32_t(u32(g + 96))};
    if (u32(g + 100)) return fail(error, "Invalid snapshot reserved field");
    next.rings.reserve(count);
    for (size_t i = 0; i < count; ++i) {
        const uint8_t* p = g + GLOBAL + i * RING;
        SnapshotRing r;
        r.shiftDelta = point(p); r.rotDelta = f64(p + 16); r.hue = f64(p + 24);
        r.lightness = f64(p + 32); r.saturation = f64(p + 40);
        r.rot = f64(p + 48); r.radius = f64(p + 56); r.scale = point(p + 64);
        r.shift = point(p + 80); r.starRatio = f64(p + 96);
        r.sides = int16_t(uint16_t(p[104] | unsigned(p[105]) << 8));
        r.mode = int16_t(uint16_t(p[106] | unsigned(p[107]) << 8));
        r.deleted = p[108] != 0; r.skipFill = p[109] != 0;
        r.color = color(p + 112); r.pinwheel = f64(p + 128); r.lineWidth = f64(p + 136);
        r.origin = point(p + 144); r.evenCurve = f64(p + 160); r.oddCurve = f64(p + 168);
        r.evenShear = f64(p + 176); r.oddShear = f64(p + 184);
        if (p[108] > 1 || p[109] > 1 || p[110] || p[111]) return fail(error, "Invalid ring flags");
        next.rings.push_back(r);
    }
    if (!next.valid(error)) return false;
    out = std::move(next);
    return true;
}

bool Snapshot::load(const std::string& path, std::string& error) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) return fail(error, "Cannot open snapshot");
    const auto len = file.tellg();
    if (len < std::streamoff(BASE) || len > std::streamoff(BASE + MAX_RINGS * RING))
        return fail(error, "Invalid snapshot file size");
    std::vector<uint8_t> bytes(size_t(len), 0);
    file.seekg(0);
    if (!file.read(reinterpret_cast<char*>(bytes.data()), std::streamsize(bytes.size())))
        return fail(error, "Truncated snapshot");
    const uint8_t* b = bytes.data();
    if (std::memcmp(b, "WHS2", 4) || b[4] != 1 || b[5] != 0 ||
        b[6] != STATE || b[7] || b[8] != GLOBAL || b[9] || b[10] != RING || b[11])
        return fail(error, "Unsupported WHS2 snapshot header/version/layout");
    const auto count = u32(b + HEADER + 32);
    if (count > MAX_RINGS || bytes.size() != BASE + size_t(count) * RING)
        return fail(error, "Invalid snapshot ring count or truncated data");
    Snapshot next;
    if (!decodeSnapshotBody(b + HEADER, count, next, error)) return false;
    *this = std::move(next);
    return true;
}
bool Snapshot::save(const std::string& path, std::string& error) const {
    if (!valid(error)) return false;
    std::vector<uint8_t> bytes(BASE + rings.size() * RING, 0);
    uint8_t* b = bytes.data();
    std::memcpy(b, "WHS2", 4); b[4] = 1; b[6] = STATE; b[8] = GLOBAL; b[10] = RING;
    const std::vector<uint8_t> body = encodeSnapshotBody(*this);
    std::memcpy(b + HEADER, body.data(), body.size());
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) return fail(error, "Cannot write snapshot");
    file.write(reinterpret_cast<const char*>(bytes.data()), std::streamsize(bytes.size()));
    file.close();
    if (!file) return fail(error, "Error writing snapshot");
    error.clear();
    return true;
}
} // namespace whorld
