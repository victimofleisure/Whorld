// Cairo canvas rasterizer implementation.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "cairo_canvas.h"
#include <algorithm>
#include <cmath>
#include <cstring>
#include <vector>

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

void drawFrameCairo(cairo_t* cr, const Frame& f, int w, int h, bool antialias, int blendMode) {
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    setColor(cr, f.background);
    cairo_rectangle(cr, 0, 0, w, h);
    cairo_fill(cr);
    // The background is painted opaquely with SOURCE; the shapes then composite
    // with the selected mode (Normal is the historical OVER).
    cairo_set_operator(cr, blendMode == 1 ? CAIRO_OPERATOR_ADD
                                          : blendMode == 2 ? CAIRO_OPERATOR_SCREEN
                                                           : CAIRO_OPERATOR_OVER);
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

void postProcessCairo(cairo_surface_t* surface, const PostProcess& pp, const Color& background) {
    // Feedback trails need per-frame history that the single-frame cairo path
    // does not keep; they remain a GPU-only effect (see main.cpp).
    const bool warp = pp.warpEnabled && validWarp(pp.warp);
    if (pp.mirrorMode == 0 && !pp.invertColor && !warp) return;
    if (!surface || cairo_surface_get_type(surface) != CAIRO_SURFACE_TYPE_IMAGE) return;
    if (cairo_image_surface_get_format(surface) != CAIRO_FORMAT_ARGB32) return;
    const int w = cairo_image_surface_get_width(surface);
    const int h = cairo_image_surface_get_height(surface);
    if (w <= 0 || h <= 0) return;
    const int stride = cairo_image_surface_get_stride(surface);
    cairo_surface_flush(surface);
    unsigned char* data = cairo_image_surface_get_data(surface);

    if (pp.mirrorMode != 0 || warp) {
        // Copy the source: resampling reads arbitrary pixels while we overwrite
        // the surface in place.
        std::vector<unsigned char> src(data, data + std::size_t(stride) * h);

        // Bilinear sample of the source at normalized coordinate (u, v), using
        // the same texel-center convention and clamp-to-edge behaviour as the
        // GL texture sampler (GL_LINEAR / GL_CLAMP_TO_EDGE).
        auto sample = [&](double u, double v, unsigned char* out) {
            const double sx = u * w - 0.5;
            const double sy = v * h - 0.5;
            const int x0 = static_cast<int>(std::floor(sx));
            const int y0 = static_cast<int>(std::floor(sy));
            const double fx = sx - x0, fy = sy - y0;
            for (int c = 0; c < 4; ++c) {
                double acc = 0;
                for (int j = 0; j <= 1; ++j) {
                    for (int i = 0; i <= 1; ++i) {
                        const int xx = std::clamp(x0 + i, 0, w - 1);
                        const int yy = std::clamp(y0 + j, 0, h - 1);
                        const double weight = (i ? fx : 1 - fx) * (j ? fy : 1 - fy);
                        acc += weight * src[std::size_t(yy) * stride + std::size_t(xx) * 4 + c];
                    }
                }
                out[c] = static_cast<unsigned char>(std::clamp(std::lround(acc), 0L, 255L));
            }
        };

        // The inverse homography maps an output pixel to its source coordinate
        // (destination -> source), in the same top-down normalized space as
        // WarpQuad. The shader applies it in the GL bottom-up texture space, so
        // the warp/mirror arithmetic below also runs bottom-up and only the
        // final sample flips v back to the top-down cairo surface.
        double inv[9];
        const bool haveWarp = warp && quadToInverseHomography(pp.warp, inv);

        // applyMirror() transcribed from the GLSL shader, in bottom-up space.
        auto mirror = [&](double x, double y, double& mu, double& mv) {
            mu = x;
            mv = y;
            if (pp.mirrorMode == 1) {
                mu = pp.center.x + std::abs(x - pp.center.x);
                mv = pp.center.y + std::abs(y - pp.center.y);
            } else if (pp.mirrorMode >= 2) {
                const double slices = (pp.mirrorMode == 2) ? 8.0 : 12.0;
                const double px = x - pp.center.x;
                const double py = y - pp.center.y;
                const double r = std::hypot(px, py);
                const double slice = M_PI / slices;
                double a = std::atan2(py, px);
                a = std::fmod(a, 2.0 * slice);
                if (a < 0) a += 2.0 * slice;
                if (a > slice) a = 2.0 * slice - a;
                mu = pp.center.x + std::cos(a) * r;
                mv = pp.center.y + std::sin(a) * r;
            }
        };

        auto putBackground = [&](int x, int y) {
            unsigned char* dst = data + std::size_t(y) * stride + std::size_t(x) * 4;
            dst[0] = static_cast<unsigned char>(
                std::clamp(std::lround(background.b * 255.0), 0L, 255L));
            dst[1] = static_cast<unsigned char>(
                std::clamp(std::lround(background.g * 255.0), 0L, 255L));
            dst[2] = static_cast<unsigned char>(
                std::clamp(std::lround(background.r * 255.0), 0L, 255L));
            dst[3] = 255;
        };

        // The result is the texture coordinate to read. For the plain mirror
        // path the coordinate is built directly from the top-down output pixel
        // (and the vertical component is flipped when sampling the top-down
        // cairo surface, matching the bottom-up GL framebuffer texture).
        for (int y = 0; y < h; ++y) {
            const double cy = (y + 0.5) / h;
            for (int x = 0; x < w; ++x) {
                const double cx = (x + 0.5) / w;
                unsigned char pixel[4];
                if (haveWarp) {
                    Point srcPt;
                    if (!projectPoint(inv, {cx, cy}, srcPt) ||
                        srcPt.x < 0.0 || srcPt.x > 1.0 || srcPt.y < 0.0 || srcPt.y > 1.0) {
                        putBackground(x, y); // outside the mapped rectangle
                        continue;
                    }
                    double mu, mv;
                    mirror(srcPt.x, 1.0 - srcPt.y, mu, mv);
                    sample(mu, 1.0 - mv, pixel);
                } else {
                    double u = cx, v = cy;
                    mirror(cx, cy, u, v);
                    sample(u, 1.0 - v, pixel);
                }
                std::memcpy(data + std::size_t(y) * stride + std::size_t(x) * 4, pixel, 4);
            }
        }
    }

    if (pp.invertColor) {
        // ARGB32 is premultiplied; the canvas is fully opaque so inverting the
        // colour channels directly matches the shader's 1.0 - rgb. Alpha is
        // left alone, as in the shader.
        for (int y = 0; y < h; ++y) {
            unsigned char* row = data + std::size_t(y) * stride;
            for (int x = 0; x < w; ++x) {
                row[x * 4 + 0] = 255 - row[x * 4 + 0];
                row[x * 4 + 1] = 255 - row[x * 4 + 1];
                row[x * 4 + 2] = 255 - row[x * 4 + 2];
            }
        }
    }
    cairo_surface_mark_dirty(surface);
}

} // namespace whorld
