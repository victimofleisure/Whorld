// AudioInput analysis: a small FFT turns a mono sample window into a level plus
// low/mid/high band energies, smoothed for reactive visuals. No ALSA here, so
// this translation unit is safely unit-testable without audio hardware.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "audio_analysis.h"

#include <algorithm>
#include <cmath>
#include <complex>
#include <vector>

namespace whorld {
namespace {
constexpr double kPi = 3.14159265358979323846;
constexpr double kLevelGain = 4.0; // RMS of loud music is ~0.1-0.3
constexpr double kBandGain = 6.0;  // a full-scale sine reaches ~1.0 after norm

// In-place iterative radix-2 Cooley-Tukey FFT.
void fft(std::vector<std::complex<double>>& a) {
    const std::size_t n = a.size();
    for (std::size_t i = 1, j = 0; i < n; ++i) {
        std::size_t bit = n >> 1;
        for (; j & bit; bit >>= 1) j ^= bit;
        j ^= bit;
        if (i < j) std::swap(a[i], a[j]);
    }
    for (std::size_t len = 2; len <= n; len <<= 1) {
        const double angle = -2 * kPi / double(len);
        const std::complex<double> step(std::cos(angle), std::sin(angle));
        for (std::size_t i = 0; i < n; i += len) {
            std::complex<double> w(1);
            for (std::size_t k = 0; k < len / 2; ++k) {
                const std::complex<double> u = a[i + k];
                const std::complex<double> v = a[i + k + len / 2] * w;
                a[i + k] = u + v;
                a[i + k + len / 2] = u - v;
                w *= step;
            }
        }
    }
}
} // namespace

void analyzeBlock(const float* samples, int count, unsigned rate, double gain, AudioFeatures& features) {
    if (!samples || count < 2 || (count & (count - 1)) != 0) return;
    const int n = count;

    std::vector<std::complex<double>> spectrum(static_cast<std::size_t>(n));
    double sumSquares = 0;
    for (int i = 0; i < n; ++i) {
        const double v = samples[i];
        sumSquares += v * v;
        const double hann = 0.5 - 0.5 * std::cos(2 * kPi * i / (n - 1));
        spectrum[static_cast<std::size_t>(i)] = std::complex<double>(v * hann, 0);
    }
    const double rms = std::sqrt(sumSquares / n);
    fft(spectrum);

    const double binHz = double(rate) / n;
    double bandMax[3] = {0, 0, 0}; // low, mid, high
    for (int k = 1; k < n / 2; ++k) {
        const double hz = k * binHz;
        const double mag = std::abs(spectrum[static_cast<std::size_t>(k)]);
        if (hz < 250) bandMax[0] = std::max(bandMax[0], mag);
        else if (hz < 2000) bandMax[1] = std::max(bandMax[1], mag);
        else if (hz < 12000) bandMax[2] = std::max(bandMax[2], mag);
    }

    // A full-scale sine concentrates |X[k]| ~ n/2 in one bin, so scaling the
    // band peak by 2/n brings a strong tone near 1.0 before kBandGain and the
    // user gain.
    const auto smooth = [](double target, double previous) {
        const double a = target > previous ? kAudioAttack : kAudioRelease;
        return previous + a * (target - previous);
    };
    const auto band = [&](double value) {
        return std::clamp(value * (2.0 / n) * kBandGain * gain, 0.0, 1.0);
    };
    features.level = smooth(std::clamp(rms * kLevelGain * gain, 0.0, 1.0), features.level);
    features.low = smooth(band(bandMax[0]), features.low);
    features.mid = smooth(band(bandMax[1]), features.mid);
    features.high = smooth(band(bandMax[2]), features.high);
}

} // namespace whorld
