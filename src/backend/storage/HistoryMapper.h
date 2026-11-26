#pragma once
#include "../BackendTypes.h"
#include "../core/JsonUtil.h"
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

    // Une entrée d'historique : id, name, type, model_id, model_name, created_at,
    // last_used_at, et "parameters" s'il est présent (cas de GET /profile/me).
    // Passe par JsonUtil : les champs nullables (model_id / model_name à null)
    // ne doivent pas lever d'exception.
    inline HistoryEntry readEntry (const nlohmann::json& e)
    {
        auto str = [&e] (const char* key) { return JsonUtil::toJuce (JsonUtil::getString (e, key)); };

        HistoryEntry h;
        h.id         = JsonUtil::getInt (e, "id");
        h.name       = str ("name");
        h.type       = str ("type");
        h.modelId    = str ("model_id");
        h.modelName  = str ("model_name");
        h.createdAt  = str ("created_at");
        h.lastUsedAt = str ("last_used_at");

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