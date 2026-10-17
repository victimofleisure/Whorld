// Pure MIDI value decoding (GPL-3.0-or-later). See midi_decode.h.
#include "midi_decode.h"

#include <algorithm>
#include <cmath>

namespace whorld {
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
} // namespace whorld
