#include "MainComponent.h"

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (AppColourIds::backgroundId));
}


void MainComponent::applyTheme(ThemePreset theme)
{
    appLookAndFeel.setThemePreset(theme);
    sendLookAndFeelChange();
    repaint();
}
