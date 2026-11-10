#pragma once

#include <juce_core/juce_core.h>

#include <optional>
#include <string>
#include <vector>

#include "../BackendTypes.h"

class HttpClient;
class SessionStore;
class BackendLogger;

struct PaletteState
{
    std::vector<std::string> colours;
    int         slot   = 0;
    bool        valid  = false;
    int         userId = 0;
    std::string token;
};

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
    ~ProfileService();

    ProfileResult getProfile();
    
    static std::optional<ProfileSnapshot> fetchAndStoreProfile (const HttpClient&, SessionStore&,
                                                            BackendLogger&, const UserSession&);

    void refreshSessionAsync (const UserSession& session,
                            std::function<void (ProfileSnapshot)> onDone);

    /** DELETE d'une entrée. Refuse les invités. true = supprimée côté serveur. */
    bool deleteHistoryEntry (int entryId);

    /** DELETE de tout l'historique. Refuse les invités. true = vidé côté serveur. */
    bool clearHistory();

    bool touchHistoryEntry (int entryId);

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

    bool waitUnlessCancelled (uint64_t ticket, int totalMs);

    bool deleteHistoryRequest (const juce::String& path, const juce::String& what);

    // Ex-globaux du .cpp : maintenant propres à CETTE instance (règle aussi le problème multi-plugins)
    std::mutex            paletteMutex;
    PaletteState          lastSent;
    PaletteState          pending;
    std::atomic<uint64_t> paletteTicket { 0 };

    // DOIT être déclaré EN DERNIER : détruit en premier, avant les membres qu'il utilise
    juce::ThreadPool pool { 2 };
};
