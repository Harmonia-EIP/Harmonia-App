#pragma once

#include "ComponentsIncludes.h"
#include <cmath>

//================= Sound basique =================
class HarmoniaSound : public juce::SynthesiserSound
{
public:
    bool appliesToNote    (int) override { return true; }
    bool appliesToChannel (int) override { return true; }
};

//================= Une voix de synthé =================
class HarmoniaVoice : public juce::SynthesiserVoice
{
public:
    HarmoniaVoice();

    bool canPlaySound (juce::SynthesiserSound* sound) override;

    void startNote (int midiNoteNumber,
                    float velocity,
                    juce::SynthesiserSound* sound,
                    int currentPitchWheelPosition) override;

    void stopNote (float velocity, bool allowTailOff) override;

    void pitchWheelMoved (int) override {}
    void controllerMoved (int, int) override {}

    void renderNextBlock (juce::AudioBuffer<float>& buffer,
                          int startSample, int numSamples) override;

    void prepare (double sampleRate, int samplesPerBlock, int numOutputChannels);

    void setParameters (const PatchParams& newParams);

    void setWaveform (int waveformIndex);
    void setFilter   (float cutoff, float resonance, int filterTypeIndex);
    void setADSR     (float attack, float decay, float sustain, float release);

private:
    void updateOscillator();
    void updateFilterType();

    juce::dsp::Oscillator<float>             osc;
    juce::dsp::StateVariableTPTFilter<float> filter;

    juce::ADSR             adsr;
    juce::ADSR::Parameters adsrParams;

    PatchParams currentParams;
    double currentSampleRate = AppConfig::Synth::defaultSampleRate;
};
