#pragma once

#define JUCE_USE_NATIVE_FILECHOOSERS 1
#include <juce_gui_basics/juce_gui_basics.h>
#include "FrequencySlider.h"
#include "VolumeSlider.h"
#include "GenerateButton.h"
#include "DataModel.h"
#include "CustomLookAndFeel.h"
#include "components/TitleComponent.h"
#include "components/TopBarComponent.h"
#include "components/FrequencyVolumeComponent.h"
#include "components/ADSRComponent.h"
#include "components/FilterComponent.h"
#include "components/BottomBarComponent.h"

class MainComponent : public juce::Component
{
public:
    MainComponent();
     ~MainComponent() override;
    void paint(juce::Graphics&) override;
    void resized() override;

private:
    CustomLookAndFeel customLookAndFeel;

    // Components

    TitleComponent title { "Harmonia" };
    TopBarComponent topBar;
    FrequencyVolumeComponent freqVolComponent;
    ADSRComponent adsrComponent;
    FilterComponent filterComponent;
    BottomBarComponent bottomBar;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};
