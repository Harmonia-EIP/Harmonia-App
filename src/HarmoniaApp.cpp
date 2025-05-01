#include "HarmoniaApp.h"

const juce::String HarmoniaApp::getApplicationName()    { return "HarmoniaApp"; }
const juce::String HarmoniaApp::getApplicationVersion() { return "0.1.0"; }

void HarmoniaApp::initialise(const juce::String&)
{
    mainWindow = std::make_unique<MainWindow>("HarmoniaApp");
}

void HarmoniaApp::shutdown()
{
    mainWindow = nullptr;
}
