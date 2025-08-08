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

    juce::String getPrompt() const;

    juce::String getFilterType() const;

    juce::String getWaveform() const;

    void setFilterType(juce::String);
    void setPrompt(juce::String);
    void setWaveform(juce::String);

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
