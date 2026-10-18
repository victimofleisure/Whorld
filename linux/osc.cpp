// Open Sound Control input over UDP (GPL-3.0-or-later).
#include "osc.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <utility>

namespace whorld {
namespace {

// Bounds-checked big-endian reader. Positions stay absolute to the packet so
// bundle payloads inherit the packet's 4-byte lattice, as OSC requires.
class Reader {
public:
    Reader(const char* data, std::size_t size) : raw_(data), size_(size) {}

    std::size_t remaining() const { return size_ - pos_; }
    bool atEnd() const { return pos_ >= size_; }

    bool u32(std::uint32_t& value) {
        if (!need(4)) return false;
        const unsigned char* p = reinterpret_cast<const unsigned char*>(raw_) + pos_;
        value = (std::uint32_t(p[0]) << 24) | (std::uint32_t(p[1]) << 16) |
                (std::uint32_t(p[2]) << 8) | std::uint32_t(p[3]);
        pos_ += 4;
        return true;
    }
    bool i32(std::int32_t& value) {
        std::uint32_t bits = 0;
        if (!u32(bits)) return false;
        std::memcpy(&value, &bits, 4);
        return true;
    }
    bool skip64() { // timetags are skipped whole; only the length must fit
        if (!need(8)) return false;
        pos_ += 8;
        return true;
    }
    bool f32(double& value) {
        std::uint32_t bits = 0;
        if (!u32(bits)) return false;
        float f;
        std::memcpy(&f, &bits, 4);
        value = f;
        return true;
    }
    bool f64(double& value) {
        if (!need(8)) return false;
        std::uint64_t bits = 0;
        const unsigned char* p = reinterpret_cast<const unsigned char*>(raw_) + pos_;
        for (int i = 0; i < 8; ++i) bits = (bits << 8) | p[i];
        std::memcpy(&value, &bits, 8);
        pos_ += 8;
        return true;
    }
    bool string(std::string& value) {
        if (pos_ >= size_) return false;
        std::size_t end = pos_;
        while (end < size_ && raw_[end] != '\0') ++end;
        if (end >= size_) return false; // lost the terminator inside the block
        value.assign(raw_ + pos_, end - pos_);
        pos_ = end + 1; // consume the NUL
        align();
        return true;
    }
    bool blob() {
        std::uint32_t size = 0;
        if (!u32(size)) return false;
        if (size > remaining()) return false;
        pos_ += size;
        align();
        return true;
    }

private:
    bool need(std::size_t bytes) const { return bytes <= remaining(); }
    void align() {
        if (pos_ % 4 != 0) pos_ = (pos_ + 3) & ~std::size_t(3);
        if (pos_ > size_) pos_ = size_;
    }

