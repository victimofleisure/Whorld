// Whorld V2 playlist interchange and MIDI mapping (GPL-3.0-or-later).
#include "playlist.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <limits>
#include <map>
#include <stdexcept>
#include <string>
#include <utility>

namespace whorld {
namespace {
namespace fs = std::filesystem;

// Limits apply before allocation and to output as well as input. Whorld's INI
// reader has no limits, but these prevent untrusted counts/lines from growing
// vectors or maps without bound.
constexpr std::size_t kMaxFile = 16 * 1024 * 1024;
constexpr std::size_t kMaxLine = 32768;
constexpr std::size_t kMaxPath = 4096; // UTF-8 bytes
constexpr std::size_t kMaxItems = 4096;
constexpr std::size_t kMaxEntries = 32768;

// Use the same ordered tag definitions as CMappingBase::Initialize, rather than
// maintaining a second, manually transcribed list of valid targets/properties.
const char* const kEvents[] = {
#define MIDICHANSTATDEF(name) #name,
#include "../trunk/Whorld/MidiCtrlrDef.h"
};
const char* const kParameters[] = {
#define PARAMDEF(name, minval, maxval, steps, scale, initval) #name,
#include "../trunk/Whorld/WhorldDef.h"
#define SHAPEDEF(name, minval, maxval, steps, initval) #name,
#include "shape_def.h"
};
const char* const kMasters[] = {
#define MASTERDEF(name, type, prefix, initval) #name,
#include "../trunk/Whorld/WhorldDef.h"
};
const char* const kMisc[] = {
#define MAPPINGDEF_MISC_TARGET(name, type) #name,
#include "../trunk/Whorld/MappingDef.h"
    // Port-local actions: step the active shape primitive from a button, and a
    // mappable color-inversion toggle (upstream only exposes it as a main bool).
    "NextShape", "PrevShape", "InvertColor",
};
const char* const kProperties[] = {
#define PARAMPROPDEF(name, type, prefix, variant) #name,
#include "../trunk/Whorld/WhorldDef.h"
};
// Gamepad controls stay string-based in the file so a mapping is readable and
// portable; the enum order here must match the GamepadAxis/GamepadButton enums.
const char* const kGamepadControls[] = {
    "LX", "LY", "RX", "RY", "LTrigger", "RTrigger",
    "A", "B", "X", "Y", "LB", "RB", "Back", "Start", "Guide",
    "LStick", "RStick", "DPadUp", "DPadDown", "DPadLeft", "DPadRight",
};
const char* const kAudioFeatures[] = {"Level", "Low", "Mid", "High", "Beat"};

// INI sections/keys are deliberately case-sensitive, as are V2 tag lookups.
using Section = std::map<std::string, std::string>;
using Ini = std::map<std::string, Section>;

const std::string* find(const Ini& ini, const std::string& section, const std::string& key) {
    const auto s = ini.find(section);
    if (s == ini.end()) return nullptr;
    const auto v = s->second.find(key);
    return v == s->second.end() ? nullptr : &v->second;
}

const std::string& required(const Ini& ini, const std::string& section, const std::string& key) {
    const auto* value = find(ini, section, key);
    if (!value) throw std::runtime_error("missing [" + section + "] " + key);
    return *value;
}

int number(const std::string& value, int limit, const std::string& context) {
    if (value.empty() || value.size() > 10) throw std::runtime_error("invalid " + context);
    unsigned int n = 0;
    for (char c : value) {
        if (c < '0' || c > '9' || n > (static_cast<unsigned int>(limit) - (c - '0')) / 10)
            throw std::runtime_error("invalid " + context);
        n = n * 10 + (c - '0');
    }
    return static_cast<int>(n);
}

template <std::size_t N>
bool contains(const std::string& tag, const char* const (&tags)[N]) {
    for (const char* candidate : tags)
        if (tag == candidate) return true;
    return false;
}

bool validTarget(const std::string& tag) {
    return contains(tag, kParameters) || contains(tag, kMasters) || contains(tag, kMisc);
}

bool validGamepadControl(const std::string& tag) { return contains(tag, kGamepadControls); }

// The name tables above follow the enum order: axes first, then buttons.
int axisIndex(const std::string& tag) {
    for (int i = 0; i < GP_AXIS_COUNT; ++i)
        if (tag == kGamepadControls[i]) return i;
    return -1;
}
int buttonIndex(const std::string& tag) {
    for (int i = 0; i < GP_BUTTON_COUNT; ++i)
        if (tag == kGamepadControls[GP_AXIS_COUNT + i]) return i;
    return -1;
}

// IniFile::WriteBinary emits the LOW nibble first, then the high nibble,
// using 'A'..'P' for each nibble (not hexadecimal).
unsigned char byteAt(const std::string& encoded, std::size_t offset) {
    const char lo = encoded[offset], hi = encoded[offset + 1];
    if (lo < 'A' || lo > 'P' || hi < 'A' || hi > 'P')
        throw std::runtime_error("invalid A-P binary encoding");
    return static_cast<unsigned char>((lo - 'A') | ((hi - 'A') << 4));
}

void appendByte(std::string& encoded, unsigned char byte) {
    encoded += static_cast<char>('A' + (byte & 15));
    encoded += static_cast<char>('A' + (byte >> 4));
}

// UTF-8 paths are the public API's strings; Path# is a sequence of UTF-16LE
// code units WITHOUT a BOM or terminator. Reject overlong UTF-8 and unpaired
// surrogates rather than silently changing a filename on round trip.
std::uint32_t nextUtf8(const std::string& text, std::size_t& pos) {
    const unsigned char lead = static_cast<unsigned char>(text[pos++]);
    if (lead < 0x80) return lead;
    int following;
    std::uint32_t cp;
    if (lead >= 0xc2 && lead <= 0xdf) { following = 1; cp = lead & 0x1f; }
    else if (lead >= 0xe0 && lead <= 0xef) { following = 2; cp = lead & 0x0f; }
    else if (lead >= 0xf0 && lead <= 0xf4) { following = 3; cp = lead & 0x07; }
    else throw std::runtime_error("invalid UTF-8 path");
    if (text.size() - pos < static_cast<std::size_t>(following))
        throw std::runtime_error("truncated UTF-8 path");
    for (int i = 0; i < following; ++i) {
        const unsigned char c = static_cast<unsigned char>(text[pos++]);
        if ((c & 0xc0) != 0x80) throw std::runtime_error("invalid UTF-8 path");
        cp = (cp << 6) | (c & 0x3f);
    }
    if ((following == 1 && cp < 0x80) || (following == 2 && cp < 0x800) ||
        (following == 3 && cp < 0x10000) || cp > 0x10ffff ||
        (cp >= 0xd800 && cp <= 0xdfff))
        throw std::runtime_error("invalid UTF-8 path");
    return cp;
}

void appendUtf8(std::string& result, std::uint32_t cp) {
    if (cp < 0x80) result += static_cast<char>(cp);
    else if (cp < 0x800) {
        result += static_cast<char>(0xc0 | (cp >> 6));
        result += static_cast<char>(0x80 | (cp & 63));
    } else if (cp < 0x10000) {
        result += static_cast<char>(0xe0 | (cp >> 12));
        result += static_cast<char>(0x80 | ((cp >> 6) & 63));
        result += static_cast<char>(0x80 | (cp & 63));
    } else {
        result += static_cast<char>(0xf0 | (cp >> 18));
        result += static_cast<char>(0x80 | ((cp >> 12) & 63));
        result += static_cast<char>(0x80 | ((cp >> 6) & 63));
        result += static_cast<char>(0x80 | (cp & 63));
    }
}

void checkPath(const std::string& path) {
    if (path.empty() || path.size() > kMaxPath) throw std::runtime_error("invalid patch path length");
    for (std::size_t pos = 0; pos < path.size();) {
        const auto cp = nextUtf8(path, pos);
        if (cp < 0x20 || cp == 0x7f) throw std::runtime_error("control character in patch path");
    }
}

std::string decodePath(const std::string& encoded) {
    if (encoded.size() % 4 || encoded.size() > kMaxPath * 4)
        throw std::runtime_error("invalid Path# length");
    std::string result;
    for (std::size_t i = 0; i < encoded.size();) {
        std::uint32_t cp = byteAt(encoded, i) | (std::uint32_t(byteAt(encoded, i + 2)) << 8);
        i += 4;
        if (cp >= 0xd800 && cp <= 0xdbff) {
            if (encoded.size() - i < 4) throw std::runtime_error("unpaired UTF-16 surrogate");
            const std::uint32_t low = byteAt(encoded, i) | (std::uint32_t(byteAt(encoded, i + 2)) << 8);
            if (low < 0xdc00 || low > 0xdfff) throw std::runtime_error("unpaired UTF-16 surrogate");
            i += 4;
            cp = 0x10000 + ((cp - 0xd800) << 10) + (low - 0xdc00);
        } else if (cp >= 0xdc00 && cp <= 0xdfff) {
            throw std::runtime_error("unpaired UTF-16 surrogate");
        }
        appendUtf8(result, cp);
    }
    checkPath(result);
    return result;
}

std::string encodePath(const std::string& path, bool& binary) {
    checkPath(path);
    binary = false;
    for (unsigned char c : path) {
        if (c < 0x20 || c > 0x7e) { binary = true; break; }
    }
    if (!binary) return path;
    std::string encoded;
    for (std::size_t pos = 0; pos < path.size();) {
        std::uint32_t cp = nextUtf8(path, pos);
        if (cp >= 0x10000) {
            cp -= 0x10000;
            const std::uint16_t high = static_cast<std::uint16_t>(0xd800 | (cp >> 10));
            appendByte(encoded, static_cast<unsigned char>(high));
            appendByte(encoded, static_cast<unsigned char>(high >> 8));
            cp = 0xdc00 | (cp & 1023);
        }
        appendByte(encoded, static_cast<unsigned char>(cp));
        appendByte(encoded, static_cast<unsigned char>(cp >> 8));
    }
    return encoded;
}

// Whorld's template Get/Put<double> copies sizeof(double) native bytes; V2
// files are produced on little-endian Windows (IEEE-754 binary64).
static_assert(sizeof(double) == 8 && std::numeric_limits<double>::is_iec559,
              "V2 playlists require IEEE-754 binary64 doubles");
// The V2 format stores a range endpoint as the native bytes of a double (16
// bytes, usually with embedded NULs). This port also accepts a plain decimal so
// that mappings can be hand-edited (the README shows Start=25 / End=80).
double decodeRange(const std::string& text) {
    if (!text.empty() && text.find_first_not_of(" \t0123456789+-.eE") == std::string::npos) {
        std::size_t used = 0;
        double value = 0;
        try { value = std::stod(text, &used); } catch (const std::exception&) { used = 0; }
        if (used == text.size() && std::isfinite(value)) return value;
    }
    if (text.size() != 16) throw std::runtime_error("invalid mapping range length");
    std::uint64_t bits = 0;
    for (unsigned i = 0; i < 8; ++i) bits |= std::uint64_t(byteAt(text, i * 2)) << (i * 8);
    double value;
    std::memcpy(&value, &bits, 8);
    if (!std::isfinite(value)) throw std::runtime_error("non-finite mapping range");
    return value;
}

std::string encodeDouble(double value) {
    if (!std::isfinite(value)) throw std::runtime_error("non-finite mapping range");
    std::uint64_t bits;
    std::memcpy(&bits, &value, 8);
    std::string encoded;
    for (unsigned i = 0; i < 8; ++i)
        appendByte(encoded, static_cast<unsigned char>(bits >> (i * 8)));
    return encoded;
}

Ini parse(const std::string& data) {
    Ini ini;
    std::string section;
    std::size_t entries = 0, pos = 0;
    if (data.compare(0, 3, "\xef\xbb\xbf") == 0) pos = 3; // optional UTF-8 BOM
    while (pos < data.size()) {
        const auto end = data.find('\n', pos);
        const auto length = (end == std::string::npos ? data.size() : end) - pos;
        if (length > kMaxLine) throw std::runtime_error("INI line too long");
        std::string line = data.substr(pos, length);
        pos = end == std::string::npos ? data.size() : end + 1;
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.find('\0') != std::string::npos || line.find('\r') != std::string::npos)
            throw std::runtime_error("invalid INI line");
        if (line.empty() || line[0] == ';') continue;
        if (line[0] == '[') {
            if (line.size() < 3 || line.back() != ']' || ++entries > kMaxEntries)
                throw std::runtime_error("invalid INI section");
            section = line.substr(1, line.size() - 2);
            if (!ini.emplace(section, Section{}).second)
                throw std::runtime_error("duplicate INI section: " + section);
        } else {
            const auto equal = line.find('=');
            if (section.empty() || equal == 0 || equal == std::string::npos || ++entries > kMaxEntries)
                throw std::runtime_error("invalid INI entry");
            const std::string key = line.substr(0, equal);
            if (!ini.at(section).emplace(key, line.substr(equal + 1)).second)
                throw std::runtime_error("duplicate INI key: " + key);
        }
    }
    return ini;
}

void append(std::string& text, const std::string& line) {
    if (line.size() > kMaxLine || text.size() > kMaxFile - line.size())
        throw std::runtime_error("playlist exceeds size limit");
    text += line;
}

void validateMapping(const Mapping& m) {
    if (!contains(m.event, kEvents) || m.channel < 0 || m.channel > 15 ||
        m.control < 0 || m.control > 127 || !validTarget(m.target) ||
        (contains(m.target, kParameters) && !contains(m.property, kProperties)) ||
        !std::isfinite(m.start) || !std::isfinite(m.end))
        throw std::runtime_error("invalid MIDI mapping");
}

void validateGamepadMapping(const GamepadMapping& m) {
    if (!validGamepadControl(m.control) || !validTarget(m.target) ||
        (contains(m.target, kParameters) && !contains(m.property, kProperties)) ||
        !std::isfinite(m.start) || !std::isfinite(m.end))
        throw std::runtime_error("invalid gamepad mapping");
}

void validateAudioMapping(const AudioMapping& m) {
    if (!contains(m.feature, kAudioFeatures) || !validTarget(m.target) ||
        (contains(m.target, kParameters) && !contains(m.property, kProperties)) ||
        !std::isfinite(m.start) || !std::isfinite(m.end))
        throw std::runtime_error("invalid audio mapping");
}

} // namespace

