#include "AuthService.h"

#include "../core/BackendLogger.h"
#include "../core/BackendStrings.h"
#include "../core/JsonUtil.h"
#include "../net/ApiError.h"
#include "../net/HttpClient.h"
#include "../storage/SessionMapper.h"
#include "../storage/SessionStore.h"

using json = nlohmann::json;

AuthService::AuthService (HttpClient& h, SessionStore& s, BackendLogger& l)
    : http (h), store (s), logger (l)
{
}

AuthResult AuthService::loginUser (const juce::String& usernameOrEmail,
                                   const juce::String& password)
{
    logger.info ("loginUser() for: " + usernameOrEmail);

    const json payload {
        { "identifier", usernameOrEmail.toStdString() },
        { "password",   password.toStdString() }
    };

    const auto response = http.post ("/auth/signin",
                                     payload.dump (-1, ' ', false, json::error_handler_t::replace),
                                     HttpOptions (HttpTimeouts::Auth));

    return finishAuthentication (response, "signin");
}

AuthResult AuthService::signupUser (const juce::String& username,
                                    const juce::String& firstname,
                                    const juce::String& lastname,
                                    const juce::String& email,
                                    const juce::String& password)
{
    logger.info ("signupUser() for: " + username);

    const json payload {
        { "username",   username.toStdString() },
        { "first_name", firstname.toStdString() },
        { "last_name",  lastname.toStdString() },
        { "email",      email.toStdString() },
        { "password",   password.toStdString() }
    };

    const auto response = http.post ("/auth/signup",
                                     payload.dump (-1, ' ', false, json::error_handler_t::replace),
                                     HttpOptions (HttpTimeouts::Auth));

#if JUCE_DEBUG
    // Dev uniquement : la réponse contient le token.
    logger.info ("signup raw response: " + JsonUtil::toJuce (response.body));
#endif

    return finishAuthentication (response, "signup");
}

void AuthService::logout()
{
    store.clear();
}

AuthResult AuthService::finishAuthentication (const HttpResponse& response, const juce::String& what)
{
    // Erreur réseau ou erreur HTTP : parseApiError fait la distinction et le message.
    if (! response.isSuccess())
    {
        logger.warn (what + " failed: " + response.describe());

        const auto error = parseApiError (response);
        return AuthResult { false, {}, error.message };
    }

    // Succès : on ne fait confiance à la réponse que si elle contient un token.
    std::optional<UserSession> session;

    if (const auto body = JsonUtil::parseObject (response.body))
        session = SessionMapper::fromAuthResponse (*body);

    if (! session.has_value())
    {
        logger.error (what + ": unusable response (invalid JSON or missing token)");
        return AuthResult { false, {}, makeInvalidResponseError (response.statusCode).message };
    }

    store.save (*session);

    return AuthResult { true, *session, {} };
}
