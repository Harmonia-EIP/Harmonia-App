#include "MainComponent.h"

MainComponent::MainComponent (HarmoniaAudioProcessor& p,
                              BackendManager& be,
                              const UserSession& s,
                              const AIModelList& models)
    : processor (p),
      backend (be),
      session (s),
      oscilloscope (AppConfig::Oscilloscope::BufferSize,
                    AppConfig::Oscilloscope::RefreshRate),
      displayScreen (oscilloscope),
      synthComponent (p.getKeyboardState()),
      ampEnvViz (p.getAPVTS(), HarmoniaPalette::sectionAmpEnv),
      lfoViz   (p.getAPVTS(), HarmoniaPalette::sectionLfo)
{
    setLookAndFeel (&lookAndFeel);
    tooltipWindow.setOpaque (false);

    oscMixPanel  = std::make_unique<SectionPanel> ("Osc 1 / Mix",  HarmoniaPalette::sectionOsc1);
    filterPanel  = std::make_unique<SectionPanel> ("Filter",       HarmoniaPalette::sectionFilter);
    screenPanel  = std::make_unique<SectionPanel> ("Display",      HarmoniaPalette::sectionDisplay);
    lfoPanel     = std::make_unique<SectionPanel> ("LFO",          HarmoniaPalette::sectionLfo);
    osc2Panel    = std::make_unique<SectionPanel> ("Osc 2",        HarmoniaPalette::sectionOsc2);
    fxPanel      = std::make_unique<SectionPanel> ("Effects",      HarmoniaPalette::sectionFx);
    ampPanel     = std::make_unique<SectionPanel> ("Amp Envelope", HarmoniaPalette::sectionAmpEnv);

    processor.setOscilloscope (&oscilloscope);

    addAndMakeVisible (particles);
    particles.toBack();

    // Référence « état serveur » pour le filtre anti-PUT inutile (vide = invité).
    backend.resetPaletteSyncState (session.isGuest ? juce::StringArray()
                                                   : session.paletteColours,
                                   session.paletteSlot);

    // Le header restaure la palette (setSlots) depuis la session dans son
    // constructeur, avant HarmoniaPalette::setTheme. applyTheme() doit donc
    // venir APRES sa creation.
    headerComponent = std::make_unique<HeaderComponent> (session, models);
    addAndMakeVisible (*headerComponent);

    // Restauration depuis le state de CETTE instance (avant les callbacks)
    {
        const juce::String identity = session.isGuest ? juce::String ("guest")
                                                      : "user_" + juce::String (session.userId);
        auto& st = processor.getAPVTS().state;

        // Identité différente de la dernière fois : on oublie l'état de l'ancienne
        if (st.getProperty (UiState::identity).toString() != identity)
        {
            st.removeProperty (UiState::presetName,   nullptr);
            st.removeProperty (UiState::paletteSlot,  nullptr);
            st.removeProperty (UiState::themeId,      nullptr);
            st.removeProperty (UiState::customColour, nullptr);
        }

        st.setProperty (UiState::identity, identity, nullptr);

        // Restauration (ne trouvera rien si on vient d'effacer)
        if (st.hasProperty (UiState::paletteSlot))
        {
            headerComponent->restoreSlot ((int) st[UiState::paletteSlot]);
            currentTheme = headerComponent->getPaletteSelector().getCurrentTheme();
        }

        restorePresetLabel();
    }

    headerComponent->onThemeChanged = [this] (HarmoniaPalette::Theme theme)
    {
        currentTheme = theme;
        saveThemeToState();
        applyTheme();
    };

    headerComponent->onPaletteChanged = [this] (const juce::StringArray& colours, int slot)
    {
        saveThemeToState (slot);
        backend.updatePaletteAsync (colours, slot);
    };

    applyTheme();

    addAndMakeVisible (oscMixPanel.get());
    addAndMakeVisible (filterPanel.get());
    addAndMakeVisible (screenPanel.get());
    addAndMakeVisible (lfoPanel.get());
    addAndMakeVisible (osc2Panel.get());
    addAndMakeVisible (fxPanel.get());
    addAndMakeVisible (ampPanel.get());

    screenPanel->addAndMakeVisible (displayScreen);
    lfoPanel->addAndMakeVisible (lfoViz);
    ampPanel->addAndMakeVisible (ampEnvViz);

    buildControls();
    wireHeaderButtons();

    headerComponent->onModeChanged = [this] (HeaderComponent::Mode m)
    {
        setRefineUi (m == HeaderComponent::Mode::Refine);
    };

    // --- Historique : le header renvoie l'entrée complète ---------------------

    headerComponent->onHistorySelected = [this] (const HeaderComponent::HistoryList::value_type& entry)
    {
        loadHistoryEntry (entry);
    };

        headerComponent->onHistoryDeleted = [safe = juce::Component::SafePointer<MainComponent> (this),
                                         &be = backend] (const HeaderComponent::HistoryList::value_type& entry)
    {
        juce::Thread::launch ([safe, &be, id = entry.id]
        {
            if (be.deleteHistoryEntry (id))
                return;

            juce::MessageManager::callAsync ([safe]
            {
                if (safe == nullptr)
                    return;

                HarmoniaAlert::warning (safe.getComponent(),
                                        "Could not delete preset",
                                        "The preset was restored. Please try again.",
                                        HarmoniaAlert::Position::TopCenter);
                safe->refreshHistoryAsync();
            });
        });
    };

    headerComponent->onHistoryCleared = [safe = juce::Component::SafePointer<MainComponent> (this),
                                         &be = backend]
    {
        juce::Thread::launch ([safe, &be]
        {
            if (be.clearHistory())
                return;

            juce::MessageManager::callAsync ([safe]
            {
                if (safe == nullptr)
                    return;

                HarmoniaAlert::warning (safe.getComponent(),
                                        "Could not clear history",
                                        "Your history was restored. Please try again.",
                                        HarmoniaAlert::Position::TopCenter);
                safe->refreshHistoryAsync();
            });
        });
    };

    addAndMakeVisible (synthComponent);

    // Disposition du clavier d'ordinateur (touche B) : retenue avec le projet / la session
    {
        auto& kb = synthComponent.getKeyboard();
        auto& st = processor.getAPVTS().state;

        if (st.getProperty (UiState::keyLayout).toString() == "qwerty")
            kb.setLayout (HarmoniaKeyboard::Layout::Qwerty);

        kb.onLayoutChanged = [&st] (HarmoniaKeyboard::Layout l)
        {
            st.setProperty (UiState::keyLayout, l == HarmoniaKeyboard::Layout::Qwerty ? "qwerty" : "azerty", nullptr);
        };

        // flèches : réglage fin du dernier knob / sélecteur cliqué ; Échap retire la cible
        kb.onArrowKey  = [this] (int dir, bool fine) { return nudgeArrowTarget (dir, fine); };
        kb.onEscapeKey = [this]
        {
            if (arrowTarget == nullptr)
                return false;
            setArrowTarget (nullptr);
            return true;
        };
    }

    // Clic sur un knob / bouton / sélecteur : le clavier d'ordinateur rejoue des notes tout de suite
    addMouseListener (this, true);

    setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);

    // Récupère la liste des modèles IA sans bloquer l'ouverture de la fenêtre.
    refreshModelsAsync();

    // Récupère palette serveur + historique sans bloquer non plus.
    refreshProfileAsync();
}

