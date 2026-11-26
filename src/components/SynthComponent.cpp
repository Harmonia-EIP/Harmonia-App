#include "SynthComponent.h"

SynthComponent::SynthComponent (juce::MidiKeyboardState& state)
    : keyboardState (state),
      keyboardComponent (state)
{
    addAndMakeVisible (keyboardComponent);

    octaveChip.setTooltip ("Octave played by the computer keys (W / X, or Z / X in QWERTY)");
    octaveChip.onStep = [this] (int dir) { keyboardComponent.shiftOctave (dir); };

    velocityChip.setTooltip ("Velocity of the computer keys and mouse (C / V)");
    velocityChip.onStep = [this] (int dir)
    {
        keyboardComponent.changeKeyVelocity ((float) dir * HarmoniaKeyboard::velocityStep);
    };

    layoutChip.setTooltip ("Computer keyboard layout (B)");
    layoutChip.onClick = [this] { keyboardComponent.toggleLayout(); };

    for (auto* c : { &octaveChip, &velocityChip, &layoutChip })
        addAndMakeVisible (c);

    keyboardComponent.onSettingsChanged = [this] { updateChips(); };
    updateChips();
}

void SynthComponent::updateChips()
{
    auto& kb = keyboardComponent;
    octaveChip.setText (kb.noteName (kb.getFirstMappedNote()));
    velocityChip.setText ("VEL " + juce::String (juce::roundToInt (kb.getKeyVelocity() * 100.0f)) + "%");
    layoutChip.setText (kb.keys().name);
    layoutChip.setHighlighted (true);
}

void SynthComponent::paint (juce::Graphics& g)
{
    const auto r = frame().toFloat();
    constexpr float corner = 10.0f;

    {
        juce::Path shadowPath;
        shadowPath.addRoundedRectangle (r.translated (0.0f, 1.5f), corner);
        juce::DropShadow (juce::Colours::black.withAlpha (0.30f), 5, { 0, 2 }).drawForPath (g, shadowPath);
    }

    g.setGradientFill (juce::ColourGradient (HarmoniaPalette::panelTop, 0.0f, r.getY(),
                                             HarmoniaPalette::panel,    0.0f, r.getBottom(), false));
    g.fillRoundedRectangle (r, corner);

    g.setColour (juce::Colours::white.withAlpha (0.06f));
    g.drawLine (r.getX() + corner, r.getY() + 1.0f, r.getRight() - corner, r.getY() + 1.0f, 1.0f);

    g.setColour (HarmoniaPalette::border);
    g.drawRoundedRectangle (r, corner, 1.0f);

    // LED + titre, comme les SectionPanel
    const auto tint = HarmoniaPalette::accent;
    const float ledR = 3.0f, ledX = r.getX() + 10.0f, ledY = r.getY() + (float) titleH * 0.5f + 3.0f;

    g.setColour (tint.withAlpha (0.40f));
    g.fillEllipse (ledX - ledR * 1.6f, ledY - ledR * 1.6f, ledR * 3.2f, ledR * 3.2f);
    g.setColour (tint);
    g.fillEllipse (ledX - ledR, ledY - ledR, ledR * 2.0f, ledR * 2.0f);

    g.setFont (juce::Font (juce::FontOptions (10.0f).withStyle ("Bold")).withExtraKerningFactor (0.20f));
    g.drawText ("KEYBOARD", juce::Rectangle<float> (ledX + 12.0f, ledY - 7.0f, 120.0f, 14.0f),
                juce::Justification::centredLeft, false);
}

void SynthComponent::resized()
{
    auto inner = frame().reduced (8, 0);

    // ligne de titre : les chips à droite
    auto titleRow = inner.removeFromTop (titleH + 6).withTrimmedTop (5).withTrimmedBottom (1);

    layoutChip.setBounds   (titleRow.removeFromRight (layoutChip.idealWidth ("AZERTY")));
    titleRow.removeFromRight (6);
    velocityChip.setBounds (titleRow.removeFromRight (velocityChip.idealWidth ("VEL 100%")));
    titleRow.removeFromRight (6);
    octaveChip.setBounds   (titleRow.removeFromRight (octaveChip.idealWidth ("C#10")));

    inner.removeFromTop (4);
    keyboardComponent.setBounds (inner.withTrimmedBottom (8));
}
