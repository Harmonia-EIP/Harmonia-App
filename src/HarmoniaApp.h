#pragma once
#include <JuceHeader.h>
#include "MainWindow.h"      
#include "BackendAuthManager.h"

class HarmoniaApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override;
    const juce::String getApplicationVersion() override;
    void initialise(const juce::String&) override;
    void shutdown() override;

private:
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<BackendAuthManager> backend;
};
