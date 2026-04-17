#include "MainComponent.h"
#include "themes/AppColourIds.h"

#include <filesystem>
#include <fstream>
#include <thread>

MainComponent::MainComponent (BackendManager& be, const UserSession& session)
    : AudioAppComponent(),
      backend (be),
      title ("Harmonia", session),
      oscilloscope (appLookAndFeel, 8192, 60),
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

    setSize (800, 600);
    setAudioChannels (0, 2);
}

MainComponent::~MainComponent()
{
    shutdownAudio();
    setLookAndFeel (nullptr);
}
