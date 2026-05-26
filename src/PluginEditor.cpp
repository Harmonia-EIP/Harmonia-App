#include "PluginEditor.h"

HarmoniaAudioProcessorEditor::HarmoniaAudioProcessorEditor (HarmoniaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    appController = std::make_unique<AppController>(p);
    addAndMakeVisible(appController.get());
    setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);
}

HarmoniaAudioProcessorEditor::~HarmoniaAudioProcessorEditor() {}


void HarmoniaAudioProcessorEditor::paint (juce::Graphics& g)
{
     g.fillAll (juce::Colours::black);
}

void HarmoniaAudioProcessorEditor::resized()
{
    appController->setBounds(getLocalBounds());
}