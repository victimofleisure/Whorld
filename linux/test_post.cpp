// CPU-only unit test for the cairo fallback post-processing in cairo_canvas.cpp.
// Unlike whorld_gpu (which returns 77 without an OpenGL context), this links
// only cairo, so it always runs and pins the mirror/invert behaviour
// deterministically without relying on a GPU.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "cairo_canvas.h"
#include "post_process.h"
#include <cairo/cairo.h>
#include <cstddef>
#include <cstdio>

using namespace whorld;

namespace {

// ARGB32 is premultiplied 0xAARRGGBB in native order, i.e. B,G,R,A bytes.
unsigned char* pixel(cairo_surface_t* s, int x, int y) {
    return cairo_image_surface_get_data(s) +
           std::size_t(y) * cairo_image_surface_get_stride(s) + std::size_t(x) * 4;
}

int fail(const char* what) {
    std::fprintf(stderr, "whorld_post: %s\n", what);
    return 1;
}

} // namespace

int main() {
    // invertColor flips R, G and B; alpha is left untouched.
    {
        cairo_surface_t* s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 2, 2);
        unsigned char* p = pixel(s, 0, 0);
        p[0] = 10; p[1] = 20; p[2] = 30; p[3] = 200;
        PostProcess pp;
        pp.invertColor = true;
        postProcessCairo(s, pp);
        p = pixel(s, 0, 0);
        if (p[0] != 245 || p[1] != 235 || p[2] != 225 || p[3] != 200) {
            cairo_surface_destroy(s);
            return fail("invertColor did not invert R/G/B or changed alpha");
        }
        cairo_surface_destroy(s);
    }

    // mirrorMode 1 (4-way quad) on a 4x4 ramp. Output pixel (x,y) samples the
    // source at the 4-fold reflection of its centre. At this size the mirrored
    // coordinates land exactly on source pixel centres, so sampling is exact:
    // the expected R values below are the source indices (y*w+x) that each
    // output pixel reads. The vertical order follows the GL framebuffer texture
    // convention that the shader is the reference for (see postProcessCairo).
    {
        const int w = 4, h = 4;
        cairo_surface_t* s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                unsigned char* p = pixel(s, x, y);
                p[0] = 0; p[1] = 0; p[2] = static_cast<unsigned char>(y * w + x); p[3] = 255;
            }
        }
        PostProcess pp;
        pp.mirrorMode = 1;
        postProcessCairo(s, pp);
        const int expected[h][w] = {
            {3, 2, 2, 3},
            {7, 6, 6, 7},
            {7, 6, 6, 7},
            {3, 2, 2, 3},
        };
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (pixel(s, x, y)[2] != expected[y][x]) {
                    char msg[128];
                    std::snprintf(msg, sizeof(msg),
                                  "quad mirror wrong at (%d,%d): got %d want %d",
                                  x, y, pixel(s, x, y)[2], expected[y][x]);
                    cairo_surface_destroy(s);
                    return fail(msg);
                }
            }
        }
        cairo_surface_destroy(s);
    }

    // No-op without post-processing: the surface must be untouched.
    {
        cairo_surface_t* s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 2, 2);
        unsigned char* p = pixel(s, 1, 1);
        p[0] = 1; p[1] = 2; p[2] = 3; p[3] = 4;
        postProcessCairo(s, PostProcess{});
        p = pixel(s, 1, 1);
        if (p[0] != 1 || p[1] != 2 || p[2] != 3 || p[3] != 4) {
            cairo_surface_destroy(s);
            return fail("postProcessCairo modified the surface with no post-processing");
        }
        cairo_surface_destroy(s);
    }

    std::printf("ok\n");
    return 0;
}
