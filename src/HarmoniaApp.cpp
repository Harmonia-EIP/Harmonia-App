#include "HarmoniaApp.h"
#include "MainWindow.h"

const juce::String HarmoniaApp::getApplicationName()    { return "HarmoniaApp"; }
const juce::String HarmoniaApp::getApplicationVersion() { return "0.3.1"; }

void HarmoniaApp::initialise(const juce::String&)
{
    // --- Initialisation du logger ---
    auto exeDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                      .getParentDirectory();

    auto logFile = exeDir.getChildFile("HarmoniaLogs.txt");

    if (!logFile.existsAsFile())
        logFile.create(); 

    auto* logStream = new juce::FileLogger(logFile, "=== Harmonia Started ===", 0);
    juce::Logger::setCurrentLogger(logStream);
    juce::Logger::writeToLog("Logger initialisé dans le dossier de l’exécutable");

    // --- Initialisation du BackendAuthManager ---
    backend = std::make_unique<BackendAuthManager>();

    // --- Vérification d'une session existante ---
    auto session = backend->loadSession();

    if (session.has_value())
    {
        juce::Logger::writeToLog("Session trouvée pour " + session->email);

        // Vérifie expiration locale
        if (juce::Time::getCurrentTime() > session->expiresAt)
        {
            juce::Logger::writeToLog("Token expiré. Vous devrez vous reconnecter.");

            // Vous n'avez pas encore de refresh côté backend :
            // -> On supprime simplement la session et on ouvre le login.
            backend->clearSession();
            mainWindow = std::make_unique<MainWindow>("HarmoniaApp", *this, *backend, std::nullopt);
            return;
        }

        // ✅ Token encore valide → connexion automatique
        juce::Logger::writeToLog("Connexion automatique !");
        mainWindow = std::make_unique<MainWindow>("HarmoniaApp", *this, *backend, session);
        return;
    }

    // --- Aucune session trouvée ---
    juce::Logger::writeToLog("Aucune session, ouverture de l’écran de connexion");
    mainWindow = std::make_unique<MainWindow>("HarmoniaApp", *this, *backend, std::nullopt);
}

void HarmoniaApp::shutdown()
{
    juce::Logger::writeToLog("Fermeture de l’application...");
    mainWindow = nullptr;
    backend = nullptr;

    juce::Logger::writeToLog("Application fermée proprement.");
    juce::Logger::setCurrentLogger(nullptr);
}
