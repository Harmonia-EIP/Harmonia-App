#include "LoginPage.h"

LoginPage::LoginPage(BackendManager& be,
                     std::function<void(const UserSession&)> onSuccessCallback)
    : backend(be), onSuccess(onSuccessCallback)
{
    authLookAndFeel.setThemePreset(AppLookAndFeel::ThemePreset::Dark);
    setLookAndFeel(&authLookAndFeel);

    waveLayers = { {
        { 24.f, 0.007f, 0.0f, 0.06f, 0.88f, HarmoniaColours::waveIndigo },
        { 18.f, 0.012f, 1.2f, 0.05f, 0.80f, HarmoniaColours::waveBlue   },
        { 12.f, 0.018f, 2.4f, 0.04f, 0.72f, HarmoniaColours::waveCyan   }
    } };

    titleLabel.setText(Strings::Titles::SignIn, juce::dontSendNotification);
    titleLabel.setFont(UIStyle::Fonts::SubTitle());
    titleLabel.setColour(juce::Label::textColourId, HarmoniaColours::textPrimary);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    subtitleLabel.setText(Strings::Titles::HarmoniaAiTitle, juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font("Inter", 10.5f, juce::Font::plain));
    subtitleLabel.setColour(juce::Label::textColourId,
                            HarmoniaColours::textPrimary.withAlpha(0.28f));
    subtitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(subtitleLabel);

    auto placeholder = findColour(AppColourIds::textSecondaryId);

    identifierField.setTextToShowWhenEmpty(Strings::Placeholders::identifier, placeholder);
    passwordField.setTextToShowWhenEmpty(Strings::Placeholders::Password, placeholder);
    passwordField.setPasswordCharacter(Strings::Placeholders::PasswordChar);

    for (auto* field : { &identifierField, &passwordField })
    {
        field->setColour(juce::TextEditor::backgroundColourId,
                         juce::Colours::white.withAlpha(0.04f));
        field->setColour(juce::TextEditor::outlineColourId,
                         juce::Colours::white.withAlpha(0.08f));
        field->setColour(juce::TextEditor::focusedOutlineColourId,
                         HarmoniaColours::waveBlue.withAlpha(0.35f));
        field->setColour(juce::TextEditor::textColourId,
                         HarmoniaColours::textPrimary);
        addAndMakeVisible(*field);
    }

    lafLogin = std::make_unique<AuthPageLookAndFeel>(AuthPageLookAndFeel::Style::Primary);
    lafBack  = std::make_unique<AuthPageLookAndFeel>(AuthPageLookAndFeel::Style::Back);

    loginButton.setLookAndFeel(lafLogin.get());
    backButton.setLookAndFeel(lafBack.get());

    addAndMakeVisible(loginButton);
    addAndMakeVisible(backButton);

    loginButton.onClick = [this]() { handleLogin(); };
    backButton.onClick  = [this]() { if (onBack) onBack(); };

    startTimerHz(60);
}

LoginPage::~LoginPage()
{
    stopTimer();
    loginButton.setLookAndFeel(nullptr);
    backButton.setLookAndFeel(nullptr);
    setLookAndFeel(nullptr);
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
    g.setColour(HarmoniaColours::iconBg);
    g.fillRoundedRectangle(bounds, 12.f);

    g.setColour(HarmoniaColours::iconBorder);
    g.drawRoundedRectangle(bounds, 12.f, 1.f);

    g.setColour(HarmoniaColours::iconTilde);
    g.setFont(juce::Font("Inter", bounds.getHeight() * 0.42f, juce::Font::bold));
    g.drawText("~", bounds, juce::Justification::centred, false);
}

void LoginPage::resized()
{
    const int   panelW = 300;
    const float cx     = (float) getWidth()  * 0.5f;
    const float cy     = (float) getHeight() * 0.44f;

    const float blockH = 336.f;
    float y = cy - blockH * 0.5f;

    const float iconSize = 44.f;
    logoIconBounds = { cx - iconSize * 0.5f, y, iconSize, iconSize };
    y += iconSize + 16.f;

    titleLabel.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 44.f).toNearestInt());
    y += 44.f;

    subtitleLabel.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 18.f).toNearestInt());
    y += 18.f + 28.f;

    identifierField.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 38.f).toNearestInt());
    y += 38.f + 9.f;

    passwordField.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 38.f).toNearestInt());
    y += 38.f + 22.f;

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
