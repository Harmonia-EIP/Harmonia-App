#include "BackendManager.h"

#include "services/AiService.h"
#include "services/AuthService.h"
#include "services/ProfileService.h"

BackendManager::BackendManager()
    : http (juce::String (AppConfig::ApiUrl))
{
    // Le logger existe déjà ici : ces deux lignes sont enfin écrites
    // (avant, logFile était assigné après les premiers writeLog).
    logger->info ("API_URL = " + http.getBaseUrl());
    logger->info ("SESSION PATH = " + sessionStore->getSessionFile().getFullPathName());

    authService    = std::make_unique<AuthService>    (http, sessionStore.get(), logger.get());
    profileService = std::make_unique<ProfileService> (http, sessionStore.get(), logger.get());
    aiService      = std::make_unique<AiService>      (http, sessionStore.get(), logger.get());
}

BackendManager::~BackendManager() = default;

//================================================
// AUTH

AuthResult BackendManager::loginUser (const juce::String& usernameOrEmail,
                                      const juce::String& password)
{
    auto result = authService->loginUser (usernameOrEmail, password);

    if (result.success)
        profileService->refreshSessionAsync (result.session);

    return result;
}

AuthResult BackendManager::signupUser (const juce::String& username,
                                       const juce::String& firstname,
                                       const juce::String& lastname,
                                       const juce::String& email,
                                       const juce::String& password)
{
    auto result = authService->signupUser (username, firstname, lastname, email, password);

    if (result.success)
        profileService->refreshSessionAsync (result.session);

    return result;
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
// AI : retourne le JSON brut (format charter) pour que le caller le passe à PresetLoader.

AiResult BackendManager::generatePreset (const juce::String& prompt, int modelId, const juce::String& backendName)
{
    return aiService->generatePreset (prompt, modelId, backendName);
}

AiResult BackendManager::refinePreset (const juce::String& prompt,
                                       const juce::String& currentJson,
                                       const juce::StringArray& lockedParamIds,
                                       int modelId,
                                       const juce::String& backendName)
{
    return aiService->refinePreset (prompt, currentJson, lockedParamIds, modelId, backendName);
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

//================================================
// SYNC

void BackendManager::syncProfileParamsInBackground (const UserSession& session)
{
    profileService->refreshSessionAsync (session);
}

std::optional<UserSession> BackendManager::syncProfileParams (const UserSession& session)
{
    return profileService->refreshSession (session);
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
