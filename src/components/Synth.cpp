#include "Synth.h"

#include <algorithm>

HarmoniaVoice::HarmoniaVoice (const HarmoniaPatches& sharedPatches, std::uint64_t voiceSeed)
    : patches (sharedPatches), seed (voiceSeed)
{
}

void HarmoniaVoice::prepare (double sampleRate, int samplesPerBlock, int)
{
    analog.prepare (sampleRate);
    dx7.prepare (sampleRate);
    left.assign ((size_t) juce::jmax (1, samplesPerBlock), 0.0f);
    right.assign ((size_t) juce::jmax (1, samplesPerBlock), 0.0f);
}

bool HarmoniaVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<HarmoniaSound*> (sound) != nullptr;
}

void HarmoniaVoice::startNote (int midiNoteNumber, float velocity, juce::SynthesiserSound*, int)
{
    const double v = juce::jlimit (0.0f, 1.0f, velocity);
    playingDx7 = patches.dx7Mode;
    if (playingDx7)
    {
        analog.kill();
        dx7.start (patches.dx7, midiNoteNumber, v);
    }
    else
    {
        dx7.kill();
        analog.start (patches.analog, midiNoteNumber, v, seed * 0x9E3779B97F4A7C15ULL + ++notesPlayed);
    }
}

void HarmoniaVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        analog.release();
        dx7.release();
    }
    else
    {
        analog.kill();
        dx7.kill();
        clearCurrentNote();
    }
}

void HarmoniaVoice::renderNextBlock (juce::AudioBuffer<float>& buffer, int startSample, int numSamples)
{
    auto isActive = [this] { return playingDx7 ? dx7.isActive() : analog.isActive(); };
    if (! isActive())
        return;

    const int channels = buffer.getNumChannels();
    const int chunkSize = (int) left.size();

    for (int done = 0; done < numSamples && isActive(); done += chunkSize)
    {
        const int n = std::min (chunkSize, numSamples - done);
        std::fill (left.begin(), left.begin() + n, 0.0f);
        std::fill (right.begin(), right.begin() + n, 0.0f);
        if (playingDx7)
            dx7.render (left.data(), right.data(), n);
        else
            analog.render (patches.analog, left.data(), right.data(), n);

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

    if (! isActive())
        clearCurrentNote();
}
