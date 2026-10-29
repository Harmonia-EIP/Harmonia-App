#include "HeaderComponent.h"
#include <BinaryData.h>

#include <algorithm>

//==============================================================================
HeaderComponent::HeaderComponent (const UserSession& s, AIModelList initialModels)
    : session (s), models (std::move (initialModels))
{
    // Titre
    juce::String username = session.pseudo.isNotEmpty()
        ? session.pseudo
        : Strings::Errors::NoUserConnected;

    titleLabel.setText (username.toUpperCase(), juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions (17.0f).withStyle ("Bold"))
                            .withExtraKerningFactor (0.10f));
    titleLabel.setBorderSize ({ 0, 0, 0, 5 });
    titleLabel.setMinimumHorizontalScale (1.0f);
    titleLabel.setTooltip (username);
    addAndMakeVisible (titleLabel);

    // Preset
    presetLabel.setText (Strings::Labels::UnsetPreset.toUpperCase(), juce::dontSendNotification);
    presetLabel.setFont (juce::Font (juce::FontOptions (10.0f).withStyle ("Bold"))
                             .withExtraKerningFactor (0.18f));
    presetLabel.setJustificationType (juce::Justification::centred);
    addAndMakeVisible (presetLabel);

    // Home
    homeButton.setTooltip (session.isGuest ? Strings::Buttons::Back
                                           : Strings::Buttons::Logout);
    addAndMakeVisible (homeButton);

    // Load / Export
    loadButton.setTooltip ("Import a preset");
    saveButton.setTooltip ("Export current preset");
    addAndMakeVisible (loadButton);
    addAndMakeVisible (saveButton);

    // Modèle IA : invité -> capsule figée, connecté -> liste du serveur
    rebuildModelPicker ({});
    addAndMakeVisible (modelPicker);

    // Historique : vide au départ, rempli par setHistory() quand le profil arrive
    historyPicker.setInteractive (false);
    historyPicker.setTooltip (session.isGuest ? "Sign in to see your history"
                                              : "Generation & Refinement History");

    // Le picker ne renvoie que des index : on les traduit ici en entrées complètes
    historyPicker.onSelect = [this] (int index)
    {
        if (juce::isPositiveAndBelow (index, (int) historyEntries.size()) && onHistorySelected)
            onHistorySelected (historyEntries[(size_t) index]);
    };

    historyPicker.onDelete = [this] (int index)
    {
        if (! juce::isPositiveAndBelow (index, (int) historyEntries.size()))
            return;

        const auto entry = historyEntries[(size_t) index];   // copie, pour l'id côté backend
        historyEntries.erase (historyEntries.begin() + index);

        if (onHistoryDeleted)
            onHistoryDeleted (entry);
    };

    historyPicker.onClear = [this]
    {
        historyEntries.clear();

        if (onHistoryCleared)
            onHistoryCleared();
    };

    addAndMakeVisible (historyPicker);

    // Palette : 3 pastilles libres (le thème est toujours Custom)
    addAndMakeVisible (paletteSelector);

    // Restauration : uniquement depuis ce que le backend a renvoyé (stocké dans la session)
    if (! session.isGuest && session.paletteColours.size() == 3)
    {
        std::array<juce::Colour, 3> cols;

        for (int i = 0; i < 3; ++i)
            cols[(size_t) i] = BackendPalette::fromHex (session.paletteColours[i]);

        paletteSelector.setSlots (cols, session.paletteSlot);
    }
    else
    {
        paletteSelector.resetToDefaults();   // invité, ou pas encore de palette backend
    }

    // Doit venir APRÈS la restauration
    HarmoniaPalette::setTheme (paletteSelector.getCurrentTheme());

    paletteSelector.onThemeUpdated = [this]
    {
        auto theme = paletteSelector.getCurrentTheme();
        HarmoniaPalette::setTheme (theme);

        applyColours();

        if (onThemeChanged)
            onThemeChanged (theme);

        // Un compte connecté synchronise à chaque changement (couleur OU pastille),
        // sauf si le changement vient du serveur lui-même
        if (! applyingServerPalette && ! session.isGuest && onPaletteChanged)
        {
            juce::StringArray hex;

            for (auto c : paletteSelector.getSlotColours())
                hex.add (BackendPalette::toHex (c));

            onPaletteChanged (hex, paletteSelector.getActiveSlot());
        }

        repaint();
    };

    // Prompt bar
    promptEditor.setMultiLine (false);
    promptEditor.setReturnKeyStartsNewLine (false);
    promptEditor.setFont (juce::Font (juce::FontOptions (14.0f)));
    promptEditor.setJustification (juce::Justification::centredLeft);
    promptEditor.setBorder (juce::BorderSize<int> (0));
    promptEditor.setIndents (4, 0);
    promptEditor.onReturnKey = [this] { actionButton.triggerClick(); };
    promptEditor.onFocusChanged = [this] { startTimerHz (60); };
    addAndMakeVisible (promptEditor);

    // when refine mode is available with ai
    logoButton.onClick = [this]
    {
        setMode (mode == Mode::Generate ? Mode::Refine : Mode::Generate);
    };
    // logoButton.onClick = [this]
    // {
    //     if (! refineEnabled)
    //     {
    //         HarmoniaAlert::info (this,
    //                              "Refine mode",
    //                              "Refine mode is coming soon.",
    //                              HarmoniaAlert::Position::TopRight,
    //                              3000);
    //         return;
    //     }

    //     setMode (mode == Mode::Generate ? Mode::Refine : Mode::Generate);
    // };
    addAndMakeVisible (logoButton);
    logoButton.setTooltip ("Refine mode (coming soon)");
    addAndMakeVisible (actionButton);

    // Logo chargé directement ici (plus besoin de setLogo depuis le parent)
    {
        auto img = juce::ImageCache::getFromMemory (BinaryData::harmonia_logo_png,
                                                    BinaryData::harmonia_logo_pngSize);
        jassert (img.isValid());   // si ça s'arrête ici : nom de ressource BinaryData incorrect
        logoButton.setLogo (img);
    }

    // Clic ailleurs que sur la barre => le curseur disparaît et le placeholder revient
    juce::Desktop::getInstance().addGlobalMouseListener (this);

    // Pas de focus sur la barre au démarrage
    juce::Timer::callAfterDelay (150, [safe = juce::Component::SafePointer<HeaderComponent> (this)]
    {
        if (safe != nullptr)
            safe->promptEditor.giveAwayKeyboardFocus();
    });

    applyColours();
    updateModeTexts();

    // Generate : l'égaliseur ondule en continu dès le démarrage
    startTimerHz (30);
}

