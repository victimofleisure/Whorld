// Audio analysis: FFT band split and level detection for audio-reactive input.
// Pure math, so it runs anywhere without audio hardware.
#include "audio_analysis.h"
#include <cmath>
#include <iostream>
#include <stdexcept>
#include <vector>
#define require(expr) do { if (!(expr)) throw std::runtime_error("Check failed: " #expr); } while (false)

using namespace whorld;

namespace {
constexpr int kN = 1024;
constexpr unsigned kRate = 48000;

// Feed the same buffer repeatedly so the attack/release smoothing converges.
AudioFeatures settle(const std::vector<float>& samples) {
    AudioFeatures f;
    for (int i = 0; i < 30; ++i) analyzeBlock(samples.data(), kN, kRate, 1.0, f);
    return f;
}
std::vector<float> sine(double hz, double amp) {
    std::vector<float> v(kN);
    for (int i = 0; i < kN; ++i)
        v[i] = float(amp * std::sin(2 * 3.14159265358979323846 * hz * i / kRate));
    return v;
}
} // namespace

int main() {
    try {
        // Silence stays silent.
        const AudioFeatures silence = settle(std::vector<float>(kN, 0.0f));
        require(silence.level < 0.01 && silence.low < 0.01 && silence.mid < 0.01 && silence.high < 0.01);

        // A full-scale tone should light up exactly one band.
        const AudioFeatures low = settle(sine(100, 1.0));
        require(low.low > 0.9 && low.mid < 0.1 && low.high < 0.1);
        require(low.level > 0.9);

        const AudioFeatures mid = settle(sine(1000, 1.0));
        require(mid.mid > 0.9 && mid.low < 0.1 && mid.high < 0.1);

        const AudioFeatures high = settle(sine(5000, 1.0));
        require(high.high > 0.9 && high.low < 0.1 && high.mid < 0.1);

        // Level tracks amplitude; a quiet tone sits well below the clipping
        // ceiling the loud tones above reach.
        const AudioFeatures quiet = settle(sine(1000, 0.02));
        require(quiet.level > 0.02 && quiet.level < 0.12);
        require(quiet.mid > 0.02 && quiet.mid < 0.3);

        // The first call rises but never overshoots.
        AudioFeatures fresh;
        analyzeBlock(sine(1000, 1.0).data(), kN, kRate, 1.0, fresh);
        require(fresh.level > 0.0 && fresh.level <= 1.0);
        require(fresh.mid > 0.0 && fresh.mid <= 1.0);

        // A non-power-of-two window is rejected (leaves features untouched).
        AudioFeatures untouched;
        std::vector<float> odd(1000, 0.5f);
        analyzeBlock(odd.data(), 1000, kRate, 1.0, untouched);
        require(untouched.level == 0 && untouched.mid == 0);

        // Gain scales the response: a quiet tone becomes loud.
        AudioFeatures boosted;
        const std::vector<float> faint = sine(1000, 0.005);
        for (int i = 0; i < 30; ++i) analyzeBlock(faint.data(), kN, kRate, 8.0, boosted);
        require(boosted.level > quiet.level);

        std::cout << "audio analysis OK\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "whorld_audio_test: " << e.what() << '\n';
        return 1;
    }
}
