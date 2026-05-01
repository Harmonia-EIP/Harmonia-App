#include "ThemeManager.h"

ThemeManager::ThemeManager(AppLookAndFeel& lf)
    : lookAndFeel(lf)
{
}

void ThemeManager::applyTheme(ThemePreset theme)
{
    lookAndFeel.setThemePreset(theme);
}

juce::Colour ThemeManager::getBackgroundColour() const
{
    return lookAndFeel.findColour(AppColourIds::backgroundId);
}