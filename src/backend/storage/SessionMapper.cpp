#include "SessionMapper.h"

#include "PaletteMapper.h"
#include "../core/JsonUtil.h"

namespace SessionMapper
{
    using json = nlohmann::json;

    std::optional<UserSession> fromAuthResponse (const json& body)
    {
        if (! body.is_object())
            return std::nullopt;

        const auto token = JsonUtil::getString (body, "token");

        if (token.empty())
            return std::nullopt;

        UserSession session;
        session.isGuest     = false;
        session.userId      = JsonUtil::getInt (body, "user_id", 0);
        session.accessToken = JsonUtil::toJuce (token);
        session.pseudo      = JsonUtil::toJuce (JsonUtil::getString (body, "username"));
        session.email       = JsonUtil::toJuce (JsonUtil::getString (body, "email"));

        // La palette vient uniquement de la réponse du backend
        PaletteMapper::readFromJson (body, session.paletteColours, session.paletteSlot);

        session.expiresAt = juce::Time::getCurrentTime()
                            + juce::RelativeTime::hours (sessionLifetimeHours);

        return session;
    }

    void applyProfileResponse (const json& body, UserSession& session)
    {
        session.userId = JsonUtil::getInt (body, "user_id", session.userId);

        const auto username = JsonUtil::getString (body, "username");
        if (! username.empty())
            session.pseudo = JsonUtil::toJuce (username);

        const auto email = JsonUtil::getString (body, "email");
        if (! email.empty())
            session.email = JsonUtil::toJuce (email);

        PaletteMapper::readFromJson (body, session.paletteColours, session.paletteSlot);
    }

    std::optional<UserSession> fromFileJson (const json& j)
    {
        if (! j.is_object())
            return std::nullopt;

        UserSession session;
        session.userId      = JsonUtil::getInt (j, "userId", 0);
        session.pseudo      = JsonUtil::toJuce (JsonUtil::getString (j, "pseudo"));
        session.email       = JsonUtil::toJuce (JsonUtil::getString (j, "email"));
        session.accessToken = JsonUtil::toJuce (JsonUtil::getString (j, "accessToken"));
        session.isGuest     = JsonUtil::getBool (j, "isGuest", false);
        session.paletteSlot = juce::jlimit (0, 2, JsonUtil::getInt (j, "paletteSlot", 0));

        const auto colours = j.find ("paletteColours");

        if (colours != j.end() && colours->is_array())
            for (const auto& c : *colours)
                if (c.is_string())
                    session.paletteColours.add (JsonUtil::toJuce (c.get<std::string>()));

        // Ancien fichier de session ou données invalides : on repart de zéro
        if (session.paletteColours.size() != 3)
            session.paletteColours.clear();

        session.expiresAt = juce::Time (JsonUtil::getInt64 (j, "expiresAt", 0));

        if (session.accessToken.isEmpty() && ! session.isGuest)
            return std::nullopt;

        return session;
    }

    json toFileJson (const UserSession& session)
    {
        json colours = json::array();

        for (const auto& c : session.paletteColours)
            colours.push_back (c.toStdString());

        return json {
            { "isGuest",        session.isGuest },
            { "userId",         session.userId },
            { "pseudo",         session.pseudo.toStdString() },
            { "email",          session.email.toStdString() },
            { "accessToken",    session.accessToken.toStdString() },
            { "expiresAt",      (long long) session.expiresAt.toMilliseconds() },
            { "paletteColours", colours },
            { "paletteSlot",    session.paletteSlot }
        };
    }
}
