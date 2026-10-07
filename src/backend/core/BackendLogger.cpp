#include "BackendLogger.h"
#include "AppPaths.h"

BackendLogger::BackendLogger()
    : logFile (AppPaths::getLogFile())
{
}

void BackendLogger::log (Level level, const juce::String& message)
{
    const char* tag = "INFO";

    if (level == Level::Warn)       tag = "WARN";
    else if (level == Level::Error) tag = "ERROR";

    const auto line = juce::Time::getCurrentTime().formatted ("%Y-%m-%d %H:%M:%S")
                      + " [" + tag + "] " + message + "\n";

    const std::lock_guard<std::mutex> lock (mutex);

    if (logFile == juce::File())
        return;

    logFile.getParentDirectory().createDirectory();
    rotateIfNeeded();
    logFile.appendText (line);
}

void BackendLogger::rotateIfNeeded()
{
    // Appelé avec le mutex déjà pris.
    if (logFile.getSize() <= maxFileSizeBytes)
        return;

    const auto previous = logFile.withFileExtension ("old.txt");   // HarmoniaLogs.old.txt

    previous.deleteFile();
    logFile.moveFileTo (previous);
}
