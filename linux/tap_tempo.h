// Tap tempo with averaging, as in V1 (issues 1.0.47/1.0.50): taps closer than
// the reset gap are averaged over the last few intervals, so a steady tap
// settles on the beat instead of chasing every jittery interval.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

namespace whorld {
struct TapTempo {
    static constexpr int kMaxTaps = 4;        // average the last 3 intervals
    static constexpr double kResetGap = 2.0;  // seconds; a longer pause restarts

    double times[kMaxTaps]{};
    int count = 0;

    void reset() { count = 0; }

    // Records a tap at `now` (seconds, any monotonic clock) and returns the
    // averaged BPM, or 0 while a lone tap is waiting for its partner.
    double tap(double now) {
        if (count > 0 && now - times[count - 1] > kResetGap) count = 0;
        if (count == kMaxTaps) {
            for (int i = 1; i < kMaxTaps; ++i) times[i - 1] = times[i];
            --count;
        }
        times[count++] = now;
        if (count < 2) return 0;
        const double span = times[count - 1] - times[0];
        if (!(span > 0)) return 0;
        return 60.0 * (count - 1) / span;
    }
};
} // namespace whorld
