#pragma once

#include "PagesIncludes.h"

/**
 * @class AuthPageLookAndFeel
 * @brief Shared LookAndFeel for LoginPage and SignupPage (buttons + text fields).
 *
 * Styles:
 *  - Primary : blue gradient fill, glowing waveBlue border (sign-in / create account)
 *  - Back    : ghost style with a drawn arrow that slides left on hover
 *  - Field   : text editors (use applyTo()); default style when none is given
 */
class AuthPageLookAndFeel : public juce::LookAndFeel_V4
{
public:
    enum class Style { Primary, Back, Field };

    explicit AuthPageLookAndFeel(Style s = Style::Field) : style(s) {}

    // =========================================================================
    // TEXT FIELDS
    // =========================================================================

    /** Applique le style à un TextEditor. icon = "user", "mail" ou "lock". */
    void applyTo(juce::TextEditor& f, const juce::String& icon)
    {
        f.setLookAndFeel(this);
        f.getProperties().set("icon", icon);

        f.setFont(juce::Font("Inter", 14.f, juce::Font::plain));
        f.setJustification(juce::Justification::centredLeft);
        f.setBorder(juce::BorderSize<int>(0));
        f.setIndents(iconAreaW, 0);

        f.setColour(juce::TextEditor::backgroundColourId,      juce::Colours::transparentBlack);
        f.setColour(juce::TextEditor::outlineColourId,         juce::Colours::transparentBlack);
        f.setColour(juce::TextEditor::focusedOutlineColourId,  juce::Colours::transparentBlack);
        f.setColour(juce::TextEditor::textColourId,            HarmoniaColours::textPrimary);
        f.setColour(juce::CaretComponent::caretColourId,       HarmoniaColours::waveBlue);
        f.setColour(juce::TextEditor::highlightColourId,       HarmoniaColours::waveBlue.withAlpha(0.30f));
        f.setColour(juce::TextEditor::highlightedTextColourId, HarmoniaColours::textPrimary);
    }

    void fillTextEditorBackground(juce::Graphics& g, int w, int h,
                                  juce::TextEditor& ed) override
    {
        const bool focus = ed.hasKeyboardFocus(true);
        const auto b = juce::Rectangle<float>((float) w, (float) h).reduced(0.5f);

        juce::ColourGradient fill(
            juce::Colours::white.withAlpha(focus ? 0.09f : 0.055f), 0.f, 0.f,
            juce::Colours::white.withAlpha(focus ? 0.05f : 0.025f), 0.f, (float) h,
            false);
        g.setGradientFill(fill);
        g.fillRoundedRectangle(b, radius);

        if (focus)
        {
            g.setColour(HarmoniaColours::waveBlue.withAlpha(0.06f));
            g.fillRoundedRectangle(b, radius);
        }

        drawFieldIcon(g, ed.getProperties()["icon"].toString(),
                      { 23.f, (float) h * 0.5f },
                      focus ? HarmoniaColours::waveBlue.withAlpha(0.95f)
                            : juce::Colours::white.withAlpha(0.38f));
    }

    void drawTextEditorOutline(juce::Graphics& g, int w, int h,
                               juce::TextEditor& ed) override
    {
        const bool focus = ed.hasKeyboardFocus(true);
        const auto b = juce::Rectangle<float>((float) w, (float) h).reduced(0.5f);

        if (focus)
        {
            g.setColour(HarmoniaColours::waveBlue.withAlpha(0.14f));
            g.drawRoundedRectangle(b.reduced(1.5f), radius - 1.5f, 2.f);
        }

        g.setColour(focus ? HarmoniaColours::waveBlue.withAlpha(0.75f)
                          : juce::Colours::white.withAlpha(0.10f));
        g.drawRoundedRectangle(b, radius, 1.f);
    }

    // =========================================================================
    // BUTTONS
    // =========================================================================

    void drawButtonBackground(juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& /*bgColour*/,
                               bool isHighlighted,
                               bool isDown) override
    {
        const auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);

