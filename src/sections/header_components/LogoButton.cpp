#include "LogoButton.h"

void LogoButton::setRefine (bool)
{
    // Le logo ne change plus selon le mode (conservé pour compatibilité)
}

void LogoButton::timerCallback()
{
    const float targetHover = (isOver() || isDown()) ? 1.0f : 0.0f;
    hoverAnim += (targetHover - hoverAnim) * 0.30f;

    bool busy = std::abs (targetHover - hoverAnim) >= 0.01f;

    if (! busy)
        hoverAnim = targetHover;

    if (spinning)
    {
        spinProgress += 0.035f;          // ~0.5 s pour un tour : augmente = plus rapide

        if (spinProgress >= 1.0f)
        {
            spinProgress = 0.0f;
            spinning = false;
        }
        else
        {
            busy = true;
        }
    }

    if (! busy)
        stopTimer();

    repaint();
}

void LogoButton::paintButton (juce::Graphics& g, bool, bool down)
{
    auto b = getLocalBounds().toFloat().reduced (1.0f);

    // fond + glow au survol
    g.setColour (juce::Colours::black.withAlpha (0.28f));
    g.fillEllipse (b);

    if (hoverAnim > 0.01f)
    {
        juce::ColourGradient glow (tint.withAlpha (0.30f * hoverAnim), b.getCentre(),
                                   tint.withAlpha (0.0f), b.getTopLeft(), true);
        g.setGradientFill (glow);
        g.fillEllipse (b);
    }

    // anneau plein (toujours, quel que soit le mode)
    g.setColour (tint.withAlpha (0.85f));
    g.drawEllipse (b, 1.4f);

    // logo : un tour complet sur lui-même au clic (ease-out)
    if (logo.isValid())
    {
        auto area = b.reduced (down ? 6.5f : 5.0f);

        juce::Graphics::ScopedSaveState state (g);

        if (spinning)
        {
            const float inv   = 1.0f - spinProgress;
            const float eased = 1.0f - inv * inv * inv;
            g.addTransform (juce::AffineTransform::rotation (eased * juce::MathConstants<float>::twoPi,
                                                             b.getCentreX(), b.getCentreY()));
        }

        g.setOpacity (0.8f + 0.2f * hoverAnim);
        g.drawImage (logo, area, juce::RectanglePlacement::centred);
    }
}
