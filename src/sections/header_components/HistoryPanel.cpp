#include "HeaderIncludes.h"
#include "HistoryPanel.h"
#include "HistoryUtils.h"

namespace
{
    constexpr float panelRadius = 10.0f;
    constexpr int   scrollW     = 6;   // épaisseur de la scrollbar
    constexpr int   scrollInset = 2;   // espace entre la scrollbar et le bord du panneau

    // trait horizontal qui s'estompe sur les bords
    void drawFadeLine (juce::Graphics& g, int y, int w)
    {
        const auto line = HarmoniaPalette::border;
        juce::ColourGradient grad (line.withAlpha (0.0f), 0.0f, (float) y,
                                   line.withAlpha (0.0f), (float) w, (float) y, false);
        grad.addColour (0.5, line);
        g.setGradientFill (grad);
        g.fillRect (0, y, w, 1);
    }

    // petite poubelle dessinée en vectoriel, centrée sur c
    void drawTrash (juce::Graphics& g, juce::Point<float> c, juce::Colour col)
    {
        g.setColour (col);

        // couvercle + poignée
        g.drawLine (c.x - 5.0f, c.y - 3.5f, c.x + 5.0f, c.y - 3.5f, 1.3f);
        g.drawLine (c.x - 1.6f, c.y - 5.5f, c.x + 1.6f, c.y - 5.5f, 1.3f);
        g.drawLine (c.x - 1.6f, c.y - 5.5f, c.x - 1.6f, c.y - 3.5f, 1.3f);
        g.drawLine (c.x + 1.6f, c.y - 5.5f, c.x + 1.6f, c.y - 3.5f, 1.3f);

        // corps
        juce::Path body;
        body.startNewSubPath (c.x - 4.0f, c.y - 2.0f);
        body.lineTo (c.x - 3.4f, c.y + 5.5f);
        body.lineTo (c.x + 3.4f, c.y + 5.5f);
        body.lineTo (c.x + 4.0f, c.y - 2.0f);
        g.strokePath (body, juce::PathStrokeType (1.3f, juce::PathStrokeType::curved,
                                                  juce::PathStrokeType::rounded));

        // deux traits intérieurs
        g.drawLine (c.x - 1.4f, c.y, c.x - 1.4f, c.y + 3.5f, 1.0f);
        g.drawLine (c.x + 1.4f, c.y, c.x + 1.4f, c.y + 3.5f, 1.0f);
    }
}

//==============================================================================
HistoryPanel::HistoryPanel()
{
    const auto acc = HarmoniaPalette::accent;

    setOpaque (false);

    search.setMultiLine (false);
    search.setReturnKeyStartsNewLine (false);
    search.setFont (juce::Font (juce::FontOptions (13.0f)));
    search.setJustification (juce::Justification::centredLeft);
    search.setBorder (juce::BorderSize<int> (0));
    search.setIndents (2, 0);
    search.setTextToShowWhenEmpty ("Search", HarmoniaPalette::textMuted);
    search.setColour (juce::TextEditor::backgroundColourId,     juce::Colours::transparentBlack);
    search.setColour (juce::TextEditor::outlineColourId,        juce::Colours::transparentBlack);
    search.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    search.setColour (juce::TextEditor::shadowColourId,         juce::Colours::transparentBlack);
    search.setColour (juce::TextEditor::textColourId,           HarmoniaPalette::textPrimary);
    search.setColour (juce::TextEditor::highlightColourId,      acc.withAlpha (0.30f));
    search.setColour (juce::CaretComponent::caretColourId,      acc);
    search.onTextChange = [this] { rebuild(); repaint(); };
    addAndMakeVisible (search);

    viewport.setViewedComponent (&content, false);
    viewport.setScrollBarsShown (true, false);
    viewport.setScrollBarThickness (scrollW);

    {
        auto& sb = viewport.getVerticalScrollBar();
        sb.setColour (juce::ScrollBar::thumbColourId,      acc.withAlpha (0.55f));
        sb.setColour (juce::ScrollBar::trackColourId,      acc.withAlpha (0.08f));
        sb.setColour (juce::ScrollBar::backgroundColourId, juce::Colours::transparentBlack);
    }

    addAndMakeVisible (viewport);

    setSize (panelW, topH + curListH + bottomH);

    juce::MessageManager::callAsync ([safe = juce::Component::SafePointer<HistoryPanel> (this)]
    {
        if (safe != nullptr)
            safe->search.grabKeyboardFocus();
    });
}

HistoryPanel::~HistoryPanel()
{
    if (onClosed)
        onClosed();
}

void HistoryPanel::setItems (const juce::StringArray& newTitles,
                             const juce::StringArray& newTypes,
                             const juce::StringArray& newDates)
{
    titles = newTitles;
    types  = newTypes;
    dates  = newDates;
    rebuild();
    repaint();
}

