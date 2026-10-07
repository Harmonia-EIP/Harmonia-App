#pragma once

#include <juce_core/juce_core.h>

#include "HttpClient.h"

/**
 * Erreur d'API déjà interprétée : un type + un message affichable.
 * (Sera remplacée par core/Result.h à l'étape 6.)
 */
struct ApiError
{
    enum class Kind
    {
        Network,        // pas de réponse du serveur
        Unauthorized,   // 401
        Validation,     // 422 : detail en tableau (email invalide, mot de passe trop court...)
        Server,         // 5xx
        Http,           // autre code d'erreur
        Parse           // 2xx mais corps inutilisable
    };

    Kind kind = Kind::Http;
    juce::String message;   // affichable à l'utilisateur
    int statusCode = 0;
};

/**
 * Remplace tous les blocs `try { parse detail } catch` du code d'origine.
 * Cas gérés : erreur réseau, detail en chaîne, detail en tableau (validation),
 * corps non JSON (HTML, vide), 401, 5xx.
 * À n'appeler que si !response.isSuccess().
 */
ApiError parseApiError (const HttpResponse& response);

/** Réponse 2xx mais inutilisable (JSON invalide, token absent...). */
ApiError makeInvalidResponseError (int statusCode);
