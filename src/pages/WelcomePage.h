#pragma once

#include "PagesIncludes.h"

// =============================================================================
// Custom LookAndFeel for the WelcomePage buttons
//  - Outlined : smoked-glass pill, gradient cyan -> indigo border,
//               glow on hover or keyboard focus (Sign in + Create account)
//  - Ghost    : text only, shadowed to stay readable over the waves (Guest),
//               underlined on hover or keyboard focus
// =============================================================================
class WelcomeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    enum class BtnStyle { Outlined, Ghost };

    explicit WelcomeLookAndFeel(BtnStyle style) : m_style(style) {}

    void drawButtonBackground(juce::Graphics& g,
                              juce::Button& button,
                              const juce::Colour&,
                              bool isMouseOver,
                              bool isDown) override
    {
        if (m_style == BtnStyle::Ghost)
            return;

        // Souris OU focus clavier (Tab) : même retour visuel
        const bool isHighlighted = isMouseOver || button.hasKeyboardFocus(true);

        // 4 px margin so the glow is not clipped
        auto area = button.getLocalBounds().toFloat().reduced(4.f);
        if (isDown)
            area = area.reduced(1.f);

        const float r = area.getHeight() * 0.5f; // pill

        // Glass fill
        g.setColour(juce::Colours::white.withAlpha(isDown ? 0.10f
                                                          : isHighlighted ? 0.07f : 0.035f));
        g.fillRoundedRectangle(area, r);

        // Soft glow on hover / focus
        if (isHighlighted || isDown)
        {
            g.setColour(HarmoniaColours::waveCyan.withAlpha(0.10f));
            g.drawRoundedRectangle(area.expanded(1.5f), r + 1.5f, 5.f);

            g.setColour(HarmoniaColours::waveCyan.withAlpha(0.16f));
            g.drawRoundedRectangle(area.expanded(0.5f), r + 0.5f, 2.f);
        }

        // Gradient border cyan -> indigo
        const float a = isHighlighted || isDown ? 1.0f : 0.65f;

        juce::ColourGradient border(
            HarmoniaColours::waveCyan.withAlpha(a),   area.getX(),     area.getY(),
            HarmoniaColours::waveIndigo.withAlpha(a), area.getRight(), area.getBottom(),
            false);
        g.setGradientFill(border);
        g.drawRoundedRectangle(area, r, 1.3f);
    }

    void drawButtonText(juce::Graphics& g,
                        juce::TextButton& button,
                        bool isMouseOver,
                        bool /*isDown*/) override
    {
        const bool isHighlighted = isMouseOver || button.hasKeyboardFocus(true);
        const auto b = button.getLocalBounds();

        if (m_style == BtnStyle::Ghost)
        {
            const juce::Font font("Inter", 13.f, juce::Font::plain);
            g.setFont(font);

            // shadow: stays readable even over a bright wave
            g.setColour(juce::Colours::black.withAlpha(0.45f));
            g.drawText(button.getButtonText(), b.translated(0, 1),
                       juce::Justification::centred, false);

            g.setColour(juce::Colours::white.withAlpha(isHighlighted ? 1.0f : 0.80f));
            g.drawText(button.getButtonText(), b, juce::Justification::centred, false);

            if (isHighlighted)
            {
                juce::GlyphArrangement ga;
                ga.addLineOfText(font, button.getButtonText(), 0.f, 0.f);
                const float tw = ga.getBoundingBox(0, -1, true).getWidth();
                g.fillRect((float) b.getCentreX() - tw * 0.5f,
                           (float) b.getCentreY() + 9.f, tw, 1.f);
            }
            return;
        }

        g.setColour(HarmoniaColours::textPrimary.withAlpha(isHighlighted ? 1.0f : 0.90f));
        g.setFont(juce::Font("Inter", 14.f, juce::Font::plain)
                      .withExtraKerningFactor(0.04f));
        g.drawFittedText(button.getButtonText(), b.reduced(10, 0),
                         juce::Justification::centred, 1);
    }

private:
    BtnStyle m_style;
};

/**
 * @class WelcomePage
 * @brief Initial landing page allowing the user to choose between sign-in and sign-up.
 *
 * @details
 * Entry point of the authentication flow. Displays:
 * - Application logo, title and subtitle
 * - Animated wave background (blue/cyan theme)
 * - A "Sign in" button (outlined pill)
 * - A "Create account" button (outlined pill)
 * - A "Continue as guest" text button
 *
 * Uses a dedicated dark blue theme independent from user preferences.
 * Interaction is delegated through the `onChoice` callback.
 *
 * Keyboard navigation:
 * - Tab / Shift+Tab : move between the three buttons (stays inside the page)
 * - Up / Down arrows : same, as a convenience
 * - Enter / Space : press the focused button
 * The "Sign in" button takes the focus when the page is shown.
 */
class WelcomePage : public juce::Component,
                    private juce::Timer
{
public:
    WelcomePage();
    ~WelcomePage() override;

    void paint(juce::Graphics& g) override;
    void resized() override;

    /** Up/Down arrows move the focus between buttons (Tab is handled by JUCE). */
    bool keyPressed(const juce::KeyPress& key) override;

    /** Gives the keyboard focus to the first button when the page appears. */
    void visibilityChanged() override;

    enum class Choice
    {
        SignIn,
        SignUp,
        Guest
    };

    std::function<void(Choice)> onChoice;

private:

    struct WaveLayer
    {
        float amplitude   = 20.f;
        float frequency   = 0.015f;
        float phaseOffset = 0.f;
        float alphaFill   = 0.12f;
        float yRatio      = 0.55f;
        juce::Colour colour;
    };

    std::array<WaveLayer, 4> waveLayers;

    AppLookAndFeel authLookAndFeel;

    // One LookAndFeel per button style
    std::unique_ptr<juce::LookAndFeel> lafSignIn;
    std::unique_ptr<juce::LookAndFeel> lafSignUp;
    std::unique_ptr<juce::LookAndFeel> lafGuest;

    juce::Image logoImage;
    juce::Rectangle<float> logoIconBounds;

    juce::Label titleLabel;
    juce::Label subtitleLabel;

    juce::TextButton signinButton;   // outlined
    juce::TextButton signupButton;   // outlined
    juce::TextButton guestButton;    // ghost

    float animationPhase = 0.f;

    void timerCallback() override;

    void drawWaveLayer(juce::Graphics& g,
                       const WaveLayer& layer,
                       float width,
                       float height,
                       float phase) const;

    void drawLogoIcon(juce::Graphics& g,
                      juce::Rectangle<float> bounds) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WelcomePage)
};