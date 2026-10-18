// Pure MIDI value decoding (GPL-3.0-or-later). See midi_decode.h.
#include "midi_decode.h"

#include <algorithm>
#include <cmath>

namespace whorld {
namespace {
// Clock smoothing constants (documented in midi_decode.h). Keeping them here
// makes the header self-describing without exposing tuning knobs.
constexpr int kMinIntervals = 4;         // ~1/6 of a beat at 120 BPM before active
constexpr double kSmoothing = 0.15;      // EMA weight of the newest interval
constexpr double kSilenceSeconds = 1.0;  // no pulse for this long -> inactive
} // namespace

int midi14bit(int msb, int lsb) {
    return (std::clamp(msb, 0, 127) << 7) | std::clamp(lsb, 0, 127);
}

int relativeDelta(int value, MidiInputMode mode) {
    value = std::clamp(value, 0, 127);
    switch (mode) {
    case MidiInputMode::RelativeTwosComplement:
        if (value == 0 || value == 64) return 0;
        return value < 64 ? value : value - 128;
    case MidiInputMode::RelativeSignMagnitude:
        if (value == 0 || value == 64) return 0; // 0x40 is "minus zero"
        return (value & 0x40) ? -(value & 0x3f) : (value & 0x3f);
    case MidiInputMode::RelativeBinaryOffset:
        return value - 64;
    default:
        return 0; // Absolute is not a relative mode
    }
}

bool TakeoverState::accept(double incoming, double current, double epsilon) {
    if (!std::isfinite(incoming) || !std::isfinite(current)) return false;
    if (engaged) return true;
    if (std::abs(incoming - current) <= epsilon) {
        engaged = true;
        return true;
    }
    // The value may have crossed between two samples, so compare the previous
    // incoming value with this one relative to the target.
    if (hasLast) {
        const double before = last - current, after = incoming - current;
        if ((before < 0 && after >= 0) || (before > 0 && after <= 0)) {
            engaged = true;
            return true;
        }
    }
    last = incoming;
    hasLast = true;
    return false;
}

void encodeControlChange(std::vector<unsigned char>& out, int channel, int control, int value) {
    out.push_back(static_cast<unsigned char>(0xb0 | (std::clamp(channel, 0, 15) & 0x0f)));
    out.push_back(static_cast<unsigned char>(std::clamp(control, 0, 127)));
    out.push_back(static_cast<unsigned char>(std::clamp(value, 0, 127)));
}

void ClockTracker::tick(double nowSeconds) {
    if (!std::isfinite(nowSeconds)) return;
    now_ = nowSeconds;
    if (!have_) {
        have_ = true;
        last_ = nowSeconds;
        return;
    }
    const double dt = nowSeconds - last_;
    last_ = nowSeconds;
    // A non-positive or very long gap means the clock restarted (or was
    // unplugged): relock from the next interval instead of averaging it in.
    if (dt <= 0 || dt > kSilenceSeconds) {
        count_ = 0;
        return;
    }
    if (count_ == 0) interval_ = dt;
    else interval_ += kSmoothing * (dt - interval_);
    ++count_;
}

void ClockTracker::update(double nowSeconds) {
    if (!std::isfinite(nowSeconds)) return;
    if (nowSeconds > now_) now_ = nowSeconds;
    // A silent clock must stop being active even though no pulse arrives to
    // say so; keep interval_ so bpm() still reports the last tempo.
    if (have_ && (now_ - last_) > kSilenceSeconds) count_ = 0;
}

void ClockTracker::start() { running_ = true; }

void ClockTracker::stop() { running_ = false; }

void ClockTracker::reset() {
    running_ = false;
    have_ = false;
    last_ = 0;
    now_ = 0;
    count_ = 0;
    interval_ = 0;
}

bool ClockTracker::active() const {
    return have_ && count_ >= kMinIntervals && (now_ - last_) <= kSilenceSeconds;
}

double ClockTracker::bpm() const {
    if (interval_ <= 0) return 0;
    return 60.0 / (interval_ * pulsesPerBeat);
}
} // namespace whorld
