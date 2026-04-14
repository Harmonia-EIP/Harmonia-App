#include "BackendManager.h"
#include "BackendAuthManager.h"
#include "BackendAiManager.h"
#include "BackendProfileManager.h"

#include <sstream>

juce::File BackendManager::getExeDir()
{
    return juce::File::getSpecialLocation(juce::File::currentExecutableFile)
        .getParentDirectory();
}

std::map<std::string, std::string> BackendManager::loadEnv()
{
    std::map<std::string, std::string> envValues;

    auto exeDir  = getExeDir();
    auto appRoot = exeDir.getParentDirectory().getParentDirectory().getParentDirectory();                  // IMPORTANT: .env un niveau au-dessus
    auto envFile = appRoot.getChildFile(".env");

    if (!envFile.existsAsFile())
    {
        // pas de log ici (pas d'instance encore), on loguera dans le ctor si envValues est vide
        return envValues;
    }

    auto content = envFile.loadFileAsString().toStdString();
    std::stringstream ss(content);
    std::string line;

    while (std::getline(ss, line))
    {
        auto pos = line.find('=');
        if (pos != std::string::npos)
        {
            auto key   = line.substr(0, pos);
            auto value = line.substr(pos + 1);
            envValues[key] = value;
        }
    }

    return envValues;
}

BackendManager::BackendManager()
{
    auto env = loadEnv();

    std::string url  = env.count("BACKEND_URL")  > 0 ? env["BACKEND_URL"]  : "http://127.0.0.1";
    std::string port = env.count("BACKEND_PORT") > 0 ? env["BACKEND_PORT"] : "8000";

    url.erase(std::remove_if(url.begin(), url.end(), ::isspace), url.end());
    port.erase(std::remove_if(port.begin(), port.end(), ::isspace), port.end());

    bool isLocal =
        url.find("localhost") != std::string::npos ||
        url.find("127.0.0.1") != std::string::npos;

    std::string fullUrl = url;

    if (isLocal)
        fullUrl += ":" + port;

    apiUrl = juce::String(fullUrl);

    auto exeDir = getExeDir();
    sessionFile = exeDir.getChildFile("HarmoniaSession.json");

    writeLog("API_URL = " + apiUrl);

    if (env.empty())
        writeLog(".env introuvable → valeurs par défaut utilisées (cherché un niveau au-dessus de l'exe)");
    else
        writeLog(".env chargé avec succès (deux niveau au-dessus de l'exe)");

    authManager = std::make_unique<BackendAuthManager>(*this);
    aiManager  = std::make_unique<BackendAiManager>(*this);
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
    auto exeDir = getExeDir();
    auto logFile = exeDir.getChildFile("HarmoniaLogs.txt");

    logFile.appendText("[Backend] " + message + "\n");
}

// Auth façade

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

// IA façade

PatchCallResult BackendManager::generatePatch(const juce::String& prompt)
{
    if (aiManager)
        return aiManager->generatePatch(prompt);
    return PatchCallResult::error("AI manager not initialized");
}

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

void BackendManager::syncProfileParamsInBackground(const UserSession& session)
{
    authManager->syncProfileParamsInBackground(session);
}

std::optional<UserSession> BackendManager::syncProfileParams(const UserSession& session)
{
    return authManager->syncProfileParams(session);
}

const juce::String& BackendManager::getApiUrl() const
{
    return apiUrl;
}


const juce::File& BackendManager::getSessionFile() const
{
    return sessionFile;
}