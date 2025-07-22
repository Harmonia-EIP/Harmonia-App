#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "WaveformSelector.h"
#include "FrequencySlider.h"
#include "VolumeSlider.h"
#include "GenerateButton.h"
#include "DataModel.h"
#include "CustomLookAndFeel.h"

class MainComponent : public juce::Component
{
public:
    MainComponent();
    void paint(juce::Graphics&) override;
    void resized() override;

    WaveformSelector waveformSelector;
    FrequencySlider frequencySlider;
    VolumeSlider volumeSlider;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;

    juce::ComboBox filterTypeSelector;
    juce::Slider cutoffSlider, resonanceSlider;
    juce::Label cutoffLabel, resonanceLabel;

    juce::Label waveformLabel;
    juce::Label frequencyLabel;
    juce::Label volumeLabel;
    juce::TextEditor promptEditor;
    juce::Label promptLabel;
    juce::TextButton clearPromptButton { "Clear Prompt" };




    GenerateButton generateButton;

private:
    CustomLookAndFeel customLookAndFeel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
