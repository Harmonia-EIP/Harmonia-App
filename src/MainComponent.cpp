// MainComponent.cpp (fix : callback layout + resized() avec switch + marges réduites)

#include "MainComponent.h"
#include "themes/AppColourIds.h"

#include <filesystem>
#include <fstream>
#include <thread>

//==============================================================================

MainComponent::MainComponent (BackendManager& be, const UserSession& session)
    : AudioAppComponent(),
      backend (be),
      title ("Harmonia", be),
      oscilloscope (appLookAndFeel, 8192, 60),
      freqVolComponent (appLookAndFeel),
      adsrComponent (appLookAndFeel),
      filterComponent (appLookAndFeel)
{
    // LookAndFeel root
    setLookAndFeel (&appLookAndFeel);

    applyThemeFromId(session.themeId);
    applyLayoutFromId(session.layoutId);

    sendLookAndFeelChange();

    std::thread([this]()
    {
        auto result = backend.getProfile();

        if (!result.success)
            return;

        juce::MessageManager::callAsync([this, result]()
        {
            applyThemeFromId(result.profile.themeId);
            applyLayoutFromId(result.profile.layoutId);
        });

    }).detach();

    // ===== UI =====
    addAndMakeVisible (title);
    addAndMakeVisible (topBar);
    addAndMakeVisible (oscilloscope);
    addAndMakeVisible (freqVolComponent);
    addAndMakeVisible (adsrComponent);
    addAndMakeVisible (filterComponent);
    addAndMakeVisible (bottomBar);
    addAndMakeVisible (synthComponent);

    freqVolComponent.onParamsChanged = [this]() { updateSynthParamsFromUI(); };
    adsrComponent.onParamsChanged    = [this]() { updateSynthParamsFromUI(); };
    filterComponent.onParamsChanged  = [this]() { updateSynthParamsFromUI(); };
    topBar.onParamsChanged           = [this]() { updateSynthParamsFromUI(); };

    // ===== LAYOUT (fix: le type vient de TitleComponent, pas AppLookAndFeel) =====
    title.onLayoutSelected = [this] (AppLookAndFeel::LayoutPreset p)
    {
        switch (p)
        {
            case AppLookAndFeel::LayoutPreset::Layout1: setLayoutMode (LayoutMode::A); break;
            case AppLookAndFeel::LayoutPreset::Layout2: setLayoutMode (LayoutMode::B); break;
            case AppLookAndFeel::LayoutPreset::Layout3: setLayoutMode (LayoutMode::C); break;
            case AppLookAndFeel::LayoutPreset::Layout4: setLayoutMode (LayoutMode::D); break;
            default: break;
        }
    };

    // ===== Theme =====
    title.onThemeSelected = [this] (AppLookAndFeel::Preset p)
    {
        appLookAndFeel.setThemePreset (p);
        sendLookAndFeelChange();
        repaint();
    };

    // ===== Synth init =====
    synth.clearVoices();
    for (int i = 0; i < 8; ++i)
        synth.addVoice (new HarmoniaVoice());

    synth.clearSounds();
    synth.addSound (new HarmoniaSound());

    // ===== EXPORT JSON =====
    bottomBar.onExportClicked = [this]()
    {
        auto freqVol    = freqVolComponent.getFreqVol();
        auto adsrValues = adsrComponent.getSlidersInfo();
        auto cutoffReso = filterComponent.getSlidersInfo();

        juce::DynamicObject::Ptr obj = new juce::DynamicObject();

        obj->setProperty ("frequency",  freqVol.first);
        obj->setProperty ("volume",     freqVol.second);
        obj->setProperty ("attack",     adsrValues[0]);
        obj->setProperty ("decay",      adsrValues[1]);
        obj->setProperty ("sustain",    adsrValues[2]);
        obj->setProperty ("release",    adsrValues[3]);
        obj->setProperty ("cutoff",     cutoffReso.first);
        obj->setProperty ("resonance",  cutoffReso.second);
        obj->setProperty ("filterType", topBar.getFilterType());
        obj->setProperty ("waveform",   topBar.getWaveform());
        obj->setProperty ("prompt",     topBar.getPrompt());

        juce::var jsonVar (obj.get());
        auto jsonString = juce::JSON::toString (jsonVar, true);

        auto defaultDir  = juce::File::getSpecialLocation (juce::File::userDocumentsDirectory);
        auto defaultFile = defaultDir.getChildFile ("harmonia_patch.json");

        auto* chooser = new juce::FileChooser ("Exporter le patch en JSON",
                                               defaultFile,
                                               "*.json");

        chooser->launchAsync (
            juce::FileBrowserComponent::saveMode
            | juce::FileBrowserComponent::canSelectFiles,
            [chooser, jsonString] (const juce::FileChooser& fc) mutable
            {
                auto file = fc.getResult();
                delete chooser;

                if (file == juce::File())
                    return;

                if (file.getFileExtension().isEmpty())
                    file = file.withFileExtension (".json");

                file.replaceWithText (jsonString);
            }
        );
    };

    // ===== LOAD JSON =====
    bottomBar.onLoadClicked = [this]()
    {
        auto* chooser = new juce::FileChooser ("Select a JSON file to load", {}, "*.json");

        chooser->launchAsync (
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser] (const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                delete chooser;

                if (! file.existsAsFile())
                    return;

                juce::FileInputStream inputStream (file);
                if (! inputStream.openedOk())
                    return;

                juce::var json = juce::JSON::parse (inputStream.readEntireStreamAsString());
                if (! json.isObject())
                    return;

                auto* obj = json.getDynamicObject();
                const bool resetMissing = bottomBar.getResetToggleState();

                auto setIfPresent = [&] (const juce::String& key, auto setter, auto defaultValue)
                {
                    if (obj->hasProperty (key))
                        setter (obj->getProperty (key));
                    else if (resetMissing)
                        setter (defaultValue);
                };

                setIfPresent ("frequency", [this] (auto v) { freqVolComponent.setFrequency ((double) v); }, 440.0);
                setIfPresent ("volume",    [this] (auto v) { freqVolComponent.setVolume    ((double) v); }, 0.8);
                setIfPresent ("attack",    [this] (auto v) { adsrComponent.setAttack       ((double) v); }, 0.1);
                setIfPresent ("decay",     [this] (auto v) { adsrComponent.setDecay        ((double) v); }, 0.1);
                setIfPresent ("sustain",   [this] (auto v) { adsrComponent.setSustain      ((double) v); }, 0.8);
                setIfPresent ("release",   [this] (auto v) { adsrComponent.setRelease      ((double) v); }, 0.5);
                setIfPresent ("cutoff",    [this] (auto v) { filterComponent.setCutoff     ((double) v); }, 1000.0);
                setIfPresent ("resonance", [this] (auto v) { filterComponent.setResonance  ((double) v); }, 1.0);
                setIfPresent ("filterType",[this] (auto v) { topBar.setFilterType          (v); }, juce::String ("Low Pass"));
                setIfPresent ("prompt",    [this] (auto v) { topBar.setPrompt              (v); }, juce::String());
                setIfPresent ("waveform",  [this] (auto v) { topBar.setWaveform            (v); }, juce::String ("Sine"));

                updateSynthParamsFromUI();
            }
        );
    };

    // ===== GENERATE IA =====
    bottomBar.onGenerateClicked = [this]()
    {
        auto prompt = topBar.getPrompt();

        if (prompt.trim().isEmpty())
        {
            juce::AlertWindow::showMessageBoxAsync (
                juce::AlertWindow::WarningIcon,
                "Prompt manquant",
                "Veuillez saisir un prompt avant de générer.");
            return;
        }

        std::thread ([this, prompt]()
        {
            auto result = backend.generatePatch (prompt);

            if (! result.success)
            {
                juce::MessageManager::callAsync ([msg = result.errorMessage]()
                {
                    juce::AlertWindow::showMessageBoxAsync (
                        juce::AlertWindow::WarningIcon,
                        "Erreur IA",
                        msg);
                });
                return;
            }

            auto p = result.params;

            juce::MessageManager::callAsync ([this, p]()
            {
                topBar.setWaveform   (p.waveform);
                topBar.setFilterType (p.filterType);

                freqVolComponent.setFreqVol (p.frequency, p.volume);
                adsrComponent.setADSR (p.attack, p.decay, p.sustain, p.release);
                filterComponent.setCutoffResonance (p.cutoff, p.resonance);

                updateSynthParamsFromUI();
            });

        }).detach();
    };

    // layout par défaut
    layoutMode = LayoutMode::A;

    setSize (800, 600);
    setAudioChannels (0, 2);
}

