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

    void drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                           float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                           juce::Slider& slider) override
    {
        auto radius  = (float) juce::jmin (width / 2, height / 2) - 4.0f;
        auto centreX = (float) x + (float) width  * 0.5f;
        auto centreY = (float) y + (float) height * 0.5f;
        auto rx = centreX - radius;
        auto ry = centreY - radius;
        auto rw = radius * 2.0f;
        auto angle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);

        g.setColour (juce::Colour(40, 45, 50));
        g.fillEllipse (rx, ry, rw, rw);

        g.setColour (juce::Colours::black);
        g.drawEllipse (rx, ry, rw, rw, 1.5f);

        juce::Path p;
        auto pointerLength   = radius * 0.7f;
        auto pointerThickness = 3.0f;
        p.addRectangle (-pointerThickness * 0.5f, -radius, pointerThickness, pointerLength);
        p.applyTransform (juce::AffineTransform::rotation (angle).translated (centreX, centreY));

        g.setColour (juce::Colours::skyblue);
        g.fillPath (p);
    }
};
