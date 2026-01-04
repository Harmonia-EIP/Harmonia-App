#pragma once

#include "BackendTypes.h"

class BackendManager;

class BackendAiManager
{
public:
    explicit BackendAiManager(BackendManager& backend);

    PatchCallResult generatePatch(const juce::String& prompt);

private:
    BackendManager& backend;
};
