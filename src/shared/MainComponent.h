#pragma once

#define JUCE_USE_NATIVE_FILECHOOSERS 1

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "components/FrequencySlider.h"
#include "components/VolumeSlider.h"
#include "components/GenerateButton.h"
#include "models/DataModel.h"

#include "themes/AppLookAndFeel.h"
#include "backendManagement/BackendManager.h"

#include "components/TitleComponent.h"
#include "components/TopBarComponent.h"
#include "components/FrequencyVolumeComponent.h"
#include "components/ADSRComponent.h"
#include "components/FilterComponent.h"
#include "components/BottomBarComponent.h"
#include "components/Synth.h"
#include "components/SynthComponent.h"
#include "components/OscilloscopeComponent.h"

#include "tools/ThemeAndLayoutConverter.h"
#include "tools/PatchSerializer.h"

/**
 * @class MainComponent
 * @brief Main UI and audio processing component of the application.
 *
 * This class is the central component of the application. It combines:
 * - Graphical User Interface (UI)
 * - Real-time audio processing (synthesizer)
 * - Backend communication (API & AI services)
 *
 * Responsibilities include:
 * - Managing UI components (sliders, controls, oscilloscope, keyboard)
 * - Handling user interactions and propagating changes to the audio engine
 * - Rendering audio using a JUCE synthesiser
 * - Synchronizing UI state with backend data
 * - Triggering backend API calls (including AI-based patch generation)
 *
 * The component follows JUCE's AudioAppComponent lifecycle.
 *
 * @note Audio processing runs in real-time and must remain performant.
 * @warning Backend calls are asynchronous and must not block the audio thread.
 */
class MainComponent : public juce::AudioAppComponent
{
public:
    using LayoutPreset = AppLookAndFeel::LayoutPreset;
    using ThemePreset  = AppLookAndFeel::ThemePreset;

    /**
     * @brief Constructs the main component.
     *
     * Initializes UI, synthesizer, theme/layout presets, and backend interactions.
     * Also triggers an asynchronous backend call to refresh the user profile.
     *
     * @param be Reference to the backend manager
     * @param session Current authenticated user session
     *
     * @pre BackendManager must remain valid during the component lifetime.
     * @post UI and audio system are initialized.
     *
     * @note May trigger asynchronous API calls to fetch updated profile data.
     */
    explicit MainComponent (BackendManager& be, const UserSession& session);

    /**
     * @brief Destructor.
     *
     * Ensures proper shutdown of the audio system and UI cleanup.
     */
    ~MainComponent() override;

    /**
     * @brief Paints the component background.
     *
     * Uses the current theme color defined in the LookAndFeel.
     */
    void paint (juce::Graphics&) override;

    /**
     * @brief Handles layout and positioning of all UI components.
     *
     * Layout dynamically adapts based on the selected LayoutPreset.
     */
    void resized() override;

    //==============================================================================
    // AUDIO LIFECYCLE

    /**
     * @brief Prepares the audio engine before playback starts.
     *
     * Initializes synthesizer voices with the correct sample rate and buffer size.
     *
     * @param samplesPerBlockExpected Expected buffer size
     * @param sampleRate Current audio sample rate
     */
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;

    /**
     * @brief Processes the next audio block.
     *
     * - Collects MIDI input from the keyboard component
     * - Renders audio via the synthesizer
     * - Sends audio data to the oscilloscope
     *
     * @param bufferToFill Audio buffer to render into
     *
     * @warning Must be real-time safe (no allocations or blocking calls).
     */
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;

    /**
     * @brief Releases audio resources.
     */
    void releaseResources() override;

    /**
     * @brief Provides access to the title component.
     *
     * @return Reference to the TitleComponent.
     */
    TitleComponent& getTitleComponent() { return title; }

private:
    //==============================================================================
    // BACKEND

    /**
     * @brief Backend manager handling API and AI interactions.
     *
     * Used for:
     * - Profile retrieval
     * - Theme/layout persistence
     * - AI-based patch generation
     */
    BackendManager& backend;

    //==============================================================================
    // UI & THEME

    AppLookAndFeel appLookAndFeel;

    TitleComponent title;
    OscilloscopeComponent oscilloscope;
    TopBarComponent topBar;
    FrequencyVolumeComponent freqVolComponent;
    ADSRComponent adsrComponent;
    FilterComponent filterComponent;
    BottomBarComponent bottomBar;

    SynthComponent synthComponent;

    //==============================================================================
    // AUDIO ENGINE

    /**
     * @brief JUCE synthesiser instance.
     *
     * Manages voices and sound generation.
     */
    juce::Synthesiser synth;

    /**
     * @brief Current audio sample rate.
     */
    double currentSampleRate = 44100.0;

    /**
     * @brief Updates synthesizer parameters from UI controls.
     *
     * Collects values from all UI components and propagates them
     * to each active voice.
     *
     * @note Called whenever a UI parameter changes.
     */
    void updateSynthParamsFromUI();

    //==============================================================================
    // LAYOUT & THEME

    LayoutPreset layoutPreset;

    /**
     * @brief Applies a UI theme preset.
     *
     * Updates the LookAndFeel and triggers a UI refresh.
     *
     * @param theme Theme preset to apply
     */
    void applyTheme(ThemePreset theme);

    /**
     * @brief Applies a layout preset.
     *
     * Repositions UI elements based on the selected layout.
     *
     * @param layout Layout preset to apply
     */
    void applyLayout(LayoutPreset layout);
    void initUI(const UserSession& session);
    void addComponents();
    void initComponentsListeners();
    void initTitleComponent();
    void initSynth();
    void initAsyncProfileLoad();
    void applyParamsToUI(const PatchParams& p);
    void initExportParameters();
    void initLoadParameters();
    void initGenerateWithAI();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};