#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/AppLookAndFeel.h"
#include "../themes/AppColourIds.h"
#include "../backendManagement/BackendTypes.h"
#include "../tools/ThemeAndLayoutConverter.h"

class TitleComponent : public juce::Component,
                       private juce::Button::Listener
{
public:
    TitleComponent (const juce::String& textToDisplay, const UserSession& userSession);
    ~TitleComponent() override;

    using LayoutPreset = AppLookAndFeel::LayoutPreset;
    using ThemePreset = AppLookAndFeel::ThemePreset;

    void paint (juce::Graphics& g) override;
    void resized() override;

    std::function<void()> onLogout;
    std::function<void(AppLookAndFeel::ThemePreset)> onThemeSelected;
    std::function<void(AppLookAndFeel::LayoutPreset)> onLayoutSelected;

private:
    void buttonClicked (juce::Button* button) override;

    UserSession currentSession;

    juce::String title;
    juce::Label titleLabel;
    juce::Label pseudoLabel; 

    juce::TextButton themeButton  { "Theme" };
    juce::TextButton layoutButton { "Layout" };
    juce::TextButton logoutButton { "Logout" };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TitleComponent)
};
