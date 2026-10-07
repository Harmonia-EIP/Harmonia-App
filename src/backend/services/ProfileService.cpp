#include "ProfileService.h"

#include "../core/BackendLogger.h"
#include "../core/BackendStrings.h"
#include "../core/JsonUtil.h"
#include "../net/ApiError.h"
#include "../net/HttpClient.h"
#include "../storage/PaletteMapper.h"
#include "../storage/SessionMapper.h"
#include "../storage/SessionStore.h"

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

namespace
{
    constexpr int paletteDebounceMs = 2500;   // attente sans nouveau changement avant le PUT

    // Etat de palette comparable (copie par valeur, aucune dépendance JUCE
    // pour rester sûr dans les threads détachés).
    struct PaletteState
    {
        std::vector<std::string> colours;
        int         slot   = 0;
        bool        valid  = false;
        int         userId = 0;
        std::string token;
    };

    bool samePalette (const PaletteState& a, const PaletteState& b)
    {
        return a.valid && b.valid && a.slot == b.slot && a.colours == b.colours;
    }

    std::vector<std::string> toVector (const juce::StringArray& colours)
    {
        std::vector<std::string> out;
        for (int i = 0; i < juce::jmin (colours.size(), 3); ++i)
            out.push_back (colours[i].toUpperCase().toStdString());
        return out;
    }

    // TODO étape 7 : ces globaux sont partagés entre toutes les instances du plugin
    // dans le même DAW. Ils deviendront des membres de PaletteSyncService.
    std::mutex                paletteMutex;
    PaletteState              lastSent;   // dernier état connu du serveur (login ou dernier PUT réussi)
    PaletteState              pending;    // dernier changement demandé, pas encore envoyé
    std::atomic<uint64_t>     paletteTicket { 0 };   // chaque changement/flush invalide les attentes en cours

    // Attend `totalMs`, par tranches de 50 ms, et abandonne dès que le ticket
    // n'est plus le dernier : un thread annulé s'arrête vite au lieu de rester
    // endormi plusieurs secondes.
    bool waitUnlessCancelled (uint64_t ticket, int totalMs)
    {
        for (int waited = 0; waited < totalMs; waited += 50)
        {
            if (paletteTicket.load() != ticket)
                return false;

            std::this_thread::sleep_for (std::chrono::milliseconds (50));
        }

        return paletteTicket.load() == ticket;
    }
}

ProfileService::ProfileService (HttpClient& h, SessionStore& s, BackendLogger& l)
    : http (h), store (s), logger (l)
{
}

//==============================================================================
// Profil

ProfileResult ProfileService::getProfile()
{
    const auto sessionOpt = store.load();

    if (! sessionOpt.has_value() || sessionOpt->isGuest)
        return ProfileResult::error (Strings::Errors::NoUserConnected.toStdString());

    const auto response = http.get ("/profile/me",
                                    HttpOptions (HttpTimeouts::Profile).withToken (sessionOpt->accessToken));

    logger.info ("GET /profile/me : " + response.describe());

#if JUCE_DEBUG
    logger.info ("GET /profile/me raw response: " + JsonUtil::toJuce (response.body));
#endif

    if (! response.isSuccess())
        return ProfileResult::error (parseApiError (response).message.toStdString());

    const auto body = JsonUtil::parseObject (response.body);

    if (! body.has_value())
        return ProfileResult::error (BackendStrings::InvalidServerResponse.toStdString());

    UserProfile profile;
    profile.id        = JsonUtil::getInt    (*body, "id", 0);
    profile.username  = JsonUtil::getString (*body, "username");
    profile.email     = JsonUtil::getString (*body, "email");
    profile.firstName = JsonUtil::getString (*body, "first_name");
    profile.lastName  = JsonUtil::getString (*body, "last_name");
    profile.createdAt = JsonUtil::getString (*body, "created_at");
    profile.role      = JsonUtil::getString (*body, "role");
    profile.isActive  = JsonUtil::getBool   (*body, "is_active", true);

    PaletteMapper::readFromJson (*body, profile.paletteColours, profile.paletteSlot);

    return ProfileResult::ok (profile);
}

//==============================================================================
// Sync de la session (remplace syncProfileParams / ...InBackground / ...FromServer)

