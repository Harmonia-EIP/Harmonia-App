/**
 * @file MainComponent.h
 * @brief Main synthesizer interface component for Harmonia.
 *
 * MainComponent is the central graphical interface of the Harmonia plugin.
 * It assembles all synthesis sections, visualizers, controls, AI generation
 * tools, preset management, and user session interactions into a single UI.
 *
 * Features:
 * - Full synthesizer parameter interface
 * - Oscillator, filter, LFO, FX, and envelope sections
 * - Oscilloscope and waveform visual feedback
 * - AI-powered preset generation
 * - Preset loading and saving
 * - Session-aware user interface
 * - Animated particle background effects
 * - Responsive layout management
 *
 * UI Sections:
 * - Oscillator 1 / Mix
 * - Filter and modulation
 * - Oscillator 2
 * - Effects
 * - LFO modulation
 * - Amplitude envelope
 * - Display / oscilloscope
 * - MIDI keyboard input
 *
 * The component communicates directly with:
 * - HarmoniaAudioProcessor
 * - BackendManager
 * - APVTS parameter system
 *
 * Threading:
 * - AI preset generation is executed asynchronously
 * - UI updates are dispatched safely through JUCE message thread callbacks
 *
 * Lifetime:
 * - Owns all UI controls and visualizers
 * - Registers/unregisters oscilloscope safely with processor
 */
#pragma once

#define JUCE_USE_NATIVE_FILECHOOSERS 1

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

#include "themes/HarmoniaPalette.h"
#include "themes/HiveLookAndFeel.h"
#include "themes/PaletteSelector.h"

#include "backendManagement/BackendManager.h"

#include "components/SynthComponent.h"
#include "components/OscilloscopeComponent.h"
#include "components/SectionPanel.h"
#include "components/KnobControl.h"
#include "components/DisplayScreen.h"
#include "components/ParticleField.h"
#include "components/EnvelopeVisualizer.h"
#include "components/LfoVisualizer.h"
#include "components/WaveformSelector.h"

#include "sections/HeaderComponent.h"

#include "config/AppConfig.h"
#include "parameters/HarmoniaParameters.h"
#include "tools/PresetLoader.h"

class MainComponent : public juce::Component
{
public:
    /**
     * @brief Constructs the main plugin interface.
     *
     * Initializes:
     * - Look and feel
     * - Visual sections
     * - Parameter controls
     * - Oscilloscope routing
     * - Preset management
     * - AI generation tools
     *
     * @param p Reference to the audio processor.
     * @param be Backend manager used for AI and session communication.
     * @param session Current authenticated user session.
     */
    MainComponent (HarmoniaAudioProcessor& p,
                   BackendManager& be,
                   const UserSession& session);

    /**
     * @brief Destructor.
     *
     * Safely unregisters oscilloscope from the processor
     * and resets look-and-feel ownership.
     */
    ~MainComponent() override;

    /**
     * @brief Paints the main background and header visuals.
     *
     * Draws:
     * - Global background
     * - Header gradient
     * - Accent separators
     *
     * @param g JUCE graphics context.
     */
    void paint (juce::Graphics&) override;

    /**
     * @brief Updates layout positions and component sizing.
     *
     * Handles:
     * - Responsive section placement
     * - Header/button layout
     * - Keyboard sizing
     * - Visualizer sizing
     * - Dynamic knob arrangement
     */
    void resized() override;

    /**
     * @brief Callback triggered when user logs out.
     */
    std::function<void()> onLogout;

    // in the private section, near the other header widgets:
    PaletteSelector paletteSelector;

private:

    HarmoniaPalette::Theme currentTheme { HarmoniaPalette::Theme::Cyan };

    void applyTheme();

    static void repaintAllChildren (juce::Component& root)
    {
        root.repaint();
        for (int i = 0; i < root.getNumChildComponents(); ++i)
            repaintAllChildren (*root.getChildComponent(i));
    }

    /** Reference to the main audio processor. */
    HarmoniaAudioProcessor& processor;

    /** Backend communication manager. */
    BackendManager& backend;

    /** Current authenticated user session. */
    UserSession session;

    //==========================================================================
    // Look & feel / visuals

    /** Custom Harmonia look-and-feel implementation. */
    HiveLookAndFeel lookAndFeel;

    /** Animated particle background layer. */
    ParticleField particles;

    //==========================================================================
    // Header UI

    std::unique_ptr<HeaderComponent> headerComponent;

