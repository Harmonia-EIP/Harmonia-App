/**
 * @file AppController.h
 * @brief Main application navigation controller.
 *
 * AppController manages the complete UI navigation flow
 * of the Harmonia application.
 *
 * Responsibilities:
 * - Restoring saved user sessions
 * - Managing authentication screens
 * - Displaying the main synthesizer interface
 * - Handling logout flow
 * - Synchronizing backend session state
 *
 * Navigation flow:
 * WelcomePage
 * ├── LoginPage
 * ├── SignupPage
 * └── MainComponent
 */
#pragma once

#include "JuceHeader.h"

#include "backendManagement/BackendManager.h"
#include "pages/WelcomePage.h"
#include "pages/LoginPage.h"
#include "pages/SignupPage.h"
#include "MainComponent.h"

/**
 * @class AppController
 * @brief Root UI controller of the Harmonia application.
 *
 * This component acts as the central navigation manager
 * responsible for dynamically switching between:
 * - Authentication pages
 * - Welcome screens
 * - Main synthesizer interface
 *
 * It also manages persistent sessions and backend
 * synchronization during application startup.
 */
class AppController : public juce::Component
{
public:
    /**
     * @brief Creates the application controller.
     *
     * On startup:
     * - Attempts to restore a saved session
     * - Verifies session expiration
     * - Opens the main interface if authenticated
     * - Otherwise shows the welcome screen
     *
     * @param p Reference to the audio processor.
     */
    AppController(HarmoniaAudioProcessor& p);

    /**
     * @brief Updates the bounds of the active screen.
     */
    void resized() override;

private:
    /** Reference to the audio processor instance. */
    HarmoniaAudioProcessor& processor;

    /** Backend manager handling authentication and sessions. */
    BackendManager backend;

    /**
     * Currently displayed screen component.
     *
     * Can contain:
     * - WelcomePage
     * - LoginPage
     * - SignupPage
     * - MainComponent
     */
    std::unique_ptr<juce::Component> currentComponent;

    /** Active authenticated user session. */
    std::optional<UserSession> currentSession;

    /**
     * @brief Displays the welcome page.
     *
     * Entry point for authentication navigation.
     */
    void showWelcomeScreen();

    /**
     * @brief Displays the login page.
     */
    void showLoginScreen();

    /**
     * @brief Displays the signup page.
     */
    void showSignupScreen();

    /**
     * @brief Displays the main application interface.
     *
     * @param session Authenticated user session.
     */
    void showMainScreen(const UserSession& session);
};