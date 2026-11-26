#pragma once

#include <juce_graphics/juce_graphics.h>

#include <algorithm>
#include <cmath>

//==============================================================================
// Contraste WCAG 2.1 (voir CONTRASTE.md à la racine du dépôt).
//
//   L      = 0.2126 R + 0.7152 G + 0.0722 B   (canaux linéarisés)
//   ratio  = (L_clair + 0.05) / (L_sombre + 0.05)    de 1:1 à 21:1
//
// Une couleur transparente est d'abord posée sur son fond (alpha compositing),
// comme à l'écran. Seuils : 4.5:1 pour du texte normal, 3:1 pour du gros
// texte (>= 24 px, ou >= 18.7 px en gras) et pour les éléments non textuels.
namespace Contrast
{
    inline constexpr float minText      = 4.5f;
    inline constexpr float minLargeText = 3.0f;

    /** Couleur `fg` (éventuellement transparente) posée sur un fond opaque `bg`. */
    inline juce::Colour composite (juce::Colour fg, juce::Colour bg)
    {
        return bg.withAlpha (1.0f).overlaidWith (fg);
    }

    /** Luminance relative WCAG, de 0 (noir) à 1 (blanc). */
    inline double relativeLuminance (juce::Colour c)
    {
        auto channel = [] (juce::uint8 v)
        {
            const double s = v / 255.0;
            return s <= 0.03928 ? s / 12.92 : std::pow ((s + 0.055) / 1.055, 2.4);
        };

        return 0.2126 * channel (c.getRed())
             + 0.7152 * channel (c.getGreen())
             + 0.0722 * channel (c.getBlue());
    }

    /** Ratio de contraste de `fg` sur `bg`, de 1 à 21. */
    inline double ratio (juce::Colour fg, juce::Colour bg)
    {
        const auto l1 = relativeLuminance (composite (fg, bg));
        const auto l2 = relativeLuminance (bg.withAlpha (1.0f));

        return (std::max (l1, l2) + 0.05) / (std::min (l1, l2) + 0.05);
    }
}
