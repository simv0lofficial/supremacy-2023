#pragma once
#include "simulation.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet {
struct MovementEnvironmentBindings;
struct MovementCaptureOffsets {
    std::uint32_t speed_scale_7f5ed2d0, duck_speed_fb7e3404, duck_amount_3309c0cf;
    std::uint32_t view_offset_c246315e, stamina_2f33c681, ducking_6b1a59b5;
    std::uint32_t fall_velocity_7e9ba7bc, field_f3c10e80, movement_type_7501cb91;
};
struct MovementCaptureConvars {
    engine::ConVarBinding gravity;
    engine::ConVarBinding jump_impulse;
    engine::ConVarBinding step_size;
    engine::ConVarBinding stamina_recovery;
    engine::ConVarBinding stamina_land;
    engine::ConVarBinding stamina_jump;
    engine::ConVarBinding stamina_maximum;
    engine::ConVarBinding stop_speed;
    engine::ConVarBinding friction;
    engine::ConVarBinding accelerate;
    engine::ConVarBinding cvar_dceaeac8;
};
struct MovementCaptureInput {
    const MovementCaptureOffsets& offsets;
    const MovementCaptureConvars& convars;
    const float& interval;
    const MovementEnvironmentBindings* environment = nullptr;
};
// Live4337D3C8 reads, including virtual130/134/260 in native order.
MovementSimulationPlayer capture_movement_player(void*, const MovementCaptureOffsets&);
MovementSimulationRules capture_movement_rules(const MovementCaptureConvars&,
                                               float interval) noexcept;
// Complete live constructor433B0D5C. Caller already owns the copied origin,
// velocity and flags; ConVar reads precede the player/getter snapshot.
void initialize_live_movement_simulation(MovementSimulation&, void* player, MovementVector position,
                                         MovementVector velocity, std::uint32_t player_flags,
                                         std::uint32_t mode, const MovementCaptureInput&);
} // namespace skeet
