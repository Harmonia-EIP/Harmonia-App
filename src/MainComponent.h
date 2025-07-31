#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "FrequencySlider.h"
#include "VolumeSlider.h"
#include "GenerateButton.h"
#include "DataModel.h"
#include "CustomLookAndFeel.h"
#include "components/TitleComponent.h"
#include "components/TopBarComponent.h"

class MainComponent : public juce::Component
{
public:
    MainComponent();
     ~MainComponent() override;
    void paint(juce::Graphics&) override;
    void resized() override;

    FrequencySlider frequencySlider;
    VolumeSlider volumeSlider;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;

    juce::ComboBox filterTypeSelector;
    juce::Slider cutoffSlider, resonanceSlider;
    juce::Label cutoffLabel, resonanceLabel;

    juce::Label frequencyLabel;
    juce::Label volumeLabel;

    GenerateButton generateButton;

private:
    CustomLookAndFeel customLookAndFeel;

    // Components

    TitleComponent title { "Harmonia" };
    TopBarComponent topBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
