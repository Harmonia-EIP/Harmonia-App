#pragma once

#include "ComponentsIncludes.h"

// Composant UI pour le synthé : pour l'instant un clavier MIDI à l'écran
class SynthComponent : public juce::Component
{
public:
    SynthComponent();
    ~SynthComponent() override = default;

    // Permet au MainComponent de récupérer l'état du clavier
    juce::MidiKeyboardState& getKeyboardState() noexcept { return keyboardState; }

    void paint(juce::Graphics& g) override;
    void resized() override;

private:
    juce::MidiKeyboardState keyboardState;
    juce::MidiKeyboardComponent keyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthComponent)
};
