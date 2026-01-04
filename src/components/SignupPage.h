#pragma once
#include <JuceHeader.h>
#include "../backendManagement/BackendManager.h"

class SignupPage : public juce::Component
{
public:
    SignupPage(BackendManager& be, std::function<void(const UserSession&)> onSignupSuccess);
    void paint(juce::Graphics&) override;
    void resized() override;

    std::function<void()> onBack;

private:
    BackendManager& backend;
    std::function<void(const UserSession&)> onSuccess;

    juce::Label titleLabel;
    juce::TextEditor usernameField, firstnameField, lastnameField, emailField, passwordField;
    juce::TextButton signupButton;
    juce::TextButton backButton;

    void handleSignup();
};
