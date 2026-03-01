#pragma once

#include <juce_core/juce_core.h>

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

struct PatchParams
{
    juce::String waveform;
    juce::String filterType;

    double frequency  = 440.0;
    double volume     = 0.8;
    double attack     = 0.1;
    double decay      = 0.1;
    double sustain    = 0.8;
    double release    = 0.5;
    double cutoff     = 1000.0;
    double resonance  = 1.0;
};

struct PatchCallResult
{
    bool success = false;
    PatchParams params;
    juce::String errorMessage;
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
};