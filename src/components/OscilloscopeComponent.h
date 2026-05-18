/**
 * @file OscilloscopeComponent.h
 * @brief Real-time audio waveform visualization component.
 *
 * OscilloscopeComponent displays incoming audio signals
 * as a continuously scrolling waveform similar to a
 * hardware oscilloscope.
 *
 * The component:
 * - Receives live audio buffers from the DSP engine
 * - Stores samples inside a circular ring buffer
 * - Periodically copies data into a rendering buffer
 * - Draws the waveform with GPU-friendly JUCE graphics
 *
 * Features:
 * - Real-time waveform rendering
 * - Stereo-to-mono signal visualization
 * - Adjustable refresh rate
 * - Configurable vertical gain
 * - Thread-safe audio/UI synchronization
 * - Smooth glowing waveform rendering
 *
 * Architecture:
 * Audio Thread:
 *     pushBuffer()
 *
 * UI Thread:
 *     timerCallback()
 *         -> copies ringBuffer to drawBuffer
 *         -> repaint()
 *
 * Rendering:
 *     paint()
 *         -> drawWaveform()
 */

#pragma once

#include "ComponentsIncludes.h"

/**
 * @class OscilloscopeComponent
 * @brief Real-time oscilloscope waveform display.
 *
 * OscilloscopeComponent visualizes audio samples
 * received from the synthesizer or DSP pipeline.
 *
 * The component uses:
 * - A circular ring buffer for audio capture
 * - A separate draw buffer for rendering safety
 * - A JUCE timer for periodic screen refreshes
 *
 * The audio thread continuously pushes samples
 * into the ring buffer while the GUI thread
 * asynchronously renders the waveform.
 *
 * Thread Safety:
 * - Uses juce::CriticalSection
 * - Prevents simultaneous audio/UI access
 *
 * Rendering Pipeline:
 * pushBuffer()
 *     -> ringBuffer
 *
 * timerCallback()
 *     -> drawBuffer
 *
 * paint()
 *     -> waveform rendering
 */
class OscilloscopeComponent : public juce::Component,
                              private juce::Timer
{
public:

    /**
     * @brief Constructs the oscilloscope component.
     *
     * Initializes:
     * - Internal ring buffers
     * - Rendering buffers
     * - Refresh timer
     *
     * Buffer size and refresh rate are automatically clamped
     * to safe operating ranges.
     *
     * @param bufferSize Number of samples stored in the ring buffer.
     *                   Minimum value is internally clamped to 256.
     *
     * @param refreshHz GUI refresh frequency in Hz.
     *                  Valid range: 10 Hz to 120 Hz.
     */
    explicit OscilloscopeComponent (int bufferSize = 2048,
                                    int refreshHz  = 60);

    /**
     * @brief Pushes audio samples into the oscilloscope buffer.
     *
     * This method is typically called from the audio thread.
     *
     * Features:
     * - Supports mono and stereo input
     * - Automatically mixes stereo to mono
     * - Writes samples into a circular ring buffer
     * - Thread-safe via CriticalSection
     *
     * Stereo Behavior:
     * - If two channels exist:
     *       sample = (L + R) * 0.5
     *
     * @param buffer Source audio buffer.
     * @param startSample Starting sample index.
     * @param numSamples Number of samples to copy.
     */
    void pushBuffer (const juce::AudioBuffer<float>& buffer,
                     int startSample,
                     int numSamples) noexcept;

    /**
     * @brief Paints the oscilloscope interface.
     *
     * Draws:
     * - Background panel
     * - Border
     * - Center reference line
     * - Waveform visualization
     *
     * Visual Style:
     * - Rounded panel
     * - Gradient background
     * - Glow waveform effect
     *
     * @param g JUCE graphics context.
     */
    void paint (juce::Graphics& g) override;

    /**
     * @brief Handles component resizing.
     *
     * No additional layout behavior is required
     * because rendering adapts dynamically to bounds.
     */
    void resized() override {}

    /**
     * @brief Sets waveform vertical amplification.
     *
     * Controls waveform rendering amplitude
     * without affecting the actual audio signal.
     *
     * Useful for:
     * - Zooming weak signals
     * - Reducing clipping visualization
     *
     * @param newGain New vertical scaling factor.
     */
    void setVerticalGain (float newGain) noexcept
    {
        verticalGain = newGain;
    }

private:

    /**
     * @brief Periodic UI refresh callback.
     *
     * Called by the JUCE timer at the configured refresh rate.
     *
     * Responsibilities:
     * - Copies ring buffer into draw buffer
     * - Reorders wrapped circular data
     * - Triggers repaint()
     *
     * Thread-safe operation is guaranteed using
     * juce::ScopedLock.
     */
    void timerCallback() override;

    /**
     * @brief Draws the waveform path.
     *
     * Converts audio samples into:
     * - X/Y screen coordinates
     * - JUCE Path geometry
     *
     * Rendering includes:
     * - Glow layer
     * - Sharp foreground waveform
     *
     * Sample Mapping:
     * - X axis = sample index
     * - Y axis = audio amplitude
     *
     * @param g Rendering graphics context.
     * @param area Target drawing area.
     */
    void drawWaveform (juce::Graphics& g,
                       juce::Rectangle<int> area);

    /**
     * @brief Synchronization lock for thread safety.
     *
     * Protects simultaneous access to:
     * - ringBuffer
     * - drawBuffer
     * - write position
     */
    juce::CriticalSection lock;

    /**
     * @brief Circular audio capture buffer.
     *
     * Continuously receives incoming audio samples
     * from the audio thread.
     */
    juce::AudioBuffer<float> ringBuffer;

    /**
     * @brief Rendering-safe waveform buffer.
     *
     * Copied from ringBuffer during timer updates
     * to avoid rendering directly from the live buffer.
     */
    juce::AudioBuffer<float> drawBuffer;

    /**
     * @brief Current circular write position.
     *
     * Indicates where the next incoming sample
     * will be stored inside the ring buffer.
     */
    int writePos = 0;

    /**
     * @brief Vertical waveform scaling factor.
     *
     * Used only for visual amplification.
     */
    float verticalGain = 1.0f;
};