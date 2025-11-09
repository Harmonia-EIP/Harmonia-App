#pragma once
#include <JuceHeader.h>
#include "BackendAuthManager.h"
#include "MainComponent.h"
#include "components/WelcomePage.h"
#include "components/LoginPage.h"
#include "components/SignupPage.h"

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(const juce::String& name,
               juce::JUCEApplication& appRef,
               BackendAuthManager& be,
               std::optional<UserSession> existingSession = std::nullopt);

    void closeButtonPressed() override;

private:
    juce::JUCEApplication& app;
    BackendAuthManager& backend;

    std::unique_ptr<juce::Component> currentComponent;

    void showWelcomeScreen();
    void showLoginScreen();
    void showSignupScreen();
    void showMainScreen();
};
