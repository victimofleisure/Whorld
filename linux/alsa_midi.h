// Optional ALSA sequencer MIDI input/output, pumped on the UI thread
// (GPL-3.0-or-later).
#pragma once
#include "midi_decode.h"
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
    // External clock state, updated by poll() from the system real-time events
    // (Clock/Start/Continue/Stop). They are never returned as MidiMessages.
    bool clockActive() const;
    bool clockRunning() const;
    double clockBpm() const;
private:
    bool openSequencer(std::string& error);
    struct Impl;
    Impl* impl;
};

// Optional ALSA sequencer MIDI output for soft-takeover feedback: echoes values
// back to a controller so LED rings, lamps and motorized faders follow.
class MidiOutput {
public:
    MidiOutput();
    ~MidiOutput();
    MidiOutput(const MidiOutput&) = delete;
    MidiOutput& operator=(const MidiOutput&) = delete;
    // Empty destination creates a subscribable virtual output port that
    // external gear can connect to; "client:port" also connects to that port.
    bool open(const std::string& destination, std::string& error);
    void close();
    bool isOpen() const;
    std::string portName() const;
    // Sends a raw MIDI byte stream (a complete message) through the sequencer
    // encoder; ignored when not open or when the bytes do not form a message.
    void sendRaw(const std::vector<unsigned char>& bytes);
    // Sends a 3-byte Control Change (see encodeControlChange); ignored when not
    // open.
    void sendControl(int channel, int control, int value);
private:
    struct Impl;
    Impl* impl;
};
} // namespace whorld
