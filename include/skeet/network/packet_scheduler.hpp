#pragma once
#include "packet_world.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/anti_aim/targets.hpp"

namespace skeet::network {
void* nearest_enemy(const rage::TargetBindings&, std::int32_t maximum_clients); // complete 43393A95
struct FakeLagTriggerFrame {
    PacketFrame packet;
    Vector predicted_origin; // frame+168
    float value_c0;
    std::int32_t tick_base; // frame+0C8
    std::uint8_t weapon_type; // frame+124
    void* weapon; // frame+00C
    std::uint32_t buttons, selected_weapon;
};
struct FakeLagTriggerEnvironment {
    PacketWorld& world;
    const rage::TargetBindings& targeting;
    PacketWeaponBindings weapons;
    void* engine_client; // actual engine virtual +380
    std::int32_t maximum_clients;
    std::ptrdiff_t active_weapon; // CD812654
    bool rule_bf567619; // complete 433240A6: absent rules => true
    std::int32_t last_outgoing;
    float current_time, interval, gravity;
};
// Complete contiguous middle of 433A069B, 433A0914..433A0C81, after the real
// shared target refresh4335BA8E. All geometry/weapon predicates run concretely;
// the caller then executes apply_fake_lag_send_gate and finish_fake_lag_tick.
void apply_fake_lag_triggers(FakeLagState&, const FakeLagTriggerFrame&,
                             const anti_aim::TargetState&, FakeLagTriggerEnvironment&,
                             std::uint32_t& command_flags, bool& collision_result);
// Complete433A069B. Uses the concrete shared target owner, so AA and packet
// processing retain the native once-per-sequence refresh contract.
void schedule_fake_lag(FakeLagState&, const FakeLagTriggerFrame&, const FakeLagActivation&,
                       bool freeze_period, anti_aim::TargetState&,
                       const anti_aim::TargetRefreshContext&, FakeLagTriggerEnvironment&,
                       const RandomInterface&, std::uint32_t& command_flags,
                       std::uint8_t& send_packet, bool& collision_result);
} // namespace skeet::network
