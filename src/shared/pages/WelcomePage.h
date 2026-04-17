#pragma once

#include "PagesIncludes.h"

/**
 * @class WelcomePage
 * @brief Initial landing page allowing the user to choose between sign-in and sign-up.
 *
 * @details
 * This component represents the entry point of the authentication flow.
 * It displays:
 * - Application title
 * - A "Sign in" button
 * - A "Sign up" button
 *
 * The component uses a dedicated dark theme (independent from user preferences)
 * to ensure visual consistency during authentication.
 *
 * Interaction is handled via a callback function (`onChoice`), allowing the parent
 * component to react to user selection.
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
     * - Initializes UI elements (title and buttons)
     * - Sets up button callbacks
     */
    WelcomePage();

    ~WelcomePage() override;

    /**
     * @brief Paints the component background.
     *
     * @param g Graphics context used for rendering
     *
     * @details
     * Renders a vertical gradient background based on the authentication theme.
     */
    void paint(juce::Graphics&) override;

    /**
     * @brief Lays out child components.
     *
     * @details
     * Positions the title and buttons with consistent spacing and margins.
     */
    void resized() override;

    /**
     * @brief Callback triggered when the user makes a choice.
     *
     * @param isSignup True if "Sign up" was selected, false if "Sign in"
     *
     * @details
     * This callback must be set by the parent component to handle navigation.
     */
    std::function<void(bool)> onChoice;

private:
    /**
     * @brief Local LookAndFeel for authentication UI.
     *
     * @details
     * Always uses a dark preset, independent of user-selected themes.
     */
    AppLookAndFeel authLookAndFeel;

    /**
     * @brief Application title label.
     */
    juce::Label titleLabel;

    /**
     * @brief Button to trigger sign-in flow.
     */
    juce::TextButton signinButton;

    /**
     * @brief Button to trigger sign-up flow.
     */
    juce::TextButton signupButton;

    float animationPhase = 0.0f;

    void timerCallback() override;
};
