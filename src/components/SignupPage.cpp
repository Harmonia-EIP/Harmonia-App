#include "SignupPage.h"

SignupPage::SignupPage(SupabaseManager& sb, std::function<void(const UserSession&)> onSignupSuccess)
    : supabase(sb), onSuccess(onSignupSuccess)
{
    // --- Titre ---
    titleLabel.setText("Create your Harmonia account", juce::dontSendNotification);
    titleLabel.setFont(juce::Font(24.0f, juce::Font::bold));
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(titleLabel);

    // --- Champs ---
    usernameField.setTextToShowWhenEmpty("Username", juce::Colours::grey);
    firstnameField.setTextToShowWhenEmpty("First name", juce::Colours::grey);
    lastnameField.setTextToShowWhenEmpty("Last name", juce::Colours::grey);
    emailField.setTextToShowWhenEmpty("Email", juce::Colours::grey);
    passwordField.setTextToShowWhenEmpty("Password", juce::Colours::grey);
    passwordField.setPasswordCharacter('*');

    for (auto* f : { &usernameField, &firstnameField, &lastnameField, &emailField, &passwordField })
    {
        f->setColour(juce::TextEditor::backgroundColourId, juce::Colours::whitesmoke);
        f->setColour(juce::TextEditor::textColourId, juce::Colours::black);
        addAndMakeVisible(*f);
    }

    // --- Boutons ---
    signupButton.setButtonText("Create account");
    signupButton.setColour(juce::TextButton::buttonColourId, juce::Colours::skyblue);
    signupButton.setColour(juce::TextButton::textColourOffId, juce::Colours::black);
    signupButton.onClick = [this]() { handleSignup(); };
    addAndMakeVisible(signupButton);

    backButton.setButtonText("← Back");
    backButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkgrey);
    backButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    backButton.onClick = [this]() { if (onBack) onBack(); };
    addAndMakeVisible(backButton);
}

void SignupPage::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::darkslategrey);
}

void SignupPage::resized()
{
    auto area = getLocalBounds().reduced(200);

    // Centrer verticalement le contenu
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
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                               "Missing fields",
                                               "Please fill in all fields.");
        return;
    }

    auto session = supabase.signupUser(email, password, username, firstname, lastname);

    if (session.has_value())
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
                                               "Welcome",
                                               "Account created successfully!");
        onSuccess(*session);
    }
    else
    {
        juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                               "Error",
                                               "Account creation failed.");
    }
}
