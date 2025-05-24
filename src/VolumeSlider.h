
#include "JuceHeader.h"

class VolumeSlider : public juce::Slider
{
public:
    VolumeSlider(const juce::String& name, double minVol, double maxVol, double defaultVol)
    {
        setName(name);
        setRange(minVol, maxVol);
        setValue(defaultVol);
        setTextBoxStyle(juce::Slider::TextBoxRight, false, 100, 20);
    }
};
