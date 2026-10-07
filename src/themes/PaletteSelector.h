#pragma once

#include <array>
#include <functional>
#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>   // juce::ColourSelector
#include "HarmoniaPalette.h"

//==============================================================================
// 3 pastilles de couleur libres + icône crayon pour éditer la pastille active.
// Nécessite HarmoniaPalette::defaultSlots, HarmoniaPalette::customAccent
// et HarmoniaPalette::setCustomAccent (juce::Colour).
//==============================================================================
class PaletteSelector : public juce::Component
{
public:
    PaletteSelector()
    {
        // couleurs par défaut des 3 slots (guest + premier démarrage)
        slotColours = HarmoniaPalette::defaultSlots;

        HarmoniaPalette::setCustomAccent (slotColours[0]);

        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    //==========================================================================
    void paint (juce::Graphics& g) override
    {
        const float r = getHeight() * 0.5f;

        g.setColour (HarmoniaPalette::panel.brighter (0.05f));
        g.fillRoundedRectangle (getLocalBounds().toFloat(), r);
        g.setColour (HarmoniaPalette::border);
        g.drawRoundedRectangle (getLocalBounds().toFloat().reduced (0.5f), r, 1.0f);

        const float dotR  = r * 0.52f;
        const float cellW = getDotsWidth() / 3.0f;
        const float cy    = getHeight() * 0.5f;

        for (int i = 0; i < 3; ++i)
        {
            const bool selected = (i == activeSlot);
            const float cx = cellW * (float) i + cellW * 0.5f;
            const auto col = slotColours[(size_t) i];

            if (selected)
            {
                g.setColour (col.withAlpha (0.22f));
                g.fillEllipse (cx - dotR * 1.7f, cy - dotR * 1.7f, dotR * 3.4f, dotR * 3.4f);
            }

            g.setColour (selected ? col : col.withAlpha (0.45f));
            g.fillEllipse (cx - dotR, cy - dotR, dotR * 2.0f, dotR * 2.0f);

            g.setColour (selected ? col.brighter (0.3f) : HarmoniaPalette::borderHi);
            g.drawEllipse (cx - dotR, cy - dotR, dotR * 2.0f, dotR * 2.0f, 1.0f);
        }

        // séparateur
        g.setColour (HarmoniaPalette::border);
        g.fillRect (getDotsWidth(), 6.0f, 1.0f, (float) getHeight() - 12.0f);

        // icône crayon (éditer la pastille active)
        auto ic = getIconArea().toFloat().withSizeKeepingCentre (12.0f, 12.0f);

        juce::Path pencil;
        pencil.startNewSubPath (ic.getX() + 1.0f, ic.getBottom() - 1.0f);
        pencil.lineTo (ic.getX() + 1.0f, ic.getBottom() - 4.0f);
        pencil.lineTo (ic.getRight() - 3.5f, ic.getY() + 0.5f);
        pencil.lineTo (ic.getRight() - 0.5f, ic.getY() + 3.5f);
        pencil.lineTo (ic.getX() + 4.0f, ic.getBottom() - 1.0f);
        pencil.closeSubPath();

        g.setColour (HarmoniaPalette::textMuted);
        g.strokePath (pencil, juce::PathStrokeType (1.2f,
                                                    juce::PathStrokeType::curved,
                                                    juce::PathStrokeType::rounded));
    }

    void mouseDown (const juce::MouseEvent& e) override
    {
        if (getIconArea().contains (e.getPosition()))
        {
            openEditor();
            return;
        }

        const int idx = juce::jlimit (0, 2, (int) ((float) e.x / (getDotsWidth() / 3.0f)));
        activeSlot = idx;
        applyActiveSlot();
    }

    //==========================================================================
    // Pour la sauvegarde / restauration (UserSession)
    std::array<juce::Colour, 3> getSlotColours() const { return slotColours; }
    int getActiveSlot() const                          { return activeSlot; }

    void setSlots (const std::array<juce::Colour, 3>& colours, int active)
    {
        slotColours = colours;
        activeSlot  = juce::jlimit (0, 2, active);
        HarmoniaPalette::setCustomAccent (slotColours[(size_t) activeSlot]);
        repaint();
    }

    // Guest / nouvel utilisateur : retour aux 3 couleurs par défaut
    void resetToDefaults()
    {
        setSlots (HarmoniaPalette::defaultSlots, 0);
    }

    HarmoniaPalette::Theme getCurrentTheme() const { return HarmoniaPalette::Theme::Custom; }

    std::function<void()> onThemeUpdated;

private:
    //==========================================================================
    // Contenu de la CallOutBox : sélecteur de couleur + champ hex
    class Picker : public juce::Component,
                   private juce::ChangeListener
    {
    public:
        Picker (juce::Colour start, std::function<void (juce::Colour)> cb)
            : selector (juce::ColourSelector::showColourspace, 4, 0),
              onColour (std::move (cb))
        {
            addAndMakeVisible (selector);
            selector.setCurrentColour (start, juce::dontSendNotification);
            selector.addChangeListener (this);

            hexLabel.setText ("HEX", juce::dontSendNotification);
            hexLabel.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold")));
            hexLabel.setColour (juce::Label::textColourId, HarmoniaPalette::textMuted);
            addAndMakeVisible (hexLabel);

            hexEditor.setInputRestrictions (7, "#0123456789abcdefABCDEF");
            hexEditor.setText (toHex (start), false);
            hexEditor.setSelectAllWhenFocused (true);
            hexEditor.onTextChange = [this] { applyHex(); };
            hexEditor.onReturnKey  = [this] { applyHex(); };
            addAndMakeVisible (hexEditor);

            setSize (240, 240);
        }

        ~Picker() override { selector.removeChangeListener (this); }

        void resized() override
        {
            auto r = getLocalBounds().reduced (6);
            auto row = r.removeFromBottom (28);
            hexLabel.setBounds (row.removeFromLeft (36));
            hexEditor.setBounds (row.reduced (0, 2));
            r.removeFromBottom (6);
            selector.setBounds (r);
        }

    private:
        static juce::String toHex (juce::Colour c) { return "#" + c.toDisplayString (false); }

        void applyHex()
        {
            auto t = hexEditor.getText().trim().removeCharacters ("#");

            if (t.length() != 6 && t.length() != 3)
                return;                                   // pas encore un code valide

            if (t.length() == 3)                          // #0af -> #00aaff
                t = t.substring (0, 1) + t.substring (0, 1)
                  + t.substring (1, 2) + t.substring (1, 2)
                  + t.substring (2, 3) + t.substring (2, 3);

            // déclenche changeListenerCallback -> onColour
            selector.setCurrentColour (juce::Colour::fromString ("ff" + t));
        }

        void changeListenerCallback (juce::ChangeBroadcaster*) override
        {
            const auto c = selector.getCurrentColour();

            if (! hexEditor.hasKeyboardFocus (true))      // ne pas écraser ce que l'utilisateur tape
                hexEditor.setText (toHex (c), false);

            if (onColour)
                onColour (c);
        }

        juce::ColourSelector selector;
        juce::Label hexLabel;
        juce::TextEditor hexEditor;
        std::function<void (juce::Colour)> onColour;
    };

    //==========================================================================
    void openEditor()
    {
        auto picker = std::make_unique<Picker> (
            slotColours[(size_t) activeSlot],
            [safe = juce::Component::SafePointer<PaletteSelector> (this)] (juce::Colour c)
            {
                if (safe == nullptr)
                    return;

                safe->slotColours[(size_t) safe->activeSlot] = c;   // modifie la pastille active
                safe->applyActiveSlot();
            });

        juce::CallOutBox::launchAsynchronously (std::move (picker),
                                                getScreenBounds().withLeft (getScreenX() + (int) getDotsWidth()),
                                                nullptr);
    }

    void applyActiveSlot()
    {
        HarmoniaPalette::setCustomAccent (slotColours[(size_t) activeSlot]);
        repaint();

        if (onThemeUpdated)
            onThemeUpdated();
    }

    float getDotsWidth() const { return (float) getWidth() - (float) getHeight(); }
    juce::Rectangle<int> getIconArea() const { return { getWidth() - getHeight(), 0, getHeight(), getHeight() }; }

    std::array<juce::Colour, 3> slotColours;
    int activeSlot = 0;
};