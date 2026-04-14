#include "JuceHeader.h"

class GenerateButton : public juce::TextButton
{
public:
    GenerateButton(const juce::String& text)
        : juce::TextButton(text)
    {
        setClickingTogglesState(false);
    }
};
