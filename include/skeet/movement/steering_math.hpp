#pragma once
#include "types.hpp"

namespace skeet {

// Complete native scalar helpers used by the air-movement solver.
float round_movement_step(float value) noexcept; // 0x4341AD12..0x4341ADBD
float planar_movement_yaw(MovementVector direction) noexcept; // 0x43397834..0x43397878
// 0x43366128..0x433662C5. Each output is optional. All angles are read before
// the forward, right, then up writes, so outputs may alias each other/input.
void movement_angle_vectors(MovementAngles angles, MovementVector* forward, MovementVector* right,
                            MovementVector* up) noexcept;
// Complete airborne input-heading block 0x43373D4B..0x43373F63. Only yaw is
// replaced; inputs are the moves saved before the solver clears the command.
void project_movement_heading(float forward_move, float side_move,
                              MovementAngleView movement_angles) noexcept;
// Raw-basis counterpart 0x43379BB2..0x43379C55; its basis and combined input
// vector are not normalized before converting the heading.
void project_raw_movement_heading(float forward_move, float side_move,
                                  MovementAngleView movement_angles) noexcept;
float normalize_movement_angle(float value) noexcept;
// Complete 0x433E9160 angle-vector normalization, including NaN handling.
void normalize_movement_angles(MovementAngleView angles) noexcept;

} // namespace skeet
