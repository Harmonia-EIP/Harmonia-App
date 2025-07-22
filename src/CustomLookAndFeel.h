#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4
{
public:
    CustomLookAndFeel()
    {
        setColour(juce::ComboBox::backgroundColourId, juce::Colour(25, 30, 35));
        setColour(juce::ComboBox::textColourId, juce::Colours::white);
        setColour(juce::ComboBox::outlineColourId, juce::Colour(40, 40, 40));
        setColour(juce::ComboBox::arrowColourId, juce::Colours::white);

        setColour(juce::PopupMenu::backgroundColourId, juce::Colour(35, 40, 45));
        setColour(juce::PopupMenu::highlightedBackgroundColourId, juce::Colour(60, 100, 140));
        setColour(juce::PopupMenu::highlightedTextColourId, juce::Colours::white);
        setColour(juce::PopupMenu::textColourId, juce::Colours::white);
    }
};
