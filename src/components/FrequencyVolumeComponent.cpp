#include "FrequencyVolumeComponent.h"
#include "../themes/AppColourIds.h"

FrequencyVolumeComponent::FrequencyVolumeComponent (AppLookAndFeel& lnF)
    : lookAndFeel (lnF)
{
    setupSlider (frequencySlider, frequencyLabel, "Frequency", 20.0, 20000.0, 440.0);
    frequencySlider.setSkewFactorFromMidPoint (1000.0);

    setupSlider (volumeSlider, volumeLabel, "Volume", 0.0, 1.0, 0.8);

    addAndMakeVisible (frequencySlider);
    addAndMakeVisible (frequencyLabel);
    addAndMakeVisible (volumeSlider);
    addAndMakeVisible (volumeLabel);

    // IMPORTANT : quand l’utilisateur touche un slider, on notifie
    frequencySlider.onValueChange = [this]()
    {
        if (onParamsChanged) onParamsChanged();
    };

    volumeSlider.onValueChange = [this]()
    {
        if (onParamsChanged) onParamsChanged();
    };
}

FrequencyVolumeComponent::~FrequencyVolumeComponent()
{
    frequencySlider.setLookAndFeel (nullptr);
    volumeSlider.setLookAndFeel (nullptr);
}

void FrequencyVolumeComponent::setFreqVol (double frequency, double volume)
{
    frequencySlider.setValue (frequency, juce::dontSendNotification);
    volumeSlider.setValue (volume, juce::dontSendNotification);
}

void FrequencyVolumeComponent::setFrequency (double freq)
{
    frequencySlider.setValue (freq, juce::dontSendNotification);
}

void FrequencyVolumeComponent::setVolume (double vol)
{
    volumeSlider.setValue (vol, juce::dontSendNotification);
}

std::pair<double, double> FrequencyVolumeComponent::getFreqVol() const
{
    return { frequencySlider.getValue(), volumeSlider.getValue() };
}

double FrequencyVolumeComponent::getFreq() const
{
    return frequencySlider.getValue();
}

double FrequencyVolumeComponent::getVolume() const
{
    return volumeSlider.getValue();
}

void FrequencyVolumeComponent::setupSlider (juce::Slider& slider,
                                           juce::Label& label,
                                           const juce::String& name,
                                           double min, double max, double def)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);

    // 🔹 Augmentation largeur du TextBox pour mieux voir les chiffres
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 80, 24); // plus large et un peu plus haut
    slider.setRange (min, max);
    slider.setValue (def);
    slider.setLookAndFeel (&lookAndFeel);

    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
}

void FrequencyVolumeComponent::applyThemeColours()
{
    auto text = findColour (AppColourIds::textPrimaryId);

    frequencyLabel.setColour (juce::Label::textColourId, text);
    volumeLabel.setColour (juce::Label::textColourId, text);
}

void FrequencyVolumeComponent::lookAndFeelChanged()
{
    applyThemeColours();
}

void FrequencyVolumeComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (AppColourIds::panelBgId));

    g.setColour (findColour (AppColourIds::panelOutlineId));
    g.drawRect (getLocalBounds(), 1);
}

void FrequencyVolumeComponent::resized()
{
    auto area = getLocalBounds();
    auto half = area.getWidth() / 2;

    const int labelHeight = 22;       // labels un peu plus gros
    const int sliderHeight = area.getHeight() - labelHeight - 10; // 10px de marge en bas

    // Frequency
    frequencyLabel.setBounds (0, 0, half, labelHeight);
    frequencySlider.setBounds (0, labelHeight, half, sliderHeight);

    // Volume
    volumeLabel.setBounds (half, 0, half, labelHeight);
    volumeSlider.setBounds (half, labelHeight, half, sliderHeight);
}
