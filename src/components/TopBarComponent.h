#pragma once

#include "ComponentsIncludes.h"

class TopBarComponent : public juce::Component
{
public:
    TopBarComponent();
    ~TopBarComponent() override = default;

    void paint (juce::Graphics&) override;
    void lookAndFeelChanged() override;
    void resized() override;

    void applyTheme();

    juce::String getPrompt() const;
    juce::String getFilterType() const;
    juce::String getWaveform() const;
    int getWaveformIndex() const;

    void setFilterType (juce::String);
    void setPrompt (juce::String);
    void setWaveform (juce::String);

    std::function<void()> onParamsChanged;

    WaveformSelector waveformSelector;
    juce::ComboBox filterTypeSelector;
    juce::TextEditor promptEditor;
    juce::TextButton clearPromptButton { Strings::Buttons::Clear };

private:
    juce::Label waveformLabel;
    juce::Label promptLabel;
    juce::Label filterLabel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (TopBarComponent)
};
