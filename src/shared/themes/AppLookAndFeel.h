// AppLookAndFeel.h
#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "AppColourIds.h"
#include "Themes.h"

class AppLookAndFeel : public juce::LookAndFeel_V4
{
public:
    enum class ThemePreset { Dark, Light, Red, Blue };
    enum class LayoutPreset { Layout1, Layout2, Layout3, Layout4 };

    AppLookAndFeel()
    {
        setThemePreset (ThemePreset::Dark);
    }

    ThemePreset getPreset() const { return preset; }

    void setThemePreset (ThemePreset p)
    {
        preset = p;

        switch (preset)
        {
            case ThemePreset::Dark:  palette = ThemePalette::dark();  break;
            case ThemePreset::Light: palette = ThemePalette::light(); break;
            case ThemePreset::Red:   palette = ThemePalette::red();   break;
            case ThemePreset::Blue:  palette = ThemePalette::blue();  break;
            default:            palette = ThemePalette::dark();  break;
        }

        applyPalette();
    }

    void toggleTheme()
    {
        setThemePreset (preset == ThemePreset::Dark ? ThemePreset::Light : ThemePreset::Dark);
    }

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        juce::ignoreUnused (slider);

        auto radius  = (float) juce::jmin (width / 2, height / 2) - 4.0f;
        auto centreX = (float) x + (float) width  * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        auto knobBg      = findColour (AppColourIds::knobBgId);
        auto knobOutline = findColour (AppColourIds::knobOutlineId);
        auto knobPointer = findColour (AppColourIds::knobPointerId);

        g.setColour (knobBg);
        g.fillEllipse (rx, ry, rw, rw);

        g.setColour (knobOutline);
        g.drawEllipse (rx, ry, rw, rw, 1.5f);

        juce::Path p;
        auto pointerLength    = radius * 0.7f;
        auto pointerThickness = 3.0f;
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));

        g.setColour (knobPointer);
        g.fillPath (p);
    }

private:
    void applyPalette()
    {
        setColour (AppColourIds::backgroundId,    palette.background);
        setColour (AppColourIds::panelBgId,       palette.panelBg);
        setColour (AppColourIds::panelOutlineId,  palette.panelOutline);

        setColour (AppColourIds::textPrimaryId,   palette.textPrimary);
        setColour (AppColourIds::textSecondaryId, palette.textSecondary);

        setColour (AppColourIds::accentId,        palette.accent);
        setColour (AppColourIds::accentHoverId,   palette.accentHover);
        setColour (AppColourIds::accentDownId,    palette.accentDown);

        setColour (AppColourIds::knobBgId,        palette.knobBg);
        setColour (AppColourIds::knobOutlineId,   palette.knobOutline);
        setColour (AppColourIds::knobPointerId,   palette.knobPointer);

        setColour (AppColourIds::oscilloscopeWaveId, palette.accent);
        setColour (AppColourIds::oscilloscopeGridId, palette.textSecondary);

        setColour (juce::ResizableWindow::backgroundColourId, palette.background);

        setColour (juce::Label::textColourId, palette.textPrimary);
        setColour (juce::Label::textWhenEditingColourId, palette.textPrimary);
        setColour (juce::Label::outlineWhenEditingColourId, palette.accent);

        setColour (juce::ComboBox::backgroundColourId, palette.panelBg);
        setColour (juce::ComboBox::textColourId, palette.textPrimary);
        setColour (juce::ComboBox::outlineColourId, palette.panelOutline);
        setColour (juce::ComboBox::arrowColourId, palette.textPrimary);

        setColour (juce::PopupMenu::backgroundColourId, palette.panelBg);
        setColour (juce::PopupMenu::textColourId, palette.textPrimary);
        setColour (juce::PopupMenu::highlightedBackgroundColourId, palette.accent);
        setColour (juce::PopupMenu::highlightedTextColourId, palette.textPrimary);

        setColour (juce::TextButton::buttonColourId, palette.panelBg);
        setColour (juce::TextButton::buttonOnColourId, palette.accentDown);
        setColour (juce::TextButton::textColourOffId, palette.textPrimary);
        setColour (juce::TextButton::textColourOnId, palette.textPrimary);

        setColour (juce::Slider::trackColourId, palette.panelOutline);
        setColour (juce::Slider::thumbColourId, palette.accent);
        setColour (juce::Slider::rotarySliderFillColourId, palette.accent);
        setColour (juce::Slider::rotarySliderOutlineColourId, palette.panelOutline);

        setColour (juce::Slider::textBoxTextColourId,        palette.textPrimary);
        setColour (juce::Slider::textBoxBackgroundColourId,  palette.panelBg);
        setColour (juce::Slider::textBoxOutlineColourId,     palette.panelOutline);
        setColour (juce::Slider::textBoxHighlightColourId,   palette.accent.withAlpha (0.25f));

        setColour (juce::TextEditor::backgroundColourId, palette.panelBg);
        setColour (juce::TextEditor::outlineColourId, palette.panelOutline);
        setColour (juce::TextEditor::textColourId, palette.textPrimary);
        setColour (juce::CaretComponent::caretColourId, palette.accent);
    }

    ThemePreset preset = ThemePreset::Dark;
    ThemePalette palette = ThemePalette::dark();
};
