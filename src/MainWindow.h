#pragma once
#include <juce_gui_extra/juce_gui_extra.h>
#include "MainComponent.h"

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(juce::String name);
    void closeButtonPressed() override;
};
