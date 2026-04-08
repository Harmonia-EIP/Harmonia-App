#pragma once

#include "BackendTypes.h"

class BackendManager;

/**
 * @class BackendAuthManager
 * @brief Handles authentication and session persistence.
 *
 * @details
 * This class is responsible for communicating with backend authentication endpoints
 * and managing the user session lifecycle locally.
 *
 * Covered endpoints:
 * - POST /auth/signin
 * - POST /auth/signup
 *
 * Responsibilities:
 * - Authenticate users
 * - Register new accounts
 * - Persist session data to disk
 * - Load and clear sessions
 * - Synchronize profile data after authentication
 *
 * @note Uses BackendManager for API configuration and logging.
 */
class BackendAuthManager
{
public:
    /**
     * @brief Constructor.
     *
     * @param backend Reference to the main BackendManager
     */
    explicit BackendAuthManager(BackendManager& backend);

    /**
     * @brief Authenticate a user via the backend API.
     *
     * @param usernameOrEmail Username or email
     * @param password User password
     * @return AuthResult containing success state, session data, and error message
     *
     * @details
     * Sends a POST request to the authentication endpoint.
     * On success, the session is stored locally and profile sync may be triggered.
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
     *
     * @details
     * Sends a POST request to the signup endpoint.
     */
    AuthResult signupUser(const juce::String& username,
                          const juce::String& firstname,
                          const juce::String& lastname,
                          const juce::String& email,
                          const juce::String& password);

    /**
     * @brief Synchronize profile parameters asynchronously.
     *
     * @param session Current user session
     *
     * @details
     * Launches a detached thread to fetch and update user-related parameters.
     *
     * @thread Detached thread execution.
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
     * @brief Load session from disk.
     *
     * @return UserSession if available, otherwise std::nullopt
     */
    std::optional<UserSession> loadSession();

    /**
     * @brief Save session to disk.
     *
     * @param session Session to persist
     */
    void saveSession(const UserSession& session);

    /**
     * @brief Clear stored session.
     */
    void clearSession();

private:
    BackendManager& backend; ///< Reference to main backend manager
};