HeaderComponent::~HeaderComponent()
{
    juce::Desktop::getInstance().removeGlobalMouseListener (this);
}

//==============================================================================
// Données du profil reçues du MainComponent
//==============================================================================
void HeaderComponent::setHistory (const HistoryList& entries)
{
    historyEntries = entries;

    DBG ("HeaderComponent: " << (int) historyEntries.size() << " history entries received");

    juce::StringArray titles, typeList, dateList;

    for (const auto& e : historyEntries)
    {
        titles.add (e.name.isNotEmpty() ? e.name : e.type);   // repli sur le type si pas de nom
        typeList.add (e.type);
        dateList.add (e.lastUsedAt.isNotEmpty() ? e.lastUsedAt : e.createdAt);
    }

    historyPicker.setItems (titles, typeList, dateList);
    historyPicker.setInteractive (! session.isGuest && ! titles.isEmpty());
}

void HeaderComponent::moveHistoryToTop (int id)
{
    auto it = std::find_if (historyEntries.begin(), historyEntries.end(),
                            [id] (const auto& e) { return e.id == id; });

    if (it == historyEntries.end())
        return;

    // Si l'entrée est déjà en tête, rotate ne change rien
    std::rotate (historyEntries.begin(), it, it + 1);
    setHistory (historyEntries);
}

