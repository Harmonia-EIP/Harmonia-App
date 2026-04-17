#pragma once
#include "../shared/JuceHeader.h"

class StandaloneWindow : public juce::DocumentWindow
{
public:
    StandaloneWindow();

    void closeButtonPressed() override;
};