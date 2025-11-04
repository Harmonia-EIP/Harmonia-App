#pragma once
#include <JuceHeader.h>
#include "SupabaseManager.h"

class LoginPage : public juce::Component
{
public:
    LoginPage(SupabaseManager& sb, std::function<void(const UserSession&)> onSuccess);

    std::function<void()> onBack;

    void paint(juce::Graphics&) override;
    void resized() override;

private:
    SupabaseManager& supabase;
    std::function<void(const UserSession&)> onSuccess;

    juce::Label titleLabel;
    juce::TextEditor emailField, passwordField;
    juce::TextButton loginButton{"Sign in"}, backButton{"← Back"};

    void handleLogin();
};
