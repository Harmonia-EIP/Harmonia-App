#pragma once
#include <JuceHeader.h>
#include "SupabaseManager.h"
#include "MainComponent.h"
#include "components/WelcomePage.h"
#include "components/LoginPage.h"
#include "components/SignupPage.h"

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(const juce::String& name,
               juce::JUCEApplication& appRef,
               SupabaseManager& sb,
               std::optional<UserSession> existingSession = std::nullopt);

    void closeButtonPressed() override;

private:
    juce::JUCEApplication& app;
    SupabaseManager& supabase;

    std::unique_ptr<juce::Component> currentComponent;

    void showWelcomeScreen();
    void showLoginScreen();
    void showSignupScreen();
    void showMainScreen();
};
