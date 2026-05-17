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
       apvts (*this, nullptr, "HARMONIA", HarmoniaParams::createLayout())
#else
     : apvts (*this, nullptr, "HARMONIA", HarmoniaParams::createLayout())
#endif
{
    paramRefs.bind (apvts);
}

HarmoniaAudioProcessor::~HarmoniaAudioProcessor() {}

const juce::String HarmoniaAudioProcessor::getName() const   { return "HarmoniaPlugin"; }
bool HarmoniaAudioProcessor::acceptsMidi() const             { return true; }
bool HarmoniaAudioProcessor::producesMidi() const            { return false; }
bool HarmoniaAudioProcessor::isMidiEffect() const            { return false; }
double HarmoniaAudioProcessor::getTailLengthSeconds() const  { return 4.0; }

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
        synth.addVoice (new HarmoniaVoice (paramRefs));

    synth.addSound (new HarmoniaSound());
    synth.setCurrentPlaybackSampleRate (sampleRate);

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* voice = dynamic_cast<HarmoniaVoice*> (synth.getVoice (i)))
            voice->prepare (sampleRate, samplesPerBlock, getTotalNumOutputChannels());

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels      = (juce::uint32) juce::jmax (1, getTotalNumOutputChannels());
    reverb.prepare (spec);
    reverb.reset();
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

    keyboardState.processNextMidiBuffer (midiMessages, 0, buffer.getNumSamples(), true);
    synth.renderNextBlock (buffer, midiMessages, 0, buffer.getNumSamples());


    const float wet = paramRefs.reverbMix != nullptr ? paramRefs.reverbMix->load() : 0.0f;
    reverbParams.wetLevel = wet;
    reverbParams.dryLevel = 1.0f - wet * 0.7f;
    reverbParams.roomSize = 0.55f + wet * 0.35f;
    reverbParams.damping  = 0.4f;
    reverbParams.width    = 1.0f;
    reverb.setParameters (reverbParams);

    juce::dsp::AudioBlock<float> block (buffer);
    juce::dsp::ProcessContextReplacing<float> ctx (block);
    reverb.process (ctx);

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

void HarmoniaAudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    if (auto state = apvts.copyState(); state.isValid())
        if (auto xml = state.createXml())
            copyXmlToBinary (*xml, destData);
}

void HarmoniaAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    if (auto xml = getXmlFromBinary (data, sizeInBytes))
        if (xml->hasTagName (apvts.state.getType()))
            apvts.replaceState (juce::ValueTree::fromXml (*xml));
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