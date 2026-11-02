#include "ModelPicker.h"
#include "../../themes/HarmoniaPalette.h"

namespace
{
    constexpr float textGap = 6.0f;   // espace entre la colonne de l'icône et le texte

    // étincelle à 4 branches (symétrique tous les 90°)
    juce::Path star (juce::Point<float> c, float outer, float inner, float rot)
    {
        juce::Path p;

        for (int i = 0; i < 8; ++i)
        {
            const float a   = rot + juce::MathConstants<float>::pi * 0.25f * (float) i
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

juce::String ModelPicker::getSelectedName() const
{
    for (const auto& it : items)
        if (it.id == selectedId)
            return it.name;

    return placeholder;
}

void ModelPicker::update()
{
    const double now = juce::Time::getMillisecondCounterHiRes();
    const float  dt  = lastMs > 0.0 ? juce::jlimit (0.0f, 0.05f, (float) ((now - lastMs) * 0.001)) : 0.0f;
    lastMs = now;

    // lissage exponentiel indépendant de la fréquence d'images
    const bool  hot    = interactive && (isMouseOver() || menuOpen);
    const float target = hot ? 1.0f : 0.0f;
    hoverAnim += (target - hoverAnim) * (1.0f - std::exp (-dt * 14.0f));

    if (std::abs (target - hoverAnim) < 0.001f)
        hoverAnim = target;

    // petit quart de tour à chaque entrée de souris
    if (hot && ! wasHot)
        rotTarget += juce::MathConstants<float>::halfPi;

    wasHot = hot;

    if (interactive)
    {
        phase += dt * juce::MathConstants<float>::twoPi / 3.4f;   // une respiration ≈ 3,4 s
        if (phase > juce::MathConstants<float>::twoPi)
            phase -= juce::MathConstants<float>::twoPi;

        // ease-out vers l'angle visé : démarre vite, se pose en douceur
        rot += (rotTarget - rot) * (1.0f - std::exp (-dt * 7.0f));

        // garde les angles petits (l'étoile est identique tous les 90°)
        if (rotTarget > juce::MathConstants<float>::twoPi)
        {
            rotTarget -= juce::MathConstants<float>::twoPi;
            rot       -= juce::MathConstants<float>::twoPi;
        }

        burst *= std::exp (-dt * 5.0f);
        if (burst < 0.002f)
            burst = 0.0f;
    }

    if (interactive || hoverAnim > 0.0f)
        repaint();
}

void ModelPicker::showMenu()
{
    if (! interactive || items.empty())
        return;

    const auto b = getLocalBounds().toFloat().reduced (1.0f);

    juce::PopupMenu menu;
    menu.setLookAndFeel (&menuLnF);
    menuLnF.setIconColumnWidth ((int) b.getHeight());   // pastille / texte alignés sur le picker

    for (const auto& it : items)
        menu.addItem (it.id, it.name, true, it.id == selectedId);

    menuOpen = true;

    // le menu se cale sur la capsule : même bord gauche, même largeur, petit espace dessous
    menu.showMenuAsync (juce::PopupMenu::Options()
                            .withTargetScreenArea (getScreenBounds().reduced (1).translated (0, 4))
                            .withPreferredPopupDirection (juce::PopupMenu::Options::PopupDirection::downwards)
                            .withMinimumWidth (getWidth() - 2)
                            .withMaximumNumColumns (1)
                            .withStandardItemHeight (36),
                        [safe = juce::Component::SafePointer<ModelPicker> (this)] (int result)
    {
        if (safe == nullptr)
            return;

        safe->menuOpen = false;

        if (result != 0 && result != safe->selectedId)
        {
            safe->selectedId = result;
            safe->burst = 1.0f;                                       // petit "pop"
            safe->rotTarget += juce::MathConstants<float>::halfPi;    // + un quart de tour
            safe->repaint();

            if (safe->onChange)
                safe->onChange();
        }
    });
}

void ModelPicker::paint (juce::Graphics& g)
{
    const auto acc  = HarmoniaPalette::accent;
    const float h   = hoverAnim;
    const float r   = 10.0f;
    const float dim = interactive ? 1.0f : 0.55f;   // capsule figée : plus discrète

    const auto b = getLocalBounds().toFloat().reduced (1.0f);

    // l'icône est centrée dans une colonne carrée à gauche, mais sans bloc ni séparateur
    const juce::Point<float> ic (b.getX() + b.getHeight() * 0.5f, b.getCentreY());

    juce::Path shape;
    shape.addRoundedRectangle (b, r);

    // fond sombre unique
    g.setColour (juce::Colours::black.withAlpha (0.28f));
    g.fillPath (shape);

    {
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (shape);

        // lueur qui part de l'icône vers la droite au survol
        if (h > 0.01f)
        {
            g.setGradientFill (juce::ColourGradient (acc.withAlpha (0.16f * h), ic.x,       ic.y,
                                                     acc.withAlpha (0.0f),      b.getRight(), ic.y,
                                                     false));
            g.fillRect (b);
        }

        // fine barre d'accent qui s'étire depuis le centre
        if (h > 0.01f)
        {
            const float w = b.getWidth() * h;
            g.setColour (acc.withAlpha (0.9f * h));
            g.fillRect (b.getCentreX() - w * 0.5f, b.getBottom() - 1.5f, w, 1.5f);
        }
    }

    // bordure
    g.setColour (HarmoniaPalette::border.interpolatedWith (acc.withAlpha (0.85f), h));
    g.drawRoundedRectangle (b.reduced (0.5f), r, 1.0f);

    // ---------- ICÔNE : respiration + quart de tour au survol ----------
    const float breathe = 0.5f + 0.5f * std::sin (phase);
    const float scale   = 1.0f + 0.06f * breathe + 0.30f * burst;
    const float outer   = 8.0f * scale;
    const float inner   = outer * (0.36f + 0.06f * h);   // les branches s'ouvrent un peu au survol
    const float haloR   = juce::jmin (b.getHeight() * 0.5f, 20.0f);

    // halo doux directement sur le fond sombre
    {
        const float a = ((0.08f + 0.08f * breathe) + 0.14f * h + 0.45f * burst) * dim;
        g.setGradientFill (juce::ColourGradient (acc.withAlpha (a),    ic.x,         ic.y,
                                                 acc.withAlpha (0.0f), ic.x + haloR, ic.y,
                                                 true));
        g.fillEllipse (juce::Rectangle<float> (haloR * 2.0f, haloR * 2.0f).withCentre (ic));
    }

    const auto star4 = star (ic, outer, inner, rot);

    g.setColour (acc.withAlpha ((0.15f + 0.20f * h) * dim));
    g.fillPath (star4);

    g.setColour (acc.brighter (0.15f * h + 0.3f * burst).withAlpha (dim));
    g.strokePath (star4, juce::PathStrokeType (1.4f, juce::PathStrokeType::mitered,
                                               juce::PathStrokeType::rounded));

    // ---------- TEXTES ----------
    const int textX = (int) (b.getX() + b.getHeight() + textGap + 2.0f * h);   // léger glissement au survol
    const int textR = (int) b.getRight() - 12;
    const int textW = juce::jmax (0, textR - textX);

    constexpr int labelH = 10, nameH = 15;
    const int top = (getHeight() - (labelH + nameH)) / 2;

    g.setColour (HarmoniaPalette::textMuted.interpolatedWith (acc, h * 0.6f));
    g.setFont (juce::Font (juce::FontOptions (8.0f).withStyle ("Bold"))
                   .withExtraKerningFactor (0.20f));
    g.drawText ("AI MODEL", textX, top, textW, labelH, juce::Justification::centredLeft, false);

    auto nameCol = HarmoniaPalette::textPrimary.interpolatedWith (acc.brighter (0.3f), h);
    if (! interactive)
        nameCol = nameCol.withAlpha (0.75f);

    g.setColour (nameCol);
    g.setFont (juce::Font (juce::FontOptions (12.5f).withStyle ("Bold")));
    g.drawText (getSelectedName(), textX, top + labelH, textW, nameH,
                juce::Justification::centredLeft, true);
}