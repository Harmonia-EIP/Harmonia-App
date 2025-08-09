#pragma once
#include <JuceHeader.h>
#include "../CustomLookAndFeel.h"

class FilterComponent : public juce::Component
{
public:
    FilterComponent(CustomLookAndFeel& lnF);
    ~FilterComponent() override;

    void resized() override;

    std::pair<double, double> getSlidersInfo() const;

    double getCutoff() const;
    double getResonance() const;
    
    void setCutoff(double);
    void setResonance(double);

    juce::Slider cutoffSlider, resonanceSlider;

private:
    juce::Label cutoffLabel, resonanceLabel;
    CustomLookAndFeel& lookAndFeel;

    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name,
                     float min, float max, float def);
};
