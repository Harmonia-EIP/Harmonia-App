#include "TopBarComponent.h"

TopBarComponent::TopBarComponent()
    : waveformSelector("Waveform")
{
    // Waveform
    waveformLabel.setText("Waveform", juce::dontSendNotification);
    waveformLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(waveformLabel);
    addAndMakeVisible(waveformSelector);

    // Filter
    filterLabel.setText("Filter", juce::dontSendNotification);
    filterLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(filterLabel);

    // Filter selector
    filterTypeSelector.addItem("Low Pass", 1);
    filterTypeSelector.addItem("High Pass", 2);
    filterTypeSelector.addItem("Band Pass", 3);
    filterTypeSelector.setSelectedId(1);
    addAndMakeVisible(filterTypeSelector);

    // Prompt
    promptLabel.setText("Prompt", juce::dontSendNotification);
    promptLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(promptLabel);

    promptEditor.setMultiLine(true);
    promptEditor.setReturnKeyStartsNewLine(true);
    promptEditor.setTextToShowWhenEmpty("Type your prompt here...", juce::Colours::grey);
    addAndMakeVisible(promptEditor);

    // Bouton clear
    addAndMakeVisible(clearPromptButton);
    clearPromptButton.onClick = [this]() { promptEditor.clear(); };
}

TopBarComponent::~TopBarComponent() {}

void TopBarComponent::paint(juce::Graphics& g)
{
    // fond si besoin
    g.fillAll(juce::Colours::transparentBlack);
}

void TopBarComponent::resized()
{
    auto area = getLocalBounds();
    int midX = area.getCentreX();   // milieu horizontal
    int labelHeight = 25;
    int compHeight = 30;
    int margin = 20;
    int space = 50;

    // ==== Zone gauche (waveform + filter) ====
    auto leftArea = area.removeFromLeft(midX);

    // Largeur pour chaque bloc
    int filterAndWaveformWidth = leftArea.getWidth() / 3;

    // Bloc Waveform
    auto waveformArea = leftArea.removeFromLeft(filterAndWaveformWidth).reduced(10);
    waveformLabel.setBounds(waveformArea.getX() + filterAndWaveformWidth / 4, margin, filterAndWaveformWidth, labelHeight);
    waveformSelector.setBounds(waveformArea.getX(), margin + labelHeight, filterAndWaveformWidth, compHeight);

    // Bloc Filter
    auto filterArea = leftArea.reduced(10);
    filterLabel.setBounds(filterArea.getX() + space + filterAndWaveformWidth / 4, margin, filterAndWaveformWidth, labelHeight);
    filterTypeSelector.setBounds(filterArea.getX() + space, margin + labelHeight, filterAndWaveformWidth, compHeight);

    // ==== Zone droite (prompt + bouton) ====
    auto rightArea = area; // ce qui reste
    int promptWidth = rightArea.getWidth() - 120; // on laisse 100px pour bouton

    // Bloc Prompt
    auto promptArea = rightArea.removeFromLeft(promptWidth).reduced(10);
    promptLabel.setBounds(promptArea.getX(), margin, promptWidth, labelHeight);
    promptEditor.setBounds(promptArea.getX(), margin + labelHeight, promptWidth, compHeight);

    // Bouton clear
    auto buttonArea = rightArea.reduced(10);
    clearPromptButton.setBounds(buttonArea.getX() + 20, margin + labelHeight, 70, compHeight);
}


