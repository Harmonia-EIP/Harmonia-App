#pragma once

#include <juce_core/juce_core.h>

#include <vector>

//==============================================================================
// Une ligne de l'historique telle qu'affichée (déjà formatée par le header) :
// le picker, le panneau et les cartes se passent cette structure plutôt que
// des listes parallèles (titres / types / dates / modèles).
struct HistoryItem
{
    juce::String title;   // nom du preset (ou type en repli), jamais vide
    juce::String type;    // "generate" / "refine"
    juce::String date;    // "28 Oct 14:03" (heure locale)
    juce::String model;   // nom affiché du modèle IA ("Harmonia V3", "Latest"...)
};

using HistoryItems = std::vector<HistoryItem>;
