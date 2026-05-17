#include "Synth.h"

namespace
{
    inline float centsToRatio (float cents) noexcept
    {
        return std::pow (2.0f, cents / 1200.0f);
    }

    inline float softClip (float x, float drive) noexcept
    {
        const float g = 1.0f + drive * 12.0f;
        return std::tanh (x * g) / std::tanh (g) * 0.95f;
    }
}

float HarmoniaVoice::renderWave (Wave w, float phase) noexcept
{
    switch (w)
    {
        case Wave::Sine:
            return std::sin (phase * juce::MathConstants<float>::twoPi);
        case Wave::Triangle:
            return 4.0f * std::abs (phase - 0.5f) - 1.0f;
        case Wave::Saw:
            return 2.0f * phase - 1.0f;
        case Wave::Square:
            return phase < 0.5f ? -1.0f : 1.0f;
    }
    return 0.0f;
}

HarmoniaVoice::HarmoniaVoice (const HarmoniaParams::AtomicRefs& refs)
    : params (refs)
{
}

void HarmoniaVoice::prepare (double sampleRate, int samplesPerBlock, int numOutputChannels)
{
    currentSampleRate = sampleRate;

    juce::dsp::ProcessSpec spec;
    spec.sampleRate       = sampleRate;
    spec.maximumBlockSize = (juce::uint32) samplesPerBlock;
    spec.numChannels      = (juce::uint32) juce::jmax (1, numOutputChannels);

    filter.prepare (spec);
    filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);

    ampEnv.setSampleRate (sampleRate);
    filterEnv.setSampleRate (sampleRate);
}

bool HarmoniaVoice::canPlaySound (juce::SynthesiserSound* sound)
{
    return dynamic_cast<HarmoniaSound*> (sound) != nullptr;
}

void HarmoniaVoice::startNote (int midiNoteNumber, float velocity,
                               juce::SynthesiserSound*, int)
{
    baseFrequencyHz = (float) juce::MidiMessage::getMidiNoteInHertz (midiNoteNumber);
    noteVelocity    = juce::jlimit (0.0f, 1.0f, velocity);

    phase1 = 0.0f;
    phase2 = 0.0f;
    lfoPhase = 0.0f;

    ampEnv.reset();
    filterEnv.reset();
    ampEnv.noteOn();
    filterEnv.noteOn();
}

void HarmoniaVoice::stopNote (float, bool allowTailOff)
{
    if (allowTailOff)
    {
        ampEnv.noteOff();
        filterEnv.noteOff();
    }
    else
    {
        ampEnv.reset();
        filterEnv.reset();
        clearCurrentNote();
    }
}

void HarmoniaVoice::renderNextBlock (juce::AudioBuffer<float>& buffer,
                                     int startSample, int numSamples)
{
    if (! isVoiceActive())
        return;

    const Wave  w1     = (Wave) (int) params.osc1Waveform->load();
    const Wave  w2     = (Wave) (int) params.osc2Waveform->load();
    const float mix    = juce::jlimit (0.0f, 1.0f, params.oscMix->load());
    const float detune = params.osc2Detune->load();
    const float noise  = juce::jlimit (0.0f, 1.0f, params.noiseLevel->load());

    const float baseCutoff = juce::jlimit (20.0f, 20000.0f, params.filterCutoff->load());
    const float resonance  = juce::jlimit (0.0f, 0.95f, params.filterResonance->load());
    const int   ftype      = (int) params.filterType->load();

    ampParams.attack  = params.ampAttack->load()  * 0.001f; // ms -> s
    ampParams.decay   = params.ampDecay->load()   * 0.001f;
    ampParams.sustain = params.ampSustain->load();
    ampParams.release = params.ampRelease->load() * 0.001f;
    ampEnv.setParameters (ampParams);

    filterEnvParams.attack  = 0.001f;
    filterEnvParams.decay   = params.filterEnvDecay->load() * 0.001f;
    filterEnvParams.sustain = 0.0f;   // env filtre = AD only (cf. charte)
    filterEnvParams.release = 0.05f;
    filterEnv.setParameters (filterEnvParams);

    const float filterEnvAmount = params.filterEnvAmount->load();    // -1..1
    const float lfoRateHz       = juce::jlimit (0.1f, 20.0f, params.lfoRate->load());
    const float lfoToPitch      = params.lfoToPitch->load();
    const float lfoToCutoff     = params.lfoToCutoff->load();
    const float velToFilter     = params.velocityToFilter->load();
    const float distortion      = params.distortionMix->load();

    switch (ftype)
    {
        case 1:  filter.setType (juce::dsp::StateVariableTPTFilterType::bandpass); break;
        case 2:  filter.setType (juce::dsp::StateVariableTPTFilterType::highpass); break;
        default: filter.setType (juce::dsp::StateVariableTPTFilterType::lowpass);  break;
    }
    filter.setResonance (resonance);

    const float detuneRatio = centsToRatio (detune);
    const float lfoPhaseInc = lfoRateHz / (float) currentSampleRate;

    const int numChannels = buffer.getNumChannels();

    for (int i = 0; i < numSamples; ++i)
    {
        // --- LFO ---
        const float lfo = std::sin (lfoPhase * juce::MathConstants<float>::twoPi);
        lfoPhase += lfoPhaseInc;
        if (lfoPhase >= 1.0f) lfoPhase -= 1.0f;

        const float vibratoCents = lfo * lfoToPitch * 50.0f;
        const float pitchRatio   = centsToRatio (vibratoCents);
        const float f1 = baseFrequencyHz * pitchRatio;
        const float f2 = baseFrequencyHz * pitchRatio * detuneRatio;

        const float s1 = renderWave (w1, phase1);
        const float s2 = renderWave (w2, phase2);
        const float n  = noiseDist (rng);

        const float oscBlend = s1 * (1.0f - mix) + s2 * mix;
        float sample         = oscBlend * (1.0f - noise) + n * noise;

        phase1 += f1 / (float) currentSampleRate;
        phase2 += f2 / (float) currentSampleRate;
        if (phase1 >= 1.0f) phase1 -= 1.0f;
        if (phase2 >= 1.0f) phase2 -= 1.0f;

        const float fEnv = filterEnv.getNextSample();
        const float octavesFromEnv = filterEnvAmount * fEnv * 4.0f;     // ±4 oct max
        const float octavesFromLfo = lfo * lfoToCutoff * 2.0f;          // ±2 oct max
        const float octavesFromVel = velToFilter * (noteVelocity - 0.5f) * 4.0f;

        float modCutoff = baseCutoff * std::pow (2.0f, octavesFromEnv + octavesFromLfo + octavesFromVel);
        modCutoff = juce::jlimit (20.0f, 20000.0f, modCutoff);
        filter.setCutoffFrequency (modCutoff);

        sample = filter.processSample (0, sample);

        if (distortion > 0.0001f)
        {
            const float dirty = softClip (sample, distortion);
            sample = sample * (1.0f - distortion) + dirty * distortion;
        }

        const float env = ampEnv.getNextSample();
        const float out = sample * env * (0.4f + 0.6f * noteVelocity);

        for (int ch = 0; ch < numChannels; ++ch)
            buffer.addSample (ch, startSample + i, out);
    }

    if (! ampEnv.isActive())
        clearCurrentNote();
}
