#include "WelcomePage.h"

WelcomePage::WelcomePage()
{
    // LookAndFeel local toujours sombre
    authLookAndFeel.setThemePreset(AppLookAndFeel::ThemePreset::Dark);
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

    // ===== ANIMATION =====
    startTimerHz(60); // 60 FPS
}

WelcomePage::~WelcomePage()
{
    setLookAndFeel(nullptr);
}

void WelcomePage::timerCallback()
{
    animationPhase += 0.03f;
    repaint();
}

void WelcomePage::paint(juce::Graphics& g)
{
    auto bg = findColour(AppColourIds::backgroundId);
    auto accent = findColour(AppColourIds::accentId);

    // ===== GRADIENT ANIMÉ =====
    auto dynamicAccent = accent.withRotatedHue(animationPhase * 0.05f);

    juce::ColourGradient gradient(
        bg,
        0, 0,
        dynamicAccent.withAlpha(0.3f),
        (float)getWidth(),
        (float)getHeight(),
        false
    );

    g.setGradientFill(gradient);
    g.fillAll();

    // ===== WAVE ANIMÉE =====
    juce::Path wave;

    float centerY = getHeight() * 0.6f;
    float amplitude = 30.0f;
    float frequency = 0.01f;

    wave.startNewSubPath(0, centerY);

    for (int x = 0; x < getWidth(); ++x)
    {
        float y = centerY + std::sin(x * frequency + animationPhase) * amplitude;
        wave.lineTo((float)x, y);
    }

    // Glow léger
    g.setColour(dynamicAccent.withAlpha(0.15f));
    g.strokePath(wave, juce::PathStrokeType(6.0f));

    // Ligne principale
    g.setColour(dynamicAccent.withAlpha(0.6f));
    g.strokePath(wave, juce::PathStrokeType(2.0f));

    // ===== TEXT COLOR =====
    titleLabel.setColour(
        juce::Label::textColourId,
        findColour(AppColourIds::textPrimaryId)
    );
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