/**
 * @file SynthComponent.h
 * @brief Virtual MIDI keyboard UI component for Harmonia.
 *
 * This component provides a graphical MIDI keyboard interface
 * using JUCE's MidiKeyboardComponent.
 *
 * It is responsible for:
 * - Displaying a playable piano keyboard
 * - Synchronizing note states with a shared MidiKeyboardState
 * - Providing a visual MIDI input surface for the synthesizer
 *
 * The component acts as a lightweight wrapper around
 * juce::MidiKeyboardComponent and automatically manages
 * keyboard rendering and resizing behavior.
 */

#pragma once

#include "ComponentsIncludes.h"

/**
 * @class SynthComponent
 * @brief UI component containing a virtual MIDI keyboard.
 *
 * SynthComponent wraps a JUCE MidiKeyboardComponent and
 * exposes the associated MidiKeyboardState used for
 * note input and synchronization.
 *
 * Features:
 * - Horizontal piano keyboard layout
 * - Automatic component resizing
 * - MIDI state sharing with the audio engine
 * - Simple black background rendering
 *
 * The keyboard allows:
 * - Mouse interaction
 * - MIDI note visualization
 * - External MIDI synchronization
 *
 * Architecture:
 * - keyboardState stores active note states
 * - keyboardComponent renders the keyboard UI
 * - paint() handles background rendering
 * - resized() updates keyboard bounds
 */
class SynthComponent : public juce::Component
{
public:

    /**
     * @brief Constructs the synthesizer keyboard component.
     *
     * Initializes the internal JUCE keyboard component
     * using a shared MidiKeyboardState.
     *
     * The keyboard is displayed horizontally and automatically
     * added to the component hierarchy.
     *
     * @param state Shared MIDI keyboard state used for
     *              note synchronization and playback.
     */
    SynthComponent(juce::MidiKeyboardState& state);

    /**
     * @brief Destructor.
     *
     * Uses the default JUCE cleanup behavior.
     */
    ~SynthComponent() override = default;

    /**
     * @brief Returns the associated keyboard state.
     *
     * Provides access to the internal MidiKeyboardState
     * used by the keyboard component.
     *
     * @return Reference to the MIDI keyboard state.
     */
    juce::MidiKeyboardState& getKeyboardState() noexcept
    {
        return keyboardState;
    }

    /**
     * @brief Paints the component background.
     *
     * Fills the entire component area with a black color.
     *
     * @param g JUCE graphics context.
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Updates child component layout.
     *
     * Resizes the internal keyboard component to occupy
     * the full available bounds of the parent component.
     */
    void resized() override;

private:

    /**
     * @brief Shared MIDI keyboard state.
     *
     * Stores note on/off information and synchronizes
     * keyboard interactions with the synthesizer engine.
     */
    juce::MidiKeyboardState& keyboardState;

    /**
     * @brief Graphical JUCE piano keyboard component.
     *
     * Responsible for rendering and interaction handling.
     */
    juce::MidiKeyboardComponent keyboardComponent;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SynthComponent)
};