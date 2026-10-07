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

AiResult AiService::generatePreset (const juce::String& prompt,
                                    int modelId,
                                    const juce::String& backendName)
{
    if (prompt.trim().isEmpty())
        return AiResult::failure (AiResult::Error::EmptyPrompt, BackendStrings::EmptyPrompt);

    const json payload {
        { "prompt",     prompt.toStdString() },
        { "model_id",   modelId },
        { "model_name", backendName.toStdString() }
    };

    return postAuthenticated ("/ai/generate-preset", payload);
}

AiResult AiService::refinePreset (const juce::String& prompt,
                                  const juce::String& currentJson,
                                  const juce::StringArray& lockedParamIds,
                                  int modelId,
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
        { "model_id",       modelId },
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
