#pragma once

#include <juce_core/juce_core.h>
#include <optional>
#include <string>
#include "BackendTypes.h"
#include <nlohmann/json.hpp>

class BackendManager;

class BackendProfileManager
{
public:
    BackendProfileManager(BackendManager& bm);

    ProfileResult getProfile();
    ProfileResult updateTheme(int themeId);
    ProfileResult updateLayout(int layoutId);

private:
    BackendManager& backend;
};