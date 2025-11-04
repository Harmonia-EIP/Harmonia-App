#include "WelcomePage.h"

WelcomePage::WelcomePage()
{
    // --- Titre principal ---
    titleLabel.setText("Harmonia", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(36.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    // --- Boutons ---
    signinButton.setButtonText("Sign in");
    signupButton.setButtonText("Sign up");

    signinButton.setColour(juce::TextButton::buttonColourId, juce::Colours::lightblue);
    signupButton.setColour(juce::TextButton::buttonColourId, juce::Colours::limegreen);

    signinButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    signupButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);

    addAndMakeVisible(signinButton);
    addAndMakeVisible(signupButton);

    // --- Actions ---
    signinButton.onClick = [this]() {
        if (onChoice)
            onChoice(false); // false = login
    };

    signupButton.onClick = [this]() {
        if (onChoice)
            onChoice(true); // true = signup
    };
}

void WelcomePage::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkslategrey);

    // Petit effet visuel simple : dégradé
    juce::ColourGradient gradient(
        juce::Colours::darkslategrey,
        0, 0,
        juce::Colours::black,
        0, (float)getHeight(),
        false
    );
    g.setGradientFill(gradient);
    g.fillAll();
}

void WelcomePage::resized()
{
    auto area = getLocalBounds().reduced(200);
    titleLabel.setBounds(area.removeFromTop(100));

    area.removeFromTop(50);

    auto buttonHeight = 50;
    signinButton.setBounds(area.removeFromTop(buttonHeight));
    area.removeFromTop(20);
    signupButton.setBounds(area.removeFromTop(buttonHeight));
}
