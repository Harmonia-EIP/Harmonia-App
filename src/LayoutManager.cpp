#include "LayoutManager.h"

void LayoutManager::setLayout(LayoutPreset layout)
{
    currentLayout = layout;
}

void LayoutManager::applyLayout(juce::Rectangle<int> area,
                               juce::Component& title,
                               juce::Component& topBar,
                               juce::Component& bottomBar,
                               juce::Component& oscilloscope,
                               juce::Component& freqVol,
                               juce::Component& adsr,
                               juce::Component& filter,
                               juce::Component& synth)
{
    area = area.reduced(10);

    title.setBounds(area.removeFromTop(50));
    topBar.setBounds(area.removeFromTop(70));
    bottomBar.setBounds(area.removeFromBottom(50));

    switch (currentLayout)
    {
        case LayoutPreset::Layout1:
            oscilloscope.setBounds(area.removeFromTop(100));
            freqVol.setBounds(area.removeFromTop(110));
            adsr.setBounds(area.removeFromTop(110));
            filter.setBounds(area.removeFromTop(110));
            synth.setBounds(area);
            break;

        case LayoutPreset::Layout2:
        {
            oscilloscope.setBounds(area.removeFromTop(170));
            synth.setBounds(area.removeFromTop(100));

            auto freqFilterArea = area.removeFromTop(120);
            auto halfWidth = freqFilterArea.getWidth() / 2;

            freqVol.setBounds(freqFilterArea.removeFromLeft(halfWidth));
            filter.setBounds(freqFilterArea);

            adsr.setBounds(area.removeFromTop(120));
            break;
        }

        case LayoutPreset::Layout3:
        {
            auto upper = area.removeFromTop(260);

            auto left = upper.removeFromLeft((int)(upper.getWidth() * 0.65f));
            oscilloscope.setBounds(left);

            auto right = upper;
            freqVol.setBounds(right.removeFromTop(130));
            filter.setBounds(right);

            adsr.setBounds(area.removeFromTop(130));
            synth.setBounds(area);
            break;
        }

        case LayoutPreset::Layout4:
        {
            auto upper = area.removeFromTop(260);

            auto left = upper.removeFromLeft((int)(upper.getWidth() * 0.35f));
            freqVol.setBounds(left.removeFromTop(130));
            filter.setBounds(left);

            auto right = upper;
            oscilloscope.setBounds(right);

            adsr.setBounds(area.removeFromTop(130));
            synth.setBounds(area);
            break;
        }

        default:
            break;
    }
}