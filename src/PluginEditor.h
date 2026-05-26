/**
 * @file PluginEditor.h
 * @brief Main graphical editor of the Harmonia plugin.
 *
 * HarmoniaAudioProcessorEditor is the root graphical
 * interface attached to the audio processor.
 *
 * Responsibilities:
 * - Hosting the application UI
 * - Managing the root layout
 * - Connecting the processor to visual components
 * - Displaying the navigation controller
 *
 * Main UI architecture:
 * HarmoniaAudioProcessorEditor
 * └── AppController
 *     ├── WelcomePage
 *     ├── LoginPage
 *     ├── SignupPage
 *     └── MainComponent
 */
#pragma once

#include "PluginProcessor.h"
#include "JuceHeader.h"
#include "config/AppConfig.h"
#include "components/SynthComponent.h"
#include "AppController.h"

/**
 * @class HarmoniaAudioProcessorEditor
 * @brief Root editor window of the Harmonia plugin.
 *
 * This component is the main JUCE editor attached
 * to the audio processor instance.
 *
 * It initializes:
 * - The application controller
 * - Global UI sizing
 * - Root rendering behavior
 *
 * The editor acts as the entry point of the
 * entire Harmonia graphical interface.
 */
class HarmoniaAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    /**
     * @brief Creates the plugin editor.
     *
     * Initializes:
     * - The root application controller
     * - The MIDI keyboard component
     * - Default window dimensions
     *
     * @param p Reference to the audio processor.
     */
    HarmoniaAudioProcessorEditor(HarmoniaAudioProcessor& p);

    /**
     * @brief Destructor.
     */
    ~HarmoniaAudioProcessorEditor() override;

    /**
     * @brief Paints the editor background.
     *
     * @param g Graphics context.
     */
    void paint(juce::Graphics& g) override;

    /**
     * @brief Updates child component layout.
     */
    void resized() override;

private:
    /** Reference to the audio processor instance. */
    HarmoniaAudioProcessor& audioProcessor;

    /**
     * Root application controller.
     *
     * Handles:
     * - Navigation
     * - Authentication screens
     * - Main application interface
     */
    std::unique_ptr<AppController> appController;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(HarmoniaAudioProcessorEditor)
};