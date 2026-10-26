#pragma once

#include <juce_core/juce_core.h>
#include <juce_graphics/juce_graphics.h>
#include <nlohmann/json.hpp>

#include <string>
#include <vector>

/**
 * Tout ce qui touche au format de la palette côté API est ICI et nulle part ailleurs :
 *  - lecture  : "palette" [{ id 1..3, color }] + "last_color_id" 1..3
 *  - écriture : "colors"  [{ id 1..3, color }] + "last_color_id" 1..3
 *  - conversion slot front 0..2 <-> id backend 1..3
 *
 * Remplace BackendPalette.h (qui reste en alias pour ne pas casser l'UI).
 */
namespace PaletteMapper
{
    inline juce::String toHex (juce::Colour c)
    {
        return "#" + c.toDisplayString (false);   // "#RRGGBB"
    }

    inline juce::Colour fromHex (const juce::String& hex)
    {
        return juce::Colour::fromString ("FF" + hex.trimCharactersAtStart ("#"));
    }

    /** "#RRGGBB" exactement. */
    inline bool isValidHex (const juce::String& s)
    {
        return s.length() == 7
            && s[0] == '#'
            && s.substring (1).containsOnly ("0123456789abcdefABCDEF");
    }

    /** Exactement 3 couleurs "#RRGGBB". */
    inline bool isValidPalette (const juce::StringArray& colours)
    {
        if (colours.size() != 3)
            return false;

        for (const auto& c : colours)
            if (! isValidHex (c))
                return false;

        return true;
    }

    /**
     * Lit "palette" et "last_color_id" d'une réponse backend.
     * Les 3 ids 1..3 doivent être présents (un doublon d'id ne compte pas pour deux).
     * Ne touche à rien si le champ est absent ou invalide.
     */
    inline void readFromJson (const nlohmann::json& body, juce::StringArray& colours, int& slot)
    {
        if (! body.is_object())
            return;

        const auto paletteIt = body.find ("palette");

        if (paletteIt != body.end() && paletteIt->is_array())
        {
            std::string slots[3];
            bool filled[3] = { false, false, false };

            for (const auto& item : *paletteIt)
            {
                if (! item.is_object())
                    continue;

                const auto idIt    = item.find ("id");
                const auto colorIt = item.find ("color");

                if (idIt == item.end() || ! idIt->is_number_integer()
                    || colorIt == item.end() || ! colorIt->is_string())
                    continue;

                const int id = idIt->get<int>();

                if (id >= 1 && id <= 3)
                {
                    slots[id - 1]  = colorIt->get<std::string>();
                    filled[id - 1] = true;
                }
            }

            if (filled[0] && filled[1] && filled[2])
            {
                colours.clear();

                for (const auto& s : slots)
                    colours.add (juce::String::fromUTF8 (s.data(), (int) s.size()));
            }
        }

        const auto slotIt = body.find ("last_color_id");

        if (slotIt != body.end() && slotIt->is_number_integer())
            slot = juce::jlimit (0, 2, slotIt->get<int>() - 1);
    }

    /** Corps du PUT /profile/{id}/palette. `slot` est 0..2 côté front. */
    inline nlohmann::json toPutBody (const std::vector<std::string>& colours, int slot)
    {
        nlohmann::json colors = nlohmann::json::array();

        for (size_t i = 0; i < colours.size() && i < 3; ++i)
            colors.push_back ({ { "id", (int) i + 1 }, { "color", colours[i] } });

        nlohmann::json body;
        body["colors"]        = colors;
        body["last_color_id"] = juce::jlimit (0, 2, slot) + 1;   // backend : 1..3
        return body;
    }
}
