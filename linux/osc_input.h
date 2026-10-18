// Open Sound Control input over UDP (GPL-3.0-or-later).
//
// OSC packets arrive as datagrams. A packet is one or more messages (messages
// after the first must live inside a "#bundle"), so the parser walks messages,
// bundles and nested bundles with the same bounds-checked reader and reports
// whatever it decoded before any malformed element. Only the first numeric
// argument matters for mapping: its value is carried, and an argument-free
// message (or one that only carries a true/falsy value) maps to 1/0 so plain
// "/addr" pulses still drive actions. The receiver owns a non-blocking UDP
// socket, so poll() never stalls the frame loop.
#pragma once
#include <cstdint>
#include <cstddef>
#include <string>
#include <vector>

namespace whorld {

struct OscMessage {
    std::string address;
    double value = 1; // first numeric/T/F argument (see header comment)
};

// Decodes an OSC 1.0 packet (message or bundle). Returns false only when
// nothing at all could be decoded; malformed tails keep earlier messages.
bool parseOsc(const char* data, std::size_t size, std::vector<OscMessage>& parsed);

// Binds a UDP socket for receiving; start returns false with a reason when the
// port is invalid or the socket cannot be created/bound.
class OscReceiver {
public:
    ~OscReceiver();
    bool start(int port, std::string& error);
    bool started() const { return socket_ >= 0; }
    // Drains datagrams into `out` without blocking; at most `maxMessages`.
    void poll(std::vector<OscMessage>& out, std::size_t maxMessages = 256);

private:
    int socket_ = -1;
};

} // namespace whorld
