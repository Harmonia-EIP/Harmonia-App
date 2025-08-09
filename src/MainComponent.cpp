#include "MainComponent.h"
#include <filesystem>
#include <fstream>

MainComponent::MainComponent()
    : freqVolComponent(customLookAndFeel),
      adsrComponent(customLookAndFeel),
      filterComponent(customLookAndFeel)
{
    addAndMakeVisible(title);
    addAndMakeVisible(topBar);

    addAndMakeVisible(freqVolComponent);
    addAndMakeVisible(adsrComponent);
    addAndMakeVisible(filterComponent);

    // LookAndFeel global
    setLookAndFeel(&customLookAndFeel);

    // Bottom bar
    addAndMakeVisible(bottomBar);

    // Bouton Generate
    bottomBar.onGenerateClicked = [this]()
    {
        DataModel model;
        auto freqVol    = freqVolComponent.getFreqVol();
        auto adsrValues = adsrComponent.getSlidersInfo();
        auto cutoffReso = filterComponent.getSlidersInfo();

        model.frequency = freqVol.first;
        model.volume    = freqVol.second;

        model.attack  = adsrValues[0];
        model.decay   = adsrValues[1];
        model.sustain = adsrValues[2];
        model.release = adsrValues[3];

        model.cutoff    = cutoffReso.first;
        model.resonance = cutoffReso.second;

        model.filterType = topBar.getFilterType();
        model.waveform   = topBar.getWaveform();
        model.prompt     = topBar.getPrompt();

        model.saveToJson();
    };

    // Bouton Load
    bottomBar.onLoadClicked = [this]()
    {
        auto* chooser = new juce::FileChooser("Select a JSON file to load", {}, "*.json");

        chooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                delete chooser;

                if (!file.existsAsFile())
                    return;

                juce::FileInputStream inputStream(file);
                if (!inputStream.openedOk())
                    return;

                juce::var json = juce::JSON::parse(inputStream.readEntireStreamAsString());
                if (!json.isObject())
                    return;

                auto* obj = json.getDynamicObject();
                const bool resetMissing = bottomBar.getResetToggleState();

                auto setIfPresent = [&](const juce::String& key, auto setter, auto defaultValue)
                {
                    if (obj->hasProperty(key))
                        setter(obj->getProperty(key));
                    else if (resetMissing)
                        setter(defaultValue);
                };

                setIfPresent("frequency", [this](auto v) { freqVolComponent.setFrequency((double)v); }, 440.0);
                setIfPresent("volume",    [this](auto v) { freqVolComponent.setVolume((double)v); }, 0.8);
                setIfPresent("attack",    [this](auto v) { adsrComponent.setAttack((double)v); }, 0.1);
                setIfPresent("decay",     [this](auto v) { adsrComponent.setDecay((double)v); }, 0.2);
                setIfPresent("sustain",   [this](auto v) { adsrComponent.setSustain((double)v); }, 0.7);
                setIfPresent("release",   [this](auto v) { adsrComponent.setRelease((double)v); }, 0.3);
                setIfPresent("cutoff",    [this](auto v) { filterComponent.setCutoff((double)v); }, 1000.0);
                setIfPresent("resonance", [this](auto v) { filterComponent.setResonance((double)v); }, 0.5);
                setIfPresent("filterType",[this](auto v) { topBar.setFilterType(v); }, juce::String("Low-Pass"));
                setIfPresent("prompt",    [this](auto v) { topBar.setPrompt(v); },     juce::String(""));
                setIfPresent("waveform",  [this](auto v) { topBar.setWaveform(v); },   juce::String("Sine"));
            }
        );
    };

    setSize(800, 600);
}

MainComponent::~MainComponent()
{
    setLookAndFeel(nullptr);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(20);

    auto titleBar = area.removeFromTop(20);
    title.setBounds(titleBar);

    auto topRow = area.removeFromTop(100);
    topBar.setBounds(topRow);

    freqVolComponent.setBounds(area.removeFromTop(120));
    adsrComponent.setBounds(area.removeFromTop(100));
    filterComponent.setBounds(area.removeFromTop(100));

    bottomBar.setBounds(area.removeFromBottom(40));
}
