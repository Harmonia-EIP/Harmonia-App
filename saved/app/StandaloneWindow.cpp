#include "StandaloneWindow.h"

StandaloneWindow::StandaloneWindow()
    : DocumentWindow("HarmoniaApp",
                     juce::Colours::black,
                     juce::DocumentWindow::allButtons)
{
    setUsingNativeTitleBar(true);
}

void StandaloneWindow::closeButtonPressed()
{
    juce::JUCEApplication::getInstance()->systemRequestedQuit();
}