void MainComponent::applyThemeFromId(int themeId)
{
    using Preset = AppLookAndFeel::Preset;

    switch (themeId)
    {
        case 1: appLookAndFeel.setThemePreset(Preset::Dark); break;
        case 2: appLookAndFeel.setThemePreset(Preset::Light); break;
        case 3: appLookAndFeel.setThemePreset(Preset::Blue); break;
        case 4: appLookAndFeel.setThemePreset(Preset::Red); break;
        default: appLookAndFeel.setThemePreset(Preset::Dark); break;
    }

    sendLookAndFeelChange();
    repaint();
}

void MainComponent::applyLayoutFromId(int layoutId)
{
    switch (layoutId)
    {
        case 1: setLayoutMode(LayoutMode::A); break;
        case 2: setLayoutMode(LayoutMode::B); break;
        case 3: setLayoutMode(LayoutMode::C); break;
        case 4: setLayoutMode(LayoutMode::D); break;
        default: setLayoutMode(LayoutMode::A); break;
    }
}

//==============================================================================

MainComponent::~MainComponent()
{
    shutdownAudio();
    setLookAndFeel (nullptr);
}

//==============================================================================

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (AppColourIds::backgroundId));
}

void MainComponent::setLayoutMode (LayoutMode m)
{
    layoutMode = m;
    resized();
    repaint();
}

