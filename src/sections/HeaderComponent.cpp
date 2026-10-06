#include "HeaderComponent.h"
#include <BinaryData.h>   // si l'include échoue, utilise le même que dans ton composant parent

//==============================================================================
// IconButton
//==============================================================================
juce::Path IconButton::makePath (Icon i)
{
    juce::Path p;   // boîte 24x24

    switch (i)
    {
        case Icon::Home:
            p.startNewSubPath (3.0f, 11.5f);
            p.lineTo (12.0f, 3.5f);
            p.lineTo (21.0f, 11.5f);

            p.startNewSubPath (5.5f, 9.5f);
            p.lineTo (5.5f, 20.5f);
            p.lineTo (10.0f, 20.5f);
            p.lineTo (10.0f, 14.5f);
            p.lineTo (14.0f, 14.5f);
            p.lineTo (14.0f, 20.5f);
            p.lineTo (18.5f, 20.5f);
            p.lineTo (18.5f, 9.5f);
            break;

        case Icon::Import:
            p.startNewSubPath (12.0f, 4.0f);   p.lineTo (12.0f, 15.0f);
            p.startNewSubPath (7.5f, 10.5f);   p.lineTo (12.0f, 15.0f);  p.lineTo (16.5f, 10.5f);
            p.startNewSubPath (4.0f, 16.0f);   p.lineTo (4.0f, 20.0f);   p.lineTo (20.0f, 20.0f);  p.lineTo (20.0f, 16.0f);
            break;

        case Icon::Export:
            p.startNewSubPath (12.0f, 15.0f);  p.lineTo (12.0f, 4.0f);
            p.startNewSubPath (7.5f, 8.5f);    p.lineTo (12.0f, 4.0f);   p.lineTo (16.5f, 8.5f);
            p.startNewSubPath (4.0f, 16.0f);   p.lineTo (4.0f, 20.0f);   p.lineTo (20.0f, 20.0f);  p.lineTo (20.0f, 16.0f);
            break;
    }

    return p;
}

void IconButton::timerCallback()
{
    const float target = (isOver() || isDown()) ? 1.0f : 0.0f;
    hoverAnim += (target - hoverAnim) * 0.30f;

    if (std::abs (target - hoverAnim) < 0.01f)
    {
        hoverAnim = target;
        stopTimer();
    }

    repaint();
}

void IconButton::paintButton (juce::Graphics& g, bool, bool down)
{
    auto b = getLocalBounds().toFloat();
    const auto accent = HarmoniaPalette::accent;
    const float h = hoverAnim;

    if (h > 0.01f)
    {
        auto r = b.reduced (3.0f);

        // glow radial
        juce::ColourGradient glow (accent.withAlpha (0.30f * h), r.getCentre(),
                                   accent.withAlpha (0.0f),      r.getTopLeft(),
                                   true);
        g.setGradientFill (glow);
        g.fillRoundedRectangle (r, 8.0f);

        // ligne néon qui s'élargit
        const float w = r.getWidth() * 0.55f * h;
        g.setColour (accent.withAlpha (0.90f * h));
        g.fillRoundedRectangle (b.getCentreX() - w * 0.5f, b.getBottom() - 4.0f, w, 1.5f, 0.75f);
    }

    auto area = b.reduced (8.0f);
    if (down)
        area = area.reduced (1.0f);

    auto path = makePath (icon);
    path.applyTransform (path.getTransformToScaleToFit (area, true));

    g.setColour (HarmoniaPalette::textMuted.interpolatedWith (accent.brighter (0.3f), h));
    g.strokePath (path, juce::PathStrokeType (1.5f,
                                              juce::PathStrokeType::curved,
                                              juce::PathStrokeType::rounded));
}

//==============================================================================
// LogoButton
//==============================================================================
void LogoButton::setRefine (bool)
{
    // Le logo ne change plus selon le mode (conservé pour compatibilité avec le .h)
}

void LogoButton::timerCallback()
{
    const float targetHover = (isOver() || isDown()) ? 1.0f : 0.0f;
    hoverAnim += (targetHover - hoverAnim) * 0.30f;

    bool busy = std::abs (targetHover - hoverAnim) >= 0.01f;

    if (! busy)
        hoverAnim = targetHover;

    if (spinning)
    {
        spinProgress += 0.035f;          // ~0.5 s pour un tour : augmente = plus rapide

        if (spinProgress >= 1.0f)
        {
            spinProgress = 0.0f;
            spinning = false;
        }
        else
        {
            busy = true;
        }
    }

    if (! busy)
        stopTimer();

    repaint();
}

