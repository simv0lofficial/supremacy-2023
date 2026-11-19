#pragma once
#include <cstdint>

namespace skeet::lagcomp {
// One source-owned configuration for the complete4347E5DE owner. Offsets
// document its native +8 receiver; portable receiver/backend fields do not
// alias that native storage. The constructor and lifecycle bind these cells.
struct HistoryRecordingConfiguration {
    const std::int32_t* target_hitboxes; // receiver+28, full owner+30
    const std::uint8_t* recording_enabled; // receiver+2C, global43467224
    const std::uint8_t* resolver_enabled; // receiver+30, global4346754C
    std::uint8_t prediction_available; // receiver+34, full owner+3C
    void* engine; // receiver+38, full owner+40
    const std::uint8_t* rework = nullptr;
    bool reworked() const noexcept {
        return rework && *rework;
    }
};
} //namespace skeet::lagcomp
