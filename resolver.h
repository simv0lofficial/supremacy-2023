#pragma once

// Player Resolver Subsystem Declarations
#include "includes.h"
#include "skeet/resolver.hpp"
#include <array>
#include <cstdint>
#include "skeet/resolver_runtime/hypotheses.hpp"
#include "skeet/animation/bones.hpp"
#include "skeet/core.hpp"
#include <cmath>
#include <span>
#include "skeet/resolver_runtime/environment_update.hpp"
#include "skeet/rage/ballistics.hpp"
#include "skeet/rage_hitboxes.hpp"
#include "skeet/lagcomp/records.hpp"
#include "skeet/rage_geometry.hpp"
#include "skeet/resolver_runtime/attempts.hpp"
#include "skeet/resolver_runtime/environment_probe.hpp"
#include "skeet/resolver_runtime/environment_score.hpp"
#include <cstddef>
#include <algorithm>
#include <limits>
#include "skeet/rage_hitchance.hpp"
#include "skeet/engine/build_eye_adjustment_bindings.hpp"
#include <functional>
#include <variant>


// Declarations from resolver.hpp

namespace skeet {

struct ResolverVariant {
    float yaw;
    float pose_yaw;
    HitboxMatrices matrices;
};

struct ResolverRecord {
    float reference_yaw; // native LagRecord +0x04
    float observed_yaw; // +0x2C
    std::uint8_t variant_mode; // +0x58; understood values 0..3
    std::uint8_t selected_attempt; // +0x59
    std::uint32_t state_flags; // +0x5C; bit 0x80 enables variants
    float selected_yaw; // +0x28
    float selected_pose_parameter; // +0x1FC
    HitboxMatrices selected_matrices; // +0x230
    ResolverVariant first; // +0x5C0 (matrices +0x5D0)
    ResolverVariant center; // +0x960 (matrices +0x970)
    ResolverVariant third; // +0xD00 (matrices +0xD10)
};

float round_half_away_from_zero(float value) noexcept;
void select_resolver_variant(ResolverRecord& record, std::uint8_t attempt) noexcept;

} // namespace skeet

// Declarations from attempts.hpp

namespace skeet::lagcomp {
struct LagRecord;
}

