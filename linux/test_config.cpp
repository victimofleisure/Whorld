// Unit test for linux/config.cpp: the show-config INI parser and the ShowConfig
// key/value mapping. Links only whorld_core, so it always runs (no SDL/ALSA/GL).
// SPDX-License-Identifier: GPL-3.0-or-later
#include "config.h"
#include "projection.h"
#include "whorld_core.h"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <random>
#include <string>
#include <utility>
#include <vector>

using namespace whorld;

#define require(expr)                                                                     \
    do {                                                                                  \
        if (!(expr)) {                                                                    \
            std::fprintf(stderr, "whorld_config: failed: %s (line %d)\n", #expr,          \
                         __LINE__);                                                       \
            return 1;                                                                     \
        }                                                                                 \
    } while (false)

namespace {

const char* const kFile = "whorld-config-test.conf";
const char* const kRound = "whorld-config-roundtrip.conf";

bool writeFile(const char* path, const std::string& text) {
    std::ofstream out(path, std::ios::binary | std::ios::trunc);
    out << text;
    return static_cast<bool>(out);
}

void removeFile(const char* path) { std::remove(path); }

bool equal(const ShowConfig& a, const ShowConfig& b) {
    return a.patch == b.patch && a.playlist == b.playlist && a.renderer == b.renderer &&
           a.scale == b.scale && a.fps == b.fps && a.wheelSense == b.wheelSense &&
           a.midiClockOut == b.midiClockOut && a.mirror == b.mirror && a.invert == b.invert &&
           a.feedback == b.feedback && a.blend == b.blend && a.morph == b.morph &&
           a.warp == b.warp && a.warpFile == b.warpFile && a.videoDriver == b.videoDriver &&
           a.fullscreen == b.fullscreen && a.hideCursor == b.hideCursor &&
           a.midiSource == b.midiSource && a.midiMode == b.midiMode &&
           a.midiTakeover == b.midiTakeover && a.midiOut == b.midiOut &&
           a.midiOutEnabled == b.midiOutEnabled && a.midiFeedback == b.midiFeedback &&
           a.midiClock == b.midiClock && a.osc == b.osc && a.oscPort == b.oscPort &&
           a.oscDirect == b.oscDirect && a.oscDirectPort == b.oscDirectPort &&
           a.audio == b.audio && a.audioDevice == b.audioDevice && a.audioGain == b.audioGain;
}

} // namespace

