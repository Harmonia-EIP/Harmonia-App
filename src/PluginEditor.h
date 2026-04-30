#pragma once

#include "PluginProcessor.h"
#include "JuceHeader.h"
#include "config/AppConfig.h"
#include "components/SynthComponent.h"
#include "AppController.h"

class HarmoniaAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    HarmoniaAudioProcessorEditor (HarmoniaAudioProcessor&);
    ~HarmoniaAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    HarmoniaAudioProcessor& audioProcessor;

    AppController appController;

    SynthComponent synthComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmoniaAudioProcessorEditor)
};