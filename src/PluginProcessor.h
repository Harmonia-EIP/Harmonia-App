#pragma once

#include "JuceHeader.h"
#include "parameters/HarmoniaParameters.h"
#include "components/Synth.h"
#include "components/OscilloscopeComponent.h"
#include "backendManagement/BackendManager.h"

class HarmoniaAudioProcessor  : public juce::AudioProcessor
{
public:
    HarmoniaAudioProcessor();
    ~HarmoniaAudioProcessor() override;

    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::MidiKeyboardState& getKeyboardState();
    juce::AudioProcessorValueTreeState& getAPVTS() { return apvts; }

    void setOscilloscope (OscilloscopeComponent* osc);

    BackendManager& getBackend();

private:
    juce::AudioProcessorValueTreeState apvts;
    HarmoniaParams::AtomicRefs paramRefs;

    std::unique_ptr<BackendManager> backend;

    juce::Synthesiser synth;
    juce::MidiKeyboardState keyboardState;

    juce::dsp::Reverb reverb;
    juce::dsp::Reverb::Parameters reverbParams;

    OscilloscopeComponent* oscilloscope = nullptr;
    juce::CriticalSection oscLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmoniaAudioProcessor)
};
