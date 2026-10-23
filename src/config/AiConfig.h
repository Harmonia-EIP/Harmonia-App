#pragma once

#include <juce_core/juce_core.h>
#include <vector>

// Un modèle IA tel que renvoyé par GET /ai/models.
struct AIModelData
{
    juce::String id;                   // envoyé au backend dans "model_name" (ex: "charter_v1")
    juce::String name;                 // affiché dans l'UI
    bool         isLatest = false;     // true = dernier modèle déployé
    juce::String version  = "unknown";
    juce::String hash     = "unknown";
};

using AIModelList = std::vector<AIModelData>;