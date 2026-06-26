#include "AppController.h"

AppController::AppController(HarmoniaAudioProcessor& p) : processor(p)
{
    auto session = backend.loadSession();

    if (session && session->expiresAt > juce::Time::getCurrentTime())
    {
        currentSession = *session;
        showMainScreen(*session);

        juce::Component::SafePointer<AppController> safeThis(this);

        juce::Thread::launch([safeThis, session]
        {
            if (safeThis == nullptr)
                return;

            auto synced = safeThis->backend.syncProfileParams(*session);

            if (safeThis == nullptr)
                return;

            if (synced)
            {
                juce::MessageManager::callAsync([safeThis, synced]
                {
                    if (safeThis == nullptr)
                        return;

                    safeThis->currentSession = *synced;
                });
            }
            else
            {
                if (safeThis != nullptr)
                    safeThis->backend.clearSession();
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

    welcome->onChoice = [this](WelcomePage::Choice choice)
    {
        switch (choice)
        {
            case WelcomePage::Choice::SignIn:
                showLoginScreen();
                break;

            case WelcomePage::Choice::SignUp:
                showSignupScreen();
                break;

            case WelcomePage::Choice::Guest:
            {
                UserSession guest;

                guest.isGuest = true;
                guest.userId = 0;
                guest.pseudo = Strings::Labels::GuestMode;
                guest.themeId = 0;
                guest.layoutId = 0;

                currentSession = guest;

                backend.writeLog("Guest mode activated");
                backend.saveSession(guest);
                showMainScreen(guest);

                break;
            }
        }
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
        currentSession.reset();
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
            backend.writeLog("Signup successful for user: " + session.themeId);
            currentSession = session;
            showMainScreen(session);
        });

    signup->onBack = [this]()
    {
        currentSession.reset();
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