// WHS2 v1 snapshots. All on-disk fields are encoded explicitly, little-endian.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "whorld_core.h"
#include <cstdint>

namespace whorld {
struct SnapshotRing {
    Point shiftDelta;
    double rotDelta = 0, hue = 0, lightness = 0, saturation = 0;
    double rot = 0, radius = 0;
    Point scale{1, 1}, shift;
    double starRatio = 1;
    int sides = 5, mode = 0;
    bool deleted = false, skipFill = false;
    Color color;
    double pinwheel = 0, lineWidth = 1;
    Point origin;
    double evenCurve = 0, oddCurve = 0, evenShear = 0, oddShear = 0;
    // Port-only shape data; NOT serialized (the Windows ring struct has no room
    // for it). A primitive is approximated by a regular polygon on save.
    RingShape shape;
};
struct SnapshotGlobal {
    double rot = 0, starRatio = 1, pinwheel = 0;
    Point scale{1, 1}, shift;
    double evenCurve = 0, oddCurve = 0, evenShear = 1, oddShear = 1;
    double lineWidth = 0;
    int sides = 0;
};
struct Snapshot {
    // Target dimensions on disk are float32. Zoom is float64; colors are float32.
    double width = 0, height = 0, zoom = 1;
    Color background{0, 0, 0, 1};
    bool convex = false;
    uint8_t flags = 0;
    SnapshotGlobal global;
    std::vector<SnapshotRing> rings; // head first (newest for outward growth)
    // Failure leaves the receiver unchanged. Only WHS2 version 1 is supported.
    bool load(const std::string& path, std::string& error);
    bool save(const std::string& path, std::string& error) const;
    bool valid(std::string& error) const;
};

// Sizes (in bytes) of the serialized snapshot parts, matching the Windows x64
// Direct2D struct layout. A WHS2 file is a 12-byte header followed by one of
// these payloads; a WHM2 movie is a 40-byte header followed by a run of them.
enum : size_t {
    SNAPSHOT_STATE_SIZE = 40,   // DRAW_STATE
    SNAPSHOT_GLOBAL_SIZE = 104, // GLOB_RING
    SNAPSHOT_RING_SIZE = 192,   // RING
    SNAPSHOT_BODY_SIZE = 144,   // STATE + GLOBAL (without rings)
};

// Serializes a snapshot into the payload shared by WHS2 snapshot files and WHM2
// movie frames: state, global ring, then rings. The result is sized exactly
// SNAPSHOT_BODY_SIZE + SNAPSHOT_RING_SIZE * rings.size().
std::vector<uint8_t> encodeSnapshotBody(const Snapshot& s);

// Parses a payload produced by encodeSnapshotBody. `bytes` must point at
// `count` rings' worth of data. Returns false and sets `error` on malformed
// input, leaving `out` unchanged.
bool decodeSnapshotBody(const uint8_t* bytes, size_t count, Snapshot& out, std::string& error);

} // namespace whorld