MainComponent::~MainComponent()
{
    // Envoie la palette en attente (debounce non expiré) sans bloquer :
    // le backend copie tout par valeur et lance un PUT court détaché.
    backend.flushPaletteIfPending();

    removeMouseListener (this);
    processor.setOscilloscope (nullptr);
    setLookAndFeel(nullptr);
}

//==============================================================================
// Focus du clavier
//==============================================================================
void MainComponent::mouseUp (const juce::MouseEvent& e)
{
    auto* origin = e.originalComponent;

    // Knob ou sélecteur cliqué : il devient la cible des flèches. Clic ailleurs (hors piano
    // et champs texte) : plus de cible.
    {
        juce::Component* lockable = nullptr;
        for (auto* c = origin; c != nullptr && c != this; c = c->getParentComponent())
            if (dynamic_cast<LockableControl*> (c) != nullptr) { lockable = c; break; }

        if (lockable != nullptr)
            setArrowTarget (lockable);
        else if (! synthComponent.isKeyboard (origin)
                 && dynamic_cast<juce::TextEditor*> (origin) == nullptr)
            setArrowTarget (nullptr);
    }

    // Clic dans un champ texte (prompt, code hex de la palette) : il garde le focus.
    // Clic sur le piano : il prend déjà le focus tout seul.
    if (dynamic_cast<juce::TextEditor*> (origin) != nullptr
        || (origin != nullptr && origin->findParentComponentOfClass<juce::TextEditor>() != nullptr)
        || synthComponent.isKeyboard (origin))
        return;

    focusKeyboardSoon();
}

void MainComponent::setArrowTarget (juce::Component* c)
{
    if (c == arrowTarget.getComponent())
        return;

    if (auto* old = dynamic_cast<LockableControl*> (arrowTarget.getComponent()))
        old->setArrowTarget (false);

    arrowTarget = c;

    if (auto* now = dynamic_cast<LockableControl*> (c))
        now->setArrowTarget (true);
}

bool MainComponent::nudgeArrowTarget (int dir, bool fine)
{
    auto* target = arrowTarget.getComponent();
    auto* lockable = dynamic_cast<LockableControl*> (target);

    // pas de cible, ou cible grisée (panneaux analogiques en mode DX7, Spread à une voix)
    if (lockable == nullptr || ! target->isEnabled() || ! target->isShowing())
        return false;

    auto* param = processor.getAPVTS().getParameter (lockable->getParamId());
    if (param == nullptr)
        return false;

    const int steps = param->getNumSteps();
    const float step = (param->isDiscrete() && steps > 1) ? 1.0f / (float) (steps - 1)   // un cran
                                                         : (fine ? 0.002f : 0.01f);     // % de la course

    const float next = juce::jlimit (0.0f, 1.0f, param->getValue() + (float) dir * step);

    if (next != param->getValue())
    {
        param->beginChangeGesture();
        param->setValueNotifyingHost (next);
        param->endChangeGesture();
    }

    return true;   // touche utilisée même en butée : elle ne part pas ailleurs
}

void MainComponent::focusKeyboardSoon()
{
    // Asynchrone : on passe APRÈS ce que le clic a lui-même déclenché (ouverture
    // de l'historique qui donne le focus à sa recherche, menu, etc.).
    juce::MessageManager::callAsync ([safe = juce::Component::SafePointer<MainComponent> (this)]
    {
        if (safe == nullptr || ! safe->isShowing())
            return;

        // Menu déroulant ou fenêtre modale ouverte (modèle IA, alerte…) : prendre
        // le focus la fermerait.
        if (juce::ModalComponentManager::getInstance()->getNumModalComponents() > 0)
            return;

        // Un champ texte hors de ce composant (recherche de l'historique, attachée
        // à la fenêtre) vient de prendre le focus : on ne le lui vole pas.
        auto* focused = juce::Component::getCurrentlyFocusedComponent();
        if (dynamic_cast<juce::TextEditor*> (focused) != nullptr && ! safe->isParentOf (focused))
            return;

        safe->synthComponent.focusKeyboard();
    });
}