void LogoButton::paintButton (juce::Graphics& g, bool, bool down)
{
    auto b = getLocalBounds().toFloat().reduced (1.0f);

    // fond + glow au survol
    g.setColour (juce::Colours::black.withAlpha (0.28f));
    g.fillEllipse (b);

    if (hoverAnim > 0.01f)
    {
        juce::ColourGradient glow (tint.withAlpha (0.30f * hoverAnim), b.getCentre(),
                                   tint.withAlpha (0.0f), b.getTopLeft(), true);
        g.setGradientFill (glow);
        g.fillEllipse (b);
    }

    // anneau plein (toujours, quel que soit le mode)
    g.setColour (tint.withAlpha (0.85f));
    g.drawEllipse (b, 1.4f);

    // logo : un tour complet sur lui-même au clic (ease-out)
    if (logo.isValid())
    {
        auto area = b.reduced (down ? 6.5f : 5.0f);

        juce::Graphics::ScopedSaveState state (g);

        if (spinning)
        {
            const float inv   = 1.0f - spinProgress;
            const float eased = 1.0f - inv * inv * inv;
            g.addTransform (juce::AffineTransform::rotation (eased * juce::MathConstants<float>::twoPi,
                                                             b.getCentreX(), b.getCentreY()));
        }

        g.setOpacity (0.8f + 0.2f * hoverAnim);
        g.drawImage (logo, area, juce::RectanglePlacement::centred);
    }
}

//==============================================================================
// ActionButton
//==============================================================================
void ActionButton::paintButton (juce::Graphics& g, bool over, bool down)
{
    auto b = getLocalBounds().toFloat().reduced (1.0f);
    const float r = juce::jmap (sharp, b.getHeight() * 0.5f, 3.0f);

    g.setColour (tint.withMultipliedBrightness (down ? 0.85f : (over ? 1.12f : 1.0f)));
    g.fillRoundedRectangle (b, r);

    // léger reflet
    juce::ColourGradient gloss (juce::Colours::white.withAlpha (0.20f), 0.0f, b.getY(),
                                juce::Colours::white.withAlpha (0.0f),  0.0f, b.getCentreY(),
                                false);
    g.setGradientFill (gloss);
    g.fillRoundedRectangle (b, r);

    g.setColour (juce::Colours::black.withAlpha (0.85f));
    g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold"))
                   .withExtraKerningFactor (0.14f));
    g.drawText (getButtonText().toUpperCase(), getLocalBounds(),
                juce::Justification::centred, false);
}

//==============================================================================
// ModelPicker
//==============================================================================
juce::String ModelPicker::getSelectedName() const
{
    for (const auto& it : items)
        if (it.id == selectedId)
            return it.name;

    return {};
}

void ModelPicker::timerCallback()
{
    const float target = (isMouseOver() || menuOpen) ? 1.0f : 0.0f;
    hoverAnim += (target - hoverAnim) * 0.30f;

    if (std::abs (target - hoverAnim) < 0.01f)
    {
        hoverAnim = target;
        stopTimer();
    }

    repaint();
}

void ModelPicker::showMenu()
{
    juce::PopupMenu menu;
    menu.setLookAndFeel (&menuLnF);

    for (const auto& it : items)
        menu.addItem (it.id, it.name, true, it.id == selectedId);

    menuOpen = true;
    startTimerHz (60);

    menu.showMenuAsync (juce::PopupMenu::Options()
                            .withTargetComponent (this)
                            .withPreferredPopupDirection (juce::PopupMenu::Options::PopupDirection::downwards)
                            .withMinimumWidth (getWidth())
                            .withStandardItemHeight (36),
                        [safe = juce::Component::SafePointer<ModelPicker> (this)] (int result)
    {
        if (safe == nullptr)
            return;

        safe->menuOpen = false;
        safe->startTimerHz (60);

        if (result != 0 && result != safe->selectedId)
        {
            safe->selectedId = result;
            safe->repaint();

            if (safe->onChange)
                safe->onChange();
        }
    });
}

