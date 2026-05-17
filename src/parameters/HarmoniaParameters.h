#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>

namespace HarmoniaParams
{
    namespace IDs
    {
        static const juce::String osc1Waveform     = "osc_1_waveform";
        static const juce::String osc2Waveform     = "osc_2_waveform";
        static const juce::String oscMix           = "osc_mix";
        static const juce::String osc2Detune       = "osc_2_detune";
        static const juce::String noiseLevel       = "noise_level";

        static const juce::String filterCutoff     = "filter_cutoff";
        static const juce::String filterResonance  = "filter_resonance";
        static const juce::String filterType       = "filter_type";

        static const juce::String ampAttack        = "amp_attack";
        static const juce::String ampDecay         = "amp_decay";
        static const juce::String ampSustain       = "amp_sustain";
        static const juce::String ampRelease       = "amp_release";
        static const juce::String filterEnvAmount  = "filter_env_amount";
        static const juce::String filterEnvDecay   = "filter_env_decay";

        static const juce::String lfoRate          = "lfo_rate";
        static const juce::String lfoToPitch       = "lfo_to_pitch";
        static const juce::String lfoToCutoff      = "lfo_to_cutoff";
        static const juce::String velocityToFilter = "velocity_to_filter";
        static const juce::String distortionMix    = "distortion_mix";
        static const juce::String reverbMix        = "reverb_mix";
    }

    inline const std::array<juce::String, 20>& orderedIds()
    {
        static const std::array<juce::String, 20> list = {
            IDs::osc1Waveform, IDs::osc2Waveform, IDs::oscMix, IDs::osc2Detune, IDs::noiseLevel,
            IDs::filterCutoff, IDs::filterResonance, IDs::filterType,
            IDs::ampAttack, IDs::ampDecay, IDs::ampSustain, IDs::ampRelease,
            IDs::filterEnvAmount, IDs::filterEnvDecay,
            IDs::lfoRate, IDs::lfoToPitch, IDs::lfoToCutoff,
            IDs::velocityToFilter, IDs::distortionMix, IDs::reverbMix
        };
        return list;
    }

    inline juce::NormalisableRange<float> makeLogRange (float minV, float maxV, float midpoint)
    {
        const float skew = std::log (0.5f) / std::log ((midpoint - minV) / (maxV - minV));
        return { minV, maxV, 0.0001f, skew };
    }

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

    struct AtomicRefs
    {
        std::atomic<float>* osc1Waveform     = nullptr;
        std::atomic<float>* osc2Waveform     = nullptr;
        std::atomic<float>* oscMix           = nullptr;
        std::atomic<float>* osc2Detune       = nullptr;
        std::atomic<float>* noiseLevel       = nullptr;

        std::atomic<float>* filterCutoff     = nullptr;
        std::atomic<float>* filterResonance  = nullptr;
        std::atomic<float>* filterType       = nullptr;

        std::atomic<float>* ampAttack        = nullptr;
        std::atomic<float>* ampDecay         = nullptr;
        std::atomic<float>* ampSustain       = nullptr;
        std::atomic<float>* ampRelease       = nullptr;
        std::atomic<float>* filterEnvAmount  = nullptr;
        std::atomic<float>* filterEnvDecay   = nullptr;

        std::atomic<float>* lfoRate          = nullptr;
        std::atomic<float>* lfoToPitch       = nullptr;
        std::atomic<float>* lfoToCutoff      = nullptr;
        std::atomic<float>* velocityToFilter = nullptr;
        std::atomic<float>* distortionMix    = nullptr;
        std::atomic<float>* reverbMix        = nullptr;

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
