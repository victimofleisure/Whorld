// GPU canvas renderer implementation (OpenGL fixed function + stencil + GLSL post-processing).
// SPDX-License-Identifier: GPL-3.0-or-later
#define GL_GLEXT_PROTOTYPES 1
#include "gl_renderer.h"
#include <GL/gl.h>
#include <GL/glext.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE 0x809D
#endif
#ifndef GL_CLAMP_TO_EDGE
#define GL_CLAMP_TO_EDGE 0x812F
#endif
#ifndef GL_BGRA
#define GL_BGRA 0x80E1
#endif

namespace whorld {
namespace {

static const char* kPostVs =
    "#version 120\n"
    "varying vec2 vTexCoord;\n"
    "void main() {\n"
    "    vTexCoord = gl_MultiTexCoord0.xy;\n"
    "    gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;\n"
    "}\n";

static const char* kPostFs =
    "#version 120\n"
    "uniform sampler2D uTexture;\n"
    "uniform sampler2D uFeedbackTex;\n"
    "uniform int uMirrorMode;\n"
    "uniform int uInvert;\n"
    "uniform float uFeedback;\n"
    "uniform float uFeedbackZoom;\n"
    "uniform float uFeedbackRot;\n"
    "uniform vec2 uCenter;\n"
    "uniform vec3 uBackgroundColor;\n"
    "uniform mat3 uWarp;\n"
    "uniform int uWarpEnabled;\n"
    "varying vec2 vTexCoord;\n"
    "vec2 applyMirror(vec2 uv) {\n"
    "    if (uMirrorMode == 0) return uv;\n"
    "    vec2 p = uv - uCenter;\n"
    "    if (uMirrorMode == 1) {\n"
    "        p = abs(p);\n"
    "        return uCenter + p;\n"
    "    }\n"
    "    float slices = (uMirrorMode == 2) ? 8.0 : 12.0;\n"
    "    const float pi = 3.141592653589793;\n"
    "    float r = length(p);\n"
    "    float a = atan(p.y, p.x);\n"
    "    float slice = pi / slices;\n"
    "    a = mod(a, 2.0 * slice);\n"
    "    if (a > slice) a = 2.0 * slice - a;\n"
    "    return uCenter + vec2(cos(a), sin(a)) * r;\n"
    "}\n"
    "void main() {\n"
    "    vec2 uv = vTexCoord;\n"
    "    if (uWarpEnabled != 0) {\n"
    "        vec3 s = uWarp * vec3(uv, 1.0);\n"
    "        uv = s.xy / s.z;\n"
    "        if (uv.x < 0.0 || uv.x > 1.0 || uv.y < 0.0 || uv.y > 1.0) {\n"
    "            gl_FragColor = vec4(uBackgroundColor, 1.0);\n"
    "            return;\n"
    "        }\n"
    "    }\n"
    "    uv = applyMirror(uv);\n"
    "    vec4 cur = texture2D(uTexture, uv);\n"
    "    vec4 result = cur;\n"
    "    if (uFeedback > 0.01) {\n"
    "        vec2 fp = vTexCoord - uCenter;\n"
    "        float s = sin(uFeedbackRot);\n"
    "        float c = cos(uFeedbackRot);\n"
    "        vec2 rotFp = vec2(fp.x * c - fp.y * s, fp.x * s + fp.y * c);\n"
    "        vec2 prevUv = uCenter + rotFp / max(0.001, uFeedbackZoom);\n"
    "        vec4 prev = texture2D(uFeedbackTex, prevUv);\n"
    "        vec3 curDelta = cur.rgb - uBackgroundColor;\n"
    "        vec3 prevDelta = max(vec3(0.0), prev.rgb - uBackgroundColor);\n"
    "        vec3 blendedRgb = uBackgroundColor + max(curDelta, prevDelta * uFeedback);\n"
    "        result.rgb = clamp(blendedRgb, 0.0, 1.0);\n"
    "        result.a = max(cur.a, prev.a * uFeedback);\n"
    "    }\n"
    "    if (uInvert != 0) {\n"
    "        result.rgb = vec3(1.0) - result.rgb;\n"
    "    }\n"
    "    gl_FragColor = result;\n"
    "}\n";

GLuint compileShader(GLenum type, const char* src, std::string& err) {
    GLuint s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    GLint ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        err = log;
        glDeleteShader(s);
        return 0;
    }
    return s;
}

GLuint createProgram(const char* vsSrc, const char* fsSrc, std::string& err) {
    std::string vErr, fErr;
    GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc, vErr);
    if (!vs) { err = "Vertex shader: " + vErr; return 0; }
    GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc, fErr);
    if (!fs) { glDeleteShader(vs); err = "Fragment shader: " + fErr; return 0; }
    GLuint prog = glCreateProgram();
    glAttachShader(prog, vs);
    glAttachShader(prog, fs);
    glLinkProgram(prog);
    glDeleteShader(vs);
    glDeleteShader(fs);
    GLint ok = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &ok);
    if (!ok) {
        char log[1024];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        err = "Link program: " + std::string(log);
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

// Converts the top-down inverse homography (destination -> source) into the
// bottom-up texture space the post shader samples in. The fragment shader's
// vTexCoord has y = 0 at the bottom of the canvas, while WarpQuad uses the
// display/cairo convention (y = 0 at the top), so the mapping is conjugated by
// Flip(x, y) = (x, 1 - y): H_gl = Flip * G * Flip. `out` is column-major, as
// GLSL mat3 and glUniformMatrix3fv expect.
bool warpGlMatrix(const WarpQuad& quad, float out[9]) {
    double g[9];
    if (!quadToInverseHomography(quad, g)) return false;
    const double m00 = g[0], m01 = -g[1], m02 = g[1] + g[2];
    const double m10 = -g[3] + g[6], m11 = g[4] - g[7], m12 = -g[4] - g[5] + g[7] + g[8];
    const double m20 = g[6], m21 = -g[7], m22 = g[7] + g[8];
    out[0] = float(m00); out[1] = float(m10); out[2] = float(m20);
    out[3] = float(m01); out[4] = float(m11); out[5] = float(m21);
    out[6] = float(m02); out[7] = float(m12); out[8] = float(m22);
    return true;
}

std::vector<Point> flatten(const Path& path, int curveSteps = 48) {
    std::vector<Point> pts;
    pts.reserve(path.segments.size() + 1);
    pts.push_back(path.start);
    Point cur = path.start;
    for (const Segment& s : path.segments) {
        if (s.curved) {
            for (int i = 1; i <= curveSteps; ++i) {
                const double t = double(i) / curveSteps, mt = 1 - t;
                const double a = mt * mt * mt, b = 3 * mt * mt * t;
                const double c = 3 * mt * t * t, d = t * t * t;
                pts.push_back({a * cur.x + b * s.c1.x + c * s.c2.x + d * s.end.x,
                               a * cur.y + b * s.c1.y + c * s.c2.y + d * s.end.y});
            }
        } else {
            pts.push_back(s.end);
        }
        cur = s.end;
    }
    if (pts.size() > 1 && std::abs(pts.front().x - pts.back().x) < 1e-9 &&
        std::abs(pts.front().y - pts.back().y) < 1e-9)
        pts.pop_back();
    return pts;
}

void setColor(Color c, double alphaScale = 1.0) {
    glColor4f(float(std::clamp(c.r, 0.0, 1.0)), float(std::clamp(c.g, 0.0, 1.0)),
              float(std::clamp(c.b, 0.0, 1.0)), float(std::clamp(c.a * alphaScale, 0.0, 1.0)));
}

void buildFan(const std::vector<Point>& p, Point apex, std::vector<GLfloat>& buf) {
    buf.clear();
    if (p.size() < 3) return;
    buf.reserve((p.size() + 2) * 2);
    buf.push_back(GLfloat(apex.x));
    buf.push_back(GLfloat(apex.y));
    for (const Point& q : p) {
        buf.push_back(GLfloat(q.x));
        buf.push_back(GLfloat(q.y));
    }
    buf.push_back(GLfloat(p.front().x));
    buf.push_back(GLfloat(p.front().y));
}

void drawFan(const std::vector<GLfloat>& buf) {
    if (buf.size() < 6) return;
    glVertexPointer(2, GL_FLOAT, 0, buf.data());
    glDrawArrays(GL_TRIANGLE_FAN, 0, GLsizei(buf.size() / 2));
}

void strokePolyline(const std::vector<Point>& p, double halfWidth) {
    const int n = static_cast<int>(p.size());
    if (n < 2 || halfWidth <= 0.01) return;
    static std::vector<GLfloat> buf;
    buf.clear();
    buf.reserve((size_t)n * 6 * 2);
    auto v = [](std::vector<GLfloat>& out, double x, double y) {
        out.push_back(GLfloat(x));
        out.push_back(GLfloat(y));
    };
    for (int i = 0; i < n; ++i) {
        const Point a = p[i], b = p[(i + 1) % n];
        const double dx = b.x - a.x, dy = b.y - a.y;
        const double len = std::hypot(dx, dy);
        if (len < 1e-9) continue;
        const double nx = -dy / len * halfWidth, ny = dx / len * halfWidth;
        v(buf, a.x + nx, a.y + ny); v(buf, b.x + nx, b.y + ny); v(buf, b.x - nx, b.y - ny);
        v(buf, a.x + nx, a.y + ny); v(buf, b.x - nx, b.y - ny); v(buf, a.x - nx, a.y - ny);
    }
    if (!buf.empty()) {
        glVertexPointer(2, GL_FLOAT, 0, buf.data());
        glDrawArrays(GL_TRIANGLES, 0, GLsizei(buf.size() / 2));
    }
    if (halfWidth > 0.4) {
        constexpr double miterLimit = 10.0;
        buf.clear();
        buf.reserve((size_t)n * 6 * 2);
        for (int i = 0; i < n; ++i) {
            const Point a = p[(i + n - 1) % n], b = p[i], c = p[(i + 1) % n];
            double d1x = b.x - a.x, d1y = b.y - a.y;
            double d2x = c.x - b.x, d2y = c.y - b.y;
            const double l1 = std::hypot(d1x, d1y), l2 = std::hypot(d2x, d2y);
            if (l1 < 1e-9 || l2 < 1e-9) continue;
            d1x /= l1; d1y /= l1; d2x /= l2; d2y /= l2;
            const double turn = d1x * d2y - d1y * d2x;
            if (std::abs(turn) < 1e-6) continue;
            const double s = turn > 0 ? 1.0 : -1.0;
            const double n1x = s * d1y, n1y = -s * d1x;
            const double n2x = s * d2y, n2y = -s * d2x;
            const double ox1 = b.x + n1x * halfWidth, oy1 = b.y + n1y * halfWidth;
            const double ox2 = b.x + n2x * halfWidth, oy2 = b.y + n2y * halfWidth;
            double bx = n1x + n2x, by = n1y + n2y;
            const double bl = std::hypot(bx, by);
            if (bl < 1e-9) continue;
            bx /= bl; by /= bl;
            const double cosHalf = bx * n1x + by * n1y;
            const double miterLen = cosHalf > 1e-6 ? halfWidth / cosHalf : 1e30;
            if (miterLen > miterLimit * halfWidth) {
                v(buf, b.x, b.y); v(buf, ox1, oy1); v(buf, ox2, oy2);
            } else {
                const double mx = b.x + bx * miterLen, my = b.y + by * miterLen;
                v(buf, b.x, b.y); v(buf, ox1, oy1); v(buf, mx, my);
                v(buf, b.x, b.y); v(buf, mx, my); v(buf, ox2, oy2);
            }
        }
        if (!buf.empty()) {
            glVertexPointer(2, GL_FLOAT, 0, buf.data());
            glDrawArrays(GL_TRIANGLES, 0, GLsizei(buf.size() / 2));
        }
    }
}

} // namespace

