#pragma once
#include <JuceHeader.h>

class TitleComponent : public juce::Component
{
public:
    explicit TitleComponent(const juce::String& textToDisplay);

    void resized() override;

private:
    juce::Label label;
    juce::String title;
};
