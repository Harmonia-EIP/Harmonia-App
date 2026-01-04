#include "FrequencyVolumeComponent.h"

FrequencyVolumeComponent::FrequencyVolumeComponent(CustomLookAndFeel& lnF)
    : lookAndFeel(lnF),
      frequencySlider("Frequency", 20.0, 20000.0, 440.0),
      volumeSlider("Volume", 0.0, 1.0, 0.8)
{
    setupSlider(frequencySlider, frequencyLabel, "Frequency");
    setupSlider(volumeSlider, volumeLabel, "Volume");

    addAndMakeVisible(frequencySlider);
    addAndMakeVisible(frequencyLabel);
    addAndMakeVisible(volumeSlider);
    addAndMakeVisible(volumeLabel);
}

FrequencyVolumeComponent::~FrequencyVolumeComponent()
{
    frequencySlider.setLookAndFeel(nullptr);
    volumeSlider.setLookAndFeel(nullptr);
}

void FrequencyVolumeComponent::setFreqVol(double frequency, double volume)
{
    frequencySlider.setValue(frequency, juce::NotificationType::dontSendNotification);
    volumeSlider.setValue(volume, juce::NotificationType::dontSendNotification);
}

void FrequencyVolumeComponent::setFrequency(double freq)
{
    frequencySlider.setValue(freq, juce::NotificationType::dontSendNotification);
}
    
void FrequencyVolumeComponent::setVolume(double vol)
{
    volumeSlider.setValue(vol, juce::NotificationType::dontSendNotification);
}

std::pair<double, double> FrequencyVolumeComponent::getFreqVol() const
{
    return std::make_pair<double, double>(frequencySlider.getValue(), volumeSlider.getValue());
}

double FrequencyVolumeComponent::getFreq() const
{
    return frequencySlider.getValue();
}
    
double FrequencyVolumeComponent::getVolume() const
{
    return volumeSlider.getValue();
}

void FrequencyVolumeComponent::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 60, 20);
    slider.setLookAndFeel(&lookAndFeel);

    label.setText(name, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    label.setJustificationType(juce::Justification::centred);
}

void FrequencyVolumeComponent::resized()
{
    auto area = getLocalBounds();
    auto half = area.getWidth() / 2;

    frequencyLabel.setBounds(0, 0, half, 20);
    frequencySlider.setBounds(0, 20, half, getHeight() - 20);

    volumeLabel.setBounds(half, 0, half, 20);
    volumeSlider.setBounds(half, 20, half, getHeight() - 20);
}
