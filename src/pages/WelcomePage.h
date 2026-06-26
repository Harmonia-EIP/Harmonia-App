#pragma once

#include "PagesIncludes.h"

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
        float alphaFill   = 0.12f;    ///< fill alpha for the filled region below
        float yRatio      = 0.55f;    ///< vertical centre (0–1 of component height)
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
