#pragma once

#include <juce_core/juce_core.h>
#include <optional>
#include <string>
#include "BackendTypes.h"
#include <nlohmann/json.hpp>

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

private:
    BackendManager& backend; ///< Reference to main backend manager
};