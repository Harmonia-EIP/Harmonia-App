#pragma once

#include "HeaderIncludes.h"

namespace HeaderColours
{
    inline juce::Colour generate() { return HarmoniaPalette::accent; }
    inline juce::Colour refine()   { return HarmoniaPalette::accent.withRotatedHue (0.5f); }
}
