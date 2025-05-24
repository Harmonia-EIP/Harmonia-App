#pragma once
#include "JuceHeader.h"
#include "MainComponent.h"

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(const juce::String& name, juce::Component* contentComponent, juce::JUCEApplication& app);
    ~MainWindow() override;

    void closeButtonPressed() override;

private:
    juce::JUCEApplication& app;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
};
