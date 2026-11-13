/**
 * @file HarmoniaParameters.h
 * @brief Centralized synthesizer parameter definitions and APVTS layout creation.
 *
 * This file defines every parameter used by the Harmonia synthesizer.
 *
 * Responsibilities:
 * - Declaring parameter identifiers
 * - Creating the AudioProcessorValueTreeState layout
 * - Providing ordered parameter access
 * - Defining logarithmic parameter ranges
 * - Providing fast atomic parameter references
 *
 * Main DSP Sections:
 * - Oscillators
 * - Noise generator
 * - Filter
 * - ADSR envelopes
 * - Filter envelope
 * - LFO modulation
 * - Velocity modulation
 * - Distortion
 * - Reverb
 *
 * Architecture:
 * IDs namespace:
 *     Contains all parameter string identifiers.
 *
 * createLayout():
 *     Builds the JUCE parameter tree.
 *
 * AtomicRefs:
 *     Stores raw atomic parameter pointers for
 *     real-time safe DSP access.
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

/**
 * @namespace HarmoniaParams
 * @brief Contains all synthesizer parameter utilities and definitions.
 */
namespace HarmoniaParams
{
    /**
     * @namespace IDs
     * @brief Contains all parameter identifier strings.
     *
     * These IDs are used by:
     * - AudioProcessorValueTreeState
     * - UI attachments
     * - DSP parameter access
     * - Automation systems
     * - Preset serialization
     *
     * Naming Convention:
     * - snake_case
     * - grouped by DSP section
     */
    namespace IDs
    {
        //==================================================
        // OSCILLATORS

        /**
         * @brief Oscillator 1 waveform selector.
         *
         * Available values:
         * - Sine
         * - Triangle
         * - Saw
         * - Square
         */
        static const juce::String osc1Waveform = "osc_1_waveform";

        /**
         * @brief Oscillator 2 waveform selector.
         */
        static const juce::String osc2Waveform = "osc_2_waveform";

        /**
         * @brief Oscillator mix amount.
         *
         * Range:
         * 0.0 = OSC1 only
         * 1.0 = OSC2 only
         */
        static const juce::String oscMix = "osc_mix";

        /**
         * @brief Oscillator 2 detune in cents.
         */
        static const juce::String osc2Detune = "osc_2_detune";

        /**
         * @brief White noise generator level.
         */
        static const juce::String noiseLevel = "noise_level";

        //==================================================
        // FILTER

        /**
         * @brief Filter cutoff frequency.
         *
         * Uses logarithmic scaling.
         */
        static const juce::String filterCutoff = "filter_cutoff";

        /**
         * @brief Filter resonance amount.
         */
        static const juce::String filterResonance = "filter_resonance";

        /**
         * @brief Filter type selector.
         *
         * Available types:
         * - Lowpass
         * - Bandpass
         * - Highpass
         */
        static const juce::String filterType = "filter_type";

        //==================================================
        // AMPLIFIER ENVELOPE

        /**
         * @brief ADSR attack time.
         */
        static const juce::String ampAttack = "amp_attack";

        /**
         * @brief ADSR decay time.
         */
        static const juce::String ampDecay = "amp_decay";

        /**
         * @brief ADSR sustain level.
         */
        static const juce::String ampSustain = "amp_sustain";

        /**
         * @brief ADSR release time.
         */
        static const juce::String ampRelease = "amp_release";

        //==================================================
        // FILTER ENVELOPE

        /**
         * @brief Filter envelope modulation amount.
         *
         * Bipolar range:
         * -1.0 to 1.0
         */
        static const juce::String filterEnvAmount = "filter_env_amount";

        /**
         * @brief Filter envelope decay time.
         */
        static const juce::String filterEnvDecay = "filter_env_decay";

        //==================================================
        // LFO + MODULATION

        /**
         * @brief LFO frequency.
         */
        static const juce::String lfoRate = "lfo_rate";

        /**
         * @brief LFO pitch modulation amount.
         */
        static const juce::String lfoToPitch = "lfo_to_pitch";

