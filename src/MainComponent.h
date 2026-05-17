#pragma once

#define JUCE_USE_NATIVE_FILECHOOSERS 1

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>

#include "PluginProcessor.h"

#include "themes/HarmoniaPalette.h"
#include "themes/HiveLookAndFeel.h"

#include "backendManagement/BackendManager.h"

#include "components/SynthComponent.h"
#include "components/OscilloscopeComponent.h"
#include "components/SectionPanel.h"
#include "components/KnobControl.h"
#include "components/DisplayScreen.h"
#include "components/ParticleField.h"
#include "components/EnvelopeVisualizer.h"
#include "components/LfoVisualizer.h"
#include "components/WaveformSelector.h"

#include "config/AppConfig.h"
#include "parameters/HarmoniaParameters.h"
#include "tools/PresetLoader.h"

class MainComponent : public juce::Component
{
public:
    MainComponent (HarmoniaAudioProcessor& p,
                   BackendManager& be,
                   const UserSession& session);

    ~MainComponent() override;

    void paint (juce::Graphics&) override;
    void resized() override;

    std::function<void()> onLogout;

private:
    HarmoniaAudioProcessor& processor;
    BackendManager& backend;
    UserSession session;

    HiveLookAndFeel lookAndFeel;
    ParticleField   particles;

    juce::Label   titleLabel;
    juce::Label   subtitleLabel;
    juce::Label   presetLabel;
    juce::TextEditor promptEditor;
    juce::TextButton generateButton { "Generate" };
    juce::TextButton loadButton     { "Load" };
    juce::TextButton saveButton     { "Save" };
    juce::TextButton logoutButton   { "Logout" };

    SectionPanel oscMixPanel  { "Osc 1 / Mix",   HarmoniaPalette::sectionOsc      };
    SectionPanel filterPanel  { "Filter",        HarmoniaPalette::sectionFilter   };
    SectionPanel screenPanel  { "Display",       HarmoniaPalette::sectionDisplay  };
    SectionPanel lfoPanel     { "LFO",           HarmoniaPalette::sectionLfo      };
    SectionPanel osc2Panel    { "Osc 2",         HarmoniaPalette::sectionOsc      };
    SectionPanel fxPanel      { "Effects",       HarmoniaPalette::sectionFx       };
    SectionPanel ampPanel     { "Amp Envelope",  HarmoniaPalette::sectionAmpEnv   };

    OscilloscopeComponent oscilloscope;
    DisplayScreen         displayScreen;
    SynthComponent        synthComponent;
    EnvelopeVisualizer    ampEnvViz;
    LfoVisualizer         lfoViz;

    std::unique_ptr<WaveformSelector>   osc1WaveSel;
    std::unique_ptr<KnobControl>        oscMixKnob;
    std::unique_ptr<KnobControl>        noiseLevelKnob;

    std::unique_ptr<KnobControl>        filterCutoffKnob;
    std::unique_ptr<KnobControl>        filterResoKnob;
    std::unique_ptr<FilterTypeSelector> filterTypeSel;
    std::unique_ptr<KnobControl>        filterEnvAmtKnob;
    std::unique_ptr<KnobControl>        filterEnvDecayKnob;
    std::unique_ptr<KnobControl>        velocityFilterKnob;

    std::unique_ptr<KnobControl>        lfoRateKnob;
    std::unique_ptr<KnobControl>        lfoToPitchKnob;
    std::unique_ptr<KnobControl>        lfoToCutoffKnob;

    std::unique_ptr<WaveformSelector>   osc2WaveSel;
    std::unique_ptr<KnobControl>        osc2DetuneKnob;
    std::unique_ptr<KnobControl>        distortionKnob;
    std::unique_ptr<KnobControl>        reverbKnob;

    std::unique_ptr<KnobControl>        attackKnob;
    std::unique_ptr<KnobControl>        decayKnob;
    std::unique_ptr<KnobControl>        sustainKnob;
    std::unique_ptr<KnobControl>        releaseKnob;

    std::unique_ptr<juce::FileChooser>  chooser;

    void buildHeader();
    void buildControls();
    void wireButtons();
    void registerJuiceFor (juce::Component* c);

    void doLoadPreset();
    void doSavePreset();
    void doGenerateWithAi();

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};