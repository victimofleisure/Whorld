// Native Linux Whorld frontend; the original MFC/Direct2D program remains in trunk/.
#include "whorld_core.h"
#include "playlist.h"
#include "gamepad.h"
#include "midi_actions.h"
#include "midi_decode.h"
#include "alsa_midi.h"
#include "snapshot.h"
#include "gl_renderer.h"
#include "movie.h"
#include "cairo_canvas.h"
#include "projection.h"
#include "video_streamer.h"
#include "audio_input.h"
#include <GL/gl.h>
#include <SDL.h>
#include <cairo/cairo.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

namespace {
using namespace whorld;
constexpr int sidebar = 286;
constexpr int rowHeight = 29;
// The shape selector is a fixed strip above the scrolling parameter list.
constexpr int kShapeStripTop = 128;
constexpr int kShapeStripHeight = 22;
constexpr int kListTop = 158;
using Surface = std::unique_ptr<cairo_surface_t, decltype(&cairo_surface_destroy)>;
using Context = std::unique_ptr<cairo_t, decltype(&cairo_destroy)>;

void text(cairo_t* cr, int x, int y, const std::string& str, int size = 13, bool bold = false) {
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, size);
    cairo_move_to(cr, x, y);
    cairo_show_text(cr, str.c_str());
}
void centeredText(cairo_t* cr, double x, int y, const std::string& str, int size = 13, bool bold = false) {
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL,
                           bold ? CAIRO_FONT_WEIGHT_BOLD : CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, size);
    cairo_text_extents_t ext{};
    cairo_text_extents(cr, str.c_str(), &ext);
    cairo_move_to(cr, x - ext.width / 2, y);
    cairo_show_text(cr, str.c_str());
}
void roundedRect(cairo_t* cr, double x, double y, double w, double h, double r) {
    r = std::min(r, std::min(w, h) / 2);
    cairo_new_sub_path(cr);
    cairo_arc(cr, x + w - r, y + r, r, -PI / 2, 0);
    cairo_arc(cr, x + w - r, y + h - r, r, 0, PI / 2);
    cairo_arc(cr, x + r, y + h - r, r, PI / 2, PI);
    cairo_arc(cr, x + r, y + r, r, PI, 3 * PI / 2);
    cairo_close_path(cr);
}
// The sidebar lists the active primitive's controls first, then the upstream
// parameters, then the masters. Row 0 is the fixed shape selector; it is not
// part of the scrolling list, so the primitive's controls are always in view.
struct PanelRow { bool master = false; int index = 0; };
int activePrimitive(const Patch& p) {
    return std::clamp(int(std::lround(p.params[PARAM_Primitive].value)), 0,
                      static_cast<int>(PRIM_COUNT) - 1);
}
std::vector<PanelRow> panelRows(const Patch& p) {
    std::vector<PanelRow> rows;
    rows.reserve(PARAM_COUNT + MASTER_COUNT);
    rows.push_back({false, PARAM_Primitive}); // the fixed shape selector
    const int primitive = activePrimitive(p);
    for (int i = SHAPE_PARAM_BEGIN + 1; i < PARAM_COUNT; ++i)
        if (shapeParamApplies(i, primitive)) rows.push_back({false, i});
    for (int i = 0; i < SHAPE_PARAM_BEGIN; ++i) rows.push_back({false, i});
    for (int i = 0; i < MASTER_COUNT; ++i) rows.push_back({true, i});
    return rows;
}
// Upstream parameters that only shape the polygon primitive; the sidebar dims
// them while another primitive is active.
bool polygonOnlyParam(int param) {
    switch (param) {
    case PARAM_PolySides: case PARAM_StarFactor: case PARAM_Pinwheel:
    case PARAM_EvenCurve: case PARAM_OddCurve: case PARAM_EvenShear: case PARAM_OddShear:
        return true;
    default: return false;
    }
}
const char* shapeLabel(int k) {
    static const char* const labels[] = {"Poly", "Circle", "Square", "Tri", "Star", "Cross", "Line"};
    return labels[std::clamp(k, 0, static_cast<int>(PRIM_COUNT) - 1)];
}
// Short labels at a small font so all seven fit on one row; widths are measured
// so the hit test and the drawing always agree.
void shapeChipRects(cairo_t* cr, double* cx, double* cw) {
    constexpr double left = 15, pad = 5, gap = 3;
    const double avail = sidebar - 2 * left;
    double total = gap * (PRIM_COUNT - 1);
    cairo_select_font_face(cr, "Sans", CAIRO_FONT_SLANT_NORMAL, CAIRO_FONT_WEIGHT_NORMAL);
    cairo_set_font_size(cr, 9);
    for (int k = 0; k < PRIM_COUNT; ++k) {
        cairo_text_extents_t ext{};
        cairo_text_extents(cr, shapeLabel(k), &ext);
        cw[k] = ext.width + 2 * pad;
        total += cw[k];
    }
    const double scale = total > avail ? avail / total : 1.0;
    double x = left;
    for (int k = 0; k < PRIM_COUNT; ++k) {
        cw[k] *= scale;
        cx[k] = x;
        x += cw[k] + gap * scale;
    }
}
int shapeChipAtX(double x) {
    cairo_surface_t* raw = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
    Context cr(cairo_create(raw), cairo_destroy);
    cairo_surface_destroy(raw);
    double cx[PRIM_COUNT] = {}, cw[PRIM_COUNT] = {};
    shapeChipRects(cr.get(), cx, cw);
    for (int k = 0; k < PRIM_COUNT; ++k)
        if (x >= cx[k] && x < cx[k] + cw[k]) return k;
    return -1;
}
void drawShapeChips(cairo_t* cr, int top, int active) {
    double cx[PRIM_COUNT] = {}, cw[PRIM_COUNT] = {};
    shapeChipRects(cr, cx, cw);
    for (int k = 0; k < PRIM_COUNT; ++k) {
        const bool on = k == active;
        cairo_set_source_rgb(cr, on ? .95 : .19, on ? .57 : .24, on ? .23 : .30);
        roundedRect(cr, cx[k], top, cw[k], kShapeStripHeight, 5);
        cairo_fill(cr);
        cairo_set_source_rgb(cr, on ? .10 : .74, on ? .11 : .79, on ? .13 : .84);
        centeredText(cr, cx[k] + cw[k] / 2, top + 15, shapeLabel(k), 9);
    }
}
void drawPanel(cairo_t* cr, const Engine& engine, const Playlist& playlist, int playlistIndex,
           bool showPlaylist, int width, int height, int selected, int scroll, int property,
           bool paused, const std::string& status, const std::string& input, bool entering,
           bool midiLearn) {
    cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
    cairo_set_source_rgb(cr, .085, .10, .14);
    cairo_rectangle(cr, 0, 0, sidebar, height);
    cairo_fill(cr);
    cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
    cairo_set_source_rgb(cr, .96, .78, .4);
    text(cr, 15, 28, "WHORLD / Linux", 19, true);
    cairo_set_source_rgb(cr, .7, .75, .8);
    text(cr, 15, 52, engine.snapshotActive() ? "SNAPSHOT" : paused ? "PAUSED" : "LIVE", 13, true);
    text(cr, 84, 52, "rings: " + std::to_string(engine.ringCount()));
    text(cr, 15, 75, showPlaylist ? "Enter play   Del remove   Ctrl+Shift+A add" : "Click a row; wheel or arrows to edit.", 12);
    text(cr, 15, 93, showPlaylist ? "[ / ] previous / next   L parameters" : "Tab: properties   Click a shape to switch", 11);
    const char* props[] = {"VALUE", "WAVE", "AMP", "FREQ", "PW", "GLOBAL"};
    const int primitive = activePrimitive(engine.patch);
    cairo_set_source_rgb(cr, .96, .78, .4);
    text(cr, 15, 120, showPlaylist ? "PLAYLIST" : std::string("Editing: ") + props[property], 13, true);
    if (midiLearn) {
        cairo_set_source_rgb(cr, 1, .45, .3);
        text(cr, sidebar - 74, 120, "LEARN?", 13, true);
    }
    // Always-visible shape selector; clicking a chip activates that primitive
    // and the list below immediately shows its adjustment controls. Row 0 of the
    // panel is this selector, so it is outlined when selected for MIDI learn.
    if (!showPlaylist) {
        drawShapeChips(cr, kShapeStripTop, primitive);
        if (selected == 0) {
            cairo_set_source_rgb(cr, .97, .85, .84);
            cairo_set_line_width(cr, 1.5);
            cairo_rectangle(cr, 13.5, kShapeStripTop - 2.5, sidebar - 28, kShapeStripHeight + 3);
            cairo_stroke(cr);
        }
    }
    const int first = kListTop, bottom = std::max(first, height - 157);
    cairo_save(cr);
    cairo_rectangle(cr, 0, first, sidebar, bottom - first);
    cairo_clip(cr);
    if (showPlaylist) {
        for (int i = scroll; i < static_cast<int>(playlist.patches.size()); ++i) {
            int y = first + (i - scroll) * rowHeight;
            if (y > bottom) break;
            if (i == playlistIndex) {
                cairo_set_source_rgb(cr, .2, .29, .36);
                cairo_rectangle(cr, 6, y + 1, sidebar - 12, rowHeight - 1);
                cairo_fill(cr);
            }
            cairo_set_source_rgb(cr, i == playlistIndex ? .97 : .75, .82, .77);
            std::string name = std::filesystem::path(playlist.patches[i]).filename().string();
            text(cr, 15, y + 19, std::to_string(i + 1) + ". " + name.substr(0, 31), 13);
        }
    } else {
        const auto rows = panelRows(engine.patch);
        for (int r = scroll; r < static_cast<int>(rows.size()); ++r) {
            const int y = first + (r - scroll) * rowHeight;
            if (y > bottom) break;
            const PanelRow& row = rows[r];
            const int param = row.master ? -1 : row.index;
            const char* name = row.master ? masterNames[row.index] : paramInfo[row.index].name;
            if (r == selected) {
                cairo_set_source_rgb(cr, .2, .29, .36);
                cairo_rectangle(cr, 6, y + 1, sidebar - 12, rowHeight - 1);
                cairo_fill(cr);
            }
            // A small dot marks rows that a MIDI, gamepad or audio mapping drives.
            bool mapped = false;
            for (const auto& m : playlist.mappings) if (m.target == name) { mapped = true; break; }
            if (!mapped) for (const auto& m : playlist.gamepadMappings) if (m.target == name) { mapped = true; break; }
            if (!mapped) for (const auto& m : playlist.audioMappings) if (m.target == name) { mapped = true; break; }
            if (mapped) {
                cairo_set_source_rgb(cr, .35, .85, .5);
                cairo_arc(cr, 9, y + 14, 2.5, 0, 6.283185307179586);
                cairo_fill(cr);
            }
            double value = row.master ? engine.patch.master[row.index] : engine.patch.params[row.index].value;
            if (!row.master && property != 0) {
                const auto& pr = engine.patch.params[row.index];
                switch (property) {
                case 1: value = pr.wave; break;
                case 2: value = pr.amp; break;
                case 3: value = pr.freq; break;
                case 4: value = pr.pw; break;
                case 5: value = pr.global; break;
                }
            }
            char buf[56];
            if (param == PARAM_Primitive && property == 0)
                std::snprintf(buf, sizeof(buf), "%s", primitiveNames[primitive]);
            else
                std::snprintf(buf, sizeof(buf), "%.4g", value);
            // Upstream polygon controls are dimmed while another primitive is active.
            const bool dim = param >= 0 && polygonOnlyParam(param) && primitive != PRIM_Polygon;
            if (r == selected) cairo_set_source_rgb(cr, .97, .85, .84);
            else if (dim) cairo_set_source_rgb(cr, .46, .5, .58);
            else cairo_set_source_rgb(cr, .75, .8, .84);
            text(cr, 15, y + 18, name, 13, r == selected);
            cairo_set_source_rgb(cr, .94, .77, .44);
            cairo_text_extents_t ext{};
            cairo_text_extents(cr, buf, &ext);
            text(cr, sidebar - 15 - static_cast<int>(ext.width), y + 18, buf);
            if (!row.master && property == 0) {
                double span = paramInfo[row.index].max - paramInfo[row.index].min;
                double fraction = span > 0 ? std::clamp((value - paramInfo[row.index].min) / span, 0.0, 1.0) : 0;
                cairo_set_source_rgb(cr, .25, .32, .39);
                cairo_rectangle(cr, 15, y + 24, sidebar - 30, 2);
                cairo_fill(cr);
                cairo_set_source_rgb(cr, .95, .57, .23);
                cairo_rectangle(cr, 15, y + 24, fraction * (sidebar - 30), 2);
                cairo_fill(cr);
            }
        }
    }
    cairo_restore(cr);
    cairo_set_source_rgb(cr, .68, .75, .8);
    int y = std::max(first + 30, height - 143);
    text(cr, 15, y, "Space pause   F fill   O outline", 12);
    text(cr, 15, y + 19, "F11 full   R clear   Esc quit   H hide", 12);
    text(cr, 15, y + 38, "Ctrl+O/S patch   Ctrl+Shift+O/S/N list", 11);
    text(cr, 15, y + 57, "L playlist  P PNG  s/-/= scale", 12);
    text(cr, 15, y + 76, "Ctrl+T/L snap  PgUp/Dn  Ctrl+M learn", 11);
    cairo_set_source_rgb(cr, .94, .75, .36);
    text(cr, 15, y + 101, status.substr(0, 37), 11);
    if (entering) {
        cairo_set_source_rgb(cr, .15, .21, .28);
        cairo_rectangle(cr, 7, height - 38, sidebar - 14, 32);
        cairo_fill(cr);
        cairo_set_source_rgb(cr, 1, 1, 1);
        text(cr, 13, height - 17, input.substr(input.size() > 32 ? input.size() - 32 : 0) + "_", 12);
    }
    (void)width;
}
Surface makeSurface(int w, int h) {
    cairo_surface_t* raw = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, w, h);
    if (cairo_surface_status(raw) != CAIRO_STATUS_SUCCESS) {
        cairo_surface_destroy(raw);
        throw std::runtime_error("Could not allocate image surface");
    }
    return Surface(raw, cairo_surface_destroy);
}
void image(Engine& engine, const std::string& output, int width, int height,
           const PostProcess& pp = {}) {
    auto surface = makeSurface(width, height);
    const Frame frame = engine.frame(width, height);
    {
        Context cr(cairo_create(surface.get()), cairo_destroy);
        drawFrameCairo(cr.get(), frame, width, height, engine.patch.antialiasing, pp.blendMode);
    }
    // The shader is the reference; warp/mirror/invert also run on the cairo path.
    postProcessCairo(surface.get(), pp, frame.background);
    cairo_surface_flush(surface.get());
    if (cairo_surface_write_to_png(surface.get(), output.c_str()) != CAIRO_STATUS_SUCCESS)
        throw std::runtime_error("Cannot write PNG: " + output);
}
// Writes a PNG from GL_BGRA rows that are stored bottom-up (as glReadPixels
// returns them). Cairo's ARGB32 is also BGRA in memory on little-endian.
void imageBgra(const std::string& output, const std::vector<unsigned char>& bgra, int width, int height) {
    auto surface = makeSurface(width, height);
    unsigned char* dst = cairo_image_surface_get_data(surface.get());
    const int stride = cairo_image_surface_get_stride(surface.get());
    for (int y = 0; y < height; ++y) {
        const unsigned char* src = bgra.data() + std::size_t(height - 1 - y) * width * 4;
        std::memcpy(dst + std::size_t(y) * stride, src, std::size_t(width) * 4);
    }
    cairo_surface_mark_dirty(surface.get());
    if (cairo_surface_write_to_png(surface.get(), output.c_str()) != CAIRO_STATUS_SUCCESS)
        throw std::runtime_error("Cannot write PNG: " + output);
}
// Cached canvas: cairo renders at a configurable scale, then SDL stretches the
// texture to the logical canvas rectangle. Rebuilt only when the size changes.
struct CanvasCache {
    Surface surface{nullptr, cairo_surface_destroy};
    SDL_Texture* texture = nullptr;
    int texW = 0, texH = 0;
};
void resetCanvas(CanvasCache& c, SDL_Renderer* renderer, int texW, int texH) {
    if (c.texture) SDL_DestroyTexture(c.texture);
    c.surface = makeSurface(texW, texH);
    c.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                  SDL_TEXTUREACCESS_STREAMING, texW, texH);
    if (!c.texture)
        throw std::runtime_error(std::string("Cannot create canvas texture: ") + SDL_GetError());
    SDL_SetTextureBlendMode(c.texture, SDL_BLENDMODE_NONE);
    c.texW = texW;
    c.texH = texH;
}
// Cached sidebar: re-rendered only when its contents change or periodically,
// so the many text draws are not repeated on every animation frame.
struct PanelCache {
    Surface surface{nullptr, cairo_surface_destroy};
    SDL_Texture* texture = nullptr;
    int w = 0, h = 0;
};
void resetPanel(PanelCache& p, SDL_Renderer* renderer, int w, int h) {
    if (p.texture) { SDL_DestroyTexture(p.texture); p.texture = nullptr; }
    p.surface = makeSurface(w, h);
    if (renderer) { // the OpenGL backend uploads the cairo surface directly
        p.texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
                                      SDL_TEXTUREACCESS_STREAMING, w, h);
        if (!p.texture)
            throw std::runtime_error(std::string("Cannot create panel texture: ") + SDL_GetError());
        SDL_SetTextureBlendMode(p.texture, SDL_BLENDMODE_NONE);
    }
    p.w = w;
    p.h = h;
}
void uploadTexture(SDL_Texture* texture, cairo_surface_t* surface) {
    if (SDL_UpdateTexture(texture, nullptr, cairo_image_surface_get_data(surface),
                          cairo_image_surface_get_stride(surface)) != 0)
        throw std::runtime_error(SDL_GetError());
}
// Property tag names, in the order the sidebar cycles them with Tab.
const char* const kProps[] = {"Val", "Wave", "Amp", "Freq", "PW", "Global"};
// Snapshot a controller's axes/buttons in the layout the mapper expects. Stick
// axes go through shapeAxis (dead-zone + response curve) so a resting stick
// reads exactly 0; triggers keep their 0..1 range with only a tiny dead-zone.
GamepadState readGamepad(SDL_GameController* c) {
    GamepadState s;
    s.axis[GP_LX] = shapeAxis(SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_LEFTX) / 32767.0);
    s.axis[GP_LY] = shapeAxis(SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_LEFTY) / 32767.0);
    s.axis[GP_RX] = shapeAxis(SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_RIGHTX) / 32767.0);
    s.axis[GP_RY] = shapeAxis(SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_RIGHTY) / 32767.0);
    s.axis[GP_LTRIGGER] = shapeAxis(SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_TRIGGERLEFT) / 32767.0, 0.05, 1.0);
    s.axis[GP_RTRIGGER] = shapeAxis(SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_TRIGGERRIGHT) / 32767.0, 0.05, 1.0);
    static const SDL_GameControllerButton kButtons[GP_BUTTON_COUNT] = {
        SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_BUTTON_B, SDL_CONTROLLER_BUTTON_X, SDL_CONTROLLER_BUTTON_Y,
        SDL_CONTROLLER_BUTTON_LEFTSHOULDER, SDL_CONTROLLER_BUTTON_RIGHTSHOULDER,
        SDL_CONTROLLER_BUTTON_BACK, SDL_CONTROLLER_BUTTON_START, SDL_CONTROLLER_BUTTON_GUIDE,
        SDL_CONTROLLER_BUTTON_LEFTSTICK, SDL_CONTROLLER_BUTTON_RIGHTSTICK,
        SDL_CONTROLLER_BUTTON_DPAD_UP, SDL_CONTROLLER_BUTTON_DPAD_DOWN,
        SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
    };
    for (int i = 0; i < GP_BUTTON_COUNT; ++i)
        s.button[i] = SDL_GameControllerGetButton(c, kButtons[i]) != 0;
    return s;
}
// One open SDL game controller plus its own previous state, so learn and button
// edge detection stay per-device while every pad shares the playlist mappings.
struct GamepadDevice {
    SDL_GameController* handle = nullptr;
    SDL_JoystickID instance = -1;
    GamepadState previous;
};
// Opens every SDL-recognised controller that is not already open. Safe to call
// repeatedly, e.g. in response to SDL_CONTROLLERDEVICEADDED.
void openGameControllers(std::vector<GamepadDevice>& devices, bool ready) {
    if (!ready) return;
    for (int i = 0; i < SDL_NumJoysticks(); ++i) {
        if (!SDL_IsGameController(i)) continue;
        SDL_GameController* handle = SDL_GameControllerOpen(i);
        if (!handle) continue;
        const SDL_JoystickID instance = SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(handle));
        bool alreadyOpen = false;
        for (const GamepadDevice& device : devices)
            if (device.instance == instance) { alreadyOpen = true; break; }
        if (alreadyOpen) { SDL_GameControllerClose(handle); continue; }
        GamepadDevice device;
        device.handle = handle;
        device.instance = instance;
        devices.push_back(device);
    }
}
// Closes the controller with the given instance id; returns true if it was open.
bool closeGameController(std::vector<GamepadDevice>& devices, SDL_JoystickID instance) {
    for (auto it = devices.begin(); it != devices.end(); ++it) {
        if (it->instance != instance) continue;
        if (it->handle) SDL_GameControllerClose(it->handle);
        devices.erase(it);
        return true;
    }
    return false;
}
void closeAllGameControllers(std::vector<GamepadDevice>& devices) {
    for (GamepadDevice& device : devices)
        if (device.handle) SDL_GameControllerClose(device.handle);
    devices.clear();
}
std::vector<std::string> gamepadNames(const std::vector<GamepadDevice>& devices) {
    std::vector<std::string> names;
    names.reserve(devices.size());
    for (const GamepadDevice& device : devices) {
        const char* name = device.handle ? SDL_GameControllerName(device.handle) : nullptr;
        names.push_back(name ? name : "connected");
    }
    return names;
}
// Best-effort short low-frequency pulse for a momentary action. Controllers
// without rumble ignore it, and a failure is never fatal.
void nudgeRumble(SDL_GameController* controller) {
#if SDL_VERSION_ATLEAST(2, 0, 9)
    if (controller) SDL_GameControllerRumble(controller, 0x4000, 0, 120);
#else
    (void)controller;
#endif
}
// Returns the combined index of the first control actuated since `previous`:
// an axis crossed the learn deadzone, or a button changed state. Axes occupy
// [0, GP_AXIS_COUNT), buttons follow; returns -1 when nothing moved.
int gamepadLearnControl(const GamepadState& current, const GamepadState& previous) {
    constexpr double deadzone = 0.5;
    for (int i = 0; i < GP_AXIS_COUNT; ++i)
        if (std::abs(current.axis[i]) > deadzone && std::abs(previous.axis[i]) <= deadzone)
            return i;
    for (int i = 0; i < GP_BUTTON_COUNT; ++i)
        if (current.button[i] != previous.button[i]) return GP_AXIS_COUNT + i;
    return -1;
}
// Bind (or rebind) the input that produced `message` to a target/property. One
// control drives one target, so any previous mapping for the same input is
// replaced (matching how MIDI learn behaves in the Windows app).
void learnMidi(Playlist& playlist, const MidiMessage& message, const std::string& target,
               const std::string& property) {
    const std::string event = midiEventName(message.event);
    playlist.mappings.erase(std::remove_if(playlist.mappings.begin(), playlist.mappings.end(),
        [&](const Mapping& m) {
            return m.event == event && m.channel == message.channel && m.control == message.control;
        }), playlist.mappings.end());
    Mapping m;
    m.event = event;
    m.channel = message.channel;
    m.control = message.control;
    m.target = target;
    m.property = property;
    m.start = 0;
    m.end = 100;
    playlist.mappings.push_back(std::move(m));
}
void help() {
    std::cout << "Whorld native Linux port\n"
              << "Usage: whorld [--patch FILE.whp] [--playlist FILE.whl] [--snapshot FILE.whs]\n"
              << "               [--movie FILE.whm] [--midi-source CLIENT:PORT] [--render IMAGE.png]\n"
              << "               [--midi-mode absolute|14bit|rel-twos|rel-sign|rel-binary]\n"
              << "               [--midi-takeover] [--list-midi]\n"
              << "               [--movie-out FILE.whm] [--movie-rate FPS] [--movie-frame N]\n"
              << "               [--frames N] [--size WIDTHxHEIGHT] [--scale F]\n"
              << "               [--renderer auto|gpu|cairo]\n"
              << "               [--mirror off|quad|kaleidoscope|radial] [--invert]\n"
              << "               [--blend normal|add|screen] [--morph SECONDS]\n"
              << "               [--feedback 0.0-0.99] [--feedback-zoom F] [--feedback-rot F]\n"
              << "               [--warp \"TLx,TLy,TRx,TRy,BRx,BRy,BLx,BLy\"] [--warp-file FILE]\n"
              << "               [--warp-save FILE]\n"
              << "               [--v4l2 /dev/videoX] [--pipe-out PATH|-] [--headless]\n"
              << "               [--audio] [--audio-device NAME] [--audio-gain F]\n"
              << "--render creates an image without requiring a display. --frames defaults to 240.\n"
              << "--mirror post-processing symmetry: off, quad (4-way), kaleidoscope (8-way), radial (12-way).\n"
              << "--invert inverts color channels.\n"
              << "--blend geometry compositing: normal (alpha over), add (additive glow) or screen.\n"
              << "--morph crossfades playlist patches over SECONDS (default 0 = instant).\n"
              << "--feedback enables video feedback trails with controllable decay (0.0-0.99).\n"
              << "--warp maps the rendered image onto a convex quadrilateral (normalized 0..1, in\n"
              << "        TL,TR,BR,BL order) for projection mapping; --warp-file reads four x y\n"
              << "        pairs from a file and --warp-save writes a template and exits.\n"
              << "--v4l2 streams live video to a V4L2 virtual camera device (e.g. /dev/video0).\n"
              << "--list-midi prints the available ALSA sequencer input sources and exits.\n"
              << "--midi-mode selects how controller bytes are read (default absolute): 14bit\n"
              << "        combines MSB/LSB CC pairs; rel-twos/rel-sign/rel-binary decode\n"
              << "        relative encoders. --midi-takeover ignores an absolute/14-bit control\n"
              << "        until it is moved to (or past) the target's current value.\n"
              << "--pipe-out streams raw RGB video frames to stdout (-) or a FIFO pipe.\n"
              << "--headless runs offscreen GPU rendering/streaming without an X11 window.\n"
              << "--audio enables microphone/line input; --audio-device picks the ALSA device\n"
              << "        (default \"default\") and --audio-gain scales the analysis (default 1.0).\n"
              << "Keys: Space pause, F/O fill and outline, L playlist, P PNG, H hide controls,\n"
              << "      M cycle mirror/kaleidoscope, B cycle blend, I toggle invert, W toggle warp,\n"
              << "      T cycle feedback trails,\n"
              << "      Ctrl+O/S patch, Ctrl+Shift+O/S/N playlist, Ctrl+T/L take/load snapshot,\n"
              << "      PgUp/PgDn step snapshots, s/-/= render scale, F11 fullscreen, R clear, Esc quit.\n"
              << "      Ctrl+M learn a MIDI/gamepad control for the selected row; Ctrl+Shift+M forgets it.\n"
              << "Gamepad and audio mappings live in the playlist ([Gamepad]/[Audio]); see README.\n";
}
} // namespace

