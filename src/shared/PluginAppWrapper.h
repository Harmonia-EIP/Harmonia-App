#pragma once

#include "JuceHeader.h"
#include <optional>

#include "backendManagement/BackendManager.h"
#include "MainComponent.h"
#include "pages/WelcomePage.h"
#include "pages/LoginPage.h"
#include "pages/SignupPage.h"

class PluginAppWrapper : public juce::Component
{
public:
    PluginAppWrapper();
    ~PluginAppWrapper() override = default;

    void resized() override;

private:
    BackendManager backend;
    std::optional<UserSession> currentSession;

    std::unique_ptr<juce::Component> currentComponent;

    void showWelcomeScreen();
    void showLoginScreen();
    void showSignupScreen();
    void showMainScreen(const UserSession& session);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (PluginAppWrapper)
};