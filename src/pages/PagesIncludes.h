#pragma once

#include "../components/ComponentsIncludes.h"
#include "../themes/AppLookAndFeel.h"
#include "../themes/AppColourIds.h"
#include "../tools/Alert.h"

namespace HarmoniaColours
{
    // Backgrounds
    static const juce::Colour bgDeep        { 0xff0d1117 };  // near-black navy
    static const juce::Colour bgMid         { 0xff111827 };  // dark navy
    static const juce::Colour bgPanel       { 0xff1a2236 };  // slightly lighter panel

    // Wave / accent colours
    static const juce::Colour waveBlue   { 0xff4f7cff };
    static const juce::Colour waveCyan   { 0xff55d8ff };
    static const juce::Colour waveIndigo { 0xff6b5cff };
    static const juce::Colour waveSlate     { 0xff334155 };  // muted slate

    // Text
    static const juce::Colour textPrimary   { 0xffffffff };
    static const juce::Colour textSubtitle  { 0xff94a3b8 };  // cool grey
    static const juce::Colour textMuted     { 0xff64748b };

    // Button fills
    static const juce::Colour btnSignInHover{ 0xff2563eb };
    static const juce::Colour btnOutline    { 0xff1e2a3a };  // dark outlined
    static const juce::Colour btnOutlineHov { 0xff253447 };
    static const juce::Colour btnBorder     { 0xff2d4a6e };

    // Logo icon
    static const juce::Colour iconBg        { 0xff1a2236 };
    static const juce::Colour iconBorder    { 0xff2d4a6e };
    static const juce::Colour iconTilde     { 0xff60a5fa };  // light blue
}