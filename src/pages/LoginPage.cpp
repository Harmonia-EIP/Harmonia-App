#include "LoginPage.h"
#include <BinaryData.h>

LoginPage::LoginPage(BackendManager& be,
                     std::function<void(const UserSession&)> onSuccessCallback)
    : backend(be), onSuccess(onSuccessCallback)
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

    titleLabel.setText(Strings::Titles::SignIn, juce::dontSendNotification);
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

    identifierField.setTextToShowWhenEmpty(Strings::Placeholders::identifier, placeholder);
    passwordField.setTextToShowWhenEmpty(Strings::Placeholders::Password, placeholder);
    passwordField.setPasswordCharacter(Strings::Placeholders::PasswordChar);

    fieldLaf.applyTo(identifierField, "user");
    fieldLaf.applyTo(passwordField,   "lock");
    addAndMakeVisible(identifierField);
    addAndMakeVisible(passwordField);

    lafLogin = std::make_unique<AuthPageLookAndFeel>(AuthPageLookAndFeel::Style::Primary);
    lafBack  = std::make_unique<AuthPageLookAndFeel>(AuthPageLookAndFeel::Style::Back);

    loginButton.setLookAndFeel(lafLogin.get());
    backButton.setLookAndFeel(lafBack.get());

    addAndMakeVisible(loginButton);
    addAndMakeVisible(backButton);

    loginButton.onClick = [this]() { handleLogin(); };
    backButton.onClick  = [this]() { if (onBack) onBack(); };

    // -------------------------------------------------------------------------
    // Navigation clavier
    // Tab / Shift+Tab restent dans la page, dans l'ordre :
    // identifiant -> mot de passe -> Sign in -> Back.
    // -------------------------------------------------------------------------
    setFocusContainerType(juce::Component::FocusContainerType::keyboardFocusContainer);

    {
        int focusOrder = 1;

        for (juce::Component* c : { (juce::Component*) &identifierField,
                                    (juce::Component*) &passwordField,
                                    (juce::Component*) &loginButton,
                                    (juce::Component*) &backButton })
        {
            c->setWantsKeyboardFocus(true);
            c->setExplicitFocusOrder(focusOrder++);
        }

        // Un clic souris sur un bouton ne laisse pas de halo de focus affiché
        loginButton.setMouseClickGrabsKeyboardFocus(false);
        backButton.setMouseClickGrabsKeyboardFocus(false);
    }

    // Entrée : identifiant -> mot de passe, puis mot de passe -> validation
    identifierField.onReturnKey = [this]() { passwordField.grabKeyboardFocus(); };
    passwordField.onReturnKey   = [this]() { handleLogin(); };

    startTimerHz(60);
}

LoginPage::~LoginPage()
{
    stopTimer();
    identifierField.setLookAndFeel(nullptr);
    passwordField.setLookAndFeel(nullptr);
    loginButton.setLookAndFeel(nullptr);
    backButton.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
}

void LoginPage::visibilityChanged()
{
    if (! isVisible())
        return;

    // Différé : la page n'est pas forcément encore rattachée à une fenêtre.
    // SafePointer car la page peut être détruite avant l'exécution.
    juce::Component::SafePointer<juce::Component> first (&identifierField);

    juce::MessageManager::callAsync([first]
    {
        if (first != nullptr && first->isShowing())
            first->grabKeyboardFocus();
    });
}

void LoginPage::timerCallback()
{
    animationPhase += 0.012f;
    repaint();
}

void LoginPage::paint(juce::Graphics& g)
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

void LoginPage::paintOverChildren(juce::Graphics& g)
{
    // Halo autour du bouton qui a le focus clavier (Tab). La page se repeint
    // déjà à 60 Hz (animation), donc pas besoin d'écouter les changements de focus.
    for (auto* b : { &loginButton, &backButton })
    {
        if (b->hasKeyboardFocus(true))
        {
            const auto r = b->getBounds().toFloat().expanded(2.f);

            g.setColour(HarmoniaColours::waveCyan.withAlpha(0.9f));
            g.drawRoundedRectangle(r, r.getHeight() * 0.5f, 1.5f);
        }
    }
}

void LoginPage::drawWaveLayer(juce::Graphics& g,
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

void LoginPage::drawLogoIcon(juce::Graphics& g, juce::Rectangle<float> bounds) const
{
    if (! logoImage.isValid())
        return;

    g.setOpacity(1.0f);
    g.setImageResamplingQuality(juce::Graphics::highResamplingQuality);
    g.drawImageWithin(logoImage,
                      (int) bounds.getX(), (int) bounds.getY(),
                      (int) bounds.getWidth(), (int) bounds.getHeight(),
                      juce::RectanglePlacement::centred);
}

void LoginPage::resized()
{
    const int   panelW = 300;
    const float cx     = (float) getWidth()  * 0.5f;
    const float cy     = (float) getHeight() * 0.44f;

    const float iconSize = 80.f;
    const float blockH   = 344.f + (iconSize - 44.f);

    float y = cy - blockH * 0.5f;

    logoIconBounds = { cx - iconSize * 0.5f, y, iconSize, iconSize };
    y += iconSize + 16.f;

    titleLabel.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 44.f).toNearestInt());
    y += 44.f;

    subtitleLabel.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 18.f).toNearestInt());
    y += 18.f + 28.f;

    identifierField.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 42.f).toNearestInt());
    y += 42.f + 9.f;

    passwordField.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 42.f).toNearestInt());
    y += 42.f + 22.f;

    loginButton.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 40.f).toNearestInt());
    y += 40.f + 9.f;

    const int backW = 140;
    backButton.setBounds(juce::Rectangle<float>(
        cx - backW * 0.5f, y, (float) backW, 34.f).toNearestInt());
}

void LoginPage::handleLogin()
{
    auto identifier = identifierField.getText().trim();
    auto password   = passwordField.getText().trim();

    if (identifier.isEmpty() || password.isEmpty())
    {
        HarmoniaAlert::warning(Strings::Errors::MissingFields,
                               Strings::Errors::MissingFieldsAdvice);
        return;
    }

    auto result = backend.loginUser(identifier, password);

    if (!result.success)
    {
        HarmoniaAlert::error(Strings::Errors::AuthenticationError, result.errorMessage);
        return;
    }

    onSuccess(result.session);
}