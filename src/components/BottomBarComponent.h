#pragma once
#include <JuceHeader.h>

class BottomBarComponent : public juce::Component
{
public:
    BottomBarComponent();
    ~BottomBarComponent() override = default;

    void resized() override;

    std::function<void()> onLoadClicked;
    std::function<void()> onGenerateClicked;

    bool getResetToggleState() const;
    void setResetToggleState(bool enabled);

    juce::TextButton&   getLoadButton();
    juce::ToggleButton& getResetToggle();
    juce::TextButton&   getGenerateButton();

private:
    juce::TextButton   loadButton;
    juce::ToggleButton resetMissingParamsToggle;
    juce::TextButton   generateButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BottomBarComponent)
};
