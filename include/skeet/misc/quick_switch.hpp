#pragma once
#include "skeet/client/commands.hpp"
#include "skeet/prediction/history.hpp"
#include "skeet/prediction/observer_receiver.hpp"
#include <cstddef>
#include <cstdint>
#include <limits>

namespace skeet::misc {
struct QuickSwitchOffsets {
    std::ptrdiff_t active_weapon; // 43467A74 / CD812654, player handle
    std::ptrdiff_t weapon_definition; // 4346908C / 5C9F0FBB, uint16
    std::ptrdiff_t grenade_redraw; // 43467BE4 / 4013FC03, byte
    std::ptrdiff_t grenade_pin_pulled; // 43467A6C / E865A877, byte
    std::ptrdiff_t grenade_throw_time; // 43467AA4 / 3B93667B, float
    std::ptrdiff_t shots_fired; // 4346A6DC / 8CFC633A, player int32
    std::ptrdiff_t ammunition; // 4346A60C / CA5EC698, weapon int32
};
struct QuickSwitchBindings {
    void* entity_list = nullptr; // 4346A890, GetClientEntityFromHandle slot4
    void* const* game_rules = nullptr; // 4346A3A4; either pointer may be null
    client::EntityCommandOwner* const& history_owner; //live43475898, getter433952F0
    const std::int32_t* last_outgoing_command = nullptr; // *43475B2C
    const std::int32_t* choked_commands = nullptr; // *434757DC
    QuickSwitchOffsets offsets{};
};
struct QuickSwitchState {
    std::int32_t pending_command = std::numeric_limits<std::int32_t>::max(); // owner+30
    std::int32_t last_switch_command = -1; // owner+34
    std::int32_t captured_command = 0; // owner+38
    float captured_throw_time = 0.0f; // owner+3C
    std::uint8_t captured_pin_pulled = 0; // owner+40
};

// Owner4347DC0A. Configuration remains the canonical BooleanConfig value
// registered by initialize_quick_switch_config (hash0D634908).
class QuickSwitchRuntime {
public:
    const std::uint8_t* enabled = nullptr; // owner+2C, descriptor+10
    QuickSwitchBindings bindings;
    QuickSwitchState state;
    prediction::NativePredictionObserverReceiver prediction_receiver{};

    // Prediction observer owner+18, vtable4346FA2C slots0/1. The engine's
    // real prediction call runs between these methods. Capture is skipped
    // for a command that already selects a weapon; finish deliberately does
    // not test enabled again and only accepts the captured command number.
    void capture_before_prediction(void* player, const client::Command&); // 4341F4A3
    void finish_prediction(void* player, const client::Command&); // 43400E0D

    // Command owner+8, vtable4346FC24 slot2, before initial frame capture.
    // Detects grenade completion or an empty Zeus after its first shot,
    // waits for the recorded command to be acknowledged, then asks the
    // actual game-rules slot27 for the next weapon. Its networkable slot10
    // supplies weapon_select; zero choked commands also clear send_packet.
    void process_phase2(void* player, void* weapon, client::Command&,
                        std::uint8_t& send_packet); // 4341704A
private:
    void* active_weapon(void* player) const;
};
} // namespace skeet::misc
