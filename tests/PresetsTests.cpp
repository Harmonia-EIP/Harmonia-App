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

    REQUIRE (doc["metadata"]["name"] == "Mon Preset");
    REQUIRE (doc["metadata"]["generated_by"] == "tests");
    REQUIRE (doc["metadata"]["charter_version"] == std::string (HarmoniaCharter::version));
    REQUIRE ((doc["metadata"]["engine"] == "analog" || doc["metadata"]["engine"] == "dx7"));

    // charte v3 : tous les paramètres, normalisés ; plus de tableau 'values' (forme v2)
    REQUIRE (doc["parameters"].is_object());
    REQUIRE_FALSE (doc.contains ("values"));
    REQUIRE (doc["parameters"].size() == HarmoniaCharter::params.size());

    for (const auto& spec : HarmoniaCharter::params)
    {
        const std::string key (spec.id);
        INFO (key);
        REQUIRE (doc["parameters"].contains (key));
        REQUIRE (doc["parameters"][key].is_number());
        REQUIRE (doc["parameters"][key].get<double>() >= 0.0);
        REQUIRE (doc["parameters"][key].get<double>() <= 1.0);
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

    // 'values' est la forme v2 : normalisée sur les plages v2, convertie par les règles de la charte
    std::array<float, HarmoniaCharter::v2Ids.size()> v2 {};
    for (size_t i = 0; i < v2.size(); ++i)
        v2[i] = (float) i / (float) (ids.size() - 1);

    for (size_t i = HarmoniaCharter::analogBegin; i < HarmoniaCharter::analogEnd; ++i)
    {
        const auto& spec = HarmoniaCharter::params[i];
        INFO (std::string (spec.id));
        auto* prm = a.getParameter (HarmoniaParamsV3::toJuce (spec.id));
        const float expected = HarmoniaCharter::upgradeFromV2Normalized (spec, v2);
        REQUIRE (prm->convertFrom0to1 (prm->getValue()) == Catch::Approx (expected).epsilon (1e-3).margin (1e-3));
    }
}

TEST_CASE ("PresetLoader: preset v2 (modèles v1/v2) converti sur les plages v3")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();
    a.getParameter ("unison_voices")->setValueNotifyingHost (1.0f);   // réglage v3 qu'un preset v2 ne connaît pas

    nlohmann::json doc;
    doc["metadata"] = { { "name", "v2" }, { "charter_version", "1.0" } };
    doc["parameters"] = { { "filter_env_amount", 0.75 },   // v2 : -1..1 -> 0.5, x 4 octaves
                          { "lfo_to_pitch", 0.2 },         // v2 : x 50 cents
                          { "reverb_mix", 1.0 },           // donne aussi reverb_size = 0.55 + 0.35
                          { "amp_decay", 0.5 } };          // v2 : 10..5000 ms, centre 300

    REQUIRE (PresetLoader::loadFromJsonString (juce::String (doc.dump()), a).success);

    auto physical = [&] (const juce::String& id) { return a.getParameter (id)->convertFrom0to1 (a.getParameter (id)->getValue()); };
    REQUIRE (physical ("filter_env_amount") == Catch::Approx (2.0f).margin (1e-3));
    REQUIRE (physical ("lfo_to_pitch") == Catch::Approx (10.0f).margin (1e-2));
    REQUIRE (physical ("reverb_size") == Catch::Approx (0.9f).margin (1e-3));
    REQUIRE (physical ("amp_decay") == Catch::Approx (300.0f).epsilon (0.01));
    REQUIRE (physical ("unison_voices") == Catch::Approx (1.0f));     // remis à sa valeur neutre
    REQUIRE (physical ("engine") == Catch::Approx (0.0f));            // analogique
}

TEST_CASE ("PresetLoader: preset v3 appliqué tel quel")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();

    nlohmann::json doc;
    doc["metadata"] = { { "name", "v3" }, { "charter_version", std::string (HarmoniaCharter::version) } };
    doc["parameters"] = { { "engine", 0.0 }, { "unison_voices", 0.4 }, { "amp_decay", 0.5 } };

    REQUIRE (PresetLoader::loadFromJsonString (juce::String (doc.dump()), a).success);
    REQUIRE (a.getParameter ("unison_voices")->getValue() == Catch::Approx (snapped (a.getParameter ("unison_voices"), 0.4f)));
    REQUIRE (a.getParameter ("amp_decay")->getValue() == Catch::Approx (0.5f).margin (1e-5));
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
// Entrées de l'historique du backend (un nom et des parameters, sans metadata)
//==============================================================================

namespace
{
    // Valeurs normalisées actuelles de tous les paramètres de la charte v3
    Snapshot snapshotAll (juce::AudioProcessorValueTreeState& a)
    {
        Snapshot s;
        for (const auto& spec : HarmoniaCharter::params)
            s[std::string (spec.id)] = a.getParameter (HarmoniaParamsV3::toJuce (spec.id))->getValue();
        return s;
    }

