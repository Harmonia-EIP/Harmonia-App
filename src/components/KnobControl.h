/**
 * @file KnobControl.h
 * @brief Rotary knob UI component linked to an APVTS parameter.
 *
 * Features:
 * - Rotary interaction
 * - Optional bipolar mode
 * - Value readout display
 * - Fast tweak detection
 * - APVTS synchronization
 * - Lock state (Refine mode): padlock next to the caption, click the caption to toggle
 */
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/HarmoniaPalette.h"
#include "LockableControl.h"

class KnobControl : public juce::Component,
                    public LockableControl
{
public:
    KnobControl (juce::AudioProcessorValueTreeState& apvts,
                 const juce::String& paramId,
                 const juce::String& displayName,
                 bool bipolar = false)
        : caption (displayName),
          parameterId (paramId)
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

    //==========================================================================
    // LockableControl

    void setLockUiVisible (bool visible) override
    {
        lockUiVisible = visible;

        if (! visible)
            setMouseCursor (juce::MouseCursor::NormalCursor);

        refreshLockVisual();
    }

    void setLocked (bool shouldLock) override
    {
        if (locked == shouldLock)
            return;

        locked = shouldLock;
        refreshLockVisual();

        if (onLockChanged)
            onLockChanged();
    }

    bool isLocked() const override              { return locked; }
    juce::String getParamId() const override    { return parameterId; }

    //==========================================================================
    void paint (juce::Graphics& g) override
    {
        const auto r = getLocalBounds();

        // Rendu "verrouille" seulement si l'UI de lock est visible (mode Refine)
        const bool lit = lockUiVisible && locked;

        const auto font = juce::Font (juce::FontOptions (9.5f).withStyle ("Bold"))
                              .withExtraKerningFactor (0.14f);
        const auto text    = caption.toUpperCase();
        const auto capArea = r.withHeight (captionH).toFloat();

        if (lockUiVisible)
        {
            juce::GlyphArrangement ga;
            ga.addLineOfText (font, text, 0.0f, 0.0f);
            const float textW = ga.getBoundingBox (0, -1, true).getWidth();

            const float iconW = 8.0f, iconH = 10.0f, gap = 4.0f;
            const float x0 = capArea.getCentreX() - (iconW + gap + textW) * 0.5f;

            LockableControl::drawPadlock (g,
                { x0, capArea.getCentreY() - iconH * 0.5f, iconW, iconH },
                locked,
                lit ? HarmoniaPalette::locked : HarmoniaPalette::textMuted.withAlpha (0.55f));

            g.setColour (lit ? HarmoniaPalette::locked : HarmoniaPalette::textMuted);
            g.setFont (font);
            g.drawText (text,
                        juce::Rectangle<float> (x0 + iconW + gap, capArea.getY(),
                                                textW + 4.0f, capArea.getHeight()),
                        juce::Justification::centredLeft, false);
        }
        else
        {
            g.setColour (HarmoniaPalette::textMuted);
            g.setFont (font);
            g.drawText (text, capArea, juce::Justification::centred);
        }

        g.setColour (lit ? HarmoniaPalette::locked : HarmoniaPalette::accent);
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

    // Le slider couvre le centre : seuls les clics sur la zone du label arrivent ici
    void mouseDown (const juce::MouseEvent& e) override
    {
        if (lockUiVisible && e.y < captionH)
            setLocked (! locked);
    }

    void mouseMove (const juce::MouseEvent& e) override
    {
        setMouseCursor (lockUiVisible && e.y < captionH
                            ? juce::MouseCursor::PointingHandCursor
                            : juce::MouseCursor::NormalCursor);
    }

    std::function<void()> onFastTweak;

private:
    void refreshLockVisual()
    {
        // lu par HiveLookAndFeel::drawRotarySlider
        slider.getProperties().set ("locked", lockUiVisible && locked);
        slider.repaint();
        repaint();
    }

    juce::Slider slider;
    juce::String caption;
    const juce::String parameterId;
    std::unique_ptr<juce::AudioProcessorValueTreeState::SliderAttachment> attachment;

    bool locked        = false;
    bool lockUiVisible = false;

    juce::uint32 lastTickMs = 0;
    float        lastValue  = 0.0f;

    static constexpr int captionH = 14;
    static constexpr int readoutH = 14;
    static constexpr float fastSpeedThreshold = 0.0025f;
};