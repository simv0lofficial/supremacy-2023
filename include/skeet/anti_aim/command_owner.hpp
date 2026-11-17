#pragma once
#include "skeet/anti_aim/runtime.hpp"
#include "skeet/client/command_frame.hpp"

namespace skeet::client {
class TargetRefreshRuntimeOwner;
}
namespace skeet::anti_aim {
struct CommandOwnerPhase2Bindings {
    MovementCollisionEngine& collision;
    MovementSimulation& workspace;
    const client::ScriptCommandEnvironment& roll_guard;
};
struct CommandOwnerPitchRandomBindings {
    void* context;
    void (*seed)(void*, std::int32_t);
    std::int32_t (*integer)(void*, std::int32_t, std::int32_t);
};
struct CommandOwnerEligibilityBindings {
    void* const& input_system;
    void* const& engine_client;
    void* const& hud_context;
    engine::MemberFunction<void*, const char*> const& find_hud;
};
struct CommandOwnerAngleBindings {
    ProximityEngine& proximity;
    BallisticEngine& ballistics;
    const BodyFreestandingMath& body_math;
    const RageBoxClipper& clipper;
    BodyFreestandingWorkspace& workspace;
    const CommandOwnerPitchRandomBindings& pitch;
    const JitterRandom& random;
};
struct CommandOwnerPhase4Bindings {
    const CommandOwnerEligibilityBindings& eligibility;
    const client::TargetRefreshRuntimeOwner& target_refresh;
    const CommandOwnerAngleBindings& angles;
};
struct CommandOwnerPhase6Bindings {
    const float& simulation_time;
    const RageVector& final_angles;
    const lagcomp::EntityNetvars& entity_offsets;
    std::uint8_t* const& reset_byte;
    void* const& animation_update;
    lagcomp::SetAbsoluteVector const& set_absolute_angles;
    const animation::BoneRuntime& bones;
};
struct CommandOwnerOffsets {
    std::uint32_t player_flags_e2fbe695, velocity_36375c23, duck_3309c0cf, rate_fb7e3404;
    std::uint32_t ducking_6b1a59b5, movement_type_7501cb91, movement_type_8b5586f6;
    std::uint32_t cached_a15c3105, tick_base_3d10b94c, field_7fcb7381, edge_3eb71843;
    std::uint32_t guard_d720aea6, clock_9f8e7f71;
    std::uint32_t rule_bf567619, rule_dda99ab5, rule_b7ea8fda;
    std::uint32_t weapon_state_e1df7e9b, weapon_time_3b93667b;
    std::uint32_t flags_38ca5228, field_043fa575, field_334b460c, field_86a5f8b3;
};
struct CommandOwnerGlobals {
    std::int32_t* const& mode; //43475B2C, pointed value read at dispatch
    std::int32_t* const& choked; //434757DC
    float* const& interval; //4346A338
    float* const& current_time; //43467EB8
    void** const& shared_player; //4346A3A4
    std::int32_t* const& sequence; //434678F8
    std::int32_t* const& maximum_clients; //43469300
    void* const& engine_globals; //43467B04
    const std::int32_t& prediction_budget; //43469274
    std::uint32_t& scheduler_flags; //434759C0
    TargetState& target;
    const std::int32_t& last_body_command; //434759C8
    const std::int32_t& shot_command; //434759CC
    const std::int32_t& last_sent_command; //434759F4
    const std::uint8_t& shot_correction; //43468410
    const RageVector& shot_position; //434759D0
    const HotkeyBinding& height_hotkey; //43467C8C
    const InputBitmap& input;
    std::uint8_t* const& trace_flag; //434683D0
    const std::uint8_t& pending_angles_active; //43468D7C
    const std::array<animation::AnimationAngleSample, 2>& pending_angles; //434759DC,E8
};
// Concrete owner data consumed by the CreateMove master's native registration
// list. The phase bindings retain actual engine boundaries and live model/state
// views; player/frame/command/global reads below are refreshed at each dispatch.
struct CommandOwner {
    const GlobalConfig& global;
    Config& config;
    AngleParameters& parameters;
    AngleStageState& state;
    PrecalculationState& precalculation;
    FinisherState& finisher;
    const CommandOwnerPhase2Bindings& phase2;
    const CommandOwnerPhase4Bindings& phase4;
    const CommandOwnerPhase6Bindings& phase6;
    const MovementCaptureInput& movement;
    const MovementEnvironmentBindings& environment;
    CommandOwnerOffsets offsets;
    CommandOwnerGlobals globals;
    void* const& engine_client;
    void* const& engine_trace;
    void* const& entity_list;
};
AngleFrame capture_angle_frame(const client::CommandFrame&) noexcept;
void command_owner_phase2(CommandOwner&, void* player, void* weapon, client::Command&,
                          const std::array<float, 3>& incoming_angles, std::uint8_t& send_packet);
void command_owner_phase4(CommandOwner&, client::CommandFrame&, void* player, client::Command&,
                          std::uint8_t& send_packet);
void command_owner_phase6(CommandOwner&, client::CommandFrame&, void* player, client::Command&,
                          std::uint8_t& send_packet);
} // namespace skeet::anti_aim
