#pragma once
#include <array>
#include <cstddef>
#include <cstdint>

namespace skeet::esp {
struct PickupObservation {
    std::uint16_t player_index, item_index;
    std::int32_t frame;
};
static_assert(sizeof(PickupObservation) == 8);
// Native constructor owner+71C/+720. Newest entries are at index0; the shared
// dormant update consumer and event callbacks use this one queue.
struct PickupObservations {
    std::uint32_t count = 0;
    std::array<PickupObservation, 128> entries{};
};
static_assert(offsetof(PickupObservations, entries) == 4);
} // namespace skeet::esp
