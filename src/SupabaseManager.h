#pragma once
#include <JuceHeader.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include <optional>

struct UserSession
{
    juce::String pseudo;
    juce::String email;
    juce::String accessToken;
    juce::String refreshToken;
    juce::Time expiresAt;
};

class SupabaseManager
{
public:
    SupabaseManager();

    
    std::optional<UserSession> loginUser(const juce::String& email, const juce::String& password);
    std::optional<UserSession> signupUser(const juce::String& email,
                                      const juce::String& password,
                                      const juce::String& username,
                                      const juce::String& firstname,
                                      const juce::String& lastname);

    std::optional<UserSession> refreshSession(const juce::String& refreshToken);

    
    void saveSession(const UserSession& session);
    std::optional<UserSession> loadSession();
    void clearSession();

private:
    juce::File sessionFile;
    const juce::String SUPABASE_URL;
    const juce::String SUPABASE_KEY;

    std::optional<UserSession> parseSession(const nlohmann::json& json);
};
