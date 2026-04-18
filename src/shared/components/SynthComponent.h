#pragma once

#include "ComponentsIncludes.h"

class SynthComponent : public juce::Component
{
public:
    SynthComponent();
    ~SynthComponent() override = default;

    juce::MidiKeyboardState& getKeyboardState() noexcept { return keyboardState; }

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthComponent)
};
