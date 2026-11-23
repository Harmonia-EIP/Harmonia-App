/**
 * @file SampleLibrary.h
 * @brief The recorded instruments installed on this machine, decoded in the background for the sample source.
 *
 * The sample packs (Harmonia-AI samples/README.md) are not in the plugin: they are downloaded once from the
 * website and unpacked into one folder (catalog.json, CREDITS.md, one sub-folder per library). By default:
 * - macOS: ~/Library/Application Support/Harmonia/Samples
 * - Windows: %APPDATA%\\Harmonia\\Samples
 * - or the folder in the HARMONIA_SAMPLES_DIR environment variable.
 *
 * The audio thread says which instrument the patch plays (want) and takes it once decoded (get): nothing is
 * read from disk or allocated on the audio thread. Instruments stay in memory (16-bit) up to a budget; the
 * least recently wanted ones are let go first, and freed on the library's own thread once no voice plays them.
 */

#pragma once

#include "../JuceHeader.h"
#include "../parameters/HarmoniaCharterV3.h"
#include "HarmoniaSamples.h"

#include <array>
#include <atomic>
#include <memory>
#include <vector>

class SampleLibrary : private juce::Thread
{
public:
    /** What the sample source can do with an instrument right now. */
    enum class Status
    {
        none,          ///< no instrument chosen (choice 0)
        notInstalled,  ///< no sample pack in the folder
        notInPack,     ///< the installed packs do not hold this instrument
        loading,       ///< being decoded in the background
        ready,
        failed         ///< a file could not be read
    };

    static constexpr int choices = (int) HarmoniaCharter::sampleInstrumentIds.size() + 1;  // 0 = none

    explicit SampleLibrary (juce::File folder = defaultFolder(), std::size_t memoryBudget = defaultBudget);
    ~SampleLibrary() override;

    static constexpr std::size_t defaultBudget = (std::size_t) 2560 * 1024 * 1024;

    /** $HARMONIA_SAMPLES_DIR, else the per-user application data folder. */
    static juce::File defaultFolder();

    juce::File getFolder() const;
    /** Message thread: use another folder (the instruments of the previous one are let go). */
    void setFolder (const juce::File& folder);
    bool isInstalled() const;

    /** Audio thread, each block: the instrument the patch plays (0 = none). Decoded in the background if needed. */
    void want (int choice) noexcept;
    /** Audio thread: the decoded instrument, or nullptr while it is not ready. */
    std::shared_ptr<const harmonia::SampledInstrument> get (int choice) const noexcept;

    Status status (int choice) const noexcept;
    /** The credits of the installed packs (CREDITS.md), to show to the user. */
    juce::String credits() const;
    std::size_t loadedBytes() const;

    /** Tests and offline renders: wait until `choice` is ready (or failed / absent). */
    bool waitUntilReady (int choice, int timeoutMs);

private:
    void run() override;
    void scanFolder();
    std::shared_ptr<harmonia::SampledInstrument> decode (int choice, const juce::var& entry);
    void letGo (int choice);
    void makeRoom (std::size_t needed, int keep);

    mutable juce::CriticalSection lock;  // folder, catalogue, recent, graveyard (never taken by the audio thread)
    juce::File folder;
    juce::var catalogue;                 // id -> catalogue entry
    bool installed = false;
    std::size_t budget;

    std::array<std::shared_ptr<const harmonia::SampledInstrument>, (std::size_t) choices> slots;  // atomic_load / atomic_store
    std::array<std::atomic<int>, (std::size_t) choices> states;
    std::atomic<int> wanted { 0 };
    std::atomic<bool> rescan { true };   // stays true until the folder asked for has been read
    std::atomic<int> generation { 0 };   // bumped by setFolder: a scan that started before is not the last
    std::vector<int> recent;  // decoded choices, least recently wanted first
    std::vector<std::shared_ptr<const harmonia::SampledInstrument>> graveyard;
    juce::AudioFormatManager formats;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (SampleLibrary)
};
