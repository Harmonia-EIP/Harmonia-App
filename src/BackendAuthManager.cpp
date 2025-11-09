#include "BackendAuthManager.h"
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;


static void writeLog(const juce::String& message)
{
    auto exeDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                      .getParentDirectory();
    auto logFile = exeDir.getChildFile("HarmoniaLogs.txt");

    logFile.appendText("[BackendAuth] " + message + "\n");
}


static std::map<std::string, std::string> loadEnv()
{
    std::map<std::string, std::string> envValues;

    auto exeDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                      .getParentDirectory();
    auto envFile = exeDir.getChildFile(".env");

    if (!envFile.existsAsFile())
    {
        writeLog(".env introuvable → valeurs par défaut utilisées");
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
            auto key = line.substr(0, pos);
            auto value = line.substr(pos + 1);
            envValues[key] = value;
        }
    }

    writeLog(".env chargé avec succès");
    return envValues;
}


BackendAuthManager::BackendAuthManager()
{
    auto env = loadEnv();

    std::string url  = env.count("BACKEND_URL")  > 0 ? env["BACKEND_URL"]  : "http://127.0.0.1";
    std::string port = env.count("BACKEND_PORT") > 0 ? env["BACKEND_PORT"] : "8000";

    API_URL = juce::String(url + ":" + port);

    writeLog("API_URL = " + API_URL);

    auto exeDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                      .getParentDirectory();
    sessionFile = exeDir.getChildFile("HarmoniaSession.json");
}



AuthResult BackendAuthManager::loginUser(
    const juce::String& usernameOrEmail, const juce::String& password)
{
    writeLog("loginUser() appelé pour: " + usernameOrEmail);

    json payload{
        {"identifier", usernameOrEmail.toStdString()},
        {"password", password.toStdString()}
    };

    auto response = cpr::Post(
        cpr::Url{ (API_URL + "/auth/signin").toStdString() },
        cpr::Header{{ "Content-Type", "application/json" }},
        cpr::Body{ payload.dump() }
    );

    writeLog("HTTP Status: " + juce::String(response.status_code));
    writeLog("Réponse brute: " + juce::String(response.text.c_str()));

    if (response.status_code != 200)
    {
        juce::String message = "Erreur inconnue";

        try {
            auto body = json::parse(response.text);
            if (body.contains("detail")) message = body["detail"].get<std::string>();
        } catch (...) { message = "Impossible de lire la réponse du serveur."; }

        return AuthResult{ false, {}, message };
    }

    auto body = json::parse(response.text);

    UserSession session;
    session.accessToken = body.value("token", "");
    session.pseudo      = body.value("username", "");
    session.email       = body.value("email", "");

    saveSession(session);

    return AuthResult{ true, session, "" };
}


AuthResult BackendAuthManager::signupUser(
    const juce::String& username,
    const juce::String& firstname,
    const juce::String& lastname,
    const juce::String& email,
    const juce::String& password)
{
    writeLog("signupUser() appelé pour: " + username);

    json payload{
        {"username", username.toStdString()},
        {"first_name", firstname.toStdString()},
        {"last_name", lastname.toStdString()},
        {"email", email.toStdString()},
        {"password", password.toStdString()}
    };

    auto response = cpr::Post(
        cpr::Url{ (API_URL + "/auth/signup").toStdString() },
        cpr::Header{{ "Content-Type", "application/json" }},
        cpr::Body{ payload.dump() }
    );

    writeLog("Status signup: " + juce::String(response.status_code));
    writeLog("Réponse brute signup: " + juce::String(response.text.c_str()));

    if (response.status_code != 200)
    {
        juce::String message = "Erreur inconnue";

        try {
            auto body = json::parse(response.text);
            if (body.contains("detail")) message = body["detail"].get<std::string>();
        } catch (...) { message = "Impossible d’analyser l’erreur signup."; }

        return AuthResult{ false, {}, message };
    }

    auto body = json::parse(response.text);

    UserSession session;
    session.accessToken = body.value("token", "");
    session.pseudo      = username;
    session.email       = email;

    saveSession(session);

    return AuthResult{ true, session, "" };
}


void BackendAuthManager::saveSession(const UserSession& session)
{
    writeLog("saveSession()");

    json j{
        {"pseudo", session.pseudo.toStdString()},
        {"email", session.email.toStdString()},
        {"accessToken", session.accessToken.toStdString()},
        {"expiresAt", (long long) juce::Time::getCurrentTime().toMilliseconds() + 3600 * 1000}
    };

    sessionFile.replaceWithText(j.dump(4));

    writeLog("Session écrite.");
}


std::optional<UserSession> BackendAuthManager::loadSession()
{
    writeLog("loadSession()");

    if (!sessionFile.existsAsFile())
        return std::nullopt;

    auto content = sessionFile.loadFileAsString();
    auto j = json::parse(content.toStdString());

    UserSession session;
    session.pseudo      = j.value("pseudo", "");
    session.email       = j.value("email", "");
    session.accessToken = j.value("accessToken", "");
    session.expiresAt   = juce::Time(j.value("expiresAt", 0));

    if (session.accessToken.isEmpty())
        return std::nullopt;

    return session;
}


void BackendAuthManager::clearSession()
{
    writeLog("clearSession()");
    if (sessionFile.existsAsFile())
        sessionFile.deleteFile();
}
