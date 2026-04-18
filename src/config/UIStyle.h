#pragma once
#include <juce_graphics/juce_graphics.h>

namespace UIStyle
{
    namespace Fonts
    {
        static constexpr float TitleSize = 36.0f;
        static constexpr float SubTitleSize = 24.0f;
        static constexpr float PseudoSize = 18.0f;

        inline juce::Font Title()
        {
            return juce::Font(TitleSize, juce::Font::bold);
        }
        inline juce::Font SubTitle()
        {
            return juce::Font(SubTitleSize, juce::Font::bold);
        }
        inline juce::Font Pseudo()
        {
            return juce::Font(PseudoSize, juce::Font::bold);
        }
    }
}