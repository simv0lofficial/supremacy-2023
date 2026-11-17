#pragma once
#include "packet_scheduler.hpp"
#include "skeet/anti_aim/config.hpp"
#include "skeet/anti_aim/movement_capture.hpp"
#include "skeet/client/command_frame.hpp"

namespace skeet::client {
class TargetRefreshRuntimeOwner;
}
namespace skeet::network {
struct PacketCommandOffsets {
    std::ptrdiff_t origin_a9ee6062, player_flags_e2fbe695, duck_3309c0cf, velocity_36375c23;
    std::ptrdiff_t state_7501cb91, state_ac7f2913, state_f3c10e80;
    std::ptrdiff_t rule_d720aea6, rule_bf567619, rule_dda99ab5, rule_b7ea8fda;
};
// References include the global pointer slots themselves. A pointer replaced
// by an earlier command owner is read again at this owner's actual phase.
struct PacketCommandGlobals {
    std::int32_t* const& choked_commands; //434757DC
    std::int32_t* const& last_outgoing; //43475B2C
    float* const& interval; //4346A338
    float* const& real_time; //4346A334
    void* const& engine_globals; //43467B04; receive interval+20
    void* const& engine_client; //4346A514
    void* const& entity_list; //4346A890
    void** const& shared_player; //4346A3A4, nullable holder
    std::int32_t* const& maximum_clients; //43469300
    std::int32_t* const& sequence; //434678F8
    const std::int32_t& last_command; //434759C4
    const std::uint8_t& force_on_next_command; //43468D7C
    const std::int32_t& duck_mode; //4346A580
    const std::int32_t& configured_prediction_budget; //43469274
    const InputBitmap& input;
    std::uint8_t* const& trace_flag; //434683D0
    bool& collision_result; //4347CA30
    void** const& trace_filter_vtable; //43475B90
};
struct PacketCommandBindings {
    PacketWorld& world;
    const client::TargetRefreshRuntimeOwner& target_refresh;
    const rage::TargetBindings& targeting;
    const anti_aim::MovementCaptureInput& movement;
    const MovementEnvironmentBindings& movement_environment;
    PacketWeaponBindings weapons;
    RandomInterface random;
    std::span<const std::uint32_t, 3> weapon_classification; //43474580
};
struct PacketCommandOwner {
    FakeLagState& state; //configuration references alias settings' three integer payloads
    anti_aim::TargetState& target; //same publication used by the AA owner
    const anti_aim::FakeLagSettings& settings; //canonical registered descriptors
    PacketCommandGlobals globals;
    PacketCommandOffsets offsets;
    PacketCommandBindings bindings;
};

PacketFrame capture_packet_frame(const client::CommandFrame&) noexcept;
// Sparse runtime tail43490237..4349025F; all unlisted state survives. The live
// overload reads RDTSC after the prefix writes, at the original native point.
void initialize_packet_command_state(FakeLagState&) noexcept;
// Explicit decoded-timestamp surface for callers that already captured it.
void initialize_packet_command_state(FakeLagState&, std::uint32_t timestamp_low) noexcept;
// Native owner+8, table43470390: phases2/4/6 and query9. Other slots are
// its actual no-op methods and do not create another runtime owner.
void process_packet_phase2(PacketCommandOwner&, void* player, void* weapon, client::Command&,
                           const std::array<float, 3>& incoming_angles, std::uint8_t& send_packet,
                           std::uint32_t& global_flags);
void process_packet_phase4(PacketCommandOwner&, client::CommandFrame&, void* player,
                           client::Command&, std::uint8_t& send_packet,
                           std::uint32_t& global_flags);
void process_packet_phase6(PacketCommandOwner&, client::CommandFrame&, void* player,
                           client::Command&, std::uint8_t& send_packet,
                           std::uint32_t& global_flags);
std::int32_t query_packet_command_limit(PacketCommandOwner&, std::int32_t running_limit) noexcept;
} // namespace skeet::network
