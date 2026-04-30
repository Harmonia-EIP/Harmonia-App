#pragma once

#include "../themes/AppLookAndFeel.h"
#include "../themes/AppColourIds.h"

class ThemeAndLayoutConverter
{
private:
public:

    static AppLookAndFeel::ThemePreset idToThemePreset(int themeId)
    {
        switch (themeId)
        {
            case 1: return AppLookAndFeel::ThemePreset::Dark;
            case 2: return AppLookAndFeel::ThemePreset::Light;
            case 3: return AppLookAndFeel::ThemePreset::Red;
            case 4: return AppLookAndFeel::ThemePreset::Blue;
            default: return AppLookAndFeel::ThemePreset::Dark;
        }
    }

    static AppLookAndFeel::LayoutPreset idToLayoutPreset(int layoutId)
    {
        switch (layoutId)
        {
            case 1: return AppLookAndFeel::LayoutPreset::Layout1;
            case 2: return AppLookAndFeel::LayoutPreset::Layout2;
            case 3: return AppLookAndFeel::LayoutPreset::Layout3;
            case 4: return AppLookAndFeel::LayoutPreset::Layout4;
            default: return AppLookAndFeel::LayoutPreset::Layout1;
        }
    }


    static int themePresetToId(AppLookAndFeel::ThemePreset preset)
    {
        switch (preset)
        {
            case AppLookAndFeel::ThemePreset::Dark:  return 1;
            case AppLookAndFeel::ThemePreset::Light: return 2;
            case AppLookAndFeel::ThemePreset::Red:   return 3;
            case AppLookAndFeel::ThemePreset::Blue:  return 4;
            default: return 1;
        }
    }

    static int layoutPresetToId(AppLookAndFeel::LayoutPreset preset)
    {
        switch (preset)
        {
            case AppLookAndFeel::LayoutPreset::Layout1: return 1;
            case AppLookAndFeel::LayoutPreset::Layout2: return 2;
            case AppLookAndFeel::LayoutPreset::Layout3: return 3;
            case AppLookAndFeel::LayoutPreset::Layout4: return 4;
            default: return 1;
        }
    }
};
