#include "HarmoniaDx7.h"

#include <algorithm>
#include <cmath>
#include <cstring>
#include <new>
#include <vector>

// msfa is Dexed's code, kept unchanged: its warnings are not ours to fix
#if defined (__clang__)
 #pragma clang diagnostic push
 #pragma clang diagnostic ignored "-Wconversion"
 #pragma clang diagnostic ignored "-Wsign-conversion"
 #pragma clang diagnostic ignored "-Wshadow"
 #pragma clang diagnostic ignored "-Wunused-parameter"
#elif defined (__GNUC__)
 #pragma GCC diagnostic push
 #pragma GCC diagnostic ignored "-Wconversion"
 #pragma GCC diagnostic ignored "-Wsign-conversion"
 #pragma GCC diagnostic ignored "-Wshadow"
 #pragma GCC diagnostic ignored "-Wunused-parameter"
#elif defined (_MSC_VER)
 #pragma warning (push, 0)
#endif
#include "dx7/msfa/aligned_buf.h"
#include "dx7/msfa/controllers.h"
#include "dx7/msfa/dx7note.h"
#include "dx7/msfa/env.h"
#include "dx7/msfa/exp2.h"
#include "dx7/msfa/fm_core.h"
#include "dx7/msfa/freqlut.h"
#include "dx7/msfa/lfo.h"
#include "dx7/msfa/pitchenv.h"
#include "dx7/msfa/porta.h"
#include "dx7/msfa/sin.h"
#if defined (__clang__)
 #pragma clang diagnostic pop
#elif defined (__GNUC__)
 #pragma GCC diagnostic pop
#elif defined (_MSC_VER)
 #pragma warning (pop)
#endif

namespace
{
    // 12-TET: (1 << 24) * log2(frequency), as in Dexed's standard tuning.
    struct StandardTuning : public TuningState
    {
        int32_t midinote_to_logfreq (int midinote) override { return 50857777 + ((1 << 24) / 12) * midinote; }
    };

    constexpr double dxVelocityScale = 0.7874015;  // Dexed's "DX7 velocity": MIDI range -> DX7 keyboard range
    constexpr int bytesPerOperator = 21;
    constexpr int voiceFieldsAt = 126;

    void initTables (double sampleRate)
    {
        static bool tables = false;
        static double tablesRate = 0.0;
        if (! tables)
        {
            Exp2::init();
            Tanh::init();
            Sin::init();
            tables = true;
        }
        if (sampleRate != tablesRate)
        {
            Freqlut::init (sampleRate);
            Lfo::init (sampleRate);
            PitchEnv::init (sampleRate);
            Env::init_sr (sampleRate);
            Porta::init_sr (sampleRate);
            tablesRate = sampleRate;
        }
    }
}

std::shared_ptr<TuningState> createStandardTuning() { return std::make_shared<StandardTuning>(); }

namespace harmonia
{
    Dx7Patch dx7PatchFromValues (const float* values) noexcept
    {
        Dx7Patch patch {};
        for (std::size_t k = 0; k < dx7ParameterCount; ++k)
        {
            const auto& spec = HarmoniaCharter::params[HarmoniaCharter::dx7Begin + k];
            const auto value = (std::uint8_t) std::lround (HarmoniaCharter::snap (spec, values[k]));
            const int i = (int) k;
            const int at = i < 6 * bytesPerOperator
                         ? (5 - i / bytesPerOperator) * bytesPerOperator + i % bytesPerOperator  // OP6 first
                         : voiceFieldsAt + (i - 6 * bytesPerOperator);
            patch[(std::size_t) at] = value;
        }
        const char name[] = "HARMONIA  ";
        std::memcpy (patch.data() + 145, name, 10);
        patch[155] = 0;
        return patch;
    }

    Dx7Patch dx7InitPatch() noexcept
    {
        std::array<float, dx7ParameterCount> values {};
        for (std::size_t k = 0; k < values.size(); ++k)
            values[k] = HarmoniaCharter::params[HarmoniaCharter::dx7Begin + k].defaultValue;
        return dx7PatchFromValues (values.data());
    }

    int dx7Velocity (int midiVelocity) noexcept
    {
        return (int) (std::clamp (midiVelocity, 0, 127) * dxVelocityScale);
    }

    //==============================================================================================================
    struct Dx7Voice::Impl
    {
        std::shared_ptr<TuningState> tuning = createStandardTuning();
        FmCore core;
        Controllers controllers;
        Lfo lfo {};
        alignas (Dx7Note) unsigned char storage[sizeof (Dx7Note)];
        Dx7Note* note = nullptr;
        Dx7Patch patch {};

        Impl()
        {
            controllers.values_[kControllerPitch] = 0x2000;
            controllers.values_[kControllerPitchRangeUp] = 3;
            controllers.values_[kControllerPitchRangeDn] = 3;
            controllers.values_[kControllerPitchStep] = 0;
            controllers.masterTune = 0;
            controllers.modwheel_cc = 0;
            controllers.foot_cc = 0;
            controllers.breath_cc = 0;
            controllers.aftertouch_cc = 0;
            controllers.portamento_enable_cc = false;
            controllers.portamento_cc = 0;
            controllers.core = &core;
            controllers.refresh();
        }

