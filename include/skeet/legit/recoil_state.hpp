#pragma once
#include "state.hpp"

namespace skeet::legit {
struct RecoilStateInput {
    Vector player_punch;
    Vector context_velocity; // weapon context+8C, copied before decay
    Vector player_velocity; // decoded48442721, future-shot local copy
    float player_next_attack, weapon_next_attack, time;
    std::uint32_t command_buttons;
};
// 4339A99B..4339ACF5, including future recoil simulation, normalized delta,
// random .2..3 motion cap, and parent-chained weapon_recoil_scale reads.
void update_recoil_state(RuntimeState&, const RecoilStateInput&, const void* recoil_scale,
                         const RecoilDecayBindings&, RandomBindings);
// Complete433D5ACC. Roll is explicitly zero, even when the frame has roll.
Vector recoil_reference_angles(const RuntimeState&, const Vector& frame_angles,
                               const void* recoil_scale);
// Complete433D1EDC, including the optional433E9160 normalization. A pure roll
// punch does not enter the addition, matching the native two-axis zero gate.
void add_player_recoil(Vector& angles, const Vector& punch, const void* recoil_scale,
                       bool normalize);
// Complete433C6416 under the real non-null weapon contract. Slot430 is called
// before testing the attack button, shot count or ammunition.
bool recoil_shot_ready(const ProfileState&, void* weapon, const client::Command&,
                       std::ptrdiff_t ammunition_offset);
} // namespace skeet::legit
