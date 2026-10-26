/**
 * @file IconChoiceSelector.h
 * @brief Icon-based selector component for choice parameters.
 *
 * IconChoiceSelector provides a custom graphical alternative
 * to traditional combo boxes by rendering selectable icons.
 *
 * Features:
 * - Icon rendering callbacks
 * - Hover highlighting
 * - Active state visualization
 * - APVTS parameter synchronization
 * - Mouse interaction support
 * - Lock state (Refine mode):
 *     * lockStripH == 0 : padlock drawn on the selected icon, click the active cell to toggle
 *     * lockStripH  > 0 : padlock centred in a strip BELOW the icon box, click the strip to toggle
 */
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/HarmoniaPalette.h"
#include "IconRenderer.h"
#include "LockableControl.h"
#include <functional>
#include <vector>

class IconChoiceSelector : public juce::Component,
                           public LockableControl,
                           private juce::AudioProcessorValueTreeState::Listener
{
public:
    using IconDrawer = std::function<void (juce::Graphics&, juce::Rectangle<float>, int, juce::Colour)>;

    /** Hauteur recommandee de la bande cadenas (a ajouter a la hauteur du selecteur). */
    static constexpr int defaultLockStripH = 14;

    IconChoiceSelector (juce::AudioProcessorValueTreeState& apvts,
                        const juce::String& paramId,
                        IconDrawer drawer)
        : state (apvts), paramID (paramId), drawIcon (std::move (drawer))
    {
        if (auto* choice = dynamic_cast<juce::AudioParameterChoice*> (apvts.getParameter (paramId)))
            numChoices = choice->choices.size();

        state.addParameterListener (paramID, this);
        currentIndex = readIndex();
    }

    ~IconChoiceSelector() override
    {
        state.removeParameterListener (paramID, this);
    }

    /** Reserve une bande en bas du composant pour le cadenas (0 = cadenas sur l'icone). */
    void setLockStripHeight (int h)
    {
        lockStripH = juce::jmax (0, h);
        repaint();
    }

    void setLockUiVisible (bool visible) override
    {
        lockUiVisible = visible;

        if (! visible)
            setMouseCursor (juce::MouseCursor::NormalCursor);

        repaint();
    }

    void setLocked (bool shouldLock) override
    {
        if (locked == shouldLock)
            return;

        locked = shouldLock;
        repaint();

        if (onLockChanged)
            onLockChanged();
    }

    bool isLocked() const override              { return locked; }
    juce::String getParamId() const override    { return paramID; }

    void paint (juce::Graphics& g) override
    {
        if (numChoices <= 0) return;

        const auto r = boxBounds().toFloat().reduced (1.0f);
        const float corner = 6.0f;

        g.setColour (HarmoniaPalette::knobTrack);
        g.fillRoundedRectangle (r, corner);
        g.setColour (HarmoniaPalette::border);
        g.drawRoundedRectangle (r, corner, 1.0f);

        const float cellW = r.getWidth() / (float) numChoices;

        for (int i = 0; i < numChoices; ++i)
        {
            auto cell = juce::Rectangle<float> (r.getX() + i * cellW, r.getY(), cellW, r.getHeight())
                            .reduced (3.0f);

            const bool active = i == currentIndex;
            const bool hover  = i == hoveredIndex && ! active;

            if (active)
            {
                g.setColour (HarmoniaPalette::accent.withAlpha (0.18f));
                g.fillRoundedRectangle (cell, corner - 2.0f);
                g.setColour (HarmoniaPalette::accent.withAlpha (0.45f));
                g.drawRoundedRectangle (cell, corner - 2.0f, 1.0f);
            }
            else if (hover)
            {
                g.setColour (HarmoniaPalette::panelHi.withAlpha (0.6f));
                g.fillRoundedRectangle (cell, corner - 2.0f);
            }

            const auto iconCol = active ? HarmoniaPalette::accent
                                        : HarmoniaPalette::textSecondary;
            const auto iconArea = cell.reduced (cell.getWidth() * 0.18f, cell.getHeight() * 0.20f);
            drawIcon (g, iconArea, i, iconCol);
        }

        if (lockUiVisible)
        {
            LockableControl::drawPadlock (g, padlockRect(), locked,
                locked ? HarmoniaPalette::textPrimary
                       : HarmoniaPalette::textPrimary.withAlpha (0.45f),
                false);
        }
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (lockUiVisible && isInLockStrip (e.position))
        {
            setLocked (! locked);
            return;
        }

        const int idx = indexAt (e.position);

        if (lockUiVisible && lockStripH == 0 && idx == currentIndex)
        {
            setLocked (! locked);
            return;
        }

        if (idx >= 0 && idx != currentIndex)
            writeIndex (idx);
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        const int idx = indexAt (e.position);
        if (idx != hoveredIndex) { hoveredIndex = idx; repaint(); }

        const bool overLock = lockUiVisible
                              && (isInLockStrip (e.position)
                                  || (lockStripH == 0 && idx == currentIndex));

        setMouseCursor (overLock ? juce::MouseCursor::PointingHandCursor
                                 : juce::MouseCursor::NormalCursor);
    }

    void mouseExit (const juce::MouseEvent&) override
    {
        setMouseCursor (juce::MouseCursor::NormalCursor);
        if (hoveredIndex != -1) { hoveredIndex = -1; repaint(); }
    }

private:

    juce::Rectangle<int> boxBounds() const
    {
        return getLocalBounds().withTrimmedBottom (lockStripH);
    }

    bool isInLockStrip (juce::Point<float> p) const
    {
        return lockStripH > 0 && p.y >= (float) (getHeight() - lockStripH);
    }

    int indexAt (juce::Point<float> p) const
    {
        if (numChoices <= 0) return -1;
        if (isInLockStrip (p)) return -1;

        const float cellW = (float) getWidth() / (float) numChoices;
        const int idx = (int) (p.x / cellW);
        return juce::jlimit (0, numChoices - 1, idx);
    }

    juce::Rectangle<float> padlockRect() const
    {
        const juce::Rectangle<float> padlock (9.0f, 11.0f);

        if (lockStripH > 0)
        {
            const auto strip = getLocalBounds().toFloat().removeFromBottom ((float) lockStripH);

            const float gap = 3.0f;
            return padlock.withCentre ({ strip.getCentreX(),
                                        strip.getCentreY() + gap * 0.5f });
        }

        const auto r = boxBounds().toFloat().reduced (1.0f);
        const float cellW = r.getWidth() / (float) juce::jmax (1, numChoices);
        const auto cell = juce::Rectangle<float> (r.getX() + currentIndex * cellW,
                                                  r.getY(), cellW, r.getHeight());
        return padlock.withCentre (cell.getCentre());
    }

    int readIndex() const
    {
        if (auto* v = state.getRawParameterValue (paramID))
            return (int) v->load();
        return 0;
    }

    void writeIndex (int newIdx)
    {
        if (auto* p = state.getParameter (paramID))
        {
            const float norm = numChoices > 1 ? (float) newIdx / (float) (numChoices - 1) : 0.0f;
            p->beginChangeGesture();
            p->setValueNotifyingHost (norm);
            p->endChangeGesture();
        }
    }

    void parameterChanged (const juce::String&, float) override
    {
        juce::MessageManager::callAsync ([this]
        {
            currentIndex = readIndex();
            repaint();
        });
    }

    juce::AudioProcessorValueTreeState& state;
    juce::String paramID;
    IconDrawer drawIcon;
    int numChoices    = 0;
    int currentIndex  = 0;
    int hoveredIndex  = -1;

    int  lockStripH    = 0;
    bool locked        = false;
    bool lockUiVisible = false;
};