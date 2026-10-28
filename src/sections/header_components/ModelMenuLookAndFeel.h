#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

//==============================================================================
// Look & feel du menu déroulant des modèles IA (uniquement pour ce menu).
// Le menu a exactement la largeur du ModelPicker (voir ModelPicker::showMenu)
// et reprend sa grille : pastille alignée sur l'icône, texte aligné sur le nom.
class ModelMenuLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModelMenuLookAndFeel();

    // largeur de la colonne "icône" du picker, pour aligner pastille et texte
    void setIconColumnWidth (int w) { iconCol = w; }

    int getPopupMenuBorderSize() override;

    int getMenuWindowFlags() override;

    void drawPopupMenuBackground (juce::Graphics&, int w, int h) override;

    void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator, int standardHeight,
                                    int& idealWidth, int& idealHeight) override;

    void drawPopupMenuItem (juce::Graphics&, const juce::Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                            bool hasSubMenu, const juce::String& text,
                            const juce::String& shortcut, const juce::Drawable* icon,
                            const juce::Colour* textColour) override;

private:
    int iconCol = 40;
};