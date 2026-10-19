# Whorld on Linux (native, work in progress)

This directory implements a **native Linux frontend**, separate from the original Windows/MFC/Direct2D app in `trunk/`. It reuses `WhorldDef.h` as the source of truth for parameter definitions and ports the `.whp` patch format, oscillator/animation math and ring geometry. Rendering uses OpenGL when available (stencil-buffer even-odd fills, MSAA, supersampling) and falls back to Cairo; the window is SDL2, and control input supports ALSA MIDI, SDL gamepads and reactive audio. No Wine is required.

## Build and launch

On Ubuntu/Debian:

```sh
sudo apt-get install cmake g++ pkg-config libsdl2-dev libcairo2-dev libasound2-dev libgl1-mesa-dev
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j2
ctest --test-dir build --output-on-failure
./build/whorld --patch linux/demo.whp
```

Launch from an X11 or Wayland desktop session. For a server or SSH session without a display, render an image instead:

```sh
./build/whorld --patch linux/demo.whp --render /tmp/whorld.png --frames 240 --size 1280x720
```

Launch any V2 patch with `--patch /path/to/file.whp`. `--playlist linux/demo.whl` opens a demo playlist (it references `demo.whp` plus two CC mappings), `--snapshot file.whs` starts in snapshot view (`--snapshot` may be repeated and `--snapshot-dir DIR` adds every `*.whs` in the directory, filling the navigation list), and `--midi-source CLIENT:PORT` connects an ALSA MIDI source (otherwise only a subscribable input port is created); `--midi-mode`/`--midi-takeover` handle 14-bit and relative controllers, and `--list-midi` lists the available sources. `--help` lists CLI options. The demo patch and playlist here were created for the port (the upstream checkout does not contain them).

### Sanitizer build

`-DWHORLD_SANITIZE=ON` builds every target with AddressSanitizer and
UndefinedBehaviorSanitizer. It is **off** by default, so the normal build and the
release binaries are untouched:

```sh
cmake -S . -B build-asan -DWHORLD_SANITIZE=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build-asan -j4
ASAN_OPTIONS=detect_leaks=0 ctest --test-dir build-asan --output-on-failure
```

The pure-CPU tests (`whorld_core`, `whorld_media`, `whorld_config`,
`whorld_projection`, `whorld_osc`, `whorld_post`) pass with leak detection left
on. The SDL/OpenGL/ALSA tests link the third-party graphics/audio stack, whose
teardown can leak (the SDL offscreen driver pulls in Mesa/libdrm frames); the
CMake test wiring already sets `ASAN_OPTIONS=detect_leaks=0` for those tests, and
running the whole suite with `ASAN_OPTIONS=detect_leaks=0` does the same, so a
third-party leak never fails the sanitized run. Use
`LSAN_OPTIONS=suppressions=FILE` if you want to keep leak checking on and only
silence the third-party frames.

### Movies

--export-all DIR renders every loaded snapshot to `DIR/snapshot-001.png`, `snapshot-002.png`, ... on the deterministic cairo path (no video overlay, no feedback trails) — the parity equivalent of the upstream Snapshot|Export All menu.

`--movie-out file.whm` records the running simulation to a WHM2 movie (`--frames N` frames at `--movie-rate FPS`, default 60) without needing a display. `--movie file.whm` plays a movie back, advancing frames by wall clock and looping; **Space** returns to the live simulation. `--movie file.whm --movie-frame N --render out.png` renders a single movie frame to an image. The format is the upstream WHM2 container: a 40-byte header (frame rate, target size, frame count, index offset), the same snapshot records used by `.whs` files, and a trailing array of 64-bit frame offsets.

### Video overlay

Arbitrary movie files (not WHM2) can run as a video layer beneath the rings: `--video FILE` opens a decoded clip that is letterboxed over the full canvas and blended *before* post-processing (mirror/warp/invert operate on the composite, on both the OpenGL and Cairo backends). `--video-blend normal|add|screen|multiply` selects the compositing operator and `--video-alpha F` (0-1) scales the overlay. **Ctrl+V** toggles the overlay; **Ctrl+J** cycles through every video entry in the playlist, auto-advancing on the master tempo (interval = 60 / BPM, minimum 0.2 s). A playlist row whose path ends in `.avi/.mp4/.m4v/.mkv/.webm/.mov/.mpg/.mpeg/.ts` opens the overlay instead of the patch loader (**Enter** plays it like any other row).

