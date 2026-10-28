#include "HarmoniaEngineV3.h"

#include <algorithm>
#include <cmath>

namespace harmonia
{
    namespace
    {
        using namespace HarmoniaCharter::analog;

        constexpr double pi = 3.14159265358979323846;
        constexpr double floorLevel = 1e-4;  // -80 dB: an exponential segment reaches its target within this
        constexpr double driftCents = 2.0;
        constexpr int driftEvery = 64;
        constexpr double pitchEnvShape = 5.0;
        constexpr double oscSplit = 0.5;  // at full width, osc 1 sits this far left of its voice and osc 2 right
        constexpr double chorusRate = 0.5, chorusCentreMs = 3.5, chorusDepthMs = 1.75;
        constexpr double delayDampHz = 6000.0;
        constexpr std::array<double, 8> fdnMs { 31.7, 37.3, 41.9, 47.3, 53.9, 59.1, 67.3, 73.1 };
        constexpr std::array<double, 8> fdnModHz { 0.11, 0.17, 0.23, 0.29, 0.13, 0.19, 0.31, 0.37 };
        constexpr double fdnModMs = 0.4;
        constexpr std::array<double, 4> diffuserMs { 4.7, 3.6, 12.7, 9.3 };
        constexpr double diffuserGain = 0.6;
        constexpr double reverbPredelayMs = 12.0;
        constexpr double reverbDampHz = 7000.0;
        constexpr double reverbWetGain = 2.4;  // median wet level of the v2 reverb at the same mix
        constexpr double maxReverbScale = 1.2;  // 0.6 + 0.6 x size at size 1
        constexpr int taps = 39;
        constexpr int historyMask = 63;

        double besselI0 (double x)
        {
            double sum = 1.0, term = 1.0;
            for (int k = 1; k < 64; ++k)
            {
                term *= (x / (2.0 * k)) * (x / (2.0 * k));
                sum += term;
                if (term < 1e-17 * sum)
                    break;
            }
            return sum;
        }

        /** Kaiser-windowed sinc low-pass for the 2x -> 1x decimation (flat to ~19 kHz at 48 kHz). */
        const std::array<double, taps>& halfBand()
        {
            static const std::array<double, taps> h = []
            {
                std::array<double, taps> out {};
                const double beta = 8.0, alpha = (taps - 1) / 2.0, cutoff = 2.0 * 21000.0 / 96000.0;
                double total = 0.0;
                for (int k = 0; k < taps; ++k)
                {
                    const double n = k - alpha;
                    const double x = cutoff * n;
                    const double sinc = n == 0.0 ? 1.0 : std::sin (pi * x) / (pi * x);
                    const double r = (k - alpha) / alpha;
                    out[(std::size_t) k] = sinc * besselI0 (beta * std::sqrt (std::max (0.0, 1.0 - r * r))) / besselI0 (beta);
                    total += out[(std::size_t) k];
                }
                for (auto& v : out)
                    v /= total;
                return out;
            }();
            return h;
        }

        double expCoef (double seconds, double rate) noexcept
        {
            return seconds > 0.0 ? std::exp (std::log (floorLevel) / (seconds * rate)) : 0.0;
        }

        double polyBlep (double t, double dt) noexcept
        {
            if (dt <= 0.0)
                return 0.0;
            if (t < dt)
            {
                t /= dt;
                return t + t - t * t - 1.0;
            }
            if (t > 1.0 - dt)
            {
                t = (t - 1.0) / dt;
                return t * t + t + t + 1.0;
            }
            return 0.0;
        }

        double oscillator (int wave, double phase, double dt, double pw) noexcept
        {
            if (wave == 0)
                return std::sin (2.0 * pi * phase);
            if (wave == 1)
                return 4.0 * std::abs (phase - 0.5) - 1.0;
            if (wave == 2)
                return 2.0 * phase - 1.0 - polyBlep (phase, dt);
            const double value = phase < pw ? -1.0 : 1.0;
            double shifted = phase - pw;
            if (shifted < 0.0)
                shifted += 1.0;
            return value - polyBlep (phase, dt) + polyBlep (shifted, dt);
        }

        double lfoValue (int wave, double phase, double held) noexcept
        {
            switch (wave)
            {
                case 0: return std::sin (2.0 * pi * phase);
                case 1: return 1.0 - 4.0 * std::abs (phase - 0.5);
                case 2: return 1.0 - 2.0 * phase;
                case 3: return phase < 0.5 ? 1.0 : -1.0;
                default: return held;
            }
        }

