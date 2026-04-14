#include "SynthComponent.h"

SynthComponent::SynthComponent()
    : keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    addAndMakeVisible(keyboardComponent);

    keyboardComponent.setAvailableRange(0, 127);
    keyboardComponent.setScrollButtonsVisible(true);
}

void SynthComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::transparentBlack);
}

void SynthComponent::resized()
{
    keyboardComponent.setBounds(getLocalBounds());
}
