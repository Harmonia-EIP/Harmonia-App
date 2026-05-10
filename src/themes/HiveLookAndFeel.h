#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "HarmoniaPalette.h"

// HiveLookAndFeel v2 : ajoute des touches "Apple modern dark" :
// rim light 1px blanc 6%, corners généreux, specular highlight sur knobs,
// drop shadows plus diffuses, gradients soignés.
class HiveLookAndFeel : public juce::LookAndFeel_V4
{
public:
    HiveLookAndFeel()
    {
        setColour (juce::ResizableWindow::backgroundColourId,    HarmoniaPalette::background);

        setColour (juce::Label::textColourId,                    HarmoniaPalette::textSecondary);
        setColour (juce::Label::textWhenEditingColourId,         HarmoniaPalette::textPrimary);
        setColour (juce::Label::outlineWhenEditingColourId,      HarmoniaPalette::accent);

        setColour (juce::ComboBox::backgroundColourId,           HarmoniaPalette::panel);
        setColour (juce::ComboBox::textColourId,                 HarmoniaPalette::accent);
        setColour (juce::ComboBox::outlineColourId,              HarmoniaPalette::border);
        setColour (juce::ComboBox::arrowColourId,                HarmoniaPalette::accent);
        setColour (juce::ComboBox::buttonColourId,               HarmoniaPalette::panel);
        setColour (juce::ComboBox::focusedOutlineColourId,       HarmoniaPalette::accent);

        setColour (juce::PopupMenu::backgroundColourId,           HarmoniaPalette::panel);
        setColour (juce::PopupMenu::textColourId,                 HarmoniaPalette::textSecondary);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, HarmoniaPalette::accent.withAlpha (0.18f));
        setColour (juce::PopupMenu::highlightedTextColourId,       HarmoniaPalette::accent);

        setColour (juce::TextButton::buttonColourId,    HarmoniaPalette::panel);
        setColour (juce::TextButton::buttonOnColourId,  HarmoniaPalette::accent);
        setColour (juce::TextButton::textColourOffId,   HarmoniaPalette::textSecondary);
        setColour (juce::TextButton::textColourOnId,    HarmoniaPalette::background);

        setColour (juce::Slider::trackColourId,                HarmoniaPalette::knobTrack);
        setColour (juce::Slider::thumbColourId,                HarmoniaPalette::textPrimary);
        setColour (juce::Slider::rotarySliderFillColourId,     HarmoniaPalette::accent);
        setColour (juce::Slider::rotarySliderOutlineColourId,  HarmoniaPalette::border);
        setColour (juce::Slider::textBoxTextColourId,          HarmoniaPalette::accent);
        setColour (juce::Slider::textBoxBackgroundColourId,    juce::Colours::transparentBlack);
        setColour (juce::Slider::textBoxOutlineColourId,       juce::Colours::transparentBlack);

