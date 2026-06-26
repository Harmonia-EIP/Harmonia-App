#pragma once

#include "PagesIncludes.h"
#include "AuthPageLookAndFeel.h"

/**
 * @class LoginPage
 * @brief Authentication page — sign in with identifier + password.
 *
 * Visual design mirrors the WelcomePage (dark gradient, animated waves).
 * Buttons use AuthPageLookAndFeel for a unified auth-flow aesthetic.
 */
class LoginPage : public juce::Component,
                  private juce::Timer
{
public:
    LoginPage(BackendManager& be,
              std::function<void(const UserSession&)> onSuccess);

    ~LoginPage() override;

    /** Triggered when the user taps the back arrow. */
    std::function<void()> onBack;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    struct WaveLayer
    {
        float amplitude   = 20.f;
        float frequency   = 0.015f;
        float phaseOffset = 0.f;
        float alphaFill   = 0.08f;
        float yRatio      = 0.82f;
        juce::Colour colour;
    };

    std::array<WaveLayer, 3> waveLayers;

    // -------------------------------------------------------------------------
    AppLookAndFeel authLookAndFeel;
    BackendManager& backend;
    std::function<void(const UserSession&)> onSuccess;

    // -------------------------------------------------------------------------
    // LookAndFeel instances
    // -------------------------------------------------------------------------
    std::unique_ptr<AuthPageLookAndFeel> lafLogin;
    std::unique_ptr<AuthPageLookAndFeel> lafBack;

    // -------------------------------------------------------------------------
    // Widgets
    // -------------------------------------------------------------------------
    juce::Rectangle<float> logoIconBounds;

    juce::Label      titleLabel;
    juce::Label      subtitleLabel;

    juce::TextEditor identifierField;
    juce::TextEditor passwordField;

    juce::TextButton loginButton { Strings::Buttons::SignIn };
    juce::TextButton backButton  { Strings::Buttons::Back  };

    // -------------------------------------------------------------------------
    float animationPhase = 0.f;

    void timerCallback() override;

    void drawWaveLayer(juce::Graphics& g,
                       const WaveLayer& layer,
                       float width,
                       float height,
                       float phase) const;

    void drawLogoIcon(juce::Graphics& g,
                      juce::Rectangle<float> bounds) const;

    void handleLogin();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoginPage)
};
