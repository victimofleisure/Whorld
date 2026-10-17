// Pure MIDI value decoding: 14-bit CC pairs, relative encoders and soft
// takeover (GPL-3.0-or-later). No ALSA/SDL dependencies, so it unit-tests
// without hardware; the frontend keeps the per-control state on the UI thread.
#pragma once

namespace whorld {
// How raw 7-bit controller bytes are interpreted. The default is Absolute so
// existing playlists and behaviour are unchanged.
enum class MidiInputMode {
    Absolute,               // read the CC value as an absolute 0..127 position
    FourteenBit,            // combine an MSB/LSB CC pair into 0..16383
    RelativeTwosComplement, // encoder deltas in two's complement
    RelativeSignMagnitude,  // encoder deltas with a sign bit
    RelativeBinaryOffset,   // encoder deltas offset by 64
};

// Combine a 7-bit MSB and LSB into a 14-bit value (0..16383).
int midi14bit(int msb, int lsb);

// Signed step count for a 7-bit relative-encoder value (0..127).
//   Two's complement: 1..63 -> +1..+63, 65..127 -> -63..-1; 0 and 64 -> 0.
//   Sign-magnitude:   bit 6 (0x40) is the sign, bits 0..5 the magnitude, so
//                     1..63 -> +1..+63 and 65..127 -> -1..-63; 0 and 64 -> 0.
//   Binary offset:    64 -> 0, 65..127 -> +1..+63, 0..63 -> -64..-1.
int relativeDelta(int value, MidiInputMode mode);

// Per-control soft takeover (pickup). A control that is not yet engaged is
// ignored so switching a fader onto a parameter never makes it jump. It engages
// when the incoming value reaches the current value (within epsilon) or crosses
// it between two samples; afterwards every value is accepted and it tracks
// normally. Keep one instance per physical control.
struct TakeoverState {
    bool engaged = false;
    bool accept(double incoming, double current, double epsilon = 0.02);
    void reset() { engaged = false; }

private:
    bool hasLast = false;
    double last = 0;
};
} // namespace whorld
