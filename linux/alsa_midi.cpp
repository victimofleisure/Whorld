// Optional ALSA sequencer MIDI input/output, pumped on the UI thread
// (GPL-3.0-or-later).
#include "alsa_midi.h"

#include <alsa/asoundlib.h>
#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstring>
#include <string>
#include <unistd.h>

namespace whorld {
namespace {
constexpr int maxEventsPerPoll = 256;
// How long to wait between attempts to (re)subscribe a configured source.
constexpr auto reconnectInterval = std::chrono::seconds(2);

int midi7(int value) { return std::clamp(value, 0, 127); }

// Monotonic seconds for the clock tracker. steady_clock cannot jump with the
// wall clock, so a tempo change caused by NTP/DPMS is impossible.
double monotonicSeconds() {
    return std::chrono::duration<double>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

// Preferred timestamp for a system clock event. ALSA stamps it with the
// sequencer's real-time clock, which keeps the true spacing when several
// pulses are drained in one UI poll (steady_clock would collapse them to ~0);
// events without a real-time stamp fall back to steady_clock. Only deltas are
// used, and a stamp switch simply makes tick() relock, so mixing is safe.
double clockSeconds(const snd_seq_event_t* event) {
    if ((event->flags & SND_SEQ_TIME_STAMP_MASK) == SND_SEQ_TIME_STAMP_REAL)
        return event->time.time.tv_sec + event->time.time.tv_nsec * 1e-9;
    return monotonicSeconds();
}

std::string sequencerUnavailable() {
    if (errno == ENOENT) {
        return "ALSA MIDI sequencer unavailable: /dev/snd/seq is missing "
               "(enable the ALSA sequencer or load the snd_seq module)";
    }
    return std::string("Unable to access /dev/snd/seq: ") + std::strerror(errno);
}

} // namespace

bool listMidiSources(std::vector<std::string>& sources, std::string& error) {
    sources.clear();
    error.clear();
    if (access("/dev/snd/seq", F_OK) != 0) {
        error = sequencerUnavailable();
        return false;
    }
    snd_seq_t* seq = nullptr;
    int rc = snd_seq_open(&seq, "default", SND_SEQ_OPEN_INPUT, 0);
    if (rc < 0) {
        error = std::string("Unable to open ALSA MIDI sequencer: ") + snd_strerror(rc);
        return false;
    }
    snd_seq_client_info_t* client = nullptr;
    snd_seq_port_info_t* port = nullptr;
    snd_seq_client_info_alloca(&client);
    snd_seq_port_info_alloca(&port);
    const int self = snd_seq_client_id(seq);
    snd_seq_client_info_set_client(client, -1);
    while (snd_seq_query_next_client(seq, client) >= 0) {
        const int clientId = snd_seq_client_info_get_client(client);
        if (clientId == self) continue; // a source cannot read from itself
        snd_seq_port_info_set_client(port, clientId);
        snd_seq_port_info_set_port(port, -1);
        while (snd_seq_query_next_port(seq, port) >= 0) {
            const unsigned int caps = snd_seq_port_info_get_capability(port);
            const unsigned int type = snd_seq_port_info_get_type(port);
            // SND_SEQ_PORT_CAP_READ marks a port others can read from, i.e. a
            // source we can subscribe to as an input client; the type bit keeps
            // the sequencer's own System timer/announce ports out of the list.
            if (!(caps & SND_SEQ_PORT_CAP_READ) || !(type & SND_SEQ_PORT_TYPE_MIDI_GENERIC)) continue;
            sources.push_back(std::to_string(clientId) + ":" +
                              std::to_string(snd_seq_port_info_get_port(port)) + "  " +
                              snd_seq_client_info_get_name(client) + " / " +
                              snd_seq_port_info_get_name(port));
        }
    }
    snd_seq_close(seq);
    return true;
}

struct MidiInput::Impl {
    snd_seq_t* seq = nullptr;
    int port = -1;
    std::string source; // remembered so a dropped subscription can be retried
    std::chrono::steady_clock::time_point nextRetry{};
    ClockTracker clock;
};

MidiInput::MidiInput() : impl(new Impl) {}

MidiInput::~MidiInput() {
    close();
    delete impl;
}

bool MidiInput::open(const std::string& source, std::string& error) {
    close();
    impl->source = source;
    // The first automatic retry waits one interval; open() itself tries now.
    impl->nextRetry = std::chrono::steady_clock::now() + reconnectInterval;
    return openSequencer(error);
}

bool MidiInput::openSequencer(std::string& error) {
    error.clear();
    if (access("/dev/snd/seq", F_OK) != 0) {
        error = sequencerUnavailable();
        return false;
    }

    int rc = snd_seq_open(&impl->seq, "default", SND_SEQ_OPEN_INPUT, SND_SEQ_NONBLOCK);
    if (rc < 0) {
        error = std::string("Unable to open ALSA MIDI sequencer: ") + snd_strerror(rc);
        close();
        return false;
    }
    rc = snd_seq_set_client_name(impl->seq, "Whorld");
    if (rc < 0) {
        error = std::string("Unable to name ALSA MIDI client: ") + snd_strerror(rc);
        close();
        return false;
    }
    impl->port = snd_seq_create_simple_port(impl->seq, "MIDI Input",
                                            SND_SEQ_PORT_CAP_WRITE | SND_SEQ_PORT_CAP_SUBS_WRITE,
                                            SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
    if (impl->port < 0) {
        error = std::string("Unable to create ALSA MIDI input port: ") + snd_strerror(impl->port);
        close();
        return false;
    }

    if (!impl->source.empty()) {
        snd_seq_addr_t addr{};
        rc = snd_seq_parse_address(impl->seq, &addr, impl->source.c_str());
        if (rc < 0) {
            error = std::string("Invalid ALSA MIDI source '") + impl->source + "': " + snd_strerror(rc);
            close();
            return false;
        }
        rc = snd_seq_connect_from(impl->seq, impl->port, addr.client, addr.port);
        if (rc < 0) {
            error = std::string("Unable to subscribe to ALSA MIDI source '") + impl->source + "': " + snd_strerror(rc);
            close();
            return false;
        }
    }
    return true;
}

bool MidiInput::ensureOpen(std::string& error) {
    error.clear();
    if (impl->seq) return true;
    // Only a configured source is retried; recreating the virtual port would
    // change its address and break external subscriptions.
    if (impl->source.empty()) return false;
    const auto now = std::chrono::steady_clock::now();
    if (now < impl->nextRetry) return false;
    impl->nextRetry = now + reconnectInterval;
    return openSequencer(error);
}

void MidiInput::close() {
    if (impl->seq) {
        snd_seq_close(impl->seq); // Closing the client also removes its port and subscriptions.
        impl->seq = nullptr;
    }
    impl->port = -1;
    impl->clock.reset(); // a closed source no longer supplies a clock
}

bool MidiInput::isOpen() const {
    return impl->seq != nullptr;
}

std::vector<MidiMessage> MidiInput::poll() {
    std::vector<MidiMessage> messages;
    if (!impl->seq) return messages;

    // Let a silent clock expire between frames (no pulse arrives to say so).
    impl->clock.update(monotonicSeconds());

    // Bound both recognized and unrecognized events so a busy source cannot stall the UI.
    for (int i = 0; i < maxEventsPerPoll; ++i) {
        snd_seq_event_t* event = nullptr;
        const int rc = snd_seq_event_input(impl->seq, &event);
        // A successful read can return 0 when this was the last queued event.
        if (rc < 0) {
            if (event) snd_seq_free_event(event);
            if (rc != -EAGAIN && rc != -EINTR) close();
            break;
        }
        if (!event) break;

        MidiMessage msg;
        bool recognized = true;
        switch (event->type) {
        // System real-time: drive the tempo tracker but never map them.
        case SND_SEQ_EVENT_CLOCK:
            impl->clock.tick(clockSeconds(event));
            recognized = false;
            break;
        case SND_SEQ_EVENT_START:
        case SND_SEQ_EVENT_CONTINUE:
            impl->clock.start();
            recognized = false;
            break;
        case SND_SEQ_EVENT_STOP:
            impl->clock.stop();
            recognized = false;
            break;
        case SND_SEQ_EVENT_NOTEOFF:
            msg.event = MidiEvent::NoteOff;
            msg.channel = std::clamp(static_cast<int>(event->data.note.channel), 0, 15);
            msg.control = midi7(event->data.note.note);
            msg.value = midi7(event->data.note.velocity);
            break;
        case SND_SEQ_EVENT_NOTEON:
            msg.event = MidiEvent::NoteOn;
            msg.channel = std::clamp(static_cast<int>(event->data.note.channel), 0, 15);
            msg.control = midi7(event->data.note.note);
            msg.value = midi7(event->data.note.velocity);
            break;
        case SND_SEQ_EVENT_KEYPRESS:
            msg.event = MidiEvent::KeyAftertouch;
            msg.channel = std::clamp(static_cast<int>(event->data.note.channel), 0, 15);
            msg.control = midi7(event->data.note.note);
            msg.value = midi7(event->data.note.velocity);
            break;
        case SND_SEQ_EVENT_CONTROLLER:
            msg.event = MidiEvent::Control;
            msg.channel = std::clamp(static_cast<int>(event->data.control.channel), 0, 15);
            msg.control = midi7(event->data.control.param);
            msg.value = midi7(event->data.control.value);
            break;
        case SND_SEQ_EVENT_PGMCHANGE:
            msg.event = MidiEvent::Program;
            msg.channel = std::clamp(static_cast<int>(event->data.control.channel), 0, 15);
            msg.value = midi7(event->data.control.value);
            break;
        case SND_SEQ_EVENT_CHANPRESS:
            msg.event = MidiEvent::ChannelAftertouch;
            msg.channel = std::clamp(static_cast<int>(event->data.control.channel), 0, 15);
            msg.value = midi7(event->data.control.value);
            break;
        case SND_SEQ_EVENT_PITCHBEND:
            msg.event = MidiEvent::Wheel;
            msg.channel = std::clamp(static_cast<int>(event->data.control.channel), 0, 15);
            msg.value = std::clamp(event->data.control.value, -8192, 8191) + 8192;
            break;
        default:
            recognized = false;
            break;
        }
        snd_seq_free_event(event);
        if (recognized) messages.push_back(msg);
    }
    return messages;
}

std::string MidiInput::portName() const {
    if (!impl->seq || impl->port < 0) return {};
    return std::to_string(snd_seq_client_id(impl->seq)) + ":" + std::to_string(impl->port);
}

bool MidiInput::clockActive() const { return impl->clock.active(); }

bool MidiInput::clockRunning() const { return impl->clock.running(); }

double MidiInput::clockBpm() const { return impl->clock.bpm(); }

struct MidiOutput::Impl {
    snd_seq_t* seq = nullptr;
    int port = -1;
    snd_midi_event_t* encoder = nullptr; // raw bytes -> sequencer events
};

MidiOutput::MidiOutput() : impl(new Impl) {}

MidiOutput::~MidiOutput() {
    close();
    delete impl;
}

bool MidiOutput::open(const std::string& destination, std::string& error) {
    close();
    error.clear();
    if (access("/dev/snd/seq", F_OK) != 0) {
        error = sequencerUnavailable();
        return false;
    }

    int rc = snd_seq_open(&impl->seq, "default", SND_SEQ_OPEN_OUTPUT, SND_SEQ_NONBLOCK);
    if (rc < 0) {
        error = std::string("Unable to open ALSA MIDI sequencer: ") + snd_strerror(rc);
        close();
        return false;
    }
    rc = snd_seq_set_client_name(impl->seq, "Whorld");
    if (rc < 0) {
        error = std::string("Unable to name ALSA MIDI client: ") + snd_strerror(rc);
        close();
        return false;
    }
    // An output port is one others read from, so it advertises READ/SUBS_READ.
    impl->port = snd_seq_create_simple_port(impl->seq, "MIDI Output",
                                            SND_SEQ_PORT_CAP_READ | SND_SEQ_PORT_CAP_SUBS_READ,
                                            SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
    if (impl->port < 0) {
        error = std::string("Unable to create ALSA MIDI output port: ") + snd_strerror(impl->port);
        close();
        return false;
    }

    if (!destination.empty()) {
        snd_seq_addr_t addr{};
        rc = snd_seq_parse_address(impl->seq, &addr, destination.c_str());
        if (rc < 0) {
            error = std::string("Invalid ALSA MIDI destination '") + destination + "': " + snd_strerror(rc);
            close();
            return false;
        }
        rc = snd_seq_connect_to(impl->seq, impl->port, addr.client, addr.port);
        if (rc < 0) {
            error = std::string("Unable to connect to ALSA MIDI destination '") + destination + "': " + snd_strerror(rc);
            close();
            return false;
        }
    }

    rc = snd_midi_event_new(64, &impl->encoder);
    if (rc < 0) {
        error = std::string("Unable to create MIDI event encoder: ") + snd_strerror(rc);
        close();
        return false;
    }
    return true;
}

void MidiOutput::close() {
    if (impl->encoder) {
        snd_midi_event_free(impl->encoder);
        impl->encoder = nullptr;
    }
    if (impl->seq) {
        snd_seq_close(impl->seq); // Closing the client also removes its port and subscriptions.
        impl->seq = nullptr;
    }
    impl->port = -1;
}

bool MidiOutput::isOpen() const { return impl->seq != nullptr; }

std::string MidiOutput::portName() const {
    if (!impl->seq || impl->port < 0) return {};
    return std::to_string(snd_seq_client_id(impl->seq)) + ":" + std::to_string(impl->port);
}

void MidiOutput::sendRaw(const std::vector<unsigned char>& bytes) {
    if (!impl->seq || !impl->encoder || bytes.empty()) return;
    snd_seq_event_t event;
    snd_seq_ev_clear(&event);
    snd_midi_event_reset_encode(impl->encoder);
    // encode() consumes a complete message and fills the event; a status byte
    // that expects more bytes than we were given is dropped rather than sent.
    const long used = snd_midi_event_encode(impl->encoder, bytes.data(),
                                            static_cast<long>(bytes.size()), &event);
    if (used < static_cast<long>(bytes.size())) return;
    snd_seq_ev_set_source(&event, impl->port);
    snd_seq_ev_set_subs(&event);
    snd_seq_ev_set_direct(&event);
    snd_seq_event_output(impl->seq, &event);
    snd_seq_drain_output(impl->seq);
}

void MidiOutput::sendControl(int channel, int control, int value) {
    if (!impl->seq) return;
    std::vector<unsigned char> bytes;
    encodeControlChange(bytes, channel, control, value);
    sendRaw(bytes);
}

} // namespace whorld
