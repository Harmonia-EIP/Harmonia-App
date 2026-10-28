/**
 * @file HarmoniaDx7.h
 * @brief Harmonia's DX7 mode: one DX7 voice played by Dexed's msfa FM core (Apache 2.0, see dx7/README.md).
 *
 * A DX7 preset is the 145 DX7 parameters of the charter, i.e. one DX7 voice exactly as stored in a
 * cartridge: no conversion. Each note is driven like the AI's reference renderer
 * (HARMONIA/native/dx7/dx7_render.cpp in Harmonia-AI), so the plugin plays the sounds the v3 models learned:
 * 64-sample blocks, the voice's own LFO, key-up on a block boundary, Dexed's DC filter, DX7 keyboard
 * velocity. No allocation while rendering.
 */

#pragma once

#include <array>
#include <cstdint>
#include <memory>

#include "../parameters/HarmoniaCharterV3.h"

namespace harmonia
{
    /** DX7 voice edit buffer: the 155-byte single-voice layout (OP6 first), then the name's last byte. */
    using Dx7Patch = std::array<std::uint8_t, 156>;

    inline constexpr std::size_t dx7ParameterCount = HarmoniaCharter::dx7End - HarmoniaCharter::dx7Begin;

    /** Patch from the physical values of the charter's DX7 parameters, in charter order (OP1..OP6, then the voice). */
    Dx7Patch dx7PatchFromValues (const float* values) noexcept;

    /** Every DX7 parameter at its charter default: the DX7 INIT VOICE. */
    Dx7Patch dx7InitPatch() noexcept;

    /** MIDI velocity (0..127) as the DX7 keyboard sends it (Dexed's "DX7 velocity" option). */
    int dx7Velocity (int midiVelocity) noexcept;

    class Dx7Voice
    {
    public:
        static constexpr int blockSize = 64;

        Dx7Voice();
        ~Dx7Voice();
        Dx7Voice (const Dx7Voice&) = delete;
        Dx7Voice& operator= (const Dx7Voice&) = delete;

        /** Sets msfa's sample-rate tables (shared by every voice). */
        void prepare (double sampleRate);

        /** velocity in 0..1, as JUCE gives it. */
        void start (const Dx7Patch& patch, int midiNote, double velocity) noexcept;
        /** velocity as msfa receives it (already scaled to the DX7 keyboard). */
        void startRaw (const Dx7Patch& patch, int midiNote, int dxVelocity) noexcept;

        void release() noexcept { releasePending = true; }
        /** Key-up at the first block boundary at or after this sample: offline renders. */
        void scheduleRelease (std::int64_t sample) noexcept { releaseAt = sample; }
        void kill() noexcept { active = false; }
        bool isActive() const noexcept { return active; }

        /** Adds numSamples (mono, to both channels) to left / right. */
        void render (float* left, float* right, int numSamples) noexcept;

    private:
        void computeBlock() noexcept;

        struct Impl;
        std::unique_ptr<Impl> impl;
        double rate = 48000.0;
        bool active = false, releasePending = false, keyIsUp = false, lastBlock = false;
        std::int64_t releaseAt = -1, sampleIndex = 0;
        std::array<float, blockSize> block {};
        int blockPos = blockSize;
        float dcIn = 0.0f, dcOut = 0.0f;
    };
}

/** C entry points for the AI's comparison bench (ctypes). The renderer mirrors HARMONIA/native/dx7's
    dx7_render(): dxVelocity is the velocity msfa receives. Return 0, or -1 on bad arguments. */
extern "C" int harmonia_render_dx7_note (const unsigned char* patch156, int midiNote, int dxVelocity, int holdSamples,
                                         int totalSamples, int sampleRate, float* out);
extern "C" int harmonia_dx7_patch_from_values (const float* values, int count, unsigned char* patch156);
