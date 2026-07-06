#pragma once

#include "BackendTypes.h"

class BackendManager;

/**
 * @class BackendProfileManager
 * @brief Handles user profile retrieval and updates.
 *
 * @details
 * This class communicates with backend profile endpoints to manage
 * user-specific data such as themes and layouts.
 *
 * Endpoints:
 * - GET /profile/me
 * - PUT /profile/{id}/theme
 * - PUT /profile/{id}/layout
 *
 * Responsibilities:
 * - Fetch user profile
 * - Update UI-related preferences
 */
class BackendProfileManager
{
public:
    /**
     * @brief Constructor.
     *
     * @param backend Reference to the main BackendManager
     */
    BackendProfileManager(BackendManager& backend);

    /**
     * @brief Retrieve user profile from backend.
     *
     * @return ProfileResult containing profile data or error
     *
     * @details
     * Sends a GET request using the current session token.
     */
    ProfileResult getProfile();

    /**
     * @brief Update user theme.
     *
     * @param themeId Theme identifier
     * @return ProfileResult indicating success or failure
     */
    ProfileResult updateTheme(int themeId);

    /**
     * @brief Update user layout.
     *
     * @param layoutId Layout identifier
     * @return ProfileResult indicating success or failure
     */
    ProfileResult updateLayout(int layoutId);

    void updateLocalTheme(int themeId);

    void updateThemeAsync(int themeId);

private:
    BackendManager& backend; ///< Reference to main backend manager

    /**
     * @brief Stateless PUT /profile/{id}/theme implementation.
     *
     * @details
     * No dependency on a BackendManager/BackendProfileManager instance, so it
     * can safely run on a detached background thread after the owning
     * BackendManager (and the plugin editor it belongs to) may already have
     * been destroyed.
     */
    static ProfileResult putThemeToServer(const juce::String& apiUrl,
                                          const juce::File& sessionFile,
                                          int themeId);
};