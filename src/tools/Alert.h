#pragma once
#include <juce_gui_basics/juce_gui_basics.h>

class Alert
{
public:
    static void show(juce::AlertWindow::AlertIconType type,
                     const juce::String& title,
                     const juce::String& message)
    {
        juce::AlertWindow::showMessageBoxAsync(type, title, message);
    }

    static void warning(const juce::String& title, const juce::String& message)
    {
        show(juce::AlertWindow::WarningIcon, title, message);
    }

    static void info(const juce::String& title, const juce::String& message)
    {
        show(juce::AlertWindow::InfoIcon, title, message);
    }

    static void error(const juce::String& title, const juce::String& message)
    {
        show(juce::AlertWindow::WarningIcon, title, message);
    }
};