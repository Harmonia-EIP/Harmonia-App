#pragma once

#include <string>
#include <vector>

#include "BackendTypes.h"

class BackendManager;

/**
 * @class BackendProfileManager
 * @brief Handles user profile retrieval and palette updates.
 *
 * @details
 * This class communicates with backend profile endpoints to manage
 * user-specific data such as the 3-colour palette.
 *
 * Endpoints:
 * - GET /profile/me
 * - PUT /profile/{id}/palette
 *
 * Responsibilities:
 * - Fetch user profile
 * - Synchronize the palette (3 colours + last selected slot)
 *
 * Palette sync strategy (limits API calls):
 * - Long debounce (2.5 s): only one PUT once the user stops changing things.
 * - "Unchanged" filter: no PUT if the palette/slot equals the last state
 *   known by the server (received at login, or last successful PUT).
 * - Non-blocking flush when the plugin window closes or on logout.
 * - The server is always the source of truth at startup.
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
     * @brief Update the palette synchronously (blocking).
     *
     * @param colours 3 hex colours "#RRGGBB"
     * @param slot    Selected slot, 0..2 (sent as last_color_id 1..3)
     * @return ProfileResult indicating success or failure
     */
    ProfileResult updatePalette(const juce::StringArray& colours, int slot);

    /**
     * @brief Update the palette in the background (long debounce).
     *
     * @details
     * The local session file is updated immediately (guests are ignored).
     * The PUT is sent only after 2.5 s without any new change, and only if
     * the state differs from the last one known by the server.
     *
     * @param colours 3 hex colours "#RRGGBB"
     * @param slot    Selected slot, 0..2
     */
    void updatePaletteAsync(const juce::StringArray& colours, int slot);

    /**
     * @brief Sends the pending palette change now, without blocking.
     *
     * @details
     * Cancels the debounce wait, then, if a change is pending and differs
     * from the server state, sends it on a detached thread with a short
     * timeout. Everything the thread needs is copied by value.
     * Call it BEFORE BackendManager::clearSession() on logout.
     */
    void flushPaletteIfPending();

    /**
     * @brief Sets the reference "server state" used by the unchanged filter.
     *
     * @details
     * Call it when a session starts (palette received from the backend).
     * Passing an empty array (guest) clears the reference.
     * Also drops any pending change.
     *
     * @param colours 3 hex colours "#RRGGBB", or empty
     * @param slot    Selected slot, 0..2
     */
    void resetPaletteSyncState(const juce::StringArray& colours, int slot);

private:
    BackendManager& backend; ///< Reference to main backend manager

    /**
     * @brief Stateless PUT /profile/{id}/palette implementation.
     *
     * @details
     * No dependency on a BackendManager/BackendProfileManager instance, so it
     * can safely run on a detached background thread after the owning
     * BackendManager (and the plugin editor it belongs to) may already have
     * been destroyed. It does not touch the session file either, so it can
     * never recreate a session cleared by a logout.
     *
     * @param timeoutMs Request timeout in milliseconds.
     */
    static ProfileResult putPaletteToServer(const juce::String& apiUrl,
                                            int userId,
                                            const std::string& token,
                                            const std::vector<std::string>& colours,
                                            int slot,
                                            int timeoutMs);
};