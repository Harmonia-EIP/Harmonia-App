#pragma once

namespace AudioConfig {
namespace Frequency {
static constexpr double Min = 20.0;
static constexpr double Max = 20000.0;
static constexpr double Default = 440.0;
static constexpr double Mid = 1000.0; // pour skew
} // namespace Frequency

namespace Volume {
static constexpr double Min = 0.0;
static constexpr double Max = 1.0;
static constexpr double Default = 0.8;
} // namespace Volume

namespace ADSR {
namespace Attack {
static constexpr float Min = 0.01f;
static constexpr float Max = 5.0f;
static constexpr float Default = 0.1f;
} // namespace Attack

namespace Decay {
static constexpr float Min = 0.01f;
static constexpr float Max = 5.0f;
static constexpr float Default = 0.1f;
} // namespace Decay

namespace Sustain {
static constexpr float Min = 0.0f;
static constexpr float Max = 1.0f;
static constexpr float Default = 0.8f;
} // namespace Sustain

namespace Release {
static constexpr float Min = 0.01f;
static constexpr float Max = 5.0f;
static constexpr float Default = 0.5f;
} // namespace Release
} // namespace ADSR

namespace Filter {
namespace Cutoff {
static constexpr float Min = 20.0f;
static constexpr float Max = 20000.0f;
static constexpr float Default = 1000.0f;
static constexpr float Mid = 1000.0f;
} // namespace Cutoff

namespace Resonance {
static constexpr float Min = 0.1f;
static constexpr float Max = 10.0f;
static constexpr float Default = 1.0f;
static constexpr float Mid = 1.0f;
} // namespace Resonance
} // namespace Filter
} // namespace AudioConfig