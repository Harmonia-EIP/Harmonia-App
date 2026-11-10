#include "AppController.h"

// `backend` est une RÉFÉRENCE vers le BackendManager du processor
// (voir AppController.h : `BackendManager& backend;`, déclaré après `processor`).
AppController::AppController(HarmoniaAudioProcessor& p)
    : processor(p), backend(p.getBackend())
{
    // Écrits ici (ouverture de l'éditeur) et plus dans le constructeur du backend :
    // un DAW instancie le plugin sans éditeur pendant le scan, et ça ne doit pas toucher au disque.
    backend.writeLog("API_URL = " + backend.getApiUrl());
    backend.writeLog("SESSION PATH = " + backend.getSessionFile().getFullPathName());

    auto session = backend.loadSession();

    if (session && session->expiresAt > juce::Time::getCurrentTime())
    {
        currentSession = *session;

        // Le sync du profil (palette + historique) est lancé par MainComponent
        // (refreshProfileAsync), qui reçoit le résultat et le distribue au header.
        showMainScreen(*session);
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

                // Invité : pas de palette backend, le header utilise les couleurs par défaut
                guest.paletteColours.clear();
                guest.paletteSlot = 0;

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
            backend.writeLog("Signup successful for user: " + session.pseudo);
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