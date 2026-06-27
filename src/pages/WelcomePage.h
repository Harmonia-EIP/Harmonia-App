#pragma once

#include "PagesIncludes.h"

// =============================================================================
// Custom LookAndFeel for the WelcomePage buttons
// =============================================================================
class WelcomeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    enum class BtnStyle { Filled, Outlined, Ghost };

    explicit WelcomeLookAndFeel(BtnStyle style) : m_style(style) {}

    void drawButtonBackground(juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour&,
                               bool isHighlighted,
                               bool) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        const float radius = 10.f;

        if (m_style == BtnStyle::Filled)
        {
            float bgAlpha     = isHighlighted ? 0.12f : 0.06f;
            float borderAlpha = isHighlighted ? 0.35f : 0.15f;

            g.setColour(juce::Colours::white.withAlpha(bgAlpha));
            g.fillRoundedRectangle(bounds, radius);

            if (isHighlighted)
            {
                g.setColour(HarmoniaColours::waveBlue.withAlpha(0.18f));
                g.drawRoundedRectangle(bounds.expanded(2.f), radius + 2.f, 3.f);
            }

            g.setColour(HarmoniaColours::waveBlue.withAlpha(borderAlpha));
            g.drawRoundedRectangle(bounds, radius, 1.f);
        }
        else if (m_style == BtnStyle::Outlined)
        {
            float fillAlpha   = isHighlighted ? 0.06f : 0.03f;
            float borderAlpha = isHighlighted ? 0.22f : 0.12f;

            g.setColour(juce::Colours::white.withAlpha(fillAlpha));
            g.fillRoundedRectangle(bounds, radius);

            g.setColour(juce::Colours::white.withAlpha(borderAlpha));
            g.drawRoundedRectangle(bounds, radius, 1.f);
        }
    }

    void drawButtonText(juce::Graphics& g,
                    juce::TextButton& button,
                    bool isHighlighted,
                    bool /*isDown*/) override
    {
        juce::Colour colour;

        if (m_style == BtnStyle::Ghost)
        {
            colour = isHighlighted
                ? HarmoniaColours::waveCyan.withAlpha(0.95f)
                : HarmoniaColours::textMuted;
        }
        else
        {
            colour = isHighlighted
                ? HarmoniaColours::textPrimary
                : HarmoniaColours::textPrimary.withAlpha(0.85f);
        }

        g.setColour(colour);
        g.setFont(juce::Font("Inter", 15.f, juce::Font::plain));
        g.drawFittedText(button.getButtonText(),
                        button.getLocalBounds(),
                        juce::Justification::centred,
                        1);
    }

private:
    BtnStyle m_style;
};

/**
 * @class WelcomePage
 * @brief Initial landing page allowing the user to choose between sign-in and sign-up.
 *
 * @details
 * This component represents the entry point of the authentication flow.
 * It displays:
 * - Application title and subtitle
 * - Animated wave background (blue/cyan theme)
 * - A "Sign in" button
 * - A "Create account" button
 * - A "Continue as guest" text button
 *
 * The component uses a dedicated dark blue theme independent from user preferences.
 * Interaction is handled via a callback function (`onChoice`).
 *
 * @note This component does not handle navigation directly; it delegates actions
 * through the callback.
 */
class WelcomePage : public juce::Component,
                    private juce::Timer
{
public:
    /**
     * @brief Constructs the WelcomePage.
     *
     * @details
     * - Applies a dark LookAndFeel preset
     * - Initializes UI elements (logo icon, title, subtitle, buttons)
     * - Sets up wave layer data and button callbacks
     * - Starts the 60 Hz animation timer
     */
    WelcomePage();

    ~WelcomePage() override;

    /**
     * @brief Paints the component background and wave animation.
     *
     * @param g Graphics context used for rendering
     *
     * @details
     * Renders a dark-to-blue vertical gradient and multiple animated
     * sine-wave layers in blue/cyan/indigo tones.
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Lays out child components.
     *
     * @details
     * Vertically centres the logo icon, title, subtitle, and buttons
     * in a narrow column, matching the Harmonia landing page layout.
     */
    void resized() override;

    enum class Choice
    {
        SignIn,
        SignUp,
        Guest
    };

    std::function<void(Choice)> onChoice;

private:

    // -------------------------------------------------------------------------
    // Wave layer descriptor
    // -------------------------------------------------------------------------
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

    // -------------------------------------------------------------------------
    // LookAndFeel
    // -------------------------------------------------------------------------
    AppLookAndFeel authLookAndFeel;

    // -------------------------------------------------------------------------
    // Button LookAndFeels (one per visual style)
    // -------------------------------------------------------------------------
    std::unique_ptr<juce::LookAndFeel> lafSignIn;
    std::unique_ptr<juce::LookAndFeel> lafSignUp;
    std::unique_ptr<juce::LookAndFeel> lafGuest;

    // -------------------------------------------------------------------------
    // Widgets
    // -------------------------------------------------------------------------

    /** Rounded-square logo icon drawn in paint(). */
    juce::Rectangle<float> logoIconBounds;

    /** "Harmonia" title. */
    juce::Label titleLabel;

    /** "AI SYNTH PRESET GENERATOR" subtitle. */
    juce::Label subtitleLabel;

    /** Primary "Sign in" button (filled accent style). */
    juce::TextButton signinButton;

    /** Secondary "Create account" button (outlined style). */
    juce::TextButton signupButton;

    /** Ghost "Continue as guest" button (no border). */
    juce::TextButton guestButton;

    // -------------------------------------------------------------------------
    // Animation
    // -------------------------------------------------------------------------
    float animationPhase = 0.f;

    void timerCallback() override;

    // -------------------------------------------------------------------------
    // Helpers
    // -------------------------------------------------------------------------

    /**
     * @brief Draws a single animated wave layer.
     *
     * @param g          Graphics context
     * @param layer      Wave descriptor
     * @param width      Component width
     * @param height     Component height
     * @param phase      Current animation phase (radians)
     */
    void drawWaveLayer(juce::Graphics& g,
                       const WaveLayer& layer,
                       float width,
                       float height,
                       float phase) const;

    /**
     * @brief Draws the tilde (~) logo icon.
     *
     * @param g      Graphics context
     * @param bounds Bounding rectangle for the icon box
     */
    void drawLogoIcon(juce::Graphics& g,
                      juce::Rectangle<float> bounds) const;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WelcomePage)
};
