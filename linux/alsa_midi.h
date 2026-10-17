// Optional ALSA sequencer MIDI input, pumped on the UI thread (GPL-3.0-or-later).
#pragma once
#include "playlist.h"
#include <string>
#include <vector>

namespace whorld {
class MidiInput {
public:
    MidiInput();
    ~MidiInput();
    MidiInput(const MidiInput&) = delete;
    MidiInput& operator=(const MidiInput&) = delete;
    // Empty source creates a subscribable virtual input port; "client:port"
    // also connects that ALSA source to us automatically.
    bool open(const std::string& source, std::string& error);
    void close();
    std::vector<MidiMessage> poll(); // nonblocking; process on main/UI thread
    std::string portName() const;
private:
    struct Impl;
    Impl* impl;
};
} // namespace whorld
