#pragma once

#include <juce_core/juce_core.h>
#include <map>
#include <string>
#include <optional>

#include "BackendTypes.h"


class BackendAuthManager;
class BackendAiManager;
class BackendProfileManager;

/**
 * @class BackendManager
 * @brief Main facade for backend interactions (authentication, AI, profile).
 *
 * @details
 * This class centralizes all communication with the backend API and delegates
 * responsibilities to specialized managers:
 *
 * - BackendAuthManager: authentication and session handling
 * - BackendAiManager: AI-based preset generation
 * - BackendProfileManager: user profile management
 *
 * It also handles:
 * - Environment configuration loading (.env)
 * - API base URL setup
 * - Local session file management
 *
 * @note Uses raw pointers for internal managers. Lifetime is managed manually.
 */
class BackendManager
{
public:
    /**
     * @brief Constructs the BackendManager.
     *
     * @details
     * - Loads environment variables from .env file
     * - Initializes internal managers
     * - Configures API base URL
     * - Defines session file location
     */
    BackendManager();

    /**
     * @brief Destructor.
     *
     * @warning Ensure all allocated managers are properly deleted to avoid leaks.
     */
    ~BackendManager();

    // ===================== AUTH =====================

    /**
     * @brief Authenticate a user.
     *
     * @param usernameOrEmail Username or email
     * @param password User password
     * @return AuthResult containing success flag, session data, and error message
     *
     * @details
     * Sends a POST request to the backend authentication endpoint.
     * On success:
     * - A session is created
     * - Session is saved locally
     * - Profile synchronization is triggered asynchronously
     */
    AuthResult loginUser(const juce::String& usernameOrEmail,
                         const juce::String& password);

    /**
     * @brief Register a new user.
     *
     * @param username Username
     * @param firstname First name
     * @param lastname Last name
     * @param email Email address
     * @param password Password
     * @return AuthResult containing session data if successful
     */
    AuthResult signupUser(const juce::String& username,
                          const juce::String& firstname,
                          const juce::String& lastname,
                          const juce::String& email,
                          const juce::String& password);

    /**
     * @brief Load session from disk.
     *
     * @return UserSession if present, otherwise std::nullopt
     *
     * @note Does not validate expiration time.
     */
    std::optional<UserSession> loadSession();

    /**
     * @brief Save session to disk.
     *
     * @param session Session to store
     */
    void saveSession(const UserSession& session);

    /**
     * @brief Clear stored session.
     */
    void clearSession();

    // ===================== AI =====================

    /**
     * @brief Generate an audio preset using AI.
     *
     * @param prompt Text description of the desired sound
     * @return PatchCallResult containing generated parameters or error
     *
     * @warning Requires a valid, non-expired session.
     */
    PatchCallResult generatePatch(const juce::String& prompt);

    // ===================== CONFIG =====================

    /**
     * @brief Get backend API base URL.
     */
    const juce::String& getApiUrl() const;

    /**
     * @brief Get session file path.
     */
    const juce::File& getSessionFile() const;

    /**
     * @brief Write a message to the backend log file.
     *
     * @param message Message to log
     */
    void writeLog(const juce::String& message) const;

    // ===================== PROFILE =====================

    /**
     * @brief Retrieve user profile from backend.
     *
     * @return ProfileResult containing profile data or error
     */
    ProfileResult getProfile();

    /**
     * @brief Synchronize profile parameters asynchronously.
     *
     * @param session Current user session
     *
     * @thread Runs in a detached thread.
     */
    void syncProfileParamsInBackground(const UserSession& session);

    /**
     * @brief Synchronize profile parameters synchronously.
     *
     * @param session Current user session
     * @return Updated session or std::nullopt on failure
     */
    std::optional<UserSession> syncProfileParams(const UserSession& session);

    /**
     * @brief Update user theme.
     *
     * @param themeId Theme identifier
     */
    ProfileResult updateTheme(int themeId);

    /**
     * @brief Update user layout.
     *
     * @param layoutId Layout identifier
     */
    ProfileResult updateLayout(int layoutId);

private:
    /**
     * @brief Get executable directory.
     */
    static juce::File getExeDir();

    /**
     * @brief Load environment variables from .env file.
     *
     * @return Map of key-value pairs
     *
     * @note The file is expected one level above the executable directory.
     */
    static std::map<std::string, std::string> loadEnv();

    juce::String apiUrl;
    juce::File   sessionFile;

    BackendAuthManager* authManager = nullptr;
    BackendAiManager*  aiManager  = nullptr;
    BackendProfileManager* profileManager = nullptr;
};