#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
// Petite icône "shuffle" discrète : pas de fond, pas de cadre. Elle est
// atténuée au repos, s'allume au survol et fait un petit "pop" au clic.
// Aucune logique de randomisation ici : elle déclenche seulement onClick
// (la génération est dans ParamRandomizer).
//==============================================================================
class RandomizeButton : public juce::Button,
                        private juce::Timer
{
public:
    RandomizeButton() : juce::Button ("randomize")
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
        setTooltip ("Randomize all unlocked parameters");
    }

    void setTint (juce::Colour c)
    {
        if (tint == c)
            return;

        tint = c;
        repaint();
    }

    void clicked() override
    {
        popStartMs = juce::Time::getMillisecondCounterHiRes();
        pop = 0.0f;
        startTimerHz (60);
    }

    void paintButton (juce::Graphics& g, bool isOver, bool isDown) override
    {
        const auto b = getLocalBounds().toFloat();

        // "pop" : l'icône grossit un peu puis revient (0 -> 1 -> 0)
        const float bump  = std::sin (pop * juce::MathConstants<float>::pi);
        const float scale = 1.0f + 0.30f * bump;

        const float size = juce::jmin (b.getWidth(), b.getHeight()) * 0.66f * scale;

        const float alpha = isDown ? 1.0f : (isOver ? 0.95f : 0.45f);
        const auto  col   = isOver || isDown ? tint.brighter (0.4f) : tint;

        drawShuffleIcon (g, b.getCentre(), size, col.withAlpha (alpha));
    }

private:
    // Deux brins qui se croisent + pointes de flèche, dessinés dans une boîte 24x24.
    static void drawShuffleIcon (juce::Graphics& g, juce::Point<float> centre,
                                 float size, juce::Colour colour)
    {
        juce::Path strands;
        strands.startNewSubPath (2.0f, 7.0f);
        strands.lineTo (6.0f, 7.0f);
        strands.cubicTo (12.0f, 7.0f, 12.0f, 17.0f, 18.0f, 17.0f);

        strands.startNewSubPath (2.0f, 17.0f);
        strands.lineTo (6.0f, 17.0f);
        strands.cubicTo (12.0f, 17.0f, 12.0f, 7.0f, 18.0f, 7.0f);

        juce::Path heads;
        heads.addTriangle (17.0f, 13.5f, 22.0f, 17.0f, 17.0f, 20.5f);
        heads.addTriangle (17.0f,  3.5f, 22.0f,  7.0f, 17.0f, 10.5f);

        const auto t = juce::AffineTransform::translation (-12.0f, -12.0f)
                           .scaled (size / 24.0f)
                           .translated (centre.x, centre.y);
        strands.applyTransform (t);
        heads.applyTransform (t);

        g.setColour (colour);
        g.strokePath (strands, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                                     juce::PathStrokeType::rounded));
        g.fillPath (heads);
    }

    void timerCallback() override
    {
        constexpr double durationMs = 280.0;
        const double t = (juce::Time::getMillisecondCounterHiRes() - popStartMs) / durationMs;

        if (t >= 1.0)
        {
            pop = 0.0f;
            stopTimer();
        }
        else
        {
            pop = (float) t;
        }

        repaint();
    }

    juce::Colour tint { juce::Colours::white };
    float        pop = 0.0f;
    double       popStartMs = 0.0;
};