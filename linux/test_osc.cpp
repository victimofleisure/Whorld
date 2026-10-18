// OSC parsing, convention addressing and a localhost UDP loopback.
#include "osc.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#define require(expr) do { if (!(expr)) throw std::runtime_error("Check failed: " #expr); } while (false)

using namespace whorld;

namespace {

// Builds an OSC string: characters, NUL, then pad to a 4-byte multiple.
void appendOscString(std::string& out, const std::string& text) {
    out.append(text);
    out.push_back('\0');
    while (out.size() % 4 != 0) out.push_back('\0');
}

void appendInt32(std::string& out, std::int32_t value) {
    const std::uint32_t bits = static_cast<std::uint32_t>(value);
    for (int b = 3; b >= 0; --b) out.push_back(static_cast<char>((bits >> (b * 8)) & 0xff));
}

void appendFloat32(std::string& out, double value) {
    const float f = static_cast<float>(value);
    std::uint32_t bits = 0;
    std::memcpy(&bits, &f, 4);
    for (int b = 3; b >= 0; --b) out.push_back(static_cast<char>((bits >> (b * 8)) & 0xff));
}

// Address plus type tag string, e.g. message("/whorld/x", "f").
std::string message(const std::string& address, const std::string& tags) {
    std::string out;
    appendOscString(out, address);
    appendOscString(out, "," + tags);
    return out;
}

void appendElement(std::string& bundle, const std::string& element) {
    appendInt32(bundle, static_cast<std::int32_t>(element.size()));
    bundle.append(element);
}

const unsigned char* bytes(const std::string& data) {
    return reinterpret_cast<const unsigned char*>(data.data());
}

} // namespace

