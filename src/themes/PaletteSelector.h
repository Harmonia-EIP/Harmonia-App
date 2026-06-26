#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "HarmoniaPalette.h"

class PaletteSelector : public juce::Component
{
public:
    PaletteSelector()
    {
        using T = HarmoniaPalette::Theme;

        for (auto t : {
                T::Cyan,
                T::Blue,
                T::Red,
                T::Purple,
                T::Dark
            })
        {
            themes.push_back (t);
        }
    }

    void paint (juce::Graphics& g) override
    {
        const float r = getHeight() * 0.5f;

        // background pill
        g.setColour (HarmoniaPalette::panel.brighter (0.05f));
        g.fillRoundedRectangle (getLocalBounds().toFloat(), r);

        g.setColour (HarmoniaPalette::border);
        g.drawRoundedRectangle (
            getLocalBounds().toFloat().reduced (0.5f),
            r,
            1.0f);

        const float dotR  = r * 0.52f;
        const float cellW = (float) getWidth() / (float) themes.size();

        for (int i = 0; i < (int) themes.size(); ++i)
        {
            const bool selected = (themes[i] == current);

            const float cx = cellW * i + cellW * 0.5f;
            const float cy = getHeight() * 0.5f;

            const auto col =
                HarmoniaPalette::themePreviewColour (themes[i]);

            // glow
            if (selected)
            {
                g.setColour (col.withAlpha (0.22f));

                g.fillEllipse (
                    cx - dotR * 1.7f,
                    cy - dotR * 1.7f,
                    dotR * 3.4f,
                    dotR * 3.4f);
            }

            // dot
            g.setColour (
                selected
                    ? col
                    : col.withAlpha (0.45f));

            g.fillEllipse (
                cx - dotR,
                cy - dotR,
                dotR * 2.0f,
                dotR * 2.0f);

            // rim
            g.setColour (
                selected
                    ? col.brighter (0.3f)
                    : HarmoniaPalette::borderHi);

            g.drawEllipse (
                cx - dotR,
                cy - dotR,
                dotR * 2.0f,
                dotR * 2.0f,
                1.0f);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        const float cellW =
            (float) getWidth() / (float) themes.size();

        const int idx =
            juce::jlimit (
                0,
                (int) themes.size() - 1,
                (int) ((float) e.x / cellW));

        current = themes[idx];

        HarmoniaPalette::setTheme (current);

        repaint();

        if (onThemeUpdated)
            onThemeUpdated();
    }

    void setCurrentTheme (HarmoniaPalette::Theme theme)
    {
        current = theme;
        HarmoniaPalette::setTheme (current);
        repaint();
    }

    HarmoniaPalette::Theme getCurrentTheme() const
    {
        return current;
    }

    std::function<void()> onThemeUpdated;

private:
    std::vector<HarmoniaPalette::Theme> themes;

    HarmoniaPalette::Theme current =
        HarmoniaPalette::Theme::Cyan;
};