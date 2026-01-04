#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_dsp/juce_dsp.h>

//================= Paramètres =================

struct HarmoniaParams
{
    float frequency      = 440.0f;
    float volume         = 0.8f;
    float attack         = 0.1f;
    float decay          = 0.1f;
    float sustain        = 0.8f;
    float release        = 0.5f;
    float cutoff         = 1000.0f;
    float resonance      = 1.0f;
    int   waveformIndex  = 0;              // 0 = Sine, 1 = Saw, 2 = Square
    juce::String filterType = "Low Pass";  // "Low Pass", "Band Pass", "High Pass"
};

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

    // SynthesiserVoice
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

    // À appeler depuis prepareToPlay
    void prepare (double sampleRate, int samplesPerBlock, int numOutputChannels);

    // API globale
    void setParameters (const HarmoniaParams& newParams);

    // API "confort" pour MainComponent (ce que vous utilisez déjà)
    void setADSR   (float attack, float decay, float sustain, float release);

    // version avec index (0 = low, 1 = band, 2 = high)
    void setFilter (float cutoff, float resonance, int filterTypeIndex);

    // version avec String ("Low Pass", "Band Pass", "High Pass")
    void setFilter (float cutoff, float resonance, const juce::String& filterTypeName);

    // waveform par index (0 = sine, 1 = saw, 2 = square)
    void setWaveform (int waveformIndex);

    // waveform par nom ("Sine", "Saw", "Square")
    void setWaveform (const juce::String& waveformName);

private:
    void updateOscillator();    // met l’osc à jour selon currentParams.waveformIndex
    void updateFilterType();    // met le filtre à jour selon currentParams.filterType

    juce::dsp::Oscillator<float>             osc;
    juce::dsp::StateVariableTPTFilter<float> filter;
    juce::dsp::Gain<float>                   gain;
    juce::ADSR                               adsr;
    juce::ADSR::Parameters                   adsrParams;

    HarmoniaParams currentParams;
    double         currentSampleRate = 44100.0;
};
