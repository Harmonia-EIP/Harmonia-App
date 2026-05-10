#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/HarmoniaPalette.h"
#include "IconRenderer.h"
#include <functional>
#include <vector>

class IconChoiceSelector : public juce::Component,
                           private juce::AudioProcessorValueTreeState::Listener
{
public:
    using IconDrawer = std::function<void (juce::Graphics&, juce::Rectangle<float>, int, juce::Colour)>;

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

    void paint (juce::Graphics& g) override
    {
        if (numChoices <= 0) return;
        const auto r = getLocalBounds().toFloat().reduced (1.0f);
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
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        const int idx = indexAt (e.position);
        if (idx >= 0 && idx != currentIndex)
            writeIndex (idx);
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        const int idx = indexAt (e.position);
        if (idx != hoveredIndex) { hoveredIndex = idx; repaint(); }
    }

    void mouseExit (const juce::MouseEvent&) override
    {
        if (hoveredIndex != -1) { hoveredIndex = -1; repaint(); }
    }

private:
    int indexAt (juce::Point<float> p) const
    {
        if (numChoices <= 0) return -1;
        const float cellW = (float) getWidth() / (float) numChoices;
        const int idx = (int) (p.x / cellW);
        return juce::jlimit (0, numChoices - 1, idx);
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
};
