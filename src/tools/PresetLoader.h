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

    // Applique des valeurs normalisées { id -> [0..1] } sur l'APVTS.
    // Seuls les ids présents dans `values` sont touchés : les ids inconnus (autre version du
    // modèle / du plugin) sont ignorés, et les paramètres absents du preset gardent leur valeur actuelle.
    // lockedIds : paramètres verrouillés, ils ne sont jamais modifiés.
    // Renvoie le nombre de paramètres du preset reconnus (verrouillés compris).
    // resetUnspecified : les paramètres absents de `values` (et non verrouillés) repartent à leur
    // défaut. "engine" n'est jamais réinitialisé : c'est à l'appelant (ou au preset) de le fixer.
    inline int applyValues (const std::map<juce::String, float>& values,
                            juce::AudioProcessorValueTreeState& apvts,
                            const juce::StringArray& lockedIds,
                            bool resetUnspecified = false)
    {
        int recognised = 0;
        bool hasDx7 = false;

        for (const auto& [id, value] : values)
        {
            if (apvts.getParameter (id) == nullptr)
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
            if (p == nullptr || lockedIds.contains (id))
                continue;

            p->beginChangeGesture();
            p->setValueNotifyingHost (juce::jlimit (0.0f, 1.0f, value));
            p->endChangeGesture();
        }

        return recognised;
    }

    // Charge une entrée de l'historique du backend : un nom et { id -> valeur normalisée [0..1] },
    // sans metadata. Les ids verrouillés ou inconnus sont ignorés ; une valeur hors de [0, 1]
    // refuse l'entrée entière.
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

        if (applyValues (parameters, apvts, lockedIds, resetUnspecified) == 0)
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

        applyValues (values, apvts, lockedIds, resetUnspecified);

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