        ~Impl() { destroyNote(); }

        void destroyNote() noexcept
        {
            if (note != nullptr)
                note->~Dx7Note();
            note = nullptr;
        }

        void begin (const Dx7Patch& p, int midinote, int velocity) noexcept
        {
            patch = p;
            destroyNote();
            lfo = Lfo {};  // the sample-and-hold state starts at 0, as in the reference renderer
            lfo.reset (patch.data() + 137);
            lfo.keydown();
            // Dx7Note's constructor leaves the operator feedback buffer uninitialized: zeroed storage keeps
            // every note independent of the previous one (and identical to the reference render)
            std::memset (storage, 0, sizeof (storage));
            note = new (storage) Dx7Note (tuning, nullptr);
            note->init (patch.data(), midinote, velocity, 1, &controllers);
            if (patch[136] != 0)
                note->oscSync();
        }
    };

    Dx7Voice::Dx7Voice() : impl (std::make_unique<Impl>()) {}
    Dx7Voice::~Dx7Voice() = default;

    void Dx7Voice::prepare (double sampleRate)
    {
        rate = sampleRate;
        initTables (sampleRate);
        active = false;
    }

    void Dx7Voice::start (const Dx7Patch& patch, int midiNote, double velocity) noexcept
    {
        startRaw (patch, midiNote, dx7Velocity ((int) std::lround (std::clamp (velocity, 0.0, 1.0) * 127.0)));
    }

    void Dx7Voice::startRaw (const Dx7Patch& patch, int midiNote, int dxVelocity) noexcept
    {
        impl->begin (patch, midiNote, dxVelocity);
        active = true;
        releasePending = keyIsUp = lastBlock = false;
        releaseAt = -1;
        sampleIndex = 0;
        blockPos = blockSize;
        dcIn = dcOut = 0.0f;
    }

    void Dx7Voice::computeBlock() noexcept
    {
        if (! keyIsUp && (releasePending || (releaseAt >= 0 && sampleIndex >= releaseAt)))
        {
            impl->note->keyup();
            keyIsUp = true;
        }
        AlignedBuf<int32_t, N> buf;
        std::fill (buf.get(), buf.get() + N, 0);
        const int32_t lfoValue = impl->lfo.getsample();
        const int32_t lfoDelay = impl->lfo.getdelay();
        impl->note->compute (buf.get(), lfoValue, lfoDelay, &impl->controllers);

        const float dcR = 1.0f - (126.0f / (float) rate);
        float peak = 0.0f;
        for (int j = 0; j < blockSize; ++j)
        {
            const int32_t val = buf.get()[j] >> 4;
            const int clip = val < -(1 << 24) ? 0x8000 : (val >= (1 << 24) ? 0x7fff : val >> 9);
            const float f = std::max (-1.0f, std::min (1.0f, (float) clip / (float) 0x8000));
            const float filtered = f - dcIn + dcR * dcOut;  // Dexed's DC filter (PluginFx)
            dcIn = f;
            dcOut = filtered;
            block[(std::size_t) j] = filtered;
            peak = std::max (peak, std::abs (filtered));
        }
        blockPos = 0;
        // the note is over once its operators are silent and the DC filter's tail has died away (no click)
        lastBlock = keyIsUp && ! impl->note->isPlaying() && peak < 1e-6f;
    }

    void Dx7Voice::render (float* left, float* right, int numSamples) noexcept
    {
        for (int i = 0; i < numSamples && active; ++i)
        {
            if (blockPos == blockSize)
            {
                if (lastBlock)
                {
                    active = false;
                    break;
                }
                computeBlock();
            }
            const float s = block[(std::size_t) blockPos++];
            left[i] += s;
            right[i] += s;
            ++sampleIndex;
        }
    }
}

extern "C" int harmonia_render_dx7_note (const unsigned char* patch156, int midiNote, int dxVelocity, int holdSamples,
                                         int totalSamples, int sampleRate, float* out)
{
    if (patch156 == nullptr || out == nullptr || totalSamples <= 0 || sampleRate <= 0)
        return -1;
    harmonia::Dx7Patch patch {};
    std::memcpy (patch.data(), patch156, patch.size());
    harmonia::Dx7Voice voice;
    voice.prepare (sampleRate);
    voice.startRaw (patch, midiNote, dxVelocity);
    voice.scheduleRelease (holdSamples);
    std::vector<float> right ((std::size_t) totalSamples, 0.0f);
    std::fill (out, out + totalSamples, 0.0f);
    voice.render (out, right.data(), totalSamples);
    return 0;
}

extern "C" int harmonia_dx7_patch_from_values (const float* values, int count, unsigned char* patch156)
{
    if (values == nullptr || patch156 == nullptr || count != (int) harmonia::dx7ParameterCount)
        return -1;
    const auto patch = harmonia::dx7PatchFromValues (values);
    std::memcpy (patch156, patch.data(), patch.size());
    return 0;
}
