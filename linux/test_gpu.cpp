// Regression test: the OpenGL canvas renderer (stencil even-odd fills and
// miter-joined outlines) must match the cairo reference closely. Returns 77
// (ctest SKIP) when no OpenGL context can be created, e.g. on a headless CI.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "cairo_canvas.h"
#include "gl_renderer.h"
#include "whorld_core.h"
#include <SDL.h>
#include <cairo/cairo.h>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

using namespace whorld;

namespace {

struct Result {
    double mean = 0;
    double bigPct = 0;
};

// Both buffers hold BGRA bytes; cairo is top-down and glReadPixels bottom-up.
Result compare(cairo_surface_t* cairoSurface, const std::vector<unsigned char>& gpu,
               int w, int h) {
    cairo_surface_flush(cairoSurface);
    const unsigned char* c = cairo_image_surface_get_data(cairoSurface);
    const int stride = cairo_image_surface_get_stride(cairoSurface);
    long sum = 0, big = 0;
    const long n = static_cast<long>(w) * h;
    for (int y = 0; y < h; ++y) {
        const unsigned char* cr = c + static_cast<long>(y) * stride;
        const unsigned char* gr = gpu.data() + static_cast<long>(h - 1 - y) * w * 4;
        for (int x = 0; x < w; ++x) {
            const int d = std::abs(cr[0] - gr[0]) + std::abs(cr[1] - gr[1]) +
                          std::abs(cr[2] - gr[2]);
            sum += d;
            if (d > 24) ++big;
            cr += 4;
            gr += 4;
        }
    }
    return {sum / static_cast<double>(n), 100.0 * big / static_cast<double>(n)};
}

// Renders one case with cairo and with OpenGL, then checks the difference.
bool checkCase(const std::string& name, const Patch& patch, int width, int height, int frames,
               bool antialias, GlRenderer& gpu, std::string& detail,
               const PostProcess& pp = {}) {
    Engine engine;
    engine.setPatch(patch);
    for (int i = 0; i < frames; ++i) {
        engine.tick(width, height);
        engine.frame(width, height); // mirrors --render: retire offscreen rings
    }
    const Frame frame = engine.frame(width, height);

    cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, width, height);
    cairo_t* cr = cairo_create(surface);
    drawFrameCairo(cr, frame, width, height, antialias);
    cairo_destroy(cr);
    // The cairo fallback applies the same warp/mirror/invert post-processing;
    // the GPU shader is the reference.
    postProcessCairo(surface, pp, frame.background);

    std::vector<unsigned char> pixels;
    if (!gpu.renderOffscreen(frame, antialias, width, height, pixels, 1.0, pp)) {
        cairo_surface_destroy(surface);
        detail = name + ": GPU render failed";
        return false;
    }
    const Result r = compare(surface, pixels, width, height);
    cairo_surface_destroy(surface);

    char buf[160];
    std::snprintf(buf, sizeof(buf), "%s: mean=%.3f big=%.3f%% (frames=%d)", name.c_str(), r.mean,
                  r.bigPct, frames);
    detail = buf;
    // Plain cases keep the original strict bound (generous enough for driver
    // differences in MSAA resolve, strict enough to catch geometry/even-odd
    // regressions). Mirror/kaleidoscope resampling runs through different
    // samplers on the two backends (GL_LINEAR vs our CPU bilinear) and folds
    // many output pixels onto the same source texels, so seams and edges differ
    // slightly more; the looser post bound still catches a wrong fold or centre
    // (which pushes the mean well past 6).
    const bool hasPost = pp.mirrorMode != 0 || pp.invertColor || pp.warpEnabled;
    return hasPost ? (r.mean < 4.5 && r.bigPct < 4.5) : (r.mean < 4.0 && r.bigPct < 3.0);
}

} // namespace

int main(int argc, char** argv) {
    const std::string patchPath = argc > 1 ? argv[1] : "linux/demo.whp";
    Patch patch;
    std::string error;
    if (!patch.load(patchPath, error)) {
        std::fprintf(stderr, "whorld_gpu_test: cannot load %s: %s\n", patchPath.c_str(),
                     error.c_str());
        return 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::printf("SKIP: SDL video unavailable (%s)\n", SDL_GetError());
        return 77;
    }
    constexpr int width = 480, height = 360;
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_Window* window = SDL_CreateWindow("whorld-gpu-test", 0, 0, width, height,
                                          SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
    if (!window) {
        std::printf("SKIP: no OpenGL window (%s)\n", SDL_GetError());
        SDL_Quit();
        return 77;
    }
    GlRenderer gpu;
    std::string glError;
    if (!gpu.init(window, glError)) {
        std::printf("SKIP: no OpenGL context (%s)\n", glError.c_str());
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 77;
    }

    bool ok = true;
    std::string detail;
    for (int frames : {10, 30, 60}) {
        ok = checkCase("antialiased", patch, width, height, frames, true, gpu, detail) && ok;
        std::printf("%s\n", detail.c_str());
    }
    Patch flat = patch;
    flat.antialiasing = false;
    ok = checkCase("hard edges", flat, width, height, 30, false, gpu, detail) && ok;
    std::printf("%s\n", detail.c_str());

    // Post-processing must match the GLSL shader. Mirror modes are tested with
    // and without colour inversion; the mirrored output is vertically
    // symmetric, so it is insensitive to the GL framebuffer row order, while an
    // invert-only case would expose that pre-existing GPU orientation quirk.
    for (int mode : {1, 2, 3}) {
        for (bool invert : {false, true}) {
            PostProcess pp;
            pp.mirrorMode = mode;
            pp.invertColor = invert;
            char name[64];
            std::snprintf(name, sizeof(name), "mirror mode %d%s", mode,
                          invert ? " + invert" : "");
            ok = checkCase(name, patch, width, height, 30, true, gpu, detail, pp) && ok;
            std::printf("%s\n", detail.c_str());
        }
    }

    // Invert-only (no mirror fold) must not be vertically mirrored by the post
    // pass: a row-order flip shows up on screen and would double-flip feedback.
    {
        PostProcess pp;
        pp.invertColor = true;
        ok = checkCase("invert only", patch, width, height, 30, true, gpu, detail, pp) && ok;
        std::printf("%s\n", detail.c_str());
    }

    // Projection mapping: the GPU inverse homography and the CPU one must agree
    // for a mild keystone and a strongly skewed convex quad. The warp maps
    // output pixels to source coordinates and paints the frame background
    // outside the quad, so the two backends must also agree on those corners.
    {
        PostProcess pp;
        pp.warpEnabled = true;
        pp.warp = {{0.08, 0.02}, {0.94, 0.10}, {0.88, 0.97}, {0.12, 0.90}};
        ok = checkCase("warp keystone", patch, width, height, 30, true, gpu, detail, pp) && ok;
        std::printf("%s\n", detail.c_str());
    }
    {
        PostProcess pp;
        pp.warpEnabled = true;
        pp.warp = {{0.22, 0.15}, {0.85, 0.03}, {0.98, 0.88}, {0.02, 0.82}};
        ok = checkCase("warp skew", patch, width, height, 30, true, gpu, detail, pp) && ok;
        std::printf("%s\n", detail.c_str());
    }

    gpu.shutdown();
    SDL_DestroyWindow(window);
    SDL_Quit();
    if (!ok) {
        std::fprintf(stderr, "whorld_gpu_test: GPU output diverges from cairo\n");
        return 1;
    }
    std::printf("ok\n");
    return 0;
}
