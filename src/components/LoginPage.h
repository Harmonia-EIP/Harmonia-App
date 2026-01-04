#pragma once
#include <JuceHeader.h>
#include "../backendManagement/BackendManager.h"

class LoginPage : public juce::Component
{
public:
    LoginPage(BackendManager& be, std::function<void(const UserSession&)> onSuccess);

    std::function<void()> onBack;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    BackendManager& backend;
    std::function<void(const UserSession&)> onSuccess;

    juce::Label titleLabel;
    juce::TextEditor emailField, passwordField;
    juce::TextButton loginButton{"Sign in"}, backButton{"Go Back"};

    void handleLogin();
};
