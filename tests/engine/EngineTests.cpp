// Tests of the v3 engines on their own (plain C++17, no JUCE): analog (pitch, release, block-size invariance,
// stability on any charter values) and DX7 mode (INIT VOICE pitch, patch layout, release). Whether they sound
// like the AI's reference renderers is checked by HARMONIA/scripts/v3/engine_parity.py (Harmonia-AI).

#include "../../src/engine/HarmoniaEngineV3.h"
#include "../../src/engine/HarmoniaDx7.h"

#include <cmath>
#include <cstdio>
#include <functional>
#include <string>
#include <vector>

using namespace harmonia;
namespace A = HarmoniaCharter::analog;

namespace
{
    int failures = 0;

    void check (bool ok, const std::string& what)
    {
        if (! ok)
        {
            std::printf ("FAIL %s\n", what.c_str());
            ++failures;
        }
    }

    void test (const char* name, const std::function<void()>& body)
    {
        const int before = failures;
        body();
        std::printf ("%s %s\n", failures == before ? "ok  " : "FAIL", name);
    }

    constexpr int sr = 48000;

    std::vector<float> renderMono (const AnalogPatch& patch, int note, double hold, double total, std::uint64_t seed = 7)
    {
        const int n = (int) (total * sr);
        std::vector<float> l ((std::size_t) n), r ((std::size_t) n);
        renderNote (patch, note, 100.0 / 127.0, hold, total, sr, seed, l.data(), r.data());
        for (std::size_t i = 0; i < l.size(); ++i)
            l[i] = 0.5f * (l[i] + r[i]);
        return l;
    }

    double rms (const std::vector<float>& x, std::size_t from, std::size_t to)
    {
        double sum = 0.0;
        for (std::size_t i = from; i < to; ++i)
            sum += (double) x[i] * x[i];
        return std::sqrt (sum / (double) (to - from));
    }

    AnalogPatch patchFromNormalized (double t)
    {
        AnalogPatch p {};
        for (int i = 0; i < A::count; ++i)
        {
            const auto& spec = HarmoniaCharter::params[HarmoniaCharter::analogBegin + (std::size_t) i];
            const double n = std::fmod (t * (i + 1) * 0.6180339887, 1.0);
            p[(std::size_t) i] = HarmoniaCharter::fromNormalized (spec, (float) n);
        }
        return p;
    }
}

