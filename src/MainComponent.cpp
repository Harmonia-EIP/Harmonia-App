#include "MainComponent.h"
#include <filesystem>
#include <fstream>

MainComponent::MainComponent()
    : waveformSelector("Waveform"),
        frequencySlider("Fréquence", 20.0, 20000.0, 440.0),
        volumeSlider("Volume", 0.0, 1.0, 0.8),
        generateButton("Générer")
{
    int width = 400;
    int height = 300;

    addAndMakeVisible(waveformSelector);
    addAndMakeVisible(frequencySlider);
    addAndMakeVisible(volumeSlider);
    addAndMakeVisible(generateButton);

    generateButton.onClick = [this]() {
        DataModel model;
        model.waveform = waveformSelector.getSelectedWaveform();
        model.frequency = frequencySlider.getValue();
        model.volume = volumeSlider.getValue();
        model.saveToJson();
    };

    setSize(width, height);
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawFittedText("Harmonia", getLocalBounds(), juce::Justification::centredTop, 1);
}

void MainComponent::resized()
{
    auto area = getLocalBounds().reduced(20);
    waveformSelector.setBounds(area.removeFromTop(40));
    frequencySlider.setBounds(area.removeFromTop(40));
    volumeSlider.setBounds(area.removeFromTop(40));
    generateButton.setBounds(area.removeFromTop(30));
}