#!/usr/bin/env bash
# Performance sweep for the whorld binary.
#
# Runs the same demo patch over a grid of canvas sizes with the offscreen SDL
# driver (no display needed) and, unless --cairo-only, both backends:
#
#   GL:    --renderer auto headless path? No: GL offscreen window via
#          SDL_VIDEODRIVER=offscreen --renderer auto (falls back inside the
#          windowed loop, which is what a VJ run does).
#   cairo: --renderer cairo (explicitly; auto uses GL when post-processing).
#
# WHORLD_TIMING=1 makes each run print every-60-iteration buckets
# "timing ms: tick=.. frame=.. canvas=.. panel=.. present=.. scale=.. gl|cairo
# rings=N"; the sweep reports the mean of each stage plus the wall-clock
# draw rate, and flags stages above 1 ms (rising toward the 33 ms drop
# threshold shown in the on-screen fps meter).
#
# Usage: perf_sweep.sh BINARY [FRAMES] [--cairo-only]
# SPDX-License-Identifier: GPL-3.0-or-later
set -euo pipefail

bin=${1:?usage: perf_sweep.sh BINARY [FRAMES] [--cairo-only]}
shift || true
frames=600
cairoOnly=0
while [ $# -gt 0 ]; do
    case "$1" in
        --cairo-only) cairoOnly=1 ;;
        *) frames=$1 ;;
    esac
    shift
done

work=$(mktemp -d)
trap 'rm -rf "$work"' EXIT
patch=$(dirname "$0")/demo.whp
[ -x "$bin" ] || { echo "perf_sweep: not executable: $bin" >&2; exit 1; }
[ -f "$patch" ] || { echo "perf_sweep: missing demo patch: $patch" >&2; exit 1; }

run_one() { # backend size
    local backend=$1 size=$2 log
    log="$work/run.log"
    if [ "$backend" = cairo ]; then
        SDL_VIDEODRIVER=offscreen WHORLD_TIMING=1 "$bin" --patch "$patch" \
            --renderer cairo --size "$size" --quit-after "$frames" \
            >/dev/null 2>"$log" || { echo "run failed ($backend $size)"; return 1; }
    else
        SDL_VIDEODRIVER=offscreen WHORLD_TIMING=1 "$bin" --patch "$patch" \
            --renderer auto --size "$size" --quit-after "$frames" \
            >/dev/null 2>"$log" || { echo "run failed ($backend $size)"; return 1; }
    fi
    # Mean of the per-60-iteration buckets, ignoring the first (warmup).
    tail -n +2 "$log" | grep '^timing ms:' | awk -v backend="$backend" '
        /^timing ms:/ {
            for (i = 2; i <= NF; ++i) {
                split($i, kv, "=")
                v = kv[2] + 0
                k = kv[1]
                if (k == "tick") tick += v
                else if (k == "frame") frame += v
                else if (k == "canvas") canvas += v
                else if (k == "panel") panel += v
                else if (k == "present") present += v
            }
            for (i = 2; i <= NF; ++i) {
                split($i, kv, "=")
                if (kv[1] == "rings") rings = kv[2] + 0
            }
            ++n
        }
        END {
            if (n == 0) { printf "%-6s  NO BUCKETS\n", backend; exit 1 }
            tick /= n; frame /= n; canvas /= n; panel /= n; present /= n
            total = tick + frame + canvas + panel + present
            printf "%-6s  rings=%-4d  per-iter ms: tick %.2f frame %.2f canvas %.2f panel %.2f present %.2f  total %.2f  (~%.0f fps)\n",
                   backend, rings, tick, frame, canvas, panel, present, total, 1000 / total
        }'
}

echo "perf_sweep: binary=$bin frames=$frames sizes: 640x480, 1280x720, 1920x1080"
printf "%-6s %s\n" size ""
for size in 640x480 1280x720 1920x1080; do
    echo "--- $size"
    if [ "$cairoOnly" = 0 ]; then run_one auto "$size" || true; fi
    run_one cairo "$size" || true
done
echo "done"
