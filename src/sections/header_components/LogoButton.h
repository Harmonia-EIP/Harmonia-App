#pragma once

#include "HeaderIncludes.h"

//==============================================================================
class LogoButton : public juce::Button,
                   private juce::Timer
{
public:
    LogoButton() : juce::Button (juce::String())
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void setLogo (const juce::Image& img) { logo = img; repaint(); }
    void setTint (juce::Colour c)         { tint = c; repaint(); }
    void setRefine (bool);

    void paintButton (juce::Graphics&, bool over, bool down) override;
    void buttonStateChanged() override { startTimerHz (60); }

    // appelé au relâchement du clic : lance un tour complet du logo
    void clicked() override
    {
        spinning = true;
        spinProgress = 0.0f;
        startTimerHz (60);
    }

private:
    void timerCallback() override;

    juce::Image logo;
    juce::Colour tint { juce::Colours::white };
    float hoverAnim = 0.0f;
    float spinProgress = 0.0f;
    bool spinning = false;
};