int main()
{
    test ("default patch plays a 440 Hz sine on A4", []
    {
        const auto x = renderMono (defaultPatch(), 69, 1.5, 2.0);
        int crossings = 0;
        for (std::size_t i = sr / 2 + 1; i < (std::size_t) sr * 3 / 2; ++i)
            crossings += (x[i - 1] < 0.0f) != (x[i] < 0.0f);
        check (std::abs (crossings / 2.0 - 440.0) <= 1.0, "pitch: " + std::to_string (crossings / 2.0) + " Hz");
        check (rms (x, sr / 2, sr) > 0.1, "audible");
    });

    test ("released note fades out and the voice stops", []
    {
        auto patch = defaultPatch();
        patch[A::amp_release] = 200.0;
        AnalogVoice voice;
        voice.prepare (sr);
        voice.start (patch, 60, 0.8, 1);
        std::vector<float> l (4800), r (4800);
        voice.render (patch, l.data(), r.data(), 4800);
        check (voice.isActive(), "active while held");
        voice.release();
        for (int block = 0; block < 10 && voice.isActive(); ++block)
            voice.render (patch, l.data(), r.data(), 4800);
        check (! voice.isActive(), "stopped within 1 s of a 200 ms release");
    });

    test ("output does not depend on the block size", []
    {
        auto patch = patchFromNormalized (0.37);
        patch[A::unison_voices] = 5;
        std::vector<std::vector<float>> outs;
        for (int block : { 1, 64, 480 })
        {
            AnalogVoice voice;
            voice.prepare (sr);
            voice.start (patch, 52, 0.7, 11);
            std::vector<float> l ((std::size_t) sr), r ((std::size_t) sr);
            for (int i = 0; i < sr; i += block)
                voice.render (patch, l.data() + i, r.data() + i, std::min (block, sr - i));
            outs.push_back (l);
        }
        check (outs[0] == outs[1] && outs[1] == outs[2], "identical renders");
    });

    test ("same seed, same note; another seed draws other unison phases", []
    {
        auto patch = defaultPatch();
        patch[A::unison_voices] = 4;
        patch[A::osc_1_waveform] = 2;
        const auto a = renderMono (patch, 48, 0.5, 1.0, 5), b = renderMono (patch, 48, 0.5, 1.0, 5);
        const auto c = renderMono (patch, 48, 0.5, 1.0, 6);
        check (a == b, "deterministic");
        check (a != c, "seed matters");
    });

    test ("any charter values give finite, bounded audio", []
    {
        for (int k = 0; k < 120; ++k)
        {
            const auto patch = patchFromNormalized (0.013 + k * 0.0371);
            const auto x = renderMono (patch, 24 + (k * 7) % 72, 0.5, 1.5, (std::uint64_t) k);
            bool finite = true;
            float peak = 0.0f;
            for (float v : x)
            {
                finite = finite && std::isfinite (v);
                peak = std::max (peak, std::abs (v));
            }
            check (finite && peak < 64.0f, "patch " + std::to_string (k) + " peak " + std::to_string (peak));
        }
    });

    test ("reverb off keeps the v2 dry law (x 2)", []
    {
        auto patch = defaultPatch();
        patch[A::reverb_mix] = 0.0;
        Effects fx;
        fx.prepare (sr);
        std::vector<float> l (64, 0.25f), r (64, -0.5f);
        fx.process (patch, l.data(), r.data(), 64);
        check (l[10] == 0.5f && r[10] == -1.0f, "dry x 2");
    });

    test ("C entry point checks its arguments", []
    {
        auto patch = defaultPatch();
        std::vector<float> out (2 * sr);
        check (harmonia_render_analog_note (patch.data(), A::count, 60, 0.8, 0.5, 1.0, sr, 1, out.data(), (int) out.size()) == sr,
               "renders");
        check (harmonia_render_analog_note (patch.data(), A::count - 1, 60, 0.8, 0.5, 1.0, sr, 1, out.data(), (int) out.size()) == -1,
               "wrong parameter count");
        check (harmonia_render_analog_note (patch.data(), A::count, 60, 0.8, 0.5, 1.0, sr, 1, out.data(), sr) == -1,
               "buffer too small");
    });

    test ("DX7 INIT VOICE plays a 440 Hz sine on A4", []
    {
        const auto patch = dx7InitPatch();
        check (patch[5 * 21 + 16] == 99 && patch[16] == 0, "OP1 at full level, OP6 silent (OP6 first in the buffer)");
        check (patch[134] == 0 && patch[144] == 24, "algorithm 1, no transpose");
        Dx7Voice voice;
        voice.prepare (sr);
        voice.start (patch, 69, 100.0 / 127.0);
        std::vector<float> l ((std::size_t) sr), r ((std::size_t) sr);
        voice.render (l.data(), r.data(), sr);
        int crossings = 0;
        for (std::size_t i = sr / 4 + 1; i < (std::size_t) sr * 3 / 4; ++i)
            crossings += (l[i - 1] < 0.0f) != (l[i] < 0.0f);
        check (std::abs (crossings - 440.0) <= 2.0, "pitch: " + std::to_string (crossings) + " Hz");
        check (l == r, "mono on both channels");
    });

    test ("DX7 patch follows the charter's parameter order", []
    {
        std::array<float, dx7ParameterCount> values {};
        for (std::size_t k = 0; k < values.size(); ++k)
            values[k] = HarmoniaCharter::params[HarmoniaCharter::dx7Begin + k].defaultValue;
        values[0] = 12.0f;              // dx7_op1_eg_rate_1
        values[5 * 21 + 16] = 55.0f;    // dx7_op6_output_level
        values[6 * 21 + 8] = 21.0f;     // dx7_algorithm (shown 22)
        const auto patch = dx7PatchFromValues (values.data());
        check (patch[5 * 21] == 12 && patch[16] == 55 && patch[134] == 21, "OP1 last, OP6 first, voice fields at 126");
    });

    test ("DX7 note releases, falls silent and frees its voice; block size does not matter", []
    {
        std::vector<std::vector<float>> outs;
        for (int blockLen : { 1, 64, 500 })
        {
            Dx7Voice voice;
            voice.prepare (sr);
            voice.start (dx7InitPatch(), 60, 0.8);
            std::vector<float> l ((std::size_t) sr * 3), r ((std::size_t) sr * 3);
            for (int i = 0; i < sr * 3; i += blockLen)
            {
                if (i >= sr / 2 && i < sr / 2 + blockLen)
                    voice.scheduleRelease (sr / 2);
                voice.render (l.data() + i, r.data() + i, std::min (blockLen, sr * 3 - i));
            }
            check (! voice.isActive(), "voice freed after the release");
            outs.push_back (l);
        }
        check (outs[0] == outs[1] && outs[1] == outs[2], "identical renders");
    });

    std::printf (failures == 0 ? "all engine tests passed\n" : "%d failure(s)\n", failures);
    return failures == 0 ? 0 : 1;
}
