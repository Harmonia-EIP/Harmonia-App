#include "SynthComponent.h"

SynthComponent::SynthComponent()
    : keyboardComponent(keyboardState, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    addAndMakeVisible(keyboardComponent);

    keyboardComponent.setAvailableRange(AppConfig::Synth::minimumAvailableRange, AppConfig::Synth::maximumAvailableRange);
    keyboardComponent.setScrollButtonsVisible(false);
}

void SynthComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::transparentBlack);
}

void SynthComponent::resized()
{
    keyboardComponent.setBounds(getLocalBounds());
}
