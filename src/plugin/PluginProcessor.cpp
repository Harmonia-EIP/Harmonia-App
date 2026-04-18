#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
HarmoniaAudioProcessor::HarmoniaAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  juce::AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
}

HarmoniaAudioProcessor::~HarmoniaAudioProcessor() {}

//==============================================================================
const juce::String HarmoniaAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool HarmoniaAudioProcessor::acceptsMidi() const { return JucePlugin_WantsMidiInput; }
bool HarmoniaAudioProcessor::producesMidi() const { return JucePlugin_ProducesMidiOutput; }
bool HarmoniaAudioProcessor::isMidiEffect() const { return JucePlugin_IsMidiEffect; }
double HarmoniaAudioProcessor::getTailLengthSeconds() const { return 0.0; }

//==============================================================================
int HarmoniaAudioProcessor::getNumPrograms() { return 1; }
int HarmoniaAudioProcessor::getCurrentProgram() { return 0; }
void HarmoniaAudioProcessor::setCurrentProgram (int) {}
const juce::String HarmoniaAudioProcessor::getProgramName (int) { return {}; }
void HarmoniaAudioProcessor::changeProgramName (int, const juce::String&) {}

//==============================================================================
void HarmoniaAudioProcessor::prepareToPlay (double, int) {}
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

//==============================================================================
void HarmoniaAudioProcessor::processBlock (juce::AudioBuffer<float>& buffer,
                                           juce::MidiBuffer&)
{
    juce::ScopedNoDenormals noDenormals;

    for (int channel = 0; channel < buffer.getNumChannels(); ++channel)
    {
        auto* channelData = buffer.getWritePointer (channel);

        for (int sample = 0; sample < buffer.getNumSamples(); ++sample)
        {
            channelData[sample] *= 0.5f;
        }
    }
}

//==============================================================================
bool HarmoniaAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* HarmoniaAudioProcessor::createEditor()
{
    return new HarmoniaAudioProcessorEditor (*this);
}

//==============================================================================
void HarmoniaAudioProcessor::getStateInformation (juce::MemoryBlock&) {}
void HarmoniaAudioProcessor::setStateInformation (const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HarmoniaAudioProcessor();
}