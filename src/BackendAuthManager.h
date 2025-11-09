#pragma once

#include <juce_core/juce_core.h>
#include <optional>
#include <string>

struct UserSession
{
    juce::String pseudo;
    juce::String email;
    juce::String accessToken;
    juce::Time expiresAt;
};

struct AuthResult {
    bool success;
    UserSession session;
    juce::String errorMessage;
};

class BackendAuthManager
{
public:
    BackendAuthManager();

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

private:
    juce::File sessionFile;

    juce::String API_URL;
};
