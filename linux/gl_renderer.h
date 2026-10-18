// GPU canvas renderer: draws Whorld frames with OpenGL, using the stencil
// buffer to reproduce cairo/Direct2D even-odd (alternate) fills exactly, and
// MSAA for antialiasing. Also provides programmable GLSL post-processing
// for kaleidoscope, quad mirroring, color inversion, and feedback trails.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "post_process.h"
#include "whorld_core.h"
#include <SDL.h>
#include <cairo/cairo.h>
#include <string>
#include <vector>

namespace whorld {

class GlRenderer {
public:
    GlRenderer() = default;
    ~GlRenderer();
    GlRenderer(const GlRenderer&) = delete;
    GlRenderer& operator=(const GlRenderer&) = delete;

    // The window must have been created with SDL_WINDOW_OPENGL. Returns false
    // and fills `error` when a usable context cannot be created.
    bool init(SDL_Window* window, std::string& error);
    bool ready() const { return context != nullptr; }

    // Draws the frame into the current framebuffer. The canvas occupies
    // [canvasX, canvasX+canvasW) x [0, canvasH) of a windowW x windowH window.
    // `superScale` rasterizes the canvas offscreen at that multiple of its
    // logical size and downsamples on compositing (supersampling); 1.0 disables
    // it. When `ui` is non-null it is uploaded and drawn over [0,uiW) x [0,uiH).
    // `pp` supplies post-processing options (mirror, invert, feedback trails).
    void render(const Frame& frame, bool antialias, int canvasX, int canvasW, int canvasH,
                int windowW, int windowH, cairo_surface_t* ui, int uiW, int uiH,
                double superScale = 1.0, const PostProcess& pp = {});

    // Renders a single frame into a w x h backbuffer and reads it back as
    // BGRA rows (w*h*4 bytes). Used for headless GPU renders. `superScale`
    // supersamples the canvas as in render().
    bool renderOffscreen(const Frame& frame, bool antialias, int w, int h,
                         std::vector<unsigned char>& bgra, double superScale = 1.0,
                         const PostProcess& pp = {});

    void swap(SDL_Window* window);
    void clearFeedback();

    // Draws the interactive warp-editor guide (a thin quad outline plus one
    // handle per corner) in screen space, on top of whatever render() left. Must
    // be called between render() and swap(), which leave a screen-space ortho in
    // place. `selected`/`hovered` are corner indices 0..3 (TL,TR,BR,BL) or -1.
    void drawWarpOverlay(const WarpQuad& quad, int selected, int hovered, int canvasX,
                         int canvasW, int canvasH, const Color& color);

    // Releases the GL context, shaders, and textures. Safe to call more than once,
    // and must run before SDL_Quit.
    void shutdown();

private:
    void drawCanvas(const Frame& frame, bool antialias, int canvasX, int canvasW, int canvasH,
                    int blendMode);
    bool ensureFbo(int w, int h);
    void uploadUi(cairo_surface_t* surface, int w, int h);
    SDL_GLContext context = nullptr;
    unsigned uiTexture = 0;
    int uiW = 0, uiH = 0;
    bool msaa = false;
    unsigned fbo = 0, fboColor = 0, fboDepthStencil = 0;
    unsigned fboMs = 0, msColor = 0, msDepthStencil = 0;
    int fboW = 0, fboH = 0, fboSamples = 0;

    // Post-processing GLSL shader and feedback ping-pong FBOs
    unsigned postProgram = 0;
    int uLocTexture = -1;
    int uLocFeedbackTex = -1;
    int uLocMirrorMode = -1;
    int uLocInvert = -1;
    int uLocFeedback = -1;
    int uLocFeedbackZoom = -1;
    int uLocFeedbackRot = -1;
    int uLocCenter = -1;
    int uLocBackgroundColor = -1;
    int uLocWarp = -1;
    int uLocWarpEnabled = -1;

    unsigned feedbackFbo[2] = {0, 0};
    unsigned feedbackColor[2] = {0, 0};
    int feedbackPing = 0;
};

} // namespace whorld
