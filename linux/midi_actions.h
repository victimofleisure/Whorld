// Dispatch mapped V2 MIDI actions onto the Linux animation state.
#pragma once
#include "playlist.h"
#include "whorld_core.h"
#include <string>

namespace whorld {
// Returns false only when an action (for example, opening a listed patch) fails.
bool applyMidiAction(const MappedAction& action, Engine& engine, Playlist& playlist,
                     bool& paused, int& currentPatch, std::string& status);
// The current value of a continuous target expressed in the same 0..1 "*amount*"
// space that applyMidiAction consumes (the inverse of its scaling). Used by soft
// takeover to compare a control's position with where the target already is.
// Targets without a continuous position (Clear, Pause, ...) return NaN.
double targetAmount(const std::string& target, const std::string& property, const Engine& engine);
}
