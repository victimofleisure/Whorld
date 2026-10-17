// Optional ALSA sequencer MIDI input, pumped on the UI thread (GPL-3.0-or-later).
#pragma once
#include "playlist.h"
#include <string>
#include <vector>

namespace whorld {
// Fills `sources` with the readable MIDI source ports visible to the sequencer,
// formatted as "client:port  name". Returns false with a clear `error` when the
// sequencer is unavailable (for example when /dev/snd/seq is missing).
bool listMidiSources(std::vector<std::string>& sources, std::string& error);

class MidiInput {
public:
    MidiInput();
    ~MidiInput();
    MidiInput(const MidiInput&) = delete;
    MidiInput& operator=(const MidiInput&) = delete;
    // Empty source creates a subscribable virtual input port; "client:port"
    // also connects that ALSA source to us automatically.
    bool open(const std::string& source, std::string& error);
    // Reopens a configured source that was missing at launch or whose
    // subscription dropped (poll() closes on error). Rate-limited to at most
    // once every two seconds, so it is cheap to call every frame. A virtual
    // input port (empty source) is never recreated, since that would change its
    // client id and break external aconnect subscriptions.
    bool ensureOpen(std::string& error);
    void close();
    bool isOpen() const;
    std::vector<MidiMessage> poll(); // nonblocking; process on main/UI thread
    std::string portName() const;
private:
    bool openSequencer(std::string& error);
    struct Impl;
    Impl* impl;
};
} // namespace whorld
