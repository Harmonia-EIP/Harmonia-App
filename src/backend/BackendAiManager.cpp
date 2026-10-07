#include "BackendAiManager.h"
#include "BackendManager.h"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

BackendAiManager::BackendAiManager(BackendManager& b)
    : backend(b)
{
}

AiResult BackendAiManager::generatePreset(const juce::String& prompt, int modelId, const juce::String& backendName)
{
    if (prompt.trim().isEmpty())
        return AiResult::failure(
            AiResult::Error::EmptyPrompt,
            "Prompt is empty"
        );

    auto sessionOpt = backend.loadSession();
    if (!sessionOpt.has_value())
        return AiResult::failure(
            AiResult::Error::NoSession,
            "No user connected"
        );

    auto session = sessionOpt.value();

    if (session.expiresAt < juce::Time::getCurrentTime())
        return AiResult::failure(
            AiResult::Error::SessionExpired,
            "Session expired"
        );

    json payload{ { "prompt", prompt.toStdString() }, { "model_id", modelId }, { "model_name", backendName.toStdString() } };

    backend.writeLog("Payload built: " + juce::String(payload.dump()));

    auto response = cpr::Post(
        cpr::Url{ (backend.getApiUrl() + "/ai/generate-preset").toStdString() },
        cpr::Header{
            { "Content-Type", "application/json" },
            { "Authorization", "Bearer " + session.accessToken.toStdString() }
        },
        cpr::Body{ payload.dump() }
    );

    if (response.error.code != cpr::ErrorCode::OK)
    {
        backend.writeLog(
            "CPR error: " +
            juce::String((int)response.error.code) +
            " - " +
            juce::String(response.error.message)
        );

        return AiResult::failure(
            AiResult::Error::Network,
            Strings::Errors::NetworkError
        );
    }

    if (response.status_code != 200)
        return AiResult::failure(
            AiResult::Error::HttpError,
            "HTTP " + juce::String(response.status_code)
        );

    if (response.text.empty())
        return AiResult::failure(
            AiResult::Error::EmptyResponse,
            "Empty response"
        );

    return AiResult::ok(juce::String(response.text));
}

AiResult BackendAiManager::refinePreset(const juce::String& prompt,
                                        const juce::String& currentJson,
                                        const juce::StringArray& lockedParamIds,
                                        int modelId,
                                        const juce::String& backendName)
{
    if (prompt.trim().isEmpty())
        return AiResult::failure(AiResult::Error::EmptyPrompt, "Prompt is empty");

    // Le preset courant est envoyé comme objet JSON, pas comme chaîne échappée.
    auto current = json::parse(currentJson.toStdString(), nullptr, false);

    if (current.is_discarded())
        return AiResult::failure(AiResult::Error::Unknown,
                                 "Current preset is not valid JSON");
    
    if (current.contains("values"))
        current.erase("values");

    json locked = json::array();
    for (const auto& id : lockedParamIds)
        locked.push_back(id.toStdString());

    json payload{
        { "prompt",         prompt.toStdString() },
        { "current_preset", current },
        { "locked_params",  locked },
        { "model_id",       modelId },
        { "model_name",     backendName.toStdString() }
    };

    return postAuthenticated("/ai/refine-preset", payload);
}

AiResult BackendAiManager::postAuthenticated(const juce::String& endpoint,
                                             const json& payload)
{
    auto sessionOpt = backend.loadSession();

    if (!sessionOpt.has_value() || sessionOpt->isGuest)
        return AiResult::failure(AiResult::Error::NoSession, "No user connected");

    const auto& session = sessionOpt.value();

    if (session.expiresAt < juce::Time::getCurrentTime())
        return AiResult::failure(AiResult::Error::SessionExpired, "Session expired");

    const auto body = payload.dump();
    backend.writeLog("POST " + endpoint + " (" + juce::String((int) body.size()) + " bytes)");

    auto response = cpr::Post(
        cpr::Url{ (backend.getApiUrl() + endpoint).toStdString() },
        cpr::Header{
            { "Content-Type",  "application/json" },
            { "Authorization", "Bearer " + session.accessToken.toStdString() }
        },
        cpr::Body{ body },
        cpr::Timeout{ 60000 }   // l'IA peut être lente, mais pas infini
    );

    if (response.error.code != cpr::ErrorCode::OK)
    {
        backend.writeLog("CPR error: " + juce::String((int) response.error.code)
                         + " - " + juce::String(response.error.message));

        return AiResult::failure(AiResult::Error::Network, Strings::Errors::NetworkError);
    }

    if (response.status_code == 401)
        return AiResult::failure(AiResult::Error::SessionExpired, "Session expired");

    if (response.status_code != 200)
        return AiResult::failure(AiResult::Error::HttpError,
                                 "HTTP " + juce::String(response.status_code));

    if (response.text.empty())
        return AiResult::failure(AiResult::Error::EmptyResponse, "Empty response");

    return AiResult::ok(juce::String(response.text));
}