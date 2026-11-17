#pragma once
#include "skeet/skins/config.hpp"
#include "skeet/skins/apply.hpp"
#include "skeet/skins/refresh.hpp"
#include "skeet/engine/offset_storage.hpp"
#include <cstddef>
#include <cstdint>

namespace skeet::skins {
struct NativeInventoryBindings {
    void* const& engine_client; // live 4346A514
    void* const& resolver; // owner's capture of4346A890 at43490315, receiver+4
    const std::byte* const& counter_rows; // live 43467EF4, first row +4
    const std::int32_t* const& counters; // live 434670C4
    const engine::NativeOffsetGlobals& offsets;
};

// Concrete entity-update receiver for 4347030C slot1. The incoming engine
// snapshot has eight-byte entries at +44 and its signed count at +4044.
// No semantic entity resolver or feature-level write callback is substituted.
class NativeInventoryRuntime {
public:
    NativeInventoryRuntime(const WeaponSkinsConfig& settings, NativeInventoryBindings bindings,
                           CounterRandom& random, WeaponRefresh& refresh);
    void process(void* local_player, std::uintptr_t incoming); // complete 4339D6B2
private:
    std::int32_t stattrak_counter(void* item);
    const WeaponSkinsConfig& settings_;
    NativeInventoryBindings bindings_;
    CounterRandom& random_;
    WeaponRefresh& refresh_;
};
} // namespace skeet::skins