void MainComponent::activatePalette()
{
    if (headerComponent != nullptr)
        HarmoniaPalette::setCustomAccent (headerComponent->getPaletteSelector().getActiveColour());
}

void MainComponent::setRefineUi (bool refine)
{
    for (auto* l : lockables)
        l->setLockUiVisible (refine);
}

juce::StringArray MainComponent::getLockedParamIds() const
{
    juce::StringArray ids;
    for (auto* l : lockables)
        if (l->isLocked())
            ids.add (l->getParamId());
    return ids;
}

void MainComponent::applyTheme()
{
    activatePalette();
    HarmoniaPalette::setTheme (currentTheme);

    oscMixPanel->setAccentColour (HarmoniaPalette::sectionOsc1);
    osc2Panel->setAccentColour   (HarmoniaPalette::sectionOsc2);
    screenPanel->setAccentColour (HarmoniaPalette::sectionDisplay);
    if (outputStrip != nullptr)
        outputStrip->repaint();

    lookAndFeel.refreshTheme();
    sendLookAndFeelChange();
    repaint();
}

void MainComponent::registerJuiceFor (juce::Component* c)
{
    if (auto* k = dynamic_cast<KnobControl*> (c))
    {
        k->onFastTweak = [this, c]
        {
            const auto centreInK = juce::Point<float> ((float) c->getWidth(), (float) c->getHeight()) * 0.5f;
            const auto centreInScreen = c->localPointToGlobal (centreInK);
            const auto centreInParticles = particles.getLocalPoint (nullptr, centreInScreen);
            particles.emitBurst (centreInParticles, 4,
                     headerComponent->getPaletteSelector().getActiveColour());
        };
    }
}

