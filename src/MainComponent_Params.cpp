#include "MainComponent.h"


void MainComponent::initExportParameters()
{
    bottomBar.onExportClicked = [this]()
    {
        auto jsonString = patchController.toJson();

        auto* chooser = new juce::FileChooser(
            "Exporter le patch en JSON",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                .getChildFile("harmonia_patch.json"),
            "*.json"
        );

        chooser->launchAsync(
            juce::FileBrowserComponent::saveMode
            | juce::FileBrowserComponent::canSelectFiles,
            [chooser, jsonString](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                delete chooser;

                if (file == juce::File())
                    return;

                if (file.getFileExtension().isEmpty())
                    file = file.withFileExtension(".json");

                file.replaceWithText(jsonString);
            }
        );
    };
}

void MainComponent::initLoadParameters()
{
    bottomBar.onLoadClicked = [this]()
    {
        auto* chooser = new juce::FileChooser(
            "Select a JSON file to load", {}, "*.json"
        );

        chooser->launchAsync(
            juce::FileBrowserComponent::openMode
            | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                delete chooser;

                if (!file.existsAsFile())
                    return;

                auto content = file.loadFileAsString();

                if (!patchController.fromJson(content))
                    return;

                patchController.applyToProcessor();
            }
        );
    };
}

void MainComponent::onUIChanged()
{
    if (isUpdatingUI)
        return;

    auto p = collectParamsFromUI();
    patchController.setPatch(p);
    patchController.applyToProcessor();
}

PatchParams MainComponent::collectParamsFromUI()
{
    auto freqVol    = freqVolComponent.getFreqVol();
    auto adsrValues = adsrComponent.getSlidersInfo();
    auto cutoffReso = filterComponent.getSlidersInfo();

    PatchParams p = patchController.getPatch();

    p.frequency = freqVol.first;
    p.volume    = freqVol.second;

    p.attack  = adsrValues[0];
    p.decay   = adsrValues[1];
    p.sustain = adsrValues[2];
    p.release = adsrValues[3];

    p.cutoff    = cutoffReso.first;
    p.resonance = cutoffReso.second;

    p.waveform = topBar.getWaveformEnum();
    p.filterType = topBar.getFilterEnum();

    p.prompt = topBar.getPrompt();

    return p;
}

void MainComponent::applyParamsToUI(const PatchParams& p)
{
    isUpdatingUI = true;

    freqVolComponent.setFreqVol(p.frequency, p.volume);
    adsrComponent.setADSR(p.attack, p.decay, p.sustain, p.release);
    filterComponent.setCutoffResonance(p.cutoff, p.resonance);

    topBar.setFilterType(PatchSerializer::filterToString(p.filterType));
    topBar.setWaveform(PatchSerializer::waveformToString(p.waveform));
    topBar.setPrompt(p.prompt);

    isUpdatingUI = false;
}