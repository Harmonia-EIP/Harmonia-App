#pragma once

#include <juce_core/juce_core.h>
#include <nlohmann/json.hpp>

#include "../BackendTypes.h"
#include "../../config/AiConfig.h"   // adapte le chemin selon ton arborescence

class HttpClient;
class SessionStore;
class BackendLogger;

struct AiModelsResult
{
    bool         success = false;
    AIModelList  models;
    juce::String errorMessage;
};

/**
 * Génération / affinage de presets par l'IA + liste des modèles disponibles.
 *
 * Endpoints : GET /ai/models, POST /ai/generate-preset, POST /ai/refine-preset
 *
 * Tous passent par le même contrôle : session présente, pas invité, pas expirée
 * -> requête avec le Bearer.
 * Pour generate / refine, le JSON brut de la réponse est renvoyé tel quel
 * (à passer au PresetLoader).
 *
 * backendName : id du modèle ("charter_v1", ...) ou "latest" pour le dernier déployé.
 */
class AiService
{
public:
    AiService (HttpClient& http, SessionStore& store, BackendLogger& logger);

    AiModelsResult fetchModels();

    AiResult generatePreset (const juce::String& prompt,
                             const juce::String& backendName);

    AiResult refinePreset (const juce::String& prompt,
                           const juce::String& currentJson,
                           const juce::StringArray& lockedParamIds,
                           const juce::String& backendName);

private:
    AiResult postAuthenticated (const juce::String& endpoint, const nlohmann::json& payload);

    HttpClient&    http;
    SessionStore&  store;
    BackendLogger& logger;
};