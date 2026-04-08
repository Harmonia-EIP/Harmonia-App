#pragma once
#include <JuceHeader.h>
#include "../backendManagement/BackendManager.h"
#include "../themes/AppLookAndFeel.h"

/**
 * @class LoginPage
 * @brief User interface for account authentication.
 *
 * @details
 * This component provides a login form allowing users to authenticate
 * using their email and password.
 *
 * Features:
 * - Input fields for email and password
 * - Input validation to ensure required fields are filled
 * - Backend API call for authentication
 * - Error feedback via dialog windows
 *
 * The component uses a dedicated dark theme for consistency during
 * the authentication process.
 *
 * @note Navigation and post-login behavior are handled externally via callbacks.
 */
class LoginPage : public juce::Component
{
public:
    /**
     * @brief Constructs the LoginPage.
     *
     * @param be Reference to BackendManager used for authentication
     * @param onSuccess Callback triggered on successful login
     *
     * @details
     * - Initializes UI components (title, fields, buttons)
     * - Applies a dark LookAndFeel preset
     * - Binds button interactions to internal logic
     */
    LoginPage(BackendManager& be,
              std::function<void(const UserSession&)> onSuccess);

    /**
     * @brief Callback triggered when the user wants to go back.
     *
     * @details
     * Typically used to navigate back to the welcome page.
     */
    std::function<void()> onBack;

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
     * Arranges the title, input fields, and buttons vertically
     * with consistent spacing and margins.
     */
    void resized() override;

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
     * Used to perform authentication API calls.
     */
    BackendManager& backend;

    /**
     * @brief Callback triggered on successful login.
     *
     * @param UserSession Authenticated user session
     */
    std::function<void(const UserSession&)> onSuccess;

    /**
     * @brief Title label displayed at the top of the form.
     */
    juce::Label titleLabel;

    /**
     * @brief Input field for user email.
     */
    juce::TextEditor emailField;

    /**
     * @brief Input field for user password.
     *
     * @details
     * Displays masked characters for security.
     */
    juce::TextEditor passwordField;

    /**
     * @brief Button used to submit login credentials.
     */
    juce::TextButton loginButton;

    /**
     * @brief Button used to navigate back to the previous screen.
     */
    juce::TextButton backButton;

    /**
     * @brief Handles the login process.
     *
     * @details
     * - Retrieves and trims user input
     * - Validates that required fields are filled
     * - Sends authentication request to backend
     * - Displays error messages if authentication fails
     * - Triggers success callback on success
     *
     * @warning This method performs a synchronous backend call.
     */
    void handleLogin();
};