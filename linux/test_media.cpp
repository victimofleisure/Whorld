// Playlist interchange, MIDI mapping and snapshot round trips.
#include "alsa_midi.h"
#include "gamepad.h"
#include "midi_actions.h"
#include "midi_decode.h"
#include "movie.h"
#include "osc.h"
#include "playlist.h"
#include "snapshot.h"
#include "tap_tempo.h"
#include "whorld_core.h"
#include <cmath>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>
#define require(expr) do { if (!(expr)) throw std::runtime_error("Check failed: " #expr); } while (false)

// Fixture mode: whorld_media_test --write-snapshot FILE.whs
// Captures a deterministic snapshot for the whorld_snapshot_image test.
static int writeSnapshot(const std::string& path) {
    using namespace whorld;
    Patch patch;
    patch.params[PARAM_RingGrowth].value = 2;
    patch.master[MASTER_Rings] = 40;
    Engine engine;
    engine.setPatch(patch);
    for (int i = 0; i < 120; ++i) { engine.tick(480, 360); engine.frame(480, 360); }
    std::string error;
    const Snapshot snapshot = engine.captureSnapshot(480, 360);
    if (snapshot.rings.empty() || !snapshot.save(path, error)) {
        std::cerr << "cannot write snapshot: " << error << "\n";
        return 1;
    }
    return 0;
}

