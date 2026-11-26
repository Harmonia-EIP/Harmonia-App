#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include "PluginProcessor.h"
#include "MainComponent.h"   // pour UiState
#include <cmath>

namespace
{
    struct NonFinite { int ch = -1, idx = -1; float v = 0.0f; };

    // Position du premier échantillon NaN/Inf (ch == -1 si tout est fini)
    NonFinite firstNonFinite (const juce::AudioBuffer<float>& b)
    {
        for (int ch = 0; ch < b.getNumChannels(); ++ch)
            for (int i = 0; i < b.getNumSamples(); ++i)
                if (! std::isfinite (b.getSample (ch, i)))
                    return { ch, i, b.getSample (ch, i) };
        return {};
    }

    struct Rig
    {
        static constexpr double sr = 48000.0;
        static constexpr int    bs = 512;

        HarmoniaAudioProcessor   proc;
        juce::AudioBuffer<float> buf { 2, bs };
        juce::MidiBuffer         midi;

        Rig()
        {
            buf.clear();   // contenu indéfini à la création
            proc.setRateAndBufferSizeDetails (sr, bs);
            proc.prepareToPlay (sr, bs);
        }

        void set (const juce::String& id, float normalised)
        {
            auto* p = proc.getAPVTS().getParameter (id);
            REQUIRE (p != nullptr);
            p->setValueNotifyingHost (normalised);
        }

        // Traite n blocs (le MIDI en attente part dans le 1er), renvoie l'énergie
        double run (int blocks)
        {
            double energy = 0.0;
            for (int i = 0; i < blocks; ++i)
            {
                proc.processBlock (buf, midi);
                midi.clear();

                const auto bad = firstNonFinite (buf);
                INFO ("non fini: canal " << bad.ch << ", echantillon " << bad.idx << ", valeur " << bad.v);
                REQUIRE (bad.ch < 0);

                const double r = buf.getRMSLevel (0, 0, bs);
                energy += r * r * bs;
            }
            return energy;
        }

        void noteOn  (int note, juce::uint8 vel = 100) { midi.addEvent (juce::MidiMessage::noteOn  (1, note, vel), 0); }
        void noteOff (int note)                        { midi.addEvent (juce::MidiMessage::noteOff (1, note), 0); }
    };
}

TEST_CASE ("Paramètres: défauts et texte valides")
{
    HarmoniaAudioProcessor p;
    REQUIRE (p.getParameters().size() > 0);

    for (auto* prm : p.getParameters())
    {
        INFO (prm->getName (64).toStdString());
        REQUIRE (prm->getValue() >= 0.0f);
        REQUIRE (prm->getValue() <= 1.0f);
        REQUIRE (prm->getDefaultValue() >= 0.0f);
        REQUIRE (prm->getDefaultValue() <= 1.0f);
        REQUIRE (prm->getText (prm->getValue(), 64).isNotEmpty());
    }
}

TEST_CASE ("Layouts de bus supportés")
{
    HarmoniaAudioProcessor p;

    auto layout = [] (juce::AudioChannelSet s)
    {
        juce::AudioProcessor::BusesLayout l;
        l.outputBuses.add (s);
        return l;
    };

    REQUIRE (p.isBusesLayoutSupported (layout (juce::AudioChannelSet::stereo())));
    REQUIRE (p.isBusesLayoutSupported (layout (juce::AudioChannelSet::mono())));
    REQUIRE_FALSE (p.isBusesLayoutSupported (layout (juce::AudioChannelSet::create5point1())));
}

TEST_CASE ("Fuzz: paramètres et MIDI aléatoires restent finis et bornés")
{
    Rig r;
    juce::Random rng (42);

    for (int iter = 0; iter < 40; ++iter)
    {
        juce::String dump;

        for (auto* prm : r.proc.getParameters())
        {
            const int mode = rng.nextInt (3);
            prm->setValueNotifyingHost (mode == 0 ? 0.0f : mode == 1 ? 1.0f : rng.nextFloat());
            dump << prm->getName (32) << "=" << prm->getText (prm->getValue(), 32) << "  ";
        }

        INFO ("iter " << iter << " | " << dump.toStdString());

        for (int n = 0; n < 4; ++n)
            r.noteOn (24 + rng.nextInt (80), (juce::uint8) (1 + rng.nextInt (127)));

        for (int b = 0; b < 8; ++b)
        {
            INFO ("bloc " << b);
            r.run (1);
            REQUIRE (r.buf.getMagnitude (0, Rig::bs) < 20.0f);
        }

        r.midi.addEvent (juce::MidiMessage::allNotesOff (1), 0);
        r.run (1);
    }
}

TEST_CASE ("Polyphonie: plus de notes que de voix")
{
    Rig r;
    for (int n = 0; n < 9; ++n)
        r.noteOn (48 + n * 3);

    const double e = r.run (50);
    REQUIRE (e > 0.0);
    REQUIRE (r.buf.getMagnitude (0, Rig::bs) < 20.0f);
}

