# Whorld on a display-less NUC (offline kiosk / projection mapping)

This guide turns an Intel NUC (or any x86 box with a GPU and a projector) into a
dedicated, **off-grid** Whorld appliance: it boots straight into a fullscreen
visualizer on the raw KMS/DRM console, with **no X server, no window manager and
no network**. It assumes you can build and copy packages onto the machine before
it goes offline.

Everything here references flags and paths that exist after running
`deploy/install.sh`:

| Path | What it is |
| --- | --- |
| `/usr/local/bin/whorld` | the visualizer binary |
| `/usr/local/share/whorld/demo.whp` | demo patch |
| `/usr/local/share/whorld/demo.whl` | demo playlist (2 patches + CC mappings) |
| `/usr/local/share/doc/whorld/DEPLOY.md` | this document |
| `/etc/default/whorld` | operator arguments (`WHORLD_ARGS`) |
| `/etc/systemd/system/whorld-kiosk.service` | the kiosk unit |

Run the new CLI flags with `whorld --help`; the ones used here are
`--video-driver NAME`, `--fullscreen`, `--hide-cursor`, `--patch`,
`--playlist`, `--midi-source`, `--list-midi`, `--osc-port`, `--warp`,
`--warp-file`, `--renderer` and `--render`.

## 1. Build and install packages ahead of time

Whorld needs SDL2, Cairo, ALSA and OpenGL (Mesa). On the build machine (or the
NUC while it still has internet), install the toolchain and the runtime:

```sh
# Ubuntu/Debian build dependencies
sudo apt-get update
sudo apt-get install -y cmake g++ pkg-config \
    libsdl2-dev libcairo2-dev libasound2-dev libgl1-mesa-dev
```

Then build and install Whorld itself with the helper (it configures, compiles
and installs everything in one idempotent pass):

```sh
cd /path/to/Whorld
sudo deploy/install.sh
```

If you must stay fully offline, download the `.deb` files on another machine
with `apt-get install --download-only` (or `apt-get -d`) and copy them over,
then `sudo dpkg -i ./*.deb`. The **runtime** packages that must be present on
the kiosk are `libsdl2-2.0-0`, `libcairo2`, `libasound2`, `libgl1-mesa-dri`
(the GPU driver — do not forget this one) and `libdrm2`. `install.sh` prints a
reminder and only *warns* when an optional step (systemd) is unavailable, so it
never breaks a half-provisioned machine.

The build itself needs no network. `deploy/install.sh` re-runs `cmake` and
`cmake --build` every time, so a source update is just "copy over, run again".

## 2. Hardware and permissions

Whorld renders through DRM/KMS, so the user that runs it must be able to open
`/dev/dri/card0` (and `/dev/dri/renderD128`). `install.sh` creates a `whorld`
system user and puts it in the right groups; to grant an existing user (for
interactive testing), run:

```sh
sudo usermod -aG video,audio,input,render "$USER"
```

- **video** — `/dev/dri/card*` (the KMS/DRM output Whorld renders to).
- **render** — `/dev/dri/renderD*` render nodes on newer kernels; not required
  for the KMS display path but added by `install.sh` when the group exists.
- **audio** — `/dev/snd/*` for `--audio` and for ALSA MIDI.
- **input** — `/dev/input/event*` for gamepads that SDL reads through evdev.

Log out and back in (or `newgrp video`) for the group change to take effect.
`install.sh` only adds groups that exist, so it is safe on older systems.

## 3. Stop the display manager and free the console

A display manager (GDM/SDDM/LightDM) fights the kiosk for the GPU and the
console. On an appliance you do not want a desktop at all:

```sh
sudo systemctl disable --now gdm3      # or sddm / lightdm
sudo systemctl set-default multi-user.target   # boot to a text console
```

Keep the machine at a plain virtual console (tty1). The kiosk unit owns
`/dev/tty1` and declares `Conflicts=getty@tty1.service`, so the login prompt is
stopped automatically when Whorld starts — you do not need to disable
`getty@tty1` by hand, though `sudo systemctl disable getty@tty1` is also fine.

### Console blanking and DPMS

A projector that loses signal for a few seconds may take a long time to
re-sync, so never let the console blank:

```sh
# Kernel command line: add consoleblank=0 (GRUB_CMDLINE_LINUX in
# /etc/default/grub, then sudo update-grub), e.g.
#   GRUB_CMDLINE_LINUX_DEFAULT="quiet splash consoleblank=0"
sudo setterm -blank 0 -powersave off -powerdown 0 < /dev/tty1
```

`whorld` calls SDL, which asks for the screensaver to be inhibited; on a bare
KMS console the `setterm`/kernel settings above are the reliable belt-and-braces
option. Also disable any hardware DPMS in the NUC firmware if it has one.

## 4. Enable the kiosk service

`deploy/install.sh` installs and **enables** the unit but does not start it.
To run it now and on every boot to tty1:

```sh
sudo systemctl start whorld-kiosk      # start now
sudo systemctl status whorld-kiosk     # is it up?
journalctl -u whorld-kiosk -f          # follow the log (SDL/DRM errors show here)
```

The unit runs:

