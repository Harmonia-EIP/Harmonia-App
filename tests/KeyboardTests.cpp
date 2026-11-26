#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "components/HarmoniaKeyboard.h"

#include <optional>

// Clavier d'ordinateur : AZERTY par défaut (Q..M, W / X octave), QWERTY avec B
// (A..;, Z / X octave). C / V : vélocité. Ctrl+W & co restent à l'hôte.

namespace
{
    juce::KeyPress key (char c, juce::ModifierKeys mods = {}) { return juce::KeyPress (c, mods, (juce::juce_wchar) c); }

    struct Kb
    {
        juce::MidiKeyboardState state;
        HarmoniaKeyboard kb { state };
        Kb() { kb.setSize (900, 100); }
    };
}

TEST_CASE ("Clavier: disposition AZERTY, Q joue le do central", "[keyboard]")
{
    Kb k;

    CHECK (k.kb.getOctave() == HarmoniaKeyboard::defaultOctave);
    CHECK (k.kb.getFirstMappedNote() == 60);
    CHECK (k.kb.getLastMappedNote() == 76);

    for (const char c : std::string_view (k.kb.keys().notes))
    {
        INFO (c);
        CHECK (k.kb.keyPressed (key (c)));   // toutes les lettres de la disposition sont des notes
    }

    CHECK_FALSE (k.kb.keyPressed (key ('a')));   // QWERTY : plus une note
    CHECK_FALSE (k.kb.keyPressed (key (';')));
}

TEST_CASE ("Clavier: W / X changent d'octave, avec des butées", "[keyboard]")
{
    Kb k;

    CHECK (k.kb.keyPressed (key ('x')));
    CHECK (k.kb.getOctave() == HarmoniaKeyboard::defaultOctave + 1);
    CHECK (k.kb.getFirstMappedNote() == 72);

    CHECK (k.kb.keyPressed (key ('w')));
    CHECK (k.kb.keyPressed (key ('W')));   // majuscule (verr. maj) : pareil
    CHECK (k.kb.getOctave() == HarmoniaKeyboard::defaultOctave - 1);

    for (int i = 0; i < 20; ++i) k.kb.keyPressed (key ('w'));
    CHECK (k.kb.getOctave() == HarmoniaKeyboard::minOctave);

    for (int i = 0; i < 20; ++i) k.kb.keyPressed (key ('x'));
    CHECK (k.kb.getOctave() == HarmoniaKeyboard::maxOctave);
    CHECK (k.kb.getLastMappedNote() <= 127);   // la dernière touche reste une note MIDI valide
}

TEST_CASE ("Clavier: C / V changent la vélocité par pas de 10 %", "[keyboard]")
{
    Kb k;

    CHECK (k.kb.getKeyVelocity() == Catch::Approx (0.8f));

    k.kb.keyPressed (key ('v'));
    CHECK (k.kb.getKeyVelocity() == Catch::Approx (0.9f));

    for (int i = 0; i < 5; ++i) k.kb.keyPressed (key ('v'));
    CHECK (k.kb.getKeyVelocity() == Catch::Approx (1.0f));

    for (int i = 0; i < 20; ++i) k.kb.keyPressed (key ('c'));
    CHECK (k.kb.getKeyVelocity() == Catch::Approx (HarmoniaKeyboard::minVelocity));   // jamais 0 : une note muette
}

TEST_CASE ("Clavier: Ctrl+W, Alt+X... restent des raccourcis de l'hôte", "[keyboard]")
{
    Kb k;

    CHECK_FALSE (k.kb.keyPressed (key ('w', juce::ModifierKeys::ctrlModifier)));
    CHECK_FALSE (k.kb.keyPressed (key ('x', juce::ModifierKeys::altModifier)));
    CHECK (k.kb.getOctave() == HarmoniaKeyboard::defaultOctave);
}

TEST_CASE ("Clavier: B bascule entre AZERTY et QWERTY", "[keyboard]")
{
    Kb k;

    std::optional<HarmoniaKeyboard::Layout> notified;
    k.kb.onLayoutChanged = [&] (HarmoniaKeyboard::Layout l) { notified = l; };

    REQUIRE (k.kb.getLayout() == HarmoniaKeyboard::Layout::Azerty);

    CHECK (k.kb.keyPressed (key ('b')));
    CHECK (k.kb.getLayout() == HarmoniaKeyboard::Layout::Qwerty);
    REQUIRE (notified.has_value());
    CHECK (*notified == HarmoniaKeyboard::Layout::Qwerty);

    CHECK (k.kb.keyPressed (key ('B')));
    CHECK (k.kb.getLayout() == HarmoniaKeyboard::Layout::Azerty);
}