bool Playlist::load(const std::string& filename, std::string& error) {
    try {
        std::ifstream file(filename, std::ios::binary | std::ios::ate);
        if (!file) throw std::runtime_error("cannot open playlist");
        const auto size = file.tellg();
        if (size < 0 || size > static_cast<std::streamoff>(kMaxFile))
            throw std::runtime_error("playlist exceeds size limit");
        std::string data(static_cast<std::size_t>(size), '\0');
        file.seekg(0);
        if (!file.read(&data[0], static_cast<std::streamsize>(data.size())) && !data.empty())
            throw std::runtime_error("cannot read playlist");
        const Ini ini = parse(data);
        if (required(ini, "Settings", "FileID") != "WhorldPlay" ||
            required(ini, "Settings", "FileVersion") != "1")
            throw std::runtime_error("unsupported playlist format or version");

        Playlist parsed; // commit only after every section has passed validation
        const int patchCount = number(required(ini, "Patch", "Count"), kMaxItems, "patch count");
        parsed.patches.reserve(static_cast<std::size_t>(patchCount));
        for (int i = 0; i < patchCount; ++i) {
            const std::string section = "Patch\\" + std::to_string(i);
            const auto* plain = find(ini, section, "Path");
            const auto* encoded = find(ini, section, "Path#");
            if (plain == encoded || (plain && encoded))
                throw std::runtime_error("missing or ambiguous [" + section + "] Path");
            if (plain) {
                checkPath(*plain);
                parsed.patches.push_back(*plain);
            } else parsed.patches.push_back(decodePath(*encoded));
        }
        const auto* mappingCountText = find(ini, "Mapping", "Count");
        const int mappingCount = mappingCountText ? number(*mappingCountText, kMaxItems, "mapping count") : 0;
        parsed.mappings.reserve(static_cast<std::size_t>(mappingCount));
        for (int i = 0; i < mappingCount; ++i) {
            const std::string section = "Mapping\\" + std::to_string(i);
            Mapping m;
            m.event = required(ini, section, "Event");
            m.target = required(ini, section, "Target");
            if (const auto* v = find(ini, section, "Channel")) m.channel = number(*v, 15, "MIDI channel");
            if (const auto* v = find(ini, section, "Control")) m.control = number(*v, 127, "MIDI control");
            if (contains(m.target, kParameters)) m.property = required(ini, section, "Prop");
            // For master/misc targets upstream ignores Prop and defaults to Val.
            if (const auto* v = find(ini, section, "Start")) m.start = decodeRange(*v);
            if (const auto* v = find(ini, section, "End")) m.end = decodeRange(*v);
            validateMapping(m);
            parsed.mappings.push_back(std::move(m));
        }
        // Gamepad and audio mappings are a Whorld/Linux extension: the sections
        // are optional, and upstream readers simply ignore them.
        const auto* gamepadCountText = find(ini, "Gamepad", "Count");
        const int gamepadCount = gamepadCountText ? number(*gamepadCountText, kMaxItems, "gamepad count") : 0;
        parsed.gamepadMappings.reserve(static_cast<std::size_t>(gamepadCount));
        for (int i = 0; i < gamepadCount; ++i) {
            const std::string section = "Gamepad\\" + std::to_string(i);
            GamepadMapping m;
            m.control = required(ini, section, "Control");
            m.target = required(ini, section, "Target");
            if (contains(m.target, kParameters)) m.property = required(ini, section, "Prop");
            if (const auto* v = find(ini, section, "Start")) m.start = decodeRange(*v);
            if (const auto* v = find(ini, section, "End")) m.end = decodeRange(*v);
            validateGamepadMapping(m);
            parsed.gamepadMappings.push_back(std::move(m));
        }
        const auto* audioCountText = find(ini, "Audio", "Count");
        const int audioCount = audioCountText ? number(*audioCountText, kMaxItems, "audio count") : 0;
        parsed.audioMappings.reserve(static_cast<std::size_t>(audioCount));
        for (int i = 0; i < audioCount; ++i) {
            const std::string section = "Audio\\" + std::to_string(i);
            AudioMapping m;
            m.feature = required(ini, section, "Feature");
            m.target = required(ini, section, "Target");
            if (contains(m.target, kParameters)) m.property = required(ini, section, "Prop");
            if (const auto* v = find(ini, section, "Start")) m.start = decodeRange(*v);
            if (const auto* v = find(ini, section, "End")) m.end = decodeRange(*v);
            validateAudioMapping(m);
            parsed.audioMappings.push_back(std::move(m));
        }
        parsed.path = filename;
        *this = std::move(parsed);
        error.clear();
        return true;
    } catch (const std::exception& e) {
        error = e.what();
        return false;
    }
}

