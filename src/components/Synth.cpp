#include "Synth.h"

#include <algorithm>

HarmoniaVoice::HarmoniaVoice (const harmonia::AnalogPatch& sharedPatch, std::uint64_t voiceSeed)
    : patch (sharedPatch), seed (voiceSeed)
{
}

void HarmoniaVoice::prepare (double sampleRate, int samplesPerBlock, int)
{
    engine.prepare (sampleRate);
    left.assign ((size_t) juce::jmax (1, samplesPerBlock), 0.0f);
    right.assign ((size_t) juce::jmax (1, samplesPerBlock), 0.0f);
}

bool HarmoniaVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<HarmoniaSound*> (sound) != nullptr;
}

void HarmoniaVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    // DX7 mode (engine = 1) comes with Harmonia-App#50; until then every preset plays on the analog engine.
    engine.start (patch, midiNoteNumber, juce::jlimit (0.0f, 1.0f, velocity), seed * 0x9E3779B97F4A7C15ULL + ++notesPlayed);
}

void HarmoniaVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        engine.release();
    }
    else
    {
        engine.kill();
        clearCurrentNote();
    }
}

void HarmoniaVoice::renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    if (! engine.isActive())
        return;

    const int channels = buffer.getNumChannels();
    const int chunkSize = (int) left.size();

    for (int done = 0; done < numSamples && engine.isActive(); done += chunkSize)
    {
        const int n = std::min (chunkSize, numSamples - done);
        std::fill (left.begin(), left.begin() + n, 0.0f);
        std::fill (right.begin(), right.begin() + n, 0.0f);
        engine.render (patch, left.data(), right.data(), n);

        if (channels >= 2)
        {
            buffer.addFrom (0, startSample + done, left.data(), n);
            buffer.addFrom (1, startSample + done, right.data(), n);
        }
        else if (channels == 1)
        {
            buffer.addFrom (0, startSample + done, left.data(), n, 0.5f);
            buffer.addFrom (0, startSample + done, right.data(), n, 0.5f);
        }
    }

    if (! engine.isActive())
        clearCurrentNote();
}
