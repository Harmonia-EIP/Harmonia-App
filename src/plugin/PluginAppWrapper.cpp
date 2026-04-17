#include "PluginAppWrapper.h"

PluginAppWrapper::PluginAppWrapper()
{
    core = std::make_unique<AppCore>();
    addAndMakeVisible(core.get());
}

void PluginAppWrapper::resized()
{
    if (core)
        core->setBounds(getLocalBounds());
}