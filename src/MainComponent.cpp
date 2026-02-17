#include "MainComponent.h"
#include "themes/AppColourIds.h"

#include <filesystem>
#include <fstream>
#include <thread>

//==============================================================================

MainComponent::MainComponent (BackendManager& be)
    : AudioAppComponent(),
      backend (be),
      title ("Harmonia", be),
      oscilloscope (8192, 60),
      freqVolComponent (appLookAndFeel),
      adsrComponent (appLookAndFeel),
      filterComponent (appLookAndFeel)
{
    // LookAndFeel root
    setLookAndFeel (&appLookAndFeel);

    // ===== UI =====
    addAndMakeVisible (title);
    addAndMakeVisible (topBar);

    // Nouveau : oscilloscope juste après topBar
    addAndMakeVisible (oscilloscope);

    addAndMakeVisible (freqVolComponent);
    addAndMakeVisible (adsrComponent);
    addAndMakeVisible (filterComponent);
    addAndMakeVisible (bottomBar);
    addAndMakeVisible (synthComponent);

    freqVolComponent.onParamsChanged = [this]()
    {
        updateSynthParamsFromUI();
    };

    adsrComponent.onParamsChanged = [this]()
    {
        updateSynthParamsFromUI();
    };

    filterComponent.onParamsChanged = [this]()
    {
        updateSynthParamsFromUI();
    };

    // Si votre TopBar change waveform / filterType via ComboBox
    topBar.onParamsChanged = [this]()
    {
        updateSynthParamsFromUI();
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

    setSize (800, 600);
    setAudioChannels (0, 2);
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

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced (20);

    title.setBounds (area.removeFromTop (50));
    topBar.setBounds (area.removeFromTop (100));

    // Nouveau : oscilloscope placé juste après topBar
    oscilloscope.setBounds (area.removeFromTop (120));

    freqVolComponent.setBounds (area.removeFromTop (100));
    adsrComponent.setBounds (area.removeFromTop (100));
    filterComponent.setBounds (area.removeFromTop (100));
    synthComponent.setBounds (area.removeFromTop (80));
    bottomBar.setBounds (area.removeFromBottom (40));
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
// SYNTH PARAMS — VERSION PROPRE ET COMPLÈTE

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
