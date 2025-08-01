#pragma once
#include <JuceHeader.h>
#include "../WaveformSelector.h"

class TopBarComponent : public juce::Component
{
public:
    TopBarComponent();
    ~TopBarComponent() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    WaveformSelector waveformSelector;
    juce::ComboBox filterTypeSelector;
    juce::TextEditor promptEditor;
    juce::TextButton clearPromptButton { "Clear" };

private:
    juce::Label waveformLabel;
    juce::Label promptLabel;
    juce::Label filterLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopBarComponent)
};
