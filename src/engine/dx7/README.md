# DX7 engine sources

`msfa/` is the FM core of Dexed (`Source/msfa` at commit `2e182b3db85c09083ab13c8b9b00565ce7d9ff85` of
https://github.com/asb2m10/dexed), copied without changes. Every file in it carries an Apache License 2.0
header (Copyright Google Inc., Pascal Gauthier, Jean Pierre Cimalando); the license text is `msfa/LICENSE`.
Dexed's "Mark I" and OPL engines (GPL-3) and its `tuning.h` / `tuning.cc` (no license header) are not
included.

The files next to `msfa/` are Harmonia's own stand-ins for the few Dexed headers msfa includes:

| File | Replaces |
|---|---|
| `Dexed.h` | Dexed's tracing macros (disabled) |
| `tuning.h` | the tuning interface `Dx7Note` calls (standard tuning only) |
| `Tunings.h`, `libMTSClient.h` | the Surge tuning library and the MTS-ESP client (no microtuning) |

`../HarmoniaDx7.h` drives one `Dx7Note` per voice the way the AI's reference renderer does
(`HARMONIA/native/dx7/dx7_render.cpp` in Harmonia-AI): 64-sample blocks, LFO, key-up on a block boundary,
Dexed's DC filter, DX7 keyboard velocity. The AI's comparison bench (`HARMONIA/scripts/v3/engine_parity.py`)
checks that both render the same samples.
