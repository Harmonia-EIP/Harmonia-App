#pragma once

#include "AuthPageLookAndFeel.h"
#include "PagesIncludes.h"

/**
 * @class SignupPage
 * @brief Authentication page — create a new Harmonia account.
 *
 * Visual design mirrors LoginPage and WelcomePage (dark gradient, animated
 * waves, logo PNG without container). Buttons use AuthPageLookAndFeel.
 */
class SignupPage : public juce::Component, private juce::Timer {
public:
  SignupPage(BackendManager &be,
             std::function<void(const UserSession &)> onSignupSuccess);

  ~SignupPage() override;

  /** Triggered when the user taps the back arrow. */
  std::function<void()> onBack;

  void paint(juce::Graphics &) override;
  void resized() override;

private:
  struct WaveLayer {
    float amplitude = 20.f;
    float frequency = 0.015f;
    float phaseOffset = 0.f;
    float alphaFill = 0.08f;
    float yRatio = 0.82f;
    juce::Colour colour;
  };

  std::array<WaveLayer, 3> waveLayers;

  // -------------------------------------------------------------------------
  AppLookAndFeel authLookAndFeel;
  AuthPageLookAndFeel fieldLaf;
  BackendManager &backend;
  std::function<void(const UserSession &)> onSuccess;

  // -------------------------------------------------------------------------
  // LookAndFeel instances
  // -------------------------------------------------------------------------
  std::unique_ptr<AuthPageLookAndFeel> lafSignup;
  std::unique_ptr<AuthPageLookAndFeel> lafBack;

  // -------------------------------------------------------------------------
  // Widgets
  // -------------------------------------------------------------------------

  /** Logo image loaded from binary resources (PNG). */
  juce::Image logoImage;

  juce::Rectangle<float> logoIconBounds;

  juce::Label titleLabel;
  juce::Label subtitleLabel;

  juce::TextEditor usernameField;
  juce::TextEditor firstnameField;
  juce::TextEditor lastnameField;
  juce::TextEditor emailField;
  juce::TextEditor passwordField;

  juce::TextButton signupButton;
  juce::TextButton backButton;

  // -------------------------------------------------------------------------
  float animationPhase = 0.f;

  void timerCallback() override;

  void drawWaveLayer(juce::Graphics &g, const WaveLayer &layer, float width,
                     float height, float phase) const;

  /** Draws the logo image only (no container, no border). */
  void drawLogoIcon(juce::Graphics &g, juce::Rectangle<float> bounds) const;

  void handleSignup();

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SignupPage)
};