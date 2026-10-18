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

// Fills the surface with a deterministic ramp: R = y*w + x, G = B = 0, opaque.
void fillRamp(cairo_surface_t* s, int w, int h) {
    for (int y = 0; y < h; ++y) {
        for (int x = 0; x < w; ++x) {
            unsigned char* p = pixel(s, x, y);
            p[0] = 0;
            p[1] = 0;
            p[2] = static_cast<unsigned char>(y * w + x);
            p[3] = 255;
        }
    }
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

    // mirrorMode 1 with a non-centre pp.center: the fold is
    // p = center + |p - center|, so this 4x4 ramp reads exactly these source
    // indices (y*w + x). Compare the centre row/column with the centre-0.5
    // case above to see the fold move with pp.center.
    {
        const int w = 4, h = 4;
        cairo_surface_t* s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
        fillRamp(s, w, h);
        PostProcess pp;
        pp.mirrorMode = 1;
        pp.center = {0.375, 0.375};
        postProcessCairo(s, pp);
        const int expected[h][w] = {
            {6, 5, 6, 7},
            {10, 9, 10, 11},
            {6, 5, 6, 7},
            {2, 1, 2, 3},
        };
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                if (pixel(s, x, y)[2] != expected[y][x]) {
                    char msg[160];
                    std::snprintf(msg, sizeof(msg),
                                  "mode 1 (non-centre) wrong at (%d,%d): got %d want %d", x, y,
                                  pixel(s, x, y)[2], expected[y][x]);
                    cairo_surface_destroy(s);
                    return fail(msg);
                }
            }
        }
        cairo_surface_destroy(s);
    }

    // mirrorMode 2 (8-way) and 3 (12-way) at pp.center 0.5. On the row and
    // column through the centre the reflected angle lands exactly on a slice
    // boundary, so the mapping is exact and both modes read the same table:
    // the centre row/column become a horizontal mirror of the centre source row
    // for x/y left of centre and the identity for x/y right of centre (index
    // y*w + x). This pins the centre-adjacent symmetry without any GPU.
    for (int mode = 2; mode <= 3; ++mode) {
        const int n = 5;
        cairo_surface_t* s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, n, n);
        fillRamp(s, n, n);
        PostProcess pp;
        pp.mirrorMode = mode;
        postProcessCairo(s, pp);
        const int centre[n] = {14, 13, 12, 13, 14}; // = source (4,2),(3,2),(2,2),(3,2),(4,2)
        for (int i = 0; i < n; ++i) {
            if (pixel(s, i, 2)[2] != centre[i]) {
                char msg[160];
                std::snprintf(msg, sizeof(msg),
                              "mode %d centre row wrong at x=%d: got %d want %d", mode, i,
                              pixel(s, i, 2)[2], centre[i]);
                cairo_surface_destroy(s);
                return fail(msg);
            }
            if (pixel(s, 2, i)[2] != centre[i]) {
                char msg[160];
                std::snprintf(msg, sizeof(msg),
                              "mode %d centre column wrong at y=%d: got %d want %d", mode, i,
                              pixel(s, 2, i)[2], centre[i]);
                cairo_surface_destroy(s);
                return fail(msg);
            }
        }
        cairo_surface_destroy(s);
    }

    // warp alone: a pure translation by (0.125, 0.125) maps every output pixel
    // inside the quad to source (x-1, y-1) exactly (the sample coordinate lands
    // on a texel centre, so there is no interpolation); the top row and left
    // column fall outside and are filled with the blue background.
    {
        const int w = 8, h = 8;
        cairo_surface_t* s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
        fillRamp(s, w, h);
        PostProcess pp;
        pp.warpEnabled = true;
        pp.warp = WarpQuad{{0.125, 0.125}, {1.125, 0.125}, {1.125, 1.125}, {0.125, 1.125}};
        postProcessCairo(s, pp, Color{0, 0, 1, 1}); // blue background
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                unsigned char* p = pixel(s, x, y);
                const bool outside = (x == 0 || y == 0);
                const unsigned char wantR =
                    outside ? 0 : static_cast<unsigned char>((y - 1) * w + (x - 1));
                const unsigned char wantB = outside ? 255 : 0;
                if (p[2] != wantR || p[0] != wantB || p[1] != 0 || p[3] != 255) {
                    char msg[160];
                    std::snprintf(msg, sizeof(msg),
                                  "warp translation wrong at (%d,%d): got BGRa %d,%d,%d,%d want "
                                  "%d,0,%d,255",
                                  x, y, p[0], p[1], p[2], p[3], wantB, wantR);
                    cairo_surface_destroy(s);
                    return fail(msg);
                }
            }
        }
        cairo_surface_destroy(s);
    }

    // warp scaling: the quad maps the unit square onto its lower-left quadrant
    // (0..0.5), magnifying the image 2x. With a source ramp R = x the bilinear
    // sample lands at sx = 2x + 0.5, exactly halfway between two source texels,
    // so the rounded value is 2x + 1 (lround rounds half away from zero); output
    // pixels outside the quad are the green background.
    {
        const int w = 8, h = 8;
        cairo_surface_t* s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                unsigned char* p = pixel(s, x, y);
                p[0] = 0;
                p[1] = 0;
                p[2] = static_cast<unsigned char>(x);
                p[3] = 255;
            }
        }
        PostProcess pp;
        pp.warpEnabled = true;
        pp.warp = WarpQuad{{0, 0}, {0.5, 0}, {0.5, 0.5}, {0, 0.5}};
        postProcessCairo(s, pp, Color{0, 1, 0, 1}); // green background
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                unsigned char* p = pixel(s, x, y);
                const bool outside = (x >= 4 || y >= 4);
                const unsigned char wantR = outside ? 0 : static_cast<unsigned char>(2 * x + 1);
                const unsigned char wantG = outside ? 255 : 0;
                if (p[2] != wantR || p[1] != wantG || p[0] != 0 || p[3] != 255) {
                    char msg[160];
                    std::snprintf(msg, sizeof(msg),
                                  "warp scaling wrong at (%d,%d): got BGRa %d,%d,%d,%d want "
                                  "0,%d,%d,255",
                                  x, y, p[0], p[1], p[2], p[3], wantG, wantR);
                    cairo_surface_destroy(s);
                    return fail(msg);
                }
            }
        }
        cairo_surface_destroy(s);
    }

    // Combination: translation warp + 4-way mirror + invert on an 8x8 ramp. The
    // source index each output pixel reads *before* inversion is the table below
    // (-1 = outside the quad, filled with the (black) background). The mirror
    // folds about the centre after the warp, and invert then flips every channel
    // (so the background becomes white and each ramp value v becomes 255 - v).
    {
        const int w = 8, h = 8;
        cairo_surface_t* s = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
        fillRamp(s, w, h);
        PostProcess pp;
        pp.warpEnabled = true;
        pp.warp = WarpQuad{{0.125, 0.125}, {1.125, 0.125}, {1.125, 1.125}, {0.125, 1.125}};
        pp.mirrorMode = 1;
        pp.invertColor = true;
        postProcessCairo(s, pp);
        const int expected[h][w] = {
            {-1, -1, -1, -1, -1, -1, -1, -1},
            {-1, 7, 6, 5, 4, 4, 5, 6},
            {-1, 15, 14, 13, 12, 12, 13, 14},
            {-1, 23, 22, 21, 20, 20, 21, 22},
            {-1, 31, 30, 29, 28, 28, 29, 30},
            {-1, 31, 30, 29, 28, 28, 29, 30},
            {-1, 23, 22, 21, 20, 20, 21, 22},
            {-1, 15, 14, 13, 12, 12, 13, 14},
        };
        for (int y = 0; y < h; ++y) {
            for (int x = 0; x < w; ++x) {
                unsigned char* p = pixel(s, x, y);
                const int src = expected[y][x];
                const unsigned char wantR =
                    src < 0 ? 255 : static_cast<unsigned char>(255 - src);
                if (p[2] != wantR || p[1] != 255 || p[0] != 255 || p[3] != 255) {
                    char msg[160];
                    std::snprintf(msg, sizeof(msg),
                                  "warp+mirror+invert wrong at (%d,%d): got BGRa %d,%d,%d,%d want "
                                  "255,255,%d,255",
                                  x, y, p[0], p[1], p[2], p[3], wantR);
                    cairo_surface_destroy(s);
                    return fail(msg);
                }
            }
        }
        cairo_surface_destroy(s);
    }

    std::printf("ok\n");
    return 0;
}