bool Playlist::save(const std::string& filename, std::string& error) {
    try {
        if (patches.size() > kMaxItems || mappings.size() > kMaxItems ||
            gamepadMappings.size() > kMaxItems || audioMappings.size() > kMaxItems)
            throw std::runtime_error("too many playlist items");
        std::string text;
        append(text, "[Settings]\nFileID=WhorldPlay\nFileVersion=1\n");
        append(text, "[Patch]\nCount=" + std::to_string(patches.size()) + "\n");
        for (std::size_t i = 0; i < patches.size(); ++i) {
            bool binary;
            const auto value = encodePath(patches[i], binary);
            append(text, "[Patch\\" + std::to_string(i) + "]\n" +
                         (binary ? "Path#=" : "Path=") + value + "\n");
        }
        append(text, "[Mapping]\nCount=" + std::to_string(mappings.size()) + "\n");
        for (std::size_t i = 0; i < mappings.size(); ++i) {
            const auto& m = mappings[i];
            validateMapping(m);
            append(text, "[Mapping\\" + std::to_string(i) + "]\nEvent=" + m.event +
                         "\nTarget=" + m.target + "\n");
            if (contains(m.target, kParameters)) append(text, "Prop=" + m.property + "\n");
            append(text, "Channel=" + std::to_string(m.channel) + "\nControl=" +
                         std::to_string(m.control) + "\n");
            // V2 omits optional defaults; all other doubles are binary A-P.
            if (m.start != 0) append(text, "Start=" + encodeDouble(m.start) + "\n");
            if (m.end != 100) append(text, "End=" + encodeDouble(m.end) + "\n");
        }
        if (!gamepadMappings.empty()) {
            append(text, "[Gamepad]\nCount=" + std::to_string(gamepadMappings.size()) + "\n");
            for (std::size_t i = 0; i < gamepadMappings.size(); ++i) {
                const auto& m = gamepadMappings[i];
                validateGamepadMapping(m);
                append(text, "[Gamepad\\" + std::to_string(i) + "]\nControl=" + m.control +
                             "\nTarget=" + m.target + "\n");
                if (contains(m.target, kParameters)) append(text, "Prop=" + m.property + "\n");
                if (m.start != 0) append(text, "Start=" + encodeDouble(m.start) + "\n");
                if (m.end != 100) append(text, "End=" + encodeDouble(m.end) + "\n");
            }
        }
        if (!audioMappings.empty()) {
            append(text, "[Audio]\nCount=" + std::to_string(audioMappings.size()) + "\n");
            for (std::size_t i = 0; i < audioMappings.size(); ++i) {
                const auto& m = audioMappings[i];
                validateAudioMapping(m);
                append(text, "[Audio\\" + std::to_string(i) + "]\nFeature=" + m.feature +
                             "\nTarget=" + m.target + "\n");
                if (contains(m.target, kParameters)) append(text, "Prop=" + m.property + "\n");
                if (m.start != 0) append(text, "Start=" + encodeDouble(m.start) + "\n");
                if (m.end != 100) append(text, "End=" + encodeDouble(m.end) + "\n");
            }
        }
        std::ofstream file(filename, std::ios::binary | std::ios::trunc);
        if (!file || !file.write(text.data(), static_cast<std::streamsize>(text.size())) ||
            !file.flush())
            throw std::runtime_error("cannot write playlist");
        path = filename;
        error.clear();
        return true;
    } catch (const std::exception& e) {
        error = e.what();
        return false;
    }
}

