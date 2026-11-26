#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>

#include "PluginProcessor.h"
#include "engine/OutputStage.h"
#include "tools/ParamRandomizer.h"
#include "tools/PresetLoader.h"

// Volume de sortie (hors charte) + limiteur doux : réglage d'écoute, jamais touché
// par l'IA, les presets ou le bouton aléatoire, et la sortie ne dépasse jamais 0 dBFS.

namespace
{
    using harmonia::output::softLimit;
    using harmonia::output::dbToGain;
    namespace Out = HarmoniaParams::Output;

    void setDb (HarmoniaAudioProcessor& p, float db)
    {
        auto* prm = p.getAPVTS().getParameter (Out::volumeId);
        prm->setValueNotifyingHost (prm->convertTo0to1 (db));
    }

    float getDb (HarmoniaAudioProcessor& p)
    {
        auto* prm = p.getAPVTS().getParameter (Out::volumeId);
        return prm->convertFrom0to1 (prm->getValue());
    }

    /** Joue un accord pendant `blocks` blocs et renvoie la crête des derniers blocs. */
    float playPeak (HarmoniaAudioProcessor& p, int blocks = 60,
                    std::initializer_list<int> notes = { 48, 55, 60, 64, 67, 72 }, float velocity = 1.0f)
    {
        constexpr int blockSize = 512;
        p.prepareToPlay (48000.0, blockSize);

        juce::AudioBuffer<float> buffer (2, blockSize);
        float peak = 0.0f;

        for (int b = 0; b < blocks; ++b)
        {
            juce::MidiBuffer midi;
            if (b == 0)
                for (const int note : notes)
                    midi.addEvent (juce::MidiMessage::noteOn (1, note, velocity), 0);

            p.processBlock (buffer, midi);

            if (b >= blocks / 2)   // après le lissage du volume et l'attaque
                peak = juce::jmax (peak, buffer.getMagnitude (0, blockSize));
        }

        return peak;
    }
}

TEST_CASE ("Sortie: le limiteur est transparent sous -1 dBFS et ne dépasse jamais 0 dBFS", "[output]")
{
    for (const float x : { 0.0f, 0.1f, -0.5f, 0.89f })
        CHECK (softLimit (x) == x);

    for (const float x : { 0.95f, 1.0f, 2.0f, 10.0f, 1000.0f, -50.0f })
    {
        INFO (x);
        CHECK (std::abs (softLimit (x)) <= 1.0f);
        CHECK (std::abs (softLimit (x)) >= harmonia::output::limiterThreshold);   // pas d'écrasement
        CHECK ((softLimit (x) > 0.0f) == (x > 0.0f));                             // signe conservé
    }

    // monotone : plus fort en entrée = plus fort (ou égal) en sortie
    float previous = 0.0f;
    for (float x = 0.0f; x < 5.0f; x += 0.01f)
    {
        CHECK (softLimit (x) >= previous);
        previous = softLimit (x);
    }
}

TEST_CASE ("Sortie: conversion dB -> gain", "[output]")
{
    CHECK (dbToGain (0.0f) == Catch::Approx (1.0f));
    CHECK (dbToGain (-6.0f) == Catch::Approx (0.501f).margin (1e-3));
    CHECK (dbToGain (6.0f) == Catch::Approx (1.995f).margin (1e-3));
    CHECK (dbToGain (-100.0f) == 0.0f);
}

TEST_CASE ("Sortie: paramètre de volume, défaut 0 dB, texte lisible", "[output]")
{
    HarmoniaAudioProcessor p;
    auto* prm = p.getAPVTS().getParameter (Out::volumeId);

    REQUIRE (prm != nullptr);
    CHECK (getDb (p) == Catch::Approx (Out::defaultDb));
    CHECK (prm->getText (prm->convertTo0to1 (-6.0f), 0) == "-6.0 dB");
    CHECK (prm->getText (prm->convertTo0to1 (3.0f), 0) == "+3.0 dB");
}

TEST_CASE ("Sortie: baisser le volume baisse le niveau", "[output]")
{
    // -12 et -24 dB : assez bas pour que le limiteur ne travaille pas (le moteur sort
    // déjà vers +2 dBFS sur une note), le rapport doit donc être exactement de 12 dB
    HarmoniaAudioProcessor mid, low;
    setDb (mid, -12.0f);
    setDb (low, -24.0f);

    const float a = playPeak (mid, 60, { 60 }, 0.4f);
    const float b = playPeak (low, 60, { 60 }, 0.4f);

    INFO ("-12 dB : " << a << "   -24 dB : " << b);
    REQUIRE (a > 0.0f);
    REQUIRE (a < harmonia::output::limiterThreshold);
    CHECK (b / a == Catch::Approx (dbToGain (-12.0f)).epsilon (0.05));
}

TEST_CASE ("Sortie: patch trop fort + volume max = jamais au-dessus de 0 dBFS", "[output]")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();

    // le pire cas réaliste : unisson max, résonance et distorsion à fond, volume +6 dB
    for (const auto& [id, norm] : { std::pair<const char*, float> { "unison_voices", 1.0f },
                                    { "filter_resonance", 1.0f }, { "distortion_mix", 1.0f },
                                    { "amp_sustain", 1.0f } })
        a.getParameter (id)->setValueNotifyingHost (norm);
    setDb (p, Out::maxDb);

    const float peak = playPeak (p);

    CHECK (peak > 0.5f);
    CHECK (peak <= 1.0f);
}

