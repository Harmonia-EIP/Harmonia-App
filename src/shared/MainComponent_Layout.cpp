#include "MainComponent.h"


void MainComponent::resized()
{
    auto area = getLocalBounds().reduced (10);

    title.setBounds (area.removeFromTop (50));
    topBar.setBounds (area.removeFromTop (70));
    bottomBar.setBounds (area.removeFromBottom (50));

    switch (layoutPreset)
    {
        case LayoutPreset::Layout1:
        {
            oscilloscope.setBounds (area.removeFromTop (100));
            freqVolComponent.setBounds (area.removeFromTop (110));
            adsrComponent.setBounds (area.removeFromTop (110));
            filterComponent.setBounds (area.removeFromTop (110));
            synthComponent.setBounds (area);
            break;
        }

        case LayoutPreset::Layout2:
        {
            oscilloscope.setBounds(area.removeFromTop(170));

            synthComponent.setBounds(area.removeFromTop(100));

            auto freqFilterArea = area.removeFromTop(120);
            auto halfWidth = freqFilterArea.getWidth() / 2;

            freqVolComponent.setBounds(freqFilterArea.removeFromLeft(halfWidth));
            filterComponent.setBounds(freqFilterArea);

            adsrComponent.setBounds(area.removeFromTop(120));

            break;
        }

        case LayoutPreset::Layout3:
        {
            auto upper = area.removeFromTop (260);

            auto left = upper.removeFromLeft ((int) (upper.getWidth() * 0.65f));
            oscilloscope.setBounds (left);

            auto right = upper;
            freqVolComponent.setBounds (right.removeFromTop (130));
            filterComponent.setBounds (right);

            adsrComponent.setBounds (area.removeFromTop (130));
            synthComponent.setBounds (area);
            break;
        }

        case LayoutPreset::Layout4:
        {
            auto upper = area.removeFromTop(260);

            auto left = upper.removeFromLeft((int)(upper.getWidth() * 0.35f));
            freqVolComponent.setBounds(left.removeFromTop(130));
            filterComponent.setBounds(left);

            auto right = upper;
            oscilloscope.setBounds(right);

            adsrComponent.setBounds(area.removeFromTop(130));
            synthComponent.setBounds(area);
            break;
        }

        default:
        {
            oscilloscope.setBounds (area.removeFromTop (120));
            freqVolComponent.setBounds (area.removeFromTop (100));
            adsrComponent.setBounds (area.removeFromTop (100));
            filterComponent.setBounds (area.removeFromTop (100));
            synthComponent.setBounds (area);
            break;
        }
    }
}

void MainComponent::applyLayout(LayoutPreset layout)
{
    layoutPreset = layout;
    resized();
    repaint();
}