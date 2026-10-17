// Whorld V2 .whl playlists and control mapping declarations (GPL-3.0-or-later).
#pragma once
#include <string>
#include <vector>

namespace whorld {
enum class MidiEvent { NoteOff, NoteOn, KeyAftertouch, Control, Program, ChannelAftertouch, Wheel };
struct MidiMessage {
    MidiEvent event = MidiEvent::Control;
    int channel = 0; // zero-based, 0..15
    int control = 0; // note/CC number for events that use it
    int value = 0;   // 7-bit for most messages, 14-bit (0..16383) for wheel
};
struct Mapping {
    std::string event = "CONTROL";
    int channel = 0;
    int control = 1;
    std::string target = "RingGrowth";
    std::string property = "Val";
    double start = 0, end = 100; // percentages, including reversed ranges
};
struct MappedAction {
    std::string target, property;
    double amount = 0; // scaled by Start/End, NOT universally clamped
};

// Gamepad controls, using SDL_GameController's layout. Stick axes are raw
// (-1..1, centered at zero); triggers are 0..1.
enum GamepadAxis { GP_LX, GP_LY, GP_RX, GP_RY, GP_LTRIGGER, GP_RTRIGGER, GP_AXIS_COUNT };
enum GamepadButton {
    GP_A, GP_B, GP_X, GP_Y, GP_LB, GP_RB, GP_BACK, GP_START, GP_GUIDE,
    GP_LSTICK, GP_RSTICK, GP_DPAD_UP, GP_DPAD_DOWN, GP_DPAD_LEFT, GP_DPAD_RIGHT,
    GP_BUTTON_COUNT
};
struct GamepadState {
    double axis[GP_AXIS_COUNT] = {};
    bool button[GP_BUTTON_COUNT] = {};
};
struct GamepadMapping {
    std::string control; // axis or button name, e.g. "LX", "RTrigger", "A"
    std::string target, property;
    double start = 0, end = 100; // percentages, applied to the 0..1 control value
};

// Sound analysis features produced by AudioInput, each normalized to 0..1.
struct AudioFeatures {
    double level = 0, low = 0, mid = 0, high = 0;
};
struct AudioMapping {
    std::string feature; // "Level", "Low", "Mid" or "High"
    std::string target, property;
    double start = 0, end = 100; // percentages, applied to the 0..1 feature
};

struct Playlist {
    std::vector<std::string> patches; // stored path strings in playlist order
    std::vector<Mapping> mappings;
    std::vector<GamepadMapping> gamepadMappings;
    std::vector<AudioMapping> audioMappings;
    std::string path; // most recently opened/saved .whl
    bool load(const std::string& filename, std::string& error);
    bool save(const std::string& filename, std::string& error);
    std::string resolvePath(std::size_t index) const;
};
// Scales a 0..1 normalized control value onto the mapping's Start..End range
// (percentages). No universal clamp: inverted/out-of-range ranges are valid, as
// in mapMidi. Relative-encoder modes reuse this for their running value.
double applyRange(const Mapping& mapping, double normalized);
// Maps a pre-normalized 0..1 control value onto every matching mapping. Shares
// mapMidi's matching and validation semantics but skips value decoding, so the
// relative-encoder modes can supply their own running value.
std::vector<MappedAction> mapMidiNormalized(const Playlist& playlist, MidiEvent event,
                                            int channel, int control, double normalized);
std::vector<MappedAction> mapMidi(const Playlist& playlist, const MidiMessage& message);
// Binds (or rebinds) a gamepad control to a target/property, replacing any
// existing mapping for the same control (mirrors the MIDI learn behaviour).
void learnGamepad(Playlist& playlist, const std::string& control, const std::string& target,
                  const std::string& property);
// The name of a gamepad control by combined index: axes [0, GP_AXIS_COUNT) then
// buttons. Returns "" for an out-of-range index. Used to name a learned control.
const char* gamepadControlName(int index);
// `previous` is only consulted for momentary targets, which fire once per press.
std::vector<MappedAction> mapGamepad(const Playlist& playlist, const GamepadState& current,
                                     const GamepadState& previous);
std::vector<MappedAction> mapAudio(const Playlist& playlist, const AudioFeatures& features);
// A sensible default set used when audio input is enabled but the playlist
// carries no [Audio] mappings of its own.
const std::vector<AudioMapping>& defaultAudioMappings();
// Targets whose action should fire once per press (Clear, Pause, Patch, ...)
// rather than being continuously held, so a held gamepad button cannot repeat
// or rapidly toggle the action.
bool momentaryTarget(const std::string& target);
// The V2 tag name for a message event, e.g. MidiEvent::Control -> "CONTROL".
std::string midiEventName(MidiEvent event);
} // namespace whorld
