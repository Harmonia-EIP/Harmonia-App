#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "PluginProcessor.h"
#include "MainComponent.h"
#include "backend/BackendTypes.h"

// Après un clic sur un knob / bouton, le clavier d'ordinateur doit jouer des notes
// tout de suite : le piano reprend le focus. Un clic dans un champ texte le garde.

namespace
{
    UserSession guest()
    {
        UserSession g;
        g.isGuest = true;
        g.pseudo  = "Guest";
        return g;
    }

    template <typename T>
    T* findFirst (juce::Component& c)
    {
        for (int i = 0; i < c.getNumChildComponents(); ++i)
        {
            auto* child = c.getChildComponent (i);
            if (auto* t = dynamic_cast<T*> (child))
                return t;
            if (auto* t = findFirst<T> (*child))
                return t;
        }
        return nullptr;
    }

    void pump (int ms = 50)
    {
        juce::MessageManager::getInstance()->runDispatchLoopUntil (ms);
    }

    // Relâchement de la souris sur `target`, tel que MainComponent le reçoit
    // de ses enfants (addMouseListener (this, true)).
    void releaseOn (MainComponent& ui, juce::Component& target)
    {
        const auto pos  = target.getLocalBounds().getCentre().toFloat();
        const auto now  = juce::Time::getCurrentTime();
        const juce::MouseEvent e (juce::Desktop::getInstance().getMainMouseSource(), pos,
                                  juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                  &target, &target, now, pos, now, 1, false);
        ui.mouseUp (e.getEventRelativeTo (&ui));
    }

    struct OnScreen
    {
        HarmoniaAudioProcessor proc;
        MainComponent ui { proc, proc.getBackend(), guest() };

        OnScreen()
        {
            ui.setSize (AppConfig::DefaultWidth, AppConfig::DefaultHeight);
            ui.addToDesktop (0);
            ui.setVisible (true);
            // le header retire le focus du prompt 150 ms après son ouverture
            // (HeaderComponent, callAfterDelay) : on attend qu'il l'ait fait
            pump (300);
        }

        ~OnScreen() { ui.removeFromDesktop(); }
    };
}

TEST_CASE ("Focus: après un clic sur un knob, le piano reprend le focus", "[ui][focus]")
{
    OnScreen f;

    auto* piano  = findFirst<juce::MidiKeyboardComponent> (f.ui);
    auto* slider = findFirst<juce::Slider> (f.ui);
    REQUIRE (piano != nullptr);
    REQUIRE (slider != nullptr);

    slider->grabKeyboardFocus();   // comme si le clic l'avait pris
    releaseOn (f.ui, *slider);
    pump();

    CHECK (piano->hasKeyboardFocus (true));
}

TEST_CASE ("Focus: un clic dans le prompt garde le focus du prompt", "[ui][focus]")
{
    OnScreen f;

    auto* prompt = findFirst<juce::TextEditor> (f.ui);
    REQUIRE (prompt != nullptr);

    prompt->grabKeyboardFocus();
    releaseOn (f.ui, *prompt);
    pump();

    CHECK (prompt->hasKeyboardFocus (true));
}

TEST_CASE ("Clavier: la disposition AZERTY / QWERTY est retenue dans l'état du plugin", "[ui][keyboard]")
{
    HarmoniaAudioProcessor proc;
    proc.getAPVTS().state.setProperty (UiState::keyLayout, "qwerty", nullptr);

    MainComponent ui { proc, proc.getBackend(), guest() };
    auto* kb = findFirst<HarmoniaKeyboard> (ui);
    REQUIRE (kb != nullptr);

    CHECK (kb->getLayout() == HarmoniaKeyboard::Layout::Qwerty);   // restaurée

    kb->toggleLayout();
    CHECK (proc.getAPVTS().state.getProperty (UiState::keyLayout).toString() == "azerty");   // retenue
}

