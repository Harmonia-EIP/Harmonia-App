#include "LoginPage.h"

LoginPage::LoginPage(BackendManager& be, std::function<void(const UserSession&)> onSuccessCallback)
    : backend(be), onSuccess(onSuccessCallback)
{
    // LookAndFeel local toujours sombre (pas dépendant de l'utilisateur)
    authLookAndFeel.setThemePreset(AppLookAndFeel::ThemePreset::Dark);
    setLookAndFeel(&authLookAndFeel);

    titleLabel.setText("Sign in to Harmonia", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    auto placeholder = findColour(AppColourIds::textSecondaryId);

    emailField.setTextToShowWhenEmpty("Email", placeholder);
    passwordField.setTextToShowWhenEmpty("Password", placeholder);
    passwordField.setPasswordCharacter('*');

    addAndMakeVisible(emailField);
    addAndMakeVisible(passwordField);

    addAndMakeVisible(loginButton);
    addAndMakeVisible(backButton);

    loginButton.onClick = [this]() { handleLogin(); };
    backButton.onClick = [this]()
    {
        if (onBack) onBack();
    };
}

void LoginPage::paint(juce::Graphics& g)
{
    g.fillAll(findColour(AppColourIds::backgroundId));

    titleLabel.setColour(juce::Label::textColourId,
                         findColour(AppColourIds::textPrimaryId));

    auto placeholder = findColour(AppColourIds::textSecondaryId);
    emailField.setTextToShowWhenEmpty("Email", placeholder);
    passwordField.setTextToShowWhenEmpty("Password", placeholder);
}

void LoginPage::resized()
{
    auto area = getLocalBounds().reduced(150);

    titleLabel.setBounds(area.removeFromTop(60));

    emailField.setBounds(area.removeFromTop(40));
    area.removeFromTop(10);

    passwordField.setBounds(area.removeFromTop(40));
    area.removeFromTop(20);

    loginButton.setBounds(area.removeFromTop(40));
    area.removeFromTop(10);

    backButton.setBounds(area.removeFromTop(40));
}

void LoginPage::handleLogin()
{
    auto email = emailField.getText().trim();
    auto password = passwordField.getText().trim();

    if (email.isEmpty() || password.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Champs vides",
            "Veuillez remplir tous les champs."
        );
        return;
    }

    auto result = backend.loginUser(email, password);

    if (!result.success)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Erreur",
            result.errorMessage
        );
        return;
    }

    onSuccess(result.session);
}
