/**
 * @file SectionPanel.h
 * @brief Styled container panel used to group synthesizer controls.
 *
 * SectionPanel provides a reusable visual container
 * with:
 * - Title rendering
 * - Accent color support
 * - Decorative LED indicator
 * - Gradient backgrounds
 * - Content margin helpers
 */
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/HarmoniaPalette.h"

/**
 * @class SectionPanel
 * @brief Visual container for synthesizer UI sections.
 *
 * This component is used to visually separate
 * synthesizer modules such as:
 * - Oscillators
 * - Filters
 * - Envelopes
 * - LFOs
 *
 * It provides a hardware-inspired interface style.
 */
class SectionPanel : public juce::Component
{
public:
    SectionPanel (const juce::String& titleText,
                  juce::Colour tintCol = HarmoniaPalette::accent)
        : title (titleText), tint (tintCol)
    {
    }

    void setAccentColour (juce::Colour newColour)
    {
        tint = newColour;
        repaint();
    }

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat().reduced (1.0f);
        const float corner = 10.0f;

        {
            juce::Path shadowPath;
            shadowPath.addRoundedRectangle (r.translated (0.0f, 1.5f), corner);
            juce::DropShadow ds (juce::Colours::black.withAlpha (0.30f), 8, { 0, 2 });
            ds.drawForPath (g, shadowPath);
        }

        juce::ColourGradient bg (HarmoniaPalette::panelTop, r.getX(), r.getY(),
                                 HarmoniaPalette::panel,    r.getX(), r.getBottom(), false);
        g.setGradientFill (bg);
        g.fillRoundedRectangle (r, corner);

        g.setColour (juce::Colours::white.withAlpha (0.06f));
        g.drawLine (r.getX() + corner, r.getY() + 1.0f,
                    r.getRight() - corner, r.getY() + 1.0f, 1.0f);

        g.setColour (HarmoniaPalette::border);
        g.drawRoundedRectangle (r, corner, 1.0f);

        const auto titleArea = juce::Rectangle<int> (16, 9, getWidth() - 32, 14);

        const float ledR = 3.0f;
        const float ledX = (float) titleArea.getX();
        const float ledY = (float) titleArea.getCentreY();

        g.setColour (tint.withAlpha (0.40f));
        g.fillEllipse (ledX - ledR * 1.6f, ledY - ledR * 1.6f, ledR * 3.2f, ledR * 3.2f);
        g.setColour (tint);
        g.fillEllipse (ledX - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f);

        g.setColour (tint);
        g.setFont (juce::Font (juce::FontOptions (10.0f).withStyle ("Bold"))
                       .withExtraKerningFactor (0.20f));
        g.drawText (title.toUpperCase(),
                    titleArea.withTrimmedLeft (12),
                    juce::Justification::centredLeft);

        g.setColour (tint.withAlpha (0.30f));
        g.fillRect (juce::Rectangle<float> (16.0f, 24.0f,
                                            (float) getWidth() - 32.0f, 1.0f));
    }

    juce::Rectangle<int> getContentBounds() const
    {
        return getLocalBounds()
            .withTrimmedTop (32)
            .withTrimmedLeft (12)
            .withTrimmedRight (12)
            .withTrimmedBottom (12);
    }

    juce::Colour getTint() const noexcept { return tint; }

private:
    juce::String title;
    juce::Colour tint;
};
