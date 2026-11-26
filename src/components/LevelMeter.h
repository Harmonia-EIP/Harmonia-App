#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <functional>

#include "../themes/HarmoniaPalette.h"

/**
 * @class LevelMeter
 * @brief Vumètre vertical du niveau de sortie (crête, en dB).
 *
 * Lit la crête via `source` (HarmoniaAudioProcessor::takeOutputPeak) à 30 Hz, sans
 * verrou. Retombée douce, maintien de crête 1 s. Couleurs : accent jusqu'à -6 dB,
 * ambre jusqu'à -1 dB, rouge au-delà (le limiteur travaille).
 */
class LevelMeter : public juce::Component,
                   public juce::SettableTooltipClient,
                   private juce::Timer
{
public:
    static constexpr float floorDb = -48.0f;

    explicit LevelMeter (std::function<float()> peakSource, bool isHorizontal = false)
        : source (std::move (peakSource)), horizontal (isHorizontal)
    {
        setTooltip ("Output level (red: the limiter is catching peaks)");
        startTimerHz (30);
    }

    ~LevelMeter() override { stopTimer(); }

    /** Niveau affiché, 0..1 linéaire (pour les tests). */
    float getDisplayedLevel() const noexcept { return level; }

    void paint (juce::Graphics& g) override
    {
        const auto b = getLocalBounds().toFloat();

        g.setColour (HarmoniaPalette::screenBg);
        g.fillRoundedRectangle (b, juce::jmin (3.0f, b.getHeight() * 0.5f));

        // graduations -6, -12, -24 dB
        g.setColour (HarmoniaPalette::screenGrid);
        for (const float db : { -6.0f, -12.0f, -24.0f })
        {
            const float p = posFor (db, b);
            if (horizontal) g.fillRect (p, b.getY(), 1.0f, b.getHeight());
            else            g.fillRect (b.getX(), p, b.getWidth(), 1.0f);
        }

        if (level > 0.0f)
        {
            const float db = juce::Decibels::gainToDecibels (level, floorDb);
            const float p  = posFor (db, b);

            g.setColour (colourFor (db));
            const auto bar = horizontal ? b.withRight (p).reduced (1.0f)
                                        : b.withTop (p).reduced (2.0f, 0.0f).withTrimmedBottom (2.0f);
            g.fillRoundedRectangle (bar, 2.0f);
        }

        if (hold > 0.0f)
        {
            const float holdDb = juce::Decibels::gainToDecibels (hold, floorDb);
            const float p = posFor (holdDb, b);
            g.setColour (colourFor (holdDb).brighter (0.3f));
            if (horizontal) g.fillRect (p - 2.0f, b.getY() + 1.0f, 2.0f, b.getHeight() - 2.0f);
            else            g.fillRect (b.getX() + 2.0f, p, b.getWidth() - 4.0f, 2.0f);
        }

        g.setColour (HarmoniaPalette::border);
        g.drawRoundedRectangle (b.reduced (0.5f), juce::jmin (3.0f, b.getHeight() * 0.5f), 1.0f);
    }

private:
    void timerCallback() override
    {
        const float peak = source ? source() : 0.0f;

        level = juce::jmax (peak, level * 0.82f);   // retombée ~ 25 dB/s
        if (level < 1.0e-4f)
            level = 0.0f;

        if (peak >= hold)
        {
            hold = peak;
            holdTicks = 30;   // 1 s
        }
        else if (--holdTicks <= 0)
        {
            hold *= 0.9f;
            if (hold < 1.0e-4f)
                hold = 0.0f;
        }

        repaint();
    }

    /** Position (y en vertical, x en horizontal) d'un niveau en dB. */
    float posFor (float db, juce::Rectangle<float> b) const noexcept
    {
        const float t = juce::jlimit (0.0f, 1.0f, (db - floorDb) / -floorDb);
        return horizontal ? b.getX() + t * b.getWidth()
                          : b.getBottom() - t * b.getHeight();
    }

    static juce::Colour colourFor (float db)
    {
        if (db > -1.0f) return juce::Colour (0xFFFF5A5F);
        if (db > -6.0f) return juce::Colour (0xFFFBBF24);
        return HarmoniaPalette::accent;
    }

    std::function<float()> source;
    bool horizontal = false;
    float level = 0.0f, hold = 0.0f;
    int holdTicks = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (LevelMeter)
};
