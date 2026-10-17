// Projection mapping: a homography that warps the rendered image from its
// rectangle onto an arbitrary convex quadrilateral, so a projector aimed at an
// off-axis surface can be corrected. Pure math with no GL/SDL dependency, so
// the OpenGL post pass and the cairo fallback can sample through the same
// transform and produce identical output.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "whorld_core.h"
#include <string>

namespace whorld {

// Normalized 0..1 output coordinates with y increasing downward, matching the
// display and the cairo image surface:
//   tl (0,0)  tr (1,0)
//   bl (0,1)  br (1,1)
// The identity warp is the unit square.
struct WarpQuad {
    Point tl{0, 0}, tr{1, 0}, br{1, 1}, bl{0, 1};
};

// Homography mapping the unit square (TL(0,0), TR(1,0), BR(1,1), BL(0,1)) onto
// `quad`, row-major 3x3 (h[0..2] is the first row). Returns false (leaving h
// untouched) for a degenerate quad: near-zero area or a singular mapping.
bool quadToHomography(const WarpQuad& quad, double h[9]);

// The inverse homography (destination screen uv -> source content uv), which is
// what the renderer samples with. Returns false for a degenerate quad.
bool quadToInverseHomography(const WarpQuad& quad, double h[9]);

// Applies a homography to a point, with the homogeneous divide. Returns false
// when the point maps to infinity (zero w).
bool projectPoint(const double h[9], Point p, Point& out);

// Inverts a row-major 3x3. Returns false when it is singular.
bool invert3x3(const double h[9], double out[9]);

// True when `quad` is a usable, non-degenerate projection mapping.
bool validWarp(const WarpQuad& quad);

// Text (de)serialization as four "x y" pairs in TL, TR, BR, BL order. The
// parser accepts whitespace or comma separators and tolerates newlines; it
// rejects any input that is not exactly eight finite numbers. Returns false and
// leaves `quad` untouched on malformed input.
std::string warpToString(const WarpQuad& quad);
bool warpFromString(const std::string& text, WarpQuad& quad);

} // namespace whorld
