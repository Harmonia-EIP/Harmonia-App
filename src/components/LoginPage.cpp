#include "LoginPage.h"

LoginPage::LoginPage(SupabaseManager& sb, std::function<void(const UserSession&)> onSuccessCallback)
    : supabase(sb), onSuccess(onSuccessCallback)
{
    titleLabel.setText("Sign in to Harmonia", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    emailField.setTextToShowWhenEmpty("Email", juce::Colours::grey);
    passwordField.setTextToShowWhenEmpty("Password", juce::Colours::grey);
    passwordField.setPasswordCharacter('*');
    addAndMakeVisible(emailField);
    addAndMakeVisible(passwordField);

    addAndMakeVisible(loginButton);
    addAndMakeVisible(backButton);

    loginButton.onClick = [this]() { handleLogin(); };
    backButton.onClick = [this]() { if (onBack) onBack(); };
}

void LoginPage::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkslategrey);
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
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                               "Champs vides",
                                               "Veuillez remplir tous les champs.");
        return;
    }

    auto session = supabase.loginUser(email, password);
    if (session.has_value())
    {
        supabase.saveSession(*session);
        onSuccess(*session);
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                               "Erreur",
                                               "Échec de connexion.");
    }
}
