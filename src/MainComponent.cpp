#include "MainComponent.h"
#include <filesystem>
#include <fstream>

MainComponent::MainComponent()
    : waveformSelector("Waveform"),
      frequencySlider("Fréquence", 20.0, 20000.0, 440.0),
      volumeSlider("Volume", 0.0, 1.0, 0.8),
      generateButton("Generate")
{
    addAndMakeVisible(clearPromptButton);
clearPromptButton.onClick = [this]() {
    promptEditor.clear();
};

    setLookAndFeel(&customLookAndFeel);
    waveformSelector.setLookAndFeel(&customLookAndFeel);
    filterTypeSelector.setLookAndFeel(&customLookAndFeel);

    // Waveform label
    waveformLabel.setText("Waveform", juce::dontSendNotification);
    waveformLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    addAndMakeVisible(waveformLabel);
    addAndMakeVisible(waveformSelector);

    // Filter ComboBox
    filterTypeSelector.addItem("Low Pass", 1);
    filterTypeSelector.addItem("High Pass", 2);
    filterTypeSelector.addItem("Band Pass", 3);
    filterTypeSelector.setSelectedId(1);
    filterTypeSelector.setTextWhenNothingSelected("Filter");
    addAndMakeVisible(filterTypeSelector);

    // Prompt editor
    promptLabel.setText("Prompt", juce::dontSendNotification);
    promptLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    promptLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(promptLabel);

    promptEditor.setMultiLine(true);
    promptEditor.setReturnKeyStartsNewLine(true);
    promptEditor.setTextToShowWhenEmpty("Type your prompt here...", juce::Colours::grey);
    addAndMakeVisible(promptEditor);

    // Frequency
    frequencySlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    frequencySlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    frequencyLabel.setText("Frequency", juce::dontSendNotification);
    frequencyLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    frequencyLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(frequencySlider);
    addAndMakeVisible(frequencyLabel);

    // Volume
    volumeSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    volumeSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    volumeLabel.setText("Volume", juce::dontSendNotification);
    volumeLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    volumeLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(volumeSlider);
    addAndMakeVisible(volumeLabel);

    // ADSR sliders
    auto setupRotarySlider = [this](juce::Slider& slider, juce::Label& label, const juce::String& name, float min, float max, float step, float defaultValue) {
        slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
        slider.setRange(min, max, step);
        slider.setValue(defaultValue);
        slider.setName(name);
        label.setText(name, juce::dontSendNotification);
        label.setColour(juce::Label::textColourId, juce::Colours::white);
        label.setJustificationType(juce::Justification::centred);
        addAndMakeVisible(slider);
        addAndMakeVisible(label);
    };

    setupRotarySlider(attackSlider, attackLabel, "Attack", 0.01f, 5.0f, 0.01f, 0.1f);
    setupRotarySlider(decaySlider, decayLabel, "Decay", 0.01f, 5.0f, 0.01f, 0.1f);
    setupRotarySlider(sustainSlider, sustainLabel, "Sustain", 0.0f, 1.0f, 0.01f, 0.8f);
    setupRotarySlider(releaseSlider, releaseLabel, "Release", 0.01f, 5.0f, 0.01f, 0.5f);

    // Filter sliders
    cutoffSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    cutoffSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    cutoffSlider.setRange(20.0, 20000.0, 1.0);
    cutoffSlider.setSkewFactorFromMidPoint(1000.0);
    cutoffSlider.setValue(1000.0);
    cutoffLabel.setText("Cutoff", juce::dontSendNotification);
    cutoffLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    cutoffLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(cutoffSlider);
    addAndMakeVisible(cutoffLabel);

    resonanceSlider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    resonanceSlider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    resonanceSlider.setRange(0.1, 10.0, 0.1);
    resonanceSlider.setValue(1.0);
    resonanceLabel.setText("Resonance", juce::dontSendNotification);
    resonanceLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    resonanceLabel.setJustificationType(juce::Justification::centred);
    addAndMakeVisible(resonanceSlider);
    addAndMakeVisible(resonanceLabel);

    // Generate button
    addAndMakeVisible(generateButton);
    generateButton.onClick = [this]() {
        DataModel model;
        model.waveform = waveformSelector.getSelectedWaveform();
        model.frequency = frequencySlider.getValue();
        model.volume = volumeSlider.getValue();
        model.attack = attackSlider.getValue();
        model.decay = decaySlider.getValue();
        model.sustain = sustainSlider.getValue();
        model.release = releaseSlider.getValue();
        model.filterType = filterTypeSelector.getText();
        model.cutoff = cutoffSlider.getValue();
        model.resonance = resonanceSlider.getValue();
        model.prompt = promptEditor.getText();
        model.saveToJson();
    };

    setSize(800, 600);
}

