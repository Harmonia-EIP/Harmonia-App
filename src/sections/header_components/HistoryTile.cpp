#include "HistoryTile.h"
#include "HistoryUtils.h"

namespace
{
    constexpr int textLeft = 10;       // début du contenu
    constexpr int padY = 10, lineH = 17;
    constexpr int badgeGap = 8, dateGap = 6, titleGap = 6;
    constexpr int crossW = 24, crossH = 22, rightPad = 4;
    constexpr int restRightPad = 10;   // marge droite des lignes 2 et 3 du titre
    constexpr int safety = 3;          // marge anti-rognage (arrondis de mesure)

    juce::Font titleFont (bool refined)
    {
        if (refined)    // look "terminal", comme la barre de prompt en mode Refine
            return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(),
                                                  11.5f, juce::Font::plain));

        return juce::Font (juce::FontOptions (12.5f).withStyle ("Bold"));
    }
    juce::Font dateFont()   { return juce::Font (juce::FontOptions (9.0f)); }

    juce::Font badgeFont()
    {
        return juce::Font (juce::FontOptions (8.0f)).withExtraKerningFactor (0.14f);
    }

    // largeur arrondie vers le haut : on ne sous-estime jamais le texte
    int textWidth (const juce::Font& f, const juce::String& s)
    {
        return (int) std::ceil (juce::GlyphArrangement::getStringWidth (f, s)) + 1;
    }

    // Colonnes de la carte (identiques pour toutes les cartes : titres et dates alignés)
    //  - titleW : largeur de la 1re ligne du titre (s'arrête avant la date)
    //  - restW  : largeur des lignes suivantes (la date n'est plus là)
    struct Cols { int badgeW, titleX, titleW, restW, dateX, dateW; };

    Cols columns (int width)
    {
        Cols c;
        c.badgeW = juce::jmax (textWidth (badgeFont(), "GEN"),
                               textWidth (badgeFont(), "REF")) + 16;
        c.dateW  = textWidth (dateFont(), "00 Mmm 00:00") + 2;
        c.titleX = textLeft + c.badgeW + badgeGap;
        c.dateX  = width - rightPad - crossW - dateGap - c.dateW;
        c.titleW = juce::jmax (20, c.dateX - titleGap - c.titleX - safety);
        c.restW  = juce::jmax (20, width - restRightPad - c.titleX - safety);
        return c;
    }

    // Retour à la ligne par mots, SANS limite de lignes. Un mot trop long est
    // découpé caractère par caractère. Les retours chariot / tabulations du
    // titre sont traités comme des espaces. La 1re ligne est plus étroite
    // (firstW) car la date est à sa droite.
    juce::StringArray wrap (const juce::String& rawText, const juce::Font& font,
                            int firstW, int restW)
    {
        const auto text = rawText.replaceCharacters ("\r\n\t", "   ");

        juce::StringArray lines;
        juce::String cur;

        auto limit = [&] { return lines.isEmpty() ? firstW : restW; };

        for (auto word : juce::StringArray::fromTokens (text, " ", ""))
        {
            if (word.isEmpty())
                continue;

            // mot plus large que la ligne : on le découpe
            while (textWidth (font, word) > limit() && word.length() > 1)
            {
                if (cur.isNotEmpty())
                {
                    lines.add (cur);
                    cur = {};
                }

                const int maxW = limit();
                int n = 1;
                while (n < word.length() && textWidth (font, word.substring (0, n + 1)) <= maxW)
                    ++n;

                lines.add (word.substring (0, n));
                word = word.substring (n);
            }

            if (word.isEmpty())
                continue;

            if (cur.isEmpty())
            {
                cur = word;
                continue;
            }

            const auto candidate = cur + " " + word;

            if (textWidth (font, candidate) <= limit())
            {
                cur = candidate;
            }
            else
            {
                lines.add (cur);
                cur = word;
            }
        }

        if (cur.isNotEmpty())
            lines.add (cur);

        if (lines.isEmpty())
            lines.add ({});

        return lines;
    }
}

//==============================================================================
HistoryTile::HistoryTile (int idx, const juce::String& t, const juce::String& d, bool ref)
    : index (idx), title (t), date (d), refined (ref)
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
    lines.add (title);
}

int HistoryTile::layoutForWidth (int width)
{
    const auto col = columns (width);

    // Aucune limite : le titre est affiché en entier, la carte grandit avec
    lines = wrap (title, titleFont (refined), col.titleW, col.restW);

    return padY * 2 + lines.size() * lineH;
}

void HistoryTile::resized()
{
    // la tile reste correcte même si le parent n'a pas appelé layoutForWidth
    const auto col = columns (getWidth());
    lines = wrap (title, titleFont (refined), col.titleW, col.restW);
}

