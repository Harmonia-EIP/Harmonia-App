#include "TopBarComponent.h"

TopBarComponent::TopBarComponent()
    : waveformSelector("Waveform")
{
    waveformLabel.setText("Waveform", juce::dontSendNotification);
    waveformLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(waveformLabel);
    addAndMakeVisible(waveformSelector);

    filterLabel.setText("Filter", juce::dontSendNotification);
    filterLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(filterLabel);

    filterTypeSelector.addItem("Low Pass", 1);
    filterTypeSelector.addItem("High Pass", 2);
    filterTypeSelector.addItem("Band Pass", 3);
    filterTypeSelector.setSelectedId(1);
    addAndMakeVisible(filterTypeSelector);

    promptLabel.setText("Prompt", juce::dontSendNotification);
    promptLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(promptLabel);

    promptEditor.setMultiLine(true);
    promptEditor.setReturnKeyStartsNewLine(true);
    promptEditor.setTextToShowWhenEmpty("Type your prompt here...", juce::Colours::grey);
    addAndMakeVisible(promptEditor);

    addAndMakeVisible(clearPromptButton);
    clearPromptButton.onClick = [this]() { promptEditor.clear(); };
}

TopBarComponent::~TopBarComponent() {}

juce::String TopBarComponent::getPrompt() const
{
    return promptEditor.getText();
}

juce::String TopBarComponent::getFilterType() const
{
    return filterTypeSelector.getText();
}

juce::String TopBarComponent::getWaveform() const
{
    return waveformSelector.getSelectedWaveform();
}

void TopBarComponent::setFilterType(juce::String filterName)
{
    auto name = filterName.trim().toLowerCase();

    if (name == "low pass")
        filterTypeSelector.setSelectedId(1);
    else if (name == "high pass")
        filterTypeSelector.setSelectedId(2);
    else if (name == "band pass")
        filterTypeSelector.setSelectedId(3);
    else
        jassertfalse;
}
    
void TopBarComponent::setPrompt(juce::String prompt)
{
    promptEditor.setText(prompt, juce::NotificationType::dontSendNotification);
}
   
void TopBarComponent::setWaveform(juce::String waveformName)
{
    auto name = waveformName.trim().toLowerCase();

    if (name == "sine")
        waveformSelector.setSelectedId(1);
    else if (name == "square")
        waveformSelector.setSelectedId(2);
    else if (name == "saw")
        waveformSelector.setSelectedId(3);
    else if (name == "triangle")
        waveformSelector.setSelectedId(4);
    else
        jassertfalse;
}

void TopBarComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::transparentBlack);
}

void TopBarComponent::resized()
{
    auto area = getLocalBounds();
    int midX = area.getCentreX();
    int labelHeight = 25;
    int compHeight = 30;
    int margin = 20;
    int space = 50;

    auto leftArea = area.removeFromLeft(midX);

    int filterAndWaveformWidth = leftArea.getWidth() / 3;

    auto waveformArea = leftArea.removeFromLeft(filterAndWaveformWidth).reduced(10);
    waveformLabel.setBounds(waveformArea.getX() + filterAndWaveformWidth / 4, margin, filterAndWaveformWidth, labelHeight);
    waveformSelector.setBounds(waveformArea.getX(), margin + labelHeight, filterAndWaveformWidth, compHeight);

    auto filterArea = leftArea.reduced(10);
    filterLabel.setBounds(filterArea.getX() + space + filterAndWaveformWidth / 4, margin, filterAndWaveformWidth, labelHeight);
    filterTypeSelector.setBounds(filterArea.getX() + space, margin + labelHeight, filterAndWaveformWidth, compHeight);

    auto rightArea = area;
    int promptWidth = rightArea.getWidth() - 120;

    auto promptArea = rightArea.removeFromLeft(promptWidth).reduced(10);
    promptLabel.setBounds(promptArea.getX(), margin, promptWidth, labelHeight);
    promptEditor.setBounds(promptArea.getX(), margin + labelHeight, promptWidth, compHeight);

    auto buttonArea = rightArea.reduced(10);
    clearPromptButton.setBounds(buttonArea.getX() + 20, margin + labelHeight, 70, compHeight);
}


