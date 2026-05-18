/**
 * @file PluginProcessor.h
 * @brief Main audio processor of the Harmonia synthesizer plugin.
 *
 * HarmoniaAudioProcessor is responsible for:
 *
 * Audio responsibilities:
 * - Polyphonic synthesizer voice management
 * - MIDI event processing
 * - Real-time audio rendering
 * - DSP effect processing
 * - Reverb processing
 *
 * UI/Data responsibilities:
 * - Parameter state management (APVTS)
 * - MIDI keyboard synchronization
 * - Oscilloscope audio streaming
 * - Plugin state serialization
 *
 * Internal architecture:
 * HarmoniaAudioProcessor
 * ├── APVTS parameters
 * ├── Harmonia synthesizer voices
 * ├── Reverb DSP processor
 * ├── MIDI keyboard state
 * └── Oscilloscope audio monitor
 */
#pragma once

#include "JuceHeader.h"
#include "parameters/HarmoniaParameters.h"
#include "components/Synth.h"
#include "components/OscilloscopeComponent.h"

/**
 * @class HarmoniaAudioProcessor
 * @brief Main JUCE audio processor for the Harmonia synthesizer.
 *
 * This class contains the complete audio engine
 * of the plugin.
 *
 * Core features:
 * - Polyphonic synthesis engine
 * - Real-time MIDI synthesis
 * - Audio DSP processing
 * - Parameter automation support
 * - Plugin state persistence
 * - Oscilloscope audio monitoring
 *
 * DSP chain:
 * MIDI Input
 * → Synth Voices
 * → Reverb
 * → Oscilloscope Tap
 * → Audio Output
 */
class HarmoniaAudioProcessor : public juce::AudioProcessor
{
public:
    /**
     * @brief Creates the audio processor.
     *
     * Initializes:
     * - Audio buses
     * - Parameter tree state
     * - Parameter references
     */
    HarmoniaAudioProcessor();

    /**
     * @brief Destroys the processor.
     */
    ~HarmoniaAudioProcessor() override;

    /**
     * @brief Prepares DSP resources before playback starts.
     *
     * Initializes:
     * - Synth voices
     * - Reverb DSP
     * - Playback sample rate
     *
     * @param sampleRate Current playback sample rate.
     * @param samplesPerBlock Maximum block size.
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    /**
     * @brief Releases playback resources.
     */
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations

    /**
     * @brief Validates supported audio bus layouts.
     *
     * Supports:
     * - Mono
     * - Stereo
     *
     * @param layouts Proposed bus layout.
     * @return True if supported.
     */
    bool isBusesLayoutSupported(const BusesLayout& layouts) const override;

   #endif

    /**
     * @brief Main real-time audio processing callback.
     *
     * Processing stages:
     * - MIDI processing
     * - Synth rendering
     * - Reverb processing
     * - Oscilloscope monitoring
     *
     * @param buffer Audio buffer to process.
     * @param midiMessages MIDI event buffer.
     */
    void processBlock(juce::AudioBuffer<float>&,
                      juce::MidiBuffer&) override;

    /**
     * @brief Creates the plugin editor.
     *
     * @return Pointer to the editor instance.
     */
    juce::AudioProcessorEditor* createEditor() override;

    /**
     * @brief Indicates that the plugin provides an editor.
     *
     * @return Always true.
     */
    bool hasEditor() const override;

    /**
     * @brief Returns the plugin name.
     */
    const juce::String getName() const override;

    /**
     * @brief Indicates MIDI input support.
     */
    bool acceptsMidi() const override;

    /**
     * @brief Indicates MIDI output support.
     */
    bool producesMidi() const override;

    /**
     * @brief Indicates whether the plugin is a MIDI effect.
     */
    bool isMidiEffect() const override;

    /**
     * @brief Returns the plugin tail duration.
     *
     * Mostly determined by the reverb effect.
     */
    double getTailLengthSeconds() const override;

    /**
     * @brief Returns the number of programs.
     */
    int getNumPrograms() override;

    /**
     * @brief Returns the current program index.
     */
    int getCurrentProgram() override;

    /**
     * @brief Changes the current program.
     *
     * @param index Program index.
     */
    void setCurrentProgram(int index) override;

    /**
     * @brief Returns the name of a program.
     *
     * @param index Program index.
     */
    const juce::String getProgramName(int index) override;

    /**
     * @brief Renames a program.
     *
     * @param index Program index.
     * @param newName New program name.
     */
    void changeProgramName(int index,
                           const juce::String& newName) override;

    /**
     * @brief Serializes plugin state.
     *
     * Stores:
     * - Parameter values
     * - Automation state
     *
     * @param destData Destination memory block.
     */
    void getStateInformation(juce::MemoryBlock& destData) override;

    /**
     * @brief Restores plugin state.
     *
     * @param data Serialized state data.
     * @param sizeInBytes Data size.
     */
    void setStateInformation(const void* data,
                             int sizeInBytes) override;

    /**
     * @brief Returns the shared MIDI keyboard state.
     */
    juce::MidiKeyboardState& getKeyboardState();

    /**
     * @brief Returns the parameter tree state.
     */
    juce::AudioProcessorValueTreeState& getAPVTS()
    {
        return apvts;
    }

    /**
     * @brief Attaches an oscilloscope component.
     *
     * The oscilloscope receives processed audio samples
     * during playback for waveform visualization.
     *
     * @param osc Oscilloscope instance.
     */
    void setOscilloscope(OscilloscopeComponent* osc);

private:
    /**
     * AudioProcessorValueTreeState storing all plugin parameters.
     */
    juce::AudioProcessorValueTreeState apvts;

    /**
     * Cached atomic parameter references for fast
     * real-time access inside DSP code.
     */
    HarmoniaParams::AtomicRefs paramRefs;

    /**
     * Polyphonic synthesizer engine.
     */
    juce::Synthesiser synth;

    /**
     * Shared MIDI keyboard state used by the UI
     * and MIDI processing pipeline.
     */
    juce::MidiKeyboardState keyboardState;

    /**
     * Global reverb DSP processor.
     */
    juce::dsp::Reverb reverb;

    /**
     * Current reverb parameter configuration.
     */
    juce::dsp::Reverb::Parameters reverbParams;

    /**
     * Optional oscilloscope visualization target.
     */
    OscilloscopeComponent* oscilloscope = nullptr;

    /**
     * Synchronization lock protecting oscilloscope access.
     */
    juce::CriticalSection oscLock;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HarmoniaAudioProcessor)
};