    float physicalValue (juce::AudioProcessorValueTreeState& a, const juce::String& id)
    {
        auto* p = a.getParameter (id);
        return p->convertFrom0to1 (p->getValue());
    }
}

TEST_CASE ("PresetLoader: entrée d'historique v3 appliquée telle quelle")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();

    const std::map<juce::String, float> entry { { "engine", 1.0f }, { "unison_voices", 0.4f },
                                                { "filter_env_amount", 0.75f }, { "dx7_algorithm", 0.5f } };

    const auto r = PresetLoader::loadFromParameters (entry, "Acid lead", a);

    REQUIRE (r.success);
    REQUIRE (r.presetName == "Acid lead");
    REQUIRE (physicalValue (a, "engine") == Catch::Approx (1.0f));   // DX7 : pas de conversion v2
    REQUIRE (a.getParameter ("filter_env_amount")->getValue() == Catch::Approx (0.75f).margin (1e-5));
    REQUIRE (a.getParameter ("unison_voices")->getValue() == Catch::Approx (snapped (a.getParameter ("unison_voices"), 0.4f)));
}

TEST_CASE ("PresetLoader: entrée d'historique v2 convertie comme un fichier v2")
{
    HarmoniaAudioProcessor fromEntry, fromFile;
    juce::Random rng (17);

    std::map<juce::String, float> entry;
    nlohmann::json doc;
    doc["metadata"] = { { "name", "v2" }, { "charter_version", "1.0" } };
    for (const auto& id : HarmoniaCharter::v2Ids)
    {
        const float v = rng.nextFloat();
        entry[HarmoniaParamsV3::toJuce (id)] = v;
        doc["parameters"][std::string (id)] = v;
    }

    REQUIRE (PresetLoader::loadFromParameters (entry, "v2", fromEntry.getAPVTS()).success);
    REQUIRE (PresetLoader::loadFromJsonString (juce::String (doc.dump()), fromFile.getAPVTS()).success);

    requireSameExcept (snapshotAll (fromFile.getAPVTS()), snapshotAll (fromEntry.getAPVTS()));
    REQUIRE (physicalValue (fromEntry.getAPVTS(), "engine") == Catch::Approx (0.0f));   // analogique
}

TEST_CASE ("PresetLoader: entrée d'historique invalide refusée sans toucher aux paramètres")
{
    const auto entry = GENERATE (std::map<juce::String, float> {},
                                 std::map<juce::String, float> { { "parametre_qui_n_existe_pas", 0.5f } },
                                 std::map<juce::String, float> { { "engine", 1.0f }, { "amp_decay", 1.5f } },
                                 std::map<juce::String, float> { { "filter_cutoff", -0.1f } },
                                 std::map<juce::String, float> { { "filter_cutoff", std::nanf ("") } });

    HarmoniaAudioProcessor p;
    juce::Random rng (23);
    randomise (p.getAPVTS(), rng);
    const auto before = snapshotAll (p.getAPVTS());

    const auto r = PresetLoader::loadFromParameters (entry, "x", p.getAPVTS());

    REQUIRE_FALSE (r.success);
    REQUIRE (r.errorMessage.isNotEmpty());
    requireSameExcept (before, snapshotAll (p.getAPVTS()));
}

TEST_CASE ("PresetLoader: entrée d'historique en mode affinage, les verrouillés gardent leur valeur")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();
    a.getParameter ("filter_cutoff")->setValueNotifyingHost (0.1f);
    const float lockedBefore = a.getParameter ("filter_cutoff")->getValue();

    const std::map<juce::String, float> entry { { "engine", 0.0f }, { "filter_cutoff", 0.9f }, { "amp_decay", 0.3f } };
    juce::StringArray locked { "filter_cutoff" };

    const auto r = PresetLoader::loadFromParameters (entry, {}, a, locked);

    REQUIRE (r.success);
    REQUIRE (r.presetName == "Preset");
    REQUIRE (a.getParameter ("filter_cutoff")->getValue() == Catch::Approx (lockedBefore));
    REQUIRE (a.getParameter ("amp_decay")->getValue() == Catch::Approx (0.3f).margin (1e-5));
}

TEST_CASE ("PresetLoader: fichier sans metadata mais avec des paramètres v3 lu en v3")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();

    REQUIRE (PresetLoader::loadFromJsonString (R"({"parameters":{"engine":1.0,"filter_env_amount":0.75}})", a).success);
    REQUIRE (physicalValue (a, "engine") == Catch::Approx (1.0f));
    REQUIRE (a.getParameter ("filter_env_amount")->getValue() == Catch::Approx (0.75f).margin (1e-5));
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