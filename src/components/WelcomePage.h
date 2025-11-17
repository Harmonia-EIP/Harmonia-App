#pragma once
#include <JuceHeader.h>

class WelcomePage : public juce::Component
{
public:
    WelcomePage();
    void paint(juce::Graphics&) override;
    void resized() override;

    std::function<void(bool)> onChoice;

private:
    juce::Label titleLabel;
    juce::TextButton signinButton, signupButton;
};
