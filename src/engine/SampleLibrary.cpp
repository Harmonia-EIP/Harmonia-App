#include "SampleLibrary.h"

#include <algorithm>
#include <cmath>

namespace
{
    enum State : int { idle = 0, decoding = 1, decoded = 2, broken = 3 };

    std::int16_t toInt16 (float x) noexcept
    {
        return (std::int16_t) std::lround (juce::jlimit (-1.0f, 32767.0f / 32768.0f, x) * 32768.0f);
    }

    juce::String idOf (int choice)
    {
        const auto id = HarmoniaCharter::sampleInstrumentIds[(std::size_t) (choice - 1)];
        return juce::String::fromUTF8 (id.data(), (int) id.size());
    }

    bool isAttack (const juce::var& region)
    {
        return region.getProperty ("trigger", "attack").toString() == "attack";
    }

    std::size_t estimatedBytes (const juce::var& entry)
    {
        std::size_t total = 0;
        if (auto* regions = entry.getProperty ("regions", {}).getArray())
            for (const auto& r : *regions)
                if (isAttack (r))
                    total += (std::size_t) (juce::int64) r.getProperty ("frames", 0)
                           * (std::size_t) (int) r.getProperty ("channels", 1) * sizeof (std::int16_t);
        return total;
    }
}

SampleLibrary::SampleLibrary (juce::File f, std::size_t memoryBudget)
    : juce::Thread ("Harmonia samples"), folder (std::move (f)), budget (memoryBudget)
{
    for (auto& s : states)
        s.store (idle);
    formats.registerBasicFormats();
    startThread (juce::Thread::Priority::low);
}

SampleLibrary::~SampleLibrary()
{
    stopThread (5000);
}

juce::File SampleLibrary::defaultFolder()
{
    const auto fromEnvironment = juce::SystemStats::getEnvironmentVariable ("HARMONIA_SAMPLES_DIR", {});
    if (fromEnvironment.isNotEmpty())
        return juce::File (fromEnvironment);
    auto base = juce::File::getSpecialLocation (juce::File::userApplicationDataDirectory);
   #if JUCE_MAC
    base = base.getChildFile ("Application Support");
   #endif
    return base.getChildFile ("Harmonia").getChildFile ("Samples");
}

juce::File SampleLibrary::getFolder() const
{
    const juce::ScopedLock sl (lock);
    return folder;
}

void SampleLibrary::setFolder (const juce::File& f)
{
    {
        const juce::ScopedLock sl (lock);
        folder = f;
    }
    ++generation;
    rescan.store (true);
    notify();
}

bool SampleLibrary::isInstalled() const
{
    const juce::ScopedLock sl (lock);
    return installed;
}

void SampleLibrary::want (int choice) noexcept
{
    wanted.store (choice);
}

std::shared_ptr<const harmonia::SampledInstrument> SampleLibrary::get (int choice) const noexcept
{
    if (choice <= 0 || choice >= choices)
        return nullptr;
    return std::atomic_load (&slots[(std::size_t) choice]);
}

SampleLibrary::Status SampleLibrary::status (int choice) const noexcept
{
    if (choice <= 0 || choice >= choices)
        return Status::none;
    {
        const juce::ScopedTryLock sl (lock);  // the UI asks; never wait for a long scan
        if (sl.isLocked())
        {
            if (! installed)
                return Status::notInstalled;
            if (! catalogue.hasProperty (idOf (choice)))
                return Status::notInPack;
        }
    }
    switch (states[(std::size_t) choice].load())
    {
        case decoded: return Status::ready;
        case broken:  return Status::failed;
        default:      return Status::loading;
    }
}

juce::String SampleLibrary::credits() const
{
    const auto file = getFolder().getChildFile ("CREDITS.md");
    return file.existsAsFile() ? file.loadFileAsString() : juce::String();
}

std::size_t SampleLibrary::loadedBytes() const
{
    std::size_t total = 0;
    for (int c = 1; c < choices; ++c)
        if (auto instrument = get (c))
            total += instrument->bytes();
    return total;
}

bool SampleLibrary::waitUntilReady (int choice, int timeoutMs)
{
    want (choice);
    notify();
    const auto end = juce::Time::getMillisecondCounter() + (juce::uint32) timeoutMs;
    while (juce::Time::getMillisecondCounter() < end)
    {
        if (! rescan.load())
        {
            const auto s = status (choice);
            if (s == Status::ready)
                return true;
            if (s == Status::failed || s == Status::notInPack || s == Status::notInstalled || s == Status::none)
                return false;
        }
        juce::Thread::sleep (5);
    }
    return false;
}

void SampleLibrary::scanFolder()
{
    for (int c = 1; c < choices; ++c)
        letGo (c);
    juce::var parsed;
    const auto file = getFolder().getChildFile ("catalog.json");
    if (file.existsAsFile())
        parsed = juce::JSON::parse (file);
    auto* byId = new juce::DynamicObject();
    if (auto* instruments = parsed.getProperty ("instruments", {}).getArray())
        for (const auto& entry : *instruments)
            byId->setProperty (entry.getProperty ("id", {}).toString(), entry);
    const juce::ScopedLock sl (lock);
    catalogue = juce::var (byId);
    installed = parsed.isObject();
}

