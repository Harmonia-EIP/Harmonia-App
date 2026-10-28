#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "PluginProcessor.h"
#include <set>

using namespace HarmoniaParams;

TEST_CASE ("orderedIds: 20 IDs uniques, tous présents dans l'APVTS")
{
    HarmoniaAudioProcessor p;
    std::set<juce::String> seen;

    for (const auto& id : orderedIds())
    {
        INFO (id.toStdString());
        REQUIRE (seen.insert (id).second);                       // pas de doublon
        REQUIRE (p.getAPVTS().getParameter (id) != nullptr);     // existe
        REQUIRE (p.getAPVTS().getRawParameterValue (id) != nullptr);
    }
    REQUIRE (seen.size() == 20);
}

TEST_CASE ("Le layout contient exactement les paramètres de la charte v3")
{
    HarmoniaAudioProcessor p;
    // 192 paramètres : engine, 46 analogiques, 145 DX7 (HARMONIA/charter/harmonia_v3.json)
    REQUIRE (p.getParameters().size() == (int) HarmoniaCharter::params.size());

    for (const auto& spec : HarmoniaCharter::params)
    {
        INFO (std::string (spec.id));
        REQUIRE (p.getAPVTS().getParameter (HarmoniaParamsV3::toJuce (spec.id)) != nullptr);
    }
}

TEST_CASE ("Valeurs par défaut et bornes : celles de la charte")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();

    for (const auto& spec : HarmoniaCharter::params)
    {
        INFO (std::string (spec.id));
        auto* prm = a.getParameter (HarmoniaParamsV3::toJuce (spec.id));
        REQUIRE (prm->convertFrom0to1 (prm->getDefaultValue()) == Catch::Approx (spec.defaultValue).margin (1e-4));
        REQUIRE (prm->convertFrom0to1 (0.0f) == Catch::Approx (spec.min).epsilon (0.001));
        REQUIRE (prm->convertFrom0to1 (1.0f) == Catch::Approx (spec.max).epsilon (0.001));
    }

    // valeurs neutres de la v3 : un preset v2 converti garde son son
    auto def = [&] (const juce::String& id)
    {
        return a.getParameter (id)->convertFrom0to1 (a.getParameter (id)->getDefaultValue());
    };
    REQUIRE (def (IDs::osc1Waveform) == Catch::Approx (0.0f));      // Sine
    REQUIRE (def (IDs::filterCutoff) == Catch::Approx (20000.0f).epsilon (0.001));
    REQUIRE (def (IDs::ampSustain)   == Catch::Approx (1.0f));
    REQUIRE (def (IDs::reverbMix)    == Catch::Approx (0.0f));
    REQUIRE (def ("unison_voices")   == Catch::Approx (1.0f));
    REQUIRE (def ("engine")          == Catch::Approx (0.0f));      // Analog
}

TEST_CASE ("makeLogRange: le milieu du slider vaut le midpoint demandé")
{
    struct Case { float lo, hi, mid; };
    const auto c = GENERATE (Case { 20.0f, 20000.0f, 1000.0f },
                             Case { 1.0f, 5000.0f, 100.0f },
                             Case { 10.0f, 10000.0f, 500.0f },
                             Case { 0.1f, 20.0f, 2.0f });

    auto range = makeLogRange (c.lo, c.hi, c.mid);
    REQUIRE (range.convertFrom0to1 (0.5f) == Catch::Approx (c.mid).epsilon (0.01));
    REQUIRE (range.convertFrom0to1 (0.0f) == Catch::Approx (c.lo));
    REQUIRE (range.convertFrom0to1 (1.0f) == Catch::Approx (c.hi));
}

TEST_CASE ("makeLogRange: aller-retour 0..1 monotone")
{
    auto range = makeLogRange (20.0f, 20000.0f, 1000.0f);
    float prev = -1.0f;
    for (int i = 0; i <= 100; ++i)
    {
        const float v = range.convertFrom0to1 (i / 100.0f);
        REQUIRE (v > prev);
        REQUIRE (range.convertTo0to1 (v) == Catch::Approx (i / 100.0f).margin (1e-3));
        prev = v;
    }
}

TEST_CASE ("AtomicRefs: tous les pointeurs liés et cohérents avec l'APVTS")
{
    HarmoniaAudioProcessor p;
    AtomicRefs refs;
    refs.bind (p.getAPVTS());

    for (auto* ptr : { refs.osc1Waveform, refs.osc2Waveform, refs.oscMix, refs.osc2Detune,
                       refs.noiseLevel, refs.filterCutoff, refs.filterResonance, refs.filterType,
                       refs.ampAttack, refs.ampDecay, refs.ampSustain, refs.ampRelease,
                       refs.filterEnvAmount, refs.filterEnvDecay, refs.lfoRate, refs.lfoToPitch,
                       refs.lfoToCutoff, refs.velocityToFilter, refs.distortionMix, refs.reverbMix })
        REQUIRE (ptr != nullptr);

    // Le pointeur atomique reflète le changement de paramètre
    auto* cutoff = p.getAPVTS().getParameter (IDs::filterCutoff);
    cutoff->setValueNotifyingHost (cutoff->convertTo0to1 (440.0f));
    REQUIRE (refs.filterCutoff->load() == Catch::Approx (440.0f).epsilon (0.01));
}

TEST_CASE ("ParameterRefs v3 : chaque paramètre de la charte est lié")
{
    HarmoniaAudioProcessor p;
    HarmoniaParamsV3::ParameterRefs refs;
    refs.bind (p.getAPVTS());

    for (auto* ptr : refs.values)
        REQUIRE (ptr != nullptr);

    auto* cutoff = p.getAPVTS().getParameter (IDs::filterCutoff);
    cutoff->setValueNotifyingHost (cutoff->convertTo0to1 (440.0f));
    REQUIRE (refs.analog (HarmoniaCharter::analog::filter_cutoff) == Catch::Approx (440.0f).epsilon (0.01));
}

TEST_CASE ("Paramètres de choix: nombre d'entrées et textes")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();

    auto* wave = dynamic_cast<juce::AudioParameterChoice*> (a.getParameter (IDs::osc1Waveform));
    auto* filt = dynamic_cast<juce::AudioParameterChoice*> (a.getParameter (IDs::filterType));
    REQUIRE (wave != nullptr);
    REQUIRE (filt != nullptr);

    REQUIRE (wave->choices.size() == 4);
    REQUIRE (wave->choices[2] == "Saw");
    REQUIRE (filt->choices.size() == 3);
    REQUIRE (filt->choices[0] == "Low-pass");
}

TEST_CASE ("Valeurs hors bornes: l'APVTS les ramène dans la plage")
{
    HarmoniaAudioProcessor p;
    auto* res = p.getAPVTS().getParameter (IDs::filterResonance);

    res->setValueNotifyingHost (5.0f);    // trop grand
    REQUIRE (res->getValue() <= 1.0f);
    res->setValueNotifyingHost (-3.0f);   // trop petit
    REQUIRE (res->getValue() >= 0.0f);
}