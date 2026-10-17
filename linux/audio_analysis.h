// Audio feature analysis for audio-reactive parameter modulation: level plus
// low/mid/high band energy, each normalized to 0..1. No audio library needed,
// so it can be unit tested in isolation.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "playlist.h"

namespace whorld {
// Analyze a block of mono samples (count must be a power of two) into
// level/low/mid/high features. The features are smoothed toward the previous
// values, so pass the same AudioFeatures in on each call.
void analyzeBlock(const float* samples, int count, unsigned rate, double gain, AudioFeatures& features);
// The per-poll smoothing factors used by analyzeBlock (exposed for tests).
constexpr double kAudioAttack = 0.55;
constexpr double kAudioRelease = 0.12;
} // namespace whorld
