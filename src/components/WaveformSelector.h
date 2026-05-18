/**
 * @file WaveformSelector.h
 * @brief Specialized waveform and filter selector components.
 *
 * This file defines icon-based selectors used to choose:
 * - Oscillator waveforms
 * - Filter types
 *
 * Both components inherit from IconChoiceSelector.
 */
#pragma once

#include "IconChoiceSelector.h"
#include "IconRenderer.h"

/**
 * @class WaveformSelector
 * @brief Icon-based waveform selection component.
 *
 * Displays selectable waveform icons using IconRenderer.
 */
class WaveformSelector : public IconChoiceSelector
{
public:
    WaveformSelector (juce::AudioProcessorValueTreeState& apvts,
                      const juce::String& paramId)
        : IconChoiceSelector (apvts, paramId,
            [] (juce::Graphics& g, juce::Rectangle<float> area, int idx, juce::Colour col)
            {
                IconRenderer::drawWave (g, area,
                                        static_cast<IconRenderer::Wave> (idx), col, 1.7f);
            })
    {}
};

/**
 * @class FilterTypeSelector
 * @brief Icon-based filter type selector component.
 *
 * Displays selectable filter response icons.
 */
class FilterTypeSelector : public IconChoiceSelector
{
public:
    FilterTypeSelector (juce::AudioProcessorValueTreeState& apvts,
                        const juce::String& paramId)
        : IconChoiceSelector (apvts, paramId,
            [] (juce::Graphics& g, juce::Rectangle<float> area, int idx, juce::Colour col)
            {
                IconRenderer::drawFilterShape (g, area,
                                               static_cast<IconRenderer::FilterShape> (idx), col, 1.7f);
            })
    {}
};