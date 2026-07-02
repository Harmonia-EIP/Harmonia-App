#pragma once
#include <juce_core/juce_core.h>

namespace Strings
{
    namespace Errors
    {
        inline const juce::String ErrorTitle = "Error";
        inline const juce::String MissingPrompt = "Missing Prompt";
        inline const juce::String MissingPromptAdvice = "Please enter a prompt before generating.";
        inline const juce::String AiError       = "AI Error";
        inline const juce::String UnknownError  = "An unknown error occurred.";
        inline const juce::String MissingFields = "Missing fields";
        inline const juce::String MissingFieldsAdvice = "Please fill in all fields.";
        inline const juce::String EmptyPrompt = "Can't generate patch from an empty prompt.";
        inline const juce::String NoUserConnected = "No user connected.";
        inline const juce::String SessionExpired = "Session expired";
        inline const juce::String ErrorHTTP = "HTTP error";
        inline const juce::String UnreadableAIResponse = "Unreadable AI response.";
        inline const juce::String ThemeUpdateFailed = "Failed to update theme.";
        inline const juce::String LayoutUpdateFailed = "Failed to update layout.";
        inline const juce::String AiGuestError = "AI generation is not available in guest mode.";
        inline const juce::String AiGuestAdvice = "Please create an account or sign in to use the AI generation feature.";
        inline const juce::String AiServerError = "The AI service is temporarily unavailable.";
        inline const juce::String PleaseTryAgainLater = "Please try again later.";
        inline const juce::String AuthenticationError = "Authentication error";
        inline const juce::String NetworkError = "Network connection failed";
        inline const juce::String NetworkErrorAdvice = "Check your internet connection and try again.";
    }

    namespace Success
    {
        inline const juce::String Welcome = "Welcome";
        inline const juce::String PatchGenerated = "Patch generated successfully!";
        inline const juce::String AccountCreated = "Account created successfully!";
        inline const juce::String LoggedOut = "Logged out successfully!";
    }

    namespace Titles
    {
        inline const juce::String CreateAccount = "Create your Harmonia account";
        inline const juce::String SignIn = "Sign in to your account";
        inline const juce::String Subtitle = "AI SUBTRACTIVE SYNTHESIZER  -  CHARTER V1";
        inline const juce::String HarmoniaAiTitle = "HARMONIA AI SYNTH";
        inline const juce::String Harmonia = "Harmonia";
    }

    namespace Labels
    {
        inline const juce::String Waveform = "Waveform";
        inline const juce::String Filter = "Filter";
        inline const juce::String Prompt = "Prompt";
        inline const juce::String User = "User";
        inline const juce::String UnsetPreset = "No preset loaded";
        inline const juce::String GeneratingPreset = "Generating preset...";
        inline const juce::String GuestMode = "Guest";
    }

    namespace Placeholders
    {
        inline const juce::String identifier = "Email or username";
        inline const juce::String Username  = "Username";
        inline const juce::String FirstName = "First name";
        inline const juce::String LastName  = "Last name";
        inline const juce::String Email     = "Email";
        inline const juce::String Password  = "Password";
        inline const juce::juce_wchar PasswordChar = '*';
        inline const juce::String Prompt = "Describe a sound and press GENERATE...";
    }

    namespace Buttons
    {
        inline const juce::String Export = "Export";
        inline const juce::String Load   = "Load";
        inline const juce::String SignIn = "Sign in";
        inline const juce::String SignUp = "Sign up";
        inline const juce::String Logout = "Logout";
        inline const juce::String Generate = "Generate";
        inline const juce::String Back = "Go back";
        inline const juce::String CreateAccount = "Create account";
        inline const juce::String Clear = "Clear";
        inline const juce::String Theme = "Theme";
        inline const juce::String Layout = "Layout";
        inline const juce::String ResetMissingParams = "Default if missing";
        inline const juce::String GuestMode = "Continue as Guest";
    }

    namespace Filter
    {
        inline const juce::String LowPass  = "Low Pass";
        inline const juce::String HighPass = "High Pass";
        inline const juce::String BandPass = "Band Pass";
    }

    namespace Waveform
    {
        inline const juce::String Sine     = "Sine";
        inline const juce::String Square   = "Square";
        inline const juce::String Saw      = "Saw";
        inline const juce::String Triangle = "Triangle";
    }

    namespace Parameters
    {
        inline const juce::String Frequency = "Frequency";
        inline const juce::String Volume    = "Volume";
        inline const juce::String Attack    = "Attack";
        inline const juce::String Decay     = "Decay";
        inline const juce::String Sustain   = "Sustain";
        inline const juce::String Release   = "Release";
        inline const juce::String Cutoff    = "Cutoff";
        inline const juce::String Resonance = "Resonance";
    }

    namespace Themes
    {
        inline const juce::String Light = "Light";
        inline const juce::String Dark  = "Dark";
        inline const juce::String Blue  = "Blue";
        inline const juce::String Red   = "Red";
    }

    namespace Layouts
    {
        inline const juce::String Layout1 = "Layout 1";
        inline const juce::String Layout2 = "Layout 2";
        inline const juce::String Layout3 = "Layout 3";
        inline const juce::String Layout4 = "Layout 4";
    }

    namespace Sections
    {
        inline const juce::String Osc1    = "Osc 1 / Mix";
        inline const juce::String Filter  = "Filter";
        inline const juce::String Display = "Display";
        inline const juce::String Lfo     = "LFO";
        inline const juce::String Osc2    = "Osc 2";
        inline const juce::String Fx      = "Effects";
        inline const juce::String AmpEnv  = "Amp Envelope";
    }

    namespace Knobs
    {
        inline const juce::String Mix      = "Mix";
        inline const juce::String Noise    = "Noise";
        inline const juce::String Cutoff   = "Cutoff";
        inline const juce::String Reso     = "Reso";
        inline const juce::String FEnvAmt  = "F.Env Amt";
        inline const juce::String FEnvDec  = "F.Env Dec";
        inline const juce::String VelFlt   = "Vel >Flt";
        inline const juce::String Rate     = "Rate";
        inline const juce::String ToPitch  = "to Pitch";
        inline const juce::String ToCutoff = "to Cutoff";
        inline const juce::String Detune   = "Detune";
        inline const juce::String Drive    = "Drive";
        inline const juce::String Reverb   = "Reverb";
    }
}
