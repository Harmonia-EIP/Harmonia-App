#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"
#include "../FrequencySlider.h"
#include "../VolumeSlider.h"

class FrequencyVolumeComponent : public juce::Component
{
public:
    FrequencyVolumeComponent(CustomLookAndFeel& lnF);
    ~FrequencyVolumeComponent() override;

    void resized() override;

    FrequencySlider frequencySlider;
    VolumeSlider volumeSlider;

private:
    juce::Label frequencyLabel, volumeLabel;
    CustomLookAndFeel& lookAndFeel;

    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name);
};
