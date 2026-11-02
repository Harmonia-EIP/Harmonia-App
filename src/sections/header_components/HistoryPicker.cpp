#include "HistoryPicker.h"
#include "HistoryOverlay.h"
#include "HistoryPanel.h"
#include "HistoryUtils.h"
#include <memory>

//==============================================================================
HistoryPicker::HistoryPicker()
{
    setMouseCursor (juce::MouseCursor::PointingHandCursor);
}

HistoryPicker::~HistoryPicker()
{
    pushToPanel = nullptr;
    delete overlay.getComponent();
    stopTimer();
}

juce::String HistoryPicker::clean (const juce::String& s)
{
    auto t = s.replaceCharacters ("\r\n\t", "   ").trim();
    return t.isEmpty() ? juce::String ("(untitled)") : t;
}

//==============================================================================
void HistoryPicker::setItems (const juce::StringArray& newTitles,
                              const juce::StringArray& newTypes,
                              const juce::StringArray& newDates)
{
    titles.clear();
    types.clear();
    dates.clear();

    for (int i = 0; i < newTitles.size(); ++i)
    {
        titles.add (clean (newTitles[i]));
        types.add (newTypes[i]);
        dates.add (HistoryUtils::formatDate (newDates[i]));
    }

    if (pushToPanel)
        pushToPanel (titles, types, dates);

    repaint();
}

void HistoryPicker::setInteractive (bool shouldBeInteractive)
{
    interactive = shouldBeInteractive;
    setMouseCursor (interactive ? juce::MouseCursor::PointingHandCursor
                                : juce::MouseCursor::NormalCursor);

    if (! interactive)
        closeMenu();

    repaint();
}

juce::String HistoryPicker::getValueText() const
{
    return juce::String (titles.size());
}

void HistoryPicker::timerCallback()
{
    const float hoverTarget = (interactive && (isMouseOver() || menuOpen)) ? 1.0f : 0.0f;
    const float openTarget  = (interactive && menuOpen) ? 1.0f : 0.0f;

    hoverAnim += (hoverTarget - hoverAnim) * 0.30f;
    openAnim  += (openTarget  - openAnim)  * 0.25f;

    if (std::abs (hoverTarget - hoverAnim) < 0.01f)  hoverAnim = hoverTarget;
    if (std::abs (openTarget  - openAnim)  < 0.01f)  openAnim  = openTarget;

    if (hoverAnim == hoverTarget && openAnim == openTarget)
        stopTimer();

    repaint();
}

//==============================================================================
void HistoryPicker::removeEntry (int index)
{
    if (! juce::isPositiveAndBelow (index, titles.size()))
        return;

    titles.remove (index);
    types.remove (index);
    dates.remove (index);

    if (pushToPanel)
        pushToPanel (titles, types, dates);

    repaint();

    if (onDelete)
        onDelete (index);

    if (titles.isEmpty())
        closeMenu();
}

void HistoryPicker::clearAll()
{
    titles.clear();
    types.clear();
    dates.clear();
    closeMenu();
    repaint();

    if (onClear)
        onClear();
}

void HistoryPicker::closeMenu()
{
    if (overlay != nullptr)
        overlay->dismiss();
}

