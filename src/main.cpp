#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_gui_extra/juce_gui_extra.h>
#include <juce_events/juce_events.h>

class MainComponent : public juce::Component
{
public:
    MainComponent()
    {
        setSize (400, 300);
    }

    void paint (juce::Graphics& g) override
    {
        g.fillAll (juce::Colours::darkslategrey);
        g.setColour (juce::Colours::white);
        g.setFont (30.0f);
        g.drawFittedText ("Hello JUCE!", getLocalBounds(), juce::Justification::centred, 1);
    }
};

class MainWindow : public juce::DocumentWindow
{
public:
    MainWindow(juce::String name) : DocumentWindow(name,
        juce::Colours::lightgrey,
        DocumentWindow::allButtons)
    {
        setUsingNativeTitleBar(true);
        setContentOwned(new MainComponent(), true);
        centreWithSize(getWidth(), getHeight());
        setVisible(true);
    }

    void closeButtonPressed() override
    {
        juce::JUCEApplication::getInstance()->systemRequestedQuit();
    }
};

class MyJUCEApp : public juce::JUCEApplication
{
public:
    const juce::String getApplicationName() override       { return "MyJUCEApp"; }
    const juce::String getApplicationVersion() override    { return "0.1.0"; }
    void initialise(const juce::String&) override          { mainWindow.reset(new MainWindow("MyJUCEApp")); }
    void shutdown() override                               { mainWindow = nullptr; }

private:
    std::unique_ptr<MainWindow> mainWindow;
};

START_JUCE_APPLICATION(MyJUCEApp)
