#pragma once

#include <juce_core/juce_core.h>

#include <string>

/**
 * HttpClient : le SEUL endroit du projet qui connaît cpr.
 * (cpr/cpr.h n'est inclus que dans HttpClient.cpp.)
 */

namespace HttpTimeouts
{
    constexpr int Connect      = 5000;    // établissement de la connexion
    constexpr int Default      = 10000;
    constexpr int Auth         = 10000;   // signin / signup
    constexpr int Profile      = 10000;   // GET /profile/me
    constexpr int Palette      = 8000;    // PUT palette normal
    constexpr int PaletteFlush = 1500;    // PUT de fermeture : court, on ne bloque pas le DAW
    constexpr int Ai           = 60000;   // l'IA peut être lente, mais pas infini
}

struct HttpOptions
{
    juce::String bearerToken;                    // vide = pas de header Authorization
    int timeoutMs        = HttpTimeouts::Default; // durée totale max de la requête
    int connectTimeoutMs = HttpTimeouts::Connect;

    HttpOptions() = default;
    explicit HttpOptions (int totalTimeoutMs) : timeoutMs (totalTimeoutMs) {}

    HttpOptions& withToken (const juce::String& token) { bearerToken = token; return *this; }
};

struct HttpResponse
{
    int statusCode = 0;
    std::string body;

    bool networkError = false;           // pas d'Internet, DNS, timeout, serveur inaccessible...
    int networkErrorCode = 0;
    juce::String networkErrorMessage;

    /** Réponse 2xx, sans erreur réseau. */
    bool isSuccess() const noexcept
    {
        return ! networkError && statusCode >= 200 && statusCode < 300;
    }

    /** Pour les logs : "HTTP 401" ou "network error 28 - ...". Ne contient jamais le corps. */
    juce::String describe() const
    {
        if (networkError)
            return "network error " + juce::String (networkErrorCode) + " - " + networkErrorMessage;

        return "HTTP " + juce::String (statusCode);
    }
};

/**
 * Sans état modifiable : copiable, utilisable depuis n'importe quel thread.
 * Un thread de fond doit en recevoir une COPIE (pas une référence vers un
 * membre du BackendManager, qui peut être détruit entre-temps).
 */
class HttpClient
{
public:
    explicit HttpClient (const juce::String& baseUrl);

    HttpResponse get  (const juce::String& path, const HttpOptions& options = {}) const;
    HttpResponse post (const juce::String& path, const std::string& jsonBody, const HttpOptions& options = {}) const;
    HttpResponse put  (const juce::String& path, const std::string& jsonBody, const HttpOptions& options = {}) const;

    const juce::String& getBaseUrl() const noexcept { return baseUrl; }

private:
    juce::String baseUrl;
};
