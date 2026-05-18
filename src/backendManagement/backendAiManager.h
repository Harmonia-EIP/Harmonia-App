/**
 * @file BackendAiManager.h
 * @brief Declaration of the BackendAiManager class.
 *
 * This class handles all AI-related backend interactions.
 * It is responsible for sending user prompts to the backend API
 * and retrieving generated preset data.
 *
 * Main responsibilities:
 * - Validate user prompts
 * - Validate user sessions
 * - Communicate with AI backend endpoints
 * - Return AI-generated preset JSON responses
 */

#pragma once

#include <juce_core/juce_core.h>
#include "BackendTypes.h"

class BackendManager;

/**
 * @class BackendAiManager
 * @brief Handles AI preset generation through the backend API.
 *
 * BackendAiManager acts as a bridge between the application
 * and the backend AI services.
 *
 * It uses:
 * - CPR for HTTP requests
 * - JSON payload serialization
 * - BackendManager for session and configuration access
 *
 * The generated preset is returned as raw JSON,
 * which can later be parsed by the preset loader.
 */
class BackendAiManager
{
public:

    /**
     * @brief Creates a BackendAiManager instance.
     *
     * @param backend Reference to the main BackendManager.
     *
     * The BackendManager is used to:
     * - Access user sessions
     * - Retrieve API configuration
     * - Centralize backend communication
     */
    explicit BackendAiManager(BackendManager& backend);

    /**
     * @brief Sends a prompt to the backend AI service.
     *
     * This method:
     * - Validates the prompt
     * - Checks whether a user session exists
     * - Verifies session expiration
     * - Sends an authenticated HTTP POST request
     * - Returns the generated preset JSON
     *
     * Backend endpoint:
     * POST /ai/generate-preset
     *
     * Request body format:
     * @code
     * {
     *     "prompt": "user input"
     * }
     * @endcode
     *
     * @param prompt User text prompt describing the preset to generate.
     *
     * @return AiResult
     * - success = true:
     *      contains generated preset JSON
     * - success = false:
     *      contains an error message
     *
     * Possible errors:
     * - Empty prompt
     * - No active session
     * - Expired session
     * - HTTP request failure
     * - Empty backend response
     */
    AiResult generatePreset(const juce::String& prompt);

private:

    /**
     * @brief Reference to the main backend manager.
     *
     * Used to:
     * - Access session information
     * - Retrieve API configuration
     * - Centralize backend services
     */
    BackendManager& backend;
};