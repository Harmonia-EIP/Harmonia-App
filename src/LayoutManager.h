#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include "themes/AppLookAndFeel.h"

class LayoutManager
{
public:
    using LayoutPreset = AppLookAndFeel::LayoutPreset;

    void setLayout(LayoutPreset layout);
    
    void applyLayout(juce::Rectangle<int> area,
                     juce::Component& title,
                     juce::Component& topBar,
                     juce::Component& bottomBar,
                     juce::Component& oscilloscope,
                     juce::Component& freqVol,
                     juce::Component& adsr,
                     juce::Component& filter,
                     juce::Component& synth);

private:
    LayoutPreset currentLayout;
};