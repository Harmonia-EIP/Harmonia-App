#pragma once

#include "HeaderIncludes.h"
#include "HistoryItem.h"

//==============================================================================
// Carte d'historique, deux zones :
//   [BADGE]  Titre .................... (x)
//   ─────────────────────────────────────
//   ✦ Modèle ..................... date
// Titre long : retour à la ligne à droite du badge, maxLines lignes au plus ;
// au-delà, la dernière ligne finit par "…" et le titre complet est dans
// l'info-bulle. La ligne méta repart du bord gauche, sous le badge.
class HistoryTile : public juce::Component,
                    public juce::SettableTooltipClient
{
public:
    static constexpr int maxLines = 3;

    HistoryTile (int index, const HistoryItem& item);

    std::function<void (int)> onSelect, onDelete;

    // Calcule le retour à la ligne du titre pour cette largeur
    // et renvoie la hauteur dont la carte a besoin.
    int layoutForWidth (int width);

    void paint (juce::Graphics& g) override;
    void mouseEnter (const juce::MouseEvent& e) override;
    void mouseMove  (const juce::MouseEvent& e) override;
    void mouseExit  (const juce::MouseEvent& e) override;
    void mouseUp    (const juce::MouseEvent& e) override;
    void resized() override;

private:
    juce::Rectangle<int> crossBounds() const;
    void wrapTitle (int width);   // découpe le titre (maxLines, "…") + info-bulle
    int  metaTop() const;         // y du filet de la ligne méta
    void updateHover (const juce::MouseEvent& e);

    int index;                       // index dans la liste complète (pas dans le filtre)
    juce::String title, date, model;
    bool refined;
    juce::StringArray lines;         // titre déjà découpé
    bool hover = false, crossHover = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HistoryTile)
};