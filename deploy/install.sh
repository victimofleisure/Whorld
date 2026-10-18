#!/usr/bin/env bash
#
# Whorld kiosk installer for a display-less NUC (KMS/DRM console, no X server).
#
# It builds Whorld with CMake, installs the binary, demo assets, desktop entry,
# icon and the systemd kiosk unit, then reloads systemd and enables the unit.
# It is idempotent and non-destructive: it never deletes anything, and it
# leaves an existing /etc/default/whorld untouched. It does NOT start the kiosk
# (the console may still be in use); the final banner prints how to start it.
#
# Usage:  sudo deploy/install.sh
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd)"
REPO_DIR="$(cd -- "${SCRIPT_DIR}/.." && pwd)"
BUILD_DIR="${REPO_DIR}/build"
PREFIX="/usr/local"
UNIT_NAME="whorld-kiosk.service"
UNIT_DEST="/etc/systemd/system/${UNIT_NAME}"
ENV_DEST="/etc/default/whorld"
KIOSK_USER="whorld"

log()  { printf '==> %s\n' "$*"; }
warn() { printf 'warning: %s\n' "$*" >&2; }

# --- privileges ------------------------------------------------------------
# system files and the systemd unit need root; re-exec through sudo when we
# can, otherwise stop with a clear message rather than failing halfway through.
if [ "$(id -u)" -ne 0 ]; then
    if command -v sudo >/dev/null 2>&1; then
        log "not root; re-running under sudo"
        exec sudo -- "$0" "$@"
    fi
    warn "must run as root to install into ${PREFIX} and /etc; re-run with sudo"
    exit 1
fi

# --- build -----------------------------------------------------------------
JOBS="$(nproc 2>/dev/null || echo 2)"
log "configuring the build in ${BUILD_DIR}"
cmake -S "${REPO_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
log "building whorld with ${JOBS} jobs"
cmake --build "${BUILD_DIR}" -j"${JOBS}"

if [ ! -x "${BUILD_DIR}/whorld" ]; then
    warn "the build did not produce ${BUILD_DIR}/whorld"
    exit 1
fi

# --- kiosk user ------------------------------------------------------------
# Whorld needs the video group (DRM/KMS plus a render node) and the audio and
# input groups (ALSA MIDI, gamepads). Only groups that exist are added, so this
# works on older systems without a render group.
group_list=""
for g in video audio input render; do
    if getent group "$g" >/dev/null 2>&1; then
        group_list="${group_list:+${group_list},}${g}"
    fi
done

if id "$KIOSK_USER" >/dev/null 2>&1; then
    log "user ${KIOSK_USER} already exists; ensuring group membership"
    if [ -n "$group_list" ]; then
        usermod -aG "$group_list" "$KIOSK_USER" \
            || warn "could not update groups for ${KIOSK_USER}; check User=/Group= in ${UNIT_DEST}"
    fi
elif command -v useradd >/dev/null 2>&1; then
    log "creating system user ${KIOSK_USER}"
    useradd_args=(--system --create-home --home-dir "/var/lib/${KIOSK_USER}" --shell /usr/sbin/nologin)
    if [ -n "$group_list" ]; then
        useradd_args+=(--groups "$group_list")
    fi
    useradd "${useradd_args[@]}" "$KIOSK_USER" \
        || warn "could not create ${KIOSK_USER}; create it manually and edit ${UNIT_DEST}"
else
    warn "no useradd available; create a '${KIOSK_USER}' user in the video group by hand"
fi

# --- install files ---------------------------------------------------------
log "installing the binary and assets under ${PREFIX}"
install -d "${PREFIX}/bin" \
           "${PREFIX}/share/whorld" \
           "${PREFIX}/share/applications" \
           "${PREFIX}/share/icons/hicolor/scalable/apps" \
           "${PREFIX}/share/doc/whorld"
install -m 0755 "${BUILD_DIR}/whorld" "${PREFIX}/bin/whorld"
install -m 0644 "${REPO_DIR}/linux/demo.whp" "${REPO_DIR}/linux/demo.whl" \
    "${PREFIX}/share/whorld/"
install -m 0644 "${REPO_DIR}/linux/whorld.desktop" \
    "${PREFIX}/share/applications/whorld.desktop"
install -m 0644 "${REPO_DIR}/linux/whorld.svg" \
    "${PREFIX}/share/icons/hicolor/scalable/apps/whorld.svg"
install -m 0644 "${SCRIPT_DIR}/README.md" \
    "${PREFIX}/share/doc/whorld/DEPLOY.md"

# --- systemd unit and operator environment ---------------------------------
if [ -d /etc/systemd/system ] && command -v systemctl >/dev/null 2>&1; then
    log "installing the systemd unit ${UNIT_DEST}"
    install -m 0644 "${SCRIPT_DIR}/${UNIT_NAME}" "${UNIT_DEST}"

    if [ -f "${ENV_DEST}" ]; then
        log "${ENV_DEST} already exists; leaving it untouched"
    else
        install -m 0644 "${SCRIPT_DIR}/whorld.env" "${ENV_DEST}"
    fi

    systemctl daemon-reload
    if systemctl enable "${UNIT_NAME}"; then
        log "enabled ${UNIT_NAME} (it will start on the next boot)"
    else
        warn "could not enable ${UNIT_NAME}; run: systemctl enable ${UNIT_NAME}"
    fi
else
    warn "no systemd; skipping the kiosk unit. Run whorld directly on the console:"
    warn "    SDL_VIDEODRIVER=kmsdrm ${PREFIX}/bin/whorld --fullscreen --hide-cursor"
fi

# --- summary ---------------------------------------------------------------
cat <<EOF

Installed:
  ${PREFIX}/bin/whorld
  ${PREFIX}/share/whorld/{demo.whp,demo.whl}
  ${PREFIX}/share/applications/whorld.desktop
  ${PREFIX}/share/icons/hicolor/scalable/apps/whorld.svg
  ${PREFIX}/share/doc/whorld/DEPLOY.md
  ${ENV_DEST}          (operator arguments; set WHORLD_ARGS)
  ${UNIT_DEST}

The kiosk was NOT started. To try it now:
  sudo systemctl start ${UNIT_NAME}
  journalctl -u ${UNIT_NAME} -f     # watch for SDL/DRM errors
  sudo systemctl stop ${UNIT_NAME}  # returns tty1 to the getty

This machine may be offline: make sure the SDL2, cairo, ALSA and Mesa
(libgl1-mesa-dri) runtime packages are installed beforehand. See
${PREFIX}/share/doc/whorld/DEPLOY.md for the full first-boot checklist.
EOF
