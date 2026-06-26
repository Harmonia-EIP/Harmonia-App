#include "WelcomePage.h"

// =============================================================================
// Custom LookAndFeel for the WelcomePage buttons
// =============================================================================
class WelcomeLookAndFeel : public juce::LookAndFeel_V4
{
public:
    enum class BtnStyle { Filled, Outlined, Ghost };

    explicit WelcomeLookAndFeel(BtnStyle style) : m_style(style) {}

    void drawButtonBackground(juce::Graphics& g,
                               juce::Button& button,
                               const juce::Colour& /*backgroundColour*/,
                               bool isHighlighted,
                               bool /*isDown*/) override
    {
        auto bounds = button.getLocalBounds().toFloat().reduced(0.5f);
        const float radius = 10.f;

        if (m_style == BtnStyle::Filled)
        {
            float bgAlpha     = isHighlighted ? 0.12f : 0.06f;
            float borderAlpha = isHighlighted ? 0.35f : 0.15f;

            g.setColour(juce::Colours::white.withAlpha(bgAlpha));
            g.fillRoundedRectangle(bounds, radius);

            // Glow on hover
            if (isHighlighted)
            {
                g.setColour(HarmoniaColours::waveBlue.withAlpha(0.18f));
                g.drawRoundedRectangle(bounds.expanded(2.f), radius + 2.f, 3.f);
            }

            g.setColour(HarmoniaColours::waveBlue.withAlpha(borderAlpha));
            g.drawRoundedRectangle(bounds, radius, 1.f);
        }
        else if (m_style == BtnStyle::Outlined)
        {
            float fillAlpha   = isHighlighted ? 0.06f : 0.03f;
            float borderAlpha = isHighlighted ? 0.22f : 0.12f;

            g.setColour(juce::Colours::white.withAlpha(fillAlpha));
            g.fillRoundedRectangle(bounds, radius);

            g.setColour(juce::Colours::white.withAlpha(borderAlpha));
            g.drawRoundedRectangle(bounds, radius, 1.f);
        }
    }

    void drawButtonText(juce::Graphics& g,
                    juce::TextButton& button,
                    bool isHighlighted,
                    bool /*isDown*/) override
    {
        juce::Colour colour;

        if (m_style == BtnStyle::Ghost)
        {
            colour = isHighlighted
                ? HarmoniaColours::waveCyan.withAlpha(0.95f)
                : HarmoniaColours::textMuted;
        }
        else
        {
            colour = isHighlighted
                ? HarmoniaColours::textPrimary
                : HarmoniaColours::textPrimary.withAlpha(0.85f);
        }

        g.setColour(colour);
        g.setFont(juce::Font("Inter", 15.f, juce::Font::plain));
        g.drawFittedText(button.getButtonText(),
                        button.getLocalBounds(),
                        juce::Justification::centred,
                        1);
    }

private:
    BtnStyle m_style;
};

// =============================================================================
// WelcomePage
// =============================================================================

