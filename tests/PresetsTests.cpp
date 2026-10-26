#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "PluginProcessor.h"
#include "tools/PresetLoader.h"
#include <cmath>
#include <map>
#include <set>
#include <string>

namespace
{
    using Snapshot = std::map<std::string, float>;

    // Valeurs normalisées actuelles des 20 paramètres
    Snapshot snapshot (juce::AudioProcessorValueTreeState& a)
    {
        Snapshot s;
        for (const auto& id : HarmoniaParams::orderedIds())
            s[id.toStdString()] = a.getParameter (id)->getValue();
        return s;
    }

    void setAll (juce::AudioProcessorValueTreeState& a, float normalised)
    {
        for (const auto& id : HarmoniaParams::orderedIds())
            a.getParameter (id)->setValueNotifyingHost (normalised);
    }

    void randomise (juce::AudioProcessorValueTreeState& a, juce::Random& rng)
    {
        for (const auto& id : HarmoniaParams::orderedIds())
            a.getParameter (id)->setValueNotifyingHost (rng.nextFloat());
    }

    // Valeur réellement obtenue après un set (les paramètres discrets sont arrondis)
    float snapped (juce::RangedAudioParameter* p, float norm)
    {
        return p->convertTo0to1 (p->convertFrom0to1 (norm));
    }

    void requireSameExcept (const Snapshot& expected, const Snapshot& actual,
                            const std::set<std::string>& except = {})
    {
        for (const auto& [id, v] : expected)
        {
            if (except.count (id) > 0)
                continue;
            INFO (id);
            REQUIRE (actual.at (id) == Catch::Approx (v).margin (1e-5));
        }
    }

    // Joue une note après un chargement : la sortie doit rester finie
    bool rendersFinite (HarmoniaAudioProcessor& p)
    {
        constexpr double sr = 48000.0;
        constexpr int    bs = 512;

        p.setRateAndBufferSizeDetails (sr, bs);
        p.prepareToPlay (sr, bs);

        juce::AudioBuffer<float> buf (2, bs);
        juce::MidiBuffer midi;
        midi.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 100), 0);

        for (int b = 0; b < 20; ++b)
        {
            buf.clear();
            p.processBlock (buf, midi);
            midi.clear();

            for (int ch = 0; ch < buf.getNumChannels(); ++ch)
                for (int i = 0; i < bs; ++i)
                    if (! std::isfinite (buf.getSample (ch, i)))
                        return false;
        }
        return true;
    }
}

//==============================================================================
// Export / import
//==============================================================================

TEST_CASE ("PresetLoader: export puis import restitue tous les paramètres")
{
    HarmoniaAudioProcessor src, dst;
    juce::Random rng (7);

    randomise (src.getAPVTS(), rng);
    const auto expected = snapshot (src.getAPVTS());

    const auto json = PresetLoader::saveToJsonString (src.getAPVTS(), "Mon Preset", "tests");

    setAll (dst.getAPVTS(), 0.0f);   // état de départ différent
    const auto r = PresetLoader::loadFromJsonString (json, dst.getAPVTS());

    REQUIRE (r.success);
    REQUIRE (r.presetName == "Mon Preset");
    requireSameExcept (expected, snapshot (dst.getAPVTS()));
}

TEST_CASE ("PresetLoader: nom de preset avec accents et unicode conservé")
{
    HarmoniaAudioProcessor src, dst;
    const juce::String name (juce::CharPointer_UTF8 ("\xC3\x89t\xC3\xA9 \xE2\x98\x80 \xE9\x9F\xB3"));

    const auto json = PresetLoader::saveToJsonString (src.getAPVTS(), name, "tests");
    const auto r    = PresetLoader::loadFromJsonString (json, dst.getAPVTS());

    REQUIRE (r.success);
    REQUIRE (r.presetName == name);
}

TEST_CASE ("PresetLoader: aller-retour via un fichier")
{
    HarmoniaAudioProcessor src, dst;
    juce::Random rng (11);
    randomise (src.getAPVTS(), rng);
    const auto expected = snapshot (src.getAPVTS());

    juce::TemporaryFile tmp (".hel.json");
    REQUIRE (tmp.getFile().replaceWithText (PresetLoader::saveToJsonString (src.getAPVTS(), "Fichier", "tests")));

    setAll (dst.getAPVTS(), 0.0f);
    const auto r = PresetLoader::loadFromFile (tmp.getFile(), dst.getAPVTS());

    REQUIRE (r.success);
    REQUIRE (r.presetName == "Fichier");
    requireSameExcept (expected, snapshot (dst.getAPVTS()));
}