        setColour (juce::TextEditor::backgroundColourId,       HarmoniaPalette::panel);
        setColour (juce::TextEditor::outlineColourId,          HarmoniaPalette::border);
        setColour (juce::TextEditor::focusedOutlineColourId,   HarmoniaPalette::accent);
        setColour (juce::TextEditor::textColourId,             HarmoniaPalette::textPrimary);
        setColour (juce::CaretComponent::caretColourId,        HarmoniaPalette::accent);
    }

    juce::Font getLabelFont (juce::Label&) override
    {
        return juce::Font (juce::FontOptions (11.0f));
    }

    juce::Font getTextButtonFont (juce::TextButton&, int) override
    {
        return juce::Font (juce::FontOptions (12.0f).withStyle ("Bold"));
    }

    juce::Font getComboBoxFont (juce::ComboBox&) override
    {
        return juce::Font (juce::FontOptions (12.0f).withStyle ("Bold"));
    }

    juce::Font getPopupMenuFont() override
    {
        return juce::Font (juce::FontOptions (12.0f));
    }

    // ---- Buttons ---------------------------------------------------------
    void drawButtonBackground (juce::Graphics& g, juce::Button& button,
                               const juce::Colour& backgroundColour,
                               bool isHighlighted, bool isDown) override
    {
        auto r = button.getLocalBounds().toFloat().reduced (1.0f);
        const float corner = 7.0f;

        const bool isAccent = button.getProperties().getWithDefault ("accent", false);

        if (isAccent)
        {
            // Soft drop shadow
            juce::Path shadowPath;
            shadowPath.addRoundedRectangle (r.translated (0.0f, 1.5f), corner);
            juce::DropShadow ds (HarmoniaPalette::accent.withAlpha (0.30f), 10, { 0, 2 });
            ds.drawForPath (g, shadowPath);

            // Cyan gradient fill
            juce::ColourGradient grad (HarmoniaPalette::accentHover, r.getX(), r.getY(),
                                       HarmoniaPalette::accentSecond, r.getRight(), r.getBottom(), false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (r, corner);

            // Rim light en haut (apple-like)
            g.setColour (juce::Colours::white.withAlpha (0.18f));
            g.drawLine (r.getX() + corner, r.getY() + 1.0f,
                        r.getRight() - corner, r.getY() + 1.0f, 1.0f);

            if (isHighlighted)
            {
                g.setColour (HarmoniaPalette::accent.withAlpha (0.40f));
                g.drawRoundedRectangle (r.expanded (1.5f), corner + 1.5f, 2.0f);
            }
        }
        else
        {
            // Subtle gradient pour profondeur
            juce::Colour top = backgroundColour.brighter (0.05f);
            juce::Colour bot = backgroundColour.darker (0.10f);
            if (isDown)             { top = backgroundColour.darker (0.10f); bot = backgroundColour.darker (0.20f); }
            else if (isHighlighted) { top = backgroundColour.brighter (0.15f); bot = backgroundColour.brighter (0.05f); }

            juce::ColourGradient grad (top, r.getX(), r.getY(), bot, r.getX(), r.getBottom(), false);
            g.setGradientFill (grad);
            g.fillRoundedRectangle (r, corner);

            // Rim light
            g.setColour (juce::Colours::white.withAlpha (0.06f));
            g.drawLine (r.getX() + corner, r.getY() + 1.0f,
                        r.getRight() - corner, r.getY() + 1.0f, 1.0f);

            g.setColour (isHighlighted ? HarmoniaPalette::accent.withAlpha (0.6f) : HarmoniaPalette::border);
            g.drawRoundedRectangle (r, corner, 1.0f);
        }
    }

    // ---- Knob ------------------------------------------------------------
    // Hive style + specular highlight Apple : corps métallique, arc cyan
    // (ou couleur custom via property "tint"), drop shadow externe diffuse,
    // petit specular en haut-gauche du body.
    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float startAngle, float endAngle,
                           juce::Slider& slider) override
    {
        auto bounds = juce::Rectangle<int> (x, y, width, height).toFloat().reduced (3.0f);
        const float radius     = juce::jmin (bounds.getWidth(), bounds.getHeight()) * 0.5f;
        const auto  centre     = bounds.getCentre();
        const float arcThick   = juce::jmax (3.0f, radius * 0.10f);
        const float arcRadius  = radius - arcThick * 0.5f - 1.0f;
        const float bodyRadius = arcRadius - arcThick - 2.0f;

        const bool   bipolar  = slider.getProperties().getWithDefault ("bipolar", false);
        const bool   hover    = slider.isMouseOverOrDragging();
        const auto   tintProp = slider.getProperties().getWithDefault ("tint", juce::var());
        const auto   tint     = tintProp.isVoid()
                                  ? HarmoniaPalette::accent
                                  : juce::Colour::fromString (tintProp.toString());

        // 1) Drop shadow diffuse
        {
            juce::Path shadowPath;
            shadowPath.addEllipse (juce::Rectangle<float> (bodyRadius * 2.0f, bodyRadius * 2.0f)
                                       .withCentre (centre.translated (0.0f, 2.0f)));
            juce::DropShadow ds (juce::Colours::black.withAlpha (0.55f), 10, { 0, 3 });
            ds.drawForPath (g, shadowPath);
        }

        // 2) Track sombre
        {
            juce::Path track;
            track.addCentredArc (centre.x, centre.y, arcRadius, arcRadius,
                                 0.0f, startAngle, endAngle, true);
            g.setColour (HarmoniaPalette::knobTrack);
            g.strokePath (track, juce::PathStrokeType (arcThick,
                                                       juce::PathStrokeType::curved,
                                                       juce::PathStrokeType::butt));
        }

        // 3) Corps métallique mat
        {
            juce::ColourGradient bodyGrad (HarmoniaPalette::knobBgHi.brighter (0.06f),
                                           centre.translated (-bodyRadius * 0.30f, -bodyRadius * 0.40f),
                                           HarmoniaPalette::knobBg.darker (0.30f),
                                           centre.translated (bodyRadius * 0.40f, bodyRadius * 0.50f), true);
            g.setGradientFill (bodyGrad);
            g.fillEllipse (juce::Rectangle<float> (bodyRadius * 2.0f, bodyRadius * 2.0f).withCentre (centre));

            // Specular highlight (small white arc top-left, "Apple modern" feel)
            juce::Path spec;
            spec.addCentredArc (centre.x, centre.y,
                                bodyRadius * 0.92f, bodyRadius * 0.92f,
                                0.0f,
                                juce::MathConstants<float>::pi * 1.20f,
                                juce::MathConstants<float>::pi * 1.65f, true);
            g.setColour (juce::Colours::white.withAlpha (0.10f));
            g.strokePath (spec, juce::PathStrokeType (1.2f, juce::PathStrokeType::curved,
                                                      juce::PathStrokeType::rounded));

            g.setColour (HarmoniaPalette::border);
            g.drawEllipse (juce::Rectangle<float> (bodyRadius * 2.0f, bodyRadius * 2.0f).withCentre (centre), 1.0f);
        }

        // 4) Active arc (tint, bipolaire = warm côté gauche)
        const float currentAngle = startAngle + sliderPos * (endAngle - startAngle);
        juce::Path arc;
        const float midAngle = (startAngle + endAngle) * 0.5f;
        bool onLeftSide = false;

        if (bipolar)
        {
            const float fromA = std::min (midAngle, currentAngle);
            const float toA   = std::max (midAngle, currentAngle);
            arc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, fromA, toA, true);
            onLeftSide = currentAngle < midAngle;
        }
        else
        {
            arc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f, startAngle, currentAngle, true);
        }

        const auto arcCol = onLeftSide ? HarmoniaPalette::warm : tint;
        g.setColour (arcCol);
        g.strokePath (arc, juce::PathStrokeType (arcThick,
                                                  juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::butt));

        // Glow toujours visible
        const float glowAlpha = hover ? 0.32f : 0.16f;
        g.setColour (arcCol.withAlpha (glowAlpha));
        g.strokePath (arc, juce::PathStrokeType (arcThick + 5.0f,
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::butt));

        // 5) Indicateur (ligne fine + cap lumineux)
        {
            const float innerR = bodyRadius * 0.30f;
            const float outerR = bodyRadius * 0.80f;
            const float thick  = juce::jmax (1.5f, radius * 0.05f);

            const float cosA = std::cos (currentAngle - juce::MathConstants<float>::halfPi);
            const float sinA = std::sin (currentAngle - juce::MathConstants<float>::halfPi);
            const juce::Point<float> p1 (centre.x + cosA * innerR, centre.y + sinA * innerR);
            const juce::Point<float> p2 (centre.x + cosA * outerR, centre.y + sinA * outerR);

            g.setColour (HarmoniaPalette::textPrimary);
            g.drawLine (p1.x, p1.y, p2.x, p2.y, thick);

            const float capR = thick * 1.2f;
            g.setColour (arcCol.withAlpha (0.55f));
            g.fillEllipse (p2.x - capR * 1.6f, p2.y - capR * 1.6f, capR * 3.2f, capR * 3.2f);
            g.setColour (HarmoniaPalette::textPrimary);
            g.fillEllipse (p2.x - capR, p2.y - capR, capR * 2.0f, capR * 2.0f);
        }
    }

    // ---- Linear sliders --------------------------------------------------
    void drawLinearSlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float minSliderPos, float maxSliderPos,
                           const juce::Slider::SliderStyle style, juce::Slider& slider) override
    {
        juce::ignoreUnused (minSliderPos, maxSliderPos, slider);

        if (style == juce::Slider::LinearVertical)
        {
            const float trackW = 6.0f;
            const float cx     = (float) x + (float) width * 0.5f;
            auto track = juce::Rectangle<float> (cx - trackW * 0.5f, (float) y + 4.0f,
                                                 trackW, (float) height - 8.0f);

            // Drop shadow inset (effet "creusé")
            g.setColour (juce::Colours::black.withAlpha (0.45f));
            g.fillRoundedRectangle (track.translated (0.0f, 1.0f), trackW * 0.5f);

            g.setColour (HarmoniaPalette::knobTrack);
            g.fillRoundedRectangle (track, trackW * 0.5f);
            g.setColour (HarmoniaPalette::border);
            g.drawRoundedRectangle (track, trackW * 0.5f, 1.0f);

            const float thumbY = juce::jlimit (track.getY(), track.getBottom(), sliderPos);
            auto fill = juce::Rectangle<float> (track.getX(), thumbY,
                                                track.getWidth(),
                                                track.getBottom() - thumbY);

            // Use knob's tint if set, else cyan
            const auto tintProp = slider.getProperties().getWithDefault ("tint", juce::var());
            const auto tint     = tintProp.isVoid()
                                    ? HarmoniaPalette::accent
                                    : juce::Colour::fromString (tintProp.toString());

            juce::ColourGradient fillGrad (tint, fill.getX(), fill.getY(),
                                           tint.darker (0.10f), fill.getX(), fill.getBottom(), false);
            g.setGradientFill (fillGrad);
            g.fillRoundedRectangle (fill, trackW * 0.5f);

            g.setColour (tint.withAlpha (0.20f));
            g.fillRoundedRectangle (fill.expanded (3.0f, 0.0f), trackW * 0.5f + 3.0f);

            // Thumb : pill rectangulaire avec gradient + rim light + grip lines
            const float thumbW = 24.0f;
            const float thumbH = 10.0f;
            auto thumb = juce::Rectangle<float> (cx - thumbW * 0.5f, thumbY - thumbH * 0.5f,
                                                 thumbW, thumbH);

            // Drop shadow
            juce::Path thumbPath;
            thumbPath.addRoundedRectangle (thumb.translated (0.0f, 1.0f), 3.0f);
            juce::DropShadow ds (juce::Colours::black.withAlpha (0.60f), 5, { 0, 1 });
            ds.drawForPath (g, thumbPath);

            juce::ColourGradient thumbGrad (HarmoniaPalette::textPrimary.brighter (0.10f),
                                            thumb.getX(), thumb.getY(),
                                            HarmoniaPalette::textSecondary.darker (0.20f),
                                            thumb.getX(), thumb.getBottom(), false);
            g.setGradientFill (thumbGrad);
            g.fillRoundedRectangle (thumb, 3.0f);

            // Rim light
            g.setColour (juce::Colours::white.withAlpha (0.40f));
            g.drawLine (thumb.getX() + 3.0f, thumb.getY() + 1.0f,
                        thumb.getRight() - 3.0f, thumb.getY() + 1.0f, 1.0f);

            g.setColour (HarmoniaPalette::border);
            g.drawRoundedRectangle (thumb, 3.0f, 1.0f);

            // Grip lines
            g.setColour (HarmoniaPalette::knobTrack.withAlpha (0.7f));
            for (int i = -1; i <= 1; ++i)
                g.drawHorizontalLine ((int) thumb.getCentreY() + i * 2,
                                      thumb.getX() + 5.0f, thumb.getRight() - 5.0f);

            return;
        }

        if (style == juce::Slider::LinearHorizontal || style == juce::Slider::LinearBar)
        {
            auto track = juce::Rectangle<float> ((float) x, (float) y + height * 0.5f - 2.0f,
                                                 (float) width, 4.0f);
            g.setColour (HarmoniaPalette::knobTrack);
            g.fillRoundedRectangle (track, 2.0f);

            auto fill = track.withWidth (juce::jmax (4.0f, sliderPos - (float) x));
            g.setColour (HarmoniaPalette::accent);
            g.fillRoundedRectangle (fill, 2.0f);

            const float thumbR = 6.0f;
            g.setColour (HarmoniaPalette::accent.withAlpha (0.25f));
            g.fillEllipse (sliderPos - thumbR - 2.0f, (float) y + height * 0.5f - thumbR - 2.0f,
                           (thumbR + 2.0f) * 2.0f, (thumbR + 2.0f) * 2.0f);
            g.setColour (HarmoniaPalette::textPrimary);
            g.fillEllipse (sliderPos - thumbR, (float) y + height * 0.5f - thumbR, thumbR * 2.0f, thumbR * 2.0f);
            return;
        }

        LookAndFeel_V4::drawLinearSlider (g, x, y, width, height, sliderPos,
                                          minSliderPos, maxSliderPos, style, slider);
    }

    // ---- ComboBox (legacy fallback ; on utilise des selectors custom partout) ----
    void drawComboBox (juce::Graphics& g, int width, int height, bool /*isDown*/,
                       int buttonX, int buttonY, int buttonW, int buttonH,
                       juce::ComboBox& box) override
    {
        auto r = juce::Rectangle<int> (0, 0, width, height).toFloat().reduced (1.0f);
        const float corner = 6.0f;

        g.setColour (HarmoniaPalette::panel);
        g.fillRoundedRectangle (r, corner);

        const bool active = box.hasKeyboardFocus (false) || box.isMouseOver();
        g.setColour (active ? HarmoniaPalette::accent : HarmoniaPalette::border);
        g.drawRoundedRectangle (r, corner, 1.0f);

        const float ax = (float) (buttonX + buttonW * 0.5f);
        const float ay = (float) (buttonY + buttonH * 0.5f);
        const float aw = 4.0f;
        const float ah = 3.0f;
        juce::Path chevron;
        chevron.startNewSubPath (ax - aw, ay - ah * 0.5f);
        chevron.lineTo (ax,        ay + ah * 0.7f);
        chevron.lineTo (ax + aw,   ay - ah * 0.5f);
        g.setColour (HarmoniaPalette::accent);
        g.strokePath (chevron, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
    }

    void positionComboBoxText (juce::ComboBox& box, juce::Label& label) override
    {
        label.setBounds (box.getLocalBounds().reduced (8, 1).withTrimmedRight (14));
        label.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
        label.setMinimumHorizontalScale (0.7f); // évite le crop tonky
    }

    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            const bool isSeparator, const bool isActive, const bool isHighlighted,
                            const bool isTicked, const bool hasSubMenu,
                            const juce::String& text, const juce::String& shortcutKeyText,
                            const juce::Drawable* icon, const juce::Colour* textColour) override
    {
        juce::ignoreUnused (textColour, hasSubMenu, shortcutKeyText, icon);

        if (isSeparator)
        {
            g.setColour (HarmoniaPalette::border);
            g.drawHorizontalLine (area.getCentreY(), area.getX() + 6, area.getRight() - 6);
            return;
        }

        if (isHighlighted && isActive)
        {
            g.setColour (HarmoniaPalette::accent.withAlpha (0.18f));
            g.fillRect (area);
            g.setColour (HarmoniaPalette::accent);
        }
        else
        {
            g.setColour (isActive ? HarmoniaPalette::textSecondary : HarmoniaPalette::textMuted);
        }

        g.setFont (juce::Font (juce::FontOptions (12.0f)));
        auto textArea = area.reduced (10, 0);
        g.drawText (text, textArea, juce::Justification::centredLeft);

        if (isTicked)
        {
            g.setColour (HarmoniaPalette::accent);
            g.fillRect (textArea.removeFromRight (6).withSizeKeepingCentre (6, 6));
        }
    }
};
