#pragma once

/**
 * SHIM DE COMPATIBILITÉ - à supprimer à l'étape 5.
 *
 * L'ancienne classe BackendAuthManager n'existe plus (remplacée par AuthService +
 * ProfileService, qui ne dépendent plus de BackendManager). Ce header ne garde
 * que les trois fonctions statiques que l'éditeur appelle peut-être directement
 * depuis un thread de fond (resync du profil au démarrage). Les chercher dans
 * l'UI avec l'inventaire de l'étape 0 : une fois migrées vers
 * BackendManager::syncProfileParams*(), supprimer ce fichier.
 */

#include <juce_core/juce_core.h>

#include <optional>

#include "BackendTypes.h"
#include "net/HttpClient.h"
#include "services/ProfileService.h"
#include "storage/SessionMapper.h"
#include "storage/SessionStore.h"

class BackendAuthManager
{
public:
    /** Lecture directe d'un fichier de session (ne passe pas par le cache du SessionStore). */
    static std::optional<UserSession> loadSessionFromFile (const juce::File& sessionFile)
    {
        if (! sessionFile.existsAsFile())
            return std::nullopt;

        const auto json = nlohmann::json::parse (sessionFile.loadFileAsString().toStdString(), nullptr, false);

        if (json.is_discarded())
            return std::nullopt;

        return SessionMapper::fromFileJson (json);
    }

    /** Écriture directe (le SessionStore détecte le changement via la date du fichier). */
    static void saveSessionToFile (const juce::File& sessionFile, const UserSession& session)
    {
        sessionFile.getParentDirectory().createDirectory();
        sessionFile.replaceWithText (SessionMapper::toFileJson (session).dump (4, ' ', false,
                                                                               nlohmann::json::error_handler_t::replace));
    }

    /**
     * GET /profile/me + mise à jour de la session.
     * Écrit dans le SessionStore partagé : `sessionFile` n'est plus utilisé
     * (c'est toujours le même fichier, AppPaths::getSessionFile()).
     */
    static std::optional<UserSession> syncProfileParamsFromServer (const juce::String& apiUrl,
                                                                   const juce::File& /*sessionFile*/,
                                                                   const UserSession& session)
    {
        juce::SharedResourcePointer<BackendLogger> logger;
        juce::SharedResourcePointer<SessionStore>  store;

        return ProfileService::fetchAndStoreProfile (HttpClient (apiUrl), store.get(), logger.get(), session);
    }
};
