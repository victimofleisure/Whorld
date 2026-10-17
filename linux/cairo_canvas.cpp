// Cairo canvas rasterizer implementation.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "cairo_canvas.h"
#include <algorithm>

namespace whorld {
namespace {

void setColor(cairo_t* cr, Color c) {
    cairo_set_source_rgba(cr, std::clamp(c.r, 0.0, 1.0), std::clamp(c.g, 0.0, 1.0),
                          std::clamp(c.b, 0.0, 1.0), std::clamp(c.a, 0.0, 1.0));
}

void addPath(cairo_t* cr, const Path& p) {
    cairo_move_to(cr, p.start.x, p.start.y);
    for (const auto& s : p.segments) {
        if (s.curved) cairo_curve_to(cr, s.c1.x, s.c1.y, s.c2.x, s.c2.y, s.end.x, s.end.y);
        else cairo_line_to(cr, s.end.x, s.end.y);
    }
    cairo_close_path(cr);
}

} // namespace

void drawFrameCairo(cairo_t* cr, const Frame& f, int w, int h, bool antialias) {
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    setColor(cr, f.background);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_fill(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_antialias(cr, antialias ? CAIRO_ANTIALIAS_GOOD : CAIRO_ANTIALIAS_NONE);
    cairo_set_fill_rule(cr, CAIRO_FILL_RULE_EVEN_ODD);
    for (const Shape& s : f.shapes) {
        if (s.fill) {
            cairo_new_path(cr);
            addPath(cr, s.outer);
            if (s.hasInner) addPath(cr, s.inner);
            setColor(cr, s.color);
            cairo_fill_preserve(cr);
            setColor(cr, s.outline ? s.outlineColor : s.color);
            cairo_set_line_width(cr, s.outline ? std::max(0.1, s.lineWidth) : 1);
            cairo_stroke(cr);
        } else {
            cairo_new_path(cr);
            addPath(cr, s.outer);
            setColor(cr, s.color);
            cairo_set_line_width(cr, std::max(0.1, s.lineWidth));
            cairo_stroke(cr);
        }
    }
}

} // namespace whorld