std::optional<UserSession> ProfileService::fetchAndStoreProfile (const HttpClient& http,
                                                                 SessionStore& store,
                                                                 BackendLogger& logger,
                                                                 const UserSession& session)
{
    if (session.isGuest || session.accessToken.isEmpty())
        return std::nullopt;

    const auto response = http.get ("/profile/me",
                                    HttpOptions (HttpTimeouts::Profile).withToken (session.accessToken));

    if (! response.isSuccess())
    {
        logger.warn ("Session sync failed: " + response.describe());
        return std::nullopt;
    }

    const auto body = JsonUtil::parseObject (response.body);

    if (! body.has_value())
    {
        logger.warn ("Session sync: invalid response");
        return std::nullopt;
    }

    // Lecture-modification-écriture atomique sur la session ACTUELLE du fichier
    // (pas sur la copie reçue en paramètre, qui peut être périmée) :
    //  - si l'utilisateur s'est déconnecté pendant la requête, il n'y a plus de
    //    session et rien n'est écrit (plus de "reconnexion fantôme") ;
    //  - si un autre compte s'est connecté entre-temps, le token diffère : on n'écrase pas ;
    //  - on ne perd pas un changement de palette écrit pendant ce temps.
    std::optional<UserSession> result;

    const bool written = store.update ([&] (UserSession& stored)
    {
        if (stored.accessToken != session.accessToken)
            return false;

        SessionMapper::applyProfileResponse (*body, stored);
        result = stored;
        return true;
    });

    if (! written)
    {
        logger.info ("Session sync ignored (session cleared or replaced meanwhile)");
        return std::nullopt;
    }

    logger.info ("Session synced from backend");
    return result;
}

std::optional<UserSession> ProfileService::refreshSession (const UserSession& session)
{
    return fetchAndStoreProfile (http, store, logger, session);
}

void ProfileService::refreshSessionAsync (const UserSession& session)
{
    if (session.isGuest || session.accessToken.isEmpty())
        return;

    // Tout est passé par valeur : le BackendManager (donc `this`, http, store, logger)
    // est détruit à chaque fermeture de l'éditeur, alors que ce thread peut encore tourner.
    // Le store et le logger sont des objets partagés du processus : en construire un
    // handle ici donne la même instance et la garde en vie tant que le thread tourne.
    // (Suppose que le store de BackendManager est bien le SharedResourcePointer par défaut.)
    const HttpClient httpCopy = http;

    std::thread ([httpCopy, session]()
    {
        try
        {
            juce::SharedResourcePointer<BackendLogger> sharedLogger;
            juce::SharedResourcePointer<SessionStore>  sharedStore;

            fetchAndStoreProfile (httpCopy, sharedStore.get(), sharedLogger.get(), session);
        }
        catch (...) {}   // un thread détaché ne doit jamais laisser remonter d'exception
    }).detach();
}

//==============================================================================
// Palette

ProfileResult ProfileService::putPaletteToServer (const HttpClient& http,
                                                  int userId,
                                                  const std::string& token,
                                                  const std::vector<std::string>& colours,
                                                  int slot,
                                                  int timeoutMs)
{
    const auto body = PaletteMapper::toPutBody (colours, slot)
                          .dump (-1, ' ', false, nlohmann::json::error_handler_t::replace);

    const auto response = http.put ("/profile/" + juce::String (userId) + "/palette",
                                    body,
                                    HttpOptions (timeoutMs).withToken (juce::String (token)));

    if (! response.isSuccess())
    {
        std::string message = response.describe().toStdString();

        if (! response.body.empty())
            message += " : " + response.body;

        return ProfileResult::error (message);
    }

    ProfileResult result;
    result.success = true;
    return result;
}

void ProfileService::resetPaletteSyncState (const juce::StringArray& colours, int slot)
{
    ++paletteTicket;   // annule les attentes de debounce en cours

    std::lock_guard<std::mutex> lock (paletteMutex);

    pending  = PaletteState{};
    lastSent = PaletteState{};

    if (colours.size() == 3)
    {
        lastSent.colours = toVector (colours);
        lastSent.slot    = juce::jlimit (0, 2, slot);
        lastSent.valid   = true;
    }
}

