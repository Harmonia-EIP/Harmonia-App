#pragma once

namespace AppConfig
{
    static constexpr auto AppName    = "HarmoniaApp";
    static constexpr auto Version    = "0.3.1";

    static constexpr int DefaultWidth  = 900;
    static constexpr int DefaultHeight = 700;

    namespace Oscilloscope
    {
        static constexpr int BufferSize  = 8192;
        static constexpr int RefreshRate = 60;
    }

    namespace Synth
    {
        static constexpr int minimumAvailableRange = 0;
        static constexpr int maximumAvailableRange = 127;
        static constexpr int availableRangeSize = maximumAvailableRange - minimumAvailableRange + 1;
        static constexpr double defaultSampleRate = 44100.0;
    }
}