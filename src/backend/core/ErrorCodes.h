#pragma once

#include <juce_core/juce_core.h>

namespace ErrorCodes
{
    inline const juce::String MissingParam       { "MISSING_PARAM" };
    inline const juce::String UserAlreadyExists  { "USER_ALREADY_EXISTS" };
    inline const juce::String UserNotFound       { "USER_NOT_FOUND" };
    inline const juce::String InvalidCredentials { "INVALID_CREDENTIALS" };
    inline const juce::String InvalidEmail       { "INVALID_EMAIL" };
    inline const juce::String TokenMissing       { "TOKEN_MISSING" };
    inline const juce::String TokenInvalid       { "TOKEN_INVALID" };
    inline const juce::String TokenExpired       { "TOKEN_EXPIRED" };
    inline const juce::String Unauthorized       { "UNAUTHORIZED" };
    inline const juce::String ProfileNotFound    { "PROFILE_NOT_FOUND" };
    inline const juce::String NoPermission       { "NO_PERMISSION" };
    inline const juce::String ValidationError    { "VALIDATION_ERROR" };

    inline bool isTokenError (const juce::String& code)
    {
        return code == TokenMissing || code == TokenInvalid || code == TokenExpired;
    }
}