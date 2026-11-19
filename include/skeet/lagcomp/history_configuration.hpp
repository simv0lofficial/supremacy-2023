#pragma once
#include <cstdint>
#include "skeet/rage/experimental_switch.hpp"

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
    rage::ExperimentalSwitch rework{};
    rage::ExperimentalSwitch jitter_fix{};
    rage::ExperimentalSwitch resolver_corrections{};
    bool jitter_enabled() const noexcept {
        return jitter_fix && resolver_enabled && *resolver_enabled;
    }
    bool reworked() const noexcept {
        return bool(rework);
    }
    bool resolver_reworked() const noexcept {
        return bool(resolver_corrections);
    }
};
} //namespace skeet::lagcomp
