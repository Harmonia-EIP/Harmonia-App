#pragma once

#include <juce_core/juce_core.h>

#include <functional>
#include <mutex>
#include <optional>

#include "../BackendTypes.h"
#include "../core/BackendLogger.h"

/**
 * Seul propriétaire du fichier de session.
 *
 *  - cache mémoire (le fichier n'est relu que s'il a changé sur le disque) ;
 *  - un mutex autour de TOUS les accès (fin des écrasements concurrents entre
 *    la sync de profil et le cache de palette) ;
 *  - update() fait lecture-modification-écriture de façon atomique ;
 *  - fichier corrompu : load() retourne nullopt, on logge, on ne plante pas ;
 *  - ne logge jamais le contenu du fichier.
 *
 * À utiliser via juce::SharedResourcePointer<SessionStore> : le fichier est
 * commun à toutes les instances du plugin, le store aussi.
 */
class SessionStore
{
public:
    SessionStore();

    std::optional<UserSession> load();

    void save (const UserSession& session);

    /** Supprime le fichier et le cache. */
    void clear();

    /**
     * Lecture-modification-écriture atomique.
     *
     * `mutator` reçoit la session actuelle et renvoie true pour l'écrire, false
     * pour annuler (ex. le token a changé entre-temps). Elle ne doit PAS appeler
     * le store (verrou déjà pris).
     *
     * @return true si la session a été écrite. false s'il n'y a pas de session
     *         (donc un logout ne peut jamais être "annulé" par une écriture tardive),
     *         si le mutator a refusé, ou si l'écriture a échoué.
     */
    bool update (const std::function<bool (UserSession&)>& mutator);

    const juce::File& getSessionFile() const noexcept { return sessionFile; }

private:
    std::optional<UserSession> loadLocked();
    std::optional<UserSession> readFromDisk();
    bool writeLocked (const UserSession& session);

    juce::SharedResourcePointer<BackendLogger> logger;   // garantit que le logger survit au store

    const juce::File sessionFile;

    std::mutex mutex;
    std::optional<UserSession> cache;
    juce::Time cachedFileTime;
    bool cacheValid = false;
};
