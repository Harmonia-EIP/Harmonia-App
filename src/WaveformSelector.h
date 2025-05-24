
#include "JuceHeader.h"

class WaveformSelector : public juce::ComboBox
{
public:
    WaveformSelector(const juce::String& name)
    {
        setName(name);
        addItem("Sine", 1);
        addItem("Square", 2);
        addItem("Saw", 3);
        addItem("Triangle", 4);
        setSelectedId(1);
    }

    juce::String getSelectedWaveform() const
    {
        return getText();
    }
};
