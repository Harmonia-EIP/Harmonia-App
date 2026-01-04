#include "TitleComponent.h"

TitleComponent::TitleComponent(const juce::String& textToDisplay, BackendManager& be)
    : backend(be), title(textToDisplay)
{
    titleLabel.setText(title, juce::dontSendNotification);
    titleLabel.setJustificationType(juce::Justification::centred);
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setFont(juce::Font(26.0f, juce::Font::bold));
    addAndMakeVisible(titleLabel);

    auto sessionOpt = backend.loadSession();
    juce::String pseudo = sessionOpt.has_value() ? sessionOpt->pseudo : "Invité";

    pseudoLabel.setText(pseudo, juce::dontSendNotification);
    pseudoLabel.setFont(juce::Font(18.0f, juce::Font::plain));
    pseudoLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    pseudoLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(pseudoLabel);

    logoutButton.setButtonText("Deconnexion");
    logoutButton.addListener(this);
    logoutButton.setColour(juce::TextButton::buttonColourId, juce::Colours::darkred);
    logoutButton.setColour(juce::TextButton::textColourOffId, juce::Colours::white);
    logoutButton.setColour(juce::TextButton::textColourOnId, juce::Colours::white);
    logoutButton.setSize(140, 36);
    addAndMakeVisible(logoutButton);
}

TitleComponent::~TitleComponent()
{
    logoutButton.removeListener(this);
}

void TitleComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black.withAlpha(0.5f));
}

void TitleComponent::resized()
{
    auto area = getLocalBounds().reduced(10);
    auto leftArea = area.removeFromLeft(240);
    pseudoLabel.setBounds(leftArea);
    auto rightArea = area.removeFromRight(180);
    logoutButton.setBounds(rightArea.reduced(5));
    titleLabel.setBounds(area);
}

void TitleComponent::buttonClicked(juce::Button* button)
{
    if (button == &logoutButton)
    {
        backend.clearSession();
        if (onLogout)
            onLogout(); 
    }
}