int main() {
    // --- parseOscMessage: float, int, string ---------------------------------
    {
        OscMessage m;
        std::string packet = message("/whorld/RingGrowth", "f");
        appendFloat32(packet, 0.5);
        require(parseOscMessage(bytes(packet), packet.size(), m));
        require(m.address == "/whorld/RingGrowth");
        require(m.floats.size() == 1 && std::abs(m.floats[0] - 0.5) < 1e-6);
        require(m.ints.empty() && m.strings.empty());
        require(std::abs(m.value - 0.5) < 1e-6);

        packet = message("/whorld/mix", "is");
        appendInt32(packet, 42);
        appendOscString(packet, "hi");
        require(parseOscMessage(bytes(packet), packet.size(), m));
        require(m.ints.size() == 1 && m.ints[0] == 42);
        require(m.strings.size() == 1 && m.strings[0] == "hi");
        require(std::abs(m.value - 42) < 1e-9); // first numeric wins

        // An argument-free message decodes with the default full-on value.
        const std::string bare = message("/whorld/naked", "");
        require(parseOscMessage(bytes(bare), bare.size(), m));
        require(m.address == "/whorld/naked" && m.value == 1);

        // A bundle is not a single message.
        require(!parseOscMessage(reinterpret_cast<const unsigned char*>("#bundle\0\0\0\0"), 12, m));
        require(!parseOscMessage(nullptr, 0, m));
    }

    // --- parseOscPacket: bare message and bundle of two ----------------------
    {
        std::vector<OscMessage> parsed;
        std::string one = message("/whorld/a", "i");
        appendInt32(one, 10);
        require(parseOscPacket(bytes(one), one.size(), parsed));
        require(parsed.size() == 1 && parsed[0].address == "/whorld/a" &&
                parsed[0].ints.size() == 1 && parsed[0].ints[0] == 10);

        std::string two = message("/whorld/bb", "f");
        appendFloat32(two, 0.25);
        std::string bundle = "#bundle";
        bundle.push_back('\0'); // the literal still needs its OSC terminator
        const unsigned char timetag[8] = {0, 0, 0, 0, 0, 0, 0, 1};
        bundle.append(reinterpret_cast<const char*>(timetag), 8);
        appendElement(bundle, one);
        appendElement(bundle, two);
        parsed.clear();
        require(parseOscPacket(bytes(bundle), bundle.size(), parsed));
        require(parsed.size() == 2);
        require(parsed[0].address == "/whorld/a" && parsed[0].ints[0] == 10);
        require(parsed[1].address == "/whorld/bb" &&
                std::abs(parsed[1].floats[0] - 0.25) < 1e-6);

        // The legacy parseOsc wrapper decodes the same bundle.
        std::vector<OscMessage> legacy;
        require(parseOsc(bundle.data(), bundle.size(), legacy) && legacy.size() == 2);
    }

    // --- malformed / truncated input must be rejected, never crash -----------
    {
        std::vector<OscMessage> parsed;
        require(!parseOscPacket(nullptr, 0, parsed));
        require(!parseOscPacket(bytes(std::string("x")), 1, parsed));
        require(parsed.empty());

        std::string packet = message("/whorld/RingGrowth", "f");
        appendFloat32(packet, 0.5);
        // Truncating any suffix must not read out of bounds and must not decode.
        for (std::size_t cut = 1; cut < packet.size(); ++cut) {
            std::vector<OscMessage> partial;
            parseOscPacket(bytes(packet), cut, partial); // must not crash
        }
        require(!parseOscPacket(bytes(packet), packet.size() - 1, parsed));

        // An address must start with '/'.
        std::string badAddress = message("whorld/x", "i");
        appendInt32(badAddress, 1);
        require(!parseOscPacket(bytes(badAddress), badAddress.size(), parsed));

        // A bundle whose member size overruns the datagram fails cleanly.
        std::string bundle = "#bundle";
        const unsigned char timetag[8] = {0, 0, 0, 0, 0, 0, 0, 1};
        bundle.append(reinterpret_cast<const char*>(timetag), 8);
        const char bad[4] = {0, 0, 0, 64};
        bundle.append(bad, 4);
        require(!parseOscPacket(bytes(bundle), bundle.size(), parsed));
    }

    // --- oscToAction: convention addressing ----------------------------------
    {
        MappedAction a;
        std::string err;

        OscMessage m;
        m.address = "/whorld/RingGrowth";
        m.floats.push_back(0.5);
        require(oscToAction(m, a, err));
        require(a.target == "RingGrowth" && a.property == "Val" &&
                std::abs(a.amount - 50.0) < 1e-9);

        m.address = "/whorld/RingGrowth/Wave";
        require(oscToAction(m, a, err));
        require(a.target == "RingGrowth" && a.property == "Wave");

        // Values clip to 0..1 before scaling.
        m.address = "/whorld/Zoom";
        m.floats.assign(1, 9.0);
        require(oscToAction(m, a, err));
        require(a.target == "Zoom" && std::abs(a.amount - 100.0) < 1e-9);
        m.floats.assign(1, 0.0);
        require(oscToAction(m, a, err));
        require(std::abs(a.amount - 0.0) < 1e-9); // Zoom is continuous, not momentary

        // Integers work too, and a bare pulse is full-on.
        m.floats.clear();
        m.ints.assign(1, 1);
        m.address = "/whorld/Mirror";
        require(oscToAction(m, a, err));
        require(a.target == "Mirror" && std::abs(a.amount - 100.0) < 1e-9);
        m.ints.clear();
        m.address = "/whorld/Clear";
        require(oscToAction(m, a, err)); // no argument = on
        require(a.target == "Clear" && std::abs(a.amount - 100.0) < 1e-9);

        // Momentary targets stay silent on zero.
        m.floats.assign(1, 0.0);
        require(!oscToAction(m, a, err));

        // Action form: nonzero means on, zero is rejected.
        m.floats.clear();
        m.ints.assign(1, 1);
        m.address = "/whorld/action/Clear";
        require(oscToAction(m, a, err));
        require(a.target == "Clear" && std::abs(a.amount - 100.0) < 1e-9);
        m.ints.assign(1, 0);
        require(!oscToAction(m, a, err));

        // Invalid addresses / targets / properties are rejected.
        m.address = "/other/RingGrowth";
        require(!oscToAction(m, a, err));
        m.address = "/whorld/Bogus";
        require(!oscToAction(m, a, err));
        m.address = "/whorld/RingGrowth/Bogus";
        require(!oscToAction(m, a, err));
        m.address = "/whorld/RingGrowth/Val/Extra";
        require(!oscToAction(m, a, err));
        m.address = "/whorld/action/";
        require(!oscToAction(m, a, err));
        m.address = "/whorld/";
        require(!oscToAction(m, a, err));
    }

    // --- UDP loopback: open on port 0, send a datagram, poll -----------------
    {
        OscReceiver receiver;
        std::string error;
        require(receiver.open(0, error));
        require(receiver.isOpen());
        const int port = receiver.port();
        require(port > 0);
        require(receiver.port() == port);

        const int client = ::socket(AF_INET, SOCK_DGRAM, 0);
        require(client >= 0);
        sockaddr_in target{};
        target.sin_family = AF_INET;
        target.sin_port = htons(static_cast<std::uint16_t>(port));
        require(::inet_pton(AF_INET, "127.0.0.1", &target.sin_addr) == 1);
        std::string payload = message("/whorld/RingGrowth", "f");
        appendFloat32(payload, 0.25);
        const ssize_t sent = ::sendto(client, payload.data(), payload.size(), 0,
                                      reinterpret_cast<const sockaddr*>(&target),
                                      sizeof(target));
        require(sent == static_cast<ssize_t>(payload.size()));

        // Loopback delivery is fast but not synchronous; retry briefly.
        std::vector<OscMessage> got;
        for (int attempt = 0; attempt < 200 && got.empty(); ++attempt) {
            std::string pollError;
            require(receiver.poll(got, pollError));
            if (got.empty())
                std::this_thread::sleep_for(std::chrono::milliseconds(5));
        }
        ::close(client);
        require(got.size() == 1);
        require(got[0].address == "/whorld/RingGrowth");
        require(got[0].floats.size() == 1 && std::abs(got[0].floats[0] - 0.25) < 1e-6);

        // A closed receiver reports that it is not open rather than blocking.
        receiver.close();
        require(!receiver.isOpen());
        std::vector<OscMessage> none;
        require(!receiver.poll(none, error));
    }

    std::cout << "OSC parse, convention mapping and UDP loopback OK\n";
    return 0;
}
