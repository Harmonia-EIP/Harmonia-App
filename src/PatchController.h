#pragma once

#include "PluginProcessor.h"
#include "tools/PatchSerializer.h"

class PatchController
{
public:
    PatchController(HarmoniaAudioProcessor& processor);

    // Core
    void setPatch(const PatchParams& patch);
    const PatchParams& getPatch() const;

    void applyToProcessor();
    void loadFromProcessor();

    // JSON
    juce::String toJson() const;
    bool fromJson(const juce::String& json);

    // Optional callback (UI sync auto)
    std::function<void(const PatchParams&)> onPatchChanged;

private:
    PatchParams currentPatch;
    HarmoniaAudioProcessor& processor;

    bool isValid(const PatchParams& p) const;
};