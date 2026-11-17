#pragma once
#include "types.hpp"

namespace skeet {

struct DuckPeekSettings {
    HotkeyBinding hotkey; // 43467C9C / 43467CA0
    bool crouch_cycle; // 43467CEC
};

// Full movement owner fields 51C..548. The master scheduler captures the
// original command buttons and updates choked/previous_buttons/last_key.
struct DuckPeekState {
    std::uint8_t phase = 0; // +51C, states 0..3
    std::int32_t delay = 0; // +520
    std::int32_t release_commands = 0; // +524
    std::int32_t crouch_commands = 0; // +528
    float target = 0; // +52C
    std::int32_t field_530 = -1; // +530, written by4347DCB2
    std::int32_t choked = 0; // +534
    std::uint32_t original_buttons = 0; // +538
    std::uint32_t previous_buttons = 0; // +53C
    std::int32_t last_key = 27; // +540
    std::uint8_t crouch_active = 0; // +544, shared with the HUD byte view
    std::uint8_t release_latch = 0; // +545
    const std::int32_t* configured_limit = nullptr; // +548
};

struct DuckPeekFrame {
    MovementPhysics physics;
    MovementVector velocity;
    float current_time;
    float player_duck_time; // decoded player field 04B798C6
    float duck_delay; // decoded terminal cvar at 43467D7C
    float weapon_simulation_time; // decoded weapon field 0176A381
    float weapon_duck_delay; // actual weapon virtual +6C4 result
    std::int32_t command_budget; // actual network_command_budget result
    bool has_weapon;
};

// Native scheduler prefix 4336F9AA..43370191, on the valid-local-player
// contract. The caller has saved original_buttons and reset scheduler_flags.
// configured_limit is the native live configuration pointer and must be valid
// when the crouch-cycle limit is queried. send_packet is only cleared here.
void process_duck_peek(const DuckPeekSettings&, InputBitmap, DuckPeekState&, const DuckPeekFrame&,
                       UserCommand&, std::uint32_t& scheduler_flags, std::uint8_t& send_packet);

} // namespace skeet
