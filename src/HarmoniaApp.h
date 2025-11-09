#pragma once
#include <JuceHeader.h>
#include "MainWindow.h"          // ✅ ajoute cette ligne
#include "BackendAuthManager.h"

class HarmoniaApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override;
    const juce::String getApplicationVersion() override;
    void initialise(const juce::String&) override;
    void shutdown() override;

private:
    std::unique_ptr<MainWindow> mainWindow;   // ✅ retire "class"
    std::unique_ptr<BackendAuthManager> backend;
};
