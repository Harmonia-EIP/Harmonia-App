#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "HistoryPanel.h"

// Calque transparent qui couvre le parent, héberge le HistoryPanel sous le bouton
// et se ferme au clic à l'extérieur. Pas de chevron : le bouton allumé suffit.
class HistoryOverlay : public juce::Component
{
public:
    static constexpr int panelGap = 10;   // espace entre le bouton et le panneau

    HistoryOverlay (juce::Component& parent, juce::Component& anchor)
    {
        setOpaque (false);
        parent.addAndMakeVisible (this);
        setBounds (parent.getLocalBounds());
        toFront (true);

        panel = std::make_unique<HistoryPanel>();
        addAndMakeVisible (*panel);

        const auto a = parent.getLocalArea (&anchor, anchor.getLocalBounds());
        const int x = juce::jlimit (6, juce::jmax (6, getWidth() - panel->getWidth() - 6),
                                    a.getCentreX() - panel->getWidth() / 2);
        panel->setTopLeftPosition (x, a.getBottom() + panelGap);   // le haut reste fixe
    }

    HistoryPanel& getPanel() { return *panel; }

    // gardé pour ne pas casser l'appel dans HistoryPicker : ne fait plus rien
    void setNotchColours (juce::Colour, juce::Colour) {}

    void dismiss()
    {
        juce::MessageManager::callAsync ([safe = SafePointer<HistoryOverlay> (this)]
        {
            delete safe.getComponent();
        });
    }

    void paint (juce::Graphics& g) override
    {
        juce::Path p;
        p.addRoundedRectangle (panel->getBounds().toFloat(), 10.0f);
        juce::DropShadow (juce::Colours::black.withAlpha (0.45f), 16, { 0, 4 }).drawForPath (g, p);
    }

    void childBoundsChanged (juce::Component*) override { repaint(); }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (! panel->getBounds().contains (e.getPosition()))
            dismiss();
    }

    void parentSizeChanged() override
    {
        if (auto* p = getParentComponent())
            setBounds (p->getLocalBounds());
    }

private:
    std::unique_ptr<HistoryPanel> panel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HistoryOverlay)
};