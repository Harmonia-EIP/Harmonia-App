#include "FilterComponent.h"
#include "../themes/AppColourIds.h"

FilterComponent::FilterComponent (AppLookAndFeel& lnF)
    : lookAndFeel (lnF)
{
    setupSlider (cutoffSlider, cutoffLabel, "Cutoff", 20.0f, 20000.0f, 1000.0f);
    cutoffSlider.setSkewFactorFromMidPoint (1000.0);

    setupSlider (resonanceSlider, resonanceLabel, "Resonance", 0.1f, 10.0f, 1.0f);

    addAndMakeVisible (cutoffSlider);
    addAndMakeVisible (cutoffLabel);
    addAndMakeVisible (resonanceSlider);
    addAndMakeVisible (resonanceLabel);

    cutoffSlider.onValueChange = [this]()
    {
        if (onParamsChanged) onParamsChanged();
    };
    resonanceSlider.onValueChange = [this]()
    {
        if (onParamsChanged) onParamsChanged();
    };
}

FilterComponent::~FilterComponent()
{
    cutoffSlider.setLookAndFeel (nullptr);
    resonanceSlider.setLookAndFeel (nullptr);
}

//==============================================================================
std::pair<double, double> FilterComponent::getSlidersInfo() const
{
    return { cutoffSlider.getValue(), resonanceSlider.getValue() };
}

double FilterComponent::getCutoff() const
{
    return cutoffSlider.getValue();
}

double FilterComponent::getResonance() const
{
    return resonanceSlider.getValue();
}

void FilterComponent::setCutoffResonance (double cutoff, double resonance)
{
    cutoffSlider.setValue (cutoff, juce::dontSendNotification);
    resonanceSlider.setValue (resonance, juce::dontSendNotification);
}

void FilterComponent::setCutoff (double cutoff)
{
    cutoffSlider.setValue (cutoff, juce::dontSendNotification);
}

void FilterComponent::setResonance (double resonance)
{
    resonanceSlider.setValue (resonance, juce::dontSendNotification);
}

//==============================================================================
void FilterComponent::setupSlider (juce::Slider& slider,
                                  juce::Label& label,
                                  const juce::String& name,
                                  float min, float max, float def)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);

    // 🔹 TextBox plus large pour mieux afficher les valeurs
    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 90, 24);

    slider.setRange (min, max);
    slider.setValue (def, juce::dontSendNotification);
    slider.setLookAndFeel (&lookAndFeel);

    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
}

//==============================================================================
void FilterComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (AppColourIds::panelBgId));

    g.setColour (findColour (AppColourIds::panelOutlineId));
    g.drawRect (getLocalBounds(), 1);
}

//==============================================================================
void FilterComponent::resized()
{
    auto area = getLocalBounds();
    auto half = area.getWidth() / 2;

    const int labelHeight     = 22; // un peu plus haut
    const int sliderHeight    = area.getHeight() - labelHeight - 8; // marge 8px en bas
    const int verticalSpacing = 1;  // espace entre label et slider

    // Cutoff
    cutoffLabel.setBounds (0, 0, half, labelHeight);
    cutoffSlider.setBounds (0, labelHeight + verticalSpacing, half, sliderHeight - verticalSpacing);

    // Resonance
    resonanceLabel.setBounds (half, 0, half, labelHeight);
    resonanceSlider.setBounds (half, labelHeight + verticalSpacing, half, sliderHeight - verticalSpacing);
}

void FilterComponent::lookAndFeelChanged()
{
    auto text = findColour (AppColourIds::textPrimaryId);

    cutoffLabel.setColour    (juce::Label::textColourId, text);
    resonanceLabel.setColour (juce::Label::textColourId, text);
}
