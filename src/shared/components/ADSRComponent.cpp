#include "ADSRComponent.h"

ADSRComponent::ADSRComponent (AppLookAndFeel& lnF)
    : lookAndFeel (lnF)
{
    setupSlider (attackSlider,  attackLabel,  Strings::Parameters::Attack,  AudioConfig::ADSR::Attack::Min, AudioConfig::ADSR::Attack::Max, AudioConfig::ADSR::Attack::Default);
    setupSlider (decaySlider,   decayLabel,   Strings::Parameters::Decay,   AudioConfig::ADSR::Decay::Min, AudioConfig::ADSR::Decay::Max, AudioConfig::ADSR::Decay::Default);
    setupSlider (sustainSlider, sustainLabel, Strings::Parameters::Sustain, AudioConfig::ADSR::Sustain::Min, AudioConfig::ADSR::Sustain::Max, AudioConfig::ADSR::Sustain::Default);
    setupSlider (releaseSlider, releaseLabel, Strings::Parameters::Release, AudioConfig::ADSR::Release::Min, AudioConfig::ADSR::Release::Max, AudioConfig::ADSR::Release::Default);

    addAndMakeVisible (attackSlider);  addAndMakeVisible (attackLabel);
    addAndMakeVisible (decaySlider);   addAndMakeVisible (decayLabel);
    addAndMakeVisible (sustainSlider); addAndMakeVisible (sustainLabel);
    addAndMakeVisible (releaseSlider); addAndMakeVisible (releaseLabel);

    attackSlider.onValueChange = [this]()
    {
        if (onParamsChanged) onParamsChanged();
    };
    decaySlider.onValueChange = [this]()
    {
        if (onParamsChanged) onParamsChanged();
    };
    sustainSlider.onValueChange = [this]()
    {
        if (onParamsChanged) onParamsChanged();
    };
    releaseSlider.onValueChange = [this]()
    {
        if (onParamsChanged) onParamsChanged();
    };
}

ADSRComponent::~ADSRComponent()
{
    attackSlider.setLookAndFeel (nullptr);
    decaySlider.setLookAndFeel (nullptr);
    sustainSlider.setLookAndFeel (nullptr);
    releaseSlider.setLookAndFeel (nullptr);
}

//==============================================================================
void ADSRComponent::setADSR (double attack, double decay, double sustain, double release)
{
    attackSlider.setValue  (attack,  juce::dontSendNotification);
    decaySlider.setValue   (decay,   juce::dontSendNotification);
    sustainSlider.setValue (sustain, juce::dontSendNotification);
    releaseSlider.setValue (release, juce::dontSendNotification);
}

void ADSRComponent::setAttack (double attack)
{
    attackSlider.setValue (attack, juce::dontSendNotification);
}

void ADSRComponent::setDecay (double decay)
{
    decaySlider.setValue (decay, juce::dontSendNotification);
}

void ADSRComponent::setSustain (double sustain)
{
    sustainSlider.setValue (sustain, juce::dontSendNotification);
}

void ADSRComponent::setRelease (double release)
{
    releaseSlider.setValue (release, juce::dontSendNotification);
}

//==============================================================================
std::vector<double> ADSRComponent::getSlidersInfo() const
{
    return {
        attackSlider.getValue(),
        decaySlider.getValue(),
        sustainSlider.getValue(),
        releaseSlider.getValue()
    };
}

double ADSRComponent::getAttack() const  { return attackSlider.getValue(); }
double ADSRComponent::getDecay() const   { return decaySlider.getValue(); }
double ADSRComponent::getSustain() const { return sustainSlider.getValue(); }
double ADSRComponent::getRelease() const { return releaseSlider.getValue(); }

//==============================================================================
void ADSRComponent::setupSlider (juce::Slider& slider,
                                juce::Label& label,
                                const juce::String& name,
                                float min, float max, float def)
{
    slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);

    slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 70, 24); 
    slider.setRange (min, max);
    slider.setValue (def, juce::dontSendNotification);
    slider.setLookAndFeel (&lookAndFeel);

    label.setText (name, juce::dontSendNotification);
    label.setJustificationType (juce::Justification::centred);
}

void ADSRComponent::paint (juce::Graphics& g)
{
    g.fillAll (findColour (AppColourIds::panelBgId));

    g.setColour (findColour (AppColourIds::panelOutlineId));
    g.drawRect (getLocalBounds(), 1);
}

void ADSRComponent::resized()
{
    auto area = getLocalBounds();
    auto w = area.getWidth() / 4;

    const int labelHeight     = 22;
    const int sliderHeight    = area.getHeight() - labelHeight - 8;
    const int verticalSpacing = 1;

    attackLabel.setBounds  (0, 0, w, labelHeight);
    attackSlider.setBounds (0, labelHeight + verticalSpacing, w, sliderHeight - verticalSpacing);

    decayLabel.setBounds   (w, 0, w, labelHeight);
    decaySlider.setBounds  (w, labelHeight + verticalSpacing, w, sliderHeight - verticalSpacing);

    sustainLabel.setBounds (2 * w, 0, w, labelHeight);
    sustainSlider.setBounds(2 * w, labelHeight + verticalSpacing, w, sliderHeight - verticalSpacing);

    releaseLabel.setBounds (3 * w, 0, w, labelHeight);
    releaseSlider.setBounds(3 * w, labelHeight + verticalSpacing, w, sliderHeight - verticalSpacing);
}

void ADSRComponent::lookAndFeelChanged()
{
    auto text = findColour (AppColourIds::textPrimaryId);

    attackLabel.setColour  (juce::Label::textColourId, text);
    decayLabel.setColour   (juce::Label::textColourId, text);
    sustainLabel.setColour (juce::Label::textColourId, text);
    releaseLabel.setColour (juce::Label::textColourId, text);
}
