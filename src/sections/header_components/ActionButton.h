#pragma once

#include "HeaderIncludes.h"

//==============================================================================
class ActionButton : public juce::Button
{
public:
    ActionButton() : juce::Button (juce::String())
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void setTint (juce::Colour c)      { tint = c; repaint(); }
    void setSharpness (float s)        { sharp = s; repaint(); }   // 0 = pilule, 1 = coins secs
    void paintButton (juce::Graphics&, bool over, bool down) override;

private:
    juce::Colour tint { juce::Colours::white };
    float sharp = 0.0f;
};