std::string Playlist::resolvePath(std::size_t index) const {
    if (index >= patches.size()) return {};
    const std::string& name = patches[index];
    if (name.empty()) return {};
    // std::filesystem on Linux doesn't recognize drive letters or UNC shares.
    // Such paths must not accidentally become relative to the playlist.
    if ((name.size() >= 3 && ((name[0] >= 'A' && name[0] <= 'Z') ||
                              (name[0] >= 'a' && name[0] <= 'z')) &&
         name[1] == ':' && (name[2] == '\\' || name[2] == '/')) ||
        (name.size() >= 2 && name[0] == '\\' && name[1] == '\\') ||
        name[0] == '/')
        return name;
    std::string relative = name;
    std::replace(relative.begin(), relative.end(), '\\', '/');
    try {
        return (fs::path(path).parent_path() / fs::path(relative)).lexically_normal().string();
    } catch (const std::exception&) {
        return name;
    }
}

double applyRange(const Mapping& mapping, double normalized) {
    const double start = mapping.start / 100.0, end = mapping.end / 100.0;
    // No universal clamp: inverted/out-of-range percentages are legitimate.
    return normalized * (end - start) + start;
}

std::vector<MappedAction> mapMidiNormalized(const Playlist& playlist, MidiEvent event,
                                            int channel, int control, double normalized) {
    std::vector<MappedAction> actions;
    const auto index = static_cast<std::size_t>(event);
    if (index >= std::size(kEvents) || channel < 0 || channel > 15) return actions;
    // Whorld's IsInputMatch tests P1 for NOTE_OFF through CONTROL only;
    // PATCH, CHAN_AFT and WHEEL take their value from P1 (or both bytes).
    const bool hasControl = index <= static_cast<std::size_t>(MidiEvent::Control);
    for (const auto& m : playlist.mappings) {
        if (m.event != kEvents[index] || m.channel != channel ||
            (hasControl && m.control != control) ||
            !validTarget(m.target) || !std::isfinite(m.start) || !std::isfinite(m.end)) continue;
        actions.push_back({m.target, m.property, applyRange(m, normalized)});
    }
    return actions;
}

