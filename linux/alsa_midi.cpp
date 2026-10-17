// Optional ALSA sequencer MIDI input, pumped on the UI thread (GPL-3.0-or-later).
#include "alsa_midi.h"

#include <alsa/asoundlib.h>
#include <algorithm>
#include <cerrno>
#include <cstring>
#include <string>
#include <unistd.h>

namespace whorld {
namespace {
constexpr int maxEventsPerPoll = 256;

int midi7(int value) { return std::clamp(value, 0, 127); }

} // namespace

struct MidiInput::Impl {
    snd_seq_t* seq = nullptr;
    int port = -1;
};

MidiInput::MidiInput() : impl(new Impl) {}

MidiInput::~MidiInput() {
    close();
    delete impl;
}

bool MidiInput::open(const std::string& source, std::string& error) {
    close();
    error.clear();
    if (access("/dev/snd/seq", F_OK) != 0) {
        if (errno == ENOENT) {
            error = "ALSA MIDI sequencer unavailable: /dev/snd/seq is missing "
                    "(enable the ALSA sequencer or load the snd_seq module)";
        } else {
            error = std::string("Unable to access /dev/snd/seq: ") + std::strerror(errno);
        }
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

    if (!source.empty()) {
        snd_seq_addr_t addr{};
        rc = snd_seq_parse_address(impl->seq, &addr, source.c_str());
        if (rc < 0) {
            error = std::string("Invalid ALSA MIDI source '") + source + "': " + snd_strerror(rc);
            close();
            return false;
        }
        rc = snd_seq_connect_from(impl->seq, impl->port, addr.client, addr.port);
        if (rc < 0) {
            error = std::string("Unable to subscribe to ALSA MIDI source '") + source + "': " + snd_strerror(rc);
            close();
            return false;
        }
    }
    return true;
}

void MidiInput::close() {
    if (impl->seq) {
        snd_seq_close(impl->seq); // Closing the client also removes its port and subscriptions.
        impl->seq = nullptr;
    }
    impl->port = -1;
}

std::vector<MidiMessage> MidiInput::poll() {
    std::vector<MidiMessage> messages;
    if (!impl->seq) return messages;

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

} // namespace whorld
