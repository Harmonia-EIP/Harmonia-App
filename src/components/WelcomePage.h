#pragma once
#include <JuceHeader.h>
#include "../themes/AppLookAndFeel.h"

class WelcomePage : public juce::Component
{
public:
    WelcomePage();
    void paint(juce::Graphics&) override;
    void resized() override;

    std::function<void(bool)> onChoice;

private:
    AppLookAndFeel authLookAndFeel;

    juce::Label titleLabel;
    juce::TextButton signinButton, signupButton;
};
