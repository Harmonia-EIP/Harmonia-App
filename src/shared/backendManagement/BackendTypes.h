#pragma once

#include <juce_core/juce_core.h>
#include "../parameters/HarmoniaParams.h"
#include "../tools/PatchSerializer.h"

struct UserSession
{
    int userId = 0;
    
    juce::String pseudo;
    juce::String email;
    juce::String accessToken;
    juce::Time   expiresAt;

    int layoutId = 0;
    int themeId  = 0;
};

struct AuthResult
{
    bool success = false;
    UserSession session;
    juce::String errorMessage;
};

struct PatchCallResult
{
    bool success = false;
    juce::String errorMessage;
    PatchParams params;

    // ===== SUCCESS =====
    static PatchCallResult ok(const PatchParams& p)
    {
        return { true, {}, p };
    }

    // ===== ERROR =====
    static PatchCallResult error(const juce::String& message)
    {
        return { false, message, {} };
    }

    // ===== HELPERS =====
    bool hasError() const
    {
        return !success && errorMessage.isNotEmpty();
    }
};

struct UserProfile
{
    int         id = 0;
    std::string username;
    std::string email;
    std::string firstName;
    std::string lastName;
    std::string createdAt;
    std::string role;
    bool        isActive = true;

    int layoutId = 1;
    int themeId  = 1;
};

struct ProfileResult
{
    bool success = false;
    std::string errorMessage;
    UserProfile profile;

    static ProfileResult ok(const UserProfile& profile)
    {
        return { true, "", profile };
    }

    static ProfileResult error(const std::string& message)
    {
        return { false, message, {} };
    }
};