GlRenderer::~GlRenderer() {
    shutdown();
}

void GlRenderer::shutdown() {
    if (!context) return;
    if (postProgram) { glDeleteProgram(postProgram); postProgram = 0; }
    for (int i = 0; i < 2; ++i) {
        if (feedbackColor[i]) { glDeleteTextures(1, &feedbackColor[i]); feedbackColor[i] = 0; }
        if (feedbackFbo[i]) { glDeleteFramebuffers(1, &feedbackFbo[i]); feedbackFbo[i] = 0; }
    }
    if (uiTexture) { glDeleteTextures(1, &uiTexture); uiTexture = 0; }
    if (fboColor) { glDeleteTextures(1, &fboColor); fboColor = 0; }
    if (fboDepthStencil) { glDeleteRenderbuffers(1, &fboDepthStencil); fboDepthStencil = 0; }
    if (fbo) { glDeleteFramebuffers(1, &fbo); fbo = 0; }
    if (msColor) { glDeleteRenderbuffers(1, &msColor); msColor = 0; }
    if (msDepthStencil) { glDeleteRenderbuffers(1, &msDepthStencil); msDepthStencil = 0; }
    if (fboMs) { glDeleteFramebuffers(1, &fboMs); fboMs = 0; }
    fboW = fboH = fboSamples = 0;
    SDL_GL_DeleteContext(context);
    context = nullptr;
    uiW = uiH = 0;
}

