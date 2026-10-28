#include "PluginProcessor.h"
#include "PluginEditor.h"

HarmoniaAudioProcessor::HarmoniaAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       ),
       apvts (*this, nullptr, "HARMONIA", HarmoniaParamsV3::createLayout())
#else
     : apvts (*this, nullptr, "HARMONIA", HarmoniaParamsV3::createLayout())
#endif
{
    paramRefs.bind (apvts);
}

HarmoniaAudioProcessor::~HarmoniaAudioProcessor()
{
    // Projet fermé / plugin déchargé : envoie le changement de palette en attente
    // (non bloquant, thread détaché avec timeout court).
    backend.flushPaletteIfPending();
}

const juce::String HarmoniaAudioProcessor::getName() const   { return "HarmoniaPlugin"; }
bool HarmoniaAudioProcessor::acceptsMidi() const             { return true; }
bool HarmoniaAudioProcessor::producesMidi() const            { return false; }
bool HarmoniaAudioProcessor::isMidiEffect() const            { return false; }
double HarmoniaAudioProcessor::getTailLengthSeconds() const  { return 12.0; }  // longest reverb: T60 = 12 s

int HarmoniaAudioProcessor::getNumPrograms()                                  { return 1; }
int HarmoniaAudioProcessor::getCurrentProgram()                               { return 0; }
void HarmoniaAudioProcessor::setCurrentProgram (int)                          {}
const juce::String HarmoniaAudioProcessor::getProgramName (int)               { return {}; }
void HarmoniaAudioProcessor::changeProgramName (int, const juce::String&)     {}

void HarmoniaAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.clearVoices();
    synth.clearSounds();

    for (int i = 0; i < 8; ++i)
        synth.addVoice (new HarmoniaVoice (patch, (std::uint64_t) i + 1));

    synth.addSound (new HarmoniaSound());
    synth.setCurrentPlaybackSampleRate (sampleRate);

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<HarmoniaVoice*> (synth.getVoice (i)))
            voice->prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());

    effects.prepare (sampleRate);
    effectsScratch.setSize (2, juce::jmax (1, samplesPerBlock));
}

void HarmoniaAudioProcessor::releaseResources() {}

#ifndef JucePlugin_PreferredChannelConfigurations
bool HarmoniaAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
}
#endif

void HarmoniaAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;
    buffer.clear();

    for (int i = 0; i < HarmoniaCharter::analog::count; ++i)
        patch[(size_t) i] = paramRefs.analog (i);

    const int numSamples = buffer.getNumSamples();
    keyboardState.processNextMidiBuffer (midiMessages, 0, numSamples, true);
    synth.renderNextBlock (buffer, midiMessages, 0, numSamples);

    if (buffer.getNumChannels() >= 2)
    {
        effects.process (patch, buffer.getWritePointer (0), buffer.getWritePointer (1), numSamples);
    }
    else if (buffer.getNumChannels() == 1)
    {
        // mono output: the effects run on the voices' mix in both channels, then fold back
        const int chunk = effectsScratch.getNumSamples();
        for (int start = 0; start < numSamples; start += chunk)
        {
            const int n = juce::jmin (chunk, numSamples - start);
            effectsScratch.copyFrom (0, 0, buffer, 0, start, n);
            effectsScratch.copyFrom (1, 0, buffer, 0, start, n);
            effects.process (patch, effectsScratch.getWritePointer (0), effectsScratch.getWritePointer (1), n);
            buffer.copyFrom (0, start, effectsScratch.getReadPointer (0), n, 0.5f);
            buffer.addFrom (0, start, effectsScratch, 1, 0, n, 0.5f);
        }
    }

    {
        const juce::ScopedLock sl (oscLock);
        if (oscilloscope != nullptr)
            oscilloscope->pushBuffer (buffer, 0, buffer.getNumSamples());
    }
}

bool HarmoniaAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* HarmoniaAudioProcessor::createEditor()
{
    return new HarmoniaAudioProcessorEditor (*this);
}

namespace
{
    const juce::Identifier charterVersionId ("charter_version");
}

void HarmoniaAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
    {
        state.setProperty (charterVersionId, HarmoniaParamsV3::toJuce (HarmoniaCharter::version), nullptr);
        if (auto xml = state.createXml())
            copyXmlToBinary (*xml, destData);
    }
}

void HarmoniaAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    const auto xml = getXmlFromBinary (data, sizeInBytes);
    if (xml == nullptr || ! xml->hasTagName (apvts.state.getType()))
        return;

    auto state = juce::ValueTree::fromXml (*xml);
    if (state.getProperty (charterVersionId).toString().startsWith ("3"))
    {
        apvts.replaceState (state);
        return;
    }

    // Saved before charter v3: the 20 v2 parameters, stored as physical values, become v3 by the
    // charter's rules; every other parameter takes its default; the UI properties are kept.
    std::array<float, HarmoniaCharter::v2Ids.size()> v2 {};
    for (size_t i = 0; i < v2.size(); ++i)
    {
        const auto id = HarmoniaParamsV3::toJuce (HarmoniaCharter::v2Ids[i]);
        const auto child = state.getChildWithProperty ("id", id);
        v2[i] = child.isValid() ? (float) child.getProperty ("value")
                                : HarmoniaCharter::fromNormalized (HarmoniaCharter::v2Params[i], 0.0f);
    }
    for (int i = state.getNumChildren(); --i >= 0;)
        if (state.getChild (i).hasProperty ("id") && state.getChild (i).hasProperty ("value"))
            state.removeChild (i, nullptr);
    apvts.replaceState (state);
    HarmoniaParamsV3::applyV2State (apvts, v2);
}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HarmoniaAudioProcessor();
}

juce::MidiKeyboardState& HarmoniaAudioProcessor::getKeyboardState()
{
    return keyboardState;
}

void HarmoniaAudioProcessor::setOscilloscope (OscilloscopeComponent* osc)
{
    const juce::ScopedLock sl (oscLock);
    oscilloscope = osc;
}