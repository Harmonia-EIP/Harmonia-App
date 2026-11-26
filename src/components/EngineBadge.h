/**
 * @file EngineBadge.h
 * @brief Small label telling which engine plays the preset ("DX7" when the DX7 mode is on).
 */

#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/HarmoniaPalette.h"

class EngineBadge : public juce::Component
{
public:
    EngineBadge() { setInterceptsMouseClicks (false, false); }

    void setText (const juce::String& newText)
    {
        text = newText;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat().reduced (0.5f);
        g.setColour (HarmoniaPalette::accent.withAlpha (0.16f));
        g.fillRoundedRectangle (r, r.getHeight() * 0.5f);
        g.setColour (HarmoniaPalette::accent.withAlpha (0.55f));
        g.drawRoundedRectangle (r, r.getHeight() * 0.5f, 1.0f);
        g.setColour (HarmoniaPalette::accent);
        g.setFont (juce::Font (juce::FontOptions (9.5f).withStyle ("Bold")).withExtraKerningFactor (0.14f));
        g.drawText (text, getLocalBounds(), juce::Justification::centred, false);
    }

private:
    juce::String text;
};
