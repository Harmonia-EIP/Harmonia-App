#include "MainComponent.h"

void MainComponent::initGenerateWithAI()
{
    bottomBar.onGenerateClicked = [this]()
    {
        auto prompt = topBar.getPrompt();

        if (prompt.trim().isEmpty())
        {
            juce::AlertWindow::showMessageBoxAsync(
                juce::AlertWindow::WarningIcon,
                "Prompt manquant",
                "Veuillez saisir un prompt avant de générer.");
            return;
        }

        std::thread([this, prompt]()
        {
            auto result = backend.generatePatch(prompt);

            if (!result.success)
            {
                juce::MessageManager::callAsync([msg = result.errorMessage]()
                {
                    juce::AlertWindow::showMessageBoxAsync(
                        juce::AlertWindow::WarningIcon,
                        "Erreur IA",
                        msg);
                });
                return;
            }

            PatchParams p = result.params;

            juce::MessageManager::callAsync([this, p]()
            {
                applyParamsToUI(p);
            });

        }).detach();
    };
}