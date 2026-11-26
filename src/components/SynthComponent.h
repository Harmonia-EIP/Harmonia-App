/**
 * @file SynthComponent.h
 * @brief Panneau « Keyboard » : le piano du plugin, dans le même cadre que les autres sections.
 *
 *   ( ● KEYBOARD                       [◂ C3 ▸] [◂ VEL 80% ▸] [AZERTY] )
 *   ( ┌───────────────────── touches sombres ──────────────────────┐ )
 *   ( │ zone jouable au clavier d'ordinateur : lettres + liseré      │ )
 *   ( └─────────────── le reste estompé, sans flèches ──────────────┘ )
 *
 * Les chips reprennent les raccourcis clavier (W/X octave, C/V vélocité, B disposition)
 * à la souris, et se mettent à jour quand on utilise ces raccourcis.
 */

#pragma once

#include "ComponentsIncludes.h"
#include "HarmoniaKeyboard.h"
#include "KeyboardChip.h"

class SynthComponent : public juce::Component
{
public:
    explicit SynthComponent (juce::MidiKeyboardState& state);
    ~SynthComponent() override = default;

    juce::MidiKeyboardState& getKeyboardState() noexcept { return keyboardState; }

    void paint (juce::Graphics& g) override;
    void resized() override;

    /**
     * @brief Gives keyboard focus to the piano, so the computer keys play notes
     *        (Q Z S E D F T G Y H U J K…) without clicking the keyboard first.
     */
    void focusKeyboard() { keyboardComponent.grabKeyboardFocus(); }

    /** Piano (octave, vélocité et disposition du clavier d'ordinateur). */
    HarmoniaKeyboard& getKeyboard() noexcept { return keyboardComponent; }

    /** True if `c` is the piano itself (or one of its children). */
    bool isKeyboard (const juce::Component* c) const
    {
        return c == &keyboardComponent || keyboardComponent.isParentOf (c);
    }

    KeyboardChip& getOctaveChip() noexcept   { return octaveChip; }
    KeyboardChip& getVelocityChip() noexcept { return velocityChip; }
    KeyboardChip& getLayoutChip() noexcept   { return layoutChip; }

    /** Même marge d'ombre que SectionPanel : cadres alignés au pixel. */
    static constexpr int margin = 6;
    static constexpr int titleH = 22;

private:
    void updateChips();
    juce::Rectangle<int> frame() const { return getLocalBounds().reduced (margin); }

    juce::MidiKeyboardState& keyboardState;
    HarmoniaKeyboard keyboardComponent;

    KeyboardChip octaveChip   { KeyboardChip::Kind::Stepper };
    KeyboardChip velocityChip { KeyboardChip::Kind::Stepper };
    KeyboardChip layoutChip   { KeyboardChip::Kind::Button };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SynthComponent)
};
