#pragma once
#include "targeting.hpp"
#include "ballistics.hpp"
#include "skeet/rage_hitchance.hpp"
#include "skeet/rage_safety.hpp"
#include "skeet/rage_hitboxes.hpp"
#include "skeet/lagcomp/entity_state.hpp"
#include "skeet/rage_shot.hpp"
#include "skeet/movement/steering_math.hpp"
#include <atomic>
namespace skeet::engine {
class JobDispatcher;
}
namespace skeet::ui {
struct SettingsMenuValues;
}

namespace skeet::rage {
struct FinalTargetContextBindings;
namespace final_target_detail {
struct Producer;
}
// Complete direction-trace owner43364CEC. Unlike the point-to-point owner,
// an unobstructed trace is a miss. The supplied direction is not normalized.
std::int32_t trace_target_direction(BallisticEngine&, BallisticEntity source,
                                    const RageVector& origin, const RageVector& direction,
                                    const BallisticFilter&, BallisticQuery&);
// 433EB28B/433C8C4D: genuine weapon slots461(data),434(armor ratio),431(damage)
// and the decoded item-definition-index netvar. Query+24 carries pellet count.
BallisticQuery prepare_target_bullet(void* weapon, std::ptrdiff_t item_offset,
                                     std::uint8_t allow_penetration);
struct FinalHitchancePlan {
    std::uint32_t samples;
    std::uint32_t sample_stride;
    std::int32_t failure_budget;
};
RageSpreadBasis final_target_spread_basis(const RageVector& point,
                                          const RageVector& origin) noexcept;
// 4341BBC3..4341BC46; the required-vote conversion uses current MXCSR rounding.
FinalHitchancePlan
plan_final_hitchance(bool safe, float minimum_chance, std::uint32_t context_flags,
                     std::uint32_t entity_index,
                     const std::array<std::uint32_t, 2>& previous_targets) noexcept;
struct FinalHitchanceVotes {
    std::atomic<std::uint32_t> packed{0};
    std::atomic<std::uint32_t> stopped{0};
};
struct FinalHitchanceSample {
    void* weapon;
    std::ptrdiff_t weapon_item_offset;
    const std::uint8_t& allow_penetration; //live4346A3E0, including each dispatched sample
    BallisticEntity source;
    BallisticEntity target;
    RageVector origin;
    RageVector point;
    RageVector point_delta;
    RageSpreadBasis basis;
    std::int32_t minimum_damage;
    std::int32_t hitgroup;
    float minimum_hitgroup_multiplier;
    float base_damage;
    bool safe;
    RageResolverHitboxes hypotheses;
    RageBoxClipper box_clipper;
    BallisticFilter direction_filter;
    BallisticFilter projected_filter;
    // The full command owner supplies concrete engine bindings. A worker then
    // constructs its own native filter; the prebound API supplies filter views.
    const FinalTargetContextBindings* live_bindings = nullptr;
};
// Complete433DF8A0 sample worker. Every evaluated sample constructs its weapon
// query through the genuine virtual getters, then runs concrete ballistics.
// Safe samples contribute zero to three votes; regular samples zero or one.
void evaluate_final_hitchance_sample(BallisticEngine&, const FinalHitchanceSample&,
                                     RageSpreadSample, std::int32_t failure_budget,
                                     FinalHitchanceVotes&);
// 4341C7ED..4341C852: validates the shared counters and publishes the fraction.
// The boundary budget comparison is >=, while worker cancellation uses >.
RageHitchance final_hitchance_fraction(const FinalHitchanceVotes&,
                                       std::int32_t failure_budget) noexcept;
RageHitchance evaluate_final_hitchance_inline(BallisticEngine&, const FinalHitchanceSample&,
                                              std::span<const RageSpreadSample, 128>,
                                              const FinalHitchancePlan&);
RageHitchance evaluate_final_hitchance_parallel(BallisticEngine&, const FinalHitchanceSample&,
                                                std::span<const RageSpreadSample, 128>,
                                                const FinalHitchancePlan&, engine::JobDispatcher&);
struct FinalHitchanceFailure {
    std::uint32_t context_flags;
    std::uint32_t weapon_flags;
    void* weapon;
    std::int32_t hitgroup;
    std::array<float, 2> velocity;
    float baseline;
    float current;
    std::int32_t point_damage;
    std::int32_t minimum_damage;
    std::int32_t* target_aim_mode;
};
// Complete failed-consumer tail4341C858..4341C9C3, including the real weapon
// maximum-speed getter at slot442 and conditional target-history mode3 write.
void apply_final_hitchance_failure(FinalHitchanceFailure&);
struct FinalHitchanceContext {
    BallisticEngine& ballistics;
    const lagcomp::EntityStateRuntime& entity_runtime;
    void* target_player;
    BallisticEntity source;
    BallisticEntity target;
    void* weapon;
    std::ptrdiff_t weapon_item_offset;
    const std::uint8_t& allow_penetration; //live4346A3E0; outlives the completed sample jobs
    std::uint32_t weapon_flags;
    std::uint32_t& context_flags;
    std::array<std::uint32_t, 2> previous_targets;
    RageVector origin;
    std::array<float, 2> velocity;
    float baseline;
    float current;
    float spread;
    float inaccuracy;
    bool check_teammates;
    bool teammates_are_enemies;
    std::int32_t team;
    std::int32_t* target_aim_mode;
    std::span<const RageLocalHitbox> local_hitboxes;
    std::span<const std::uint32_t> hitbox_bones;
    const std::uint32_t* current_bone_counter;
    std::span<Matrix3x4> saved_bones;
    RageTrigonometry trigonometry;
    RageBoxClipper box_clipper;
    BallisticFilter direction_filter;
    BallisticFilter projected_filter;
    BallisticFilter teammate_filter;
    std::span<const RageSpreadSample, 128> samples;
    // Read at the native inline/threadpool branch, after model/record work.
    // The menu object aliases the actual byte; contexts never cache its value.
    const ui::SettingsMenuValues& settings_menu;
    // Runtime-owned dispatcher; it must outlive every submitted task.
    engine::JobDispatcher* jobs;
};
// 4341B4C2 consumer after the actual entity/studio-model engine reads have
// populated the typed inputs. Covers teammate rays, hitbox construction, record
// transaction, weighted jobs, publication and failure-state updates. A missing
// target preserves the caller's chance, as in the native entry.
bool evaluate_target_hitchance(FinalHitchanceContext&, const TargetCandidate&,
                               const lagcomp::LagRecord&, std::int32_t minimum_damage,
                               float minimum_chance, float& chance);
// Concrete4341B4C2 stage overload. The producer performs studio and player-cache
// reads at their native consumer sites, after the earlier engine trace stages.
bool evaluate_target_hitchance(FinalHitchanceContext&, const TargetCandidate&,
                               const lagcomp::LagRecord&, std::int32_t minimum_damage,
                               float minimum_chance, float& chance, final_target_detail::Producer&);
struct FinalTargetEvaluation {
    FinalHitchanceContext* context;
    const lagcomp::LagRecord* record;
    std::int32_t minimum_damage;
    float minimum_chance;
};
struct TargetShotChoice {
    TargetCandidate* candidate = nullptr;
    std::size_t ordinal = no_rage_point;
};
// Complete43354AD8..43354C61 selection loop: real final-hitchance calls,
// original alternate-point promotions, Revolver bypass and final fraction gate.
// evaluations correspond to the already sorted batch, preserving its ordering.
TargetShotChoice select_target_for_shot(TargetBatch&, std::span<const FinalTargetEvaluation>,
                                        std::uint32_t context_flags, std::uint32_t weapon_flags,
                                        std::int32_t weapon_id, float chance_floor,
                                        float point_penalty);
struct TargetAngleMath {
    double (*atan2)(double y, double x);
};
struct FinalTeammateConvars {
    const void* mode_43467c3c;
    const void* coefficient_43467084;
    const void* coefficient_43468c90;
    const void* teammates_are_enemies;
};
// Complete4341B7A5..4341B8B1 policy and terminal-parent ConVar reads.
bool final_teammate_precheck_required(std::uint32_t context_flags,
                                      const FinalTeammateConvars&) noexcept;
// 433551BA..433553BB: angle construction, conditional recoil subtraction and
// complete native angle normalization. compensate_recoil is the concrete
// 433EA238/category-policy +43467144 gate consumed by this block.
MovementAngles target_aim_angles(const RageVector& point, const RageVector& origin,
                                 const RageVector& recoil, bool compensate_recoil,
                                 std::uint32_t& weapon_flags, const TargetAngleMath&);
// 43355DEF..43355EA9 after the enclosing scheduler's firing gates admit the
// selected candidate. Preserves the native attack-button-before-weapon64 order.
RageShotCommit commit_target_choice(const TargetShotChoice&, RageShotState&, RageShotCommitState&,
                                    UserCommand&, float special_gate_value, bool alternate_path,
                                    bool send_mode) noexcept;
}
