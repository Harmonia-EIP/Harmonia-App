#include <catch2/catch_test_macros.hpp>
#include <catch2/generators/catch_generators.hpp>
#include <set>
#include <string>
#include <typeinfo>
#include <vector>

#include "PluginProcessor.h"
#include "MainComponent.h"
#include "components/LockableControl.h"
#include "backend/BackendTypes.h"

// Tous les tests sont en mode guest : pas de session utilisateur,
// donc aucun thread / requête backend à gérer.

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

    // proc est déclaré avant ui : il est détruit après lui (ordre inverse de construction).
    struct Fixture
    {
        HarmoniaAudioProcessor proc;
        MainComponent ui;

        explicit Fixture (bool prepare = true, bool setSize = true)
            : proc(),
              ui ((prepare ? (proc.prepareToPlay (48000.0, 512), 0) : 0,
                   proc), proc.getBackend(), makeGuest())
        {
            if (setSize)
                ui.setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);
        }
    };

    void collect (juce::Component& c, std::vector<juce::Component*>& out)
    {
        for (int i = 0; i < c.getNumChildComponents(); ++i)
        {
            auto* child = c.getChildComponent (i);
            out.push_back (child);
            collect (*child, out);
        }
    }

    template <typename T>
    void collectOfType (juce::Component& c, std::vector<T*>& out)
    {
        for (int i = 0; i < c.getNumChildComponents(); ++i)
        {
            auto* child = c.getChildComponent (i);
            if (auto* t = dynamic_cast<T*> (child))
                out.push_back (t);
            collectOfType<T> (*child, out);
        }
    }

    void pump (int ms = 50)
    {
        juce::MessageManager::getInstance()->runDispatchLoopUntil (ms);
    }

    void renderBlocks (HarmoniaAudioProcessor& proc, int n)
    {
        juce::AudioBuffer<float> buf (2, 512);
        juce::MidiBuffer midi;
        for (int i = 0; i < n; ++i)
        {
            buf.clear();
            proc.processBlock (buf, midi);
        }
    }
}

// ============================================================
// Cycle de vie
// ============================================================

TEST_CASE ("UI: construction et destruction répétées", "[ui][lifecycle]")
{
    HarmoniaAudioProcessor proc;
    proc.prepareToPlay (48000.0, 512);

    for (int i = 0; i < 3; ++i)
    {
        MainComponent ui (proc, proc.getBackend(), makeGuest());
        ui.setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);
        pump();
        renderBlocks (proc, 1);
    }
    renderBlocks (proc, 1);
}

TEST_CASE ("UI: s'ouvre même si prepareToPlay n'a pas été appelé", "[ui][lifecycle]")
{
    // Dans un vrai hôte, l'éditeur peut être créé avant prepareToPlay.
    Fixture f (false);
    pump();
    REQUIRE (f.ui.getNumChildComponents() > 0);
}

TEST_CASE ("UI: construction sans setSize ne plante pas", "[ui][lifecycle]")
{
    Fixture f (true, false);
    pump();
    SUCCEED();
}

TEST_CASE ("UI: la boucle de messages peut tourner après le chargement", "[ui][lifecycle]")
{
    Fixture f;
    pump (200);
    REQUIRE (f.ui.getNumChildComponents() > 0);
}

TEST_CASE ("UI: destruction pendant que l'audio tourne", "[ui][lifecycle]")
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
    renderBlocks (proc, 10);
    SUCCEED();
}

// ============================================================
// Arbre de composants
// ============================================================

TEST_CASE ("Arbre: l'UI contient des composants", "[ui][tree]")
{
    Fixture f;

    std::vector<juce::Component*> all;
    collect (f.ui, all);

    INFO ("nombre de composants: " << all.size());
    REQUIRE (! all.empty());
}

TEST_CASE ("Arbre: chaque enfant référence bien son parent", "[ui][tree]")
{
    Fixture f;

    std::vector<juce::Component*> all;
    collect (f.ui, all);

    for (auto* c : all)
    {
        INFO (typeid (*c).name());
        REQUIRE (c->getParentComponent() != nullptr);
    }
}

TEST_CASE ("Arbre: pas de composant enfant nul", "[ui][tree]")
{
    Fixture f;

    std::vector<juce::Component*> stack { &f.ui };
    while (! stack.empty())
    {
        auto* c = stack.back();
        stack.pop_back();
        for (int i = 0; i < c->getNumChildComponents(); ++i)
        {
            auto* child = c->getChildComponent (i);
            REQUIRE (child != nullptr);
            stack.push_back (child);
        }
    }
}

TEST_CASE ("Arbre: l'UI contient des sliders, des boutons et un clavier", "[ui][tree]")
{
    Fixture f;

    std::vector<juce::Slider*> sliders;
    std::vector<juce::Button*> buttons;
    std::vector<juce::MidiKeyboardComponent*> keyboards;
    collectOfType (f.ui, sliders);
    collectOfType (f.ui, buttons);
    collectOfType (f.ui, keyboards);

    INFO ("sliders=" << sliders.size() << " buttons=" << buttons.size());
    REQUIRE (! sliders.empty());
    REQUIRE (! buttons.empty());
    REQUIRE (keyboards.size() == 1);
}

