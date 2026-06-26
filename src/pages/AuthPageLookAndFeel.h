#pragma once

#include "PagesIncludes.h"

/**
 * @class AuthPageLookAndFeel
 * @brief Shared LookAndFeel for LoginPage and SignupPage buttons.
 *
 * Provides two styles:
 *  - Primary : semi-transparent with waveBlue border accent (sign-in / create account)
 *  - Back    : ghost style with subtle border and muted arrow prefix
 */
class AuthPageLookAndFeel : public juce::LookAndFeel_V4
{
public:
    enum class Style { Primary, Back };

    explicit AuthPageLookAndFeel(Style s) : style(s) {}

    void drawButtonBackground(juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& /*bgColour*/,
                               bool isHighlighted,
                               bool isDown) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        const float r = 8.f;

        if (style == Style::Primary)
        {
            const float bgAlpha     = isDown ? 0.14f : (isHighlighted ? 0.10f : 0.06f);
            const float borderAlpha = isDown ? 0.50f : (isHighlighted ? 0.38f : 0.22f);

            g.setColour(juce::Colours::white.withAlpha(bgAlpha));
            g.fillRoundedRectangle(bounds, r);

            if (isHighlighted)
            {
                g.setColour(HarmoniaColours::waveBlue.withAlpha(0.14f));
                g.drawRoundedRectangle(bounds.expanded(1.5f), r + 1.5f, 2.5f);
            }

            g.setColour(HarmoniaColours::waveBlue.withAlpha(borderAlpha));
            g.drawRoundedRectangle(bounds, r, 1.f);
        }
        else // Back
        {
            const float bgAlpha     = isHighlighted ? 0.05f : 0.0f;
            const float borderAlpha = isHighlighted ? 0.18f : 0.10f;

            g.setColour(juce::Colours::white.withAlpha(bgAlpha));
            g.fillRoundedRectangle(bounds, r);

            g.setColour(juce::Colours::white.withAlpha(borderAlpha));
            g.drawRoundedRectangle(bounds, r, 1.f);
        }
    }

    void drawButtonText(juce::Graphics& g,
                        juce::TextButton& button,
                        bool isHighlighted,
                        bool /*isDown*/) override
    {
        const auto b = button.getLocalBounds();

        if (style == Style::Primary)
        {
            g.setColour(isHighlighted
                        ? HarmoniaColours::textPrimary
                        : HarmoniaColours::textPrimary.withAlpha(0.82f));
            g.setFont(juce::Font("Inter", 14.f, juce::Font::plain));
            g.drawFittedText(button.getButtonText(), b, juce::Justification::centred, 1);
        }
        else // Back
        {
            const juce::Colour col = isHighlighted
                ? juce::Colours::white.withAlpha(0.55f)
                : juce::Colours::white.withAlpha(0.30f);

            g.setFont(juce::Font("Inter", 13.f, juce::Font::plain));

            // Arrow glyph
            auto arrowBounds = b.withWidth(28);
            g.setColour(col.withAlpha(col.getAlpha() * 0.75f));
            g.drawFittedText(juce::CharPointer_UTF8("\xe2\x86\x90"),
                             arrowBounds, juce::Justification::centred, 1);

            // Label
            g.setColour(col);
            g.drawFittedText(button.getButtonText(),
                             b.withTrimmedLeft(20), juce::Justification::centred, 1);
        }
    }

private:
    Style style;
};