The overlay is built on ffmpeg: when the build finds `libavcodec/libavformat/libavutil/libswscale`, the decode runs on its own thread paced by the clip's frame rate and re-opens at EOF to loop; without those libraries the feature compiles out and `--video`/`Ctrl+V` reports that ffmpeg support is unavailable (everything else is unaffected). For deterministic output, `--video-frame N` decodes exactly frame *N* for `--render` (the `whorld_video` test drives this with an ffmpeg-generated fixture). The overlay is *not* baked into WHM2 recordings; live rendering and streaming paths include it.

### Movie export (H.264/.avi/.mp4)

`--movie-export FILE.avi|.mp4|.mov|.mkv` renders the same canvas the live viewer draws - including post-processing and blending - and encodes it with ffmpeg's H.264 encoder into the container implied by the extension, at `--movie-rate FPS` for `--frames N` frames. The CPU/cairo path is deterministic on any machine; `--feedback` trails (and `--renderer gpu`) switch to the offscreen OpenGL renderer with identical shader post-processing. Requires the ffmpeg dev libraries; without them the flag reports that video export is unavailable (WHM2 recording is independent). The `whorld_movie_export`/`whorld_movie_export_roundtrip` CTest pair records a clip and re-decodes it through the video overlay layer.

### Renderer selection

`--renderer auto|gpu|cairo` picks the canvas backend (`auto` is the default). The OpenGL backend draws each ring with the stencil buffer so the even-odd (alternate) fill matches Direct2D exactly, into a multisampled offscreen framebuffer that is resolved and composited; it is typically several times faster than the Cairo path at high ring counts. `gpu` fails fast if OpenGL is unavailable; `auto` quietly falls back to Cairo. The sidebar is always drawn with Cairo (for crisp text) and uploaded as a texture.

`--blend normal|add|screen` selects how the rings are composited while the canvas is drawn (`normal` is the default alpha-over; `add` is an additive glow, `screen` a brightened wash), on both the OpenGL and Cairo backends. `--morph SECONDS` crossfades between playlist patches over that many seconds during playback instead of switching abruptly (`0`, the default, is instant). A manual A/B crossfader is available: load side B with **Ctrl+X** (typed `.whp` filename) or launch with `--crossfade-b FILE.whp` and optionally `--crossfade-pos 0..1`; **Z**/**X** nudge the fader 25% toward A/B, **Ctrl+Shift+X** turns it off (back to side A). A basic docking control: **Ctrl+D** flips the control pane between the left and right edges (persisted in the config `panelSide` key, settable via `--panel-side left|right`). Continuous patch fields blend linearly (`Engine::mixPatch` in `linux/whorld_core.cpp`); discrete fields switch at the halfway point, matching the timed morph.

`--scale F` (or the **s**/**-**, **=** keys while running) sets the canvas render scale from 0.25 to 2.0. The Cairo backend rasterizes the canvas at that fraction of its on-screen size; the OpenGL backend supersamples at that multiple and downsamples on compositing, so values above 1.0 trade GPU time for smoother edges. With no `--scale`, the Cairo backend adapts the scale automatically to stay within the frame budget while the OpenGL backend stays at 1.0.

### Show config file

`--config FILE` reads startup options from a human-editable `key = value` file so a kiosk can start from one readable file instead of a long `WHORLD_ARGS` line. The format is the same INI-ish style as `.whl`: one entry per line, `#` or `;` starts a full-line comment, blank lines are ignored, surrounding whitespace is trimmed and keys are case-sensitive. Values are *defaults* — any flag you also pass on the command line overrides the file — and unknown keys are preserved when a config is rewritten. `deploy/whorld.conf` is an annotated example.

```ini
# /usr/local/share/whorld/whorld.conf
patch = /usr/local/share/whorld/demo.whp
renderer = gpu
mirror = quad
warp = 0.06,0.04,0.95,0.10,0.90,0.96,0.10,0.88
fullscreen = true
hideCursor = true
midiSource = 24:0
midiMode = 14bit
oscDirectPort = 9000
```

