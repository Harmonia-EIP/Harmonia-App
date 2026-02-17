#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/AppLookAndFeel.h"

class FrequencyVolumeComponent : public juce::Component
{
public:
    explicit FrequencyVolumeComponent (AppLookAndFeel& lnF);
    ~FrequencyVolumeComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;
    void lookAndFeelChanged() override;

    void setFreqVol (double frequency, double volume);
    void setFrequency (double freq);
    void setVolume (double vol);

    std::pair<double, double> getFreqVol() const;
    double getFreq() const;
    double getVolume() const;

private:
    void setupSlider (juce::Slider& slider,
                      juce::Label& label,
                      const juce::String& name,
                      double min, double max, double def);

    void applyThemeColours();

    AppLookAndFeel& lookAndFeel;

    juce::Slider frequencySlider;
    juce::Slider volumeSlider;

    juce::Label frequencyLabel;
    juce::Label volumeLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (FrequencyVolumeComponent)
};