        if (style == Style::Primary)
        {
            const float topA    = isDown ? 0.38f : (isHighlighted ? 0.32f : 0.22f);
            const float botA    = isDown ? 0.20f : (isHighlighted ? 0.16f : 0.10f);
            const float borderA = isDown ? 0.90f : (isHighlighted ? 0.80f : 0.55f);

            juce::ColourGradient fill(
                HarmoniaColours::waveBlue.withAlpha(topA), 0.f, bounds.getY(),
                HarmoniaColours::waveBlue.withAlpha(botA), 0.f, bounds.getBottom(),
                false);
            g.setGradientFill(fill);
            g.fillRoundedRectangle(bounds, radius);

            g.setColour(juce::Colours::white.withAlpha(0.08f));
            g.drawLine(bounds.getX() + radius, bounds.getY() + 1.f,
                       bounds.getRight() - radius, bounds.getY() + 1.f, 1.f);

            if (isHighlighted || isDown)
            {
                g.setColour(HarmoniaColours::waveBlue.withAlpha(0.16f));
                g.drawRoundedRectangle(bounds.reduced(1.5f), radius - 1.5f, 2.f);
            }

            g.setColour(HarmoniaColours::waveBlue.withAlpha(borderA));
            g.drawRoundedRectangle(bounds, radius, 1.f);
        }
        else // Back
        {
            const float bgAlpha     = isHighlighted ? 0.05f : 0.0f;
            const float borderAlpha = isHighlighted ? 0.18f : 0.10f;

            g.setColour(juce::Colours::white.withAlpha(bgAlpha));
            g.fillRoundedRectangle(bounds, radius);

            g.setColour(juce::Colours::white.withAlpha(borderAlpha));
            g.drawRoundedRectangle(bounds, radius, 1.f);
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
                        : HarmoniaColours::textPrimary.withAlpha(0.92f));
            g.setFont(juce::Font("Inter", 14.f, juce::Font::bold));
            g.drawFittedText(button.getButtonText(), b, juce::Justification::centred, 1);
        }
        else // Back
        {
            const juce::Colour col = isHighlighted
                ? juce::Colours::white.withAlpha(0.65f)
                : juce::Colours::white.withAlpha(0.35f);

            const juce::Font font("Inter", 13.f, juce::Font::plain);
            g.setFont(font);

            juce::GlyphArrangement ga;
            ga.addLineOfText(font, button.getButtonText(), 0.f, 0.f);
            const float textW = ga.getBoundingBox(0, -1, true).getWidth();

            constexpr float arrowW = 12.f;
            constexpr float gap    = 8.f;
            const float groupW = arrowW + gap + textW;

            const float x0 = (float) b.getCentreX() - groupW * 0.5f
                             - (isHighlighted ? 2.f : 0.f);
            const float cy = (float) b.getCentreY();

            juce::Path arrow;
            arrow.startNewSubPath(x0 + arrowW, cy);
            arrow.lineTo(x0, cy);
            arrow.startNewSubPath(x0 + 5.f, cy - 4.f);
            arrow.lineTo(x0, cy);
            arrow.lineTo(x0 + 5.f, cy + 4.f);

            g.setColour(col);
            g.strokePath(arrow, juce::PathStrokeType(1.4f,
                                                     juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

            g.setColour(col);
            g.drawText(button.getButtonText(),
                       juce::Rectangle<float>(x0 + arrowW + gap, (float) b.getY(),
                                              textW + 4.f, (float) b.getHeight()),
                       juce::Justification::centredLeft, false);
        }
    }

private:
    static constexpr float radius    = 10.f;
    static constexpr int   iconAreaW = 46; // marge gauche du texte dans les champs

    static void drawFieldIcon(juce::Graphics& g, const juce::String& icon,
                              juce::Point<float> c, juce::Colour colour)
    {
        constexpr float halfPi = juce::MathConstants<float>::halfPi;
        juce::Path p;

        if (icon == "user")
        {
            p.addEllipse(c.x - 3.f, c.y - 8.f, 6.f, 6.f);
            p.addCentredArc(c.x, c.y + 7.f, 6.f, 5.5f, 0.f, -halfPi, halfPi, true);
        }
        else if (icon == "lock")
        {
            p.addRoundedRectangle(c.x - 5.f, c.y - 1.f, 10.f, 8.f, 1.5f);
            p.addCentredArc(c.x, c.y - 1.f, 3.f, 3.5f, 0.f, -halfPi, halfPi, true);
        }
        else if (icon == "mail")
        {
            p.addRoundedRectangle(c.x - 6.f, c.y - 4.5f, 12.f, 9.f, 1.5f);
            p.startNewSubPath(c.x - 6.f, c.y - 3.f);
            p.lineTo(c.x, c.y + 1.f);
            p.lineTo(c.x + 6.f, c.y - 3.f);
        }
        else
        {
            return;
        }

        g.setColour(colour);
        g.strokePath(p, juce::PathStrokeType(1.4f,
                                             juce::PathStrokeType::curved,
                                             juce::PathStrokeType::rounded));
    }

    Style style;
};