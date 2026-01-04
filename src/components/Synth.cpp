#include "Synth.h"
#include <cmath>

//==============================================================================
// Constructeur
HarmoniaVoice::HarmoniaVoice()
{
    // Forme d'onde par défaut: sinus
    osc.initialise ([] (float x) { return std::sin (x); }, 128);

    filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
    gain.setGainLinear (0.8f);

    currentParams = HarmoniaParams{};
}

//==============================================================================
// Préparation DSP
void HarmoniaVoice::prepare (double sampleRate, int samplesPerBlock, int numOutputChannels)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = static_cast<juce::uint32> (samplesPerBlock);
    spec.numChannels      = static_cast<juce::uint32> (numOutputChannels);

    osc.prepare (spec);
    filter.prepare (spec);
    gain.prepare (spec);

    adsr.setSampleRate (sampleRate);
}

//==============================================================================
// Peut-on jouer ce son ?
bool HarmoniaVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<HarmoniaSound*> (sound) != nullptr;
}

//==============================================================================
// Màj complète des paramètres
void HarmoniaVoice::setParameters (const HarmoniaParams& newParams)
{
    currentParams = newParams;

    // ADSR
    adsrParams.attack  = newParams.attack;
    adsrParams.decay   = newParams.decay;
    adsrParams.sustain = newParams.sustain;
    adsrParams.release = newParams.release;
    adsr.setParameters (adsrParams);

    // Filtre
    filter.setCutoffFrequency (newParams.cutoff);
    filter.setResonance       (newParams.resonance);
    updateFilterType();

    // Volume
    gain.setGainLinear (newParams.volume);

    // Oscillateur
    updateOscillator();
}

//==============================================================================
// Petit helpers internes

void HarmoniaVoice::updateOscillator()
{
    switch (currentParams.waveformIndex)
    {
        case 0: // Sine
            osc.initialise ([] (float x) { return std::sin (x); }, 128);
            break;

        case 1: // Saw
            osc.initialise ([] (float x)
            {
                return juce::jmap (x,
                                   -juce::MathConstants<float>::pi,
                                    juce::MathConstants<float>::pi,
                                   -1.0f, 1.0f);
            }, 128);
            break;

        case 2: // Square
            osc.initialise ([] (float x) { return x < 0.0f ? -1.0f : 1.0f; }, 128);
            break;

        default:
            break;
    }
}

void HarmoniaVoice::updateFilterType()
{
    if (currentParams.filterType.containsIgnoreCase ("Band"))
        filter.setType (juce::dsp::StateVariableTPTFilterType::bandpass);
    else if (currentParams.filterType.containsIgnoreCase ("High"))
        filter.setType (juce::dsp::StateVariableTPTFilterType::highpass);
    else
        filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);
}

//==============================================================================
// API "confort" utilisée par MainComponent

void HarmoniaVoice::setADSR (float attack, float decay, float sustain, float release)
{
    currentParams.attack  = attack;
    currentParams.decay   = decay;
    currentParams.sustain = sustain;
    currentParams.release = release;

    setParameters (currentParams);
}

void HarmoniaVoice::setFilter (float cutoff, float resonance, int filterTypeIndex)
{
    currentParams.cutoff    = cutoff;
    currentParams.resonance = resonance;

    // 0 = Low, 1 = Band, 2 = High (comme dans ton MainComponent)
    switch (filterTypeIndex)
    {
        case 1: currentParams.filterType = "Band Pass"; break;
        case 2: currentParams.filterType = "High Pass"; break;
        default: currentParams.filterType = "Low Pass"; break;
    }

    setParameters (currentParams);
}

void HarmoniaVoice::setFilter (float cutoff, float resonance, const juce::String& filterTypeName)
{
    currentParams.cutoff     = cutoff;
    currentParams.resonance  = resonance;
    currentParams.filterType = filterTypeName;

    setParameters (currentParams);
}

void HarmoniaVoice::setWaveform (int waveformIndex)
{
    currentParams.waveformIndex = waveformIndex;
    setParameters (currentParams);
}

void HarmoniaVoice::setWaveform (const juce::String& waveformName)
{
    if (waveformName.containsIgnoreCase ("saw"))
        currentParams.waveformIndex = 1;
    else if (waveformName.containsIgnoreCase ("square") || waveformName.containsIgnoreCase ("pulse"))
        currentParams.waveformIndex = 2;
    else
        currentParams.waveformIndex = 0; // défaut: sine

    setParameters (currentParams);
}

//==============================================================================
// Gestion des notes
void HarmoniaVoice::startNote (int midiNoteNumber, float velocity,
                               juce::SynthesiserSound*,
                               int /*currentPitchWheelPosition*/)
{
    juce::ignoreUnused (velocity);

    // On ignore currentParams.frequency et on prend la vraie note MIDI
    auto freqHz = juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    osc.setFrequency ((float) freqHz);

    adsr.noteOn();
}


void HarmoniaVoice::stopNote (float velocity, bool allowTailOff)
{
    juce::ignoreUnused (velocity);

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

//==============================================================================
// Rendu audio
void HarmoniaVoice::renderNextBlock (juce::AudioBuffer<float>& buffer,
                                     int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    const auto numChannels = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        float s = osc.processSample (0.0f);        // oscillateur
        s = filter.processSample (0, s);          // filtre mono
        s = gain.processSample (s);               // gain

        const float env = adsr.getNextSample();   // enveloppe
        const float out = s * env;

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.addSample (ch, startSample + i, out);
    }

    if (! adsr.isActive())
        clearCurrentNote();
}
