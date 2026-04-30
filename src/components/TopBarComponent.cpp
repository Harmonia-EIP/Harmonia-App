#include "TopBarComponent.h"

TopBarComponent::TopBarComponent()
    : waveformSelector (Strings::Labels::Waveform)
{
    waveformLabel.setText (Strings::Labels::Waveform, juce::dontSendNotification);
    waveformLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (waveformLabel);
    addAndMakeVisible (waveformSelector);

    filterLabel.setText (Strings::Labels::Filter, juce::dontSendNotification);
    filterLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (filterLabel);

    filterTypeSelector.addItem (Strings::Filter::LowPass,  1);
    filterTypeSelector.addItem (Strings::Filter::HighPass, 2);
    filterTypeSelector.addItem (Strings::Filter::BandPass, 3);
    filterTypeSelector.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (filterTypeSelector);

    promptLabel.setText (Strings::Labels::Prompt, juce::dontSendNotification);
    promptLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (promptLabel);

    promptEditor.setMultiLine (false);
    promptEditor.setReturnKeyStartsNewLine (false);
    addAndMakeVisible (promptEditor);

    addAndMakeVisible (clearPromptButton);
    clearPromptButton.onClick = [this]()
    {
        promptEditor.clear();
        if (onParamsChanged) onParamsChanged();
    };

    waveformSelector.onChange = [this]()
    {
        if (onParamsChanged) onParamsChanged();
    };

    filterTypeSelector.onChange = [this]()
    {
        if (onParamsChanged) onParamsChanged();
    };

    promptEditor.onTextChange = [this]()
    {
        if (onParamsChanged) onParamsChanged();
    };
}

void TopBarComponent::lookAndFeelChanged()
{
    applyTheme();
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

int TopBarComponent::getWaveformIndex() const
{
    return waveformSelector.getSelectedId();
}

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

    if (name == Strings::Filter::LowPass.toLowerCase())        filterTypeSelector.setSelectedId (1, juce::dontSendNotification);
    else if (name == Strings::Filter::HighPass.toLowerCase())  filterTypeSelector.setSelectedId (2, juce::dontSendNotification);
    else if (name == Strings::Filter::BandPass.toLowerCase())  filterTypeSelector.setSelectedId (3, juce::dontSendNotification);
}

void TopBarComponent::setPrompt (juce::String prompt)
{
    promptEditor.setText (prompt, juce::dontSendNotification);
}

void TopBarComponent::setWaveform (juce::String waveformName)
{
    auto name = waveformName.trim().toLowerCase();

    if (name == Strings::Waveform::Sine.toLowerCase())           waveformSelector.setSelectedId (1, juce::dontSendNotification);
    else if (name == Strings::Waveform::Square.toLowerCase())    waveformSelector.setSelectedId (2, juce::dontSendNotification);
    else if (name == Strings::Waveform::Saw.toLowerCase())       waveformSelector.setSelectedId (3, juce::dontSendNotification);
    else if (name == Strings::Waveform::Triangle.toLowerCase())  waveformSelector.setSelectedId (4, juce::dontSendNotification);
}

void TopBarComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (AppColourIds::panelBgId));
    g.setColour (findColour (AppColourIds::panelOutlineId));
    g.drawRect (getLocalBounds(), 1);
}

void TopBarComponent::resized()
{
    auto area = getLocalBounds().reduced (8, 6);

    const int labelHeight     = 22;
    const int compHeight      = 26;
    const int marginTop       = 4;
    const int spacing         = 28;
    const int verticalSpacing = 10;

    const int halfWidth = area.getWidth() / 2;

    auto leftArea  = area.removeFromLeft (halfWidth);
    auto rightArea = area;

    const int controlWidth = leftArea.getWidth() / 2 - spacing;

    waveformLabel.setBounds (leftArea.getX(),
                             marginTop,
                             controlWidth,
                             labelHeight);

    waveformSelector.setBounds (leftArea.getX(),
                                marginTop + labelHeight + verticalSpacing,
                                controlWidth,
                                compHeight);

    filterLabel.setBounds (leftArea.getX() + controlWidth + spacing,
                           marginTop,
                           controlWidth,
                           labelHeight);

    filterTypeSelector.setBounds (leftArea.getX() + controlWidth + spacing,
                                  marginTop + labelHeight + verticalSpacing,
                                  controlWidth,
                                  compHeight);

    const int buttonWidth = 90;
    const int promptWidth = rightArea.getWidth() - buttonWidth - spacing;

    promptLabel.setBounds (rightArea.getX(),
                           marginTop,
                           promptWidth,
                           labelHeight);

    promptEditor.setBounds (rightArea.getX(),
                            marginTop + labelHeight + verticalSpacing,
                            promptWidth,
                            compHeight);

    clearPromptButton.setBounds (rightArea.getRight() - buttonWidth,
                                 marginTop + labelHeight + verticalSpacing,
                                 buttonWidth,
                                 compHeight);
}
