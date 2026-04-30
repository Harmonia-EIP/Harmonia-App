#include "PluginEditor.h"

HarmoniaAudioProcessorEditor::HarmoniaAudioProcessorEditor (HarmoniaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p), synthComponent(audioProcessor.getKeyboardState()), appController(p)
{
    
    addAndMakeVisible(appController);
    setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);
}

HarmoniaAudioProcessorEditor::~HarmoniaAudioProcessorEditor() {}


void HarmoniaAudioProcessorEditor::paint (juce::Graphics& g)
{
     g.fillAll (juce::Colours::black);
}

void HarmoniaAudioProcessorEditor::resized()
{
    appController.setBounds(getLocalBounds());
}