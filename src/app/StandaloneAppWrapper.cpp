#include "StandaloneAppWrapper.h"

StandaloneAppWrapper::StandaloneAppWrapper()
{
    core = std::make_unique<AppCore>();
    addAndMakeVisible(core.get());
}

void StandaloneAppWrapper::resized()
{
    if (core)
        core->setBounds(getLocalBounds());
}