TEST_CASE ("Sortie: le vumètre reçoit la crête puis repart de zéro", "[output]")
{
    HarmoniaAudioProcessor p;
    playPeak (p, 20);

    CHECK (p.takeOutputPeak() > 0.0f);
    CHECK (p.takeOutputPeak() == 0.0f);   // lu une fois : remis à zéro
}

TEST_CASE ("Sortie: ni les presets, ni l'IA, ni l'aléatoire ne touchent au volume", "[output]")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();
    setDb (p, -9.0f);

    // pas dans un preset sauvegardé (donc pas envoyé à l'IA en Refine)
    const auto json = PresetLoader::saveToJsonString (a, "x", "test");
    CHECK_FALSE (json.contains (Out::volumeId));

    // chargement complet (resetUnspecified) : le volume reste
    REQUIRE (PresetLoader::loadFromJsonString (json, a, {}, true).success);
    CHECK (getDb (p) == Catch::Approx (-9.0f).margin (0.05f));

    // entrée d'historique : pareil
    REQUIRE (PresetLoader::loadFromParameters ({ { "osc_mix", 0.3f } }, "h", a).success);
    CHECK (getDb (p) == Catch::Approx (-9.0f).margin (0.05f));

    // bouton aléatoire : pareil
    ParamRandomizer::randomize (a, {});
    CHECK (getDb (p) == Catch::Approx (-9.0f).margin (0.05f));
}

TEST_CASE ("Sortie: le volume est retenu avec le projet", "[output]")
{
    juce::MemoryBlock state;
    {
        HarmoniaAudioProcessor p;
        setDb (p, -4.5f);
        p.getStateInformation (state);
    }

    HarmoniaAudioProcessor restored;
    restored.setStateInformation (state.getData(), (int) state.getSize());
    CHECK (getDb (restored) == Catch::Approx (-4.5f).margin (0.05f));
}

// ============================================================
// Isolation du volume : même s'il arrive dans un preset, l'IA ou l'historique, il est ignoré
// ============================================================

TEST_CASE ("Sortie: un preset (fichier ou réponse IA) qui contient le volume ne le touche pas", "[output]")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();
    setDb (p, -9.0f);

    nlohmann::json doc;
    doc["metadata"]   = { { "name", "piege" }, { "charter_version", std::string (HarmoniaCharter::version) } };
    doc["parameters"] = { { "osc_mix", 0.25 }, { Out::volumeId.toStdString(), 1.0 } };   // +6 dB demandé

    REQUIRE (PresetLoader::loadFromJsonString (juce::String (doc.dump()), a, {}, true).success);

    CHECK (getDb (p) == Catch::Approx (-9.0f).margin (0.05f));                       // volume intact
    CHECK (a.getParameter ("osc_mix")->getValue() == Catch::Approx (0.25f).margin (1e-3)); // le reste est appliqué
}

TEST_CASE ("Sortie: un preset qui ne contient QUE le volume ne change rien", "[output]")
{
    HarmoniaAudioProcessor p;
    setDb (p, -9.0f);

    nlohmann::json doc;
    doc["metadata"]   = { { "charter_version", std::string (HarmoniaCharter::version) } };
    doc["parameters"] = { { Out::volumeId.toStdString(), 0.0 } };

    PresetLoader::loadFromJsonString (juce::String (doc.dump()), p.getAPVTS(), {}, true);
    CHECK (getDb (p) == Catch::Approx (-9.0f).margin (0.05f));

    // entrée d'historique avec seulement le volume : refusée (aucun paramètre de preset reconnu)
    CHECK_FALSE (PresetLoader::loadFromParameters ({ { Out::volumeId, 0.0f } }, "h", p.getAPVTS()).success);
    CHECK (getDb (p) == Catch::Approx (-9.0f).margin (0.05f));
}

TEST_CASE ("Sortie: une entrée d'historique qui contient le volume ne le touche pas", "[output]")
{
    HarmoniaAudioProcessor p;
    setDb (p, -9.0f);

    // entrée v3 (engine présent) + volume
    REQUIRE (PresetLoader::loadFromParameters ({ { "engine", 0.0f }, { "osc_mix", 0.6f }, { Out::volumeId, 1.0f } },
                                               "h", p.getAPVTS()).success);
    CHECK (getDb (p) == Catch::Approx (-9.0f).margin (0.05f));
}

TEST_CASE ("Sortie: le volume n'est pas dans la charte (donc jamais généré ni envoyé à l'IA)", "[output]")
{
    CHECK (HarmoniaCharter::indexOf (Out::volumeId.toStdString()) < 0);
    CHECK_FALSE (PresetLoader::isPresetParameter (Out::volumeId));
    CHECK (PresetLoader::isPresetParameter ("osc_mix"));
}

TEST_CASE ("Sortie: un ancien projet (avant v3) garde le volume par défaut", "[output]")
{
    // état v2 : pas de charter_version, quelques paramètres historiques
    juce::ValueTree state ("HARMONIA");
    juce::ValueTree child ("PARAM");
    child.setProperty ("id", "osc_mix", nullptr);
    child.setProperty ("value", 0.3f, nullptr);
    state.appendChild (child, nullptr);

    juce::MemoryBlock block;
    juce::AudioProcessor::copyXmlToBinary (*state.createXml(), block);

    HarmoniaAudioProcessor p;   // instance neuve, comme quand le DAW ouvre le projet
    p.setStateInformation (block.getData(), (int) block.getSize());

    CHECK (getDb (p) == Catch::Approx (Out::defaultDb).margin (0.05f));
}