std::shared_ptr<harmonia::SampledInstrument> SampleLibrary::decode (int choice, const juce::var& entry)
{
    auto instrument = std::make_shared<harmonia::SampledInstrument>();
    instrument->id = idOf (choice).toStdString();
    instrument->gain = (double) entry.getProperty ("gain", 1.0);
    const auto root = getFolder();
    auto* regions = entry.getProperty ("regions", {}).getArray();
    if (regions == nullptr)
        return nullptr;
    for (const auto& r : *regions)
    {
        if (! isAttack (r))
            continue;
        if (threadShouldExit() || rescan.load())  // another folder was asked for meanwhile
            return nullptr;
        const auto path = r.getProperty ("file", {}).toString();
        std::unique_ptr<juce::AudioFormatReader> reader (formats.createReaderFor (root.getChildFile (path)));
        if (reader == nullptr || reader->lengthInSamples <= 1)
            return nullptr;
        const int frames = (int) reader->lengthInSamples;
        const int channels = juce::jmin (2, (int) reader->numChannels);
        juce::AudioBuffer<float> audio (channels, frames);
        reader->read (&audio, 0, frames, 0, true, channels > 1);
        harmonia::Recording rec;
        rec.sampleRate = reader->sampleRate;
        rec.left.resize ((std::size_t) frames);
        std::transform (audio.getReadPointer (0), audio.getReadPointer (0) + frames, rec.left.begin(), toInt16);
        if (channels > 1)
        {
            rec.right.resize ((std::size_t) frames);
            std::transform (audio.getReadPointer (1), audio.getReadPointer (1) + frames, rec.right.begin(), toInt16);
        }
        harmonia::SampleRegion region;
        region.key = (int) r.getProperty ("key", 60);
        region.lokey = (int) r.getProperty ("lokey", 0);
        region.hikey = (int) r.getProperty ("hikey", 127);
        region.lovel = (int) r.getProperty ("lovel", 1);
        region.hivel = (int) r.getProperty ("hivel", 127);
        region.roundRobin = (int) r.getProperty ("round_robin", 1);
        region.tuneCents = (double) r.getProperty ("tune_cents", 0.0);
        region.volumeDb = (double) r.getProperty ("volume_db", 0.0);
        region.file = path.toStdString();
        region.recording = instrument->recordings.size();
        instrument->recordings.push_back (std::move (rec));
        instrument->regions.push_back (std::move (region));
    }
    return instrument->regions.empty() ? nullptr : instrument;
}

void SampleLibrary::letGo (int choice)
{
    auto& slot = slots[(std::size_t) choice];
    auto previous = std::atomic_load (&slot);
    std::atomic_store (&slot, std::shared_ptr<const harmonia::SampledInstrument>());
    states[(std::size_t) choice].store (idle);
    recent.erase (std::remove (recent.begin(), recent.end(), choice), recent.end());
    if (previous != nullptr)
        graveyard.push_back (std::move (previous));  // freed here once no voice holds it
}

void SampleLibrary::makeRoom (std::size_t needed, int keep)
{
    while (! recent.empty() && loadedBytes() + needed > budget)
    {
        const auto oldest = std::find_if (recent.begin(), recent.end(), [keep] (int c) { return c != keep; });
        if (oldest == recent.end())
            break;
        letGo (*oldest);
    }
}

void SampleLibrary::run()
{
    while (! threadShouldExit())
    {
        if (rescan.load())
        {
            const int asked = generation.load();
            scanFolder();
            if (generation.load() == asked)
                rescan.store (false);  // only now does status() report on the new folder
        }

        const int choice = wanted.load();
        if (choice > 0 && choice < choices)
        {
            recent.erase (std::remove (recent.begin(), recent.end(), choice), recent.end());
            juce::var entry;
            {
                const juce::ScopedLock sl (lock);
                entry = catalogue.getProperty (idOf (choice), {});
            }
            auto& state = states[(std::size_t) choice];
            if (entry.isObject() && state.load() == idle)
            {
                state.store (decoding);
                makeRoom (estimatedBytes (entry), choice);
                if (auto instrument = decode (choice, entry))
                {
                    std::atomic_store (&slots[(std::size_t) choice],
                                       std::shared_ptr<const harmonia::SampledInstrument> (std::move (instrument)));
                    state.store (decoded);
                }
                else
                {
                    state.store (rescan.load() || threadShouldExit() ? idle : broken);
                }
            }
            if (state.load() == decoded)
                recent.push_back (choice);
        }

        graveyard.erase (std::remove_if (graveyard.begin(), graveyard.end(),
                                         [] (const auto& p) { return p.use_count() <= 1; }),
                         graveyard.end());
        wait (25);
    }
}
