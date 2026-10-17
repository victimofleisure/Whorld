// Native, dependency-free implementation of Whorld's patch and drawing model.
// Based on trunk/Whorld/{Patch,WhorldBase,WhorldDraw,Oscillator,hls}.cpp.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "whorld_core.h"
#include "snapshot.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <deque>
#include <fstream>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <string>
#include <type_traits>

namespace whorld {

const std::array<ParamInfo, PARAM_COUNT> paramInfo = {{
#define PARAMDEF(name, minval, maxval, steps, scale, initval) {#name, minval, maxval, initval},
#include "../trunk/Whorld/WhorldDef.h"
#define SHAPEDEF(name, minval, maxval, steps, initval) {#name, minval, maxval, initval},
#include "shape_def.h"
}};
const std::array<const char*, PRIM_COUNT> primitiveNames = {{
    "Polygon", "Circle", "Square", "Triangle", "Star", "Cross", "Line"
}};
bool shapeParamApplies(int param, int primitive) {
    switch (param) {
    case PARAM_Primitive: return true;
    case PARAM_PolyCorner: return primitive == PRIM_Polygon;
    case PARAM_CircleStart:
    case PARAM_CircleSweep: return primitive == PRIM_Circle;
    case PARAM_SquareCorner: return primitive == PRIM_Square;
    case PARAM_TriCorner: return primitive == PRIM_Triangle;
    case PARAM_StarPoints:
    case PARAM_StarInner:
    case PARAM_StarSharp: return primitive == PRIM_Star;
    case PARAM_CrossWidth:
    case PARAM_CrossLength: return primitive == PRIM_Cross;
    case PARAM_BarLength:
    case PARAM_BarThickness: return primitive == PRIM_Line;
    default: return true;
    }
}
const std::array<const char*, MASTER_COUNT> masterNames = {{
#define MASTERDEF(name, type, prefix, initval) #name,
#include "../trunk/Whorld/WhorldDef.h"
}};

namespace {
constexpr double TAU = 2 * PI;
constexpr int MAX_SIDES = 50; // upstream limit (also bounds hostile global PolySides)
constexpr double MAX_VALUE = 1e6;

double finite(double x, double fallback = 0) {
    return std::isfinite(x) ? std::clamp(x, -MAX_VALUE, MAX_VALUE) : fallback;
}
double wrap(double x, double limit) {
    double v = std::fmod(x, limit);
    return v < 0 ? v + limit : v;
}
double reflect(double x, double limit) {
    const double v = wrap(x, 2 * limit);
    return v < limit ? v : 2 * limit - v;
}
double pow2(double exponent) { return std::exp2(std::clamp(finite(exponent), -20.0, 20.0)); }
Color hls(double hue, double light, double sat, double alpha) {
    const double l = std::clamp(finite(light), 0.0, 1.0);
    const double s = std::clamp(finite(sat), 0.0, 1.0);
    const double m2 = l <= 0.5 ? l + l * s : l + s - l * s;
    const double m1 = 2 * l - m2;
    auto channel = [=](double h) {
        h = wrap(h, 360);
        if (h < 60) return m1 + (m2 - m1) * h / 60;
        if (h < 180) return m2;
        if (h < 240) return m1 + (m2 - m1) * (240 - h) / 60;
        return m1;
    };
    hue = finite(hue);
    return {channel(hue + 120), channel(hue), channel(hue - 120),
            std::clamp(finite(alpha), 0.0, 1.0)};
}
// Fixed private RNG: no process-global rand() and reproducible frame sequences.
double random01(uint32_t& state) {
    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;
    return double(state) / double(std::numeric_limits<uint32_t>::max());
}
struct Oscillator {
    double clock = 0, frequency = 0, previousPhase = 0, phaseOffset = 0, randomValue = 0, previousValue = 0;
    int wave = 0;
    double pw = 0.5;
    void frequencyTo(double f) {
        f = finite(f);
        // Preserve the effective phase, offset included, across a frequency change.
        const double phase = wrap(clock * frequency / 60 + phaseOffset, 1);
        frequency = f;
        clock = f ? (phase - phaseOffset) * 60 / f : 0;
    }
    double value(double t, uint32_t& rng) {
        double pos = finite(t * frequency / 60) + phaseOffset;
        double phase = wrap(pos, 1);
        double v = 0;
        switch (wave) {
        case 0: {
            const double p = wrap(pos * 2 + 0.5, 2);
            v = p < 1 ? 2 * p - 1 : 3 - 2 * p;
            break;
        }
        case 1: v = std::sin(pos * TAU); break;
        case 2: v = phase * 2 - 1; break;
        case 3: v = 1 - phase * 2; break;
        case 4: v = phase < 0.5 ? 1 : -1; break;
        case 5: v = phase < pw ? 1 : -1; break;
        case 6: case 7:
            if (phase < 0.5 && previousPhase > 0.5) {
                previousValue = randomValue;
                randomValue = random01(rng) * 2 - 1;
            }
            previousPhase = phase;
            v = wave == 6 ? randomValue : previousValue + (randomValue - previousValue) * phase;
            break;
        default: break;
        }
        return v;
    }
};

bool number(std::istringstream& in, double& result) {
    double value;
    if (!(in >> value) || !std::isfinite(value)) return false;
    result = value;
    return true;
}
bool integer(std::istringstream& in, int& result) {
    long long value;
    if (!(in >> value) || value < std::numeric_limits<int>::min() ||
        value > std::numeric_limits<int>::max()) return false;
    result = static_cast<int>(value);
    return true;
}
bool boolean(std::istringstream& in, bool& result) {
    std::string v;
    if (!(in >> v)) return false;
    if (v == "1" || v == "true" || v == "TRUE") result = true;
    else if (v == "0" || v == "false" || v == "FALSE") result = false;
    else return false;
    return true;
}
const std::array<const char*, 11> mainNames = {{
#define MAINDEF(name, type, prefix, initval, variant) #name,
#include "../trunk/Whorld/WhorldDef.h"
}};

// Global ring offsets, in the same precomputed form that snapshots store on
// disk: the renderer adds these to each ring's own values.
struct GlobalRing {
    double rot = 0, star = 1, pinwheel = 0;
    Point scale{1, 1}, shift{};
    double evenCurve = 0, oddCurve = 0, evenShear = 1, oddShear = 1, lineWidth = 0;
    int polySides = 0;
};
GlobalRing globalsFromParams(const std::array<double, PARAM_COUNT>& g) {
    GlobalRing out;
    out.rot = g[PARAM_RotateSpeed] / 5 * 180;
    out.star = std::max(pow2(g[PARAM_StarFactor]), 0.01);
    out.pinwheel = g[PARAM_Pinwheel];
    const double aspect = std::max(pow2(g[PARAM_AspectRatio]), 1e-9);
    out.scale = {aspect > 1 ? aspect : 1, aspect < 1 ? 1 / aspect : 1};
    out.shift = {std::sin(g[PARAM_SkewAngle]) * g[PARAM_SkewRadius],
                 std::cos(g[PARAM_SkewAngle]) * g[PARAM_SkewRadius]};
    out.evenCurve = g[PARAM_EvenCurve];
    out.oddCurve = g[PARAM_OddCurve] / out.star;
    out.evenShear = g[PARAM_EvenShear] + 1;
    out.oddShear = g[PARAM_OddShear] + 1;
    out.lineWidth = g[PARAM_LineWidth];
    out.polySides = int(std::round(g[PARAM_PolySides]));
    return out;
}
GlobalRing globalsFromSnapshot(const SnapshotGlobal& g) {
    GlobalRing out;
    out.rot = finite(g.rot);
    out.star = std::max(finite(g.starRatio, 1), 1e-6);
    out.pinwheel = finite(g.pinwheel);
    out.scale = {finite(g.scale.x, 1), finite(g.scale.y, 1)};
    out.shift = {finite(g.shift.x), finite(g.shift.y)};
    out.evenCurve = finite(g.evenCurve);
    out.oddCurve = finite(g.oddCurve);
    out.evenShear = finite(g.evenShear, 1);
    out.oddShear = finite(g.oddShear, 1);
    out.lineWidth = finite(g.lineWidth);
    out.polySides = g.sides;
    return out;
}
SnapshotGlobal snapshotFromGlobals(const GlobalRing& g) {
    SnapshotGlobal out;
    out.rot = g.rot;
    out.starRatio = g.star;
    out.pinwheel = g.pinwheel;
    out.scale = g.scale;
    out.shift = g.shift;
    out.evenCurve = g.evenCurve;
    out.oddCurve = g.oddCurve;
    out.evenShear = g.evenShear;
    out.oddShear = g.oddShear;
    out.lineWidth = g.lineWidth;
    out.sides = g.polySides;
    return out;
}
// --- Port-only shape primitive geometry -------------------------------------
// Outlines are built in the ring's local frame: the origin is the ring center
// and coordinates are already multiplied by the ring's anisotropic scale, so
// they only need the shared origin offset and window scale applied afterwards.

std::vector<Point> polygonVerts(int sides, double rotEven, double rotOdd,
                                double rEven, double rOdd, double sx, double sy) {
    const int count = std::max(1, sides) * 2;
    std::vector<Point> v;
    v.reserve(count);
    for (int j = 0; j < count; ++j) {
        const int odd = j & 1;
        const double theta = PI / sides * j + (odd ? rotOdd : rotEven);
        const double rho = odd ? rOdd : rEven;
        v.push_back({std::sin(theta) * sx * rho, std::cos(theta) * sy * rho});
    }
    return v;
}
Path straightPath(const std::vector<Point>& v) {
    Path p;
    p.start = v.front();
    for (std::size_t i = 1; i < v.size(); ++i) p.segments.push_back({Point{}, Point{}, v[i], false});
    return p;
}
// Replaces every corner with a cubic bezier, approximating a fillet of radius
// k * min(adjacent edge lengths) / 2. k = 0 reproduces the straight polygon.
Path roundedPath(const std::vector<Point>& v, double k) {
    const int n = static_cast<int>(v.size());
    if (n < 3 || k <= 1e-6) return straightPath(v);
    Path p;
    std::vector<Point> first(n), second(n);
    for (int i = 0; i < n; ++i) {
        const Point& a = v[(i + n - 1) % n];
        const Point& b = v[i];
        const Point& c = v[(i + 1) % n];
        const Point e1{b.x - a.x, b.y - a.y}, e2{c.x - b.x, c.y - b.y};
        const double l1 = std::hypot(e1.x, e1.y), l2 = std::hypot(e2.x, e2.y);
        const double r = k * 0.5 * std::min(l1, l2);
        first[i] = l1 > 1e-12 ? Point{b.x - e1.x / l1 * r, b.y - e1.y / l1 * r} : b;
        second[i] = l2 > 1e-12 ? Point{b.x + e2.x / l2 * r, b.y + e2.y / l2 * r} : b;
    }
    constexpr double fillet = 0.5522847498307936; // cubic circle constant
    p.start = first[0];
    for (int i = 0; i < n; ++i) {
        Segment seg;
        seg.curved = true;
        seg.c1 = {first[i].x + (v[i].x - first[i].x) * fillet,
                  first[i].y + (v[i].y - first[i].y) * fillet};
        seg.c2 = {second[i].x + (v[i].x - second[i].x) * fillet,
                  second[i].y + (v[i].y - second[i].y) * fillet};
        seg.end = second[i];
        p.segments.push_back(seg);
        if (i + 1 < n) p.segments.push_back({Point{}, Point{}, first[i + 1], false});
    }
    return p;
}
// The upstream polygon-with-curves path; the origin is the ring center, so the
// curve control lengths are measured from it exactly as in the original code.
Path curvedPath(const std::vector<Point>& v, const double curve[2], const double shear[2]) {
    const int n = static_cast<int>(v.size());
    Path p;
    p.start = v[0];
    std::vector<Point> before(n), after(n);
    for (int j = 0; j < n; ++j) {
        const int odd = j & 1;
        const Point& t = v[j];
        before[j] = {t.x - t.y * curve[odd], t.y + t.x * curve[odd]};
        after[j] = {t.x + t.y * curve[odd] * shear[odd], t.y - t.x * curve[odd] * shear[odd]};
    }
    for (int j = 0; j < n; ++j) {
        const int next = (j + 1) % n;
        p.segments.push_back({after[j], before[next], v[next], true});
    }
    return p;
}
// Builds a primitive's outline in the ring's local coordinates. `rEven`/`rOdd`
// are the two radii used by the polygon primitive; the other primitives derive
// their own radii from `rEven` so that they scale with the ring.
Path buildPrimitivePath(const RingShape& s, int sides, double rotEven, double rotOdd,
                        double rEven, double rOdd, double sx, double sy,
                        const double curve[2], const double shear[2]) {
    if (s.primitive == PRIM_Circle) {
        const double rx = sx * rEven, ry = sy * rEven;
        auto point = [=](double phi) { return Point{rx * std::sin(phi), ry * std::cos(phi)}; };
        auto deriv = [=](double phi) { return Point{rx * std::cos(phi), -ry * std::sin(phi)}; };
        Path p;
        const double start = rotEven + DTR(s.arcStart);
        const double sweep = DTR(std::clamp(s.arcSweep, 0.0, 360.0));
        if (sweep >= TAU - 1e-6) { // full circle: four 90-degree bezier arcs
            p.start = point(0);
            for (int i = 0; i < 4; ++i) {
                const double f0 = TAU * i / 4, f1 = TAU * (i + 1) / 4;
                const double beta = 4.0 / 3 * std::tan((f1 - f0) / 4);
                const Point a = point(f0), b = point(f1), da = deriv(f0), db = deriv(f1);
                p.segments.push_back({{a.x + beta * da.x, a.y + beta * da.y},
                                      {b.x - beta * db.x, b.y - beta * db.y}, b, true});
            }
        } else { // pie slice: center, then the arc, closing back to the center
            p.start = {0, 0};
            p.segments.push_back({Point{}, Point{}, point(start), false});
            const int steps = std::max(1, static_cast<int>(std::ceil(sweep / (PI / 2))));
            const double dphi = sweep / steps;
            const double beta = 4.0 / 3 * std::tan(dphi / 4);
            Point cur = point(start);
            double phi = start;
            for (int i = 0; i < steps; ++i) {
                const double next = phi + dphi;
                const Point end = point(next), da = deriv(phi), db = deriv(next);
                p.segments.push_back({{cur.x + beta * da.x, cur.y + beta * da.y},
                                      {end.x - beta * db.x, end.y - beta * db.y}, end, true});
                cur = end;
                phi = next;
            }
        }
        return p;
    }
    std::vector<Point> v;
    double corner = 0;
    bool useCurves = false;
    switch (s.primitive) {
    case PRIM_Square:
        v = polygonVerts(4, rotEven + PI / 4, rotEven + PI / 4, rEven, rEven * std::cos(PI / 4), sx, sy);
        corner = s.corner;
        break;
    case PRIM_Triangle:
        v = polygonVerts(3, rotEven, rotEven, rEven, rEven * std::cos(PI / 3), sx, sy);
        corner = s.corner;
        break;
    case PRIM_Star: {
        const int points = std::clamp(static_cast<int>(std::lround(s.points)), 3, 24);
        v = polygonVerts(points, rotEven, rotEven, rEven,
                         rEven * std::clamp(s.inner, 0.0, 1.0), sx, sy);
        corner = s.sharp;
        break;
    }
    case PRIM_Cross: {
        const double length = std::clamp(s.crossLength, 0.0, 1.0);
        const double width = std::min(std::clamp(s.crossWidth, 0.0, 1.0), length);
        const double xR = sx * rEven, yR = sy * rEven;
        const double shape[12][2] = {
            {width, length}, {-width, length}, {-width, width}, {-length, width},
            {-length, -width}, {-width, -width}, {-width, -length}, {width, -length},
            {width, -width}, {length, -width}, {length, width}, {width, width}};
        const double c = std::cos(rotEven), sn = std::sin(rotEven);
        for (const auto& q : shape)
            v.push_back({(q[0] * c - q[1] * sn) * xR, (q[0] * sn + q[1] * c) * yR});
        break;
    }
    case PRIM_Line: {
        const double length = std::clamp(s.barLength, 0.0, 2.0);
        const double half = std::clamp(s.barThickness, 0.0, 1.0) * 0.5;
        const double xR = sx * rEven, yR = sy * rEven;
        const double shape[4][2] = {{-length, -half}, {length, -half}, {length, half}, {-length, half}};
        const double c = std::cos(rotEven), sn = std::sin(rotEven);
        for (const auto& q : shape)
            v.push_back({(q[0] * c - q[1] * sn) * xR, (q[0] * sn + q[1] * c) * yR});
        break;
    }
    default: // PRIM_Polygon: the two radii and rotations come from the ring
        v = polygonVerts(sides, rotEven, rotOdd, rEven, rOdd, sx, sy);
        if (s.corner > 0) corner = s.corner;
        else useCurves = true;
        break;
    }
    if (useCurves && (curve[0] != 0 || curve[1] != 0)) return curvedPath(v, curve, shear);
    return roundedPath(v, corner);
}
// Snapshot/movie ring data is the Windows polygon form, so a primitive is
// written as the closest regular polygon.
void approximateShape(const RingShape& s, int& sides, double& starRatio) {
    switch (s.primitive) {
    case PRIM_Circle: sides = MAX_SIDES; starRatio = std::cos(PI / MAX_SIDES); break;
    case PRIM_Square: sides = 4; starRatio = std::cos(PI / 4); break;
    case PRIM_Triangle: sides = 3; starRatio = std::cos(PI / 3); break;
    case PRIM_Star:
        sides = std::clamp(static_cast<int>(std::lround(s.points)), 3, MAX_SIDES);
        starRatio = std::max(1e-3, std::clamp(s.inner, 0.0, 1.0));
        break;
    case PRIM_Cross:
    case PRIM_Line:
    default: sides = 4; starRatio = std::cos(PI / 4); break;
    }
}

// Draws rings in geometry coordinates of geomWidth x geomHeight, scaled by
// scale and moved by offset; snapshots letterbox into a differently sized
// window this way. Rings are a deque while live and a vector in a snapshot.
template <class Rings>
Frame renderRings(int geomWidth, int geomHeight, Rings&& rings, const GlobalRing& g,
                  Color background, double zoom, double canvasScale, bool convex,
                  double scale, Point offset) {
    Frame result;
    result.background = background;
    const double halfWidth = std::abs(geomWidth * canvasScale) / 2;
    const double halfHeight = std::abs(geomHeight * canvasScale) / 2;
    Path previous;
    bool hasPrevious = false, previousSkip = false;
    Color previousColor;
    if (convex && rings.size() > 0) {
        previousColor = rings[rings.size() - 1].color;
        previousSkip = rings[rings.size() - 1].skipFill;
    }
    for (std::size_t index = 0; index < rings.size(); ++index) {
        auto& r = rings[convex ? rings.size() - 1 - index : index];
        int sides = std::clamp(r.sides + g.polySides, 1, MAX_SIDES);
        const double rot[2] = {r.rot + g.rot, r.rot + g.rot + r.pinwheel + PI / sides * g.pinwheel};
        const double radius[2] = {r.radius * zoom, r.radius * zoom * r.starRatio * g.star};
        const Point center{r.origin.x + geomWidth / 2.0, r.origin.y + geomHeight / 2.0};
        const Point raw{(r.shift.x * zoom + g.shift.x * radius[0]) + center.x,
                        (r.shift.y * zoom + g.shift.y * radius[0]) + center.y};
        const Point shift{offset.x + raw.x * scale, offset.y + raw.y * scale};
        const double curve[2] = {r.evenCurve + g.evenCurve, r.oddCurve + g.oddCurve};
        const double shear[2] = {r.evenShear + g.evenShear, r.oddShear + g.oddShear};
        const bool curved = curve[0] != 0 || curve[1] != 0;
        Path path;
        path.center = shift; // filled as a fan from the ring's center
        const int primitive = std::clamp(r.shape.primitive, 0, static_cast<int>(PRIM_COUNT) - 1);
        bool visible = false;
        if (primitive == PRIM_Polygon && r.shape.corner <= 0) {
            // Unchanged upstream geometry: straight or curved alternating vertices.
            const int count = sides * 2;
            std::vector<Point> vertices(count), before(count), after(count);
            for (int j = 0; j < count; ++j) {
                const int odd = j & 1;
                const double theta = PI / sides * j + rot[odd];
                Point pt{finite(std::sin(theta) * r.scale.x * g.scale.x * radius[odd] + raw.x),
                         finite(std::cos(theta) * r.scale.y * g.scale.y * radius[odd] + raw.y)};
                if (pt.x >= center.x - halfWidth && pt.x < center.x + halfWidth &&
                    pt.y >= center.y - halfHeight && pt.y < center.y + halfHeight) visible = true;
                pt = {offset.x + pt.x * scale, offset.y + pt.y * scale};
                vertices[j] = pt;
                const double dx = pt.x - shift.x, dy = pt.y - shift.y;
                before[j] = {finite(pt.x - dy * curve[odd]), finite(pt.y + dx * curve[odd])};
                after[j] = {finite(pt.x + dy * curve[odd] * shear[odd]),
                            finite(pt.y - dx * curve[odd] * shear[odd])};
            }
            path.start = vertices[0];
            path.segments.reserve(count);
            for (int j = 0; j < count; ++j) {
                const int next = (j + 1) % count;
                path.segments.push_back({curved ? after[j] : Point{},
                                          curved ? before[next] : Point{}, vertices[next], curved});
            }
        } else {
            const Path local = buildPrimitivePath(r.shape, sides, rot[0], rot[1],
                                                  radius[0], radius[1],
                                                  r.scale.x * g.scale.x, r.scale.y * g.scale.y,
                                                  curve, shear);
            auto place = [&](Point q) {
                const Point t{finite(q.x + raw.x), finite(q.y + raw.y)};
                if (t.x >= center.x - halfWidth && t.x < center.x + halfWidth &&
                    t.y >= center.y - halfHeight && t.y < center.y + halfHeight) visible = true;
                return Point{offset.x + t.x * scale, offset.y + t.y * scale};
            };
            path.start = place(local.start);
            path.segments.reserve(local.segments.size());
            for (const Segment& s : local.segments) {
                Segment out;
                out.curved = s.curved;
                out.end = place(s.end);
                if (s.curved) { out.c1 = place(s.c1); out.c2 = place(s.c2); }
                path.segments.push_back(out);
            }
        }
        if constexpr (!std::is_const_v<std::remove_reference_t<Rings>>) {
            r.deleted = !visible; // offscreen rings are pruned by the next tick
        }
        auto filled = [&](const Path& outer, const Path* inner, Color fill) {
            Shape shape;
            shape.outer = outer;
            if (inner) { shape.inner = *inner; shape.hasInner = true; }
            shape.fill = true;
            shape.outline = true; // D2D outlines even unoutlined fills to close antialiasing gaps
            shape.color = fill;
            shape.outlineColor = (r.mode & DM_OUTLINE) ? background : fill;
            shape.lineWidth = std::max(0.0, finite((r.mode & DM_OUTLINE) ?
                              r.lineWidth + g.lineWidth : 1));
            result.shapes.push_back(std::move(shape));
        };
        if (r.mode & DM_FILL) {
            if (!r.skipFill && !previousSkip) {
                if (!convex || hasPrevious)
                    filled(path, hasPrevious ? &previous : nullptr, convex ? previousColor : r.color);
                if (convex && index + 1 == rings.size()) filled(path, nullptr, r.color);
            }
        } else {
            Shape shape;
            shape.outer = path;
            shape.outline = true;
            shape.color = shape.outlineColor = r.color;
            shape.lineWidth = std::max(0.0, finite(r.lineWidth + g.lineWidth));
            result.shapes.push_back(std::move(shape));
        }
        previous = std::move(path);
        previousColor = r.color;
        previousSkip = r.skipFill;
        hasPrevious = true;
    }
    return result;
}
} // namespace

Patch::Patch() {
    for (int i = 0; i < PARAM_COUNT; ++i) params[i].value = paramInfo[i].initial;
    const double defaults[] = {
#define MASTERDEF(name, type, prefix, initval) initval,
#include "../trunk/Whorld/WhorldDef.h"
    };
    std::copy(std::begin(defaults), std::end(defaults), master.begin());
}

bool Patch::load(const std::string& path, std::string& error) {
    std::ifstream file(path);
    if (!file) { error = "Cannot open patch: " + path; return false; }
    std::string line;
    if (!std::getline(file, line)) { error = "Missing WHORLD version header"; return false; }
    if (line.compare(0, 3, "\xef\xbb\xbf") == 0) line.erase(0, 3);
    std::istringstream header(line);
    std::string id;
    header >> id;
    if (id.size() <= 6 || id.compare(0, 6, "WHORLD") ||
        id.find_first_not_of("0123456789", 6) != std::string::npos) {
        error = "Invalid WHORLD version header";
        return false;
    }
    Patch next;
    std::size_t lineNo = 1;
    while (std::getline(file, line)) {
        ++lineNo;
        if (line.size() > 65536) { error = "Patch line too long: " + std::to_string(lineNo); return false; }
        std::istringstream in(line);
        std::string tag;
        if (!(in >> tag) || tag[0] == '#' || tag[0] == ';') continue;
        bool valid = true, known = false;
        for (int i = 0; i < PARAM_COUNT; ++i) {
            if (tag != paramInfo[i].name) continue;
            known = true;
            ParamRow& row = next.params[i];
            valid = number(in, row.value);
            // Earlier versions can omit trailing columns. Preserve their defaults.
            if (valid && in >> std::ws && !in.eof()) valid = integer(in, row.wave);
            if (valid && in >> std::ws && !in.eof()) valid = number(in, row.amp);
            if (valid && in >> std::ws && !in.eof()) valid = number(in, row.freq);
            if (valid && in >> std::ws && !in.eof()) valid = number(in, row.pw);
            if (valid && in >> std::ws && !in.eof()) valid = number(in, row.global);
            break;
        }
        if (!known) for (int i = 0; i < MASTER_COUNT; ++i) {
            if (tag == masterNames[i]) {
                known = true;
                valid = number(in, next.master[i]);
                break;
            }
        }
        if (!known) for (std::size_t i = 0; i < mainNames.size(); ++i) {
            if (tag != mainNames[i]) continue;
            known = true;
            switch (i) {
            case 0: valid = number(in, next.origin.x) && number(in, next.origin.y); break;
            case 1: valid = integer(in, next.drawMode); break;
            case 2: valid = integer(in, next.orgMotion); break;
            case 3: valid = number(in, next.hue); break;
            case 4: valid = boolean(in, next.mirror); break;
            case 5: valid = boolean(in, next.reverse); break;
            case 6: valid = boolean(in, next.convex); break;
            case 7: valid = boolean(in, next.invertColor); break;
            case 8: valid = boolean(in, next.loopHue); break;
            case 9: valid = boolean(in, next.zoomCenter); break;
            case 10: valid = boolean(in, next.antialiasing); break;
            }
            break;
        }
        if (known && !valid) {
            error = "Invalid value for " + tag + " at line " + std::to_string(lineNo);
            return false;
        }
    }
    if (file.bad()) { error = "Error reading patch: " + path; return false; }
    *this = next;
    error.clear();
    return true;
}

bool Patch::save(const std::string& path, std::string& error) const {
    std::ofstream file(path, std::ios::trunc);
    if (!file) { error = "Cannot write patch: " + path; return false; }
    file << std::setprecision(17) << "WHORLD4\n";
    for (int i = 0; i < PARAM_COUNT; ++i) {
        const auto& r = params[i];
        file << paramInfo[i].name << '\t' << r.value << '\t' << r.wave << '\t'
             << r.amp << '\t' << r.freq << '\t' << r.pw << '\t' << r.global << '\n';
    }
    for (int i = 0; i < MASTER_COUNT; ++i) file << masterNames[i] << '\t' << master[i] << '\n';
    file << "Origin\t" << origin.x << ' ' << origin.y << '\n'
         << "DrawMode\t" << drawMode << "\nOrgMotion\t" << orgMotion
         << "\nHue\t" << hue << "\nMirror\t" << mirror << "\nReverse\t" << reverse
         << "\nConvex\t" << convex << "\nInvertColor\t" << invertColor
         << "\nLoopHue\t" << loopHue << "\nZoomCenter\t" << zoomCenter
         << "\nAntialiasing\t" << antialiasing << '\n';
    file.close();
    if (!file) { error = "Error writing patch: " + path; return false; }
    error.clear();
    return true;
}

struct Engine::Impl {
    using Ring = SnapshotRing;
    std::deque<Ring> rings; // head: newest for outward growth, oldest for inward growth
    std::array<Oscillator, PARAM_COUNT> osc{};
    std::array<ParamRow, PARAM_COUNT> previous{};
    std::array<double, PARAM_COUNT> globals{};
    std::array<double, PARAM_COUNT> values{};
    Point origin, target;
    double zoom = 1, zoomTarget = 1, hue = 0, hueBase = 0, huePos = 0;
    double offset = 0, growth = 0, copyTheta = 0, randomClock = 0;
    Color background{0, 0, 0, 1};
    uint32_t rng = 0x6d2b79f5u;
    bool flush = true, randomHigh = false;
    int previousWidth = 0, previousHeight = 0;
    std::unique_ptr<Snapshot> displayed;

