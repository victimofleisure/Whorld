// Open Sound Control input over UDP (GPL-3.0-or-later).
//
// This module feeds two OSC paths:
//
//  1. Convention addressing (`oscToAction`), enabled by `--osc-port N`. It needs
//     no playlist setup: the address names the target directly.
//
//        /whorld/<Target>              property defaults to "Val"
//        /whorld/<Target>/<Property>   e.g. /whorld/RingGrowth/Wave
//        /whorld/action/<Target>       any nonzero argument means "on" (a pulse)
//
//     <Target> and <Property> are validated against the same tables the playlist
//     mappings use (`validTarget`/`validProperty`), and are case-sensitive like
//     every other Whorld tag. The value is the first argument: the first float
//     if the message carries one, otherwise the first integer (strings are
//     ignored). It is expected in 0..1 -- the same default domain as an [Osc]
//     mapping -- then clipped to 0..1 and scaled by 100 into the percentage
//     space `MappedAction::amount` uses, so `/whorld/RingGrowth/Val 0.5` sets the
//     parameter to the middle of its range. A message with no numeric argument
//     is treated as 1 (a full-on pulse), so a bare `/whorld/Clear` works from
//     tools that only send an address. Momentary targets (Clear, Pause, ...)
//     stay silent on a zero value, exactly like the other input sources.
//
//  2. Playlist `[Osc\N]` mappings (`--osc`), which match a stored address
//     pattern (with `?`/`*` wildcards) against the incoming address; the legacy
//     `parseOsc` below feeds that path.
//
// A packet is one message or a "#bundle"; bundles may nest, and a datagram may
// carry several messages. The parser is bounds-checked so hostile or truncated
// UDP datagrams are rejected without reading out of bounds. The receiver owns a
// non-blocking socket bound to all interfaces (INADDR_ANY, which includes
// loopback), so poll() never stalls the frame loop.
#pragma once
#include "playlist.h"
#include <cstddef>
#include <string>
#include <vector>

namespace whorld {

// A decoded OSC message. Arguments are separated by type. `value` additionally
// carries the first numeric (or True/False) argument for the playlist-mapping
// path; it defaults to 1 when the message has no arguments, so bare addresses
// still act as pulses.
struct OscMessage {
    std::string address;
    std::vector<int> ints;
    std::vector<double> floats;
    std::vector<std::string> strings;
    double value = 1; // first numeric/T/F argument (playlist mapping path)
};

// Decodes exactly one OSC message (not a bundle). Returns false when the buffer
// is not a well-formed message. Unknown argument type tags stop argument
// decoding (the message still lands with the arguments decoded so far), since
// their width cannot be known without implementing them.
bool parseOscMessage(const unsigned char* data, std::size_t len, OscMessage& out);

// Decodes one OSC packet: a bare message or a bundle (including nested
// bundles). Returns false only when nothing at all could be decoded; a
// malformed tail keeps the messages decoded before the error.
bool parseOscPacket(const unsigned char* data, std::size_t len, std::vector<OscMessage>& out);

// Legacy strict variant used by the playlist-mapping path: identical to
// parseOscPacket except that an unknown type tag rejects the message.
bool parseOsc(const char* data, std::size_t size, std::vector<OscMessage>& parsed);

// A non-blocking UDP receiver. The header comment documents the binding.
class OscReceiver {
public:
    ~OscReceiver();
    // Binds a non-blocking UDP socket on all interfaces. Port 0 asks the OS for
    // an ephemeral port (read it back with port()); 1..65535 bind explicitly.
    bool open(int port, std::string& error);
    // Legacy entry point for --osc: same as open() but rejects port 0.
    bool start(int port, std::string& error);
    void close();
    bool isOpen() const { return socket_ >= 0; }
    bool started() const { return socket_ >= 0; } // legacy alias for --osc
    // The bound port, resolved from the socket (useful after open(0)).
    int port() const { return port_; }
    // Drains every pending datagram without blocking. Returns false with
    // `error` only on a real socket error, never for an empty queue.
    bool poll(std::vector<OscMessage>& out, std::string& error);
    // Legacy variant used by --osc, bounded by maxMessages.
    void poll(std::vector<OscMessage>& out, std::size_t maxMessages = 256);

private:
    int socket_ = -1;
    int port_ = 0;
};

// Translates a convention-addressed message into an action (see the header
// comment). Returns false with `error` for an address outside /whorld/, an
// unknown target/property, or a zero value aimed at an action/momentary target;
// the caller ignores those datagrams.
bool oscToAction(const OscMessage& message, MappedAction& out, std::string& error);

} // namespace whorld
