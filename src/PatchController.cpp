#include "PatchController.h"

PatchController::PatchController(HarmoniaAudioProcessor& p)
    : processor(p)
{
    currentPatch = processor.getPatch();
}

void PatchController::setPatch(const PatchParams& patch)
{
    if (!isValid(patch))
        return;

    currentPatch = patch;

    if (onPatchChanged)
        onPatchChanged(currentPatch);
}

const PatchParams& PatchController::getPatch() const
{
    return currentPatch;
}

void PatchController::applyToProcessor()
{
    processor.setPatch(currentPatch);
}

void PatchController::loadFromProcessor()
{
    currentPatch = processor.getPatch();

    if (onPatchChanged)
        onPatchChanged(currentPatch);
}

// ================= JSON =================

juce::String PatchController::toJson() const
{
    return PatchSerializer::toJson(currentPatch);
}

bool PatchController::fromJson(const juce::String& json)
{
    auto result = PatchSerializer::fromJson(json);
    if (!result.has_value())
        return false;

    if (!isValid(*result))
        return false;

    currentPatch = *result;

    if (onPatchChanged)
        onPatchChanged(currentPatch);

    return true;
}

// ================= Validation =================

bool PatchController::isValid(const PatchParams& p) const
{
    return p.frequency > 0.0f
        && p.volume >= 0.0f && p.volume <= 1.0f;
}