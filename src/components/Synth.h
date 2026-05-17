#pragma once

#include "../JuceHeader.h"
#include "../parameters/HarmoniaParameters.h"
#include <random>

class HarmoniaSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote    (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

class HarmoniaVoice : public juce::SynthesiserVoice
{
public:
    explicit HarmoniaVoice (const HarmoniaParams::AtomicRefs& refs);

    bool canPlaySound (juce::SynthesiserSound* sound) override;

    void startNote (int midiNoteNumber, float velocity,
                    juce::SynthesiserSound* sound,
                    int currentPitchWheelPosition) override;

    void stopNote (float velocity, bool allowTailOff) override;

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& buffer,
                          int startSample, int numSamples) override;

    void prepare (double sampleRate, int samplesPerBlock, int numOutputChannels);

private:
    enum class Wave { Sine = 0, Triangle = 1, Saw = 2, Square = 3 };

    static float renderWave (Wave w, float phase) noexcept;

    const HarmoniaParams::AtomicRefs& params;

    double currentSampleRate = 44100.0;

    float phase1 = 0.0f;
    float phase2 = 0.0f;
    float lfoPhase = 0.0f;

    float baseFrequencyHz = 440.0f;
    float noteVelocity    = 1.0f;

    std::mt19937 rng { 0x4321 };
    std::uniform_real_distribution<float> noiseDist { -1.0f, 1.0f };

    juce::dsp::StateVariableTPTFilter<float> filter;

    juce::ADSR ampEnv;
    juce::ADSR::Parameters ampParams;

    juce::ADSR filterEnv;
    juce::ADSR::Parameters filterEnvParams;
};