    const char* raw_;
    std::size_t size_;
    std::size_t pos_ = 0;
};

bool isBundle(const std::string& address) {
    return address.size() == 7 && std::memcmp(address.data(), "#bundle", 7) == 0;
}

// Decodes the type tag and arguments of a message whose address was already
// consumed. `strict` rejects unknown type tags (the legacy parseOsc contract);
// otherwise decoding stops at the first unknown tag but keeps the message.
bool parseMessageBody(Reader& reader, const std::string& address, OscMessage& message,
                      bool strict) {
    std::string tags;
    if (!reader.string(tags) || tags.empty() || tags[0] != ',') return false;
    message = OscMessage{};
    message.address = address;
    bool haveValue = false;
    for (std::size_t i = 1; i < tags.size(); ++i) {
        const char tag = tags[i];
        if (tag == 'i') {
            std::int32_t n = 0;
            if (!reader.i32(n)) return false;
            message.ints.push_back(n);
            if (!haveValue) { message.value = n; haveValue = true; }
        } else if (tag == 'f') {
            double v = 0;
            if (!reader.f32(v)) return false;
            message.floats.push_back(v);
            if (!haveValue) { message.value = v; haveValue = true; }
        } else if (tag == 'd') {
            double v = 0;
            if (!reader.f64(v)) return false;
            message.floats.push_back(v);
            if (!haveValue) { message.value = v; haveValue = true; }
        } else if (tag == 's') {
            std::string s;
            if (!reader.string(s)) return false;
            message.strings.push_back(std::move(s));
        } else if (tag == 'b') {
            if (!reader.blob()) return false;
        } else if (tag == 'T') {
            if (!haveValue) { message.value = 1; haveValue = true; }
        } else if (tag == 'F') {
            if (!haveValue) { message.value = 0; haveValue = true; }
        } else if (strict) {
            return false;
        } else {
            break; // unknown tag: its width is unknown, so stop here
        }
    }
    return true;
}

// Decodes one element (message or bundle) starting at the reader's position;
// a bundle's members follow inside the same reader, sharing the packet lattice.
bool parseElement(Reader& reader, std::vector<OscMessage>& out, int depth, bool strict) {
    if (depth > 8) return false; // bound work for nested/hostile input
    std::string address;
    if (!reader.string(address)) return false;
    if (isBundle(address)) {
        if (!reader.skip64()) return false; // OSC time tag
        while (!reader.atEnd()) {
            std::uint32_t size = 0;
            if (!reader.u32(size)) return false;
            if (size == 0 || size > reader.remaining()) return false;
            if (!parseElement(reader, out, depth + 1, strict)) return false;
        }
        return true;
    }
    if (address.empty() || address[0] != '/') return false; // OSC addresses start with '/'
    OscMessage message;
    if (!parseMessageBody(reader, address, message, strict)) return false;
    out.push_back(std::move(message));
    return true;
}

constexpr std::size_t kMaxPacket = 1u << 20;

} // namespace

bool parseOscMessage(const unsigned char* data, std::size_t len, OscMessage& out) {
    if (!data || len == 0 || len > kMaxPacket) return false;
    Reader reader(reinterpret_cast<const char*>(data), len);
    std::string address;
    if (!reader.string(address)) return false;
    if (address.empty() || address[0] != '/') return false;
    return parseMessageBody(reader, address, out, false);
}

bool parseOscPacket(const unsigned char* data, std::size_t len, std::vector<OscMessage>& out) {
    if (!data || len == 0 || len > kMaxPacket) return false;
    Reader reader(reinterpret_cast<const char*>(data), len);
    if (!parseElement(reader, out, 0, false)) return !out.empty();
    return true;
}

bool parseOsc(const char* data, std::size_t size, std::vector<OscMessage>& parsed) {
    if (!data || size == 0 || size > kMaxPacket) return false;
    Reader reader(data, size);
    if (!parseElement(reader, parsed, 0, true)) return !parsed.empty();
    return true;
}

OscReceiver::~OscReceiver() { close(); }

void OscReceiver::close() {
    if (socket_ >= 0) ::close(socket_);
    socket_ = -1;
    port_ = 0;
}

bool OscReceiver::open(int port, std::string& error) {
    if (port < 0 || port > 65535) {
        error = "OSC port must be 0-65535";
        return false;
    }
    close();
    const int handle = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (handle < 0) {
        error = "cannot create UDP socket";
        return false;
    }
    int reuse = 1;
    if (::setsockopt(handle, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        ::close(handle);
        error = "cannot set SO_REUSEADDR";
        return false;
    }
    ::fcntl(handle, F_SETFL, O_NONBLOCK);
    sockaddr_in address{};
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY); // all interfaces, incl. loopback
    address.sin_port = htons(static_cast<std::uint16_t>(port));
    if (::bind(handle, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) < 0) {
        const std::string reason = std::strerror(errno);
        ::close(handle);
        error = "cannot bind UDP port " + std::to_string(port) + ": " + reason;
        return false;
    }
    socket_ = handle;
    // Resolve the actual port, which the OS chooses when port 0 was requested.
    sockaddr_in bound{};
    socklen_t boundLen = sizeof(bound);
    if (::getsockname(handle, reinterpret_cast<sockaddr*>(&bound), &boundLen) == 0)
        port_ = ntohs(bound.sin_port);
    else
        port_ = port;
    return true;
}

bool OscReceiver::start(int port, std::string& error) {
    if (port < 1 || port > 65535) {
        error = "OSC port must be 1-65535";
        return false;
    }
    return open(port, error);
}

bool OscReceiver::poll(std::vector<OscMessage>& out, std::string& error) {
    if (socket_ < 0) {
        error = "OSC receiver is not open";
        return false;
    }
    char buffer[8192];
    for (;;) {
        const ssize_t received = ::recv(socket_, buffer, sizeof(buffer), MSG_DONTWAIT);
        if (received > 0) {
            parseOscPacket(reinterpret_cast<const unsigned char*>(buffer),
                           static_cast<std::size_t>(received), out);
        } else if (received == 0) {
            break; // a zero-length datagram; nothing more to decode
        } else if (errno == EAGAIN || errno == EWOULDBLOCK) {
            break; // queue drained
        } else if (errno == EINTR) {
            continue;
        } else {
            error = std::strerror(errno);
            return false;
        }
    }
    return true;
}

void OscReceiver::poll(std::vector<OscMessage>& out, std::size_t maxMessages) {
    if (socket_ < 0) return;
    char buffer[8192];
    while (out.size() < maxMessages) {
        const ssize_t received = ::recv(socket_, buffer, sizeof(buffer), MSG_DONTWAIT);
        if (received <= 0) break; // drained (an error after a datagram stops here)
        parseOsc(buffer, static_cast<std::size_t>(received), out);
    }
}

bool oscToAction(const OscMessage& message, MappedAction& out, std::string& error) {
    const std::string prefix = "/whorld/";
    if (message.address.compare(0, prefix.size(), prefix) != 0) {
        error = "OSC address must start with /whorld/";
        return false;
    }
    // Split the remainder on '/'; empty segments are rejected below.
    std::vector<std::string> parts;
    std::size_t start = prefix.size();
    while (true) {
        const std::size_t slash = message.address.find('/', start);
        const std::size_t end = slash == std::string::npos ? message.address.size() : slash;
        parts.push_back(message.address.substr(start, end - start));
        if (slash == std::string::npos) break;
        start = slash + 1;
    }
    bool action = false;
    if (parts.front() == "action") {
        action = true;
        parts.erase(parts.begin());
    }
    if (parts.empty() || parts.front().empty()) {
        error = "OSC address is missing a target";
        return false;
    }
    if (parts.size() > 2) {
        error = "OSC address has too many segments";
        return false;
    }
    out.target = parts.front();
    out.property = parts.size() == 2 ? parts[1] : "Val";
    if (!validTarget(out.target)) {
        error = "unknown OSC target " + out.target;
        return false;
    }
    if (!validProperty(out.property)) {
        error = "unknown OSC property " + out.property;
        return false;
    }
    // The value is the first float if there is one, else the first integer; a
    // message with neither is a full-on pulse.
    double value = 1;
    if (!message.floats.empty()) value = message.floats.front();
    else if (!message.ints.empty()) value = message.ints.front();
    if (action) {
        if (value == 0) {
            error = "OSC action needs a nonzero argument";
            return false;
        }
        out.amount = 100;
        return true;
    }
    value = std::clamp(value, 0.0, 1.0);
    if (momentaryTarget(out.target) && value == 0) {
        error = "zero value for momentary target " + out.target;
        return false;
    }
    out.amount = value * 100.0;
    return true;
}

} // namespace whorld
