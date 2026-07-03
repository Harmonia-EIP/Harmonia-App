#pragma once

#include <juce_gui_basics/juce_gui_basics.h>

#include "../themes/HarmoniaPalette.h"
#include "../themes/HiveLookAndFeel.h"
#include "../themes/PaletteSelector.h"
#include "../backendManagement/BackendManager.h"
#include "../config/AiConfig.h"

class HeaderComponent : public juce::Component
{
public:
    HeaderComponent (const UserSession& session);

    void paint (juce::Graphics&) override;
    void resized() override;

    juce::TextButton& getGenerateButton() { return generateButton; }
    juce::TextButton& getLoadButton()     { return loadButton; }
    juce::TextButton& getSaveButton()     { return saveButton; }
    juce::TextButton& getLogoutButton()   { return logoutButton; }

    juce::TextEditor& getPromptEditor()   { return promptEditor; }

    juce::Label& getPresetLabel()         { return presetLabel; }

    PaletteSelector& getPaletteSelector() { return paletteSelector; }

    std::function<void(HarmoniaPalette::Theme)> onThemeChanged;

    AIModel getSelectedModel() const;
    juce::String getSelectedModelName() const;
    juce::String getSelectedBackendName() const;
    int getSelectedModelId() const;

private:
    UserSession session;

    juce::Label titleLabel;
    juce::Label subtitleLabel;
    juce::Label presetLabel;

    juce::ComboBox modelSelector;

    juce::TextEditor promptEditor;

    juce::TextButton generateButton;
    juce::TextButton loadButton;
    juce::TextButton saveButton;
    juce::TextButton logoutButton;

    PaletteSelector paletteSelector;

};