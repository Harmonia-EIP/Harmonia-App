#include "SynthComponent.h"

SynthComponent::SynthComponent()
    : keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    addAndMakeVisible(keyboardComponent);

    // Plage de notes visible sur le clavier (optionnel, ajustez comme vous voulez)
    keyboardComponent.setAvailableRange(36, 84);
}

void SynthComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::transparentBlack);
}

void SynthComponent::resized()
{
    keyboardComponent.setBounds(getLocalBounds());
}
