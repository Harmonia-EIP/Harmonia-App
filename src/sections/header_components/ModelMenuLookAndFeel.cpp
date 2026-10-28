#include "ModelMenuLookAndFeel.h"
#include "../../themes/HarmoniaPalette.h"

namespace
{
    // même étincelle à 4 branches que le picker
    juce::Path star (juce::Point<float> c, float outer, float inner)
    {
        juce::Path p;

        for (int i = 0; i < 8; ++i)
        {
            const float a   = juce::MathConstants<float>::pi * 0.25f * (float) i
                                  - juce::MathConstants<float>::halfPi;
            const float rad = (i % 2 == 0) ? outer : inner;
            const juce::Point<float> q (c.x + std::cos (a) * rad,
                                        c.y + std::sin (a) * rad);

            if (i == 0) p.startNewSubPath (q);
            else        p.lineTo (q);
        }

        p.closeSubPath();
        return p;
    }
}

ModelMenuLookAndFeel::ModelMenuLookAndFeel()
{
    // fond transparent : le panneau est peint dans drawPopupMenuBackground
    // (nécessaire pour avoir de vrais coins arrondis)
    setColour (juce::PopupMenu::backgroundColourId, juce::Colours::transparentBlack);
}

// 0 : la fenêtre du menu fait exactement la largeur demandée (celle du picker)
int ModelMenuLookAndFeel::getPopupMenuBorderSize() { return 0; }

// pas d'ombre native : elle est rectangulaire et apparaît dans les coins arrondis
int ModelMenuLookAndFeel::getMenuWindowFlags() { return 0; }

void ModelMenuLookAndFeel::drawPopupMenuBackground (juce::Graphics& g, int w, int h)
{
    const auto b = juce::Rectangle<float> ((float) w, (float) h).reduced (0.5f);

    // même fond sombre que la capsule
    g.setColour (HarmoniaPalette::panel.darker (0.45f));
    g.fillRoundedRectangle (b, 10.0f);

    g.setColour (HarmoniaPalette::border);
    g.drawRoundedRectangle (b.reduced (0.5f), 10.0f, 1.0f);
}

void ModelMenuLookAndFeel::getIdealPopupMenuItemSize (const juce::String&, bool isSeparator, int,
                                                      int& idealWidth, int& idealHeight)
{
    // largeur minimale : c'est withMinimumWidth() qui décide, donc la largeur du picker.
    // Les noms trop longs sont tronqués avec "…" au dessin.
    idealWidth  = 50;
    idealHeight = isSeparator ? 8 : 36;
}

void ModelMenuLookAndFeel::drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                                              bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                                              bool, const juce::String& text,
                                              const juce::String&, const juce::Drawable*,
                                              const juce::Colour*)
{
    if (isSeparator)
    {
        g.setColour (HarmoniaPalette::border);
        g.fillRect (area.reduced (12, 0).withSizeKeepingCentre (area.getWidth() - 24, 1));
        return;
    }

    const auto r   = area.toFloat().reduced (5.0f, 1.5f);
    const auto acc = HarmoniaPalette::accent;

    // centre de la colonne d'icône, aligné sur l'étincelle du picker
    const juce::Point<float> ic ((float) area.getX() + (float) iconCol * 0.5f, r.getCentreY());

    // modèle actif : voile à peine teinté
    if (isTicked && ! isHighlighted)
    {
        g.setColour (acc.withAlpha (0.07f));
        g.fillRoundedRectangle (r, 7.0f);
    }

    // survol : lueur qui part de l'icône vers la droite (comme le picker), sans barre
    if (isHighlighted && isActive)
    {
        juce::Graphics::ScopedSaveState state (g);

        juce::Path clip;
        clip.addRoundedRectangle (r, 7.0f);
        g.reduceClipRegion (clip);

        g.setGradientFill (juce::ColourGradient (acc.withAlpha (isTicked ? 0.26f : 0.20f), ic.x, ic.y,
                                                 acc.withAlpha (0.0f), r.getRight(), ic.y,
                                                 false));
        g.fillRect (r);
    }

    // étincelle : pleine si actif, contour fantôme en aperçu au survol, rien sinon
    if (isTicked || (isHighlighted && isActive))
    {
        const auto sp = star (ic, 7.0f, 7.0f * 0.38f);

        if (isTicked)
        {
            g.setColour (acc.withAlpha (0.30f));
            g.fillPath (sp);
            g.setColour (acc.brighter (isHighlighted ? 0.15f : 0.0f));
        }
        else
        {
            g.setColour (acc.withAlpha (0.45f));
        }

        g.strokePath (sp, juce::PathStrokeType (1.3f, juce::PathStrokeType::mitered,
                                                juce::PathStrokeType::rounded));
    }

    // texte aligné sur le nom du modèle dans le picker
    auto col = isTicked || isHighlighted ? acc.brighter (0.3f)
                                         : HarmoniaPalette::textPrimary.withAlpha (0.8f);
    if (! isActive)
        col = col.withAlpha (0.4f);

    g.setColour (col);
    g.setFont (juce::Font (juce::FontOptions (12.5f).withStyle (isTicked ? "Bold" : "Regular")));
    g.drawText (text,
                juce::Rectangle<float> ((float) area.getX() + (float) iconCol + 6.0f, r.getY(),
                                        r.getRight() - ((float) area.getX() + (float) iconCol + 6.0f) - 8.0f,
                                        r.getHeight()),
                juce::Justification::centredLeft, true);
}