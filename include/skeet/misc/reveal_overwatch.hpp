#pragma once
#include "skeet/engine/owner_registry.hpp"
#include "skeet/misc/config.hpp"
#include <cstdint>

namespace skeet::misc {
// Owner4348E4CC. The configuration value is the existing BooleanConfig
// registered asD351D519; the saved byte is separate state at owner+20.
class RevealOverwatchRuntime {
public:
    explicit RevealOverwatchRuntime(void* const& engine_cell) : engine(engine_cell) {}
    const std::uint8_t* enabled = nullptr; // owner+1C
    void* const& engine; //live4346A514, read separately by both update callbacks
    std::uint8_t pending = 0; // owner+20

    void begin_network_update(); //4331F346, secondary endpoint slot4
    void end_network_update(); //4334E15D, secondary endpoint slot1
    void reset() noexcept; //433843EB, lifecycle slot5 and slot7 forwarder
};

// Complete owner constructor4348E4CC, applied to caller-supplied native heap
// storage. The caller provides at least reveal_overwatch_owner_size() bytes
// from43387448's aligned allocation; lifecycle slot0 frees that allocation.
// Registration is lifecycle, update endpoint, then descriptor initialization.
std::size_t reveal_overwatch_owner_size() noexcept;
RevealOverwatchRuntime& construct_reveal_overwatch(void* storage,
                                                   engine::OwnerRegistryBinding owners,
                                                   engine::OwnerRegistryBinding updates,
                                                   ConfigRegistries&, RevealOverwatchConfig&,
                                                   void* const& engine);
} // namespace skeet::misc
