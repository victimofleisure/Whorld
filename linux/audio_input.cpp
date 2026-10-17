// AudioInput: ALSA capture feeding the shared analyzer in audio_analysis.h.
// Non-blocking; polled on the UI thread.
// SPDX-License-Identifier: GPL-3.0-or-later
#include "audio_input.h"
#include "audio_analysis.h"

#include <alsa/asoundlib.h>
#include <cerrno>
#include <vector>

namespace whorld {
namespace {
constexpr int kWindow = 1024;    // analysis window, power of two
constexpr int kReadFrames = 256; // frames per ALSA read
} // namespace

struct AudioInput::Impl {
    snd_pcm_t* pcm = nullptr;
    int channels = 1;
    unsigned rate = 48000;
    double gain = 1;
    std::string device{"default"};
    int writePos = 0;            // circular position in mono
    std::vector<float> mono;     // last kWindow mono samples
    std::vector<float> ordered;  // mono in chronological order, for the FFT
    std::vector<float> scratch;  // interleaved read buffer
};

AudioInput::AudioInput() : impl(new Impl) {}

AudioInput::~AudioInput() {
    close();
    delete impl;
}

bool AudioInput::open(const std::string& device, double gain, unsigned rate, std::string& error) {
    close();
    error.clear();
    const std::string dev = device.empty() ? "default" : device;

    int rc = snd_pcm_open(&impl->pcm, dev.c_str(), SND_PCM_STREAM_CAPTURE, SND_PCM_NONBLOCK);
    if (rc < 0) {
        impl->pcm = nullptr;
        error = "cannot open audio input '" + dev + "': " + snd_strerror(rc);
        return false;
    }
    // Mono is simplest; fall back to stereo and downmix if the device needs it.
    rc = snd_pcm_set_params(impl->pcm, SND_PCM_FORMAT_FLOAT_LE, SND_PCM_ACCESS_RW_INTERLEAVED,
                            1, rate, 1 /*soft resample*/, 40000 /*latency us*/);
    impl->channels = 1;
    if (rc < 0) {
        rc = snd_pcm_set_params(impl->pcm, SND_PCM_FORMAT_FLOAT_LE, SND_PCM_ACCESS_RW_INTERLEAVED,
                                2, rate, 1, 40000);
        impl->channels = 2;
    }
    if (rc < 0) {
        error = "cannot configure audio input '" + dev + "': " + snd_strerror(rc);
        close();
        return false;
    }

    impl->rate = rate;
    impl->gain = gain > 0 ? gain : 1.0;
    impl->device = dev;
    impl->writePos = 0;
    impl->mono.assign(kWindow, 0);
    impl->ordered.assign(kWindow, 0);
    impl->scratch.assign(std::size_t(256) * impl->channels, 0);
    snd_pcm_prepare(impl->pcm);
    return true;
}

void AudioInput::close() {
    if (impl->pcm) {
        snd_pcm_close(impl->pcm);
        impl->pcm = nullptr;
    }
    feat = AudioFeatures{};
}

void AudioInput::poll() {
    if (!impl->pcm) return;
    bool got = false;
    for (int iter = 0; iter < 4; ++iter) {
        impl->scratch.resize(std::size_t(256) * impl->channels);
        snd_pcm_sframes_t n = snd_pcm_readi(impl->pcm, impl->scratch.data(), 256);
        if (n == -EAGAIN || n == 0) break;
        if (n < 0) {
            if (n == -EPIPE) { snd_pcm_prepare(impl->pcm); continue; } // overrun: restart
            if (n == -ESTRPIPE) continue;                              // suspended: retry later
            break;
        }
        got = true;
        for (snd_pcm_sframes_t f = 0; f < n; ++f) {
            double sum = 0;
            for (int c = 0; c < impl->channels; ++c)
                sum += impl->scratch[std::size_t(f) * impl->channels + c];
            impl->mono[std::size_t(impl->writePos)] = float(sum / impl->channels);
            impl->writePos = (impl->writePos + 1) % kWindow;
        }
    }
    if (got) analyze();
    // No new window was analysed, so the onset flag from the previous frame is
    // stale; clearing it keeps a beat-driven momentary action from re-firing.
    else feat.onset = false;
}

void AudioInput::analyze() {
    for (int i = 0; i < kWindow; ++i)
        impl->ordered[std::size_t(i)] = impl->mono[std::size_t((impl->writePos + i) % kWindow)];
    analyzeBlock(impl->ordered.data(), kWindow, impl->rate, impl->gain, feat);
}

bool AudioInput::active() const { return impl->pcm != nullptr; }

std::string AudioInput::deviceName() const { return impl->device; }

} // namespace whorld