//==============================================================================
void HistoryPanel::paint (juce::Graphics& g)
{
    const auto acc = HarmoniaPalette::accent;
    const int w = getWidth();

    const auto bounds = getLocalBounds().toFloat();

    juce::Path shape;
    shape.addRoundedRectangle (bounds, panelRadius);

    {
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (shape);

        g.setColour (HarmoniaPalette::panel.darker (0.45f));
        g.fillPath (shape);

        g.setGradientFill (juce::ColourGradient (acc.withAlpha (0.08f), 0.0f, 0.0f,
                                                 acc.withAlpha (0.0f),  0.0f, (float) topH + 30.0f,
                                                 false));
        g.fillRect (0, 0, w, topH + 30);

        // --- recherche : pilule + loupe ---
        {
            const auto box = searchBox().toFloat();
            const bool focused = search.hasKeyboardFocus (true);

            g.setColour (juce::Colours::black.withAlpha (0.28f));
            g.fillRoundedRectangle (box, box.getHeight() * 0.5f);

            if (focused)
            {
                g.setColour (acc.withAlpha (0.10f));
                g.fillRoundedRectangle (box, box.getHeight() * 0.5f);
            }

            g.setColour (focused ? acc.withAlpha (0.85f) : HarmoniaPalette::border);
            g.drawRoundedRectangle (box.reduced (0.5f), box.getHeight() * 0.5f, 1.0f);

            const juce::Point<float> lens (box.getX() + 16.0f, box.getCentreY() - 1.0f);
            g.setColour (focused ? acc : HarmoniaPalette::textMuted);
            g.drawEllipse (juce::Rectangle<float> (8.0f, 8.0f).withCentre (lens), 1.4f);
            g.drawLine (lens.x + 2.8f, lens.y + 2.8f, lens.x + 5.2f, lens.y + 5.2f, 1.4f);
        }

        // --- poubelle : discrète, éclaircie au survol ---
        {
            const auto zone = clearZone().toFloat();

            if (clearHover)
            {
                g.setColour (HistoryUtils::danger().withAlpha (0.12f));
                g.fillEllipse (zone);
            }

            drawTrash (g, zone.getCentre(),
                       clearHover ? HistoryUtils::danger()
                                  : HarmoniaPalette::textMuted.withAlpha (0.7f));
        }

        drawFadeLine (g, topH - 1, w);

        if (tiles.isEmpty())
        {
            g.setColour (HarmoniaPalette::textMuted);
            g.setFont (juce::Font (juce::FontOptions (12.0f)));
            g.drawText (search.getText().isNotEmpty() ? "No results" : "No history",
                        0, topH, w, curListH, juce::Justification::centred, false);
        }
    }

    g.setColour (HarmoniaPalette::border);
    g.drawRoundedRectangle (bounds.reduced (0.5f), panelRadius, 1.0f);
}

void HistoryPanel::resized()
{
    search.setBounds (searchBox().withTrimmedLeft (30).withTrimmedRight (12));

    viewport.setBounds (0, topH + 1, getWidth() - scrollInset, curListH - 3);
    layoutTiles();
}

//==============================================================================
void HistoryPanel::mouseMove (const juce::MouseEvent& e)
{
    const bool over = clearZone().contains (e.getPosition());

    if (over != clearHover)
    {
        clearHover = over;
        setMouseCursor (over ? juce::MouseCursor::PointingHandCursor
                             : juce::MouseCursor::NormalCursor);
        repaint (clearZone());
    }
}

void HistoryPanel::mouseExit (const juce::MouseEvent&)
{
    if (clearHover)
    {
        clearHover = false;
        setMouseCursor (juce::MouseCursor::NormalCursor);
        repaint (clearZone());
    }
}

void HistoryPanel::mouseUp (const juce::MouseEvent& e)
{
    if (clearZone().contains (e.getPosition()) && onClear)
        onClear();
}

//==============================================================================
juce::Rectangle<int> HistoryPanel::searchBox() const
{
    // la barre laisse la place de la poubelle à droite
    return { pad, 7, getWidth() - 2 * pad - clearSize - 6, 26 };
}

juce::Rectangle<int> HistoryPanel::clearZone() const
{
    return { getWidth() - pad - clearSize, 7, clearSize, 26 };
}

bool HistoryPanel::matches (int i, const juce::String& q) const
{
    return q.isEmpty()
        || titles[i].containsIgnoreCase (q)
        || types[i].containsIgnoreCase (q)
        || dates[i].containsIgnoreCase (q);
}

void HistoryPanel::rebuild()
{
    tiles.clear();

    const auto q = search.getText().trim();

    for (int i = 0; i < titles.size(); ++i)
    {
        if (! matches (i, q))
            continue;

        auto* t = tiles.add (new HistoryTile (i, titles[i], dates[i],
                                              HistoryUtils::isRefined (types[i])));
        t->onSelect = [this] (int idx) { if (onSelect) onSelect (idx); };
        t->onDelete = [this] (int idx) { if (onDelete) onDelete (idx); };
        content.addAndMakeVisible (t);
    }

    layoutTiles();
}

void HistoryPanel::layoutTiles()
{
    if (getWidth() <= 0)
        return;

    // place les tuiles pour une largeur donnée et renvoie la hauteur du contenu
    auto place = [this] (int tileW)
    {
        int y = pad;

        for (auto* t : tiles)
        {
            const int h = t->layoutForWidth (tileW);
            t->setBounds (pad, y, tileW, h);
            y += h + gap;
        }

        return tiles.isEmpty() ? 0 : y - gap + pad;
    };

    // 1re passe : alignées sur la barre de recherche (bord droit à getWidth() - pad)
    int tileW    = getWidth() - 2 * pad;
    int contentH = place (tileW);

    const int newListH = juce::jlimit (minListH, listH, contentH);

    // 2e passe si ça défile : on laisse 4 px de respiration avant la scrollbar
        // 2e passe si ça défile : 2 px de respiration avant la scrollbar
    if (contentH > newListH - 3)
    {
        tileW    = getWidth() - 2 * pad - scrollW;
        contentH = place (tileW);
    }

    content.setSize (viewport.getWidth() - scrollW, contentH);

    if (newListH != curListH)
    {
        curListH = newListH;
        setSize (getWidth(), topH + curListH + bottomH);
    }
}