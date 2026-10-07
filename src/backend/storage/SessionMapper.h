#pragma once

#include <nlohmann/json.hpp>

#include <optional>

#include "../BackendTypes.h"

/**
 * Conversions UserSession <-> JSON, écrites UNE seule fois
 * (avant : copiées 4 fois entre login, signup et les deux syncs).
 */
namespace SessionMapper
{
    // TODO (étape 1.9, décision à prendre) : durée fournie par le serveur
    // (expires_in / exp du JWT) ou 1 h fixe. En attendant : 1 h fixe, comme avant.
    constexpr int sessionLifetimeHours = 1;

    /** Réponse de /auth/signin ou /auth/signup. nullopt si le token est absent, vide ou invalide. */
    std::optional<UserSession> fromAuthResponse (const nlohmann::json& body);

    /** Réponse de GET /profile/me appliquée sur une session existante (token et expiration inchangés). */
    void applyProfileResponse (const nlohmann::json& body, UserSession& session);

    /** Contenu du fichier de session. nullopt si invalide. */
    std::optional<UserSession> fromFileJson (const nlohmann::json& j);

    nlohmann::json toFileJson (const UserSession& session);
}
