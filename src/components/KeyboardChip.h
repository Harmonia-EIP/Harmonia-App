#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

#include "../themes/HarmoniaPalette.h"

/**
 * @class KeyboardChip
 * @brief Petite pastille cliquable de la ligne de titre du clavier.
 *
 *   Stepper :  ( ◂  C3  ▸ )   clic à gauche = onStep (-1), à droite = onStep (+1)
 *   Bouton  :  (  AZERTY  )   clic n'importe où = onClick
 *
 * La molette fait aussi onStep (stepper uniquement).
 */
class KeyboardChip : public juce::Component,
                     public juce::SettableTooltipClient
{
public:
    enum class Kind { Stepper, Button };

    explicit KeyboardChip (Kind k) : kind (k)
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    std::function<void (int)> onStep;   // Stepper : -1 / +1
    std::function<void()>     onClick;  // Button

    void setText (const juce::String& newText)
    {
        if (newText != text) { text = newText; repaint(); }
    }

    const juce::String& getText() const noexcept { return text; }

    void setHighlighted (bool h) { if (h != highlighted) { highlighted = h; repaint(); } }

    /** Largeur idéale pour son texte le plus long attendu. */
    int idealWidth (const juce::String& longestText) const
    {
        return (int) std::ceil (juce::GlyphArrangement::getStringWidth (font(), longestText))
             + (kind == Kind::Stepper ? 2 * arrowZone : 0) + 16;
    }

    void paint (juce::Graphics& g) override
    {
        const auto b = getLocalBounds().toFloat().reduced (0.5f);
        const float r = b.getHeight() * 0.5f;
        const auto acc = HarmoniaPalette::accent;

        g.setColour (HarmoniaPalette::knobBg);
        g.fillRoundedRectangle (b, r);

        if (hoverZone != 0)
        {
            g.setColour (acc.withAlpha (0.14f));
            if (kind == Kind::Button)
                g.fillRoundedRectangle (b, r);
            else
                g.fillRoundedRectangle (hoverZone < 0 ? b.withWidth ((float) arrowZone + r)
                                                      : b.withLeft (b.getRight() - (float) arrowZone - r), r);
        }

        g.setColour (highlighted ? acc.withAlpha (0.65f) : juce::Colour (0xFF3A4148));
        g.drawRoundedRectangle (b, r, 1.0f);

        g.setFont (font());
        g.setColour (highlighted ? acc : HarmoniaPalette::textPrimary);
        g.drawText (text, b, juce::Justification::centred, false);

        if (kind == Kind::Stepper)
        {
            g.setColour (HarmoniaPalette::textSecondary);
            const float cy = b.getCentreY(), s = 3.2f;

            juce::Path left, right;
            const float lx = b.getX() + (float) arrowZone * 0.6f;
            left.addTriangle (lx - s, cy, lx + s * 0.6f, cy - s, lx + s * 0.6f, cy + s);

            const float rx = b.getRight() - (float) arrowZone * 0.6f;
            right.addTriangle (rx + s, cy, rx - s * 0.6f, cy - s, rx - s * 0.6f, cy + s);

            g.fillPath (left);
            g.fillPath (right);
        }
    }

    void mouseUp (const juce::MouseEvent& e) override
    {
        if (! getLocalBounds().contains (e.getPosition()))
            return;

        if (kind == Kind::Button) { if (onClick) onClick(); return; }
        if (onStep) onStep (e.x < getWidth() / 2 ? -1 : +1);
    }

    void mouseWheelMove (const juce::MouseEvent&, const juce::MouseWheelDetails& w) override
    {
        if (kind == Kind::Stepper && onStep && w.deltaY != 0.0f)
            onStep (w.deltaY > 0.0f ? +1 : -1);
    }

    void mouseMove (const juce::MouseEvent& e) override  { setHover (e.x); }
    void mouseEnter (const juce::MouseEvent& e) override { setHover (e.x); }
    void mouseExit (const juce::MouseEvent&) override    { hoverZone = 0; repaint(); }

private:
    static constexpr int arrowZone = 14;

    static juce::Font font() { return juce::Font (juce::FontOptions (10.5f).withStyle ("Bold")); }

    void setHover (int x)
    {
        const int z = kind == Kind::Button ? 1 : (x < getWidth() / 2 ? -1 : +1);
        if (z != hoverZone) { hoverZone = z; repaint(); }
    }

    Kind kind;
    juce::String text;
    bool highlighted = false;
    int hoverZone = 0;   // -1 gauche, +1 droite (ou bouton), 0 aucun

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (KeyboardChip)
};
