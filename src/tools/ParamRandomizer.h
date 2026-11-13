#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <algorithm>
#include <cmath>
#include <cstddef>

#include "../parameters/HarmoniaParameters.h"

//==============================================================================
// Génère un patch aléatoire MAIS jouable.
//
// Les plages musicales (en unités réelles) sont écrites dans randomize(), mais
// TOUT passe par le charter :
//  - toute plage est rognée à [min, max] du charter (+ DBG si elle débordait)
//  - la valeur finale est arrondie par HarmoniaCharter::snap (int / choice / pas)
//  - le nombre de choix (waveforms, filter type) vient de numChoices
//  - la valeur "éteinte" des tirages sparse est le min du charter
//  - la normalisation (skew compris) est faite par le paramètre lui-même
//
// Si le charter change une borne ou une unité, rien ne casse : au pire une plage
// est rognée et tu as un message DBG qui te dit laquelle ajuster.
//
// Paramètres ignorés : `lockedIds` et `excludedIds`. Thread message uniquement.
// Ne touche PAS à "engine".
//==============================================================================
namespace ParamRandomizer
{
    namespace A = HarmoniaCharter::analog;

    // Les 20 premiers analogiques sont les historiques : on le vérifie.
    static_assert (HarmoniaCharter::v2Ids.size() == 20,
                   "Le randomizer suppose 20 parametres historiques");
    static_assert (HarmoniaCharter::v2Ids[0] == "osc_1_waveform"
                   && HarmoniaCharter::v2Ids[19] == "reverb_mix",
                   "Ordre v2 modifie : verifier le randomizer");

    inline constexpr std::size_t numLegacy = HarmoniaCharter::v2Ids.size();

    inline const HarmoniaCharter::Param& spec (A::Index i)
    {
        return HarmoniaCharter::params[HarmoniaCharter::analogBegin + (std::size_t) i];
    }