        /**
         * @brief LFO cutoff modulation amount.
         */
        static const juce::String lfoToCutoff = "lfo_to_cutoff";

        /**
         * @brief Velocity-to-filter modulation amount.
         */
        static const juce::String velocityToFilter = "velocity_to_filter";

        //==================================================
        // FX

        /**
         * @brief Distortion wet amount.
         */
        static const juce::String distortionMix = "distortion_mix";

        /**
         * @brief Reverb wet amount.
         */
        static const juce::String reverbMix = "reverb_mix";
    }

    /**
     * @brief Returns all parameter IDs in deterministic order.
     *
     * Useful for:
     * - Preset serialization
     * - Parameter iteration
     * - Automation export
     * - AI preset generation
     *
     * @return Ordered array of all parameter IDs.
     */
    inline const std::array<juce::String, 20>& orderedIds()
    {
        static const std::array<juce::String, 20> list =
        {
            IDs::osc1Waveform,
            IDs::osc2Waveform,
            IDs::oscMix,
            IDs::osc2Detune,
            IDs::noiseLevel,

            IDs::filterCutoff,
            IDs::filterResonance,
            IDs::filterType,

            IDs::ampAttack,
            IDs::ampDecay,
            IDs::ampSustain,
            IDs::ampRelease,

            IDs::filterEnvAmount,
            IDs::filterEnvDecay,

            IDs::lfoRate,
            IDs::lfoToPitch,
            IDs::lfoToCutoff,

            IDs::velocityToFilter,
            IDs::distortionMix,
            IDs::reverbMix
        };

        return list;
    }

    /**
     * @brief Creates a logarithmic parameter range.
     *
     * Used for perceptually logarithmic parameters:
     * - Frequencies
     * - Envelope times
     * - LFO rates
     *
     * JUCE internally uses a skew factor to
     * create non-linear parameter scaling.
     *
     * @param minV Minimum value.
     * @param maxV Maximum value.
     * @param midpoint Desired midpoint value.
     *
     * @return Configured logarithmic range.
     */
    inline juce::NormalisableRange<float> makeLogRange (float minV,
                                                        float maxV,
                                                        float midpoint)
    {
        const float skew =
            std::log (0.5f) /
            std::log ((midpoint - minV) / (maxV - minV));

        return { minV, maxV, 0.0001f, skew };
    }

    /**
     * @brief Creates the synthesizer parameter layout.
     *
     * Builds all APVTS parameters used by the synthesizer.
     *
     * Includes:
     * - Float parameters
     * - Choice parameters
     * - Labels
     * - Default values
     * - Parameter ranges
     * - Skew factors
     *
     * Parameter Categories:
     * - Oscillators
     * - Filter
     * - Envelopes
     * - Modulation
     * - Effects
     *
     * @return Fully initialized JUCE parameter layout.
     */
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using APF      = juce::AudioParameterFloat;
        using APC      = juce::AudioParameterChoice;
        using Range    = juce::NormalisableRange<float>;
        using ParamID  = juce::ParameterID;

        constexpr int versionHint = 1;

        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        const juce::StringArray waveforms { "Sine", "Triangle", "Saw", "Square" };
        const juce::StringArray filterTypes { "Lowpass", "Bandpass", "Highpass" };

        // Part1 Osc
        layout.add (std::make_unique<APC> (ParamID { IDs::osc1Waveform, versionHint },
                                           "Osc 1 Waveform", waveforms, 2)); // Saw
        layout.add (std::make_unique<APC> (ParamID { IDs::osc2Waveform, versionHint },
                                           "Osc 2 Waveform", waveforms, 1)); // Triangle
        layout.add (std::make_unique<APF> (ParamID { IDs::oscMix, versionHint },
                                           "Osc Mix",
                                           Range { 0.0f, 1.0f, 0.0001f }, 0.5f));
        layout.add (std::make_unique<APF> (ParamID { IDs::osc2Detune, versionHint },
                                           "Osc 2 Detune",
                                           Range { 0.0f, 100.0f, 0.01f }, 0.0f,
                                           juce::AudioParameterFloatAttributes().withLabel ("cents")));
        layout.add (std::make_unique<APF> (ParamID { IDs::noiseLevel, versionHint },
                                           "Noise Level",
                                           Range { 0.0f, 1.0f, 0.0001f }, 0.0f));

