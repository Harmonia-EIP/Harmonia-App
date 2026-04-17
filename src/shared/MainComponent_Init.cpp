#include "MainComponent.h"

void MainComponent::initUI(const UserSession& session)
{
    setLookAndFeel (&appLookAndFeel);

    applyTheme(ThemeAndLayoutConverter::idToThemePreset(session.themeId));
    applyLayout(ThemeAndLayoutConverter::idToLayoutPreset(session.layoutId));

    sendLookAndFeelChange();
}

void MainComponent::addComponents()
{
    addAndMakeVisible(title);
    addAndMakeVisible(topBar);
    addAndMakeVisible(oscilloscope);
    addAndMakeVisible(freqVolComponent);
    addAndMakeVisible(adsrComponent);
    addAndMakeVisible(filterComponent);
    addAndMakeVisible(bottomBar);
    addAndMakeVisible(synthComponent);
}

void MainComponent::initComponentsListeners()
{
    freqVolComponent.onParamsChanged = [this](){updateSynthParamsFromUI();};
    adsrComponent.onParamsChanged    = [this](){updateSynthParamsFromUI();};
    filterComponent.onParamsChanged  = [this](){updateSynthParamsFromUI();};
    topBar.onParamsChanged           = [this](){updateSynthParamsFromUI();};
}


void MainComponent::initTitleComponent()
{
    title.onLayoutSelected = [this](LayoutPreset layout)
    {
        applyLayout(layout);
        backend.updateLayout(ThemeAndLayoutConverter::layoutPresetToId(layout));
    };

    title.onThemeSelected = [this] (ThemePreset p)
    {
        applyTheme(p);
        backend.updateTheme(ThemeAndLayoutConverter::themePresetToId(p));
    };
    title.onLogout = [this]()
    {
        backend.clearSession();
        if (title.onLogout) title.onLogout();
    };
}

void MainComponent::initAsyncProfileLoad()
{
    std::thread([this]()
    {
        auto result = backend.getProfile();

        if (!result.success)
            return;

        juce::MessageManager::callAsync([this, result]()
        {
            applyTheme(ThemeAndLayoutConverter::idToThemePreset(result.profile.themeId));
            applyLayout(ThemeAndLayoutConverter::idToLayoutPreset(result.profile.layoutId));
        });

    }).detach();
}