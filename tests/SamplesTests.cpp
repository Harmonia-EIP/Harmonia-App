// Source « échantillon » de la charte 3.1 : choix de l'enregistrement, lecture dans le moteur, bibliothèque.
// Aucun vrai échantillon : de petits enregistrements de synthèse écrits par les tests.

#include <catch2/catch_test_macros.hpp>
#include <catch2/matchers/catch_matchers_floating_point.hpp>

#include "engine/HarmoniaEngineV3.h"
#include "engine/SampleLibrary.h"

#include <cmath>

namespace
{
    using namespace HarmoniaCharter::analog;

    harmonia::Recording sine (double frequency, double seconds, double rate = 32000.0, float level = 0.5f)
    {
        harmonia::Recording rec;
        rec.sampleRate = rate;
        const auto n = (std::size_t) (seconds * rate);
        for (std::size_t i = 0; i < n; ++i)
            rec.left.push_back ((std::int16_t) std::lround (level * 32767.0 * std::sin (2.0 * 3.14159265358979 * frequency * (double) i / rate)));
        return rec;
    }

    harmonia::SampleRegion region (int key, int lo, int hi, int lovel, int hivel, int rr, std::size_t rec, std::string file)
    {
        harmonia::SampleRegion r;
        r.key = key; r.lokey = lo; r.hikey = hi; r.lovel = lovel; r.hivel = hivel; r.roundRobin = rr;
        r.recording = rec; r.file = std::move (file);
        return r;
    }

    std::shared_ptr<harmonia::SampledInstrument> testInstrument()
    {
        auto inst = std::make_shared<harmonia::SampledInstrument>();
        inst->recordings.push_back (sine (261.6256, 3.0));   // C4
        inst->recordings.push_back (sine (523.2511, 3.0));   // C5
        inst->regions = { region (60, 0, 66, 1, 127, 1, 0, "a"),
                          region (72, 67, 127, 1, 63, 1, 1, "b"),
                          region (72, 67, 127, 64, 127, 2, 1, "d"),
                          region (72, 67, 127, 64, 127, 1, 1, "c") };
        return inst;
    }

    harmonia::AnalogPatch samplePatch (double level)
    {
        auto p = harmonia::defaultPatch();
        p[filter_cutoff] = 20000.0;
        p[amp_sustain] = 1.0;
        p[sample_instrument] = 1.0;
        p[sample_level] = level;
        return p;
    }

    /** Frequency from the zero crossings of a stretch of signal. */
    double frequencyOf (const std::vector<float>& x, int rate, int from, int to)
    {
        int crossings = 0;
        for (int i = from + 1; i < to; ++i)
            if ((x[(size_t) i - 1] < 0.0f) != (x[(size_t) i] < 0.0f))
                ++crossings;
        return crossings / 2.0 / ((to - from) / (double) rate);
    }
}

TEST_CASE ("Échantillons : l'enregistrement choisi suit la note, la vélocité et la prise (comme l'IA)")
{
    auto inst = testInstrument();
    CHECK (inst->region (50, 100, 0)->file == "a");
    CHECK (inst->region (70, 30, 0)->file == "b");
    // deux prises sur la couche forte, triées par round robin puis par fichier
    CHECK (inst->region (70, 100, 0)->file == "c");
    CHECK (inst->region (70, 100, 1)->file == "d");
    CHECK (inst->region (70, 100, 2)->file == "c");
    // hors de toute plage : la note enregistrée la plus proche
    harmonia::SampledInstrument narrow;
    narrow.regions = { region (60, 58, 62, 1, 127, 1, 0, "low"), region (72, 70, 74, 1, 127, 1, 0, "high") };
    CHECK (narrow.region (66, 100, 0)->file == "low");   // égalité : la plus grave
    CHECK (narrow.region (90, 100, 0)->file == "high");
}

TEST_CASE ("Échantillons : le moteur joue l'instrument à la hauteur de la note, transposable")
{
    auto inst = testInstrument();
    const int rate = 48000, n = rate * 2;
    for (const auto& [note, tune, expected] : { std::tuple<int, double, double> { 60, 0.0, 261.63 }, { 64, 0.0, 329.63 },
                                                { 72, 0.0, 523.25 }, { 60, 12.0, 523.25 } })
    {
        auto p = samplePatch (1.0);
        p[sample_tune] = tune;
        std::vector<float> l ((size_t) n), r ((size_t) n);
        const auto sample = harmonia::SampleNote::make (inst, note, 100, 0);
        REQUIRE (sample.playable());
        harmonia::renderNote (p, note, 100.0 / 127.0, 1.5, 2.0, rate, 7, l.data(), r.data(), sample);
        CHECK_THAT (frequencyOf (l, rate, rate / 4, rate), Catch::Matchers::WithinRel (expected, 0.01));
    }
}

