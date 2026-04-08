#pragma once
#include <JuceHeader.h>
#include "../backendManagement/BackendManager.h"
#include "../themes/AppLookAndFeel.h"

/**
 * @class SignupPage
 * @brief User interface for account creation.
 *
 * @details
 * This component provides a complete sign-up form allowing users to create
 * a new account by entering their personal information.
 *
 * Features:
 * - Input fields for username, first name, last name, email, and password
 * - Validation to ensure all fields are filled
 * - Backend API call for account creation
 * - Success and error feedback via dialog windows
 *
 * The component uses a dedicated dark theme for consistency during the
 * authentication process.
 *
 * @note Navigation is handled externally via callbacks.
 */
class SignupPage : public juce::Component
{
public:
    /**
     * @brief Constructs the SignupPage.
     *
     * @param be Reference to BackendManager used for API calls
     * @param onSignupSuccess Callback triggered on successful account creation
     *
     * @details
     * - Initializes UI components (labels, input fields, buttons)
     * - Applies a dark LookAndFeel preset
     * - Binds button interactions to internal logic
     */
    SignupPage(BackendManager& be,
               std::function<void(const UserSession&)> onSignupSuccess);

    /**
     * @brief Paints the component background.
     *
     * @param g Graphics context
     *
     * @details
     * Fills the background using theme colors and updates text styling dynamically.
     */
    void paint(juce::Graphics&) override;

    /**
     * @brief Lays out child components.
     *
     * @details
     * Arranges form fields and buttons vertically with consistent spacing
     * and centers the content within the available area.
     */
    void resized() override;

    /**
     * @brief Callback triggered when the user wants to go back.
     *
     * @details
     * Typically used to navigate back to the welcome or login page.
     */
    std::function<void()> onBack;

private:
    /**
     * @brief Local LookAndFeel for authentication UI.
     *
     * @details
     * Always uses a dark theme, independent of user preferences.
     */
    AppLookAndFeel authLookAndFeel;

    /**
     * @brief Reference to backend manager.
     *
     * @details
     * Used to perform API calls such as account creation.
     */
    BackendManager& backend;

    /**
     * @brief Callback triggered on successful signup.
     *
     * @param UserSession Newly created user session
     */
    std::function<void(const UserSession&)> onSuccess;

    /**
     * @brief Title label displayed at the top of the form.
     */
    juce::Label titleLabel;

    /**
     * @brief Input field for username.
     */
    juce::TextEditor usernameField;

    /**
     * @brief Input field for first name.
     */
    juce::TextEditor firstnameField;

    /**
     * @brief Input field for last name.
     */
    juce::TextEditor lastnameField;

    /**
     * @brief Input field for email address.
     */
    juce::TextEditor emailField;

    /**
     * @brief Input field for password.
     *
     * @details
     * Displays masked characters for security.
     */
    juce::TextEditor passwordField;

    /**
     * @brief Button to submit the signup form.
     */
    juce::TextButton signupButton;

    /**
     * @brief Button to return to the previous screen.
     */
    juce::TextButton backButton;

    /**
     * @brief Handles the signup process.
     *
     * @details
     * - Retrieves and trims user input
     * - Validates that all fields are filled
     * - Sends signup request to backend
     * - Displays feedback (success or error)
     * - Triggers success callback on completion
     *
     * @warning This method performs a synchronous backend call.
     */
    void handleSignup();
};