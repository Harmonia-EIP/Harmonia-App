#pragma once

#include "PagesIncludes.h"
#include "AuthPageLookAndFeel.h"
/**
 * @class LoginPage
 * @brief Authentication page — sign in with identifier + password.
 *
 * Visual design mirrors the WelcomePage (dark gradient, animated waves,
 * logo PNG without container). Buttons use AuthPageLookAndFeel.
 */
class LoginPage : public juce::Component,
                  private juce::Timer
{
public:
    LoginPage(BackendManager& be,
              std::function<void(const UserSession&)> onSuccess);

    ~LoginPage() override;

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
    AuthPageLookAndFeel fieldLaf;
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

    /** Logo image loaded from binary resources (PNG). */
    juce::Image logoImage;

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

    /** Draws the logo image only (no container, no border). */
    void drawLogoIcon(juce::Graphics& g,
                      juce::Rectangle<float> bounds) const;

    void handleLogin();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(LoginPage)
};