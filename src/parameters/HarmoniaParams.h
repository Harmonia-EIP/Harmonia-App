#pragma once
#include <juce_core/juce_core.h>

enum class FilterType
{
    LOWPASS,
    HIGHPASS,
    BANDPASS
};

enum class Waveform
{
    SINE,
    SQUARE,
    SAW,
    TRIANGLE
};

struct PatchParams
{
    double frequency  = 440.0;
    double volume     = 0.8;

    double attack     = 0.1;
    double decay      = 0.1;
    double sustain    = 0.8;
    double release    = 0.5;

    double cutoff     = 1000.0;
    double resonance  = 1.0;

    FilterType filterType = FilterType::LOWPASS;
    Waveform waveform     = Waveform::SINE;

    juce::String prompt;
};