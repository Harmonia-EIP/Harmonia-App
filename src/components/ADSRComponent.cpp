#include "ADSRComponent.h"

ADSRComponent::ADSRComponent(CustomLookAndFeel& lnF) : lookAndFeel(lnF)
{
    setupSlider(attackSlider, attackLabel, "Attack", 0.01f, 5.0f, 0.1f);
    setupSlider(decaySlider, decayLabel, "Decay", 0.01f, 5.0f, 0.1f);
    setupSlider(sustainSlider, sustainLabel, "Sustain", 0.0f, 1.0f, 0.8f);
    setupSlider(releaseSlider, releaseLabel, "Release", 0.01f, 5.0f, 0.5f);

    addAndMakeVisible(attackSlider); addAndMakeVisible(attackLabel);
    addAndMakeVisible(decaySlider); addAndMakeVisible(decayLabel);
    addAndMakeVisible(sustainSlider); addAndMakeVisible(sustainLabel);
    addAndMakeVisible(releaseSlider); addAndMakeVisible(releaseLabel);
}

ADSRComponent::~ADSRComponent()
{
    attackSlider.setLookAndFeel(nullptr);
    decaySlider.setLookAndFeel(nullptr);
    sustainSlider.setLookAndFeel(nullptr);
    releaseSlider.setLookAndFeel(nullptr);
}

void ADSRComponent::setAttack(double attack)
{
    attackSlider.setValue(attack, juce::NotificationType::dontSendNotification);
}

void ADSRComponent::setDecay(double decay)
{
    decaySlider.setValue(decay, juce::NotificationType::dontSendNotification);
}

void ADSRComponent::setSustain(double sustain)
{
    sustainSlider.setValue(sustain, juce::NotificationType::dontSendNotification);
}
    
void ADSRComponent::setRelease(double release)
{
    releaseSlider.setValue(release, juce::NotificationType::dontSendNotification);
}

std::vector<double> ADSRComponent::getSlidersInfo() const
{
    std::vector<double> adsrValues = {
        attackSlider.getValue(),
        decaySlider.getValue(),
        sustainSlider.getValue(),
        releaseSlider.getValue()
    };
    return adsrValues;
}

void ADSRComponent::setupSlider(juce::Slider& slider, juce::Label& label, const juce::String& name,
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

void ADSRComponent::resized()
{
    auto area = getLocalBounds();
    auto w = area.getWidth() / 4;

    attackLabel.setBounds(0, 0, w, 20);
    attackSlider.setBounds(0, 20, w, getHeight() - 20);

    decayLabel.setBounds(w, 0, w, 20);
    decaySlider.setBounds(w, 20, w, getHeight() - 20);

    sustainLabel.setBounds(2 * w, 0, w, 20);
    sustainSlider.setBounds(2 * w, 20, w, getHeight() - 20);

    releaseLabel.setBounds(3 * w, 0, w, 20);
    releaseSlider.setBounds(3 * w, 20, w, getHeight() - 20);
}
