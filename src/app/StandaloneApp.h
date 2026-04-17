#pragma once
#include "../shared/JuceHeader.h"

class StandaloneApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override;
    const juce::String getApplicationVersion() override;

    void initialise(const juce::String&) override;
    void shutdown() override;

private:
    std::unique_ptr<juce::DocumentWindow> window;
};