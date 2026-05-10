#pragma once

#include <juce_core/juce_core.h>
#include <map>
#include <string>
#include <optional>

#include "BackendTypes.h"

class BackendAuthManager;
class BackendProfileManager;

// Facade for backend interactions (auth + profile).
// AI preset generation is now handled directly in the UI via JSON returned by
// the backend (charter format). See PresetLoader for the on-device parsing.
class BackendManager
{
public:
    BackendManager();
    ~BackendManager();

    // ===================== AUTH =====================
    AuthResult loginUser(const juce::String& usernameOrEmail,
                         const juce::String& password);

    AuthResult signupUser(const juce::String& username,
                          const juce::String& firstname,
                          const juce::String& lastname,
                          const juce::String& email,
                          const juce::String& password);

    std::optional<UserSession> loadSession();
    void saveSession(const UserSession& session);
    void clearSession();

    // ===================== AI =====================
    // Calls /ai/generate-preset and returns the raw JSON response (charter format)
    // so the UI can apply it via PresetLoader::loadFromJsonString. Empty on error.
    juce::String generatePresetJson(const juce::String& prompt, juce::String& errorOut);

    // ===================== CONFIG =====================
    const juce::String& getApiUrl() const;
    const juce::File& getSessionFile() const;
    void writeLog(const juce::String& message) const;

    // ===================== PROFILE =====================
    ProfileResult getProfile();

    void syncProfileParamsInBackground(const UserSession& session);
    std::optional<UserSession> syncProfileParams(const UserSession& session);

    ProfileResult updateTheme(int themeId);
    ProfileResult updateLayout(int layoutId);

private:
    juce::File getAppDataDir() const;
    juce::String loadConfig();

    juce::File logFile;

    juce::String apiUrl;
    juce::File   sessionFile;

    std::unique_ptr<BackendAuthManager>    authManager;
    std::unique_ptr<BackendProfileManager> profileManager;
};