TEST_CASE ("Formes d'onde en grille 2 x 2 : chaque case choisit la bonne forme", "[ui][selector]")
{
    HarmoniaAudioProcessor proc;
    auto& a = proc.getAPVTS();

    WaveformSelector sel (a, HarmoniaParams::IDs::osc1Waveform);
    sel.setLockStripHeight (IconChoiceSelector::defaultLockStripH);
    sel.setColumns (2);
    sel.setBounds (0, 0, 100, 2 * 26 + IconChoiceSelector::defaultLockStripH);

    auto clickAt = [&] (float x, float y)
    {
        const juce::Point<float> pos (x, y);
        const auto now = juce::Time::getCurrentTime();
        sel.mouseDown (juce::MouseEvent (juce::Desktop::getInstance().getMainMouseSource(), pos,
                                         juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                         &sel, &sel, now, pos, now, 1, false));
        pump (20);   // le sélecteur met à jour son index en asynchrone (callAsync), comme dans l'app
        return (int) a.getRawParameterValue (HarmoniaParams::IDs::osc1Waveform)->load();
    };

    // ordre de lecture : haut-gauche, haut-droite, bas-gauche, bas-droite
    CHECK (clickAt (75.0f, 13.0f) == 1);
    CHECK (clickAt (25.0f, 39.0f) == 2);
    CHECK (clickAt (75.0f, 39.0f) == 3);
    CHECK (clickAt (25.0f, 13.0f) == 0);

    // la bande du cadenas, sous la grille, ne change pas la forme
    CHECK (clickAt (25.0f, 60.0f) == 0);
}

TEST_CASE ("12 / 24 dB : le cadenas à gauche verrouille sans changer la pente", "[ui][selector]")
{
    HarmoniaAudioProcessor proc;
    auto& a = proc.getAPVTS();

    FilterSlopeSelector sel (a, "filter_slope");
    sel.setLockOnLeft (true);
    sel.setBounds (0, 0, 60 + IconChoiceSelector::lockColumnW, 15);
    sel.setLockUiVisible (true);

    auto clickAt = [&] (float x)
    {
        const juce::Point<float> pos (x, 7.0f);
        const auto now = juce::Time::getCurrentTime();
        sel.mouseDown (juce::MouseEvent (juce::Desktop::getInstance().getMainMouseSource(), pos,
                                         juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                         &sel, &sel, now, pos, now, 1, false));
        pump (20);
    };

    const auto slope = [&] { return (int) a.getRawParameterValue ("filter_slope")->load(); };
    const int before = slope();

    clickAt (5.0f);   // colonne du cadenas
    CHECK (sel.isLocked());
    CHECK (slope() == before);

    clickAt (5.0f);
    CHECK_FALSE (sel.isLocked());

    // un clic sur la case active ne verrouille plus (le cadenas n'est plus dessus)
    const float activeX = IconChoiceSelector::lockColumnW + (before == 0 ? 15.0f : 45.0f);
    clickAt (activeX);
    CHECK_FALSE (sel.isLocked());

    clickAt (IconChoiceSelector::lockColumnW + (before == 0 ? 45.0f : 15.0f));   // l'autre case
    CHECK (slope() != before);
}

// ============================================================
// Flèches : réglage fin du dernier knob / sélecteur cliqué
// ============================================================

namespace
{
    template <typename T>
    T* findByParam (juce::Component& root, const juce::String& id)
    {
        for (auto* child : root.getChildren())
        {
            if (auto* t = dynamic_cast<T*> (child))
                if (t->getParamId() == id)
                    return t;
            if (auto* t = findByParam<T> (*child, id))
                return t;
        }
        return nullptr;
    }

    bool press (MainComponent& ui, int keyCode, juce::ModifierKeys mods = {})
    {
        auto* piano = findFirst<HarmoniaKeyboard> (ui);
        return piano->keyPressed (juce::KeyPress (keyCode, mods, 0));
    }
}

