#include "IconButton.h"

juce::Path IconButton::makePath (Icon i)
{
    juce::Path p;   // boîte 24x24

    switch (i)
    {
        case Icon::Home:
            p.startNewSubPath (3.0f, 11.5f);
            p.lineTo (12.0f, 3.5f);
            p.lineTo (21.0f, 11.5f);

            p.startNewSubPath (5.5f, 9.5f);
            p.lineTo (5.5f, 20.5f);
            p.lineTo (10.0f, 20.5f);
            p.lineTo (10.0f, 14.5f);
            p.lineTo (14.0f, 14.5f);
            p.lineTo (14.0f, 20.5f);
            p.lineTo (18.5f, 20.5f);
            p.lineTo (18.5f, 9.5f);
            break;

        case Icon::Import:
            p.startNewSubPath (12.0f, 4.0f);   p.lineTo (12.0f, 15.0f);
            p.startNewSubPath (7.5f, 10.5f);   p.lineTo (12.0f, 15.0f);  p.lineTo (16.5f, 10.5f);
            p.startNewSubPath (4.0f, 16.0f);   p.lineTo (4.0f, 20.0f);   p.lineTo (20.0f, 20.0f);  p.lineTo (20.0f, 16.0f);
            break;

        case Icon::Export:
            p.startNewSubPath (12.0f, 15.0f);  p.lineTo (12.0f, 4.0f);
            p.startNewSubPath (7.5f, 8.5f);    p.lineTo (12.0f, 4.0f);   p.lineTo (16.5f, 8.5f);
            p.startNewSubPath (4.0f, 16.0f);   p.lineTo (4.0f, 20.0f);   p.lineTo (20.0f, 20.0f);  p.lineTo (20.0f, 16.0f);
            break;
    }

    return p;
}

void IconButton::timerCallback()
{
    const float target = (isOver() || isDown()) ? 1.0f : 0.0f;
    hoverAnim += (target - hoverAnim) * 0.30f;

    if (std::abs (target - hoverAnim) < 0.01f)
    {
        hoverAnim = target;
        stopTimer();
    }

    repaint();
}

void IconButton::paintButton (juce::Graphics& g, bool, bool down)
{
    auto b = getLocalBounds().toFloat();
    const auto accent = HarmoniaPalette::accent;
    const float h = hoverAnim;

    if (h > 0.01f)
    {
        auto r = b.reduced (3.0f);

        // glow radial
        juce::ColourGradient glow (accent.withAlpha (0.30f * h), r.getCentre(),
                                   accent.withAlpha (0.0f),      r.getTopLeft(),
                                   true);
        g.setGradientFill (glow);
        g.fillRoundedRectangle (r, 8.0f);

        // ligne néon qui s'élargit
        const float w = r.getWidth() * 0.55f * h;
        g.setColour (accent.withAlpha (0.90f * h));
        g.fillRoundedRectangle (b.getCentreX() - w * 0.5f, b.getBottom() - 4.0f, w, 1.5f, 0.75f);
    }

    auto area = b.reduced (8.0f);
    if (down)
        area = area.reduced (1.0f);

    auto path = makePath (icon);
    path.applyTransform (path.getTransformToScaleToFit (area, true));

    g.setColour (HarmoniaPalette::textMuted.interpolatedWith (accent.brighter (0.3f), h));
    g.strokePath (path, juce::PathStrokeType (1.5f,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
}
