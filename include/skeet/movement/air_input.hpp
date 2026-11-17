#pragma once
#include "air_strafe.hpp"
#include "steering_math.hpp"

namespace skeet {

// Complete solver input-heading stages 0x43373CBC..0x43373F63 and
// 0x43379B3B..0x43379C59. The boolean selects their proven basis behavior.
// False continues at the ground-speed tail; command writes before that edge
// are preserved. The caller has already selected the airborne steering path.
bool prepare_air_strafe_heading(const AirStrafeSelection& selection, MovementVector velocity,
                                bool normalized_basis, UserCommand& command,
                                MovementAngleView movement_angles) noexcept;

} // namespace skeet