// ============================================================
// Layout
// ============================================================

TEST_CASE ("Layout: aucun composant visible ne sort de son parent", "[ui][layout]")
{
    const auto scale = GENERATE (1.0f, 1.25f, 1.5f);

    Fixture f (true, false);
    f.ui.setSize ((int) (AppConfig::DefaultWidth  * scale),
                  (int) (AppConfig::DefaultHeight * scale));

    std::vector<juce::Component*> all;
    collect (f.ui, all);
    REQUIRE (! all.empty());

    for (auto* c : all)
    {
        if (! c->isVisible() || c->getWidth() == 0 || c->getHeight() == 0)
            continue;

        std::string chain;
        for (auto* p = c; p != nullptr; p = p->getParentComponent())
            chain += std::string (typeid (*p).name()) + " <- ";

        INFO ("scale: " << scale
              << "\nchaine: " << chain
              << "\nbounds: " << c->getBounds().toString().toStdString()
              << " parent: " << c->getParentComponent()->getLocalBounds().toString().toStdString());

        REQUIRE (c->getParentComponent()->getLocalBounds().contains (c->getBounds()));
    }
}

TEST_CASE ("Layout: les sections principales ne se chevauchent pas", "[ui][layout]")
{
    Fixture f;

    std::vector<juce::Component*> blocks;
    for (int i = 0; i < f.ui.getNumChildComponents(); ++i)
    {
        auto* c = f.ui.getChildComponent (i);
        if (c->isVisible() && c->getBounds() != f.ui.getLocalBounds())   // exclut le fond
            blocks.push_back (c);
    }

    INFO ("nombre de sections: " << blocks.size());
    for (auto* b : blocks)
        INFO (typeid (*b).name() << " " << b->getBounds().toString().toStdString());

    REQUIRE (blocks.size() >= 8);   // header, 7 panneaux, clavier (à ajuster si le layout change)

    for (size_t i = 0; i < blocks.size(); ++i)
        for (size_t j = i + 1; j < blocks.size(); ++j)
        {
            INFO (typeid (*blocks[i]).name() << " " << blocks[i]->getBounds().toString().toStdString()
                  << " vs " << typeid (*blocks[j]).name() << " " << blocks[j]->getBounds().toString().toStdString());
            REQUIRE_FALSE (blocks[i]->getBounds().intersects (blocks[j]->getBounds()));
        }
}

TEST_CASE ("Layout: les sections sont dans les limites de la fenêtre", "[ui][layout]")
{
    Fixture f;

    for (int i = 0; i < f.ui.getNumChildComponents(); ++i)
    {
        auto* c = f.ui.getChildComponent (i);
        if (! c->isVisible())
            continue;
        INFO (typeid (*c).name() << " " << c->getBounds().toString().toStdString());
        REQUIRE (f.ui.getLocalBounds().contains (c->getBounds()));
    }
}

TEST_CASE ("Layout: les contrôles visibles ont une taille non nulle", "[ui][layout]")
{
    Fixture f;

    std::vector<LockableControl*> lockables;
    collectOfType (f.ui, lockables);
    REQUIRE (! lockables.empty());

    for (auto* l : lockables)
    {
        // LockableControl n'est pas forcément un juce::Component : on passe par le cast
        auto* comp = dynamic_cast<juce::Component*> (l);
        INFO (l->getParamId().toStdString());
        REQUIRE (comp != nullptr);

        if (! comp->isVisible())
            continue;

        REQUIRE (comp->getWidth()  > 0);
        REQUIRE (comp->getHeight() > 0);
    }
}

TEST_CASE ("Layout: redimensionnements successifs sans plantage", "[ui][layout]")
{
    Fixture f;

    const int sizes[][2] = {
        { AppConfig::DefaultWidth,       AppConfig::DefaultHeight },
        { AppConfig::DefaultWidth * 2,   AppConfig::DefaultHeight * 2 },
        { AppConfig::DefaultWidth,       AppConfig::DefaultHeight },
        { AppConfig::DefaultWidth * 3/2, AppConfig::DefaultHeight * 3/2 },
    };

    for (auto& s : sizes)
    {
        f.ui.setSize (s[0], s[1]);
        pump (10);
        REQUIRE (f.ui.getWidth()  == s[0]);
        REQUIRE (f.ui.getHeight() == s[1]);
    }
}

// ============================================================
// Contrôles et paramètres
// ============================================================

