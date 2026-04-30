#include "TitleComponent.h"

TitleComponent::TitleComponent (const juce::String& textToDisplay, const UserSession& userSession)
    : currentSession(userSession), title (textToDisplay)
{
    titleLabel.setText (title, juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setFont (UIStyle::Fonts::SubTitle());
    addAndMakeVisible (titleLabel);

            
    juce::String pseudo = currentSession.pseudo.isNotEmpty() ? currentSession.pseudo : Strings::Labels::User;

    pseudoLabel.setText (pseudo, juce::dontSendNotification);
    pseudoLabel.setFont (UIStyle::Fonts::Pseudo());
    pseudoLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (pseudoLabel);

    themeButton.addListener (this);
    themeButton.setSize (90, 32);
    addAndMakeVisible (themeButton);

    layoutButton.addListener (this);
    layoutButton.setSize (90, 32);
    addAndMakeVisible (layoutButton);

    logoutButton.addListener (this);
    logoutButton.setSize (80, 30);
    addAndMakeVisible (logoutButton);
}

TitleComponent::~TitleComponent()
{
    themeButton.removeListener (this);
    layoutButton.removeListener (this);
    logoutButton.removeListener (this);
}

void TitleComponent::paint (juce::Graphics& g)
{
    auto bg = findColour (AppColourIds::panelBgId).withAlpha (0.5f);
    g.fillAll (bg);

    titleLabel.setColour (juce::Label::textColourId,
                          findColour (AppColourIds::textPrimaryId));

    pseudoLabel.setColour (juce::Label::textColourId,
                           findColour (AppColourIds::textSecondaryId));
}

void TitleComponent::resized()
{
    auto area = getLocalBounds().reduced (6);

    titleLabel.setBounds (area);

    auto leftArea = area.removeFromLeft (260);
    pseudoLabel.setBounds (leftArea.reduced (10, 0));

    auto rightArea = getLocalBounds().reduced (6);
    rightArea = rightArea.removeFromRight (330);

    auto logoutArea = rightArea.removeFromRight (90);
    logoutButton.setBounds (logoutArea.reduced (4));

    auto themeArea = rightArea.removeFromRight (100);
    themeButton.setBounds (themeArea.reduced (4));

    auto layoutArea = rightArea.removeFromRight (100);
    layoutButton.setBounds (layoutArea.reduced (4));
}

void TitleComponent::buttonClicked(juce::Button* button)
{
    if (button == &logoutButton)
    {
        if (onLogout)
            onLogout();
        return;
    }

    if (button == &themeButton)
    {
        juce::PopupMenu m;

        auto currentPreset =
            ThemeAndLayoutConverter::idToThemePreset(currentSession.themeId);

        m.addItem(1, Strings::Themes::Dark,  true, currentPreset == ThemePreset::Dark);
        m.addItem(2, Strings::Themes::Light, true, currentPreset == ThemePreset::Light);
        m.addItem(3, Strings::Themes::Red,   true, currentPreset == ThemePreset::Red);
        m.addItem(4, Strings::Themes::Blue,  true, currentPreset == ThemePreset::Blue);

        m.showMenuAsync(
            juce::PopupMenu::Options()
                .withTargetComponent(themeButton)
                .withPreferredPopupDirection(
                    juce::PopupMenu::Options::PopupDirection::downwards),
            [this](int result)
            {
                if (result <= 0 || !onThemeSelected)
                    return;

                currentSession.themeId = result;

                auto preset =
                    ThemeAndLayoutConverter::idToThemePreset(result);

                onThemeSelected(preset);
            });

        return;
    }

    if (button == &layoutButton)
    {
        juce::PopupMenu m;

        auto currentPreset =
            ThemeAndLayoutConverter::idToLayoutPreset(currentSession.layoutId);

        m.addItem(1, Strings::Layouts::Layout1, true, currentPreset == LayoutPreset::Layout1);
        m.addItem(2, Strings::Layouts::Layout2, true, currentPreset == LayoutPreset::Layout2);
        m.addItem(3, Strings::Layouts::Layout3, true, currentPreset == LayoutPreset::Layout3);
        m.addItem(4, Strings::Layouts::Layout4, true, currentPreset == LayoutPreset::Layout4);

        m.showMenuAsync(
            juce::PopupMenu::Options()
                .withTargetComponent(layoutButton)
                .withPreferredPopupDirection(
                    juce::PopupMenu::Options::PopupDirection::downwards),
            [this](int result)
            {
                if (result <= 0 || !onLayoutSelected)
                    return;

                currentSession.layoutId = result;

                auto preset =
                    ThemeAndLayoutConverter::idToLayoutPreset(result);

                onLayoutSelected(preset);
            });

        return;
    }
}