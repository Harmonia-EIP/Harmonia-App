#include "BackendManager.h"
#include "BackendAuthManager.h"
#include "BackendAiManager.h"

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
    auto appRoot = exeDir.getParentDirectory();                  // IMPORTANT: .env un niveau au-dessus
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

    apiUrl = juce::String(url + ":" + port);

    auto exeDir = getExeDir();
    sessionFile = exeDir.getChildFile("HarmoniaSession.json");

    writeLog("API_URL = " + apiUrl);

    if (env.empty())
        writeLog(".env introuvable → valeurs par défaut utilisées (cherché un niveau au-dessus de l'exe)");
    else
        writeLog(".env chargé avec succès (un niveau au-dessus de l'exe)");

    authManager = new BackendAuthManager(*this);
    aiManager  = new BackendAiManager(*this);
}

BackendManager::~BackendManager()
{
    delete authManager;
    delete aiManager;
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
    return aiManager->generatePatch(prompt);
}
