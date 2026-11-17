#pragma once
#include "bunny_hop.hpp"
#include "command_state.hpp"
#include "simulation_environment.hpp"

namespace skeet {

struct PlayerAirGateSettings {
    bool air_strafe_enabled; // owner +2C, latched before the water gate
    bool bunny_hop_enabled; // owner +194
    float maximum_move; // owner +1C4
    float brake_reduction_limit; // owner +1CC
    float brake_threshold; // owner +1D0
    const void* air_accelerate; // owner +14, live ConVar parent chain
};

struct PlayerAirGateFields {
    std::uint32_t movement_type_7501cb91;
    std::uint32_t water_state_ac7f2913;
    std::uint32_t velocity_36375c23;
    std::uint32_t weapon_definition_5c9f0fbb;
    std::uint32_t weapon_blocked_b8969256;
    std::uint32_t player_next_attack_de530a8b;
    std::uint32_t weapon_next_attack_f71ce331;
};

struct PlayerAirGateBindings {
    PlayerAirGateFields fields;
    const MovementEnvironmentBindings& environment;
    const std::uint32_t* speed_policy_flags; // 43467E40
    const float* current_time; // 43467EB8
    BunnyHopGuard bunny_hop_guard;
};

struct PlayerAirGateFrame {
    void* player;
    void* weapon;
    const MovementFeedback* feedback;
};

enum class PlayerAirGateContinuation {
    air_solver, // 4337322A
    ground_speed_tail, // 4337C895/4337C899
    complete, // 4337CAA4
};

struct PlayerAirGateResult {
    PlayerAirGateContinuation continuation;
    bool air_strafe_enabled;
};

// Complete 43372C76..4337322A player gate. The live player450 call and the
// following speed-limit read use the shared environment helper. Subsequent
// acceleration and velocity reads retain their positions after that call.
PlayerAirGateResult process_player_air_gate(const PlayerAirGateSettings&, const PlayerAirGateFrame&,
                                            const PlayerAirGateBindings&,
                                            std::uint32_t& scheduler_flags, UserCommand&,
                                            MovementAngleView movement_angles);

} // namespace skeet