TEST_CASE ("PresetLoader: fichier introuvable")
{
    HarmoniaAudioProcessor p;
    juce::Random rng (3);
    randomise (p.getAPVTS(), rng);
    const auto before = snapshot (p.getAPVTS());

    const auto missing = juce::File::getSpecialLocation (juce::File::tempDirectory)
                             .getChildFile ("harmonia_missing_preset_xyz.hel.json");

    const auto r = PresetLoader::loadFromFile (missing, p.getAPVTS());

    REQUIRE_FALSE (r.success);
    REQUIRE (r.errorMessage == "File not found");
    requireSameExcept (before, snapshot (p.getAPVTS()));
}

TEST_CASE ("PresetLoader: structure du JSON exporté")
{
    HarmoniaAudioProcessor p;
    juce::Random rng (5);
    randomise (p.getAPVTS(), rng);

    const auto json = PresetLoader::saveToJsonString (p.getAPVTS(), "Mon Preset", "tests");
    const auto doc  = nlohmann::json::parse (json.toStdString());
    const auto& ids = HarmoniaParams::orderedIds();

    REQUIRE (doc["metadata"]["name"] == "Mon Preset");
    REQUIRE (doc["metadata"]["generated_by"] == "tests");
    REQUIRE (doc["metadata"]["charter_version"] == "1.0");

    REQUIRE (doc["parameters"].is_object());
    REQUIRE (doc["values"].is_array());
    REQUIRE (doc["parameters"].size() == ids.size());
    REQUIRE (doc["values"].size()     == ids.size());

    for (size_t i = 0; i < ids.size(); ++i)
    {
        INFO (ids[i].toStdString());
        const auto key = ids[i].toStdString();
        REQUIRE (doc["parameters"].contains (key));
        REQUIRE (doc["parameters"][key].is_number());
        REQUIRE (doc["parameters"][key].get<double>() >= 0.0);
        REQUIRE (doc["parameters"][key].get<double>() <= 1.0);
        REQUIRE (doc["values"][i] == doc["parameters"][key]);   // même ordre que orderedIds()
    }

    const auto text = json.toLowerCase();
    REQUIRE_FALSE (text.contains ("token"));
    REQUIRE_FALSE (text.contains ("password"));
}

//==============================================================================
// Entrées invalides
//==============================================================================

TEST_CASE ("PresetLoader: JSON invalide ou vide est refusé sans toucher aux paramètres")
{
    const auto text = GENERATE (as<std::string>{},
                                "",
                                "   ",
                                "not json",
                                "{",
                                R"({"parameters": {)",
                                "[]",
                                "42",
                                "null",
                                "{}",
                                R"({"parameters": {}})",
                                R"({"parameters": 5})",
                                R"({"parameters": {"osc_1_waveform": "high"}})",
                                R"({"values": []})",
                                R"({"values": "abc"})",
                                R"({"values": [null, "x"]})");

    HarmoniaAudioProcessor p;
    juce::Random rng (9);
    randomise (p.getAPVTS(), rng);
    const auto before = snapshot (p.getAPVTS());

    INFO ("entrée: " << text);
    const auto r = PresetLoader::loadFromJsonString (juce::String (text), p.getAPVTS());

    REQUIRE_FALSE (r.success);
    REQUIRE (r.errorMessage.isNotEmpty());
    requireSameExcept (before, snapshot (p.getAPVTS()));
}

TEST_CASE ("PresetLoader: metadata.name d'un type inattendu ne doit pas lever d'exception")
{
    HarmoniaAudioProcessor p;

    const auto text = GENERATE (as<std::string>{},
                                R"({"metadata":{"name":123},"parameters":{"x":0.5}})",
                                R"({"metadata":{"name":null},"parameters":{"x":0.5}})",
                                R"({"metadata":{"name":["a"]},"parameters":{"x":0.5}})");

    INFO ("entrée: " << text);
    REQUIRE_NOTHROW (PresetLoader::loadFromJsonString (juce::String (text), p.getAPVTS()));
}

