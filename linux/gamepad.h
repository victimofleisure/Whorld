// Pure gamepad axis shaping and default-profile helpers (GPL-3.0-or-later).
// This module deliberately has no SDL dependency so the logic can be unit
// tested without a controller attached.
#pragma once
#include "playlist.h"
#include <string>
#include <vector>

namespace whorld {
// Shapes one raw axis reading into a usable control value, clamped to [-1, 1].
// A 1-D dead-zone first zeroes the noise band around centre: inputs whose
// magnitude is <= `deadzone` become exactly 0, and the remaining live band is
// rescaled so |raw| == deadzone -> 0 and |raw| == 1 -> 1, preserving sign (so
// a stick still reaches full scale at the physical limit). `exponent` is then
// applied to the magnitude: values > 1 make the response softer near centre and
// finer at the extremes, which tames twitchy sticks; 1 is linear. Non-finite
// inputs and a dead-zone that covers the whole range read as 0.
double shapeAxis(double raw, double deadzone = 0.15, double exponent = 1.6);

// True when a gamepad mapping set has at least one entry. The frontend uses
// this to decide whether to substitute defaultGamepadMappings().
bool hasGamepadMappings(const std::vector<GamepadMapping>& mappings);

// A playable default profile for a standard Xbox-style controller. Every entry
// uses a control name accepted by mapGamepad and a target accepted by
// validTarget, so the set validates and round-trips through Playlist::save().
// Sticks are centred controls chosen so their rest position is a neutral value;
// triggers rest at their Start value; buttons drive BOOL/ACTION targets.
const std::vector<GamepadMapping>& defaultGamepadMappings();

// A short status line for the connected controllers: "Controller: NAME" for a
// single pad, "Controllers: N (A, B, ...)" for several, or "" for none.
std::string controllerSummary(const std::vector<std::string>& names);
} // namespace whorld