void MainComponent::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
    g.setColour(juce::Colours::white);
    g.setFont(16.0f);
    g.drawFittedText("Harmonia", getLocalBounds(), juce::Justification::centredTop, 1);
}

void MainComponent::resized()
{
     auto area = getLocalBounds().reduced(20);
    auto topRow = area.removeFromTop(40);

    // Gauche : waveform + filtre
    waveformLabel.setBounds(topRow.removeFromLeft(80));
    waveformSelector.setBounds(topRow.removeFromLeft(150));
    filterTypeSelector.setBounds(topRow.removeFromLeft(120).translated(10, 0));

    // Droite : prompt label + editor + bouton clear
    int promptEditorWidth = 220;
    int promptLabelWidth = 60;
    int promptHeight = 40;

    promptLabel.setBounds(topRow.removeFromLeft(promptLabelWidth).translated(30, 0));
    promptEditor.setBounds(topRow.removeFromLeft(promptEditorWidth).translated(30, 0));
    clearPromptButton.setBounds(topRow.removeFromLeft(100).translated(30, 0));

    // Frequency & Volume
    auto freqVolRow = area.removeFromTop(100);
    int freqVolW = freqVolRow.getWidth() / 2;

    frequencyLabel.setBounds(freqVolRow.getX(), freqVolRow.getY(), freqVolW, 20);
    frequencySlider.setBounds(freqVolRow.getX(), freqVolRow.getY() + 20, freqVolW, 80);

    volumeLabel.setBounds(freqVolRow.getX() + freqVolW, freqVolRow.getY(), freqVolW, 20);
    volumeSlider.setBounds(freqVolRow.getX() + freqVolW, freqVolRow.getY() + 20, freqVolW, 80);

    // ADSR
    auto adsrRow = area.removeFromTop(100);
    int sliderWidth = adsrRow.getWidth() / 4;

    attackLabel.setBounds(adsrRow.getX(), adsrRow.getY(), sliderWidth, 20);
    attackSlider.setBounds(adsrRow.getX(), adsrRow.getY() + 20, sliderWidth, 80);

    decayLabel.setBounds(adsrRow.getX() + sliderWidth, adsrRow.getY(), sliderWidth, 20);
    decaySlider.setBounds(adsrRow.getX() + sliderWidth, adsrRow.getY() + 20, sliderWidth, 80);

    sustainLabel.setBounds(adsrRow.getX() + 2 * sliderWidth, adsrRow.getY(), sliderWidth, 20);
    sustainSlider.setBounds(adsrRow.getX() + 2 * sliderWidth, adsrRow.getY() + 20, sliderWidth, 80);

    releaseLabel.setBounds(adsrRow.getX() + 3 * sliderWidth, adsrRow.getY(), sliderWidth, 20);
    releaseSlider.setBounds(adsrRow.getX() + 3 * sliderWidth, adsrRow.getY() + 20, sliderWidth, 80);

    // Filters
    auto filterRow = area.removeFromTop(100);
    int halfWidth = filterRow.getWidth() / 2;

    cutoffLabel.setBounds(filterRow.getX(), filterRow.getY(), halfWidth, 20);
    cutoffSlider.setBounds(filterRow.getX(), filterRow.getY() + 20, halfWidth, 80);

    resonanceLabel.setBounds(filterRow.getX() + halfWidth, filterRow.getY(), halfWidth, 20);
    resonanceSlider.setBounds(filterRow.getX() + halfWidth, filterRow.getY() + 20, halfWidth, 80);

    generateButton.setBounds(area.removeFromBottom(30));
}