        /** juce::dsp::StateVariableTPTFilter step: low-pass, band-pass and high-pass outputs. */
        struct Svf
        {
            double lp, bp, hp;
        };

        Svf svfStep (double x, double g, double r2, std::array<double, 2>& s) noexcept
        {
            const double h = 1.0 / (1.0 + r2 * g + g * g);
            const double hp = h * (x - s[0] * (g + r2) - s[1]);
            const double bp = hp * g + s[0];
            s[0] = hp * g + bp;
            const double lp = bp * g + s[1];
            s[1] = bp * g + lp;
            return { lp, bp, hp };
        }

        double pick (const Svf& y, int type) noexcept
        {
            return type == 0 ? y.lp : (type == 1 ? y.bp : y.hp);
        }

        int asInt (double value) noexcept { return (int) std::lround (value); }
    }

    AnalogPatch defaultPatch()
    {
        AnalogPatch patch {};
        for (int i = 0; i < count; ++i)
            patch[(std::size_t) i] = HarmoniaCharter::params[HarmoniaCharter::analogBegin + (std::size_t) i].defaultValue;
        return patch;
    }

    double Random::uniform() noexcept
    {
        std::uint64_t z = (state += 0x9E3779B97F4A7C15ULL);
        z = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ULL;
        z = (z ^ (z >> 27)) * 0x94D049BB133111EBULL;
        z ^= z >> 31;
        return (double) (z >> 11) * (1.0 / 9007199254740992.0);
    }

    //==============================================================================================================
    void AnalogVoice::Envelope::step (double attackRate, double decayCoef, double sustainLevel, double releaseCoef) noexcept
    {
        switch (stage)
        {
            case attack:
                value += attackRate;
                if (value >= 1.0)
                {
                    value = 1.0;
                    stage = decay;
                }
                break;
            case decay:
                value = sustainLevel + (value - sustainLevel) * decayCoef;
                if (value - sustainLevel <= floorLevel)
                {
                    value = sustainLevel;
                    stage = sustain;
                }
                break;
            case sustain:
                value = sustainLevel;
                break;
            case releasing:
                value *= releaseCoef;
                if (value <= floorLevel)
                {
                    value = 0.0;
                    stage = idle;
                }
                break;
            case idle:
                break;
        }
    }

    void AnalogVoice::prepare (double sampleRate)
    {
        fast = sampleRate * oversample;
        active = false;
    }

    void AnalogVoice::start (const AnalogPatch& p, int midiNote, double noteVelocity, std::uint64_t seed) noexcept
    {
        random = Random (seed);
        note = midiNote;
        velocity = std::clamp (noteVelocity, 0.0, 1.0);
        baseFrequency = 440.0 * std::exp2 ((midiNote - 69) / 12.0);
        unison = std::clamp (asInt (p[unison_voices]), 1, maxUnison);
        for (int u = 0; u < unison; ++u)
        {
            const auto k = (std::size_t) u;
            phase1[k] = unison > 1 ? random.uniform() : 0.0;
            phase2[k] = unison > 1 ? random.uniform() : 0.0;
            for (std::size_t j = 0; j < 4; ++j)
            {
                driftRate[k][j] = random.uniform (0.07, 0.35);
                driftPhase[k][j] = random.uniform (0.0, 2.0 * pi);
            }
            drift[k] = { 0.0, 0.0 };
        }
        lfoPhase = 0.0;
        lfoHeld = random.uniform (-1.0, 1.0);
        for (auto& stage : filter)
            stage = { 0.0, 0.0 };
        for (auto& channel : history)
            channel.fill (0.0);
        historyPos = 0;
        fastIndex = 0;
        amp.start();
        filterEnv.start();
        releasePending = released = false;
        releaseAt = -1;
        active = true;
    }

    void AnalogVoice::updateDrift (double t) noexcept
    {
        for (int u = 0; u < unison; ++u)
        {
            const auto k = (std::size_t) u;
            for (std::size_t osc = 0; osc < 2; ++osc)
                drift[k][osc] = driftCents / 150.0 * (std::sin (2.0 * pi * driftRate[k][osc * 2] * t + driftPhase[k][osc * 2])
                                                       + 0.5 * std::sin (2.0 * pi * driftRate[k][osc * 2 + 1] * t + driftPhase[k][osc * 2 + 1]));
        }
    }

