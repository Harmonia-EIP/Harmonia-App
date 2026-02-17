#include "TopBarComponent.h"
#include "../themes/AppColourIds.h"

TopBarComponent::TopBarComponent()
    : waveformSelector ("Waveform")
{
    waveformLabel.setText ("Waveform", juce::dontSendNotification);
    waveformLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (waveformLabel);
    addAndMakeVisible (waveformSelector);

    filterLabel.setText ("Filter", juce::dontSendNotification);
    filterLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (filterLabel);

    filterTypeSelector.addItem ("Low Pass",  1);
    filterTypeSelector.addItem ("High Pass", 2);
    filterTypeSelector.addItem ("Band Pass", 3);
    filterTypeSelector.setSelectedId (1);
    addAndMakeVisible (filterTypeSelector);

    promptLabel.setText ("Prompt", juce::dontSendNotification);
    promptLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (promptLabel);

    promptEditor.setMultiLine (true);
    promptEditor.setReturnKeyStartsNewLine (true);
    addAndMakeVisible (promptEditor);

    clearPromptButton.setButtonText ("Clear");
    addAndMakeVisible (clearPromptButton);
    clearPromptButton.onClick = [this]() { promptEditor.clear(); };

    applyTheme();
}

TopBarComponent::~TopBarComponent() {}

//==============================================================================
void TopBarComponent::lookAndFeelChanged()
{
    applyTheme();
    repaint();
}

void TopBarComponent::applyTheme()
{
    auto textPrimary   = findColour (AppColourIds::textPrimaryId);
    auto textSecondary = findColour (AppColourIds::textSecondaryId);

    waveformLabel.setColour (juce::Label::textColourId, textPrimary);
    filterLabel.setColour   (juce::Label::textColourId, textPrimary);
    promptLabel.setColour   (juce::Label::textColourId, textPrimary);

    promptEditor.setTextToShowWhenEmpty ("Type your prompt here...", textSecondary);
}

//==============================================================================
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

void TopBarComponent::setFilterType (juce::String filterName)
{
    auto name = filterName.trim().toLowerCase();

    if (name == "low pass")      filterTypeSelector.setSelectedId (1);
    else if (name == "high pass") filterTypeSelector.setSelectedId (2);
    else if (name == "band pass") filterTypeSelector.setSelectedId (3);
    else                         jassertfalse;
}

void TopBarComponent::setPrompt (juce::String prompt)
{
    promptEditor.setText (prompt, juce::dontSendNotification);
}

void TopBarComponent::setWaveform (juce::String waveformName)
{
    auto name = waveformName.trim().toLowerCase();

    if (name == "sine")       waveformSelector.setSelectedId (1);
    else if (name == "square")   waveformSelector.setSelectedId (2);
    else if (name == "saw")      waveformSelector.setSelectedId (3);
    else if (name == "triangle") waveformSelector.setSelectedId (4);
    else                         jassertfalse;
}

//==============================================================================
void TopBarComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (AppColourIds::panelBgId));

    g.setColour (findColour (AppColourIds::panelOutlineId));
    g.drawRect (getLocalBounds(), 1);
}

void TopBarComponent::resized()
{
    auto area = getLocalBounds();
    const int midX        = area.getCentreX();
    const int labelHeight = 25;
    const int compHeight  = 30;
    const int margin      = 20;
    const int space       = 50;

    auto leftArea = area.removeFromLeft (midX);

    const int filterAndWaveformWidth = leftArea.getWidth() / 3;

    auto waveformArea = leftArea.removeFromLeft (filterAndWaveformWidth).reduced (10);
    waveformLabel.setBounds (waveformArea.getX() + filterAndWaveformWidth / 4,
                             margin,
                             filterAndWaveformWidth,
                             labelHeight);
    waveformSelector.setBounds (waveformArea.getX(),
                                margin + labelHeight,
                                filterAndWaveformWidth,
                                compHeight);

    auto filterArea = leftArea.reduced (10);
    filterLabel.setBounds (filterArea.getX() + space + filterAndWaveformWidth / 4,
                           margin,
                           filterAndWaveformWidth,
                           labelHeight);
    filterTypeSelector.setBounds (filterArea.getX() + space,
                                  margin + labelHeight,
                                  filterAndWaveformWidth,
                                  compHeight);

    auto rightArea = area;
    const int promptWidth = rightArea.getWidth() - 120;

    auto promptArea = rightArea.removeFromLeft (promptWidth).reduced (10);
    promptLabel.setBounds (promptArea.getX(),
                           margin,
                           promptWidth,
                           labelHeight);
    promptEditor.setBounds (promptArea.getX(),
                            margin + labelHeight,
                            promptWidth,
                            compHeight);

    auto buttonArea = rightArea.reduced (10);
    clearPromptButton.setBounds (buttonArea.getX() + 20,
                                 margin + labelHeight,
                                 70,
                                 compHeight);
}
