#pragma once

#include "HeaderIncludes.h"

//==============================================================================
// Champ de prompt avec placeholder dessiné à la main (style différent selon le mode).
// Affiché seulement quand le champ est vide ET sans focus.
class PromptEditor : public juce::TextEditor
{
public:
    void setHint (const juce::String& text, juce::Colour colour, bool monospace);

    std::function<void()> onFocusChanged;

    void paint (juce::Graphics&) override;
    void focusGained (FocusChangeType) override;
    void focusLost (FocusChangeType) override;

private:
    juce::String hint;
    juce::Colour hintCol { juce::Colours::grey };
    bool mono = false;
};
