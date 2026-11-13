/**
 * @file HarmoniaParameters.h
 * @brief Point d'entree unique des parametres Harmonia (46 analogiques + 145 DX7 + engine).
 *
 * Tout est lu dans HarmoniaCharter.h (genere) : IDs, plages, skews, defauts, groupes.
 * Ce fichier ne contient que la glue JUCE :
 *   - IDs::        noms CamelCase des parametres analogiques (+ engine)
 *   - orderedIds() les 20 IDs historiques, dans l'ordre v1
 *   - orderedAnalogIds() les 46 IDs analogiques, dans l'ordre du charter
 *   - createLayout() layout APVTS complet, groupes par module / operateur DX7
 *   - AtomicRefs   pointeurs atomiques pour le DSP (refs.filterCutoff->load())
 *
 * Les IDs ne sont PAS ecrits en dur : un static_assert verifie a la compilation que
 * chaque nom correspond bien a l'ID du charter.
 *
 * Pour ajouter un parametre analogique : regenerer le charter, puis ajouter une ligne
 * X(nomCamelCase, id_snake_case) dans HARMONIA_ANALOG_LIST (a la fin, dans l'ordre du charter).
 *
 * Chargement de presets : le charter fournit indexOf(id), snap(), fromNormalized() et
 * upgradeFromV2(); un preset incomplet (20 params au lieu de 46+) se charge en ne touchant
 * que les IDs presents, les autres gardent leur valeur par defaut.
 *
 * Unites v3 : filterEnvAmount en octaves (-10..10), lfoToPitch en cents (0..1200),
 *             lfoToCutoff en octaves (0..4).
 */

#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <array>
#include <atomic>
#include <cmath>
#include <memory>
#include <vector>

#include "HarmoniaCharter.h"

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
                   "Charter: ordre ou ID modifie pour " #key);
HARMONIA_ANALOG_LIST (HARMONIA_CHECK_ID)
#undef HARMONIA_CHECK_ID

static_assert ((std::size_t) HarmoniaCharter::analog::count
                   == HarmoniaCharter::analogEnd - HarmoniaCharter::analogBegin,
               "analog::count ne correspond plus a la plage analogique du charter");

namespace HarmoniaParams
{
    /** string_view (charter) -> juce::String, UTF-8. */
    inline juce::String toJuce (std::string_view s)
    {
        return juce::String::fromUTF8 (s.data(), (int) s.size());
    }

    namespace IDs
    {
        inline juce::String idOf (HarmoniaCharter::analog::Index i)
        {
            return toJuce (HarmoniaCharter::params[HarmoniaCharter::analogBegin + (std::size_t) i].id);
        }

        inline const juce::String engine = toJuce (HarmoniaCharter::params[0].id);

       #define HARMONIA_DEFINE_ID(name, key) \
        inline const juce::String name = idOf (HarmoniaCharter::analog::key);
        HARMONIA_ANALOG_LIST (HARMONIA_DEFINE_ID)
       #undef HARMONIA_DEFINE_ID
    }

    /** Les 20 IDs historiques, dans l'ordre v1 (anciens presets IA). */
    inline const std::array<juce::String, 20>& orderedIds()
    {
        static const std::array<juce::String, 20> list = []
        {
            std::array<juce::String, 20> a;
            for (std::size_t i = 0; i < a.size(); ++i)
                a[i] = toJuce (HarmoniaCharter::v2Ids[i]);
            return a;
        }();
        return list;
    }

    /** Les 46 IDs analogiques, dans l'ordre du charter (= ordre d'un patch analogique). */
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

    /** Plage logarithmique (meme formule que le charter pour ses skews). */
    inline juce::NormalisableRange<float> makeLogRange (float minV, float maxV, float midpoint)
    {
        const float skew = std::log (0.5f) / std::log ((midpoint - minV) / (maxV - minV));
        return { minV, maxV, 0.0001f, skew };
    }

