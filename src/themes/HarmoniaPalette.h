#pragma once
#include <juce_graphics/juce_graphics.h>

// Palette "Hive 2 + Apple modern dark" : sci-fi sobre avec accents multi-tons
// utilisés UNIQUEMENT pour différencier les sections (LEDs, underlines, visualizers).
// Les contrôles interactifs (knobs, sliders) restent uniformément cyan pour la
// cohérence d'usage.
namespace HarmoniaPalette
{
    // Surfaces
    inline const juce::Colour background     { 0xFF1E2226 };
    inline const juce::Colour panel          { 0xFF262A30 };
    inline const juce::Colour panelHi        { 0xFF2C3137 };
    inline const juce::Colour panelTop       { 0xFF2A2F35 }; // top edge for gradient
    inline const juce::Colour border         { 0xFF15181C };
    inline const juce::Colour borderHi       { 0xFF2A2F35 };

    // Texte
    inline const juce::Colour textPrimary    { 0xFFE0E5EA };
    inline const juce::Colour textSecondary  { 0xFFA0A5AB };
    inline const juce::Colour textMuted      { 0xFF6B7178 };

    // Accent primaire cyan (knobs, valeurs actives, focus)
    inline const juce::Colour accent         { 0xFF00D4FF };
    inline const juce::Colour accentSecond   { 0xFF00B5DB };
    inline const juce::Colour accentHover    { 0xFF4FE2FF };

    // Accents secondaires (par section, sobres)
    inline const juce::Colour violet         { 0xFFA78BFA }; // Filter
    inline const juce::Colour mint           { 0xFF34D399 }; // LFO
    inline const juce::Colour amber          { 0xFFFBBF24 }; // Amp envelope
    inline const juce::Colour rose           { 0xFFFB7185 }; // FX
    inline const juce::Colour warm           { 0xFFFB7185 }; // bipolaire négatif

    // Knob body
    inline const juce::Colour knobBg         { 0xFF1A1E22 };
    inline const juce::Colour knobBgHi       { 0xFF22262A };
    inline const juce::Colour knobTrack      { 0xFF15181C };

    // Display "screen"
    inline const juce::Colour screenBg       { 0xFF0A0D11 };
    inline const juce::Colour screenGrid     { 0xFF1F2730 };

    // Mappings sémantiques de section
    inline const juce::Colour sectionOsc       = accent;
    inline const juce::Colour sectionFilter    = violet;
    inline const juce::Colour sectionDisplay   = accent;
    inline const juce::Colour sectionLfo       = mint;
    inline const juce::Colour sectionFx        = rose;
    inline const juce::Colour sectionAmpEnv    = amber;
}
