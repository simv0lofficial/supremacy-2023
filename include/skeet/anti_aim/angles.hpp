#pragma once
#include "skeet/anti_aim/config.hpp"
#include "skeet/anti_aim/state.hpp"
#include "skeet/anti_aim/scheduler.hpp"
#include "skeet/anti_aim/pitch.hpp"
#include "skeet/anti_aim/body_yaw.hpp"
#include "skeet/anti_aim/edge.hpp"
#include <limits>

namespace skeet::anti_aim {
struct AngleParameters {
    float value_134 = 60.0f, value_138 = 180.0f, value_13c = 118.0f, value_140 = 58.0f;
    float value_144 = 179.0f, value_148 = 90.0f, value_14c = 20.0f, value_150 = 89.0f,
          value_154 = 0.75f;
    std::array<float, 12> pattern_15c{-1, 1, 0, -1, 1, -1, 0, 1, -1, 0, 1, 0};
};
struct AngleStageState : State {
    float cached_yaw_18c = std::numeric_limits<float>::max();
    float saved_yaw_198 = 0.0f, reference_yaw_258 = 0.0f;
};
struct AngleFrame {
    ActionGateFrame gates;
    float scalar_c0, scalar_38, height_164, angle_9c, angle_b8, angle_a0, interpolation_158;
    BallisticVector origin_10c, origin_168, origin_dc;
    std::int32_t command_ac;
};
struct AnglePlayer {
    ActionGatePlayer gates;
    float edge_scalar_3eb71843;
    std::uint8_t state_7501cb91;
    float weapon_pitch_340;
    AnimationYawInput animation;
    float feet_yaw_80;
};
template <class Body> struct AngleStageBindingsFor {
    ProximityEngine& proximity;
    EdgeEngine& edge;
    BallisticEngine& ballistics;
    const FreestandingContext& freestanding;
    const Body& body;
    const PitchInterfaces& pitch;
    const JitterRandom& random;
    std::uint8_t* trace_flag;
    std::int32_t last_body_command, shot_command, last_sent_command;
    bool shot_correction;
};
using AngleStageBindings = AngleStageBindingsFor<BodyYawFreestandingBindings>;
using LiveAngleStageBindings = AngleStageBindingsFor<LiveBodyYawFreestandingBindings>;
// Complete main angle stage4332D3D2..4332EF25, after targetrefresh. It calls
// concrete action/pitch/edge/freestanding/yaw/jitter/body solvers in nativeorder.
// The separate eligibility/cycle prefix runs before this stage.
void process_angle_stage(const GlobalConfig&, const Config&, const AngleParameters&,
                         AngleStageState&, AngleFrame&, const AnglePlayer&, ActionGateGlobals&,
                         client::Command&, std::uint8_t& send_packet, const AngleStageBindings&);
void process_angle_stage(const GlobalConfig&, const Config&, const AngleParameters&,
                         AngleStageState&, AngleFrame&, const AnglePlayer&, ActionGateGlobals&,
                         client::Command&, std::uint8_t& send_packet,
                         const LiveAngleStageBindings&);
} // namespace skeet::anti_aim
