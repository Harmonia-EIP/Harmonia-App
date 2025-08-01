#pragma once
#include "JuceHeader.h"

class FrequencySlider : public juce::Slider
{
public:
    FrequencySlider(const juce::String& name, double minHz, double maxHz, double defaultHz)
    {
        setName(name);
        setRange(minHz, maxHz);
        setValue(defaultHz);
        setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    }
};