std::vector<MappedAction> mapMidi(const Playlist& playlist, const MidiMessage& message) {
    const auto event = static_cast<std::size_t>(message.event);
    // A Control message carries a 14-bit value when it came from an MSB/LSB pair
    // (the 14-bit input mode); WHEEL is always 14-bit. Everything else is 7-bit.
    const bool fourteenBit = message.event == MidiEvent::Wheel || message.event == MidiEvent::Control;
    if (event >= std::size(kEvents) || message.channel < 0 || message.channel > 15 ||
        message.value < 0 || message.value > (fourteenBit ? 16383 : 127) ||
        message.control < 0 || message.control > 127)
        return {};
    double normalized;
    if (fourteenBit && message.value > 127)
        // Same dual-range shape as the 7-bit formula so both endpoints map to
        // exactly 0 and 1 despite the asymmetric 0..16383 range.
        normalized = message.value <= 8192 ? message.value / 16384.0 : (message.value - 1) / 16382.0;
    else if (message.event == MidiEvent::Wheel)
        normalized = message.value <= 8192 ? message.value / 16384.0 : (message.value - 1) / 16382.0;
    else
        normalized = message.value <= 64 ? message.value / 128.0 : (message.value - 1) / 126.0;
    return mapMidiNormalized(playlist, message.event, message.channel, message.control, normalized);
}

