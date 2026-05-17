#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/HarmoniaPalette.h"

class KnobControl : public juce::Component
{
public:
    KnobControl (juce::AudioProcessorValueTreeState& apvts,
                 const juce::String& paramId,
                 const juce::String& displayName,
                 bool bipolar = false)
        : caption (displayName)
    {
        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::NoTextBox, false, 0, 0);
        slider.setRotaryParameters (juce::MathConstants<float>::pi * 1.25f,
                                    juce::MathConstants<float>::pi * 2.75f,
                                    true);
        slider.getProperties().set ("bipolar", bipolar);
        slider.setVelocityModeParameters (1.0, 1, 0.07, false);
        addAndMakeVisible (slider);

        attachment = std::make_unique<juce::AudioProcessorValueTreeState::SliderAttachment>(
            apvts, paramId, slider);

        slider.onValueChange = [this]
        {
            const auto t = juce::Time::getMillisecondCounter();
            const auto v = (float) slider.getValue();
            if (lastTickMs > 0)
            {
                const float dv = std::abs (v - lastValue);
                const auto  dt = (int) (t - lastTickMs);
                if (dt > 0 && dv / (float) std::max (1, dt) > fastSpeedThreshold)
                    if (onFastTweak) onFastTweak();
            }
            lastTickMs = t;
            lastValue  = v;
            repaint();
        };
    }

    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds();

        g.setColour (HarmoniaPalette::textMuted);
        g.setFont (juce::Font (juce::FontOptions (9.5f).withStyle ("Bold"))
                       .withExtraKerningFactor (0.14f));
        g.drawText (caption.toUpperCase(),
                    r.withHeight (captionH),
                    juce::Justification::centred);

        g.setColour (HarmoniaPalette::accent);
        g.setFont (juce::Font (juce::FontOptions (10.0f)));
        g.drawText (slider.getTextFromValue (slider.getValue()),
                    r.withTop (r.getBottom() - readoutH),
                    juce::Justification::centred);
    }

    void resized() override
    {
        auto r = getLocalBounds();
        r.removeFromTop (captionH);
        r.removeFromBottom (readoutH);
        slider.setBounds (r.reduced (2));
    }

    std::function<void()> onFastTweak;

private:
    juce::Slider slider;
    juce::String caption;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    juce::uint32 lastTickMs = 0;
    float        lastValue  = 0.0f;

    static constexpr int captionH = 14;
    static constexpr int readoutH = 14;
    static constexpr float fastSpeedThreshold = 0.0025f;
};
