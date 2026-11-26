#pragma once

#include <algorithm>
#include <cmath>

/**
 * Étage de sortie du plugin (C++ pur, sans JUCE) : volume puis limiteur doux.
 *
 * Le limiteur est transparent sous -1 dBFS ; au-dessus, il arrondit les crêtes
 * (courbe tanh) sans jamais dépasser 0 dBFS. Il protège des presets générés ou
 * aléatoires trop forts (unisson, résonance, distorsion empilés), pas un outil de son.
 */
namespace harmonia::output
{
    inline constexpr float limiterThreshold = 0.891f;   // -1 dBFS
    inline constexpr float silenceDb        = -60.0f;

    inline float dbToGain (float db) noexcept
    {
        return db <= silenceDb ? 0.0f : std::pow (10.0f, db / 20.0f);
    }

    inline float softLimit (float x) noexcept
    {
        const float a = std::abs (x);

        if (a <= limiterThreshold)
            return x;

        constexpr float knee = 1.0f - limiterThreshold;
        const float y = limiterThreshold + knee * std::tanh ((a - limiterThreshold) / knee);

        return std::copysign (y, x);
    }
}
