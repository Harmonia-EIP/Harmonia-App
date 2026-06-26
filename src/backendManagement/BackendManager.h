/**
 * @file BackendManager.h
 * @brief Main backend facade used to centralize all backend services.
 *
 * BackendManager acts as the application's main entry point
 * for all backend-related operations.
 *
 * Managed services:
 * - Authentication
 * - User profile management
 * - AI preset generation
 * - Session persistence
 * - User preference synchronization
 * - Backend configuration
 *
 * This architecture simplifies communication between
 * the UI layer and backend services.
 */

#pragma once

#include <juce_core/juce_core.h>
#include <map>
#include <string>
#include <optional>

#include "BackendTypes.h"

#include "BackendAiManager.h"
#include "BackendAuthManager.h"
#include "BackendProfileManager.h"

class BackendAuthManager;
class BackendProfileManager;
class BackendAiManager;

/**
 * @class BackendManager
 * @brief Central facade for all backend operations.
 *
 * BackendManager encapsulates specialized backend modules:
 * - BackendAuthManager
 * - BackendProfileManager
 * - BackendAiManager
 *
 * Responsibilities:
 * - Manage user authentication
 * - Handle local session storage
 * - Synchronize profile settings
 * - Load backend configuration
 * - Provide logging utilities
 * - Delegate backend requests
 *
 * This class follows the Facade design pattern
 * to provide a simplified API for the application.
 */
class BackendManager
{
public:

    /**
     * @brief Creates and initializes the backend manager.
     *
     * During initialization:
     * - Configuration is loaded
     * - Application directories are created
     * - Backend managers are instantiated
     * - Session file paths are initialized
     */
    BackendManager();

    /**
     * @brief Destroys the backend manager and releases resources.
     */
    ~BackendManager();

    // =========================================================
    // AUTHENTICATION
    // =========================================================

    /**
     * @brief Authenticates a user using username/email and password.
     *
     * @param usernameOrEmail Username or email address.
     * @param password User password.
     *
     * @return AuthResult Authentication result.
     */
    AuthResult loginUser(const juce::String& usernameOrEmail,
                         const juce::String& password);

    /**
     * @brief Creates a new user account.
     *
     * @param username User username.
     * @param firstname User first name.
     * @param lastname User last name.
     * @param email User email address.
     * @param password User password.
     *
     * @return AuthResult Registration result.
     */
    AuthResult signupUser(const juce::String& username,
                          const juce::String& firstname,
                          const juce::String& lastname,
                          const juce::String& email,
                          const juce::String& password);

    /**
     * @brief Loads the locally stored user session.
     *
     * @return Optional UserSession if available.
     */
    std::optional<UserSession> loadSession();

    /**
     * @brief Saves the current user session locally.
     *
     * @param session Session data to save.
     */
    void saveSession(const UserSession& session);

    /**
     * @brief Clears the locally stored session.
     */
    void clearSession();

    // =========================================================
    // AI
    // =========================================================

    /**
     * @brief Generates an AI preset from a text prompt.
     *
     * The backend returns raw JSON data that can later
     * be parsed by the preset loader.
     *
     * @param prompt User prompt.
     *
     * @return AiResult Generated preset result.
     */
    AiResult generatePreset(const juce::String& prompt);

    // =========================================================
    // CONFIGURATION
    // =========================================================

    /**
     * @brief Returns the backend API base URL.
     *
     * @return API URL string.
     */
    const juce::String& getApiUrl() const;

    /**
     * @brief Returns the session file location.
     *
     * @return Session file path.
     */
    const juce::File& getSessionFile() const;

    /**
     * @brief Writes a message into the backend log file.
     *
     * @param message Message to append.
     */
    void writeLog(const juce::String& message) const;

    // =========================================================
    // PROFILE
    // =========================================================

    /**
     * @brief Retrieves the current user profile.
     *
     * @return ProfileResult User profile data.
     */
    ProfileResult getProfile();

    /**
     * @brief Synchronizes profile parameters asynchronously.
     *
     * @param session Current user session.
     */
    void syncProfileParamsInBackground(const UserSession& session);

    /**
     * @brief Synchronizes profile parameters immediately.
     *
     * @param session Current user session.
     *
     * @return Updated session if synchronization succeeds.
     */
    std::optional<UserSession> syncProfileParams(const UserSession& session);

    /**
     * @brief Updates the current UI theme.
     *
     * @param themeId Theme identifier.
     *
     * @return ProfileResult Operation result.
     */
    ProfileResult updateTheme(int themeId);

    /**
     * @brief Updates the current UI layout.
     *
     * @param layoutId Layout identifier.
     *
     * @return ProfileResult Operation result.
     */
    ProfileResult updateLayout(int layoutId);

    void updateLocalTheme(int themeId);

    void updateThemeAsync(int themeId);

private:

    /**
     * @brief Returns the application data directory.
     *
     * Creates the directory if it does not already exist.
     *
     * @return Application data folder.
     */
    juce::File getAppDataDir() const;

    /**
     * @brief Backend log file.
     */
    juce::File logFile;

    /**
     * @brief Backend API base URL.
     */
    juce::String apiUrl;

    /**
     * @brief Local session file path.
     */
    juce::File sessionFile;

    /**
     * @brief Authentication manager instance.
     */
    std::unique_ptr<BackendAuthManager> authManager;

    /**
     * @brief Profile manager instance.
     */
    std::unique_ptr<BackendProfileManager> profileManager;

    /**
     * @brief AI manager instance.
     */
    std::unique_ptr<BackendAiManager> aiManager;
};