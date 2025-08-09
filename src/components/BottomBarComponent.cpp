#include "BottomBarComponent.h"

BottomBarComponent::BottomBarComponent()
{
    addAndMakeVisible(loadButton);
    addAndMakeVisible(resetMissingParamsToggle);
    addAndMakeVisible(generateButton);

    loadButton.setButtonText("Load");
    resetMissingParamsToggle.setButtonText("Default if missing");
    generateButton.setButtonText("Generate");

    loadButton.onClick     = [this] { if (onLoadClicked)      onLoadClicked(); };
    generateButton.onClick = [this] { if (onGenerateClicked)  onGenerateClicked(); };
}

void BottomBarComponent::resized()
{
    auto area = getLocalBounds().reduced(10, 0);
    const int h = juce::jmin(40, area.getHeight());

    juce::FlexBox row;
    row.flexDirection  = juce::FlexBox::Direction::row;
    row.justifyContent = juce::FlexBox::JustifyContent::spaceBetween;
    row.alignItems     = juce::FlexBox::AlignItems::center;

    row.items.add(juce::FlexItem(loadButton)
                    .withMinWidth(110).withHeight(h - 8));
    row.items.add(juce::FlexItem(resetMissingParamsToggle)
                    .withMinWidth(220).withHeight(h - 8));

    // Espace flexible pour pousser "Generate" à droite
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

juce::TextButton& BottomBarComponent::getLoadButton()        { return loadButton; }
juce::ToggleButton& BottomBarComponent::getResetToggle()     { return resetMissingParamsToggle; }
juce::TextButton& BottomBarComponent::getGenerateButton()    { return generateButton; }
