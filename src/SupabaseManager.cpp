#include "SupabaseManager.h"
using json = nlohmann::json;

SupabaseManager::SupabaseManager()
    : SUPABASE_URL("https://gtzupyingamgmwjicham.supabase.co"),
      SUPABASE_KEY("eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Imd0enVweWluZ2FtZ213amljaGFtIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NjEwMzE5MzgsImV4cCI6MjA3NjYwNzkzOH0.vJdm4z6hA5p25q-i1tfexp1j2jUdKWqmvobi9IiWSIw")
{
    auto exeDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                      .getParentDirectory();
    sessionFile = exeDir.getChildFile("HarmoniaSession.json");
}

std::optional<UserSession> SupabaseManager::loginUser(const juce::String& email,
                                                      const juce::String& password)
{
    auto response = cpr::Post(
        cpr::Url{ SUPABASE_URL.toStdString() + "/auth/v1/token?grant_type=password" },
        cpr::Header{
            {"apikey", SUPABASE_KEY.toStdString()},
            {"Content-Type", "application/json"}
        },
        cpr::Body{ json{
            {"email", email.toStdString()},
            {"password", password.toStdString()}
        }.dump() }
    );

    if (response.status_code == 200)
    {
        auto body = json::parse(response.text);
        auto session = parseSession(body);
        if (session.has_value())
        {
            saveSession(*session);
            return session;
        }
    }

    return std::nullopt;
}

std::optional<UserSession> SupabaseManager::signupUser(const juce::String& email,
                                                       const juce::String& password,
                                                       const juce::String& username,
                                                       const juce::String& firstname,
                                                       const juce::String& lastname)
{
    auto response = cpr::Post(
        cpr::Url{ SUPABASE_URL.toStdString() + "/auth/v1/signup" },
        cpr::Header{
            {"apikey", SUPABASE_KEY.toStdString()},
            {"Content-Type", "application/json"}
        },
        cpr::Body{ json{
            {"email", email.toStdString()},
            {"password", password.toStdString()},
            {"data", {
                {"username", username.toStdString()},
                {"firstname", firstname.toStdString()},
                {"lastname", lastname.toStdString()}
            }}
        }.dump() }
    );

    if (response.status_code == 200)
    {
        auto body = json::parse(response.text);
        auto session = parseSession(body);
        if (session.has_value())
        {
            session->pseudo = username;
            saveSession(*session);
            return session;
        }
    }

    return std::nullopt;
}

std::optional<UserSession> SupabaseManager::refreshSession(const juce::String& refreshToken)
{
    auto response = cpr::Post(
        cpr::Url{ SUPABASE_URL.toStdString() + "/auth/v1/token?grant_type=refresh_token" },
        cpr::Header{
            {"apikey", SUPABASE_KEY.toStdString()},
            {"Content-Type", "application/json"}
        },
        cpr::Body{ json{{"refresh_token", refreshToken.toStdString()}}.dump() }
    );

    if (response.status_code == 200)
    {
        auto body = json::parse(response.text);
        auto session = parseSession(body);
        if (session.has_value())
        {
            saveSession(*session);
            return session;
        }
    }

    return std::nullopt;
}

void SupabaseManager::saveSession(const UserSession& session)
{
    json j{
        {"pseudo", session.pseudo.toStdString()},
        {"email", session.email.toStdString()},
        {"accessToken", session.accessToken.toStdString()},
        {"refreshToken", session.refreshToken.toStdString()},
        {"expiresAt", session.expiresAt.toMilliseconds()}
    };

    sessionFile.replaceWithText(j.dump(4));
}

std::optional<UserSession> SupabaseManager::loadSession()
{
    if (!sessionFile.existsAsFile())
        return std::nullopt;

    auto content = sessionFile.loadFileAsString();
    auto j = json::parse(content.toStdString());

    UserSession s;
    s.pseudo = j.value("pseudo", "");
    s.email = j.value("email", "");
    s.accessToken = j.value("accessToken", "");
    s.refreshToken = j.value("refreshToken", "");
    s.expiresAt = juce::Time(j.value("expiresAt", 0));

    if (s.email.isEmpty() || s.accessToken.isEmpty())
        return std::nullopt;

    return s;
}

void SupabaseManager::clearSession()
{
    if (sessionFile.existsAsFile())
        sessionFile.deleteFile();
}

std::optional<UserSession> SupabaseManager::parseSession(const json& j)
{
    if (!j.contains("access_token"))
        return std::nullopt;

    UserSession session;
    session.accessToken = j.value("access_token", "");
    session.refreshToken = j.value("refresh_token", "");
    session.email = j.value("user", json{}).value("email", "");
    session.pseudo = j.value("user", json{}).value("user_metadata", json{}).value("username", "");
    session.expiresAt = juce::Time::getCurrentTime() + juce::RelativeTime::hours(12);

    return session;
}