int main(int argc, char** argv) {
    try {
        int width = 1100, height = 740, frames = 240;
        std::string patchFile, renderFile, playlistFile, midiSource, snapshotFile;
        std::string movieFile, movieOut;
        double movieRate = 60;
        int movieFrame = 0;
        std::string rendererMode = "auto";
        double forcedScale = 0; // 0 means adaptive
        PostProcess postProcess;
        std::string v4l2Device;
        std::string pipeOutPath;
        std::string warpSaveFile;
        bool headless = false;
        std::string audioDevice;
        double audioGain = 1.0;
        bool audioEnabled = false;
        double morphSeconds = 0; // 0 = instant playlist patch changes
        bool listMidi = false;
        MidiInputMode midiMode = MidiInputMode::Absolute;
        bool midiTakeover = false;
        for (int i = 1; i < argc; ++i) {
            std::string arg = argv[i];
            if (arg == "--help" || arg == "-h") { help(); return 0; }
            if (arg == "--invert") { postProcess.invertColor = true; continue; }
            if (arg == "--headless") { headless = true; continue; }
            if (arg == "--audio") { audioEnabled = true; continue; }
            if (arg == "--list-midi") { listMidi = true; continue; }
            if (arg == "--midi-takeover") { midiTakeover = true; continue; }
            if (i + 1 >= argc) throw std::runtime_error("Missing value for " + arg);
            std::string value = argv[++i];
            if (arg == "--patch") patchFile = value;
            else if (arg == "--playlist") playlistFile = value;
            else if (arg == "--snapshot") snapshotFile = value;
            else if (arg == "--midi-source") midiSource = value;
            else if (arg == "--midi-mode") {
                if (value == "absolute") midiMode = MidiInputMode::Absolute;
                else if (value == "14bit") midiMode = MidiInputMode::FourteenBit;
                else if (value == "rel-twos") midiMode = MidiInputMode::RelativeTwosComplement;
                else if (value == "rel-sign") midiMode = MidiInputMode::RelativeSignMagnitude;
                else if (value == "rel-binary") midiMode = MidiInputMode::RelativeBinaryOffset;
                else throw std::runtime_error(
                    "--midi-mode must be absolute, 14bit, rel-twos, rel-sign or rel-binary");
            }
            else if (arg == "--render") renderFile = value;
            else if (arg == "--scale") {
                forcedScale = std::stod(value);
                if (forcedScale < 0.25 || forcedScale > 2.0)
                    throw std::runtime_error("--scale must be between 0.25 and 2.0");
            }
            else if (arg == "--renderer") {
                rendererMode = value;
                if (rendererMode != "auto" && rendererMode != "gpu" && rendererMode != "cairo")
                    throw std::runtime_error("--renderer must be auto, gpu or cairo");
            }
            else if (arg == "--blend") {
                if (value == "normal" || value == "0") postProcess.blendMode = 0;
                else if (value == "add" || value == "additive" || value == "1") postProcess.blendMode = 1;
                else if (value == "screen" || value == "2") postProcess.blendMode = 2;
                else throw std::runtime_error("Invalid --blend option (expected normal, add, screen)");
            }
            else if (arg == "--morph") {
                morphSeconds = std::stod(value);
                if (!(morphSeconds >= 0) || morphSeconds > 3600)
                    throw std::runtime_error("--morph must be between 0 and 3600 seconds");
            }
            else if (arg == "--mirror") {
                if (value == "quad" || value == "1") postProcess.mirrorMode = 1;
                else if (value == "kaleidoscope" || value == "2") postProcess.mirrorMode = 2;
                else if (value == "radial" || value == "3") postProcess.mirrorMode = 3;
                else if (value == "off" || value == "0") postProcess.mirrorMode = 0;
                else throw std::runtime_error("Invalid --mirror option (expected off, quad, kaleidoscope, radial)");
            }
            else if (arg == "--feedback") {
                postProcess.feedback = std::stod(value);
                if (postProcess.feedback < 0.0 || postProcess.feedback > 0.999)
                    throw std::runtime_error("--feedback must be between 0.0 and 0.99");
            }
            else if (arg == "--feedback-zoom") {
                postProcess.feedbackZoom = std::stod(value);
            }
            else if (arg == "--feedback-rot") {
                postProcess.feedbackRot = std::stod(value);
            }
            else if (arg == "--warp") {
                if (!warpFromString(value, postProcess.warp))
                    throw std::runtime_error(
                        "--warp expects \"TLx,TLy,TRx,TRy,BRx,BRy,BLx,BLy\"");
                if (!validWarp(postProcess.warp))
                    throw std::runtime_error("--warp quad is degenerate");
                postProcess.warpEnabled = true;
            }
            else if (arg == "--warp-file") {
                std::ifstream in(value);
                if (!in) throw std::runtime_error("Cannot open warp file: " + value);
                std::stringstream text;
                text << in.rdbuf();
                if (!warpFromString(text.str(), postProcess.warp))
                    throw std::runtime_error("Malformed warp file: " + value);
                if (!validWarp(postProcess.warp))
                    throw std::runtime_error("Degenerate warp quad in " + value);
                postProcess.warpEnabled = true;
            }
            else if (arg == "--warp-save") {
                warpSaveFile = value;
            }
            else if (arg == "--v4l2") {
                v4l2Device = value;
            }
            else if (arg == "--pipe-out") {
                pipeOutPath = value;
            }
            else if (arg == "--audio-device") audioDevice = value;
            else if (arg == "--audio-gain") {
                audioGain = std::stod(value);
                if (audioGain <= 0 || audioGain > 100)
                    throw std::runtime_error("--audio-gain must be between 0 and 100");
            }
            else if (arg == "--frames") frames = std::stoi(value);
            else if (arg == "--movie") movieFile = value;
            else if (arg == "--movie-out") movieOut = value;
            else if (arg == "--movie-frame") movieFrame = std::stoi(value);
            else if (arg == "--movie-rate") {
                movieRate = std::stod(value);
                if (movieRate < 1 || movieRate > 240) throw std::runtime_error("--movie-rate must be 1-240");
            }
            else if (arg == "--size") {
                auto x = value.find('x');
                if (x == std::string::npos) throw std::runtime_error("Size must be WIDTHxHEIGHT");
                width = std::stoi(value.substr(0, x));
                height = std::stoi(value.substr(x + 1));
            } else throw std::runtime_error("Unknown option " + arg);
        }
        if (listMidi) {
            // Discovery must work without a display and even when the sequencer
            // is absent, so print a clear message and exit successfully either way.
            std::vector<std::string> sources;
            std::string listError;
            if (!listMidiSources(sources, listError)) {
                std::cout << "no MIDI sequencer: " << listError << '\n';
            } else if (sources.empty()) {
                std::cout << "no ALSA MIDI input sources\n";
            } else {
                for (const std::string& source : sources) std::cout << source << '\n';
            }
            return 0;
        }
        if (!warpSaveFile.empty()) {
            std::ofstream out(warpSaveFile);
            out << warpToString(postProcess.warp);
            if (!out) throw std::runtime_error("Cannot write warp file: " + warpSaveFile);
            std::cout << "Saved " << warpSaveFile << '\n';
            return 0;
        }
        if (width < 64 || height < 64 || width > 8192 || height > 8192 || frames < 0 || frames > 100000)
            throw std::runtime_error("Invalid image dimensions or frame count");
        Engine engine;
        Playlist playlist;
        std::vector<Snapshot> snapshots;
        int playlistIndex = -1, snapshotIndex = -1;
        std::string error;
        if (!playlistFile.empty()) {
            if (!playlist.load(playlistFile, error)) throw std::runtime_error(error);
            if (patchFile.empty() && !playlist.patches.empty()) {
                playlistIndex = 0;
                patchFile = playlist.resolvePath(0);
            }
        }
        // Microphone/line input drives the visuals through [Audio] mappings; if
        // the playlist has none, fall back to a useful default set.
        if (audioEnabled && playlist.audioMappings.empty())
            playlist.audioMappings = defaultAudioMappings();
        if (!patchFile.empty()) {
            Patch patch;
            if (!patch.load(patchFile, error)) throw std::runtime_error(error);
            engine.setPatch(patch);
            if (engine.patch.invertColor) postProcess.invertColor = true;
            if (engine.patch.mirror && postProcess.mirrorMode == 0) postProcess.mirrorMode = 1;
        }
        if (!snapshotFile.empty()) {
            Snapshot snapshot;
            if (!snapshot.load(snapshotFile, error)) throw std::runtime_error(error);
            if (!engine.displaySnapshot(snapshot, width, height))
                throw std::runtime_error("Invalid snapshot: " + snapshotFile);
            snapshots.push_back(snapshot);
            snapshotIndex = 0;
        }
        VideoStreamer streamer;
        if (!v4l2Device.empty()) {
            std::string sErr;
            if (!streamer.openV4L2(v4l2Device, width, height, sErr))
                throw std::runtime_error(sErr);
            std::cout << "Streaming video to V4L2 device: " << v4l2Device << " (" << width << "x" << height << ")\n";
        } else if (!pipeOutPath.empty()) {
            std::string sErr;
            if (!streamer.openPipe(pipeOutPath, width, height, sErr))
                throw std::runtime_error(sErr);
            if (pipeOutPath != "-")
                std::cout << "Streaming video to pipe: " << pipeOutPath << " (" << width << "x" << height << ")\n";
        }

        if (headless) {
            if (SDL_Init(SDL_INIT_VIDEO) != 0)
                throw std::runtime_error(std::string("SDL initialization failed: ") + SDL_GetError());
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
            SDL_Window* offscreen = SDL_CreateWindow("whorld-stream", 0, 0, width, height,
                                                     SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
            if (!offscreen)
                throw std::runtime_error(std::string("Cannot create GPU window: ") + SDL_GetError());
            GlRenderer gpu;
            std::string glError;
            if (!gpu.init(offscreen, glError))
                throw std::runtime_error("Cannot initialize GPU renderer: " + glError);
            std::vector<unsigned char> pixels;
            std::vector<unsigned char> rgbFlipped(width * height * 3);
            for (int f = 0; frames == 0 || f < frames; ++f) {
                engine.tick(width, height);
                Frame frame = engine.frame(width, height);
                if (!gpu.renderOffscreen(frame, engine.patch.antialiasing, width, height, pixels,
                                         forcedScale > 0 ? forcedScale : 1.0, postProcess))
                    break;
                for (int y = 0; y < height; ++y) {
                    const unsigned char* srcRow = pixels.data() + (height - 1 - y) * width * 4;
                    unsigned char* dstRow = rgbFlipped.data() + y * width * 3;
                    for (int x = 0; x < width; ++x) {
                        dstRow[x * 3 + 0] = srcRow[x * 4 + 2];
                        dstRow[x * 3 + 1] = srcRow[x * 4 + 1];
                        dstRow[x * 3 + 2] = srcRow[x * 4 + 0];
                    }
                }
                if (streamer.active()) {
                    if (!streamer.writeFrame(rgbFlipped.data(), rgbFlipped.size())) break;
                }
                SDL_Delay(1000 / int(movieRate));
            }
            SDL_DestroyWindow(offscreen);
            SDL_Quit();
            return 0;
        }

        if (!renderFile.empty()) {
            if (!movieFile.empty()) {
                // Render a frame of a movie instead of ticking the simulation.
                MovieReader source;
                if (!source.open(movieFile, error)) throw std::runtime_error(error);
                Snapshot frame;
                if (!source.frame(std::size_t(movieFrame), frame, error))
                    throw std::runtime_error(error);
                if (!engine.displaySnapshot(frame, width, height))
                    throw std::runtime_error("Invalid movie frame");
                snapshots.push_back(frame);
                snapshotIndex = 0;
            } else {
                for (int i = 0; i < frames; ++i) {
                    engine.tick(width, height);
                    engine.frame(width, height); // mark offscreen rings for deletion
                }
            }
            const bool wantPost = postProcess.mirrorMode != 0 || postProcess.invertColor ||
                                  postProcess.blendMode != 0 ||
                                  postProcess.feedback > 0.01 ||
                                  (postProcess.warpEnabled && validWarp(postProcess.warp));
            // OpenGL offscreen rendering is required for feedback trails and is
            // preferred in auto mode whenever post-processing is requested, so
            // --mirror/--invert/--feedback are not silently dropped. --renderer
            // cairo always uses the cairo path (mirror/invert run on the CPU);
            // --renderer gpu must use OpenGL or fail.
            const bool tryGpu = rendererMode == "gpu" || (rendererMode == "auto" && wantPost);
            bool rendered = false;
            if (tryGpu && SDL_Init(SDL_INIT_VIDEO) == 0) {
                // Headless GPU render: render the frame with OpenGL and read it
                // back, so the GPU path can be compared with the cairo one.
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
                SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
                SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
                SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
                SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
                SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
                SDL_Window* offscreen = SDL_CreateWindow("whorld-gpu", 0, 0, width, height,
                                                         SDL_WINDOW_OPENGL | SDL_WINDOW_HIDDEN);
                GlRenderer gpu;
                std::string glError;
                if (offscreen && gpu.init(offscreen, glError)) {
                    std::vector<unsigned char> pixels;
                    if (!gpu.renderOffscreen(engine.frame(width, height), engine.patch.antialiasing,
                                             width, height, pixels, forcedScale > 0 ? forcedScale : 1.0, postProcess))
                        throw std::runtime_error("GPU render failed");
                    imageBgra(renderFile, pixels, width, height);
                    rendered = true;
                } else if (rendererMode == "gpu") {
                    // --renderer gpu fails fast when OpenGL is missing.
                    if (offscreen) SDL_DestroyWindow(offscreen);
                    SDL_Quit();
                    throw std::runtime_error(offscreen
                        ? "Cannot initialize GPU renderer: " + glError
                        : std::string("Cannot create GPU window: ") + SDL_GetError());
                }
                if (offscreen) SDL_DestroyWindow(offscreen);
                SDL_Quit();
            } else if (tryGpu && rendererMode == "gpu") {
                throw std::runtime_error(std::string("SDL initialization failed: ") + SDL_GetError());
            }
            if (!rendered) {
                // Feedback trails keep per-frame history, which only the GPU
                // backend stores; the cairo path cannot reproduce them.
                if (postProcess.feedback > 0.01)
                    std::cerr << "whorld: feedback trails require the GPU backend; rendering without them\n";
                image(engine, renderFile, width, height, postProcess);
            }
            const std::size_t rings = engine.snapshotActive() && !snapshots.empty()
                                    ? snapshots.front().rings.size() : engine.ringCount();
            std::cout << "Saved " << renderFile << " (" << rings << " rings)\n";
            return 0;
        }
        if (!movieOut.empty()) {
            MovieWriter writer;
            if (!writer.open(movieOut, movieRate, width, height, error))
                throw std::runtime_error(error);
            for (int i = 0; i < frames; ++i) {
                engine.tick(width, height);
                engine.frame(width, height); // retire offscreen rings
                if (!writer.write(engine.captureSnapshot(width, height), error))
                    throw std::runtime_error(error);
            }
            if (!writer.close(error)) throw std::runtime_error(error);
            std::cout << "Saved " << movieOut << " (" << writer.frameCount() << " frames at "
                      << movieRate << " fps)\n";
            return 0;
        }
        MovieReader movie;
        bool movieActive = false;
        size_t movieIndex = 0;
        bool movieHasShown = false;
        std::chrono::steady_clock::time_point movieStart;
        if (!movieFile.empty()) {
            if (!movie.open(movieFile, error)) throw std::runtime_error(error);
            if (movie.frameCount() == 0) throw std::runtime_error("Movie has no frames: " + movieFile);
            movieActive = true;
            movieStart = std::chrono::steady_clock::now();
        }
        if (SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS) != 0)
            throw std::runtime_error(std::string("SDL initialization failed: ") + SDL_GetError());
        // Prefer the OpenGL canvas renderer (stencil even-odd fills on the GPU);
        // fall back to SDL_Renderer + cairo when OpenGL is unavailable.
        SDL_Window* window = nullptr;
        SDL_Renderer* renderer = nullptr;
        GlRenderer gl;
        bool useGl = false;
        if (rendererMode != "cairo") {
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 2);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 1);
            SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
            SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
            SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
            window = SDL_CreateWindow("Whorld - Linux", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_OPENGL);
            if (window) {
                std::string glError;
                if (gl.init(window, glError)) {
                    useGl = true;
                } else {
                    SDL_DestroyWindow(window);
                    window = nullptr;
                    if (rendererMode == "gpu")
                        throw std::runtime_error("Cannot initialize GPU renderer: " + glError);
                    std::cerr << "whorld: OpenGL unavailable (" << glError << "), using cairo\n";
                }
            } else if (rendererMode == "gpu") {
                throw std::runtime_error(std::string("Cannot create GPU window: ") + SDL_GetError());
            }
        }
        if (!useGl) {
            window = SDL_CreateWindow("Whorld - Linux", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                width, height, SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
            if (!window) throw std::runtime_error(std::string("Cannot create window: ") + SDL_GetError());
            renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
            if (!renderer) renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
            if (!renderer) throw std::runtime_error(std::string("Cannot create renderer: ") + SDL_GetError());
        }
        if (std::getenv("WHORLD_TIMING"))
            std::fprintf(stderr, "canvas backend: %s (mode=%s)\n", useGl ? "opengl" : "cairo-sdl",
                         rendererMode.c_str());
        bool running = true, paused = false, hidden = false, dragging = false, entering = false;
        bool showPlaylist = false;
        bool warnedCairoFeedback = false;
        int selected = 0, property = 0, scroll = 0;
        std::string input, status = "Ready", command;
        // Render scale: 1.0 is native resolution. cairo rasterizes at that scale
        // (and adaptive mode lowers it when a frame costs too much CPU); OpenGL
        // supersamples the offscreen canvas at that scale and still applies MSAA.
        constexpr double minScale = 0.5;
        double renderScale = forcedScale > 0 ? forcedScale : 1.0;
        bool autoScale = forcedScale <= 0 && !useGl;
        double smoothWorkMs = 0;
        CanvasCache canvas;
        PanelCache panel;
        bool panelDirty = true;
        auto lastAdjust = std::chrono::steady_clock::now();
        auto lastPanel = std::chrono::steady_clock::now();
        double mx = .5, my = .5;
        MidiInput midi;
        std::string midiError;
        if (midi.open(midiSource, midiError)) status = "MIDI " + midi.portName();
        else std::cerr << "whorld: MIDI disabled: " << midiError << '\n';
        // Microphone/line input drives parameters through the [Audio] mappings.
        AudioInput audio;
        bool audioReady = false;
        if (audioEnabled) {
            std::string audioError;
            audioReady = audio.open(audioDevice, audioGain, 48000, audioError);
            if (!audioReady) std::cerr << "whorld: audio disabled: " << audioError << '\n';
        }
        // Gamepad input (any SDL-mapped controller). Controllers are polled each
        // frame and applied through the playlist's [Gamepad] mappings.
        bool gamepadReady = SDL_InitSubSystem(SDL_INIT_GAMECONTROLLER) == 0;
        if (!gamepadReady)
            std::cerr << "whorld: game controllers unavailable: " << SDL_GetError() << '\n';
        // Open every recognised pad and give each its own previous-state slot;
        // all of them share the playlist's [Gamepad] mappings.
        std::vector<GamepadDevice> controllers;
        bool gamepadDefaultsLoaded = false;
        auto offerGamepadDefaults = [&]() -> bool {
            if (gamepadDefaultsLoaded || controllers.empty() || hasGamepadMappings(playlist.gamepadMappings))
                return false;
            playlist.gamepadMappings = defaultGamepadMappings();
            gamepadDefaultsLoaded = true;
            return true;
        };
        openGameControllers(controllers, gamepadReady);
        if (offerGamepadDefaults())
            status = "Gamepad defaults loaded (save playlist to keep)";
        else if (!controllers.empty())
            status = controllerSummary(gamepadNames(controllers));
        if (hasGamepadMappings(playlist.gamepadMappings)) gamepadDefaultsLoaded = true;
        // MIDI learn: Ctrl+M arms the next incoming message for the selected row.
        bool midiLearn = false;
        auto learnTargetName = [&]() -> std::string {
            if (showPlaylist) return {};
            const auto rows = panelRows(engine.patch);
            if (selected < 0 || selected >= static_cast<int>(rows.size())) return {};
            const PanelRow& row = rows[selected];
            return row.master ? masterNames[row.index] : paramInfo[row.index].name;
        };
        // Mirror/InvertColor are rendered by the post-process shader, whose state
        // is a copy of the patch, so a mapped change must be pushed across.
        auto syncTarget = [&](const std::string& target) {
            if (target == "Mirror") postProcess.mirrorMode = engine.patch.mirror ? 1 : 0;
            else if (target == "InvertColor") postProcess.invertColor = engine.patch.invertColor;
        };
        // Per-control MIDI decoding state, kept on the UI thread next to the poll
        // loop. Absolute mode (the default) uses none of it.
        std::map<int, int> midiMsb;              // 14-bit: (channel<<7|control) -> MSB
        std::map<int, double> midiRelative;      // relative: running 0..1 value
        std::map<int, TakeoverState> midiPickup; // soft takeover per control
        std::map<int, double> midiLastAmount;    // fallback takeover reference
        // The first [Mapping] that listens on a CONTROL message, or null.
        auto midiControlMapping = [&](int channel, int control) -> const Mapping* {
            for (const Mapping& m : playlist.mappings)
                if (m.event == "CONTROL" && m.channel == channel && m.control == control) return &m;
            return nullptr;
        };
        auto applyAction = [&](const MappedAction& action) {
            applyMidiAction(action, engine, playlist, paused, playlistIndex, status);
            syncTarget(action.target);
            panelDirty = true;
        };
        // Decodes one message for the selected input mode and dispatches the
        // resulting actions. Absolute and 14-bit reuse mapMidi; relative modes
        // track a running value per mapped control so a delta moves the target
        // from wherever it already is instead of being read as a position.
        auto dispatchMidi = [&](MidiMessage message) {
            if (midiMode == MidiInputMode::FourteenBit && message.event == MidiEvent::Control) {
                const int key = message.channel * 128 + message.control;
                if (message.control < 32) { // MSB: remember until the LSB arrives
                    midiMsb[key] = message.value;
                    return;
                }
                if (message.control < 64) { // LSB: combine into a 14-bit value
                    const auto it = midiMsb.find(message.channel * 128 + message.control - 32);
                    message.value = midi14bit(it == midiMsb.end() ? 0 : it->second, message.value);
                    message.control -= 32;
                }
                // 64..127 are normal switches, not value LSBs; fall through and
                // let mapMidi read them as 7-bit so mapped buttons keep working.
            }
            const bool relative = midiMode == MidiInputMode::RelativeTwosComplement ||
                                  midiMode == MidiInputMode::RelativeSignMagnitude ||
                                  midiMode == MidiInputMode::RelativeBinaryOffset;
            if (relative) {
                if (message.event != MidiEvent::Control) return;
                const Mapping* mapping = midiControlMapping(message.channel, message.control);
                if (!mapping) return; // only mapped controls are tracked
                const int key = message.channel * 128 + message.control;
                auto running = midiRelative.find(key);
                if (running == midiRelative.end()) {
                    // Seed from the target's current position so the first click
                    // is relative to where the parameter already is.
                    double seed = 0.5;
                    const double amount = targetAmount(mapping->target, mapping->property, engine);
                    const double span = (mapping->end - mapping->start) / 100.0;
                    if (std::isfinite(amount) && span != 0)
                        seed = std::clamp((amount - mapping->start / 100.0) / span, 0.0, 1.0);
                    running = midiRelative.emplace(key, seed).first;
                }
                running->second = std::clamp(running->second +
                                                 relativeDelta(message.value, midiMode) / 127.0, 0.0, 1.0);
                for (const MappedAction& action : mapMidiNormalized(playlist, message.event,
                                                                    message.channel, message.control,
                                                                    running->second))
                    applyAction(action);
                return;
            }
            for (const MappedAction& action : mapMidi(playlist, message)) {
                if (midiTakeover) {
                    // Gate absolute/14-bit controls until the knob reaches (or
                    // crosses) where the target already is, so it never jumps.
                    const int key = message.channel * 128 + message.control;
                    const double current = targetAmount(action.target, action.property, engine);
                    const double reference = std::isfinite(current) ? current : midiLastAmount[key];
                    if (!midiPickup[key].accept(action.amount, reference)) continue;
                    midiLastAmount[key] = action.amount;
                }
                applyAction(action);
            }
        };
        auto canvasWidth = [&] { return width - (hidden ? 0 : sidebar); };
        std::string playingPatch;   // basename of the current playlist entry
        bool morphingShown = false; // whether the status bar currently says "Morphing"
        auto playPatch = [&](int index) {
            if (index < 0 || index >= static_cast<int>(playlist.patches.size())) {
                status = playlist.patches.empty() ? "Playlist is empty" : "No such playlist entry";
                return;
            }
            const std::string path = playlist.resolvePath(index);
            Patch next;
            std::string failure;
            if (!next.load(path, failure)) { status = failure; return; }
            engine.stopSnapshot();
            if (morphSeconds > 0) {
                engine.morphTo(next, morphSeconds);
                playingPatch = std::filesystem::path(playlist.patches[index]).filename().string();
                status = "Morphing to " + playingPatch;
            } else {
                engine.setPatch(next); // instant, unchanged
                status = "Playing " + playlist.patches[index];
            }
            morphingShown = engine.morphing();
            playlistIndex = index;
            patchFile = path;
        };
        auto showSnapshot = [&](int index) {
            if (index < 0 || index >= static_cast<int>(snapshots.size())) {
                status = "No snapshots loaded";
                return;
            }
            if (!engine.displaySnapshot(snapshots[index], canvasWidth(), height)) {
                status = "Invalid snapshot";
                return;
            }
            snapshotIndex = index;
            status = "Snapshot " + std::to_string(index + 1) + "/" + std::to_string(snapshots.size());
        };
        auto listRows = [&] {
            return showPlaylist ? static_cast<int>(playlist.patches.size())
                                : static_cast<int>(panelRows(engine.patch).size());
        };
        auto last = std::chrono::steady_clock::now();
        const bool timing = std::getenv("WHORLD_TIMING") != nullptr;
        double accTick = 0, accFrame = 0, accCanvas = 0, accPanel = 0, accPresent = 0;
        int accN = 0;
        auto msSince = [](std::chrono::steady_clock::time_point a, std::chrono::steady_clock::time_point b) {
            return std::chrono::duration<double, std::milli>(b - a).count();
        };
        while (running) {
            SDL_Event ev;
            while (SDL_PollEvent(&ev)) {
                panelDirty = true; // any event may change what the sidebar shows
                if (ev.type == SDL_QUIT) running = false;
                else if (ev.type == SDL_WINDOWEVENT && ev.window.event == SDL_WINDOWEVENT_SIZE_CHANGED) {
                    width = ev.window.data1;
                    height = ev.window.data2;
                    panelDirty = true;
                } else if (ev.type == SDL_TEXTINPUT && entering) {
                    if (input.size() < 1024) input += ev.text.text;
                } else if (ev.type == SDL_KEYDOWN) {
                    SDL_Keycode key = ev.key.keysym.sym;
                    bool ctrl = (ev.key.keysym.mod & KMOD_CTRL) != 0;
                    bool shift = (ev.key.keysym.mod & KMOD_SHIFT) != 0;
                    if (entering) {
                        if (key == SDLK_ESCAPE) { entering = false; SDL_StopTextInput(); }
                        else if (key == SDLK_BACKSPACE && !input.empty()) input.pop_back();
                        else if (key == SDLK_RETURN) {
                            entering = false; SDL_StopTextInput();
                            try {
                                std::string failure;
                                if (command == "open") {
                                    Patch patch;
                                    if (!patch.load(input, failure)) throw std::runtime_error(failure);
                                    engine.stopSnapshot();
                                    engine.setPatch(patch); patchFile = input; status = "Loaded " + input;
                                } else if (command == "save") {
                                    if (!engine.patch.save(input, failure)) throw std::runtime_error(failure);
                                    patchFile = input; status = "Saved " + input;
                                } else if (command == "openlist") {
                                    if (!playlist.load(input, failure)) throw std::runtime_error(failure);
                                    if (playlistIndex >= static_cast<int>(playlist.patches.size()))
                                        playlistIndex = playlist.patches.empty() ? -1 : 0;
                                    status = "Loaded " + std::to_string(playlist.patches.size()) + " patches";
                                } else if (command == "savelist") {
                                    if (!playlist.save(input, failure)) throw std::runtime_error(failure);
                                    status = "Saved " + input;
                                } else if (command == "newlist") {
                                    playlist = Playlist{};
                                    playlistIndex = -1;
                                    showPlaylist = true;
                                    status = "New playlist";
                                } else if (command == "takesnap") {
                                    const Snapshot snapshot = engine.captureSnapshot(canvasWidth(), height);
                                    if (!snapshot.save(input, failure)) throw std::runtime_error(failure);
                                    status = "Saved " + input;
                                } else if (command == "loadsnap") {
                                    Snapshot snapshot;
                                    if (!snapshot.load(input, failure)) throw std::runtime_error(failure);
                                    snapshots.push_back(snapshot);
                                    if (!engine.displaySnapshot(snapshot, canvasWidth(), height))
                                        throw std::runtime_error("Invalid snapshot: " + input);
                                    snapshotIndex = static_cast<int>(snapshots.size()) - 1;
                                    status = "Loaded " + input;
                                }
                            } catch (const std::exception& e) { status = e.what(); }
                        }
                    } else if (ctrl && shift && (key == SDLK_o || key == SDLK_s || key == SDLK_n)) {
                        command = key == SDLK_o ? "openlist" : key == SDLK_s ? "savelist" : "newlist";
                        input = playlist.path.empty() ? "playlist.whl" : playlist.path;
                        entering = true; SDL_StartTextInput(); status = command + " playlist:";
                    } else if (ctrl && shift && key == SDLK_a) {
                        if (patchFile.empty()) status = "Save a patch first";
                        else {
                            playlist.patches.push_back(patchFile);
                            playlistIndex = static_cast<int>(playlist.patches.size()) - 1;
                            showPlaylist = true;
                            status = "Added " + patchFile;
                        }
                    } else if (ctrl && key == SDLK_t) {
                        command = "takesnap"; input = "snapshot.whs";
                        entering = true; SDL_StartTextInput(); status = "take snapshot:";
                    } else if (ctrl && key == SDLK_l) {
                        command = "loadsnap"; input = "snapshot.whs";
                        entering = true; SDL_StartTextInput(); status = "load snapshot:";
                    } else if (ctrl && (key == SDLK_o || key == SDLK_s)) {
                        command = key == SDLK_o ? "open" : "save";
                        input = patchFile.empty() ? "patch.whp" : patchFile;
                        entering = true; SDL_StartTextInput(); status = command + " patch:";
                    } else if (ctrl && key == SDLK_m) {
                        const std::string target = learnTargetName();
                        if (shift) {
                            // Forget the mapping(s) that drive the selected row.
                            const std::size_t before = playlist.mappings.size();
                            playlist.mappings.erase(std::remove_if(playlist.mappings.begin(),
                                playlist.mappings.end(), [&](const Mapping& m) { return m.target == target; }),
                                playlist.mappings.end());
                            status = target.empty() ? "No target selected"
                                 : "Forgot " + std::to_string(before - playlist.mappings.size()) +
                                   " mapping(s) for " + target;
                        } else if (target.empty()) {
                            status = "MIDI learn: show parameters first (L)";
                        } else {
                            midiLearn = !midiLearn;
                            status = midiLearn ? "MIDI learn: move a control for " + target : "MIDI learn: off";
                        }
                    } else if (key == SDLK_ESCAPE) running = false;
                    else if (key == SDLK_SPACE) {
                        if (movieActive) { movieActive = false; engine.stopSnapshot(); paused = false; }
                        else if (engine.snapshotActive()) { engine.stopSnapshot(); paused = false; }
                        else paused = !paused;
                    }
                    else if (key == SDLK_F11) SDL_SetWindowFullscreen(window,
                        SDL_GetWindowFlags(window) & SDL_WINDOW_FULLSCREEN_DESKTOP ? 0 : SDL_WINDOW_FULLSCREEN_DESKTOP);
                    else if (key == SDLK_m) {
                        postProcess.mirrorMode = (postProcess.mirrorMode + 1) % 4;
                        const char* mNames[] = {"Mirror: Off", "Mirror: 4-Way Quad", "Mirror: 8-Way Kaleidoscope", "Mirror: 12-Way Radial"};
                        status = mNames[postProcess.mirrorMode];
                    } else if (key == SDLK_b) {
                        postProcess.blendMode = (postProcess.blendMode + 1) % 3;
                        const char* bNames[] = {"Blend: Normal", "Blend: Additive", "Blend: Screen"};
                        status = bNames[postProcess.blendMode];
                    } else if (key == SDLK_i) {
                        postProcess.invertColor = !postProcess.invertColor;
                        engine.patch.invertColor = postProcess.invertColor;
                        status = postProcess.invertColor ? "Invert: ON" : "Invert: OFF";
                    } else if (key == SDLK_w) {
                        postProcess.warpEnabled = !postProcess.warpEnabled;
                        if (!postProcess.warpEnabled) {
                            status = "Warp: off";
                        } else {
                            char wbuf[160];
                            std::snprintf(wbuf, sizeof(wbuf),
                                "Warp: on TL(%.3f,%.3f) TR(%.3f,%.3f) BR(%.3f,%.3f) BL(%.3f,%.3f)",
                                postProcess.warp.tl.x, postProcess.warp.tl.y,
                                postProcess.warp.tr.x, postProcess.warp.tr.y,
                                postProcess.warp.br.x, postProcess.warp.br.y,
                                postProcess.warp.bl.x, postProcess.warp.bl.y);
                            status = wbuf;
                        }
                    } else if (!ctrl && key == SDLK_t) {
                        if (postProcess.feedback <= 0.01) postProcess.feedback = 0.70;
                        else if (postProcess.feedback < 0.80) postProcess.feedback = 0.85;
                        else if (postProcess.feedback < 0.90) postProcess.feedback = 0.93;
                        else postProcess.feedback = 0.0;
                        if (postProcess.feedback <= 0.01) status = "Feedback trails: OFF";
                        else {
                            char fbuf[48];
                            std::snprintf(fbuf, sizeof(fbuf), "Feedback trails: %.2f", postProcess.feedback);
                            status = fbuf;
                        }
                    } else if (key == SDLK_h) hidden = !hidden;
                    else if (key == SDLK_r) { engine.clear(); if (useGl) gl.clearFeedback(); }
                    else if (key == SDLK_f) engine.setDrawMode(engine.patch.drawMode ^ DM_FILL);
                    else if (key == SDLK_o) engine.setDrawMode(engine.patch.drawMode ^ DM_OUTLINE);
                    else if (key == SDLK_p) {
                        try { image(engine, "whorld.png", std::max(64, width - (hidden ? 0 : sidebar)), height); status = "Saved whorld.png"; }
                        catch (const std::exception& e) { status = e.what(); }
                    } else if (key == SDLK_l) showPlaylist = !showPlaylist;
                    else if (key == SDLK_s) {
                        autoScale = !autoScale;
                        if (!autoScale) renderScale = 1.0;
                        status = autoScale ? "Render scale: auto" : "Render scale: 1.00 (fixed)";
                    } else if (key == SDLK_MINUS || key == SDLK_EQUALS) {
                        autoScale = false;
                        renderScale = std::clamp(renderScale + (key == SDLK_EQUALS ? .25 : -.25), minScale, 2.0);
                        char buf[32];
                        std::snprintf(buf, sizeof(buf), "Render scale: %.2f", renderScale);
                        status = buf;
                    }
                    else if (key == SDLK_PAGEUP) showSnapshot(snapshotIndex - 1);
                    else if (key == SDLK_PAGEDOWN) showSnapshot(snapshotIndex + 1);
                    else if (showPlaylist && key == SDLK_RETURN) playPatch(playlistIndex);
                    else if (showPlaylist && key == SDLK_DELETE) {
                        if (playlistIndex >= 0 && playlistIndex < static_cast<int>(playlist.patches.size())) {
                            playlist.patches.erase(playlist.patches.begin() + playlistIndex);
                            if (playlistIndex >= static_cast<int>(playlist.patches.size()))
                                playlistIndex = static_cast<int>(playlist.patches.size()) - 1;
                            scroll = std::clamp(scroll, 0, std::max(0, listRows() - 1));
                            status = "Removed entry";
                        }
                    } else if (showPlaylist && (key == SDLK_LEFTBRACKET || key == SDLK_RIGHTBRACKET)) {
                        playPatch(playlistIndex + (key == SDLK_RIGHTBRACKET ? 1 : -1));
                    } else if (showPlaylist && key == SDLK_UP) {
                        if (playlistIndex > 0) --playlistIndex;
                        else if (playlistIndex < 0 && !playlist.patches.empty()) playlistIndex = 0;
                    } else if (showPlaylist && key == SDLK_DOWN) {
                        if (playlistIndex < static_cast<int>(playlist.patches.size()) - 1) ++playlistIndex;
                    } else if (key == SDLK_TAB) property = (property + 1) % 6;
                    else if (key == SDLK_UP) selected = std::max(0, selected - 1);
                    else if (key == SDLK_DOWN) selected = std::min(listRows() - 1, selected + 1);
                    else if (key == SDLK_LEFT || key == SDLK_RIGHT) {
                        int dir = key == SDLK_LEFT ? -1 : 1;
                        double factor = (ev.key.keysym.mod & KMOD_SHIFT) ? 10 : 1;
                        const auto rows = panelRows(engine.patch);
                        const PanelRow& row = rows[std::clamp(selected, 0, static_cast<int>(rows.size()) - 1)];
                        if (!row.master) {
                            const int index = row.index;
                            auto& pr = engine.patch.params[index];
                            double delta = (paramInfo[index].max - paramInfo[index].min) / 200 * factor * dir;
                            switch (property) {
                            case 0:
                                if (index == PARAM_Primitive || index == PARAM_StarPoints)
                                    pr.value = std::clamp(double(int(std::lround(pr.value)) + dir),
                                                          paramInfo[index].min, paramInfo[index].max);
                                else
                                    pr.value = std::clamp(pr.value + delta, paramInfo[index].min, paramInfo[index].max);
                                break;
                            case 1: pr.wave = (pr.wave + dir + 8) % 8; break;
                            case 2: pr.amp = std::clamp(pr.amp + delta, -100.0, 100.0); break;
                            case 3: pr.freq = std::clamp(pr.freq + dir * .01 * factor, -100.0, 100.0); break;
                            case 4: pr.pw = std::clamp(pr.pw + dir * .01 * factor, 0.0, 1.0); break;
                            case 5: pr.global = std::clamp(pr.global + delta, -100.0, 100.0); break;
                            }
                        } else {
                            auto& val = engine.patch.master[row.index];
                            val = std::clamp(val + dir * .05 * factor, -100.0, 2000.0);
                        }
                    }
                } else if (ev.type == SDL_CONTROLLERDEVICEADDED) {
                    openGameControllers(controllers, gamepadReady);
                    if (offerGamepadDefaults())
                        status = "Gamepad defaults loaded (save playlist to keep)";
                    else {
                        const std::string summary = controllerSummary(gamepadNames(controllers));
                        if (!summary.empty()) status = summary;
                    }
                } else if (ev.type == SDL_CONTROLLERDEVICEREMOVED) {
                    closeGameController(controllers, ev.cdevice.which);
                    const std::string summary = controllerSummary(gamepadNames(controllers));
                    status = summary.empty() ? "Controller disconnected" : summary;
                } else if (ev.type == SDL_MOUSEWHEEL && !hidden) {
                    int x, y; SDL_GetMouseState(&x, &y);
                    if (x < sidebar && y >= kListTop) {
                        int visible = std::max(1, (std::max(kListTop, height - 157) - kListTop) / rowHeight);
                        scroll = std::clamp(scroll - ev.wheel.y * 2, 0, std::max(0, listRows() - visible));
                        if (showPlaylist) {
                            playlistIndex = std::clamp(playlistIndex, scroll,
                                std::min(listRows() - 1, scroll + visible - 1));
                        } else {
                            selected = std::clamp(selected, scroll,
                                std::min(listRows() - 1, scroll + visible - 1));
                        }
                    }
                } else if (ev.type == SDL_MOUSEBUTTONDOWN && ev.button.button == SDL_BUTTON_LEFT) {
                    if (!hidden && ev.button.x < sidebar) {
                        const int bottom = std::max(kListTop, height - 157);
                        if (ev.button.y >= kShapeStripTop && ev.button.y < kShapeStripTop + kShapeStripHeight) {
                            // Click a shape chip to activate that primitive.
                            const int chip = shapeChipAtX(ev.button.x);
                            if (chip >= 0) engine.patch.params[PARAM_Primitive].value = chip;
                        } else if (ev.button.y >= kListTop && ev.button.y < bottom) {
                            const int row = scroll + (ev.button.y - kListTop) / rowHeight;
                            if (showPlaylist) {
                                if (row >= 0 && row < static_cast<int>(playlist.patches.size())) {
                                    playlistIndex = row;
                                    if (ev.button.clicks > 1) playPatch(playlistIndex);
                                }
                            } else {
                                selected = std::clamp(row, 0, listRows() - 1);
                                const auto rows = panelRows(engine.patch);
                                const PanelRow& sel = rows[selected];
                                if (!sel.master && (ev.button.y - kListTop) % rowHeight >= 21) {
                                    const auto& info = paramInfo[sel.index];
                                    double f = std::clamp((ev.button.x - 15.0) / (sidebar - 30.0), 0.0, 1.0);
                                    engine.patch.params[sel.index].value =
                                        info.min + f * (info.max - info.min);
                                }
                            }
                        }
                    } else dragging = true;
                } else if (ev.type == SDL_MOUSEBUTTONUP) dragging = false;
                else if (ev.type == SDL_MOUSEMOTION && dragging) {
                    int left = hidden ? 0 : sidebar;
                    mx = std::clamp((ev.motion.x - left) / double(std::max(1, width - left)), 0.0, 1.0);
                    my = std::clamp(ev.motion.y / double(std::max(1, height)), 0.0, 1.0);
                    engine.patch.orgMotion = OM_DRAG;
                }
            }
            // Reconnect a configured source that was missing at launch or whose
            // subscription dropped. ensureOpen() rate-limits itself, so this is
            // cheap every frame and never busy-loops.
            if (!midi.isOpen() && midi.ensureOpen(midiError)) {
                status = "MIDI reconnected " + midi.portName();
                panelDirty = true;
            }
            // Poll MIDI once per frame (previously only ran while dragging).
            if (midi.isOpen()) {
                for (const MidiMessage& message : midi.poll()) {
                    if (midiLearn) {
                        const std::string target = learnTargetName();
                        if (!target.empty()) {
                            const auto rows = panelRows(engine.patch);
                            const bool master = selected >= 0 && selected < static_cast<int>(rows.size()) &&
                                                rows[selected].master;
                            const std::string prop = master ? "Val" : kProps[property];
                            learnMidi(playlist, message, target, prop);
                            status = "Learned " + midiEventName(message.event) + " ch" +
                                     std::to_string(message.channel + 1) + " #" +
                                     std::to_string(message.control) + " -> " + target;
                        }
                        midiLearn = false;
                        panelDirty = true;
                        continue;
                    }
                    dispatchMidi(message);
                }
            }
            // Gamepad: refresh state once per frame, then read every open pad
            // (only changed buttons emit). Poll while learning too, even with no
            // mappings, or learn never fires.
            if (gamepadReady) SDL_GameControllerUpdate();
            if (!controllers.empty() && (midiLearn || !playlist.gamepadMappings.empty())) {
                for (GamepadDevice& device : controllers) {
                    const GamepadState current = readGamepad(device.handle);
                    if (midiLearn) {
                        // Learn works from any connected pad; one press ends it.
                        const int control = gamepadLearnControl(current, device.previous);
                        if (control >= 0) {
                            const std::string target = learnTargetName();
                            if (!target.empty()) {
                                const auto rows = panelRows(engine.patch);
                                const bool master = selected >= 0 && selected < static_cast<int>(rows.size()) &&
                                                    rows[selected].master;
                                const std::string name = gamepadControlName(control);
                                learnGamepad(playlist, name, target, master ? "Val" : kProps[property]);
                                status = "Learned " + name + " -> " + target;
                                panelDirty = true;
                            }
                            midiLearn = false; // a learned control must not also fire its action
                        }
                    } else {
                        for (const MappedAction& action : mapGamepad(playlist, current, device.previous)) {
                            applyMidiAction(action, engine, playlist, paused, playlistIndex, status);
                            syncTarget(action.target);
                            panelDirty = true;
                            // Rumble once per press of a momentary/action target.
                            if (momentaryTarget(action.target) && action.amount != 0)
                                nudgeRumble(device.handle);
                        }
                    }
                    device.previous = current;
                }
            }
            // Audio: continuously map level/bands onto the configured targets.
            if (audioReady) {
                audio.poll();
                for (const MappedAction& action : mapAudio(playlist, audio.features())) {
                    applyMidiAction(action, engine, playlist, paused, playlistIndex, status);
                    syncTarget(action.target);
                }
            }
            int left = hidden ? 0 : sidebar;
            int cw = width - left;
            if (cw < 1 || height < 1) { SDL_Delay(20); continue; }
            auto frameStart = std::chrono::steady_clock::now();
            // Movie playback: advance frames by wall clock and display them via
            // the engine's snapshot path, which freezes live animation.
            if (movieActive) {
                const double elapsed = std::chrono::duration<double>(
                    std::chrono::steady_clock::now() - movieStart).count();
                size_t idx = size_t(elapsed * std::max(1.0, movie.frameRate())) % movie.frameCount();
                if (idx != movieIndex || !movieHasShown) {
                    Snapshot snap;
                    if (movie.frame(idx, snap, error)) {
                        engine.displaySnapshot(snap, cw, height);
                        movieIndex = idx;
                        movieHasShown = true;
                    } else {
                        movieActive = false;
                        status = error;
                    }
                }
            }
            // The core runs at 60 simulation ticks/second; draw at 30 FPS.
            if (!paused) {
                engine.tick(cw, height, mx, my);
                engine.tick(cw, height, mx, my);
            }
            // Keep a status note visible for the duration of a patch crossfade.
            if (engine.morphing()) {
                if (!morphingShown) panelDirty = true;
                status = "Morphing to " + playingPatch;
                morphingShown = true;
            } else if (morphingShown) {
                status = "Playing " + playingPatch;
                morphingShown = false;
                panelDirty = true;
            }
            auto tTick = std::chrono::steady_clock::now();
            const Frame frame = engine.frame(cw, height);
            auto tFrame = std::chrono::steady_clock::now();

            // The sidebar is cached and only redrawn when needed, or every
            // 200 ms to refresh values such as the live ring count.
            SDL_Rect canvasDst{left, 0, cw, height};
            cairo_surface_t* uiSurface = nullptr;
            if (!hidden) {
                const auto sincePanel = std::chrono::duration_cast<std::chrono::milliseconds>(
                    frameStart - lastPanel).count();
                if (sincePanel >= 200) { panelDirty = true; lastPanel = frameStart; }
                if (panel.w != sidebar || panel.h != height) resetPanel(panel, renderer, sidebar, height);
                if (panelDirty) {
                    Context cr(cairo_create(panel.surface.get()), cairo_destroy);
                    drawPanel(cr.get(), engine, playlist, playlistIndex, showPlaylist, width,
                              height, selected, scroll, property, paused, status, input, entering,
                              midiLearn);
                    cairo_surface_flush(panel.surface.get());
                    panelDirty = false;
                    if (useGl) uiSurface = panel.surface.get();
                    else uploadTexture(panel.texture, panel.surface.get());
                }
            }
            // Canvas: OpenGL draws the geometry with the GPU (stencil even-odd
            // fills, MSAA); otherwise cairo rasterizes at renderScale and SDL
            // stretches the result over the canvas rectangle.
            if (useGl) {
                gl.render(frame, engine.patch.antialiasing, left, cw, height, width, height,
                          uiSurface, hidden ? 0 : sidebar, height, renderScale, postProcess);
                gl.swap(window);
                if (streamer.active()) {
                    const std::size_t fBytes = static_cast<std::size_t>(cw) * height * 3;
                    static std::vector<unsigned char> streamBuf;
                    static std::vector<unsigned char> streamFlipped;
                    if (streamBuf.size() != fBytes) {
                        streamBuf.resize(fBytes);
                        streamFlipped.resize(fBytes);
                    }
                    glReadPixels(left, 0, cw, height, GL_RGB, GL_UNSIGNED_BYTE, streamBuf.data());
                    for (int y = 0; y < height; ++y) {
                        std::memcpy(streamFlipped.data() + y * cw * 3,
                                    streamBuf.data() + (height - 1 - y) * cw * 3,
                                    cw * 3);
                    }
                    streamer.writeFrame(streamFlipped.data(), streamFlipped.size());
                }
            } else {
                const int texW = std::max(1, int(std::lround(cw * renderScale)));
                const int texH = std::max(1, int(std::lround(height * renderScale)));
                if (canvas.texW != texW || canvas.texH != texH) resetCanvas(canvas, renderer, texW, texH);
                {
                    Context cr(cairo_create(canvas.surface.get()), cairo_destroy);
                    cairo_scale(cr.get(), double(texW) / cw, double(texH) / height);
                    drawFrameCairo(cr.get(), frame, cw, height, engine.patch.antialiasing,
                                   postProcess.blendMode);
                }
                // Mirror/invert on the CPU; feedback trails remain GPU-only.
                if (postProcess.feedback > 0.01 && !warnedCairoFeedback) {
                    warnedCairoFeedback = true;
                    std::cerr << "whorld: feedback trails require the GPU backend; "
                                 "the cairo canvas ignores them\n";
                }
                postProcessCairo(canvas.surface.get(), postProcess, frame.background);
                cairo_surface_flush(canvas.surface.get());
                uploadTexture(canvas.texture, canvas.surface.get());
                SDL_RenderClear(renderer);
                SDL_RenderCopy(renderer, canvas.texture, nullptr, &canvasDst);
                if (!hidden) {
                    SDL_Rect panelDst{0, 0, sidebar, height};
                    SDL_RenderCopy(renderer, panel.texture, nullptr, &panelDst);
                }
                SDL_RenderPresent(renderer);
            }
            auto tPanel = std::chrono::steady_clock::now();
            auto tCanvas = tPanel;
            auto workEnd = tPanel;
            if (timing) {
                accTick += msSince(frameStart, tTick);
                accFrame += msSince(tTick, tFrame);
                accCanvas += msSince(tFrame, tCanvas);
                accPanel += msSince(tCanvas, tPanel);
                accPresent += msSince(tPanel, workEnd);
                if (++accN >= 60) {
                    std::fprintf(stderr, "timing ms: tick=%.2f frame=%.2f canvas=%.2f "
                        "panel=%.2f present=%.2f scale=%.2f %s rings=%zu\n", accTick / accN, accFrame / accN,
                        accCanvas / accN, accPanel / accN, accPresent / accN, renderScale,
                        useGl ? "gl" : "cairo", engine.ringCount());
                    accTick = accFrame = accCanvas = accPanel = accPresent = 0;
                    accN = 0;
                }
            }

            // Adaptive render scale: keep frame cost under the 33 ms budget
            // without dropping below minScale; raise it back if we have room.
            const double workMs = std::chrono::duration<double, std::milli>(workEnd - frameStart).count();
            smoothWorkMs = smoothWorkMs > 0 ? smoothWorkMs * .8 + workMs * .2 : workMs;
            if (autoScale) {
                const auto sinceAdjust = std::chrono::duration_cast<std::chrono::milliseconds>(
                    workEnd - lastAdjust).count();
                if (sinceAdjust >= 500) {
                    if (smoothWorkMs > 30 && renderScale > minScale + 1e-6)
                        renderScale = std::max(minScale, renderScale - .1);
                    else if (smoothWorkMs < 12 && renderScale < 1.0 - 1e-6)
                        renderScale = std::min(1.0, renderScale + .1);
                    lastAdjust = workEnd;
                }
            }
            int visible = std::max(1, (std::max(kListTop, height - 157) - kListTop) / rowHeight);
            const int tracked = showPlaylist ? playlistIndex : selected;
            if (tracked < scroll) scroll = tracked;
            else if (tracked >= scroll + visible) scroll = tracked - visible + 1;
            scroll = std::clamp(scroll, 0, std::max(0, listRows() - visible));
            // Switching primitives can shorten the row list; keep the cursor in range.
            if (!showPlaylist) selected = std::clamp(selected, 0, std::max(0, listRows() - 1));
            auto now = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - last).count();
            if (elapsed < 33) SDL_Delay(static_cast<Uint32>(33 - elapsed));
            last = std::chrono::steady_clock::now();
        }
        if (canvas.texture) SDL_DestroyTexture(canvas.texture);
        if (panel.texture) SDL_DestroyTexture(panel.texture);
        closeAllGameControllers(controllers);
        audio.close();
        gl.shutdown();
        if (renderer) SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
    } catch (const std::exception& e) {
        std::cerr << "whorld: " << e.what() << '\n';
        return 1;
    }
}
