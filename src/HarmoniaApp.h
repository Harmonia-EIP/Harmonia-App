#pragma once
#include <JuceHeader.h>
#include "MainWindow.h"      
#include "backendManagement/BackendManager.h"

/**
 * @class HarmoniaApp
 * @brief Main entry point of the Harmonia application.
 *
 * This class manages the full lifecycle of the application, including
 * initialization, backend setup, session handling, and UI creation.
 *
 * During initialization, it:
 * - Sets up file-based logging in the executable directory
 * - Instantiates the BackendManager
 * - Loads and validates a persisted user session
 * - Performs remote synchronization of user profile parameters via backend APIs
 * - Initializes the main window with or without a valid session context
 *
 * The backend layer may perform external API calls (including AI-driven services)
 * to retrieve, validate, or enrich user-related data.
 *
 * @note This class follows the JUCE application lifecycle model.
 * @see juce::JUCEApplication
 */
class HarmoniaApp : public juce::JUCEApplication
{
public:
    /**
     * @brief Returns the application name.
     *
     * @return The name of the application as a juce::String.
     */
    const juce::String getApplicationName() override;

    /**
     * @brief Returns the application version.
     *
     * @return The current version string of the application.
     */
    const juce::String getApplicationVersion() override;

    /**
     * @brief Initializes the application.
     *
     * Called once at startup, this function:
     * - Initializes the logging system (file-based)
     * - Creates the backend manager
     * - Loads a previously saved session (if any)
     * - Validates session expiration
     * - Synchronizes user profile parameters via backend API calls
     *   (may include AI-based processing or remote data enrichment)
     * - Creates the main window with appropriate state
     *
     * If a valid session exists:
     * - The backend attempts to synchronize user parameters
     * - On success → application starts with user context
     * - On failure → session is cleared and fallback mode is used
     *
     * If no valid session exists:
     * - The application starts in a default (unauthenticated) state
     *
     * @param commandLine Command line arguments (currently unused).
     *
     * @pre Must be called by the JUCE framework during application startup.
     * @post Backend and UI are initialized and ready.
     *
     * @note Logging output is written next to the executable.
     * @warning Network/API failures will invalidate the current session.
     */
    void initialise(const juce::String& commandLine) override;

    /**
     * @brief Shuts down the application.
     *
     * Ensures proper cleanup of:
     * - UI components (main window)
     * - Backend resources
     * - Logging system
     *
     * @pre Must be called by the JUCE framework during shutdown.
     * @post All owned resources are released and logger is reset.
     *
     * @note The logger is explicitly cleared to prevent dangling pointers.
     */
    void shutdown() override;

private:
    /**
     * @brief Main application window.
     *
     * Owns and manages the primary UI of the application.
     */
    std::unique_ptr<MainWindow> mainWindow;

    /**
     * @brief Backend manager handling sessions and remote services.
     *
     * Responsible for:
     * - Session persistence and validation
     * - Communication with remote APIs
     * - Retrieval and synchronization of user profile data
     * - Interaction with AI-based backend services
     *
     * @note May perform network calls and should be considered latency-sensitive.
     */
    std::unique_ptr<BackendManager> backend;
};