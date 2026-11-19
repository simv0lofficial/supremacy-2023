#include "skeet/lagcomp/record_validity.hpp"
#include "skeet/resolver_runtime/jitter.hpp"
// Player Resolver Subsystem Implementation
#include "skeet/resolver.hpp"
#include <bit>
#include <cmath>
#include "skeet/resolver_runtime/attempts.hpp"
#include "skeet/resolver_runtime/build.hpp"
#include "skeet/resolver_runtime/build_validation.hpp"
#include "skeet/lagcomp/records.hpp"
#include <cstring>
#include "skeet/resolver_runtime/environment_entry.hpp"
#include "skeet/client/player_state.hpp"
#include <cassert>
#include "skeet/resolver_runtime/environment_probe.hpp"
#include "skeet/rage/point_penetration.hpp"
#include <xmmintrin.h>
#include "skeet/resolver_runtime/environment_score.hpp"
#include <limits>
#include "skeet/resolver_runtime/environment_update.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/resolver_runtime/hypotheses.hpp"
#include <cstdint>
#include "skeet/animation/record_pose_scope.hpp"
#include "skeet/resolver_runtime/next_attempt.hpp"
#include "skeet/engine/interface_call.hpp"


// ============================================================================
// Subcomponent: resolver.cpp
// ============================================================================

namespace skeet {

// 0x4341AD12..0x4341ADBD. Preserve the binary's ceil/difference test.
float round_half_away_from_zero(float value) noexcept {
    const bool negative = value < 0.0f;
    const float magnitude = negative ? -value : value;
    float rounded = static_cast<float>(std::ceil(static_cast<double>(magnitude)));
    if (rounded - magnitude > 0.5f)
        rounded -= 1.0f;
    return negative ? -rounded : rounded;
}

// Complete function 0x433AB612..0x433AB7BE, with helper calls recovered in C++.
void select_resolver_variant(ResolverRecord& record, std::uint8_t attempt) noexcept {
    if (!(record.state_flags & 0x80) || record.variant_mode == 0 || record.variant_mode > 3)
        return;

    const auto cycle = attempt % 3;
    const ResolverVariant* variant;
    if (record.variant_mode < 3) {
        // Native table 0x4347223C contains {0,3,3,3}.
        variant = cycle == 0 ? &record.center : cycle == 1 ? &record.third : &record.first;
    } else {
        float delta = record.observed_yaw - record.reference_yaw;
        if (std::isnan(delta))
            delta = 0.0f;
        else if (std::fabs(delta) > 180.0f) {
            const float turns = round_half_away_from_zero(std::fabs(delta / 360.0f));
            const float correction = turns * 360.0f;
            delta = delta < 0.0f ? delta + correction : delta - correction;
        }
        const bool positive = delta > 0.0f;
        if (cycle == 0)
            variant = positive ? &record.first : &record.third;
        else if (cycle == 1)
            variant = positive ? &record.third : &record.first;
        else
            variant = &record.center;
    }

    float pose = (variant->pose_yaw - (-60.0f)) / 120.0f;
    pose += 0.0f;
    // MAXSS/MINSS select the second operand on unordered comparisons.
    pose = pose > 0.0f ? pose : 0.0f;
    pose = pose < 1.0f ? pose : 1.0f;
    record.selected_pose_parameter = pose;
    record.selected_yaw = variant->yaw;
    copy_hitbox_matrices(variant->matrices, record.selected_matrices);
    record.selected_attempt = attempt;
}

} // namespace skeet

// ============================================================================
// Subcomponent: attempts.cpp
// ============================================================================

