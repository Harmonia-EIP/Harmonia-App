/**
 * @file HarmoniaParameters.h
 * @brief Couche de compatibilité v1 -> v3.
 *
 * Expose l'API de la v1 (IDs::, orderedIds(), makeLogRange(), createLayout(),
 * AtomicRefs avec pointeurs nommés) au-dessus du charter v3 généré.
 *
 * Les IDs ne sont PAS écrits en dur : ils sont lus dans HarmoniaCharter::params
 * via l'enum HarmoniaCharter::analog::Index. Un static_assert vérifie à la
 * compilation que le nom attendu correspond bien à l'ID du charter.
 *
 * Pour ajouter un paramètre analogique : régénérer le charter, puis ajouter
 * une ligne X(nomCamelCase, id_snake_case) dans HARMONIA_ANALOG_LIST.
 *
 * NB : en v3 certaines valeurs ont changé d'unité (voir notes de migration) :
 *   filterEnvAmount : octaves (-10..10)   lfoToPitch : cents (0..1200)
 *   lfoToCutoff     : octaves (0..4)
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>
#include <cmath>

#include "HarmoniaCharterV3.h"
#include "HarmoniaParamsV3.h"

// Ordre = ordre de HarmoniaCharter::analog::Index (les 20 premiers = ordre v1).
#define HARMONIA_ANALOG_LIST(X)            \
    X(osc1Waveform,     osc_1_waveform)    \
    X(osc2Waveform,     osc_2_waveform)    \
    X(oscMix,           osc_mix)           \
    X(osc2Detune,       osc_2_detune)      \
    X(noiseLevel,       noise_level)       \
    X(filterCutoff,     filter_cutoff)     \
    X(filterResonance,  filter_resonance)  \
    X(filterType,       filter_type)       \
    X(ampAttack,        amp_attack)        \
    X(ampDecay,         amp_decay)         \
    X(ampSustain,       amp_sustain)       \
    X(ampRelease,       amp_release)       \
    X(filterEnvAmount,  filter_env_amount) \
    X(filterEnvDecay,   filter_env_decay)  \
    X(lfoRate,          lfo_rate)          \
    X(lfoToPitch,       lfo_to_pitch)      \
    X(lfoToCutoff,      lfo_to_cutoff)     \
    X(velocityToFilter, velocity_to_filter)\
    X(distortionMix,    distortion_mix)    \
    X(reverbMix,        reverb_mix)        \
    /* --- nouveaux en v3 --- */           \
    X(osc1Coarse,       osc_1_coarse)      \
    X(osc2Coarse,       osc_2_coarse)      \
    X(pulseWidth,       pulse_width)       \
    X(oscSync,          osc_sync)          \
    X(fmAmount,         fm_amount)         \
    X(ringMod,          ring_mod)          \
    X(unisonVoices,     unison_voices)     \
    X(unisonDetune,     unison_detune)     \
    X(filterSlope,      filter_slope)      \
    X(filterKeytrack,   filter_keytrack)   \
    X(filterEnvAttack,  filter_env_attack) \
    X(filterEnvSustain, filter_env_sustain)\
    X(filterEnvRelease, filter_env_release)\
    X(velocityToAmp,    velocity_to_amp)   \
    X(pitchEnvAmount,   pitch_env_amount)  \
    X(pitchEnvDecay,    pitch_env_decay)   \
    X(lfoWaveform,      lfo_waveform)      \
    X(lfoDelay,         lfo_delay)         \
    X(lfoToAmp,         lfo_to_amp)        \
    X(lfoToPw,          lfo_to_pw)         \
    X(chorusMix,        chorus_mix)        \
    X(delayTime,        delay_time)        \
    X(delayFeedback,    delay_feedback)    \
    X(delayMix,         delay_mix)         \
    X(reverbSize,       reverb_size)       \
    X(stereoWidth,      stereo_width)

