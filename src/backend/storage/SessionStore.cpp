#include "SessionStore.h"

#include "SessionMapper.h"
#include "../core/AppPaths.h"

SessionStore::SessionStore()
    : sessionFile (AppPaths::getSessionFile())
{
}

std::optional<UserSession> SessionStore::load()
{
    const std::lock_guard<std::mutex> lock (mutex);
    return loadLocked();
}

void SessionStore::save (const UserSession& session)
{
    const std::lock_guard<std::mutex> lock (mutex);

    if (writeLocked (session))
        logger->info ("Session saved");
}

void SessionStore::clear()
{
    const std::lock_guard<std::mutex> lock (mutex);

    if (sessionFile.existsAsFile() && ! sessionFile.deleteFile())
        logger->warn ("Could not delete the session file");

    cache.reset();
    cachedFileTime = juce::Time();
    cacheValid = false;

    logger->info ("Session cleared");
}

bool SessionStore::update (const std::function<bool (UserSession&)>& mutator)
{
    const std::lock_guard<std::mutex> lock (mutex);

    auto current = loadLocked();

    if (! current.has_value())
        return false;

    if (! mutator (*current))
        return false;

    return writeLocked (*current);
}

//==============================================================================
// Les fonctions ci-dessous sont appelées avec le mutex déjà pris.

std::optional<UserSession> SessionStore::loadLocked()
{
    if (! sessionFile.existsAsFile())
    {
        cache.reset();
        cachedFileTime = juce::Time();
        cacheValid = false;
        return std::nullopt;
    }

    // Une seule stat() par appel au lieu d'une lecture + parse : on ne relit
    // que si le fichier a été modifié (autre instance, autre processus, à la main).
    const auto modified = sessionFile.getLastModificationTime();

    if (! cacheValid || modified != cachedFileTime)
    {
        cache = readFromDisk();
        cachedFileTime = modified;
        cacheValid = true;
    }

    return cache;
}

std::optional<UserSession> SessionStore::readFromDisk()
{
    const auto text = sessionFile.loadFileAsString().toStdString();
    const auto json = nlohmann::json::parse (text, nullptr, false);

    if (json.is_discarded())
    {
        logger->warn ("Session file is corrupted, ignoring it");
        return std::nullopt;
    }

    auto session = SessionMapper::fromFileJson (json);

    if (! session.has_value())
        logger->info ("Session file contains no valid session");

    return session;
}

bool SessionStore::writeLocked (const UserSession& session)
{
    try
    {
        sessionFile.getParentDirectory().createDirectory();

        if (! sessionFile.replaceWithText (SessionMapper::toFileJson (session).dump (4)))
        {
            logger->error ("Could not write the session file");
            return false;
        }
    }
    catch (const std::exception& e)
    {
        logger->error ("Could not serialise the session: " + juce::String (e.what()));
        return false;
    }

    cache = session;
    cachedFileTime = sessionFile.getLastModificationTime();
    cacheValid = true;
    return true;
}
