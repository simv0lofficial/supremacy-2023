#pragma once
#include "skeet/config.hpp"

namespace skeet::misc {
class PingSpikeRuntime {
public:
    const std::uint8_t* enabled = nullptr; // 0x434691FC
    const RegisteredHotkey* hotkey = nullptr; // 0x43469388
    InputBitmap keys{};
    void* engine = nullptr;
    const std::int32_t* milliseconds = nullptr; // 0x43467EDC
    const std::int32_t* maximum_unlag_milliseconds = nullptr; // 0x43468230
    const double* latency_4347a508 = nullptr;
    const double* latency_4347b120 = nullptr;
    const float* interval_per_tick = nullptr; // *0x4346A338
    bool active() const; // complete 0x43419A40
    void rewind_sequence(void* channel) const; // complete 0x433B15CE, channel +0x1C
};
} // namespace skeet::misc