        // Part 2 Filters
        layout.add (std::make_unique<APF> (ParamID { IDs::filterCutoff, versionHint },
                                           "Filter Cutoff",
                                           makeLogRange (20.0f, 20000.0f, 1000.0f), 8000.0f,
                                           juce::AudioParameterFloatAttributes().withLabel ("Hz")));
        layout.add (std::make_unique<APF> (ParamID { IDs::filterResonance, versionHint },
                                           "Filter Resonance",
                                           Range { 0.0f, 0.95f, 0.0001f }, 0.1f));
        layout.add (std::make_unique<APC> (ParamID { IDs::filterType, versionHint },
                                           "Filter Type", filterTypes, 0));

        // Part 3 Enveloppess
        layout.add (std::make_unique<APF> (ParamID { IDs::ampAttack, versionHint },
                                           "Amp Attack",
                                           makeLogRange (1.0f, 5000.0f, 100.0f), 5.0f,
                                           juce::AudioParameterFloatAttributes().withLabel ("ms")));
        layout.add (std::make_unique<APF> (ParamID { IDs::ampDecay, versionHint },
                                           "Amp Decay",
                                           makeLogRange (10.0f, 5000.0f, 300.0f), 200.0f,
                                           juce::AudioParameterFloatAttributes().withLabel ("ms")));
        layout.add (std::make_unique<APF> (ParamID { IDs::ampSustain, versionHint },
                                           "Amp Sustain",
                                           Range { 0.0f, 1.0f, 0.0001f }, 0.8f));
        layout.add (std::make_unique<APF> (ParamID { IDs::ampRelease, versionHint },
                                           "Amp Release",
                                           makeLogRange (10.0f, 10000.0f, 500.0f), 400.0f,
                                           juce::AudioParameterFloatAttributes().withLabel ("ms")));
        layout.add (std::make_unique<APF> (ParamID { IDs::filterEnvAmount, versionHint },
                                           "Filter Env Amount",
                                           Range { -1.0f, 1.0f, 0.0001f }, 0.0f));
        layout.add (std::make_unique<APF> (ParamID { IDs::filterEnvDecay, versionHint },
                                           "Filter Env Decay",
                                           makeLogRange (10.0f, 5000.0f, 300.0f), 200.0f,
                                           juce::AudioParameterFloatAttributes().withLabel ("ms")));

        // Part 4 Lfos + modulations
        layout.add (std::make_unique<APF> (ParamID { IDs::lfoRate, versionHint },
                                           "LFO Rate",
                                           makeLogRange (0.1f, 20.0f, 2.0f), 4.0f,
                                           juce::AudioParameterFloatAttributes().withLabel ("Hz")));
        layout.add (std::make_unique<APF> (ParamID { IDs::lfoToPitch, versionHint },
                                           "LFO -> Pitch",
                                           Range { 0.0f, 1.0f, 0.0001f }, 0.0f));
        layout.add (std::make_unique<APF> (ParamID { IDs::lfoToCutoff, versionHint },
                                           "LFO -> Cutoff",
                                           Range { 0.0f, 1.0f, 0.0001f }, 0.0f));
        layout.add (std::make_unique<APF> (ParamID { IDs::velocityToFilter, versionHint },
                                           "Velocity -> Filter",
                                           Range { 0.0f, 1.0f, 0.0001f }, 0.0f));
        layout.add (std::make_unique<APF> (ParamID { IDs::distortionMix, versionHint },
                                           "Distortion",
                                           Range { 0.0f, 1.0f, 0.0001f, 1.8f }, 0.0f)); // skew>1 = expo
        layout.add (std::make_unique<APF> (ParamID { IDs::reverbMix, versionHint },
                                           "Reverb Mix",
                                           Range { 0.0f, 1.0f, 0.0001f }, 0.2f));

