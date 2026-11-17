#pragma once
#include "skeet/animation/entity_update.hpp"

namespace skeet::esp {
struct HitboxHighlightState;
}
namespace skeet::rage {
struct PreambleState;
struct AcquisitionStorage;
struct CommandOwner;

//4340BCDA receives the complete configuration owner, eight bytes before its
// gameplay interface. References retain the already shared source owners.
struct RageResetBindings {
    PreambleState& state;
    AcquisitionStorage& acquisition;
    std::int32_t& shot_index; //complete+170, gameplay+168
    std::int32_t& field_174; //complete+174; constructor/reset sentinel-1
    esp::HitboxHighlightState& highlight; //target_handle complete+180
    std::array<animation::AnimationAngleSample, 2>& pending_angles; //434759DC/E8
};
// Complete4340BCDA, also reached through primary slot7's433CF2F5->slot5.
// Logical ends/counters are reset; frame, spread, evaluation and work buffers
// retain their incoming bytes. The current command flags remain untouched.
void reset_rage_state(const RageResetBindings&) noexcept;
void reset_rage_command_owner(CommandOwner&, std::int32_t& field_174) noexcept;
}
