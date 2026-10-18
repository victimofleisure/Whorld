// Pure MIDI value decoding: 14-bit CC pairs, relative encoders, soft takeover
// and external clock tracking (GPL-3.0-or-later). No ALSA/SDL dependencies, so
// it unit-tests without hardware; the frontend keeps the per-control state on
// the UI thread.
#pragma once
#include <vector>

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

// Appends a 3-byte Control Change message (0xB0|channel, control, value) to
// `out`. `channel` is zero-based and clamped to 0..15; `control` and `value`
// are clamped to 0..127, so callers cannot emit an invalid packet.
void encodeControlChange(std::vector<unsigned char>& out, int channel, int control, int value);

// Follows an external MIDI clock so the tempo can track another application or
// a master device. Clock pulses arrive at 24 per quarter note (24 PPQN); the
// tracker smooths the pulse intervals and reports the resulting BPM.
//
// Algorithm: a first-order EMA over the pulse interval (weight `kSmoothing` =
// 0.15 on the newest sample, the first interval after a lock seeds it), chosen
// over a median so it costs O(1) state and settles within a beat while still
// shrugging off a single jittery pulse. `active()` needs at least
// `kMinIntervals` = 4 intervals and expires after `kSilenceSeconds` = 1.0 s
// without a pulse; `bpm()` keeps returning the last smoothed value so the
// caller can leave the last tempo when the clock stops. It is pure, so a test
// can feed explicit timestamps:
//     ClockTracker clock;
//     clock.start();
//     for (int i = 0; i < 48; ++i) clock.tick(i * 60.0 / (120.0 * 24.0));
//     // clock.active() && clock.running() && clock.bpm() ~ 120
class ClockTracker {
public:
    static constexpr double pulsesPerBeat = 24.0; // MIDI clock is 24 PPQN

    // Called once per received clock pulse at a monotonic timestamp (seconds).
    void tick(double nowSeconds);
    // Marks the passage of time between pulses so a silent clock expires even
    // when no further pulse arrives. Call once per frontend frame.
    void update(double nowSeconds);
    void start(); // MIDI Start/Continue: transport is running
    void stop();  // MIDI Stop: transport is stopped, pulses may continue
    void reset(); // forget transport, timing and smoothing
    bool running() const { return running_; }
    // True while enough pulses have arrived and none is more than ~1 s late.
    bool active() const;
    double bpm() const;

private:
    bool running_ = false;
    bool have_ = false;    // a previous pulse time is known
    double last_ = 0;      // time of the last pulse
    double now_ = 0;       // latest observed time (tick or update)
    int count_ = 0;        // accepted intervals since the last lock
    double interval_ = 0;  // smoothed seconds between pulses
};
} // namespace whorld
