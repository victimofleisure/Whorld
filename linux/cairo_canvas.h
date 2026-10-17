// Cairo canvas rasterizer, shared by the SDL/cairo fallback frontend and the
// GPU-vs-cairo regression test so both draw frames identically.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "post_process.h"
#include "whorld_core.h"
#include <cairo/cairo.h>

namespace whorld {

// Rasterizes a frame into the current cairo context at w x h logical pixels,
// filling each ring with the even-odd (alternate) rule and stroking its
// outline, exactly as the Direct2D original does.
void drawFrameCairo(cairo_t* cr, const Frame& frame, int w, int h, bool antialias);

// Applies projection warp, mirror and/or color inversion to an ARGB32 image
// surface in place, matching the GLSL post-process shader used by GlRenderer
// (the shader is the reference). When the warp maps an output pixel outside the
// source, `background` is written so the projector corners stay clean.
// `feedback` is intentionally not handled here: feedback trails are GPU-only and
// are left untouched. A no-op for non-ARGB32 or non-image surfaces.
void postProcessCairo(cairo_surface_t* surface, const PostProcess& pp,
                      const Color& background = Color{});

} // namespace whorld
