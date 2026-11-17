#pragma once
#include "skeet/core.hpp"
#include "skeet/events/registry.hpp"
#include <cstdint>
#include <vector>

namespace skeet::misc {
struct RebuyFixOffsets {
    std::uint32_t team = 0; // 434688B0 / 8B5586F6
    std::uint32_t buy_zone = 0; // 4346A3B4 / FAEB9B15
    std::uint32_t field_f7df3858 = 0; // 43468D34; update reads this offset + 4
    std::uint32_t inventory = 0; // 434685D4 / FC3297F7, 48 handles
    std::uint32_t armor = 0; // 43467908 / 47A6F50C
    std::uint32_t helmet = 0; // 43468C04 / 7D6280C7
};

// Owner 4348217D. This restores the Rebuy fix feature, including its numeric
// engine buy commands; Automatic weapons is a separate command-processing owner.
class RebuyFixRuntime {
public:
    const std::uint8_t* enabled = nullptr; // owner +24, hash 6ED248E1
    void* engine = nullptr;
    void* entity_list = nullptr;
    void* buy_anywhere_cvar = nullptr; // 434670E8 ^ 308C7E6B
    void* local_player = nullptr;
    const float* current_time = nullptr; // *4346A334
    const double* time_4347a500 = nullptr;
    const double* time_4347b118 = nullptr;
    RebuyFixOffsets offsets;
    float last_buy_time = 0; // owner +28
    bool blocked = false; // owner +2C
    std::uint32_t previous_buttons = 0; // owner +30
    std::vector<std::int32_t> inventory; // owner +34
    std::uint8_t purchase_subscription = 0; // owner +50, raw observer state

    void update(const UserCommand&); // complete 43407336, receiver owner +8
    void player_spawn(events::GameEventReader&); // 4337F2EA -> 4339532B
    void item_purchase(events::GameEventReader&); // 433D0935 -> 433AD011
    void item_purchase(events::GameEventReader&, void* local); //actual event-master EDX snapshot

    // The constructor always subscribes to player_spawn. The BooleanConfig's
    // native change observer 433C8E40 separately toggles item_purchase.
    void bind_spawn(events::EventRegistry&, bool engine_connected);
    void bind_spawn(events::EventRegistry&);
    void sync_purchase_subscription(events::EventRegistry&, bool engine_connected);
    void sync_purchase_subscription(events::EventRegistry&);
    void disable_purchase_subscription(events::EventRegistry&); //4332B079 ->4336B697(0)
    void unbind_events(events::EventRegistry&, bool engine_connected);

private:
    bool local_event(events::GameEventReader&);
    static void spawn_listener(void*, events::GameEventReader&, events::EventPlayerIndices&);
    static void purchase_listener(void*, events::GameEventReader&, events::EventPlayerIndices&);
};
} // namespace skeet::misc