    //==========================================================================
    // Section containers

    std::unique_ptr<SectionPanel> oscMixPanel;
    std::unique_ptr<SectionPanel> filterPanel;
    std::unique_ptr<SectionPanel> screenPanel;
    std::unique_ptr<SectionPanel> lfoPanel;
    std::unique_ptr<SectionPanel> osc2Panel;
    std::unique_ptr<SectionPanel> fxPanel;
    std::unique_ptr<SectionPanel> ampPanel;

    //==========================================================================
    // Visualizers & displays

    /** Audio waveform oscilloscope. */
    OscilloscopeComponent oscilloscope;

    /** Display wrapper around oscilloscope. */
    DisplayScreen displayScreen;

    /** MIDI keyboard component. */
    SynthComponent synthComponent;

    /** ADSR envelope visualizer. */
    EnvelopeVisualizer ampEnvViz;

    /** LFO waveform visualizer. */
    LfoVisualizer lfoViz;

    //==========================================================================
    // Oscillator 1 controls

    /** Oscillator 1 waveform selector. */
    std::unique_ptr<WaveformSelector> osc1WaveSel;

    /** Oscillator mix control. */
    std::unique_ptr<KnobControl> oscMixKnob;

    /** Noise level control. */
    std::unique_ptr<KnobControl> noiseLevelKnob;

    //==========================================================================
    // Filter controls

    /** Filter cutoff frequency control. */
    std::unique_ptr<KnobControl> filterCutoffKnob;

    /** Filter resonance control. */
    std::unique_ptr<KnobControl> filterResoKnob;

    /** Filter type selector. */
    std::unique_ptr<FilterTypeSelector> filterTypeSel;

    /** Filter envelope modulation amount control. */
    std::unique_ptr<KnobControl> filterEnvAmtKnob;

    /** Filter envelope decay control. */
    std::unique_ptr<KnobControl> filterEnvDecayKnob;

    /** Velocity-to-filter modulation control. */
    std::unique_ptr<KnobControl> velocityFilterKnob;

    //==========================================================================
    // LFO controls

    /** LFO rate control. */
    std::unique_ptr<KnobControl> lfoRateKnob;

    /** LFO-to-pitch modulation control. */
    std::unique_ptr<KnobControl> lfoToPitchKnob;

    /** LFO-to-cutoff modulation control. */
    std::unique_ptr<KnobControl> lfoToCutoffKnob;

    //==========================================================================
    // Oscillator 2 controls

    /** Oscillator 2 waveform selector. */
    std::unique_ptr<WaveformSelector> osc2WaveSel;

    /** Oscillator 2 detune control. */
    std::unique_ptr<KnobControl> osc2DetuneKnob;

    //==========================================================================
    // FX controls

    /** Distortion amount control. */
    std::unique_ptr<KnobControl> distortionKnob;

    /** Reverb mix control. */
    std::unique_ptr<KnobControl> reverbKnob;

    //==========================================================================
    // ADSR controls

    /** Envelope attack control. */
    std::unique_ptr<KnobControl> attackKnob;

    /** Envelope decay control. */
    std::unique_ptr<KnobControl> decayKnob;

    /** Envelope sustain control. */
    std::unique_ptr<KnobControl> sustainKnob;

    /** Envelope release control. */
    std::unique_ptr<KnobControl> releaseKnob;

    //==========================================================================
    // Utilities

    /** File chooser used for preset loading/saving. */
    std::unique_ptr<juce::FileChooser> chooser;

    //==========================================================================
    // Internal helpers

    /**
     * @brief Creates and registers all parameter controls.
     *
     * Instantiates:
     * - Knobs
     * - Selectors
     * - Modulation controls
     */
    void buildControls();

    /**
     * @brief Connects button callbacks and interactions.
     */
    void wireHeaderButtons();

    /**
     * @brief Registers particle feedback interactions for a control.
     *
     * Adds animated particle bursts when controls are rapidly tweaked.
     *
     * @param c Component to monitor.
     */
    void registerJuiceFor (juce::Component* c);

    /**
     * @brief Opens and loads a preset file.
     */
    void doLoadPreset();

    /**
     * @brief Saves current synthesizer state as a preset.
     */
    void doSavePreset();

    /**
     * @brief Generates a new preset using AI backend services.
     *
     * Sends user prompt to backend, parses generated preset,
     * and applies parameters to the synthesizer.
     */
    void doGenerateWithAi();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};