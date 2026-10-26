#pragma once

#include <juce_core/juce_core.h>

/**
 * Chemins des fichiers du backend.
 *
 * Aucune de ces fonctions ne crée quoi que ce soit : le dossier est créé
 * "à la demande" par celui qui écrit (Logger, SessionStore). Ça évite de
 * toucher au disque quand un DAW instancie le plugin juste pour le scanner.
 */
namespace AppPaths
{
    inline juce::File getAppDataDir()
    {
        return juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory)
                   .getChildFile ("Harmonia");
    }

    inline juce::File getSessionFile() { return getAppDataDir().getChildFile ("HarmoniaSession.json"); }
    inline juce::File getLogFile()     { return getAppDataDir().getChildFile ("HarmoniaLogs.txt"); }
}
