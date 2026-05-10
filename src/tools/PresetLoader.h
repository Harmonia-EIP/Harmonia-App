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

    // Charge un fichier hel.json (schéma : metadata + parameters{name:value} + values[])
    // et applique les 20 valeurs normalisées sur l'APVTS.
    // Accepte deux formes :
    //   1. parameters: { "osc_1_waveform": 0.5, ... }     (recommandée)
    //   2. values: [0.5, 0.3, ...]  (fallback ; ordre = HarmoniaParams::orderedIds())
    inline LoadResult loadFromJsonString (const juce::String& jsonText,
                                          juce::AudioProcessorValueTreeState& apvts)
    {
        LoadResult r;

        nlohmann::json doc;
        try { doc = nlohmann::json::parse (jsonText.toStdString()); }
        catch (const std::exception& e)
        {
            r.errorMessage = juce::String ("Invalid JSON: ") + e.what();
            return r;
        }

        if (doc.contains ("metadata") && doc["metadata"].contains ("name"))
            r.presetName = juce::String (doc["metadata"].value ("name", std::string("Preset")));

        auto applyNorm = [&apvts] (const juce::String& id, float normValue)
        {
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
            const size_t n  = std::min (arr.size(), ids.size());
            for (size_t i = 0; i < n; ++i)
                if (arr[i].is_number())
                    applyNorm (ids[i], (float) arr[i].get<double>());
            applied = n > 0;
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
                                    juce::AudioProcessorValueTreeState& apvts)
    {
        if (! file.existsAsFile())
            return { false, {}, "File not found" };
        return loadFromJsonString (file.loadFileAsString(), apvts);
    }

    // Sérialise l'état actuel de l'APVTS au format hel.json (avec parameters + values).
    // Utilisé par le bouton "Save preset".
    inline juce::String saveToJsonString (juce::AudioProcessorValueTreeState& apvts,
                                          const juce::String& presetName = "User Preset")
    {
        nlohmann::json doc;
        doc["metadata"] = {
            { "name", presetName.toStdString() },
            { "generated_by", "User" },
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
