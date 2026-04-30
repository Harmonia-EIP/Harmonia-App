#include "SynthComponent.h"

SynthComponent::SynthComponent(juce::MidiKeyboardState& state)
    : keyboardState(state),
      keyboardComponent(state, juce::MidiKeyboardComponent::horizontalKeyboard)
{
    addAndMakeVisible(keyboardComponent);
}

void SynthComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void SynthComponent::resized()
{
    keyboardComponent.setBounds(getLocalBounds());
}