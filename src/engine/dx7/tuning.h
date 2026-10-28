// Harmonia's stand-in for Dexed's tuning.h: the tuning interface msfa's Dx7Note calls, standard tuning only.
// Dexed's own tuning.h / tuning.cc (no license header, microtuning through the Surge library) are not used.
#ifndef __SYNTH_TUNING_H
#define __SYNTH_TUNING_H

#include <cstdint>
#include <memory>

class TuningState
{
public:
    virtual ~TuningState() = default;
    virtual int32_t midinote_to_logfreq (int midinote) = 0;
    virtual bool is_standard_tuning() { return true; }
};

std::shared_ptr<TuningState> createStandardTuning();

#endif