void ProfileService::updatePaletteAsync (const juce::StringArray& colours, int slot)
{
    if (! PaletteMapper::isValidPalette (colours))
    {
        logger.warn ("updatePaletteAsync ignored: palette must have exactly 3 colours #RRGGBB");
        return;
    }

    // Invalide toute attente de debounce précédente
    const auto ticket = ++paletteTicket;

    PaletteState requested;

    // Cache local immédiat (la palette est restaurée au prochain démarrage même si
    // le DAW est tué avant le PUT) + récupération de userId/token, en une seule
    // opération atomique sur le store. Invités et "pas de session" : rien à faire.
    const bool accepted = store.update ([&] (UserSession& session)
    {
        if (session.isGuest)
            return false;

        session.paletteColours = colours;
        session.paletteSlot    = juce::jlimit (0, 2, slot);

        requested.colours = toVector (colours);
        requested.slot    = session.paletteSlot;
        requested.valid   = true;
        requested.userId  = session.userId;
        requested.token   = session.accessToken.toStdString();
        return true;
    });

    if (! accepted)
        return;

    {
        std::lock_guard<std::mutex> lock (paletteMutex);

        // Retour à l'état déjà connu du serveur : rien à envoyer, et on annule
        // l'éventuel changement en attente.
        if (samePalette (requested, lastSent))
        {
            pending.valid = false;
            return;
        }

        pending = requested;
    }

    const HttpClient httpCopy = http;

    std::thread ([httpCopy, ticket]()
    {
        try
        {
            // Debounce long : annulé si un changement plus récent (ou un flush) arrive
            if (! waitUnlessCancelled (ticket, paletteDebounceMs))
                return;

            PaletteState snap;
            {
                std::lock_guard<std::mutex> lock (paletteMutex);

                if (! pending.valid)
                    return;

                snap = pending;

                if (samePalette (snap, lastSent))
                {
                    pending.valid = false;
                    return;
                }
            }

            const auto result = putPaletteToServer (httpCopy, snap.userId, snap.token,
                                                    snap.colours, snap.slot, HttpTimeouts::Palette);

            if (result.success)
            {
                std::lock_guard<std::mutex> lock (paletteMutex);

                lastSent = snap;

                if (samePalette (pending, snap))
                    pending.valid = false;
            }
            else
            {
                // lastSent n'est PAS mis à jour et pending reste valide : le prochain
                // changement ou le flush de fermeture retentera.
                juce::SharedResourcePointer<BackendLogger> sharedLogger;
                sharedLogger->warn ("Palette update failed: " + juce::String (result.errorMessage));
            }
        }
        catch (...) {}
    }).detach();
}

void ProfileService::flushPaletteIfPending()
{
    // Annule le debounce en attente : c'est nous qui envoyons, maintenant
    ++paletteTicket;

    PaletteState snap;
    {
        std::lock_guard<std::mutex> lock (paletteMutex);

        if (! pending.valid)
            return;

        snap = pending;

        if (samePalette (snap, lastSent))
        {
            pending.valid = false;
            return;
        }

        // `pending` reste valide tant que le serveur n'a pas confirmé : si le PUT
        // échoue, le changement n'est pas perdu (ancien bug : il était effacé avant l'envoi).
    }

    // Tout est copié par valeur : le thread ne touche ni `this`, ni
    // BackendManager, ni le fichier de session (qu'un logout peut avoir vidé).
    const HttpClient httpCopy = http;

    std::thread ([httpCopy, snap]()
    {
        try
        {
            const auto result = putPaletteToServer (httpCopy, snap.userId, snap.token,
                                                    snap.colours, snap.slot, HttpTimeouts::PaletteFlush);

            if (result.success)
            {
                std::lock_guard<std::mutex> lock (paletteMutex);

                lastSent = snap;

                if (samePalette (pending, snap))
                    pending.valid = false;
            }
            else
            {
                juce::SharedResourcePointer<BackendLogger> sharedLogger;
                sharedLogger->warn ("Palette flush failed: " + juce::String (result.errorMessage));
            }
        }
        catch (...) {}
    }).detach();
}

ProfileResult ProfileService::updatePalette (const juce::StringArray& colours, int slot)
{
    if (! PaletteMapper::isValidPalette (colours))
        return ProfileResult::error (BackendStrings::InvalidPalette.toStdString());

    const auto sessionOpt = store.load();

    if (! sessionOpt.has_value() || sessionOpt->isGuest)
        return ProfileResult::error (Strings::Errors::NoUserConnected.toStdString());

    const auto cols = toVector (colours);

    auto result = putPaletteToServer (http, sessionOpt->userId,
                                      sessionOpt->accessToken.toStdString(),
                                      cols, slot, HttpTimeouts::Palette);

    if (result.success)
    {
        PaletteState sent;
        sent.colours = cols;
        sent.slot    = juce::jlimit (0, 2, slot);
        sent.valid   = true;

        std::lock_guard<std::mutex> lock (paletteMutex);
        lastSent = sent;
        pending.valid = false;
    }

    logger.info ("PUT palette : " + juce::String (result.success ? "ok" : "failed"));
    return result;
}
