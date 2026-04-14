#include "TopBarComponent.h"

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

    if (name == "low pass")        filterTypeSelector.setSelectedId (1, juce::dontSendNotification);
    else if (name == "high pass")  filterTypeSelector.setSelectedId (2, juce::dontSendNotification);
    else if (name == "band pass")  filterTypeSelector.setSelectedId (3, juce::dontSendNotification);

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
    auto area = getLocalBounds().reduced (8, 6);  // un peu plus de marge globale

    const int labelHeight     = 22;   // plus grand
    const int compHeight      = 26;   // plus grand
    const int marginTop       = 4;   // plus d’espace en haut
    const int spacing         = 28;   // espace horizontal augmenté
    const int verticalSpacing = 10;   // plus d’espace entre label et composant

    const int halfWidth = area.getWidth() / 2;

    auto leftArea  = area.removeFromLeft (halfWidth);
    auto rightArea = area;

    // ===== LEFT SIDE =====
    const int controlWidth = leftArea.getWidth() / 2 - spacing;

    // Waveform
    waveformLabel.setBounds (leftArea.getX(),
                             marginTop,
                             controlWidth,
                             labelHeight);

    waveformSelector.setBounds (leftArea.getX(),
                                marginTop + labelHeight + verticalSpacing,
                                controlWidth,
                                compHeight);

    // Filter
    filterLabel.setBounds (leftArea.getX() + controlWidth + spacing,
                           marginTop,
                           controlWidth,
                           labelHeight);

    filterTypeSelector.setBounds (leftArea.getX() + controlWidth + spacing,
                                  marginTop + labelHeight + verticalSpacing,
                                  controlWidth,
                                  compHeight);

    // ===== RIGHT SIDE =====
    const int buttonWidth = 90;  // bouton un peu plus gros
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
