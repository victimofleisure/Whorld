// Unit test for the pure-math projection mapping in projection.cpp (the
// homography behind --warp). Links only whorld_core, so it always runs.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "projection.h"
#include "whorld_core.h"
#include <cmath>
#include <cstdio>
#include <string>

using namespace whorld;

#define require(expr)                                                                      \
    do {                                                                                   \
        if (!(expr)) {                                                                     \
            std::fprintf(stderr, "whorld_projection: failed: %s (line %d)\n", #expr,       \
                         __LINE__);                                                        \
            return 1;                                                                      \
        }                                                                                  \
    } while (false)

namespace {

bool near(double a, double b, double eps = 1e-9) { return std::fabs(a - b) <= eps; }
bool nearPt(Point a, Point b, double eps = 1e-9) {
    return near(a.x, b.x, eps) && near(a.y, b.y, eps);
}
// Twice the signed area of triangle (a, b, p); zero when p lies on line a-b.
double cross(Point a, Point b, Point p) {
    return (b.x - a.x) * (p.y - a.y) - (b.y - a.y) * (p.x - a.x);
}

} // namespace

int main() {
    const WarpQuad quad{{0.10, 0.20}, {0.90, 0.08}, {0.82, 0.94}, {0.14, 0.86}};

    // Default warp is the unit square and is valid.
    require(validWarp(WarpQuad{}));
    require(nearPt(WarpQuad{}.tl, {0, 0}) && nearPt(WarpQuad{}.br, {1, 1}));

    // Forward homography maps the four unit-square corners to the quad corners,
    // in TL, TR, BR, BL order.
    double h[9];
    require(quadToHomography(quad, h));
    Point p;
    require(projectPoint(h, {0, 0}, p) && nearPt(p, quad.tl));
    require(projectPoint(h, {1, 0}, p) && nearPt(p, quad.tr));
    require(projectPoint(h, {1, 1}, p) && nearPt(p, quad.br));
    require(projectPoint(h, {0, 1}, p) && nearPt(p, quad.bl));

    // The inverse round-trips against the forward map over the whole square.
    double inv[9];
    require(quadToInverseHomography(quad, inv));
    for (double y = 0.0; y <= 1.0 + 1e-12; y += 0.125) {
        for (double x = 0.0; x <= 1.0 + 1e-12; x += 0.125) {
            const Point src{x, y};
            Point dst, back;
            require(projectPoint(h, src, dst));
            require(projectPoint(inv, dst, back));
            require(nearPt(src, back, 1e-9));
        }
    }

    // The square centre maps to the quad centre (the intersection of its
    // diagonals), and the inverse brings it back to (0.5, 0.5).
    Point centre;
    require(projectPoint(h, {0.5, 0.5}, centre));
    require(std::fabs(cross(quad.tl, quad.br, centre)) < 1e-9);
    require(std::fabs(cross(quad.tr, quad.bl, centre)) < 1e-9);
    Point centreBack;
    require(projectPoint(inv, centre, centreBack));
    require(nearPt(centreBack, {0.5, 0.5}, 1e-9));
    // For the identity mapping the centre is exactly the square centre.
    double id[9];
    require(quadToHomography(WarpQuad{}, id));
    require(projectPoint(id, {0.5, 0.5}, p) && nearPt(p, {0.5, 0.5}));

    // invert3x3 agrees with quadToInverseHomography and rejects singular input.
    {
        double direct[9];
        require(invert3x3(h, direct));
        for (int i = 0; i < 9; ++i) require(near(direct[i], inv[i], 1e-12));
        const double singular[9] = {1, 2, 3, 2, 4, 6, 0, 0, 1};
        double out[9];
        require(!invert3x3(singular, out));
    }

    // Degenerate quads are rejected: all four corners equal, and collinear.
    {
        const WarpQuad collapsed{{0.3, 0.4}, {0.3, 0.4}, {0.3, 0.4}, {0.3, 0.4}};
        require(!validWarp(collapsed));
        double bad[9];
        require(!quadToHomography(collapsed, bad));
        require(!quadToInverseHomography(collapsed, bad));
    }
    {
        const WarpQuad collinear{{0.0, 0.0}, {0.25, 0.25}, {0.75, 0.75}, {1.0, 1.0}};
        require(!validWarp(collinear));
        double bad[9];
        require(!quadToHomography(collinear, bad));
    }

    // Text round-trip via warpToString/warpFromString, plus comma parsing and
    // the documented corner order.
    {
        const std::string text = warpToString(quad);
        WarpQuad parsed;
        require(warpFromString(text, parsed));
        require(nearPt(parsed.tl, quad.tl) && nearPt(parsed.tr, quad.tr) &&
                nearPt(parsed.br, quad.br) && nearPt(parsed.bl, quad.bl));

        WarpQuad commas;
        require(warpFromString("0.1,0.2 0.9,0.08 0.82,0.94 0.14,0.86", commas));
        require(nearPt(commas.tl, quad.tl) && nearPt(commas.bl, quad.bl));

        WarpQuad spaced;
        require(warpFromString("1 0\n1 1\n0 1\n0 0", spaced));
        require(nearPt(spaced.tl, {1, 0}) && nearPt(spaced.br, {0, 1}));
    }
    {
        // Malformed inputs must be rejected and leave the quad untouched.
        WarpQuad q{{0.1, 0.1}, {0.2, 0.2}, {0.3, 0.3}, {0.4, 0.4}};
        const WarpQuad before = q;
        require(!warpFromString("", q));
        require(!warpFromString("0 0 1 0 1 1 0", q));          // too few
        require(!warpFromString("0 0 1 0 1 1 0 1 2", q));      // too many
        require(!warpFromString("0 0 x 0 1 1 0 1", q));        // non-numeric
        require(nearPt(q.tl, before.tl) && nearPt(q.bl, before.bl));
    }

    // Interactive corner editing helpers used by the on-screen warp editor.
    {
        // warpCornerAt picks the nearest corner inside the radius and returns
        // None outside it.
        require(warpCornerAt(quad, {0.11, 0.21}, 0.05) == WarpCorner::TL);
        require(warpCornerAt(quad, {0.89, 0.09}, 0.05) == WarpCorner::TR);
        require(warpCornerAt(quad, {0.81, 0.93}, 0.05) == WarpCorner::BR);
        require(warpCornerAt(quad, {0.15, 0.85}, 0.05) == WarpCorner::BL);
        require(warpCornerAt(quad, {0.5, 0.5}, 0.05) == WarpCorner::None);
        // With several corners in range the closest one wins.
        require(warpCornerAt(quad, {0.48, 0.14}, 0.60) == WarpCorner::TL);
        require(warpCornerAt(quad, {0.52, 0.14}, 0.60) == WarpCorner::TR);

        // warpCornerPoint agrees with the quad members.
        require(nearPt(warpCornerPoint(quad, WarpCorner::TL), quad.tl));
        require(nearPt(warpCornerPoint(quad, WarpCorner::BL), quad.bl));

        // setWarpCorner moves only the requested corner.
        WarpQuad moved = quad;
        setWarpCorner(moved, WarpCorner::TR, {0.95, 0.02});
        require(nearPt(moved.tr, {0.95, 0.02}));
        require(nearPt(moved.tl, quad.tl) && nearPt(moved.br, quad.br) && nearPt(moved.bl, quad.bl));
        // Coordinates are clamped to the generous -0.5..1.5 range.
        setWarpCorner(moved, WarpCorner::TR, {5.0, -5.0});
        require(nearPt(moved.tr, {1.5, -0.5}));
        // None is a no-op.
        const WarpQuad beforeNone = moved;
        setWarpCorner(moved, WarpCorner::None, {0.25, 0.25});
        require(nearPt(moved.tr, beforeNone.tr) && nearPt(moved.tl, beforeNone.tl));

        // nudgeWarpCorner moves only the requested corner by the delta.
        WarpQuad nudged = quad;
        nudgeWarpCorner(nudged, WarpCorner::BL, 0.01, -0.02);
        require(nearPt(nudged.bl, {quad.bl.x + 0.01, quad.bl.y - 0.02}));
        require(nearPt(nudged.tl, quad.tl) && nearPt(nudged.tr, quad.tr) &&
                nearPt(nudged.br, quad.br));

        // snapWarpQuad snaps near-edge values and leaves interior values alone.
        WarpQuad snap{{0.004, 0.996}, {0.5, 0.3}, {1.0, 0.0}, {0.15, 0.995}};
        snapWarpQuad(snap);
        require(near(snap.tl.x, 0.0) && near(snap.tl.y, 1.0));
        require(near(snap.tr.x, 0.5) && near(snap.tr.y, 0.3));
        require(near(snap.br.x, 1.0) && near(snap.br.y, 0.0));
        require(near(snap.bl.x, 0.15) && near(snap.bl.y, 1.0));
        // A value just outside epsilon is untouched.
        WarpQuad keep{{0.02, 0.03}, {1.0, 1.0}, {1.0, 1.0}, {0.0, 0.0}};
        snapWarpQuad(keep, 0.01);
        require(near(keep.tl.x, 0.02) && near(keep.tl.y, 0.03));

        // identityWarp / isIdentityWarp.
        require(isIdentityWarp(identityWarp()));
        require(isIdentityWarp(WarpQuad{}));
        require(nearPt(identityWarp().tl, {0, 0}) && nearPt(identityWarp().bl, {0, 1}));
        require(!isIdentityWarp(quad));
        const WarpQuad almost{{0, 0}, {1, 0}, {1, 1}, {0, 1 + 1e-6}};
        require(!isIdentityWarp(almost, 1e-9));
        require(isIdentityWarp(almost, 1e-5));
    }

    std::printf("ok\n");
    return 0;
}
