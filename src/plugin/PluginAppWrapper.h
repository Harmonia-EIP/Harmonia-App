#pragma once
#include "JucePluginHeader.h"
#include "../shared/AppCore.h"

class PluginAppWrapper : public juce::Component
{
public:
    PluginAppWrapper();

    void resized() override;

private:
    std::unique_ptr<AppCore> core;
};