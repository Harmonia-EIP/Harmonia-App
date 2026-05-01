#pragma once

#include <functional>
#include "backendManagement/BackendManager.h"
#include "models/DataModel.h"

class AIManager
{
public:
    AIManager(BackendManager& backend);

    void generate(const juce::String& prompt);

    std::function<void(const PatchParams&)> onSuccess;
    std::function<void(const juce::String&)> onError;

private:
    BackendManager& backend;
};