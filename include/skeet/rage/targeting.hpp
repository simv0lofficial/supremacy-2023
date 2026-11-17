#pragma once
#include "skeet/rage_geometry.hpp"
#include "skeet/rage_selection.hpp"
#include "skeet/rage_policy.hpp"
#include "skeet/lagcomp/records.hpp"
#include "skeet/players/state.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet::rage {
// The explicit decoded object profile is retained for standalone semantic
// inputs. Production supplies a live canonical reference, including its XOR
// representation; copying a TargetBindings keeps that same reference live.
using TargetConVarBinding = engine::ConVarBinding;

struct TargetOffsets {
    // Decoded values of 43468E78, 4346A5DC, 43468BF8 and 43467DD0.
    // Their byte tests are recovered; no unproved netvar names are assigned.
    std::array<std::ptrdiff_t, 4> rejection_bytes;
    std::ptrdiff_t team;
    std::ptrdiff_t survival_team;
    std::ptrdiff_t movement_type;
    std::ptrdiff_t movement_flags;
    std::ptrdiff_t origin;
    std::ptrdiff_t duck_amount;
    std::ptrdiff_t active_weapon;
    std::ptrdiff_t weapon_identifier;
};
struct TargetBindings {
    void* local_player;
    void* entity_list;
    // Native ConVar objects. Parent pointer +0x1C, encoded integer +0x30.
    TargetConVarBinding game_type;
    TargetConVarBinding teammates_are_enemies;
    // Original 0x20-byte per-player table, shared with PLAYERS/plist.
    const players::State& player_adjustments;
    TargetOffsets offsets;
};

// 4339EF8C: complete team/survival-team predicate, including ConVar parents.
bool target_is_enemy(const TargetBindings&, const void* target) noexcept;
// 43358962: calls entity slots 158/122, networkable slots 9/10, then the
// recovered enemy predicate. Only actual engine virtuals remain dependencies.
bool target_is_eligible(const TargetBindings&, void* target);
// 433849B5: complete smoothstep duck interpolation of native eye height.
struct TargetEyePositionOffsets {
    std::ptrdiff_t origin;
    std::ptrdiff_t duck_amount;
};
RageVector target_eye_position(TargetEyePositionOffsets, const void* target) noexcept;
RageVector target_eye_position(const TargetBindings&, const void* target) noexcept;
struct TargetView {
    RageVector origin;
    RageVector forward;
    std::int32_t field_of_view;
    float minimum_cosine;
};
// 43351D0E..43351E2D, after the eligibility predicate succeeds. The native
// field-of-view value 180 skips the eye-position and cosine calculation.
bool target_passes_scan_filter(const TargetBindings&, const void* target,
                               const TargetView&) noexcept;

struct TargetPoint {
    RageVector position;
    RageVector reference_position;
    std::int32_t reference_damage;
    std::int32_t damage;
    std::int32_t hitbox; // native point+20 / candidate+2C, studio hitbox index
    std::int32_t hitgroup;
    std::uint32_t flags;
    float chance;
    float descriptor_weight;
    std::uint32_t priority;
};
static_assert(sizeof(TargetPoint) == 0x38);
struct TargetCandidate {
    std::uint32_t flags;
    std::uint32_t entity_index;
    std::int32_t health;
    TargetPoint primary;
    TargetPoint alternate_first;
    TargetPoint alternate_second;
    std::uint32_t priority;
    std::uint32_t target_state_flags;
    bool prefer_body;
    bool prefer_safe;
    bool weapon_priority_enabled;
};
struct TargetBatch {
    std::array<TargetCandidate*, 16> candidates{};
    std::size_t count = 0;
};
// 4336070E..43360869, copying all 14 dwords of each selected point into the
// candidate. Unassigned primary/alternate slots retain their previous bytes.
void select_target_points(TargetCandidate&, std::span<const TargetPoint>,
                          std::uint32_t priority_mask);
struct TargetRecordPolicy {
    RageBodyAimPolicy body;
    std::uint32_t evaluation_flags;
    bool delay_shot;
};
// 43351A28..43351B6D. False stops the remaining record traversal on an
// alternative-record result after ordinal zero, before policy mutation.
bool prepare_target_record_policy(TargetRecordPolicy&, std::uint32_t record_flags,
                                  std::uint32_t ordinal, std::uint8_t body_disablers) noexcept;
// 433547C9..43354908. Traverses exactly 80 completion bits, retains the
// highest scan priority per (entity_index & 63), and admits at most 16 groups.
// Equal priority preserves the earlier candidate. Existing groups can still
// be replaced after all 16 slots have been occupied.
TargetBatch collect_target_candidates(std::span<TargetCandidate, 80>,
                                      const std::array<std::uint32_t, 3>& completed,
                                      std::uint32_t context_flags,
                                      std::array<std::uint32_t, 2>& target_mask) noexcept;
// 43354925..43354AA0. Recalculates priorities after grouping. Weapon lookup
// is performed through entity-list slots 3 and 4 only when its gate is set.
void recalculate_target_priorities(TargetBatch&, const TargetBindings&);
// Native 433FF5C7 comparator passed to the host CRT qsort, as in 43354ABE.
void sort_target_candidates(TargetBatch&);
// Concrete post-scan chain 433547C9..43354AD8: group by target, refresh the
// surviving record priorities using engine weapon state, and sort them.
TargetBatch rank_completed_target_candidates(std::span<TargetCandidate, 80>,
                                             const std::array<std::uint32_t, 3>& completed,
                                             std::uint32_t context_flags,
                                             std::array<std::uint32_t, 2>& target_mask,
                                             const TargetBindings&);

struct TargetRecordHistory {
    RageVector origin;
    std::uint32_t first;
    std::uint32_t count;
    std::array<const lagcomp::LagRecordHeader*, 32> records;
};
struct TargetRecordChoice {
    const lagcomp::LagRecordHeader* record = nullptr;
    std::uint32_t ordinal = 0;
};
// 4336C59D..4336C64E, the opening traversal of history_select_lag_record.
// Compares consecutive origins, rejecting differences above 4096 squared,
// then chooses the first simulation tick no newer than the requested tick.
// A request older than the ring selects its last record. Record interpolation,
// resolver mutation, extrapolation and finalization follow this stage.
TargetRecordChoice choose_target_record(const TargetRecordHistory&, std::int32_t requested_tick,
                                        float interval) noexcept;
struct TargetLagRecordHistory {
    RageVector origin;
    std::uint32_t first;
    std::uint32_t count;
    std::array<lagcomp::LagRecord*, 32> records;
};
struct TargetRecordCopy {
    lagcomp::LagRecord* source = nullptr;
    std::uint32_t ordinal = 0;
};
// 4336C572..4336C692: complete opening selection/copy stage. Clears the
// output flags and destination source dword before traversal, advances a
// pending resolver attempt on the chosen source, then copies all 0x10A0 bytes.
// The destination's first dword becomes the native 32-bit source address.
TargetRecordCopy prepare_target_record_copy(TargetLagRecordHistory&, std::int32_t requested_tick,
                                            float interval, std::uint32_t options,
                                            lagcomp::LagRecord& destination,
                                            std::uint32_t& output_flags) noexcept;

} // namespace skeet::rage