int main(int argc, char** argv) {
    using namespace whorld;
    if (argc == 3 && std::string(argv[1]) == "--write-snapshot")
        return writeSnapshot(argv[2]);
    namespace fs = std::filesystem;
    const fs::path dir = fs::temp_directory_path() / "whorld-media-test";
    fs::create_directories(dir);
    std::error_code ec;
    const std::string patchPath = (dir / "one.whp").string();
    const std::string listPath = (dir / "list.whl").string();
    const std::string snapPath = (dir / "one.whs").string();

    Patch patch;
    patch.params[PARAM_RingGrowth].value = 2;
    patch.master[MASTER_Rings] = 40;
    std::string error;
    require(patch.save(patchPath, error));
    require(patch.save((dir / "two.whp").string(), error));

    Playlist playlist;
    playlist.patches = {"one.whp", "two.whp"};
    playlist.mappings.push_back({"CONTROL", 0, 1, "RingGrowth", "Val", 0, 100});
    playlist.mappings.push_back({"CONTROL", 0, 7, "Fill", "Val", 0, 100});
    require(playlist.save(listPath, error));
    Playlist loaded;
    require(loaded.load(listPath, error));
    require(loaded.path == listPath);
    require(loaded.patches.size() == 2 && loaded.patches[0] == "one.whp");
    require(loaded.mappings.size() == 2);
    require(loaded.mappings[0].target == "RingGrowth" && loaded.mappings[0].control == 1);
    require(loaded.resolvePath(0) == (dir / "one.whp").string());
    require(loaded.resolvePath(5).empty());

    // An invalid mapping must be rejected rather than written out.
    Playlist bad;
    bad.mappings.push_back({"CONTROL", 0, 1, "NoSuchTarget", "Val", 0, 100});
    require(!bad.save((dir / "bad.whl").string(), error));
    require(!bad.load(listPath + ".missing", error));

    Engine engine;
    engine.setPatch(patch);
    for (int i = 0; i < 120; ++i) { engine.tick(480, 360); engine.frame(480, 360); }
    require(engine.ringCount() > 0);

    MidiMessage message;
    message.event = MidiEvent::Control;
    message.channel = 0;
    message.control = 1;
    message.value = 127; // full scale maps to 1.0
    const auto actions = mapMidi(loaded, message);
    require(actions.size() == 1);
    require(actions[0].target == "RingGrowth");
    require(std::abs(actions[0].amount - 1.0) < 1e-9);
    bool paused = false;
    int current = -1;
    std::string status;
    require(applyMidiAction(actions[0], engine, loaded, paused, current, status));
    require(engine.patch.params[PARAM_RingGrowth].value == paramInfo[PARAM_RingGrowth].max);

    message.control = 7;
    const auto toggles = mapMidi(loaded, message);
    require(toggles.size() == 1 && toggles[0].target == "Fill");
    require(applyMidiAction(toggles[0], engine, loaded, paused, current, status));
    require((engine.patch.drawMode & DM_FILL) != 0);
    message.control = 42; // no mapping listens on this control
    require(mapMidi(loaded, message).empty());

    // 14-bit CC decoding, relative-encoder tables and soft takeover are pure
    // logic in midi_decode, so they run without any hardware.
    require(midi14bit(0, 0) == 0);
    require(midi14bit(1, 0) == 128);
    require(midi14bit(64, 0) == 8192);
    require(midi14bit(127, 127) == 16383);

    // Relative encoder conventions (documented in midi_decode.h).
    require(relativeDelta(1, MidiInputMode::RelativeTwosComplement) == 1);
    require(relativeDelta(63, MidiInputMode::RelativeTwosComplement) == 63);
    require(relativeDelta(64, MidiInputMode::RelativeTwosComplement) == 0);
    require(relativeDelta(65, MidiInputMode::RelativeTwosComplement) == -63);
    require(relativeDelta(127, MidiInputMode::RelativeTwosComplement) == -1);
    require(relativeDelta(0, MidiInputMode::RelativeTwosComplement) == 0);
    require(relativeDelta(63, MidiInputMode::RelativeSignMagnitude) == 63);
    require(relativeDelta(64, MidiInputMode::RelativeSignMagnitude) == 0);
    require(relativeDelta(65, MidiInputMode::RelativeSignMagnitude) == -1);
    require(relativeDelta(127, MidiInputMode::RelativeSignMagnitude) == -63);
    require(relativeDelta(0, MidiInputMode::RelativeBinaryOffset) == -64);
    require(relativeDelta(63, MidiInputMode::RelativeBinaryOffset) == -1);
    require(relativeDelta(64, MidiInputMode::RelativeBinaryOffset) == 0);
    require(relativeDelta(65, MidiInputMode::RelativeBinaryOffset) == 1);
    require(relativeDelta(127, MidiInputMode::RelativeBinaryOffset) == 63);
    require(relativeDelta(127, MidiInputMode::Absolute) == 0); // not a relative mode

    // Soft takeover: ignore until the control reaches or crosses the target,
    // then track it freely.
    {
        TakeoverState pickup;
        require(!pickup.accept(0.10, 0.50));
        require(!pickup.accept(0.20, 0.50));
        require(!pickup.accept(0.40, 0.50)); // approaching, not yet crossed
        require(pickup.accept(0.80, 0.50));  // crossed between samples
        require(pickup.accept(0.30, 0.50));  // engaged: tracks anywhere
        require(pickup.accept(0.00, 0.50));

        TakeoverState near;
        require(near.accept(0.51, 0.50)); // reaches the target within epsilon
        TakeoverState edge;
        require(!edge.accept(0.00, 0.50, 0.02));
        require(edge.accept(0.49, 0.50, 0.02));
        TakeoverState down;
        require(!down.accept(0.90, 0.50));
        require(down.accept(0.20, 0.50)); // crossed downward
        TakeoverState away;
        require(!away.accept(0.10, 0.50));
        require(!away.accept(0.05, 0.50)); // moving away must not engage
    }

    // Exhaustive relative-encoder tables for all 128 7-bit values: each
    // convention is decoded independently here (midi_decode.h documents them).
    {
        for (int v = 0; v <= 127; ++v) {
            int twos = 0;
            if (v != 0 && v != 64) twos = v < 64 ? v : v - 128;
            require(relativeDelta(v, MidiInputMode::RelativeTwosComplement) == twos);

            int sign = 0;
            if (v != 0 && v != 64) sign = (v & 0x40) ? -(v & 0x3f) : (v & 0x3f);
            require(relativeDelta(v, MidiInputMode::RelativeSignMagnitude) == sign);

            require(relativeDelta(v, MidiInputMode::RelativeBinaryOffset) == v - 64);
            require(relativeDelta(v, MidiInputMode::Absolute) == 0);
        }
        // midi14bit is the exact MSB<<7|LSB combine and clamps both bytes.
        for (int msb = 0; msb <= 127; ++msb)
            for (int lsb = 0; lsb <= 127; ++lsb)
                require(midi14bit(msb, lsb) == msb * 128 + lsb);
        require(midi14bit(-1, -1) == 0);
        require(midi14bit(0, 200) == 127);
        require(midi14bit(200, 0) == 127 << 7);
        require(midi14bit(64, 64) == 64 * 128 + 64);
    }

    // shapeAxis: monotone, bounded and dead-zoned across a fine sweep for
    // several parameter sets, including the exact endpoints and dead-zone edge.
    {
        const double deadzones[] = {0.0, 0.05, 0.15, 0.4};
        const double exponents[] = {1.0, 1.6, 0.5, 3.0};
        for (double dz : deadzones) {
            for (double ex : exponents) {
                double previous = -2.0;
                for (int i = 0; i <= 2000; ++i) {
                    const double raw = -1.0 + i * 0.001;
                    const double shaped = shapeAxis(raw, dz, ex);
                    require(std::isfinite(shaped));
                    require(shaped >= -1.0 - 1e-12 && shaped <= 1.0 + 1e-12);
                    require(shaped >= previous - 1e-12); // non-decreasing
                    previous = shaped;
                    if (std::abs(raw) <= dz)
                        require(shaped == 0.0);
                    else
                        require((raw > 0) == (shaped > 0) || shaped == 0.0);
                }
                require(std::abs(shapeAxis(1.0, dz, ex) - 1.0) < 1e-12);
                require(std::abs(shapeAxis(-1.0, dz, ex) + 1.0) < 1e-12);
            }
        }
    }

    // Random soft-takeover sequences: accept() must agree with the engaged
    // flag, engagement is sticky, and non-finite samples never engage.
    {
        std::mt19937 rng(0x7a1ce0u); // fixed seed: reproducible
        std::uniform_real_distribution<double> value(-0.2, 1.2);
        for (int t = 0; t < 300; ++t) {
            TakeoverState state;
            const double current = value(rng);
            bool everEngaged = false;
            for (int i = 0; i < 40; ++i) {
                const double incoming = value(rng);
                const bool accepted = state.accept(incoming, current);
                require(accepted == state.engaged);
                if (accepted) everEngaged = true;
                if (everEngaged) require(state.engaged);
            }
            TakeoverState fresh;
            require(!fresh.accept(std::nan(""), current));
            require(!fresh.accept(0.5, std::nan("")));
            require(!fresh.engaged);
        }
    }

    // encodeControlChange emits an exact 3-byte CC and clamps its arguments.
    {
        std::vector<unsigned char> cc;
        encodeControlChange(cc, 0, 1, 64);
        require(cc.size() == 3);
        require(cc[0] == 0xb0 && cc[1] == 0x01 && cc[2] == 0x40);
        cc.clear();
        encodeControlChange(cc, 15, 127, 127);
        require(cc[0] == 0xbf && cc[1] == 0x7f && cc[2] == 0x7f);
        cc.clear();
        encodeControlChange(cc, 99, -5, 999); // channel/control/value all clamp
        require(cc[0] == 0xbf && cc[1] == 0x00 && cc[2] == 0x7f);
        // Appends to the vector instead of replacing it.
        encodeControlChange(cc, 1, 2, 3);
        require(cc.size() == 6);
        require(cc[3] == 0xb1 && cc[4] == 0x02 && cc[5] == 0x03);
    }

    // ClockTracker follows an external 24-PPQN clock with explicit timestamps,
    // so the tempo math is exercised without any hardware. One beat at 120 BPM
    // is 0.5 s, so its 24 pulses are 60/(120*24) s apart.
    {
        const double ppq = ClockTracker::pulsesPerBeat;
        const double step120 = 60.0 / (120.0 * ppq);
        const double step90 = 60.0 / (90.0 * ppq);
        ClockTracker clock;
        require(!clock.active()); // nothing has arrived yet
        require(!clock.running());
        clock.start();
        require(clock.running());
        // Exactly the right number of intervals at 120 BPM.
        for (int i = 0; i < 48; ++i) clock.tick(i * step120);
        const double last120 = 47 * step120;
        require(clock.active());
        require(clock.running());
        require(std::abs(clock.bpm() - 120.0) < 1.0);
        // ~1 s of silence expires an active clock but keeps the last tempo so
        // the caller can leave Tempo where it was.
        clock.update(last120 + 1.5);
        require(!clock.active());
        require(clock.running()); // transport state is independent of pulses
        require(std::abs(clock.bpm() - 120.0) < 1.0);
        clock.stop();
        require(!clock.running());

        // A second tempo proves the BPM is derived, not hard-coded, and that a
        // fresh lock needs a few intervals before it reports active.
        ClockTracker ninety;
        ninety.start();
        for (int i = 0; i < 6; ++i) ninety.tick(i * step90);
        require(!ninety.active() || std::abs(ninety.bpm() - 90.0) < 1.0);
        for (int i = 6; i < 48; ++i) ninety.tick(i * step90);
        require(ninety.active());
        require(std::abs(ninety.bpm() - 90.0) < 1.0);

        // reset() forgets the transport, the lock and the smoothed tempo.
        ninety.reset();
        require(!ninety.running());
        require(!ninety.active());
        require(ninety.bpm() == 0);

        // A long gap between pulses relocks instead of polluting the average.
        ClockTracker relock;
        relock.start();
        for (int i = 0; i < 48; ++i) relock.tick(i * step120);
        require(relock.active());
        const double gap = last120 + 1.5;
        relock.tick(gap); // a stray pulse after a long silence
        require(!relock.active()); // must re-accumulate before reporting again
        for (int i = 1; i < 48; ++i) relock.tick(gap + i * step120);
        require(relock.active());
        require(std::abs(relock.bpm() - 120.0) < 1.0);
    }

    // applyRange honours Start/End, including reversed ranges, without clamping.
    {
        Mapping full;
        full.start = 0;
        full.end = 100;
        require(std::abs(applyRange(full, 0.5) - 0.5) < 1e-9);
        Mapping reversed;
        reversed.start = 80;
        reversed.end = 20;
        require(std::abs(applyRange(reversed, 0.0) - 0.8) < 1e-9);
        require(std::abs(applyRange(reversed, 1.0) - 0.2) < 1e-9);
        require(std::abs(applyRange(reversed, 0.5) - 0.5) < 1e-9);
        Mapping wide;
        wide.start = -50;
        wide.end = 150;
        require(std::abs(applyRange(wide, 1.0) - 1.5) < 1e-9); // out of range is allowed
    }

    // A Control message with a 14-bit value normalizes like the pitch wheel;
    // plain 7-bit values keep the existing dual-range formula.
    {
        MidiMessage wide;
        wide.event = MidiEvent::Control;
        wide.channel = 0;
        wide.control = 1;
        wide.value = 16383;
        require(std::abs(mapMidi(loaded, wide)[0].amount - 1.0) < 1e-9);
        wide.value = 8192;
        require(std::abs(mapMidi(loaded, wide)[0].amount - 0.5) < 1e-9);
        wide.value = 0;
        require(std::abs(mapMidi(loaded, wide)[0].amount) < 1e-9);
        wide.value = 128; // just above zero on the 14-bit scale
        require(mapMidi(loaded, wide)[0].amount > 0 && mapMidi(loaded, wide)[0].amount < 0.01);

        MidiMessage narrow;
        narrow.event = MidiEvent::Control;
        narrow.channel = 0;
        narrow.control = 1;
        narrow.value = 127; // the 7-bit path is unchanged
        require(std::abs(mapMidi(loaded, narrow)[0].amount - 1.0) < 1e-9);
        narrow.value = 64;
        require(std::abs(mapMidi(loaded, narrow)[0].amount - 0.5) < 1e-9);
        narrow.event = MidiEvent::NoteOn; // a 7-bit event still rejects 200
        narrow.value = 200;
        require(mapMidi(loaded, narrow).empty());
    }

    // mapMidiNormalized is the shared core the relative modes use: it takes the
    // normalized value directly and applies the mapping's range.
    {
        Playlist ranged;
        ranged.mappings.push_back({"CONTROL", 1, 3, "RingGrowth", "Val", 25, 75});
        const auto half = mapMidiNormalized(ranged, MidiEvent::Control, 1, 3, 0.5);
        require(half.size() == 1 && std::abs(half[0].amount - 0.5) < 1e-9);
        const auto top = mapMidiNormalized(ranged, MidiEvent::Control, 1, 3, 1.0);
        require(std::abs(top[0].amount - 0.75) < 1e-9);
        require(mapMidiNormalized(ranged, MidiEvent::Control, 1, 4, 0.5).empty());
    }

    // Gamepad and audio mappings round-trip and dispatch to the same targets.
    {
        Playlist controls;
        controls.patches = {"one.whp"};
        controls.gamepadMappings.push_back({"A", "Clear", "Val", 0, 100});
        controls.gamepadMappings.push_back({"B", "Fill", "Val", 0, 100});
        controls.gamepadMappings.push_back({"LX", "Zoom", "Val", 25, 75});
        controls.audioMappings.push_back({"Level", "RingGrowth", "Val", 0, 100});
        controls.audioMappings.push_back({"Low", "PolySides", "Val", 20, 80});
        const std::string controlPath = (dir / "controls.whl").string();
        require(controls.save(controlPath, error));
        Playlist reloaded;
        require(reloaded.load(controlPath, error));
        require(reloaded.mappings.empty());
        require(reloaded.gamepadMappings.size() == 3 && reloaded.audioMappings.size() == 2);
        require(reloaded.gamepadMappings[2].control == "LX");
        require(reloaded.gamepadMappings[2].start == 25 && reloaded.gamepadMappings[2].end == 75);
        require(reloaded.audioMappings[0].feature == "Level");

        const auto hasTarget = [](const std::vector<MappedAction>& v, const char* t) {
            for (const auto& a : v) if (a.target == t) return true;
            return false;
        };
        const auto amount = [](const std::vector<MappedAction>& v, const char* t) {
            for (const auto& a : v) if (a.target == t) return a.amount;
            return -1.0;
        };

        // A button fires on its rising edge: a momentary target (Clear) only on
        // press, while a BOOL target (Fill) also emits on release so it can be
        // held. Axes are continuous and emit every frame.
        GamepadState current, previous;
        require(mapGamepad(reloaded, current, previous).size() == 1); // idle: axis only
        current.button[GP_A] = true;
        current.button[GP_B] = true;
        const auto press = mapGamepad(reloaded, current, previous);
        require(hasTarget(press, "Clear") && std::abs(amount(press, "Clear") - 1.0) < 1e-9);
        require(hasTarget(press, "Fill") && std::abs(amount(press, "Fill") - 1.0) < 1e-9);
        previous = current; // the frontend advances `previous` every frame
        const auto held = mapGamepad(reloaded, current, previous);
        require(held.size() == 1 && held[0].target == "Zoom"); // no button repeat
        current.button[GP_B] = false; // release: Fill goes back off
        const auto release = mapGamepad(reloaded, current, previous);
        require(release.size() == 2 && hasTarget(release, "Fill") && std::abs(amount(release, "Fill")) < 1e-9);
        require(!hasTarget(release, "Clear"));

        // A full-right stick spans the mapped Start..End range.
        GamepadState centered, idle;
        centered.axis[GP_LX] = 1.0;
        const auto moved = mapGamepad(reloaded, centered, idle);
        require(moved.size() == 1 && std::abs(moved[0].amount - 0.75) < 1e-9);

        // Audio features scale onto the mapped targets.
        AudioFeatures features;
        features.level = 1.0;
        features.low = 0.5;
        const auto sound = mapAudio(reloaded, features);
        require(sound.size() == 2);
        require(sound[0].target == "RingGrowth" && std::abs(sound[0].amount - 1.0) < 1e-9);
        require(sound[1].target == "PolySides" && std::abs(sound[1].amount - 0.5) < 1e-9);

        // Unknown control/feature names are rejected before writing.
        Playlist badGamepad;
        badGamepad.gamepadMappings.push_back({"NoSuchControl", "Clear", "Val", 0, 100});
        require(!badGamepad.save((dir / "bad-gamepad.whl").string(), error));
        Playlist badAudio;
        badAudio.audioMappings.push_back({"NoSuchFeature", "RingGrowth", "Val", 0, 100});
        require(!badAudio.save((dir / "bad-audio.whl").string(), error));

        // The built-in default audio set is itself valid.
        Playlist defaults;
        defaults.audioMappings = defaultAudioMappings();
        require(defaults.save((dir / "default-audio.whl").string(), error));
    }

    // Port-local targets: Mirror and InvertColor dispatch to the patch (the
    // frontend then syncs them into the renderer's post-processing state).
    {
        Engine fx;
        fx.setPatch(patch);
        Playlist none;
        bool fp = false;
        int fc = 0;
        std::string fs;
        require(applyMidiAction({"Mirror", "Val", 100}, fx, none, fp, fc, fs));
        require(fx.patch.mirror);
        require(applyMidiAction({"Mirror", "Val", 0}, fx, none, fp, fc, fs));
        require(!fx.patch.mirror);
        require(applyMidiAction({"InvertColor", "Val", 100}, fx, none, fp, fc, fs));
        require(fx.patch.invertColor);
        require(applyMidiAction({"InvertColor", "Val", 0}, fx, none, fp, fc, fs));
        require(!fx.patch.invertColor);
    }

    // Tap tempo averages the last few intervals and restarts after a pause.
    {
        TapTempo tap;
        require(tap.tap(10.0) == 0.0);
        require(std::abs(tap.tap(10.5) - 120.0) < 1e-9);
        require(tap.tap(20.0) == 0.0); // a long pause restarts the measurement
        require(std::abs(tap.tap(20.4) - 150.0) < 1e-9);
        require(std::abs(tap.tap(21.0) - 120.0) < 1e-9);   // intervals .4, .6
        require(std::abs(tap.tap(21.5) - 120.0) < 1e-9);
        // Only the last four taps count; this one averages .6, .5, .1 -> 150.
        require(std::abs(tap.tap(21.6) - 150.0) < 1e-9);
        tap.reset();
        require(tap.tap(30.0) == 0.0);
        // A mapped Tap button drives Tempo and ignores the release edge.
        Engine tempoEngine;
        Playlist none;
        bool tp = false; int tc = 0; std::string ts;
        tempoEngine.patch.master[MASTER_Tempo] = 100;
        require(applyMidiAction({"Tap", "Val", 100}, tempoEngine, none, tp, tc, ts, &tap));
        require(applyMidiAction({"Tap", "Val", 100}, tempoEngine, none, tp, tc, ts, &tap));
        require(tempoEngine.patch.master[MASTER_Tempo] >= 20.0 &&
                tempoEngine.patch.master[MASTER_Tempo] <= 300.0);
        require(ts.find("BPM") != std::string::npos);
        require(applyMidiAction({"Tap", "Val", 0}, tempoEngine, none, tp, tc, ts, &tap));
    }

    // Single step advances only while paused, and ignores the release edge.
    {
        Engine stepper;
        Playlist none;
        bool sp = false; int sc = 0; std::string ss; bool step = false;
        require(applyMidiAction({"Step", "Val", 100}, stepper, none, sp, sc, ss, nullptr, &step));
        require(!step); // running: nothing to step
        sp = true;
        require(applyMidiAction({"Step", "Val", 100}, stepper, none, sp, sc, ss, nullptr, &step));
        require(step);
        step = false;
        require(applyMidiAction({"Step", "Val", 0}, stepper, none, sp, sc, ss, nullptr, &step));
        require(!step); // release must not step again
    }

    // Wheel zoom uses the upstream normalized master space: 0.5 is the
    // property's nominal value (1.0x zoom) and the ends clamp.
    {
        require(std::abs(normalizeMaster(MASTER_Zoom, 1.0) - 0.5) < 1e-6);
        require(std::abs(denormalizeMaster(MASTER_Zoom, 0.5) - 1.0) < 1e-6);
        const double up = normalizeMaster(MASTER_Zoom, 2.0);
        require(up > 0.5 && up < 1.0);
        require(std::abs(denormalizeMaster(MASTER_Zoom, 1.0) - 10.0) < 1e-9);
        require(std::abs(denormalizeMaster(MASTER_Zoom, -1.0) - denormalizeMaster(MASTER_Zoom, 0.0)) < 1e-9);
        const double round = denormalizeMaster(MASTER_Damping, normalizeMaster(MASTER_Damping, 0.42));
        require(std::abs(round - 0.42) < 1e-6);
        require(std::abs(denormalizeMaster(MASTER_Zoom, normalizeMaster(MASTER_Zoom, 1.7)) - 1.7) < 1e-4);
    }

    // Gamepad learn binds a control and replaces any prior mapping for it.
    {
        Playlist learned;
        learnGamepad(learned, "A", "Clear", "Val");
        require(learned.gamepadMappings.size() == 1);
        require(learned.gamepadMappings[0].control == "A" && learned.gamepadMappings[0].target == "Clear");
        learnGamepad(learned, "A", "Pause", "Val");
        require(learned.gamepadMappings.size() == 1 && learned.gamepadMappings[0].target == "Pause");
        learnGamepad(learned, "LX", "Zoom", "Val");
        require(learned.gamepadMappings.size() == 2);
        const std::string learnedPath = (dir / "learned.whl").string();
        require(learned.save(learnedPath, error));
        Playlist reloadedLearn;
        require(reloadedLearn.load(learnedPath, error));
        require(reloadedLearn.gamepadMappings.size() == 2);
        require(reloadedLearn.gamepadMappings[0].control == "A" &&
                reloadedLearn.gamepadMappings[0].target == "Pause");
        require(reloadedLearn.gamepadMappings[1].control == "LX");
        // Combined control index: axes first, then buttons.
        require(std::string(gamepadControlName(0)) == "LX");
        require(std::string(gamepadControlName(GP_AXIS_COUNT + GP_A)) == "A");
        require(std::string(gamepadControlName(-1)).empty());
    }

    // shapeAxis: dead-zone, sign preservation, response curve, clamping.
    {
        // With no dead-zone and a linear curve it is the identity (up to clamp).
        require(std::abs(shapeAxis(0.0, 0.0, 1.0)) < 1e-12);
        require(std::abs(shapeAxis(0.5, 0.0, 1.0) - 0.5) < 1e-12);
        require(std::abs(shapeAxis(-0.5, 0.0, 1.0) + 0.5) < 1e-12);
        require(std::abs(shapeAxis(1.0, 0.0, 1.0) - 1.0) < 1e-12);
        // The dead-zone zeroes the noise band exactly.
        require(shapeAxis(0.0, 0.15, 1.6) == 0.0);
        require(shapeAxis(0.10, 0.15, 1.6) == 0.0);
        require(shapeAxis(-0.10, 0.15, 1.6) == 0.0);
        require(shapeAxis(0.15, 0.15, 1.6) == 0.0);
        // Sign is preserved and the two halves mirror each other.
        require(shapeAxis(0.5, 0.15, 1.6) > 0);
        require(shapeAxis(-0.5, 0.15, 1.6) < 0);
        require(std::abs(shapeAxis(-0.5, 0.15, 1.6) + shapeAxis(0.5, 0.15, 1.6)) < 1e-12);
        // Full deflection still reaches 1 (the live band is rescaled).
        require(std::abs(shapeAxis(1.0, 0.15, 1.6) - 1.0) < 1e-12);
        const double just = shapeAxis(0.15 + 1e-6, 0.15, 1.6);
        require(just > 0 && just < 0.05);
        // A curve exponent > 1 is softer near centre than the linear response.
        const double middle = (0.15 + 1.0) / 2;
        require(shapeAxis(middle, 0.15, 1.6) < shapeAxis(middle, 0.15, 1.0));
        // Monotonic across the whole range.
        double previous = -1.0;
        for (int i = 0; i <= 100; ++i) {
            const double shaped = shapeAxis(-1.0 + i * 0.02, 0.15, 1.6);
            require(shaped >= previous - 1e-12);
            previous = shaped;
        }
        // Out-of-range and non-finite input is handled safely.
        require(std::abs(shapeAxis(2.0, 0.15, 1.6) - 1.0) < 1e-12);
        require(std::abs(shapeAxis(-2.0, 0.15, 1.6) + 1.0) < 1e-12);
        require(shapeAxis(std::nan(""), 0.15, 1.6) == 0.0);
        require(shapeAxis(0.5, 1.0, 1.6) == 0.0); // dead-zone covers the range
    }

    // The built-in default gamepad profile must be valid, complete and
    // round-trip through save/load unchanged.
    {
        const auto& defaults = defaultGamepadMappings();
        require(!defaults.empty());
        require(hasGamepadMappings(defaults));
        require(!hasGamepadMappings({}));
        // Saving proves every control and target name is accepted.
        Playlist profile;
        profile.gamepadMappings = defaults;
        const std::string profilePath = (dir / "default-gamepad.whl").string();
        require(profile.save(profilePath, error));
        Playlist reloadedProfile;
        require(reloadedProfile.load(profilePath, error));
        require(reloadedProfile.gamepadMappings.size() == defaults.size());
        for (std::size_t i = 0; i < defaults.size(); ++i) {
            const GamepadMapping& before = defaults[i];
            const GamepadMapping& after = reloadedProfile.gamepadMappings[i];
            require(before.control == after.control && before.target == after.target);
            require(before.property == after.property);
            require(std::abs(before.start - after.start) < 1e-9);
            require(std::abs(before.end - after.end) < 1e-9);
        }
        // An Xbox-style pad is fully covered out of the box.
        const auto covers = [&](const char* control) {
            for (const GamepadMapping& m : defaults) if (m.control == control) return true;
            return false;
        };
        for (const char* control : {"LX", "LY", "RX", "RY", "LTrigger", "RTrigger",
                                    "A", "B", "X", "Y", "LB", "RB",
                                    "DPadUp", "DPadDown", "DPadLeft", "DPadRight"})
            require(covers(control));
        // The profile actually dispatches: A maps to the momentary Pause action,
        // and a full stick sweep reaches a defined target.
        Playlist engineList;
        engineList.gamepadMappings = defaults;
        GamepadState idle, pressed;
        pressed.button[GP_A] = true;
        bool sawPause = false;
        for (const MappedAction& action : mapGamepad(engineList, pressed, idle))
            if (action.target == "Pause" && std::abs(action.amount - 1.0) < 1e-9) sawPause = true;
        require(sawPause);
        GamepadState full, none;
        full.axis[GP_LX] = 1.0;
        bool sawZoom = false;
        for (const MappedAction& action : mapGamepad(engineList, full, none))
            if (action.target == "Zoom" && std::abs(action.amount - 1.0) < 1e-9) sawZoom = true;
        require(sawZoom);
    }

    // Pure multi-controller status formatting.
    {
        require(controllerSummary({}).empty());
        require(controllerSummary({"Xbox Wireless Controller"}) == "Controller: Xbox Wireless Controller");
        require(controllerSummary({"A", "B"}) == "Controllers: 2 (A, B)");
        require(controllerSummary({"A", "B", "C"}) == "Controllers: 3 (A, B, C)");
    }

    // InvertColor is a valid target in both MIDI and gamepad sections.
    {
        Playlist invert;
        invert.mappings.push_back({"CONTROL", 0, 5, "InvertColor", "Val", 0, 100});
        invert.gamepadMappings.push_back({"X", "InvertColor", "Val", 0, 100});
        const std::string invertPath = (dir / "invert.whl").string();
        require(invert.save(invertPath, error));
        Playlist reloadedInvert;
        require(reloadedInvert.load(invertPath, error));
        require(reloadedInvert.mappings.size() == 1 && reloadedInvert.mappings[0].target == "InvertColor");
        require(reloadedInvert.gamepadMappings.size() == 1 &&
                reloadedInvert.gamepadMappings[0].target == "InvertColor");
    }

    // RandomPhase is accepted and changes the emitted frame, matching upstream's
    // randomize-modulator-phases action.
    {
        Patch phasePatch;
        phasePatch.params[PARAM_RingGrowth].value = 4;
        phasePatch.params[PARAM_RingGrowth].amp = 2;
        phasePatch.params[PARAM_RingGrowth].freq = 3;
        phasePatch.params[PARAM_BkLightness].value = .5;
        phasePatch.params[PARAM_BkLightness].amp = .3;
        phasePatch.params[PARAM_BkLightness].freq = 3;
        phasePatch.master[MASTER_Rings] = 10;
        Engine randomized, untouched;
        randomized.setPatch(phasePatch);
        untouched.setPatch(phasePatch);
        Playlist none;
        bool rp = false;
        int rc = 0;
        std::string rs;
        require(applyMidiAction({"RandomPhase", "Val", 100}, randomized, none, rp, rc, rs));
        const auto sum = [](const Frame& f) {
            double v = f.background.r + f.background.g + f.background.b;
            for (const Shape& shape : f.shapes)
                for (const Segment& seg : shape.outer.segments) {
                    const double dx = seg.end.x - 200, dy = seg.end.y - 150;
                    v += dx * dx + dy * dy;
                }
            return v;
        };
        for (int i = 0; i < 10; ++i) { randomized.tick(400, 300); untouched.tick(400, 300); }
        require(randomized.ringCount() > 0 && untouched.ringCount() > 0);
        require(sum(randomized.frame(400, 300)) != sum(untouched.frame(400, 300)));
    }

    // Ranges may be hand-written as plain decimals (as the README shows), not
    // only as the packed binary doubles that save() emits.
    {
        const std::string handPath = (dir / "hand.whl").string();
        std::ofstream hand(handPath, std::ios::binary);
        hand << "[Settings]\nFileID=WhorldPlay\nFileVersion=1\n"
             << "[Patch]\nCount=0\n"
             << "[Mapping]\nCount=1\n[Mapping\\0]\nEvent=CONTROL\nTarget=RingGrowth\n"
             << "Prop=Val\nChannel=0\nControl=1\nStart=25\nEnd=80\n"
             << "[Gamepad]\nCount=1\n[Gamepad\\0]\nControl=LX\nTarget=Zoom\nProp=Val\nStart=25.5\nEnd=80.25\n"
             << "[Audio]\nCount=1\n[Audio\\0]\nFeature=High\nTarget=RotateSpeed\nProp=Val\nStart=50\nEnd=100\n";
        hand.close();
        Playlist parsed;
        require(parsed.load(handPath, error));
        require(parsed.mappings.size() == 1 && parsed.mappings[0].start == 25 && parsed.mappings[0].end == 80);
        require(parsed.gamepadMappings.size() == 1);
        require(std::abs(parsed.gamepadMappings[0].start - 25.5) < 1e-9);
        require(std::abs(parsed.gamepadMappings[0].end - 80.25) < 1e-9);
        require(parsed.audioMappings.size() == 1 && parsed.audioMappings[0].start == 50);
    }

    // A snapshot freezes the display while the simulation keeps its state.
    const auto signature = [](const Frame& frame) {
        double sum = 0;
        for (const Shape& shape : frame.shapes)
            for (const Segment& segment : shape.outer.segments) sum += segment.end.x;
        return sum;
    };
    const double liveBefore = signature(engine.frame(480, 360));
    const std::size_t frozenAt = engine.ringCount();
    Snapshot captured = engine.captureSnapshot(480, 360);
    require(captured.width == 480 && captured.height == 360);
    require(captured.rings.size() == frozenAt);
    require(captured.save(snapPath, error));
    Snapshot restored;
    require(restored.load(snapPath, error));
    require(restored.width == captured.width && restored.height == captured.height);
    require(std::abs(restored.zoom - captured.zoom) < 1e-9);
    require(restored.rings.size() == captured.rings.size());
    require(std::abs(restored.rings.front().radius - captured.rings.front().radius) < 1e-6);
    require(std::abs(restored.global.starRatio - captured.global.starRatio) < 1e-9);

    {
        std::ofstream garbage(snapPath, std::ios::trunc);
        garbage << "not a snapshot";
    }
    Snapshot broken;
    require(!broken.load(snapPath, error));
    require(captured.save(snapPath, error));
    require(captured.valid(error));
    Snapshot tooMany = captured;
    tooMany.rings.assign(MAX_RINGS + 1, SnapshotRing{});
    require(!tooMany.valid(error));

    // WHM2 movie round trip: frames decode back to the snapshots that were
    // written, and the header preserves the frame rate and target size.
    {
        const std::string moviePath = (dir / "test.whm").string();
        MovieWriter writer;
        require(writer.open(moviePath, 30, 480, 360, error));
        std::vector<Snapshot> source;
        for (int i = 0; i < 5; ++i) {
            source.push_back(engine.captureSnapshot(480, 360));
            require(writer.write(source.back(), error));
            engine.tick(480, 360);
            engine.frame(480, 360);
        }
        require(writer.frameCount() == 5);
        require(writer.close(error));
        MovieReader movie;
        require(movie.open(moviePath, error));
        require(movie.frameCount() == 5);
        require(std::abs(movie.frameRate() - 30) < 1e-9);
        require(movie.width() == 480 && movie.height() == 360);
        for (std::size_t i = 0; i < source.size(); ++i) {
            Snapshot decoded;
            require(movie.frame(i, decoded, error));
            require(decoded.rings.size() == source[i].rings.size());
            require(std::abs(decoded.rings.front().radius - source[i].rings.front().radius) < 1e-9);
            require(std::abs(decoded.global.starRatio - source[i].global.starRatio) < 1e-9);
        }
        require(!movie.frame(5, broken, error)); // out of range
        MovieReader missing;
        require(!missing.open(moviePath + ".missing", error));
    }

    require(engine.displaySnapshot(captured, 480, 360));
    require(engine.snapshotActive());
    const Frame frozenFrame = engine.frame(480, 360);
    require(!frozenFrame.shapes.empty());
    // The rendered frame is the snapshot's geometry, not the live simulation.
    Engine replica;
    replica.setPatch(patch);
    require(replica.displaySnapshot(captured, 480, 360));
    require(signature(replica.frame(480, 360)) == signature(frozenFrame));
    for (int i = 0; i < 60; ++i) { engine.tick(480, 360); engine.frame(480, 360); }
    require(engine.ringCount() == frozenAt); // display is frozen, not the patch
    require(signature(engine.frame(480, 360)) == signature(frozenFrame));
    require(engine.patch.params[PARAM_RingGrowth].value == paramInfo[PARAM_RingGrowth].max);
    // A snapshot letterboxes into a window of any size.
    require(!engine.frame(900, 500).shapes.empty());
    require(!engine.frame(320, 240).shapes.empty());

    engine.stopSnapshot();
    require(!engine.snapshotActive());
    for (int i = 0; i < 60; ++i) { engine.tick(480, 360); engine.frame(480, 360); }
    require(signature(engine.frame(480, 360)) != liveBefore); // live simulation resumed

    // OSC: packet parsing, address matching, mapping and learn round trips.
    {
        std::vector<OscMessage> empty0;
        require(!parseOsc("", 0, empty0));

        // Helpers to build OSC packets by hand (big-endian, 4-byte lattice).
        // Appends an OSC string: characters, NUL, then pad to a 4 multiple.
        auto appendOscString = [](std::string& out, const std::string& text) {
            out.append(text);
            out.push_back('\0');
            while (out.size() % 4 != 0) out.push_back('\0');
        };
        auto packet = [&](const char* address, char tag) -> std::string {
            std::string out;
            appendOscString(out, address);
            appendOscString(out, std::string(",") + tag);
            return out;
        };
        auto packetValue = [&](const char* address, char tag, double value) -> std::string {
            std::string out = packet(address, tag);
            if (tag == 'f') {
                std::uint32_t bits;
                const float f = static_cast<float>(value);
                std::memcpy(&bits, &f, 4);
                for (int b = 3; b >= 0; --b) out.push_back(static_cast<char>((bits >> (b * 8)) & 0xff));
            } else {
                const std::int32_t n = static_cast<std::int32_t>(value);
                for (int b = 3; b >= 0; --b) out.push_back(static_cast<char>((n >> (b * 8)) & 0xff));
            }
            return out;
        };
        {
            std::vector<OscMessage> parsed;
            const std::string bytes = packetValue("/whorld/test", 'f', 0.5);
            require(parseOsc(bytes.data(), bytes.size(), parsed));
            require(parsed.size() == 1 && parsed[0].address == "/whorld/test");
            require(parsed[0].value == 0.5);
        }
        {
            std::vector<OscMessage> parsed;
            const std::string bytes = packetValue("/whorld/test", 'i', 42);
            require(parseOsc(bytes.data(), bytes.size(), parsed) &&
                    parsed.size() == 1 && parsed[0].value == 42);
        }
        {
            // True/false tags carry no argument bytes.
            std::vector<OscMessage> parsed;
            const std::string on = packet("/whorld/act", 'T');
            require(parseOsc(on.data(), on.size(), parsed) && parsed.size() == 1 &&
                    parsed[0].value == 1.0);
            parsed.clear();
            const std::string off = packet("/whorld/act", 'F');
            require(parseOsc(off.data(), off.size(), parsed) && parsed.size() == 1 &&
                    parsed[0].value == 0.0);
        }
        {
            // An argument-free message still lands, with the default value.
            std::vector<OscMessage> parsed;
            // An address with an empty typetag string (just ",") carries no
            // arguments at all; the message still lands with value 1.
            std::string bare;
            appendOscString(bare, "/whorld/naked");
            appendOscString(bare, ",");
            require(parseOsc(bare.data(), bare.size(), parsed) && parsed.size() == 1 &&
                    parsed[0].address == "/whorld/naked" && parsed[0].value == 1);
        }
        {
            // A bundle of two messages; members share the packet lattice.
            std::string bundle = "#bundle";
            bundle.push_back('\0'); // the literal still needs its OSC terminator
            const unsigned char timetag[8] = { 0,0,0,0, 0,0,0,1 };
            bundle.append(reinterpret_cast<const char*>(timetag), 8);
            std::string one = packetValue("/whorld/a", 'i', 10);
            std::string two = packetValue("/whorld/bb", 'i', 20);
            for (std::string* element : { &one, &two }) {
                std::uint32_t bits = static_cast<std::uint32_t>(element->size());
                for (int b = 3; b >= 0; --b) bundle.push_back(static_cast<char>((bits >> (b * 8)) & 0xff));
                bundle.append(*element);
            }
            std::vector<OscMessage> parsed;
            require(parseOsc(bundle.data(), bundle.size(), parsed));
            require(parsed.size() == 2);
            require(parsed[0].address == "/whorld/a" && parsed[0].value == 10);
            require(parsed[1].address == "/whorld/bb" && parsed[1].value == 20);
        }
        {
            // Malformed bundle (member size overruns the packet) fails cleanly.
            std::string bundle = "#bundle";
            const unsigned char timetag[8] = { 0,0,0,0, 0,0,0,1 };
            bundle.append(reinterpret_cast<const char*>(timetag), 8);
            const char bad[4] = { 0, 0, 0, 64 };
            bundle.append(bad, 4);
            std::vector<OscMessage> parsed;
            require(!parseOsc(bundle.data(), bundle.size(), parsed));
        }
    }
    {
        Playlist oscPlaylist;
        OscMapping mapping;
        mapping.address = "/whorld/zoom*";
        mapping.target = "Zoom";
        mapping.property = "Val";
        mapping.min = 0;
        mapping.max = 100;
        mapping.start = 0;
        mapping.end = 200;
        oscPlaylist.oscMappings.push_back(mapping);
        OscMapping second;
        second.address = "/whorld/clear";
        second.target = "Clear";
        oscPlaylist.oscMappings.push_back(second);
        auto actions = mapOsc(oscPlaylist, "/whorld/zoom/lift", 40);
        require(actions.size() == 1 && actions[0].target == "Zoom" &&
                actions[0].property == "Val");
        require(std::abs(actions[0].amount - 0.8) < 1e-9); // 40% of 0..200%
        require(mapOsc(oscPlaylist, "/whorld/other", 40).empty());
        // Wildcards match by the stored pattern only ('?' one char, '*' any run).
        require(!oscPatternMatch("/whorld", "/whorld/"));
        require(oscPatternMatch("/whorld", "/whorld"));
        require(oscPatternMatch("/whorld?", "/whorld1"));
        require(!oscPatternMatch("/whorld?", "/whorld12"));
        require(oscPatternMatch("*", "/anything") && !oscPatternMatch("*x", "/abc"));
        // Momentary targets fire on non-zero datagrams only.
        actions = mapOsc(oscPlaylist, "/whorld/clear", 1);
        require(actions.size() == 1 && actions[0].target == "Clear");
        require(mapOsc(oscPlaylist, "/whorld/clear", 0).empty());
        // Out-of-domain values clamp into 0..1 before Start/End scaling.
        oscPlaylist.oscMappings.clear();
        OscMapping clamped;
        clamped.address = "/whorld/zoom*";
        clamped.target = "Zoom";
        clamped.property = "Val";
        clamped.min = 0;
        clamped.max = 1;
        clamped.start = 0;
        clamped.end = 500;
        oscPlaylist.oscMappings.push_back(clamped);
        actions = mapOsc(oscPlaylist, "/whorld/zoomx", 9.0);
        require(actions.size() == 1 && std::abs(actions[0].amount - 5.0) < 1e-9); // clamped to 100% of 0..500%
    }
    {
        // Learn replaces the previous binding for the same address.
        Playlist oscPlaylist;
        learnOsc(oscPlaylist, "/whorld/x", "RingGrowth", "Wave");
        learnOsc(oscPlaylist, "/whorld/x", "Zoom", "Val");
        require(oscPlaylist.oscMappings.size() == 1 &&
                oscPlaylist.oscMappings[0].target == "Zoom");
        learnOsc(oscPlaylist, "/whorld/y", "Rings", "Val");
        require(oscPlaylist.oscMappings.size() == 2);
    }
    {
        // OSC sections round trip through the playlist file format.
        const std::string oscWhl = (dir / "osc.whl").string();
        Playlist saved;
        Patch origin;
        require(origin.save((dir / "origin.whp").string(), error));
        saved.patches.push_back((dir / "origin.whp").string());
        OscMapping m;
        m.address = "/whorld/test";
        m.target = "RingGrowth";
        m.property = "PW";
        m.min = -2;
        m.max = 6;
        m.start = 10;
        m.end = 90;
        saved.oscMappings.push_back(m);
        require(saved.save(oscWhl, error));
        Playlist loaded;
        require(loaded.load(oscWhl, error));
        require(loaded.oscMappings.size() == saved.oscMappings.size());
        require(loaded.oscMappings[0].address == "/whorld/test" &&
                loaded.oscMappings[0].target == "RingGrowth" &&
                loaded.oscMappings[0].property == "PW" &&
                loaded.oscMappings[0].min == -2 && loaded.oscMappings[0].max == 6 &&
                loaded.oscMappings[0].start == 10 && loaded.oscMappings[0].end == 90);
    }

    std::cout << "Playlist, MIDI mapping and snapshot roundtrips OK\n";
    fs::remove_all(dir, ec);
    return 0;
}
