#include "BackendAiManager.h"
#include "BackendManager.h"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

BackendAiManager::BackendAiManager(BackendManager& b)
    : backend(b)
{
}

AiResult BackendAiManager::generatePreset(const juce::String& prompt)
{
    if (prompt.trim().isEmpty())
        return AiResult::error("Prompt is empty");

    auto sessionOpt = backend.loadSession();
    if (!sessionOpt.has_value())
        return AiResult::error("No user connected");

    auto session = sessionOpt.value();

    if (session.expiresAt < juce::Time::getCurrentTime())
        return AiResult::error("Session expired");

    json payload{ { "prompt", prompt.toStdString() } };

    auto response = cpr::Post(
        cpr::Url{ (backend.getApiUrl() + "/ai/generate-preset").toStdString() },
        cpr::Header{
            { "Content-Type", "application/json" },
            { "Authorization", "Bearer " + session.accessToken.toStdString() }
        },
        cpr::Body{ payload.dump() }
    );

    if (response.status_code != 200)
        return AiResult::error("HTTP " + juce::String(response.status_code));

    if (response.text.empty())
        return AiResult::error("Empty response");

    return AiResult::ok(juce::String(response.text));
}