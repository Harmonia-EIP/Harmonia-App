#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/HarmoniaPalette.h"
#include "OscilloscopeComponent.h"

class DisplayScreen : public juce::Component
{
public:
    explicit DisplayScreen (OscilloscopeComponent& oscillo)
        : oscilloscope (oscillo)
    {
        addAndMakeVisible (oscilloscope);
    }

    void paint (juce::Graphics& g) override
    {
        const auto rOuter = getLocalBounds().toFloat();
        const auto r      = rOuter.reduced (3.0f);
        const float corner = 6.0f;

        for (int i = 8; i >= 1; --i)
        {
            const float a = 0.06f - i * 0.005f;
            if (a <= 0.0f) continue;
            g.setColour (HarmoniaPalette::accent.withAlpha (a));
            g.drawRoundedRectangle (r.expanded ((float) i), corner + i, 1.0f);
        }

        g.setColour (HarmoniaPalette::screenBg);
        g.fillRoundedRectangle (r, corner);

        g.setColour (HarmoniaPalette::accent.withAlpha (0.6f));
        g.drawRoundedRectangle (r, corner, 1.0f);

        g.setColour (HarmoniaPalette::screenGrid.withAlpha (0.6f));
        const int rows = 4;
        for (int i = 1; i < rows; ++i)
        {
            const float y = r.getY() + r.getHeight() * (float) i / (float) rows;
            g.drawHorizontalLine ((int) y, r.getX() + 8.0f, r.getRight() - 8.0f);
        }

        g.setColour (HarmoniaPalette::accent.withAlpha (0.5f));
        g.setFont (juce::Font (juce::FontOptions (9.0f).withStyle ("Bold")));
        g.drawText ("OSCILLOSCOPE",
                    r.toNearestInt().reduced (10, 6).removeFromTop (12),
                    juce::Justification::topLeft);
    }

    void resized() override
    {
        oscilloscope.setBounds (getLocalBounds().reduced (10, 18));
    }

private:
    OscilloscopeComponent& oscilloscope;
};
