/**
 * @file Synth.h
 * @brief Synthesizer voice and sound definitions for Harmonia.
 *
 * Included classes:
 * - HarmoniaSound
 * - HarmoniaVoice: a juce::SynthesiserVoice that plays a note on one of the two v3 engines
 *
 * Both engines are plain C++ and match what the AI models are trained on (Harmonia-AI):
 * - analog (engine/HarmoniaEngineV3.h, reference HARMONIA/src/synth/engine_v3.py): 2 oscillators x unison,
 *   noise, 12/24 dB filter, exponential envelopes, pitch envelope, LFO, drive, at twice the sample rate;
 *   chorus, delay and reverb run once on the mix, in the processor;
 * - DX7 (engine/HarmoniaDx7.h): one DX7 voice played by Dexed's msfa FM core, as on the instrument.
 */

#pragma once

#include "../JuceHeader.h"
#include "../engine/HarmoniaEngineV3.h"
#include "../engine/HarmoniaDx7.h"

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
 * @brief What the voices play, filled by the processor at the start of each block (audio thread).
 */
struct HarmoniaPatches
{
    harmonia::AnalogPatch analog = harmonia::defaultPatch();  ///< physical values, charter order
    harmonia::Dx7Patch dx7 = harmonia::dx7InitPatch();       ///< DX7 voice built from the charter's DX7 parameters
    bool dx7Mode = false;                                     ///< parameter "engine": 0 analog, 1 DX7
};

/**
 * @class HarmoniaVoice
 * @brief One polyphonic voice: plays a note on the engine selected when the note starts.
 *
 * Analog notes follow automation block by block; a DX7 note keeps the voice it started with.
 */
class HarmoniaVoice : public juce::SynthesiserVoice
{
public:
    HarmoniaVoice (const HarmoniaPatches& patches, std::uint64_t seed);

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
    const HarmoniaPatches& patches;
    harmonia::AnalogVoice analog;
    harmonia::Dx7Voice dx7;
    bool playingDx7 = false;
    std::vector<float> left, right;   ///< scratch, sized in prepare()
    std::uint64_t seed;
    std::uint64_t notesPlayed = 0;    ///< each note draws its own unison phases, drift and noise
};
