#include "AppController.h"

AppController::AppController(HarmoniaAudioProcessor& p) : processor(p)
{
    auto session = backend.loadSession();

    if (session && session->expiresAt > juce::Time::getCurrentTime())
    {
        currentSession = *session;
        showMainScreen(*session);

        juce::Thread::launch([this, session]
        {
            auto synced = backend.syncProfileParams(*session);
            if (synced)
            {
                juce::MessageManager::callAsync([this, synced]
                {
                    currentSession = *synced;
                });
            }
            else
            {
                backend.clearSession();
            }
        });
        return;
    }

    showWelcomeScreen();
}


void AppController::resized()
{
    if (currentComponent)
        currentComponent->setBounds(getLocalBounds());
}


void AppController::showWelcomeScreen()
{
    auto welcome = std::make_unique<WelcomePage>();

    welcome->onChoice = [this](bool signup)
    {
        if (signup)
            showSignupScreen();
        else
            showLoginScreen();
    };

    currentComponent = std::move(welcome);
    addAndMakeVisible(currentComponent.get());
    resized();
}


void AppController::showLoginScreen()
{
    auto login = std::make_unique<LoginPage>(backend,
        [this](const UserSession& session)
        {
            currentSession = session;
            showMainScreen(session);
        });

    login->onBack = [this]()
    {
        showWelcomeScreen();
    };

    currentComponent = std::move(login);
    addAndMakeVisible(currentComponent.get());
    resized();
}


void AppController::showSignupScreen()
{
    auto signup = std::make_unique<SignupPage>(backend,
        [this](const UserSession& session)
        {
            currentSession = session;
            showMainScreen(session);
        });

    signup->onBack = [this]()
    {
        showWelcomeScreen();
    };

    currentComponent = std::move(signup);
    addAndMakeVisible(currentComponent.get());
    resized();
}


void AppController::showMainScreen(const UserSession& session)
{
    auto main = std::make_unique<MainComponent>(processor, backend, session);

    main->onLogout = [this]()
    {
        currentSession.reset();
        showWelcomeScreen();
    };

    currentComponent = std::move(main);
    addAndMakeVisible(currentComponent.get());
    resized();
}