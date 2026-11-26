#include <catch2/catch_test_macros.hpp>

#include "PluginProcessor.h"
#include "MainComponent.h"
#include "backend/BackendTypes.h"
#include "components/LockableControl.h"

// Capture de l'interface en PNG, pour vérifier un changement de layout sans lancer le plugin.
// Masqué (tag [.snapshot]) : ne tourne que sur demande.
//
//   Harmonia_Tests.exe "[.snapshot]"            -> harmonia_ui.png dans le dossier temporaire
//   HARMONIA_SNAPSHOT=C:\chemin\ui.png ...       -> à cet endroit

TEST_CASE ("Capture de l'interface (guest)", "[.snapshot]")
{
    HarmoniaAudioProcessor proc;
    proc.prepareToPlay (48000.0, 512);

    UserSession guest;
    guest.isGuest = true;
    guest.pseudo  = "Guest";

    MainComponent ui (proc, proc.getBackend(), guest);
    ui.setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);

    // HARMONIA_SNAPSHOT_REFINE=1 : cadenas visibles, comme en mode Refine
    if (juce::SystemStats::getEnvironmentVariable ("HARMONIA_SNAPSHOT_REFINE", {}) == "1")
    {
        std::function<void (juce::Component&)> showLocks = [&] (juce::Component& c)
        {
            for (auto* child : c.getChildren())
            {
                if (auto* l = dynamic_cast<LockableControl*> (child))
                    l->setLockUiVisible (true);
                showLocks (*child);
            }
        };
        showLocks (ui);
    }

    juce::MessageManager::getInstance()->runDispatchLoopUntil (200);

    const auto image = ui.createComponentSnapshot (ui.getLocalBounds(), true, 1.0f);

    const auto envPath = juce::SystemStats::getEnvironmentVariable ("HARMONIA_SNAPSHOT", {});
    const auto file = envPath.isNotEmpty() ? juce::File (envPath)
                                           : juce::File::getSpecialLocation (juce::File::tempDirectory)
                                                 .getChildFile ("harmonia_ui.png");
    file.deleteFile();

    juce::FileOutputStream out (file);
    REQUIRE (out.openedOk());
    REQUIRE (juce::PNGImageFormat().writeImageToStream (image, out));

    WARN ("Capture : " << file.getFullPathName());
}

TEST_CASE ("Layout: l'écran (oscilloscope) est pile au centre de l'interface", "[ui][layout]")
{
    HarmoniaAudioProcessor proc;

    UserSession guest;
    guest.isGuest = true;

    MainComponent ui (proc, proc.getBackend(), guest);

    for (const int width : { AppConfig::DefaultWidth, 1000, 1200 })
    {
        ui.setSize (width, AppConfig::DefaultHeight);

        // le panneau Display est le parent de l'écran de l'oscilloscope
        std::function<DisplayScreen* (juce::Component&)> find = [&] (juce::Component& c) -> DisplayScreen*
        {
            for (auto* child : c.getChildren())
            {
                if (auto* d = dynamic_cast<DisplayScreen*> (child)) return d;
                if (auto* d = find (*child)) return d;
            }
            return nullptr;
        };

        auto* screen = find (ui);
        REQUIRE (screen != nullptr);
        auto* panel = screen->getParentComponent();

        INFO ("largeur " << width << " : panneau " << panel->getBounds().toString());
        CHECK (std::abs (panel->getBounds().getCentreX() - width / 2) <= 1);
    }
}

#include "pages/PagesIncludes.h"
#include "pages/WelcomePage.h"
#include "pages/LoginPage.h"
#include "pages/SignupPage.h"

// Captures des pages d'accueil / connexion / inscription à la taille de la fenêtre.
//   Harmonia_Tests.exe "[.snapshot-pages]"   -> harmonia_welcome.png, _login.png, _signup.png (dossier temporaire)
TEST_CASE ("Capture des pages d'accueil et de connexion", "[.snapshot-pages]")
{
    HarmoniaAudioProcessor proc;
    auto& be = proc.getBackend();

    WelcomePage welcome;
    LoginPage   login  (be, [] (const UserSession&) {});
    SignupPage  signup (be, [] (const UserSession&) {});

    const auto dir = juce::File::getSpecialLocation (juce::File::tempDirectory);

    for (auto [page, name] : { std::pair<juce::Component*, const char*> { &welcome, "welcome" },
                               { &login, "login" }, { &signup, "signup" } })
    {
        page->setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);
        juce::MessageManager::getInstance()->runDispatchLoopUntil (150);

        const auto file = dir.getChildFile ("harmonia_" + juce::String (name) + ".png");
        file.deleteFile();
        juce::FileOutputStream out (file);
        REQUIRE (out.openedOk());
        REQUIRE (juce::PNGImageFormat().writeImageToStream (page->createComponentSnapshot (page->getLocalBounds()), out));
        WARN ("Capture : " << file.getFullPathName());
    }
}