    void AnalogVoice::updateTuning (const AnalogPatch& p) noexcept
    {
        for (int u = 0; u < unison; ++u)
        {
            const auto k = (std::size_t) u;
            const double pos = unison > 1 ? 2.0 * u / (unison - 1) - 1.0 : 0.0;
            unisonOffset[k] = pos * p[unison_detune] / 100.0;
            tune1[k] = std::exp2 ((p[osc_1_coarse] + unisonOffset[k] + drift[k][0]) / 12.0);
            tune2[k] = std::exp2 ((p[osc_2_coarse] + unisonOffset[k] + drift[k][1]) / 12.0 + p[osc_2_detune] / 1200.0);
        }
    }

    void AnalogVoice::render (const AnalogPatch& p, float* left, float* right, int numSamples) noexcept
    {
        if (! active)
            return;

        // block settings (the reference reads them once per note; the plugin follows automation per block)
        const int w1 = asInt (p[osc_1_waveform]), w2 = asInt (p[osc_2_waveform]);
        const double mix = p[osc_mix], noise = p[noise_level];
        const double baseCutoff = p[filter_cutoff];
        const double q = 0.707 * std::pow (17.0, std::clamp (p[filter_resonance], 0.0, 0.95) / 0.95);
        const double r2 = 1.0 / q, r2Flat = 1.0 / 0.7071067811865476;
        const int filterType = asInt (p[filter_type]);
        const bool slope24 = p[filter_slope] > 0.5;
        const double aRate = 1.0 / (p[amp_attack] * 1e-3 * fast), aDecay = expCoef (p[amp_decay] * 1e-3, fast);
        const double aRelease = expCoef (p[amp_release] * 1e-3, fast), aSustain = p[amp_sustain];
        const double fRate = 1.0 / (p[filter_env_attack] * 1e-3 * fast), fDecay = expCoef (p[filter_env_decay] * 1e-3, fast);
        const double fRelease = expCoef (p[filter_env_release] * 1e-3, fast), fSustain = p[filter_env_sustain];
        const double fAmount = p[filter_env_amount];
        const double lfoRate = p[lfo_rate], lfoPitchCents = p[lfo_to_pitch], lfoCutoffOct = p[lfo_to_cutoff];
        const int lfoWave = asInt (p[lfo_waveform]);
        const double lfoDelay = p[lfo_delay] * 1e-3, lfoAmp = p[lfo_to_amp], lfoPw = p[lfo_to_pw];
        const double pw0 = p[pulse_width], fm = p[fm_amount], ring = p[ring_mod];
        const bool sync = p[osc_sync] > 0.5;
        const double penvAmount = p[pitch_env_amount], penvDecay = p[pitch_env_decay] * 1e-3;
        const double velAmp = p[velocity_to_amp];
        const double gain = (1.0 - velAmp) + velAmp * velocity;
        const double keyOct = p[filter_keytrack] * (note - 60) / 12.0 + p[velocity_to_filter] * (velocity - 0.5) * 4.0;
        const double distortion = p[distortion_mix];
        const double width = std::clamp (p[stereo_width], 0.0, 1.0);
        const double uniNorm = 1.0 / std::sqrt ((double) unison);

        for (int u = 0; u < unison; ++u)
        {
            const double pos = unison > 1 ? 2.0 * u / (unison - 1) - 1.0 : 0.0;
            for (std::size_t k = 0; k < 4; k += 2)
            {
                const double split = k == 0 ? -oscSplit : oscSplit;
                const double pan = std::clamp ((pos + split) * width, -1.0, 1.0);
                const double angle = (pan + 1.0) * pi / 4.0;
                gains[(std::size_t) u][k] = std::cos (angle) * std::sqrt (2.0);
                gains[(std::size_t) u][k + 1] = std::sin (angle) * std::sqrt (2.0);
            }
        }
        updateTuning (p);
        const auto& h = halfBand();

        for (int i = 0; i < numSamples && active; ++i)
        {
            for (int f = 0; f < oversample; ++f)
            {
                if (releaseAt >= 0 && fastIndex >= releaseAt)
                    releasePending = true;
                if (releasePending && ! released)
                {
                    released = true;
                    if (amp.stage != idle)
                        amp.stage = releasing;
                    if (filterEnv.stage != idle)
                        filterEnv.stage = releasing;
                }
                double outL = 0.0, outR = 0.0;
                if (released && amp.stage == idle)
                {
                    active = false;  // the remaining samples of this output sample stay silent
                }
                else
                {
                    const double t = (double) fastIndex / fast;

                    double lfo = lfoValue (lfoWave, lfoPhase, lfoHeld);
                    if (lfoDelay > 0.0 && t < lfoDelay)
                        lfo *= t / lfoDelay;
                    lfoPhase += lfoRate / fast;
                    if (lfoPhase >= 1.0)
                    {
                        lfoPhase -= 1.0;
                        lfoHeld = random.uniform (-1.0, 1.0);
                    }

                    double semis = lfo * lfoPitchCents / 100.0;
                    if (penvAmount != 0.0 && penvDecay > 0.0)
                        semis += penvAmount * std::exp (-pitchEnvShape * t / penvDecay);
                    if (fastIndex % driftEvery == 0)
                    {
                        updateDrift (t);
                        updateTuning (p);
                    }
                    const double common = baseFrequency * std::exp2 (semis / 12.0) / fast;
                    const double pw = std::clamp (pw0 + lfo * lfoPw * 0.45, 0.05, 0.95);

                    double l = 0.0, r = 0.0;
                    for (int u = 0; u < unison; ++u)
                    {
                        const auto k = (std::size_t) u;
                        const double dt1 = common * tune1[k];
                        const double o1 = oscillator (w1, phase1[k], dt1, pw);
                        // cross-modulation as on the OB-X: osc 1 bends osc 2's pitch (fm semitones per unit of osc 1)
                        double dt2 = common * tune2[k] * (fm != 0.0 ? std::exp2 (fm * o1 / 12.0) : 1.0);
                        dt2 = std::min (dt2, 0.45);
                        const double o2 = oscillator (w2, phase2[k], dt2, pw);
                        double a1 = o1 * (1.0 - mix), a2 = o2 * mix;
                        if (ring > 0.0)
                        {
                            a1 *= 1.0 - ring;
                            a2 *= 1.0 - ring;
                            const double centre = o1 * o2 * ring;
                            l += centre;
                            r += centre;
                        }
                        l += a1 * gains[k][0] + a2 * gains[k][2];
                        r += a1 * gains[k][1] + a2 * gains[k][3];
                        phase1[k] += dt1;
                        const bool wrapped = phase1[k] >= 1.0;
                        if (wrapped)
                            phase1[k] -= 1.0;
                        if (sync && wrapped)
                        {
                            phase2[k] = dt1 > 0.0 ? phase1[k] / dt1 * dt2 : 0.0;
                        }
                        else
                        {
                            phase2[k] += dt2;
                            if (phase2[k] >= 1.0)
                                phase2[k] -= std::floor (phase2[k]);
                        }
                    }
                    l *= uniNorm;
                    r *= uniNorm;
                    if (noise > 0.0)
                    {
                        const double n = random.uniform (-1.0, 1.0);
                        l = l * (1.0 - noise) + n * noise;
                        r = r * (1.0 - noise) + n * noise;
                    }

                    filterEnv.step (fRate, fDecay, fSustain, fRelease);
                    const double octaves = fAmount * filterEnv.value + lfo * lfoCutoffOct + keyOct;
                    const double cutoff = std::clamp (baseCutoff * std::exp2 (octaves), 20.0, 20000.0);
                    const double g = std::tan (pi * std::min (cutoff, 0.49 * fast) / fast);

                    amp.step (aRate, aDecay, aSustain, aRelease);
                    double level = amp.value * gain;
                    if (lfoAmp > 0.0)
                        level *= 1.0 - lfoAmp * 0.5 * (1.0 - lfo);

                    for (std::size_t ch = 0; ch < 2; ++ch)
                    {
                        double y = pick (svfStep (ch == 0 ? l : r, g, r2, filter[ch]), filterType);
                        if (slope24)
                            y = pick (svfStep (y, g, r2Flat, filter[ch + 2]), filterType);
                        if (distortion > 0.0001)
                        {
                            const double drive = 1.0 + distortion * 12.0;
                            y = y * (1.0 - distortion) + std::tanh (y * drive) / std::tanh (drive) * 0.95 * distortion;
                        }
                        (ch == 0 ? outL : outR) = y * level;
                    }
                    ++fastIndex;
                }

                historyPos = (historyPos + 1) & historyMask;
                history[0][(std::size_t) historyPos] = outL;
                history[1][(std::size_t) historyPos] = outR;
            }

            double accL = 0.0, accR = 0.0;
            for (int k = 0; k < taps; ++k)
            {
                const auto at = (std::size_t) ((historyPos - k) & historyMask);
                accL += h[(std::size_t) k] * history[0][at];
                accR += h[(std::size_t) k] * history[1][at];
            }
            left[i] += (float) accL;
            right[i] += (float) accR;
        }
    }

