// Linux port of Whorld's patch format, animation and geometry (GPL-3.0-or-later).
#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

namespace whorld {
constexpr double PI = 3.14159265358979323846;
constexpr double DTR(double x) { return x * PI / 180.0; }
constexpr int MAX_RINGS = 1000;
// Upper bound on frames accepted from a .whm movie, so a malformed file cannot
// make the reader allocate or iterate without limit.
constexpr uint64_t MAX_MOVIE_FRAMES = 1u << 20;
enum Param {
#define PARAMDEF(name, minval, maxval, steps, scale, initval) PARAM_##name,
#include "../trunk/Whorld/WhorldDef.h"
#define SHAPEDEF(name, minval, maxval, steps, initval) PARAM_##name,
#include "shape_def.h"
    PARAM_COUNT
};
// The port-only shape parameters are appended after the upstream ones, and
// PARAM_Primitive is the first of them; the sidebar dims the ones that do not
// apply to the currently selected primitive.
constexpr int SHAPE_PARAM_BEGIN = PARAM_Primitive;
// Shape primitives. A ring keeps the primitive that was selected when it was
// born, like every other geometry parameter.
enum Primitive {
    PRIM_Polygon = 0, PRIM_Circle, PRIM_Square, PRIM_Triangle, PRIM_Star, PRIM_Cross, PRIM_Line,
    PRIM_COUNT
};
extern const std::array<const char*, PRIM_COUNT> primitiveNames;
// True if `param` (a shape parameter) affects the given primitive.
bool shapeParamApplies(int param, int primitive);
struct RingShape {
    int primitive = PRIM_Polygon;
    double corner = 0;                    // corner rounding (polygon, square, triangle)
    double arcStart = 0, arcSweep = 360;  // circle arc, in degrees
    double points = 5, inner = .5, sharp = 0; // star
    double crossWidth = .4, crossLength = 1;  // cross
    double barLength = 1, barThickness = .2;  // line
};
enum Master {
#define MASTERDEF(name, type, prefix, initval) MASTER_##name,
#include "../trunk/Whorld/WhorldDef.h"
    MASTER_COUNT
};
enum { DM_FILL = 1, DM_OUTLINE = 2, OM_PARK = 0, OM_DRAG = 1, OM_RANDOM = 2 };
struct ParamInfo { const char* name; double min, max, initial; };
extern const std::array<ParamInfo, PARAM_COUNT> paramInfo;
extern const std::array<const char*, MASTER_COUNT> masterNames;
struct Point { double x = 0, y = 0; };
struct Color { double r = 0, g = 0, b = 0, a = 1; };
struct ParamRow { double value = 0; int wave = 0; double amp = 0, freq = 0, pw = 0.5, global = 0; };
struct Patch {
    std::array<ParamRow, PARAM_COUNT> params{};
    std::array<double, MASTER_COUNT> master{};
    Point origin{0.5, 0.5};
    int drawMode = 0, orgMotion = OM_PARK;
    double hue = 0;
    bool mirror = false, reverse = false, convex = false, invertColor = false;
    bool loopHue = false, zoomCenter = false, antialiasing = true;
    Patch();
    // On error, do not change the existing patch. Unknown tags are ignored.
    bool load(const std::string& path, std::string& error);
    bool save(const std::string& path, std::string& error) const;
};
struct Segment { Point c1, c2, end; bool curved = false; };
struct Path { Point start; std::vector<Segment> segments; Point center; };
struct Shape {
    Path outer, inner;
    bool fill = false, hasInner = false, outline = false;
    Color color, outlineColor;
    double lineWidth = 1;
};
struct Frame { Color background; std::vector<Shape> shapes; };
struct Snapshot; // defined in snapshot.h
class Engine {
public:
    Patch patch;
    Engine();
    void setPatch(const Patch& next); // retains existing rings, like upstream
    // Crossfades the live patch toward `target` over `seconds` of simulation
    // time (ticks run at 60 Hz). `value`, `amp`, `freq`, `pw`, `global`,
    // `master[]` and `hue` are interpolated linearly; the waveform switches at
    // the halfway point. Other fields change only when the morph completes,
    // where the engine lands exactly on `target` (as setPatch) and stops. A
    // non-positive `seconds` falls back to an instant setPatch. Any explicit
    // setPatch cancels an active morph.
    void morphTo(const Patch& target, double seconds);
    bool morphing() const;
    void setDrawMode(int mode); // changes existing rings too
    void clear();
    // Gives every oscillator a fresh random phase, like upstream's
    // Random Phase action. Deterministic for a given engine (uses its RNG).
    void randomizePhase();
    void tick(int width, int height, double mouseX = 0.5, double mouseY = 0.5);
    Frame frame(int width, int height); // calculates visibility and returns ordered geometry
    std::size_t ringCount() const;
    Snapshot captureSnapshot(int width, int height) const;
    // Displays a frozen copy, preserving the live simulation until stopSnapshot().
    bool displaySnapshot(const Snapshot& snapshot, int width, int height);
    void stopSnapshot();
    bool snapshotActive() const;
private:
    struct Impl;
    Frame render(int width, int height, Impl& state, const Snapshot* snapshot);
    // Shared ownership is not intended: each engine owns one simulation.
    Impl* impl;
public:
    ~Engine();
    Engine(const Engine&) = delete;
    Engine& operator=(const Engine&) = delete;
};
} // namespace whorld
