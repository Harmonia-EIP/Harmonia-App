#pragma once

#define JUCE_USE_NATIVE_FILECHOOSERS 1

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

#include "components/FrequencySlider.h"
#include "components/VolumeSlider.h"
#include "components/GenerateButton.h"
#include "models/DataModel.h"

#include "LayoutManager.h"
#include "ThemeManager.h"
#include "themes/AppLookAndFeel.h"
#include "backendManagement/BackendManager.h"

#include "components/TitleComponent.h"
#include "components/TopBarComponent.h"
#include "components/FrequencyVolumeComponent.h"
#include "components/ADSRComponent.h"
#include "components/FilterComponent.h"
#include "components/BottomBarComponent.h"
#include "components/SynthComponent.h"
#include "components/OscilloscopeComponent.h"

#include "tools/ThemeAndLayoutConverter.h"
#include "tools/PatchSerializer.h"

#include "config/String.h"
#include "config/AppConfig.h"

#include "PatchController.h"

#include "AIManager.h"

class MainComponent : public juce::Component
{
public:
    using LayoutPreset = AppLookAndFeel::LayoutPreset;
    using ThemePreset  = AppLookAndFeel::ThemePreset;

    MainComponent(HarmoniaAudioProcessor& p,
                  BackendManager& be,
                  const UserSession& session);


    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    TitleComponent& getTitleComponent() { return title; }
    
    private:
    //==============================================================================
    // PROCESSOR
    
    HarmoniaAudioProcessor& processor;
    
    // fallback keyboard (si pas de processor)
    juce::MidiKeyboardState localKeyboardState;

    PatchController patchController;

    //==============================================================================
    // BACKEND

    BackendManager& backend;
    AIManager aiManager;

    //==============================================================================
    // UI

    AppLookAndFeel appLookAndFeel;

    TitleComponent title;
    OscilloscopeComponent oscilloscope;
    TopBarComponent topBar;
    FrequencyVolumeComponent freqVolComponent;
    ADSRComponent adsrComponent;
    FilterComponent filterComponent;
    BottomBarComponent bottomBar;

    SynthComponent synthComponent;

    //==============================================================================
    // LAYOUT

    LayoutManager layoutManager;
    ThemeManager themeManager;

    bool isUpdatingUI = false;

    void applyTheme(ThemePreset theme);
    void applyLayout(LayoutPreset layout);

    void initUI(const UserSession& session);
    void addComponents();
    void initComponentsListeners();
    void initTitleComponent();
    void initAsyncProfileLoad();
    void applyParamsToUI(const PatchParams& p);
    void initExportParameters();
    void initLoadParameters();
    void initGenerateWithAI();
    void initAIManager();
    void onUIChanged();
    PatchParams collectParamsFromUI();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};