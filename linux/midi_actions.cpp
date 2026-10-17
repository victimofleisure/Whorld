// Mapping semantics ported from trunk/Whorld/MidiManager.cpp and MasterRowDlg.cpp.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "midi_actions.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstdlib>

namespace whorld {
namespace {
struct Slider { double min, max, base, scale; };
constexpr Slider sliders[MASTER_COUNT] = {
    {-100, 100, 20, 100}, {-100, 100, 10, 100}, {0, 100, 20, 101},
    {0, 100, 10, 100}, {0, 100, 30, 100}, {0, 250, 0, 1},
    {0, 360, 0, 1}, {100, 300, 0, 100}, {1, 20, 0, 1}, {0, 600, 0, 1}
};
double expNorm(double x, double base, double scale) {
    return (std::pow(base, x / scale) - 1) / (base - 1);
}
double masterDenorm(int index, double amount) {
    const auto& s = sliders[index];
    // Preserve upstream's out-of-range response within a safe numeric bound.
    double x = std::clamp(amount, -5.0, 5.0) * (s.max - s.min) + s.min;
    switch (index) {
    case MASTER_Damping: return expNorm(s.scale - x, s.base, s.scale);
    case MASTER_Trail: return expNorm(x, s.base, s.scale);
    case MASTER_Rings: return expNorm(x, s.base, s.scale) * MAX_RINGS;
    default: return s.base ? std::pow(s.base, x / s.scale) : x / s.scale;
    }
}
bool hasGlobal(int param) {
    switch (param) {
#define GLOBALPARAMDEF(name) case PARAM_##name: return true;
#include "../trunk/Whorld/WhorldDef.h"
    default: return false;
    }
}
}
bool applyMidiAction(const MappedAction& action, Engine& engine, Playlist& playlist,
                     bool& paused, int& currentPatch, std::string& status) {
    const double amount = std::isfinite(action.amount) ? std::clamp(action.amount, -100.0, 100.0) : 0;
    for (int i = 0; i < PARAM_COUNT; ++i) {
        if (action.target != paramInfo[i].name) continue;
        auto& row = engine.patch.params[i];
        const double value = paramInfo[i].min + (paramInfo[i].max - paramInfo[i].min) * amount;
        if (action.property == "Val") row.value = value;
        else if (action.property == "Wave") row.wave = std::clamp(static_cast<int>(std::lround(amount * 7)), 0, 7);
        else if (action.property == "Amp") row.amp = value;
        else if (action.property == "Freq") row.freq = amount;
        else if (action.property == "PW") row.pw = amount;
        else if (action.property == "Global" && hasGlobal(i))
            row.global = (amount - .5) * paramInfo[i].max * 2;
        return true;
    }
    for (int i = 0; i < MASTER_COUNT; ++i) {
        if (action.target != masterNames[i]) continue;
        engine.patch.master[i] = masterDenorm(i, amount);
        return true;
    }
    const std::string& target = action.target;
    bool enabled = amount != 0;
    auto& p = engine.patch;
    if (target == "OriginX") p.origin.x = amount;
    else if (target == "OriginY") p.origin.y = amount;
    else if (target == "Fill") engine.setDrawMode(enabled ? p.drawMode | DM_FILL : p.drawMode & ~DM_FILL);
    else if (target == "Outline") engine.setDrawMode(enabled ? p.drawMode | DM_OUTLINE : p.drawMode & ~DM_OUTLINE);
    else if (target == "Mirror") p.mirror = enabled; // upstream also leaves rendering unimplemented
    else if (target == "OriginDrag") p.orgMotion = enabled ? OM_DRAG : OM_PARK;
    else if (target == "OriginRandom") p.orgMotion = enabled ? OM_RANDOM : OM_PARK;
    else if (target == "Reverse") p.reverse = enabled;
    else if (target == "Convex") p.convex = enabled;
    else if (target == "LoopHue") p.loopHue = enabled;
    else if (target == "ZoomCenter") p.zoomCenter = enabled;
    else if (target == "Antialiasing") p.antialiasing = enabled;
    else if (target == "Clear") engine.clear();
    else if (target == "Pause") paused = !paused;
    else if (target == "RandomOrigin") {
        p.origin = {double(std::rand()) / RAND_MAX, double(std::rand()) / RAND_MAX};
        p.orgMotion = OM_PARK;
    } else if (target == "NextShape" || target == "PrevShape") {
        // Ignore the release edge so a mapped button steps once per press.
        if (amount != 0) {
            const int step = target == "NextShape" ? 1 : -1;
            int prim = static_cast<int>(std::lround(p.params[PARAM_Primitive].value)) + step;
            prim = ((prim % PRIM_COUNT) + PRIM_COUNT) % PRIM_COUNT;
            p.params[PARAM_Primitive].value = prim;
            status = std::string("Shape: ") + primitiveNames[prim];
        }
    } else if (target == "RandomPhase") {
        status = "Random oscillator phase not yet supported";
    } else if (target == "Patch") {
        int index = static_cast<int>(std::lround(amount * 127));
        if (index < 0 || index >= static_cast<int>(playlist.patches.size())) {
            status = "MIDI patch index out of range";
            return false;
        }
        Patch next;
        if (!next.load(playlist.resolvePath(index), status)) return false;
        engine.setPatch(next);
        currentPatch = index;
        status = "Playing " + playlist.patches[index];
    }
    return true;
}
} // namespace whorld
