#include "SignupPage.h"
#include "../themes/AppColourIds.h"

SignupPage::SignupPage(BackendManager& be, std::function<void(const UserSession&)> onSignupSuccess)
    : backend(be), onSuccess(onSignupSuccess)
{
    // LookAndFeel local toujours sombre (pas dépendant de l'utilisateur)
    authLookAndFeel.setThemePreset(AppLookAndFeel::ThemePreset::Dark);
    setLookAndFeel(&authLookAndFeel);

    titleLabel.setText("Create your Harmonia account", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    auto placeholder = findColour(AppColourIds::textSecondaryId);

    usernameField.setTextToShowWhenEmpty("Username", placeholder);
    firstnameField.setTextToShowWhenEmpty("First name", placeholder);
    lastnameField.setTextToShowWhenEmpty("Last name", placeholder);
    emailField.setTextToShowWhenEmpty("Email", placeholder);
    passwordField.setTextToShowWhenEmpty("Password", placeholder);
    passwordField.setPasswordCharacter('*');

    for (auto* f : { &usernameField, &firstnameField, &lastnameField, &emailField, &passwordField })
        addAndMakeVisible(*f);

    signupButton.setButtonText("Create account");
    signupButton.onClick = [this]() { handleSignup(); };
    addAndMakeVisible(signupButton);

    backButton.setButtonText("Go Back");
    backButton.onClick = [this]()
    {
        if (onBack) onBack();
    };
    addAndMakeVisible(backButton);
}

void SignupPage::paint(juce::Graphics& g)
{
    g.fillAll(findColour(AppColourIds::backgroundId));

    titleLabel.setColour(juce::Label::textColourId,
                         findColour(AppColourIds::textPrimaryId));

    // Placeholder recalé (utile si un jour vous changez la palette auth)
    auto placeholder = findColour(AppColourIds::textSecondaryId);
    usernameField.setTextToShowWhenEmpty("Username", placeholder);
    firstnameField.setTextToShowWhenEmpty("First name", placeholder);
    lastnameField.setTextToShowWhenEmpty("Last name", placeholder);
    emailField.setTextToShowWhenEmpty("Email", placeholder);
    passwordField.setTextToShowWhenEmpty("Password", placeholder);
}

void SignupPage::resized()
{
    auto area = getLocalBounds().reduced(100);

    auto totalHeight = 60 + 20 + (5 * 35 + 4 * 10) + 30 + 40 + 10 + 30;
    area = area.withTrimmedTop((area.getHeight() - totalHeight) / 2);

    titleLabel.setBounds(area.removeFromTop(60));
    area.removeFromTop(20);

    int h = 35;
    int space = 10;

    usernameField.setBounds(area.removeFromTop(h));
    area.removeFromTop(space);
    firstnameField.setBounds(area.removeFromTop(h));
    area.removeFromTop(space);
    lastnameField.setBounds(area.removeFromTop(h));
    area.removeFromTop(space);
    emailField.setBounds(area.removeFromTop(h));
    area.removeFromTop(space);
    passwordField.setBounds(area.removeFromTop(h));

    area.removeFromTop(30);
    signupButton.setBounds(area.removeFromTop(40));
    area.removeFromTop(10);
    backButton.setBounds(area.removeFromTop(30));
}

void SignupPage::handleSignup()
{
    auto username = usernameField.getText().trim();
    auto firstname = firstnameField.getText().trim();
    auto lastname = lastnameField.getText().trim();
    auto email = emailField.getText().trim();
    auto password = passwordField.getText().trim();

    if (username.isEmpty() || firstname.isEmpty() || lastname.isEmpty() || email.isEmpty() || password.isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Missing fields",
            "Please fill in all fields."
        );
        return;
    }

    auto result = backend.signupUser(username, firstname, lastname, email, password);

    if (!result.success)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            "Erreur",
            result.errorMessage
        );
        return;
    }

    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        "Welcome",
        "Account created successfully!"
    );

    onSuccess(result.session);
}
