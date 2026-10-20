// Random patch generation (V1 F10): geometry and oscillators are randomized,
// while tempo and the performance-sensitive extremes stay in a usable band.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "midi_actions.h" // denormalizeMaster: the upstream slider mapping
#include "whorld_core.h"
#include <random>

namespace whorld {
inline void randomizePatch(Engine& engine, unsigned seed) {
    std::mt19937 rng(seed);
    std::uniform_real_distribution<double> unit(0.0, 1.0);
    std::uniform_int_distribution<int> wave(0, 7);
    Patch next = engine.patch;
    for (int i = 0; i < PARAM_COUNT; ++i) {
        auto& row = next.params[i];
        row.value = paramInfo[i].min + (paramInfo[i].max - paramInfo[i].min) * unit(rng);
        row.wave = wave(rng);
        row.amp = unit(rng);
        row.freq = unit(rng);
        row.pw = 0.05 + 0.9 * unit(rng);
    }
    // Masters stay in the middle 40% of their slider range so a random patch
    // does not land on a 1000-ring or stuck-damping extreme.
    for (int i = 0; i < MASTER_COUNT; ++i) {
        if (i == MASTER_Tempo) continue;
        next.master[i] = denormalizeMaster(i, 0.3 + 0.4 * unit(rng));
    }
    next.hue = 360.0 * unit(rng);
    next.origin = {0.2 + 0.6 * unit(rng), 0.2 + 0.6 * unit(rng)};
    engine.setPatch(next);
    engine.clear(); // V1 1.0.82: a random patch flushes the view history
}
} // namespace whorld
