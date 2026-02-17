#include "WelcomePage.h"
#include "../themes/AppColourIds.h"

WelcomePage::WelcomePage()
{
    // LookAndFeel local toujours sombre (pas dépendant de l'utilisateur)
    authLookAndFeel.setThemePreset(AppLookAndFeel::Preset::Dark);
    setLookAndFeel(&authLookAndFeel);

    // ===== TITLE =====
    titleLabel.setText("Harmonia", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(36.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // ===== BUTTONS =====
    signinButton.setButtonText("Sign in");
    signupButton.setButtonText("Sign up");
    addAndMakeVisible(signinButton);
    addAndMakeVisible(signupButton);

    // ===== CALLBACKS =====
    signinButton.onClick = [this]()
    {
        if (onChoice) onChoice(false);
    };

    signupButton.onClick = [this]()
    {
        if (onChoice) onChoice(true);
    };
}


void WelcomePage::paint(juce::Graphics& g)
{
    // Fond basé sur le thème AUTH (toujours dark)
    auto bg = findColour(AppColourIds::backgroundId);
    auto accent = findColour(AppColourIds::accentId);

    juce::ColourGradient gradient(
        bg,
        0, 0,
        accent.withAlpha(0.25f),
        0, (float)getHeight(),
        false
    );

    g.setGradientFill(gradient);
    g.fillAll();

    titleLabel.setColour(juce::Label::textColourId,
                         findColour(AppColourIds::textPrimaryId));
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
