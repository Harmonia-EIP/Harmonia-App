#include "PluginEditor.h"


HarmoniaAudioProcessorEditor::HarmoniaAudioProcessorEditor (HarmoniaAudioProcessor& p)
    : AudioProcessorEditor (&p), audioProcessor (p)
{
    
    addAndMakeVisible(pluginAppWrapper);
    setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);
}

HarmoniaAudioProcessorEditor::~HarmoniaAudioProcessorEditor() {}


void HarmoniaAudioProcessorEditor::paint (juce::Graphics& g)
{
     g.fillAll (juce::Colours::black);
}

void HarmoniaAudioProcessorEditor::resized()
{
    pluginAppWrapper.setBounds(getLocalBounds());
}