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

        // Onset/beat: a train of periodic bursts yields roughly one pulse per
        // burst (the pulse is the refractory period), and silence is quiet.
        {
            const std::vector<float> tone = sine(1000, 1.0);
            const std::vector<float> silence(kN, 0.0f);
            AudioFeatures f;
            int onsets = 0;
            const int period = 8; // blocks between bursts
            const int bursts = 10;
            for (int c = 0; c < bursts; ++c) {
                analyzeBlock(tone.data(), kN, kRate, 1.0, f);
                if (f.onset) ++onsets;
                for (int s = 1; s < period; ++s) analyzeBlock(silence.data(), kN, kRate, 1.0, f);
            }
            require(onsets >= bursts - 1 && onsets <= bursts);
        }
        // A single onset raises the pulse, which then decays to zero while the
        // onset flag stays false.
        {
            AudioFeatures f;
            analyzeBlock(sine(1000, 1.0).data(), kN, kRate, 1.0, f);
            require(f.onset && f.beat > 0.9);
            double previous = f.beat;
            const std::vector<float> silence(kN, 0.0f);
            for (int i = 0; i < 5 && previous > 0; ++i) {
                analyzeBlock(silence.data(), kN, kRate, 1.0, f);
                require(!f.onset);
                require(f.beat < previous);
                previous = f.beat;
            }
            require(f.beat == 0.0);
        }
        // Silence never produces an onset or a pulse.
        {
            AudioFeatures f;
            const std::vector<float> silence(kN, 0.0f);
            for (int i = 0; i < 30; ++i) analyzeBlock(silence.data(), kN, kRate, 1.0, f);
            require(!f.onset && f.beat == 0.0);
        }

        // mapAudio: "Beat" drives a range target with the pulse value, but a
        // momentary target (Clear) fires only on the onset frame.
        {
            Playlist pl;
            AudioMapping range;
            range.feature = "Beat";
            range.target = "Zoom";
            range.property = "Val";
            range.start = 0;
            range.end = 100;
            AudioMapping momentary;
            momentary.feature = "Beat";
            momentary.target = "Clear";
            momentary.property = "Val";
            momentary.start = 0;
            momentary.end = 100;
            pl.audioMappings = {range, momentary};
            AudioFeatures f;
            f.beat = 0.5;
            f.onset = false;
            auto actions = mapAudio(pl, f);
            require(actions.size() == 1);
            require(actions[0].target == "Zoom");
            require(std::abs(actions[0].amount - 0.5) < 1e-9);
            f.beat = 1.0;
            f.onset = true;
            actions = mapAudio(pl, f);
            require(actions.size() == 2);
        }

        std::cout << "audio analysis OK\n";
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "whorld_audio_test: " << e.what() << '\n';
        return 1;
    }
}
