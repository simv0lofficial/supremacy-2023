#pragma once
#include <array>
#include <cstdint>

namespace skeet::resolver_runtime {

// The animation-state members read by 0x433FAC08. Offsets refer to the
// animation-state object, except eye_yaw/animation_fraction (record +1AC/+538).
struct HypothesisInput {
    float speed_fraction; // XMM0 argument
    float feet_yaw; // animation state +84
    float movement_yaw; // +88
    float movement_yaw_target; // +8C
    float duck_amount; // +A4
    float running_fraction; // +F8
    float ducking_fraction; // +FC
    float minimum_yaw; // +334
    float maximum_yaw; // +338
    float eye_yaw; // record +1AC
    float animation_fraction; // record +538
    float velocity_x;
    float velocity_y;
    float velocity_z;
    float lower_body_yaw;
    float interval;
    std::uint32_t record_flags; // record +5C
    std::uint32_t moving; // animation state byte +108
};

struct Hypothesis {
    float yaw; // variant +0
    float pose_yaw; // variant +4
    float body_pose_parameter; // temporary entity pose parameter +2C
    float move_pose_parameter; // temporary entity pose parameter +1C
};

// Numeric chain of 0x433FAC08 through each 0x433FB639 bone-build boundary.
// Each candidate starts from the same animation state and runs two iterations.
// Bone construction is a separate native dependency (0x433FF63C).
std::array<Hypothesis, 3> solve_hypotheses(const HypothesisInput& input) noexcept;

} // namespace skeet::resolver_runtime
