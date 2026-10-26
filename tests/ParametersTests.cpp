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

TEST_CASE ("Le layout ne contient que les paramètres déclarés")
{
    HarmoniaAudioProcessor p;
    // 20 paramètres exposés, ni plus ni moins (détecte un oubli dans orderedIds)
    REQUIRE (p.getParameters().size() == (int) orderedIds().size());
}

TEST_CASE ("Valeurs par défaut (dénormalisées)")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();

    auto def = [&] (const juce::String& id)
    {
        return a.getParameter (id)->convertFrom0to1 (a.getParameter (id)->getDefaultValue());
    };

    REQUIRE (def (IDs::osc1Waveform)    == Catch::Approx (2.0f));   // Saw
    REQUIRE (def (IDs::osc2Waveform)    == Catch::Approx (1.0f));   // Triangle
    REQUIRE (def (IDs::oscMix)          == Catch::Approx (0.5f));
    REQUIRE (def (IDs::filterCutoff)    == Catch::Approx (8000.0f).epsilon (0.01));
    REQUIRE (def (IDs::filterResonance) == Catch::Approx (0.1f));
    REQUIRE (def (IDs::filterType)      == Catch::Approx (0.0f));   // Lowpass
    REQUIRE (def (IDs::ampSustain)      == Catch::Approx (0.8f));
    REQUIRE (def (IDs::reverbMix)       == Catch::Approx (0.2f));
}

TEST_CASE ("Bornes réelles des paramètres")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();

    struct Bounds { const juce::String* id; float lo, hi; };
    const Bounds table[] = {
        { &IDs::osc2Detune,       0.0f,    100.0f },
        { &IDs::filterCutoff,     20.0f,   20000.0f },
        { &IDs::filterResonance,  0.0f,    0.95f },
        { &IDs::ampAttack,        1.0f,    5000.0f },
        { &IDs::ampDecay,         10.0f,   5000.0f },
        { &IDs::ampRelease,       10.0f,   10000.0f },
        { &IDs::filterEnvAmount,  -1.0f,   1.0f },
        { &IDs::filterEnvDecay,   10.0f,   5000.0f },
        { &IDs::lfoRate,          0.1f,    20.0f },
    };

    for (const auto& b : table)
    {
        INFO (b.id->toStdString());
        auto* prm = a.getParameter (*b.id);
        REQUIRE (prm->convertFrom0to1 (0.0f) == Catch::Approx (b.lo).epsilon (0.001));
        REQUIRE (prm->convertFrom0to1 (1.0f) == Catch::Approx (b.hi).epsilon (0.001));
    }
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
    REQUIRE (filt->choices[0] == "Lowpass");
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