void MainComponent::buildControls()
{
    auto& a = processor.getAPVTS();

    osc1WaveSel    = std::make_unique<WaveformSelector>(a, HarmoniaParams::IDs::osc1Waveform);
    osc1WaveSel->setLockStripHeight (IconChoiceSelector::defaultLockStripH);
    osc1WaveSel->setColumns (2);   // grille 2 x 2, à côté de Mix / Noise
    oscMixKnob     = std::make_unique<KnobControl>     (a, HarmoniaParams::IDs::oscMix,     "Mix");
    noiseLevelKnob = std::make_unique<KnobControl>     (a, HarmoniaParams::IDs::noiseLevel, "Noise");
    oscMixPanel->addAndMakeVisible (*osc1WaveSel);
    oscMixPanel->addAndMakeVisible (*oscMixKnob);
    oscMixPanel->addAndMakeVisible (*noiseLevelKnob);

    filterCutoffKnob   = std::make_unique<KnobControl>      (a, HarmoniaParams::IDs::filterCutoff,    "Cutoff");
    filterResoKnob     = std::make_unique<KnobControl>      (a, HarmoniaParams::IDs::filterResonance, "Reso");
    filterTypeSel      = std::make_unique<FilterTypeSelector>(a, HarmoniaParams::IDs::filterType);
    filterTypeSel->setLockStripHeight (IconChoiceSelector::defaultLockStripH);
    filterEnvAmtKnob   = std::make_unique<KnobControl>      (a, HarmoniaParams::IDs::filterEnvAmount, "F.Env Amt", true);
    filterEnvDecayKnob = std::make_unique<KnobControl>      (a, HarmoniaParams::IDs::filterEnvDecay,  "F.Env Dec");
    velocityFilterKnob = std::make_unique<KnobControl>      (a, HarmoniaParams::IDs::velocityToFilter,"Vel >Flt");
    filterPanel->addAndMakeVisible (*filterCutoffKnob);
    filterPanel->addAndMakeVisible (*filterResoKnob);
    filterPanel->addAndMakeVisible (*filterTypeSel);
    filterPanel->addAndMakeVisible (*filterEnvAmtKnob);
    filterPanel->addAndMakeVisible (*filterEnvDecayKnob);
    filterPanel->addAndMakeVisible (*velocityFilterKnob);

    lfoRateKnob     = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::lfoRate,     "Rate");
    lfoToPitchKnob  = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::lfoToPitch,  "to Pitch");
    lfoToCutoffKnob = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::lfoToCutoff, "to Cutoff");
    lfoPanel->addAndMakeVisible (*lfoRateKnob);
    lfoPanel->addAndMakeVisible (*lfoToPitchKnob);
    lfoPanel->addAndMakeVisible (*lfoToCutoffKnob);

    osc2WaveSel    = std::make_unique<WaveformSelector>(a, HarmoniaParams::IDs::osc2Waveform);
    osc2WaveSel->setLockStripHeight (IconChoiceSelector::defaultLockStripH);
    osc2DetuneKnob = std::make_unique<KnobControl>     (a, HarmoniaParams::IDs::osc2Detune, "Detune");
    osc2Panel->addAndMakeVisible (*osc2WaveSel);
    osc2Panel->addAndMakeVisible (*osc2DetuneKnob);

    distortionKnob = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::distortionMix, "Drive");
    reverbKnob     = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::reverbMix,     "Reverb");
    fxPanel->addAndMakeVisible (*distortionKnob);
    fxPanel->addAndMakeVisible (*reverbKnob);

    // v3 essentials
    unisonVoicesKnob = std::make_unique<KnobControl>(a, "unison_voices",  "Unison");
    unisonDetuneKnob = std::make_unique<KnobControl>(a, "unison_detune",  "Spread");
    stereoWidthKnob  = std::make_unique<KnobControl>(a, "stereo_width",   "Width");
    oscMixPanel->addAndMakeVisible (*unisonVoicesKnob);
    oscMixPanel->addAndMakeVisible (*unisonDetuneKnob);
    oscMixPanel->addAndMakeVisible (*stereoWidthKnob);

    osc2CoarseKnob = std::make_unique<KnobControl>(a, "osc_2_coarse", "Interval", true);
    osc2Panel->addAndMakeVisible (*osc2CoarseKnob);

    filterSlopeSel = std::make_unique<FilterSlopeSelector>(a, "filter_slope");
    filterSlopeSel->setLockOnLeft (true);   // cadenas à gauche du switch (pas sur "12")
    filterPanel->addAndMakeVisible (*filterSlopeSel);

    delayMixKnob   = std::make_unique<KnobControl>(a, "delay_mix",   "Delay");
    reverbSizeKnob = std::make_unique<KnobControl>(a, "reverb_size", "Size");
    fxPanel->addAndMakeVisible (*delayMixKnob);
    fxPanel->addAndMakeVisible (*reverbSizeKnob);

    screenPanel->addChildComponent (engineBadge);
    engineBadge.setText ("DX7");

    if (auto* engine = a.getParameter ("engine"))
    {
        engineWatch = std::make_unique<juce::ParameterAttachment> (*engine, [this] (float value) { showEngine (value > 0.5f); });
        engineWatch->sendInitialUpdate();
    }
    if (auto* voices = a.getParameter ("unison_voices"))
    {
        // the spread only exists with more than one voice
        unisonWatch = std::make_unique<juce::ParameterAttachment> (*voices, [this] (float value)
        {
            unisonDetuneKnob->setEnabled (value > 1.5f);
            unisonDetuneKnob->setAlpha (value > 1.5f ? 1.0f : 0.4f);
        });
        unisonWatch->sendInitialUpdate();
    }

    attackKnob  = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::ampAttack,  "Attack");
    decayKnob   = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::ampDecay,   "Decay");
    sustainKnob = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::ampSustain, "Sustain");
    releaseKnob = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::ampRelease, "Release");
    ampPanel->addAndMakeVisible (*attackKnob);
    ampPanel->addAndMakeVisible (*decayKnob);
    ampPanel->addAndMakeVisible (*sustainKnob);
    ampPanel->addAndMakeVisible (*releaseKnob);

    // Volume de sortie (hors charte : ni IA, ni presets, ni aléatoire) + vumètre.
    // Pas dans `lockables` : il n'y a rien à verrouiller pour l'IA.
    outputStrip = std::make_unique<OutputStrip> (a, [&proc = processor] { return proc.takeOutputPeak(); });
    addAndMakeVisible (*outputStrip);

    for (auto* c : {
            (juce::Component*) osc1WaveSel.get(),
            (juce::Component*) osc2WaveSel.get(),
            (juce::Component*) filterTypeSel.get(),
            (juce::Component*) filterSlopeSel.get(),
            (juce::Component*) unisonVoicesKnob.get(),
            (juce::Component*) unisonDetuneKnob.get(),
            (juce::Component*) stereoWidthKnob.get(),
            (juce::Component*) osc2CoarseKnob.get(),
            (juce::Component*) delayMixKnob.get(),
            (juce::Component*) reverbSizeKnob.get(),
            (juce::Component*) oscMixKnob.get(),
            (juce::Component*) noiseLevelKnob.get(),
            (juce::Component*) filterCutoffKnob.get(),
            (juce::Component*) filterResoKnob.get(),
            (juce::Component*) filterEnvAmtKnob.get(),
            (juce::Component*) filterEnvDecayKnob.get(),
            (juce::Component*) velocityFilterKnob.get(),
            (juce::Component*) lfoRateKnob.get(),
            (juce::Component*) lfoToPitchKnob.get(),
            (juce::Component*) lfoToCutoffKnob.get(),
            (juce::Component*) osc2DetuneKnob.get(),
            (juce::Component*) distortionKnob.get(),
            (juce::Component*) reverbKnob.get(),
            (juce::Component*) attackKnob.get(),
            (juce::Component*) decayKnob.get(),
            (juce::Component*) sustainKnob.get(),
            (juce::Component*) releaseKnob.get(),
        })
    {
        registerJuiceFor (c);
        if (auto* l = dynamic_cast<LockableControl*> (c))
            lockables.push_back (l);
    }
}

void MainComponent::showEngine (bool dx7)
{
    // a DX7 voice ignores the analog panels: dim them rather than hide them, so the layout stays put
    for (auto* panel : { oscMixPanel.get(), filterPanel.get(), lfoPanel.get(),
                         osc2Panel.get(), fxPanel.get(), ampPanel.get() })
    {
        panel->setEnabled (! dx7);
        panel->setAlpha (dx7 ? 0.35f : 1.0f);
    }
    engineBadge.setVisible (dx7);
}

void MainComponent::wireHeaderButtons()
{
    headerComponent->getLoadButton().onClick =
        [this] { doLoadPreset(); };

    headerComponent->getSaveButton().onClick =
        [this] { doSavePreset(); };

    headerComponent->getGenerateButton().onClick = [this]
    {
        if (headerComponent->getMode() == HeaderComponent::Mode::Refine)
            doRefineWithAi();
        else
            doGenerateWithAi();
    };
    headerComponent->onRandomize = [this]
    {
        const auto locked = headerComponent->getMode() == HeaderComponent::Mode::Refine
                                ? getLockedParamIds()
                                : juce::StringArray();

        if (auto* engine = processor.getAPVTS().getParameter ("engine"))
            engine->setValueNotifyingHost (0.0f);

        ParamRandomizer::randomize (processor.getAPVTS(), locked);
        setPresetName ("Random");
    };

    headerComponent->getLogoutButton().onClick =
        [this]
        {
            // AVANT clearSession() : sinon le token est effacé et le flush
            // ne peut plus s'authentifier.
            backend.flushPaletteIfPending();

            backend.clearSession();

            // Déconnexion explicite : on oublie tout l'état d'UI de cette identité
            auto& st = processor.getAPVTS().state;
            st.removeProperty (UiState::identity,     nullptr);
            st.removeProperty (UiState::presetName,   nullptr);
            st.removeProperty (UiState::paletteSlot,  nullptr);
            st.removeProperty (UiState::themeId,      nullptr);
            st.removeProperty (UiState::customColour, nullptr);

            if (onLogout)
                onLogout();
        };

    headerComponent->getPromptEditor()
        .setEnabled (true);

    headerComponent->getGenerateButton()
        .setEnabled (true);
}

