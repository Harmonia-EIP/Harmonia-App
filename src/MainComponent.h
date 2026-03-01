#pragma once

#define JUCE_USE_NATIVE_FILECHOOSERS 1

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_dsp/juce_dsp.h>

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "FrequencySlider.h"
#include "VolumeSlider.h"
#include "GenerateButton.h"
#include "DataModel.h"

#include "themes/AppLookAndFeel.h"
#include "backendManagement/BackendManager.h"

#include "components/TitleComponent.h"
#include "components/TopBarComponent.h"
#include "components/FrequencyVolumeComponent.h"
#include "components/ADSRComponent.h"
#include "components/FilterComponent.h"
#include "components/BottomBarComponent.h"
#include "components/Synth.h"
#include "components/SynthComponent.h"
#include "components/OscilloscopeComponent.h"

#include "tools/ThemeAndLayoutConverter.h"

class MainComponent : public juce::AudioAppComponent
{
public:
    using LayoutPreset = AppLookAndFeel::LayoutPreset;
    using ThemePreset = AppLookAndFeel::ThemePreset;

    explicit MainComponent (BackendManager& be, const UserSession& session);
    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    TitleComponent& getTitleComponent() { return title; }

private:
    BackendManager& backend;

    AppLookAndFeel appLookAndFeel;

    TitleComponent title;
    OscilloscopeComponent oscilloscope;
    TopBarComponent topBar;
    FrequencyVolumeComponent freqVolComponent;
    ADSRComponent adsrComponent;
    FilterComponent filterComponent;
    BottomBarComponent bottomBar;

    SynthComponent synthComponent;

    juce::Synthesiser synth;
    double currentSampleRate = 44100.0;

    void updateSynthParamsFromUI();

    LayoutPreset layoutPreset;

    void applyTheme(ThemePreset theme);
    void applyLayout(LayoutPreset layout);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
