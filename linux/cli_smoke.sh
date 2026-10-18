#!/usr/bin/env bash
# Command-line smoke / error-path matrix for the whorld binary.
#
# Exercises --help, the malformed/degenerate --warp paths, a missing --config,
# an unknown flag, a --config-save/--config round trip rendered twice with the
# cairo backend (byte-identical, so deterministic on the CPU path) and
# --list-midi. Runs with the offscreen SDL driver and --renderer cairo, so it
# needs no display and no GPU.
#
# Usage: cli_smoke.sh BINARY WORKDIR LINUX_SOURCE_DIR
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail

bin=${1:?usage: cli_smoke.sh BINARY WORKDIR LINUX_SOURCE_DIR}
work=${2:?usage: cli_smoke.sh BINARY WORKDIR LINUX_SOURCE_DIR}
src=${3:?usage: cli_smoke.sh BINARY WORKDIR LINUX_SOURCE_DIR}

patch="$src/demo.whp"
[ -x "$bin" ] || { echo "cli_smoke: not executable: $bin" >&2; exit 1; }
[ -f "$patch" ] || { echo "cli_smoke: missing patch: $patch" >&2; exit 1; }

rm -rf "$work"
mkdir -p "$work"

fail() { echo "cli_smoke: $*" >&2; exit 1; }

# Runs "$@" and requires success (exit 0). Captures output for message checks.
expect_ok() {
    if ! "$@" >"$work/last.out" 2>&1; then
        cat "$work/last.out" >&2
        fail "expected success: $*"
    fi
}

# Runs "$@" and requires a non-zero exit. The command runs inside `if`, so a
# failure does not trip `set -e`; output is captured for a message grep.
expect_fail() {
    if "$@" >"$work/last.out" 2>&1; then
        cat "$work/last.out" >&2
        fail "expected failure: $*"
    fi
}

# 1. --help exits 0 and documents the usage.
expect_ok "$bin" --help
grep -q "Usage: whorld" "$work/last.out" || fail "--help did not print usage"

# 2. A degenerate --warp (all four corners identical) is rejected clearly.
expect_fail "$bin" --warp "0,0,0,0,0,0,0,0"
grep -qi "degenerate" "$work/last.out" || fail "degenerate --warp message unclear"

# 3. A malformed --warp (too few numbers) is rejected clearly.
expect_fail "$bin" --warp "0 0 1 0 1 1 0"
grep -qi "warp" "$work/last.out" || fail "malformed --warp message unclear"

# 4. --config pointing at a missing file is rejected clearly.
expect_fail "$bin" --config "$work/does-not-exist.conf"
grep -qi "config" "$work/last.out" || fail "missing --config message unclear"

# 5. An unknown flag is rejected (an extra value keeps it off the
#    "missing value" path so the "unknown option" branch is exercised).
expect_fail "$bin" --frobnicate value
grep -qi "unknown" "$work/last.out" || fail "unknown flag message unclear"

# 6. --config-save writes a file that --config can reload; two cairo renders of
#    the reloaded show are byte-identical (deterministic, no display or GPU).
conf="$work/show.conf"
expect_ok "$bin" --patch "$patch" --mirror quad --blend screen --morph 1.5 \
    --config-save "$conf"
[ -s "$conf" ] || fail "--config-save did not write $conf"
grep -q "mirror" "$conf" || fail "config-save output is missing the mirror key"

expect_ok "$bin" --config "$conf" --renderer cairo \
    --render "$work/a.png" --frames 30 --size 160x120
expect_ok "$bin" --config "$conf" --renderer cairo \
    --render "$work/b.png" --frames 30 --size 160x120
[ -s "$work/a.png" ] || fail "cairo render did not write a PNG"
cmp -s "$work/a.png" "$work/b.png" || fail "two cairo renders differ (not deterministic)"

# 7. --list-midi exits 0 even when /dev/snd/seq is absent (it prints a clear
#    message and still succeeds), so this is reliable without MIDI hardware.
expect_ok "$bin" --list-midi

echo "cli smoke OK"