TEST_CASE ("Clavier: en QWERTY, A..; jouent et Z / X changent d'octave", "[keyboard]")
{
    Kb k;
    k.kb.setLayout (HarmoniaKeyboard::Layout::Qwerty);

    for (const char c : std::string_view ("awsedftgyhujkolp;"))
    {
        INFO (c);
        CHECK (k.kb.keyPressed (key (c)));
    }

    // 'q' et 'm' ne sont plus des notes ; 'w' est une note (do#), plus l'octave
    CHECK_FALSE (k.kb.keyPressed (key ('q')));
    CHECK_FALSE (k.kb.keyPressed (key ('m')));
    CHECK (k.kb.getOctave() == HarmoniaKeyboard::defaultOctave);

    k.kb.keyPressed (key ('z'));
    CHECK (k.kb.getOctave() == HarmoniaKeyboard::defaultOctave - 1);
    k.kb.keyPressed (key ('x'));
    CHECK (k.kb.getOctave() == HarmoniaKeyboard::defaultOctave);

    k.kb.keyPressed (key ('v'));
    CHECK (k.kb.getKeyVelocity() == Catch::Approx (0.9f));
}

TEST_CASE ("Clavier: les deux dispositions utilisent les mêmes touches physiques", "[keyboard]")
{
    // même nombre de notes, et la seule touche de contrôle qui change est l'octave -
    // (W en AZERTY = Z en QWERTY : la même touche, en bas à gauche)
    const auto az = HarmoniaKeyboard::keysFor (HarmoniaKeyboard::Layout::Azerty);
    const auto qw = HarmoniaKeyboard::keysFor (HarmoniaKeyboard::Layout::Qwerty);

    CHECK (std::string_view (az.notes).size() == (size_t) HarmoniaKeyboard::numMappedNotes);
    CHECK (std::string_view (qw.notes).size() == (size_t) HarmoniaKeyboard::numMappedNotes);
    CHECK (az.octaveDown == 'w');
    CHECK (qw.octaveDown == 'z');
    CHECK (az.octaveUp == qw.octaveUp);
    CHECK (az.velocityDown == qw.velocityDown);
    CHECK (az.velocityUp == qw.velocityUp);

    // aucune touche de contrôle n'est aussi une note
    for (const auto& k : { az, qw })
        for (const char ctl : { k.octaveDown, k.octaveUp, k.velocityDown, k.velocityUp, HarmoniaKeyboard::switchLayoutKey })
        {
            INFO (k.name << " " << ctl);
            CHECK (std::string_view (k.notes).find (ctl) == std::string_view::npos);
        }
}

// ============================================================
// Panneau Keyboard : plage affichée et chips
// ============================================================

#include "components/SynthComponent.h"

namespace
{
    void clickChip (KeyboardChip& chip, bool rightHalf)
    {
        const juce::Point<float> pos (rightHalf ? (float) chip.getWidth() - 3.0f : 3.0f, (float) chip.getHeight() * 0.5f);
        const auto now = juce::Time::getCurrentTime();
        chip.mouseUp (juce::MouseEvent (juce::Desktop::getInstance().getMainMouseSource(), pos,
                                        juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                        &chip, &chip, now, pos, now, 1, false));
    }
}

TEST_CASE ("Clavier: la plage affichée suit l'octave, sans défilement", "[keyboard]")
{
    Kb k;

    // une octave sous la zone jouable, 4 octaves en tout
    CHECK (k.kb.getVisibleRange() == juce::Range<int> (48, 96));

    k.kb.keyPressed (key ('x'));
    CHECK (k.kb.getVisibleRange() == juce::Range<int> (60, 108));

    for (int i = 0; i < 20; ++i) k.kb.keyPressed (key ('x'));
    CHECK (k.kb.getVisibleRange().getEnd() == 127);                    // butée haute : la plage reste dans le MIDI
    CHECK (k.kb.getVisibleRange().contains (k.kb.getFirstMappedNote()));

    for (int i = 0; i < 20; ++i) k.kb.keyPressed (key ('w'));
    CHECK (k.kb.getVisibleRange().getStart() == 0);
}

