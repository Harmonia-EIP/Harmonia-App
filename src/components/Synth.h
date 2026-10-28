/**
 * @file Synth.h
 * @brief Synthesizer voice and sound definitions for Harmonia.
 *
 * Included classes:
 * - HarmoniaSound
 * - HarmoniaVoice: a juce::SynthesiserVoice around the v3 analog engine (engine/HarmoniaEngineV3.h)
 *
 * The engine itself is plain C++ and matches the reference the AI models are trained on
 * (HARMONIA/src/synth/engine_v3.py in Harmonia-AI): 2 oscillators x unison, noise, 12/24 dB filter,
 * exponential envelopes, pitch envelope, LFO, drive, at twice the sample rate. Chorus, delay and
 * reverb run once on the mix, in the processor.
 */

#pragma once

#include "../JuceHeader.h"
#include "../engine/HarmoniaEngineV3.h"

#include <vector>

/**
 * @class HarmoniaSound
 * @brief Sound object accepted by every Harmonia voice, on every note and channel.
 */
class HarmoniaSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

/**
 * @class HarmoniaVoice
 * @brief One polyphonic voice: plays the processor's current analog patch.
 *
 * The patch (physical values in charter order) is filled by the processor at the start of each
 * block, on the audio thread, before the synthesiser renders the voices.
 */
class HarmoniaVoice : public juce::SynthesiserVoice
{
public:
    HarmoniaVoice (const harmonia::AnalogPatch& patch, std::uint64_t seed);

    bool canPlaySound (juce::SynthesiserSound* sound) override;

    void startNote (int midiNoteNumber,
                    float velocity,
                    juce::SynthesiserSound* sound,
                    int currentPitchWheelPosition) override;

    void stopNote (float velocity, bool allowTailOff) override;

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& buffer,
                          int startSample,
                          int numSamples) override;

    void prepare (double sampleRate,
                  int samplesPerBlock,
                  int numOutputChannels);

private:
    const harmonia::AnalogPatch& patch;
    harmonia::AnalogVoice engine;
    std::vector<float> left, right;   ///< scratch, sized in prepare()
    std::uint64_t seed;
    std::uint64_t notesPlayed = 0;    ///< each note draws its own unison phases, drift and noise
};
