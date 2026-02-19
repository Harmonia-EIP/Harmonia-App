// TitleComponent.cpp
#include "TitleComponent.h"
#include "../themes/AppColourIds.h"

TitleComponent::TitleComponent (const juce::String& textToDisplay, BackendManager& be)
    : backend (be), title (textToDisplay)
{
    // ===== TITLE =====
    titleLabel.setText (title, juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setFont (juce::Font (28.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    // ===== USER / PSEUDO =====
    auto sessionOpt = backend.loadSession();
    juce::String pseudo = sessionOpt.has_value() ? sessionOpt->pseudo : "Invité";

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

void TitleComponent::buttonClicked (juce::Button* button)
{
    // ===== LOGOUT =====
    if (button == &logoutButton)
    {
        backend.clearSession();
        if (onLogout) onLogout();
        return;
    }

    // ===== THEME MENU =====
    if (button == &themeButton)
    {
        juce::PopupMenu m;

        auto* laf = dynamic_cast<AppLookAndFeel*> (&getLookAndFeel());
        auto currentPreset = laf ? laf->getPreset() : AppLookAndFeel::Preset::Dark;

        m.addItem (1, "Dark",  true, currentPreset == AppLookAndFeel::Preset::Dark);
        m.addItem (2, "Light", true, currentPreset == AppLookAndFeel::Preset::Light);
        m.addItem (3, "Red",   true, currentPreset == AppLookAndFeel::Preset::Red);
        m.addItem (4, "Blue",  true, currentPreset == AppLookAndFeel::Preset::Blue);

        m.showMenuAsync (
            juce::PopupMenu::Options()
                .withTargetComponent (themeButton)
                .withPreferredPopupDirection (juce::PopupMenu::Options::PopupDirection::downwards),
            [this] (int result)
            {
                if (! onThemeSelected || result <= 0)
                    return;

                switch (result)
                {
                    case 1: onThemeSelected (AppLookAndFeel::Preset::Dark);  break;
                    case 2: onThemeSelected (AppLookAndFeel::Preset::Light); break;
                    case 3: onThemeSelected (AppLookAndFeel::Preset::Red);   break;
                    case 4: onThemeSelected (AppLookAndFeel::Preset::Blue);  break;
                    default: break;
                }
            });

        return;
    }

    // ===== LAYOUT MENU =====
    if (button == &layoutButton)
    {
        juce::PopupMenu m;

        m.addItem (1, "Layout 1", true, currentLayout == AppLookAndFeel::LayoutPreset::Layout1);
        m.addItem (2, "Layout 2", true, currentLayout == AppLookAndFeel::LayoutPreset::Layout2);
        m.addItem (3, "Layout 3", true, currentLayout == AppLookAndFeel::LayoutPreset::Layout3);
        m.addItem (4, "Layout 4", true, currentLayout == AppLookAndFeel::LayoutPreset::Layout4);

        m.showMenuAsync (
            juce::PopupMenu::Options()
                .withTargetComponent (layoutButton)
                .withPreferredPopupDirection (juce::PopupMenu::Options::PopupDirection::downwards),
            [this] (int result)
            {
                if (! onLayoutSelected || result <= 0)
                    return;

                switch (result)
                {
                    case 1: currentLayout = AppLookAndFeel::LayoutPreset::Layout1; onLayoutSelected (AppLookAndFeel::LayoutPreset::Layout1); break;
                    case 2: currentLayout = AppLookAndFeel::LayoutPreset::Layout2; onLayoutSelected (AppLookAndFeel::LayoutPreset::Layout2); break;
                    case 3: currentLayout = AppLookAndFeel::LayoutPreset::Layout3; onLayoutSelected (AppLookAndFeel::LayoutPreset::Layout3); break;
                    case 4: currentLayout = AppLookAndFeel::LayoutPreset::Layout4; onLayoutSelected (AppLookAndFeel::LayoutPreset::Layout4); break;
                    default: break;
                }
            });

        return;
    }
}
