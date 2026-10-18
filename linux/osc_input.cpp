#include "osc_input.h"

#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cerrno>
#include <cstring>

namespace whorld {
namespace {

// Bounds-checked big-endian reader. Positions stay absolute to the packet so
// bundle payloads inherit the packet's 4-byte lattice, as OSC requires.
class Reader {
public:
    Reader(const char* data, std::size_t size) : raw_(data), size_(size) {}

    const char* raw() const { return raw_; }
    std::size_t position() const { return pos_; }
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
    bool need(std::size_t bytes) const {
        return bytes <= remaining();
    }
    void align() {
        if (pos_ % 4 != 0) pos_ = (pos_ + 3) & ~std::size_t(3);
        if (pos_ > size_) pos_ = size_;
    }

    const char* raw_;
    std::size_t size_;
    std::size_t pos_ = 0;
};

bool bundleElementCount(const std::string& address) {
    return address.size() == 7 && std::memcmp(address.data(), "#bundle", 7) == 0;
}

// Decodes one element (message or bundle) starting at the reader's position;
// a bundle's members follow inside the same reader, sharing the packet lattice.
bool parseElement(Reader& reader, std::vector<OscMessage>& out, int depth) {
    if (depth > 8) return false; // bound work for nested/hostile input
    std::string address;
    if (!reader.string(address)) return false;
    if (bundleElementCount(address)) {
        if (!reader.skip64()) return false; // OSC time tag
        while (!reader.atEnd()) {
            std::uint32_t size = 0;
            if (!reader.u32(size)) return false;
            if (size == 0 || size > reader.remaining()) return false;
            if (!parseElement(reader, out, depth + 1)) return false;
        }
        return true;
    }
    if (address.empty() || address[0] != '/') return false; // OSC addresses start with '/'
    std::string tags;
    if (!reader.string(tags) || tags.empty() || tags[0] != ',') return false;
    OscMessage message;
    message.address = address;
    bool haveValue = false;
    for (std::size_t i = 1; i < tags.size(); ++i) {
        const char tag = tags[i];
        if (tag == 'i') {
            std::int32_t n = 0;
            if (!reader.i32(n)) return false;
            if (!haveValue) { message.value = n; haveValue = true; }
        } else if (tag == 'f') {
            double v = 0;
            if (!reader.f32(v)) return false;
            if (!haveValue) { message.value = v; haveValue = true; }
        } else if (tag == 'd') {
            double v = 0;
            if (!reader.f64(v)) return false;
            if (!haveValue) { message.value = v; haveValue = true; }
        } else if (tag == 's') {
            std::string s;
            if (!reader.string(s)) return false;
        } else if (tag == 'b') {
            if (!reader.blob()) return false;
        } else if (tag == 'T') {
            if (!haveValue) { message.value = 1; haveValue = true; }
        } else if (tag == 'F') {
            if (!haveValue) { message.value = 0; haveValue = true; }
        } else {
            return false; // unknown type: refuse rather than desynchronize
        }
    }
    out.push_back(message); // argument-free messages keep value 1
    return true;
}

} // namespace

bool parseOsc(const char* data, std::size_t size, std::vector<OscMessage>& parsed) {
    if (!data || size == 0 || size > 1u << 20) return false;
    Reader reader(data, size);
    if (!parseElement(reader, parsed, 0)) return !parsed.empty();
    return true;
}

OscReceiver::~OscReceiver() {
    if (socket_ >= 0) ::close(socket_);
}

bool OscReceiver::start(int port, std::string& error) {
    if (port < 1 || port > 65535) {
        error = "OSC port must be 1-65535";
        return false;
    }
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
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(static_cast<std::uint16_t>(port));
    if (::bind(handle, reinterpret_cast<const sockaddr*>(&address), sizeof(address)) < 0) {
        const std::string reason = std::strerror(errno);
        ::close(handle);
        error = "cannot bind UDP port " + std::to_string(port) + ": " + reason;
        return false;
    }
    socket_ = handle;
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

} // namespace whorld
