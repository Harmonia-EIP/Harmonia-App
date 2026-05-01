#include "AIManager.h"
#include <thread>
#include <juce_events/juce_events.h>

AIManager::AIManager(BackendManager& b)
    : backend(b)
{
}

void AIManager::generate(const juce::String& prompt)
{
    std::thread([this, prompt]()
    {
        auto result = backend.generatePatch(prompt);

        if (!result.success)
        {
            juce::MessageManager::callAsync([this, msg = result.errorMessage]()
            {
                if (onError)
                    onError(msg);
            });
            return;
        }

        PatchParams p = result.params;

        juce::MessageManager::callAsync([this, p]()
        {
            if (onSuccess)
                onSuccess(p);
        });

    }).detach();
}