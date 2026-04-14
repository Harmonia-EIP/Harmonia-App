#pragma once

#include "BackendTypes.h"

class BackendManager;

/**
 * @class BackendAiManager
 * @brief Handles AI-based preset generation via backend API.
 *
 * @details
 * This class communicates with the AI service to generate audio presets
 * based on a textual prompt.
 *
 * Endpoint:
 * - POST /ai/generate-preset
 *
 * Responsibilities:
 * - Send prompt to backend AI
 * - Parse JSON response
 * - Convert response into PatchParams
 *
 * @warning Requires a valid authenticated session (JWT token).
 */
class BackendAiManager
{
public:
    /**
     * @brief Constructor.
     *
     * @param backend Reference to the main BackendManager
     */
    explicit BackendAiManager(BackendManager& backend);

    /**
     * @brief Generate an audio preset from a text prompt.
     *
     * @param prompt Description of the desired sound
     * @return PatchCallResult containing generated parameters or error
     *
     * @details
     * Sends a request to the AI backend and parses the response into usable parameters.
     *
     * @warning Fails if:
     * - prompt is empty
     * - session is missing
     * - session is expired
     */
    PatchCallResult generatePatch(const juce::String& prompt);

private:
    BackendManager& backend; ///< Reference to main backend manager
};