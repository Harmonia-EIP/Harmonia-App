#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

#include "HistoryTile.h"

//==============================================================================
// Panneau : [ recherche ........ ] [🗑]
//           [ liste défilable       ]   <- hauteur adaptée au contenu (minListH..listH)
class HistoryPanel : public juce::Component,
                     public juce::TooltipClient
{
public:
    static constexpr int panelW = 290;
    static constexpr int topH = 40, bottomH = 0;
    static constexpr int listH = 310;      // hauteur MAX de la liste
    static constexpr int minListH = 60;    // hauteur MIN
    static constexpr int pad = 10, gap = 6;
    static constexpr int clearSize = 26;   // bouton poubelle

    std::function<void (int)> onSelect, onDelete;
    std::function<void()> onClear, onClosed;

    HistoryPanel();
    ~HistoryPanel() override;

    void setItems (const juce::StringArray& newTitles,
                   const juce::StringArray& newTypes,
                   const juce::StringArray& newDates);

    juce::String getTooltip() override { return clearHover ? "Clear history" : juce::String(); }

    void paint (juce::Graphics& g) override;
    void resized() override;
    void mouseMove (const juce::MouseEvent& e) override;
    void mouseExit (const juce::MouseEvent& e) override;
    void mouseUp   (const juce::MouseEvent& e) override;

private:
    juce::Rectangle<int> searchBox() const;
    juce::Rectangle<int> clearZone() const;

    bool matches (int i, const juce::String& q) const;
    void rebuild();
    void layoutTiles();

    juce::StringArray titles, types, dates;
    bool clearHover = false;
    int  curListH = minListH;

    juce::TextEditor search;
    juce::Viewport viewport;
    juce::Component content;
    juce::OwnedArray<HistoryTile> tiles;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HistoryPanel)
};