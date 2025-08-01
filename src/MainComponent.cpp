#include "MainComponent.h"
#include <filesystem>
#include <fstream>

//==============================================================================

MainComponent::MainComponent()
    : generateButton("Generate"),
      freqVolComponent(customLookAndFeel),
      adsrComponent(customLookAndFeel),
      filterComponent(customLookAndFeel)
{
    // Title Component
    addAndMakeVisible(title);

    // Top Bar Component
    addAndMakeVisible(topBar);

    
    
    addAndMakeVisible(freqVolComponent);
    addAndMakeVisible(adsrComponent);
    addAndMakeVisible(filterComponent);
    
    // LookAndFeel global
    setLookAndFeel(&customLookAndFeel);
   

    // Generate button
    addAndMakeVisible(generateButton);
    // generateButton.onClick = [this]() {
    //     DataModel model;
    //     model.frequency = frequencySlider.getValue();
    //     model.volume = volumeSlider.getValue();
    //     model.attack = attackSlider.getValue();
    //     model.decay = decaySlider.getValue();
    //     model.sustain = sustainSlider.getValue();
    //     model.release = releaseSlider.getValue();
    //     model.filterType = filterTypeSelector.getText();
    //     model.cutoff = cutoffSlider.getValue();
    //     model.resonance = resonanceSlider.getValue();
    //     model.saveToJson();
    // };

    setSize(800, 600);
}

MainComponent::~MainComponent()
{
    // Important : reset LookAndFeel pour éviter les use-after-free
    setLookAndFeel(nullptr);

}

//==============================================================================

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


    generateButton.setBounds(area.removeFromBottom(30));
}