namespace skeet::resolver_runtime {

std::uint8_t classify_winning_pose(float pose_yaw) noexcept {
    if (pose_yaw < -20.0f)
        return 2;
    return pose_yaw > 20.0f ? 1 : 0;
}

void advance_pending_attempt(ResolverRecord& record, std::uint32_t options) noexcept {
    if ((options & 4U) != 0 && (record.state_flags & 0xc0U) == 0xc0U) {
        record.state_flags &= ~0x40U;
        select_resolver_variant(record, static_cast<std::uint8_t>(record.selected_attempt + 1U));
    }
}

std::uint32_t propagate_attempt(RecordHistory& history, std::uint8_t mode,
                                std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept {
    if (previous_attempt == new_attempt)
        return 0;
    std::uint32_t processed = 0;
    for (std::uint32_t index = 0; index != history.count; ++index) {
        auto& record = *history.records[(history.first + index) & 31U];
        if ((record.state_flags & 0x22U) != 0 || record.variant_mode != mode ||
            record.selected_attempt != previous_attempt)
            break;
        select_resolver_variant(record, new_attempt);
        record.state_flags = (record.state_flags & ~0x40U) | 2U;
        ++processed;
        if ((record.state_flags & 0x200U) != 0)
            break;
    }
    return processed;
}

void apply_feedback(RecordHistory& history, ResolverRecord& source, std::uint8_t previous_attempt,
                    std::uint8_t new_attempt) noexcept {
    propagate_attempt(history, source.variant_mode, previous_attempt, new_attempt);
    source.state_flags |= 2U;
}

void publish_feedback(SelectionState& state, RecordHistory& history, ResolverRecord& source,
                      std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept {
    state.attempt_by_mode[source.variant_mode] = new_attempt;
    state.status = 1;
    if (source.variant_mode == 1)
        state.attempt_by_mode[2] = new_attempt;
    else if (source.variant_mode == 2)
        state.attempt_by_mode[1] = new_attempt;
    apply_feedback(history, source, previous_attempt, new_attempt);
}

} // namespace skeet::resolver_runtime

// ============================================================================
// Subcomponent: build.cpp
// ============================================================================

namespace skeet::resolver_runtime {
namespace {
struct RestorePoses {
    std::span<float, 24> destination;
    std::array<float, 24> saved;
    explicit RestorePoses(std::span<float, 24> poses) : destination(poses) {
        std::memcpy(saved.data(), poses.data(), sizeof(saved));
    }
    ~RestorePoses() {
        std::memcpy(destination.data(), saved.data(), sizeof(saved));
    }
};
}

std::uint32_t build_hypotheses(ResolverRecord& record, const HypothesisInput& input,
                               HypothesisBuildContext& context) {
    RestorePoses restore(context.pose_parameters);

    if (context.strict)
        record.state_flags &= ~0x80u;
    if (context.strict && !valid_hypothesis_input(input))
        return 0;

    auto parameters = input;
    parameters.record_flags = record.state_flags;
    const auto hypotheses = solve_hypotheses(parameters);
    const std::array<ResolverVariant*, 3> variants = {&record.first, &record.center, &record.third};
    std::uint32_t completed = 0;
    for (std::size_t index = 0; index != variants.size(); ++index) {
        auto& variant = *variants[index];
        const auto& hypothesis = hypotheses[index];
        if (context.strict && !valid_hypothesis(hypothesis))
            return completed;
        variant.yaw = hypothesis.yaw;
        variant.pose_yaw = hypothesis.pose_yaw;
        context.pose_parameters[11] = hypothesis.body_pose_parameter;
        context.pose_parameters[7] = hypothesis.move_pose_parameter;
        const auto count = animation::build_resolver_hitbox_matrices(
            context.entity, context.model, context.origin, variant.yaw, variant.matrices,
            context.engine, context.trigonometry);
        if (count <= 0 || (context.strict && !complete_hitbox_matrices(count, variant.matrices)))
            return completed;
        ++completed;
    }
    record.state_flags |= 0x80U;
    return completed;
}

std::uint32_t build_hypotheses(lagcomp::LagRecord& record, const HypothesisInput& input,
                               HypothesisBuildContext& context) {
    RestorePoses restore(context.pose_parameters);
    if (context.strict)
        record.header.flags &= ~0x80u;
    if (context.strict && !valid_hypothesis_input(input))
        return 0;
    auto parameters = input;
    parameters.record_flags = record.header.flags;
    const auto hypotheses = solve_hypotheses(parameters);
    std::uint32_t completed = 0;
    for (std::size_t index = 0; index != record.alternatives.size(); ++index) {
        auto& variant = record.alternatives[index];
        const auto& hypothesis = hypotheses[index];
        if (context.strict && !valid_hypothesis(hypothesis))
            return completed;
        variant.state[0] = std::bit_cast<std::uint32_t>(hypothesis.yaw);
        variant.state[1] = std::bit_cast<std::uint32_t>(hypothesis.pose_yaw);
        context.pose_parameters[11] = hypothesis.body_pose_parameter;
        context.pose_parameters[7] = hypothesis.move_pose_parameter;
        const auto count = animation::build_resolver_hitbox_matrices(
            context.entity, context.model, context.origin, hypothesis.yaw, variant.matrices,
            context.engine, context.trigonometry);
        if (count <= 0 || (context.strict && !complete_hitbox_matrices(count, variant.matrices)))
            return completed;
        ++completed;
    }
    record.header.flags |= 0x80U;
    return completed;
}

} // namespace skeet::resolver_runtime

// ============================================================================
// Subcomponent: environment_entry.cpp
// ============================================================================

namespace skeet::resolver_runtime {
bool has_pending_environment_shot(std::span<const lagcomp::HistoryEvaluation> pending,
                                  std::uint32_t selected_index) noexcept {
    for (const auto& shot : pending)
        if ((shot.words[0] & 0x20U) == 0 && shot.state.words[1] == selected_index &&
            (shot.words[0] & 0x40U) != 0)
            return true;
    return false;
}
namespace {
client::PlayerState* prepare(client::PlayerStateCache& cache, const EnvironmentEntryInput& input,
                             const EnvironmentEntryEngine& engine, EnvironmentTarget*& target) {
    if (!engine.enabled_68(engine.context) || input.enabled_cvar == 0.0f || !input.local_exists ||
        input.local_field_9d920e35 != 0 || input.selected_entity == -1)
        return nullptr;
    target = engine.lookup_10(engine.context, input.selected_entity);
    if (target == nullptr || engine.dormant_24(engine.context, *target) ||
        (target->flags_38ca5228 & 8U) != 0 || target->byte_043fa575 != target->byte_334b460c)
        return nullptr;
    const auto index = std::uint32_t(input.selected_entity) & 0xffffU;
    if (has_pending_environment_shot(input.pending_shots, index) || !target->needs_update)
        return nullptr;
    assert(index != 0);
    auto& player = *cache.get(index);
    auto owner = player.environment();
    const LagRecordHistory history{player.history.records, player.history.first,
                                   player.history.count};
    auto update = input.update;
    update.target_origin = target->origin;
    if (!environment_update_ready(owner, history, player.native.resolver_index, update))
        return nullptr;
    return &player;
}
}
client::PlayerState* prepare_environment_update(client::PlayerStateCache& cache,
                                                const EnvironmentEntryInput& input,
                                                const EnvironmentEntryEngine& engine) {
    EnvironmentTarget* target;
    return prepare(cache, input, engine, target);
}
bool update_environment_entry(client::PlayerStateCache& cache, const EnvironmentEntryInput& input,
                              const EnvironmentEntryEngine& gates,
                              const EnvironmentEngine& engine) {
    EnvironmentTarget* target;
    auto* player = prepare(cache, input, gates, target);
    if (player == nullptr)
        return false;
    auto owner = player->environment();
    LagRecordHistory history{player->history.records, player->history.first, player->history.count};
    auto request = input.update;
    request.target_origin = target->origin;
    request.hitboxes = target->hitboxes;
    const bool published = update_environment(owner, history, request, engine);
    player->environment(owner);
    return published;
}
bool update_environment_entry(client::PlayerState& player, const EnvironmentLiveUpdateInput& input,
                              const EnvironmentEngine& engine) {
    if (!environment_update_ready(player, input))
        return false;
    return update_environment(player, input, engine);
}
} // namespace skeet::resolver_runtime

// ============================================================================
// Subcomponent: environment_probe.cpp
// ============================================================================

namespace skeet::resolver_runtime {
namespace {
constexpr float epsilon = 1.1920928955078125e-7f;
RageVector subtract(const RageVector& a, const RageVector& b) {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}
RageVector normalize(const RageVector& value) {
    const float length =
        std::sqrt((value[0] * value[0] + value[1] * value[1]) + value[2] * value[2]);
    const float inverse = 1.0f / (length + epsilon);
    return {inverse * value[0], inverse * value[1], inverse * value[2]};
}
RageVector perpendicular(const RageVector& horizontal) {
    if (horizontal[0] == 0.0f && horizontal[1] == 0.0f)
        return {0.0f, -1.0f, 0.0f};
    const float vertical_product = horizontal[2] * 0.0f;
    return normalize({horizontal[1] - vertical_product, vertical_product - horizontal[0],
                      horizontal[0] * 0.0f - horizontal[1] * 0.0f});
}
EnvironmentRay ray(const RageVector& start, const RageVector& end, bool hull) {
    const auto delta = subtract(end, start);
    const bool swept = (delta[1] * delta[1] + delta[0] * delta[0]) + delta[2] * delta[2] != 0.0f;
    return {{start, delta},
            hull ? RageVector{-0.0f, -0.0f, -0.0f} : RageVector{},
            hull ? RageVector{16.0f, 16.0f, 2.0f} : RageVector{},
            !hull,
            swept};
}
struct Suppress {
    std::uint8_t* flag;
    std::uint8_t saved = 0;
    explicit Suppress(std::uint8_t* value) : flag(value) {
        if (flag != nullptr) {
            saved = *flag;
            *flag = 0;
        }
    }
    ~Suppress() {
        if (flag != nullptr)
            *flag = saved;
    }
};
}

EnvironmentProbe probe_environment(const RageVector& local_origin, const RageVector& target_origin,
                                   const EnvironmentEngine& engine) {
    Suppress suppress(engine.suppression_flag);
    EnvironmentProbe result{};
    const RageVector local{local_origin[0], local_origin[1], local_origin[2] + 64.0f};
    const RageVector target{target_origin[0], target_origin[1], target_origin[2] + 64.0f};
    const auto delta = subtract(target, local);
    BallisticTrace trace;
    engine.trace_ray(engine.context, {ray(local, target, false), 1, 0}, trace);
    if (trace.fraction == 1.0f && trace.start_solid == 0)
        return result;

    const auto direction = normalize(delta);
    RageVector angles;
    engine.view_angles(engine.context, angles);
    constexpr float radians = 0.01745329238474369f;
    const float sy = engine.trigonometry.sine(angles[1] * radians);
    const float cy = engine.trigonometry.cosine(angles[1] * radians);
    const float sp = engine.trigonometry.sine(angles[0] * radians);
    const float cp = engine.trigonometry.cosine(angles[0] * radians);
    const float facing = ((cp * sy) * direction[1] + (cp * cy) * direction[0]) - direction[2] * sp;
    if (!(facing >= 0.0f))
        return result;

    const auto horizontal = normalize({delta[0], delta[1], 0.0f});
    const auto side_direction = perpendicular(horizontal);
    constexpr float signs[2] = {1.0f, -1.0f};
    BallisticTrace hull_trace;
    for (std::size_t side = 0; side != 2; ++side) {
        RageVector endpoint;
        for (std::size_t axis = 0; axis != 3; ++axis)
            endpoint[axis] = side_direction[axis] * signs[side] * 48.0f + target[axis];
        const RageVector hull_start{target[0] + 0.0f, target[1] + 0.0f, target[2] + 0.0f};
        engine.trace_ray(engine.context, {ray(hull_start, endpoint, true), 1, 1}, hull_trace);
        result.shifted_target_points[side] =
            hull_trace.fraction < 1.0f || hull_trace.all_solid != 0 || hull_trace.start_solid != 0
                ? hull_trace.end
                : endpoint;
    }

    bool blocked[2] = {false, false};
    std::int32_t combined = 0;
    float offset = 0.0f;
    for (std::uint32_t step = 0; step != 3; ++step) {
        for (std::size_t side = 0; side != 2; ++side) {
            if (blocked[side])
                continue;
            const float distance = signs[side] * offset;
            RageVector sample;
            for (std::size_t axis = 0; axis != 3; ++axis)
                sample[axis] = side_direction[axis] * distance + local[axis];
            if ((engine.point_contents(engine.context, sample, 1) & 1U) != 0) {
                blocked[side] = true;
                if (blocked[0] && blocked[1]) {
                    if (engine.reworked())
                        return result;
                    // 43348FFC bypasses the perpendicular recomputation and
                    // reuses the current local-eye vector in native +74..+7C.
                    result.direction = local;
                    result.selected = true;
                    return result;
                }
            } else {
                for (std::size_t point = 0; point != 2; ++point) {
                    BallisticQuery query;
                    query.allow_penetration = 1;
                    query.weapon_id = 0;
                    query.penetration = 2.5f;
                    query.range = 8192.0f;
                    query.range_modifier = 0.98f;
                    query.armor_ratio = 1.65f;
                    query.base_damage = 80.0f;
                    query.reserved = 1;
                    trace_point_penetration(engine.penetration, sample,
                                            result.shifted_target_points[point], 1, query,
                                            engine.suppression_flag);
                    const auto damage = _mm_cvttss_si32(_mm_set_ss(query.damage));
                    combined |= damage;
                    result.damage[point] = damage;
                }
                if (combined != 0)
                    break;
            }
        }
        if (combined != 0)
            break;
        offset += 32.0f;
    }
    if (result.damage[0] == result.damage[1])
        return result;
    result.direction = perpendicular(horizontal);
    if (result.damage[1] == 0)
        for (auto& value : result.direction)
            value = -value;
    else if (result.damage[0] != 0)
        return result;
    result.selected = true;
    return result;
}
} // namespace skeet::resolver_runtime

// ============================================================================
// Subcomponent: environment_score.cpp
// ============================================================================

namespace skeet::resolver_runtime {
namespace {
RageVector transform(const RageVector& point, const Matrix3x4& matrix) noexcept {
    RageVector result;
    for (std::size_t axis = 0; axis != 3; ++axis) {
        const auto& row = matrix.rows[axis];
        result[axis] = ((point[0] * row[0] + point[1] * row[1]) + point[2] * row[2]) + row[3];
    }
    return result;
}
Matrix3x4 concatenate(const Matrix3x4& left, const Matrix3x4& right) noexcept {
    Matrix3x4 result;
    for (std::size_t row = 0; row != 3; ++row)
        for (std::size_t column = 0; column != 4; ++column)
            result.rows[row][column] = ((left.rows[row][2] * right.rows[2][column] +
                                         left.rows[row][1] * right.rows[1][column]) +
                                        left.rows[row][0] * right.rows[0][column]) +
                                       (column == 3 ? left.rows[row][3] : 0.0f);
    return result;
}
Matrix3x4 angle_matrix(float yaw, const RageVector& origin, const RageTrigonometry& trig) {
    const float radians = yaw * 0.01745329238474369f;
    const float sy = trig.sine(radians), cy = trig.cosine(radians);
    const float sp = trig.sine(0.0f), cp = trig.cosine(0.0f);
    const float sr = trig.sine(0.0f), cr = trig.cosine(0.0f);
    const float srcy = sr * cy, srsy = sr * sy, crcy = cr * cy, crsy = cr * sy;
    return {{{cp * cy, srcy * sp - crsy, crcy * sp + srsy, origin[0]},
             {cp * sy, srsy * sp + crcy, crsy * sp - srcy, origin[1]},
             {-sp, sr * cp, cr * cp, origin[2]}}};
}
Matrix3x4 rebase_matrix(float original_yaw, const EnvironmentRebase& context,
                        const RageTrigonometry& trig) {
    float yaw = original_yaw + context.yaw_delta;
    if (std::isnan(yaw))
        yaw = 0.0f;
    else if (std::fabs(yaw) > 180.0f) {
        const float turns = round_half_away_from_zero(std::fabs(yaw / 360.0f));
        const float correction = turns * 360.0f;
        yaw = yaw < 0.0f ? yaw + correction : yaw - correction;
    }
    const auto old = angle_matrix(original_yaw, context.original_origin, trig);
    Matrix3x4 inverse;
    for (std::size_t row = 0; row != 3; ++row) {
        for (std::size_t column = 0; column != 3; ++column)
            inverse.rows[row][column] = old.rows[column][row];
        inverse.rows[row][3] = -((inverse.rows[row][1] * context.original_origin[1] +
                                  inverse.rows[row][0] * context.original_origin[0]) +
                                 inverse.rows[row][2] * context.original_origin[2]);
    }
    return concatenate(angle_matrix(yaw, context.current_origin, trig), inverse);
}
EnvironmentSelection score(const lagcomp::LagRecord& record,
                           const std::array<EnvironmentCapsule, 2>& hitboxes,
                           const RageVector& direction, const RageVector& origin,
                           const EnvironmentRebase* rebase, const RageTrigonometry* trig,
                           bool strict) {
    EnvironmentSelection selection{std::numeric_limits<float>::max(), 0.0f, 0};
    const EnvironmentSelection invalid{std::numeric_limits<float>::infinity(), 0.0f, 0};
    if (strict) {
        const float squared =
            direction[0] * direction[0] + direction[1] * direction[1] + direction[2] * direction[2];
        if (!(record.header.flags & 0x80u) || !std::isfinite(squared) || squared < 1e-8f)
            return invalid;
        for (float value : origin)
            if (!std::isfinite(value))
                return invalid;
        for (const auto& box : hitboxes)
            if (box.matrix_index >= 19 || !(box.radius > 0.0f) || !std::isfinite(box.radius))
                return invalid;
    }
    std::array<float, 2> scores{};
    std::size_t score_index = 0;
    for (const std::size_t variant_index : {0U, 2U}) {
        const auto& variant = record.alternatives[variant_index];
        Matrix3x4 correction{};
        if (rebase != nullptr)
            correction = rebase_matrix(std::bit_cast<float>(variant.state[0]), *rebase, *trig);
        float maximum = -std::numeric_limits<float>::max();
        for (const auto& hitbox : hitboxes) {
            const auto& original_matrix = variant.matrices[hitbox.matrix_index];
            const auto matrix =
                rebase != nullptr ? concatenate(correction, original_matrix) : original_matrix;
            const auto first = transform(hitbox.minimum, matrix);
            const auto second = transform(hitbox.maximum, matrix);
            const float facing =
                ((second[1] - first[1]) * direction[1] + (second[0] - first[0]) * direction[0]) +
                (second[2] - first[2]) * direction[2];
            const auto& endpoint = facing >= 0.0f ? second : first;
            const float length =
                std::sqrt((direction[0] * direction[0] + direction[1] * direction[1]) +
                          direction[2] * direction[2]);
            const float scale = hitbox.radius / length;
            const float y = (scale * direction[1] + endpoint[1]) - origin[1];
            const float x = (scale * direction[0] + endpoint[0]) - origin[0];
            const float inverse =
                1.0f / (std::sqrt((x * x + y * y) + 0.0f) + 1.1920928955078125e-7f);
            const float value = ((inverse * y) * direction[1] + (inverse * x) * direction[0]) +
                                (inverse * 0.0f) * direction[2];
            if (strict && !std::isfinite(value))
                return invalid;
            if (value > maximum)
                maximum = value;
        }
        scores[score_index++] = maximum;
        if (selection.score > maximum) {
            selection.score = maximum;
            selection.pose_yaw = std::bit_cast<float>(variant.state[1]);
        }
    }
    // Equal projections do not identify a side. Do not let enumeration order
    // become apparent geometric evidence.
    if (strict && std::fabs(scores[0] - scores[1]) <=
                      1e-5f * std::max(1.0f, std::max(std::fabs(scores[0]), std::fabs(scores[1]))))
        return invalid;
    selection.attempt = classify_winning_pose(selection.pose_yaw);
    return selection;
}
}
EnvironmentSelection score_environment_hypotheses(const lagcomp::LagRecord& record,
                                                  const std::array<EnvironmentCapsule, 2>& hitboxes,
                                                  const RageVector& direction,
                                                  const RageVector& origin, bool strict) noexcept {
    return score(record, hitboxes, direction, origin, nullptr, nullptr, strict);
}
EnvironmentSelection score_environment_hypotheses(const lagcomp::LagRecord& record,
                                                  const std::array<EnvironmentCapsule, 2>& hitboxes,
                                                  const RageVector& direction,
                                                  const RageVector& origin,
                                                  const EnvironmentRebase& rebase,
                                                  const RageTrigonometry& trig, bool strict) {
    return score(record, hitboxes, direction, origin, &rebase, &trig, strict);
}
} // namespace skeet::resolver_runtime

// ============================================================================
// Subcomponent: environment_update.cpp
// ============================================================================

namespace skeet::resolver_runtime {
namespace {
template <class T>
[[gnu::always_inline]] inline T read(const void* object, std::ptrdiff_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
struct RestoreSuppression {
    std::uint8_t* flag;
    std::uint8_t saved = 0;
    explicit RestoreSuppression(std::uint8_t* pointer) : flag(pointer) {
        if (flag != nullptr) {
            saved = *flag;
            *flag = 0;
        }
    }
    ~RestoreSuppression() {
        if (flag != nullptr)
            *flag = saved;
    }
};
float squared_distance(const RageVector& a, const RageVector& b) {
    const float y = a[1] - b[1], x = a[0] - b[0], z = a[2] - b[2];
    return (y * y + x * x) + z * z;
}
struct OwnerView {
    std::array<std::uint8_t*, 4> attempts;
    std::uint8_t& status;
    float& probe_time;
    float& probe_stamp;
    std::int32_t& probe_count;
    float& reference_time;
    RageVector& reference_origin;
    std::int32_t& last_update_tick;
    float& reference_yaw;
};
OwnerView view(EnvironmentOwner& owner) noexcept {
    auto& attempts = owner.selection.attempt_by_mode;
    return {{&attempts[0], &attempts[1], &attempts[2], &attempts[3]},
            owner.selection.status,
            owner.probe_time,
            owner.probe_stamp,
            owner.probe_count,
            owner.reference_time,
            owner.reference_origin,
            owner.last_update_tick,
            owner.reference_yaw};
}
OwnerView view(client::PlayerState& owner) noexcept {
    auto& state = owner.native;
    return {{&state.resolver_slots[0].attempt, &state.resolver_slots[1].attempt,
             &state.resolver_slots[2].attempt, &state.resolver_slots[3].attempt},
            state.resolver_status,
            state.probe_time,
            state.probe_stamp,
            state.probe_count,
            state.reference_time,
            state.reference_origin,
            state.last_update_tick,
            state.reference_yaw};
}
struct CapturedRequest {
    const EnvironmentUpdateInput& input;
    const LagRecordHistory& records;
    LagRecordHistory history() const noexcept {
        return records;
    }
    [[gnu::always_inline]] inline float local_time() const noexcept {
        return input.local_time;
    }
    [[gnu::always_inline]] inline float current_time() const noexcept {
        return input.current_time;
    }
    std::int32_t tick() const noexcept {
        return lagcomp::corrected_command_tick(input.command_tick, input.previous_command,
                                               input.current_command);
    }
    RageVector local_origin() const noexcept {
        return input.local_origin;
    }
    RageVector target_origin() const noexcept {
        return input.target_origin;
    }
    const std::array<EnvironmentCapsule, 2>* hitboxes() const noexcept {
        return input.hitboxes;
    }
};
struct LiveRequest {
    client::PlayerState& owner;
    const EnvironmentLiveUpdateInput& input;
    std::array<EnvironmentCapsule, 2> capsules;
    LagRecordHistory history() const noexcept {
        return {owner.history.records, owner.history.first, owner.history.count};
    }
    [[gnu::always_inline]] inline float local_time() const noexcept {
        const auto* chain = input.local_time_entity;
        return chain && *chain ? read<float>(*chain, input.local_time_offset) : 0.0f;
    }
    [[gnu::always_inline]] inline float current_time() const noexcept {
        return input.current_time;
    }
    std::int32_t tick() const noexcept {
        return lagcomp::corrected_command_tick(input.command_tick, input.previous_command,
                                               input.current_command);
    }
    RageVector local_origin() const noexcept {
        return input.local_origin;
    }
    RageVector target_origin() const noexcept {
        return read<RageVector>(input.target, input.target_origin_offset);
    }
    const std::array<EnvironmentCapsule, 2>* hitboxes() noexcept {
        //43349119..4334928B:433FD30D returns the current wrapper/header, then
        // the current selected set supplies descriptors0 and3. This lookup is
        // deliberately after every world trace and penetration query.
        const auto& fields = input.bones.fields;
        const auto* wrapper = read<const void*>(input.target, fields.studio_wrapper_32e4daf1);
        if (!wrapper)
            return nullptr;
        const auto* header = read<const std::byte*>(wrapper, 0);
        if (!header)
            return nullptr;
        const auto relative =
            std::uint32_t(read<std::int32_t>(input.target, fields.hitbox_set_c2b6fb10)) * 12u +
            std::uint32_t(read<std::int32_t>(header, 0xB0));
        const auto* set = header + std::bit_cast<std::int32_t>(relative);
        if (!set || read<std::int32_t>(set, 4) == 0)
            return nullptr;
        for (std::size_t index = 0; index != 2; ++index) {
            const auto ordinal = index == 0 ? 0u : 3u;
            const auto offset = std::uint32_t(read<std::int32_t>(set, 8)) + ordinal * 0x44u;
            const auto* descriptor = set + std::bit_cast<std::int32_t>(offset);
            if (!descriptor)
                return nullptr;
            const auto radius = read<float>(descriptor, 0x30);
            if (!(radius > 0.0f))
                return nullptr;
            capsules[index] = {ordinal, read<RageVector>(descriptor, 8),
                               read<RageVector>(descriptor, 0x14), radius};
        }
        return &capsules;
    }
};
template <class Request>
bool ready(OwnerView owner, std::uint32_t count, std::uint8_t resolver_index,
           const Request& input) noexcept {
    if (count == 0 || (resolver_index != 1 && resolver_index != 2) || owner.status == 3)
        return false;
    const auto local_time = input.local_time();
    if (local_time == owner.reference_time && owner.status == 2 &&
        squared_distance(owner.reference_origin, input.target_origin()) < 10000.0f)
        return false;
    if (local_time == owner.probe_time &&
        !(std::fabs(input.current_time() - owner.probe_stamp) > 5.0f))
        return false;
    return true;
}
void publish(OwnerView owner, LagRecordHistory& history, lagcomp::LagRecord& source,
             std::uint8_t selected_mode, std::uint8_t previous_attempt,
             std::uint8_t attempt) noexcept {
    // The first destination slot was selected before the probe. The later
    // movement-mode synchronization reads the source mode again at49D92.
    *owner.attempts[selected_mode] = attempt;
    owner.status = 1;
    const auto mode = source.header.resolver_index;
    if (mode == 1)
        *owner.attempts[2] = attempt;
    else if (mode == 2)
        *owner.attempts[1] = attempt;
    apply_feedback(history, source, previous_attempt, attempt);
}
template <class Request>
bool update(OwnerView owner, Request& input, const EnvironmentEngine& engine) {
    RestoreSuppression suppression(engine.suppression_flag);
    const auto tick = input.tick();
    if (tick <= owner.last_update_tick)
        return false;
    owner.last_update_tick = tick;
    const auto history = input.history();
    for (std::uint32_t index = 0; index != history.count; ++index) {
        const auto flags = history.records[(history.first + index) & 31U]->header.flags;
        if ((flags & 0x200U) != 0)
            break;
        if ((flags & 0x20U) != 0)
            return false;
    }
    auto& source = *history.records[history.first & 31U];
    const auto& header = source.header;
    const auto mode = header.resolver_index;
    if (tick <= header.command_tick || (header.flags & 0x22U) != 0 || (mode != 1 && mode != 2))
        return false;
    const auto previous_attempt = *owner.attempts[mode];
    const auto local_time = input.local_time();
    if (local_time == owner.probe_time &&
        (owner.probe_count >= 2 || std::fabs(input.current_time() - owner.probe_stamp) < 5.0f))
        return false;
    if (local_time == owner.reference_time && owner.status == 2 &&
        squared_distance(owner.reference_origin, input.target_origin()) < 10000.0f)
        return false;
    if ((header.flags & 0x80U) == 0)
        return false;
    const auto local_origin = input.local_origin();
    const auto probe = probe_environment(local_origin, input.target_origin(), engine);
    if (!probe.selected)
        return false;

    const float yaw_delta = owner.reference_yaw - std::bit_cast<float>(header.player_history_1ac);
    const auto target_origin = input.target_origin();
    const bool rebase = yaw_delta != 0.0f || header.origin[0] != target_origin[0] ||
                        header.origin[1] != target_origin[1] ||
                        header.origin[2] != target_origin[2];
    const auto* hitboxes = input.hitboxes();
    if (hitboxes == nullptr)
        return false;
    for (const auto& hitbox : *hitboxes)
        if (!(hitbox.radius > 0.0f))
            return false;
    const RageVector direction{-probe.direction[0], -probe.direction[1], -probe.direction[2]};
    const RageVector origin{local_origin[0], local_origin[1], local_origin[2] + 64.0f};
    EnvironmentSelection selection;
    if (rebase) {
        const EnvironmentRebase correction{yaw_delta, header.origin, target_origin};
        selection = score_environment_hypotheses(source, *hitboxes, direction, origin, correction,
                                                 engine.trigonometry);
    } else
        selection = score_environment_hypotheses(source, *hitboxes, direction, origin);
    owner.probe_time = local_time;
    owner.last_update_tick = input.tick();
    auto publication_history = input.history();
    publish(owner, publication_history, source, mode, previous_attempt,
            std::uint8_t(selection.attempt));
    return true;
}
}
bool environment_update_ready(EnvironmentOwner& owner, const LagRecordHistory& history,
                              std::uint8_t resolver_index,
                              const EnvironmentUpdateInput& input) noexcept {
    const CapturedRequest request{input, history};
    return ready(view(owner), history.count, resolver_index, request);
}
bool environment_update_ready(client::PlayerState& owner,
                              const EnvironmentLiveUpdateInput& input) noexcept {
    const LiveRequest request{owner, input, {}};
    return ready(view(owner), owner.history.count, owner.native.resolver_index, request);
}
bool update_environment(EnvironmentOwner& owner, LagRecordHistory& history,
                        const EnvironmentUpdateInput& input, const EnvironmentEngine& engine) {
    CapturedRequest request{input, history};
    return update(view(owner), request, engine);
}
bool update_environment(client::PlayerState& owner, const EnvironmentLiveUpdateInput& input,
                        const EnvironmentEngine& engine) {
    LiveRequest request{owner, input, {}};
    return update(view(owner), request, engine);
}
} // namespace skeet::resolver_runtime

// ============================================================================
// Subcomponent: hypotheses.cpp
// ============================================================================

namespace skeet::resolver_runtime {
namespace {
float clamp_native(float value, float low, float high) noexcept {
    value = value > low ? value : low;
    return value < high ? value : high;
}

float normalize(float value) noexcept {
    if (std::isnan(value))
        return 0.0f;
    if (std::fabs(value) > 180.0f) {
        const float turns = round_half_away_from_zero(std::fabs(value / 360.0f));
        const float correction = turns * 360.0f;
        value = value < 0.0f ? value + correction : value - correction;
    }
    return value;
}

float difference(float destination, float source) noexcept {
    // The subtraction is single precision before the external _CIfmod call.
    float result = static_cast<float>(std::fmod(static_cast<double>(destination - source), 360.0));
    if (destination > source) {
        if (result >= 180.0f)
            result -= 360.0f;
    } else if (result <= -180.0f)
        result += 360.0f;
    return result;
}

float quantize(float value) noexcept {
    const float scaled = value * 182.0444488525390625f;
    // CVTTSS2SI returns the integer indefinite value for NaN/out-of-range.
    std::int32_t integer = std::numeric_limits<std::int32_t>::min();
    if (scaled >= -2147483648.0f && scaled < 2147483648.0f)
        integer = static_cast<std::int32_t>(scaled);
    return static_cast<float>(static_cast<std::uint32_t>(integer) & 0xffffU) * 0.0054931640625f;
}
}

std::array<Hypothesis, 3> solve_hypotheses(const HypothesisInput& in) noexcept {
    float modifier =
        ((0.8f - in.speed_fraction * 0.3f) - 1.0f) * clamp_native(in.running_fraction, 0.0f, 1.0f) +
        1.0f;
    if (in.duck_amount > 0.0f) {
        const float duck = in.duck_amount * clamp_native(in.ducking_fraction, 0.0f, 1.0f);
        modifier += duck * (0.5f - modifier);
    }
    const float maximum = in.maximum_yaw * modifier;
    const float minimum = in.minimum_yaw * modifier;
    constexpr float seeds[3] = {118.0f, 0.0f, -118.0f};
    std::array<Hypothesis, 3> output{};
    for (std::uint32_t index = 0; index != 3; ++index) {
        float feet = index == 1 ? in.eye_yaw : in.feet_yaw;
        float movement = in.movement_yaw;
        float target = in.movement_yaw_target;
        for (std::uint32_t iteration = 0; iteration != 2; ++iteration) {
            const float eye = iteration == 0 ? seeds[index] + in.eye_yaw : in.eye_yaw;
            const float eye_delta = difference(eye, feet);
            if (eye_delta > maximum)
                feet = eye - std::fabs(maximum);
            else if (eye_delta < minimum)
                feet = std::fabs(minimum) + eye;
            feet = normalize(feet);

            const float horizontal_speed =
                std::sqrt(in.velocity_x * in.velocity_x + in.velocity_y * in.velocity_y);
            const bool moving = horizontal_speed > 0.1f || in.velocity_z > 100.0f;
            const float step = std::fabs(moving ? (in.speed_fraction * 20.0f + 30.0f) * in.interval
                                                : in.interval * 100.0f);
            const float goal = quantize(moving ? eye : in.lower_body_yaw);
            feet = quantize(feet);
            const float delta = normalize(goal - feet);
            if (delta > step)
                feet += step;
            else if (delta < -step)
                feet -= step;
            else
                feet = goal;

            if (horizontal_speed > 0.0f && in.moving != 0) {
                double angle = std::atan2(static_cast<double>(-in.velocity_y),
                                          static_cast<double>(-in.velocity_x));
                float direction = static_cast<float>((angle * 180.0) / 3.14159265358979323846);
                if (direction < 0.0f)
                    direction += 360.0f;
                target = normalize(difference(direction, feet));
            }
            const float move_delta = normalize(difference(target, movement));
            if ((in.record_flags & 0x400U) != 0 || in.animation_fraction >= 1.0f)
                movement = target;
            else {
                const float running = clamp_native(in.running_fraction, 0.0f, 1.0f);
                const float ducking = clamp_native(in.ducking_fraction, 0.0f, 1.0f);
                const float blend = (ducking - running) * in.duck_amount + running;
                const float exponent = std::log(0.18f) * -1.4427f;
                movement = normalize((std::pow(blend, exponent) + 0.1f) * move_delta + movement);
            }
        }
        feet = normalize(feet);
        float pose = difference(in.eye_yaw, feet);
        if (pose >= 0.0f && in.maximum_yaw != 0.0f)
            pose = (pose / in.maximum_yaw) * 58.0f;
        else if (in.minimum_yaw != 0.0f)
            pose = (pose / in.minimum_yaw) * -58.0f;
        output[index] = {feet, clamp_native(pose, -60.0f, 60.0f),
                         clamp_native((pose - -60.0f) / 120.0f + 0.0f, 0.0f, 1.0f),
                         clamp_native((movement - -180.0f) / 360.0f + 0.0f, 0.0f, 1.0f)};
    }
    return output;
}

} // namespace skeet::resolver_runtime

// ============================================================================
// Subcomponent: lag_record.cpp
// ============================================================================

namespace skeet::resolver_runtime {

namespace {
void apply_variant(lagcomp::LagRecord& record, std::size_t index, std::uint8_t attempt) noexcept {
    const auto& variant = record.alternatives[index];
    float pose = (std::bit_cast<float>(variant.state[1]) - -60.0f) / 120.0f + 0.0f;
    pose = pose > 0.0f ? pose : 0.0f;
    pose = pose < 1.0f ? pose : 1.0f;
    record.animation.pose_parameters[11] = pose;
    record.header.animation_yaw = std::bit_cast<float>(variant.state[0]);
    copy_hitbox_matrices(variant.matrices, record.matrices);
    record.header.resolver_mode = attempt;
}
}

void select_yaw_variant(lagcomp::LagRecord& record, std::uint8_t attempt) noexcept {
    apply_variant(record, yaw_variant_index(attempt), attempt);
}

void select_variant(lagcomp::LagRecord& record, std::uint8_t attempt) noexcept {
    auto& header = record.header;
    const auto mode = header.resolver_index;
    if ((header.flags & 0x80U) == 0 || mode == 0 || mode > 3)
        return;
    const auto cycle = attempt % 3;
    if (mode < 3) {
        select_yaw_variant(record, attempt);
    } else {
        float delta = std::bit_cast<float>(header.player_history_1ac) -
                      std::bit_cast<float>(header.player_history_180);
        if (std::isnan(delta))
            delta = 0.0f;
        else if (std::fabs(delta) > 180.0f) {
            const float turns = round_half_away_from_zero(std::fabs(delta / 360.0f));
            const float correction = turns * 360.0f;
            delta = delta < 0.0f ? delta + correction : delta - correction;
        }
        const bool positive = delta > 0.0f;
        const auto index = cycle == 0 ? (positive ? 0 : 2) : cycle == 1 ? (positive ? 2 : 0) : 1;
        apply_variant(record, index, attempt);
    }
}

void advance_pending_attempt(lagcomp::LagRecord& record, std::uint32_t options) noexcept {
    if ((options & 4U) != 0 && (record.header.flags & 0xc0U) == 0xc0U) {
        record.header.flags &= ~0x40U;
        select_variant(record, static_cast<std::uint8_t>(record.header.resolver_mode + 1U));
    }
}

std::uint32_t propagate_attempt(LagRecordHistory& history, std::uint8_t mode,
                                std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept {
    if (previous_attempt == new_attempt)
        return 0;
    std::uint32_t processed = 0;
    for (std::uint32_t index = 0; index != history.count; ++index) {
        auto& record = *history.records[(history.first + index) & 31U];
        auto& header = record.header;
        if ((header.flags & 0x22U) != 0 || header.resolver_index != mode ||
            header.resolver_mode != previous_attempt)
            break;
        select_variant(record, new_attempt);
        header.flags = (header.flags & ~0x40U) | 2U;
        ++processed;
        if ((header.flags & 0x200U) != 0)
            break;
    }
    return processed;
}

void apply_feedback(LagRecordHistory& history, lagcomp::LagRecord& source,
                    std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept {
    propagate_attempt(history, source.header.resolver_index, previous_attempt, new_attempt);
    source.header.flags |= 2U;
}

void publish_feedback(SelectionState& state, LagRecordHistory& history, lagcomp::LagRecord& source,
                      std::uint8_t previous_attempt, std::uint8_t new_attempt) noexcept {
    const auto mode = source.header.resolver_index;
    state.attempt_by_mode[mode] = new_attempt;
    state.status = 1;
    if (mode == 1)
        state.attempt_by_mode[2] = new_attempt;
    else if (mode == 2)
        state.attempt_by_mode[1] = new_attempt;
    apply_feedback(history, source, previous_attempt, new_attempt);
}

} // namespace skeet::resolver_runtime

// ============================================================================
// Subcomponent: live_build.cpp
// ============================================================================

namespace skeet::resolver_runtime {
namespace {
class RestoreLivePoses {
public:
    explicit RestoreLivePoses(std::span<float, 24> destination) noexcept : poses(destination) {
        std::memcpy(saved.data(), poses.data(), sizeof saved);
    }
    ~RestoreLivePoses() {
        std::memcpy(poses.data(), saved.data(), sizeof saved);
    }
    std::span<float, 24> poses;

private:
    std::array<std::uint32_t, 24> saved;
};
}
std::uint32_t build_hypotheses(lagcomp::LagRecord& record, const HypothesisInput& input,
                               void* player, const animation::BoneRuntime& runtime, bool strict) {
    animation::RecordPoseScope record_pose(player, record, runtime, strict);
    if (strict)
        record.header.flags &= ~0x80u;
    if (!record_pose.valid() || (strict && !valid_hypothesis_input(input)))
        return 0;
    RestoreLivePoses restore(runtime.pose_parameters(player));
    auto parameters = input;
    parameters.record_flags = record.header.flags;
    if (strict) {
        parameters.velocity_x = record.header.velocity[0];
        parameters.velocity_y = record.header.velocity[1];
        parameters.velocity_z = record.header.velocity[2];
        parameters.duck_amount = record.header.duck_amount;
        parameters.feet_yaw = record.header.animation_yaw;
        parameters.lower_body_yaw = record.header.field_a2a8247f;
        parameters.movement_yaw = record.animation.pose_parameters[7] * 360.0f - 180.0f;
    }
    const auto hypotheses = solve_hypotheses(parameters);
    std::uint32_t completed = 0;
    for (std::size_t index = 0; index != record.alternatives.size(); ++index) {
        auto& alternative = record.alternatives[index];
        const auto& hypothesis = hypotheses[index];
        if (strict && !valid_hypothesis(hypothesis))
            return completed;
        alternative.state[0] = std::bit_cast<std::uint32_t>(hypothesis.yaw);
        alternative.state[1] = std::bit_cast<std::uint32_t>(hypothesis.pose_yaw);
        restore.poses[11] = hypothesis.body_pose_parameter;
        restore.poses[7] = hypothesis.move_pose_parameter;
        std::int32_t count = 0;
        if (strict) {
            std::array<Matrix3x4, animation::maximum_bones> matrices;
            const animation::BoneRequest request{
                {0.0f, hypothesis.yaw, 0.0f}, record.header.origin, 0.0f, 0x80300u, -1, 0, false};
            if (runtime.build_bone_matrices(player, request, matrices))
                count = runtime.select_hitbox_bone_matrices(player, matrices, alternative.matrices);
        } else
            count = runtime.build_resolver_hitbox_matrices(player, hypothesis.yaw,
                                                           alternative.matrices);
        if (count <= 0 || (strict && !complete_hitbox_matrices(count, alternative.matrices)))
            return completed;
        ++completed;
    }
    record.header.flags |= 0x80u;
    return completed;
}
} // namespace skeet::resolver_runtime

// ============================================================================
// Subcomponent: next_attempt.cpp
// ============================================================================

namespace skeet::resolver_runtime {

bool intersects_hypothesis(const RageRay& ray, const std::array<const void*, 19>& descriptors,
                           const HitboxMatrices& matrices, const RageBoxClipper& engine) {
    RageTrace trace;
    for (std::size_t index = 0; index != 19; ++index) {
        RageScanHitbox hitbox{};
        hitbox.engine_hitbox = descriptors[index];
        hitbox.transform = matrices[index];
        if (engine.clip(engine.context, ray, hitbox, trace) >= 0)
            return true;
    }
    return false;
}

std::uint8_t choose_next_attempt(const AttemptOwner& owner, const AttemptShot& shot,
                                 float current_time, bool geometry_enabled, const RageVector* start,
                                 const RageVector* direction, const AttemptEngine& engine) {
    const auto mode = shot.record.header.resolver_index;
    const auto previous = shot.record.header.resolver_mode;
    std::uint8_t increment = static_cast<std::uint8_t>((~shot.shot_flags) & 1U);
    if (mode == 0) {
        auto* target = engine.lookup_handle(engine.context, shot.target_handle);
        if (target != nullptr && target->animation_adjustment != nullptr) {
            const float elapsed = current_time - owner.previous_pose_time;
            if (elapsed >= 0.0f && elapsed < 0.5f) {
                float adjustment = 0.0f;
                if (target->studio_model != nullptr)
                    adjustment = engine.convert_pose_parameter(engine.context, target->studio_model,
                                                               11, target->body_pose_parameter);
                *target->animation_adjustment += adjustment;
                *target->animation_adjustment += adjustment;
            }
        }
    } else if (mode < 3) {
        if (geometry_enabled && start != nullptr && direction != nullptr &&
            (shot.shot_flags & 0x28U) == 0) {
            auto* target = engine.lookup_handle(engine.context, shot.target_handle);
            if (target != nullptr && !target->hidden && target->animation_adjustment != nullptr &&
                (shot.record.header.flags & 0x80U) == 0 && (shot.prefix_flags & 0x1eU) == 0 &&
                target->studio_model != nullptr && target->hitbox_set &&
                target->hitbox_count != 0) {
                RageRay ray{*start, {}};
                for (std::size_t axis = 0; axis != 3; ++axis) {
                    const float endpoint = (*direction)[axis] * shot.distance + (*start)[axis];
                    ray.delta[axis] = endpoint - (*start)[axis];
                }
                constexpr std::size_t order[3] = {1, 2, 0};
                const auto attempt = static_cast<std::uint8_t>(previous + increment);
                const auto index = order[attempt % 3];
                if (intersects_hypothesis(ray, target->hitboxes,
                                          shot.record.alternatives[index].matrices, engine.clipper))
                    ++increment;
            }
        }
    } else if (mode == 3 && previous % 3 == 1 &&
               std::fabs(current_time - owner.previous_selection_time) > 10.0f)
        increment = 2;
    return static_cast<std::uint8_t>(previous + increment);
}

namespace {
template <class T> T field(const void* object, std::ptrdiff_t at) {
    T result;
    std::memcpy(&result, static_cast<const std::byte*>(object) + at, sizeof result);
    return result;
}
}
std::uint8_t choose_next_attempt(const AttemptOwner& owner, const AttemptShot& shot, float,
                                 bool geometry_enabled, const RageVector* start,
                                 const RageVector* direction, const RawAttemptEngine& source) {
    const auto mode = shot.record.header.resolver_index;
    const auto previous = shot.record.header.resolver_mode;
    auto increment = static_cast<std::uint8_t>((~shot.shot_flags) & 1u);
    if (mode == 0) {
        auto* target = engine::invoke<void*>(source.entity_list, 3, shot.target_handle);
        if (target)
            if (auto* animation = field<std::byte*>(target, source.animation_state)) {
                const float elapsed = *source.current_time - owner.previous_pose_time;
                if (elapsed >= 0.0f && elapsed < 0.5f) {
                    float adjustment = 0.0f;
                    const auto* wrapper = field<const void*>(target, source.studio_wrapper);
                    if (wrapper && field<const void*>(wrapper, 0))
                        adjustment = source.pose.convert(
                            wrapper, 11, field<float>(target, source.pose_parameters + 44));
                    auto value = field<float>(animation, 0x80);
                    value += adjustment;
                    value += adjustment;
                    std::memcpy(animation + 0x80, &value, sizeof value);
                }
            }
    } else if (mode < 3) {
        if (geometry_enabled && start && direction && !(shot.shot_flags & 0x28u)) {
            auto* target = engine::invoke<void*>(source.entity_list, 3, shot.target_handle);
            if (target && !field<std::uint8_t>(target, source.hidden) &&
                field<const void*>(target, source.animation_state) &&
                !(shot.record.header.flags & 0x80u) && !(shot.prefix_flags & 0x1Eu)) {
                const auto* wrapper = field<const std::byte*>(target, source.studio_wrapper);
                const auto* header = wrapper ? field<const std::byte*>(wrapper, 0) : nullptr;
                if (header) {
                    const auto* set =
                        header + field<std::int32_t>(header, 0xB0) +
                        std::ptrdiff_t(field<std::int32_t>(target, source.hitbox_set)) * 12;
                    if (field<std::uint32_t>(set, 4)) {
                        RageRay ray{*start, {}};
                        for (std::size_t axis = 0; axis < 3; ++axis) {
                            const auto endpoint =
                                (*direction)[axis] * shot.distance + (*start)[axis];
                            ray.delta[axis] = endpoint - (*start)[axis];
                        }
                        constexpr std::size_t order[]{1, 2, 0};
                        const auto index =
                            order[static_cast<std::uint8_t>(previous + increment) % 3];
                        std::array<const void*, 19> descriptors;
                        const auto* first = set + field<std::int32_t>(set, 8);
                        for (std::size_t i = 0; i < descriptors.size(); ++i)
                            descriptors[i] = first + i * 0x44;
                        if (intersects_hypothesis(ray, descriptors,
                                                  shot.record.alternatives[index].matrices,
                                                  source.clipper))
                            ++increment;
                    }
                }
            }
        }
    } else if (mode == 3 && previous % 3 == 1 &&
               std::fabs(*source.current_time - owner.previous_selection_time) > 10.0f)
        increment = 2;
    return static_cast<std::uint8_t>(previous + increment);
}
std::uint8_t choose_next_attempt(const AttemptOwner& owner, const AttemptShot& shot,
                                 float current_time, bool geometry_enabled, const RageVector* start,
                                 const RageVector* direction, const AttemptEngineBinding& source) {
    return std::visit(
        [&](auto selected) {
            return choose_next_attempt(owner, shot, current_time, geometry_enabled, start,
                                       direction, selected.get());
        },
        source.source());
}

} // namespace skeet::resolver_runtime

// ============================================================================
// Subcomponent: jitter.cpp
// ============================================================================

namespace skeet::resolver_runtime {
namespace {
float yaw_distance(float a, float b) noexcept {
    return std::fabs(std::remainder(a - b, 360.0f));
}
JitterPoseEstimate closest_pose(float anchor, const std::array<float, 3>& attempts,
                                float confidence_margin) noexcept {
    float best = std::numeric_limits<float>::infinity(), next = best;
    std::uint8_t attempt = 0;
    for (std::uint8_t i = 0; i < attempts.size(); ++i) {
        const auto error = yaw_distance(attempts[i], anchor);
        if (error < best) {
            next = best;
            best = error;
            attempt = i;
        } else {
            next = std::min(next, error);
        }
    }
    const float margin = next - best;
    return {anchor, best, margin, margin >= confidence_margin ? attempt : std::uint8_t{0xff}};
}

JitterPoseEstimate estimate_record_pose(const lagcomp::LagRecord& record, float eye) noexcept {
    const auto& header = record.header;
    const float anchor = std::bit_cast<float>(header.player_history_180);
    if (!std::isfinite(eye) || !std::isfinite(anchor) || !std::isfinite(header.animation_yaw))
        return {};
    std::array<float, 3> attempts;
    for (std::uint8_t attempt = 0; attempt < attempts.size(); ++attempt) {
        attempts[attempt] = std::bit_cast<float>(record.alternatives[yaw_variant_index(attempt)].state[0]);
        if (!std::isfinite(attempts[attempt]))
            return {};
    }
    const float range = std::max(yaw_distance(eye, attempts[1]), yaw_distance(eye, attempts[2]));
    const float confidence = std::max(2.0f, range * 0.1f);
    if (!(range > 0.0f && range <= 90.0f) || yaw_distance(eye, anchor) > range + confidence)
        return {};
    const auto estimate = closest_pose(std::remainder(anchor, 360.0f), attempts, confidence);
    if (!estimate.valid() || yaw_distance(header.animation_yaw, anchor) - estimate.error < confidence)
        return {};
    const auto& variant = record.alternatives[yaw_variant_index(estimate.attempt)];
    if (!std::isfinite(std::bit_cast<float>(variant.state[1])) ||
        !complete_hitbox_matrices(static_cast<std::int32_t>(variant.matrices.size()), variant.matrices))
        return {};
    return estimate;
}
}
JitterPoseEstimate estimate_jitter_pose(float eye_yaw, float animation_yaw,
                                        float maximum_yaw) noexcept {
    if (!std::isfinite(eye_yaw) || !std::isfinite(animation_yaw) ||
        !std::isfinite(maximum_yaw) || maximum_yaw <= 0 || maximum_yaw > 90.0f)
        return {};
    const auto confidence_margin = std::max(2.0f, maximum_yaw * 0.1f);
    const auto anchor = std::remainder(animation_yaw, 360.0f);
    const auto eye = std::remainder(eye_yaw, 360.0f);
    if (yaw_distance(eye, anchor) > maximum_yaw + confidence_margin)
        return {anchor};
    return closest_pose(anchor, {eye, eye - maximum_yaw, eye + maximum_yaw}, confidence_margin);
}
const char* jitter_decision_name(JitterDecision decision) noexcept {
    switch (decision) {
    case JitterDecision::animation_yaw: return "animation-yaw-prior";
    case JitterDecision::shot_feedback: return "retain-shot-hypothesis";
    case JitterDecision::environment: return "environment-evidence";
    case JitterDecision::movement: return "animation-movement-evidence";
    case JitterDecision::shared: return "shared-yaw-evidence";
    case JitterDecision::forced_yaw: return "forced-body-yaw";
    case JitterDecision::shot_record: return "target-shot-record";
    case JitterDecision::correction_disabled: return "correction-disabled";
    case JitterDecision::ambiguous_pose: return "no-clear-pose";
    case JitterDecision::unchanged_pose: return "stock-pose-retained";
    case JitterDecision::history_pose: return "history-animation-yaw";
    default: return "no-jitter-correction";
    }
}
void JitterTracking::remember(const JitterRecordEvidence& evidence) noexcept {
    if (evidence.tick < 0)
        return;
    for (auto& record : records)
        if (record.tick == evidence.tick) {
            record = evidence;
            return;
        }
    records[next_record++ % records.size()] = evidence;
}
const JitterRecordEvidence* JitterTracking::find(const lagcomp::LagRecordHeader& header) const noexcept {
    const auto yaw = std::bit_cast<float>(header.player_history_1ac);
    if (header.simulation_tick < 0 || !std::isfinite(yaw))
        return nullptr;
    for (const auto& record : records)
        if (record.tick == header.simulation_tick && record.receive_tick == header.receive_tick &&
            yaw_distance(record.observed_yaw, yaw) < 0.01f)
            return &record;
    return nullptr;
}

bool JitterRecordEvidence::applied_to(const lagcomp::LagRecordHeader& header) const noexcept {
    return applied() && estimate.attempt == header.resolver_mode;
}

void correct_jitter_history(lagcomp::RecordRing& history, JitterTracking& tracking) noexcept {
    if (history.count < 2 || history.count > history.records.size() ||
        (history.at(0).header.flags & 0x200u))
        return;
    for (std::uint32_t age = 1; age < history.count; ++age) {
        auto& record = history.at(age);
        const auto& header = record.header;
        if (header.flags & 0x200u)
            break;
        if ((header.flags & 0x84u) != 0x84u || (header.flags & 0x6au) ||
            header.resolver_state != 0 || header.resolver_index > 2)
            continue;
        const auto* captured = tracking.find(header);
        if (captured && (captured->history_checked || captured->applied() ||
                         captured->decision == JitterDecision::forced_yaw ||
                         captured->decision == JitterDecision::shot_feedback))
            continue;
        if (lagcomp::record_precedes_discontinuity(history, header.simulation_tick))
            break;
        auto evidence = captured ? *captured : JitterRecordEvidence{};
        evidence.tick = header.simulation_tick;
        evidence.receive_tick = header.receive_tick;
        evidence.observed_yaw = std::bit_cast<float>(header.player_history_1ac);
        evidence.history_checked = true;
        const auto estimate = estimate_record_pose(record, evidence.observed_yaw);
        if (estimate.valid()) {
            select_yaw_variant(record, estimate.attempt);
            evidence.estimate = estimate;
            evidence.decision = JitterDecision::history_pose;
        }
        tracking.remember(evidence);
    }
}
} // namespace skeet::resolver_runtime