    //==============================================================================================================
    void Effects::prepare (double sampleRate)
    {
        rate = sampleRate;
        for (auto& b : chorus)
            b.assign ((std::size_t) std::max (2, (int) (0.02 * rate)), 0.0);
        for (auto& b : echo)
            b.assign ((std::size_t) (2.0 * rate) + 2, 0.0);
        const int lineSize = (int) (fdnMs.back() * 1e-3 * rate * maxReverbScale + fdnModMs * 1e-3 * rate) + 4;
        for (auto& b : lines)
            b.assign ((std::size_t) lineSize, 0.0);
        for (std::size_t d = 0; d < diffusers.size(); ++d)
            diffusers[d].assign ((std::size_t) std::max (1, (int) (diffuserMs[d] * 1e-3 * rate)), 0.0);
        predelay.assign ((std::size_t) (reverbPredelayMs * 1e-3 * rate) + 1, 0.0);
        reset();
    }

    void Effects::reset() noexcept
    {
        index = 0;
        for (auto* group : { &chorus, &echo })
            for (auto& b : *group)
                std::fill (b.begin(), b.end(), 0.0);
        for (auto& b : lines)
            std::fill (b.begin(), b.end(), 0.0);
        for (auto& b : diffusers)
            std::fill (b.begin(), b.end(), 0.0);
        std::fill (predelay.begin(), predelay.end(), 0.0);
        echoLowpass = {};
        lineLowpass = {};
        mods = {};
        diffuserPos = {};
    }

