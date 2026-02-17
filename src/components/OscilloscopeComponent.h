#pragma once

#include <JuceHeader.h>

// Include explicite des classes audio (chez vous JuceHeader ne les expose pas ici)
#include <juce_audio_basics/juce_audio_basics.h>

class OscilloscopeComponent : public juce::Component,
                              private juce::Timer
{
public:
    // bufferSize : taille du ring buffer
    // refreshHz  : fréquence de repaint
    explicit OscilloscopeComponent (int bufferSize = 2048, int refreshHz = 60);

    void pushBuffer (const juce::AudioBuffer<float>& buffer,
                     int startSample,
                     int numSamples) noexcept;

    void paint (juce::Graphics& g) override;

private:
    void timerCallback() override;
    void drawWaveform (juce::Graphics& g, juce::Rectangle<int> area);

    juce::CriticalSection lock;

    juce::AudioBuffer<float> ringBuffer;
    juce::AudioBuffer<float> drawBuffer;

    int writePos = 0;
    float verticalGain = 1.0f;
};
