/**
 * @file HarmoniaSamples.h
 * @brief Recorded instruments played by the sample source of the analog engine (charter 3.1), plain C++17.
 *
 * Port of HARMONIA/src/samples/player.py (Harmonia-AI): which recording answers a note and a velocity.
 * The recordings come from the sample packs (Salamander Grand Piano, VSCO 2, VCSL), decoded by
 * SampleLibrary (JUCE side) into 16-bit planar memory; the engine only reads them.
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

namespace harmonia
{
    /** One recording, 16-bit, planar (right is empty for a mono recording). */
    struct Recording
    {
        std::vector<std::int16_t> left, right;
        double sampleRate = 48000.0;

        std::int64_t frames() const noexcept { return (std::int64_t) left.size(); }
        std::size_t bytes() const noexcept { return (left.size() + right.size()) * sizeof (std::int16_t); }
    };

    /** Which keys and velocities a recording answers (catalog.json of the packs). */
    struct SampleRegion
    {
        int key = 60, lokey = 0, hikey = 127, lovel = 1, hivel = 127, roundRobin = 1;
        double tuneCents = 0.0, volumeDb = 0.0;
        std::string file;            ///< path in the pack, sorts the round robins like the reference
        std::size_t recording = 0;   ///< index in SampledInstrument::recordings
    };

    /** One instrument of the catalogue, its attack regions and their recordings. */
    struct SampledInstrument
    {
        std::string id;
        double gain = 1.0;  ///< brings the instrument to the oscillators' level (build_packs.py)
        std::vector<SampleRegion> regions;
        std::vector<Recording> recordings;

        /** The region for `note` at `velocity` (1..127); outside the recorded range, the nearest recorded note;
            `take` chooses among round robins. Same rule as SampleBank.region in the reference. */
        const SampleRegion* region (int note, int velocity, std::uint64_t take) const noexcept;

        std::size_t bytes() const noexcept;
    };

    /** What a voice plays: the instrument stays alive as long as the note does. */
    struct SampleNote
    {
        std::shared_ptr<const SampledInstrument> instrument;
        const Recording* recording = nullptr;
        double rootKey = 60.0;  ///< key of the recording, corrected by its tuning
        double gain = 0.0;

        bool playable() const noexcept { return recording != nullptr && recording->frames() > 1; }

        /** The note for `instrument` at this key and velocity, or an empty one. */
        static SampleNote make (std::shared_ptr<const SampledInstrument> instrument, int note, int velocity,
                                std::uint64_t take);
    };
}
