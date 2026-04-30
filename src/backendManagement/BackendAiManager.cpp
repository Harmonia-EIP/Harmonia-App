#include "BackendAiManager.h"

using json = nlohmann::json;

BackendAiManager::BackendAiManager(BackendManager& bm)
    : backend(bm)
{
}

PatchCallResult BackendAiManager::generatePatch(const juce::String& prompt)
{
    PatchCallResult result;

    if (prompt.trim().isEmpty())
    {
        result.errorMessage = Strings::Errors::EmptyPrompt.toStdString();
        return result;
    }

    auto sessionOpt = backend.loadSession();
    if (!sessionOpt.has_value())
    {
        result.errorMessage = Strings::Errors::NoUserConnected.toStdString();
        return result;
    }

    auto session = sessionOpt.value();
    auto now = juce::Time::getCurrentTime();
    if (session.expiresAt < now)
    {
        result.errorMessage = Strings::Errors::SessionExpired.toStdString();
        return result;
    }

    auto url = backend.getApiUrl() + "/ai/generate-preset";

    json payload{
        { "prompt", prompt.toStdString() }
    };

    auto response = cpr::Post(
        cpr::Url{ url.toStdString() },
        cpr::Header{
            { "Content-Type", "application/json" },
            { "Authorization", "Bearer " + session.accessToken.toStdString() }
        },
        cpr::Body{ payload.dump() }
    );

    if (response.status_code != 200)
    {
        result.errorMessage =
            Strings::Errors::ErrorHTTP + juce::String(response.status_code);
        return result;
    }

    json body;
    try
    {
        body = json::parse(response.text);
    }
    catch (...)
    {
        result.errorMessage = Strings::Errors::UnreadableAIResponse.toStdString();
        return result;
    }

    PatchParams p;
    p.waveform = PatchSerializer::waveformFromString(
        juce::String(body.value("waveform", "Sine"))
    );

    p.filterType = PatchSerializer::filterFromString(
        juce::String(body.value("filterType", "Low Pass"))
    );
    p.frequency  = body.value("frequency",  440.0);
    p.volume     = body.value("volume",     0.8);
    p.attack     = body.value("attack",     0.1);
    p.decay      = body.value("decay",      0.1);
    p.sustain    = body.value("sustain",    0.8);
    p.release    = body.value("release",    0.5);
    p.cutoff     = body.value("cutoff",     1000.0);
    p.resonance  = body.value("resonance",  1.0);

    result.success = true;
    result.params  = p;
    return result;
}
