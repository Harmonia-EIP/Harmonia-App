#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include "../parameters/HarmoniaParameters.h"

namespace PresetLoader
{
    struct LoadResult
    {
        bool success = false;
        juce::String presetName;
        juce::String errorMessage;
    };

    //==========================================================================
    // Presets v1 / v2
    //
    // Les modèles charter_v1 / harmonia_v2 (et les anciens fichiers, et leurs entrées
    // d'historique) renvoient les 20 paramètres historiques normalisés sur les plages v2
    // (metadata.charter_version "1.0"). Appliqués tels quels sur les plages v3, ils
    // donnent un autre son : on les convertit avec les règles de la charte.

    // Vrai si le preset suit la charte v3 (metadata.charter_version "3...").
    inline bool isCharterV3 (const nlohmann::json& doc)
    {
        if (! doc.contains ("metadata") || ! doc["metadata"].is_object())
            return false;

        const auto& m = doc["metadata"];
        return m.contains ("charter_version") && m["charter_version"].is_string()
            && m["charter_version"].get<std::string>().rfind ("3", 0) == 0;
    }

    // Vrai si les valeurs contiennent un paramètre apparu en v3 (engine, unison_voices, dx7_...) :
    // seul indice quand le preset arrive sans metadata (entrée d'historique du backend).
    // Un preset v1/v2 n'a que les 20 paramètres historiques ; un id inconnu ne compte pas.
    inline bool hasV3Parameters (const std::map<juce::String, float>& values)
    {
        for (const auto& entry : values)
        {
            const auto id = entry.first.toStdString();

            if (HarmoniaCharter::indexOf (id) >= 0
                && std::find (HarmoniaCharter::v2Ids.begin(), HarmoniaCharter::v2Ids.end(), id)
                       == HarmoniaCharter::v2Ids.end())
                return true;
        }

        return false;
    }

    // Valeurs v2 normalisées -> valeurs v3 normalisées, par les règles de la charte
    // (HarmoniaCharter::upgradeFromV2Normalized) :
    //  - paramètre avec une règle v2 : converti, s'il vient d'un paramètre présent dans le preset
    //  - paramètre apparu en v3 (pas de règle) : sa valeur neutre, pour garder le son du preset
    //  - moteur : analogique (un preset v2 ne connaît pas le DX7)
    inline std::map<juce::String, float> upgradeV2Values (const std::map<juce::String, float>& values)
    {
        std::array<float, HarmoniaCharter::v2Ids.size()> v2 {};
        std::array<bool,  HarmoniaCharter::v2Ids.size()> present {};

        for (size_t i = 0; i < v2.size(); ++i)
        {
            const auto it = values.find (HarmoniaParams::toJuce (HarmoniaCharter::v2Ids[i]));
            present[i] = it != values.end();
            v2[i] = present[i] ? juce::jlimit (0.0f, 1.0f, it->second) : 0.0f;
        }

        // Aucun paramètre reconnu : rien à appliquer (l'appelant refuse le preset)
        if (std::none_of (present.begin(), present.end(), [] (bool b) { return b; }))
            return {};

        std::map<juce::String, float> out;

        for (size_t i = HarmoniaCharter::analogBegin; i < HarmoniaCharter::analogEnd; ++i)
        {
            const auto& spec = HarmoniaCharter::params[i];
            const auto id = HarmoniaParams::toJuce (spec.id);

            if (spec.v2.source < 0)
                out[id] = HarmoniaCharter::toNormalized (spec, spec.defaultValue);
            else if (present[(size_t) spec.v2.source])
                out[id] = HarmoniaCharter::toNormalized (spec, HarmoniaCharter::upgradeFromV2Normalized (spec, v2));
            // source absente du preset : le paramètre n'est pas touché
        }

        out[HarmoniaParams::IDs::engine] = 0.0f;
        return out;
    }

