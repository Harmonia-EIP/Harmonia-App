#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../themes/HarmoniaPalette.h"
#include "../themes/HiveLookAndFeel.h"
#include "../themes/PaletteSelector.h"
#include "../backend/BackendManager.h"
#include "../config/AiConfig.h"
#include "../backend/BackendPalette.h"
#include <BinaryData.h>

//==============================================================================
namespace HeaderColours
{
    inline juce::Colour generate() { return HarmoniaPalette::accent; }
    inline juce::Colour refine()   { return HarmoniaPalette::accent.withRotatedHue (0.5f); }
}

//==============================================================================
// Police de la barre de prompt : UNE seule source de vérité pour le texte saisi
// et le placeholder, en Generate comme en Refine.
// La police monospace paraît plus petite que la police normale à hauteur égale :
// on corrige son échelle pour que la hauteur de lettres soit la même qu'en Generate.
namespace PromptFont
{
    inline constexpr float size = 14.0f;   // <- LA taille, pour les deux modes

    // Échelle du mono par rapport à Generate. 1.0 = même valeur numérique (14).
    //   - le texte Refine paraît trop GROS  -> baisse (0.94f, 0.90f, ...)
    //   - le texte Refine paraît trop PETIT -> monte  (1.04f, 1.08f, ...)
    inline constexpr float monoScale = 0.90f;

    // Espacement des lettres du mono (proportion de la taille de police).
    // Une police monospace est naturellement plus "aérée" : on la resserre.
    //   - encore trop espacé -> plus négatif (-0.10f, -0.14f, ...)
    //   - trop serré         -> plus proche de 0 (-0.03f, 0.0f)
    inline constexpr float monoKerning = -0.07f;

    inline juce::Font make (bool mono)
    {
        if (! mono)
            return juce::Font (juce::FontOptions (size));

        return juce::Font (juce::FontOptions (juce::Font::getDefaultMonospacedFontName(),
                                              size * monoScale,
                                              juce::Font::plain))
                   .withExtraKerningFactor (monoKerning);
    }
}

//==============================================================================
class IconButton : public juce::Button,
                   private juce::Timer
{
public:
    enum class Icon { Home, Import, Export };

    explicit IconButton (Icon i) : juce::Button (juce::String()), icon (i)
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void paintButton (juce::Graphics&, bool over, bool down) override;
    void buttonStateChanged() override { startTimerHz (60); }

private:
    void timerCallback() override;
    static juce::Path makePath (Icon);

    Icon icon;
    float hoverAnim = 0.0f;
};

//==============================================================================
class LogoButton : public juce::Button,
                   private juce::Timer
{
public:
    LogoButton() : juce::Button (juce::String())
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void setLogo (const juce::Image& img) { logo = img; repaint(); }
    void setTint (juce::Colour c)         { tint = c; repaint(); }
    void setRefine (bool);

    void paintButton (juce::Graphics&, bool over, bool down) override;
    void buttonStateChanged() override { startTimerHz (60); }

    // appelé au relâchement du clic : lance un tour complet du logo
    void clicked() override
    {
        spinning = true;
        spinProgress = 0.0f;
        startTimerHz (60);
    }

private:
    void timerCallback() override;

    juce::Image logo;
    juce::Colour tint { juce::Colours::white };
    float hoverAnim = 0.0f;
    float spinProgress = 0.0f;
    bool spinning = false;
};

//==============================================================================
class ActionButton : public juce::Button
{
public:
    ActionButton() : juce::Button (juce::String())
    {
        setMouseCursor (juce::MouseCursor::PointingHandCursor);
    }

    void setTint (juce::Colour c)      { tint = c; repaint(); }
    void setSharpness (float s)        { sharp = s; repaint(); }   // 0 = pilule, 1 = coins secs
    void paintButton (juce::Graphics&, bool over, bool down) override;

private:
    juce::Colour tint { juce::Colours::white };
    float sharp = 0.0f;
};

//==============================================================================
// Look & feel du menu déroulant des modèles IA (uniquement pour ce menu)
class ModelMenuLookAndFeel : public juce::LookAndFeel_V4
{
public:
    ModelMenuLookAndFeel()
    {
        setColour (juce::PopupMenu::backgroundColourId, juce::Colours::transparentBlack);
    }

