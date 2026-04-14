#pragma once
#include <juce_core/juce_core.h>

struct HarmoniaParams
{
    float frequency      = 440.0f;
    float volume         = 0.8f;

    float attack         = 0.1f;
    float decay          = 0.1f;
    float sustain        = 0.8f;
    float release        = 0.5f;

    float cutoff         = 1000.0f;
    float resonance      = 1.0f;

    int   waveformIndex  = 0;              // 0 = Sine, 1 = Saw, 2 = Square
    juce::String filterType = "Low Pass";  // "Low Pass", "Band Pass", "High Pass"
};
