#include "whorld_core.h"
#include "midi_actions.h"
#include "snapshot.h"
#include <stdexcept>
#define require(expr) do { if (!(expr)) throw std::runtime_error("Check failed: " #expr); } while (false)
#include <cmath>
#include <fstream>
#include <iostream>
#include <string>

int main() {
    using namespace whorld;
    Patch patch;
    require(PARAM_COUNT == 35);
    require(patch.params[PARAM_RingSpacing].value == 5);
    require(patch.params[PARAM_Lightness].value == .5);
    patch.params[PARAM_EvenCurve].value = .75;
    patch.params[PARAM_RingGrowth].wave = 1;
    patch.params[PARAM_RingGrowth].amp = .3;
    patch.master[MASTER_Zoom] = 1.5;
    patch.origin = {.25, .8};
    patch.drawMode = DM_FILL | DM_OUTLINE;
    patch.loopHue = true;
    const std::string file = "/tmp/opencode/whorld-core-test.whp";
    std::string error;
    require(patch.save(file, error));
    Patch loaded;
    require(loaded.load(file, error));
    require(loaded.params[PARAM_EvenCurve].value == .75);
    require(loaded.params[PARAM_RingGrowth].wave == 1);
    require(loaded.params[PARAM_RingGrowth].amp == .3);
    require(loaded.master[MASTER_Zoom] == 1.5);
    require(loaded.drawMode == (DM_FILL | DM_OUTLINE));
    require(loaded.origin.x == .25 && loaded.origin.y == .8 && loaded.loopHue);
    {
        std::ofstream out(file);
        out << "NOT_A_WHORLD\nRingGrowth 900\n";
    }
    require(!loaded.load(file, error));
    require(loaded.params[PARAM_RingGrowth].value == patch.params[PARAM_RingGrowth].value);
    {
        std::ofstream out(file);
        out << "WHORLD4\nUnusedProperty 123\nRingGrowth 2.8\nPolySides 8\nDrawMode 1\n";
    }
    require(loaded.load(file, error));
    require(loaded.params[PARAM_RingGrowth].value == 2.8);
    require(loaded.params[PARAM_PolySides].value == 8);
    require(loaded.params[PARAM_Lightness].value == .5);
    Engine engine;
    engine.setPatch(loaded);
    for (int i = 0; i < 120; ++i) {
        engine.tick(480, 360);
        engine.frame(480, 360);
    }
    require(engine.ringCount() > 0);
    Frame frame = engine.frame(480, 360);
    require(!frame.shapes.empty());
    require(frame.shapes[0].outer.segments.size() >= 6);
    Engine curved;
    curved.setPatch(patch);
    for (int i = 0; i < 120; ++i) { curved.tick(480, 360); curved.frame(480, 360); }
    auto curvedFrame = curved.frame(480, 360);
    require(!curvedFrame.shapes.empty());
    require(curvedFrame.shapes[0].outer.segments[0].curved);
    curved.setDrawMode(0);
    require(!curved.frame(480, 360).shapes[0].fill);
    // An untrusted patch must not cause an unbounded ring-spawn loop.
    patch.params[PARAM_RingGrowth].value = 1e9;
    patch.params[PARAM_RingSpacing].value = -100;
    patch.master[MASTER_Rings] = 15;
    Engine bounded;
    bounded.setPatch(patch);
    for (int i = 0; i < 3; ++i) { bounded.tick(400, 300); bounded.frame(400, 300); }
    require(bounded.ringCount() <= 15);
    // Shape primitives: the selector changes the generated outline.
    {
        require(PRIM_COUNT == 7);
        require(shapeParamApplies(PARAM_CircleSweep, PRIM_Circle));
        require(!shapeParamApplies(PARAM_CircleSweep, PRIM_Square));
        Patch sp;
        sp.params[PARAM_Primitive].value = PRIM_Star;
        sp.params[PARAM_StarPoints].value = 7;
        sp.params[PARAM_StarInner].value = .3;
        sp.params[PARAM_StarSharp].value = .5;
        const std::string shapeFile = "/tmp/opencode/whorld-shape-test.whp";
        require(sp.save(shapeFile, error));
        Patch shapeLoaded;
        require(shapeLoaded.load(shapeFile, error));
        require(shapeLoaded.params[PARAM_Primitive].value == PRIM_Star);
        require(shapeLoaded.params[PARAM_StarPoints].value == 7);
        require(shapeLoaded.params[PARAM_StarInner].value == .3);
        require(shapeLoaded.params[PARAM_StarSharp].value == .5);
        auto build = [&](int prim) {
            Patch bp;
            bp.params[PARAM_Primitive].value = prim;
            bp.params[PARAM_RingGrowth].value = 30;
            bp.master[MASTER_Rings] = 3;
            Engine e;
            e.setPatch(bp);
            for (int i = 0; i < 30; ++i) { e.tick(400, 300); e.frame(400, 300); }
            return e.frame(400, 300);
        };
        auto segmentCount = [&](int prim) {
            Frame f = build(prim);
            return f.shapes.empty() ? std::size_t(0) : f.shapes[0].outer.segments.size();
        };
        auto firstShape = [&](int prim) {
            Frame f = build(prim);
            require(!f.shapes.empty());
            return f.shapes[0];
        };
        auto aspect = [](const Shape& sh) {
            Point lo{1e30, 1e30}, hi{-1e30, -1e30};
            auto cover = [&](Point p) {
                lo.x = std::min(lo.x, p.x); hi.x = std::max(hi.x, p.x);
                lo.y = std::min(lo.y, p.y); hi.y = std::max(hi.y, p.y);
            };
            cover(sh.outer.start);
            for (const auto& s : sh.outer.segments) cover(s.end);
            return (hi.x - lo.x) / std::max(1e-9, hi.y - lo.y);
        };
        require(segmentCount(PRIM_Circle) == 4);   // four bezier arcs
        require(segmentCount(PRIM_Square) == 7);   // four corners -> 8 vertices
        require(segmentCount(PRIM_Triangle) == 5);
        require(segmentCount(PRIM_Star) == 9);     // five points -> 10 vertices
        require(segmentCount(PRIM_Cross) == 11);
        require(segmentCount(PRIM_Line) == 3);
        // Proportions: a square, cross and circle are as wide as they are tall,
        // a line is a thin bar, and a triangle is wider than it is tall.
        require(std::abs(aspect(firstShape(PRIM_Square)) - 1) < .05);
        require(std::abs(aspect(firstShape(PRIM_Cross)) - 1) < .05);
        require(std::abs(aspect(firstShape(PRIM_Circle)) - 1) < .05);
        require(std::abs(aspect(firstShape(PRIM_Triangle)) - 1.1547) < .1);
        require(std::abs(aspect(firstShape(PRIM_Star)) - 1) < .5);
        require(aspect(firstShape(PRIM_Line)) > 3);
        {
            // Every circle endpoint lies on one circle about their centroid.
            Shape circle = firstShape(PRIM_Circle);
            Point c{};
            for (const auto& s : circle.outer.segments) { c.x += s.end.x; c.y += s.end.y; }
            c.x /= circle.outer.segments.size();
            c.y /= circle.outer.segments.size();
            double lo = 1e30, hi = 0;
            for (const auto& s : circle.outer.segments) {
                const double d = std::hypot(s.end.x - c.x, s.end.y - c.y);
                lo = std::min(lo, d);
                hi = std::max(hi, d);
            }
            require(hi > 0 && lo / hi > .98);
        }
        {
            Frame circle = build(PRIM_Circle);
            require(!circle.shapes.empty() && circle.shapes[0].outer.segments[0].curved);
            Frame square = build(PRIM_Square);
            require(!square.shapes.empty() && !square.shapes[0].outer.segments[0].curved);
        }
        // Rounded corners turn the straight square into a curved path.
        Patch rounded;
        rounded.params[PARAM_Primitive].value = PRIM_Square;
        rounded.params[PARAM_SquareCorner].value = .8;
        rounded.params[PARAM_RingGrowth].value = 30;
        rounded.master[MASTER_Rings] = 3;
        Engine rounder;
        rounder.setPatch(rounded);
        for (int i = 0; i < 30; ++i) { rounder.tick(400, 300); rounder.frame(400, 300); }
        Frame roundFrame = rounder.frame(400, 300);
        require(!roundFrame.shapes.empty() && roundFrame.shapes[0].outer.segments[0].curved);
        // Each primitive lists only its own adjustment controls.
        const int expectedControls[PRIM_COUNT] = {1, 2, 1, 1, 3, 2, 2};
        for (int prim = 0; prim < PRIM_COUNT; ++prim) {
            require(shapeParamApplies(PARAM_Primitive, prim));
            int applicable = 0;
            for (int p = SHAPE_PARAM_BEGIN + 1; p < PARAM_COUNT; ++p)
                if (shapeParamApplies(p, prim)) ++applicable;
            require(applicable == expectedControls[prim]);
        }
        // NextShape/PrevShape step the primitive and wrap; the release edge of a
        // mapped button (amount 0) must not step again.
        {
            Patch linePatch;
            linePatch.params[PARAM_Primitive].value = PRIM_Line;
            Engine shifter;
            shifter.setPatch(linePatch);
            Playlist noPlaylist;
            bool paused = false;
            int current = 0;
            applyMidiAction({"NextShape", "Val", 100}, shifter, noPlaylist, paused, current, error);
            require(shifter.patch.params[PARAM_Primitive].value == PRIM_Polygon);
            applyMidiAction({"PrevShape", "Val", 100}, shifter, noPlaylist, paused, current, error);
            require(shifter.patch.params[PARAM_Primitive].value == PRIM_Line);
            applyMidiAction({"NextShape", "Val", 0}, shifter, noPlaylist, paused, current, error);
            require(shifter.patch.params[PARAM_Primitive].value == PRIM_Line);
        }
        // A snapshot stores the Windows polygon approximation, not the primitive.
        Patch circular;
        circular.params[PARAM_Primitive].value = PRIM_Circle;
        circular.params[PARAM_RingGrowth].value = 30;
        circular.master[MASTER_Rings] = 4;
        Engine snap;
        snap.setPatch(circular);
        for (int i = 0; i < 10; ++i) { snap.tick(300, 200); snap.frame(300, 200); }
        require(snap.ringCount() > 0);
        Snapshot captured = snap.captureSnapshot(300, 200);
        require(!captured.rings.empty());
        std::string snapError;
        require(captured.valid(snapError));
        for (const auto& r : captured.rings) {
            require(r.sides == 50);
            require(r.starRatio > 0);
            require(r.shape.primitive == PRIM_Polygon);
        }
    }
    std::cout << "Whorld patch roundtrip and animated geometry OK\n";
}
