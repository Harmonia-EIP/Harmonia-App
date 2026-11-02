#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_core/juce_core.h>
#include <nlohmann/json.hpp>
#include "../parameters/HarmoniaParameters.h"

namespace PresetLoader
{
    struct LoadResult
    {
        bool success = false;
        juce::String presetName;
        juce::String errorMessage;
    };

    // Applique une map { id -> valeur normalisée [0..1] } sur l'APVTS.
    // Les ids verrouillés ou inconnus sont ignorés.
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

        bool applied = false;

        for (const auto& [id, value] : parameters)
        {
            if (lockedIds.contains (id))
                continue;

            if (auto* p = apvts.getParameter (id))
            {
                p->beginChangeGesture();
                p->setValueNotifyingHost (value);
                p->endChangeGesture();
                applied = true;
            }
        }

        if (! applied)
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
    //   2. values: [0.5, 0.3, ...]  (fallback ; ordre = HarmoniaParams::orderedIds())
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

        bool applied = false;

        if (doc.contains ("parameters") && doc["parameters"].is_object())
        {
            for (auto it = doc["parameters"].begin(); it != doc["parameters"].end(); ++it)
            {
                if (! it.value().is_number())
                    continue;
                applyNorm (juce::String (it.key()), (float) it.value().get<double>());
                applied = true;
            }
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

            // Le preset est valide : appliquer toutes les valeurs.
            for (size_t i = 0; i < ids.size(); ++i)
                applyNorm (ids[i], static_cast<float> (arr[i].get<double>()));

            applied = true;
        }

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

    // Sérialise l'état actuel de l'APVTS au format hel.json (avec parameters + values).
    // Utilisé par le bouton "Save preset".
    inline juce::String saveToJsonString (juce::AudioProcessorValueTreeState& apvts,
                                          const juce::String& presetName, const juce::String& generatedBy)
    {
        nlohmann::json doc;
        doc["metadata"] = {
            { "name", presetName.toStdString() },
            { "generated_by", generatedBy.toStdString() },
            { "charter_version", "1.0" }
        };

        nlohmann::json paramsObj  = nlohmann::json::object();
        nlohmann::json valuesArr  = nlohmann::json::array();

        for (const auto& id : HarmoniaParams::orderedIds())
        {
            if (auto* p = apvts.getParameter (id))
            {
                const float v = p->getValue(); // normalisé [0..1]
                paramsObj[id.toStdString()] = v;
                valuesArr.push_back (v);
            }
        }

        doc["parameters"] = paramsObj;
        doc["values"]     = valuesArr;

        return juce::String (doc.dump (4));
    }
}