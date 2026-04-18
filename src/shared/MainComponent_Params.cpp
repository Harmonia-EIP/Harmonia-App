#include "MainComponent.h"


void MainComponent::initExportParameters()
{
    bottomBar.onExportClicked = [this]()
    {
        PatchParams p;

        auto freqVol    = freqVolComponent.getFreqVol();
        auto adsrValues = adsrComponent.getSlidersInfo();
        auto cutoffReso = filterComponent.getSlidersInfo();

        p.frequency = freqVol.first;
        p.volume    = freqVol.second;

        p.attack  = adsrValues[0];
        p.decay   = adsrValues[1];
        p.sustain = adsrValues[2];
        p.release = adsrValues[3];

        p.cutoff    = cutoffReso.first;
        p.resonance = cutoffReso.second;

        p.filterType = PatchSerializer::filterFromString(topBar.getFilterType());
        p.waveform   = PatchSerializer::waveformFromString(topBar.getWaveform());
        p.prompt     = topBar.getPrompt();

        auto jsonString = PatchSerializer::toJson(p);

        auto defaultDir  = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory);
        auto defaultFile = defaultDir.getChildFile("harmonia_patch.json");

        auto* chooser = new juce::FileChooser("Exporter le patch en JSON", defaultFile, "*.json");

        chooser->launchAsync(
            juce::FileBrowserComponent::saveMode
            | juce::FileBrowserComponent::canSelectFiles,
            [chooser, jsonString](const juce::FileChooser& fc) mutable
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
        auto* chooser = new juce::FileChooser("Select a JSON file to load", {}, "*.json");

        chooser->launchAsync(
            juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this, chooser](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                delete chooser;

                if (!file.existsAsFile())
                    return;

                auto content = file.loadFileAsString();

                auto params = PatchSerializer::fromJson(content);
                if (!params.has_value())
                    return;

                const auto& p = *params;

                freqVolComponent.setFreqVol(p.frequency, p.volume);
                adsrComponent.setADSR(p.attack, p.decay, p.sustain, p.release);
                filterComponent.setCutoffResonance(p.cutoff, p.resonance);

                topBar.setFilterType(PatchSerializer::filterToString(p.filterType));
                topBar.setWaveform(PatchSerializer::waveformToString(p.waveform));
                topBar.setPrompt(p.prompt);

                updateSynthParamsFromUI();
            }
        );
    };
}

void MainComponent::updateSynthParamsFromUI()
{
    auto freqVol    = freqVolComponent.getFreqVol();
    auto adsrValues = adsrComponent.getSlidersInfo();
    auto cutoffReso = filterComponent.getSlidersInfo();

    PatchParams params;

    params.frequency = freqVol.first;
    params.volume    = freqVol.second;

    params.attack  = adsrValues[0];
    params.decay   = adsrValues[1];
    params.sustain = adsrValues[2];
    params.release = adsrValues[3];

    params.cutoff    = cutoffReso.first;
    params.resonance = cutoffReso.second;

    params.filterType = PatchSerializer::filterFromString(
        topBar.getFilterType()
    );

    params.waveform = [&]()
    {
        switch (topBar.getWaveformIndex())
        {
            case 1: return Waveform::SAW;
            case 2: return Waveform::SQUARE;
            case 3: return Waveform::TRIANGLE;
            default: return Waveform::SINE;
        }
    }();

    for (int i = 0; i < synth.getNumVoices(); ++i)
    {
        if (auto* v = dynamic_cast<HarmoniaVoice*>(synth.getVoice(i)))
            v->setParameters(params);
    }
}