void GlRenderer::clearFeedback() {
    if (!context) return;
    for (int i = 0; i < 2; ++i) {
        if (feedbackFbo[i]) {
            glBindFramebuffer(GL_FRAMEBUFFER, feedbackFbo[i]);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    feedbackPing = 0;
}

bool GlRenderer::init(SDL_Window* window, std::string& error) {
    context = SDL_GL_CreateContext(window);
    if (!context) { error = SDL_GetError(); return false; }
    SDL_GL_SetSwapInterval(0);
    int samples = 0;
    glGetIntegerv(GL_SAMPLES, &samples);
    msaa = samples > 0;
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);

    std::string progErr;
    postProgram = createProgram(kPostVs, kPostFs, progErr);
    if (postProgram) {
        uLocTexture = glGetUniformLocation(postProgram, "uTexture");
        uLocFeedbackTex = glGetUniformLocation(postProgram, "uFeedbackTex");
        uLocMirrorMode = glGetUniformLocation(postProgram, "uMirrorMode");
        uLocInvert = glGetUniformLocation(postProgram, "uInvert");
        uLocFeedback = glGetUniformLocation(postProgram, "uFeedback");
        uLocFeedbackZoom = glGetUniformLocation(postProgram, "uFeedbackZoom");
        uLocFeedbackRot = glGetUniformLocation(postProgram, "uFeedbackRot");
        uLocCenter = glGetUniformLocation(postProgram, "uCenter");
        uLocBackgroundColor = glGetUniformLocation(postProgram, "uBackgroundColor");
        uLocWarp = glGetUniformLocation(postProgram, "uWarp");
        uLocWarpEnabled = glGetUniformLocation(postProgram, "uWarpEnabled");
    } else {
        std::fprintf(stderr, "whorld: GLSL post-process shader unavailable: %s\n", progErr.c_str());
    }

    error.clear();
    return true;
}

void GlRenderer::drawCanvas(const Frame& frame, bool antialias, int canvasX, int canvasW,
                            int canvasH) {
    (void)canvasW;
    (void)canvasH;
    glPushMatrix();
    glTranslatef(float(canvasX), 0, 0);
    glEnable(GL_BLEND);
    if (msaa && antialias) glEnable(GL_MULTISAMPLE);
    else glDisable(GL_MULTISAMPLE);
    glStencilMask(0x01);
    std::vector<GLfloat> outerFan, innerFan;
    glEnableClientState(GL_VERTEX_ARRAY);
    for (const Shape& s : frame.shapes) {
        const std::vector<Point> outer = flatten(s.outer);
        const bool hasInner = s.hasInner && !s.inner.segments.empty();
        const std::vector<Point> inner = hasInner ? flatten(s.inner) : std::vector<Point>{};
        if (s.fill && outer.size() >= 3) {
            buildFan(outer, s.outer.center, outerFan);
            if (hasInner) buildFan(inner, s.inner.center, innerFan);
            glEnable(GL_STENCIL_TEST);
            glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
            glDisable(GL_BLEND);
            glStencilFunc(GL_ALWAYS, 0, 0x01);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            drawFan(outerFan);
            if (hasInner) drawFan(innerFan);

            glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
            glEnable(GL_BLEND);
            glStencilFunc(GL_NOTEQUAL, 0, 0x01);
            glStencilOp(GL_KEEP, GL_KEEP, GL_INVERT);
            setColor(s.color);
            drawFan(outerFan);
            if (hasInner) drawFan(innerFan);
            glDisable(GL_STENCIL_TEST);
        }
        if (s.outline && outer.size() >= 2) {
            const double half = std::max(0.1, s.lineWidth) / 2;
            setColor(s.fill ? s.outlineColor : s.color);
            glEnable(GL_BLEND);
            strokePolyline(outer, half);
            if (s.fill && hasInner && inner.size() >= 2) strokePolyline(inner, half);
        }
    }
    glDisableClientState(GL_VERTEX_ARRAY);
    glStencilMask(0xFF);
    glPopMatrix();
}

void GlRenderer::uploadUi(cairo_surface_t* surface, int w, int h) {
    cairo_surface_flush(surface);
    unsigned char* data = cairo_image_surface_get_data(surface);
    const int stride = cairo_image_surface_get_stride(surface);
    if (uiTexture == 0) glGenTextures(1, &uiTexture);
    glBindTexture(GL_TEXTURE_2D, uiTexture);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, stride / 4);
    if (w != uiW || h != uiH) {
        uiW = w;
        uiH = h;
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, w, h, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
    } else {
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, data);
    }
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