std::string midiEventName(MidiEvent event) {
    const auto index = static_cast<std::size_t>(event);
    return index < std::size(kEvents) ? kEvents[index] : std::string();
}

bool momentaryTarget(const std::string& target) {
    return target == "Clear" || target == "RandomOrigin" || target == "RandomPhase" ||
           target == "Pause" || target == "Patch" || target == "NextShape" ||
           target == "PrevShape";
}

const char* gamepadControlName(int index) {
    return index >= 0 && index < GP_AXIS_COUNT + GP_BUTTON_COUNT ? kGamepadControls[index] : "";
}

// Binds (or rebinds) the gamepad control that produced a learn event to a
// target/property. One control drives one target, so any previous mapping for
// the same control is replaced (matching learnMidi and the Windows app).
void learnGamepad(Playlist& playlist, const std::string& control, const std::string& target,
                  const std::string& property) {
    playlist.gamepadMappings.erase(std::remove_if(playlist.gamepadMappings.begin(),
        playlist.gamepadMappings.end(),
        [&](const GamepadMapping& m) { return m.control == control; }),
        playlist.gamepadMappings.end());
    GamepadMapping m;
    m.control = control;
    m.target = target;
    m.property = property;
    m.start = 0;
    m.end = 100;
    playlist.gamepadMappings.push_back(std::move(m));
}