void MainComponent::doLoadPreset()
{
    chooser = std::make_unique<juce::FileChooser>(
        "Load Harmonia preset (hel.json)",
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
        "*.json");

    chooser->launchAsync (
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [safe = juce::Component::SafePointer<MainComponent> (this)] (const juce::FileChooser& fc)
        {
            if (safe == nullptr)
                return;

            auto file = fc.getResult();
            if (! file.existsAsFile()) return;

            // Les cadenas ne comptent qu'en mode Refine (en Generate ils sont masques)
            const auto locked = safe->headerComponent->getMode() == HeaderComponent::Mode::Refine
                                    ? safe->getLockedParamIds()
                                    : juce::StringArray();

            auto result = PresetLoader::loadFromFile (file, safe->processor.getAPVTS(), locked);

            if (result.success)
            {
                safe->setPresetName (result.presetName);
            }
            else
            {
                HarmoniaAlert::error (safe.getComponent(), Strings::Errors::ErrorTitle,
                                      result.errorMessage, HarmoniaAlert::Position::Center);
                safe->restorePresetLabel();
            }
        });
}

//------------------------------------------------------------------------------
// Charge une entrée de l'historique :
//  - Generate : tous les paramètres sont remplacés
//  - Refine   : les paramètres verrouillés gardent leur valeur
void MainComponent::loadHistoryEntry (const HeaderComponent::HistoryList::value_type& entry)
{
    const auto locked = headerComponent->getMode() == HeaderComponent::Mode::Refine
                            ? getLockedParamIds()
                            : juce::StringArray();

    auto r = PresetLoader::loadFromParameters (entry.parameters, entry.name,
                                               processor.getAPVTS(), locked);

    if (! r.success)
    {
        HarmoniaAlert::error (this, Strings::Errors::ErrorTitle,
                              r.errorMessage.isNotEmpty() ? r.errorMessage
                                                          : Strings::Errors::UnreadableAIResponse,
                              HarmoniaAlert::Position::Center);
        return;
    }

    setPresetName (r.presetName);
    touchHistoryAsync (entry.id);

    // le panneau d'historique est hors de ce composant : on rend la main au clavier ici
    focusKeyboardSoon();
}

void MainComponent::saveThemeToState (int slot)
{
    if (slot < 0)
        slot = headerComponent->getPaletteSelector().getActiveSlot();

    auto& st = processor.getAPVTS().state;
    st.setProperty (UiState::themeId,      (int) currentTheme, nullptr);
    st.setProperty (UiState::customColour,
                headerComponent->getPaletteSelector().getActiveColour().toString(), nullptr);
    st.setProperty (UiState::paletteSlot,  slot, nullptr);
}

void MainComponent::setPresetName (const juce::String& name)
{
    const auto up = name.toUpperCase();

    // 1. affichage
    headerComponent->getPresetLabel().setText (up, juce::dontSendNotification);

    // 2. sauvegarde dans le state du processor
    processor.getAPVTS().state.setProperty (UiState::presetName, up, nullptr);
}

void MainComponent::restorePresetLabel()
{
    auto& st = processor.getAPVTS().state;

    headerComponent->getPresetLabel().setText (
        st.hasProperty (UiState::presetName) ? st[UiState::presetName].toString()
                                             : Strings::Labels::UnsetPreset.toUpperCase(),
        juce::dontSendNotification);
}

void MainComponent::doSavePreset()
{
    chooser = std::make_unique<juce::FileChooser>(
        "Save Harmonia preset",
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory)
            .getChildFile ("harmonia_preset.json"),
        "*.json");

    chooser->launchAsync (
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (file == juce::File()) return;
            if (file.getFileExtension().isEmpty())
                file = file.withFileExtension (".json");
            juce::String pseudo = session.pseudo.isNotEmpty() ? session.pseudo : "Unknown";

            juce::String presetName = file.getFileNameWithoutExtension();

            const auto json = PresetLoader::saveToJsonString (processor.getAPVTS(), presetName, pseudo);
            file.replaceWithText (json);
            setPresetName (file.getFileNameWithoutExtension());
        });
}

//==============================================================================
// Liste des modèles IA
//==============================================================================

// Récupère GET /ai/models en arrière-plan puis met à jour le header.
// Invité : rien à faire (le header affiche une capsule figée).
// Échec réseau : on garde l'état actuel, la génération continue d'utiliser "latest".
void MainComponent::refreshModelsAsync()
{
    if (session.isGuest)
        return;

    juce::Thread::launch ([safe = juce::Component::SafePointer<MainComponent> (this),
                           &be = backend]
    {
        const auto result = be.fetchAiModels();

        juce::MessageManager::callAsync ([safe, result]
        {
            if (safe == nullptr || ! result.success)
                return;

            safe->headerComponent->setModels (result.models);
        });
    });
}