bool GlRenderer::ensureFbo(int w, int h) {
    if (fbo && fboW == w && fboH == h) return true;
    if (fboColor) { glDeleteTextures(1, &fboColor); fboColor = 0; }
    if (fboDepthStencil) { glDeleteRenderbuffers(1, &fboDepthStencil); fboDepthStencil = 0; }
    if (fbo) { glDeleteFramebuffers(1, &fbo); fbo = 0; }
    if (msColor) { glDeleteRenderbuffers(1, &msColor); msColor = 0; }
    if (msDepthStencil) { glDeleteRenderbuffers(1, &msDepthStencil); msDepthStencil = 0; }
    if (fboMs) { glDeleteFramebuffers(1, &fboMs); fboMs = 0; }

    for (int i = 0; i < 2; ++i) {
        if (feedbackColor[i]) { glDeleteTextures(1, &feedbackColor[i]); feedbackColor[i] = 0; }
        if (feedbackFbo[i]) { glDeleteFramebuffers(1, &feedbackFbo[i]); feedbackFbo[i] = 0; }
    }

    glGenTextures(1, &fboColor);
    glBindTexture(GL_TEXTURE_2D, fboColor);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glGenFramebuffers(1, &fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, fboColor, 0);

    for (int i = 0; i < 2; ++i) {
        glGenTextures(1, &feedbackColor[i]);
        glBindTexture(GL_TEXTURE_2D, feedbackColor[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        glGenFramebuffers(1, &feedbackFbo[i]);
        glBindFramebuffer(GL_FRAMEBUFFER, feedbackFbo[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, feedbackColor[i], 0);
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
    }
    feedbackPing = 0;

    bool complete = glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    int want = msaa ? 4 : 0;
    if (want > 0) {
        glGenFramebuffers(1, &fboMs);
        glBindFramebuffer(GL_FRAMEBUFFER, fboMs);
        glGenRenderbuffers(1, &msColor);
        glBindRenderbuffer(GL_RENDERBUFFER, msColor);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, want, GL_RGBA8, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, msColor);
        glGenRenderbuffers(1, &msDepthStencil);
        glBindRenderbuffer(GL_RENDERBUFFER, msDepthStencil);
        glRenderbufferStorageMultisample(GL_RENDERBUFFER, want, GL_DEPTH24_STENCIL8, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, msDepthStencil);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE) {
            fboSamples = want;
        } else {
            glDeleteRenderbuffers(1, &msColor); msColor = 0;
            glDeleteRenderbuffers(1, &msDepthStencil); msDepthStencil = 0;
            glDeleteFramebuffers(1, &fboMs); fboMs = 0;
        }
    }
    if (fboSamples == 0) {
        glBindFramebuffer(GL_FRAMEBUFFER, fbo);
        glGenRenderbuffers(1, &fboDepthStencil);
        glBindRenderbuffer(GL_RENDERBUFFER, fboDepthStencil);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, w, h);
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                                  GL_RENDERBUFFER, fboDepthStencil);
        complete = complete &&
                   glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    fboW = w;
    fboH = h;
    msaa = fboSamples > 0;
    if (!complete) {
        std::fprintf(stderr, "whorld: offscreen framebuffer incomplete, using cairo\n");
        return false;
    }
    return true;
}

