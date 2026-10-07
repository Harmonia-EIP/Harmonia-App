#include "MainComponent.h"

MainComponent::MainComponent (HarmoniaAudioProcessor& p,
                              BackendManager& be,
                              const UserSession& s)
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
    headerComponent = std::make_unique<HeaderComponent> (session);
    addAndMakeVisible (*headerComponent);

    // Application visuelle uniquement : la synchro backend passe par
    // onPaletteChanged.
    headerComponent->onThemeChanged =
        [this] (HarmoniaPalette::Theme theme)
    {
        currentTheme = theme;
        applyTheme();
    };

    // Palette complete + pastille active (slot 0..2). Le header ne l'emet pas
    // pour un invité, et le backend ignore de toute façon les invités.
    headerComponent->onPaletteChanged =
        [this] (const juce::StringArray& colours, int slot)
    {
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

    addAndMakeVisible (synthComponent);

    setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);
}

MainComponent::~MainComponent()
{
    // Envoie la palette en attente (debounce non expiré) sans bloquer :
    // le backend copie tout par valeur et lance un PUT court détaché.
    backend.flushPaletteIfPending();

    processor.setOscilloscope (nullptr);
    setLookAndFeel(nullptr);

    HarmoniaPalette::setTheme(
        HarmoniaPalette::Theme::Dark);
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
    HarmoniaPalette::setTheme (currentTheme);

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
            particles.emitBurst (centreInParticles, 4, HarmoniaPalette::accent);
        };
    }
}

void MainComponent::buildControls()
{
    auto& a = processor.getAPVTS();

    osc1WaveSel    = std::make_unique<WaveformSelector>(a, HarmoniaParams::IDs::osc1Waveform);
    osc1WaveSel->setLockStripHeight (IconChoiceSelector::defaultLockStripH);
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

    attackKnob  = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::ampAttack,  "Attack");
    decayKnob   = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::ampDecay,   "Decay");
    sustainKnob = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::ampSustain, "Sustain");
    releaseKnob = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::ampRelease, "Release");
    ampPanel->addAndMakeVisible (*attackKnob);
    ampPanel->addAndMakeVisible (*decayKnob);
    ampPanel->addAndMakeVisible (*sustainKnob);
    ampPanel->addAndMakeVisible (*releaseKnob);

    for (auto* c : {
            (juce::Component*) osc1WaveSel.get(),        // <-- ajoute
            (juce::Component*) osc2WaveSel.get(),        // <-- ajoute
            (juce::Component*) filterTypeSel.get(), 
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

    headerComponent->getLogoutButton().onClick =
        [this]
        {
            // AVANT clearSession() : sinon le token est effacé et le flush
            // ne peut plus s'authentifier.
            backend.flushPaletteIfPending();

            backend.clearSession();

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
        [this] (const juce::FileChooser& fc)
        {
            auto file = fc.getResult();
            if (! file.existsAsFile()) return;

            // Les cadenas ne comptent qu'en mode Refine (en Generate ils sont masques)
            const auto locked = headerComponent->getMode() == HeaderComponent::Mode::Refine
                                    ? getLockedParamIds()
                                    : juce::StringArray();

            auto result = PresetLoader::loadFromFile (file, processor.getAPVTS(), locked);
            headerComponent->getPresetLabel().setText (result.success ? result.presetName.toUpperCase()
                                                : ("ERR: " + result.errorMessage),
                                 juce::dontSendNotification);
        });
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
            headerComponent->getPresetLabel().setText (file.getFileNameWithoutExtension().toUpperCase(),
                                 juce::dontSendNotification);
        });
}

// =============================================================================
// A remplacer dans MainComponent.cpp : l'ancienne doGenerateWithAi() en entier.
// =============================================================================
 
