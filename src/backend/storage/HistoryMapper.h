#pragma once
#include "../BackendTypes.h"
#include <nlohmann/json.hpp>

namespace HistoryMapper
{
    // { "osc_1_waveform": 0.5, ... } -> map
    inline std::map<juce::String, float> readParameters (const nlohmann::json& obj)
    {
        std::map<juce::String, float> out;

        if (! obj.is_object())
            return out;

        for (auto it = obj.begin(); it != obj.end(); ++it)
            if (it.value().is_number())
                out[juce::String (it.key())] = it.value().get<float>();

        return out;
    }

    // Une entrée d'historique : id, name, type, created_at, last_used_at,
    // et "parameters" s'il est présent (cas de GET /profile/me).
    inline HistoryEntry readEntry (const nlohmann::json& e)
    {
        HistoryEntry h;
        h.id         = e.value ("id", 0);
        h.name       = juce::String (e.value ("name", std::string()));
        h.type       = juce::String (e.value ("type", std::string()));
        h.createdAt  = juce::String (e.value ("created_at", std::string()));
        h.lastUsedAt = juce::String (e.value ("last_used_at", std::string()));

        if (e.contains ("parameters"))
            h.parameters = readParameters (e["parameters"]);

        return h;
    }

    inline std::vector<HistoryEntry> readFromJson (const nlohmann::json& body)
    {
        std::vector<HistoryEntry> out;

        if (! body.contains ("history") || ! body["history"].is_array())
            return out;

        for (const auto& e : body["history"])
            if (e.is_object())
                out.push_back (readEntry (e));

        return out;
    }
}