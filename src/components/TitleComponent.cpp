#include "TitleComponent.h"

TitleComponent::TitleComponent(const juce::String& textToDisplay)
    : title(textToDisplay)
{
    addAndMakeVisible(label);

    label.setText(title, juce::dontSendNotification);
    label.setJustificationType(juce::Justification::centred);
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    label.setFont(juce::Font(24.0f, juce::Font::bold));
}

void TitleComponent::resized()
{
    label.setBounds(getLocalBounds());
}