std::vector<MappedAction> mapGamepad(const Playlist& playlist, const GamepadState& current,
                                     const GamepadState& previous) {
    std::vector<MappedAction> actions;
    for (const auto& m : playlist.gamepadMappings) {
        if (!validTarget(m.target) || !std::isfinite(m.start) || !std::isfinite(m.end)) continue;
        double normalized;
        const int axis = axisIndex(m.control);
        if (axis >= 0) {
            // Sticks are bipolar and centered on zero; triggers already run 0..1.
            normalized = axis <= GP_RY ? (current.axis[axis] + 1) / 2 : current.axis[axis];
        } else {
            const int button = buttonIndex(m.control);
            if (button < 0) continue;
            const bool now = current.button[button], before = previous.button[button];
            if (now == before) continue;                      // only edges change anything
            if (momentaryTarget(m.target) && !now) continue;  // fire once, on press
            normalized = now ? 1.0 : 0.0;
        }
        const double start = m.start / 100.0, end = m.end / 100.0;
        actions.push_back({m.target, m.property, normalized * (end - start) + start});
    }
    return actions;
}

std::vector<MappedAction> mapAudio(const Playlist& playlist, const AudioFeatures& features) {
    std::vector<MappedAction> actions;
    for (const auto& m : playlist.audioMappings) {
        if (!validTarget(m.target) || !std::isfinite(m.start) || !std::isfinite(m.end)) continue;
        double feature = 0;
        bool beat = false;
        if (m.feature == "Level") feature = features.level;
        else if (m.feature == "Low") feature = features.low;
        else if (m.feature == "Mid") feature = features.mid;
        else if (m.feature == "High") feature = features.high;
        else if (m.feature == "Beat") { feature = features.beat; beat = true; }
        else continue;
        // A beat pulse lasts several frames, so a momentary target (Clear,
        // Patch, NextShape, ...) must fire once per onset rather than on every
        // frame of the decay.
        if (beat && momentaryTarget(m.target) && !features.onset) continue;
        const double start = m.start / 100.0, end = m.end / 100.0;
        actions.push_back({m.target, m.property, std::clamp(feature, 0.0, 1.0) * (end - start) + start});
    }
    return actions;
}

const std::vector<AudioMapping>& defaultAudioMappings() {
    // At silence every feature is 0, which lands on Start, so the mappings keep
    // a quiet signal near rest: no extra ring growth, mid zoom and no spin.
    // "Beat" is deliberately not part of the defaults: adding an onset mapping
    // would change the out-of-the-box output, so it stays opt-in via [Audio].
    static const std::vector<AudioMapping> defaults = {
        {"Level", "RingGrowth", "Val", 0, 100},
        {"Low", "Zoom", "Val", 25, 80},
        {"High", "RotateSpeed", "Val", 50, 100},
    };
    return defaults;
}

} // namespace whorld
