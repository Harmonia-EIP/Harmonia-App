#pragma once

#include <juce_core/juce_core.h>
#include <map>
#include <string>
#include <optional>

#include "BackendTypes.h"

class BackendAuthManager;
class BackendAiManager;

class BackendManager
{
public:
    BackendManager();
    ~BackendManager();

    // Auth façade
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

    // IA / backend API façade
    PatchCallResult generatePatch(const juce::String& prompt);

    // Infos backend
    const juce::String& getApiUrl() const      { return apiUrl; }
    const juce::File&   getSessionFile() const { return sessionFile; }

    void writeLog(const juce::String& message) const;

private:
    static juce::File getExeDir();
    static std::map<std::string, std::string> loadEnv();

    juce::String apiUrl;
    juce::File   sessionFile;

    BackendAuthManager* authManager = nullptr;
    BackendAiManager*  aiManager  = nullptr;

    friend class BackendAuthManager;
    friend class BackendAiManager;
};
