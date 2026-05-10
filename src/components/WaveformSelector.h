#pragma once

#include "IconChoiceSelector.h"
#include "IconRenderer.h"

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
