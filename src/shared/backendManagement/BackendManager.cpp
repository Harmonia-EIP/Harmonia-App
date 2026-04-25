#include "BackendManager.h"
#include "BackendAuthManager.h"
#include "BackendAiManager.h"
#include "BackendProfileManager.h"

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


juce::String BackendManager::loadConfig()
{
    auto dir = getAppDataDir();
    auto configFile = dir.getChildFile("config.json");

    writeLog("CONFIG PATH = " + configFile.getFullPathName());

    if (!configFile.existsAsFile())
    {
        json defaultConfig{
            { "api_url", "http://127.0.0.1:8000" }
        };

        configFile.replaceWithText(defaultConfig.dump(4));
        writeLog("config.json créé automatiquement");

        return "http://127.0.0.1:8000";
    }

    try
    {
        auto content = configFile.loadFileAsString();
        auto j = json::parse(content.toStdString());

        if (j.contains("api_url"))
            return juce::String(j["api_url"].get<std::string>());
    }
    catch (...)
    {
        writeLog("Erreur parsing config.json");
    }

    return "http://127.0.0.1:8000";
}


BackendManager::BackendManager()
{
    // API
    apiUrl = loadConfig();
    writeLog("API_URL = " + apiUrl);

    // SESSION FILE
    auto dir = getAppDataDir();
    sessionFile = dir.getChildFile("HarmoniaSession.json");

    writeLog("SESSION PATH = " + sessionFile.getFullPathName());

    // LOG FILE
    logFile = dir.getChildFile("HarmoniaLogs.txt");


    authManager    = std::make_unique<BackendAuthManager>(*this);
    aiManager      = std::make_unique<BackendAiManager>(*this);
    profileManager = std::make_unique<BackendProfileManager>(*this);
}


BackendManager::~BackendManager()
{
    authManager.reset();
    aiManager.reset();
    profileManager.reset();
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
// AI
PatchCallResult BackendManager::generatePatch(const juce::String& prompt)
{
    if (aiManager)
        return aiManager->generatePatch(prompt);

    return PatchCallResult::error("AI manager not initialized");
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