// Cairo canvas rasterizer, shared by the SDL/cairo fallback frontend and the
// GPU-vs-cairo regression test so both draw frames identically.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "whorld_core.h"
#include <cairo/cairo.h>

namespace whorld {

// Rasterizes a frame into the current cairo context at w x h logical pixels,
// filling each ring with the even-odd (alternate) rule and stroking its
// outline, exactly as the Direct2D original does.
void drawFrameCairo(cairo_t* cr, const Frame& frame, int w, int h, bool antialias);

} // namespace whorld
