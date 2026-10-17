// Dispatch mapped V2 MIDI actions onto the Linux animation state.
#pragma once
#include "playlist.h"
#include "whorld_core.h"
#include <string>

namespace whorld {
// Returns false only when an action (for example, opening a listed patch) fails.
bool applyMidiAction(const MappedAction& action, Engine& engine, Playlist& playlist,
                     bool& paused, int& currentPatch, std::string& status);
}