//------------------------------------------------------------------------------
// Verifie invite + prompt vide. Remplit `prompt`, renvoie false si on doit s'arreter.
bool MainComponent::getAiPrompt (juce::String& prompt)
{
    if (session.isGuest)
    {
        juce::AlertWindow::showMessageBoxAsync (
            juce::AlertWindow::WarningIcon,
            Strings::Errors::AiGuestError,
            Strings::Errors::AiGuestAdvice);
        return false;
    }
 
    prompt = headerComponent->getPromptEditor().getText();
 
    if (prompt.trim().isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync (
            juce::AlertWindow::WarningIcon,
            Strings::Errors::MissingPrompt,
            Strings::Errors::MissingPromptAdvice);
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
 
    const int          modelId     = headerComponent->getSelectedModelId();
    const juce::String backendName = headerComponent->getSelectedBackendName();
 
    runAiRequest ([this, prompt, modelId, backendName]
                  {
                      return backend.generatePreset (prompt, modelId, backendName);
                  },
                  {});
}
 
//------------------------------------------------------------------------------
// REFINE : on envoie le preset actuel + les parametres verrouilles.
// -> backend.refinePreset(...) est a creer dans BackendManager (voir plus bas).
void MainComponent::doRefineWithAi()
{
    juce::String prompt;
    if (! getAiPrompt (prompt))
        return;
 
    const int          modelId     = headerComponent->getSelectedModelId();
    const juce::String backendName = headerComponent->getSelectedBackendName();
 
    const auto locked = getLockedParamIds();
 
    const juce::String pseudo = session.pseudo.isNotEmpty() ? session.pseudo : "Unknown";
    const juce::String currentJson =
        PresetLoader::saveToJsonString (processor.getAPVTS(), "current", pseudo);
 
    runAiRequest ([this, prompt, currentJson, locked, modelId, backendName]
                  {
                      return backend.refinePreset (prompt, currentJson, locked,
                                                   modelId, backendName);
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
 
    headerComponent->getPresetLabel().setText (Strings::Labels::GeneratingPreset,
                                               juce::dontSendNotification);
 
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
 
            auto r = PresetLoader::loadFromJsonString (result.json,
                                                       safe->processor.getAPVTS());
 
            if (! r.success)
            {
                HarmoniaAlert::error (
                    Strings::Errors::ErrorTitle,
                    (r.errorMessage.isNotEmpty() ? r.errorMessage
                                                 : Strings::Errors::UnreadableAIResponse)
                        + "\n\n" + Strings::Errors::PleaseTryAgainLater);
 
                safe->headerComponent->getPresetLabel().setText (
                    Strings::Labels::UnsetPreset.toUpperCase(),
                    juce::dontSendNotification);
                return;
            }
 
            // Filet de securite : meme si l'IA a touche a un parametre verrouille,
            // on remet la valeur d'origine.
            for (const auto& [id, value] : keep)
                if (auto* prm = safe->processor.getAPVTS().getParameter (id))
                    prm->setValueNotifyingHost (value);
 
            safe->headerComponent->getPresetLabel().setText (r.presetName.toUpperCase(),
                                                             juce::dontSendNotification);
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
 
    HarmoniaAlert::error (Strings::Errors::AiError, message + "\n\n" + advice);
 
    headerComponent->getPresetLabel().setText (
        Strings::Labels::UnsetPreset.toUpperCase(),
        juce::dontSendNotification);
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (HarmoniaPalette::background);
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();

    particles.setBounds (bounds);

    auto headerArea = bounds.removeFromTop (76);
    headerComponent->setBounds (headerArea);

    auto kbArea = bounds.removeFromBottom (100);
    synthComponent.setBounds (kbArea);

    auto ampArea = bounds.removeFromBottom (180).reduced (16, 8);
    ampPanel->setBounds (ampArea);

    {
        auto inner = ampPanel->getContentBounds();

        const int vizH = (int) (inner.getHeight() * 0.40f);

        ampEnvViz.setBounds (inner.removeFromTop (vizH));

        inner.removeFromTop (6);

        const int n = 4;
        const int cellW = inner.getWidth() / n;

        attackKnob->setBounds (
            juce::Rectangle<int> (
                inner.getX() + 0 * cellW,
                inner.getY(),
                cellW,
                inner.getHeight()).reduced (4));

        decayKnob->setBounds (
            juce::Rectangle<int> (
                inner.getX() + 1 * cellW,
                inner.getY(),
                cellW,
                inner.getHeight()).reduced (4));

        sustainKnob->setBounds (
            juce::Rectangle<int> (
                inner.getX() + 2 * cellW,
                inner.getY(),
                cellW,
                inner.getHeight()).reduced (4));

        releaseKnob->setBounds (
            juce::Rectangle<int> (
                inner.getX() + 3 * cellW,
                inner.getY(),
                cellW,
                inner.getHeight()).reduced (4));
    }

    auto mainRow = bounds.reduced (16, 6);

    const int gap = 10;

    const int leftW   = (int) (mainRow.getWidth() * 0.33f);
    const int rightW  = (int) (mainRow.getWidth() * 0.29f);
    const int centerW = mainRow.getWidth() - leftW - rightW - 2 * gap;

    auto leftCol = mainRow.removeFromLeft (leftW);

    mainRow.removeFromLeft (gap);

    auto centerCol = mainRow.removeFromLeft (centerW);

    mainRow.removeFromLeft (gap);

    auto rightCol = mainRow;

    // =========================================================
    // LEFT COLUMN
    // =========================================================

    {
        const int osc1H = (int) (leftCol.getHeight() * 0.48f);

        auto osc1Box = leftCol.removeFromTop (osc1H);

        leftCol.removeFromTop (gap);

        auto filterBox = leftCol;

        oscMixPanel->setBounds (osc1Box);
        filterPanel->setBounds (filterBox);

        {
           auto inner = oscMixPanel->getContentBounds();

            // boite d'icones (28) + bande du cadenas : taille fixe
            const int selRowH = 28 + IconChoiceSelector::defaultLockStripH;

            osc1WaveSel->setBounds (inner.removeFromTop (selRowH));

            inner.removeFromTop (1);

            const int cellW = inner.getWidth() / 2;

            oscMixKnob->setBounds (
                juce::Rectangle<int> (
                    inner.getX(),
                    inner.getY(),
                    cellW,
                    inner.getHeight()).reduced (4, 0));

            noiseLevelKnob->setBounds (
                juce::Rectangle<int> (
                    inner.getX() + cellW,
                    inner.getY(),
                    cellW,
                    inner.getHeight()).reduced (4, 0));
        }

        {
            auto inner = filterPanel->getContentBounds();

            const int rowH = inner.getHeight() / 2;
            const int cellW = inner.getWidth() / 3;

            filterCutoffKnob->setBounds (
                juce::Rectangle<int> (inner.getX(), inner.getY(), cellW, rowH).reduced (0, 2));

            filterResoKnob->setBounds (
                juce::Rectangle<int> (inner.getX() + cellW, inner.getY(), cellW, rowH).reduced (0, 2));


            auto typeCell = juce::Rectangle<int> (
                inner.getX() + 2 * cellW,
                inner.getY(),
                cellW,
                rowH).reduced (4);

            // boite d'icones (28) + bande du cadenas (14)
            const int selH = juce::jmin (28 + IconChoiceSelector::defaultLockStripH,
                                        typeCell.getHeight());

            typeCell = typeCell.withSizeKeepingCentre (
                typeCell.getWidth(),
                selH);

            filterTypeSel->setBounds (typeCell);

            filterEnvAmtKnob->setBounds (
                juce::Rectangle<int> (inner.getX(), inner.getY() + rowH, cellW, rowH).reduced (0, 2));

            filterEnvDecayKnob->setBounds (
                juce::Rectangle<int> (inner.getX() + cellW, inner.getY() + rowH, cellW, rowH).reduced (0, 2));

            velocityFilterKnob->setBounds (
                juce::Rectangle<int> (inner.getX() + 2 * cellW, inner.getY() + rowH, cellW, rowH).reduced (0, 2));
        }
    }

    // =========================================================
    // CENTER COLUMN
    // =========================================================

    {
        const int screenH = (int) (centerCol.getHeight() * 0.50f);

        auto screenBox = centerCol.removeFromTop (screenH);

        centerCol.removeFromTop (gap);

        auto lfoBox = centerCol;

        screenPanel->setBounds (screenBox);
        lfoPanel->setBounds (lfoBox);

        displayScreen.setBounds (
            screenPanel->getContentBounds().reduced (4));

        auto inner = lfoPanel->getContentBounds();

        const int vizH = (int) (inner.getHeight() * 0.40f);

        lfoViz.setBounds (inner.removeFromTop (vizH));

        inner.removeFromTop (6);

        const int cellW = inner.getWidth() / 3;

        lfoRateKnob->setBounds (
            juce::Rectangle<int> (
                inner.getX(),
                inner.getY(),
                cellW,
                inner.getHeight()).reduced (4));

        lfoToPitchKnob->setBounds (
            juce::Rectangle<int> (
                inner.getX() + cellW,
                inner.getY(),
                cellW,
                inner.getHeight()).reduced (4));

        lfoToCutoffKnob->setBounds (
            juce::Rectangle<int> (
                inner.getX() + 2 * cellW,
                inner.getY(),
                cellW,
                inner.getHeight()).reduced (4));
    }

    // =========================================================
    // RIGHT COLUMN
    // =========================================================

    {
        const int osc2H = (int) (rightCol.getHeight() * 0.55f);

        auto osc2Box = rightCol.removeFromTop (osc2H);

        rightCol.removeFromTop (gap);

        auto fxBox = rightCol;

        osc2Panel->setBounds (osc2Box);
        fxPanel->setBounds (fxBox);

        {
            auto inner = osc2Panel->getContentBounds();

            const int selH = 28;

            osc2WaveSel->setBounds (
                inner.removeFromTop (selH + IconChoiceSelector::defaultLockStripH));

            inner.removeFromTop (2);

            osc2DetuneKnob->setBounds (
                inner.reduced (4));
        }

        {
            auto inner = fxPanel->getContentBounds();

            const int cellW = inner.getWidth() / 2;

            distortionKnob->setBounds (
                juce::Rectangle<int> (
                    inner.getX(),
                    inner.getY(),
                    cellW,
                    inner.getHeight()).reduced (4));

            reverbKnob->setBounds (
                juce::Rectangle<int> (
                    inner.getX() + cellW,
                    inner.getY(),
                    cellW,
                    inner.getHeight()).reduced (4));
        }
    }
}