namespace skeet::resolver_runtime {

struct RecordHistory {
    std::array<ResolverRecord*, 32> records;
    std::uint32_t first;
    std::uint32_t count;
};
struct LagRecordHistory {
    std::array<lagcomp::LagRecord*, 32> records;
    std::uint32_t first;
    std::uint32_t count;
};

struct SelectionState {
    // Native owner +10 + mode*24. Modes 1 and 2 share their winning attempt
    // when feedback publishes a new selection (+34 and +58 respectively).
    std::array<std::uint8_t, 4> attempt_by_mode;
    std::uint8_t status; // native owner +A0
};

// 0x43349D0C..0x43349D27: convert the winning hypothesis pose to attempt.
std::uint8_t classify_winning_pose(float pose_yaw) noexcept;

// 0x4336C64E..0x4336C674, before the selected lag record is copied.
void advance_pending_attempt(ResolverRecord& record, std::uint32_t options) noexcept;
// Direct adapter for the complete captured record layout, without projecting or
// copying the animation layers and unrelated capture fields.
void select_variant(lagcomp::LagRecord& record, std::uint8_t attempt) noexcept;
void advance_pending_attempt(lagcomp::LagRecord& record, std::uint32_t options) noexcept;

// Shared transition at 0x43349D92..0x43349E21 and
// 0x43413823..0x4341389A. Processing stops at the first incompatible record;
// flag 0x200 stops propagation after that record has been updated.
std::uint32_t propagate_attempt(RecordHistory& history, std::uint8_t mode,
                                std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept;
std::uint32_t propagate_attempt(LagRecordHistory& history, std::uint8_t mode,
                                std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept;

// The shot-feedback owner marks its source even when propagation stops early.
void apply_feedback(RecordHistory& history, ResolverRecord& source, std::uint8_t previous_attempt,
                    std::uint8_t new_attempt) noexcept;
void apply_feedback(LagRecordHistory& history, lagcomp::LagRecord& source,
                    std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept;

// 0x43349D69..0x43349E25: publish the selected attempt, synchronize the two
// movement modes, propagate it through compatible history, and mark the source.
void publish_feedback(SelectionState& state, RecordHistory& history, ResolverRecord& source,
                      std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept;
void publish_feedback(SelectionState& state, LagRecordHistory& history, lagcomp::LagRecord& source,
                      std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept;

} // namespace skeet::resolver_runtime

// Declarations from build.hpp

namespace skeet::lagcomp {
struct LagRecord;
}
namespace skeet::animation {
struct BoneRuntime;
}

namespace skeet::resolver_runtime {

struct HypothesisBuildContext {
    animation::BoneEntityState& entity;
    const animation::BoneModel& model;
    const RageVector& origin;
    std::span<float, 24> pose_parameters;
    const animation::BoneEngine& engine;
    const RageTrigonometry& trigonometry;
    bool strict = false;
};

// Full 0x433FAC08 orchestration: three hypotheses, the recovered skeletal
// builder for each, native early termination, and restoration of all 24 poses.
// The returned count is the number of successful candidate builds (0..3).
std::uint32_t build_hypotheses(ResolverRecord& record, const HypothesisInput& input,
                               HypothesisBuildContext& context);
std::uint32_t build_hypotheses(lagcomp::LagRecord& record, const HypothesisInput& input,
                               HypothesisBuildContext& context);
// Live433FAC08 composition. Every433FF63C call reads the supplied entity's
// current origin, model, studio set and genuine engine pose virtual; no model
// or BoneEntityState projection is retained between the three candidates.
std::uint32_t build_hypotheses(lagcomp::LagRecord& record, const HypothesisInput& input,
                               void* player, const animation::BoneRuntime& runtime,
                               bool strict = false);

} // namespace skeet::resolver_runtime

// Declarations from build_validation.hpp

namespace skeet::resolver_runtime {
inline bool valid_hypothesis_input(const HypothesisInput& value) noexcept {
    for (float field : std::array{
             value.speed_fraction, value.feet_yaw, value.movement_yaw, value.movement_yaw_target,
             value.duck_amount, value.running_fraction, value.ducking_fraction, value.minimum_yaw,
             value.maximum_yaw, value.eye_yaw, value.animation_fraction, value.velocity_x,
             value.velocity_y, value.velocity_z, value.lower_body_yaw, value.interval})
        if (!std::isfinite(field))
            return false;
    return value.interval > 0.0f && value.minimum_yaw < value.maximum_yaw;
}
inline bool valid_hypothesis(const Hypothesis& value) noexcept {
    return std::isfinite(value.yaw) && std::isfinite(value.pose_yaw) &&
           std::isfinite(value.body_pose_parameter) && std::isfinite(value.move_pose_parameter);
}
inline bool complete_hitbox_matrices(std::int32_t count,
                                     std::span<const Matrix3x4> matrices) noexcept {
    if (count != 19 || matrices.size() < 19)
        return false;
    for (const auto& matrix : matrices.first(19)) {
        for (const auto& row : matrix.rows)
            for (float value : row)
                if (!std::isfinite(value))
                    return false;
        const auto& a = matrix.rows;
        const float determinant = a[0][0] * (a[1][1] * a[2][2] - a[1][2] * a[2][1]) -
                                  a[0][1] * (a[1][0] * a[2][2] - a[1][2] * a[2][0]) +
                                  a[0][2] * (a[1][0] * a[2][1] - a[1][1] * a[2][0]);
        if (!std::isfinite(determinant) || std::fabs(determinant) < 1e-6f)
            return false;
    }
    return true;
}
}

// Declarations from environment_entry.hpp

namespace skeet::client {
struct PlayerState;
class PlayerStateCache;
}
namespace skeet::resolver_runtime {
struct EnvironmentTarget {
    std::uint32_t flags_38ca5228;
    std::uint8_t byte_043fa575, byte_334b460c;
    bool needs_update; // global4347BE0E, stride20, selected low16-bit index
    RageVector origin;
    const std::array<EnvironmentCapsule, 2>* hitboxes;
};
struct EnvironmentEntryEngine {
    void* context;
    bool (*enabled_68)(void*); // original owner+28, actual virtual +68
    EnvironmentTarget* (*lookup_10)(void*, std::int32_t); // entity-list virtual +10
    bool (*dormant_24)(void*, EnvironmentTarget&); // target networkable virtual +24
};
struct EnvironmentEntryInput {
    float enabled_cvar; // 43407DE2 decoded cvar value
    bool local_exists;
    std::uint8_t local_field_9d920e35;
    std::int32_t selected_entity; // native global4346DB1C
    std::span<const lagcomp::HistoryEvaluation> pending_shots;
    EnvironmentUpdateInput update;
};

// The projected entry/input overloads retain explicit logical inputs for
// semantic callers. The engine-owned HistoryOwner uses the live overload.
// Complete433279A4: pending shot flags20/40 and native target field+94.
bool has_pending_environment_shot(std::span<const lagcomp::HistoryEvaluation>,
                                  std::uint32_t selected_index) noexcept;
// Complete outer4334832C gates through43348558. Active selected indices are
// nonzero, as required by the native valid control-flow path at4334844C.
client::PlayerState* prepare_environment_update(client::PlayerStateCache&,
                                                const EnvironmentEntryInput&,
                                                const EnvironmentEntryEngine&);
// Complete4334832C: concrete shared cache, entry guards, environment probe,
// penetration/scoring and owner publication. Only genuine engine calls bind.
bool update_environment_entry(client::PlayerStateCache&, const EnvironmentEntryInput&,
                              const EnvironmentEntryEngine&, const EnvironmentEngine&);
// Live owner adapter after its actual engine/UI/local/entity/pending/adjustment
// gates and cache lookup. Shared owner gates and the complete body stay here.
bool update_environment_entry(client::PlayerState&, const EnvironmentLiveUpdateInput&,
                              const EnvironmentEngine&);
} // namespace skeet::resolver_runtime

// Declarations from environment_probe.hpp

namespace skeet::resolver_runtime {
struct EnvironmentRay {
    RageRay ray;
    RageVector start_offset;
    RageVector extents;
    bool is_ray;
    bool swept;
};
struct EnvironmentTraceRequest {
    EnvironmentRay ray;
    std::uint32_t mask;
    // Both native filters reject every entity (43382057); their trace types
    // differ: initial line=0 (43336942), hulls=1 (433686FC).
    std::uint32_t trace_type;
};
struct EnvironmentEngine {
    void* context;
    void (*trace_ray)(void*, const EnvironmentTraceRequest&, BallisticTrace&);
    std::uint32_t (*point_contents)(void*, const RageVector&, std::uint32_t mask);
    void (*view_angles)(void*, RageVector&); // genuine engine virtual +48
    BallisticEngine& penetration;
    RageTrigonometry trigonometry;
    std::uint8_t* suppression_flag = nullptr; // native global434683D0 pointed byte
    const std::uint8_t* rework = nullptr;
    bool reworked() const noexcept {
        return rework && *rework;
    }
};
struct EnvironmentProbe {
    bool selected;
    RageVector direction; // chosen vector before native scoring sign inversion
    std::array<RageVector, 2> shifted_target_points;
    std::array<std::int32_t, 2> damage;
};

// 433486FC..43349119 environment probing. Input origins are unraised; the
// native +64 eye offset is applied here. No internal ballistic call is bound
// through a callback: penetration uses the recovered 433C0AE9 implementation.
EnvironmentProbe probe_environment(const RageVector& local_origin, const RageVector& target_origin,
                                   const EnvironmentEngine&);
} // namespace skeet::resolver_runtime

// Declarations from environment_score.hpp

namespace skeet::resolver_runtime {
struct EnvironmentCapsule {
    std::uint32_t matrix_index;
    RageVector minimum;
    RageVector maximum;
    float radius;
};
struct EnvironmentSelection {
    float score;
    float pose_yaw;
    std::uint32_t attempt;
    bool valid() const noexcept {
        return std::isfinite(score);
    }
};
struct EnvironmentRebase {
    float yaw_delta;
    RageVector original_origin; // native selected record +30
    RageVector current_origin; // current engine entity origin
};
// Native 433492B0..43349D27 with matrices already expressed in the current
// entity space. The environment probe compares first and third hypotheses,
// takes the worse of two capsule projections, then selects the smaller score.
EnvironmentSelection score_environment_hypotheses(const lagcomp::LagRecord&,
                                                  const std::array<EnvironmentCapsule, 2>&,
                                                  const RageVector& direction,
                                                  const RageVector& origin,
                                                  bool strict = false) noexcept;
// The same complete solver including native 433492D1..433497DD matrix rebasing.
EnvironmentSelection score_environment_hypotheses(const lagcomp::LagRecord&,
                                                  const std::array<EnvironmentCapsule, 2>&,
                                                  const RageVector& direction,
                                                  const RageVector& origin,
                                                  const EnvironmentRebase&, const RageTrigonometry&,
                                                  bool strict = false);
} // namespace skeet::resolver_runtime

// Declarations from environment_update.hpp

namespace skeet::animation {
struct BoneRuntime;
}
namespace skeet::client {
struct PlayerState;
}
namespace skeet::resolver_runtime {
struct EnvironmentOwner {
    SelectionState selection;
    float probe_time; // native owner +B0
    float probe_stamp; // +B4
    std::int32_t probe_count; // +B8
    float reference_time; // +BC
    RageVector reference_origin; // +C0
    std::int32_t last_update_tick; // +15C
    float reference_yaw; // +1AC
};
struct EnvironmentUpdateInput {
    float local_time; // local entity field decoded with 9F8E7F71 (433FC87C)
    float current_time;
    std::int32_t command_tick;
    const std::int32_t* previous_command;
    const std::int32_t* current_command;
    RageVector local_origin;
    RageVector target_origin;
    // Native model hitboxes 0 and 3. Null represents an unavailable model/set.
    const std::array<EnvironmentCapsule, 2>* hitboxes;
};
struct EnvironmentLiveUpdateInput {
    void* target;
    const animation::BoneRuntime& bones;
    // Actual nullable4346A3A4 pointer chain. Both native433FC87C calls read it
    // at their own gate, rather than receiving a precomputed local clock.
    void* const* const& local_time_entity;
    std::ptrdiff_t local_time_offset; //9F8E7F71
    const float& current_time; //*4346A334
    const std::int32_t& command_tick;
    const std::int32_t* const& previous_command;
    const std::int32_t* const& current_command;
    const RageVector& local_origin; //43475BD4, read only at probe entry
    std::ptrdiff_t target_origin_offset; //A9EE6062
};

// Shared native owner/history/time gates43348478..43348558, after the entry's
// genuine engine, pending-shot and player-adjustment checks have succeeded.
bool environment_update_ready(EnvironmentOwner&, const LagRecordHistory&,
                              std::uint8_t resolver_index, const EnvironmentUpdateInput&) noexcept;
bool environment_update_ready(client::PlayerState&, const EnvironmentLiveUpdateInput&) noexcept;

// Complete environment update body 43348558..43349E47. The caller has passed
// the preceding engine/UI/pending-shot gates and supplies a nonempty history.
// Returns whether a new environment selection reached the publication stage.
bool update_environment(EnvironmentOwner&, LagRecordHistory&, const EnvironmentUpdateInput&,
                        const EnvironmentEngine&);
// Same body using the canonical owner's live fields and ring. Studio/set and
// hitboxes0/3 are read here, after the world probe; there is no model factory or
// internal lookup callback. Publication changes only the native written fields.
bool update_environment(client::PlayerState&, const EnvironmentLiveUpdateInput&,
                        const EnvironmentEngine&);
} // namespace skeet::resolver_runtime

// Declarations from hypotheses.hpp

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

// Declarations from movement_fit.hpp

namespace skeet::resolver_runtime {
struct MovementPoseFit {
    float pose = 0.0f, error = std::numeric_limits<float>::infinity();
    unsigned branch = 0, evaluations = 0;
    bool identified = false;
};
// Invert the model's displacement curve, without fitting across players or
// learning from shots. Multiple compatible yaw intervals remain unresolved.
template <class Residual>
MovementPoseFit fit_movement_pose(float minimum, float maximum, unsigned branches, float tolerance,
                                  Residual residual) {
    MovementPoseFit best;
    if (!std::isfinite(minimum) || !std::isfinite(maximum) || !(maximum > minimum) ||
        branches < 1 || branches > 4 || !(tolerance > 0.0f))
        return best;
    struct Solution {
        float pose, error;
        unsigned branch;
    };
    std::array<Solution, 12> solutions{};
    unsigned count = 0;
    const auto evaluate = [&](float pose, unsigned branch) {
        ++best.evaluations;
        const float error = residual(pose, branch);
        return std::isfinite(error) && error >= 0.0f ? error
                                                     : std::numeric_limits<float>::infinity();
    };
    constexpr unsigned intervals = 16;
    const float step = (maximum - minimum) / float(intervals);
    for (unsigned branch = 0; branch < branches; ++branch) {
        std::array<float, intervals + 1> errors{};
        for (unsigned i = 0; i <= intervals; ++i)
            errors[i] = evaluate(minimum + float(i) * step, branch);
        unsigned minima = 0;
        for (unsigned i = 0; i <= intervals; ++i) {
            if ((i && errors[i] > errors[i - 1]) || (i < intervals && errors[i] > errors[i + 1]))
                continue;
            // A flat or highly oscillatory curve does not identify one pose.
            if (++minima > 3 || count == solutions.size())
                return best;
            float low = minimum + float(i ? i - 1 : 0) * step;
            float high = minimum + float(std::min(i + 1, intervals)) * step;
            float pose = minimum + float(i) * step, error = errors[i];
            for (unsigned iteration = 0; iteration < 8; ++iteration) {
                const float left = low + (high - low) / 3.0f, right = high - (high - low) / 3.0f;
                const float a = evaluate(left, branch), b = evaluate(right, branch);
                if (a < error) {
                    pose = left;
                    error = a;
                }
                if (b < error) {
                    pose = right;
                    error = b;
                }
                if (a <= b)
                    high = right;
                else
                    low = left;
            }
            solutions[count++] = {pose, error, branch};
            if (error < best.error) {
                best.pose = pose;
                best.error = error;
                best.branch = branch;
            }
        }
    }
    if (!(best.error <= tolerance))
        return best;
    for (unsigned i = 0; i < count; ++i)
        if (solutions[i].error <= best.error + tolerance &&
            (solutions[i].branch != best.branch || std::fabs(solutions[i].pose - best.pose) > 8.0f))
            return best;
    // Also reject shallow minima: low residual alone cannot constrain yaw.
    for (float side : {-8.0f, 8.0f}) {
        const float probe = best.pose + side;
        if (probe >= minimum && probe <= maximum &&
            evaluate(probe, best.branch) <= best.error + tolerance)
            return best;
    }
    best.identified = true;
    return best;
}
}

// Declarations from next_attempt.hpp

namespace skeet::resolver_runtime {

struct AttemptOwner {
    float previous_selection_time; // native owner +88
    float previous_pose_time; // native owner +1A0
};
struct AttemptShot {
    const lagcomp::LagRecord& record; // native shot +180
    std::uint32_t prefix_flags; // native shot +00
    std::uint32_t shot_flags; // native shot +C4
    std::uint32_t target_handle; // native shot +94
    float distance; // native shot +74
};
struct AttemptTarget {
    bool hidden; // entity byte decoded with 9D920E35
    float* animation_adjustment; // animation state +80; null means no state
    const void* studio_model; // non-null only when wrapper and header exist
    bool hitbox_set;
    std::uint32_t hitbox_count;
    float body_pose_parameter; // entity pose parameter 11
    std::array<const void*, 19> hitboxes;
};
struct AttemptEngine {
    void* context;
    // Actual entity-list virtual +0C, receiving the native target handle.
    AttemptTarget* (*lookup_handle)(void*, std::uint32_t);
    // Actual engine target 43468CA4, called through 433A0EC6.
    float (*convert_pose_parameter)(void*, const void* studio_model, std::uint32_t, float);
    RageBoxClipper clipper;
};
// The live4337CB8A path obtains each field only after its preceding gates.
// A semantic AttemptTarget projection cannot preserve those partial reads.
struct RawAttemptEngine {
    void* const& entity_list;
    float* const& current_time; //4346A334, read after the actual lookup
    std::ptrdiff_t hidden, animation_state, studio_wrapper, pose_parameters, hitbox_set;
    const engine::PoseRangeRuntime& pose;
    RageBoxClipper clipper;
};
class AttemptEngineBinding {
    std::variant<std::reference_wrapper<const AttemptEngine>,
                 std::reference_wrapper<const RawAttemptEngine>>
        source_;

public:
    AttemptEngineBinding(const AttemptEngine& source) noexcept : source_(std::cref(source)) {}
    AttemptEngineBinding(const RawAttemptEngine& source) noexcept : source_(std::cref(source)) {}
    const auto& source() const noexcept {
        return source_;
    }
};

// Complete 433D725B. Uses all 19 model descriptors and the corresponding
// hypothesis matrix; the trace persists between failed engine clip calls.
bool intersects_hypothesis(const RageRay&, const std::array<const void*, 19>& hitboxes,
                           const HitboxMatrices&, const RageBoxClipper&);

// Complete 4337CB8A. The mode-0 path updates animation_adjustment; other modes
// only choose the next attempt. start/direction are nullable native arguments.
std::uint8_t choose_next_attempt(const AttemptOwner&, const AttemptShot&, float current_time,
                                 bool geometry_enabled, const RageVector* start,
                                 const RageVector* direction, const AttemptEngine&);
std::uint8_t choose_next_attempt(const AttemptOwner&, const AttemptShot&, float current_time,
                                 bool geometry_enabled, const RageVector* start,
                                 const RageVector* direction, const RawAttemptEngine&);
std::uint8_t choose_next_attempt(const AttemptOwner&, const AttemptShot&, float current_time,
                                 bool geometry_enabled, const RageVector* start,
                                 const RageVector* direction, const AttemptEngineBinding&);

} // namespace skeet::resolver_runtime