TEST_CASE ("Flèches : le knob cliqué se règle finement, le piano garde le focus", "[ui][arrows]")
{
    OnScreen f;
    auto& a = f.proc.getAPVTS();
    auto* param = a.getParameter (HarmoniaParams::IDs::oscMix);
    auto* knob  = findByParam<KnobControl> (f.ui, HarmoniaParams::IDs::oscMix);
    REQUIRE (knob != nullptr);

    param->setValueNotifyingHost (0.5f);

    // sans cible : les flèches ne font rien (et partent à l'hôte)
    CHECK_FALSE (press (f.ui, juce::KeyPress::upKey));

    releaseOn (f.ui, *knob);
    pump();
    CHECK (f.ui.getArrowTarget() == knob);
    CHECK (knob->isArrowTarget());

    CHECK (press (f.ui, juce::KeyPress::upKey));
    CHECK (param->getValue() == Catch::Approx (0.51f).margin (1e-4));

    press (f.ui, juce::KeyPress::leftKey);
    press (f.ui, juce::KeyPress::leftKey);
    CHECK (param->getValue() == Catch::Approx (0.49f).margin (1e-4));

    press (f.ui, juce::KeyPress::rightKey, juce::ModifierKeys::shiftModifier);   // Maj : pas fin
    CHECK (param->getValue() == Catch::Approx (0.492f).margin (1e-4));

    // les lettres jouent toujours des notes pendant ce temps
    auto* piano = findFirst<HarmoniaKeyboard> (f.ui);
    CHECK (piano->keyPressed (juce::KeyPress ('q', {}, 'q')));

    // Échap : plus de cible
    CHECK (press (f.ui, juce::KeyPress::escapeKey));
    CHECK (f.ui.getArrowTarget() == nullptr);
    CHECK_FALSE (knob->isArrowTarget());
}

TEST_CASE ("Flèches : un sélecteur avance d'un cran, butées respectées", "[ui][arrows]")
{
    OnScreen f;
    auto& a = f.proc.getAPVTS();
    auto* sel = findByParam<IconChoiceSelector> (f.ui, HarmoniaParams::IDs::osc2Waveform);
    REQUIRE (sel != nullptr);

    a.getParameter (HarmoniaParams::IDs::osc2Waveform)->setValueNotifyingHost (0.0f);
    releaseOn (f.ui, *sel);
    pump();

    const auto wave = [&] { return (int) a.getRawParameterValue (HarmoniaParams::IDs::osc2Waveform)->load(); };

    press (f.ui, juce::KeyPress::upKey);
    CHECK (wave() == 1);

    for (int i = 0; i < 10; ++i) press (f.ui, juce::KeyPress::upKey);
    CHECK (wave() == 3);   // butée : 4 formes d'onde

    CHECK (press (f.ui, juce::KeyPress::upKey));   // en butée : la touche reste à nous
}

TEST_CASE ("Flèches : un knob grisé (Spread à une voix) ne bouge pas", "[ui][arrows]")
{
    OnScreen f;
    auto& a = f.proc.getAPVTS();

    a.getParameter (HarmoniaParams::IDs::unisonVoices)->setValueNotifyingHost (0.0f);   // 1 voix -> Spread grisé
    pump();

    auto* spread = findByParam<KnobControl> (f.ui, HarmoniaParams::IDs::unisonDetune);
    REQUIRE (spread != nullptr);
    REQUIRE_FALSE (spread->isEnabled());

    const float before = a.getParameter (HarmoniaParams::IDs::unisonDetune)->getValue();
    releaseOn (f.ui, *spread);
    pump();

    CHECK_FALSE (press (f.ui, juce::KeyPress::upKey));
    CHECK (a.getParameter (HarmoniaParams::IDs::unisonDetune)->getValue() == before);
}

TEST_CASE ("Volume : jamais verrouillable pour l'IA, jamais cible des flèches", "[ui][output]")
{
    OnScreen f;

    // aucun contrôle verrouillable (donc envoyé à l'IA en Refine) ne porte le volume
    std::function<void (juce::Component&)> check = [&] (juce::Component& c)
    {
        for (auto* child : c.getChildren())
        {
            if (auto* l = dynamic_cast<LockableControl*> (child))
                CHECK (l->getParamId() != HarmoniaParams::Output::volumeId);
            check (*child);
        }
    };
    check (f.ui);

    // un clic sur le bandeau Output ne donne pas de cible aux flèches
    auto* strip = findFirst<OutputStrip> (f.ui);
    REQUIRE (strip != nullptr);
    releaseOn (f.ui, strip->getSlider());
    pump();
    CHECK (f.ui.getArrowTarget() == nullptr);
}
