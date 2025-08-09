#include "FilterComponent.h"

FilterComponent::FilterComponent(CustomLookAndFeel& lnF) : lookAndFeel(lnF)
{
    setupSlider(cutoffSlider, cutoffLabel, "Cutoff", 20.0, 20000.0, 1000.0);
    cutoffSlider.setSkewFactorFromMidPoint(1000.0);

    setupSlider(resonanceSlider, resonanceLabel, "Resonance", 0.1, 10.0, 1.0);

    addAndMakeVisible(cutoffSlider); addAndMakeVisible(cutoffLabel);
    addAndMakeVisible(resonanceSlider); addAndMakeVisible(resonanceLabel);
}

FilterComponent::~FilterComponent()
{
    cutoffSlider.setLookAndFeel(nullptr);
    resonanceSlider.setLookAndFeel(nullptr);
}

std::pair<double, double> FilterComponent::getSlidersInfo() const
{
    return std::make_pair<double, double>(cutoffSlider.getValue(), resonanceSlider.getValue());
}

double FilterComponent::getCutoff() const
{
    return cutoffSlider.getValue();
}
    
double FilterComponent::getResonance() const
{
    return resonanceSlider.getValue();
}

void FilterComponent::setCutoff(double cutoff)
{
    cutoffSlider.setValue(cutoff, juce::NotificationType::dontSendNotification);
}

void FilterComponent::setResonance(double resonance)
{
    resonanceSlider.setValue(resonance, juce::NotificationType::dontSendNotification);
}

void FilterComponent::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name,
                             float min, float max, float def)
{
    slider.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 20);
    slider.setRange(min, max);
    slider.setValue(def);
    slider.setLookAndFeel(&lookAndFeel);

    label.setText(name, juce::dontSendNotification);
    label.setColour(juce::Label::textColourId, juce::Colours::white);
    label.setJustificationType(juce::Justification::centred);
}

void FilterComponent::resized()
{
    auto area = getLocalBounds();
    auto half = area.getWidth() / 2;

    cutoffLabel.setBounds(0, 0, half, 20);
    cutoffSlider.setBounds(0, 20, half, getHeight() - 20);

    resonanceLabel.setBounds(half, 0, half, 20);
    resonanceSlider.setBounds(half, 20, half, getHeight() - 20);
}