void HeaderComponent::addHistoryEntry (const HistoryEntry& e)
{
    historyEntries.insert (historyEntries.begin(), e);

    // Même plafond que le serveur : la plus ancienne sort de la liste
    if (historyEntries.size() > maxHistoryEntries)
        historyEntries.resize (maxHistoryEntries);

    setHistory (historyEntries);
}

void HeaderComponent::setPaletteFromServer (const juce::StringArray& hexColours, int activeSlot)
{
    if (session.isGuest || hexColours.size() != 3)
        return;

    std::array<juce::Colour, 3> cols;

    for (int i = 0; i < 3; ++i)
        cols[(size_t) i] = BackendPalette::fromHex (hexColours[i]);

    {
        // Pendant ce bloc, onThemeUpdated ne renvoie rien au serveur
        const juce::ScopedValueSetter<bool> guard (applyingServerPalette, true);
        paletteSelector.setSlots (cols, juce::jlimit (0, 2, activeSlot));
    }

    // Même application que dans le constructeur / reapplyTheme
    const auto theme = paletteSelector.getCurrentTheme();
    HarmoniaPalette::setTheme (theme);
    applyColours();

    if (onThemeChanged)
        onThemeChanged (theme);

    repaint();
}

//==============================================================================
void HeaderComponent::restoreSlot (int slot)
{
    paletteSelector.setSlots (paletteSelector.getSlotColours(),
                              juce::jlimit (0, 2, slot));
    reapplyTheme();
}

void HeaderComponent::reapplyTheme()
{
    // remet la couleur globale à celle de la pastille active de CETTE instance
    paletteSelector.setSlots (paletteSelector.getSlotColours(),
                              paletteSelector.getActiveSlot());

    HarmoniaPalette::setTheme (paletteSelector.getCurrentTheme());
    applyColours();
    repaint();
}

void HeaderComponent::mouseDown (const juce::MouseEvent& e)
{
    if (e.originalComponent != &promptEditor && ! promptEditor.isParentOf (e.originalComponent))
        promptEditor.giveAwayKeyboardFocus();
}

//==============================================================================
// La couleur ne dépend plus du mode : seul le style de la barre change
juce::Colour HeaderComponent::modeColour() const
{
    return paletteSelector.getActiveColour();
}

void HeaderComponent::setMode (Mode m, bool notify)
{
    if (m == mode)
        return;

    mode = m;
    targetAnim = (m == Mode::Refine) ? 1.0f : 0.0f;

    logoButton.setRefine (m == Mode::Refine);   // no-op, le logo ne change pas
    updateModeTexts();
    startTimerHz (60);

    if (notify && onModeChanged)
        onModeChanged (mode);
}

void HeaderComponent::timerCallback()
{
    modeAnim += (targetAnim - modeAnim) * 0.25f;

    const bool modeSettled = std::abs (targetAnim - modeAnim) < 0.01f;

    if (modeSettled)
        modeAnim = targetAnim;

    const float focusTarget = promptEditor.hasKeyboardFocus (true) ? 1.0f : 0.0f;
    focusAnim += (focusTarget - focusAnim) * 0.25f;

    const bool focusSettled = std::abs (focusTarget - focusAnim) < 0.01f;

    if (focusSettled)
        focusAnim = focusTarget;

    // L'égaliseur s'efface (fondu) dès qu'il y a le focus ou du texte dans la barre
    const bool  hasText    = promptEditor.getTotalNumChars() > 0;
    const float waveTarget = (promptEditor.hasKeyboardFocus (true) || hasText) ? 0.0f : 1.0f;
    waveAnim += (waveTarget - waveAnim) * 0.25f;

    const bool waveSettled = std::abs (waveTarget - waveAnim) < 0.01f;

    if (waveSettled)
        waveAnim = waveTarget;

    // Les transitions sont finies : on repasse à 30 Hz (Generate : l'égaliseur ondule,
    // Refine : le faisceau tourne). Le timer ne s'arrête plus.
    if (modeSettled && focusSettled && waveSettled && getTimerInterval() != 33)
        startTimerHz (30);

    const auto c = modeColour();
    promptEditor.setColour (juce::CaretComponent::caretColourId, c);
    promptEditor.setColour (juce::TextEditor::highlightColourId, c.withAlpha (0.30f));
    actionButton.setTint (c);
    actionButton.setSharpness (modeAnim);
    logoButton.setTint (c);

    repaint (promptBounds.expanded (30));
}

