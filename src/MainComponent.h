#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "WaveformSelector.h"
#include "FrequencySlider.h"
#include "VolumeSlider.h"
#include "GenerateButton.h"
#include "DataModel.h"

class MainComponent : public juce::Component
{
public:
    MainComponent();
    void paint(juce::Graphics&) override;
    void resized() override;

    WaveformSelector waveformSelector;
    FrequencySlider frequencySlider;
    VolumeSlider volumeSlider;
    GenerateButton generateButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
