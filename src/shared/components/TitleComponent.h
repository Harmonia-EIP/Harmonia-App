#pragma once

#include "ComponentsIncludes.h"

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

    juce::TextButton themeButton  { Strings::Buttons::Theme };
    juce::TextButton layoutButton { Strings::Buttons::Layout };
    juce::TextButton logoutButton { Strings::Buttons::Logout };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TitleComponent)
};
