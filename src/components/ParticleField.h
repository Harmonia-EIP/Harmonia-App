#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "../themes/HarmoniaPalette.h"
#include <random>

class ParticleField : public juce::Component, private juce::Timer
{
public:
    ParticleField()
    {
        setInterceptsMouseClicks (false, false);

        ambient.resize (16);
        std::uniform_real_distribution<float> ux (0.0f, 1.0f);
        std::uniform_real_distribution<float> uy (0.0f, 1.0f);
        std::uniform_real_distribution<float> us (0.04f, 0.12f);
        std::uniform_real_distribution<float> ur (0.8f, 2.2f);
        for (auto& p : ambient)
        {
            p.x = ux (rng);
            p.y = uy (rng);
            p.speed = us (rng);
            p.radius = ur (rng);
            p.alpha = 0.06f + ux (rng) * 0.06f;
        }

        startTimerHz (45);
    }

    void emitBurst (juce::Point<float> origin, int count = 5, juce::Colour col = HarmoniaPalette::accent)
    {
        std::uniform_real_distribution<float> ang (0.0f, juce::MathConstants<float>::twoPi);
        std::uniform_real_distribution<float> spd (0.4f, 1.6f);
        std::uniform_real_distribution<float> lf  (0.4f, 0.9f);
        for (int i = 0; i < count; ++i)
        {
            Burst b;
            const float a = ang (rng);
            const float s = spd (rng);
            b.x  = origin.x;
            b.y  = origin.y;
            b.vx = std::cos (a) * s;
            b.vy = std::sin (a) * s - 0.4f;
            b.life = lf (rng);
            b.lifeMax = b.life;
            b.colour = col;
            bursts.push_back (b);
        }
    }

    void paint (juce::Graphics& g) override
    {
        const float w = (float) getWidth();
        const float h = (float) getHeight();
        if (w <= 0 || h <= 0) return;

        for (auto const& p : ambient)
        {
            g.setColour (HarmoniaPalette::accent.withAlpha (p.alpha));
            g.fillEllipse (p.x * w - p.radius, p.y * h - p.radius,
                           p.radius * 2.0f, p.radius * 2.0f);
        }

        for (auto const& b : bursts)
        {
            const float a = juce::jlimit (0.0f, 1.0f, b.life / b.lifeMax);
            const float r = 1.5f + (1.0f - a) * 2.0f;
            g.setColour (b.colour.withAlpha (a * 0.85f));
            g.fillEllipse (b.x - r, b.y - r, r * 2.0f, r * 2.0f);
        }
    }

private:
    void timerCallback() override
    {
        for (auto& p : ambient)
        {
            p.y -= p.speed * 0.005f;
            if (p.y < -0.05f)
            {
                std::uniform_real_distribution<float> ux (0.0f, 1.0f);
                p.y = 1.05f;
                p.x = ux (rng);
            }
        }

        for (auto& b : bursts)
        {
            b.x += b.vx * 1.2f;
            b.y += b.vy * 1.2f;
            b.vy += 0.04f;
            b.life -= 0.025f;
        }
        bursts.erase (std::remove_if (bursts.begin(), bursts.end(),
            [] (const Burst& b) { return b.life <= 0.0f; }), bursts.end());

        repaint();
    }

    struct Ambient { float x, y, speed, radius, alpha; };
    struct Burst   { float x, y, vx, vy, life, lifeMax; juce::Colour colour; };

    std::vector<Ambient> ambient;
    std::vector<Burst>   bursts;
    std::mt19937 rng { 0xC0FFEE };
};