void HeaderComponent::applyColours()
{
    const auto c = modeColour();

    presetLabel.setColour (juce::Label::textColourId, HarmoniaPalette::textMuted);

    promptEditor.setColour (juce::TextEditor::backgroundColourId,     juce::Colours::transparentBlack);
    promptEditor.setColour (juce::TextEditor::outlineColourId,        juce::Colours::transparentBlack);
    promptEditor.setColour (juce::TextEditor::focusedOutlineColourId, juce::Colours::transparentBlack);
    promptEditor.setColour (juce::TextEditor::shadowColourId,         juce::Colours::transparentBlack);
    promptEditor.setColour (juce::CaretComponent::caretColourId,      c);
    promptEditor.setColour (juce::TextEditor::highlightColourId,      c.withAlpha (0.30f));

    actionButton.setTint (c);
    logoButton.setTint (c);
    updateModeTexts();
}

void HeaderComponent::updateModeTexts()
{
    const bool refine = (mode == Mode::Refine);

    // Placeholder (textes dans Strings::Placeholders)
    if (refine)
        promptEditor.setHint (Strings::Placeholders::Refine,
                              HarmoniaPalette::accent.withAlpha (0.80f),
                              true);
    else
        promptEditor.setHint (Strings::Placeholders::Prompt,
                              HarmoniaPalette::textMuted.brighter (0.25f),
                              false);

    actionButton.setButtonText (refine ? Strings::Buttons::Refine
                                       : Strings::Buttons::Generate);

    logoButton.setTooltip (refine ? "Switch to Generate mode"
                                  : "Switch to Refine mode");

    // Refine : police monospace (look terminal) / Generate : police normale
    promptEditor.applyFontToAllText (PromptFont::make (refine));
    promptEditor.repaint();
}

//==============================================================================
// Modèles IA
//==============================================================================
void HeaderComponent::setModels (const AIModelList& newModels)
{
    // On lit le choix de l'utilisateur AVANT de remplacer la liste
    const auto* current = getSelectedModel();
    const juce::String keepId = current != nullptr ? current->id : juce::String();

    models = newModels;
    rebuildModelPicker (keepId);
}

void HeaderComponent::rebuildModelPicker (const juce::String& keepId)
{
    modelPicker.clearItems();

    if (session.isGuest)
    {
        modelPicker.setPlaceholder ("Harmonia AI");
        modelPicker.setInteractive (false);
        modelPicker.setTooltip ("Sign in to choose an AI model");
        return;
    }

    if (models.empty())
    {
        // Liste pas encore chargée ou échec réseau : "latest" marche quand même côté serveur
        modelPicker.setPlaceholder ("Latest model");
        modelPicker.setInteractive (false);
        modelPicker.setTooltip ("Using the latest AI model");
        return;
    }

    int sel = -1;

    for (size_t i = 0; i < models.size(); ++i)
    {
        modelPicker.addItem (models[i].name, (int) i + 1);

        if (keepId.isNotEmpty() && models[i].id == keepId)
            sel = (int) i;
    }

    // Pas de choix précédent : on prend le dernier déployé
    if (sel < 0)
        for (size_t i = 0; i < models.size(); ++i)
            if (models[i].isLatest)
            {
                sel = (int) i;
                break;
            }

    if (sel < 0)
        sel = 0;

    modelPicker.setSelectedId (sel + 1);
    modelPicker.setInteractive (true);
    modelPicker.setTooltip ("Choose the AI model");
}

const AIModelData* HeaderComponent::getSelectedModel() const
{
    if (session.isGuest)
        return nullptr;

    const int idx = modelPicker.getSelectedId() - 1;

    return juce::isPositiveAndBelow (idx, (int) models.size()) ? &models[(size_t) idx]
                                                               : nullptr;
}