TEST_CASE ("Release: retour au silence après noteOff")
{
    Rig r;
    r.set (HarmoniaParams::IDs::reverbMix, 0.0f);
    r.set (HarmoniaParams::IDs::distortionMix, 0.0f);
    r.set (HarmoniaParams::IDs::ampRelease, 0.0f);

    r.noteOn (60);
    REQUIRE (r.run (20) > 0.0);

    r.noteOff (60);
    r.run (100);                       // ~1 s

    REQUIRE (r.run (10) < 1e-6);
}

TEST_CASE ("Reverb: queue audible seulement si reverbMix > 0")
{
    auto tailEnergy = [] (float reverbMix)
    {
        Rig r;
        r.set (HarmoniaParams::IDs::reverbMix, reverbMix);
        r.set (HarmoniaParams::IDs::ampRelease, 0.0f);
        r.noteOn (60);
        r.run (10);
        r.noteOff (60);
        r.run (20);                    // laisse la voix s'éteindre
        return r.run (20);
    };

    REQUIRE (tailEnergy (1.0f) > tailEnergy (0.0f) * 10.0 + 1e-9);
}

TEST_CASE ("Filtre: cutoff bas = moins d'énergie que cutoff haut")
{
    auto energy = [] (float cutoff)
    {
        Rig r;
        r.set (HarmoniaParams::IDs::filterCutoff, cutoff);
        r.set (HarmoniaParams::IDs::reverbMix, 0.0f);
        r.noteOn (72);
        return r.run (40);
    };

    REQUIRE (energy (0.05f) <= energy (0.95f));
}

TEST_CASE ("Vélocité: forte > faible")
{
    auto energy = [] (juce::uint8 vel)
    {
        Rig r;
        r.set (HarmoniaParams::IDs::reverbMix, 0.0f);
        r.noteOn (60, vel);
        return r.run (20);
    };

    REQUIRE (energy (127) > energy (10));
}

TEST_CASE ("prepareToPlay rappelé avec un autre sample rate")
{
    Rig r;
    r.noteOn (60);
    r.run (5);

    for (double sr : { 44100.0, 96000.0, 48000.0 })
    {
        r.proc.releaseResources();
        r.proc.setRateAndBufferSizeDetails (sr, Rig::bs);
        r.proc.prepareToPlay (sr, Rig::bs);

        r.noteOn (64);
        REQUIRE (r.run (10) > 0.0);
    }
}

TEST_CASE ("State: propriétés UI conservées, aucun secret dans le XML")
{
    HarmoniaAudioProcessor a, b;
    a.getAPVTS().state.setProperty (UiState::presetName,  "MON PRESET", nullptr);
    a.getAPVTS().state.setProperty (UiState::paletteSlot, 2, nullptr);

    juce::MemoryBlock mb;
    a.getStateInformation (mb);

    auto xml = juce::AudioProcessor::getXmlFromBinary (mb.getData(), (int) mb.getSize());
    REQUIRE (xml != nullptr);
    const auto text = xml->toString().toLowerCase();
    REQUIRE_FALSE (text.contains ("token"));
    REQUIRE_FALSE (text.contains ("password"));

    b.setStateInformation (mb.getData(), (int) mb.getSize());
    REQUIRE (b.getAPVTS().state[UiState::presetName].toString() == "MON PRESET");
    REQUIRE ((int) b.getAPVTS().state[UiState::paletteSlot] == 2);
}

TEST_CASE ("Régression NaN: résonance 0 / cutoff modulé vers le bas")
{
    SECTION ("résonance = 0")
    {
        Rig r;
        r.set (HarmoniaParams::IDs::filterResonance, 0.0f);
        r.noteOn (60);
        r.run (4);
    }

    SECTION ("cutoff bas + env -1 + LFO au max")
    {
        Rig r;
        r.set (HarmoniaParams::IDs::filterCutoff,     0.0f);   // 20 Hz
        r.set (HarmoniaParams::IDs::filterEnvAmount,  0.0f);   // -1
        r.set (HarmoniaParams::IDs::lfoToCutoff,      1.0f);
        r.set (HarmoniaParams::IDs::lfoRate,          1.0f);
        r.noteOn (60);
        r.run (20);
    }
}
TEST_CASE ("Mode DX7 : un preset DX7 joue sur le cœur FM, sans effets")
{
    Rig r;
    auto& a = r.proc.getAPVTS();
    a.getParameter ("reverb_mix")->setValueNotifyingHost (1.0f);   // ne doit pas toucher une voix DX7
    a.getParameter ("engine")->setValueNotifyingHost (1.0f);       // DX7 (INIT VOICE)
    r.noteOn (69);
    const double held = r.run (20);
    r.noteOff (69);
    r.run (40);
    const double tail = r.run (20);

    REQUIRE (held > 0.0);
    REQUIRE (tail < held * 1e-3);   // INIT VOICE se tait vite après le relâchement, pas de queue de reverb
}
