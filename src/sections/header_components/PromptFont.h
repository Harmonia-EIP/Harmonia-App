#pragma once

#include "HeaderIncludes.h"

// Police de la barre de prompt : UNE seule source de vérité pour le texte saisi
// et le placeholder, en Generate comme en Refine.
// La police monospace paraît plus petite que la police normale à hauteur égale :
// on corrige son échelle pour que la hauteur de lettres soit la même qu'en Generate.
namespace PromptFont
{
    inline constexpr float size = 14.0f;   // <- LA taille, pour les deux modes

    // Échelle du mono par rapport à Generate. 1.0 = même valeur numérique (14).
    //   - le texte Refine paraît trop GROS  -> baisse (0.94f, 0.90f, ...)
    //   - le texte Refine paraît trop PETIT -> monte  (1.04f, 1.08f, ...)
    inline constexpr float monoScale = 0.90f;

    // Espacement des lettres du mono (proportion de la taille de police).
    //   - encore trop espacé -> plus négatif (-0.10f, -0.14f, ...)
    //   - trop serré         -> plus proche de 0 (-0.03f, 0.0f)
    inline constexpr float monoKerning = -0.07f;

    inline juce::Font make (bool mono)
    {
        if (! mono)
            return juce::Font (juce::FontOptions (size));

        return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(),
                                              size * monoScale,
                                              juce::Font::plain))
                   .withExtraKerningFactor (monoKerning);
    }
}
