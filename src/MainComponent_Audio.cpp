#include "MainComponent.h"

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