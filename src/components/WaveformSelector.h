#pragma once

#include "ComponentsIncludes.h"

class WaveformSelector : public juce::ComboBox
{
public:
    WaveformSelector(const juce::String& name)
    {
        setName(name);
        addItem(Strings::Waveform::Sine, 1);
        addItem(Strings::Waveform::Square, 2);
        addItem(Strings::Waveform::Saw, 3);
        addItem(Strings::Waveform::Triangle, 4);
        setSelectedId(1);
    }

    juce::String getSelectedWaveform() const
    {
        return getText();
    }
};
