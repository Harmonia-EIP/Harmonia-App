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
    processor.setOscilloscope (&oscilloscope);

    addAndMakeVisible (particles);
    particles.toBack();

    buildHeader();

    addAndMakeVisible (oscMixPanel);
    addAndMakeVisible (filterPanel);
    addAndMakeVisible (screenPanel);
    addAndMakeVisible (lfoPanel);
    addAndMakeVisible (osc2Panel);
    addAndMakeVisible (fxPanel);
    addAndMakeVisible (ampPanel);

    screenPanel.addAndMakeVisible (displayScreen);
    lfoPanel.addAndMakeVisible (lfoViz);
    ampPanel.addAndMakeVisible (ampEnvViz);

    buildControls();
    wireButtons();

    addAndMakeVisible (synthComponent);

    setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);
}

MainComponent::~MainComponent()
{
    processor.setOscilloscope (nullptr);
    setLookAndFeel (nullptr);
}

void MainComponent::buildHeader()
{
    titleLabel.setText ("HARMONIA", juce::dontSendNotification);
    titleLabel.setFont (juce::Font (juce::FontOptions (22.0f).withStyle ("Bold"))
                            .withExtraKerningFactor (0.10f));
    titleLabel.setColour (juce::Label::textColourId, HarmoniaPalette::accent);
    addAndMakeVisible (titleLabel);

    subtitleLabel.setText ("AI SUBTRACTIVE SYNTHESIZER  -  CHARTER V1",
                           juce::dontSendNotification);
    subtitleLabel.setFont (juce::Font (juce::FontOptions (9.5f))
                               .withExtraKerningFactor (0.20f));
    subtitleLabel.setColour (juce::Label::textColourId, HarmoniaPalette::textMuted);
    addAndMakeVisible (subtitleLabel);

    promptEditor.setMultiLine (false);
    promptEditor.setReturnKeyStartsNewLine (false);
    promptEditor.setTextToShowWhenEmpty ("Describe a sound and press GENERATE...",
                                         HarmoniaPalette::textMuted);
    promptEditor.setFont (juce::Font (juce::FontOptions (12.5f)));
    promptEditor.setIndents (10, 6);
    addAndMakeVisible (promptEditor);

    generateButton.getProperties().set ("accent", true);
    addAndMakeVisible (generateButton);

    presetLabel.setText ("INIT", juce::dontSendNotification);
    presetLabel.setFont (juce::Font (juce::FontOptions (10.5f).withStyle ("Bold"))
                             .withExtraKerningFactor (0.18f));
    presetLabel.setColour (juce::Label::textColourId, HarmoniaPalette::accent);
    presetLabel.setJustificationType (juce::Justification::centredLeft);
    addAndMakeVisible (presetLabel);

    addAndMakeVisible (loadButton);
    addAndMakeVisible (saveButton);
    addAndMakeVisible (logoutButton);
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
    oscMixPanel.addAndMakeVisible (*osc1WaveSel);
    oscMixPanel.addAndMakeVisible (*oscMixKnob);
    oscMixPanel.addAndMakeVisible (*noiseLevelKnob);

    filterCutoffKnob   = std::make_unique<KnobControl>      (a, HarmoniaParams::IDs::filterCutoff,    "Cutoff");
    filterResoKnob     = std::make_unique<KnobControl>      (a, HarmoniaParams::IDs::filterResonance, "Reso");
    filterTypeSel      = std::make_unique<FilterTypeSelector>(a, HarmoniaParams::IDs::filterType);
    filterEnvAmtKnob   = std::make_unique<KnobControl>      (a, HarmoniaParams::IDs::filterEnvAmount, "F.Env Amt", true);
    filterEnvDecayKnob = std::make_unique<KnobControl>      (a, HarmoniaParams::IDs::filterEnvDecay,  "F.Env Dec");
    velocityFilterKnob = std::make_unique<KnobControl>      (a, HarmoniaParams::IDs::velocityToFilter,"Vel >Flt");
    filterPanel.addAndMakeVisible (*filterCutoffKnob);
    filterPanel.addAndMakeVisible (*filterResoKnob);
    filterPanel.addAndMakeVisible (*filterTypeSel);
    filterPanel.addAndMakeVisible (*filterEnvAmtKnob);
    filterPanel.addAndMakeVisible (*filterEnvDecayKnob);
    filterPanel.addAndMakeVisible (*velocityFilterKnob);

    lfoRateKnob     = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::lfoRate,     "Rate");
    lfoToPitchKnob  = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::lfoToPitch,  "to Pitch");
    lfoToCutoffKnob = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::lfoToCutoff, "to Cutoff");
    lfoPanel.addAndMakeVisible (*lfoRateKnob);
    lfoPanel.addAndMakeVisible (*lfoToPitchKnob);
    lfoPanel.addAndMakeVisible (*lfoToCutoffKnob);

    osc2WaveSel    = std::make_unique<WaveformSelector>(a, HarmoniaParams::IDs::osc2Waveform);
    osc2DetuneKnob = std::make_unique<KnobControl>     (a, HarmoniaParams::IDs::osc2Detune, "Detune");
    osc2Panel.addAndMakeVisible (*osc2WaveSel);
    osc2Panel.addAndMakeVisible (*osc2DetuneKnob);

    distortionKnob = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::distortionMix, "Drive");
    reverbKnob     = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::reverbMix,     "Reverb");
    fxPanel.addAndMakeVisible (*distortionKnob);
    fxPanel.addAndMakeVisible (*reverbKnob);

    attackKnob  = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::ampAttack,  "Attack");
    decayKnob   = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::ampDecay,   "Decay");
    sustainKnob = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::ampSustain, "Sustain");
    releaseKnob = std::make_unique<KnobControl>(a, HarmoniaParams::IDs::ampRelease, "Release");
    ampPanel.addAndMakeVisible (*attackKnob);
    ampPanel.addAndMakeVisible (*decayKnob);
    ampPanel.addAndMakeVisible (*sustainKnob);
    ampPanel.addAndMakeVisible (*releaseKnob);

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

