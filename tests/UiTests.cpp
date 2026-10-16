#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <set>
#include <typeinfo>

#include "PluginProcessor.h"
#include "MainComponent.h"
#include "components/LockableControl.h"

namespace
{
    UserSession makeGuest()
    {
        UserSession g;
        g.isGuest = true;
        g.userId  = 0;
        g.pseudo  = "Guest";
        g.paletteColours.clear();
        g.paletteSlot = 0;
        return g;
    }

    UserSession makeUser()
    {
        UserSession s;
        s.isGuest = false;
        s.userId  = 7;
        s.pseudo  = "tester";
        s.paletteSlot = 0;
        s.expiresAt = juce::Time::getCurrentTime() + juce::RelativeTime::days (1);
        return s;
    }

    void collect (juce::Component& c, std::vector<juce::Component*>& out)
    {
        for (int i = 0; i < c.getNumChildComponents(); ++i)
        {
            auto* child = c.getChildComponent (i);
            out.push_back (child);
            collect (*child, out);
        }
    }

    void collectLockables (juce::Component& c, std::vector<LockableControl*>& out)
    {
        for (int i = 0; i < c.getNumChildComponents(); ++i)
        {
            auto* child = c.getChildComponent (i);
            if (auto* l = dynamic_cast<LockableControl*> (child))
                out.push_back (l);
            collectLockables (*child, out);
        }
    }

    void collectLabels (juce::Component& c, std::vector<juce::Label*>& out)
    {
        for (int i = 0; i < c.getNumChildComponents(); ++i)
        {
            auto* child = c.getChildComponent (i);
            if (auto* l = dynamic_cast<juce::Label*> (child))
                out.push_back (l);
            collectLabels (*child, out);
        }
    }

    void pump (int ms = 50)
    {
        juce::MessageManager::getInstance()->runDispatchLoopUntil (ms);
    }
}

TEST_CASE ("MainComponent: construction et destruction répétées")
{
    HarmoniaAudioProcessor proc;
    proc.prepareToPlay (48000.0, 512);

    for (int i = 0; i < 3; ++i)
    {
        MainComponent ui (proc, proc.getBackend(), makeGuest());
        ui.setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);
        pump();

        juce::AudioBuffer<float> buf (2, 512);
        juce::MidiBuffer midi;
        proc.processBlock (buf, midi);
    }

    juce::AudioBuffer<float> buf (2, 512);
    juce::MidiBuffer midi;
    proc.processBlock (buf, midi);
}

TEST_CASE ("Layout: aucun composant visible ne sort de son parent")
{
    const bool guest = GENERATE (true, false);
    const auto scale = GENERATE (1.0f, 1.5f);   // 0.8 retiré : taille jamais atteinte en vrai

    HarmoniaAudioProcessor proc;
    MainComponent ui (proc, proc.getBackend(), guest ? makeGuest() : makeUser());
    ui.setSize ((int) (AppConfig::DefaultWidth  * scale),
                (int) (AppConfig::DefaultHeight * scale));

    std::vector<juce::Component*> all;
    collect (ui, all);
    REQUIRE (! all.empty());

    for (auto* c : all)
    {
        if (! c->isVisible() || c->getWidth() == 0 || c->getHeight() == 0)
            continue;

        std::string chain;
        for (auto* p = c; p != nullptr; p = p->getParentComponent())
            chain += std::string (typeid (*p).name()) + " <- ";

        INFO ("chaine: " << chain
              << "\nbounds: " << c->getBounds().toString().toStdString()
              << " parent: " << c->getParentComponent()->getLocalBounds().toString().toStdString());

        REQUIRE (c->getParentComponent()->getLocalBounds().contains (c->getBounds()));
    }
}

TEST_CASE ("Layout: les sections principales ne se chevauchent pas")
{
    const bool guest = GENERATE (true, false);

    HarmoniaAudioProcessor proc;
    MainComponent ui (proc, proc.getBackend(), guest ? makeGuest() : makeUser());
    ui.setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);

    std::vector<juce::Component*> blocks;
    for (int i = 0; i < ui.getNumChildComponents(); ++i)
    {
        auto* c = ui.getChildComponent (i);
        if (c->isVisible() && c->getBounds() != ui.getLocalBounds())   // exclut le fond (particules)
            blocks.push_back (c);
    }
    REQUIRE (blocks.size() >= 8);   // header, 7 panneaux, clavier

    for (size_t i = 0; i < blocks.size(); ++i)
        for (size_t j = i + 1; j < blocks.size(); ++j)
        {
            INFO (blocks[i]->getBounds().toString().toStdString() << " vs "
                  << blocks[j]->getBounds().toString().toStdString());
            REQUIRE_FALSE (blocks[i]->getBounds().intersects (blocks[j]->getBounds()));
        }
}

