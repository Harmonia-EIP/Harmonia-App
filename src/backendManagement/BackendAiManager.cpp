#include "BackendAiManager.h"
#include "BackendManager.h"

#include <cpr/cpr.h>
#include <nlohmann/json.hpp>

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
        result.errorMessage = "Prompt vide.";
        return result;
    }

    auto sessionOpt = backend.loadSession();
    if (!sessionOpt.has_value())
    {
        result.errorMessage = "Aucun utilisateur connecté.";
        return result;
    }

    auto session = sessionOpt.value();
    auto now = juce::Time::getCurrentTime();
    if (session.expiresAt < now)
    {
        result.errorMessage = "Session expirée. Veuillez vous reconnecter.";
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
            "Erreur HTTP " + juce::String(response.status_code);
        return result;
    }

    json body;
    try
    {
        body = json::parse(response.text);
    }
    catch (...)
    {
        result.errorMessage = "Réponse IA illisible.";
        return result;
    }

    PatchParams p;
    p.waveform   = body.value("waveform",   "Sine");
    p.filterType = body.value("filterType", "Low Pass");
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
