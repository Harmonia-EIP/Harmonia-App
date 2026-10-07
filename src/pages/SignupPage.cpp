#include "SignupPage.h"
#include <BinaryData.h>

SignupPage::SignupPage(BackendManager& be,
                       std::function<void(const UserSession&)> onSignupSuccess)
    : backend(be), onSuccess(onSignupSuccess)
{
    authLookAndFeel.setThemePreset(AppLookAndFeel::ThemePreset::Dark);
    setLookAndFeel(&authLookAndFeel);

    logoImage = juce::ImageCache::getFromMemory(BinaryData::harmonia_logo_png,
                                                BinaryData::harmonia_logo_pngSize);

    waveLayers = { {
        { 24.f, 0.007f, 0.0f, 0.06f, 0.88f, HarmoniaColours::waveIndigo },
        { 18.f, 0.012f, 1.2f, 0.05f, 0.80f, HarmoniaColours::waveBlue   },
        { 12.f, 0.018f, 2.4f, 0.04f, 0.72f, HarmoniaColours::waveCyan   }
    } };

    titleLabel.setText(Strings::Titles::CreateAccount, juce::dontSendNotification);
    titleLabel.setFont(juce::Font("Space Grotesk", 24.f, juce::Font::bold)
                           .withExtraKerningFactor(-0.01f));
    titleLabel.setColour(juce::Label::textColourId, HarmoniaColours::textPrimary);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText(Strings::Titles::HarmoniaAiTitle.toUpperCase(),
                          juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font("Inter", 10.f, juce::Font::plain)
                              .withExtraKerningFactor(0.30f));
    subtitleLabel.setColour(juce::Label::textColourId,
                            HarmoniaColours::waveBlue.withAlpha(0.65f));
    subtitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(subtitleLabel);

    const auto placeholder = HarmoniaColours::textPrimary.withAlpha(0.35f);

    usernameField.setTextToShowWhenEmpty(Strings::Placeholders::Username,   placeholder);
    firstnameField.setTextToShowWhenEmpty(Strings::Placeholders::FirstName, placeholder);
    lastnameField.setTextToShowWhenEmpty(Strings::Placeholders::LastName,   placeholder);
    emailField.setTextToShowWhenEmpty(Strings::Placeholders::Email,         placeholder);
    passwordField.setTextToShowWhenEmpty(Strings::Placeholders::Password,   placeholder);
    passwordField.setPasswordCharacter(Strings::Placeholders::PasswordChar);

    for (auto* field : { &usernameField, &firstnameField, &lastnameField,
                         &emailField,    &passwordField })
    {
        addAndMakeVisible(*field);
    }

    fieldLaf.applyTo(usernameField,  "user");
    fieldLaf.applyTo(firstnameField, "user");
    fieldLaf.applyTo(lastnameField,  "user");
    fieldLaf.applyTo(emailField,     "mail");
    fieldLaf.applyTo(passwordField,  "lock");

    lafSignup = std::make_unique<AuthPageLookAndFeel>(AuthPageLookAndFeel::Style::Primary);
    lafBack   = std::make_unique<AuthPageLookAndFeel>(AuthPageLookAndFeel::Style::Back);

    signupButton.setButtonText(Strings::Buttons::CreateAccount);
    signupButton.setLookAndFeel(lafSignup.get());

    backButton.setButtonText(Strings::Buttons::Back);
    backButton.setLookAndFeel(lafBack.get());

    addAndMakeVisible(signupButton);
    addAndMakeVisible(backButton);

    signupButton.onClick = [this]() { handleSignup(); };
    backButton.onClick   = [this]() { if (onBack) onBack(); };

    // -------------------------------------------------------------------------
    // Navigation clavier
    // Tab / Shift+Tab restent dans la page, dans l'ordre :
    // pseudo -> prénom -> nom -> email -> mot de passe -> Create account -> Back.
    // -------------------------------------------------------------------------
    setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer);

    {
        int focusOrder = 1;

        for (juce::Component* c : { (juce::Component*) &usernameField,
                                    (juce::Component*) &firstnameField,
                                    (juce::Component*) &lastnameField,
                                    (juce::Component*) &emailField,
                                    (juce::Component*) &passwordField,
                                    (juce::Component*) &signupButton,
                                    (juce::Component*) &backButton })
        {
            c->setWantsKeyboardFocus(true);
            c->setExplicitFocusOrder(focusOrder++);
        }

        // Un clic souris sur un bouton ne laisse pas de halo de focus affiché
        signupButton.setMouseClickGrabsKeyboardFocus(false);
        backButton.setMouseClickGrabsKeyboardFocus(false);
    }

    // Entrée : champ suivant, puis validation depuis le mot de passe
    usernameField.onReturnKey  = [this]() { firstnameField.grabKeyboardFocus(); };
    firstnameField.onReturnKey = [this]() { lastnameField.grabKeyboardFocus(); };
    lastnameField.onReturnKey  = [this]() { emailField.grabKeyboardFocus(); };
    emailField.onReturnKey     = [this]() { passwordField.grabKeyboardFocus(); };
    passwordField.onReturnKey  = [this]() { handleSignup(); };

    startTimerHz(60);
}