TEST_CASE ("État UI: l'identité détermine ce qui est restauré")
{
    HarmoniaAudioProcessor proc;
    auto& st = proc.getAPVTS().state;

    SECTION ("même identité: le nom de preset est conservé")
    {
        st.setProperty (UiState::identity,   "guest",      nullptr);
        st.setProperty (UiState::presetName, "MON PRESET", nullptr);

        MainComponent ui (proc, proc.getBackend(), makeGuest());
        REQUIRE (st[UiState::presetName].toString() == "MON PRESET");
    }

    SECTION ("identité différente: l'ancien état est oublié")
    {
        st.setProperty (UiState::identity,    "user_42", nullptr);
        st.setProperty (UiState::presetName,  "ANCIEN",  nullptr);
        st.setProperty (UiState::paletteSlot, 2,         nullptr);

        MainComponent ui (proc, proc.getBackend(), makeGuest());
        REQUIRE (st[UiState::identity].toString() == "guest");
        REQUIRE_FALSE (st.hasProperty (UiState::presetName));
        REQUIRE_FALSE (st.hasProperty (UiState::paletteSlot));
    }
}

TEST_CASE ("Guest: chaque paramètre a exactement un contrôle à l'écran")
{
    HarmoniaAudioProcessor proc;
    MainComponent ui (proc, proc.getBackend(), makeGuest());
    ui.setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);

    std::vector<LockableControl*> lockables;
    collectLockables (ui, lockables);

    std::multiset<juce::String> ids;
    for (auto* l : lockables)
        ids.insert (l->getParamId());

    for (const auto& id : HarmoniaParams::orderedIds())
    {
        INFO (id.toStdString());
        REQUIRE (ids.count (id) == 1);
    }
    REQUIRE (ids.size() == HarmoniaParams::orderedIds().size());
}

TEST_CASE ("Guest: aucun contrôle verrouillé au départ")
{
    HarmoniaAudioProcessor proc;
    MainComponent ui (proc, proc.getBackend(), makeGuest());

    std::vector<LockableControl*> lockables;
    collectLockables (ui, lockables);
    REQUIRE (! lockables.empty());

    for (auto* l : lockables)
        REQUIRE_FALSE (l->isLocked());
}

TEST_CASE ("Guest: le clavier de l'UI fait sonner le synthé")
{
    HarmoniaAudioProcessor proc;
    proc.prepareToPlay (48000.0, 512);
    MainComponent ui (proc, proc.getBackend(), makeGuest());

    proc.getKeyboardState().noteOn (1, 60, 0.8f);

    juce::AudioBuffer<float> buf (2, 512);
    juce::MidiBuffer midi;
    float peak = 0.0f;
    for (int i = 0; i < 20; ++i)
    {
        proc.processBlock (buf, midi);
        peak = juce::jmax (peak, buf.getMagnitude (0, 512));
    }
    REQUIRE (peak > 0.001f);
}

TEST_CASE ("Guest: l'oscilloscope est détaché quand l'UI est fermée")
{
    HarmoniaAudioProcessor proc;
    proc.prepareToPlay (48000.0, 512);
    proc.getKeyboardState().noteOn (1, 60, 1.0f);

    juce::AudioBuffer<float> buf (2, 512);
    juce::MidiBuffer midi;
    {
        MainComponent ui (proc, proc.getBackend(), makeGuest());
        proc.processBlock (buf, midi);
    }
    for (int i = 0; i < 10; ++i)
        proc.processBlock (buf, midi);
    SUCCEED();
}

TEST_CASE ("Guest: le label de preset affiche l'état par défaut")
{
    HarmoniaAudioProcessor proc;
    MainComponent ui (proc, proc.getBackend(), makeGuest());

    std::vector<juce::Label*> labels;
    collectLabels (ui, labels);

    bool found = false;
    for (auto* l : labels)
        if (l->getText() == Strings::Labels::UnsetPreset.toUpperCase())
            found = true;
    REQUIRE (found);
}

TEST_CASE ("Guest: le nom de preset restauré vient du state")
{
    HarmoniaAudioProcessor proc;
    auto& st = proc.getAPVTS().state;
    st.setProperty (UiState::identity,   "guest",   nullptr);
    st.setProperty (UiState::presetName, "MON SON", nullptr);

    MainComponent ui (proc, proc.getBackend(), makeGuest());

    std::vector<juce::Label*> labels;
    collectLabels (ui, labels);

    bool found = false;
    for (auto* l : labels)
        if (l->getText() == "MON SON")
            found = true;
    REQUIRE (found);
}