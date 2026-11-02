#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>
#include <vector>

#include "ModelMenuLookAndFeel.h"

//==============================================================================
// Sélecteur de modèle IA : capsule + menu déroulant de même largeur.
// L'icône vit directement sur le fond sombre de la capsule (pas de bloc séparé).
// Peut être "figé" (setInteractive (false)) : invité, ou liste pas encore chargée.
class ModelPicker : public juce::Component,
                    public juce::SettableTooltipClient
{
public:
    ModelPicker() { setMouseCursor (juce::MouseCursor::PointingHandCursor); }

    void addItem (const juce::String& name, int id) { items.push_back ({ name, id }); }
    void clearItems()                               { items.clear(); selectedId = 0; repaint(); }
    void setSelectedId (int id)                     { selectedId = id; repaint(); }
    int  getSelectedId() const                      { return selectedId; }

    // Texte affiché quand aucun modèle n'est sélectionnable
    void setPlaceholder (const juce::String& text)  { placeholder = text; repaint(); }

    // false = capsule figée : pas de menu, pas de survol, icône immobile
    void setInteractive (bool shouldBeInteractive)
    {
        interactive = shouldBeInteractive;
        setMouseCursor (interactive ? juce::MouseCursor::PointingHandCursor
                                    : juce::MouseCursor::NormalCursor);
        repaint();
    }

    std::function<void()> onChange;

    void paint (juce::Graphics&) override;
    void mouseDown (const juce::MouseEvent&) override { if (interactive) showMenu(); }

private:
    struct Item { juce::String name; int id; };

    void update();   // appelé à chaque rafraîchissement d'écran (VBlank)
    void showMenu();
    juce::String getSelectedName() const;

    std::vector<Item> items;
    ModelMenuLookAndFeel menuLnF;
    juce::String placeholder;
    int selectedId = 0;
    bool menuOpen = false;
    bool interactive = true;

    // animation (basée sur le temps réel, fluide quelle que soit la fréquence d'écran)
    float  hoverAnim = 0.0f;   // 0..1, survol / menu ouvert
    float  phase     = 0.0f;   // respiration (radians)
    float  burst     = 0.0f;   // petit "pop" au changement de modèle
    float  rot       = 0.0f;   // angle courant de l'étincelle
    float  rotTarget = 0.0f;   // angle visé : +90° à chaque survol / changement
    bool   wasHot    = false;
    double lastMs    = 0.0;

    // Synchronisé sur le rafraîchissement de l'écran : doit rester le dernier membre
    juce::VBlankAttachment vblank { this, [this] { update(); } };
};