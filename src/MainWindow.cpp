#include "MainWindow.h"

MainWindow::MainWindow(const juce::String& name, juce::Component* contentComponent, juce::JUCEApplication& appRef)
    : juce::DocumentWindow(name,
                           juce::Desktop::getInstance().getDefaultLookAndFeel()
                               .findColour(juce::ResizableWindow::backgroundColourId),
                           juce::DocumentWindow::allButtons),
      app(appRef)
{
    setUsingNativeTitleBar(true);
    setContentOwned(contentComponent, true);
    centreWithSize(getWidth(), getHeight());
    setVisible(true);
    setResizable(true, true);
}

MainWindow::~MainWindow() = default;

void MainWindow::closeButtonPressed()
{
    app.systemRequestedQuit();
}