TEST_CASE ("PresetLoader: nom absent ou vide -> \"Preset\"")
{
    HarmoniaAudioProcessor p;

    auto r = PresetLoader::loadFromJsonString (R"({"parameters":{"x":0.5}})", p.getAPVTS());
    REQUIRE (r.success);
    REQUIRE (r.presetName == "Preset");

    r = PresetLoader::loadFromJsonString (R"({"metadata":{"name":""},"parameters":{"x":0.5}})", p.getAPVTS());
    REQUIRE (r.success);
    REQUIRE (r.presetName == "Preset");
}

//==============================================================================
// Application des valeurs
//==============================================================================

TEST_CASE ("PresetLoader: valeurs hors bornes ramenées dans [0, 1]")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();

    nlohmann::json doc;
    doc["parameters"][HarmoniaParams::IDs::filterCutoff.toStdString()]    = 1e30;
    doc["parameters"][HarmoniaParams::IDs::filterResonance.toStdString()] = -1e30;
    doc["parameters"][HarmoniaParams::IDs::ampSustain.toStdString()]      = 2.0;
    doc["parameters"][HarmoniaParams::IDs::reverbMix.toStdString()]       = -3.0;

    const auto r = PresetLoader::loadFromJsonString (juce::String (doc.dump()), a);
    REQUIRE (r.success);

    REQUIRE (a.getParameter (HarmoniaParams::IDs::filterCutoff)->getValue()    == Catch::Approx (1.0f));
    REQUIRE (a.getParameter (HarmoniaParams::IDs::filterResonance)->getValue() == Catch::Approx (0.0f).margin (1e-6));
    REQUIRE (a.getParameter (HarmoniaParams::IDs::ampSustain)->getValue()      == Catch::Approx (1.0f));
    REQUIRE (a.getParameter (HarmoniaParams::IDs::reverbMix)->getValue()       == Catch::Approx (0.0f).margin (1e-6));
}

TEST_CASE ("PresetLoader: un paramètre inconnu est ignoré, les autres sont appliqués")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();
    setAll (a, 0.0f);
    const auto before = snapshot (a);

    nlohmann::json doc;
    doc["parameters"]["parametre_qui_n_existe_pas"] = 0.5;
    doc["parameters"][HarmoniaParams::IDs::filterCutoff.toStdString()] = 0.25;

    REQUIRE_NOTHROW (PresetLoader::loadFromJsonString (juce::String (doc.dump()), a));

    auto* cutoff = a.getParameter (HarmoniaParams::IDs::filterCutoff);
    REQUIRE (cutoff->getValue() == Catch::Approx (snapped (cutoff, 0.25f)).margin (1e-4));
    requireSameExcept (before, snapshot (a), { HarmoniaParams::IDs::filterCutoff.toStdString() });
}

TEST_CASE ("PresetLoader: un paramètre absent du preset garde sa valeur")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();
    juce::Random rng (21);
    randomise (a, rng);
    const auto before = snapshot (a);

    nlohmann::json doc;
    doc["parameters"][HarmoniaParams::IDs::lfoRate.toStdString()] = 0.8;

    REQUIRE (PresetLoader::loadFromJsonString (juce::String (doc.dump()), a).success);
    requireSameExcept (before, snapshot (a), { HarmoniaParams::IDs::lfoRate.toStdString() });
}

TEST_CASE ("PresetLoader: les paramètres verrouillés ne sont jamais modifiés")
{
    HarmoniaAudioProcessor src, dst;
    setAll (src.getAPVTS(), 1.0f);
    setAll (dst.getAPVTS(), 0.0f);

    const auto expectedSrc = snapshot (src.getAPVTS());
    const auto beforeDst   = snapshot (dst.getAPVTS());
    const auto json        = PresetLoader::saveToJsonString (src.getAPVTS(), "Lock", "tests");

    juce::StringArray locked;
    locked.add (HarmoniaParams::IDs::filterCutoff);
    locked.add (HarmoniaParams::IDs::ampSustain);

    REQUIRE (PresetLoader::loadFromJsonString (json, dst.getAPVTS(), locked).success);

    const auto after = snapshot (dst.getAPVTS());

    for (const auto& id : locked)
    {
        INFO (id.toStdString());
        REQUIRE (after.at (id.toStdString()) == Catch::Approx (beforeDst.at (id.toStdString())).margin (1e-6));
    }

    std::set<std::string> lockedStd { HarmoniaParams::IDs::filterCutoff.toStdString(),
                                      HarmoniaParams::IDs::ampSustain.toStdString() };
    requireSameExcept (expectedSrc, after, lockedStd);   // tout le reste est appliqué
}

