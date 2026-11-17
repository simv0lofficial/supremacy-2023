#pragma once
#include "accuracy_history.hpp"
#include "studio.hpp"
#include "recoil_state.hpp"
#include "skeet/client/command_frame.hpp"
#include "skeet/lagcomp/record_selection_live.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet::legit {
// Complete43366128 basis, using the actual scalar CRT import bindings.
RageSpreadBasis spread_basis(const Vector& angles, const RageTrigonometry&);
// Complete43398F25:32 normalized directions, native permutation stride4,
// one pellet, and the exact Revolver spread/inaccuracy override.
std::array<Vector, 32> accuracy_spread_directions(const rage::SpreadParameters&,
                                                  std::uint32_t weapon_flags_7c,
                                                  const RageSpreadBasis&, const rage::SpreadTables&,
                                                  const rage::SpreadRandom&,
                                                  const rage::ShotgunPattern&,
                                                  const RageTrigonometry&);
struct AccuracyEnvironment {
    const rage::TargetBindings& targeting;
    const lagcomp::EntityStateRuntime& entity;
    const lagcomp::RecordSelectionLiveContext& selection;
    std::ptrdiff_t player_punch;
    engine::ConVarBinding recoil_scale;
    const std::uint8_t& suppress_recoil_43467144;
    const std::uint8_t& normalize_angles_434693ec;
    // Actual clocks, reloaded on their respective sides of engine calls.
    const float* time;
    const float* interpolation_43476928;
    const float* interval;
    const std::int32_t* maximum_clients;
    std::span<Matrix3x4, 256> saved_bones;
    AccuracyMatrixScratch& matrix_scratch;
    BallisticEngine& ballistics;
    const rage::SpreadTables& spread_tables;
    const rage::SpreadRandom& random;
    const rage::ShotgunPattern& shotgun_pattern;
    engine::ConVarBinding shotgun_patterns_convar; //live43468520 parent chain
    const lagcomp::RotationMath& rotation;
    std::span<const TargetTrack, 64> targets;
    TriggerTraceBindings trace;
    void* const* trace_filter_vtable; // actual external43475B90, slots0..4
    const anti_aim::TargetTraceEntities& trace_entities;
};
// Complete4332FD2B composition. An optional hit byte selects trigger semantics;
// it is only set on success and otherwise retains its incoming value. The
// original entity scope restores even when no history tick was selected.
std::int32_t accuracy_tick(ProfileState&, const client::CommandFrame&, void* local_player,
                           const client::Command&, bool* hit, const AccuracyEnvironment&);
} // namespace skeet::legit
