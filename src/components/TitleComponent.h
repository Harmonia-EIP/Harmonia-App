#pragma once
#include <juce_gui_basics/juce_gui_basics.h>
#include "../SupabaseManager.h"

class TitleComponent : public juce::Component,
                       public juce::Button::Listener
{
public:
    TitleComponent(const juce::String& textToDisplay, SupabaseManager& supabaseManager);
    ~TitleComponent() override;

    std::function<void()> onLogout;

    void paint(juce::Graphics&) override;
    void resized() override;
    void buttonClicked(juce::Button*) override;

private:
    SupabaseManager& supabase;
    juce::String title;
    juce::Label titleLabel;
    juce::Label pseudoLabel;
    juce::TextButton logoutButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TitleComponent)
};