int main() {
    // --- parser: comments, blank lines, whitespace and typed getters --------
    require(writeFile(kFile,
        "\xef\xbb\xbf# show defaults; comment with a hash or semicolon\n"
        "; a semicolon comment\n"
        "\n"
        "  renderer  =  gpu   \n"
        "\tfullscreen\t=\tOn\n"
        "oscPort = 9000\n"
        "scale = 0.75\n"
        "audioGain = 1.5\n"
        "invert = no\n"
        "patch = /shows/a.whp\n"
        "unknownKey = keep me\n"));
    {
        Config config;
        std::string error;
        require(config.load(kFile, error));
        require(config.has("renderer") && !config.has("rendererX"));
        require(config.getString("renderer", "auto") == "gpu");
        require(config.getString("missing", "fallback") == "fallback");
        require(config.getBool("fullscreen", false) == true);   // "On"
        require(config.getBool("invert", true) == false);       // "no"
        require(config.getBool("missing", true) == true);
        require(config.getInt("oscPort", 0) == 9000);
        require(config.getInt("missing", 42) == 42);
        require(config.getDouble("scale", 0) == 0.75);
        require(config.getDouble("missing", 2.0) == 2.0);
        // A malformed number falls back rather than throwing.
        config.set("bad", "not-a-number");
        require(config.getDouble("bad", 7.0) == 7.0);
        require(config.getInt("bad", 8) == 8);

        // Round-trip: unknown keys survive a save/load, known keys do not move.
        std::string saveError;
        require(config.save(kRound, saveError));
        Config again;
        require(again.load(kRound, saveError));
        require(again.getString("unknownKey", "") == "keep me");
        require(again.getString("renderer", "") == "gpu");
        require(again.getInt("oscPort", 0) == 9000);
        require(again.getDouble("scale", 0) == 0.75);
    }

    // --- malformed input is an error, not a crash --------------------------
    {
        Config config;
        std::string error;
        require(writeFile(kFile, "this line has no equals sign\n"));
        require(!config.load(kFile, error));
        require(!error.empty());
        require(writeFile(kFile, "key = 1\nkey = 2\n"));
        require(!config.load(kFile, error)); // duplicate key
        require(writeFile(kFile, "= value\n"));
        require(!config.load(kFile, error)); // empty key
        require(!config.load("no-such-config-file.conf", error));
    }

    // --- enum helpers accept the same spellings as the flags ---------------
    {
        int mode = 99;
        require(parseMirrorMode("quad", mode) && mode == 1);
        require(parseMirrorMode("kaleidoscope", mode) && mode == 2);
        require(parseMirrorMode("off", mode) && mode == 0);
        require(!parseMirrorMode("bogus", mode));
        require(parseBlendMode("add", mode) && mode == 1);
        require(parseBlendMode("screen", mode) && mode == 2);
        require(!parseBlendMode("bogus", mode));
        MidiInputMode midi = MidiInputMode::Absolute;
        require(parseMidiModeInput("14bit", midi) && midi == MidiInputMode::FourteenBit);
        require(parseMidiModeInput("rel-sign", midi) && midi == MidiInputMode::RelativeSignMagnitude);
        require(!parseMidiModeInput("bogus", midi));
    }

    // --- ShowConfig: load -> save -> load equality -------------------------
    {
        ShowConfig show;
        show.patch = "/shows/main.whp";
        show.playlist = "/shows/main.whl";
        show.renderer = "cairo";
        show.scale = 0.75;
        show.fps = 45;
        show.wheelSense = 4.5;
        show.midiClockOut = true;
        show.mirror = 3;
        show.invert = true;
        show.feedback = 0.72;
        show.blend = 2;
        show.morph = 2.5;
        show.warp = "0.06,0.04,0.95,0.10,0.90,0.96,0.10,0.88";
        show.warpFile = "/shows/warp.txt";
        show.videoDriver = "kmsdrm";
        show.fullscreen = true;
        show.hideCursor = true;
        show.midiSource = "24:0";
        show.midiMode = MidiInputMode::FourteenBit;
        show.midiTakeover = true;
        show.midiOut = "20:0";
        show.midiOutEnabled = true;
        show.midiFeedback = true;
        show.midiClock = true;
        show.osc = true;
        show.oscPort = 9000;
        show.oscDirect = true;
        show.oscDirectPort = 9100;
        show.audio = true;
        show.audioDevice = "hw:1,0";
        show.audioGain = 1.5;

        std::string error;
        require(saveShowConfig(kFile, show, error));

        ShowConfig first;
        require(loadShowConfig(kFile, first, error));
        require(first.patch == show.patch && first.playlist == show.playlist);
        require(first.renderer == "cairo" && first.scale == 0.75);
        require(first.fps == 45 && std::abs(first.wheelSense - 4.5) < 1e-9);
        require(first.midiClockOut);
        require(first.mirror == 3 && first.invert && first.feedback == 0.72);
        require(first.blend == 2 && first.morph == 2.5);
        WarpQuad quad;
        require(warpFromString(show.warp, quad));
        std::string expected = warpToString(quad);
        for (char& c : expected) if (c == '\n') c = ' ';
        const auto last = expected.find_last_not_of(" \t");
        expected.erase(last == std::string::npos ? 0 : last + 1);
        require(first.warp == expected && !first.warp.empty());
        require(first.warpFile == show.warpFile && first.videoDriver == "kmsdrm");
        require(first.fullscreen && first.hideCursor);
        require(first.midiSource == "24:0" && first.midiMode == MidiInputMode::FourteenBit);
        require(first.midiTakeover && first.midiOut == "20:0" && first.midiOutEnabled);
        require(first.midiFeedback && first.midiClock);
        require(first.osc && first.oscPort == 9000 && first.oscDirect && first.oscDirectPort == 9100);
        require(first.audio && first.audioDevice == "hw:1,0" && first.audioGain == 1.5);

        require(saveShowConfig(kRound, first, error));
        ShowConfig second;
        require(loadShowConfig(kRound, second, error));
        require(equal(first, second));
    }

    // --- bad enum/range values are rejected with an error ------------------
    {
        ShowConfig show;
        std::string error;
        require(writeFile(kFile, "renderer = opengl\n"));
        require(!loadShowConfig(kFile, show, error) && !error.empty());
        require(writeFile(kFile, "mirror = flip\n"));
        require(!loadShowConfig(kFile, show, error));
        require(writeFile(kFile, "scale = 9\n"));
        require(!loadShowConfig(kFile, show, error));
        require(writeFile(kFile, "fps = 0\n"));
        require(!loadShowConfig(kFile, show, error));
        require(writeFile(kFile, "fps = 500\n"));
        require(!loadShowConfig(kFile, show, error));
        require(writeFile(kFile, "wheelSense = 0\n"));
        require(!loadShowConfig(kFile, show, error));
        require(writeFile(kFile, "warp = 0 0 1 0 1\n"));
        require(!loadShowConfig(kFile, show, error));
    }

    // --- fuzz: random/garbage INI text must never crash the loader -----------
    {
        std::mt19937 rng(0xc0ffeeu); // fixed seed: reproducible
        std::uniform_int_distribution<int> lenDist(0, 2048);
        std::uniform_int_distribution<int> byteDist(0, 255);
        Config config;
        std::string error;
        for (int iter = 0; iter < 400; ++iter) {
            const int len = lenDist(rng);
            std::string text(static_cast<std::size_t>(len), '\0');
            for (char& c : text) c = static_cast<char>(byteDist(rng));
            require(writeFile(kFile, text));
            // Any outcome is acceptable; the loader must not crash or throw.
            config.load(kFile, error);
        }
    }

    // --- round-trip random key/value sets -----------------------------------
    {
        std::mt19937 rng(0xf00d1234u);
        // No whitespace/newline: save emits one line per key and load trims the
        // edges, so keeping values whitespace-free makes the round trip exact.
        const std::string valueChars = "abcXYZ019+-=.,_-()[]{}!@$%^&*;'\"<>?~`";
        std::uniform_int_distribution<int> countDist(0, 40);
        std::uniform_int_distribution<int> valueLen(0, 24);
        std::uniform_int_distribution<int> charIndex(0, int(valueChars.size()) - 1);
        std::string error;
        for (int iter = 0; iter < 50; ++iter) {
            Config config;
            const int count = countDist(rng);
            std::vector<std::pair<std::string, std::string>> expected;
            for (int i = 0; i < count; ++i) {
                const std::string key = "key" + std::to_string(i);
                std::string value;
                const int len = valueLen(rng);
                for (int c = 0; c < len; ++c) value.push_back(valueChars[charIndex(rng)]);
                config.set(key, value);
                expected.emplace_back(key, value);
            }
            require(config.save(kRound, error));
            Config loaded;
            require(loaded.load(kRound, error));
            for (const auto& entry : expected)
                require(loaded.getString(entry.first, "<missing>") == entry.second);
        }
    }

    removeFile(kFile);
    removeFile(kRound);
    std::printf("ok\n");
    return 0;
}