| Key | Value |
| --- | --- |
| `patch`, `playlist` | paths, as `--patch` / `--playlist` |
| `renderer` | `auto` \| `gpu` \| `cairo` |
| `scale` | `0` (adaptive) or `0.25`..`2.0` |
| `mirror` | `off` \| `quad` \| `kaleidoscope` \| `radial` |
| `invert` | `true` \| `false` |
| `feedback` | `0.0`..`0.99` |
| `blend` | `normal` \| `add` \| `screen` |
| `morph` | seconds, `0`..`3600` |
| `panelSide` | `left` or `right`; which edge the control pane docks to (Ctrl+D) |
| `warp`, `warpFile` | the `--warp` 8-number string, or a four-`x y`-pair file |
| `videoDriver` | `SDL_VIDEODRIVER` name (`kmsdrm`, `x11`, ...) |
| `fullscreen`, `hideCursor` | `true` \| `false` |
| `midiSource` | `CLIENT:PORT` |
| `midiMode` | `absolute` \| `14bit` \| `rel-twos` \| `rel-sign` \| `rel-binary` |
| `midiTakeover`, `midiFeedback`, `midiClock` | `true` \| `false` |
| `midiOut` | `CLIENT:PORT` (empty = virtual output port) |
| `midiOutEnabled` | `true` \| `false` |
| `osc`, `oscPort` | `--osc` mapping mode and its UDP port |
| `oscDirect`, `oscDirectPort` | `--osc-port` convention addressing and its port |
| `audio`, `audioDevice`, `audioGain` | microphone/line input options |

Booleans accept `true`/`false`, `yes`/`no`, `on`/`off` and `1`/`0`. `--config-save FILE` writes the effective settings of the current command line to `FILE` (every key, so the file is self-documenting) and exits, which is the easy way to capture a working setup: run the show you want with `--config-save /tmp/show.conf`, then review and use it with `--config`. With no `--config`, startup is unchanged.

## Shape primitives

A ring can be drawn as one of seven shape primitives instead of the default polygon. The `Primitive` parameter selects the shape, and each primitive has its own adjustment parameters. Like every other geometry parameter, the shape is captured when a ring is born, so changing the selector or a control affects newly created rings while existing rings keep their shape.

| Primitive | Controls |
| --- | --- |
| Polygon | `PolyCorner` (corner rounding); the upstream `PolySides`, `StarFactor`, `Pinwheel`, `EvenCurve`, `OddCurve` and shear parameters still apply |
| Circle | `CircleStart`, `CircleSweep` (a true bezier circle; a sweep below 360 draws a pie slice) |
| Square | `SquareCorner` |
| Triangle | `TriCorner` |
| Star | `StarPoints`, `StarInner`, `StarSharp` |
| Cross | `CrossWidth`, `CrossLength` |
| Line | `BarLength`, `BarThickness` |

A row of shape chips sits above the parameter list. Click a chip to activate that primitive; the list below then shows only that shape's controls, so switching from the star to the cross swaps the star controls for the cross controls. The `Primitive` row is still an ordinary parameter (in the list and in `.whp`/mapping tables), so it can be driven by an oscillator or mapped to hardware; `NextShape`/`PrevShape` are button-friendly targets that step through the primitives one press at a time. The shape parameters are saved in `.whp` patches (the Windows build ignores the unknown names and falls back to a polygon). Upstream polygon parameters stay visible but are dimmed while another primitive is active.

Snapshots and movies stay byte-compatible with Windows: the ring struct has no spare room, so a primitive is written as the closest regular polygon (a circle becomes a 50-sided polygon, a star becomes a `StarPoints`-sided star, a square/triangle/cross/line becomes a 4-sided polygon). The live simulation keeps the exact shape; only saved files are approximated.

## Input devices

Whorld has always accepted ALSA MIDI; this port adds gamepads and audio-reactive input. All three drive the same targets: any parameter row (with its `Val`/`Wave`/`Amp`/`Freq`/`PW`/`Global` properties), the ten masters, and the misc actions (`Fill`, `Outline`, `Clear`, `Pause`, ...). Mapped rows show a small green dot in the sidebar.

### MIDI controllers

`--midi-source CLIENT:PORT` connects an ALSA MIDI source; run without it and patch into the created `Whorld:MIDI Input` port instead (`aconnect CLIENT:PORT Whorld:0`). Use `--list-midi` to print the available ALSA sequencer input sources (`client:port` plus names) and exit. When `--midi-source` is given, Whorld retries the subscription every couple of seconds, so a device that is unplugged and replugged (or that was missing at launch) reattaches automatically; the status bar reports the reconnection. Mappings live in the open playlist's `[Mapping\N]` sections:

```ini
[Mapping\0]
Event=CONTROL
Target=RingGrowth
Prop=Val
Channel=0
Control=1
Start=0
End=100
```

