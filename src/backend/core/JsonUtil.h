#pragma once

#include <juce_core/juce_core.h>
#include <nlohmann/json.hpp>

#include <optional>
#include <string>

/**
 * Accès JSON qui ne lèvent jamais d'exception.
 *
 * `json::value("clé", défaut)` lève une type_error si la clé existe mais avec
 * un autre type (ex. "token": null). Dans un plugin, une exception non
 * attrapée = crash du DAW, donc tout passe par ces helpers.
 */
namespace JsonUtil
{
    using json = nlohmann::json;

    /** Parse sans exception. nullopt si le texte n'est pas du JSON ou n'est pas un objet. */
    inline std::optional<json> parseObject (const std::string& text)
    {
        auto parsed = json::parse (text, nullptr, false);

        if (parsed.is_discarded() || ! parsed.is_object())
            return std::nullopt;

        return parsed;
    }

    inline std::string getString (const json& j, const char* key, const std::string& fallback = {})
    {
        if (j.is_object())
        {
            const auto it = j.find (key);

            if (it != j.end() && it->is_string())
                return it->get<std::string>();
        }

        return fallback;
    }

    inline int getInt (const json& j, const char* key, int fallback = 0)
    {
        if (j.is_object())
        {
            const auto it = j.find (key);

            if (it != j.end() && it->is_number_integer())
                return it->get<int>();
        }

        return fallback;
    }

    inline juce::int64 getInt64 (const json& j, const char* key, juce::int64 fallback = 0)
    {
        if (j.is_object())
        {
            const auto it = j.find (key);

            if (it != j.end() && it->is_number_integer())
                return (juce::int64) it->get<long long>();
        }

        return fallback;
    }

    inline bool getBool (const json& j, const char* key, bool fallback = false)
    {
        if (j.is_object())
        {
            const auto it = j.find (key);

            if (it != j.end() && it->is_boolean())
                return it->get<bool>();
        }

        return fallback;
    }

    /** std::string (UTF-8) -> juce::String, en précisant l'encodage. */
    inline juce::String toJuce (const std::string& s)
    {
        return juce::String::fromUTF8 (s.data(), (int) s.size());
    }
}
