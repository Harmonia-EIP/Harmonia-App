#include "BackendManager.h"
#include "BackendAuthManager.h"
#include "BackendProfileManager.h"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


juce::File BackendManager::getAppDataDir() const
{
    auto appData = juce::File::getSpecialLocation(
        juce::File::userApplicationDataDirectory);

    auto harmoniaDir = appData.getChildFile("Harmonia");

    if (!harmoniaDir.exists())
        harmoniaDir.createDirectory();

    return harmoniaDir;
}


BackendManager::BackendManager()
{
    apiUrl = AppConfig::ApiUrl;
    writeLog("API_URL = " + apiUrl);

    auto dir = getAppDataDir();
    sessionFile = dir.getChildFile("HarmoniaSession.json");
    writeLog("SESSION PATH = " + sessionFile.getFullPathName());

    logFile = dir.getChildFile("HarmoniaLogs.txt");

    authManager    = std::make_unique<BackendAuthManager>(*this);
    profileManager = std::make_unique<BackendProfileManager>(*this);
    aiManager = std::make_unique<BackendAiManager>(*this);
}


BackendManager::~BackendManager()
{
    authManager.reset();
    profileManager.reset();
    aiManager.reset();
}


void BackendManager::writeLog(const juce::String& message) const
{
    if (logFile != juce::File())
        logFile.appendText("[Backend] " + message + "\n");
}

//================================================
// AUTH
AuthResult BackendManager::loginUser(const juce::String& usernameOrEmail,
                                     const juce::String& password)
{
    return authManager->loginUser(usernameOrEmail, password);
}

AuthResult BackendManager::signupUser(const juce::String& username,
                                      const juce::String& firstname,
                                      const juce::String& lastname,
                                      const juce::String& email,
                                      const juce::String& password)
{
    return authManager->signupUser(username, firstname, lastname, email, password);
}

std::optional<UserSession> BackendManager::loadSession()
{
    return authManager->loadSession();
}

void BackendManager::saveSession(const UserSession& session)
{
    authManager->saveSession(session);
}

void BackendManager::clearSession()
{
    authManager->clearSession();
}

//================================================
// AI : retourne le JSON brut (format charter) pour que le caller le passe à PresetLoader.
AiResult BackendManager::generatePreset(const juce::String& prompt)
{
    return aiManager->generatePreset(prompt);
}

//================================================
// PROFILE
ProfileResult BackendManager::getProfile()
{
    if (profileManager)
        return profileManager->getProfile();

    return ProfileResult::error("Profile manager not initialized");
}

ProfileResult BackendManager::updateTheme(int themeId)
{
    if (profileManager)
        return profileManager->updateTheme(themeId);

    return ProfileResult::error("Profile manager not initialized");
}

ProfileResult BackendManager::updateLayout(int layoutId)
{
    if (profileManager)
        return profileManager->updateLayout(layoutId);

    return ProfileResult::error("Profile manager not initialized");
}

void BackendManager::updateLocalTheme(int themeId)
{
    if (profileManager)
        profileManager->updateLocalTheme(themeId);
}

void BackendManager::updateThemeAsync(int themeId)
{
    if (profileManager)
        profileManager->updateThemeAsync(themeId);
}

//================================================
// SYNC
void BackendManager::syncProfileParamsInBackground(const UserSession& session)
{
    authManager->syncProfileParamsInBackground(session);
}

std::optional<UserSession> BackendManager::syncProfileParams(const UserSession& session)
{
    return authManager->syncProfileParams(session);
}

//================================================
// GETTERS
const juce::String& BackendManager::getApiUrl() const
{
    return apiUrl;
}

const juce::File& BackendManager::getSessionFile() const
{
    return sessionFile;
}
