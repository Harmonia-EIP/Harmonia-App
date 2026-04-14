#pragma once
#include "JuceHeader.h"
#include "backendManagement/BackendManager.h"
#include "MainComponent.h"
#include "pages/WelcomePage.h"
#include "pages/LoginPage.h"
#include "pages/SignupPage.h"

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(const juce::String& name,
               juce::JUCEApplication& appRef,
               BackendManager& be,
               std::optional<UserSession> existingSession = std::nullopt);

    void closeButtonPressed() override;

private:
    juce::JUCEApplication& app;
    BackendManager& backend;

    std::unique_ptr<juce::Component> currentComponent;

    std::optional<UserSession> currentSession;

    void showWelcomeScreen();
    void showLoginScreen();
    void showSignupScreen();
    void showMainScreen(const UserSession& session);
};
