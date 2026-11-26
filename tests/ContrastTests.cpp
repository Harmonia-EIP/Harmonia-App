#include <catch2/catch_test_macros.hpp>

#include "themes/Contrast.h"
#include "themes/HarmoniaPalette.h"

// Règle du plugin (CONTRASTE.md) : avec les 3 couleurs par défaut de la palette,
// chaque texte garde un contraste WCAG >= 4.5:1 sur le fond où il est dessiné.
// Si une couleur choisie par l'utilisateur ne passe pas, c'est son choix.

namespace
{
    namespace P = HarmoniaPalette;

    struct Case { const char* what; juce::Colour fg, bg; };

    void checkAll (const std::vector<Case>& cases)
    {
        for (const auto& c : cases)
        {
            INFO (c.what << " : " << c.fg.toDisplayString (true) << " sur " << c.bg.toDisplayString (false)
                         << " = " << Contrast::ratio (c.fg, c.bg) << ":1");
            CHECK (Contrast::ratio (c.fg, c.bg) >= Contrast::minText);
        }
    }
}

TEST_CASE ("Contraste: la formule WCAG donne les valeurs de référence", "[contrast]")
{
    CHECK (Contrast::ratio (juce::Colours::black, juce::Colours::white) == 21.0);
    CHECK (Contrast::ratio (juce::Colours::white, juce::Colours::white) == 1.0);
    CHECK (std::abs (Contrast::ratio (juce::Colour (0xff767676), juce::Colours::white) - 4.54) < 0.01);

    // texte transparent : il est d'abord posé sur son fond
    CHECK (Contrast::ratio (juce::Colours::white.withAlpha (0.5f), juce::Colours::black)
           == Contrast::ratio (juce::Colour (0xff808080), juce::Colours::black));
}

TEST_CASE ("Contraste: les 3 couleurs par défaut passent WCAG AA", "[contrast]")
{
    for (const auto slot : P::defaultSlots)
    {
        DYNAMIC_SECTION ("accent " << slot.toDisplayString (false))
        {
            P::setCustomAccent (slot);

            const auto acc       = P::accent;
            const auto capsuleBg = P::panel.darker (0.55f);
            const auto tileBg    = P::panel.darker (0.45f)
                                       .overlaidWith (juce::Colours::black.withAlpha (0.25f))
                                       .overlaidWith (acc.withAlpha (0.05f));

            checkAll ({
                { "libellés des knobs",            P::textMuted,                   P::panel },
                { "valeur des knobs",              acc,                            P::panel },
                { "titre de section Osc/Display",  acc,                            P::panelTop },
                { "nom du preset",                 P::textMuted,                   P::panelTop },
                { "placeholder Generate",          P::textMuted.brighter (0.25f),  capsuleBg },
                { "placeholder Refine",            acc,                            capsuleBg },
                { "« AI MODEL »",                  P::textMuted,                   P::panelTop.overlaidWith (juce::Colours::black.withAlpha (0.28f)) },
                { "labels",                        P::textSecondary,               P::background },
                { "historique : titre",            P::textPrimary,                 tileBg },
                { "historique : modèle",           P::textPrimary.withAlpha (0.9f), tileBg },
                { "historique : date",             P::textSecondary,               tileBg },
                { "historique vide",               P::textMuted,                   P::panel.darker (0.45f) },
                { "« OSCILLOSCOPE »",              P::textMuted,                   P::screenBg },
                // clavier (touches sombres, HarmoniaKeyboard::applyColours)
                { "clavier : nom des do (C3...)",  P::textSecondary,               juce::Colour (0xFF2C3137) },
            });
        }
    }

    P::setCustomAccent (P::defaultSlots[0]);   // ne pas laisser l'état global modifié
}

TEST_CASE ("Contraste: les couleurs de section fixes passent WCAG AA", "[contrast]")
{
    checkAll ({
        { "section Filter", juce::Colour (0xFFA78BFA), P::panelTop },
        { "section LFO",    juce::Colour (0xFF34D399), P::panelTop },
        { "section FX",     juce::Colour (0xFFEB6F92), P::panelTop },
        { "section Amp",    juce::Colour (0xFFFBBF24), P::panelTop },
    });
}

TEST_CASE ("Contraste: lettres des touches du clavier", "[contrast]")
{
    const juce::Colour letter (0xFFE0E5EA);
    checkAll ({
        { "lettre sur touche blanche sombre", letter, juce::Colour (0xFF2C3137) },
        { "lettre sur touche noire",          letter, juce::Colour (0xFF0F1114) },
    });
}