    // Applique des valeurs normalisées { id -> [0..1] } sur l'APVTS.
    // Seuls les ids présents dans `values` sont touchés : les ids inconnus (autre version du
    // modèle / du plugin) sont ignorés, et les paramètres absents du preset gardent leur valeur actuelle.
    // lockedIds : paramètres verrouillés, ils ne sont jamais modifiés.
    // Renvoie le nombre de paramètres du preset reconnus (verrouillés compris).
    // resetUnspecified : les paramètres absents de `values` (et non verrouillés) repartent à leur
    // défaut. "engine" n'est jamais réinitialisé : c'est à l'appelant (ou au preset) de le fixer.
    // Seuls les paramètres de la charte font partie d'un preset : les autres (volume de sortie,
    // HarmoniaParams::Output) ne sont JAMAIS touchés, même si un fichier, l'IA ou l'historique les envoie.
    inline bool isPresetParameter (const juce::String& id)
    {
        return HarmoniaCharter::indexOf (id.toStdString()) >= 0;
    }

    inline int applyValues (const std::map<juce::String, float>& values,
                            juce::AudioProcessorValueTreeState& apvts,
                            const juce::StringArray& lockedIds,
                            bool resetUnspecified = false)
    {
        int recognised = 0;
        bool hasDx7 = false;

        for (const auto& [id, value] : values)
        {
            if (! isPresetParameter (id) || apvts.getParameter (id) == nullptr)
                continue;

            ++recognised;
            if (id.startsWith ("dx7_"))
                hasDx7 = true;
        }

        if (recognised == 0)
            return 0;

        if (resetUnspecified)
        {
            for (const auto& spec : HarmoniaCharter::params)
            {
                const auto id = HarmoniaParams::toJuce (spec.id);

                if (id == HarmoniaParams::IDs::engine   // géré plus bas
                    || lockedIds.contains (id)
                    || values.count (id) > 0)
                    continue;

                if (auto* p = apvts.getParameter (id))
                {
                    p->beginChangeGesture();
                    p->setValueNotifyingHost (p->getDefaultValue());
                    p->endChangeGesture();
                }
            }

            // Le preset ne dit pas quel moteur : on le déduit de ses paramètres.
            if (values.count (HarmoniaParams::IDs::engine) == 0)
            {
                if (auto* e = apvts.getParameter (HarmoniaParams::IDs::engine))
                {
                    e->beginChangeGesture();
                    e->setValueNotifyingHost (hasDx7 ? 1.0f : 0.0f);
                    e->endChangeGesture();
                }
            }
        }

        for (const auto& [id, value] : values)
        {
            auto* p = apvts.getParameter (id);
            if (p == nullptr || lockedIds.contains (id) || ! isPresetParameter (id))
                continue;

            p->beginChangeGesture();
            p->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, value));
            p->endChangeGesture();
        }

        return recognised;
    }

    // Charge une entrée de l'historique du backend : un nom et { id -> valeur normalisée [0..1] },
    // sans metadata. Le format (v3, ou v1/v2 à convertir) se lit sur les ids (hasV3Parameters).
    // Les ids verrouillés ou inconnus sont ignorés ; une valeur hors de [0, 1] refuse l'entrée entière.
    inline LoadResult loadFromParameters (const std::map<juce::String, float>& parameters,
                                          const juce::String& presetName,
                                          juce::AudioProcessorValueTreeState& apvts,
                                          const juce::StringArray& lockedIds = {},
                                          bool resetUnspecified = true)
    {
        LoadResult r;

        // Valider avant de toucher au moindre paramètre
        for (const auto& [id, value] : parameters)
        {
            if (! std::isfinite (value) || value < 0.0f || value > 1.0f)
            {
                r.errorMessage = "Invalid preset: value out of range";
                return r;
            }
        }

        const auto values = hasV3Parameters (parameters) ? parameters : upgradeV2Values (parameters);

        if (applyValues (values, apvts, lockedIds, resetUnspecified) == 0)
        {
            r.errorMessage = "No matching parameters found in history entry";
            return r;
        }

        r.success = true;
        r.presetName = presetName.isNotEmpty() ? presetName : juce::String ("Preset");
        return r;
    }

    // Charge un fichier hel.json (schéma : metadata + parameters{name:value} + values[])
    // et applique les valeurs normalisées sur l'APVTS.
    // Accepte deux formes :
    //   1. parameters: { "osc_1_waveform": 0.5, ... }     (recommandée, nombre de paramètres libre)
    //   2. values: [0.5, 0.3, ...]  (fallback ; ordre = HarmoniaParams::orderedIds())
    // Un preset partiel est accepté : seuls les paramètres présents sont modifiés.
    // Un preset v3 (charter_version "3...", ou des paramètres apparus en v3) s'applique tel quel ;
    // un preset v1/v2 (modèle charter_v1, anciens fichiers, forme `values`) est converti.
    //
    // lockedIds : paramètres verrouillés, ils ne sont jamais modifiés.
    inline LoadResult loadFromJsonString (const juce::String& jsonText,
                                      juce::AudioProcessorValueTreeState& apvts,
                                      const juce::StringArray& lockedIds = {},
                                      bool resetUnspecified = false)
    {
        LoadResult r;

        nlohmann::json doc;
        try { doc = nlohmann::json::parse (jsonText.toStdString()); }
        catch (const std::exception& e)
        {
            r.errorMessage = juce::String ("Invalid JSON: ") + e.what();
            return r;
        }

        if (doc.contains ("metadata")
            && doc["metadata"].is_object()
            && doc["metadata"].contains ("name")
            && doc["metadata"]["name"].is_string())
        {
            r.presetName = juce::String (doc["metadata"]["name"].get<std::string>());
        }

        // Valeurs normalisées du preset, par identifiant.
        std::map<juce::String, float> values;

        if (doc.contains ("parameters") && doc["parameters"].is_object())
        {
            for (auto it = doc["parameters"].begin(); it != doc["parameters"].end(); ++it)
                if (it.value().is_number())
                    values[juce::String (it.key())] = (float) it.value().get<double>();
        }
        else if (doc.contains ("values") && doc["values"].is_array())
        {
            const auto& ids = HarmoniaParams::orderedIds();
            const auto& arr = doc["values"];

            // Le tableau doit correspondre exactement aux paramètres attendus.
            if (arr.size() != ids.size())
            {
                r.errorMessage = "Invalid preset: incorrect number of values";
                return r;
            }

            // Valider toutes les valeurs avant de modifier le moindre paramètre.
            for (const auto& value : arr)
            {
                if (! value.is_number())
                {
                    r.errorMessage = "Invalid preset: 'values' must contain only numbers";
                    return r;
                }

                const double number = value.get<double>();

                if (! std::isfinite (number) || number < 0.0 || number > 1.0)
                {
                    r.errorMessage = "Invalid preset: value out of range";
                    return r;
                }
            }

            for (size_t i = 0; i < ids.size(); ++i)
                values[ids[i]] = static_cast<float> (arr[i].get<double>());
        }

        if (values.empty())
        {
            r.errorMessage = "JSON contains neither 'parameters' object nor 'values' array";
            return r;
        }

        const bool v3 = isCharterV3 (doc) || hasV3Parameters (values);
        applyValues (v3 ? values : upgradeV2Values (values), apvts, lockedIds, resetUnspecified);

        r.success = true;
        if (r.presetName.isEmpty())
            r.presetName = "Preset";
        return r;
    }

    inline LoadResult loadFromFile (const juce::File& file,
                                    juce::AudioProcessorValueTreeState& apvts,
                                    const juce::StringArray& lockedIds = {})
    {
        if (! file.existsAsFile())
            return { false, {}, "File not found" };
        return loadFromJsonString (file.loadFileAsString(), apvts, lockedIds);
    }

    // Sérialise l'état actuel de l'APVTS au format hel.json : tous les paramètres, normalisés.
    // Utilisé par le bouton "Save preset" et par l'affinage (preset courant).
    inline juce::String saveToJsonString (juce::AudioProcessorValueTreeState& apvts,
                                          const juce::String& presetName, const juce::String& generatedBy)
    {
        const bool dx7 = apvts.getRawParameterValue ("engine") != nullptr
                      && apvts.getRawParameterValue ("engine")->load() > 0.5f;

        nlohmann::json doc;
        doc["metadata"] = {
            { "name", presetName.toStdString() },
            { "generated_by", generatedBy.toStdString() },
            { "charter_version", std::string (HarmoniaCharter::version) },
            { "engine", dx7 ? "dx7" : "analog" }
        };

        nlohmann::json paramsObj = nlohmann::json::object();
        for (const auto& spec : HarmoniaCharter::params)
            if (auto* p = apvts.getParameter (HarmoniaParams::toJuce (spec.id)))
                paramsObj[std::string (spec.id)] = p->getValue(); // normalisé [0..1]

        doc["parameters"] = paramsObj;
        return juce::String (doc.dump (4));
    }
}