juce::String HeaderComponent::getSelectedModelName() const
{
    const auto* m = getSelectedModel();
    return m != nullptr ? m->name : juce::String();
}

juce::String HeaderComponent::getSelectedBackendName() const
{
    const auto* m = getSelectedModel();

    if (m == nullptr)
        return "latest";

    // Le dernier déployé -> "latest" : l'app suit les nouveaux déploiements toute seule
    return m->isLatest ? "latest" : m->id;
}

//==============================================================================
void HeaderComponent::paintPromptBar (juce::Graphics& g)
{
    auto b = promptBounds.toFloat();
    const auto col = modeColour();
    const float m = modeAnim;          // 0 = Generate, 1 = Refine
    const float f = focusAnim;         // 1 = en cours de saisie

    // Generate = pilule, Refine = quasi carré
    const float radius = juce::jmap (m, b.getHeight() * 0.5f, 3.0f);

    juce::Path shape;
    shape.addRoundedRectangle (b, radius);

    // halo : gros en Generate (et encore plus pendant la saisie), presque éteint en Refine
    juce::DropShadow (col.withAlpha ((0.24f + 0.18f * f) * (1.0f - 0.75f * m)), 14, { 0, 0 })
        .drawForPath (g, shape);

    // fond (plus sombre et plat en Refine)
    juce::ColourGradient fill (HarmoniaPalette::panel.darker (0.55f + 0.15f * m), 0.0f, b.getY(),
                               HarmoniaPalette::panel.darker (0.25f + 0.25f * m), 0.0f, b.getBottom(),
                               false);
    g.setGradientFill (fill);
    g.fillPath (shape);

    g.setColour (col.withAlpha (0.05f + 0.05f * m + 0.03f * f));
    g.fillPath (shape);

    {
        juce::Graphics::ScopedSaveState state (g);
        g.reduceClipRegion (shape);

        if (m < 0.98f)
        {
            // Generate : lueur derrière le logo
            const float lx = b.getX() + b.getHeight() * 0.5f;
            juce::ColourGradient aura (col.withAlpha ((0.16f + 0.08f * f) * (1.0f - m)), lx, b.getCentreY(),
                                       col.withAlpha (0.0f), lx + 130.0f, b.getCentreY(),
                                       true);
            g.setGradientFill (aura);
            g.fillRect (b);

            // Generate : reflet doux sur la moitié haute
            juce::ColourGradient gloss (juce::Colours::white.withAlpha (0.07f * (1.0f - m)), 0.0f, b.getY(),
                                        juce::Colours::white.withAlpha (0.0f),               0.0f, b.getCentreY(),
                                        false);
            g.setGradientFill (gloss);
            g.fillRect (b);

            // Generate : fin liseré lumineux le long du bord haut
            g.setColour (juce::Colours::white.withAlpha (0.10f * (1.0f - m)));
            g.fillRect (b.getX() + radius * 0.6f, b.getY() + 1.0f, b.getWidth() - radius * 1.2f, 1.0f);

            // Generate : mini égaliseur qui ondule, à droite du texte
            // (s'efface en fondu pendant la saisie)
            if (waveAnim > 0.01f)
            {
                const auto eb = promptEditor.getBounds().toFloat();
                const float now = (float) (juce::Time::getMillisecondCounterHiRes() / 1000.0);

                constexpr int   nBars = 10;
                constexpr float step  = 5.0f;
                const float x0   = eb.getRight() - nBars * step - 6.0f;
                const float maxH = b.getHeight() * 0.5f;

                g.setColour (col.withAlpha (0.28f * (1.0f - m) * waveAnim));

                for (int i = 0; i < nBars; ++i)
                {
                    const float v1 = 0.5f + 0.5f * std::sin (now * 4.4f + (float) i * 0.55f);
                    const float v2 = 0.5f + 0.5f * std::sin (now * 7.5f + (float) i * 1.30f);
                    const float h  = 3.0f + (maxH - 3.0f) * 0.55f * (0.6f * v1 + 0.4f * v2);

                    g.fillRoundedRectangle (x0 + (float) i * step, b.getCentreY() - h * 0.5f, 2.0f, h, 1.0f);
                }
            }
        }

        // Refine : lignes de scan + faisceau qui balaie la barre
        if (m > 0.02f)
        {
            g.setColour (col.withAlpha (0.10f * m));

            for (float y = b.getY() + 3.0f; y < b.getBottom(); y += 4.0f)
                g.fillRect (b.getX(), y, b.getWidth(), 1.0f);

            const double t = std::fmod (juce::Time::getMillisecondCounterHiRes() / 2600.0, 1.0);
            const float x = b.getX() - 60.0f + (float) t * (b.getWidth() + 120.0f);

            juce::ColourGradient beam (col.withAlpha (0.0f), x - 50.0f, 0.0f,
                                       col.withAlpha (0.0f), x + 50.0f, 0.0f,
                                       false);
            beam.addColour (0.5, col.withAlpha (0.22f * m));
            g.setGradientFill (beam);
            g.fillRect (b);
        }
    }

    // bordure Generate : dégradé accent -> accent éclairci, plus vive pendant la saisie
    if (m < 0.98f)
    {
        juce::ColourGradient edge (col.withAlpha ((0.85f + 0.15f * f) * (1.0f - m)), b.getX(), 0.0f,
                                   col.brighter (0.35f).withAlpha ((0.30f + 0.40f * f) * (1.0f - m)), b.getRight(), 0.0f,
                                   false);
        g.setGradientFill (edge);
        g.strokePath (shape, juce::PathStrokeType (1.4f));
    }

    // bordure Refine : pointillée
    if (m > 0.02f)
    {
        juce::Path dashed;
        const float dashes[] = { 6.0f, 4.0f };
        juce::PathStrokeType (1.2f).createDashedStroke (dashed, shape, dashes, 2);

        g.setColour (col.withAlpha ((0.70f + 0.25f * f) * m));
        g.fillPath (dashed);
    }

    // séparateur logo | input
    g.setColour (col.withAlpha (0.25f));
    g.fillRect (juce::Rectangle<float> ((float) logoButton.getRight() + 7.0f,
                                        b.getY() + 12.0f,
                                        1.0f,
                                        b.getHeight() - 24.0f));
}

