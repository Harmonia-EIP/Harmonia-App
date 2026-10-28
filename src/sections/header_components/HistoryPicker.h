#pragma once

#include "HeaderIncludes.h"

#include <functional>

class HistoryOverlay;

//==============================================================================
// Capsule "cartes empilées + nombre de générations". Un clic ouvre le panneau
// d'historique dans un calque attaché à la fenêtre racine (pas de CallOutBox).
// Figée (setInteractive (false)) : invité, ou historique vide.
class HistoryPicker : public juce::Component,
                      public juce::SettableTooltipClient,
                      private juce::Timer
{
public:
    HistoryPicker();
    ~HistoryPicker() override;

    void setItems (const juce::StringArray& newTitles,
                   const juce::StringArray& newTypes,
                   const juce::StringArray& newDates);

    void setInteractive (bool shouldBeInteractive);

    std::function<void (int)> onSelect;
    std::function<void (int)> onDelete;
    std::function<void()> onClear;

    void paint (juce::Graphics&) override;
    void mouseEnter (const juce::MouseEvent&) override { startTimerHz (60); }
    void mouseExit  (const juce::MouseEvent&) override { startTimerHz (60); }
    void mouseDown  (const juce::MouseEvent&) override { if (interactive) showMenu(); }

private:
    void timerCallback() override;
    void showMenu();
    void closeMenu();
    void removeEntry (int index);
    void clearAll();
    juce::String getValueText() const;

    static juce::String clean (const juce::String& s);

    juce::StringArray titles;
    juce::StringArray types;
    juce::StringArray dates;

    juce::Component::SafePointer<HistoryOverlay> overlay;

    std::function<void (const juce::StringArray&, const juce::StringArray&,
                        const juce::StringArray&)> pushToPanel;

    bool menuOpen = false;
    bool interactive = true;
    float hoverAnim = 0.0f;
    float openAnim  = 0.0f;
};