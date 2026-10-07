#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <nlohmann/json.hpp>

namespace BackendPalette
{
    inline juce::String toHex (juce::Colour c)
    {
        return "#" + c.toDisplayString (false);   // "#RRGGBB"
    }

    inline juce::Colour fromHex (const juce::String& hex)
    {
        return juce::Colour::fromString ("FF" + hex.trimCharactersAtStart ("#"));
    }

    // Lit "palette" ([{id, color}, ...]) et "last_color_id" (1..3) d'une réponse backend.
    // Côté front le slot est 0..2 (= last_color_id - 1).
    // Ne touche à rien si le champ est absent ou invalide.
    inline void readFromJson (const nlohmann::json& body, juce::StringArray& colours, int& slot)
    {
        if (body.contains ("palette") && body["palette"].is_array())
        {
            std::string slots[3];
            int found = 0;

            for (const auto& item : body["palette"])
            {
                if (! item.is_object())
                    continue;

                const int id = item.value ("id", 0);

                if (id >= 1 && id <= 3 && item.contains ("color") && item["color"].is_string())
                {
                    slots[id - 1] = item["color"].get<std::string>();
                    ++found;
                }
            }

            if (found == 3)
            {
                colours.clear();
                for (const auto& s : slots)
                    colours.add (s);
            }
        }

        if (body.contains ("last_color_id") && body["last_color_id"].is_number_integer())
            slot = juce::jlimit (0, 2, body["last_color_id"].get<int>() - 1);
    }
}