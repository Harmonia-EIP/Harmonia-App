#include "BackendProfileManager.h"
#include "BackendManager.h"
#include "BackendAuthManager.h"
#include "BackendPalette.h"
#include <cpr/cpr.h>

#include <atomic>
#include <chrono>
#include <mutex>
#include <thread>

using json = nlohmann::json;

namespace
{
    constexpr int paletteDebounceMs     = 2500;   // attente sans nouveau changement avant le PUT
    constexpr int paletteFlushTimeoutMs = 1500;   // PUT de fermeture : court, on ne bloque pas le DAW
    constexpr int palettePutTimeoutMs   = 8000;   // PUT normal

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

BackendProfileManager::BackendProfileManager(BackendManager& bm)
    : backend(bm)
{
}

ProfileResult BackendProfileManager::getProfile()
{
    ProfileResult result;

    auto sessionOpt = backend.loadSession();
    if (!sessionOpt.has_value())
    {
        result.success = false;
        result.errorMessage = Strings::Errors::NoUserConnected.toStdString();
        return result;
    }

    auto& session = sessionOpt.value();

    auto url = backend.getApiUrl() + "/profile/me";
    auto response = cpr::Get(
        cpr::Url{ url.toStdString() },
        cpr::Header{
            { "Authorization", "Bearer " + session.accessToken.toStdString() },
            { "Content-Type", "application/json" }
        }
    );

    backend.writeLog("GET /profile/me : " + juce::String(response.status_code));
    backend.writeLog("Réponse : " + juce::String(response.text.c_str()));

    if (response.status_code != 200)
    {
        result.success = false;
        try
        {
            auto body = json::parse(response.text);
            if (body.contains("detail"))
                result.errorMessage = body["detail"].get<std::string>();
            else
                result.errorMessage = Strings::Errors::UnknownError.toStdString();
        }
        catch (...)
        {
            result.errorMessage = Strings::Errors::UnknownError.toStdString();
        }
        return result;
    }

    try
    {
        auto body = json::parse(response.text);

        UserProfile profile;
        profile.id       = body.value("id", 0);
        profile.username = body.value("username", "");
        profile.email    = body.value("email", "");
        profile.firstName= body.value("first_name", "");
        profile.lastName = body.value("last_name", "");
        profile.createdAt= body.value("created_at", "");
        profile.role     = body.value("role", "");
        profile.isActive = body.value("is_active", true);

        BackendPalette::readFromJson(body, profile.paletteColours, profile.paletteSlot);

        result.success = true;
        result.profile = profile;
    }
    catch (...)
    {
        result.success = false;
        result.errorMessage = Strings::Errors::UnknownError.toStdString();
    }

    return result;
}

void BackendProfileManager::resetPaletteSyncState(const juce::StringArray& colours, int slot)
{
    ++paletteTicket;   // annule les attentes de debounce en cours

    std::lock_guard<std::mutex> lock(paletteMutex);

    pending = PaletteState{};
    lastSent = PaletteState{};

    if (colours.size() == 3)
    {
        lastSent.colours = toVector(colours);
        lastSent.slot    = juce::jlimit(0, 2, slot);
        lastSent.valid   = true;
    }
}

void BackendProfileManager::updatePaletteAsync(const juce::StringArray& colours, int slot)
{
    // Captured by value (not `this`/`backend`): the owning BackendManager is
    // destroyed every time the plugin editor window closes, but this thread
    // is detached and may still be in flight when that happens.
    const auto apiUrl      = backend.getApiUrl();
    const auto sessionFile = backend.getSessionFile();
    const auto logFile     = backend.getLogFile();

    // Invalide toute attente de debounce précédente
    const auto ticket = ++paletteTicket;

    PaletteState requested;

    try
    {
        auto sessionOpt = BackendAuthManager::loadSessionFromFile(sessionFile);
        if (!sessionOpt.has_value() || sessionOpt->isGuest)
            return;   // pas de compte : rien à synchroniser

        // Cache local immédiat : la palette restaurée au prochain démarrage,
        // même si le DAW est tué avant le PUT.
        auto session = sessionOpt.value();
        session.paletteColours = colours;
        session.paletteSlot    = slot;
        BackendAuthManager::saveSessionToFile(sessionFile, session);

        requested.colours = toVector(colours);
        requested.slot    = juce::jlimit(0, 2, slot);
        requested.valid   = true;
        requested.userId  = session.userId;
        requested.token   = session.accessToken.toStdString();
    }
    catch (...)
    {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(paletteMutex);

        // Retour à l'état déjà connu du serveur : rien à envoyer, et on annule
        // l'éventuel changement en attente.
        if (samePalette(requested, lastSent))
        {
            pending.valid = false;
            return;
        }

        pending = requested;
    }

    std::thread([apiUrl, logFile, ticket]()
    {
        try
        {
            // Debounce long : annulé si un changement plus récent (ou un flush) arrive
            if (!waitUnlessCancelled(ticket, paletteDebounceMs))
                return;

            PaletteState snap;
            {
                std::lock_guard<std::mutex> lock(paletteMutex);

                if (!pending.valid)
                    return;

                snap = pending;

                if (samePalette(snap, lastSent))
                {
                    pending.valid = false;
                    return;
                }
            }

            auto result = putPaletteToServer(apiUrl, snap.userId, snap.token,
                                             snap.colours, snap.slot, palettePutTimeoutMs);

            if (result.success)
            {
                std::lock_guard<std::mutex> lock(paletteMutex);

                lastSent = snap;

                if (samePalette(pending, snap))
                    pending.valid = false;
            }
            else if (logFile != juce::File())
            {
                // lastSent n'est PAS mis à jour : le prochain changement ou
                // le flush de fermeture retentera.
                logFile.appendText(
                    "[Backend] Erreur update palette : "
                    + juce::String(result.errorMessage) + "\n");
            }
        }
        catch (...) {}   // un thread détaché ne doit jamais laisser remonter d'exception
    }).detach();
}

void BackendProfileManager::flushPaletteIfPending()
{
    // Annule le debounce en attente : c'est nous qui envoyons, maintenant
    ++paletteTicket;

    PaletteState snap;
    {
        std::lock_guard<std::mutex> lock(paletteMutex);

        if (!pending.valid)
            return;

        snap = pending;
        pending.valid = false;

        if (samePalette(snap, lastSent))
            return;
    }

    // Tout est copié par valeur : le thread ne touche ni `this`, ni
    // BackendManager, ni le fichier de session (qu'un logout peut avoir vidé).
    const auto apiUrl  = backend.getApiUrl();
    const auto logFile = backend.getLogFile();

    std::thread([apiUrl, logFile, snap]()
    {
        try
        {
            auto result = putPaletteToServer(apiUrl, snap.userId, snap.token,
                                             snap.colours, snap.slot, paletteFlushTimeoutMs);

            if (result.success)
            {
                std::lock_guard<std::mutex> lock(paletteMutex);
                lastSent = snap;
            }
            else if (logFile != juce::File())
            {
                logFile.appendText(
                    "[Backend] Erreur flush palette : "
                    + juce::String(result.errorMessage) + "\n");
            }
        }
        catch (...) {}
    }).detach();
}

ProfileResult BackendProfileManager::updatePalette(const juce::StringArray& colours, int slot)
{
    ProfileResult result;

    auto sessionOpt = BackendAuthManager::loadSessionFromFile(backend.getSessionFile());
    if (!sessionOpt.has_value())
    {
        result.success = false;
        result.errorMessage = Strings::Errors::NoUserConnected.toStdString();
        return result;
    }

    const auto& session = sessionOpt.value();
    const auto cols = toVector(colours);

    result = putPaletteToServer(backend.getApiUrl(), session.userId,
                                session.accessToken.toStdString(),
                                cols, slot, palettePutTimeoutMs);

    if (result.success && cols.size() == 3)
    {
        PaletteState sent;
        sent.colours = cols;
        sent.slot    = juce::jlimit(0, 2, slot);
        sent.valid   = true;

        std::lock_guard<std::mutex> lock(paletteMutex);
        lastSent = sent;
        pending.valid = false;
    }

    backend.writeLog("PUT /palette : " + juce::String(result.success ? "200" : "error"));
    return result;
}

ProfileResult BackendProfileManager::putPaletteToServer(const juce::String& apiUrl,
                                                         int userId,
                                                         const std::string& token,
                                                         const std::vector<std::string>& colours,
                                                         int slot,
                                                         int timeoutMs)
{
    ProfileResult result;

    json colors = json::array();
    for (size_t i = 0; i < colours.size() && i < 3; ++i)
        colors.push_back({ { "id", (int) i + 1 }, { "color", colours[i] } });

    json body;
    body["colors"]        = colors;
    body["last_color_id"] = juce::jlimit(0, 2, slot) + 1;   // backend : 1..3

    auto url = apiUrl + "/profile/" + juce::String(userId) + "/palette";

    auto response = cpr::Put(
        cpr::Url{ url.toStdString() },
        cpr::Header{
            { "Authorization", "Bearer " + token },
            { "Content-Type", "application/json" }
        },
        cpr::Body{ body.dump() },
        cpr::Timeout{ timeoutMs }
    );

    if (response.status_code != 200)
    {
        result.success = false;
        result.errorMessage = "HTTP " + std::to_string(response.status_code)
                              + " : " + response.text;
        return result;
    }

    result.success = true;
    return result;
}