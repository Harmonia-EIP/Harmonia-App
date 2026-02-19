#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/AppLookAndFeel.h"
#include "../backendManagement/BackendManager.h"

class TitleComponent : public juce::Component,
                       private juce::Button::Listener
{
public:
    TitleComponent (const juce::String& textToDisplay, BackendManager& be);
    ~TitleComponent() override;

    void paint (juce::Graphics& g) override;
    void resized() override;

    std::function<void()> onLogout;
    std::function<void(AppLookAndFeel::Preset)> onThemeSelected;
    std::function<void(AppLookAndFeel::LayoutPreset)> onLayoutSelected;

private:
    void buttonClicked (juce::Button* button) override;

    BackendManager& backend;

    juce::String title;
    juce::Label titleLabel;
    juce::Label pseudoLabel; 

    juce::TextButton themeButton  { "Theme" };
    juce::TextButton layoutButton { "Layout" };
    juce::TextButton logoutButton { "Logout" };

    AppLookAndFeel::LayoutPreset currentLayout = AppLookAndFeel::LayoutPreset::Layout1;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TitleComponent)
};
