#pragma once

#include "HeaderIncludes.h"

//==============================================================================
class IconButton : public juce::Button,
                   private juce::Timer
{
public:
    enum class Icon { Home, Import, Export };

    explicit IconButton (Icon i) : juce::Button (juce::String()), icon (i)
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void paintButton (juce::Graphics&, bool over, bool down) override;
    void buttonStateChanged() override { startTimerHz (60); }

private:
    void timerCallback() override;
    static juce::Path makePath (Icon);

    Icon icon;
    float hoverAnim = 0.0f;
};