void MainComponent::wireButtons()
{
    loadButton.onClick     = [this] { doLoadPreset(); };
    saveButton.onClick     = [this] { doSavePreset(); };
    generateButton.onClick = [this] { doGenerateWithAi(); };

    const bool signedIn = session.accessToken.isNotEmpty();
    logoutButton.setButtonText (signedIn ? "Logout" : "Sign in");
    logoutButton.onClick = [this]
    {
        backend.clearSession();
        if (onLogout) onLogout();
    };

    promptEditor.setEnabled (signedIn);
    generateButton.setEnabled (signedIn);
    if (! signedIn)
        promptEditor.setTextToShowWhenEmpty ("Sign in to generate sounds with AI...",
                                             HarmoniaPalette::textMuted);
}

void MainComponent::doLoadPreset()
{
    chooser = std::make_unique<juce::FileChooser>(
        "Load Harmonia preset (hel.json)",
        juce::File::getSpecialLocation (juce::File::userDocumentsDirectory),
        "*.json");

    juce::Component::SafePointer<MainComponent> safeThis (this);
    chooser->launchAsync (
        juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
        [safeThis] (const juce::FileChooser& fc)
        {
            auto* self = safeThis.getComponent();
            if (self == nullptr) return;

            auto file = fc.getResult();
            if (! file.existsAsFile()) return;
            auto result = PresetLoader::loadFromFile (file, self->processor.getAPVTS());
            self->presetLabel.setText (result.success ? result.presetName.toUpperCase()
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

    juce::Component::SafePointer<MainComponent> safeThis (this);
    chooser->launchAsync (
        juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles,
        [safeThis] (const juce::FileChooser& fc)
        {
            auto* self = safeThis.getComponent();
            if (self == nullptr) return;

            auto file = fc.getResult();
            if (file == juce::File()) return;
            if (file.getFileExtension().isEmpty())
                file = file.withFileExtension (".json");
            const auto json = PresetLoader::saveToJsonString (self->processor.getAPVTS(), "User Preset");
            file.replaceWithText (json);
            self->presetLabel.setText (file.getFileNameWithoutExtension().toUpperCase(),
                                       juce::dontSendNotification);
        });
}

void MainComponent::doGenerateWithAi()
{
    const auto prompt = promptEditor.getText();
    if (prompt.trim().isEmpty()) return;

    presetLabel.setText ("GENERATING...", juce::dontSendNotification);

    juce::Component::SafePointer<MainComponent> safeThis (this);
    BackendManager* backendPtr = &backend;
    juce::Thread::launch ([safeThis, backendPtr, prompt]
    {
        juce::String err;
        const auto json = backendPtr->generatePresetJson (prompt, err);

        juce::MessageManager::callAsync ([safeThis, json, err]
        {
            auto* s = safeThis.getComponent();
            if (s == nullptr) return;

            if (json.isEmpty())
            {
                s->presetLabel.setText ("AI ERR: " + err, juce::dontSendNotification);
                return;
            }
            auto r = PresetLoader::loadFromJsonString (json, s->processor.getAPVTS());
            s->presetLabel.setText (r.success ? r.presetName.toUpperCase()
                                              : ("PARSE ERR: " + r.errorMessage),
                                    juce::dontSendNotification);
        });
    });
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (HarmoniaPalette::background);

    auto header = getLocalBounds().removeFromTop (76);

    juce::ColourGradient hbg (HarmoniaPalette::panelTop, 0, 0,
                              HarmoniaPalette::panel,    0, (float) header.getHeight(), false);
    g.setGradientFill (hbg);
    g.fillRect (header);

    g.setColour (juce::Colours::white.withAlpha (0.04f));
    g.fillRect (header.getX(), header.getY() + 1, header.getWidth(), 1);

    g.setColour (HarmoniaPalette::border);
    g.fillRect (header.getX(), header.getBottom() - 1, header.getWidth(), 1);

    g.setColour (HarmoniaPalette::accent.withAlpha (0.40f));
    g.fillRect (header.getX(), header.getBottom(), header.getWidth(), 1);
}

void MainComponent::resized()
{
    auto bounds = getLocalBounds();
    particles.setBounds (bounds);

    auto header = bounds.removeFromTop (76).reduced (16, 12);
    auto leftHeader = header.removeFromLeft (260);
    titleLabel.setBounds    (leftHeader.removeFromTop (28));
    subtitleLabel.setBounds (leftHeader.removeFromTop (16));

    logoutButton.setBounds (header.removeFromRight (78).reduced (0, 12));
    header.removeFromRight (8);
    saveButton.setBounds   (header.removeFromRight (66).reduced (0, 12));
    header.removeFromRight (4);
    loadButton.setBounds   (header.removeFromRight (66).reduced (0, 12));
    header.removeFromRight (16);

    auto promptRow = header.removeFromTop (32);
    generateButton.setBounds (promptRow.removeFromRight (104));
    promptRow.removeFromRight (8);
    promptEditor.setBounds (promptRow);
    presetLabel.setBounds (header.removeFromTop (16));

    auto kbArea = bounds.removeFromBottom (90);
    synthComponent.setBounds (kbArea);

    auto ampArea = bounds.removeFromBottom (170).reduced (16, 8);
    ampPanel.setBounds (ampArea);
    {
        auto inner = ampPanel.getContentBounds();
        const int vizH = (int) (inner.getHeight() * 0.42f);
        ampEnvViz.setBounds (inner.removeFromTop (vizH));
        inner.removeFromTop (6);

        const int n = 4;
        const int cellW = inner.getWidth() / n;
        attackKnob ->setBounds (juce::Rectangle<int> (inner.getX() + 0 * cellW, inner.getY(), cellW, inner.getHeight()).reduced (4));
        decayKnob  ->setBounds (juce::Rectangle<int> (inner.getX() + 1 * cellW, inner.getY(), cellW, inner.getHeight()).reduced (4));
        sustainKnob->setBounds (juce::Rectangle<int> (inner.getX() + 2 * cellW, inner.getY(), cellW, inner.getHeight()).reduced (4));
        releaseKnob->setBounds (juce::Rectangle<int> (inner.getX() + 3 * cellW, inner.getY(), cellW, inner.getHeight()).reduced (4));
    }

    auto mainRow = bounds.reduced (16, 12);
    const int gap = 10;

    const int leftW   = (int) (mainRow.getWidth() * 0.33f);
    const int rightW  = (int) (mainRow.getWidth() * 0.29f);
    const int centerW = mainRow.getWidth() - leftW - rightW - 2 * gap;

    auto leftCol   = mainRow.removeFromLeft (leftW);
    mainRow.removeFromLeft (gap);
    auto centerCol = mainRow.removeFromLeft (centerW);
    mainRow.removeFromLeft (gap);
    auto rightCol  = mainRow;

    {
        const int osc1H = (int) (leftCol.getHeight() * 0.36f);
        auto osc1Box   = leftCol.removeFromTop (osc1H);
        leftCol.removeFromTop (gap);
        auto filterBox = leftCol;

        oscMixPanel.setBounds (osc1Box);
        filterPanel.setBounds (filterBox);

        {
            auto inner = oscMixPanel.getContentBounds();
            const int selH = juce::jmin (28, inner.getHeight() / 2);
            auto selRow   = inner.removeFromTop (selH);
            inner.removeFromTop (4);
            osc1WaveSel->setBounds (selRow);

            const int cellW = inner.getWidth() / 2;
            oscMixKnob    ->setBounds (juce::Rectangle<int> (inner.getX(),         inner.getY(), cellW, inner.getHeight()).reduced (4));
            noiseLevelKnob->setBounds (juce::Rectangle<int> (inner.getX() + cellW, inner.getY(), cellW, inner.getHeight()).reduced (4));
        }

        {
            auto inner = filterPanel.getContentBounds();
            const int rowH = inner.getHeight() / 2;
            const int cellW = inner.getWidth() / 3;

            filterCutoffKnob ->setBounds (juce::Rectangle<int> (inner.getX(),             inner.getY(), cellW, rowH).reduced (4));
            filterResoKnob   ->setBounds (juce::Rectangle<int> (inner.getX() + cellW,     inner.getY(), cellW, rowH).reduced (4));
            auto typeCell = juce::Rectangle<int> (inner.getX() + 2 * cellW, inner.getY(), cellW, rowH).reduced (4);
            const int selH = 28;
            typeCell = typeCell.withSizeKeepingCentre (typeCell.getWidth(), selH);
            filterTypeSel->setBounds (typeCell);

            filterEnvAmtKnob  ->setBounds (juce::Rectangle<int> (inner.getX(),             inner.getY() + rowH, cellW, rowH).reduced (4));
            filterEnvDecayKnob->setBounds (juce::Rectangle<int> (inner.getX() + cellW,     inner.getY() + rowH, cellW, rowH).reduced (4));
            velocityFilterKnob->setBounds (juce::Rectangle<int> (inner.getX() + 2 * cellW, inner.getY() + rowH, cellW, rowH).reduced (4));
        }
    }

    {
        const int screenH = (int) (centerCol.getHeight() * 0.55f);
        auto screenBox = centerCol.removeFromTop (screenH);
        centerCol.removeFromTop (gap);
        auto lfoBox    = centerCol;

        screenPanel.setBounds (screenBox);
        lfoPanel.setBounds (lfoBox);

        displayScreen.setBounds (screenPanel.getContentBounds().reduced (4));

        auto inner = lfoPanel.getContentBounds();
        const int vizH = (int) (inner.getHeight() * 0.40f);
        lfoViz.setBounds (inner.removeFromTop (vizH));
        inner.removeFromTop (6);

        const int cellW = inner.getWidth() / 3;
        lfoRateKnob    ->setBounds (juce::Rectangle<int> (inner.getX(),             inner.getY(), cellW, inner.getHeight()).reduced (4));
        lfoToPitchKnob ->setBounds (juce::Rectangle<int> (inner.getX() + cellW,     inner.getY(), cellW, inner.getHeight()).reduced (4));
        lfoToCutoffKnob->setBounds (juce::Rectangle<int> (inner.getX() + 2 * cellW, inner.getY(), cellW, inner.getHeight()).reduced (4));
    }

    {
        const int osc2H = (int) (rightCol.getHeight() * 0.50f);
        auto osc2Box = rightCol.removeFromTop (osc2H);
        rightCol.removeFromTop (gap);
        auto fxBox   = rightCol;

        osc2Panel.setBounds (osc2Box);
        fxPanel.setBounds (fxBox);

        {
            auto inner = osc2Panel.getContentBounds();
            const int selH = 28;
            osc2WaveSel ->setBounds (inner.removeFromTop (selH));
            inner.removeFromTop (4);
            osc2DetuneKnob->setBounds (inner.reduced (4));
        }

        {
            auto inner = fxPanel.getContentBounds();
            const int cellW = inner.getWidth() / 2;
            distortionKnob->setBounds (juce::Rectangle<int> (inner.getX(),         inner.getY(), cellW, inner.getHeight()).reduced (4));
            reverbKnob    ->setBounds (juce::Rectangle<int> (inner.getX() + cellW, inner.getY(), cellW, inner.getHeight()).reduced (4));
        }
    }
}
