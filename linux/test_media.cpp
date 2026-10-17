// Playlist interchange, MIDI mapping and snapshot round trips.
#include "alsa_midi.h"
#include "midi_actions.h"
#include "midi_decode.h"
#include "movie.h"
#include "playlist.h"
#include "snapshot.h"
#include "whorld_core.h"
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <stdexcept>
#include <string>
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

    std::cout << "Playlist, MIDI mapping and snapshot roundtrips OK\n";
    fs::remove_all(dir, ec);
    return 0;
}
