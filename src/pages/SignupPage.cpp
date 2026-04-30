#include "SignupPage.h"

SignupPage::SignupPage(BackendManager& be, std::function<void(const UserSession&)> onSignupSuccess)
    : backend(be), onSuccess(onSignupSuccess)
{
    authLookAndFeel.setThemePreset(AppLookAndFeel::ThemePreset::Dark);
    setLookAndFeel(&authLookAndFeel);

    titleLabel.setText(Strings::Titles::CreateAccount, juce::dontSendNotification);
    titleLabel.setFont(UIStyle::Fonts::SubTitle());
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    auto placeholder = findColour(AppColourIds::textSecondaryId);

    usernameField.setTextToShowWhenEmpty(Strings::Placeholders::Username, placeholder);
    firstnameField.setTextToShowWhenEmpty(Strings::Placeholders::FirstName, placeholder);
    lastnameField.setTextToShowWhenEmpty(Strings::Placeholders::LastName, placeholder);
    emailField.setTextToShowWhenEmpty(Strings::Placeholders::Email, placeholder);
    passwordField.setTextToShowWhenEmpty(Strings::Placeholders::Password, placeholder);
    passwordField.setPasswordCharacter(Strings::Placeholders::PasswordChar);

    for (auto* f : { &usernameField, &firstnameField, &lastnameField, &emailField, &passwordField })
        addAndMakeVisible(*f);

    signupButton.setButtonText(Strings::Buttons::CreateAccount);
    signupButton.onClick = [this]() { handleSignup(); };
    addAndMakeVisible(signupButton);

    backButton.setButtonText(Strings::Buttons::Back);
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

    auto placeholder = findColour(AppColourIds::textSecondaryId);
    usernameField.setTextToShowWhenEmpty(Strings::Placeholders::Username, placeholder);
    firstnameField.setTextToShowWhenEmpty(Strings::Placeholders::FirstName, placeholder);
    lastnameField.setTextToShowWhenEmpty(Strings::Placeholders::LastName, placeholder);
    emailField.setTextToShowWhenEmpty(Strings::Placeholders::Email, placeholder);
    passwordField.setTextToShowWhenEmpty(Strings::Placeholders::Password, placeholder);
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
            Strings::Errors::MissingFields,
            Strings::Errors::MissingFieldsAdvice
        );
        return;
    }

    auto result = backend.signupUser(username, firstname, lastname, email, password);

    if (!result.success)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            Strings::Errors::ErrorTitle,
            result.errorMessage
        );
        return;
    }

    juce::AlertWindow::showMessageBoxAsync(
        juce::AlertWindow::InfoIcon,
        Strings::Success::Welcome,
        Strings::Success::AccountCreated
    );

    onSuccess(result.session);
}