void GlRenderer::render(const Frame& frame, bool antialias, int canvasX, int canvasW, int canvasH,
                        int windowW, int windowH, cairo_surface_t* ui, int uiW_, int uiH_,
                        double superScale, const PostProcess& pp) {
    if (!context) return;
    if (superScale < 1.0) superScale = 1.0;
    const int fboW_ = std::max(1, int(std::lround(canvasW * superScale)));
    const int fboH_ = std::max(1, int(std::lround(canvasH * superScale)));
    const bool offscreen = ensureFbo(fboW_, fboH_);
    if (offscreen) {
        const bool multisample = fboSamples > 0;
        glBindFramebuffer(GL_FRAMEBUFFER, multisample ? fboMs : fbo);
        glViewport(0, 0, fboW_, fboH_);
        glMatrixMode(GL_PROJECTION);
        glLoadIdentity();
        glOrtho(0, canvasW, canvasH, 0, -1, 1);
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        const Color bg = frame.background;
        glClearColor(float(bg.r), float(bg.g), float(bg.b), 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        drawCanvas(frame, antialias, 0, canvasW, canvasH);
        if (multisample) {
            glBindFramebuffer(GL_READ_FRAMEBUFFER, fboMs);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, fbo);
            glBlitFramebuffer(0, 0, fboW_, fboH_, 0, 0, fboW_, fboH_,
                              GL_COLOR_BUFFER_BIT, GL_NEAREST);
        }
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    GLuint finalTexture = fboColor;
    float warpMat[9] = {};
    const bool warpOn = postProgram != 0 && pp.warpEnabled && warpGlMatrix(pp.warp, warpMat);
    const bool hasPost = warpOn || ((postProgram != 0) &&
                         (pp.mirrorMode != 0 || pp.invertColor || pp.feedback > 0.01));

    if (offscreen && hasPost) {
        if (pp.feedback > 0.01) {
            int readIdx = feedbackPing;
            int writeIdx = 1 - feedbackPing;
            glBindFramebuffer(GL_FRAMEBUFFER, feedbackFbo[writeIdx]);
            glViewport(0, 0, fboW_, fboH_);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            // y runs bottom-up here, matching the offscreen texture rows, so the
            // post pass preserves orientation (an invert-only output must not be
            // vertically mirrored on screen). Mirror modes are centre-symmetric
            // and stay unchanged.
            glOrtho(0, 1, 0, 1, -1, 1);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glUseProgram(postProgram);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboColor);
            glUniform1i(uLocTexture, 0);

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, feedbackColor[readIdx]);
            glUniform1i(uLocFeedbackTex, 1);

            glUniform1i(uLocMirrorMode, pp.mirrorMode);
            glUniform1i(uLocInvert, pp.invertColor ? 1 : 0);
            glUniform1f(uLocFeedback, float(pp.feedback));
            glUniform1f(uLocFeedbackZoom, float(pp.feedbackZoom));
            glUniform1f(uLocFeedbackRot, float(pp.feedbackRot));
            glUniform2f(uLocCenter, float(pp.center.x), float(pp.center.y));
            glUniform3f(uLocBackgroundColor, float(frame.background.r),
                        float(frame.background.g), float(frame.background.b));
            if (warpOn) {
                glUniform1i(uLocWarpEnabled, 1);
                glUniformMatrix3fv(uLocWarp, 1, GL_FALSE, warpMat);
            } else {
                glUniform1i(uLocWarpEnabled, 0);
            }

            glDisable(GL_BLEND);
            glBegin(GL_QUADS);
            glTexCoord2f(0, 0); glVertex2f(0, 0);
            glTexCoord2f(1, 0); glVertex2f(1, 0);
            glTexCoord2f(1, 1); glVertex2f(1, 1);
            glTexCoord2f(0, 1); glVertex2f(0, 1);
            glEnd();

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, 0);
            glActiveTexture(GL_TEXTURE0);
            glUseProgram(0);

            feedbackPing = writeIdx;
            finalTexture = feedbackColor[feedbackPing];
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        } else {
            glBindFramebuffer(GL_FRAMEBUFFER, feedbackFbo[0]);
            glViewport(0, 0, fboW_, fboH_);
            glMatrixMode(GL_PROJECTION);
            glLoadIdentity();
            // See the feedback branch above: bottom-up y keeps orientation.
            glOrtho(0, 1, 0, 1, -1, 1);
            glMatrixMode(GL_MODELVIEW);
            glLoadIdentity();

            glUseProgram(postProgram);
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, fboColor);
            glUniform1i(uLocTexture, 0);
            glUniform1i(uLocFeedbackTex, 0);
            glUniform1i(uLocMirrorMode, pp.mirrorMode);
            glUniform1i(uLocInvert, pp.invertColor ? 1 : 0);
            glUniform1f(uLocFeedback, 0.0f);
            glUniform1f(uLocFeedbackZoom, 1.0f);
            glUniform1f(uLocFeedbackRot, 0.0f);
            glUniform2f(uLocCenter, float(pp.center.x), float(pp.center.y));
            glUniform3f(uLocBackgroundColor, float(frame.background.r),
                        float(frame.background.g), float(frame.background.b));
            if (warpOn) {
                glUniform1i(uLocWarpEnabled, 1);
                glUniformMatrix3fv(uLocWarp, 1, GL_FALSE, warpMat);
            } else {
                glUniform1i(uLocWarpEnabled, 0);
            }

            glDisable(GL_BLEND);
            glBegin(GL_QUADS);
            glTexCoord2f(0, 0); glVertex2f(0, 0);
            glTexCoord2f(1, 0); glVertex2f(1, 0);
            glTexCoord2f(1, 1); glVertex2f(1, 1);
            glTexCoord2f(0, 1); glVertex2f(0, 1);
            glEnd();

            glUseProgram(0);
            finalTexture = feedbackColor[0];
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
        }
    }

    glViewport(0, 0, windowW, windowH);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();
    glOrtho(0, windowW, windowH, 0, -1, 1);
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    const Color bg = frame.background;
    glClearColor(float(bg.r), float(bg.g), float(bg.b), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glDisable(GL_STENCIL_TEST);
    if (offscreen) {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ZERO);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, finalTexture);
        setColor({1, 1, 1, 1});
        glBegin(GL_QUADS);
        glTexCoord2f(0, 1); glVertex2f(float(canvasX), 0);
        glTexCoord2f(1, 1); glVertex2f(float(canvasX + canvasW), 0);
        glTexCoord2f(1, 0); glVertex2f(float(canvasX + canvasW), float(canvasH));
        glTexCoord2f(0, 0); glVertex2f(float(canvasX), float(canvasH));
        glEnd();
        glDisable(GL_TEXTURE_2D);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    } else {
        drawCanvas(frame, antialias, canvasX, canvasW, canvasH);
    }
    if (ui) uploadUi(ui, uiW_, uiH_);
    if (uiTexture && uiW_ > 0 && uiH_ > 0) {
        glEnable(GL_BLEND);
        glEnable(GL_TEXTURE_2D);
        glBindTexture(GL_TEXTURE_2D, uiTexture);
        setColor({1, 1, 1, 1});
        glBegin(GL_QUADS);
        glTexCoord2f(0, 0); glVertex2f(0, 0);
        glTexCoord2f(1, 0); glVertex2f(float(uiW_), 0);
        glTexCoord2f(1, 1); glVertex2f(float(uiW_), float(uiH_));
        glTexCoord2f(0, 1); glVertex2f(0, float(uiH_));
        glEnd();
        glDisable(GL_TEXTURE_2D);
    }
    glFlush();
}

bool GlRenderer::renderOffscreen(const Frame& frame, bool antialias, int w, int h,
                                 std::vector<unsigned char>& bgra, double superScale,
                                 const PostProcess& pp) {
    if (!context) return false;
    render(frame, antialias, 0, w, h, w, h, nullptr, 0, 0, superScale, pp);
    bgra.assign(static_cast<std::size_t>(w) * h * 4, 0);
    glReadPixels(0, 0, w, h, GL_BGRA, GL_UNSIGNED_BYTE, bgra.data());
    return true;
}

void GlRenderer::swap(SDL_Window* window) {
    if (context) SDL_GL_SwapWindow(window);
}

} // namespace whorld
