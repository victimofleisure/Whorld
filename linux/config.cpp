// Show-config parser and ShowConfig (de)serialization (GPL-3.0-or-later).
#include "config.h"
#include "projection.h"

#include <cmath>
#include <cctype>
#include <cstdio>
#include <fstream>
#include <sstream>
#include <stdexcept>
#include <utility>

namespace whorld {
namespace {

// Limits apply before allocation, so an untrusted config cannot grow the map
// without bound; they are generous for a hand-written show file.
constexpr std::size_t kMaxConfigBytes = 1024 * 1024;
constexpr std::size_t kMaxConfigLine = 8192;
constexpr std::size_t kMaxConfigEntries = 4096;

std::string trim(const std::string& text) {
    const auto begin = text.find_first_not_of(" \t\r");
    if (begin == std::string::npos) return {};
    const auto end = text.find_last_not_of(" \t\r");
    return text.substr(begin, end - begin + 1);
}

std::string lowered(std::string text) {
    for (char& c : text)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return text;
}

bool parseDouble(const std::string& text, double& value) {
    if (text.empty()) return false;
    std::size_t used = 0;
    try {
        value = std::stod(text, &used);
    } catch (const std::exception&) {
        return false;
    }
    return used == text.size() && std::isfinite(value);
}

bool parseInt(const std::string& text, long& value) {
    if (text.empty()) return false;
    std::size_t used = 0;
    try {
        value = std::stol(text, &used);
    } catch (const std::exception&) {
        return false;
    }
    return used == text.size();
}

// Shortest decimal that still round-trips the exact double, so ordinary values
// stay readable (0.75) while awkward ones keep every bit.
std::string formatDouble(double value) {
    char buffer[64];
    std::snprintf(buffer, sizeof buffer, "%.15g", value);
    double back = 0;
    if (!parseDouble(buffer, back) || back != value)
        std::snprintf(buffer, sizeof buffer, "%.17g", value);
    return buffer;
}

// warpToString emits one "x y" pair per line (good for --warp-save files), but
// a config value lives on a single `key = value` line; fold the newlines/tabs
// into spaces so the corners stay on one line.
std::string singleLine(std::string text) {
    for (char& c : text)
        if (c == '\n' || c == '\r' || c == '\t') c = ' ';
    return trim(text);
}

} // namespace

bool Config::load(const std::string& path, std::string& error) {
    std::ifstream file(path, std::ios::binary | std::ios::ate);
    if (!file) { error = "cannot open config file: " + path; return false; }
    const auto size = file.tellg();
    if (size < 0 || size > static_cast<std::streamoff>(kMaxConfigBytes)) {
        error = "config file is too large: " + path;
        return false;
    }
    std::string data(static_cast<std::size_t>(size), '\0');
    file.seekg(0);
    if (!file.read(&data[0], static_cast<std::streamsize>(data.size())) && !data.empty()) {
        error = "cannot read config file: " + path;
        return false;
    }

    std::map<std::string, std::string> parsed;
    std::istringstream stream(data);
    std::string line;
    bool first = true;
    int lineNumber = 0;
    while (std::getline(stream, line)) {
        ++lineNumber;
        if (first) {
            first = false;
            if (line.compare(0, 3, "\xef\xbb\xbf") == 0) line.erase(0, 3); // optional BOM
        }
        if (line.size() > kMaxConfigLine) { error = "config line too long"; return false; }
        const std::string text = trim(line);
        if (text.empty() || text[0] == '#' || text[0] == ';') continue;
        const auto equal = text.find('=');
        if (equal == std::string::npos) {
            error = "config line " + std::to_string(lineNumber) + " has no '='";
            return false;
        }
        const std::string key = trim(text.substr(0, equal));
        const std::string value = trim(text.substr(equal + 1));
        if (key.empty()) {
            error = "config line " + std::to_string(lineNumber) + " has an empty key";
            return false;
        }
        if (!parsed.emplace(key, value).second) {
            error = "config line " + std::to_string(lineNumber) + " repeats key " + key;
            return false;
        }
        if (parsed.size() > kMaxConfigEntries) { error = "config has too many keys"; return false; }
    }
    entries_ = std::move(parsed);
    error.clear();
    return true;
}

bool Config::save(const std::string& path, std::string& error) const {
    std::ofstream file(path, std::ios::binary | std::ios::trunc);
    if (!file) { error = "cannot write config file: " + path; return false; }
    for (const auto& entry : entries_)
        file << entry.first << " = " << entry.second << '\n';
    file.flush();
    if (!file) { error = "cannot write config file: " + path; return false; }
    error.clear();
    return true;
}

bool Config::has(const std::string& key) const {
    return entries_.find(key) != entries_.end();
}

std::string Config::getString(const std::string& key, const std::string& fallback) const {
    const auto it = entries_.find(key);
    return it == entries_.end() ? fallback : it->second;
}

bool Config::getBool(const std::string& key, bool fallback) const {
    const auto it = entries_.find(key);
    if (it == entries_.end()) return fallback;
    const std::string value = lowered(it->second);
    if (value == "1" || value == "true" || value == "yes" || value == "on") return true;
    if (value == "0" || value == "false" || value == "no" || value == "off") return false;
    return fallback;
}

double Config::getDouble(const std::string& key, double fallback) const {
    const auto it = entries_.find(key);
    double value = 0;
    if (it == entries_.end() || !parseDouble(it->second, value)) return fallback;
    return value;
}

int Config::getInt(const std::string& key, int fallback) const {
    const auto it = entries_.find(key);
    long value = 0;
    if (it == entries_.end() || !parseInt(it->second, value)) return fallback;
    return static_cast<int>(value);
}

void Config::set(const std::string& key, const std::string& value) {
    entries_[key] = value;
}

bool parseMirrorMode(const std::string& value, int& mode) {
    if (value == "off" || value == "0") mode = 0;
    else if (value == "quad" || value == "1") mode = 1;
    else if (value == "kaleidoscope" || value == "2") mode = 2;
    else if (value == "radial" || value == "3") mode = 3;
    else return false;
    return true;
}

bool parseBlendMode(const std::string& value, int& mode) {
    if (value == "normal" || value == "0") mode = 0;
    else if (value == "add" || value == "additive" || value == "1") mode = 1;
    else if (value == "screen" || value == "2") mode = 2;
    else return false;
    return true;
}

bool parseMidiModeInput(const std::string& value, MidiInputMode& mode) {
    if (value == "absolute") mode = MidiInputMode::Absolute;
    else if (value == "14bit") mode = MidiInputMode::FourteenBit;
    else if (value == "rel-twos") mode = MidiInputMode::RelativeTwosComplement;
    else if (value == "rel-sign") mode = MidiInputMode::RelativeSignMagnitude;
    else if (value == "rel-binary") mode = MidiInputMode::RelativeBinaryOffset;
    else return false;
    return true;
}

const char* mirrorModeName(int mode) {
    switch (mode) {
    case 1: return "quad";
    case 2: return "kaleidoscope";
    case 3: return "radial";
    default: return "off";
    }
}

const char* blendModeName(int mode) {
    switch (mode) {
    case 1: return "add";
    case 2: return "screen";
    default: return "normal";
    }
}

const char* midiModeName(MidiInputMode mode) {
    switch (mode) {
    case MidiInputMode::FourteenBit: return "14bit";
    case MidiInputMode::RelativeTwosComplement: return "rel-twos";
    case MidiInputMode::RelativeSignMagnitude: return "rel-sign";
    case MidiInputMode::RelativeBinaryOffset: return "rel-binary";
    default: return "absolute";
    }
}

bool loadShowConfig(const std::string& path, ShowConfig& show, std::string& error) {
    Config config;
    if (!config.load(path, error)) return false;

    ShowConfig parsed;
    parsed.unknown = config; // preserved so a later save re-emits unknown keys
    parsed.patch = config.getString("patch", parsed.patch);
    parsed.playlist = config.getString("playlist", parsed.playlist);
    parsed.renderer = config.getString("renderer", parsed.renderer);
    if (!parsed.renderer.empty() && parsed.renderer != "auto" &&
        parsed.renderer != "gpu" && parsed.renderer != "cairo") {
        error = "config renderer must be auto, gpu or cairo";
        return false;
    }
    if (parsed.renderer.empty()) parsed.renderer = "auto";
    parsed.scale = config.getDouble("scale", parsed.scale);
    parsed.fps = config.getInt("fps", parsed.fps);
    if (parsed.fps < 1 || parsed.fps > 200) {
        error = "config fps must be between 1 and 200";
        return false;
    }
    parsed.wheelSense = config.getDouble("wheelSense", parsed.wheelSense);
    if (parsed.wheelSense < 0.1 || parsed.wheelSense > 10.0) {
        error = "config wheelSense must be between 0.1 and 10 (percent per notch)";
        return false;
    }
    if (parsed.scale != 0 && (parsed.scale < 0.25 || parsed.scale > 2.0)) {
        error = "config scale must be 0 (adaptive) or between 0.25 and 2.0";
        return false;
    }
    if (config.has("mirror") && !config.getString("mirror").empty() &&
        !parseMirrorMode(config.getString("mirror"), parsed.mirror)) {
        error = "config mirror must be off, quad, kaleidoscope or radial";
        return false;
    }
    parsed.invert = config.getBool("invert", parsed.invert);
    parsed.feedback = config.getDouble("feedback", parsed.feedback);
    if (parsed.feedback < 0.0 || parsed.feedback > 0.999) {
        error = "config feedback must be between 0.0 and 0.99";
        return false;
    }
    if (config.has("blend") && !config.getString("blend").empty() &&
        !parseBlendMode(config.getString("blend"), parsed.blend)) {
        error = "config blend must be normal, add or screen";
        return false;
    }
    parsed.morph = config.getDouble("morph", parsed.morph);
    if (!(parsed.morph >= 0) || parsed.morph > 3600) {
        error = "config morph must be between 0 and 3600 seconds";
        return false;
    }
    parsed.warp = config.getString("warp", parsed.warp);
    if (!parsed.warp.empty()) {
        WarpQuad quad;
        if (!warpFromString(parsed.warp, quad)) {
            error = "config warp expects \"TLx,TLy,TRx,TRy,BRx,BRy,BLx,BLy\"";
            return false;
        }
        if (!validWarp(quad)) { error = "config warp quad is degenerate"; return false; }
        parsed.warp = singleLine(warpToString(quad)); // canonical, one line
    }
    parsed.warpFile = config.getString("warpFile", parsed.warpFile);
    parsed.videoDriver = config.getString("videoDriver", parsed.videoDriver);
    parsed.fullscreen = config.getBool("fullscreen", parsed.fullscreen);
    parsed.hideCursor = config.getBool("hideCursor", parsed.hideCursor);
    parsed.panelSide = config.getString("panelSide", parsed.panelSide);
    if (config.has("panelSide") && parsed.panelSide != "left" && parsed.panelSide != "right") {
        error = "config panelSide must be left or right";
        return false;
    }
    parsed.midiSource = config.getString("midiSource", parsed.midiSource);
    if (config.has("midiMode") && !config.getString("midiMode").empty() &&
        !parseMidiModeInput(config.getString("midiMode"), parsed.midiMode)) {
        error = "config midiMode must be absolute, 14bit, rel-twos, rel-sign or rel-binary";
        return false;
    }
    parsed.midiTakeover = config.getBool("midiTakeover", parsed.midiTakeover);
    parsed.midiOut = config.getString("midiOut", parsed.midiOut);
    parsed.midiOutEnabled = config.getBool("midiOutEnabled", parsed.midiOutEnabled);
    parsed.midiFeedback = config.getBool("midiFeedback", parsed.midiFeedback);
    parsed.midiClock = config.getBool("midiClock", parsed.midiClock);
    parsed.midiClockOut = config.getBool("midiClockOut", parsed.midiClockOut);
    parsed.osc = config.getBool("osc", parsed.osc);
    parsed.oscPort = config.getInt("oscPort", parsed.oscPort);
    if (parsed.oscPort < 1 || parsed.oscPort > 65535) {
        error = "config oscPort must be between 1 and 65535";
        return false;
    }
    parsed.oscDirect = config.getBool("oscDirect", parsed.oscDirect);
    parsed.oscDirectPort = config.getInt("oscDirectPort", parsed.oscDirectPort);
    if (parsed.oscDirectPort < 1 || parsed.oscDirectPort > 65535) {
        error = "config oscDirectPort must be between 1 and 65535";
        return false;
    }
    parsed.audio = config.getBool("audio", parsed.audio);
    parsed.audioDevice = config.getString("audioDevice", parsed.audioDevice);
    parsed.audioGain = config.getDouble("audioGain", parsed.audioGain);
    if (parsed.audioGain <= 0 || parsed.audioGain > 100) {
        error = "config audioGain must be between 0 and 100";
        return false;
    }

    show = std::move(parsed);
    error.clear();
    return true;
}

bool saveShowConfig(const std::string& path, const ShowConfig& show, std::string& error) {
    Config config = show.unknown; // keep unrecognised keys from the source file
    config.set("patch", show.patch);
    config.set("playlist", show.playlist);
    config.set("renderer", show.renderer);
    config.set("scale", formatDouble(show.scale));
    config.set("fps", std::to_string(show.fps));
    config.set("wheelSense", formatDouble(show.wheelSense));
    config.set("mirror", mirrorModeName(show.mirror));
    config.set("invert", show.invert ? "true" : "false");
    config.set("feedback", formatDouble(show.feedback));
    config.set("blend", blendModeName(show.blend));
    config.set("morph", formatDouble(show.morph));
    config.set("warp", singleLine(show.warp));
    config.set("warpFile", show.warpFile);
    config.set("videoDriver", show.videoDriver);
    config.set("fullscreen", show.fullscreen ? "true" : "false");
    config.set("hideCursor", show.hideCursor ? "true" : "false");
    config.set("panelSide", show.panelSide);
    config.set("midiSource", show.midiSource);
    config.set("midiMode", midiModeName(show.midiMode));
    config.set("midiTakeover", show.midiTakeover ? "true" : "false");
    config.set("midiOut", show.midiOut);
    config.set("midiOutEnabled", show.midiOutEnabled ? "true" : "false");
    config.set("midiFeedback", show.midiFeedback ? "true" : "false");
    config.set("midiClock", show.midiClock ? "true" : "false");
    config.set("midiClockOut", show.midiClockOut ? "true" : "false");
    config.set("osc", show.osc ? "true" : "false");
    config.set("oscPort", std::to_string(show.oscPort));
    config.set("oscDirect", show.oscDirect ? "true" : "false");
    config.set("oscDirectPort", std::to_string(show.oscDirectPort));
    config.set("audio", show.audio ? "true" : "false");
    config.set("audioDevice", show.audioDevice);
    config.set("audioGain", formatDouble(show.audioGain));
    return config.save(path, error);
}

} // namespace whorld