// Garde-fous de compilation : l'ID du charter doit correspondre au nom attendu.
#define HARMONIA_CHECK_ID(name, key)                                                              \
    static_assert (HarmoniaCharter::params[HarmoniaCharter::analogBegin                           \
                                           + (std::size_t) HarmoniaCharter::analog::key].id == #key, \
                   "Charter v3: ordre ou ID modifie pour " #key);
HARMONIA_ANALOG_LIST (HARMONIA_CHECK_ID)
#undef HARMONIA_CHECK_ID

static_assert ((std::size_t) HarmoniaCharter::analog::count
                   == HarmoniaCharter::analogEnd - HarmoniaCharter::analogBegin,
               "analog::count ne correspond plus a la plage analogique du charter");

namespace HarmoniaParams
{
    namespace IDs
    {
        inline juce::String idOf (HarmoniaCharter::analog::Index i)
        {
            return HarmoniaParamsV3::toJuce (
                HarmoniaCharter::params[HarmoniaCharter::analogBegin + (std::size_t) i].id);
        }

        inline const juce::String engine = "engine";

       #define HARMONIA_DEFINE_ID(name, key) \
        inline const juce::String name = idOf (HarmoniaCharter::analog::key);
        HARMONIA_ANALOG_LIST (HARMONIA_DEFINE_ID)
       #undef HARMONIA_DEFINE_ID
    }

    /** Les 20 IDs de la v1, dans l'ordre v1 (compatible ancien code / presets IA v1). */
    inline const std::array<juce::String, 20>& orderedIds()
    {
        static const std::array<juce::String, 20> list = []
        {
            std::array<juce::String, 20> a;
            for (std::size_t i = 0; i < a.size(); ++i)
                a[i] = HarmoniaParamsV3::toJuce (HarmoniaCharter::v2Ids[i]);
            return a;
        }();
        return list;
    }

    /** Les 46 IDs analogiques de la v3, dans l'ordre du charter (= ordre d'un patch analogique). */
    inline const std::array<juce::String, HarmoniaCharter::analog::count>& orderedAnalogIds()
    {
        static const std::array<juce::String, HarmoniaCharter::analog::count> list = []
        {
            std::array<juce::String, HarmoniaCharter::analog::count> a;
            for (std::size_t i = 0; i < a.size(); ++i)
                a[i] = IDs::idOf ((HarmoniaCharter::analog::Index) i);
            return a;
        }();
        return list;
    }

    /** Identique a la v1 (le charter v3 precalcule deja ses skews avec la meme formule). */
    inline juce::NormalisableRange<float> makeLogRange (float minV, float maxV, float midpoint)
    {
        const float skew = std::log (0.5f) / std::log ((midpoint - minV) / (maxV - minV));
        return { minV, maxV, 0.0001f, skew };
    }

    /** Layout complet v3 (groupes par module et par operateur DX7). */
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        return HarmoniaParamsV3::createLayout();
    }

    /** Acces temps reel, pointeurs nommes comme en v1 : refs.filterCutoff->load(). */
    struct AtomicRefs
    {
        std::atomic<float>* engine = nullptr;

       #define HARMONIA_DECLARE_REF(name, key) std::atomic<float>* name = nullptr;
        HARMONIA_ANALOG_LIST (HARMONIA_DECLARE_REF)
       #undef HARMONIA_DECLARE_REF

        void bind (juce::AudioProcessorValueTreeState& apvts)
        {
            engine = apvts.getRawParameterValue (IDs::engine);
            jassert (engine != nullptr);

           #define HARMONIA_BIND_REF(name, key)                           \
            name = apvts.getRawParameterValue (IDs::name);                \
            jassert (name != nullptr);
            HARMONIA_ANALOG_LIST (HARMONIA_BIND_REF)
           #undef HARMONIA_BIND_REF
        }
    };

    /** Les refs indexees du wrapper v3 restent disponibles : refs.analog (HarmoniaCharter::analog::filter_cutoff). */
    using IndexedRefs = HarmoniaParamsV3::ParameterRefs;
}