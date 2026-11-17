#pragma once
#include <cstdint>
#include <cstddef>

namespace skeet::lagcomp {
// Engine14 returns in ST0 on native x86. Keep that precision through the
// post-call interval read, divide and half-tick addition; round once to float
// before CVTTSS2SI. The scalar receive_time_to_tick entry remains separate.
std::int32_t sample_receive_tick(void* engine, const float& receive_interval);
// Receive proxies reload gpGlobals itself after the engine virtual returns.
std::int32_t sample_receive_tick(void* engine, std::byte* const& live_globals);
} //namespace skeet::lagcomp
