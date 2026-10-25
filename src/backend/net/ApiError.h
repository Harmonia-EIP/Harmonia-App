#pragma once

#include <juce_core/juce_core.h>

#include "../core/BackendStrings.h"
#include "../core/ErrorCodes.h"
#include "../core/JsonUtil.h"
#include "HttpClient.h"

/**
 * Erreur d'API déjà interprétée : un type + un code + un message affichable.
 * (Sera remplacée par core/Result.h à l'étape 6.)
 */
struct ApiError
{
    enum class Kind
    {
        Network,        // pas de réponse du serveur
        Unauthorized,   // session invalide (token) ou 401 sans code
        Validation,     // 422 / VALIDATION_ERROR
        Server,         // 5xx
        Http,           // autre code d'erreur
        Parse           // 2xx mais corps inutilisable
    };

    Kind kind = Kind::Http;
    juce::String code;      // code backend, vide si inconnu (pour la logique)
    juce::String message;   // affichable à l'utilisateur
    juce::String detail;    // texte technique du backend : LOGS UNIQUEMENT
    int statusCode = 0;
};

/**
 * Cas gérés : erreur réseau, error_code connu, validation (fields optionnel),
 * corps non JSON (HTML, vide), 401, 5xx.
 * À n'appeler que si !response.isSuccess().
 */
ApiError parseApiError (const HttpResponse& response);

/** Réponse 2xx mais inutilisable (JSON invalide, token absent...). */
ApiError makeInvalidResponseError (int statusCode);