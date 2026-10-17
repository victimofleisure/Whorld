// AudioInput: ALSA capture with level and 3-band analysis for audio-reactive
// parameter modulation. Non-blocking; polled on the UI thread.
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once
#include "playlist.h"
#include <string>

namespace whorld {
class AudioInput {
public:
    AudioInput();
    ~AudioInput();
    AudioInput(const AudioInput&) = delete;
    AudioInput& operator=(const AudioInput&) = delete;
    // `device` may be empty (ALSA "default"); `gain` scales the analysis.
    bool open(const std::string& device, double gain, unsigned rate, std::string& error);
    void close();
    void poll(); // read whatever is ready; re-analyze the latest window
    const AudioFeatures& features() const { return feat; }
    bool active() const;
    std::string deviceName() const;
private:
    void analyze();
    struct Impl;
    Impl* impl;
    AudioFeatures feat;
};
} // namespace whorld