```
/usr/local/bin/whorld --fullscreen --hide-cursor \
    --patch /usr/local/share/whorld/demo.whp $WHORLD_ARGS
```

with `SDL_VIDEODRIVER=kmsdrm`, `SDL_KMSDRM_DEVICE_INDEX=0` and
`SDL_AUDIODRIVER=alsa` in the environment.

### Operator arguments

Put your show options in `/etc/default/whorld` as a single `WHORLD_ARGS` line,
then `sudo systemctl restart whorld-kiosk`:

```ini
WHORLD_ARGS=--midi-source 24:0 --osc-port 9000
```

Anything from `whorld --help` works (see `deploy/whorld.env` for a commented
list). For one-off changes use a drop-in instead of editing the unit:

```sh
sudo systemctl edit whorld-kiosk      # add e.g. a different ExecStart=
```

If the kiosk shows a black screen or the projector never gets a signal, watch
`journalctl -u whorld-kiosk`. A wrong `--video-driver` or a missing DRM node
prints `SDL initialization failed: ... (available drivers: ...)`, so the log
tells you exactly which drivers this SDL build found.

### Choosing the video driver

`--video-driver NAME` sets `SDL_VIDEODRIVER` before SDL initializes, and it is
the supported way to pick a backend on a machine with no desktop:

- `kmsdrm` — the appliance default: SDL drives the DRM/KMS framebuffer
  directly, no X server.
- `x11` / `wayland` — fall back to one of these if you *are* running a minimal
  desktop (see §5).
- `offscreen` — hidden GPU context; used automatically by `--headless`, and
  handy for verifying the GPU from a shell (see §9).

When neither `--video-driver` nor an environment override is given, SDL picks a
default; the unit always passes `kmsdrm` explicitly.

### Minimal X / Openbox fallback

KMS/DRM is the goal, but some GPU/driver combinations (or an older NUC BIOS)
behave better under a tiny X session. If you need that fallback, install a
minimal stack and start Whorld from the X session, not the systemd unit above:

```sh
sudo apt-get install -y xserver-xorg xinit x11-xserver-utils openbox
```

Then run, from the console (as the kiosk user):

```sh
SDL_VIDEODRIVER=x11 startx /usr/local/bin/whorld -- \
    --fullscreen --hide-cursor --patch /usr/local/share/whorld/demo.whp
```

and keep `xset s off -dpms` in `~/.xinitrc` to stop the X screensaver from
blanking the projector. Everything else in this guide is unchanged; only the
`SDL_VIDEODRIVER` value and the service's launcher differ. If you go this
route, teach the unit to use `x11` (`Environment=SDL_VIDEODRIVER=x11`) or add a
`.xinitrc` autostart and leave the unit disabled.

### Autologin (optional)

The kiosk unit handles the console without a login, so autologin is not
required. If you prefer an interactive tty that starts Whorld for you, override
`getty@tty1` with `--autologin whorld` and launch from the shell profile; keep
it simple and let the systemd unit own the console instead.

## 5. Projection mapping workflow

Whorld warps the rendered image onto a convex quadrilateral so it lands square
on a surface that the projector sees at an angle. The corners are normalized to
`0..1` in the order **top-left, top-right, bottom-right, bottom-left**.

1. **Find the quad.** Project the un-warped image, measure where the corners of
   your target surface fall in the projector's frame, and write them down.
2. **Set it on the command line** for a repeatable show:

   ```sh
   whorld --patch /usr/local/share/whorld/demo.whp \
       --warp "0.06,0.04,0.95,0.10,0.90,0.96,0.10,0.88"
   ```

   or keep four `x y` pairs in a file and use `--warp-file warp.txt`
   (`--warp-save warp.txt` writes a template to start from).
3. **Toggle and confirm at runtime** with the **W** key: the status line shows
   whether warping is on and prints the four corners, so you can verify the
   numbers without restarting.
4. Put the chosen `--warp`/`--warp-file` in `/etc/default/whorld` and restart
   the kiosk.

`--warp` works in the offscreen `--render` path too, so you can preview a warp
on a workstation with just `--render out.png`.

### Useful show controls

While Whorld is running on the console, the keyboard still drives it:
**F11** toggles fullscreen, **H** hides the sidebar/controls, **W** toggles
warp, **M** cycles the mirror/kaleidoscope modes, **B** cycles blending,
**I** toggles color inversion, **T** cycles video-feedback trails, **Space**
pauses, **R** clears, and **Esc** quits. `--hide-cursor` hides the pointer at
launch; moving the mouse brings it back if you need it.

## 6. USB MIDI

Any class-compliant USB-MIDI device works with no per-device driver: the ALSA
`usb-audio` module binds it and it appears in the sequencer. Users in the
**audio** group can reach the ALSA devices.

```sh
sudo usermod -aG audio whorld        # install.sh already does this
aconnect -l                          # list ports once the device is plugged in
whorld --list-midi                   # one line per available input source
```

`whorld --list-midi` prints the `client:port` addresses and names, then exits —
run it once (or from a shell while the kiosk is stopped) to find the address for
your controller, then set it in `/etc/default/whorld`:

```ini
WHORLD_ARGS=--midi-source 24:0
```

`--midi-source CLIENT:PORT` accepts numeric (`24:0`) or named (`"nanoKONTROL2:0"`)
addresses and re-subscribes automatically after a hotplug, so unplugging and
replugging the controller does not need a `aconnect` re-run. Leave it unset and
Whorld creates a subscribable `Whorld:MIDI Input` port instead
(`aconnect 24:0 Whorld:0`). Mappings live in the open playlist's `[Mapping\N]`
sections; use **Ctrl+M** to learn a control, **Ctrl+Shift+M** to forget it and
**Ctrl+Shift+S** to save the playlist. Add `--midi-out --midi-feedback` to echo
mapped values back to a controller with LED rings or motorized faders.

On headless/offline systems with no udev rules the ports are usually
world-readable through the `audio` group; if `--list-midi` reports "no MIDI
sequencer", the kernel module `snd-seq-midi` is missing — install the ALSA
modules (`kmod`/`linux-modules-*-extra`) before going offline.

## 7. Xbox controllers

SDL2 reads gamepads through evdev, so the running user needs the **input**
group (already added by `install.sh`). Plug the pad in and Whorld opens every
SDL-recognised controller automatically, with hotplug. With no `[Gamepad]`
section in the playlist it loads an Xbox-style default layout: left stick zooms
and spins, the right stick reshapes the polygon, the triggers add ring growth /
line width, **A/B/X/Y** pause/clear/fill/outline, **LB/RB** toggle mirror and
inversion, and the D-pad steps the primitive and patch. Mappings are session-only
until you save the playlist. The same **Ctrl+M** learns gamepads too.

Wireless Xbox pads need their USB dongle or Bluetooth stack present at boot;
because the machine is offline, pair them ahead of time and keep the receiver
plugged into a port that is powered before Whorld starts.

## 8. Network-free control (OSC)

If you want to drive Whorld from a phone or a second machine on a local switch,
`--osc-port N` listens on UDP and maps addresses straight onto targets with no
playlist setup: `/whorld/<Target>`, `/whorld/<Target>/<Property>` and
`/whorld/action/<Target>` (for example `/whorld/RingGrowth/Val 0.5` or
`/whorld/action/Clear 1`). Set it in the environment file:

```ini
WHORLD_ARGS=--osc-port 9000
```

No internet is involved — a direct Ethernet cable or a dumb switch is enough.

## 9. Verify the GPU without a display

Before trusting the projector, prove the GPU path works from a shell:

```sh
# hidden GPU context, writes a PNG and exits
/usr/local/bin/whorld --video-driver offscreen --renderer gpu \
    --render /tmp/whorld-check.png --frames 120 --size 1280x720 \
    --patch /usr/local/share/whorld/demo.whp
```

A successful run prints `Saved /tmp/whorld-check.png (N rings)`. This is the
same path the CTest smoke test `whorld_headless_gpu` uses. If it fails, the
error lists the available SDL video drivers. (On a machine that has no display,
`--renderer cairo` also works and needs no GPU at all.)

## 10. First-boot checklist

1. `deploy/install.sh` completed without warnings about systemd or the user.
2. Display manager disabled; default target is `multi-user.target`.
3. `consoleblank=0` on the kernel command line and/or `setterm -blank 0`.
4. `id whorld` shows the `video` group (and `audio`/`input`).
5. `/dev/dri/card0` exists and is readable by the `whorld` user.
6. `systemctl start whorld-kiosk` → fullscreen image on the projector.
7. `journalctl -u whorld-kiosk -f` shows no `SDL initialization failed`.
8. MIDI: `whorld --list-midi` lists the controller; `--midi-source` in
   `/etc/default/whorld`; controller knobs move parameters.
9. Projection: **W** shows the warp corners; the image lands square on the
   surface; the quad is saved in `/etc/default/whorld`.
10. Pointer is hidden at launch; **F11/H/W/M/B/I/T/Space/Esc** still respond.
11. `sudo systemctl restart whorld-kiosk` comes back to the same show.
12. Reboot the NUC and confirm it comes up unattended.

## 11. Troubleshooting

- **`SDL initialization failed: ... (available drivers: ...)`** — the requested
  driver is not built in or a required node is missing. Compare the list with
  your `--video-driver`/unit setting; on a display-less box use `kmsdrm` (unit
  default) or `offscreen` (render checks).
- **Black screen, journal is empty** — the unit may be fighting a display
  manager or getty; confirm `gdm3`/`sddm`/`lightdm` is disabled and that
  `Conflicts=getty@tty1.service` is present.
- **`Permission denied` on `/dev/dri/card0`** — the `whorld` user is missing
  the `video` group; re-run `sudo usermod -aG video whorld` and restart.
- **No MIDI sources** — `snd-seq-midi` not loaded, or the process is not in the
  `audio` group.
- **Projector sleeps / loses sync** — blanking/DPMS is still on; see §3.
- **Mappings are forgotten between reboots** — gamepad/audio defaults are
  session-only; save the playlist with **Ctrl+Shift+S** and start that playlist
  with `--playlist`.
