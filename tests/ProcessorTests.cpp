#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_approx.hpp>
#include <catch2/generators/catch_generators.hpp>
#include "PluginProcessor.h"
#include <cmath>

namespace
{
    bool allFinite (const juce::AudioBuffer<float>& b)
    {
        for (int ch = 0; ch < b.getNumChannels(); ++ch)
            for (int i = 0; i < b.getNumSamples(); ++i)
                if (! std::isfinite (b.getSample (ch, i)))
                    return false;
        return true;
    }
}

TEST_CASE ("State: aller-retour conserve les paramètres")
{
    HarmoniaAudioProcessor a, b;

    auto* cutoff = a.getAPVTS().getParameter (HarmoniaParams::IDs::filterCutoff);
    REQUIRE (cutoff != nullptr);
    cutoff->setValueNotifyingHost (0.25f);

    juce::MemoryBlock mb;
    a.getStateInformation (mb);
    b.setStateInformation (mb.getData(), (int) mb.getSize());

    REQUIRE (b.getAPVTS().getParameter (HarmoniaParams::IDs::filterCutoff)->getValue()
             == Catch::Approx (0.25f));
}

TEST_CASE ("State: données invalides ignorées")
{
    HarmoniaAudioProcessor p;
    auto* prm = p.getAPVTS().getParameter (HarmoniaParams::IDs::reverbMix);
    prm->setValueNotifyingHost (0.6f);

    const char garbage[] = "pas du xml";
    p.setStateInformation (garbage, (int) sizeof (garbage));

    REQUIRE (prm->getValue() == Catch::Approx (0.6f));
}

TEST_CASE ("processBlock: silence sans note, son fini avec note")
{
    const auto sr = GENERATE (44100.0, 48000.0, 96000.0);
    const auto bs = GENERATE (1, 64, 512, 4096);

    HarmoniaAudioProcessor proc;
    proc.setRateAndBufferSizeDetails (sr, bs);
    proc.prepareToPlay (sr, bs);

    juce::AudioBuffer<float> buf (2, bs);
    juce::MidiBuffer midi;

    SECTION ("sans note: silence")
    {
        proc.processBlock (buf, midi);
        REQUIRE (allFinite (buf));
        REQUIRE (buf.getMagnitude (0, bs) == Catch::Approx (0.0f).margin (1e-6));
    }

    SECTION ("note on: signal non nul et fini")
    {
        midi.addEvent (juce::MidiMessage::noteOn (1, 60, (juce::uint8) 100), 0);

        const int blocks = juce::jmax (1, (int) (sr * 0.5 / bs));
        float peak = 0.0f;

        for (int i = 0; i < blocks; ++i)
        {
            proc.processBlock (buf, midi);
            REQUIRE (allFinite (buf));
            peak = juce::jmax (peak, buf.getMagnitude (0, bs));
            midi.clear();
        }

        REQUIRE (peak > 0.001f);
    }
}
TEST_CASE ("State v2 (avant la charte v3) : converti par les règles de la charte")
{
    HarmoniaAudioProcessor p;
    p.getAPVTS().getParameter ("chorus_mix")->setValueNotifyingHost (1.0f);

    // état enregistré par un plugin v2 : valeurs physiques des 20 paramètres, pas de charter_version
    juce::XmlElement xml ("HARMONIA");
    xml.setAttribute ("presetName", "ancien projet");
    auto addParam = [&] (const char* id, double value)
    {
        auto* e = xml.createNewChildElement ("PARAM");
        e->setAttribute ("id", id);
        e->setAttribute ("value", value);
    };
    addParam ("filter_env_amount", 0.5);   // x 4 octaves en v3
    addParam ("reverb_mix", 0.4);
    addParam ("filter_cutoff", 1200.0);

    juce::MemoryBlock mb;
    juce::AudioProcessor::copyXmlToBinary (xml, mb);
    p.setStateInformation (mb.getData(), (int) mb.getSize());

    auto& a = p.getAPVTS();
    auto physical = [&] (const juce::String& id) { return a.getParameter (id)->convertFrom0to1 (a.getParameter (id)->getValue()); };
    REQUIRE (physical ("filter_env_amount") == Catch::Approx (2.0f).margin (1e-3));
    REQUIRE (physical ("reverb_size") == Catch::Approx (0.55f + 0.35f * 0.4f).margin (1e-3));
    REQUIRE (physical ("filter_cutoff") == Catch::Approx (1200.0f).epsilon (0.001));
    REQUIRE (physical ("chorus_mix") == Catch::Approx (0.0f));      // paramètre v3 : valeur neutre
    REQUIRE (a.state.getProperty ("presetName").toString() == "ancien projet");

    // réenregistré, il est désormais en v3 et se relit tel quel
    juce::MemoryBlock again;
    p.getStateInformation (again);
    HarmoniaAudioProcessor q;
    q.setStateInformation (again.getData(), (int) again.getSize());
    REQUIRE (q.getAPVTS().getParameter ("filter_env_amount")->getValue()
             == Catch::Approx (a.getParameter ("filter_env_amount")->getValue()));
}
