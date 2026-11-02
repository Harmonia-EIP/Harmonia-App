#pragma once

#include "HeaderIncludes.h"

//==============================================================================
// Petits utilitaires partagés par HistoryPicker, HistoryPanel et HistoryTile
namespace HistoryUtils
{
    inline juce::Colour danger() { return juce::Colour (0xffe5534b); }

    bool isRefined (const juce::String& type);

    // "2026-10-28T14:03:11.123456Z" (UTC) -> "28 Oct 14:03" (heure locale)
    juce::String formatDate (const juce::String& iso);
}
