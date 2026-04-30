#include "LoginPage.h"

LoginPage::LoginPage(BackendManager& be, std::function<void(const UserSession&)> onSuccessCallback)
    : backend(be), onSuccess(onSuccessCallback)
{
    authLookAndFeel.setThemePreset(AppLookAndFeel::ThemePreset::Dark);
    setLookAndFeel(&authLookAndFeel);

    titleLabel.setText(Strings::Titles::SignIn, juce::dontSendNotification);
    titleLabel.setFont(UIStyle::Fonts::SubTitle());
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    auto placeholder = findColour(AppColourIds::textSecondaryId);

    identifierField.setTextToShowWhenEmpty(Strings::Placeholders::identifier, placeholder);
    passwordField.setTextToShowWhenEmpty(Strings::Placeholders::Password, placeholder);
    passwordField.setPasswordCharacter(Strings::Placeholders::PasswordChar);

    addAndMakeVisible(identifierField);
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
    identifierField.setTextToShowWhenEmpty(Strings::Placeholders::identifier, placeholder);
    passwordField.setTextToShowWhenEmpty(Strings::Placeholders::Password, placeholder);
}

void LoginPage::resized()
{
    auto area = getLocalBounds().reduced(150);

    titleLabel.setBounds(area.removeFromTop(60));

    identifierField.setBounds(area.removeFromTop(40));
    area.removeFromTop(10);

    passwordField.setBounds(area.removeFromTop(40));
    area.removeFromTop(20);

    loginButton.setBounds(area.removeFromTop(40));
    area.removeFromTop(10);

    backButton.setBounds(area.removeFromTop(40));
}

void LoginPage::handleLogin()
{
    auto identifier = identifierField.getText().trim();
    auto password = passwordField.getText().trim();

    if (identifier.isEmpty() || password.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            Strings::Errors::MissingFields,
            Strings::Errors::MissingFieldsAdvice
        );
        return;
    }

    auto result = backend.loginUser(identifier, password);

    if (!result.success)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            Strings::Errors::ErrorTitle,
            result.errorMessage
        );
        return;
    }

    onSuccess(result.session);
}
