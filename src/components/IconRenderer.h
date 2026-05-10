#pragma once

#include <juce_graphics/juce_graphics.h>

namespace IconRenderer
{
    enum class Wave { Sine = 0, Triangle = 1, Saw = 2, Square = 3 };
    enum class FilterShape { Lowpass = 0, Bandpass = 1, Highpass = 2 };

    inline void drawWave (juce::Graphics& g, juce::Rectangle<float> area,
                          Wave w, juce::Colour col, float strokeW = 1.6f)
    {
        const float left  = area.getX();
        const float right = area.getRight();
        const float midY  = area.getCentreY();
        const float halfH = area.getHeight() * 0.40f;
        const float w_    = right - left;

        juce::Path p;
        switch (w)
        {
            case Wave::Sine:
            {
                const int steps = 48;
                for (int i = 0; i <= steps; ++i)
                {
                    const float t = (float) i / (float) steps;
                    const float x = left + w_ * t;
                    const float y = midY - std::sin (t * juce::MathConstants<float>::twoPi) * halfH;
                    if (i == 0) p.startNewSubPath (x, y);
                    else        p.lineTo (x, y);
                }
                break;
            }

            case Wave::Triangle:
            {
                p.startNewSubPath (left,                  midY);
                p.lineTo          (left + w_ * 0.25f,     midY - halfH);
                p.lineTo          (left + w_ * 0.75f,     midY + halfH);
                p.lineTo          (right,                 midY);
                break;
            }

            case Wave::Saw:
            {
                p.startNewSubPath (left,              midY + halfH);
                p.lineTo          (left + w_ * 0.5f,  midY - halfH);
                p.lineTo          (left + w_ * 0.5f,  midY + halfH);
                p.lineTo          (right,             midY - halfH);
                break;
            }

            case Wave::Square:
            {
                p.startNewSubPath (left,                 midY + halfH);
                p.lineTo          (left,                 midY - halfH);
                p.lineTo          (left + w_ * 0.5f,     midY - halfH);
                p.lineTo          (left + w_ * 0.5f,     midY + halfH);
                p.lineTo          (right,                midY + halfH);
                p.lineTo          (right,                midY - halfH);
                break;
            }
        }

        g.setColour (col);
        g.strokePath (p, juce::PathStrokeType (strokeW, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
    }

    inline void drawFilterShape (juce::Graphics& g, juce::Rectangle<float> area,
                                 FilterShape s, juce::Colour col, float strokeW = 1.6f)
    {
        const float left  = area.getX();
        const float right = area.getRight();
        const float top   = area.getY();
        const float bot   = area.getBottom();
        const float midY  = area.getCentreY();
        const float w_    = right - left;
        const float h_    = bot - top;

        juce::Path p;
        switch (s)
        {
            case FilterShape::Lowpass:
            {
                p.startNewSubPath (left,                top + h_ * 0.20f);
                p.lineTo          (left + w_ * 0.55f,   top + h_ * 0.25f);
                p.lineTo          (left + w_ * 0.75f,   top + h_ * 0.40f);
                p.lineTo          (right,               bot - h_ * 0.10f);
                break;
            }

            case FilterShape::Bandpass:
            {
                p.startNewSubPath (left,                bot - h_ * 0.10f);
                p.lineTo          (left + w_ * 0.30f,   midY);
                p.lineTo          (left + w_ * 0.50f,   top + h_ * 0.15f);
                p.lineTo          (left + w_ * 0.70f,   midY);
                p.lineTo          (right,               bot - h_ * 0.10f);
                break;
            }

            case FilterShape::Highpass:
            {
                p.startNewSubPath (left,                bot - h_ * 0.10f);
                p.lineTo          (left + w_ * 0.25f,   top + h_ * 0.40f);
                p.lineTo          (left + w_ * 0.45f,   top + h_ * 0.25f);
                p.lineTo          (right,               top + h_ * 0.20f);
                break;
            }
        }

        g.setColour (col);
        g.strokePath (p, juce::PathStrokeType (strokeW, juce::PathStrokeType::curved,
                                                juce::PathStrokeType::rounded));
    }
}
