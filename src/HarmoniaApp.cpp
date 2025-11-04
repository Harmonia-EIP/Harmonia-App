#include "HarmoniaApp.h"
#include "MainWindow.h"

const juce::String HarmoniaApp::getApplicationName()    { return "HarmoniaApp"; }
const juce::String HarmoniaApp::getApplicationVersion() { return "0.3.1"; }

void HarmoniaApp::initialise(const juce::String&)
{
    // --- Initialisation du logger ---
    auto exeDir = juce::File::getSpecialLocation(juce::File::currentExecutableFile)
                      .getParentDirectory();

    // --- Fichier de log dans le même dossier que l’exe ---
    auto logFile = exeDir.getChildFile("HarmoniaLogs.txt");

    if (!logFile.existsAsFile())
        logFile.create(); // crée un fichier vide si inexistant

    auto* logStream = new juce::FileLogger(logFile, "=== Harmonia Started ===", 0);
    juce::Logger::setCurrentLogger(logStream);
    juce::Logger::writeToLog("📘 Logger initialisé dans le dossier de l’exécutable");
    // --- Supabase Manager ---
    supabase = std::make_unique<SupabaseManager>();

    // --- Chargement d'une session existante ---
    auto session = supabase->loadSession();

    if (session.has_value())
    {
        juce::Logger::writeToLog("🔍 Session trouvée pour " + session->email);

        // Vérifie si le token est expiré
        if (juce::Time::getCurrentTime() > session->expiresAt)
        {
            juce::Logger::writeToLog("🕐 Token expiré, tentative de refresh...");

            auto refreshed = supabase->refreshSession(session->refreshToken);
            if (refreshed.has_value())
            {
                juce::Logger::writeToLog("✅ Session rafraîchie !");
                session = refreshed;
                supabase->saveSession(*session);
                mainWindow = std::make_unique<MainWindow>("HarmoniaApp", *this, *supabase, session);
                return;
            }
            else
            {
                juce::Logger::writeToLog("❌ Refresh échoué, retour à l’écran de login");
            }
        }
        else
        {
            juce::Logger::writeToLog("✅ Token encore valide, connexion automatique !");
            mainWindow = std::make_unique<MainWindow>("HarmoniaApp", *this, *supabase, session);
            return;
        }
    }

    // --- Aucune session valide ---
    juce::Logger::writeToLog("🚪 Aucune session, ouverture de l’écran de connexion");
    mainWindow = std::make_unique<MainWindow>("HarmoniaApp", *this, *supabase, std::nullopt);
}

void HarmoniaApp::shutdown()
{
    juce::Logger::writeToLog("🛑 Fermeture de l’application...");
    mainWindow = nullptr;
    supabase = nullptr;

    juce::Logger::writeToLog("📕 Application fermée proprement.");
    juce::Logger::setCurrentLogger(nullptr);
}
