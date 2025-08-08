#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"

class ADSRComponent : public juce::Component
{
public:
    ADSRComponent(CustomLookAndFeel& lnF);
    ~ADSRComponent() override;

    void resized() override;

    std::vector<double> getSlidersInfo() const;
    void setAttack(double);
    void setDecay(double);
    void setSustain(double);
    void setRelease(double);
    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;


private:
    juce::Label attackLabel, decayLabel, sustainLabel, releaseLabel;
    CustomLookAndFeel& lookAndFeel;

    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name,
                     float min, float max, float def);
};
