#pragma once
#include <JuceHeader.h>
#include "BackendAuthManager.h"

class SignupPage : public juce::Component
{
public:
    SignupPage(BackendAuthManager& be, std::function<void(const UserSession&)> onSignupSuccess);
    void paint(juce::Graphics&) override;
    void resized() override;

    // Callback pour revenir à la page précédente
    std::function<void()> onBack;

private:
    BackendAuthManager& backend;
    std::function<void(const UserSession&)> onSuccess;

    // UI
    juce::Label titleLabel;
    juce::TextEditor usernameField, firstnameField, lastnameField, emailField, passwordField;
    juce::TextButton signupButton { "Create account" };
    juce::TextButton backButton { "← Back" };

    void handleSignup();
};