void ModelPicker::paint (juce::Graphics& g)
{
    auto b = getLocalBounds().toFloat().reduced (1.0f);
    const auto acc = HarmoniaPalette::accent;
    const float h = hoverAnim;
    const float r = 10.0f;

    // fond
    g.setColour (juce::Colours::black.withAlpha (0.28f));
    g.fillRoundedRectangle (b, r);

    // lueur qui part de la gauche
    if (h > 0.01f)
    {
        juce::ColourGradient glow (acc.withAlpha (0.20f * h), b.getX() + 16.0f, b.getCentreY(),
                                   acc.withAlpha (0.0f),      b.getRight(),      b.getCentreY(),
                                   false);
        g.setGradientFill (glow);
        g.fillRoundedRectangle (b, r);
    }

    // bordure
    g.setColour (HarmoniaPalette::border.interpolatedWith (acc.withAlpha (0.85f), h));
    g.drawRoundedRectangle (b.reduced (0.5f), r, 1.0f);

    // point lumineux
    const juce::Point<float> dot (b.getX() + 15.0f, b.getCentreY());

    g.setColour (acc.withAlpha (0.20f + 0.15f * h));
    g.fillEllipse (juce::Rectangle<float> (16.0f, 16.0f).withCentre (dot));

    g.setColour (acc);
    g.fillEllipse (juce::Rectangle<float> (6.0f, 6.0f).withCentre (dot));

    // textes
    auto textArea = getLocalBounds().withTrimmedLeft (30).withTrimmedRight (26);

    g.setColour (HarmoniaPalette::textMuted);
    g.setFont (juce::Font (juce::FontOptions (8.0f).withStyle ("Bold"))
                   .withExtraKerningFactor (0.20f));
    g.drawText ("AI MODEL", textArea.getX(), 5, textArea.getWidth(), 10,
                juce::Justification::centredLeft, false);

    g.setColour (HarmoniaPalette::textPrimary.interpolatedWith (acc.brighter (0.3f), h));
    g.setFont (juce::Font (juce::FontOptions (12.5f).withStyle ("Bold")));
    g.drawText (getSelectedName(), textArea.getX(), 15, textArea.getWidth(), 15,
                juce::Justification::centredLeft, true);

    // chevron (se retourne quand le menu est ouvert)
    const float cx = (float) getWidth() - 14.0f;
    const float cy = (float) getHeight() * 0.5f;
    const float dir = menuOpen ? -1.0f : 1.0f;

    juce::Path chevron;
    chevron.startNewSubPath (cx - 4.0f, cy - 1.5f * dir);
    chevron.lineTo          (cx,        cy + 2.0f * dir);
    chevron.lineTo          (cx + 4.0f, cy - 1.5f * dir);

    g.setColour (HarmoniaPalette::textMuted.interpolatedWith (acc, h));
    g.strokePath (chevron, juce::PathStrokeType (1.5f,
                                                 juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
}

//==============================================================================
// HeaderComponent
//==============================================================================
HeaderComponent::HeaderComponent (const UserSession& s)
    : session (s)
{
    // Titre
    juce::String username = session.pseudo.isNotEmpty()
        ? session.pseudo
        : Strings::Errors::NoUserConnected;

    titleLabel.setText (username.toUpperCase(), juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions (17.0f).withStyle ("Bold"))
                            .withExtraKerningFactor (0.10f));
    titleLabel.setBorderSize ({ 0, 5, 0, 5 });   // plus de marge verticale parasite
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

    // Modèle IA
    for (const auto& m : aiModels)
        modelPicker.addItem (m.name, (int) m.id);

    modelPicker.setSelectedId ((int) AIModel::Model1);
    modelPicker.setTooltip ("Choose the AI model");
    addAndMakeVisible (modelPicker);

    // Palette : 3 pastilles libres (le thème est toujours Custom)
    addAndMakeVisible (paletteSelector);

    // Restauration depuis la session (à activer quand UserSession a ces champs) :
    // paletteSelector.setSlots (session.paletteColours, session.paletteSlot);

    HarmoniaPalette::setTheme (paletteSelector.getCurrentTheme());

    paletteSelector.onThemeUpdated = [this]
    {
        auto theme = paletteSelector.getCurrentTheme();
        HarmoniaPalette::setTheme (theme);

        applyColours();

        if (onThemeChanged)
            onThemeChanged (theme);

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

    // Logo = switch de mode
    logoButton.onClick = [this]
    {
        setMode (mode == Mode::Generate ? Mode::Refine : Mode::Generate);
    };
    addAndMakeVisible (logoButton);

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

void HeaderComponent::mouseDown (const juce::MouseEvent& e)
{
    if (e.originalComponent != &promptEditor && ! promptEditor.isParentOf (e.originalComponent))
        promptEditor.giveAwayKeyboardFocus();
}

//==============================================================================
// La couleur ne dépend plus du mode : seul le style de la barre change
juce::Colour HeaderComponent::modeColour() const
{
    return HarmoniaPalette::accent;
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
    // Même taille visuelle dans les deux modes (voir PromptFont dans le .h)
    promptEditor.applyFontToAllText (PromptFont::make (refine));
    promptEditor.repaint();
}

//==============================================================================
AIModel HeaderComponent::getSelectedModel() const
{
    return static_cast<AIModel> (modelPicker.getSelectedId());
}

int HeaderComponent::getSelectedModelId() const
{
    return modelPicker.getSelectedId();
}

juce::String HeaderComponent::getSelectedModelName() const
{
    auto id = getSelectedModel();

    for (const auto& m : aiModels)
        if (m.id == id)
            return m.name;

    return {};
}

juce::String HeaderComponent::getSelectedBackendName() const
{
    auto id = getSelectedModel();

    for (const auto& m : aiModels)
        if (m.id == id)
            return m.backendname;

    return {};
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

                constexpr int   nBars = 16;
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

void HeaderComponent::resized()
{
    auto row = getLocalBounds().reduced (16, 16);
    const int cy = row.getCentreY();

    // ---------- GAUCHE : maison + pseudo, puis couleurs dessous ----------
    const int homeSize = 30;
    const int topY = 6;
    const int titleNudge = 1;   // 1 ou 2 px : ajuste à l'œil pour aligner pseudo et maison

    homeButton.setBounds (row.getX(), topY, homeSize, homeSize);

    const int titleW = juce::jmin (200, titleLabel.getFont().getStringWidth (titleLabel.getText()) + 8);
    titleLabel.setBounds (homeButton.getRight() + 10, topY + titleNudge, titleW, homeSize);

    paletteSelector.setBounds (row.getX(), homeButton.getBottom() + 6, 118, 24);

    const int leftEdge = juce::jmax (titleLabel.getRight(), paletteSelector.getRight());

    // ---------- DROITE : modèle + load/export (même axe que la barre) ----------
    const int rowH  = 34;
    const int barCy = cy - 6;          // <- descends/monte la barre ET les boutons ici

    fileGroupBounds = { row.getRight() - 68, barCy - rowH / 2, 68, rowH };
    loadButton.setBounds (fileGroupBounds.withWidth (34));
    saveButton.setBounds (fileGroupBounds.withTrimmedLeft (34));

    modelPicker.setBounds (fileGroupBounds.getX() - 10 - 140, barCy - rowH / 2, 140, rowH);
    const int rightEdge = modelPicker.getX();

    // ---------- CENTRE : barre de prompt + preset dessous ----------
    const int barH = 42, labelH = 14, gap = 4;
    const int top = barCy - barH / 2;  // la barre est centrée sur le même axe

    const int sideW = juce::jmax (leftEdge - row.getX(), row.getRight() - rightEdge);
    const int centredW = row.getWidth() - 2 * sideW - 32;

    if (centredW >= 340)
    {
        const int w = juce::jmin (centredW, 620);
        promptBounds = { getWidth() / 2 - w / 2, top, w, barH };
    }
    else
    {
        promptBounds = { leftEdge + 16, top, juce::jmax (0, rightEdge - leftEdge - 32), barH };
    }

    presetLabel.setBounds (promptBounds.getX(), promptBounds.getBottom() + gap,
                           promptBounds.getWidth(), labelH);

    auto bar = promptBounds.reduced (6, 5);

    logoButton.setBounds (bar.removeFromLeft (bar.getHeight()));
    bar.removeFromLeft (14);

    actionButton.setBounds (bar.removeFromRight (100).withSizeKeepingCentre (100, 30));
    bar.removeFromRight (10);

    promptEditor.setBounds (bar);
}