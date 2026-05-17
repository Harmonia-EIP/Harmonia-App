#pragma once

#include "ComponentsIncludes.h"

class OscilloscopeComponent : public juce::Component,
                              private juce::Timer
{
public:
    explicit OscilloscopeComponent (int bufferSize = 2048,
                                    int refreshHz  = 60);

    void pushBuffer (const juce::AudioBuffer<float>& buffer,
                     int startSample,
                     int numSamples) noexcept;

    void paint (juce::Graphics& g) override;
    void resized() override {}

    void setVerticalGain (float newGain) noexcept { verticalGain = newGain; }

private:
    void timerCallback() override;
    void drawWaveform (juce::Graphics& g, juce::Rectangle<int> area);

    juce::CriticalSection lock;

    juce::AudioBuffer<float> ringBuffer;
    juce::AudioBuffer<float> drawBuffer;

    int writePos = 0;
    float verticalGain = 1.0f;
};
