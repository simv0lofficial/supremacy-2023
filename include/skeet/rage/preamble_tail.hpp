#pragma once
#include "preamble.hpp"
#include "action_state.hpp"
#include "skeet/client/command_transform.hpp"
#include "skeet/client/verified_command.hpp"
#include "skeet/legit/runtime.hpp"
#include "skeet/movement/simulation_capture.hpp"
#include "skeet/movement/simulation_environment.hpp"

namespace skeet::rage {
struct PreambleTailBindings {
    const PreambleBindings& shared;
    esp::HitboxHighlightState& action_state;
    const ActionStateBindings& action;
    const legit::Settings* legit_settings; // nullable native4347588C
    TargetAngleMath angle_math; // genuine msvcrt atan2
    float (*arctangent)(float); // __libm_sse2_atanf
    void* input; // actual CInput virtual8
    FiringPlayerOffsets player_offsets;
    const client::CommandButtonPolicy& button_policy;
    client::CommandTrigonometry command_trigonometry;
    client::ScriptCommandEnvironment command_environment;
    std::span<client::VerifiedCommand, 150> verified_commands; // CInput+F4
    client::CommandChecksum checksum; // genuine optional43467C48
    void* prediction; // engine owner43469264
};
enum class PreambleTailEntry {
    after_firing, //433576F0
    actions, //43357733
    reconcile_command, //433583F2
    prediction_clock, //433584E5
    return_only, //43358507
};
struct PreambleTailResult {
    bool shot_marker_updated = false;
    bool historical_command_published = false;
};
// Complete43389A9A. This keeps the two discarded RNG draws per pellet and
// averages only the engine shotgun pattern component, without inaccuracy.
RageSpreadSample preamble_pattern_correction(const SpreadParameters&, std::uint32_t seed,
                                             const SpreadRandom&, const ShotgunPattern&,
                                             const RageTrigonometry&);
// Complete inline exits433576F0..4335850C. Action visualization calls the
// concrete433320D4 owner; simulation, angle limits and command finalizers reuse
// their actual recovered modules. The explicit entry identifies the native
// caller jump, so return-only paths do not run action/reconciliation work.
PreambleTailResult finish_rage_command(PreambleState&, PreambleResult&,
                                       client::CommandFrame& original, void* local_player,
                                       const PreambleTailBindings&, PreambleTailEntry);
} // namespace skeet::rage