// Recharge uniquement l'historique (ne touche pas à la palette).
void MainComponent::refreshHistoryAsync()
{
    if (session.isGuest)
        return;

    backend.syncProfileParamsInBackground (session,
        [safe = juce::Component::SafePointer<MainComponent> (this)] (ProfileSnapshot snap)
        {
            juce::MessageManager::callAsync ([safe, history = std::move (snap.history)]
            {
                if (safe != nullptr)
                    safe->headerComponent->setHistory (history);
            });
        });
}

// Prévient le serveur que l'entrée vient d'être utilisée, puis recharge la liste.
void MainComponent::touchHistoryAsync (int entryId)
{
    if (session.isGuest || entryId <= 0)
        return;

    headerComponent->moveHistoryToTop (entryId);   // mise à jour locale immédiate

    juce::Thread::launch ([safe = juce::Component::SafePointer<MainComponent> (this),
                           &be = backend, entryId]
    {
        if (be.touchHistoryEntry (entryId))
            return;

        juce::MessageManager::callAsync ([safe]
        {
            if (safe == nullptr)
                return;

            HarmoniaAlert::warning (safe.getComponent(),
                                    "History not updated",
                                    "Could not sync with the server.",
                                    HarmoniaAlert::Position::TopCenter);
            safe->refreshHistoryAsync();
        });
    });
}

// Récupère GET /profile/me en arrière-plan (palette serveur + historique),
// puis met à jour le header. Invité : rien à faire.
void MainComponent::refreshProfileAsync()
{
    if (session.isGuest)
        return;

    backend.syncProfileParamsInBackground (session,
        [safe = juce::Component::SafePointer<MainComponent> (this)] (ProfileSnapshot snap)
        {
            // Appelé depuis un thread du pool : on repasse sur le message thread
            juce::MessageManager::callAsync ([safe, snap = std::move (snap)]
            {
                if (safe == nullptr)
                    return;

                // Référence « état serveur » pour le filtre anti-PUT
                safe->backend.resetPaletteSyncState (snap.session.paletteColours,
                                                     snap.session.paletteSlot);

                // <<< C'est ici que ça part au header >>>
                safe->headerComponent->setPaletteFromServer (snap.session.paletteColours,
                                                             snap.session.paletteSlot);
                safe->headerComponent->setAvatarUrl (snap.session.avatarUrl);
                safe->headerComponent->setHistory (snap.history);
            });
        });
}

//==============================================================================
// Verifie invite + prompt vide. Remplit `prompt`, renvoie false si on doit s'arreter.
bool MainComponent::getAiPrompt (juce::String& prompt)
{
    if (session.isGuest)
    {
        HarmoniaAlert::warning (this,
                                Strings::Errors::AiGuestError,
                                Strings::Errors::AiGuestAdvice,
                                HarmoniaAlert::Position::Center);
        return false;
    }

    prompt = headerComponent->getPromptEditor().getText();

    if (prompt.trim().isEmpty())
    {
        HarmoniaAlert::warning (this,
                                Strings::Errors::MissingPrompt,
                                Strings::Errors::MissingPromptAdvice,
                                HarmoniaAlert::Position::TopCenter);
        return false;
    }

    return true;
}

//------------------------------------------------------------------------------
// GENERATE : on repart de zero, aucun parametre protege.
void MainComponent::doGenerateWithAi()
{
    juce::String prompt;
    if (! getAiPrompt (prompt))
        return;

    // "latest" ou l'id du modele choisi dans le menu
    const juce::String backendName = headerComponent->getSelectedBackendName();

    runAiRequest ([this, prompt, backendName]
                  {
                      return backend.generatePreset (prompt, backendName);
                  },
                  {});
}

//------------------------------------------------------------------------------
// REFINE : on envoie le preset actuel + les parametres verrouilles.
void MainComponent::doRefineWithAi()
{
    juce::String prompt;
    if (! getAiPrompt (prompt))
        return;

    const juce::String backendName = headerComponent->getSelectedBackendName();

    const auto locked = getLockedParamIds();

    const juce::String pseudo = session.pseudo.isNotEmpty() ? session.pseudo : "Unknown";
    const juce::String currentJson =
        PresetLoader::saveToJsonString (processor.getAPVTS(), "current", pseudo);

    runAiRequest ([this, prompt, currentJson, locked, backendName]
                  {
                      return backend.refinePreset (prompt, currentJson, locked, backendName);
                  },
                  locked);
}

//------------------------------------------------------------------------------
// Partage entre Generate et Refine :
// thread -> requete -> chargement du preset -> restauration des verrous.
void MainComponent::runAiRequest (std::function<AiResult()> request,
                                  juce::StringArray lockedIds)
{
    // Valeurs des parametres verrouilles AVANT la requete (thread UI)
    std::vector<std::pair<juce::String, float>> keep;

    for (const auto& id : lockedIds)
        if (auto* prm = processor.getAPVTS().getParameter (id))
            keep.push_back ({ id, prm->getValue() });

    headerComponent->setPresetName (Strings::Labels::GeneratingPreset);

    juce::Thread::launch ([safe = juce::Component::SafePointer<MainComponent> (this),
                           request = std::move (request),
                           keep = std::move (keep)]
    {
        const auto result = request();

        juce::MessageManager::callAsync ([safe, result, keep]
        {
            if (safe == nullptr)
                return;

            if (! result.success)
            {
                safe->showAiError (result);
                return;
            }

            juce::StringArray locked;
            for (const auto& [id, value] : keep)
                locked.add (id);

            auto r = PresetLoader::loadFromJsonString (result.json,
                                                    safe->processor.getAPVTS(),
                                                    locked,
                                                    true);

            if (! r.success)
            {
                HarmoniaAlert::error (
                    safe.getComponent(), Strings::Errors::ErrorTitle,
                    (r.errorMessage.isNotEmpty() ? r.errorMessage
                                                 : Strings::Errors::UnreadableAIResponse)
                        + "\n\n" + Strings::Errors::PleaseTryAgainLater,
                    HarmoniaAlert::Position::Center);

                safe->restorePresetLabel();
                return;
            }

            // Filet de securite : meme si l'IA a touche a un parametre verrouille,
            // on remet la valeur d'origine.
            for (const auto& [id, value] : keep)
                if (auto* prm = safe->processor.getAPVTS().getParameter (id))
                    prm->setValueNotifyingHost (value);

            safe->setPresetName (r.presetName);

            // Le serveur a renvoye l'entree creee : pas besoin de GET /profile/me.
            if (result.historyEntry.has_value())
                safe->headerComponent->addHistoryEntry (*result.historyEntry);
            else
                safe->refreshHistoryAsync();   // filet de securite
        });
    });
}

