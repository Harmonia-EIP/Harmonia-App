#include "FrequencyVolumeComponent.h"

FrequencyVolumeComponent::FrequencyVolumeComponent (AppLookAndFeel& lnF)
    : lookAndFeel (lnF)
{
    setupSlider (frequencySlider, frequencyLabel, Strings::Parameters::Frequency, AudioConfig::Frequency::Min, AudioConfig::Frequency::Max, AudioConfig::Frequency::Default);
    frequencySlider.setSkewFactorFromMidPoint (AudioConfig::Frequency::Mid);

    setupSlider (volumeSlider, volumeLabel, Strings::Parameters::Volume, AudioConfig::Volume::Min, AudioConfig::Volume::Max, AudioConfig::Volume::Default);

    addAndMakeVisible (frequencySlider);
    addAndMakeVisible (frequencyLabel);
    addAndMakeVisible (volumeSlider);
    addAndMakeVisible (volumeLabel);

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

    const int labelHeight = 22;
    const int sliderHeight = area.getHeight() - labelHeight - 10;

    frequencyLabel.setBounds (0, 0, half, labelHeight);
    frequencySlider.setBounds (0, labelHeight, half, sliderHeight);

    volumeLabel.setBounds (half, 0, half, labelHeight);
    volumeSlider.setBounds (half, labelHeight, half, sliderHeight);
}