`Event` is `CONTROL`, `NOTE_ON`, `NOTE_OFF`, `KEY_AFT`, `PATCH`, `CHAN_AFT` or `WHEEL`; `Start`/`End` are percentages, so a knob can be scaled or inverted.

**MIDI learn:** select a parameter or master row, press **Ctrl+M**, then move a control. The next message is bound to that row's current Tab property (`Val` by default), replacing any previous mapping for the same control. **Ctrl+Shift+M** forgets the mappings for the selected row. Save the playlist (**Ctrl+Shift+S**) to keep them.

### Choosing MIDI hardware

The mappable surface is **65 targets**: 35 parameters (each with the six properties `Val`/`Wave`/`Amp`/`Freq`/`PW`/`Global`), 10 masters (`Speed`, `Zoom`, `Damping`, `Trail`, `Rings`, `Tempo`, `HueSpan`, `CanvasScale`, `Copies`, `Spread`) and 20 misc targets (`OriginX/Y`, `Fill`, `Outline`, `Mirror`, `OriginDrag`, `OriginRandom`, `Reverse`, `Convex`, `LoopHue`, `ZoomCenter`, `Antialiasing`, `RandomPhase`, `RandomOrigin`, `Clear`, `Pause`, `Patch`, `NextShape`, `PrevShape`, `InvertColor`). One physical control drives one `(target, property)` pair, so a "complete" surface is ~65 controls -- but mappings are saved per playlist and re-learned at runtime, so a small surface is re-used across playlists and layers/banks multiply it.

| Tier | Knobs | Buttons/pads | Faders | Covers |
| --- | --- | --- | --- | --- |
| Minimum | 8 | 8 | 0-8 | a core live set (~8 parameters plus Pause/Clear/Fill/Mirror) |
| Sweet spot | 16 | 16 | 0-8 | all the expressive parameters and most BOOL/action targets |
| Complete | 24 | 24+ | 8-9 | the 35 parameters, 10 masters and 20 misc targets, all live |

By default Whorld reads the raw CC value (0-127) as **absolute**, so faders, pots and absolute-mode encoders work unchanged. `--midi-mode` selects other decoders:

| `--midi-mode` | Reads | Use for |
| --- | --- | --- |
| `absolute` (default) | CC 0-127 as a position | faders, pots, absolute-mode encoders |
| `14bit` | MSB (CC 0-31) + LSB (CC 32-63) as 0-16383 | high-resolution faders/knobs |
| `rel-twos` | two's-complement encoder deltas | relative encoders reporting `1..63` / `65..127` |
| `rel-sign` | sign-magnitude encoder deltas (bit 6 is the sign) | encoders using a sign bit |
| `rel-binary` | binary-offset encoder deltas (`64` = 0) | encoders offset around 64 |

`--midi-takeover` adds **soft takeover** (pickup) for absolute and 14-bit controls: a control that has not reached the target's current value yet is ignored, so switching a fader onto a parameter never makes it jump; it engages as soon as the control is moved to (or past) that value and then tracks normally. Relative modes are inherently relative and ignore takeover. Pads/buttons may send Note On/Off or CC (every `Event` type above is accepted).

**MIDI clock (`--midi-clock`)**: follows an external clock (Ableton Live, Traktor, a synth, ...) as the master tempo. Whorld listens for the 24-PPQN clock plus Start/Continue/Stop and drives `Tempo` from the measured BPM (smoothed over the pulse intervals, clamped to 20-300); the status bar shows `BPM <value> (run)` or `(stopped)` while the clock is active. ~1 s without a pulse (or a MIDI Stop plus silence) stops the override and Whorld keeps the last tempo until the clock returns. The tempo only changes when the flag is given.

**MIDI output (`--midi-out [CLIENT:PORT]`)**: creates an ALSA MIDI output so a controller can be driven back. With no argument Whorld creates a virtual `Whorld:MIDI Output` port that you connect with `aconnect Whorld:MIDI Output 'Device':0` (or pass `--midi-out 'Device':0` to connect at launch). Add `--midi-feedback` to echo each mapped incoming CC (and mapped Note On/Off) straight back out; a controller with LED rings, button lamps or motorized faders then reflects the real parameter value. Only controls that actually appear in the playlist `[Mapping]`s are echoed, and clock/transport is never echoed.

Control input goes through the ALSA sequencer, so **any USB-MIDI class-compliant device works with no per-device driver** — the kernel's `snd-usb-audio` module binds it and it appears in `aconnect -l`. Only the *optional* configuration editors differ by platform:

| Controller | Knobs / buttons / faders | Configuration editor | Editor on Linux |
| --- | --- | --- | --- |
| Akai MIDImix | 24 / 16 / 9 | none (fixed CC map) | n/a — zero config |
| Korg nanoKONTROL2 | 8 / 24 / 8 | KORG KONTROL Editor | no (Windows/macOS only) |
| Behringer X-Touch Mini | 8 / 16 / 1 | X-Touch Editor | no (Windows/macOS); on-device MIDI-mode switch works |
| Novation Launch Control XL 3 | 24 / 43 / 8 | Novation Components (browser) | yes (Chrome WebMIDI), though XL3 is reported finicky |
| Faderfox EC4 / PC12 | 16 / 7 / 0  and  72 / — / 0 | Faderfox editor (browser) | yes (Chrome WebMIDI) |
| Intech Studio Grid EN16 + BU16 | 16 / 0 / 0  and  0 / 16 / 0 | Grid Editor | yes (web app) |
| DJ TechTools Midi Fighter Twister | 16 / 16 / 0 | Midi Fighter Utility | no (Windows/macOS only) |

**MIDI learn makes the editors optional.** Because **Ctrl+M** binds whatever a control sends, you can wire up a fixed-map box (MIDImix, nanoKONTROL2) or an unknown default map entirely from the Whorld side. An editor is only needed to change the *controller's own* CC numbers, LED behaviour or encoder mode — most relevant for the Twister (confirm it is in absolute CC mode).

No device auto-connects: after plugging in, run `aconnect 'Device':0 Whorld:0`, or launch with `--midi-source 'Device':0` (both numeric `24:0` and named addresses work). With `--midi-source` Whorld re-subscribes automatically after a hotplug, so `aconnect` does not need re-running.

Verified on the port's reference machine (Ubuntu, kernel 6.8): `snd-usb-audio` and `snd-seq-midi` are available (`CONFIG_SND_USB_AUDIO_MIDI_V2=y`), `/dev/snd/seq` is present, and `--midi-source` accepts numeric and named ALSA addresses.

### Gamepads

Any number of controllers SDL recognises work out of the box, with hotplug; every connected pad shares the same mapping set, and a removed pad is closed cleanly. Mappings live in the playlist's `[Gamepad\N]` sections:

```ini
[Gamepad\0]
Control=LX
Target=Zoom
Prop=Val
Start=25
End=80
```

`Control` is an axis (`LX`, `LY`, `RX`, `RY`, `LTrigger`, `RTrigger`) or a button (`A`, `B`, `X`, `Y`, `LB`, `RB`, `Back`, `Start`, `Guide`, `LStick`, `RStick`, `DPadUp`, `DPadDown`, `DPadLeft`, `DPadRight`). Axes are continuous (sticks run -1..1 and span `Start`..`End`); buttons act on their edges, so one bound to `Clear`/`Pause`/`Patch` fires once per press and one bound to `Fill`/`Outline` stays on while held.