    double Effects::readDelay (const std::vector<double>& buffer, int position, double delaySamples) noexcept
    {
        const int size = (int) buffer.size();
        double d = position - delaySamples;
        while (d < 0.0)
            d += size;
        const int i0 = (int) d;
        const double frac = d - i0;
        const int i1 = (i0 + 1) % size;
        return buffer[(std::size_t) i0] * (1.0 - frac) + buffer[(std::size_t) i1] * frac;
    }

    void Effects::process (const AnalogPatch& p, float* left, float* right, int numSamples) noexcept
    {
        const double chorusMix = p[chorus_mix];
        const double echoSamples = std::max (1.0, p[delay_time] * 1e-3 * rate);
        const double echoFeedback = p[delay_feedback], echoMix = p[delay_mix];
        const double echoDamp = std::exp (-2.0 * pi * delayDampHz / rate);
        const double wet = p[reverb_mix], size = p[reverb_size];
        const double dryGain = (1.0 - wet * 0.7) * 2.0;  // v2 dry law (juce::Reverb's dry level x 2)

        std::array<double, 8> lengths {}, fb {};
        double inGain = 0.0;
        const double damp = std::exp (-2.0 * pi * reverbDampHz / rate);
        if (wet > 0.0)
        {
            const double t60 = 0.3 + 11.7 * size * size;
            const double scale = 0.6 + 0.6 * size;
            double meanFb = 0.0;
            for (std::size_t j = 0; j < 8; ++j)
            {
                lengths[j] = fdnMs[j] * 1e-3 * rate * scale;
                fb[j] = std::pow (10.0, -3.0 * lengths[j] / (t60 * rate));
                meanFb += fb[j] / 8.0;
            }
            inGain = std::sqrt (1.0 - meanFb * meanFb);  // the impulse response carries the same energy at any size
        }

        const int chorusSize = (int) chorus[0].size();
        const int echoSize = (int) echo[0].size();
        const int lineSize = (int) lines[0].size();
        const int preSize = (int) predelay.size();

        for (int i = 0; i < numSamples; ++i, ++index)
        {
            std::array<double, 2> x { left[i], right[i] };

            const double lfo = std::sin (2.0 * pi * chorusRate * (double) index / rate);
            const int cPos = (int) (index % chorusSize);
            for (std::size_t ch = 0; ch < 2; ++ch)
            {
                chorus[ch][(std::size_t) cPos] = x[ch];
                if (chorusMix > 0.0)
                {
                    const double ms = chorusCentreMs + chorusDepthMs * (ch == 0 ? lfo : -lfo);
                    const double w = readDelay (chorus[ch], cPos, ms * 1e-3 * rate);
                    x[ch] = x[ch] * (1.0 - 0.5 * chorusMix) + w * 0.5 * chorusMix;
                }
            }

            if (echoMix > 0.0)
            {
                const int ePos = (int) (index % echoSize);
                for (std::size_t ch = 0; ch < 2; ++ch)
                {
                    const double w = readDelay (echo[ch], ePos, echoSamples);
                    echoLowpass[ch] = w * (1.0 - echoDamp) + echoLowpass[ch] * echoDamp;
                    echo[ch][(std::size_t) ePos] = x[ch] + echoLowpass[ch] * echoFeedback;
                    x[ch] += w * echoMix;
                }
            }

            if (wet <= 0.0)
            {
                left[i] = (float) (x[0] * dryGain);
                right[i] = (float) (x[1] * dryGain);
                continue;
            }

            const double in = 0.5 * (x[0] + x[1]) * inGain;
            const auto prePos = (std::size_t) (index % preSize);
            double delayed = predelay[prePos];
            predelay[prePos] = in;
            for (std::size_t d = 0; d < diffusers.size(); ++d)  // Schroeder all-pass diffusers
            {
                auto& buffer = diffusers[d];
                const auto k = (std::size_t) diffuserPos[d];
                const double held = buffer[k];
                const double v = delayed + diffuserGain * held;
                buffer[k] = v;
                delayed = held - diffuserGain * v;
                diffuserPos[d] = (int) ((k + 1) % buffer.size());
            }
            if (index % 16 == 0)  // slow modulation: control rate is enough
                for (std::size_t j = 0; j < 8; ++j)
                    mods[j] = fdnModMs * 1e-3 * rate * std::sin (2.0 * pi * fdnModHz[j] * (double) index / rate);

            const int lPos = (int) (index % lineSize);
            std::array<double, 8> tap {};
            double total = 0.0;
            for (std::size_t j = 0; j < 8; ++j)
            {
                const double y = readDelay (lines[j], lPos, lengths[j] + mods[j]);
                lineLowpass[j] = y * (1.0 - damp) + lineLowpass[j] * damp;
                tap[j] = lineLowpass[j] * fb[j];
                total += tap[j];
            }
            total *= 2.0 / 8.0;
            for (std::size_t j = 0; j < 8; ++j)  // Householder feedback matrix: I - 2/N
                lines[j][(std::size_t) lPos] = tap[j] - total + delayed;
            const double l = tap[0] - tap[1] + tap[2] - tap[3] + tap[4] - tap[5] + tap[6] - tap[7];
            const double r = tap[0] + tap[1] - tap[2] - tap[3] + tap[4] + tap[5] - tap[6] - tap[7];
            left[i] = (float) (x[0] * dryGain + l * wet * reverbWetGain / 2.0);
            right[i] = (float) (x[1] * dryGain + r * wet * reverbWetGain / 2.0);
        }
    }

