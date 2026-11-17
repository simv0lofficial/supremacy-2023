#pragma once
#include "skeet/anti_aim/jitter.hpp"
#include "skeet/anti_aim/body_freestanding.hpp"

namespace skeet::anti_aim {
struct AnimationYawInput {
    float duck_amount_a4, running_fraction_f8, ducking_fraction_fc;
    float speed_fraction_11c, minimum_yaw_334, maximum_yaw_338;
};
// Complete433EAEEB. Native writes maximum before reading/writing minimum.
void animation_yaw_limits(const AnimationYawInput&, float& minimum, float& maximum) noexcept;
struct BodyYawSettings {
    std::int32_t mode, jitter_mode, jitter_seed_98, static_offset_9c;
    float opposite_angle_134, jitter_maximum_144;
    std::array<float, 12> skitter;
};
struct BodyYawState {
    std::uint32_t phase_190;
    float cached_opposite_1a0, reference_yaw_258;
};
// Regular branch of43365675 with freestanding_body_yaw disabled. Valid modes
// are1..3; the native caller excludes0. Special freestanding solves are separate.
float regular_body_yaw(const BodyYawSettings&, BodyYawState&, std::int32_t frame_command,
                       std::int32_t last_command, float base_yaw, const JitterRandom&);
struct BodyBlendSettings {
    float minimum_magnitude_134, maximum_magnitude_138, base_angle_13c, expanded_angle_144,
        scale_154;
};
struct BodyBlendInput {
    float offset, minimum_yaw, maximum_yaw, base_yaw, feet_yaw;
    std::int32_t choked_commands, choke_limit;
};
// Native numeric/packet blend4332E973..4332EE37, exitingbeforecommand-yaw
// normalization. The integer truncation gate intentionally treats(-1,1)as0.
void blend_body_yaw(const BodyBlendSettings&, const BodyBlendInput&, float& command_yaw,
                    std::uint8_t& send_packet) noexcept;
struct BodyYawRequest {
    bool freestanding_body_yaw;
    std::int32_t frame_command, last_command, shot_command;
    RageVector frame_origin_dc;
    float command_pitch, base_yaw, maximum_yaw;
};
template <class Bones> struct BodyYawFreestandingBindingsFor {
    BodyFreestandingSettings settings;
    const BodyFreestandingContext& context;
    Bones& bones;
    const BodyFreestandingMath& math;
    const RageBoxClipper& clipper;
    BodyFreestandingWorkspace& workspace;
};
using BodyYawFreestandingBindings = BodyYawFreestandingBindingsFor<BodyFreestandingBones>;
using LiveBodyYawFreestandingBindings = BodyYawFreestandingBindingsFor<LiveBodyFreestandingSource>;
// Complete43365675 on native valid body modes1..3. Special bone/coverage
// solves execute before the regular branch; only actual engine calls are bound.
float solve_body_yaw(const BodyYawSettings&, BodyYawState&, const BodyYawRequest&,
                     const BodyYawFreestandingBindings&, const JitterRandom&);
float solve_body_yaw(const BodyYawSettings&, BodyYawState&, const BodyYawRequest&,
                     const LiveBodyYawFreestandingBindings&, const JitterRandom&);
} // namespace skeet::anti_aim
