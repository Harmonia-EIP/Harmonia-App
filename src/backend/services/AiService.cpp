#include "AiService.h"

#include "../core/BackendLogger.h"
#include "../core/BackendStrings.h"
#include "../core/JsonUtil.h"
#include "../net/ApiError.h"
#include "../net/HttpClient.h"
#include "../storage/SessionStore.h"

using json = nlohmann::json;

AiService::AiService (HttpClient& h, SessionStore& s, BackendLogger& l)
    : http (h), store (s), logger (l)
{
}

//==============================================================================
AiModelsResult AiService::fetchModels()
{
    const auto sessionOpt = store.load();

    // Pas de session, ou invité : on ne part même pas sur le réseau.
    if (! sessionOpt.has_value() || sessionOpt->isGuest)
        return { false, {}, BackendStrings::LoginRequired };

    const auto& session = *sessionOpt;

    if (session.expiresAt < juce::Time::getCurrentTime())
        return { false, {}, BackendStrings::SessionExpired };

    const auto response = http.get ("/ai/models",
                                    HttpOptions (HttpTimeouts::Default).withToken (session.accessToken));

    if (! response.isSuccess())
    {
        logger.warn ("GET /ai/models failed: " + response.describe());
        return { false, {}, parseApiError (response).message };
    }

    const auto root = json::parse (response.body, nullptr, false);

    if (root.is_discarded() || ! root.is_object()
        || ! root.contains ("models") || ! root["models"].is_array())
        return { false, {}, "Invalid models response" };

    auto getStr = [] (const json& o, const char* key) -> juce::String
    {
        const auto it = o.find (key);

        return (it != o.end() && it->is_string()) ? juce::String (it->get<std::string>())
                                                  : juce::String();
    };

    AIModelList list;

    for (const auto& m : root["models"])
    {
        if (! m.is_object())
            continue;

        AIModelData d;
        d.id = getStr (m, "id");

        if (d.id.isEmpty())
            continue;

        const auto name = getStr (m, "name");
        d.name = name.isNotEmpty() ? name : d.id;

        const auto version = getStr (m, "model_version");
        const auto hash    = getStr (m, "model_hash");

        if (version.isNotEmpty()) d.version = version;
        if (hash.isNotEmpty())    d.hash    = hash;

        const auto lat = m.find ("is_latest");
        d.isLatest = (lat != m.end() && lat->is_boolean() && lat->get<bool>());

        list.push_back (std::move (d));
    }

    logger.info ("GET /ai/models : " + juce::String ((int) list.size()) + " model(s)");

    return { true, std::move (list), {} };
}

//==============================================================================
AiResult AiService::generatePreset (const juce::String& prompt,
                                    const juce::String& backendName)
{
    if (prompt.trim().isEmpty())
        return AiResult::failure (AiResult::Error::EmptyPrompt, BackendStrings::EmptyPrompt);

    const json payload {
        { "prompt",     prompt.toStdString() },
        { "model_name", backendName.toStdString() }
    };

    return postAuthenticated ("/ai/generate-preset", payload);
}

AiResult AiService::refinePreset (const juce::String& prompt,
                                  const juce::String& currentJson,
                                  const juce::StringArray& lockedParamIds,
                                  const juce::String& backendName)
{
    if (prompt.trim().isEmpty())
        return AiResult::failure (AiResult::Error::EmptyPrompt, BackendStrings::EmptyPrompt);

    // Le preset courant est envoyé comme objet JSON, pas comme chaîne échappée.
    auto current = json::parse (currentJson.toStdString(), nullptr, false);

    if (current.is_discarded())
        return AiResult::failure (AiResult::Error::Unknown, BackendStrings::InvalidCurrentPreset);

    if (current.is_object())
        current.erase ("values");   // sans effet si la clé est absente

    json locked = json::array();
    for (const auto& id : lockedParamIds)
        locked.push_back (id.toStdString());

    const json payload {
        { "prompt",         prompt.toStdString() },
        { "current_preset", current },
        { "locked_params",  locked },
        { "model_name",     backendName.toStdString() }
    };

    return postAuthenticated ("/ai/refine-preset", payload);
}

AiResult AiService::postAuthenticated (const juce::String& endpoint, const json& payload)
{
    const auto sessionOpt = store.load();

    // Pas de session, ou invité : on ne part même pas sur le réseau.
    if (! sessionOpt.has_value() || sessionOpt->isGuest)
        return AiResult::failure (AiResult::Error::NoSession, BackendStrings::LoginRequired);

    const auto& session = *sessionOpt;

    if (session.expiresAt < juce::Time::getCurrentTime())
        return AiResult::failure (AiResult::Error::SessionExpired, BackendStrings::SessionExpired);

    const auto body = payload.dump (-1, ' ', false, json::error_handler_t::replace);

    logger.info ("POST " + endpoint + " (" + juce::String ((int) body.size()) + " bytes)");

    const auto response = http.post (endpoint, body,
                                     HttpOptions (HttpTimeouts::Ai).withToken (session.accessToken));

    if (! response.isSuccess())
    {
        logger.warn ("POST " + endpoint + " failed: " + response.describe());

        const auto error = parseApiError (response);

        switch (error.kind)
        {
            case ApiError::Kind::Network:
                return AiResult::failure (AiResult::Error::Network, error.message);

            case ApiError::Kind::Unauthorized:
                return AiResult::failure (AiResult::Error::SessionExpired, BackendStrings::SessionExpired);

            default:
                return AiResult::failure (AiResult::Error::HttpError, error.message);
        }
    }

    if (response.body.empty())
        return AiResult::failure (AiResult::Error::EmptyResponse, "Empty response");

    return AiResult::ok (JsonUtil::toJuce (response.body));
}