#include <catch2/catch_test_macros.hpp>

#include "PluginProcessor.h"
#include "tools/ParamRandomizer.h"

namespace
{
    float physical (juce::AudioProcessorValueTreeState& a, const juce::String& id)
    {
        auto* p = a.getParameter (id);
        return p->convertFrom0to1 (p->getValue());
    }

    std::map<juce::String, float> snapshot (juce::AudioProcessorValueTreeState& a)
    {
        std::map<juce::String, float> s;
        for (const auto& spec : HarmoniaCharter::params)
        {
            const auto id = HarmoniaParams::toJuce (spec.id);
            s[id] = a.getParameter (id)->getValue();
        }
        return s;
    }
}

TEST_CASE ("Random: chaque valeur reste dans la charte et sur un pas légal", "[random]")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();

    for (int run = 0; run < 50; ++run)
    {
        ParamRandomizer::randomize (a, {});

        for (size_t i = HarmoniaCharter::analogBegin; i < HarmoniaCharter::analogEnd; ++i)
        {
            const auto& spec = HarmoniaCharter::params[i];
            const float v = physical (a, HarmoniaParams::toJuce (spec.id));

            INFO (std::string (spec.id) << " = " << v);
            REQUIRE (v >= spec.min - 1e-4f);
            REQUIRE (v <= spec.max + 1e-4f);
            REQUIRE (HarmoniaCharter::snap (spec, v) == v);
        }
    }
}

TEST_CASE ("Random: le patch reste jouable (jamais muet)", "[random]")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();

    for (int run = 0; run < 50; ++run)
    {
        ParamRandomizer::randomize (a, {});
        REQUIRE (physical (a, HarmoniaParams::IDs::ampSustain) >= 0.35f - 1e-4f);
    }
}

TEST_CASE ("Random: verrouillés, exclus, moteur et DX7 ne bougent pas", "[random]")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();

    a.getParameter (HarmoniaParams::IDs::engine)->setValueNotifyingHost (1.0f);
    const auto before = snapshot (a);

    const juce::StringArray locked   { HarmoniaParams::IDs::filterCutoff, HarmoniaParams::IDs::ampAttack };
    const juce::StringArray excluded { HarmoniaParams::IDs::reverbMix };

    ParamRandomizer::randomize (a, locked, excluded);
    const auto after = snapshot (a);

    for (const auto& id : locked)    CHECK (after.at (id) == before.at (id));
    for (const auto& id : excluded)  CHECK (after.at (id) == before.at (id));
    CHECK (after.at (HarmoniaParams::IDs::engine) == before.at (HarmoniaParams::IDs::engine));

    for (size_t i = HarmoniaCharter::dx7Begin; i < HarmoniaCharter::dx7End; ++i)
    {
        const auto id = HarmoniaParams::toJuce (HarmoniaCharter::params[i].id);
        INFO (id.toStdString());
        CHECK (after.at (id) == before.at (id));
    }
}

TEST_CASE ("Random: deux tirages donnent deux patchs différents", "[random]")
{
    HarmoniaAudioProcessor p;
    auto& a = p.getAPVTS();

    ParamRandomizer::randomize (a, {});
    const auto first = snapshot (a);
    ParamRandomizer::randomize (a, {});

    CHECK (snapshot (a) != first);
}
