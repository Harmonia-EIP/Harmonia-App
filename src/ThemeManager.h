#pragma once

#include "themes/AppLookAndFeel.h"
#include "themes/AppColourIds.h"

class ThemeManager
{
public:
    using ThemePreset = AppLookAndFeel::ThemePreset;

    ThemeManager(AppLookAndFeel& lookAndFeel);

    void applyTheme(ThemePreset theme);
    juce::Colour getBackgroundColour() const;

private:
    AppLookAndFeel& lookAndFeel;
};