    void updateHue(const Patch& p, double delta, double colorSpeed) {
        if (p.loopHue) {
            const double span = std::abs(finite(p.master[MASTER_HueSpan]));
            huePos += colorSpeed * delta;
            hue = span > 1e-9 ? hueBase - reflect(huePos, span) : hueBase;
        } else hue += colorSpeed * delta;
        hue = wrap(finite(hue), 360);
    }
    void addRing(const Patch& p, int width, int height, const std::array<double, PARAM_COUNT>& v,
                 double remainingGrowth) {
        Ring r;
        double spacing = std::max(1.0, finite(v[PARAM_RingSpacing], 5));
        const bool reverse = growth < 0;
        double signedOffset = reverse ? -offset : offset;
        r.rotDelta = finite(v[PARAM_RotateSpeed]);
        r.rot = r.rotDelta * (signedOffset - remainingGrowth);
        const double canvas = std::clamp(finite(p.master[MASTER_CanvasScale] * zoom, 1.4), -100.0, 100.0);
        r.radius = reverse ? std::max(width, height) * canvas / 2 / std::max(zoom, 1e-6)
                                - offset - remainingGrowth : offset - remainingGrowth;
        const double aspect = std::max(pow2(v[PARAM_AspectRatio]), 1e-9);
        r.scale = {aspect > 1 ? aspect : 1, aspect < 1 ? 1 / aspect : 1};
        r.shiftDelta = {std::sin(v[PARAM_SkewAngle]) * v[PARAM_SkewRadius],
                        std::cos(v[PARAM_SkewAngle]) * v[PARAM_SkewRadius]};
        r.shift = {r.shiftDelta.x * (signedOffset - remainingGrowth),
                   r.shiftDelta.y * (signedOffset - remainingGrowth)};
        const int copies = std::clamp(int(std::round(finite(p.master[MASTER_Copies]))), 1, 256);
        if (copies > 1 && p.master[MASTER_Spread] != 0) {
            r.shift.x += std::sin(copyTheta) * finite(p.master[MASTER_Spread]);
            r.shift.y += std::cos(copyTheta) * finite(p.master[MASTER_Spread]);
            copyTheta = wrap(copyTheta + TAU / copies, TAU);
        }
        r.hue = hue;
        r.lightness = std::clamp(finite(v[PARAM_Lightness]), 0.0, 1.0);
        r.saturation = std::clamp(finite(v[PARAM_Saturation]), 0.0, 1.0);
        r.color = hls(r.hue, r.lightness, r.saturation, v[PARAM_Alpha]);
        r.sides = std::clamp(int(std::round(v[PARAM_PolySides])), 3, MAX_SIDES);
        r.mode = p.drawMode;
        r.starRatio = std::cos(PI / r.sides) * std::max(pow2(v[PARAM_StarFactor]), 0.01);
        r.pinwheel = PI / r.sides * v[PARAM_Pinwheel];
        r.lineWidth = v[PARAM_LineWidth];
        r.origin = origin;
        r.evenCurve = v[PARAM_EvenCurve];
        r.oddCurve = v[PARAM_OddCurve] / r.starRatio;
        r.evenShear = v[PARAM_EvenShear];
        r.oddShear = v[PARAM_OddShear];
        // Port-only shape primitive, captured at birth like the geometry above.
        const int primitive = std::clamp(int(std::lround(v[PARAM_Primitive])), 0,
                                         static_cast<int>(PRIM_COUNT) - 1);
        r.shape.primitive = primitive;
        r.shape.corner = std::clamp(v[primitive == PRIM_Square ? PARAM_SquareCorner
                                        : primitive == PRIM_Triangle ? PARAM_TriCorner
                                        : PARAM_PolyCorner], 0.0, 1.0);
        r.shape.arcStart = v[PARAM_CircleStart];
        r.shape.arcSweep = std::clamp(v[PARAM_CircleSweep], 0.0, 360.0);
        r.shape.points = std::clamp(v[PARAM_StarPoints], 3.0, 24.0);
        r.shape.inner = std::clamp(v[PARAM_StarInner], 0.0, 1.0);
        r.shape.sharp = std::clamp(v[PARAM_StarSharp], 0.0, 1.0);
        r.shape.crossWidth = std::clamp(v[PARAM_CrossWidth], 0.0, 1.0);
        r.shape.crossLength = std::clamp(v[PARAM_CrossLength], 0.0, 1.0);
        r.shape.barLength = std::clamp(v[PARAM_BarLength], 0.0, 2.0);
        r.shape.barThickness = std::clamp(v[PARAM_BarThickness], 0.0, 1.0);
        if (!rings.empty()) {
            const Ring& neighbor = reverse ? rings.back() : rings.front();
            r.skipFill = std::abs(r.radius + (reverse ? -spacing : spacing) - neighbor.radius) > spacing * 2;
        } else r.skipFill = reverse;
        if (reverse) rings.push_back(r);
        else rings.push_front(r);
    }
};

Engine::Engine() : impl(new Impl) { setPatch(patch); }
Engine::~Engine() { delete impl; }
void Engine::setPatch(const Patch& next) {
    patch = next;
    impl->zoomTarget = std::max(1e-6, finite(patch.master[MASTER_Zoom], 1));
    impl->zoom = impl->zoomTarget; // upstream applies patch zoom without damping
    impl->hue = wrap(finite(patch.hue), 360);
    if (patch.loopHue) { impl->hueBase = impl->hue; impl->huePos = 0; }
    if (patch.orgMotion == OM_RANDOM) { impl->randomClock = 0; impl->randomHigh = false; }
    impl->copyTheta = TAU / std::clamp(int(std::round(finite(patch.master[MASTER_Copies]))), 1, 256) / 2;
    impl->flush = true;
    for (int i = 0; i < PARAM_COUNT; ++i) {
        impl->globals[i] = finite(patch.params[i].global);
        impl->osc[i].wave = patch.params[i].wave;
        impl->osc[i].pw = finite(patch.params[i].pw, 0.5);
        impl->osc[i].frequencyTo(patch.params[i].freq);
    }
}
void Engine::setDrawMode(int mode) {
    patch.drawMode = mode;
    for (auto& ring : impl->rings) ring.mode = mode;
}
void Engine::clear() { impl->rings.clear(); impl->offset = 0; }
void Engine::randomizePhase() {
    // A phase offset (rather than rewriting the clock) keeps the tick math and
    // frequency-resync continuity intact while giving every LFO a new start.
    for (auto& o : impl->osc) o.phaseOffset = random01(impl->rng);
}
std::size_t Engine::ringCount() const { return impl->rings.size(); }

void Engine::tick(int width, int height, double mouseX, double mouseY) {
    if (impl->displayed || width <= 0 || height <= 0) return;
    auto& s = *impl;
    const auto& p = patch;
    const double damping = std::clamp(finite(p.master[MASTER_Damping]), 0.0, 1.0);
    const double trail = std::clamp(finite(p.master[MASTER_Trail]), 0.0, 1.0);
    if (s.previousWidth != width || s.previousHeight != height) {
        if (s.previousWidth && s.previousHeight) {
            s.origin.x *= double(width) / s.previousWidth;
            s.origin.y *= double(height) / s.previousHeight;
            for (auto& r : s.rings) {
                r.origin.x *= double(width) / s.previousWidth;
                r.origin.y *= double(height) / s.previousHeight;
            }
        } else s.origin = {(finite(p.origin.x, 0.5) - 0.5) * width,
                           (finite(p.origin.y, 0.5) - 0.5) * height};
        s.previousWidth = width;
        s.previousHeight = height;
    }
    Point target{(finite(p.origin.x, 0.5) - 0.5) * width,
                 (finite(p.origin.y, 0.5) - 0.5) * height};
    if (p.orgMotion == OM_DRAG) {
        target = {(std::clamp(finite(mouseX, 0.5), 0.0, 1.0) - 0.5) * width,
                  (std::clamp(finite(mouseY, 0.5), 0.0, 1.0) - 0.5) * height};
    } else if (p.orgMotion == OM_RANDOM && finite(p.master[MASTER_Tempo]) != 0) {
        s.randomClock = wrap(s.randomClock + finite(p.master[MASTER_Tempo]) / 3600, 1);
        const bool high = s.randomClock < 0.5;
        if (high && !s.randomHigh)
            s.target = {(random01(s.rng) - 0.5) * width, (random01(s.rng) - 0.5) * height};
        s.randomHigh = high;
        target = s.target;
    }
    s.target = target;
    s.origin.x += (target.x - s.origin.x) * damping;
    s.origin.y += (target.y - s.origin.y) * damping;
    double oldZoom = s.zoom;
    s.zoomTarget = std::max(1e-6, finite(p.master[MASTER_Zoom], 1));
    s.zoom = std::max(1e-6, s.zoom + (s.zoomTarget - s.zoom) * damping);
    const double dz = s.zoom / oldZoom;
    if (p.zoomCenter) { s.origin.x *= dz; s.origin.y *= dz; }
    for (auto& r : s.rings) {
        r.origin.x = (r.origin.x - (p.zoomCenter ? 0 : s.origin.x)) * dz + (p.zoomCenter ? 0 : s.origin.x);
        r.origin.y = (r.origin.y - (p.zoomCenter ? 0 : s.origin.y)) * dz + (p.zoomCenter ? 0 : s.origin.y);
    }
    for (int i = 0; i < PARAM_COUNT; ++i)
        s.globals[i] += (finite(p.params[i].global) - s.globals[i]) * damping;
    const double speed = finite(p.master[MASTER_Speed], 1) * (p.reverse ? -1 : 1);
    std::array<double, PARAM_COUNT> oldClocks{};
    for (int i = 0; i < PARAM_COUNT; ++i) {
        auto& o = s.osc[i];
        oldClocks[i] = o.clock;
        o.wave = p.params[i].wave;
        o.pw = finite(p.params[i].pw, 0.5);
        if (s.flush) o.frequency = finite(p.params[i].freq);
        else if (o.frequency != finite(p.params[i].freq)) o.frequencyTo(p.params[i].freq);
        if (s.flush) s.previous[i] = p.params[i];
    }
    s.flush = false;
    const auto& rg = p.params[PARAM_RingGrowth];
    s.growth = finite((finite(rg.value) + s.osc[PARAM_RingGrowth].value(oldClocks[PARAM_RingGrowth] + 1, s.rng)
                        * finite(rg.amp)) * speed);
    const double absGrowth = std::abs(s.growth);
    double prevFraction = 0;
    const int limit = std::clamp(int(std::round(finite(p.master[MASTER_Rings]))), 0, MAX_RINGS);
    // The upstream loop adds at least one ring for any positive offset. A hard
    // bound is essential when spacing/growth originate from untrusted files.
    for (int created = 0; s.offset > 0 && created < MAX_RINGS; ++created) {
        const double fraction = std::clamp(absGrowth ? 1 - s.offset / absGrowth : 0, 0.0, 1.0);
        std::array<double, PARAM_COUNT> v{};
        for (int i = 0; i < PARAM_COUNT; ++i) {
            const auto& before = s.previous[i];
            const auto& now = p.params[i];
            const double amp = finite(before.amp) + (finite(now.amp) - finite(before.amp)) * fraction;
            v[i] = finite(finite(before.value) + (finite(now.value) - finite(before.value)) * fraction
                + s.osc[i].value(oldClocks[i] + fraction, s.rng) * amp);
        }
        v[PARAM_RingGrowth] = finite(v[PARAM_RingGrowth] * speed);
        s.updateHue(p, fraction - prevFraction, finite(v[PARAM_ColorSpeed] * speed));
        if (limit > 0) s.addRing(p, width, height, v, s.growth);
        s.offset -= std::max(1.0, v[PARAM_RingSpacing]);
        prevFraction = fraction;
        while (s.rings.size() > static_cast<std::size_t>(limit)) {
            if (s.growth < 0) s.rings.pop_front(); else s.rings.pop_back();
        }
    }
    if (s.offset > 0) s.offset = 0; // couldn't finish within cap
    s.offset = std::clamp(s.offset + absGrowth, -MAX_VALUE, MAX_VALUE);
    for (int i = 0; i < PARAM_COUNT; ++i) {
        auto& o = s.osc[i];
        o.clock = oldClocks[i] + 1;
        if (std::abs(o.clock) > 1e8) o.clock = wrap(o.clock * o.frequency / 60, 1) * 60 / (o.frequency ? o.frequency : 1);
        const auto& row = p.params[i];
        s.values[i] = finite(finite(row.value) + o.value(o.clock, s.rng) * finite(row.amp));
        s.previous[i] = row;
    }
    s.updateHue(p, 1 - prevFraction, finite(s.values[PARAM_ColorSpeed] * speed));
    s.background = hls(s.values[PARAM_BkHue], s.values[PARAM_BkLightness],
                       s.values[PARAM_BkSaturation], s.values[PARAM_BkAlpha]);
    Point prev = s.origin;
    for (auto& r : s.rings) {
        r.origin.x += (prev.x - r.origin.x) * (1 - trail);
        r.origin.y += (prev.y - r.origin.y) * (1 - trail);
        prev = r.origin;
        r.radius = finite(r.radius + s.growth);
        r.rot = finite(r.rot + r.rotDelta * s.growth);
        r.shift.x = finite(r.shift.x + r.shiftDelta.x * s.growth);
        r.shift.y = finite(r.shift.y + r.shiftDelta.y * s.growth);
    }
    if (s.growth < 0)
        s.rings.erase(std::remove_if(s.rings.begin(), s.rings.end(),
                       [](const Impl::Ring& r) { return r.radius <= 0; }), s.rings.end());
    else
        s.rings.erase(std::remove_if(s.rings.begin(), s.rings.end(),
                       [](const Impl::Ring& r) { return r.deleted; }), s.rings.end());
    while (s.rings.size() > static_cast<std::size_t>(limit)) s.rings.pop_back();
}

Frame Engine::frame(int width, int height) {
    if (impl->displayed) return render(width, height, *impl, impl->displayed.get());
    return render(width, height, *impl, nullptr);
}

Frame Engine::render(int width, int height, Impl& s, const Snapshot* snapshot) {
    if (width <= 0 || height <= 0) {
        Frame empty;
        empty.background = snapshot ? snapshot->background : s.background;
        return empty;
    }
    const double zoom = snapshot ? finite(snapshot->zoom, 1) : s.zoom;
    const GlobalRing globals = snapshot ? globalsFromSnapshot(snapshot->global)
                                        : globalsFromParams(s.globals);
    const Color background = snapshot ? snapshot->background : s.background;
    // Snapshots keep their capture size and are letterboxed into the window.
    const int geomWidth = snapshot ? std::max(1, int(snapshot->width)) : width;
    const int geomHeight = snapshot ? std::max(1, int(snapshot->height)) : height;
    double scale = 1;
    Point offset{};
    if (snapshot) {
        scale = std::min(width / double(geomWidth), height / double(geomHeight));
        offset = {(width - geomWidth * scale) / 2, (height - geomHeight * scale) / 2};
    }
    const double canvasScale = snapshot ? 1 : finite(patch.master[MASTER_CanvasScale] * zoom, 1);
    const bool convex = snapshot ? snapshot->convex : patch.convex;
    if (snapshot)
        return renderRings(geomWidth, geomHeight, snapshot->rings, globals, background,
                           zoom, canvasScale, convex, scale, offset);
    return renderRings(geomWidth, geomHeight, s.rings, globals, background,
                       zoom, canvasScale, convex, scale, offset);
}

Snapshot Engine::captureSnapshot(int width, int height) const {
    Snapshot snapshot;
    snapshot.width = width;
    snapshot.height = height;
    snapshot.zoom = impl->zoom;
    snapshot.background = impl->background;
    snapshot.convex = patch.convex;
    snapshot.global = snapshotFromGlobals(globalsFromParams(impl->globals));
    snapshot.rings.clear();
    snapshot.rings.reserve(impl->rings.size());
    for (const auto& ring : impl->rings) {
        SnapshotRing out = ring;
        if (ring.shape.primitive != PRIM_Polygon) {
            // Windows has no shape field, so store the closest regular polygon,
            // and make the in-memory copy match what the saved file will hold.
            // Curve and pinwheel inputs, which the primitive ignores, are dropped
            // so the polygon is not distorted.
            approximateShape(ring.shape, out.sides, out.starRatio);
            out.shape = RingShape{};
            out.pinwheel = 0;
            out.evenCurve = 0;
            out.oddCurve = 0;
            out.evenShear = 0;
            out.oddShear = 0;
            snapshot.rings.push_back(out);
        } else snapshot.rings.push_back(std::move(out));
    }
    return snapshot;
}

bool Engine::displaySnapshot(const Snapshot& snapshot, int width, int height) {
    std::string error;
    if (!snapshot.valid(error)) return false;
    impl->displayed = std::make_unique<Snapshot>(snapshot);
    (void)width;
    (void)height;
    return true;
}

void Engine::stopSnapshot() { impl->displayed.reset(); }

bool Engine::snapshotActive() const { return static_cast<bool>(impl->displayed); }
} // namespace whorld
