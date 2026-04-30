#include "StandaloneApp.h"
#include "StandaloneAppWrapper.h"
#include "StandaloneWindow.h"
#include "../config/AppConfig.h"

const juce::String StandaloneApp::getApplicationName()    { return AppConfig::AppName; }
const juce::String StandaloneApp::getApplicationVersion() { return AppConfig::Version; }

void StandaloneApp::initialise(const juce::String&)
{
    window = std::make_unique<StandaloneWindow>();

    window->setUsingNativeTitleBar(true);

    window->setContentOwned(new StandaloneAppWrapper(), true);

    window->centreWithSize(AppConfig::DefaultWidth, AppConfig::DefaultHeight);
    window->setResizable(true, true);
    window->setVisible(true);
}

void StandaloneApp::shutdown()
{
    window = nullptr;
}
