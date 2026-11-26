#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <functional>

#include "LevelMeter.h"
#include "SectionPanel.h"
#include "../parameters/HarmoniaParameters.h"
#include "../themes/HarmoniaPalette.h"

/**
 * @class OutputStrip
 * @brief Bandeau de sortie compact (même cadre et même ombre que les SectionPanel) :
 *        titre et valeur empilés à gauche, slider + vumètre sur toute la largeur restante.
 *
 *   ( ● OUTPUT    [=========o--------------] )
 *   (   -6.0 dB   [▮▮▮▮▮▮▮▮▮▮▮             ] )
 *
 * Le volume est hors charte (HarmoniaParams::Output) : ni IA, ni presets, ni aléatoire.
 * Double-clic sur le slider : retour à 0 dB.
 */
class OutputStrip : public juce::Component
{
public:
    OutputStrip (juce::AudioProcessorValueTreeState& apvts, std::function<float()> peakSource)
        : meter (std::move (peakSource), true)
    {
        namespace Out = HarmoniaParams::Output;

        slider.setSliderStyle (juce::Slider::LinearHorizontal);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setTooltip ("Output volume (double-click: 0 dB)");
        addAndMakeVisible (slider);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment> (apvts, Out::volumeId, slider);
        slider.setDoubleClickReturnValue (true, Out::defaultDb);

        param = apvts.getParameter (Out::volumeId);

        valueLabel.setJustificationType (juce::Justification::centredLeft);
        valueLabel.setBorderSize ({});
        valueLabel.setFont (juce::Font (juce::FontOptions (10.5f).withStyle ("Bold")));
        valueLabel.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (valueLabel);

        addAndMakeVisible (meter);

        slider.onValueChange = [this] { updateValueText(); };
        updateValueText();
    }

    juce::Slider& getSlider() noexcept   { return slider; }
    LevelMeter&   getMeter() noexcept    { return meter; }
    juce::String  getValueText() const   { return valueLabel.getText(); }

    // même marge d'ombre que SectionPanel : cadres alignés au pixel dans la colonne
    static constexpr int margin = SectionPanel::shadowMargin;
    static constexpr int preferredHeight = 42 + 2 * margin;

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds().toFloat().reduced ((float) margin);
        constexpr float corner = 10.0f;

        {
            juce::Path shadowPath;
            shadowPath.addRoundedRectangle (r.translated (0.0f, 1.5f), corner);
            juce::DropShadow (juce::Colours::black.withAlpha (0.30f), 5, { 0, 2 }).drawForPath (g, shadowPath);
        }

        g.setGradientFill (juce::ColourGradient (HarmoniaPalette::panelTop, 0.0f, r.getY(),
                                                 HarmoniaPalette::panel,    0.0f, r.getBottom(), false));
        g.fillRoundedRectangle (r, corner);

        g.setColour (juce::Colours::white.withAlpha (0.06f));
        g.drawLine (r.getX() + corner, r.getY() + 1.0f, r.getRight() - corner, r.getY() + 1.0f, 1.0f);

        g.setColour (HarmoniaPalette::border);
        g.drawRoundedRectangle (r, corner, 1.0f);

        // LED + titre, comme les SectionPanel
        const auto tint = HarmoniaPalette::accent;
        const auto titleRow = titleArea();
        const float ledR = 3.0f, ledX = 16.0f, ledY = (float) titleRow.getCentreY();

        g.setColour (tint.withAlpha (0.40f));
        g.fillEllipse (ledX - ledR * 1.6f, ledY - ledR * 1.6f, ledR * 3.2f, ledR * 3.2f);
        g.setColour (tint);
        g.fillEllipse (ledX - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f);

        g.setFont (juce::Font (juce::FontOptions (10.0f).withStyle ("Bold")).withExtraKerningFactor (0.20f));
        g.drawText ("OUTPUT", titleRow.toFloat(), juce::Justification::centredLeft, false);
    }

    void resized() override
    {
        valueLabel.setBounds (titleArea().translated (0, rowH));

        auto r = getLocalBounds().reduced (margin).reduced (8, 7).withTrimmedLeft (10 + titleW);

        auto meterRow = r.removeFromBottom (5);
        r.removeFromBottom (3);

        slider.setBounds (r);
        meter.setBounds (meterRow.reduced (6, 0));   // aligné sur la course du slider
    }

    void lookAndFeelChanged() override
    {
        valueLabel.setColour (juce::Label::textColourId, HarmoniaPalette::textPrimary);
    }

private:
    void updateValueText()
    {
        valueLabel.setColour (juce::Label::textColourId, HarmoniaPalette::textPrimary);

        if (param != nullptr)
            valueLabel.setText (param->getCurrentValueAsText(), juce::dontSendNotification);
    }

    static constexpr int titleW = 56, rowH = 14;

    /** Ligne du titre (la valeur en dB est juste en dessous). */
    juce::Rectangle<int> titleArea() const
    {
        const auto frame = getLocalBounds().reduced (margin);
        return { 28, frame.getCentreY() - rowH, titleW, rowH };
    }

    juce::Slider slider;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;
    juce::RangedAudioParameter* param = nullptr;
    juce::Label valueLabel;
    LevelMeter meter;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (OutputStrip)
};