void HistoryPicker::showMenu()
{
    if (overlay != nullptr)
    {
        closeMenu();
        return;
    }

    if (! interactive || titles.isEmpty())
        return;

    auto* top = getTopLevelComponent();

    if (top == nullptr)
        return;

    using Safe = juce::Component::SafePointer<HistoryPicker>;
    Safe safe (this);

    auto* o = new HistoryOverlay (*top, *this);
    overlay = o;

    o->setNotchColours (HarmoniaPalette::panel.darker (0.45f),   // même fond que le panneau
                        HarmoniaPalette::border);

    auto& panel = o->getPanel();
    panel.setItems (titles, types, dates);

    panel.onSelect = [safe] (int index)
    {
        juce::MessageManager::callAsync ([safe, index]
        {
            if (safe == nullptr)
                return;

            safe->closeMenu();

            if (safe->onSelect)
                safe->onSelect (index);
        });
    };

    panel.onDelete = [safe] (int index)
    {
        juce::MessageManager::callAsync ([safe, index]
        {
            if (safe != nullptr)
                safe->removeEntry (index);
        });
    };

    panel.onClear = [safe]
    {
        juce::MessageManager::callAsync ([safe]
        {
            if (safe != nullptr)
                safe->clearAll();
        });
    };

    panel.onClosed = [safe]
    {
        if (safe == nullptr)
            return;

        safe->pushToPanel = nullptr;
        safe->menuOpen = false;
        safe->startTimerHz (60);
    };

    auto* rawPanel = &panel;

    pushToPanel = [rawPanel] (const juce::StringArray& t, const juce::StringArray& ty,
                              const juce::StringArray& d)
    {
        rawPanel->setItems (t, ty, d);
    };

    menuOpen = true;
    startTimerHz (60);
}

//==============================================================================
void HistoryPicker::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (1.0f);
    const auto acc = HarmoniaPalette::accent;
    const float h = hoverAnim;
    const float o = openAnim;
    const float lit = juce::jmax (h, o);
    const float r = 10.0f;

    g.setColour (juce::Colours::black.withAlpha (0.28f));
    g.fillRoundedRectangle (b, r);

    if (h > 0.01f)
    {
        juce::ColourGradient glow (acc.withAlpha (0.20f * h), b.getX() + 14.0f, b.getCentreY(),
                                   acc.withAlpha (0.0f),      b.getRight(),     b.getCentreY(),
                                   false);
        g.setGradientFill (glow);
        g.fillRoundedRectangle (b, r);
    }

    if (o > 0.01f)
    {
        g.setColour (acc.withAlpha (0.08f * o));
        g.fillRoundedRectangle (b, r);
    }

    g.setColour (HarmoniaPalette::border.interpolatedWith (acc.withAlpha (0.85f), lit));
    g.drawRoundedRectangle (b.reduced (0.5f), r, 1.0f);

    if (! interactive)
        g.beginTransparencyLayer (0.55f);

    const auto txt = titles.size() > 99 ? juce::String ("99+") : getValueText();
    const auto font = juce::Font (juce::FontOptions (12.5f).withStyle ("Bold"));
    const float textW = (float) juce::GlyphArrangement::getStringWidthInt (font, txt);

    const float iconW = 22.0f, iconH = 17.0f, gapIT = 8.0f;
    const float startX = b.getCentreX() - (iconW + gapIT + textW) * 0.5f;
    const float iy = b.getCentreY() - iconH * 0.5f;

    {
        const float cw = 17.0f, chh = 10.0f, rad = 3.0f;

        auto card = [&] (float dx, float dy, juce::Colour edge, juce::Colour tint)
        {
            const juce::Rectangle<float> rc (startX + dx, iy + dy, cw, chh);

            g.setColour (HarmoniaPalette::panel);
            g.fillRoundedRectangle (rc, rad);

            g.setColour (tint);
            g.fillRoundedRectangle (rc, rad);

            g.setColour (edge);
            g.drawRoundedRectangle (rc.reduced (0.6f), rad, 1.2f);
        };

        card (3.0f + 1.8f * o, 0.0f - 1.2f * o, HarmoniaPalette::textMuted.withAlpha (0.55f), juce::Colours::transparentBlack);
        card (1.5f + 0.9f * o, 3.0f - 0.2f * o, acc.withAlpha (0.45f),                       juce::Colours::transparentBlack);
        card (0.0f,            7.0f,            acc.brighter (0.25f * lit).withAlpha (0.95f), acc.withAlpha (0.14f));
    }

    auto valueCol = HarmoniaPalette::textPrimary.interpolatedWith (acc.brighter (0.3f), h);

    g.setColour (valueCol);
    g.setFont (font);
    g.drawText (txt, juce::Rectangle<float> (startX + iconW + gapIT, b.getY(), textW + 4.0f, b.getHeight()),
                juce::Justification::centredLeft, false);

    if (! interactive)
        g.endTransparencyLayer();
}