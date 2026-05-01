#include "MainComponent.h"
#include "themes/AppColourIds.h"

#include <filesystem>
#include <fstream>
#include <thread>

MainComponent::MainComponent (HarmoniaAudioProcessor& p,
                              BackendManager& be,
                              const UserSession& session)
    : processor (p),
      patchController(p),
      backend (be),
      aiManager (be),
      title (AppConfig::AppName, session),
      oscilloscope (appLookAndFeel,
                    AppConfig::Oscilloscope::BufferSize,
                    AppConfig::Oscilloscope::RefreshRate),
      freqVolComponent (appLookAndFeel),
      adsrComponent (appLookAndFeel),
      filterComponent (appLookAndFeel),
      synthComponent (p.getKeyboardState()),
      themeManager (appLookAndFeel)
{
    processor.setOscilloscope(&oscilloscope);

    initUI(session);
    addComponents();
    initComponentsListeners();
    initTitleComponent();
    
    patchController.onPatchChanged = [this](const PatchParams& p)
    {
        applyParamsToUI(p);
    };
    
    patchController.loadFromProcessor();
    
    initExportParameters();
    initLoadParameters();
    initGenerateWithAI();

    initAIManager();
    
    initAsyncProfileLoad();

    setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);
}

MainComponent::~MainComponent()
{
    processor.setOscilloscope(nullptr);
    setLookAndFeel (nullptr);
}

void MainComponent::resized()
{
    layoutManager.applyLayout(
        getLocalBounds(),
        title,
        topBar,
        bottomBar,
        oscilloscope,
        freqVolComponent,
        adsrComponent,
        filterComponent,
        synthComponent
    );
}

void MainComponent::applyLayout(LayoutPreset layout)
{
    layoutManager.setLayout(layout);
    resized();
}

void MainComponent::paint (juce::Graphics& g)
{
    g.fillAll(themeManager.getBackgroundColour());
}

void MainComponent::applyTheme(ThemePreset theme)
{
    themeManager.applyTheme(theme);
    sendLookAndFeelChange();
    repaint();
}
