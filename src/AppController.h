#pragma once
#include "JuceHeader.h"

#include "backendManagement/BackendManager.h"
#include "pages/WelcomePage.h"
#include "pages/LoginPage.h"
#include "pages/SignupPage.h"
#include "MainComponent.h"

class AppController : public juce::Component
{
public:
    AppController(HarmoniaAudioProcessor& p);

    void resized() override;

private:
    HarmoniaAudioProcessor& processor;
    BackendManager backend;

    std::unique_ptr<juce::Component> currentComponent;
    std::optional<UserSession> currentSession;

    void showWelcomeScreen();
    void showLoginScreen();
    void showSignupScreen();
    void showMainScreen(const UserSession& session);
};