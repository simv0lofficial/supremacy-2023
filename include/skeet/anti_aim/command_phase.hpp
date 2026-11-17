#pragma once
#include "skeet/anti_aim/angles.hpp"
#include "skeet/anti_aim/eligibility.hpp"
#include "skeet/anti_aim/targets.hpp"

namespace skeet::anti_aim {
struct CommandPhaseInput {
    std::uint32_t secondary_frame_flags;
    bool player_a15c3105;
    EligibilityGuard guard;
};
struct CommandPhaseBindings {
    const EligibilityBindings& eligibility;
    const TargetRefreshContext& target_refresh;
    const AngleStageBindings& angles;
};
// Complete phase4 owner4332D1C6. The UI yaw enum has native range0..5.
// TargetState is the shared4346DB14 publication also consumed by RAGE/packets.
void process_command_angles(const GlobalConfig&, const Config&, const AngleParameters&,
                            AngleStageState&, AngleFrame&, const AnglePlayer&, ActionGateGlobals&,
                            client::Command&, std::uint8_t& send_packet, TargetState&,
                            const CommandPhaseInput&, const CommandPhaseBindings&);
} // namespace skeet::anti_aim
