#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"

class ADSRComponent : public juce::Component
{
public:
    ADSRComponent(CustomLookAndFeel& lnF);
    ~ADSRComponent() override;

    void resized() override;

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;

private:
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;
    CustomLookAndFeel& lookAndFeel;

    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name,
                     float min, float max, float def);
};
