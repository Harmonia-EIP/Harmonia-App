#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/HarmoniaPalette.h"
#include "../parameters/HarmoniaParameters.h"

class LfoVisualizer : public juce::Component, private juce::Timer
{
public:
    LfoVisualizer (juce::AudioProcessorValueTreeState& apvts,
                   juce::Colour tint = HarmoniaPalette::mint)
        : state (apvts), accent (tint)
    {
        startTimerHz (45);
    }

    void paint (juce::Graphics& g) override
    {
        auto r = getLocalBounds().toFloat().reduced (1.0f);
        const float corner = 5.0f;

        g.setColour (HarmoniaPalette::screenBg);
        g.fillRoundedRectangle (r, corner);
        g.setColour (HarmoniaPalette::border);
        g.drawRoundedRectangle (r, corner, 1.0f);

        g.setColour (HarmoniaPalette::screenGrid);
        g.drawHorizontalLine ((int) r.getCentreY(), r.getX() + 4.0f, r.getRight() - 4.0f);

        const float left  = r.getX() + 6.0f;
        const float right = r.getRight() - 6.0f;
        const float midY  = r.getCentreY();
        const float halfH = (r.getHeight() - 12.0f) * 0.42f;
        const float w     = right - left;

        const float rateHz = juce::jlimit (0.1f, 20.0f, getRaw (HarmoniaParams::IDs::lfoRate));
        const float cycles = juce::jlimit (0.5f, 6.0f, rateHz * 0.4f);

        const int   steps = 200;
        juce::Path  wave;
        for (int i = 0; i <= steps; ++i)
        {
            const float t  = (float) i / (float) steps;
            const float ph = (t * cycles + scrollPhase) * juce::MathConstants<float>::twoPi;
            const float y  = midY - std::sin (ph) * halfH;
            const float x  = left + w * t;
            if (i == 0) wave.startNewSubPath (x, y);
            else        wave.lineTo (x, y);
        }

        g.setColour (accent.withAlpha (0.28f));
        g.strokePath (wave, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved));
        g.setColour (accent);
        g.strokePath (wave, juce::PathStrokeType (1.4f, juce::PathStrokeType::curved));

        const float phNow = scrollPhase * juce::MathConstants<float>::twoPi;
        const float yNow  = midY - std::sin (phNow) * halfH;
        g.setColour (accent.withAlpha (0.55f));
        g.fillEllipse (left - 5.0f, yNow - 5.0f, 10.0f, 10.0f);
        g.setColour (HarmoniaPalette::textPrimary);
        g.fillEllipse (left - 2.5f, yNow - 2.5f, 5.0f, 5.0f);
    }

private:
    void timerCallback() override
    {
        const float rate = juce::jlimit (0.1f, 20.0f, getRaw (HarmoniaParams::IDs::lfoRate));
        scrollPhase += (rate / 45.0f) * 0.18f;
        if (scrollPhase > 1.0f) scrollPhase -= 1.0f;
        repaint();
    }

    float getRaw (const juce::String& id) const
    {
        if (auto* v = state.getRawParameterValue (id)) return v->load();
        return 0.0f;
    }

    juce::AudioProcessorValueTreeState& state;
    juce::Colour accent;
    float scrollPhase = 0.0f;
};
