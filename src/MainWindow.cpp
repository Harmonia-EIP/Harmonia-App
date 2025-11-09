#include "MainWindow.h"
#include "MainComponent.h"

MainWindow::MainWindow(const juce::String& name,
                       juce::JUCEApplication& appRef,
                       BackendAuthManager& be,
                       std::optional<UserSession> existingSession)
    : juce::DocumentWindow(name,
                           juce::Desktop::getInstance().getDefaultLookAndFeel()
                               .findColour(juce::ResizableWindow::backgroundColourId),
                           juce::DocumentWindow::allButtons),
      app(appRef),
      backend(be)
{
    setUsingNativeTitleBar(true);

    if (existingSession.has_value())
        showMainScreen();
    else
        showWelcomeScreen();

    centreWithSize(900, 700);
    setResizable(true, true);
    setVisible(true);
}

void MainWindow::closeButtonPressed()
{
    app.systemRequestedQuit();
}

void MainWindow::showWelcomeScreen()
{
    auto* welcome = new WelcomePage();

    welcome->onChoice = [this](bool signup) {
        if (signup)
            showSignupScreen();
        else
            showLoginScreen();
    };

    currentComponent.reset(welcome);
    setContentOwned(currentComponent.get(), false);
}

void MainWindow::showLoginScreen()
{
    auto* login = new LoginPage(backend, [this](const UserSession& session) {
        showMainScreen();
    });

    login->onBack = [this]() {
        showWelcomeScreen();
    };

    currentComponent.reset(login);
    setContentOwned(currentComponent.get(), false);
}

void MainWindow::showSignupScreen()
{
    auto* signup = new SignupPage(backend, [this](const UserSession& session) {
        showMainScreen();
    });

    signup->onBack = [this]() {
        showWelcomeScreen();
    };

    currentComponent.reset(signup);
    setContentOwned(currentComponent.get(), false);
}

void MainWindow::showMainScreen()
{
    auto* main = new MainComponent(backend);
    main->getTitleComponent().onLogout = [this]() {
        showWelcomeScreen();
    };

    currentComponent.reset(main);
    setContentOwned(currentComponent.get(), false);
}
