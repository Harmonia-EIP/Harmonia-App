#pragma once
#include <juce_core/juce_core.h>

namespace Strings
{
    namespace Errors
    {
        static const juce::String ErrorTitle = "Error";
        static const juce::String MissingPrompt = "Missing Prompt";
        static const juce::String MissingPromptAdvice = "Please enter a prompt before generating.";
        static const juce::String AiError       = "AI Error";
        static const juce::String UnknownError  = "An unknown error occurred.";
        static const juce::String MissingFields = "Missing fields";
        static const juce::String MissingFieldsAdvice = "Please fill in all fields.";
    }

    namespace Success
    {
        static const juce::String Welcome = "Welcome";
        static const juce::String PatchGenerated = "Patch generated successfully!";
        static const juce::String AccountCreated = "Account created successfully!";
        static const juce::String LoggedOut = "Logged out successfully!";
    }

    namespace Titles
    {
        static const juce::String CreateAccount = "Create your Harmonia account";
        static const juce::String SignIn = "Sign in to your account";
    }

    namespace Labels
    {
        static const juce::String Waveform = "Waveform";
        static const juce::String Filter = "Filter";
        static const juce::String Prompt = "Prompt";
        static const juce::String User = "User";
    }

    namespace Placeholders
    {
        static const juce::String identifier = "Email or username";
        static const juce::String Username  = "Username";
        static const juce::String FirstName = "First name";
        static const juce::String LastName  = "Last name";
        static const juce::String Email     = "Email";
        static const juce::String Password  = "Password";
        static const juce::juce_wchar PasswordChar = '*';
    }

    namespace Buttons
    {
        static const juce::String Export = "Exporter";
        static const juce::String Load   = "Charger";
        static const juce::String SignIn = "Sign in";
        static const juce::String SignUp = "Sign up";
        static const juce::String Logout = "Logout";
        static const juce::String Generate = "Generate";
        static const juce::String Back = "Go back";
        static const juce::String CreateAccount = "Create account";
        static const juce::String Clear = "Clear";
        static const juce::String Theme = "Theme";
        static const juce::String Layout = "Layout";
        static const juce::String ResetMissingParams = "Default if missing";
    }

    namespace Filter
    {
        static const juce::String LowPass  = "Low Pass";
        static const juce::String HighPass = "High Pass";
        static const juce::String BandPass = "Band Pass";
    }

    namespace Waveform
    {
        static const juce::String Sine     = "Sine";
        static const juce::String Square   = "Square";
        static const juce::String Saw      = "Saw";
        static const juce::String Triangle = "Triangle";
    }

    namespace Parameters
    {
        static const juce::String Frequency = "Frequency";
        static const juce::String Volume    = "Volume";
        static const juce::String Attack    = "Attack";
        static const juce::String Decay     = "Decay";
        static const juce::String Sustain   = "Sustain";
        static const juce::String Release   = "Release";
        static const juce::String Cutoff    = "Cutoff";
        static const juce::String Resonance = "Resonance";
    }

    namespace Themes
    {
        static const juce::String Light = "Light";
        static const juce::String Dark  = "Dark";
        static const juce::String Blue  = "Blue";
        static const juce::String Red   = "Red";
    }

    namespace Layouts
    {
        static const juce::String Layout1 = "Layout 1";
        static const juce::String Layout2 = "Layout 2";
        static const juce::String Layout3 = "Layout 3";
        static const juce::String Layout4 = "Layout 4";
    }
}