TEST_CASE ("Échantillons : niveau 0 ou pas d'instrument décodé = exactement le son de la charte 3.0")
{
    auto inst = testInstrument();
    const int rate = 48000, n = rate;
    auto p = harmonia::defaultPatch();
    p[osc_1_waveform] = 2;
    p[unison_voices] = 3;
    std::vector<float> ref ((size_t) n), refR ((size_t) n);
    harmonia::renderNote (p, 57, 0.8, 0.5, 1.0, rate, 7, ref.data(), refR.data());
    for (double level : { 0.0, 0.7 })
    {
        auto q = p;
        q[sample_instrument] = 1;
        q[sample_level] = level;
        std::vector<float> l ((size_t) n), r ((size_t) n);
        // level 0 with an instrument; level 0.7 without a decoded instrument (empty SampleNote)
        const auto sample = level == 0.0 ? harmonia::SampleNote::make (inst, 57, 100, 0) : harmonia::SampleNote {};
        harmonia::renderNote (q, 57, 0.8, 0.5, 1.0, rate, 7, l.data(), r.data(), sample);
        CHECK (l == ref);
        CHECK (r == refR);
    }
}

TEST_CASE ("Échantillons : la bibliothèque lit un paquet, signale ce qui manque, et décode en arrière-plan")
{
    auto folder = juce::File::createTempFile ("harmonia-samples");
    folder.createDirectory();

    SampleLibrary library (folder);
    CHECK (library.status (0) == SampleLibrary::Status::none);
    CHECK_FALSE (library.waitUntilReady (1, 2000));
    CHECK (library.status (1) == SampleLibrary::Status::notInstalled);

    // un paquet minuscule : le premier instrument de la charte, une note enregistrée
    const auto id = juce::String (HarmoniaCharter::sampleInstrumentIds[0].data(), HarmoniaCharter::sampleInstrumentIds[0].size());
    auto wav = folder.getChildFile ("lib/c4.wav");
    wav.getParentDirectory().createDirectory();
    {
        juce::WavAudioFormat format;
        std::unique_ptr<juce::OutputStream> out (wav.createOutputStream());
        std::unique_ptr<juce::AudioFormatWriter> writer (format.createWriterFor (out.get(), 32000, 1, 16, {}, 0));
        REQUIRE (writer != nullptr);
        out.release();
        juce::AudioBuffer<float> buffer (1, 32000);
        for (int i = 0; i < 32000; ++i)
            buffer.setSample (0, i, 0.5f * std::sin (2.0f * 3.14159265f * 261.6256f * (float) i / 32000.0f));
        writer->writeFromAudioSampleBuffer (buffer, 0, 32000);
    }
    folder.getChildFile ("catalog.json").replaceWithText (
        "{\"version\": 1, \"instruments\": [{\"id\": \"" + id + "\", \"gain\": 2.0, \"regions\": ["
        "{\"file\": \"lib/c4.wav\", \"key\": 60, \"lokey\": 0, \"hikey\": 127, \"lovel\": 1, \"hivel\": 127,"
        " \"round_robin\": 1, \"trigger\": \"attack\", \"frames\": 32000, \"channels\": 1}]}]}");
    folder.getChildFile ("CREDITS.md").replaceWithText ("Salamander Grand Piano by Alexander Holm, CC-BY-3.0");
    library.setFolder (folder);

    REQUIRE (library.waitUntilReady (1, 5000));
    auto inst = library.get (1);
    REQUIRE (inst != nullptr);
    CHECK (inst->gain == 2.0);
    CHECK (inst->recordings.front().frames() == 32000);
    CHECK (library.loadedBytes() == 64000);
    CHECK (library.credits().contains ("Alexander Holm"));
    CHECK_FALSE (library.waitUntilReady (2, 2000));
    CHECK (library.status (2) == SampleLibrary::Status::notInPack);

    folder.deleteRecursively();
}