//------------------------------------------------------------------------------
void MainComponent::showAiError (const AiResult& result)
{
    juce::String message;
    juce::String advice = Strings::Errors::PleaseTryAgainLater;

    switch (result.error)
    {
        case AiResult::Error::Network:
            message = Strings::Errors::NetworkError;
            advice  = Strings::Errors::NetworkErrorAdvice;
            break;

        case AiResult::Error::HttpError:
            message = Strings::Errors::AiServerError;
            break;

        case AiResult::Error::SessionExpired:
            message = "Session expired";
            advice  = "Please log in again.";
            break;

        case AiResult::Error::NoSession:
            message = "Not connected";
            advice  = "Please sign in.";
            break;

        case AiResult::Error::EmptyResponse:
            message = Strings::Errors::UnknownError;
            break;

        case AiResult::Error::EmptyPrompt:
            message = Strings::Errors::MissingPrompt;
            advice  = Strings::Errors::MissingPromptAdvice;
            break;

        default:
            message = result.errorMessage.isNotEmpty()
                        ? result.errorMessage
                        : Strings::Errors::UnknownError;
            break;
    }

    HarmoniaAlert::error (this, Strings::Errors::AiError, message + "\n\n" + advice, HarmoniaAlert::Position::Center);

    restorePresetLabel();
}

void MainComponent::paint (juce::Graphics& g)
{
    activatePalette();
    g.fillAll (HarmoniaPalette::background);
}

void MainComponent::resized()
{
    // Disposition : l'oscilloscope pile au centre (colonnes gauche et droite de même largeur).
    //
    //   [ Osc 1 / Mix ] [ Display ] [ Osc 2   ]
    //   [             ] [         ] [ Effects ]
    //   [ LFO         ] [ Filter  ] [         ]
    //   [             ] [         ] [ Output  ]
    //   [ Amp Envelope : Attack Decay | courbe | Sustain Release ]
    //   [ Keyboard : titre + chips | touches                     ]
    constexpr int headerH = 76, keyboardH = 170, ampH = 124;   // clavier : grandes touches (fenêtre 750 px)
    // cadres des panneaux à 16 px du bord, comme le contenu du header (HeaderComponent : reduced (16, ...)) ;
    // les panneaux dessinent leur cadre 6 px à l'intérieur (place de l'ombre)
    constexpr int headerContentMargin = 16;
    constexpr int gap = 10, sideMargin = headerContentMargin - SectionPanel::shadowMargin;
    constexpr float centerShare    = 0.35f;   // colonne de l'écran et de Filter
    constexpr float filterShare    = 0.56f;   // Filter (2 rangées de knobs) : l'écran au-dessus garde le reste
    constexpr float lfoShare       = 0.46f;   // LFO (graphe plus haut) : Osc 1 un peu réduit
    constexpr int   osc2H          = 174;     // Osc 2 : formes d'onde + 2 knobs (en dessous, plus de place pour les knobs)

    auto bounds = getLocalBounds();
    particles.setBounds (bounds);

    headerComponent->setBounds (bounds.removeFromTop (headerH));
    // même marges latérales que les panneaux : cadres alignés au pixel
    synthComponent.setBounds (bounds.removeFromBottom (keyboardH).reduced (sideMargin, 0).withTrimmedBottom (2));

    ampPanel->setBounds (bounds.removeFromBottom (ampH).reduced (sideMargin, 6));

    // marges égales à gauche et à droite + colonnes latérales égales = écran centré au pixel
    auto main = bounds.reduced (sideMargin, 4);

    const int sideW   = (main.getWidth() - 2 * gap - (int) ((float) main.getWidth() * centerShare)) / 2;
    const int centerW = main.getWidth() - 2 * gap - 2 * sideW;

    auto leftCol = main.removeFromLeft (sideW);
    main.removeFromLeft (gap);
    auto centerCol = main.removeFromLeft (centerW);
    main.removeFromLeft (gap);
    auto rightCol = main;

    auto takeBottom = [&] (juce::Rectangle<int>& col, float share)
    {
        auto b = col.removeFromBottom ((int) ((float) col.getHeight() * share));
        col.removeFromBottom (gap);
        return b;
    };

    // centre : l'écran, Filter dessous
    filterPanel->setBounds (takeBottom (centerCol, filterShare));
    screenPanel->setBounds (centerCol);

    // gauche : Osc 1, LFO dessous
    lfoPanel->setBounds    (takeBottom (leftCol, lfoShare));
    oscMixPanel->setBounds (leftCol);

    // droite : Osc 2 (hauteur fixe), le bandeau Output en bas, Effects prend tout l'entre-deux
    osc2Panel->setBounds (rightCol.removeFromTop (osc2H));
    rightCol.removeFromTop (gap);
    outputStrip->setBounds (rightCol.removeFromBottom (OutputStrip::preferredHeight));
    rightCol.removeFromBottom (gap);
    fxPanel->setBounds (rightCol);

    layoutAmp();
    layoutOsc1();
    layoutFilter();
    layoutDisplay();
    layoutLfo();
    layoutOsc2();
    layoutFx();
}

