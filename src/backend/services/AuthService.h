#pragma once

#include <juce_core/juce_core.h>

#include "../BackendTypes.h"

class HttpClient;
class SessionStore;
class BackendLogger;
struct HttpResponse;

/**
 * Authentification : login, signup, logout.
 *
 * Endpoints : POST /auth/signin, POST /auth/signup
 *
 * Ne dépend plus de BackendManager : il reçoit ses dépendances au constructeur.
 * La resynchronisation du profil après login est déclenchée par BackendManager
 * (elle vit maintenant dans ProfileService).
 */
class AuthService
{
public:
    AuthService (HttpClient& http, SessionStore& store, BackendLogger& logger);

    AuthResult loginUser (const juce::String& usernameOrEmail,
                          const juce::String& password);

    AuthResult signupUser (const juce::String& username,
                           const juce::String& firstname,
                           const juce::String& lastname,
                           const juce::String& email,
                           const juce::String& password);

    /** Supprime la session locale. Appeler flushPaletteIfPending() AVANT. */
    void logout();

private:
    AuthResult finishAuthentication (const HttpResponse& response, const juce::String& what);

    HttpClient&    http;
    SessionStore&  store;
    BackendLogger& logger;
};
