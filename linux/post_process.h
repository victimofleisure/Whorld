// Post-processing options shared by the OpenGL and cairo canvas paths. Kept in
// its own header, with no SDL/OpenGL dependency, so cairo_canvas can reproduce
// mirror/invert without pulling in the GPU renderer. The fields and defaults
// mirror the GLSL post-process shader uniforms in gl_renderer.cpp.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "projection.h"
#include "whorld_core.h"

namespace whorld {

struct PostProcess {
    int mirrorMode = 0;        // 0: none, 1: 4-way quad mirror, 2: 8-way kaleidoscope, 3: 12-way radial
    bool invertColor = false;  // invert RGB
    // Geometry compositing mode used while rasterizing the rings (0: Normal,
    // 1: Additive, 2: Screen). Normal keeps the original alpha-over output, so
    // the default leaves every existing render unchanged.
    int blendMode = 0;
    double feedback = 0.0;     // 0.0 (off) to 0.99
    double feedbackZoom = 1.0; // zoom for feedback trails (1.0 = none)
    double feedbackRot = 0.0;  // rotation for feedback trails (0.0 = none)
    Point center = {0.5, 0.5}; // normalized center
    WarpQuad warp{};           // quadrilateral projection map (identity default)
    bool warpEnabled = false;  // disabled by default, so nothing changes
};

} // namespace whorld
