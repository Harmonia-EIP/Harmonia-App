#include "MainComponent.h"

MainComponent::MainComponent (HarmoniaAudioProcessor& p,
                              BackendManager& be,
                              const UserSession& s)
    : processor (p),
      backend (be),
      session (s),
      oscilloscope (AppConfig::Oscilloscope::BufferSize,
                    AppConfig::Oscilloscope::RefreshRate),
      currentTheme (static_cast<HarmoniaPalette::Theme>(s.themeId)),
      displayScreen (oscilloscope),
      synthComponent (p.getKeyboardState()),
      ampEnvViz (p.getAPVTS(), HarmoniaPalette::sectionAmpEnv),
      lfoViz   (p.getAPVTS(), HarmoniaPalette::sectionLfo)
{
    setLookAndFeel (&lookAndFeel);

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

    headerComponent = std::make_unique<HeaderComponent> (session);
    addAndMakeVisible (*headerComponent);

    headerComponent->onThemeChanged =
        [this](HarmoniaPalette::Theme theme)
    {
        currentTheme = theme;

        applyTheme();

        lookAndFeel.refreshTheme();
        headerComponent->refreshTheme();

        sendLookAndFeelChange();

        repaint();


        backend.updateLocalTheme(
            static_cast<int>(theme));

        if (!session.isGuest)
            backend.updateThemeAsync(
                static_cast<int>(theme));
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

    addAndMakeVisible (synthComponent);

    setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);
}

MainComponent::~MainComponent()
{
    processor.setOscilloscope (nullptr);
    setLookAndFeel(nullptr);
    
    HarmoniaPalette::setTheme(
        HarmoniaPalette::Theme::Dark);
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
    oscMixKnob     = std::make_unique<KnobControl>     (a, HarmoniaParams::IDs::oscMix,     "Mix");
    noiseLevelKnob = std::make_unique<KnobControl>     (a, HarmoniaParams::IDs::noiseLevel, "Noise");
    oscMixPanel->addAndMakeVisible (*osc1WaveSel);
    oscMixPanel->addAndMakeVisible (*oscMixKnob);
    oscMixPanel->addAndMakeVisible (*noiseLevelKnob);

    filterCutoffKnob   = std::make_unique<KnobControl>      (a, HarmoniaParams::IDs::filterCutoff,    "Cutoff");
    filterResoKnob     = std::make_unique<KnobControl>      (a, HarmoniaParams::IDs::filterResonance, "Reso");
    filterTypeSel      = std::make_unique<FilterTypeSelector>(a, HarmoniaParams::IDs::filterType);
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
    }
}

void MainComponent::wireHeaderButtons()
{
    headerComponent->getLoadButton().onClick =
        [this] { doLoadPreset(); };

    headerComponent->getSaveButton().onClick =
        [this] { doSavePreset(); };

    headerComponent->getGenerateButton().onClick =
        [this] { doGenerateWithAi(); };

    headerComponent->getLogoutButton().onClick =
        [this]
        {
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
            auto result = PresetLoader::loadFromFile (file, processor.getAPVTS());
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

void MainComponent::doGenerateWithAi()
{
    const auto prompt = headerComponent->getPromptEditor().getText();

    if (prompt.trim().isEmpty())
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            Strings::Errors::MissingPrompt,
            Strings::Errors::MissingPromptAdvice
        );
        return;
    }

    headerComponent->getPresetLabel().setText (Strings::Labels::GeneratingPreset, juce::dontSendNotification);

    juce::Thread::launch ([this, prompt]
    {
        auto result = backend.generatePreset(prompt);

        juce::MessageManager::callAsync ([this, result]
        {
            if (!result.success)
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    Strings::Errors::AiError,
                    result.errorMessage.isNotEmpty()
                        ? result.errorMessage
                        : Strings::Errors::UnknownError
                );

                headerComponent->getPresetLabel().setText(
                    Strings::Labels::UnsetPreset.toUpperCase(),
                    juce::dontSendNotification
                );
                return;
            }

            auto r = PresetLoader::loadFromJsonString(
                result.json,
                processor.getAPVTS()
            );

            if (!r.success)
            {
                juce::AlertWindow::showMessageBoxAsync(
                    juce::AlertWindow::WarningIcon,
                    Strings::Errors::ErrorTitle,
                    r.errorMessage.isNotEmpty()
                        ? r.errorMessage
                        : Strings::Errors::UnreadableAIResponse
                );

                headerComponent->getPresetLabel().setText(
                    Strings::Labels::UnsetPreset.toUpperCase(),
                    juce::dontSendNotification
                );
                return;
            }

            headerComponent->getPresetLabel().setText(
                r.presetName.toUpperCase(),
                juce::dontSendNotification
            );
        });
    });
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
        const int osc1H = (int) (leftCol.getHeight() * 0.40f);

        auto osc1Box = leftCol.removeFromTop (osc1H);

        leftCol.removeFromTop (gap);

        auto filterBox = leftCol;

        oscMixPanel->setBounds (osc1Box);
        filterPanel->setBounds (filterBox);

        {
            auto inner = oscMixPanel->getContentBounds();

            const int selH = juce::jmin (28, inner.getHeight() / 2);

            auto selRow = inner.removeFromTop (selH);

            inner.removeFromTop (4);

            osc1WaveSel->setBounds (selRow);

            const int cellW = inner.getWidth() / 2;

            oscMixKnob->setBounds (
                juce::Rectangle<int> (
                    inner.getX(),
                    inner.getY(),
                    cellW,
                    inner.getHeight()).reduced (4));

            noiseLevelKnob->setBounds (
                juce::Rectangle<int> (
                    inner.getX() + cellW,
                    inner.getY(),
                    cellW,
                    inner.getHeight()).reduced (4));
        }

        {
            auto inner = filterPanel->getContentBounds();

            const int rowH = inner.getHeight() / 2;
            const int cellW = inner.getWidth() / 3;

            filterCutoffKnob->setBounds (
                juce::Rectangle<int> (
                    inner.getX(),
                    inner.getY(),
                    cellW,
                    rowH).reduced (4));

            filterResoKnob->setBounds (
                juce::Rectangle<int> (
                    inner.getX() + cellW,
                    inner.getY(),
                    cellW,
                    rowH).reduced (4));

            auto typeCell = juce::Rectangle<int> (
                inner.getX() + 2 * cellW,
                inner.getY(),
                cellW,
                rowH).reduced (4);

            const int selH = 28;

            typeCell = typeCell.withSizeKeepingCentre (
                typeCell.getWidth(),
                selH);

            filterTypeSel->setBounds (typeCell);

            filterEnvAmtKnob->setBounds (
                juce::Rectangle<int> (
                    inner.getX(),
                    inner.getY() + rowH,
                    cellW,
                    rowH).reduced (4));

            filterEnvDecayKnob->setBounds (
                juce::Rectangle<int> (
                    inner.getX() + cellW,
                    inner.getY() + rowH,
                    cellW,
                    rowH).reduced (4));

            velocityFilterKnob->setBounds (
                juce::Rectangle<int> (
                    inner.getX() + 2 * cellW,
                    inner.getY() + rowH,
                    cellW,
                    rowH).reduced (4));
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
                inner.removeFromTop (selH));

            inner.removeFromTop (4);

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