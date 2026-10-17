// Projection mapping (quad -> homography) implementation. See projection.h.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "projection.h"
#include <cmath>
#include <cstdio>
#include <sstream>

namespace whorld {
namespace {

// Absolute tolerance for determinants / homogeneous w. The quads are normalized
// to 0..1, so these are comfortably above double rounding noise yet far below
// any meaningful mapping.
constexpr double kEps = 1e-9;

double det3x3(const double h[9]) {
    return h[0] * (h[4] * h[8] - h[5] * h[7]) -
           h[1] * (h[3] * h[8] - h[5] * h[6]) +
           h[2] * (h[3] * h[7] - h[4] * h[6]);
}

} // namespace

bool quadToHomography(const WarpQuad& q, double h[9]) {
    const double x0 = q.tl.x, y0 = q.tl.y;
    const double x1 = q.tr.x, y1 = q.tr.y;
    const double x2 = q.br.x, y2 = q.br.y;
    const double x3 = q.bl.x, y3 = q.bl.y;

    // Corners (0,0) and (1,0) fix the translation column: m02 = x0, m12 = y0.
    const double m02 = x0, m12 = y0, m22 = 1.0;
    double m00, m01, m10, m11, m20, m21;

    // A parallelogram has no projective terms, so the general 2x2 solve below
    // would be 0/0; take the affine branch directly.
    const double sx = x0 - x1 + x2 - x3;
    const double sy = y0 - y1 + y2 - y3;
    if (std::abs(sx) < kEps && std::abs(sy) < kEps) {
        m00 = x1 - x0;
        m01 = x3 - x0;
        m10 = y1 - y0;
        m11 = y3 - y0;
        m20 = 0.0;
        m21 = 0.0;
    } else {
        // Solve [dx1 dx2; dy1 dy2] * (m20, m21) = (sx, sy).
        const double dx1 = x1 - x2, dx2 = x3 - x2;
        const double dy1 = y1 - y2, dy2 = y3 - y2;
        const double det = dx1 * dy2 - dx2 * dy1;
        if (std::abs(det) < kEps) return false;
        m20 = (sx * dy2 - dx2 * sy) / det;
        m21 = (dx1 * sy - sx * dy1) / det;
        m00 = x1 - x0 + m20 * x1;
        m01 = x3 - x0 + m21 * x3;
        m10 = y1 - y0 + m20 * y1;
        m11 = y3 - y0 + m21 * y3;
    }

    double out[9] = {m00, m01, m02, m10, m11, m12, m20, m21, m22};
    if (!std::isfinite(det3x3(out))) return false;
    if (std::abs(det3x3(out)) < kEps) return false;
    for (int i = 0; i < 9; ++i) h[i] = out[i];
    return true;
}

bool quadToInverseHomography(const WarpQuad& quad, double h[9]) {
    double forward[9];
    if (!quadToHomography(quad, forward)) return false;
    return invert3x3(forward, h);
}

bool projectPoint(const double h[9], Point p, Point& out) {
    const double w = h[6] * p.x + h[7] * p.y + h[8];
    if (std::abs(w) < 1e-12) return false;
    out.x = (h[0] * p.x + h[1] * p.y + h[2]) / w;
    out.y = (h[3] * p.x + h[4] * p.y + h[5]) / w;
    return true;
}

bool invert3x3(const double h[9], double out[9]) {
    // Adjugate (transposed cofactor matrix) divided by the determinant.
    const double aj = h[4] * h[8] - h[5] * h[7];
    const double bj = h[3] * h[8] - h[5] * h[6];
    const double cj = h[3] * h[7] - h[4] * h[6];
    const double det = h[0] * aj - h[1] * bj + h[2] * cj;
    if (std::abs(det) < kEps) return false;
    const double inv = 1.0 / det;
    out[0] = aj * inv;
    out[1] = -(h[1] * h[8] - h[2] * h[7]) * inv;
    out[2] = (h[1] * h[5] - h[2] * h[4]) * inv;
    out[3] = -bj * inv;
    out[4] = (h[0] * h[8] - h[2] * h[6]) * inv;
    out[5] = -(h[0] * h[5] - h[2] * h[3]) * inv;
    out[6] = cj * inv;
    out[7] = -(h[0] * h[7] - h[1] * h[6]) * inv;
    out[8] = (h[0] * h[4] - h[1] * h[3]) * inv;
    return true;
}

bool validWarp(const WarpQuad& quad) {
    double h[9];
    return quadToHomography(quad, h);
}

std::string warpToString(const WarpQuad& q) {
    char buf[256];
    std::snprintf(buf, sizeof(buf), "%.17g %.17g\n%.17g %.17g\n%.17g %.17g\n%.17g %.17g\n",
                  q.tl.x, q.tl.y, q.tr.x, q.tr.y, q.br.x, q.br.y, q.bl.x, q.bl.y);
    return buf;
}

bool warpFromString(const std::string& text, WarpQuad& quad) {
    std::string normalized = text;
    for (char& c : normalized)
        if (c == ',') c = ' ';
    std::istringstream in(normalized);
    double v[8];
    for (int i = 0; i < 8; ++i) {
        if (!(in >> v[i])) return false;
    }
    double extra = 0;
    if (in >> extra) return false; // more than eight numbers
    for (double d : v) {
        if (!std::isfinite(d)) return false;
    }
    quad = WarpQuad{{v[0], v[1]}, {v[2], v[3]}, {v[4], v[5]}, {v[6], v[7]}};
    return true;
}

} // namespace whorld
