#include "MainComponent.h"
#include "themes/AppColourIds.h"

#include <filesystem>
#include <fstream>
#include <thread>

MainComponent::MainComponent (BackendManager& be, const UserSession& session)
    : AudioAppComponent(),
      backend (be),
      title ("Harmonia", session),
      oscilloscope (appLookAndFeel, AppConfig::Oscilloscope::BufferSize, AppConfig::Oscilloscope::RefreshRate),
      freqVolComponent (appLookAndFeel),
      adsrComponent (appLookAndFeel),
      filterComponent (appLookAndFeel)
{   
    initUI(session);
    initAsyncProfileLoad();
    addComponents();
    initComponentsListeners();
    initTitleComponent();
    initSynth();
    
    initExportParameters();

    initLoadParameters();
    initGenerateWithAI();

    layoutPreset = ThemeAndLayoutConverter::idToLayoutPreset(session.layoutId);

    setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);
    setAudioChannels (0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
    setLookAndFeel (nullptr);
}
