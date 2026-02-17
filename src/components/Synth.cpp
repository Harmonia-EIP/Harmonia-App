#include "Synth.h"
#include <cmath>

HarmoniaVoice::HarmoniaVoice()
{
    osc.initialise([](float x) { return std::sin(x); }, 128);

    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);

    currentParams = HarmoniaParams{};
}

void HarmoniaVoice::prepare (double sampleRate, int samplesPerBlock, int numOutputChannels)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels      = (juce::uint32) numOutputChannels;

    osc.prepare (spec);
    filter.prepare (spec);

    adsr.setSampleRate (sampleRate);
}

bool HarmoniaVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<HarmoniaSound*> (sound) != nullptr;
}

void HarmoniaVoice::setParameters (const HarmoniaParams& newParams)
{
    currentParams = newParams;

    adsrParams.attack  = newParams.attack;
    adsrParams.decay   = newParams.decay;
    adsrParams.sustain = newParams.sustain;
    adsrParams.release = newParams.release;
    adsr.setParameters (adsrParams);

    filter.setCutoffFrequency (newParams.cutoff);
    filter.setResonance       (newParams.resonance);
    updateFilterType();

    updateOscillator();
}

void HarmoniaVoice::setADSR (float attack, float decay, float sustain, float release)
{
    currentParams.attack  = attack;
    currentParams.decay   = decay;
    currentParams.sustain = sustain;
    currentParams.release = release;
    setParameters(currentParams);
}

void HarmoniaVoice::setFilter (float cutoff, float resonance, int filterTypeIndex)
{
    currentParams.cutoff    = cutoff;
    currentParams.resonance = resonance;

    switch (filterTypeIndex)
    {
        case 1: currentParams.filterType = "Band Pass"; break;
        case 2: currentParams.filterType = "High Pass"; break;
        default: currentParams.filterType = "Low Pass"; break;
    }

    setParameters(currentParams);
}

void HarmoniaVoice::setWaveform (int waveformIndex)
{
    currentParams.waveformIndex = waveformIndex;
    setParameters(currentParams);
}

void HarmoniaVoice::updateOscillator()
{
    switch (currentParams.waveformIndex)
    {
        case 0: // sine
            osc.initialise([](float x) { return std::sin(x); }, 128);
            break;

        case 1: // saw
            osc.initialise([](float x)
            {
                return juce::jmap(x,
                                  -juce::MathConstants<float>::pi,
                                   juce::MathConstants<float>::pi,
                                  -1.0f, 1.0f);
            }, 128);
            break;

        case 2: // square
            osc.initialise([](float x) { return x < 0.0f ? -1.0f : 1.0f; }, 128);
            break;

        default:
            break;
    }
}

void HarmoniaVoice::updateFilterType()
{
    if (currentParams.filterType.containsIgnoreCase("Band"))
        filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
    else if (currentParams.filterType.containsIgnoreCase("High"))
        filter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
    else
        filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
}

void HarmoniaVoice::startNote (int midiNoteNumber, float velocity,
                               juce::SynthesiserSound*, int)
{
    juce::ignoreUnused(velocity);

    auto freqHz = juce::MidiMessage::getMidiNoteInHertz(midiNoteNumber);
    osc.setFrequency((float) freqHz);

    adsr.noteOn();
}

void HarmoniaVoice::stopNote (float velocity, bool allowTailOff)
{
    juce::ignoreUnused(velocity);

    if (allowTailOff)
    {
        adsr.noteOff();
    }
    else
    {
        adsr.reset();
        clearCurrentNote();
    }
}

void HarmoniaVoice::renderNextBlock (juce::AudioBuffer<float>& buffer,
                                     int startSample, int numSamples)
{
    if (!isVoiceActive())
        return;

    const int numChannels = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        float s = osc.processSample(0.0f);
        s = filter.processSample(0, s);

        const float env = adsr.getNextSample();

        // IMPORTANT: volume appliqué ici = changement immédiat assuré
        const float out = s * env * currentParams.volume;

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.addSample(ch, startSample + i, out);
    }

    if (!adsr.isActive())
        clearCurrentNote();
}
