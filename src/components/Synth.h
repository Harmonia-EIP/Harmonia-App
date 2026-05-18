/**
 * @file Synth.h
 * @brief Core synthesizer voice and sound definitions for Harmonia.
 *
 * This file contains the main synthesis engine classes used by
 * the Harmonia synthesizer.
 *
 * Included classes:
 * - HarmoniaSound
 * - HarmoniaVoice
 *
 * Main features:
 * - Polyphonic synthesizer voices
 * - Dual oscillator architecture
 * - Multiple waveform generation
 * - ADSR amplitude envelope
 * - Filter envelope modulation
 * - LFO modulation
 * - Resonant multimode filtering
 * - Velocity sensitivity
 * - Distortion processing
 * - Noise generation
 */

#pragma once

#include "../JuceHeader.h"
#include "../parameters/HarmoniaParameters.h"
#include <random>

/**
 * @class HarmoniaSound
 * @brief Generic synthesizer sound definition.
 *
 * HarmoniaSound represents a playable synthesizer sound
 * compatible with all MIDI notes and channels.
 *
 * This class acts as the sound type required by JUCE's
 * synthesizer architecture.
 */
class HarmoniaSound : public juce::SynthesiserSound
{
public:

    /**
     * @brief Indicates whether the sound can play a MIDI note.
     *
     * This implementation accepts all notes.
     *
     * @param note MIDI note number.
     * @return Always returns true.
     */
    bool appliesToNote (int) override { return true; }

    /**
     * @brief Indicates whether the sound can play a MIDI channel.
     *
     * This implementation accepts all channels.
     *
     * @param channel MIDI channel number.
     * @return Always returns true.
     */
    bool appliesToChannel (int) override { return true; }
};

/**
 * @class HarmoniaVoice
 * @brief Main synthesizer voice implementation.
 *
 * HarmoniaVoice is responsible for generating audio
 * for a single polyphonic synthesizer voice.
 *
 * Synthesis architecture:
 * - Dual oscillator engine
 * - Noise generator
 * - Multimode resonant filter
 * - LFO modulation
 * - ADSR amplitude envelope
 * - Filter modulation envelope
 * - Distortion stage
 *
 * Supported waveform types:
 * - Sine
 * - Triangle
 * - Saw
 * - Square
 *
 * Modulation capabilities:
 * - Pitch modulation
 * - Filter cutoff modulation
 * - Velocity modulation
 * - Envelope-controlled filtering
 *
 * The voice uses real-time parameter access through
 * HarmoniaParams::AtomicRefs.
 */
class HarmoniaVoice : public juce::SynthesiserVoice
{
public:

    /**
     * @brief Creates a synthesizer voice.
     *
     * @param refs Atomic parameter references used for real-time synthesis.
     */
    explicit HarmoniaVoice (const HarmoniaParams::AtomicRefs& refs);

    /**
     * @brief Checks whether this voice can play a given sound.
     *
     * @param sound Sound instance to validate.
     * @return True if the sound is a HarmoniaSound.
     */
    bool canPlaySound (juce::SynthesiserSound* sound) override;

    /**
     * @brief Starts playback of a MIDI note.
     *
     * This method:
     * - Initializes oscillator phases
     * - Computes note frequency
     * - Stores note velocity
     * - Resets modulation phases
     * - Triggers envelopes
     *
     * @param midiNoteNumber MIDI note value.
     * @param velocity MIDI note velocity.
     * @param sound Associated synthesizer sound.
     * @param currentPitchWheelPosition Current pitch wheel value.
     */
    void startNote (int midiNoteNumber,
                    float velocity,
                    juce::SynthesiserSound* sound,
                    int currentPitchWheelPosition) override;

    /**
     * @brief Stops playback of the current note.
     *
     * Depending on allowTailOff:
     * - envelopes may enter release stage
     * - or the voice may stop immediately
     *
     * @param velocity Release velocity.
     * @param allowTailOff Enables envelope release phase.
     */
    void stopNote (float velocity, bool allowTailOff) override;

    /**
     * @brief Handles pitch wheel changes.
     *
     * Currently unused.
     *
     * @param value Pitch wheel value.
     */
    void pitchWheelMoved (int) override {}

    /**
     * @brief Handles MIDI controller changes.
     *
     * Currently unused.
     *
     * @param controller Controller number.
     * @param value Controller value.
     */
    void controllerMoved (int, int) override {}

    /**
     * @brief Generates audio samples for the current block.
     *
     * Main DSP operations:
     * - Oscillator generation
     * - LFO modulation
     * - Noise mixing
     * - Envelope processing
     * - Filter modulation
     * - Distortion processing
     * - Audio output generation
     *
     * @param buffer Destination audio buffer.
     * @param startSample First sample index.
     * @param numSamples Number of samples to generate.
     */
    void renderNextBlock (juce::AudioBuffer<float>& buffer,
                          int startSample,
                          int numSamples) override;

    /**
     * @brief Prepares DSP resources before playback.
     *
     * This method initializes:
     * - Filter processing
     * - Sample rate
     * - Envelope sample rates
     * - DSP specifications
     *
     * @param sampleRate Current audio sample rate.
     * @param samplesPerBlock Maximum block size.
     * @param numOutputChannels Number of audio channels.
     */
    void prepare (double sampleRate,
                  int samplesPerBlock,
                  int numOutputChannels);

private:

    /**
     * @enum Wave
     * @brief Available oscillator waveform types.
     */
    enum class Wave
    {
        Sine = 0,      /**< Sinusoidal waveform */
        Triangle = 1, /**< Triangle waveform */
        Saw = 2,      /**< Sawtooth waveform */
        Square = 3    /**< Square waveform */
    };

    /**
     * @brief Generates a waveform sample.
     *
     * @param w Waveform type.
     * @param phase Oscillator phase.
     * @return Generated sample value.
     */
    static float renderWave (Wave w, float phase) noexcept;

    /**
     * @brief Real-time parameter references.
     */
    const HarmoniaParams::AtomicRefs& params;

    /**
     * @brief Current audio sample rate.
     */
    double currentSampleRate = 44100.0;

    /**
     * @brief Oscillator 1 phase accumulator.
     */
    float phase1 = 0.0f;

    /**
     * @brief Oscillator 2 phase accumulator.
     */
    float phase2 = 0.0f;

    /**
     * @brief LFO phase accumulator.
     */
    float lfoPhase = 0.0f;

    /**
     * @brief Current MIDI note frequency in Hz.
     */
    float baseFrequencyHz = 440.0f;

    /**
     * @brief Current note velocity.
     */
    float noteVelocity = 1.0f;

    /**
     * @brief Random number generator used for noise synthesis.
     */
    std::mt19937 rng { 0x4321 };

    /**
     * @brief White noise distribution generator.
     */
    std::uniform_real_distribution<float> noiseDist { -1.0f, 1.0f };

    /**
     * @brief Resonant multimode state variable filter.
     */
    juce::dsp::StateVariableTPTFilter<float> filter;

    /**
     * @brief Main amplitude ADSR envelope.
     */
    juce::ADSR ampEnv;

    /**
     * @brief Parameters for the amplitude envelope.
     */
    juce::ADSR::Parameters ampParams;

    /**
     * @brief Filter modulation envelope.
     */
    juce::ADSR filterEnv;

    /**
     * @brief Parameters for the filter envelope.
     */
    juce::ADSR::Parameters filterEnvParams;
};