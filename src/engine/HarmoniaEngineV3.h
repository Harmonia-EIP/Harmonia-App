/**
 * @file HarmoniaEngineV3.h
 * @brief Harmonia v3 analog engine, plain C++17 (no JUCE): the sound the v3 AI models are trained on.
 *
 * Port of the reference implementation HARMONIA/src/synth/engine_v3.py (Harmonia-AI). Per voice:
 * 2 oscillators (sine, triangle, band-limited saw and pulse) x unison (1..16 voices) + noise
 * -> state-variable filter (12 / 24 dB) -> drive -> amplifier, computed at twice the sample rate
 * and brought back by a half-band filter; then, on the mix, chorus -> delay -> reverb (feedback
 * delay network). Parameters are physical values in charter order (HarmoniaCharter::analog::Index).
 *
 * Real-time safe: memory is allocated in prepare(), never while rendering.
 */

#pragma once

#include <array>
#include <cstdint>
#include <vector>

#include "../parameters/HarmoniaCharterV3.h"

namespace harmonia
{
    /** Physical values of the analog parameters, in charter order. */
    using AnalogPatch = std::array<double, HarmoniaCharter::analog::count>;

    /** Every analog parameter at its charter default. */
    AnalogPatch defaultPatch();

    /** Small, fast generator (SplitMix64) for unison phases, drift, noise and sample & hold. */
    class Random
    {
    public:
        explicit Random (std::uint64_t seed = 0x4321) noexcept : state (seed) {}
        double uniform() noexcept;  // [0, 1)
        double uniform (double low, double high) noexcept { return low + (high - low) * uniform(); }

    private:
        std::uint64_t state;
    };

    /** One note of the analog engine. */
    class AnalogVoice
    {
    public:
        static constexpr int maxUnison = 16;
        static constexpr int oversample = 2;

        /** Output sample rate; the voice runs at twice that. */
        void prepare (double sampleRate);

        void start (const AnalogPatch& patch, int midiNote, double velocity, std::uint64_t seed) noexcept;
        void release() noexcept { releasePending = true; }
        /** Releases the note when the voice reaches this sample of its own (twice faster) clock: offline renders. */
        void scheduleRelease (std::int64_t fastSample) noexcept { releaseAt = fastSample; }
        void kill() noexcept { active = false; }
        bool isActive() const noexcept { return active; }
        int getNote() const noexcept { return note; }

        /** Adds numSamples output-rate samples to left / right. */
        void render (const AnalogPatch& patch, float* left, float* right, int numSamples) noexcept;

        /** Output samples by which this voice lags the reference's zero-phase decimation. */
        static constexpr int latency = 9;

    private:
        enum Stage { idle, attack, decay, sustain, releasing };

        struct Envelope
        {
            Stage stage = idle;
            double value = 0.0;
            void start() noexcept { stage = attack; value = 0.0; }
            void step (double attackRate, double decayCoef, double sustainLevel, double releaseCoef) noexcept;
        };

        void renderFast (const AnalogPatch& patch, double& left, double& right) noexcept;
        void updateDrift (double t) noexcept;
        void updateTuning (const AnalogPatch& patch) noexcept;

        double fast = 96000.0;  // voice rate: twice the output rate
        bool active = false;
        bool releasePending = false;
        bool released = false;
        std::int64_t releaseAt = -1;
        int note = -1;
        double velocity = 0.0;
        double baseFrequency = 440.0;
        std::int64_t fastIndex = 0;
        Random random;

        int unison = 1;
        std::array<double, maxUnison> unisonOffset {}, phase1 {}, phase2 {}, tune1 {}, tune2 {};
        std::array<std::array<double, 4>, maxUnison> gains {};  // osc1 L, osc1 R, osc2 L, osc2 R
        std::array<std::array<double, 4>, maxUnison> driftRate {}, driftPhase {};  // [u][osc * 2 + sine]
        std::array<std::array<double, 2>, maxUnison> drift {};

        Envelope amp, filterEnv;
        double lfoPhase = 0.0, lfoHeld = 0.0;
        std::array<std::array<double, 2>, 4> filter {};  // channel x stage

        // half-band decimation, per channel: the last taps fast samples
        std::array<std::array<double, 64>, 2> history {};
        int historyPos = 0;
    };

    /** Chorus -> delay -> reverb on the stereo mix of the voices, as the reference applies them to one note. */
    class Effects
    {
    public:
        void prepare (double sampleRate);
        void reset() noexcept;
        void process (const AnalogPatch& patch, float* left, float* right, int numSamples) noexcept;

    private:
        static double readDelay (const std::vector<double>& buffer, int position, double delaySamples) noexcept;

        double rate = 48000.0;
        std::int64_t index = 0;

        std::array<std::vector<double>, 2> chorus;
        std::array<std::vector<double>, 2> echo;
        std::array<double, 2> echoLowpass {};

        std::array<std::vector<double>, 8> lines;
        std::array<double, 8> lineLowpass {}, mods {};
        std::array<std::vector<double>, 4> diffusers;
        std::array<int, 4> diffuserPos {};
        std::vector<double> predelay;
    };

    /** One note offline, aligned with the reference renderer (`render` in engine_v3.py): the voice is held
        for holdSeconds, then released; the output lasts totalSeconds. left / right hold sampleRate * totalSeconds. */
    void renderNote (const AnalogPatch& patch, int midiNote, double velocity, double holdSeconds, double totalSeconds,
                     int sampleRate, std::uint64_t seed, float* left, float* right);
}

/** C entry point for the AI's comparison bench (ctypes): physical values in charter order. Returns the
    number of samples per channel written to out (left then right), or -1 on bad arguments. */
extern "C" int harmonia_render_analog_note (const double* physical, int count, int midiNote, double velocity,
                                            double holdSeconds, double totalSeconds, int sampleRate,
                                            unsigned long long seed, float* out, int capacity);
