/**
 * @file EnvelopeVisualizer.h
 * @brief ADSR envelope visualization component.
 *
 * EnvelopeVisualizer renders a real-time graphical
 * representation of the synthesizer amplitude envelope.
 *
 * The component listens to:
 * - Attack
 * - Decay
 * - Sustain
 * - Release
 *
 * parameters from the AudioProcessorValueTreeState.
 *
 * Features:
 * - Real-time updates
 * - Animated ADSR curve
 * - Gradient envelope fill
 * - Sustain and peak markers
 * - Harmonia visual styling
 */
#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/HarmoniaPalette.h"
#include "../parameters/HarmoniaParameters.h"


/**
 * @class EnvelopeVisualizer
 * @brief Visualizes ADSR envelope parameters in real time.
 *
 * This component listens to APVTS parameter changes and redraws
 * the ADSR envelope whenever a parameter is modified.
 *
 * The displayed envelope includes:
 * - Attack ramp
 * - Decay slope
 * - Sustain plateau
 * - Release curve
 *
 * The component is purely visual and does not process audio.
 */
class EnvelopeVisualizer : public juce::Component,
                           private juce::AudioProcessorValueTreeState::Listener
{
public:
    EnvelopeVisualizer (juce::AudioProcessorValueTreeState& apvts,
                        juce::Colour tint = HarmoniaPalette::sectionAmpEnv)
        : state (apvts), accent (tint)
    {
        for (auto& id : { HarmoniaParams::IDs::ampAttack,
                          HarmoniaParams::IDs::ampDecay,
                          HarmoniaParams::IDs::ampSustain,
                          HarmoniaParams::IDs::ampRelease })
            state.addParameterListener (id, this);
    }

    ~EnvelopeVisualizer() override
    {
        for (auto& id : { HarmoniaParams::IDs::ampAttack,
                          HarmoniaParams::IDs::ampDecay,
                          HarmoniaParams::IDs::ampSustain,
                          HarmoniaParams::IDs::ampRelease })
            state.removeParameterListener (id, this);
    }

    void setAccentColour (juce::Colour c)
    {
        accent = c;
        repaint();
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
        for (int i = 1; i < 4; ++i)
        {
            const float y = r.getY() + r.getHeight() * (float) i / 4.0f;
            g.drawHorizontalLine ((int) y, r.getX() + 4.0f, r.getRight() - 4.0f);
        }

        const float a   = getNorm (HarmoniaParams::IDs::ampAttack);
        const float d   = getNorm (HarmoniaParams::IDs::ampDecay);
        const float s   = getRaw  (HarmoniaParams::IDs::ampSustain);
        const float rel = getNorm (HarmoniaParams::IDs::ampRelease);

        const float left  = r.getX() + 6.0f;
        const float right = r.getRight() - 6.0f;
        const float top   = r.getY() + 6.0f;
        const float bot   = r.getBottom() - 6.0f;
        const float w     = right - left;

        const float wA = w * (0.05f + a * 0.30f);
        const float wD = w * (0.05f + d * 0.25f);
        const float wS = w * 0.20f;
        const float wR = w * (0.10f + rel * 0.30f);

        const float total = wA + wD + wS + wR;
        const float scale = total > 0.0f ? w / total : 1.0f;

        const float xA = left;
        const float xB = xA + wA * scale;
        const float xC = xB + wD * scale;
        const float xD = xC + wS * scale;
        const float xE = xD + wR * scale;

        const float yPeak    = top;
        const float ySustain = bot - (bot - top) * s;
        const float yEnd     = bot;

        juce::Path env;
        env.startNewSubPath (xA, bot);
        env.lineTo (xB, yPeak);
        env.lineTo (xC, ySustain);
        env.lineTo (xD, ySustain);
        env.lineTo (xE, yEnd);

        juce::Path fill = env;
        fill.lineTo (xE, bot);
        fill.lineTo (xA, bot);
        fill.closeSubPath();
        juce::ColourGradient grad (accent.withAlpha (0.22f), 0, top,
                                   accent.withAlpha (0.0f),  0, bot, false);
        g.setGradientFill (grad);
        g.fillPath (fill);

        g.setColour (accent.withAlpha (0.30f));
        g.strokePath (env, juce::PathStrokeType (4.0f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));
        g.setColour (accent);
        g.strokePath (env, juce::PathStrokeType (1.5f, juce::PathStrokeType::curved,
                                                 juce::PathStrokeType::rounded));

        auto dot = [&] (juce::Point<float> p)
        {
            g.setColour (HarmoniaPalette::screenBg);
            g.fillEllipse (p.x - 3.0f, p.y - 3.0f, 6.0f, 6.0f);
            g.setColour (accent);
            g.drawEllipse (p.x - 3.0f, p.y - 3.0f, 6.0f, 6.0f, 1.3f);
        };
        dot ({ xB, yPeak });
        dot ({ xC, ySustain });
        dot ({ xD, ySustain });
    }

private:
    void parameterChanged (const juce::String&, float) override
    {
        juce::MessageManager::callAsync ([this] { repaint(); });
    }

    float getNorm (const juce::String& id) const
    {
        if (auto* p = state.getParameter (id)) return p->getValue();
        return 0.0f;
    }

    float getRaw (const juce::String& id) const
    {
        if (auto* v = state.getRawParameterValue (id)) return v->load();
        return 0.0f;
    }

    juce::AudioProcessorValueTreeState& state;
    juce::Colour accent;
};
