/**
 * @file LockableControl.h
 * @brief Interface commune aux controles dont le parametre peut etre verrouille
 *        en mode Refine (KnobControl, selecteurs d'icones...).
 *
 * Etat visuel :
 * - lockUiVisible = false (mode Generate) : aucun cadenas, rendu normal.
 * - lockUiVisible = true  (mode Refine)   : cadenas ouvert discret,
 *                                           ou ferme + lumineux si verrouille.
 *
 * L'etat "locked" persiste quand on repasse en Generate, il est juste masque.
 */
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/HarmoniaPalette.h"
#include <functional>

class LockableControl
{
public:
    virtual ~LockableControl() = default;

    virtual void setLockUiVisible (bool visible) = 0;
    virtual void setLocked (bool shouldLock) = 0;
    virtual bool isLocked() const = 0;
    virtual juce::String getParamId() const = 0;

    std::function<void()> onLockChanged;

    /** Petit cadenas vectoriel. closed = verrouille (plein), sinon ouvert (contour).
        Quand il est ferme, une lueur serree (~1px) est dessinee autour de la forme. */
    static void drawPadlock (juce::Graphics& g, juce::Rectangle<float> a,
                             bool closed, juce::Colour colour, bool glow = true)
    {
        const float bodyTop = a.getY() + a.getHeight() * 0.45f;
        const auto  body    = juce::Rectangle<float> (a.getX(), bodyTop,
                                                      a.getWidth(), a.getBottom() - bodyTop);

        const float r     = a.getWidth() * 0.30f;
        const float cx    = a.getCentreX();
        const float arcCy = a.getY() + r + 0.5f;

        juce::Path shackle;
        shackle.startNewSubPath (cx - r, bodyTop);
        shackle.lineTo (cx - r, arcCy);
        shackle.addCentredArc (cx, arcCy, r, r, 0.0f,
                               -juce::MathConstants<float>::halfPi,
                                juce::MathConstants<float>::halfPi, false);

        if (closed)
            shackle.lineTo (cx + r, bodyTop);

        
        if (closed && glow)
        {
            g.setColour (colour.withAlpha (0.22f));
            g.strokePath (shackle, juce::PathStrokeType (3.0f,
                                                         juce::PathStrokeType::curved,
                                                         juce::PathStrokeType::rounded));
            g.fillRoundedRectangle (body.expanded (1.0f), 2.2f);
        }

        g.setColour (colour);
        g.strokePath (shackle, juce::PathStrokeType (1.3f,
                                                     juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));

        if (closed)
            g.fillRoundedRectangle (body, 1.5f);
        else
            g.drawRoundedRectangle (body.reduced (0.6f), 1.5f, 1.2f);
    }
};