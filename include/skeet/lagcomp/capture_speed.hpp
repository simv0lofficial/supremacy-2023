#pragma once
#include "history.hpp"

namespace skeet::lagcomp {
struct HistoryCaptureSpeedBindings {
    const EntityStateRuntime& entity;
    std::uint32_t flags_e2fbe695;
    std::uint32_t maximum_speed_3eb71843;
    std::uint32_t mode_7090d8dd;
    void* convar_4b52a9bf;
};
// Complete434115E3..117E6. XMM2 is zero on both incoming native paths
// (11452/115DD). The landing branch checks the actual ConVar integer; the
// already-grounded branch calls the player's maximum-speed virtual276.
// Only horizontal velocity changes; Z retains its original bits.
bool apply_history_capture_speed_limit(const RecordRing&, void* player,
                                       const HistoryCaptureSpeedBindings&);
} // namespace skeet::lagcomp
