#pragma once
#include "collision.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/config.hpp"
#include "skeet/lagcomp/history.hpp"

namespace skeet {
struct BlockbotConfig {
    BooleanConfig enabled; // native phase owner +0C, value +1C
    HotkeyConfig hotkey; // +20, registered binding +30
};
struct BlockbotState {
    MovementVector locked_right{}; // native owner +48
    std::uint8_t activated = 0; // +54; reset only by the entry gates
    std::int32_t target_handle = -1; // +58
    std::uint8_t flags = 0; // +5C; bit0 marks successful prediction
};
// Native constructor43480261 registers the boolean, then an Escape/hold
// hotkey. The preexisting boolean value is preserved.
void initialize_blockbot_config(ConfigRegistries&, BlockbotConfig&) noexcept;
// Exact sparse runtime writes434802BC..434802CC: only the low byte of
// locked_right.x is cleared; its other bytes/y/z survive construction.
void initialize_blockbot_state(BlockbotState&) noexcept;
struct BlockbotOffsets {
    std::ptrdiff_t move_type; // 7501CB91, byte
    std::ptrdiff_t ground_flags; // E2FBE695, byte
    std::ptrdiff_t team; // 8B5586F6, int32
    std::ptrdiff_t view_offset; // C246315E, vector
    std::ptrdiff_t position_history; // 3CDE8B30, engine interpolation history
    std::ptrdiff_t network_origin; // A9EE6062, vector
    std::ptrdiff_t maximum_speed; // 3EB71843, float
    std::ptrdiff_t rejected_9d920e35, rejected_cde290d2, rejected_d070684c; // bytes
    std::ptrdiff_t rule_d720aea6; // byte
};
struct BlockbotBindings {
    void* engine_client; // 4346A514, actual GetViewAngles slot18
    void* entity_list; // 4346A890, actual index/handle slots3/4
    void* const* game_rules; // 4346A3A4, both levels optional
    const std::int32_t* max_clients; // 43469300
    const lagcomp::TickTimingSamples* timing; // canonical43478C70..78
    const float* interval; // 4346A338, read on each predicted tick
    MovementCollisionEngine* collision; // actual EngineTrace::TraceRay
    std::uint8_t* trace_flag; // optional native trace guard target
    BlockbotOffsets offsets;
};
// Complete valid-player433B92DC. Config value/hotkey are native owner+1C/+30.
// Includes concrete teammate scoring433EA084, center433FB98A and three-sample
// finite differences433B171A. Requires a nonnull player once enabled and hotkey
// active, matching the original deliberate invalid-instruction null paths.
// Retains a usable target, projects its velocity through hull traces, and
// adjusts only side_move and movement_angles; command view angles stay intact.
void process_blockbot(BlockbotState&, bool enabled, const HotkeyBinding&, InputBitmap, void* player,
                      client::Command&, MovementAngleView, const BlockbotBindings&);
} // namespace skeet