    int getPopupMenuBorderSize() override { return 6; }

    void drawPopupMenuBackground (juce::Graphics& g, int w, int h) override
    {
        auto b = juce::Rectangle<float> ((float) w, (float) h).reduced (0.5f);

        g.setColour (HarmoniaPalette::panel.darker (0.45f));
        g.fillRoundedRectangle (b, 12.0f);

        g.setColour (HarmoniaPalette::border);
        g.drawRoundedRectangle (b.reduced (0.5f), 12.0f, 1.0f);
    }

    void getIdealPopupMenuItemSize (const juce::String& text, bool isSeparator, int /*standardHeight*/,
                                    int& idealWidth, int& idealHeight) override
    {
        if (isSeparator)
        {
            idealWidth  = 50;
            idealHeight = 8;
            return;
        }

        juce::Font f (juce::FontOptions (13.0f).withStyle ("Bold"));
        idealWidth  = f.getStringWidth (text) + 56;
        idealHeight = 36;
    }

    void drawPopupMenuItem (juce::Graphics& g, const juce::Rectangle<int>& area,
                            bool isSeparator, bool isActive, bool isHighlighted, bool isTicked,
                            bool /*hasSubMenu*/, const juce::String& text,
                            const juce::String& /*shortcut*/, const juce::Drawable* /*icon*/,
                            const juce::Colour* /*textColour*/) override
    {
        if (isSeparator)
        {
            g.setColour (HarmoniaPalette::border);
            g.fillRect (area.reduced (10, 0).withSizeKeepingCentre (area.getWidth() - 20, 1));
            return;
        }

        auto r = area.toFloat().reduced (1.0f, 1.0f);
        const auto acc = HarmoniaPalette::accent;

        if (isHighlighted && isActive)
        {
            juce::ColourGradient glow (acc.withAlpha (0.24f), r.getX(),     r.getCentreY(),
                                       acc.withAlpha (0.04f), r.getRight(), r.getCentreY(),
                                       false);
            g.setGradientFill (glow);
            g.fillRoundedRectangle (r, 8.0f);
        }

        // pastille : pleine + halo si sélectionné, anneau discret sinon
        const juce::Point<float> dot (r.getX() + 15.0f, r.getCentreY());

        if (isTicked)
        {
            g.setColour (acc.withAlpha (0.25f));
            g.fillEllipse (juce::Rectangle<float> (16.0f, 16.0f).withCentre (dot));
            g.setColour (acc);
            g.fillEllipse (juce::Rectangle<float> (6.0f, 6.0f).withCentre (dot));
        }
        else
        {
            g.setColour (HarmoniaPalette::textMuted.withAlpha (isHighlighted ? 0.9f : 0.5f));
            g.drawEllipse (juce::Rectangle<float> (6.0f, 6.0f).withCentre (dot), 1.2f);
        }

        auto col = isTicked || isHighlighted ? acc.brighter (0.3f) : HarmoniaPalette::textPrimary;
        if (! isActive)
            col = col.withAlpha (0.4f);

        g.setColour (col);
        g.setFont (juce::Font (juce::FontOptions (13.0f).withStyle (isTicked ? "Bold" : "Regular")));
        g.drawText (text, r.withTrimmedLeft (32.0f).withTrimmedRight (10.0f),
                    juce::Justification::centredLeft, true);
    }
};

//==============================================================================
// Sélecteur de modèle IA : capsule + menu déroulant
class ModelPicker : public juce::Component,
                    public juce::SettableTooltipClient,
                    private juce::Timer
{
public:
    ModelPicker() { setMouseCursor (juce::MouseCursor::PointingHandCursor); }

    void addItem (const juce::String& name, int id) { items.push_back ({ name, id }); }
    void setSelectedId (int id)                     { selectedId = id; repaint(); }
    int  getSelectedId() const                      { return selectedId; }

    std::function<void()> onChange;

    void paint (juce::Graphics&) override;
    void mouseEnter (const juce::MouseEvent&) override { startTimerHz (60); }
    void mouseExit  (const juce::MouseEvent&) override { startTimerHz (60); }
    void mouseDown  (const juce::MouseEvent&) override { showMenu(); }

private:
    struct Item { juce::String name; int id; };

    void timerCallback() override;
    void showMenu();
    juce::String getSelectedName() const;

    std::vector<Item> items;
    ModelMenuLookAndFeel menuLnF;
    int selectedId = 0;
    bool menuOpen = false;
    float hoverAnim = 0.0f;
};

