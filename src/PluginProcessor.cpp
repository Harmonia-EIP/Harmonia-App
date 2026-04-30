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
    return "HarmoniaPlugin";
}

bool HarmoniaAudioProcessor::acceptsMidi() const { return true; }
bool HarmoniaAudioProcessor::producesMidi() const { return false; }
bool HarmoniaAudioProcessor::isMidiEffect() const { return false; }
double HarmoniaAudioProcessor::getTailLengthSeconds() const { return 0.0; }

//==============================================================================
int HarmoniaAudioProcessor::getNumPrograms() { return 1; }
int HarmoniaAudioProcessor::getCurrentProgram() { return 0; }
void HarmoniaAudioProcessor::setCurrentProgram (int) {}
const juce::String HarmoniaAudioProcessor::getProgramName (int) { return {}; }
void HarmoniaAudioProcessor::changeProgramName (int, const juce::String&) {}


void HarmoniaAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.clearVoices();
    synth.clearSounds();

    for (int i = 0; i < 8; ++i)
        synth.addVoice(new HarmoniaVoice());

    synth.addSound(new HarmoniaSound());

    synth.setCurrentPlaybackSampleRate(sampleRate);

    // 🔥 préparation des voices (important pour DSP)
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<HarmoniaVoice*>(synth.getVoice(i)))
        {
            voice->prepare(sampleRate,
                           samplesPerBlock,
                           getTotalNumOutputChannels());
        }
    }
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
    buffer.clear();

    keyboardState.processNextMidiBuffer(
        midiMessages,
        0,
        buffer.getNumSamples(),
        true
    );

    synth.renderNextBlock(buffer, midiMessages, 0, buffer.getNumSamples());

    {
        const juce::ScopedLock sl(oscLock);

        if (oscilloscope != nullptr)
            oscilloscope->pushBuffer(buffer, 0, buffer.getNumSamples());
    }
}


bool HarmoniaAudioProcessor::hasEditor() const { return true; }

juce::AudioProcessorEditor* HarmoniaAudioProcessor::createEditor()
{
    return new HarmoniaAudioProcessorEditor (*this);
}


void HarmoniaAudioProcessor::getStateInformation (juce::MemoryBlock&) {}
void HarmoniaAudioProcessor::setStateInformation (const void*, int) {}

juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new HarmoniaAudioProcessor();
}

juce::MidiKeyboardState& HarmoniaAudioProcessor::getKeyboardState()
{
    return keyboardState;
}

void HarmoniaAudioProcessor::setParams(const PatchParams& params)
{
    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* voice = dynamic_cast<HarmoniaVoice*>(synth.getVoice(i)))
        {
            voice->setParameters(params);
        }
    }
}

void HarmoniaAudioProcessor::setOscilloscope(OscilloscopeComponent* osc)
{
    const juce::ScopedLock sl(oscLock);
    oscilloscope = osc;
}