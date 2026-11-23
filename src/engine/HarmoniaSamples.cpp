#include "HarmoniaSamples.h"

#include <algorithm>
#include <climits>
#include <cmath>
#include <cstdlib>

namespace harmonia
{
    const SampleRegion* SampledInstrument::region (int note, int velocity, std::uint64_t take) const noexcept
    {
        if (regions.empty())
            return nullptr;
        velocity = std::clamp (velocity, 1, 127);

        // the recorded note answering this key, else the nearest one (the lower on a tie)
        int key = INT_MIN;
        bool inside = false;
        for (const auto& r : regions)
            if (r.lokey <= note && note <= r.hikey)
                inside = true;
        if (! inside)
        {
            int best = INT_MAX;
            for (const auto& r : regions)
            {
                const int d = std::abs (r.key - note);
                if (d < best || (d == best && r.key < key))
                {
                    best = d;
                    key = r.key;
                }
            }
        }
        auto answers = [&] (const SampleRegion& r) { return inside ? (r.lokey <= note && note <= r.hikey) : r.key == key; };

        // the velocity layer; failing that, the region whose range comes closest
        const SampleRegion* layer[64];
        int count = 0;
        const SampleRegion* closest = nullptr;
        int closestDistance = INT_MAX;
        for (const auto& r : regions)
        {
            if (! answers (r))
                continue;
            if (r.lovel <= velocity && velocity <= r.hivel && count < 64)
                layer[count++] = &r;
            const int d = std::min (std::abs (r.lovel - velocity), std::abs (r.hivel - velocity));
            if (d < closestDistance)
            {
                closestDistance = d;
                closest = &r;
            }
        }
        if (count == 0)
            return closest;
        std::sort (layer, layer + count, [] (const SampleRegion* a, const SampleRegion* b)
        {
            return a->roundRobin != b->roundRobin ? a->roundRobin < b->roundRobin : a->file < b->file;
        });
        return layer[take % (std::uint64_t) count];
    }

    std::size_t SampledInstrument::bytes() const noexcept
    {
        std::size_t total = 0;
        for (const auto& r : recordings)
            total += r.bytes();
        return total;
    }

    SampleNote SampleNote::make (std::shared_ptr<const SampledInstrument> instrument, int note, int velocity,
                                 std::uint64_t take)
    {
        SampleNote out;
        if (instrument == nullptr)
            return out;
        const auto* r = instrument->region (note, velocity, take);
        if (r == nullptr || r->recording >= instrument->recordings.size())
            return out;
        out.recording = &instrument->recordings[r->recording];
        out.rootKey = r->key - r->tuneCents / 100.0;
        out.gain = instrument->gain * std::pow (10.0, r->volumeDb / 20.0);
        out.instrument = std::move (instrument);
        return out;
    }
}