    /** Layout APVTS complet, genere depuis le charter (un groupe par module / operateur DX7). */
    inline juce::AudioProcessorValueTreeState::ParameterLayout createLayout()
    {
        using namespace HarmoniaCharter;
        static constexpr int versionHint = 1;

        auto makeParam = [] (const Param& p) -> std::unique_ptr<juce::RangedAudioParameter>
        {
            const juce::ParameterID pid { toJuce (p.id), versionHint };
            const auto name = toJuce (p.label);
            const auto unit = toJuce (p.unit);
            const int  off  = p.displayOffset;

            switch (p.kind)
            {
                case Kind::Float:
                    return std::make_unique<juce::AudioParameterFloat> (
                        pid, name,
                        juce::NormalisableRange<float> { p.min, p.max, p.interval, p.skew },
                        p.defaultValue,
                        juce::AudioParameterFloatAttributes().withLabel (unit));

                case Kind::Int:
                    return std::make_unique<juce::AudioParameterInt> (
                        pid, name, (int) p.min, (int) p.max, (int) p.defaultValue,
                        juce::AudioParameterIntAttributes()
                            .withLabel (unit)
                            .withStringFromValueFunction ([off] (int v, int) { return juce::String (v + off); }));

                case Kind::Choice:
                {
                    juce::StringArray items;
                    for (int i = 0; i < p.numChoices; ++i)
                        items.add (toJuce (p.choices[i]));
                    return std::make_unique<juce::AudioParameterChoice> (pid, name, items, (int) p.defaultValue);
                }

                case Kind::Bool:
                {
                    const auto offText = toJuce (p.choices[0]);
                    const auto onText  = toJuce (p.choices[1]);
                    return std::make_unique<juce::AudioParameterBool> (
                        pid, name, p.defaultValue >= 0.5f,
                        juce::AudioParameterBoolAttributes()
                            .withStringFromValueFunction ([offText, onText] (bool v, int) { return v ? onText : offText; }));
                }
            }
            return nullptr;
        };

        juce::AudioProcessorValueTreeState::ParameterLayout layout;

        for (const auto& g : groups)
        {
            auto group = std::make_unique<juce::AudioProcessorParameterGroup> (toJuce (g.id), toJuce (g.label), "|");

            for (const auto& p : params)
                if (p.group == g.id)
                    group->addChild (makeParam (p));

            layout.add (std::move (group));
        }

        return layout;
    }

    /** Acces temps reel : pointeurs nommes (refs.filterCutoff->load()) ou indexes. */
    struct AtomicRefs
    {
        std::atomic<float>* engine = nullptr;

       #define HARMONIA_DECLARE_REF(name, key) std::atomic<float>* name = nullptr;
        HARMONIA_ANALOG_LIST (HARMONIA_DECLARE_REF)
       #undef HARMONIA_DECLARE_REF

        /** Index = HarmoniaCharter::analog::Index (ex: refs.analog[HarmoniaCharter::analog::filter_cutoff]). */
        std::array<std::atomic<float>*, HarmoniaCharter::analog::count> analog {};

        /** Index = (position dans HarmoniaCharter::params) - HarmoniaCharter::dx7Begin. */
        std::array<std::atomic<float>*, HarmoniaCharter::dx7End - HarmoniaCharter::dx7Begin> dx7 {};

        void bind (juce::AudioProcessorValueTreeState& apvts)
        {
            engine = apvts.getRawParameterValue (IDs::engine);
            jassert (engine != nullptr);

           #define HARMONIA_BIND_REF(name, key)                           \
            name = apvts.getRawParameterValue (IDs::name);                \
            jassert (name != nullptr);
            HARMONIA_ANALOG_LIST (HARMONIA_BIND_REF)
           #undef HARMONIA_BIND_REF

            for (std::size_t i = 0; i < analog.size(); ++i)
                analog[i] = apvts.getRawParameterValue (IDs::idOf ((HarmoniaCharter::analog::Index) i));

            for (std::size_t i = 0; i < dx7.size(); ++i)
                dx7[i] = apvts.getRawParameterValue (toJuce (HarmoniaCharter::params[HarmoniaCharter::dx7Begin + i].id));
        }
    };
}