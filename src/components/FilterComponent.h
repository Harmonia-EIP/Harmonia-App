#pragma once
#include <JuceHeader.h>
#include "../themes/AppLookAndFeel.h"

class FilterComponent : public juce::Component
{
public:
    FilterComponent(AppLookAndFeel& lnF);
    ~FilterComponent() override;

    void resized() override;
    
    void paint(juce::Graphics& g) override;
    void lookAndFeelChanged() override;

    std::pair<double, double> getSlidersInfo() const;

    void setCutoffResonance(double cutoff, double resonance);
    
    double getCutoff() const;
    double getResonance() const;
    
    void setCutoff(double);
    void setResonance(double);

    juce::Slider cutoffSlider, resonanceSlider;

    std::function<void()> onParamsChanged;

private:
    juce::Label cutoffLabel, resonanceLabel;
    AppLookAndFeel& lookAndFeel;

    void setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name,
                     float min, float max, float def);
};
