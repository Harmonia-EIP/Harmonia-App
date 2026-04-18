#pragma once

#include "../shared/JuceHeader.h"
#include "PluginAppWrapper.h"
#include "../config/AppConfig.h"

class HarmoniaAudioProcessorEditor  : public juce::AudioProcessorEditor
{
public:
    HarmoniaAudioProcessorEditor (HarmoniaAudioProcessor&);
    ~HarmoniaAudioProcessorEditor() override;

    void paint (juce::Graphics&) override;
    void resized() override;

private:
    HarmoniaAudioProcessor& audioProcessor;

    PluginAppWrapper pluginAppWrapper;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HarmoniaAudioProcessorEditor)
};