TEST_CASE ("Panneau clavier: les chips pilotent octave, vélocité et disposition", "[keyboard][ui]")
{
    juce::MidiKeyboardState state;
    SynthComponent panel (state);
    panel.setSize (868, 120);
    auto& kb = panel.getKeyboard();

    CHECK (panel.getOctaveChip().getText() == "C3");
    CHECK (panel.getVelocityChip().getText() == "VEL 80%");
    CHECK (panel.getLayoutChip().getText() == "AZERTY");

    clickChip (panel.getOctaveChip(), true);
    CHECK (kb.getOctave() == HarmoniaKeyboard::defaultOctave + 1);
    CHECK (panel.getOctaveChip().getText() == "C4");

    clickChip (panel.getOctaveChip(), false);
    CHECK (kb.getOctave() == HarmoniaKeyboard::defaultOctave);

    clickChip (panel.getVelocityChip(), true);
    CHECK (panel.getVelocityChip().getText() == "VEL 90%");

    clickChip (panel.getLayoutChip(), false);
    CHECK (kb.getLayout() == HarmoniaKeyboard::Layout::Qwerty);
    CHECK (panel.getLayoutChip().getText() == "QWERTY");

    // raccourci clavier -> les chips suivent aussi
    kb.keyPressed (juce::KeyPress ('z', {}, 'z'));   // QWERTY : Z = octave -
    CHECK (panel.getOctaveChip().getText() == "C2");
}

TEST_CASE ("Clavier: la molette / le pavé tactile font glisser l'affichage", "[keyboard]")
{
    Kb k;
    const auto start = k.kb.getVisibleRange();
    const int octave = k.kb.getOctave();

    auto wheel = [&] (float dx, float dy)
    {
        juce::MouseWheelDetails w {};
        w.deltaX = dx;
        w.deltaY = dy;
        const auto now = juce::Time::getCurrentTime();
        const juce::Point<float> pos (100.0f, 50.0f);
        k.kb.mouseWheelMove (juce::MouseEvent (juce::Desktop::getInstance().getMainMouseSource(), pos,
                                               juce::ModifierKeys(), 0.0f, 0.0f, 0.0f, 0.0f, 0.0f,
                                               &k.kb, &k.kb, now, pos, now, 1, false), w);
    };

    // un cran de molette vers le bas : vers les aigus, l'octave jouée ne bouge pas
    wheel (0.0f, -0.11f);
    CHECK (k.kb.getVisibleRange().getStart() > start.getStart());
    CHECK (k.kb.getOctave() == octave);
    CHECK_FALSE (juce::MidiMessage::isMidiNoteBlack (k.kb.getVisibleRange().getStart()));   // commence sur une blanche

    // pavé tactile : petits glissements horizontaux qui s'accumulent
    const auto afterWheel = k.kb.getVisibleRange();
    for (int i = 0; i < 10; ++i)
        wheel (-0.02f, 0.0f);
    CHECK (k.kb.getVisibleRange().getStart() < afterWheel.getStart());

    // toujours 29 touches blanches : la largeur des touches ne change pas en glissant
    int whites = 0;
    for (int n = k.kb.getVisibleRange().getStart(); n <= k.kb.getVisibleRange().getEnd(); ++n)
        whites += juce::MidiMessage::isMidiNoteBlack (n) ? 0 : 1;
    CHECK (whites == HarmoniaKeyboard::visibleWhiteKeys);

    // butées
    for (int i = 0; i < 100; ++i) wheel (0.0f, -0.5f);
    CHECK (k.kb.getVisibleRange().getEnd() <= 127);
    for (int i = 0; i < 100; ++i) wheel (0.0f, 0.5f);
    CHECK (k.kb.getVisibleRange().getStart() == 0);

    // changer d'octave (W / X) recadre l'affichage sur la zone jouable
    k.kb.keyPressed (key ('x'));
    CHECK (k.kb.getVisibleRange().contains (k.kb.getFirstMappedNote()));
}