    inline int randomize (juce::AudioProcessorValueTreeState& apvts,
                          const juce::StringArray& lockedIds,
                          const juce::StringArray& excludedIds = {})
    {
        using namespace HarmoniaParams;

        juce::Random rng;
        int changed = 0;

        auto isFree = [&] (const juce::String& id)
        {
            return ! lockedIds.contains (id) && ! excludedIds.contains (id);
        };

        // Envoie une valeur réelle : snap par le charter, normalisation par le paramètre.
        auto setReal = [&] (A::Index idx, float value)
        {
            const auto id = IDs::idOf (idx);
            if (! isFree (id))
                return;

            if (auto* p = apvts.getParameter (id))
            {
                const auto& s = spec (idx);

                if (value < s.min || value > s.max)
                    DBG ("ParamRandomizer: " << id << " = " << value << " hors charter ["
                         << s.min << ", " << s.max << "] -> borne");

                const float v = HarmoniaCharter::snap (s, value);   // clamp + arrondi

                p->beginChangeGesture();
                p->setValueNotifyingHost (p->convertTo0to1 (v));
                p->endChangeGesture();
                ++changed;
            }
        };

        // Plage rognée au charter.
        auto clampLo = [] (A::Index i, float v) { return std::max (v, spec (i).min); };
        auto clampHi = [] (A::Index i, float v) { return std::min (v, spec (i).max); };

        auto lin = [&] (A::Index i, float lo, float hi)
        {
            lo = clampLo (i, lo);
            hi = clampHi (i, hi);
            return lo + (hi - lo) * rng.nextFloat();
        };

        // log : bias > 1 favorise les petites valeurs
        auto logv = [&] (A::Index i, float lo, float hi, float bias = 1.0f)
        {
            lo = std::max (clampLo (i, lo), 1.0e-3f);
            hi = clampHi (i, hi);
            if (hi <= lo)
                return lo;
            return lo * std::pow (hi / lo, std::pow (rng.nextFloat(), bias));
        };

        // `chanceOff` de rester au min du charter, sinon une valeur dans [lo, hi]
        auto sparse = [&] (A::Index i, float chanceOff, float lo, float hi)
        {
            return rng.nextFloat() < chanceOff ? spec (i).min : lin (i, lo, hi);
        };

        auto choice = [&] (A::Index i)
        {
            return (float) rng.nextInt (std::max (1, spec (i).numChoices));
        };

        // ---------- paramètres apparus en v3 : retour aux défauts ----------
        for (std::size_t i = numLegacy; i < (std::size_t) A::count; ++i)
        {
            const auto id = IDs::idOf ((A::Index) i);

            if (! isFree (id))
                continue;

            if (auto* p = apvts.getParameter (id))
            {
                p->beginChangeGesture();
                p->setValueNotifyingHost (p->getDefaultValue());
                p->endChangeGesture();
                ++changed;
            }
        }

        // ---------- filtre : le type d'abord, il conditionne le cutoff ----------
        int filterType = 0;

        if (isFree (IDs::filterType))
        {
            const float r = rng.nextFloat();
            filterType = r < 0.70f ? 0 : (r < 0.85f ? 1 : 2);   // surtout lowpass
            filterType = std::min (filterType, spec (A::filter_type).numChoices - 1);
            setReal (A::filter_type, (float) filterType);
        }
        else if (auto* raw = apvts.getRawParameterValue (IDs::filterType))
        {
            filterType = (int) raw->load();                      // verrouillé : on garde le sien
        }

        switch (filterType)
        {
            case 1:  setReal (A::filter_cutoff, logv (A::filter_cutoff, 300.0f, 6000.0f));  break;  // bandpass
            case 2:  setReal (A::filter_cutoff, logv (A::filter_cutoff, 60.0f,  2500.0f));  break;  // highpass
            default: setReal (A::filter_cutoff, logv (A::filter_cutoff, 400.0f, 14000.0f)); break;  // lowpass
        }

        setReal (A::filter_resonance, lin (A::filter_resonance, 0.05f, 0.70f));

        // ---------- oscillateurs ----------
        setReal (A::osc_1_waveform, choice (A::osc_1_waveform));
        setReal (A::osc_2_waveform, choice (A::osc_2_waveform));
        setReal (A::osc_mix,        lin (A::osc_mix, 0.10f, 0.90f));
        setReal (A::osc_2_detune,   sparse (A::osc_2_detune, 0.20f, 2.0f, 40.0f));    // cents
        setReal (A::noise_level,    sparse (A::noise_level, 0.70f, 0.02f, 0.20f));

        // ---------- enveloppe d'amplitude (ms) ----------
        setReal (A::amp_attack,  logv (A::amp_attack, 1.0f,  600.0f,  2.0f));   // plutôt rapide
        setReal (A::amp_decay,   logv (A::amp_decay,  60.0f, 2500.0f));
        setReal (A::amp_sustain, lin  (A::amp_sustain, 0.35f, 1.0f));            // jamais muet
        setReal (A::amp_release, logv (A::amp_release, 60.0f, 3000.0f, 1.3f));

        // ---------- enveloppe de filtre ----------
        setReal (A::filter_env_amount, lin  (A::filter_env_amount, -3.0f, 6.0f)); // octaves
        setReal (A::filter_env_decay,  logv (A::filter_env_decay, 60.0f, 2500.0f));

        // ---------- LFO / modulations ----------
        setReal (A::lfo_rate,           logv (A::lfo_rate, 0.2f, 12.0f));
        setReal (A::lfo_to_pitch,       sparse (A::lfo_to_pitch,       0.70f, 5.0f, 60.0f));  // cents
        setReal (A::lfo_to_cutoff,      sparse (A::lfo_to_cutoff,      0.50f, 0.2f, 2.0f));   // octaves
        setReal (A::velocity_to_filter, sparse (A::velocity_to_filter, 0.40f, 0.05f, 0.70f));

        // ---------- FX ----------
        setReal (A::distortion_mix, sparse (A::distortion_mix, 0.60f, 0.05f, 0.50f));
        setReal (A::reverb_mix,     lin (A::reverb_mix, 0.05f, 0.55f));

        return changed;
    }
}