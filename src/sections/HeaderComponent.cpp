#include "HeaderComponent.h"

HeaderComponent::HeaderComponent (const UserSession& s)
    : session (s)
{
    juce::String username = session.pseudo.isNotEmpty()
        ? session.pseudo
        : Strings::Errors::NoUserConnected;

    titleLabel.setText (
        username.toUpperCase(),
        juce::dontSendNotification);

    titleLabel.setFont (
        juce::Font (
            juce::FontOptions (22.0f)
                .withStyle ("Bold"))
            .withExtraKerningFactor (0.10f));

    addAndMakeVisible (titleLabel);

    subtitleLabel.setText (
        Strings::Titles::Subtitle,
        juce::dontSendNotification);

    subtitleLabel.setFont (
        juce::Font (
            juce::FontOptions (9.5f))
                .withExtraKerningFactor (0.20f));

    subtitleLabel.setColour (
        juce::Label::textColourId,
        juce::Colour (0xff8a8a8a));

    addAndMakeVisible (subtitleLabel);

    promptEditor.setMultiLine (false);

    promptEditor.setReturnKeyStartsNewLine (false);

    promptEditor.setTextToShowWhenEmpty (
        Strings::Placeholders::Prompt,
        HarmoniaPalette::textMuted);

    promptEditor.setFont (
        juce::Font (
            juce::FontOptions (12.5f)));

    promptEditor.setIndents (10, 6);

    addAndMakeVisible (promptEditor);

    generateButton.getProperties().set (
        "accent",
        true);

    addAndMakeVisible (generateButton);

    presetLabel.setText (
        Strings::Labels::UnsetPreset.toUpperCase(),
        juce::dontSendNotification);

    presetLabel.setFont (
        juce::Font (
            juce::FontOptions (10.5f)
                .withStyle ("Bold"))
            .withExtraKerningFactor (0.18f));

    presetLabel.setJustificationType (
        juce::Justification::centredLeft);

    generateButton.setButtonText (Strings::Buttons::Generate);
    loadButton.setButtonText     (Strings::Buttons::Load);
    saveButton.setButtonText     (Strings::Buttons::Export);
    if (session.isGuest)
    {
        logoutButton.setButtonText(Strings::Buttons::Back);
    }
    else
    {
        logoutButton.setButtonText(Strings::Buttons::Logout);
    }

    for (auto* b : {
            &generateButton,
            &loadButton,
            &saveButton,
            &logoutButton
        })
    {
        b->setColour (
            juce::TextButton::textColourOffId,
            HarmoniaPalette::textMuted);
    }

    addAndMakeVisible (presetLabel);

    addAndMakeVisible (loadButton);
    addAndMakeVisible (saveButton);
    addAndMakeVisible (logoutButton);
    addAndMakeVisible (paletteSelector);

    paletteSelector.setCurrentTheme(
        HarmoniaPalette::themeFromId(session.themeId));

    paletteSelector.onThemeUpdated =
        [this]
    {
        auto theme = paletteSelector.getCurrentTheme();

        HarmoniaPalette::setTheme(theme);

        if (onThemeChanged)
            onThemeChanged(theme);

        repaint();
    };
}

void HeaderComponent::paint (juce::Graphics& g)
{
    auto header = getLocalBounds();

    juce::ColourGradient hbg (
        HarmoniaPalette::panelTop,
        0,
        0,
        HarmoniaPalette::panel,
        0,
        (float) header.getHeight(),
        false);

    g.setGradientFill (hbg);

    g.fillRect (header);

    g.setColour (
        juce::Colours::white.withAlpha (0.04f));

    g.fillRect (
        header.getX(),
        header.getY() + 1,
        header.getWidth(),
        1);

    g.setColour (
        HarmoniaPalette::border);

    g.fillRect (
        header.getX(),
        header.getBottom() - 1,
        header.getWidth(),
        1);

    g.setColour (
        HarmoniaPalette::accent.withAlpha (0.40f));

    g.fillRect (
        header.getX(),
        header.getBottom(),
        header.getWidth(),
        1);
}

void HeaderComponent::refreshTheme()
{
    titleLabel.setColour(
        juce::Label::textColourId,
        HarmoniaPalette::textPrimary);

    subtitleLabel.setColour(
        juce::Label::textColourId,
        HarmoniaPalette::textMuted);

    presetLabel.setColour(
        juce::Label::textColourId,
        HarmoniaPalette::textSecondary);


    promptEditor.setColour(
        juce::TextEditor::textColourId,
        HarmoniaPalette::textPrimary);

    promptEditor.setTextToShowWhenEmpty (
        Strings::Placeholders::Prompt,
        HarmoniaPalette::textMuted);


    for (auto* b :
        {
            &generateButton,
            &loadButton,
            &saveButton,
            &logoutButton
        })
    {
        b->setColour(
            juce::TextButton::textColourOffId,
            HarmoniaPalette::textMuted);

        b->setColour(
            juce::TextButton::textColourOnId,
            HarmoniaPalette::background);
    }

    repaint();
}

void HeaderComponent::resized()
{
    auto header =
        getLocalBounds().reduced (16, 10);

    auto leftHeader =
        header.removeFromLeft (260);

    titleLabel.setBounds (
        leftHeader.removeFromTop (28));

    subtitleLabel.setBounds (
        leftHeader.removeFromTop (16));

    const int palW = 110;
    const int palH = 20;
    const int offsetX = 35;

    paletteSelector.setBounds (
        titleLabel.getRight() - palW - offsetX,
        titleLabel.getY()
            + (titleLabel.getHeight() - palH) / 2,
        palW,
        palH);

    const int buttonW = 72;
    const int buttonH = 28;
    const int gap = 8;

    auto buttonsArea =
        header.removeFromRight (
            buttonW * 3 + gap * 2);

    auto logoutArea =
        buttonsArea.removeFromRight (buttonW);

    buttonsArea.removeFromRight (gap);

    auto saveArea =
        buttonsArea.removeFromRight (buttonW);

    buttonsArea.removeFromRight (gap);

    auto loadArea =
        buttonsArea.removeFromRight (buttonW);

    auto centerY =
        header.getCentreY();

    auto place =
        [&] (juce::Rectangle<int> area)
    {
        return juce::Rectangle<int> (
            area.getX(),
            centerY - buttonH / 2,
            buttonW,
            buttonH);
    };

    logoutButton.setBounds (
        place (logoutArea));

    saveButton.setBounds (
        place (saveArea));

    loadButton.setBounds (
        place (loadArea));

    header.removeFromTop (8);

    header.removeFromRight (12);

    auto promptRow =
        header.removeFromTop (32);

    auto generateArea =
        promptRow.removeFromRight (104);

    promptRow.removeFromRight (8);

    promptEditor.setBounds (promptRow);

    generateButton.setBounds (generateArea);

    presetLabel.setBounds (
        header.removeFromTop (16));
}