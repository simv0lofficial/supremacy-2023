#pragma once
#include "command_state.hpp"
#include "simulation.hpp"
#include "bunny_hop.hpp"
#include "raw_air_solver.hpp"
#include "normalized_air_final.hpp"
#include "skeet/movement_config.hpp"
#include "skeet/client/commands.hpp"

namespace skeet {

// References bind the actual movement owner's five runtime scalars. They are
// not copied into a release-preroll snapshot.
struct MovementReleaseTuning {
    const float& angle_scale; // +0x1C0
    const float& side_magnitude; // +0x1C4
    const float& maximum_angle; // +0x1C8
    const float& feedback_turn_limit; // +0x1D0
    const float& normalized_angle_step; // +0x1D4
};
struct MovementReleaseOffsets {
    std::uint32_t player_state_ac7f2913;
    std::uint32_t weapon_definition_5c9f0fbb;
    std::uint32_t forecast_position_a9ee6062;
};

struct MovementReleaseRuntime {
    const MovementConfig& config;
    MovementReleaseTuning tuning;
    InputBitmap input;
    engine::ConVarBinding bunny_hop_guard; //live encoded4346A4E4 parent chain
    engine::ConVarBinding pre_speed_guard; //live encoded43468EA4 parent chain
    const void* const& jump_speed_cap; //owner+18, populated by its actual constructor
    const void* const& air_acceleration; // owner +0x14 parent chain, live cell
    const std::int32_t& force_duck_mode; // 4346A580
    const float& engine_time; // *4346A334
    void* engine_client;
    MovementReleaseOffsets offsets;
    const MovementEnvironmentBindings& environment;
    const MovementCaptureInput& capture;
    MovementCollisionEngine& collision;
    BallisticEngine& ballistics;
    void* const& surface_properties;
};

// Complete secondary movement-command synthesis at native 433A194E. The
// following simulation tick is owned by the caller, as in the native preroll.
// The caller's null runtime pointer implements the native absent-owner edge.
void synthesize_release_command(MovementReleaseRuntime&, void* player, void* weapon,
                                client::Command&, MovementSimulation&, MovementFeedback&);

} // namespace skeet