//==============================================================================
// Contenu de chaque panneau
//==============================================================================
namespace
{
    // cellules de même largeur sur une ligne
    void layoutRow (juce::Rectangle<int> area, std::initializer_list<juce::Component*> items, int padX, int padY)
    {
        const int cellW = area.getWidth() / juce::jmax (1, (int) items.size());
        int i = 0;
        for (auto* c : items)
            c->setBounds (juce::Rectangle<int> (area.getX() + cellW * i++, area.getY(), cellW, area.getHeight())
                              .reduced (padX, padY));
    }
}

void MainComponent::layoutAmp()
{
    // Attack, Decay | courbe | Sustain, Release : la courbe au centre, entre ses réglages
    auto inner = ampPanel->getContentBounds().withTrimmedTop (-4);
    const int sideW = (int) ((float) inner.getWidth() * 0.20f);   // la courbe prend 60 % de la largeur

    layoutRow (inner.removeFromLeft (sideW),  { attackKnob.get(), decayKnob.get() },    4, 0);
    layoutRow (inner.removeFromRight (sideW), { sustainKnob.get(), releaseKnob.get() }, 4, 0);
    ampEnvViz.setBounds (inner.reduced (10, 0));
}

void MainComponent::layoutOsc1()
{
    // Formes d'onde en grille 2 x 2 à gauche, Mix et Noise à droite sur la même hauteur ;
    // Unison, Spread et Width récupèrent toute la rangée du bas.
    //
    //   [ ~  /\ ]  Mix   Noise
    //   [ /| [] ]
    //   Unison   Spread   Width
    auto inner = oscMixPanel->getContentBounds();

    auto top = inner.removeFromTop (inner.getHeight() / 2);
    inner.removeFromTop (2);

    // la grille occupe la même hauteur qu'un knob de la ligne : son haut s'aligne sur les
    // libellés MIX / NOISE, le bas de la boîte sur le bas des cadrans (le knob réduit sa zone
    // de 2 px), et la bande du cadenas (14 px) tombe pile sur la ligne des valeurs (14 px)
    static_assert (IconChoiceSelector::defaultLockStripH == 14, "bande du cadenas = hauteur des valeurs des knobs");
    auto waves = top.removeFromLeft ((int) ((float) top.getWidth() * 0.46f));
    osc1WaveSel->setBounds (waves.withTrimmedTop (2).withTrimmedBottom (2));
    top.removeFromLeft (6);
    layoutRow (top, { oscMixKnob.get(), noiseLevelKnob.get() }, 2, 0);

    layoutRow (inner, { unisonVoicesKnob.get(), unisonDetuneKnob.get(), stereoWidthKnob.get() }, 4, 0);
}

void MainComponent::layoutFilter()
{
    auto inner = filterPanel->getContentBounds();
    auto top = inner.removeFromTop (inner.getHeight() / 2);

    const int cellW = top.getWidth() / 3;
    layoutRow (top.removeFromLeft (cellW * 2), { filterCutoffKnob.get(), filterResoKnob.get() }, 0, 2);

    // 3e cellule : 12 / 24 dB au-dessus du type de filtre, même largeur de boîte
    //   [ 12 | 24 ]          <- cadenas dans une colonne à gauche (déborde dans la marge)
    //   [ LP  BP  HP ]
    //   [   cadenas  ]
    constexpr int slopeH = 22, slopeGap = 4;
    constexpr int typeH  = 28 + IconChoiceSelector::defaultLockStripH;

    auto cell = top.reduced (4, 0);
    auto block = cell.withSizeKeepingCentre (cell.getWidth(), juce::jmin (cell.getHeight(), slopeH + slopeGap + typeH));

    const auto slopeRow = block.removeFromTop (slopeH);
    filterSlopeSel->setBounds (slopeRow.withX (slopeRow.getX() - IconChoiceSelector::lockColumnW)
                                       .withWidth (slopeRow.getWidth() + IconChoiceSelector::lockColumnW));
    block.removeFromTop (slopeGap);
    filterTypeSel->setBounds (block);

    layoutRow (inner, { filterEnvAmtKnob.get(), filterEnvDecayKnob.get(), velocityFilterKnob.get() }, 0, 2);
}

void MainComponent::layoutDisplay()
{
    engineBadge.setBounds (screenPanel->getWidth() - 16 - 44, 7, 44, 16);
    displayScreen.setBounds (screenPanel->getContentBounds().reduced (4));
}

void MainComponent::layoutLfo()
{
    auto inner = lfoPanel->getContentBounds();
    lfoViz.setBounds (inner.removeFromTop ((int) ((float) inner.getHeight() * 0.36f)));
    inner.removeFromTop (4);
    layoutRow (inner, { lfoRateKnob.get(), lfoToPitchKnob.get(), lfoToCutoffKnob.get() }, 4, 0);
}

void MainComponent::layoutOsc2()
{
    auto inner = osc2Panel->getContentBounds();
    osc2WaveSel->setBounds (inner.removeFromTop (28 + IconChoiceSelector::defaultLockStripH));
    inner.removeFromTop (2);
    layoutRow (inner, { osc2DetuneKnob.get(), osc2CoarseKnob.get() }, 4, 0);
}

void MainComponent::layoutFx()
{
    // Drive | Delay | Reverb, Size
    layoutRow (fxPanel->getContentBounds(),
               { distortionKnob.get(), delayMixKnob.get(), reverbKnob.get(), reverbSizeKnob.get() }, 2, 2);
}
