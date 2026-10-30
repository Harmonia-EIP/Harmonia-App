#pragma once

#include "HeaderIncludes.h"

//==============================================================================
// Carte d'historique, une seule ligne par défaut :
//   [ barre ] [BADGE]  Titre ............  date  (x)
// Si le titre est trop long, il passe à la ligne (3 lignes max) à droite du
// badge ; badge, date et croix restent alignés sur la première ligne.
class HistoryTile : public juce::Component
{
public:
    static constexpr int maxLines = 3;

    HistoryTile (int index, const juce::String& title, const juce::String& date, bool refined);

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
    void updateHover (const juce::MouseEvent& e);

    int index;                       // index dans la liste complète (pas dans le filtre)
    juce::String title, date;
    bool refined;
    juce::StringArray lines;         // titre déjà découpé
    bool hover = false, crossHover = false;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (HistoryTile)
};