#pragma once

#include <juce_core/juce_core.h>
#include <nlohmann/json.hpp>

#include "../BackendTypes.h"

class HttpClient;
class SessionStore;
class BackendLogger;

/**
 * Génération / affinage de presets par l'IA.
 *
 * Endpoints : POST /ai/generate-preset, POST /ai/refine-preset
 *
 * Les deux passent par le même chemin (postAuthenticated) :
 * session présente, pas invité, pas expirée -> POST avec le Bearer.
 * Le JSON brut de la réponse est renvoyé tel quel (à passer au PresetLoader).
 */
class AiService
{
public:
    AiService (HttpClient& http, SessionStore& store, BackendLogger& logger);

    AiResult generatePreset (const juce::String& prompt,
                             int modelId,
                             const juce::String& backendName);

    AiResult refinePreset (const juce::String& prompt,
                           const juce::String& currentJson,
                           const juce::StringArray& lockedParamIds,
                           int modelId,
                           const juce::String& backendName);

private:
    AiResult postAuthenticated (const juce::String& endpoint, const nlohmann::json& payload);

    HttpClient&    http;
    SessionStore&  store;
    BackendLogger& logger;
};