TEST_CASE ("PresetLoader: tout verrouillé -> rien ne change")
{
    HarmoniaAudioProcessor src, dst;
    setAll (src.getAPVTS(), 1.0f);
    setAll (dst.getAPVTS(), 0.0f);
    const auto before = snapshot (dst.getAPVTS());

    juce::StringArray all;
    for (const auto& id : HarmoniaParams::orderedIds())
        all.add (id);

    PresetLoader::loadFromJsonString (PresetLoader::saveToJsonString (src.getAPVTS(), "x", "tests"),
                                      dst.getAPVTS(), all);

    requireSameExcept (before, snapshot (dst.getAPVTS()));
}

//==============================================================================
// Forme "values" (fallback)
//==============================================================================

TEST_CASE ("PresetLoader: forme 'values' appliquée dans l'ordre de orderedIds()")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();
    setAll (a, 0.0f);

    const auto& ids = HarmoniaParams::orderedIds();

    nlohmann::json doc;
    doc["values"] = nlohmann::json::array();
    for (size_t i = 0; i < ids.size(); ++i)
        doc["values"].push_back ((double) i / (double) (ids.size() - 1));

    REQUIRE (PresetLoader::loadFromJsonString (juce::String (doc.dump()), a).success);

    for (size_t i = 0; i < ids.size(); ++i)
    {
        INFO (ids[i].toStdString());
        auto* prm = a.getParameter (ids[i]);
        const float x = (float) i / (float) (ids.size() - 1);
        REQUIRE (prm->getValue() == Catch::Approx (snapped (prm, x)).margin (1e-4));
    }
}

TEST_CASE ("PresetLoader: 'values' trop court, trop long ou avec des non-nombres")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();
    const auto& ids = HarmoniaParams::orderedIds();

    
    SECTION ("tableau plus court : refusé sans modification")
    {
        setAll (a, 0.0f);
        const auto before = snapshot (a);

        REQUIRE_FALSE (
            PresetLoader::loadFromJsonString (
                R"({"values":[0.9,0.9,0.9]})", a
            ).success
        );

        requireSameExcept (before, snapshot (a));
    }

    SECTION ("tableau plus long: les valeurs en trop sont ignorées")
    {
        nlohmann::json doc;
        doc["values"] = nlohmann::json::array();
        for (size_t i = 0; i < ids.size() + 10; ++i)
            doc["values"].push_back (0.5);

        REQUIRE_NOTHROW (PresetLoader::loadFromJsonString (juce::String (doc.dump()), a));
    }

    SECTION ("une valeur non numérique est sautée")
    {
        setAll (a, 0.0f);
        const auto before = snapshot (a);

        REQUIRE_FALSE(
            PresetLoader::loadFromJsonString(
                R"({"values":[0.9,null,0.9]})", a
            ).success
        );

        requireSameExcept (before, snapshot (a),
                           { ids[0].toStdString(), ids[2].toStdString() });   // ids[1] inchangé
    }
}

TEST_CASE ("PresetLoader: 'parameters' est prioritaire sur 'values'")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();
    setAll (a, 0.0f);
    const auto before = snapshot (a);

    nlohmann::json doc;
    doc["parameters"][HarmoniaParams::IDs::filterCutoff.toStdString()] = 0.3;
    doc["values"] = nlohmann::json::array();
    for (size_t i = 0; i < HarmoniaParams::orderedIds().size(); ++i)
        doc["values"].push_back (1.0);

    REQUIRE (PresetLoader::loadFromJsonString (juce::String (doc.dump()), a).success);
    requireSameExcept (before, snapshot (a), { HarmoniaParams::IDs::filterCutoff.toStdString() });
}

//==============================================================================
// Preset + audio
//==============================================================================

TEST_CASE ("PresetLoader: un preset extrême importé reste audio-safe")
{
    const float extreme = GENERATE (0.0f, 1.0f);

    HarmoniaAudioProcessor src, dst;
    setAll (src.getAPVTS(), extreme);

    const auto json = PresetLoader::saveToJsonString (src.getAPVTS(), "Extreme", "tests");
    REQUIRE (PresetLoader::loadFromJsonString (json, dst.getAPVTS()).success);

    REQUIRE (rendersFinite (dst));
}