With no `[Gamepad]` section in the playlist and at least one pad connected at launch, a standard Xbox-style default profile is loaded automatically: left stick zooms and spins, the right stick reshapes the polygon, the triggers add ring growth / line width, **A/B/X/Y** pause, clear, fill and outline, **LB/RB** toggle mirror and colour inversion, and the D-pad steps the primitive, selects a patch and re-randomizes the origin. The defaults are session-only until you save the playlist. Stick axes get a dead-zone and response curve so a resting stick reads exactly zero (and holds its mapping's `Start` value), and momentary actions pulse the controller's rumble where the pad supports it.

**Gamepad learn:** the same **Ctrl+M** used for MIDI also learns gamepads. Select a parameter or master row, press **Ctrl+M**, then move a stick past its deadzone or press a button; the control is bound to that row's current Tab property, replacing any previous mapping for the same control, and the mapped action does not also fire on the learn frame. **Ctrl+Shift+M** still forgets the mappings for the selected row, and **Ctrl+Shift+S** saves the playlist to keep them.

### Microphone / line input (audio-reactive)

`--audio` enables capture from the ALSA `default` device; `--audio-device NAME` selects another and `--audio-gain F` (default 1.0) scales the analysis. Each frame the input is reduced to a level plus low (<250 Hz), mid (250-2000 Hz) and high (2-12 kHz) bands, applied through the playlist's `[Audio\N]` sections:

```ini
[Audio\0]
Feature=Level
Target=RingGrowth
Prop=Val
Start=0
End=100
```

`Feature` is `Level`, `Low`, `Mid`, `High` or `Beat`. `Beat` is an onset pulse: it jumps to 1 when a transient is detected and decays over a few frames, so it can drive a parameter range continuously. For a momentary action (`Clear`, `Patch`, `NextShape`, ...) a `Beat` mapping fires once per onset instead of on every frame of the pulse. With `--audio` and no `[Audio]` section in the playlist, a default set is used (level -> `RingGrowth`, low -> `Zoom`, high -> `RotateSpeed`); `Beat` is deliberately not in the defaults, so the out-of-the-box output is unchanged. Raise `--audio-gain` for quiet sources.

The `[Gamepad]` and `[Audio]` sections are a Linux-port extension: upstream readers ignore them, and the port only writes them when they are non-empty.

### Open Sound Control (OSC)

OSC 1.0 messages arrive over UDP (bundled and nested messages work, and one datagram may carry a bundle). The first numeric or boolean argument is the value; an argument-free message maps to 1, which makes pulses from anything comfortable: `/whorld/clear 1` pressed once in TouchOSC clears once. There are two ways to map addresses onto targets:

**Direct addressing** — `--osc-port N` listens on UDP port `N` and needs no playlist setup: the address names the target. `/whorld/<Target>` uses the property `Val`, `/whorld/<Target>/<Property>` names a parameter property (`Val`/`Wave`/`Amp`/`Freq`/`PW`/`Global`), and `/whorld/action/<Target>` fires any nonzero argument (a pulse) at an action. Targets and properties are the same case-sensitive names the tables in the sidebar use (for example `/whorld/RingGrowth/Wave`). The value is taken from the first float argument, or the first integer if there is none, is clipped to 0..1 and scaled by 100 percent, so `/whorld/RingGrowth/Val 0.5` sets the parameter to the middle of its range; a message with no numeric argument is full-on. Momentary targets (`Clear`, `Pause`, ...) stay silent on zero. For example `oscsend localhost 9000 /whorld/RingGrowth/Val f 0.5` and `oscsend localhost 9000 /whorld/action/Clear i 1`.

**Playlist mappings** — `--osc [PORT]` listens on UDP (default port 8000) and applies messages through the playlist's `[Osc\N]` sections, which support wildcards, learn and non-0..1 domains:

```ini
[Osc\0]
Address=/whorld/ringgrowth
Target=RingGrowth
Prop=Val
Min=0
Max=1
Start=0
End=100
```

`Address` is matched against the message address with the OSC wildcards `?` (one character) and `*` (any run); the port does not implement `[]` character sets or `{}` alternation, so use a `*` between literal fragments instead. The value is placed into `Min`..`Max` (defaults 0..1, so a 0..127 sender needs `Min=0`, `Max=127`), clipped to 0..1, and then scaled onto `Start`/`End` exactly like the other inputs. Momentary targets (`Clear`, `Pause`, `Patch`, ...) fire when a datagram carries a non-zero value and stay silent on zero, since senders normally transmit only on change.

**OSC learn:** **Ctrl+M** also learns OSC. With `--osc` running, select a row, press **Ctrl+M**, then send any message from your controller; its full address is bound to the row's current Tab property (rebinding the same address, as the other inputs do), and **Ctrl+Shift+S** saves the playlist to keep it. The exact incoming address is stored, so wildcards only come from hand-written mappings.

Because the listener is a plain UDP socket, anything that can speak OSC 1.0 works: TouchOSC/Lemur on phones, Max, TouchDesigner, Reaper, and command-line tools such as `oscsend` (from `oscoutils`): `oscsend localhost 8000 /whorld/ringgrowth f 0.5`, or with direct addressing `oscsend localhost 9000 /whorld/RingGrowth/Val f 0.5`.

## Desktop launcher

`linux/whorld.desktop` plus `linux/whorld.svg` are installed by `cmake --install`, so Whorld appears in the desktop application menu with an icon:

```sh
cmake --install build --prefix ~/.local   # no sudo; user-level install
```

That puts the binary in `~/.local/bin` (add it to `PATH` if the launcher does not start), the launcher in `~/.local/share/applications/` and the icon in `~/.local/share/icons/hicolor/scalable/apps/`. A system-wide install is `sudo cmake --install build`. The launcher runs plain `whorld`; pass flags like `--patch` from a terminal when needed.

## Kiosk deployment (no desktop, KMS/DRM)

For a dedicated, offline appliance the port can run on the bare KMS/DRM console with no X server. Three flags support that:

- `--video-driver NAME` sets `SDL_VIDEODRIVER` before any `SDL_Init`, so a kiosk can choose `kmsdrm` (DRM/KMS framebuffer, no X), `x11`, `wayland` or `offscreen`. With `--headless` and no explicit driver, the `offscreen` driver is selected when this SDL build provides it (otherwise SDL's default is left alone). An unknown driver fails with the list of drivers the build actually has, e.g. `SDL initialization failed: bogus not available (available drivers: x11, wayland, KMSDRM, offscreen, dummy, evdev)`.
- `--fullscreen` starts the window in `SDL_WINDOW_FULLSCREEN_DESKTOP`, so it comes up fullscreen with no window manager; **F11** still toggles it at runtime.
- `--hide-cursor` hides the mouse pointer (`SDL_ShowCursor(SDL_DISABLE)`) for projection; moving the mouse brings it back. The other shortcuts (**H**, **F11**, **W**, ...) keep working.

A repeatable display-less GPU check (no window, writes a PNG and exits):

```sh
SDL_VIDEODRIVER=offscreen ./build/whorld --renderer gpu \
    --render /tmp/check.png --frames 5 --size 160x120 --patch linux/demo.whp
```

The `deploy/` directory holds a systemd unit (`whorld-kiosk.service`), an idempotent installer (`install.sh`) and an off-grid NUC guide (`README.md`) covering console blanking, stopping the display manager, USB MIDI, Xbox controllers, OSC, projection mapping and a first-boot checklist.

## Controls

- Click a parameter or master row, use **Left/Right** to edit and **Shift+Left/Right** for larger steps; **Up/Down** selects a row. **Tab** cycles between base value, oscillator waveform, amplitude, frequency, pulse width and global offset. Mouse wheel scrolls the panel.
- **F** toggles filled rings; **O** toggles outlines; **Space** pauses (or returns to live view from a snapshot); **R** clears rings & feedback trails; **F11** toggles fullscreen; **H** hides the controls; **Esc** quits. Drag the canvas to set the origin.
- **M** cycles post-processing symmetry: Off -> 4-Way Quad Mirror -> 8-Way Kaleidoscope -> 12-Way Radial Kaleidoscope.
- **B** cycles geometry blending: Normal -> Additive -> Screen.
- **I** toggles color inversion.
- **W** toggles projection-mapping warp, which maps the frame onto a convex quadrilateral set with `--warp "TLx,TLy,TRx,TRy,BRx,BRy,BLx,BLy"` (or `--warp-file`); the status line shows whether it is on and the corners.
- **T** cycles video feedback trails (Off -> 0.70 -> 0.85 -> 0.93 decay).
- **Ctrl+O** / **Ctrl+S** opens/saves a `.whp` patch: type the filename into the box and press Enter (Esc cancels). **P** writes `whorld.png` into the current directory.
- **Ctrl+T** saves a snapshot to a typed filename (default `snapshot.whs`); **Ctrl+L** loads one; **PgUp/PgDn** step through the snapshots loaded in this session; **G** toggles the slideshow (auto-advance across the loaded snapshots at `--slideshow-seconds N`, default 5, wrapping at the end). While a snapshot is on screen the view is frozen; **Space** returns to the live simulation and stops the slideshow. The manual A/B crossfader: **Z**/**X** nudge toward A/B (25% per press), **Ctrl+X** loads side B, **Ctrl+Shift+X** exits.
- **L** toggles the playlist bar. In the playlist: **Enter** plays the selected patch, **Del** removes it, **[**/**]** steps through patches, and **Ctrl+Shift+O**/**S**/**N** open/save/new a `.whl` playlist, **Ctrl+Shift+A** adds the current patch, **Up/Down** move the selection.
- **Ctrl+V** toggles the video overlay (opened from a video playlist row or `--video`); **Ctrl+J** cycles video entries in the playlist, auto-advancing on the master tempo. See **Video overlay** above.
- MIDI CC/notes are mapped through the mappings stored in the open playlist (`mapMidi` in `playlist.cpp`); targets include parameter rows (Val/Wave/Amp/Freq/PW/Global) and the master/misc actions. Gamepads and audio input use the same targets via the `[Gamepad]`/`[Audio]` sections; see **Input devices** above.

## Port status

Live geometry, straight and curved rings, shape primitives (`linux/shape_def.h`: polygon, circle, square, triangle, star, cross, line), color and alpha, `.whp` load/save, basic parameter editing, fullscreen and PNG output are supported. Playlists (`.whl`), snapshots (`.whs`), snapshot viewing, WHM2 movies (`.whm`, record/playback/single-frame render) and ALSA MIDI input are supported; see `linux/playlist.cpp`, `linux/snapshot.cpp`, `linux/movie.cpp`, `linux/midi_actions.cpp` and `linux/alsa_midi.cpp`. Canvas rendering runs on the GPU through OpenGL when available (`linux/gl_renderer.cpp`) with a Cairo fallback (`linux/cairo_canvas.cpp`). The full Windows docking UI is only partially ported: the control pane can dock on the left or right edge (Ctrl+D / `--panel-side`), while arbitrary toolbars and floating panes are **not** ported yet. The Cairo fallback runs on the CPU and can be slower than Direct2D at high ring counts; select the OpenGL backend for those. `Mirror` (off, 4-way quad mirror, 8-way kaleidoscope, and 12-way radial modes) and `InvertColor` are now rendered via GLSL post-processing shaders (`linux/gl_renderer.cpp`), alongside configurable video feedback trails (`--feedback 0.0-0.99`, `--feedback-zoom`, `--feedback-rot`), additive/screen geometry blending (`--blend`, key **B**) and patch crossfades during playlist playback (`--morph`, see `Engine::morphTo` in `linux/whorld_core.cpp`). Projection mapping warps the image onto a convex quadrilateral with the same per-pixel transform on both backends (`--warp`, `--warp-file`, `--warp-save`; `linux/projection.cpp`). Real-time inter-service video streaming is supported via V4L2 virtual cameras (`--v4l2 /dev/video0`) and raw RGB pipes (`--pipe-out`). OSC input over UDP (`--osc [PORT]` and direct `--osc-port N`) binds addresses to the same 65 targets as MIDI, with `?`/`*` wildcards, Ctrl+M learn and a mapping-free address convention (`linux/osc.cpp`). Headless offscreen GPU rendering/streaming is supported via `--headless`, and a display-less appliance can select the SDL driver (`--video-driver NAME`, e.g. `kmsdrm`), start fullscreen (`--fullscreen`) and hide the pointer (`--hide-cursor`); see `deploy/`. Patches, playlists, snapshots and movies from an untrusted source are validated and size-bounded to keep malformed values from hanging the application.

Tests: `whorld_core` (patch/mapping math and patch morphing), `whorld_media` (playlist, MIDI/14-bit/relative-encoder/soft-takeover and external-clock logic, CC encoding, gamepad/audio mappings, snapshot and movie round trips; also the exhaustive relative-encoder tables and a fixed-seed random soft-takeover sweep), `whorld_audio` (FFT band and onset/beat analysis, `Beat` audio mapping), OSC packet parsing and `[Osc]`-mapping tests (inside `whorld_media`), `whorld_osc` (packet/bundle parsing, convention addressing, a localhost UDP loopback and fixed-seed byte/address fuzzing), `whorld_projection` (quad/homography math for `--warp`, plus fixed-seed random-convex-quad property tests and `warpFromString` fuzzing), `whorld_config` (show-config INI parser, typed getters, unknown-key preservation, a `ShowConfig` round trip and fixed-seed garbage-input/round-trip fuzzing), the headless PNG renders `whorld_image`, `whorld_playlist_image`, `whorld_snapshot_image`, `whorld_warp_image` and `whorld_movie_frame`, the `--config-save`/`--config` smoke pair `whorld_config_save` + `whorld_config_image`, the `whorld_post` CPU golden tests (exact mirror/kaleidoscope/warp/invert pixel expectations), the `whorld_cli_smoke` error-path matrix (`--help`, malformed `--warp`, missing `--config`, unknown flag, a deterministic `--config-save`/`--config` cairo render pair and `--list-midi`), the video overlay test `whorld_video` (deterministic ffmpeg decode, letterboxing, pacing and looping; skipped without ffmpeg support), the display-less offscreen GPU render `whorld_headless_gpu`, the movie recording test `whorld_movie`, the `whorld_list_midi` discovery smoke test, and `whorld_gpu` (which renders the demo patch with both backends — including additive and screen blending — and fails if the OpenGL output diverges from Cairo; it skips when no OpenGL context is available). All are driven by CTest.

The upstream license is GPL; see `../LICENSE`.
