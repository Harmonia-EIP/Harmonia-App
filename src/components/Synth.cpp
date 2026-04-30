#include "Synth.h"

HarmoniaVoice::HarmoniaVoice()
{
    osc.initialise([](float x) { return std::sin(x); }, AppConfig::Synth::availableRangeSize);

    filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);

    currentParams = PatchParams{};
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

void HarmoniaVoice::setParameters (const PatchParams& newParams)
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
        case 1: currentParams.filterType = FilterType::BANDPASS; break;
        case 2: currentParams.filterType = FilterType::HIGHPASS; break;
        default: currentParams.filterType = FilterType::LOWPASS; break;
    }

    setParameters(currentParams);
}

void HarmoniaVoice::setWaveform (int waveformIndex)
{
    switch (waveformIndex)
    {
        case 1: currentParams.waveform = Waveform::SAW; break;
        case 2: currentParams.waveform = Waveform::SQUARE; break;
        case 3: currentParams.waveform = Waveform::TRIANGLE; break;
        default: currentParams.waveform = Waveform::SINE; break;
    }

    setParameters(currentParams);
}

void HarmoniaVoice::updateOscillator()
{
    switch (currentParams.waveform)
    {
        case Waveform::SINE:
            osc.initialise([](float x) { return std::sin(x); }, AppConfig::Synth::availableRangeSize);
            break;

        case Waveform::SAW:
            osc.initialise([](float x)
            {
                return juce::jmap(x,
                                  -juce::MathConstants<float>::pi,
                                   juce::MathConstants<float>::pi,
                                  -1.0f, 1.0f);
            }, AppConfig::Synth::availableRangeSize);
            break;

        case Waveform::SQUARE:
            osc.initialise([](float x) { return x < 0.0f ? -1.0f : 1.0f; }, AppConfig::Synth::availableRangeSize);
            break;

        case Waveform::TRIANGLE:
            osc.initialise([](float x)
            {
                return std::asin(std::sin(x)) * (2.0f / juce::MathConstants<float>::pi);
            }, AppConfig::Synth::availableRangeSize);
            break;
    }
}

void HarmoniaVoice::updateFilterType()
{
    switch (currentParams.filterType)
    {
        case FilterType::LOWPASS:
            filter.setType(juce::dsp::StateVariableTPTFilterType::lowpass);
            break;

        case FilterType::HIGHPASS:
            filter.setType(juce::dsp::StateVariableTPTFilterType::highpass);
            break;

        case FilterType::BANDPASS:
            filter.setType(juce::dsp::StateVariableTPTFilterType::bandpass);
            break;
    }
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

        const float out = s * env * currentParams.volume;

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.addSample(ch, startSample + i, out);
    }

    if (!adsr.isActive())
        clearCurrentNote();
}