    //==============================================================================================================
    void renderNote (const AnalogPatch& patch, int midiNote, double velocity, double holdSeconds, double totalSeconds,
                     int sampleRate, std::uint64_t seed, float* left, float* right)
    {
        const int n = (int) (totalSeconds * sampleRate);
        const int padded = n + AnalogVoice::latency;
        std::vector<float> l ((std::size_t) padded, 0.0f), r ((std::size_t) padded, 0.0f);
        AnalogVoice voice;
        voice.prepare (sampleRate);
        voice.start (patch, midiNote, velocity, seed);
        voice.scheduleRelease ((std::int64_t) (holdSeconds * sampleRate * AnalogVoice::oversample));
        voice.render (patch, l.data(), r.data(), padded);
        std::copy (l.begin() + AnalogVoice::latency, l.end(), left);
        std::copy (r.begin() + AnalogVoice::latency, r.end(), right);
        Effects effects;
        effects.prepare (sampleRate);
        effects.process (patch, left, right, n);
    }
}

extern "C" int harmonia_render_analog_note (const double* physical, int count, int midiNote, double velocity,
                                            double holdSeconds, double totalSeconds, int sampleRate,
                                            unsigned long long seed, float* out, int capacity)
{
    const int n = (int) (totalSeconds * sampleRate);
    if (physical == nullptr || out == nullptr || count != HarmoniaCharter::analog::count || sampleRate <= 0
        || n <= 0 || capacity < 2 * n)
        return -1;
    harmonia::AnalogPatch patch {};
    std::copy (physical, physical + count, patch.begin());
    harmonia::renderNote (patch, midiNote, velocity, holdSeconds, totalSeconds, sampleRate, seed, out, out + n);
    return n;
}
