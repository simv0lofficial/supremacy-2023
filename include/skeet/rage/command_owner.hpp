#pragma once
#include "command_phase2.hpp"
#include "command_phase6.hpp"
#include "command_phase8.hpp"
#include "multiple_targets.hpp"
#include "preamble_tail.hpp"
#include "targeting_acquired.hpp"

namespace skeet::rage {
struct FinalTargetContextBindings;
struct PostFiringBindings;
// One owner composes the recovered secondary-vtable phases. All referenced
// bindings describe the current engine snapshot and share the same canonical
// state, acquisition storage, player cache and runtime-owned job dispatcher.
// ScanContext.configuration_flags aliases state.flags; its target_mask aliases
// state.previous_targets (+1A4), distinct from grouping's state.target_mask.
// readiness.prediction_time and preamble.prediction_time reference one clock.
struct CommandOwner {
    PreambleState& state;
    AcquisitionStorage& acquisition;
    const PreambleBindings& preamble;
    const SingleTargetBindings& single_target;
    const MultipleTargetBindings& multiple_targets;
    const FinalTargetContextBindings& final_target;
    FiringSharedState& firing_shared;
    const FiringReadinessBindings& readiness;
    const FiringPostAngleBindings& post_angles;
    const PostFiringBindings& post_firing;
    const PreambleTailBindings& tail;
    const CommandPhase2Bindings& phase2;
    const CommandPhase6Bindings& phase6;
    const ui::SettingsMenuValues& settings_menu; // canonical MISC Settings byte owners
};
// Native43358716, called before CommandFrame construction.
void process_rage_command_phase2(CommandOwner&, void* player, void* weapon, client::Command&,
                                 std::array<float, 3>& incoming_angles, std::uint8_t& send_packet);
// Complete phase4 owner4334F55F. The selected historical frame/command flow
// through acquisition, lazy final evaluation, firing, continuation and tail.
// send_packet is the live byte represented by the frame's native+8 pointer.
void process_rage_command(CommandOwner&, client::CommandFrame&, void* player, client::Command&,
                          std::uint8_t& send_packet);
// Native4335CBFB and433B76DE use the same live global angle pair/frame ring.
void process_rage_phase6(CommandOwner&, const client::CommandFrame&, const client::Command&);
void capture_rage_command_frame(CommandOwner&, const client::CommandFrame&) noexcept;
} // namespace skeet::rage
