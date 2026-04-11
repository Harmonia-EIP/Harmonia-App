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
    session.userId      = body.value("user_id", 0);
    session.accessToken = body.value("token", "");
    session.pseudo      = body.value("username", "");
    session.email       = body.value("email", "");
    session.layoutId    = body.value("layout_id", 0);
    session.themeId     = body.value("theme_id", 0);

    session.expiresAt = juce::Time::getCurrentTime()
                        + juce::RelativeTime::hours(1);

    saveSession(session);

    syncProfileParamsInBackground(session);

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
    session.userId      = body.value("user_id", 0);
    session.accessToken = body.value("token", "");
    session.pseudo      = body.value("username", "");
    session.email       = body.value("email", "");
    session.layoutId    = body.value("layout_id", 0);
    session.themeId     = body.value("theme_id", 0);

    session.expiresAt = juce::Time::getCurrentTime()
                        + juce::RelativeTime::hours(1);

    saveSession(session);

    syncProfileParamsInBackground(session);

    return AuthResult{ true, session, {} };
}

void BackendAuthManager::saveSession(const UserSession& session)
{
    backend.writeLog("saveSession()");

    json j{
        { "userId",      session.userId },
        { "pseudo",      session.pseudo.toStdString() },
        { "email",       session.email.toStdString() },
        { "accessToken", session.accessToken.toStdString() },
        { "expiresAt",   (long long) session.expiresAt.toMilliseconds() },
        { "layoutId",    session.layoutId },
        { "themeId",     session.themeId }
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
    session.userId      = j.value("userId", 0);
    session.pseudo      = j.value("pseudo", "");
    session.email       = j.value("email", "");
    session.accessToken = j.value("accessToken", "");
    session.layoutId = j.value("layoutId", 0);
    session.themeId  = j.value("themeId", 0);

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

void BackendAuthManager::syncProfileParamsInBackground(const UserSession& session)
{
    std::thread([this, session]()
    {
        auto url = backend.getApiUrl() + "/profile/me";

        auto response = cpr::Get(
            cpr::Url{ url.toStdString() },
            cpr::Header{
                { "Authorization", "Bearer " + session.accessToken.toStdString() }
            }
        );

        if (response.status_code != 200)
            return;

        try
        {
            auto body = json::parse(response.text);

            UserSession updated = session;
            updated.userId   = body.value("user_id", session.userId);
            updated.pseudo   = body.value("username", session.pseudo.toStdString()).c_str();
            updated.email    = body.value("email", session.email.toStdString()).c_str();
            updated.layoutId = body.value("layout_id", session.layoutId);
            updated.themeId  = body.value("theme_id", session.themeId);

            saveSession(updated);

            backend.writeLog("Session sync depuis backend.");
        }
        catch (...) {}
    }).detach();
}

std::optional<UserSession> BackendAuthManager::syncProfileParams(const UserSession& session)
{
    auto url = backend.getApiUrl() + "/profile/me";

    auto response = cpr::Get(
        cpr::Url{ url.toStdString() },
        cpr::Header{
            { "Authorization", "Bearer " + session.accessToken.toStdString() }
        }
    );

    if (response.status_code != 200)
    {
        backend.writeLog("Blocking sync failed, status: " + std::to_string(response.status_code));
        return std::nullopt;
    }

    try
    {
        auto body = json::parse(response.text);

        UserSession updated = session;
        updated.userId   = body.value("user_id", session.userId);
        updated.pseudo   = body.value("username", session.pseudo.toStdString()).c_str();
        updated.email    = body.value("email", session.email.toStdString()).c_str();
        updated.layoutId = body.value("layout_id", session.layoutId);
        updated.themeId  = body.value("theme_id", session.themeId);

        saveSession(updated);

        backend.writeLog("Session synced (blocking) from backend.");

        return updated;
    }
    catch (const std::exception& e)
    {
        backend.writeLog("Blocking sync JSON parse error: " + std::string(e.what()));
        return std::nullopt;
    }
    catch (...)
    {
        backend.writeLog("Blocking sync unknown error");
        return std::nullopt;
    }
}