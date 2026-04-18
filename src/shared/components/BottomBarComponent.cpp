#include "BottomBarComponent.h"

BottomBarComponent::BottomBarComponent()
{
    addAndMakeVisible(loadButton);
    addAndMakeVisible(exportButton);
    addAndMakeVisible(resetMissingParamsToggle);
    addAndMakeVisible(generateButton);

    loadButton.setButtonText(Strings::Buttons::Load);
    exportButton.setButtonText(Strings::Buttons::Export);
    resetMissingParamsToggle.setButtonText(Strings::Buttons::ResetMissingParams);
    generateButton.setButtonText(Strings::Buttons::Generate);

    loadButton.onClick     = [this] { if (onLoadClicked)     onLoadClicked(); };
    exportButton.onClick   = [this] { if (onExportClicked)   onExportClicked(); };
    generateButton.onClick = [this] { if (onGenerateClicked) onGenerateClicked(); };
}

void BottomBarComponent::paint (juce::Graphics& g)
{
    
    g.fillAll (findColour (AppColourIds::panelBgId));

    g.setColour (findColour (AppColourIds::panelOutlineId));
    g.drawRect (getLocalBounds(), 1);

    auto textColour = findColour (AppColourIds::textPrimaryId);

    loadButton.setColour     (juce::TextButton::textColourOffId, textColour);
    exportButton.setColour   (juce::TextButton::textColourOffId, textColour);
    generateButton.setColour (juce::TextButton::textColourOffId, textColour);

    resetMissingParamsToggle.setColour (
        juce::ToggleButton::textColourId,
        findColour (AppColourIds::textSecondaryId));
}

void BottomBarComponent::resized()
{
    auto area = getLocalBounds().reduced(10, 1);
    const int h = juce::jmin(40, area.getHeight());

    juce::FlexBox row;
    row.flexDirection  = juce::FlexBox::Direction::row;
    row.justifyContent = juce::FlexBox::JustifyContent::flexStart;
    row.alignItems     = juce::FlexBox::AlignItems::center;

    row.items.add(juce::FlexItem(loadButton)
                    .withMinWidth(110).withHeight(h - 8));
    
    row.items.add(juce::FlexItem().withWidth(4));

    row.items.add(juce::FlexItem(exportButton)
                    .withMinWidth(110).withHeight(h - 8));

    row.items.add(juce::FlexItem(resetMissingParamsToggle)
                    .withMinWidth(220).withHeight(h - 8));

    row.items.add(juce::FlexItem().withFlex(1.0f));

    row.items.add(juce::FlexItem(generateButton)
                    .withMinWidth(140).withHeight(h - 8));

    row.performLayout(area);
}

bool BottomBarComponent::getResetToggleState() const
{
    return resetMissingParamsToggle.getToggleState();
}

void BottomBarComponent::setResetToggleState(bool enabled)
{
    resetMissingParamsToggle.setToggleState(enabled, juce::dontSendNotification);
}

juce::TextButton& BottomBarComponent::getLoadButton()     { return loadButton; }
juce::ToggleButton& BottomBarComponent::getResetToggle()  { return resetMissingParamsToggle; }
juce::TextButton& BottomBarComponent::getExportButton()   { return exportButton; }
juce::TextButton& BottomBarComponent::getGenerateButton() { return generateButton; }
