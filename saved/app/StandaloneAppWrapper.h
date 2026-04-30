#pragma once
#include "../shared/JuceHeader.h"
#include "../shared/AppCore.h"

class StandaloneAppWrapper : public juce::Component
{
public:
    StandaloneAppWrapper();

    void resized() override;

private:
    std::unique_ptr<AppCore> core;
};