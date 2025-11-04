#pragma once
#include <JuceHeader.h>
#include "SupabaseManager.h"

class SignupPage : public juce::Component
{
public:
    SignupPage(SupabaseManager& sb, std::function<void(const UserSession&)> onSignupSuccess);
    void paint(juce::Graphics&) override;
    void resized() override;

    // Callback pour revenir à la page précédente
    std::function<void()> onBack;

private:
    SupabaseManager& supabase;
    std::function<void(const UserSession&)> onSuccess;

    // UI
    juce::Label titleLabel;
    juce::TextEditor usernameField, firstnameField, lastnameField, emailField, passwordField;
    juce::TextButton signupButton { "Create account" };
    juce::TextButton backButton { "← Back" };

    void handleSignup();
};
