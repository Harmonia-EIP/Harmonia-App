#pragma once

#include <juce_core/juce_core.h>

#include <map>

#include "../BackendTypes.h"   // apporte Strings::Errors::*

/**
 * Messages visibles par l'utilisateur ajoutés par le refacto.
 *
 * TODO : les déplacer dans config/Strings.h (je n'ai pas ce fichier) puis
 * supprimer celui-ci. Une seule langue (anglais, comme Strings::Errors).
 */
namespace BackendStrings
{
    inline const juce::String InvalidEmail          { "Please enter a valid email address." };
    inline const juce::String MissingField          { "A required field is missing." };
    inline const juce::String InvalidInput          { "Invalid input." };
    inline const juce::String InvalidServerResponse { "Invalid response from the server." };
    inline const juce::String ServerError           { "The server encountered an error. Please try again later." };
    inline const juce::String SessionExpired        { "Session expired" };
    inline const juce::String LoginRequired         { "Please sign in to use this feature." };
    inline const juce::String EmptyPrompt           { "Prompt is empty" };
    inline const juce::String InvalidCurrentPreset  { "Current preset is not valid JSON" };
    inline const juce::String InvalidPalette        { "The palette must contain exactly 3 colours (#RRGGBB)." };

    // Messages par code d'erreur backend
    inline const juce::String UserAlreadyExists     { "An account with this email already exists." };
    inline const juce::String UserNotFound          { "User not found." };
    inline const juce::String InvalidCredentials    { "Incorrect email or password." };
    inline const juce::String NotAllowed            { "This action is not allowed." };
    inline const juce::String NoPermission          { "You do not have permission to do this." };
    inline const juce::String ProfileNotFound       { "Profile not found." };
    inline const juce::String AiUnavailable         { "The AI service is unavailable. Please try again later." };
    inline const juce::String AiBadResponse         { "The AI service returned an invalid response." };
    inline const juce::String AiNotConfigured       { "The AI service is not configured." };

    inline juce::String passwordTooShort (int minLength)
    {
        return "Password must contain at least " + juce::String (minLength) + " characters.";
    }

    /** Message affichable pour un error_code backend. Chaîne vide si le code est inconnu. */
    inline juce::String forErrorCode (const juce::String& code)
    {
        static const std::map<juce::String, juce::String> table
        {
            { "MISSING_PARAM",         MissingField },
            { "USER_ALREADY_EXISTS",   UserAlreadyExists },
            { "USER_NOT_FOUND",        UserNotFound },
            { "INVALID_CREDENTIALS",   InvalidCredentials },
            { "INVALID_EMAIL",         InvalidEmail },
            { "TOKEN_MISSING",         SessionExpired },
            { "TOKEN_INVALID",         SessionExpired },
            { "TOKEN_EXPIRED",         SessionExpired },
            { "UNAUTHORIZED",          NotAllowed },
            { "NO_PERMISSION",         NoPermission },
            { "PROFILE_NOT_FOUND",     ProfileNotFound },
            { "VALIDATION_ERROR",      InvalidInput },
            { "AI_URL_NOT_CONFIGURED", AiNotConfigured },
            { "AI_NETWORK_ERROR",      AiUnavailable },
            { "AI_BAD_STATUS",         AiUnavailable },
            { "AI_INVALID_JSON",       AiBadResponse },
            { "AI_INVALID_RESPONSE",   AiBadResponse },
            { "NO_ROLE_SEEDS",         ServerError },
        };

        const auto it = table.find (code);
        return it != table.end() ? it->second : juce::String();
    }
}