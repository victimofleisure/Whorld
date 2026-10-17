// Playlist interchange, MIDI mapping and snapshot round trips.
#include "alsa_midi.h"
#include "midi_actions.h"
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