        return layout;
    }

    /**
     * @struct AtomicRefs
     * @brief Real-time safe parameter access structure.
     *
     * Stores raw atomic<float> pointers retrieved from
     * AudioProcessorValueTreeState.
     *
     * Benefits:
     * - Lock-free DSP access
     * - Real-time safe
     * - Faster than repeated APVTS lookups
     * - Minimal CPU overhead
     *
     * Used directly inside:
     * - Synth voices
     * - DSP rendering
     * - Modulation systems
     */
    struct AtomicRefs
    {
        //==================================================
        // OSCILLATORS

        std::atomic<float>* osc1Waveform     = nullptr;
        std::atomic<float>* osc2Waveform     = nullptr;
        std::atomic<float>* oscMix           = nullptr;
        std::atomic<float>* osc2Detune       = nullptr;
        std::atomic<float>* noiseLevel       = nullptr;

        //==================================================
        // FILTER

        std::atomic<float>* filterCutoff     = nullptr;
        std::atomic<float>* filterResonance  = nullptr;
        std::atomic<float>* filterType       = nullptr;

        //==================================================
        // ENVELOPES

        std::atomic<float>* ampAttack        = nullptr;
        std::atomic<float>* ampDecay         = nullptr;
        std::atomic<float>* ampSustain       = nullptr;
        std::atomic<float>* ampRelease       = nullptr;

        std::atomic<float>* filterEnvAmount  = nullptr;
        std::atomic<float>* filterEnvDecay   = nullptr;

        //==================================================
        // MODULATION

        std::atomic<float>* lfoRate          = nullptr;
        std::atomic<float>* lfoToPitch       = nullptr;
        std::atomic<float>* lfoToCutoff      = nullptr;
        std::atomic<float>* velocityToFilter = nullptr;

        //==================================================
        // FX

        std::atomic<float>* distortionMix    = nullptr;
        std::atomic<float>* reverbMix        = nullptr;

        /**
         * @brief Binds all atomic parameter pointers.
         *
         * Retrieves raw parameter pointers from the APVTS.
         *
         * Must be called before accessing parameters
         * inside the DSP engine.
         *
         * @param apvts AudioProcessorValueTreeState instance.
         */
        void bind (juce::AudioProcessorValueTreeState& apvts)
        {
            osc1Waveform     = apvts.getRawParameterValue (IDs::osc1Waveform);
            osc2Waveform     = apvts.getRawParameterValue (IDs::osc2Waveform);
            oscMix           = apvts.getRawParameterValue (IDs::oscMix);
            osc2Detune       = apvts.getRawParameterValue (IDs::osc2Detune);
            noiseLevel       = apvts.getRawParameterValue (IDs::noiseLevel);

            filterCutoff     = apvts.getRawParameterValue (IDs::filterCutoff);
            filterResonance  = apvts.getRawParameterValue (IDs::filterResonance);
            filterType       = apvts.getRawParameterValue (IDs::filterType);

            ampAttack        = apvts.getRawParameterValue (IDs::ampAttack);
            ampDecay         = apvts.getRawParameterValue (IDs::ampDecay);
            ampSustain       = apvts.getRawParameterValue (IDs::ampSustain);
            ampRelease       = apvts.getRawParameterValue (IDs::ampRelease);
            filterEnvAmount  = apvts.getRawParameterValue (IDs::filterEnvAmount);
            filterEnvDecay   = apvts.getRawParameterValue (IDs::filterEnvDecay);

            lfoRate          = apvts.getRawParameterValue (IDs::lfoRate);
            lfoToPitch       = apvts.getRawParameterValue (IDs::lfoToPitch);
            lfoToCutoff      = apvts.getRawParameterValue (IDs::lfoToCutoff);
            velocityToFilter = apvts.getRawParameterValue (IDs::velocityToFilter);
            distortionMix    = apvts.getRawParameterValue (IDs::distortionMix);
            reverbMix        = apvts.getRawParameterValue (IDs::reverbMix);
        }
    };
}