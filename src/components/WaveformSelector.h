#pragma once

#include "ComponentsIncludes.h"

class WaveformSelector : public juce::ComboBox
{
public:
    WaveformSelector(const juce::String& name)
    {
        setName(name);
    }

    juce::String getSelectedWaveform() const
    {
        return getText();
    }
};