void HeaderComponent::paint (juce::Graphics& g)
{
    auto header = getLocalBounds();

    juce::ColourGradient hbg (HarmoniaPalette::panelTop, 0.0f, 0.0f,
                              HarmoniaPalette::panel,    0.0f, (float) header.getHeight(),
                              false);
    g.setGradientFill (hbg);
    g.fillRect (header);

    g.setColour (juce::Colours::white.withAlpha (0.04f));
    g.fillRect (header.getX(), header.getY() + 1, header.getWidth(), 1);

    g.setColour (HarmoniaPalette::border);
    g.fillRect (header.getX(), header.getBottom() - 2, header.getWidth(), 1);

    g.setColour (modeColour().withAlpha (0.40f));
    g.fillRect (header.getX(), header.getBottom() - 1, header.getWidth(), 1);

    // capsule Load | Export
    {
        // même géométrie que ModelPicker::paint (reduced 1 px) => même hauteur visible
        auto r = fileGroupBounds.toFloat().reduced (1.0f);

        g.setColour (juce::Colours::black.withAlpha (0.28f));
        g.fillRoundedRectangle (r, 10.0f);

        g.setColour (HarmoniaPalette::border);
        g.drawRoundedRectangle (r.reduced (0.5f), 10.0f, 1.0f);

        g.fillRect (juce::Rectangle<float> (r.getCentreX() - 0.5f, r.getY() + 8.0f,
                                            1.0f, r.getHeight() - 16.0f));
    }

    paintPromptBar (g);
}

void HeaderComponent::setPresetName (const juce::String& name)
{
    presetLabel.setText (name.toUpperCase(), juce::dontSendNotification);
}

