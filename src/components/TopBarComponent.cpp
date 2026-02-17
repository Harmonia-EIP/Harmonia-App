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
    filterTypeSelector.setSelectedId (1, juce::dontSendNotification);
    addAndMakeVisible (filterTypeSelector);

    promptLabel.setText ("Prompt", juce::dontSendNotification);
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

    // ===== IMPORTANT : notifier MainComponent sur changement user =====
    waveformSelector.onChange = [this]()
    {
        if (onParamsChanged) onParamsChanged();
    };

    filterTypeSelector.onChange = [this]()
    {
        if (onParamsChanged) onParamsChanged();
    };

    // Optionnel : si vous voulez update en live pendant qu’on tape
    // (sinon vous pouvez le retirer)
    promptEditor.onTextChange = [this]()
    {
        if (onParamsChanged) onParamsChanged();
    };

    applyTheme();
}

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

int TopBarComponent::getWaveformIndex() const
{
    // ATTENTION :
    // Si votre WaveformSelector renvoie 1=Sine,2=Square,3=Saw,4=Triangle
    // et que votre synth attend 0=Sine,1=Saw,2=Square
    // alors il faut mapper dans updateSynthParamsFromUI (je vous montre plus bas).
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

    if (name == "low pass")        filterTypeSelector.setSelectedId (1, juce::dontSendNotification);
    else if (name == "high pass")  filterTypeSelector.setSelectedId (2, juce::dontSendNotification);
    else if (name == "band pass")  filterTypeSelector.setSelectedId (3, juce::dontSendNotification);

    // IMPORTANT : si vous voulez que le load JSON déclenche aussi une update, appelez :
    // if (onParamsChanged) onParamsChanged();
}

void TopBarComponent::setPrompt (juce::String prompt)
{
    promptEditor.setText (prompt, juce::dontSendNotification);
    // idem : if (onParamsChanged) onParamsChanged(); (optionnel)
}

void TopBarComponent::setWaveform (juce::String waveformName)
{
    auto name = waveformName.trim().toLowerCase();

    if (name == "sine")           waveformSelector.setSelectedId (1, juce::dontSendNotification);
    else if (name == "square")    waveformSelector.setSelectedId (2, juce::dontSendNotification);
    else if (name == "saw")       waveformSelector.setSelectedId (3, juce::dontSendNotification);
    else if (name == "triangle")  waveformSelector.setSelectedId (4, juce::dontSendNotification);
}

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
