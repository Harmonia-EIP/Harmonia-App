#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <nlohmann/json.hpp>
#include <algorithm>
#include <cmath>
#include <map>
#include "../parameters/HarmoniaParameters.h"
#include "../parameters/HarmoniaParametersV3.h"

namespace PresetLoader
{
    struct LoadResult
    {
        bool success = false;
        juce::String presetName;
        juce::String errorMessage;
    };

    // Vrai si le preset suit la charte v3 (metadata.charter_version "3...") : ses valeurs sont
    // normalisées sur les plages v3. Sinon c'est un preset v1/v2 (modèles v1/v2, anciens fichiers),
    // normalisé sur les 20 plages v2.
    inline bool isCharterV3 (const nlohmann::json& doc)
    {
        if (! doc.contains ("metadata") || ! doc["metadata"].is_object())
            return false;
        const auto& m = doc["metadata"];
        return m.contains ("charter_version") && m["charter_version"].is_string()
            && m["charter_version"].get<std::string>().rfind ("3", 0) == 0;
    }

    // Vrai si les valeurs contiennent un paramètre apparu en v3 (engine, unison_voices, dx7_...) : seul
    // indice quand le preset arrive sans metadata, comme une entrée d'historique du backend (un nom et
    // ses parameters). Un preset v1/v2 n'a que les 20 paramètres v2 ; un id inconnu ne compte pas.
    inline bool hasV3Parameters (const std::map<juce::String, float>& values)
    {
        for (const auto& entry : values)
        {
            const auto id = entry.first.toStdString();
            if (HarmoniaCharter::indexOf (id) >= 0
                && std::find (HarmoniaCharter::v2Ids.begin(), HarmoniaCharter::v2Ids.end(), id) == HarmoniaCharter::v2Ids.end())
                return true;
        }
        return false;
    }

    // Applique des valeurs normalisées { id -> [0..1] } sur l'APVTS. Un preset v3 s'applique tel quel.
    // Un preset v1/v2 (normalisé sur les 20 plages v2) passe par les règles de la charte
    // (HarmoniaCharter::upgradeFromV2) ; les paramètres apparus en v3 reprennent leur valeur neutre,
    // pour que le son reste celui du preset, et le moteur passe en analogique.
    // lockedIds : paramètres verrouillés, ils ne sont jamais modifiés.
    // Renvoie le nombre de paramètres du preset reconnus (verrouillés compris).
    inline int applyValues (const std::map<juce::String, float>& values, bool charterV3,
                            juce::AudioProcessorValueTreeState& apvts,
                            const juce::StringArray& lockedIds)
    {
        auto applyNorm = [&apvts, &lockedIds] (const juce::String& id, float normValue)
        {
            if (lockedIds.contains (id))
                return;   // verrouille : on ne touche pas

            if (auto* p = apvts.getParameter (id))
            {
                normValue = juce::jlimit (0.0f, 1.0f, normValue);
                p->beginChangeGesture();
                p->setValueNotifyingHost (normValue);
                p->endChangeGesture();
            }
        };

        int recognised = 0;

        if (charterV3)
        {
            for (const auto& [id, value] : values)
            {
                if (apvts.getParameter (id) != nullptr)
                    ++recognised;
                applyNorm (id, value);
            }
            return recognised;
        }

        // preset v1/v2 : valeurs normalisées sur les plages v2 -> valeurs physiques v3
        std::array<float, HarmoniaCharter::v2Ids.size()> v2 {};
        std::array<bool, HarmoniaCharter::v2Ids.size()> present {};
        for (size_t i = 0; i < v2.size(); ++i)
        {
            const auto it = values.find (HarmoniaParamsV3::toJuce (HarmoniaCharter::v2Ids[i]));
            present[i] = it != values.end();
            v2[i] = present[i] ? juce::jlimit (0.0f, 1.0f, it->second) : 0.0f;
            recognised += present[i] ? 1 : 0;
        }
        for (size_t i = HarmoniaCharter::analogBegin; i < HarmoniaCharter::analogEnd; ++i)
        {
            const auto& p = HarmoniaCharter::params[i];
            const auto id = HarmoniaParamsV3::toJuce (p.id);
            if (p.v2.source < 0)
                applyNorm (id, HarmoniaCharter::toNormalized (p, p.defaultValue));
            else if (present[(size_t) p.v2.source])
                applyNorm (id, HarmoniaCharter::toNormalized (p, HarmoniaCharter::upgradeFromV2Normalized (p, v2)));
            // absent du preset : le paramètre garde sa valeur
        }
        applyNorm ("engine", 0.0f);  // analogique
        return recognised;
    }

    // Charge une entrée de l'historique du backend : un nom et { id -> valeur normalisée [0..1] },
    // sans metadata. Le format (v3, ou v1/v2 à convertir) se lit sur les ids (hasV3Parameters).
    // Les ids verrouillés ou inconnus sont ignorés ; une valeur hors de [0, 1] refuse l'entrée entière.
    inline LoadResult loadFromParameters (const std::map<juce::String, float>& parameters,
                                          const juce::String& presetName,
                                          juce::AudioProcessorValueTreeState& apvts,
                                          const juce::StringArray& lockedIds = {})
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

        if (applyValues (parameters, hasV3Parameters (parameters), apvts, lockedIds) == 0)
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
    //   1. parameters: { "osc_1_waveform": 0.5, ... }     (recommandée)
    //   2. values: [0.5, 0.3, ...]  (fallback v2 ; ordre = HarmoniaParams::orderedIds())
    // Un preset v3 (metadata.charter_version "3...", ou des paramètres apparus en v3) s'applique
    // tel quel ; un preset v1/v2 est converti (voir applyValues).
    //
    // lockedIds : paramètres verrouillés, ils ne sont jamais modifiés.
    inline LoadResult loadFromJsonString (const juce::String& jsonText,
                                          juce::AudioProcessorValueTreeState& apvts,
                                          const juce::StringArray& lockedIds = {})
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
                if (!value.is_number())
                {
                    r.errorMessage = "Invalid preset: 'values' must contain only numbers";
                    return r;
                }

                const double number = value.get<double>();

                if (!std::isfinite(number)
                    || number < 0.0
                    || number > 1.0)
                {
                    r.errorMessage = "Invalid preset: value out of range";
                    return r;
                }
            }

            for (size_t i = 0; i < ids.size(); ++i)
                values[ids[i]] = static_cast<float> (arr[i].get<double>());
        }

        const bool applied = ! values.empty();

        if (applied)
            applyValues (values, isCharterV3 (doc) || hasV3Parameters (values), apvts, lockedIds);

        if (! applied)
        {
            r.errorMessage = "JSON contains neither 'parameters' object nor 'values' array";
            return r;
        }

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

    // Sérialise l'état actuel de l'APVTS au format hel.json, charte v3 : tous les paramètres,
    // normalisés. Utilisé par le bouton "Save preset" et par l'affinage (preset courant).
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
            if (auto* p = apvts.getParameter (HarmoniaParamsV3::toJuce (spec.id)))
                paramsObj[std::string (spec.id)] = p->getValue(); // normalisé [0..1]

        doc["parameters"] = paramsObj;
        return juce::String (doc.dump (4));
    }
}