SignupPage::~SignupPage()
{
    stopTimer();
    for (auto* field : { &usernameField, &firstnameField, &lastnameField,
                         &emailField,    &passwordField })
        field->setLookAndFeel(nullptr);
    signupButton.setLookAndFeel(nullptr);
    backButton.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

void SignupPage::visibilityChanged()
{
    if (! isVisible())
        return;

    // Différé : la page n'est pas forcément encore rattachée à une fenêtre.
    // SafePointer car la page peut être détruite avant l'exécution.
    juce::Component::SafePointer<juce::Component> first (&usernameField);

    juce::MessageManager::callAsync([first]
    {
        if (first != nullptr && first->isShowing())
            first->grabKeyboardFocus();
    });
}

void SignupPage::timerCallback()
{
    animationPhase += 0.012f;
    repaint();
}

void SignupPage::paint(juce::Graphics& g)
{
    const float w = (float) getWidth();
    const float h = (float) getHeight();

    juce::ColourGradient bg(
        HarmoniaColours::bgDeep, 0.f, 0.f,
        HarmoniaColours::bgMid,  0.f, h,
        false);
    bg.addColour(0.55, juce::Colour(0xff0f1923));
    g.setGradientFill(bg);
    g.fillAll();

    {
        const float cx     = w * 0.5f;
        const float cy     = h * 0.22f;
        const float radius = w * 0.42f;

        juce::ColourGradient glow(
            HarmoniaColours::waveBlue.withAlpha(0.06f), cx, cy,
            juce::Colours::transparentBlack,             cx + radius, cy,
            true);
        g.setGradientFill(glow);
        g.fillEllipse(cx - radius, cy - radius * 0.6f, radius * 2.f, radius * 1.2f);
    }

    for (const auto& layer : waveLayers)
        drawWaveLayer(g, layer, w, h, animationPhase);

    drawLogoIcon(g, logoIconBounds);

    {
        const float lineW = 56.f;
        const float lx    = w * 0.5f - lineW * 0.5f;
        const float ly    = (float) subtitleLabel.getBottom() + 10.f;

        juce::ColourGradient line(
            HarmoniaColours::waveBlue.withAlpha(0.0f), lx,         ly,
            HarmoniaColours::waveBlue.withAlpha(0.0f), lx + lineW, ly,
            false);
        line.addColour(0.5, HarmoniaColours::waveBlue.withAlpha(0.7f));
        g.setGradientFill(line);
        g.fillRect(lx, ly, lineW, 1.f);
    }
}

void SignupPage::paintOverChildren(juce::Graphics& g)
{
    // Halo autour du bouton qui a le focus clavier (Tab). La page se repeint
    // déjà à 60 Hz (animation), donc pas besoin d'écouter les changements de focus.
    for (auto* b : { &signupButton, &backButton })
    {
        if (b->hasKeyboardFocus(true))
        {
            const auto r = b->getBounds().toFloat().expanded(2.f);

            g.setColour(HarmoniaColours::waveCyan.withAlpha(0.9f));
            g.drawRoundedRectangle(r, r.getHeight() * 0.5f, 1.5f);
        }
    }
}

void SignupPage::drawWaveLayer(juce::Graphics& g,
                                const WaveLayer& layer,
                                float width, float height, float phase) const
{
    const float cy = height * layer.yRatio;

    juce::Path path;
    path.startNewSubPath(0.f, height);

    for (int x = 0; x <= (int) width; ++x)
    {
        float y = cy
            + std::sin((float) x * layer.frequency + phase + layer.phaseOffset)
              * layer.amplitude;
        path.lineTo((float) x, y);
    }

    path.lineTo(width, height);
    path.closeSubPath();

    g.setColour(layer.colour.withAlpha(layer.alphaFill));
    g.fillPath(path);
}

void SignupPage::drawLogoIcon(juce::Graphics& g, juce::Rectangle<float> bounds) const
{
    // Logo seul : pas de carré, pas de bordure, pas de "~"
    if (! logoImage.isValid())
        return;

    g.setOpacity(1.0f); // sinon le logo hérite de l'alpha de la dernière vague
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImageWithin(logoImage,
                      (int) bounds.getX(), (int) bounds.getY(),
                      (int) bounds.getWidth(), (int) bounds.getHeight(),
                      juce::RectanglePlacement::centred);
}

void SignupPage::resized()
{
    const int   panelW = 300;
    const float cx     = (float) getWidth()  * 0.5f;
    const float cy     = (float) getHeight() * 0.47f;

    const float iconSize = 80.f;
    const float blockH   = 441.f + (iconSize - 44.f);

    float y = cy - blockH * 0.5f;

    logoIconBounds = { cx - iconSize * 0.5f, y, iconSize, iconSize };
    y += iconSize + 16.f;

    titleLabel.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 44.f).toNearestInt());
    y += 44.f;

    subtitleLabel.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 18.f).toNearestInt());
    y += 18.f + 22.f;

    const float fh  = 36.f;
    const float gap = 8.f;

    for (auto* field : { &usernameField, &firstnameField, &lastnameField,
                         &emailField,    &passwordField })
    {
        field->setBounds(juce::Rectangle<float>(
            cx - panelW * 0.5f, y, (float) panelW, fh).toNearestInt());
        y += fh + gap;
    }

    y += 14.f;

    signupButton.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 40.f).toNearestInt());
    y += 40.f + 9.f;

    const int backW = 140;
    backButton.setBounds(juce::Rectangle<float>(
        cx - backW * 0.5f, y, (float) backW, 34.f).toNearestInt());
}

void SignupPage::handleSignup()
{
    auto username  = usernameField.getText().trim();
    auto firstname = firstnameField.getText().trim();
    auto lastname  = lastnameField.getText().trim();
    auto email     = emailField.getText().trim();
    auto password  = passwordField.getText().trim();

    if (username.isEmpty() || firstname.isEmpty() || lastname.isEmpty()
        || email.isEmpty()  || password.isEmpty())
    {
        HarmoniaAlert::warning(Strings::Errors::MissingFields,
                               Strings::Errors::MissingFieldsAdvice);
        return;
    }

    auto result = backend.signupUser(username, firstname, lastname, email, password);

    if (!result.success)
    {
        HarmoniaAlert::error(Strings::Errors::AuthenticationError, result.errorMessage);
        return;
    }

    HarmoniaAlert::info(Strings::Success::Welcome, Strings::Success::AccountCreated);
    onSuccess(result.session);
}