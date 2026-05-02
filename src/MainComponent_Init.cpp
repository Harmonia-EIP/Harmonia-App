#include "MainComponent.h"

void MainComponent::initUI(const UserSession& session)
{
    setLookAndFeel(&appLookAndFeel);

    applyTheme(ThemeAndLayoutConverter::idToThemePreset(session.themeId));

    layoutManager.setLayout(
        ThemeAndLayoutConverter::idToLayoutPreset(session.layoutId)
    );

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
    freqVolComponent.onParamsChanged = [this]() { onUIChanged(); };
    adsrComponent.onParamsChanged    = [this]() { onUIChanged(); };
    filterComponent.onParamsChanged  = [this]() { onUIChanged(); };
    topBar.onParamsChanged           = [this]() { onUIChanged(); };
}


void MainComponent::initTitleComponent()
{
    title.onLayoutSelected = [this](LayoutPreset layout)
    {
        applyLayout(layout);

        auto layoutId = ThemeAndLayoutConverter::layoutPresetToId(layout);

        auto sessionOpt = backend.loadSession();
        if (sessionOpt)
        {
            auto session = *sessionOpt;
            session.layoutId = layoutId;
            backend.saveSession(session);
        }

        juce::Component::SafePointer<MainComponent> safeThis(this);
        
        juce::Thread::launch([safeThis, layout]
            {
            if (safeThis == nullptr) return;

            safeThis->backend.updateLayout(ThemeAndLayoutConverter::layoutPresetToId(layout));
        });
    };

    title.onThemeSelected = [this] (ThemePreset preset)
    {
        applyTheme(preset);

        auto themeId = ThemeAndLayoutConverter::themePresetToId(preset);

        auto sessionOpt = backend.loadSession();
        if (sessionOpt)
        {
            auto session = *sessionOpt;
            session.themeId = themeId;
            backend.saveSession(session);
        }
        
        juce::Component::SafePointer<MainComponent> safeThis(this);

        juce::Thread::launch([safeThis, preset]
            {
            if (safeThis == nullptr) return;
            safeThis->backend.updateTheme(ThemeAndLayoutConverter::themePresetToId(preset));
        });
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

void MainComponent::initGenerateWithAI()
{
    bottomBar.onGenerateClicked = [this]()
    {
        auto prompt = topBar.getPrompt();

        if (prompt.trim().isEmpty())
        {
            Alert::warning(
                Strings::Errors::MissingPrompt,
                Strings::Errors::MissingPromptAdvice
            );
            return;
        }

        aiManager.generate(prompt);
    };
}

void MainComponent::initAIManager()
{
    aiManager.onSuccess = [this](const PatchParams& p)
    {
        patchController.setPatch(p);
        patchController.applyToProcessor();
    };

    aiManager.onError = [](const juce::String& msg)
    {
        juce::AlertWindow::showMessageBoxAsync(
            juce::AlertWindow::WarningIcon,
            Strings::Errors::AiError,
            msg);
    };
}