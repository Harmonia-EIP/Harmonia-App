
/**
 * @file BackendTypes.h
 * @brief Defines backend-related data structures.
 *
 * This file contains all shared backend structures used
 * across the application.
 *
 * Included structures:
 * - UserSession
 * - AuthResult
 * - UserProfile
 * - ProfileResult
 * - AiResult
 */
#pragma once

#include <nlohmann/json.hpp>
#include "../config/String.h"
#include "../config/AppConfig.h"
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

struct AiResult
{
    bool success = false;
    juce::String json;          
    juce::String errorMessage;
    
    static AiResult ok(const juce::String& json)
    {
        return { true, json, {} };
    }

    static AiResult error(const juce::String& message)
    {
        return { false, {}, message };
    }
};