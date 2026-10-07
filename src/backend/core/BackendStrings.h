#pragma once

#include <juce_core/juce_core.h>

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

    inline juce::String passwordTooShort (int minLength)
    {
        return "Password must contain at least " + juce::String (minLength) + " characters.";
    }
}
