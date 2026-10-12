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