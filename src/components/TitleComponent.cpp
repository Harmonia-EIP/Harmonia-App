#include "TitleComponent.h"
#include "../themes/AppColourIds.h"

TitleComponent::TitleComponent (const juce::String& textToDisplay, BackendManager& be)
    : backend (be), title (textToDisplay)
{
    // ===== TITLE =====
    titleLabel.setText (title, juce::dontSendNotification);
    titleLabel.setJustificationType (juce::Justification::centred);
    titleLabel.setFont (juce::Font (26.0f, juce::Font::bold));
    addAndMakeVisible (titleLabel);

    // ===== USER / PSEUDO =====
    auto sessionOpt = backend.loadSession();
    juce::String pseudo = sessionOpt.has_value() ? sessionOpt->pseudo : "Invité";

    pseudoLabel.setText (pseudo, juce::dontSendNotification);
    pseudoLabel.setFont (juce::Font (18.0f, juce::Font::plain));
    pseudoLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (pseudoLabel);

    // ===== THEME BUTTON =====
    themeButton.addListener (this);
    themeButton.setSize (90, 36);
    addAndMakeVisible (themeButton);

    // ===== LOGOUT BUTTON =====
    logoutButton.addListener (this);
    logoutButton.setSize (140, 36);
    addAndMakeVisible (logoutButton);
}

TitleComponent::~TitleComponent()
{
    themeButton.removeListener (this);
    logoutButton.removeListener (this);
}

void TitleComponent::paint (juce::Graphics& g)
{
    // Fond semi-transparent basé sur la couleur du thème
    auto bg = findColour (AppColourIds::panelBgId).withAlpha (0.5f);
    g.fillAll (bg);

    // Labels
    titleLabel.setColour (juce::Label::textColourId,
                          findColour (AppColourIds::textPrimaryId));

    pseudoLabel.setColour (juce::Label::textColourId,
                           findColour (AppColourIds::textSecondaryId));
}

void TitleComponent::resized()
{
    auto area = getLocalBounds().reduced (10);

    auto leftArea = area.removeFromLeft (240);
    pseudoLabel.setBounds (leftArea);

    auto rightArea = area.removeFromRight (280);

    auto logoutArea = rightArea.removeFromRight (180);
    logoutButton.setBounds (logoutArea.reduced (5));

    auto themeArea = rightArea.removeFromRight (100);
    themeButton.setBounds (themeArea.reduced (5));

    titleLabel.setBounds (area);
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

        // détecter le thème actuel depuis le LookAndFeel appliqué
        auto* laf = dynamic_cast<AppLookAndFeel*> (&getLookAndFeel());
        auto currentPreset = laf ? laf->getPreset() : AppLookAndFeel::Preset::Dark;

        m.addItem (1, "Dark", true, currentPreset == AppLookAndFeel::Preset::Dark);
        m.addItem (2, "Light", true, currentPreset == AppLookAndFeel::Preset::Light);
        m.addItem (3, "Red", true, currentPreset == AppLookAndFeel::Preset::Red);
        m.addItem (4, "Blue", true, currentPreset == AppLookAndFeel::Preset::Blue);

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
    }
}