TEST_CASE ("Contrôles: chaque paramètre a exactement un contrôle à l'écran", "[ui][params]")
{
    Fixture f;

    std::vector<LockableControl*> lockables;
    collectOfType (f.ui, lockables);

    std::multiset<juce::String> ids;
    for (auto* l : lockables)
        ids.insert (l->getParamId());

    // les 20 paramètres historiques sont tous à l'écran...
    for (const auto& id : HarmoniaParams::orderedIds())
    {
        INFO (id.toStdString());
        REQUIRE (ids.count (id) == 1);
    }

    // ...plus les essentiels v3 (unison, width, interval, slope, delay, size), jamais en double
    for (const auto& id : ids)
    {
        INFO (id.toStdString());
        REQUIRE (ids.count (id) == 1);
    }
    REQUIRE (ids.size() > HarmoniaParams::orderedIds().size());
}

TEST_CASE ("Contrôles: chaque contrôle est lié à un paramètre existant", "[ui][params]")
{
    Fixture f;

    std::vector<LockableControl*> lockables;
    collectOfType (f.ui, lockables);
    REQUIRE (! lockables.empty());

    for (auto* l : lockables)
    {
        INFO (l->getParamId().toStdString());
        REQUIRE (f.proc.getAPVTS().getParameter (l->getParamId()) != nullptr);
    }
}

TEST_CASE ("Contrôles: aucun contrôle verrouillé au départ", "[ui][params]")
{
    Fixture f;

    std::vector<LockableControl*> lockables;
    collectOfType (f.ui, lockables);
    REQUIRE (! lockables.empty());

    for (auto* l : lockables)
    {
        INFO (l->getParamId().toStdString());
        REQUIRE_FALSE (l->isLocked());
    }
}

TEST_CASE ("Contrôles: changer chaque paramètre avec l'UI ouverte ne plante pas", "[ui][params]")
{
    Fixture f;

    for (const auto& id : HarmoniaParams::orderedIds())
    {
        auto* p = f.proc.getAPVTS().getParameter (id);
        REQUIRE (p != nullptr);

        const float original = p->getValue();
        p->setValueNotifyingHost (0.0f);
        p->setValueNotifyingHost (1.0f);
        p->setValueNotifyingHost (original);
    }
    pump (100);
    SUCCEED();
}

// ============================================================
// État UI (identité / preset)
// ============================================================

TEST_CASE ("État UI: l'identité détermine ce qui est restauré", "[ui][state]")
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

TEST_CASE ("État UI: l'identité guest est écrite au chargement", "[ui][state]")
{
    Fixture f;
    REQUIRE (f.proc.getAPVTS().state[UiState::identity].toString() == "guest");
}

TEST_CASE ("État UI: réouvrir l'UI conserve l'état guest", "[ui][state]")
{
    HarmoniaAudioProcessor proc;
    auto& st = proc.getAPVTS().state;
    st.setProperty (UiState::identity,   "guest",    nullptr);
    st.setProperty (UiState::presetName, "PERSISTE", nullptr);

    for (int i = 0; i < 3; ++i)
    {
        MainComponent ui (proc, proc.getBackend(), makeGuest());
        REQUIRE (st[UiState::presetName].toString() == "PERSISTE");
    }
}

// ============================================================
// Label de preset
// ============================================================

TEST_CASE ("Preset: le label affiche l'état par défaut", "[ui][preset]")
{
    Fixture f;

    std::vector<juce::Label*> labels;
    collectOfType (f.ui, labels);

    bool found = false;
    for (auto* l : labels)
        if (l->getText() == Strings::Labels::UnsetPreset.toUpperCase())
            found = true;
    REQUIRE (found);
}

TEST_CASE ("Preset: le nom restauré vient du state", "[ui][preset]")
{
    HarmoniaAudioProcessor proc;
    auto& st = proc.getAPVTS().state;
    st.setProperty (UiState::identity,   "guest",   nullptr);
    st.setProperty (UiState::presetName, "MON SON", nullptr);

    MainComponent ui (proc, proc.getBackend(), makeGuest());

    std::vector<juce::Label*> labels;
    collectOfType (ui, labels);

    bool found = false;
    for (auto* l : labels)
        if (l->getText() == "MON SON")
            found = true;
    REQUIRE (found);
}

// ============================================================
// Lien UI <-> audio
// ============================================================

TEST_CASE ("Audio: le clavier de l'UI fait sonner le synthé", "[ui][audio]")
{
    Fixture f;

    f.proc.getKeyboardState().noteOn (1, 60, 0.8f);

    juce::AudioBuffer<float> buf (2, 512);
    juce::MidiBuffer midi;
    float peak = 0.0f;
    for (int i = 0; i < 20; ++i)
    {
        f.proc.processBlock (buf, midi);
        peak = juce::jmax (peak, buf.getMagnitude (0, 512));
    }
    REQUIRE (peak > 0.001f);
}

TEST_CASE ("Audio: processBlock tourne pendant que l'UI redessine", "[ui][audio]")
{
    Fixture f;
    f.proc.getKeyboardState().noteOn (1, 64, 0.8f);

    for (int i = 0; i < 5; ++i)
    {
        renderBlocks (f.proc, 4);
        f.ui.repaint();
        pump (20);
    }
    SUCCEED();
}