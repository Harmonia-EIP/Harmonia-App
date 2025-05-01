#pragma once
#include <juce_events/juce_events.h>
#include "MainWindow.h"

class HarmoniaApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override;
    const juce::String getApplicationVersion() override;
    void initialise(const juce::String&) override;
    void shutdown() override;

private:
    std::unique_ptr<MainWindow> mainWindow;
};
