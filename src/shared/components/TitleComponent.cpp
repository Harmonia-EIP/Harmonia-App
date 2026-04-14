#include "TitleComponent.h"

TitleComponent::TitleComponent (const juce::String& textToDisplay, const UserSession& userSession)
    : currentSession(userSession), title (textToDisplay)
{
    // ===== TITLE =====
    titleLabel.setText (title, juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setFont (juce::Font (28.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    // ===== USER / PSEUDO =====
            
    juce::String pseudo = currentSession.pseudo.isNotEmpty() ? currentSession.pseudo : "User";

    pseudoLabel.setText (pseudo, juce::dontSendNotification);
    pseudoLabel.setFont (juce::Font (22.0f, juce::Font::bold));
    pseudoLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (pseudoLabel);

    // ===== THEME BUTTON =====
    themeButton.addListener (this);
    themeButton.setSize (90, 32);
    addAndMakeVisible (themeButton);

    // ===== LAYOUT BUTTON =====
    layoutButton.addListener (this);
    layoutButton.setSize (90, 32);
    addAndMakeVisible (layoutButton);

    // ===== LOGOUT BUTTON =====
    logoutButton.addListener (this);
    logoutButton.setSize (80, 30); // plus petit
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

    // ===== TITLE (centré sur toute la largeur) =====
    titleLabel.setBounds (area);

    // ===== LEFT : PSEUDO =====
    auto leftArea = area.removeFromLeft (260);
    pseudoLabel.setBounds (leftArea.reduced (10, 0));

    // ===== RIGHT : BUTTONS =====
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
    // ===== LOGOUT =====
    if (button == &logoutButton)
    {
        if (onLogout)
            onLogout();
        return;
    }

    // ===== THEME MENU =====
    if (button == &themeButton)
    {
        juce::PopupMenu m;

        auto currentPreset =
            ThemeAndLayoutConverter::idToThemePreset(currentSession.themeId);

        m.addItem(1, "Dark",  true, currentPreset == ThemePreset::Dark);
        m.addItem(2, "Light", true, currentPreset == ThemePreset::Light);
        m.addItem(3, "Red",   true, currentPreset == ThemePreset::Red);
        m.addItem(4, "Blue",  true, currentPreset == ThemePreset::Blue);

        m.showMenuAsync(
            juce::PopupMenu::Options()
                .withTargetComponent(themeButton)
                .withPreferredPopupDirection(
                    juce::PopupMenu::Options::PopupDirection::downwards),
            [this](int result)
            {
                if (result <= 0 || !onThemeSelected)
                    return;

                // 🔥 update session FIRST
                currentSession.themeId = result;

                // convert id → preset
                auto preset =
                    ThemeAndLayoutConverter::idToThemePreset(result);

                onThemeSelected(preset);
            });

        return;
    }

    // ===== LAYOUT MENU =====
    if (button == &layoutButton)
    {
        juce::PopupMenu m;

        auto currentPreset =
            ThemeAndLayoutConverter::idToLayoutPreset(currentSession.layoutId);

        m.addItem(1, "Layout 1", true, currentPreset == LayoutPreset::Layout1);
        m.addItem(2, "Layout 2", true, currentPreset == LayoutPreset::Layout2);
        m.addItem(3, "Layout 3", true, currentPreset == LayoutPreset::Layout3);
        m.addItem(4, "Layout 4", true, currentPreset == LayoutPreset::Layout4);

        m.showMenuAsync(
            juce::PopupMenu::Options()
                .withTargetComponent(layoutButton)
                .withPreferredPopupDirection(
                    juce::PopupMenu::Options::PopupDirection::downwards),
            [this](int result)
            {
                if (result <= 0 || !onLayoutSelected)
                    return;

                // 🔥 update session FIRST
                currentSession.layoutId = result;

                auto preset =
                    ThemeAndLayoutConverter::idToLayoutPreset(result);

                onLayoutSelected(preset);
            });

        return;
    }
}