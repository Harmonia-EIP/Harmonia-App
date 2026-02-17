#include "OscilloscopeComponent.h"

OscilloscopeComponent::OscilloscopeComponent (int bufferSize, int refreshHz)
{
    bufferSize = juce::jmax (256, bufferSize);
    refreshHz  = juce::jlimit (10, 120, refreshHz);

    ringBuffer.setSize (1, bufferSize);
    ringBuffer.clear();

    drawBuffer.setSize (1, bufferSize);
    drawBuffer.clear();

    startTimerHz (refreshHz);
}

void OscilloscopeComponent::pushBuffer (const juce::AudioBuffer<float>& buffer,
                                       int startSample,
                                       int numSamples) noexcept
{
    const int chs = buffer.getNumChannels();
    if (chs <= 0 || numSamples <= 0)
        return;

    const int ringSize = ringBuffer.getNumSamples();

    const float* ch0 = buffer.getReadPointer (0, startSample);
    const float* ch1 = (chs > 1) ? buffer.getReadPointer (1, startSample) : nullptr;

    const juce::ScopedLock sl (lock);

    for (int i = 0; i < numSamples; ++i)
    {
        float s = ch0[i];
        if (ch1 != nullptr)
            s = 0.5f * (s + ch1[i]); // downmix mono

        ringBuffer.setSample (0, writePos, s);

        writePos++;
        if (writePos >= ringSize)
            writePos = 0;
    }
}

void OscilloscopeComponent::timerCallback()
{
    const juce::ScopedLock sl (lock);

    const int ringSize = ringBuffer.getNumSamples();
    const int tail     = ringSize - writePos;

    // Copie sans FloatVectorOperations (pour éviter tout souci de module)
    std::memcpy (drawBuffer.getWritePointer (0),
                 ringBuffer.getReadPointer (0, writePos),
                 (size_t) tail * sizeof(float));

    if (writePos > 0)
    {
        std::memcpy (drawBuffer.getWritePointer (0) + tail,
                     ringBuffer.getReadPointer (0, 0),
                     (size_t) writePos * sizeof(float));
    }

    repaint();
}

void OscilloscopeComponent::paint (juce::Graphics& g)
{
    g.fillAll (juce::Colours::black.withAlpha (0.7f));

    auto area = getLocalBounds().reduced (8);
    g.setColour (juce::Colours::white.withAlpha (0.15f));
    g.drawRect (area);

    drawWaveform (g, area);
}

void OscilloscopeComponent::drawWaveform (juce::Graphics& g, juce::Rectangle<int> area)
{
    const int n = drawBuffer.getNumSamples();
    if (n <= 1)
        return;

    auto* data = drawBuffer.getReadPointer (0);

    const float midY  = (float) area.getCentreY();
    const float halfH = (float) area.getHeight() * 0.45f;

    const float x0 = (float) area.getX();
    const float w  = (float) area.getWidth();

    auto sampleToX = [=] (int i)
    {
        return x0 + (w * (float) i / (float) (n - 1));
    };

    auto sampleToY = [=] (float s)
    {
        s = juce::jlimit (-1.0f, 1.0f, s * verticalGain);
        return midY - s * halfH;
    };

    juce::Path p;
    p.preallocateSpace ((size_t) n * 3);

    p.startNewSubPath (juce::Point<float> (sampleToX (0), sampleToY (data[0])));

    for (int i = 1; i < n; ++i)
        p.lineTo (juce::Point<float> (sampleToX (i), sampleToY (data[i])));

    g.setColour (juce::Colours::lime.withAlpha (0.9f));
    g.strokePath (p, juce::PathStrokeType (2.0f));
}