void HeaderComponent::resized()
{
    auto row = getLocalBounds().reduced (16, 16);
    const int cy = row.getCentreY();

    // ---------- GAUCHE : maison + pseudo, puis couleurs dessous ----------
    const int homeSize   = 30;
    const int topY       = 6;
    const int titleNudge = 1;
    const int homeGap    = 4;     // pseudo collé à la maison
    const int leftZoneW  = 124;   // largeur FIXE de la zone gauche, quel que soit le pseudo

    homeButton.setBounds (row.getX(), topY, homeSize, homeSize);

    const int titleX    = homeButton.getRight() + homeGap;
    const int titleMaxW = row.getX() + leftZoneW - titleX;
    const int textW     = titleLabel.getFont().getStringWidth (titleLabel.getText()) + 4;

    titleLabel.setBounds (titleX, topY + titleNudge, juce::jmin (titleMaxW, textW), homeSize);

    paletteSelector.setBounds (row.getX(), homeButton.getBottom() + 6, 118, 24);

    // Ne dépend plus du pseudo : le reste du layout ne bouge plus
    const int leftEdge = row.getX() + leftZoneW;

    // ---------- DROITE : modèle + load/export (même axe que la barre) ----------
    const int rowH      = 34;
    const int barCy     = cy - 6;      // <- descends/monte la barre ET les boutons ici
    const int pickerW   = 140;
    const int pickerGap = 10;

    // Historique : capsule "cartes empilées + nombre" (~64 px)
    const int historyW = 64;

    // Un seul écart pour les deux côtés de la barre
    const int barGap = 20;

    fileGroupBounds = { row.getRight() - 68, barCy - rowH / 2, 68, rowH };
    loadButton.setBounds (fileGroupBounds.withWidth (34));
    saveButton.setBounds (fileGroupBounds.withTrimmedLeft (34));

    const int modelX = fileGroupBounds.getX() - pickerGap - pickerW;

    // ---------- CENTRE : barre de prompt + preset dessous ----------
    const int barH = 42, labelH = 14, gap = 4;
    const int top = barCy - barH / 2;
    const int cx  = getWidth() / 2;

    // Demi-largeur max : à gauche (zone pseudo + historique), à droite (model picker)
    const int halfLeft  = cx - (leftEdge + pickerGap + historyW + barGap);
    const int halfRight = modelX - barGap - cx;
    const int half      = juce::jmin (halfLeft, halfRight, 310);   // 310 = 620 / 2

    if (half >= 170)   // barre >= 340 px
    {
        // Barre parfaitement centrée, écart identique des deux côtés
        promptBounds = { cx - half, top, half * 2, barH };

        historyPicker.setBounds (promptBounds.getX() - barGap - historyW,
                                 barCy - rowH / 2, historyW, rowH);

        modelPicker.setBounds (promptBounds.getRight() + barGap,
                               barCy - rowH / 2, pickerW, rowH);
    }
    else
    {
        // Fenêtre étroite : on revient à un placement entre les deux
        modelPicker.setBounds (modelX, barCy - rowH / 2, pickerW, rowH);

        historyPicker.setBounds (leftEdge + pickerGap, barCy - rowH / 2, historyW, rowH);

        const int barX = historyPicker.getRight() + barGap;
        promptBounds = { barX, top,
                         juce::jmax (0, modelPicker.getX() - barGap - barX), barH };
    }

    // nom du preset : du bord gauche du logo au bord droit du bouton GENERATE
    presetLabel.setBounds (promptBounds.getX() + 6, promptBounds.getBottom() + gap,
                           promptBounds.getWidth() - 12, labelH);

    auto bar = promptBounds.reduced (6, 5);

    logoButton.setBounds (bar.removeFromLeft (bar.getHeight()));
    bar.removeFromLeft (14);

    actionButton.setBounds (bar.removeFromRight (100).withSizeKeepingCentre (100, 30));
    bar.removeFromRight (10);

    promptEditor.setBounds (bar);
}