WelcomePage::WelcomePage()
{
    authLookAndFeel.setThemePreset(AppLookAndFeel::ThemePreset::Dark);
    setLookAndFeel(&authLookAndFeel);

    // -------------------------------------------------------------------------
    // Wave layers  (back → front)
    // -------------------------------------------------------------------------
    waveLayers = { {
        // Back → front (plus basses et plus espacées)
        { 36.f, 0.006f, 0.0f, 0.20f, 0.78f, HarmoniaColours::waveIndigo },
        { 28.f, 0.010f, 1.1f, 0.18f, 0.68f, HarmoniaColours::waveBlue   },
        { 22.f, 0.014f, 2.3f, 0.14f, 0.58f, HarmoniaColours::waveSlate  },
        { 14.f, 0.020f, 0.7f, 0.10f, 0.48f, HarmoniaColours::waveCyan   },
    } };

    // -------------------------------------------------------------------------
    // Title label  –  "Harmonia"
    // -------------------------------------------------------------------------
    titleLabel.setText("Harmonia", juce::dontSendNotification);
    titleLabel.setFont(juce::Font("Space Grotesk", 40.f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, HarmoniaColours::textPrimary);
    titleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(titleLabel);

    // -------------------------------------------------------------------------
    // Subtitle label  –  "AI SYNTH PRESET GENERATOR"
    // -------------------------------------------------------------------------
    subtitleLabel.setText("AI SYNTH PRESET GENERATOR", juce::dontSendNotification);
    subtitleLabel.setFont(juce::Font("Inter", 11.f, juce::Font::plain));
    subtitleLabel.setColour(juce::Label::textColourId, HarmoniaColours::textSubtitle);
    subtitleLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(subtitleLabel);

    // -------------------------------------------------------------------------
    // Buttons
    // -------------------------------------------------------------------------
    signinButton.setButtonText("Sign in");
    signupButton.setButtonText("Create account");
    guestButton.setButtonText("Continue as guest");

    lafSignIn = std::make_unique<WelcomeLookAndFeel>(WelcomeLookAndFeel::BtnStyle::Outlined);
    lafSignUp = std::make_unique<WelcomeLookAndFeel>(WelcomeLookAndFeel::BtnStyle::Outlined);
    lafGuest  = std::make_unique<WelcomeLookAndFeel>(WelcomeLookAndFeel::BtnStyle::Ghost);

    signinButton.setLookAndFeel(lafSignIn.get());
    signupButton.setLookAndFeel(lafSignUp.get());
    guestButton.setLookAndFeel(lafGuest.get());

    for (auto* b : { &signinButton, &signupButton, &guestButton })
        addAndMakeVisible(b);

    signinButton.onClick = [this] { if (onChoice) onChoice(Choice::SignIn); };
    signupButton.onClick = [this] { if (onChoice) onChoice(Choice::SignUp); };
    guestButton.onClick  = [this] { if (onChoice) onChoice(Choice::Guest); };

    // -------------------------------------------------------------------------
    // Timer
    // -------------------------------------------------------------------------
    startTimerHz(60);
}

WelcomePage::~WelcomePage()
{
    stopTimer();

    signinButton.setLookAndFeel(nullptr);
    signupButton.setLookAndFeel(nullptr);
    guestButton.setLookAndFeel(nullptr);

    setLookAndFeel(nullptr);
}

// -----------------------------------------------------------------------------
void WelcomePage::timerCallback()
{
    animationPhase += 0.018f;   // smooth ~1 second full cycle at 60 Hz
    repaint();
}

// =============================================================================
// paint
// =============================================================================
void WelcomePage::paint(juce::Graphics& g)
{
    const float w = (float) getWidth();
    const float h = (float) getHeight();

    // -------------------------------------------------------------------------
    // 1. Background gradient  (deep navy top → dark-blue bottom)
    // -------------------------------------------------------------------------
    juce::ColourGradient bg(
        HarmoniaColours::bgDeep,  0.f,  0.f,
        HarmoniaColours::bgMid,   0.f,  h,
        false
    );
    bg.addColour(0.5, juce::Colour(0xff0f1923));
    g.setGradientFill(bg);
    g.fillAll();

    // -------------------------------------------------------------------------
    // 2. Subtle radial glow behind title area
    // -------------------------------------------------------------------------
    {
        const float cx = w * 0.5f;
        const float cy = h * 0.30f;
        const float radius = w * 0.55f;

        juce::ColourGradient glow(
            HarmoniaColours::waveBlue.withAlpha(0.08f), cx, cy,
            juce::Colours::transparentBlack,            cx + radius, cy,
            true
        );
        g.setGradientFill(glow);
        g.fillEllipse(cx - radius, cy - radius * 0.6f, radius * 2.f, radius * 1.2f);
    }

    // -------------------------------------------------------------------------
    // 3. Wave layers (back → front)
    // -------------------------------------------------------------------------
    for (const auto& layer : waveLayers)
        drawWaveLayer(g, layer, w, h, animationPhase);

    // -------------------------------------------------------------------------
    // 4. Logo icon
    // -------------------------------------------------------------------------
    drawLogoIcon(g, logoIconBounds);
}

// =============================================================================
// drawWaveLayer
// =============================================================================
void WelcomePage::drawWaveLayer(juce::Graphics& g,
                                 const WaveLayer& layer,
                                 float width,
                                 float height,
                                 float phase) const
{
    const float cx  = height * layer.yRatio;
    const float amp = layer.amplitude;
    const float freq = layer.frequency;
    const float phi  = phase + layer.phaseOffset;

    // -- Filled region below the wave --
    juce::Path filled;
    filled.startNewSubPath(0.f, height);

    for (int x = 0; x <= (int) width; ++x)
    {
        float y = cx + std::sin((float) x * freq + phi) * amp
                      + std::sin((float) x * freq * 0.53f + phi * 1.3f) * amp * 0.4f;
        if (x == 0)
            filled.lineTo(0.f, y);
        else
            filled.lineTo((float) x, y);
    }

    filled.lineTo(width, height);
    filled.closeSubPath();

    g.setColour(layer.colour.withAlpha(layer.alphaFill));
    g.fillPath(filled);

    // -- Stroke line on top --
    juce::Path stroke;
    bool started = false;
    for (int x = 0; x <= (int) width; ++x)
    {
        float y = cx + std::sin((float) x * freq + phi) * amp
                      + std::sin((float) x * freq * 0.53f + phi * 1.3f) * amp * 0.4f;
        if (!started) { stroke.startNewSubPath(0.f, y); started = true; }
        else           stroke.lineTo((float) x, y);
    }

    // Wide glow pass
    // Outer glow
    g.setColour(layer.colour.withAlpha(0.08f));
    g.strokePath(stroke, juce::PathStrokeType(14.f));

    // Mid glow
    g.setColour(layer.colour.withAlpha(0.18f));
    g.strokePath(stroke, juce::PathStrokeType(7.f));

    // Sharp crest
    g.setColour(layer.colour.withAlpha(0.85f));
    g.strokePath(stroke, juce::PathStrokeType(1.4f));
}

// =============================================================================
// drawLogoIcon
// =============================================================================
void WelcomePage::drawLogoIcon(juce::Graphics& g,
                                juce::Rectangle<float> bounds) const
{
    // Rounded square background
    g.setColour(HarmoniaColours::iconBg);
    g.fillRoundedRectangle(bounds, 14.f);

    g.setColour(HarmoniaColours::iconBorder);
    g.drawRoundedRectangle(bounds, 14.f, 1.f);

    // Tilde "~" glyph
    g.setColour(HarmoniaColours::iconTilde);
    g.setFont(juce::Font("Inter", bounds.getHeight() * 0.42f, juce::Font::bold));
    g.drawText("~", bounds, juce::Justification::centred, false);
}

// =============================================================================
// resized
// =============================================================================
void WelcomePage::resized()
{
    const int   panelW = 340;
    const float cx     = (float) getWidth()  * 0.5f;
    const float cy     = (float) getHeight() * 0.43f;

    // Vertical block height:
    //   56 (icon) + 16 (gap) + 48 (title) + 8 (gap) + 20 (subtitle)
    // + 40 (gap) + 50 (btn) + 14 (gap) + 50 (btn) + 14 (gap) + 40 (ghost)
    // = 356
    const float blockH = 356.f;
    float y = cy - blockH * 0.5f;

    // ---- Logo icon ----
    const float iconSize = 56.f;
    logoIconBounds = juce::Rectangle<float>(
        cx - iconSize * 0.5f, y, iconSize, iconSize);
    y += iconSize + 20.f;

    // ---- Title ----
    titleLabel.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 48.f).toNearestInt());
    y += 48.f + 8.f;

    // ---- Subtitle ----
    subtitleLabel.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 20.f).toNearestInt());
    y += 20.f + 44.f;

    // ---- Sign-in button (filled) ----
    signinButton.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 50.f).toNearestInt());
    y += 50.f + 14.f;

    // ---- Create account button (outlined) ----
    signupButton.setBounds(juce::Rectangle<float>(
        cx - panelW * 0.5f, y, (float) panelW, 50.f).toNearestInt());
    y += 50.f + 14.f;

    // ---- Guest button (ghost, shorter) ----
    const int guestW = 170;
        guestButton.setBounds(juce::Rectangle<float>(
            cx - guestW * 0.5f, y, (float) guestW, 40.f).toNearestInt());
}
