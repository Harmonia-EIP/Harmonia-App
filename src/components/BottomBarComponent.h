#pragma once
#include <JuceHeader.h>

class BottomBarComponent : public juce::Component
{
public:
    BottomBarComponent();
    ~BottomBarComponent() override = default;

    void resized() override;

    std::function<void()> onLoadClicked;
    std::function<void()> onExportClicked;
    std::function<void()> onGenerateClicked;

    bool getResetToggleState() const;
    void setResetToggleState(bool enabled);

    juce::TextButton&   getLoadButton();
    juce::ToggleButton& getResetToggle();
    juce::TextButton&   getGenerateButton();
    juce::TextButton&   getExportButton();

private:
    juce::TextButton   loadButton, generateButton, exportButton;
    juce::ToggleButton resetMissingParamsToggle;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BottomBarComponent)
};
