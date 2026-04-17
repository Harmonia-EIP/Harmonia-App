#include "StandaloneApp.h"
#include "StandaloneAppWrapper.h"
#include "StandaloneWindow.h"

const juce::String StandaloneApp::getApplicationName()    { return "HarmoniaApp"; }
const juce::String StandaloneApp::getApplicationVersion() { return "0.3.1"; }

void StandaloneApp::initialise(const juce::String&)
{
    window = std::make_unique<StandaloneWindow>();

    window->setUsingNativeTitleBar(true);

    window->setContentOwned(new StandaloneAppWrapper(), true);

    window->centreWithSize(900, 700);
    window->setResizable(true, true);
    window->setVisible(true);
}

void StandaloneApp::shutdown()
{
    window = nullptr;
}
