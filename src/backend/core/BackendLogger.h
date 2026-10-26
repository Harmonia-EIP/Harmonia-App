#pragma once

#include <juce_core/juce_core.h>

#include <mutex>

/**
 * Log fichier thread-safe avec niveaux, horodatage et rotation (~1 Mo).
 *
 * Nommé BackendLogger (et pas Logger) parce que juce::Logger existe : avec
 * `using namespace juce;` le nom serait ambigu.
 *
 * Utilisé via juce::SharedResourcePointer<BackendLogger> : toutes les instances
 * du plugin dans le même processus partagent le même objet, donc le même mutex
 * (avant, plusieurs instances faisaient appendText() sur le même fichier sans
 * verrou commun).
 *
 * Ne crée le dossier / le fichier qu'à la première écriture.
 */
class BackendLogger
{
public:
    enum class Level { Info, Warn, Error };

    BackendLogger();

    void log (Level level, const juce::String& message);

    void info  (const juce::String& message) { log (Level::Info,  message); }
    void warn  (const juce::String& message) { log (Level::Warn,  message); }
    void error (const juce::String& message) { log (Level::Error, message); }

    const juce::File& getLogFile() const noexcept { return logFile; }

private:
    void rotateIfNeeded();

    static constexpr juce::int64 maxFileSizeBytes = 1024 * 1024;

    const juce::File logFile;
    std::mutex mutex;
};
