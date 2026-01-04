#include "BackendAuthManager.h"
#include "BackendManager.h"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

BackendAuthManager::BackendAuthManager(BackendManager& bm)
    : backend(bm)
{
}

AuthResult BackendAuthManager::loginUser(
    const juce::String& usernameOrEmail,
    const juce::String& password)
{
    backend.writeLog("loginUser() appelé pour: " + usernameOrEmail);

    json payload{
        { "identifier", usernameOrEmail.toStdString() },
        { "password",   password.toStdString() }
    };

    auto url = backend.getApiUrl() + "/auth/signin";

    auto response = cpr::Post(
        cpr::Url{ url.toStdString() },
        cpr::Header{ { "Content-Type", "application/json" } },
        cpr::Body{ payload.dump() }
    );

    backend.writeLog("HTTP Status: " + juce::String(response.status_code));
    backend.writeLog("Réponse brute: " + juce::String(response.text.c_str()));

    if (response.status_code != 200)
    {
        juce::String message = "Erreur inconnue";

        try
        {
            auto body = json::parse(response.text);
            if (body.contains("detail"))
                message = body["detail"].get<std::string>();
        }
        catch (...)
        {
            message = "Impossible de lire la réponse du serveur.";
        }

        return AuthResult{ false, {}, message };
    }

    auto body = json::parse(response.text);

    UserSession session;
    session.accessToken = body.value("token", "");
    session.pseudo      = body.value("username", "");
    session.email       = body.value("email", "");
    session.expiresAt   = juce::Time::getCurrentTime()
                          + juce::RelativeTime::hours(1);

    saveSession(session);

    return AuthResult{ true, session, {} };
}

AuthResult BackendAuthManager::signupUser(
    const juce::String& username,
    const juce::String& firstname,
    const juce::String& lastname,
    const juce::String& email,
    const juce::String& password)
{
    backend.writeLog("signupUser() appelé pour: " + username);

    json payload{
        { "username",   username.toStdString() },
        { "first_name", firstname.toStdString() },
        { "last_name",  lastname.toStdString() },
        { "email",      email.toStdString() },
        { "password",   password.toStdString() }
    };

    auto url = backend.getApiUrl() + "/auth/signup";

    auto response = cpr::Post(
        cpr::Url{ url.toStdString() },
        cpr::Header{ { "Content-Type", "application/json" } },
        cpr::Body{ payload.dump() }
    );

    backend.writeLog("Status signup: " + juce::String(response.status_code));
    backend.writeLog("Réponse brute signup: " + juce::String(response.text.c_str()));

    if (response.status_code != 200)
    {
        juce::String message = "Erreur inconnue";

        try
        {
            auto body = json::parse(response.text);
            if (body.contains("detail"))
                message = body["detail"].get<std::string>();
        }
        catch (...)
        {
            message = "Impossible d’analyser l’erreur signup.";
        }

        return AuthResult{ false, {}, message };
    }

    auto body = json::parse(response.text);

    UserSession session;
    session.accessToken = body.value("token", "");
    session.pseudo      = username;
    session.email       = email;
    session.expiresAt   = juce::Time::getCurrentTime()
                          + juce::RelativeTime::hours(1);

    saveSession(session);

    return AuthResult{ true, session, {} };
}

void BackendAuthManager::saveSession(const UserSession& session)
{
    backend.writeLog("saveSession()");

    json j{
        { "pseudo",      session.pseudo.toStdString() },
        { "email",       session.email.toStdString() },
        { "accessToken", session.accessToken.toStdString() },
        { "expiresAt",   (long long) session.expiresAt.toMilliseconds() }
    };

    backend.getSessionFile().replaceWithText(j.dump(4));
    backend.writeLog("Session écrite.");
}

std::optional<UserSession> BackendAuthManager::loadSession()
{
    backend.writeLog("loadSession()");

    auto sessionFile = backend.getSessionFile();

    if (!sessionFile.existsAsFile())
        return std::nullopt;

    auto content = sessionFile.loadFileAsString();
    auto j = json::parse(content.toStdString());

    UserSession session;
    session.pseudo      = j.value("pseudo", "");
    session.email       = j.value("email", "");
    session.accessToken = j.value("accessToken", "");

    auto expiresMs = j.value("expiresAt", static_cast<int64_t>(0));
    session.expiresAt = juce::Time(expiresMs);

    if (session.accessToken.isEmpty())
        return std::nullopt;

    return session;
}

void BackendAuthManager::clearSession()
{
    backend.writeLog("clearSession()");

    auto sessionFile = backend.getSessionFile();
    if (sessionFile.existsAsFile())
        sessionFile.deleteFile();
}
