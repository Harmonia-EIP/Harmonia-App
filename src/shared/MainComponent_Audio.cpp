#include "MainComponent.h"

void MainComponent::initSynth()
{
    synth.clearVoices();
    for (int i = 0; i < 8; ++i)
        synth.addVoice (new HarmoniaVoice());

    synth.clearSounds();
    synth.addSound (new HarmoniaSound());
}

void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    currentSampleRate = sampleRate;
    synth.setCurrentPlaybackSampleRate (sampleRate);

    for (int i = 0; i < synth.getNumVoices(); ++i)
        if (auto* v = dynamic_cast<HarmoniaVoice*> (synth.getVoice (i)))
            v->prepare (sampleRate, samplesPerBlockExpected, 2);
}

void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    bufferToFill.clearActiveBufferRegion();

    juce::MidiBuffer midi;
    synthComponent.getKeyboardState().processNextMidiBuffer (
        midi,
        bufferToFill.startSample,
        bufferToFill.numSamples,
        true
    );

    synth.renderNextBlock (*bufferToFill.buffer, midi,
                           bufferToFill.startSample, bufferToFill.numSamples);

    oscilloscope.pushBuffer (*bufferToFill.buffer,
                             bufferToFill.startSample,
                             bufferToFill.numSamples);
}

void MainComponent::releaseResources() {}

void MainComponent::applyParamsToUI(const PatchParams& p)
{
    freqVolComponent.setFreqVol(p.frequency, p.volume);
    adsrComponent.setADSR(p.attack, p.decay, p.sustain, p.release);
    filterComponent.setCutoffResonance(p.cutoff, p.resonance);

    topBar.setFilterType(PatchSerializer::filterToString(p.filterType));
    topBar.setWaveform(PatchSerializer::waveformToString(p.waveform));
    topBar.setPrompt(p.prompt);

    updateSynthParamsFromUI();
}