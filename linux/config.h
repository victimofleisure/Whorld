// Whorld show configuration: a small, human-editable INI-style file that sets
// the same startup options as the command line, so a kiosk can start from one
// readable file instead of a long WHORLD_ARGS line. A config supplies defaults;
// an explicit command-line flag always overrides it. Everything here is pure
// (no SDL/ALSA/OpenGL), so it unit-tests without a display.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "midi_decode.h"
#include <map>
#include <string>

namespace whorld {

// A generic INI-ish key/value store, matching the .whl style: one `key = value`
// per line, `#` or `;` starts a full-line comment, blank lines are ignored and
// keys are case-sensitive with surrounding whitespace trimmed. Only a leading
// `#`/`;` marks a comment, so values (paths, colours) may contain them.
//
// Unknown keys are kept verbatim, so a load/save round-trip does not lose keys
// this port does not understand yet. Duplicate keys and lines without an `=`
// are errors, and a failed load leaves the existing entries untouched.
class Config {
public:
    // Reads `path`, replacing every entry on success. Returns false and sets
    // `error` when the file cannot be read or a line is malformed.
    bool load(const std::string& path, std::string& error);
    // Writes every entry as `key = value`, sorted by key (so output is
    // deterministic). Returns false and sets `error` on an I/O failure.
    bool save(const std::string& path, std::string& error) const;
    bool has(const std::string& key) const;
    std::string getString(const std::string& key, const std::string& fallback = {}) const;
    // Booleans accept 1/0, true/false, yes/no and on/off, case-insensitively;
    // any other value yields `fallback`.
    bool getBool(const std::string& key, bool fallback = false) const;
    // Numbers are parsed in full; a malformed or trailing-garbage value yields
    // `fallback`.
    double getDouble(const std::string& key, double fallback = 0) const;
    int getInt(const std::string& key, int fallback = 0) const;
    void set(const std::string& key, const std::string& value);

private:
    std::map<std::string, std::string> entries_; // sorted, so save is stable
};

// Enum spellings shared by the command line and the config loader, so the
// accepted names cannot drift between the two. Each returns false (leaving the
// out parameter untouched) for anything it does not recognise and otherwise
// stores the same value the matching flag would.
bool parseMirrorMode(const std::string& value, int& mode); // off/quad/kaleidoscope/radial
bool parseBlendMode(const std::string& value, int& mode);  // normal/add/screen
bool parseMidiModeInput(const std::string& value, MidiInputMode& mode);
// Canonical spellings, used when saving a config.
const char* mirrorModeName(int mode);
const char* blendModeName(int mode);
const char* midiModeName(MidiInputMode mode);

// The startup settings a show config supplies as defaults. Every field is
// applied before the command line is parsed, so an explicit flag always wins.
//
// Serialized keys (lower camelCase, one per field; a missing key keeps the
// default shown by the field initializer):
//   patch, playlist                         paths (as --patch / --playlist)
//   renderer                                auto|gpu|cairo
//   scale                                   0 (adaptive) or 0.25..2.0
//   mirror                                   off|quad|kaleidoscope|radial
//   invert                                   true|false
//   feedback                                 0.0..0.99
//   blend                                    normal|add|screen
//   morph                                    seconds, 0..3600
//   warp                                     "TLx,TLy,TRx,TRy,BRx,BRy,BLx,BLy"
//   warpFile                                 path to four "x y" pairs
//   videoDriver                              SDL_VIDEODRIVER name
//   fullscreen, hideCursor                   true|false
//   midiSource                               CLIENT:PORT
//   midiMode                                 absolute|14bit|rel-twos|rel-sign|rel-binary
//   midiTakeover, midiFeedback, midiClock, midiClockOut  true|false
//   midiOut                                  CLIENT:PORT (empty = virtual port)
//   midiOutEnabled                           true|false (off = no MIDI output)
//   osc, oscPort                             --osc and its UDP port
//   oscDirect, oscDirectPort                 --osc-port convention addressing
//   audio, audioDevice, audioGain            audio input options
//
// saveShowConfig writes every key above (even at its default) in sorted order,
// and re-emits any unknown keys read from an existing file, so a save is a
// readable superset of what was loaded. A warp string is normalized to the
// canonical warpToString form on load.
struct ShowConfig {
    std::string patch;
    std::string playlist;
    std::string renderer = "auto";
    double scale = 0;        // 0 = adaptive (the CLI default)
    int mirror = 0;          // post_process.h mirrorMode
    bool invert = false;
    double feedback = 0;
    int blend = 0;           // post_process.h blendMode
    double morph = 0;
    std::string warp;        // empty = no warp
    std::string warpFile;
    std::string videoDriver;
    bool fullscreen = false;
    bool hideCursor = false;
    std::string panelSide = "left"; // ctrl+D toggles; also --panel-side
    std::string midiSource;
    MidiInputMode midiMode = MidiInputMode::Absolute;
    bool midiTakeover = false;
    std::string midiOut;     // destination; empty = virtual output port
    bool midiOutEnabled = false;
    bool midiFeedback = false;
    bool midiClock = false;
    bool midiClockOut = false; // --midi-clock-out: send 24 PPQN + transport
    double wheelSense = 6.25;  // percent of the Zoom range per wheel notch (upstream default)
    bool osc = false;
    int oscPort = 8000;
    bool oscDirect = false;
    int oscDirectPort = 8000;
    bool audio = false;
    std::string audioDevice;
    double audioGain = 1.0;
    // Unrecognised keys from the file this struct was loaded from, re-emitted
    // by saveShowConfig so a round-trip loses nothing. Empty for a struct built
    // in code (e.g. by --config-save).
    Config unknown;
};

// Loads/saves a ShowConfig built on Config. loadShowConfig validates the enum
// spellings and numeric ranges and returns false with `error` on a bad file or
// value; `show` is only replaced on success. saveShowConfig returns false with
// `error` when the file cannot be written.
bool loadShowConfig(const std::string& path, ShowConfig& show, std::string& error);
bool saveShowConfig(const std::string& path, const ShowConfig& show, std::string& error);

} // namespace whorld
