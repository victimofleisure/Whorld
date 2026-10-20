// Dispatch mapped V2 MIDI actions onto the Linux animation state.
#pragma once
#include "playlist.h"
#include "tap_tempo.h"
#include "whorld_core.h"
#include <string>

namespace whorld {
// Returns false only when an action (for example, opening a listed patch) fails.
// A non-null `tap` state lets the "Tap" target drive the tempo from a mapped
// button; callers that do not expose tap tempo may leave it out.
bool applyMidiAction(const MappedAction& action, Engine& engine, Playlist& playlist,
                     bool& paused, int& currentPatch, std::string& status,
                     TapTempo* tap = nullptr);
// The current value of a continuous target expressed in the same 0..1 "*amount*"
// space that applyMidiAction consumes (the inverse of its scaling). Used by soft
// takeover to compare a control's position with where the target already is.
// Targets without a continuous position (Clear, Pause, ...) return NaN.
double targetAmount(const std::string& target, const std::string& property, const Engine& engine);
// Master property conversions in the upstream (CMasterRowDlg) normalized space:
// 0..1 maps to the property's range the way the dialog's slider does. Used by
// interactive controls such as mouse-wheel zoom. Index must be a MASTER_* value.
double normalizeMaster(int index, double value);
double denormalizeMaster(int index, double normalized);
}