//==============================================================================
// Champ de prompt avec placeholder dessiné à la main (style différent selon le mode).
// Affiché seulement quand le champ est vide ET sans focus.
class PromptEditor : public juce::TextEditor
{
public:
    void setHint (const juce::String& text, juce::Colour colour, bool monospace)
    {
        hint = text;
        hintCol = colour;
        mono = monospace;
        repaint();
    }

    std::function<void()> onFocusChanged;

    void paint (juce::Graphics& g) override
    {
        juce::TextEditor::paint (g);

        if (getTotalNumChars() > 0 || hasKeyboardFocus (false))
            return;

        // même taille visuelle dans les deux modes (voir PromptFont)
        const juce::Font font = mono
            ? PromptFont::make (true)
            : juce::Font (juce::FontOptions (PromptFont::size).withStyle ("Italic")).withExtraKerningFactor (0.02f);

        g.setColour (hintCol);
        g.setFont (font);
        g.drawText (hint, getLocalBounds().withTrimmedLeft (4),
                    juce::Justification::centredLeft, true);
    }

    void focusGained (FocusChangeType t) override
    {
        juce::TextEditor::focusGained (t);
        if (onFocusChanged) onFocusChanged();
    }

    void focusLost (FocusChangeType t) override
    {
        juce::TextEditor::focusLost (t);
        if (onFocusChanged) onFocusChanged();
    }

private:
    juce::String hint;
    juce::Colour hintCol { juce::Colours::grey };
    bool mono = false;
};

//==============================================================================
class HeaderComponent : public juce::Component,
                        private juce::Timer
{
public:
    enum class Mode { Generate, Refine };

    HeaderComponent (const UserSession& session);
    ~HeaderComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;
    void mouseDown (const juce::MouseEvent&) override;

    void setLogo (const juce::Image& img) { logoButton.setLogo (img); }

    juce::Button& getGenerateButton() { return actionButton; }
    juce::Button& getLoadButton()     { return loadButton; }
    juce::Button& getSaveButton()     { return saveButton; }
    juce::Button& getLogoutButton()   { return homeButton; }
    juce::Button& getLogoButton()     { return logoButton; }

    juce::TextEditor& getPromptEditor()    { return promptEditor; }
    juce::Label&      getPresetLabel()     { return presetLabel; }
    PaletteSelector&  getPaletteSelector() { return paletteSelector; }

    Mode getMode() const { return mode; }
    void setMode (Mode m, bool notify = true);

    std::function<void (HarmoniaPalette::Theme)> onThemeChanged;
    std::function<void (Mode)> onModeChanged;

    // Palette modifiée (couleur ou pastille active) : 3 hex "#RRGGBB" + slot 0..2.
    // Jamais appelé pour un invité.
    std::function<void (const juce::StringArray&, int)> onPaletteChanged;

    AIModel getSelectedModel() const;
    juce::String getSelectedModelName() const;
    juce::String getSelectedBackendName() const;
    int getSelectedModelId() const;

private:
    void timerCallback() override;

        // Passe a true quand le Refine est pret cote IA
    bool refineEnabled = false;

    juce::Colour modeColour() const;
    void applyColours();
    void updateModeTexts();
    void paintPromptBar (juce::Graphics&);

    UserSession session;

    juce::Label titleLabel;
    juce::Label presetLabel;

    IconButton homeButton { IconButton::Icon::Home };
    IconButton loadButton { IconButton::Icon::Import };
    IconButton saveButton { IconButton::Icon::Export };

    LogoButton   logoButton;
    ActionButton actionButton;

    ModelPicker      modelPicker;
    PromptEditor     promptEditor;
    PaletteSelector  paletteSelector;

    juce::Rectangle<int> promptBounds, fileGroupBounds;

    Mode mode = Mode::Generate;
    float modeAnim = 0.0f, targetAnim = 0.0f;
    float focusAnim = 0.0f;   // 0 = barre sans focus, 1 = en cours de saisie
    float waveAnim  = 1.0f;   // 1 = égaliseur visible, 0 = caché (saisie en cours)
};