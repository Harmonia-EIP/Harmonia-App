#pragma once

#include "BackendTypes.h"

class BackendManager;

class BackendAuthManager
{
public:
    explicit BackendAuthManager(BackendManager& backend);

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
    BackendManager& backend;
};
