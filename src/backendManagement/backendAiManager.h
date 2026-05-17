#pragma once

#include <juce_core/juce_core.h>
#include "BackendTypes.h"

class BackendManager;

class BackendAiManager
{
public:
    explicit BackendAiManager(BackendManager& backend);

    AiResult generatePreset(const juce::String& prompt);

private:
    BackendManager& backend;
};
