#include "ActionButton.h"

void ActionButton::paintButton (juce::Graphics& g, bool over, bool down)
{
    auto b = getLocalBounds().toFloat().reduced (1.0f);
    const float r = juce::jmap (sharp, b.getHeight() * 0.5f, 3.0f);

    g.setColour (tint.withMultipliedBrightness (down ? 0.85f : (over ? 1.12f : 1.0f)));
    g.fillRoundedRectangle (b, r);

    // léger reflet
    juce::ColourGradient gloss (juce::Colours::white.withAlpha (0.20f), 0.0f, b.getY(),
                                juce::Colours::white.withAlpha (0.0f),  0.0f, b.getCentreY(),
                                false);
    g.setGradientFill (gloss);
    g.fillRoundedRectangle (b, r);

    g.setColour (juce::Colours::black.withAlpha (0.85f));
    g.setFont (juce::Font (juce::FontOptions (11.0f).withStyle ("Bold"))
                   .withExtraKerningFactor (0.14f));
    g.drawText (getButtonText().toUpperCase(), getLocalBounds(),
                juce::Justification::centred, false);
}
