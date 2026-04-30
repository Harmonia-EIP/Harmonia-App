#include "MainComponent.h"
#include "themes/AppColourIds.h"

#include <filesystem>
#include <fstream>
#include <thread>

MainComponent::MainComponent (HarmoniaAudioProcessor& p,
                              BackendManager& be,
                              const UserSession& session)
    : processor (p),
      backend (be),
      title ("Harmonia", session),
      oscilloscope (appLookAndFeel,
                    AppConfig::Oscilloscope::BufferSize,
                    AppConfig::Oscilloscope::RefreshRate),
      freqVolComponent (appLookAndFeel),
      adsrComponent (appLookAndFeel),
      filterComponent (appLookAndFeel),
      synthComponent (p.getKeyboardState())
{
    processor.setOscilloscope(&oscilloscope);

    initUI(session);
    initAsyncProfileLoad();
    addComponents();
    initComponentsListeners();
    initTitleComponent();

    initExportParameters();
    initLoadParameters();
    initGenerateWithAI();

    layoutPreset = ThemeAndLayoutConverter::idToLayoutPreset(session.layoutId);

    setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);
}

//==============================================================================
MainComponent::~MainComponent()
{
    processor.setOscilloscope(nullptr);
    setLookAndFeel (nullptr);
}