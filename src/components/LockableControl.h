/**
 * @file LockableControl.h
 * @brief Interface commune aux controles dont le parametre peut etre verrouille
 *        en mode Refine (KnobControl, selecteurs d'icones...).
 *
 * Etat visuel :
 * - lockUiVisible = false (mode Generate) : aucun cadenas, rendu normal.
 * - lockUiVisible = true  (mode Refine)   : cadenas ouvert discret,
 *                                           ou ferme + couleur "locked" si verrouille.
 *
 * L'etat "locked" persiste quand on repasse en Generate, il est juste masque.
 */
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/HarmoniaPalette.h"

class LockableControl
{
public:
    virtual ~LockableControl() = default;

    virtual void setLockUiVisible (bool visible) = 0;
    virtual void setLocked (bool shouldLock) = 0;
    virtual bool isLocked() const = 0;
    virtual juce::String getParamId() const = 0;

    std::function<void()> onLockChanged;

    /** Petit cadenas vectoriel. closed = verrouille (plein), sinon ouvert (contour). */
    static void drawPadlock (juce::Graphics& g, juce::Rectangle<float> a,
                             bool closed, juce::Colour colour)
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
            shackle.lineTo (cx + r, bodyTop);   // ouvert : la branche droite reste "levee"

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