//==============================================================================
void HistoryTile::paint (juce::Graphics& g)
{
    const auto acc = HarmoniaPalette::accent;
    const auto b = getLocalBounds().toFloat().reduced (1.0f);
    const float r = refined ? 4.0f : 10.0f;   // GEN = pilule douce, REF = quasi carré
    const int w = getWidth();

    juce::Path shape;
    shape.addRoundedRectangle (b, r);

    // fond (plus sombre et plat pour REF)
    g.setColour (juce::Colours::black.withAlpha (refined ? 0.34f : 0.25f));
    g.fillPath (shape);

    g.setColour (acc.withAlpha (refined ? 0.04f : 0.05f));
    g.fillPath (shape);

    {
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (shape);

        if (! refined)
        {
            // GEN : reflet doux sur la moitié haute + fin liseré lumineux
            g.setGradientFill (juce::ColourGradient (juce::Colours::white.withAlpha (0.06f), 0.0f, b.getY(),
                                                     juce::Colours::white.withAlpha (0.0f),  0.0f, b.getCentreY(),
                                                     false));
            g.fillRect (b);

            g.setColour (juce::Colours::white.withAlpha (0.08f));
            g.fillRect (b.getX() + r * 0.6f, b.getY() + 1.0f, b.getWidth() - r * 1.2f, 1.0f);
        }
        else
        {
            // REF : fines lignes de scan, comme la barre de prompt en mode Refine
            g.setColour (acc.withAlpha (0.07f));

            for (float y = b.getY() + 3.0f; y < b.getBottom(); y += 4.0f)
                g.fillRect (b.getX(), y, b.getWidth(), 1.0f);
        }

        // lueur qui part de la gauche (comme la capsule)
        if (hover)
        {
            g.setGradientFill (juce::ColourGradient (acc.withAlpha (0.18f), b.getX() + 10.0f, b.getCentreY(),
                                                     acc.withAlpha (0.0f),  b.getRight(),     b.getCentreY(),
                                                     false));
            g.fillRect (b);
        }
    }

    if (! refined)
    {
        // GEN : bordure pleine, dégradé accent -> accent éclairci
        juce::ColourGradient edge (acc.withAlpha (hover ? 0.95f : 0.55f), b.getX(), 0.0f,
                                   acc.brighter (0.35f).withAlpha (hover ? 0.60f : 0.25f), b.getRight(), 0.0f,
                                   false);
        g.setGradientFill (edge);
        g.strokePath (shape, juce::PathStrokeType (1.2f));
    }
    else
    {
        // REF : bordure pointillée
        juce::Path dashed;
        const float dashes[] = { 5.0f, 3.5f };
        juce::PathStrokeType (1.1f).createDashedStroke (dashed, shape, dashes, 2);

        g.setColour (acc.withAlpha (hover ? 0.95f : 0.60f));
        g.fillPath (dashed);
    }

    const auto col = columns (w);
    const int rowY = padY + lineH / 2;           // centre de la première ligne

    // --- badge (à gauche du titre) : GEN ou REF ---
    const auto label = refined ? juce::String ("REF") : juce::String ("GEN");
    const auto badge = juce::Rectangle<int> (textLeft, rowY - 7, col.badgeW, 14).toFloat();

    if (refined)
    {
        g.setColour (acc.withAlpha (0.50f));
        g.drawRoundedRectangle (badge.reduced (0.5f), 7.0f, 0.8f);
    }
    else
    {
        g.setColour (acc.withAlpha (0.10f));
        g.fillRoundedRectangle (badge, 7.0f);
    }

    g.setColour ((refined ? acc : acc.brighter (0.3f)).withAlpha (0.9f));
    g.setFont (badgeFont());
    g.drawText (label, badge, juce::Justification::centred, false);

    // --- titre : la 1re ligne s'arrête avant la date, les suivantes prennent toute la largeur ---
    // (la zone de dessin est élargie de 'safety' : le texte n'est jamais rogné,
    //  et comme le retour à la ligne a gardé cette marge, il ne déborde pas)
    g.setColour (HarmoniaPalette::textPrimary);
    g.setFont (titleFont (refined));

    for (int i = 0; i < lines.size(); ++i)
        g.drawText (lines[i], col.titleX, padY + i * lineH,
                    (i == 0 ? col.titleW : col.restW) + safety, lineH,
                    juce::Justification::centredLeft, false);

    // --- date (alignée sur la première ligne) ---
    g.setColour (HarmoniaPalette::textMuted);
    g.setFont (dateFont());
    g.drawText (date, col.dateX, rowY - 7, col.dateW, 14, juce::Justification::centredRight, false);

    // --- croix : simplement plus claire au survol (pas de pastille, pas de rouge) ---
    {
        const auto c = crossBounds().toFloat().getCentre();
        const float s = 3.6f;

        g.setColour (crossHover ? acc.brighter (0.6f)
                                : HarmoniaPalette::textMuted.withAlpha (hover ? 1.0f : 0.6f));
        g.drawLine (c.x - s, c.y - s, c.x + s, c.y + s, 1.5f);
        g.drawLine (c.x - s, c.y + s, c.x + s, c.y - s, 1.5f);
    }
}

//==============================================================================
void HistoryTile::mouseEnter (const juce::MouseEvent& e)
{
    hover = true;
    updateHover (e);
}

void HistoryTile::mouseMove (const juce::MouseEvent& e)
{
    updateHover (e);
}

void HistoryTile::mouseExit (const juce::MouseEvent&)
{
    hover = crossHover = false;
    repaint();
}

void HistoryTile::mouseUp (const juce::MouseEvent& e)
{
    if (! getLocalBounds().contains (e.getPosition()))
        return;

    if (crossBounds().contains (e.getPosition()))
    {
        if (onDelete) onDelete (index);
    }
    else if (onSelect)
    {
        onSelect (index);
    }
}

juce::Rectangle<int> HistoryTile::crossBounds() const
{
    return { getWidth() - rightPad - crossW, padY + lineH / 2 - crossH / 2, crossW, crossH };
}

void HistoryTile::updateHover (const juce::MouseEvent& e)
{
    crossHover = crossBounds().contains (e.getPosition());
    repaint();
}