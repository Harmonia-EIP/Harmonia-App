#pragma once

#include <juce_core/juce_core.h>

#include <optional>
#include <string>
#include <vector>

#include "../BackendTypes.h"

class HttpClient;
class SessionStore;
class BackendLogger;

/**
 * Profil utilisateur et synchronisation de la palette.
 *
 * Endpoints : GET /profile/me, PUT /profile/{id}/palette
 *
 * Reprend la synchronisation de profil qui vivait (en 3 variantes) dans
 * l'ancien BackendAuthManager : une seule implémentation, fetchAndStoreProfile().
 *
 * Stratégie de sync de la palette (inchangée) :
 *  - debounce long (2,5 s) : un seul PUT quand l'utilisateur arrête de toucher ;
 *  - filtre "inchangé" : pas de PUT si la palette == dernier état connu du serveur ;
 *  - flush non bloquant à la fermeture de l'éditeur / au logout ;
 *  - le serveur reste la source de vérité au démarrage.
 *
 * NOTE (étape 7, pas encore faite) : l'état de sync (lastSent, pending, ticket)
 * est toujours global au processus, donc partagé par toutes les instances du plugin.
 */
class ProfileService
{
public:
    ProfileService (HttpClient& http, SessionStore& store, BackendLogger& logger);

    ProfileResult getProfile();

    //==========================================================================
    // Sync de la session depuis le serveur (GET /profile/me)

    /** Bloquant. Retourne la session mise à jour, ou nullopt si échec / session changée entre-temps. */
    std::optional<UserSession> refreshSession (const UserSession& session);

    /** Même chose sur un thread de fond. Le résultat est écrit dans le SessionStore. */
    void refreshSessionAsync (const UserSession& session);

    /**
     * Implémentation sans état (utilisable depuis n'importe quel thread).
     * Public uniquement pour la compatibilité avec BackendAuthManager.h (shim) ;
     * à repasser en privé quand l'éditeur ne l'appelle plus directement.
     */
    static std::optional<UserSession> fetchAndStoreProfile (const HttpClient& http,
                                                            SessionStore& store,
                                                            BackendLogger& logger,
                                                            const UserSession& session);

    //==========================================================================
    // Palette

    /** Bloquant. Refuse les invités et toute palette qui n'a pas exactement 3 couleurs "#RRGGBB". */
    ProfileResult updatePalette (const juce::StringArray& colours, int slot);

    /** Met à jour le cache local tout de suite, envoie le PUT après 2,5 s sans changement. Invités ignorés. */
    void updatePaletteAsync (const juce::StringArray& colours, int slot);

    /** Envoie le changement en attente maintenant, sans bloquer. Appeler AVANT clearSession() au logout. */
    void flushPaletteIfPending();

    /** Définit l'état "connu du serveur" du filtre anti-PUT inutiles. Vide = invité. */
    void resetPaletteSyncState (const juce::StringArray& colours, int slot);

private:
    static ProfileResult putPaletteToServer (const HttpClient& http,
                                             int userId,
                                             const std::string& token,
                                             const std::vector<std::string>& colours,
                                             int slot,
                                             int timeoutMs);

    HttpClient&    http;
    SessionStore&  store;
    BackendLogger& logger;
};
