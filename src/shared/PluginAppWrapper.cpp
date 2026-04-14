#include "PluginAppWrapper.h"

PluginAppWrapper::PluginAppWrapper()
{
    auto session = backend.loadSession();

    if (session && session->expiresAt > juce::Time::getCurrentTime())
    {
        auto synced = backend.syncProfileParams(*session);

        if (synced)
        {
            currentSession = synced;
            showMainScreen(*synced);
            return;
        }
        else
        {
            backend.clearSession();
        }
    }

    showWelcomeScreen();
}

//================================================

void PluginAppWrapper::resized()
{
    if (currentComponent)
        currentComponent->setBounds(getLocalBounds());
}

//================================================

void PluginAppWrapper::showWelcomeScreen()
{
    auto* welcome = new WelcomePage();

    welcome->onChoice = [this](bool signup)
    {
        if (signup)
            showSignupScreen();
        else
            showLoginScreen();
    };

    currentComponent.reset(welcome);
    addAndMakeVisible(currentComponent.get());
    resized();
}

//================================================

void PluginAppWrapper::showLoginScreen()
{
    auto* login = new LoginPage(backend,
        [this](const UserSession& session)
        {
            currentSession = session;
            showMainScreen(session);
        });

    login->onBack = [this]()
    {
        showWelcomeScreen();
    };

    currentComponent.reset(login);
    addAndMakeVisible(currentComponent.get());
    resized();
}

//================================================

void PluginAppWrapper::showSignupScreen()
{
    auto* signup = new SignupPage(backend,
        [this](const UserSession& session)
        {
            currentSession = session;
            showMainScreen(session);
        });

    signup->onBack = [this]()
    {
        showWelcomeScreen();
    };

    currentComponent.reset(signup);
    addAndMakeVisible(currentComponent.get());
    resized();
}

//================================================

void PluginAppWrapper::showMainScreen(const UserSession& session)
{
    auto* main = new MainComponent(backend, session);

    main->getTitleComponent().onLogout = [this]()
    {
        backend.clearSession();
        showWelcomeScreen();
    };

    currentComponent.reset(main);
    addAndMakeVisible(currentComponent.get());
    resized();
}