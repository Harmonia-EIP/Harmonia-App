#include "AppController.h"

AppController::AppController(HarmoniaAudioProcessor& p) : processor(p)
{
    auto session = backend.loadSession();

    if (session && session->expiresAt > juce::Time::getCurrentTime())
    {
        currentSession = *session;
        showMainScreen(*session);

        juce::Component::SafePointer<AppController> safeThis(this);

        // Captured by value (not `this`/`backend`): the network request below
        // can take seconds, and the user may close the plugin window (which
        // destroys this AppController and its BackendManager) well before it
        // returns. `safeThis` is only ever checked-then-used on the message
        // thread via callAsync, where the check and the use happen
        // atomically - checking it from this background thread and then
        // dereferencing `backend` afterwards would be a TOCTOU race.
        const auto apiUrl      = backend.getApiUrl();
        const auto sessionFile = backend.getSessionFile();

        juce::Thread::launch([safeThis, session, apiUrl, sessionFile]
        {
            auto synced = BackendAuthManager::syncProfileParamsFromServer(apiUrl, sessionFile, *session);

            juce::MessageManager::callAsync([safeThis, synced]
            {
                if (safeThis == nullptr)
                    return;

                if (synced)
                    safeThis->currentSession = *synced;
                else
                    safeThis->backend.clearSession();
            });
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