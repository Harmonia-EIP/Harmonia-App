#include "BackendManager.h"

#include "services/AiService.h"
#include "services/AuthService.h"
#include "services/ProfileService.h"

BackendManager::BackendManager()
    : http (juce::String (AppConfig::ApiUrl))
{
    // Aucun accès disque ici (ni log) : un DAW instancie le plugin sans éditeur
    // pendant le scan. Les logs de démarrage sont écrits par AppController.

    // SharedResourcePointer::get() renvoie une référence : les services prennent des références.
    authService    = std::make_unique<AuthService>    (http, *sessionStore, *logger);
    profileService = std::make_unique<ProfileService> (http, *sessionStore, *logger);
    aiService      = std::make_unique<AiService>      (http, *sessionStore, *logger);
}

BackendManager::~BackendManager() = default;

//================================================
// AUTH
// Le sync du profil n'est plus lancé ici : il est déclenché par MainComponent
// (refreshProfileAsync), qui reçoit le résultat et le distribue à l'UI.

AuthResult BackendManager::loginUser (const juce::String& usernameOrEmail,
                                      const juce::String& password)
{
    return authService->loginUser (usernameOrEmail, password);
}

AuthResult BackendManager::signupUser (const juce::String& username,
                                       const juce::String& firstname,
                                       const juce::String& lastname,
                                       const juce::String& email,
                                       const juce::String& password)
{
    return authService->signupUser (username, firstname, lastname, email, password);
}

std::optional<UserSession> BackendManager::loadSession()
{
    return sessionStore->load();
}

void BackendManager::saveSession (const UserSession& session)
{
    sessionStore->save (session);
}

void BackendManager::clearSession()
{
    authService->logout();
}

//================================================
// AI : generate / refine retournent le JSON brut (format charter) pour le PresetLoader.

AiModelsResult BackendManager::fetchAiModels()
{
    return aiService->fetchModels();
}

AiResult BackendManager::generatePreset (const juce::String& prompt,
                                         const juce::String& backendName)
{
    return aiService->generatePreset (prompt, backendName);
}

AiResult BackendManager::refinePreset (const juce::String& prompt,
                                       const juce::String& currentJson,
                                       const juce::StringArray& lockedParamIds,
                                       const juce::String& backendName)
{
    return aiService->refinePreset (prompt, currentJson, lockedParamIds, backendName);
}

//================================================
// PROFILE

ProfileResult BackendManager::getProfile()
{
    return profileService->getProfile();
}

ProfileResult BackendManager::updatePalette (const juce::StringArray& colours, int slot)
{
    return profileService->updatePalette (colours, slot);
}

void BackendManager::updatePaletteAsync (const juce::StringArray& colours, int slot)
{
    profileService->updatePaletteAsync (colours, slot);
}

void BackendManager::flushPaletteIfPending()
{
    profileService->flushPaletteIfPending();
}

void BackendManager::resetPaletteSyncState (const juce::StringArray& colours, int slot)
{
    profileService->resetPaletteSyncState (colours, slot);
}

bool BackendManager::deleteHistoryEntry (int entryId)
{
    return profileService->deleteHistoryEntry (entryId);
}

bool BackendManager::clearHistory()
{
    return profileService->clearHistory();
}

bool BackendManager::touchHistoryEntry (int entryId)
{
    return profileService->touchHistoryEntry (entryId);
}

//================================================
// SYNC

// `onDone` est appelé sur un thread du pool : l'appelant doit repasser
// sur le message thread (SafePointer + MessageManager::callAsync).
void BackendManager::syncProfileParamsInBackground (const UserSession& session,
                                                    std::function<void (ProfileSnapshot)> onDone)
{
    profileService->refreshSessionAsync (session, std::move (onDone));
}

//================================================
// GETTERS / LOGS

const juce::String& BackendManager::getApiUrl() const
{
    return http.getBaseUrl();
}

const juce::File& BackendManager::getSessionFile() const
{
    return sessionStore->getSessionFile();
}

const juce::File& BackendManager::getLogFile() const
{
    return logger->getLogFile();
}

void BackendManager::writeLog (const juce::String& message) const
{
    logger->info (message);
}
