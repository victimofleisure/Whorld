// Pure gamepad axis shaping and default-profile helpers (GPL-3.0-or-later).
#include "gamepad.h"

#include <algorithm>
#include <cmath>

namespace whorld {

double shapeAxis(double raw, double deadzone, double exponent) {
    if (!std::isfinite(raw)) return 0.0;
    raw = std::clamp(raw, -1.0, 1.0); // tolerate out-of-range hardware values
    if (!std::isfinite(deadzone) || deadzone < 0) deadzone = 0;
    if (!std::isfinite(exponent) || exponent <= 0) exponent = 1;
    if (deadzone >= 1.0) return 0.0; // dead-zone covers the whole range
    const double magnitude = std::abs(raw);
    if (magnitude <= deadzone) return 0.0;
    // Rescale the live band so the dead-zone edge is 0 and full deflection is 1,
    // then bend it with the response curve; copysign restores the direction.
    const double live = (magnitude - deadzone) / (1.0 - deadzone);
    const double shaped = std::pow(std::min(live, 1.0), exponent);
    return std::copysign(std::min(shaped, 1.0), raw);
}

bool hasGamepadMappings(const std::vector<GamepadMapping>& mappings) {
    return !mappings.empty();
}

const std::vector<GamepadMapping>& defaultGamepadMappings() {
    // Neutral at rest: the sticks are centred so a centred stick holds the
    // target's neutral value (Zoom 1, RotateSpeed 0, StarFactor/Pinwheel 0).
    // The triggers rest at their Start value, which is the upstream init value
    // for RingGrowth (1) and LineWidth (1). Buttons use BOOL/ACTION targets;
    // Clear/Pause/Patch/NextShape/PrevShape are momentary (fire once per press).
    // Only parameter targets carry a property; masters and misc actions ignore
    // Prop, so leaving it empty keeps the set byte-for-byte round-trippable.
    static const std::vector<GamepadMapping> defaults = {
        // Left stick: zoom and spin, the two most useful live controls.
        {"LX", "Zoom", "", 0, 100},
        {"LY", "RotateSpeed", "Val", 0, 100},
        // Right stick: reshape the active polygon.
        {"RX", "StarFactor", "Val", 0, 100},
        {"RY", "Pinwheel", "Val", 0, 100},
        // Triggers: squeeze for faster growth / thicker lines.
        {"LTrigger", "RingGrowth", "Val", 10, 100},
        {"RTrigger", "LineWidth", "Val", 2, 80},
        // Face buttons: playback toggles.
        {"A", "Pause", "", 0, 100},
        {"B", "Clear", "", 0, 100},
        {"X", "Fill", "", 0, 100},
        {"Y", "Outline", "", 0, 100},
        // Shoulders: post-processing toggles.
        {"LB", "Mirror", "", 0, 100},
        {"RB", "InvertColor", "", 0, 100},
        // D-pad: step the primitive, select a playlist patch, re-randomize origin.
        {"DPadLeft", "PrevShape", "", 0, 100},
        {"DPadRight", "NextShape", "", 0, 100},
        {"DPadUp", "Patch", "", 0, 100},
        {"DPadDown", "RandomOrigin", "", 0, 100},
    };
    return defaults;
}

std::string controllerSummary(const std::vector<std::string>& names) {
    if (names.empty()) return {};
    if (names.size() == 1) return "Controller: " + names.front();
    std::string summary = "Controllers: " + std::to_string(names.size()) + " (";
    for (std::size_t i = 0; i < names.size(); ++i) {
        if (i) summary += ", ";
        summary += names[i];
    }
    summary += ")";
    return summary;
}

} // namespace whorld