void MainComponent::resized()
{
    // moins de marge globale
    auto area = getLocalBounds().reduced (10);

    title.setBounds (area.removeFromTop (50));
    topBar.setBounds (area.removeFromTop (70));
    bottomBar.setBounds (area.removeFromBottom (50));

    switch (layoutMode)
    {
        // Layout 1 (A) : proche de l’actuel
        case LayoutMode::A:
        {
            oscilloscope.setBounds (area.removeFromTop (100));
            freqVolComponent.setBounds (area.removeFromTop (110));
            adsrComponent.setBounds (area.removeFromTop (110));
            filterComponent.setBounds (area.removeFromTop (110));
            synthComponent.setBounds (area);
            break;
        }

        // Layout 2 (B) : oscillo plus grand
        case LayoutMode::B:
        {
            oscilloscope.setBounds(area.removeFromTop(170));

            // ===== KEYBOARD =====
            synthComponent.setBounds(area.removeFromTop(100)); // tu peux ajuster la hauteur

            // ===== FREQ + FILTER côte à côte =====
            auto freqFilterArea = area.removeFromTop(120); // hauteur de la ligne freq+filter
            auto halfWidth = freqFilterArea.getWidth() / 2;

            freqVolComponent.setBounds(freqFilterArea.removeFromLeft(halfWidth));
            filterComponent.setBounds(freqFilterArea); // le reste à droite

            // ===== ADSR =====
            adsrComponent.setBounds(area.removeFromTop(120));

            break;
        }

        // Layout 3 (C) : oscillo gauche, contrôles droite
        case LayoutMode::C:
        {
            auto upper = area.removeFromTop (260);

            auto left = upper.removeFromLeft ((int) (upper.getWidth() * 0.65f));
            oscilloscope.setBounds (left);

            auto right = upper;
            freqVolComponent.setBounds (right.removeFromTop (130));
            filterComponent.setBounds (right);

            adsrComponent.setBounds (area.removeFromTop (130));
            synthComponent.setBounds (area);
            break;
        }

        // Layout 4 (D) : clavier plus grand
        case LayoutMode::D:
        {
            auto upper = area.removeFromTop(260);

            // On met l'oscilloscope à droite
            auto left = upper.removeFromLeft((int)(upper.getWidth() * 0.35f));
            freqVolComponent.setBounds(left.removeFromTop(130));
            filterComponent.setBounds(left);

            auto right = upper;
            oscilloscope.setBounds(right);

            adsrComponent.setBounds(area.removeFromTop(130));
            synthComponent.setBounds(area);
            break;
        }

        default:
        {
            oscilloscope.setBounds (area.removeFromTop (120));
            freqVolComponent.setBounds (area.removeFromTop (100));
            adsrComponent.setBounds (area.removeFromTop (100));
            filterComponent.setBounds (area.removeFromTop (100));
            synthComponent.setBounds (area);
            break;
        }
    }
}

//==============================================================================
// AUDIO

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    synth.setCurrentPlaybackSampleRate (sampleRate);

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<HarmoniaVoice*> (synth.getVoice (i)))
            v->prepare (sampleRate, samplesPerBlockExpected, 2);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    juce::MidiBuffer midi;
    synthComponent.getKeyboardState().processNextMidiBuffer (
        midi,
        bufferToFill.startSample,
        bufferToFill.numSamples,
        true
    );

    synth.renderNextBlock (*bufferToFill.buffer, midi,
                           bufferToFill.startSample, bufferToFill.numSamples);

    oscilloscope.pushBuffer (*bufferToFill.buffer,
                             bufferToFill.startSample,
                             bufferToFill.numSamples);
}

void MainComponent::releaseResources() {}

//==============================================================================
// SYNTH PARAMS

void MainComponent::updateSynthParamsFromUI()
{
    auto freqVol    = freqVolComponent.getFreqVol();
    auto adsrValues = adsrComponent.getSlidersInfo();
    auto cutoffReso = filterComponent.getSlidersInfo();

    HarmoniaParams params;

    params.frequency = (float) freqVol.first;
    params.volume    = (float) freqVol.second;

    params.attack  = (float) adsrValues[0];
    params.decay   = (float) adsrValues[1];
    params.sustain = (float) adsrValues[2];
    params.release = (float) adsrValues[3];

    params.cutoff    = (float) cutoffReso.first;
    params.resonance = (float) cutoffReso.second;

    params.filterType    = topBar.getFilterType();
    params.waveformIndex = topBar.getWaveformIndex();

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<HarmoniaVoice*> (synth.getVoice (i)))
            v->setParameters (params);
}

void MainComponent::triggerPreviewNote()
{
    synth.noteOn (1, 60, 0.8f);
}
