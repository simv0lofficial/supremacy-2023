// Aimbot Subsystem Implementation
#include "util/address.h"
#include "skeet/legit/accuracy.hpp"
#include "skeet/legit/trace_filter.hpp"
#include "skeet/hud/penetration_runtime.hpp"
#include "skeet/client/player_state.hpp"
#include <bit>
#include <cmath>
#include <cstring>
#include "skeet/legit/accuracy_history.hpp"
#include "skeet/legit/studio.hpp"
#include "skeet/engine/opaque_native_word.hpp"
#include "skeet/legit/acquisition.hpp"
#include "skeet/anti_aim/targets.hpp"
#include "skeet/legit/runtime.hpp"
#include <stdexcept>
#include "skeet/legit/command.hpp"
#include "skeet/lagcomp/update_timing.hpp"
#include "skeet/movement/steering_math.hpp"
#include "skeet/legit/historical_bones.hpp"
#include <algorithm>
#include <immintrin.h>
#include "skeet/legit/historical_bones_live.hpp"
#include "skeet/legit/manual_motion.hpp"
#include "skeet/legit/owner.hpp"
#include <array>
#include <new>
#include "skeet/legit/recoil_state.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/legit/runtime_owner.hpp"
#include "skeet/client/command_capture_runtime_owner.hpp"
#include "skeet/client/history_runtime_owner.hpp"
#include "skeet/client/player_runtime_owner.hpp"
#include "skeet/client/runtime_composition.hpp"
#include "skeet/client/runtime_storage.hpp"
#include "skeet/client/native_startup.hpp"
#include "skeet/animation/runtime_owner.hpp"
#include "skeet/engine/scene_runtime_owner.hpp"
#include "skeet/engine/build_legit_bindings.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/engine/netvar_initialization.hpp"
#include "skeet/esp/runtime_owner.hpp"
#include "skeet/lagcomp/native_history_boundaries.hpp"
#include "skeet/rage/settings.hpp"
#include "skeet/rage/spread_storage.hpp"
#include "skeet/misc/config.hpp"
#include "skeet/legit/state.hpp"
#include "skeet/legit/target_selection.hpp"
#include "skeet/legit/tracking.hpp"
#include "skeet/legit/trigger_trace.hpp"
#include "skeet/rage/acquisition.hpp"
#include <memory>
#include "skeet/rage/acquisition_frame.hpp"
#include "skeet/rage/preamble.hpp"
#include "skeet/menu/misc_menu.hpp"
#include "skeet/rage/feedback_owner.hpp"
#include "skeet/movement/extrapolation.hpp"
#include "skeet/rage_safe_points.hpp"
#include "detail/rage_point_math.hpp"
#include "detail/rage_math.hpp"
#include "detail/action_detail.hpp"
#include "detail/scanning_detail.hpp"
#include "detail/final_target_context_detail.hpp"
#include "detail/owner_detail.hpp"
#include "detail/post_firing_detail.hpp"
#include "detail/targeting_pipeline_detail.hpp"
#include "skeet/rage_safety.hpp"
#include "skeet/rage/scanning_geometry.hpp"
#include "skeet/client/target_refresh_runtime_owner.hpp"
#include "skeet/rage/body_aim_probe.hpp"
#include "skeet/rage/targeting_commit.hpp"
#include <limits>
#include "skeet/rage/command_owner.hpp"
#include "skeet/rage/final_target_context.hpp"
#include "skeet/rage/post_firing.hpp"
#include "skeet/rage/command_phase2.hpp"
#include "skeet/rage/command_phase6.hpp"
#include "skeet/rage/command_phase8.hpp"
#include "skeet/rage/command_source.hpp"
#include "skeet/rage/entity_update.hpp"
#include "skeet/client/entity_command_owner.hpp"
#include "skeet/rage/firing.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/rage/group_storage.hpp"
#include "skeet/client/shutdown.hpp"
#include <utility>
#include "skeet/config_storage/storage.hpp"
#include "skeet/rage/initialization.hpp"
#include "skeet/esp/hitbox_highlight.hpp"
#include "skeet/rage/multiple_targets.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/rage/preamble_tail.hpp"
#include "skeet/rage/aim_fire_event.hpp"
#include "skeet/rage/pending_shot_publication.hpp"
#include "skeet/rage/shot_diagnostics.hpp"
#include "skeet/rage/prediction_mask_owner.hpp"
#include "skeet/movement/rage_stop.hpp"
#include "skeet/engine/owner_allocation.hpp"
#include "skeet/rage/prediction_mask_runtime_owner.hpp"
#include "skeet/rage/target_relationships.hpp"
#include "skeet/rage/prediction_masks.hpp"
#include "skeet/esp/player_engine.hpp"
#include "skeet/esp/owner_runtime.hpp"
#include "skeet/rage/record_candidates.hpp"
#include "skeet/client/command_orchestrator.hpp"
#include <xmmintrin.h>
#include "skeet/rage/reset.hpp"
#include "skeet/rage/runtime_owner.hpp"
#include "skeet/rage/shot_publication.hpp"
#include "skeet/rage/buffered_shot_repair.hpp"
#include "skeet/client/command_checksum.hpp"
#include "skeet/client/movement_hud_runtime_owner.hpp"
#include "skeet/client/visual_runtime_owner.hpp"
#include "skeet/client/prediction_history_owner.hpp"
#include "skeet/engine/build_hitbox_clip_bindings.hpp"
#include "skeet/esp/other_config.hpp"
#include "skeet/effects/runtime_owner.hpp"
#include "skeet/lagcomp/history_runtime.hpp"
#include "skeet/script/source_runtime_owner.hpp"
#include <optional>
#include "skeet/rage/scan_hypotheses.hpp"
#include "detail/rage_safe_point_detail.hpp"
#include "skeet/rage/scanning_chance.hpp"
#include "skeet/rage/single_target.hpp"
#include "skeet/rage_damage.hpp"
#include "skeet/lagcomp/live_record_attachments.hpp"
#include <cstdlib>
#include "skeet/rage/targeting.hpp"
#include "skeet/rage/targeting_acquired.hpp"
#include <atomic>
#include "skeet/rage_segments.hpp"
#include "skeet/engine/jobs.hpp"
#include "skeet/rage/targeting_pipeline.hpp"
#include <vector>
#include "skeet/resolver_runtime/attempts.hpp"
#include "skeet/rage_config.hpp"
#include "rage_math.hpp"
#include "skeet/rage_geometry.hpp"
#include "rage_safe_point_detail.hpp"
#include "skeet/rage_hitboxes.hpp"
#include "skeet/rage_hitchance.hpp"
#include "rage_point_math.hpp"
#include "skeet/rage_points.hpp"
#include "skeet/rage_policy.hpp"
#include "skeet/rage_selection.hpp"


// ============================================================================
// Subcomponent: accuracy.cpp
// ============================================================================

namespace skeet::legit {
namespace {
/* [stripped helper] */
float add(float first, float second) {
    __asm__ __volatile__("addss %1, %0" : "+x"(first) : "x"(second));
    return first;
}
float multiply(float first, float second) {
    __asm__ __volatile__("mulss %1, %0" : "+x"(first) : "x"(second));
    return first;
}
// Keep the full128 table as evidence for the stride rather than relying on a
// cache generated by a different per-weapon query.
constexpr std::array<std::uint8_t, 128> permutation{
    2,   3,   4,   10,  13,  15,  27,  29, 37, 38,  41,  46,  47,  49,  50,  52,  56,  57,  58,
    62,  66,  68,  70,  72,  76,  83,  87, 97, 108, 109, 110, 111, 113, 121, 122, 125, 135, 1,
    5,   6,   8,   17,  25,  26,  28,  31, 32, 36,  39,  44,  51,  61,  67,  71,  74,  77,  78,
    79,  80,  81,  84,  89,  90,  91,  92, 96, 98,  101, 102, 104, 107, 114, 115, 119, 9,   11,
    16,  19,  20,  22,  30,  48,  54,  59, 60, 65,  69,  73,  75,  82,  85,  86,  88,  94,  99,
    103, 105, 112, 116, 117, 118, 124, 0,  7,  12,  14,  18,  21,  23,  24,  33,  34,  35,  40,
    42,  43,  45,  53,  55,  63,  64,  93, 95, 100, 106, 120, 133, 137};
Vector target_center(void* target) {
    auto result = *engine::invoke<const Vector*>(target, 10);
    const auto minimum = *engine::invoke<const Vector*>(target, 76);
    const auto maximum = *engine::invoke<const Vector*>(target, 77);
    result[2] = (minimum[2] + maximum[2]) * 0.5f + result[2];
    return result;
}
std::int32_t add_tick(std::int32_t first, std::int32_t second) {
    return std::bit_cast<std::int32_t>(std::uint32_t(first) + std::uint32_t(second));
}
bool convar_enabled(const void* convar) {
    for (;;) {
        const auto* parent = read<const void*>(convar, 0x1C);
        if (parent == convar)
            break;
        convar = parent;
    }
    return read<std::uint32_t>(convar, 0x30) !=
           std::uint32_t(reinterpret_cast<std::uintptr_t>(convar));
}
}
RageSpreadBasis spread_basis(const Vector& angles, const RageTrigonometry& math) {
    constexpr float radians = 0.01745329238474369f;
    const auto pitch = angles[0] * radians, yaw = angles[1] * radians, roll = angles[2] * radians;
    const auto sp = math.sine(pitch), cp = math.cosine(pitch), sy = math.sine(yaw),
               cy = math.cosine(yaw), sr = math.sine(roll), cr = math.cosine(roll);
    const auto minus_cr = cr * -1.0f, sr_sp = sr * sp, cr_sp = cr * sp;
    return {{cy * cp, sy * cp, -sp},
            {minus_cr * -sy - sr_sp * cy, minus_cr * cy - sr_sp * sy, (sr * -1.0f) * cp},
            {cr_sp * cy - (-sy) * sr, cr_sp * sy - sr * cy, cr * cp}};
}
std::array<Vector, 32> accuracy_spread_directions(const rage::SpreadParameters& input,
                                                  std::uint32_t flags, const RageSpreadBasis& basis,
                                                  const rage::SpreadTables& tables,
                                                  const rage::SpreadRandom& random,
                                                  const rage::ShotgunPattern& pattern,
                                                  const RageTrigonometry& math) {
    auto parameters = input;
    if (parameters.weapon_id == 64) {
        parameters.spread = 0.00052f;
        parameters.inaccuracy = (flags & 6u) ? 0.003f : 0.0077f;
        parameters.fire_mode = 0;
    }
    parameters.pellet_count = 1;
    std::array<Vector, 32> result;
    for (unsigned i = 0; i != 32; ++i) {
        const auto sample = rage::generate_spread_sample(parameters, permutation[i * 4], tables,
                                                         random, pattern, math);
        auto& direction = result[i];
        for (unsigned axis = 0; axis != 3; ++axis)
            direction[axis] =
                add(multiply(basis.second[axis], sample.second),
                    add(multiply(basis.first[axis], sample.first), basis.center[axis]));
        const auto inverse =
            1.0f / (std::sqrt((direction[0] * direction[0] + direction[1] * direction[1]) +
                              direction[2] * direction[2]) +
                    1.1920928955078125e-7f);
        for (auto& component : direction)
            component *= inverse;
    }
    return result;
}
std::int32_t accuracy_tick(ProfileState& profile, const client::CommandFrame& frame,
                           void* local_player, const client::Command& command, bool* hit,
                           const AccuracyEnvironment& environment) {
    const auto direction = read<Vector>(&frame, 0x174);
    auto* target = hud::closest_penetration_target(
        environment.targeting, *environment.maximum_clients, frame.origin_dc, direction, 500.0f);
    if (!target)
        return -1;
    if (!(command.movement.buttons & 1u) && hit && !(profile.flags & 0x10u) &&
        environment.trace.through_smoke &&
        environment.trace.through_smoke(environment.trace.context, frame.origin_dc,
                                        target_center(target)))
        return -1;
    lagcomp::EntityRecordScope scope(environment.entity, target, environment.saved_bones);
    auto selected = -1;
    auto best_cosine = -1.0f;
    const auto mode = profile.accuracy_boost;
    const auto range = float(profile.accuracy_range);
    const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(target) + 8, 10);
    auto& ring = environment.selection.players.get(std::uint32_t(index))->history;
    const auto latency = read<float>(&frame, 0x198);
    if (ring.count) {
        const auto center_time =
            *environment.time - (latency + *environment.interpolation_43476928);
        const auto width = 0.2f - *environment.interval;
        const auto first_candidate =
            lagcomp::time_to_tick(center_time - width, *environment.interval);
        const auto last_candidate =
            lagcomp::time_to_tick(width + center_time, *environment.interval);
        const auto newest = ring.at(0).header.simulation_tick;
        auto first = ring.at(ring.count - 1).header.simulation_tick;
        if (first <= first_candidate)
            first = first_candidate;
        auto last = newest;
        if (last_candidate <= newest)
            last = last_candidate;
        if (mode == 1) {
            const auto recent =
                add_tick(newest, -lagcomp::time_to_tick(0.1f, *environment.interval));
            if (first < recent && recent < last)
                first = recent;
        }
        const auto& fields = environment.selection.attachments.bones.fields;
        const StudioOffsets studio{fields.studio_wrapper_32e4daf1, fields.hitbox_set_c2b6fb10};
        const AccuracyHistoryRay ray{frame.origin_dc, direction, range};
        for (const auto hitbox : {0u, 3u})
            search_target_accuracy_history(target, studio, environment.targeting.offsets.origin,
                                           ring, ray, hitbox, first, last, selected, best_cosine,
                                           *environment.interval, environment.matrix_scratch,
                                           environment.rotation);
        if (selected != -1) {
            const auto& base = environment.selection;
            const lagcomp::HistoryTickTiming timing{
                *environment.time,
                latency,
                *environment.interpolation_43476928,
                *environment.interval,
                lagcomp::resolve_history_tick_timing(base.timing).maximum_unlag_ms,
                read<std::int32_t>(&frame, 0xD4),
                std::bit_cast<std::int32_t>(frame.field_18c),
                base.timing.live_globals,
                &frame.field_d4,
                &frame.field_18c};
            const lagcomp::RecordSelectionLiveContext current{base.players,
                                                              base.preparation,
                                                              timing,
                                                              base.rotation,
                                                              base.interpolation_scratch,
                                                              base.attachments,
                                                              base.current_bone_counter};
            lagcomp::select_and_apply_history_record_live(scope, current, selected, 0);
        }
    }
    auto angles = command.angles;
    if (!environment.suppress_recoil_43467144)
        add_player_recoil(angles, read<Vector>(local_player, environment.player_punch),
                          environment.recoil_scale, environment.normalize_angles_434693ec);
    const RageTrigonometry trigonometry{environment.rotation.sine, environment.rotation.cosine};
    const auto basis = spread_basis(angles, trigonometry);
    std::array<Vector, 32> samples;
    const bool direct = bool(frame.flags & 0x4000u);
    if (!direct) {
        rage::SpreadParameters parameters{std::uint32_t(frame.weapon_id_d8),
                                          std::bit_cast<std::int32_t>(frame.field_b0),
                                          read<float>(&frame, 0xD0),
                                          std::bit_cast<float>(frame.field_c4),
                                          std::bit_cast<float>(frame.field_100),
                                          1,
                                          false,
                                          frame.weapon_kind_124};
        parameters.shotgun_patterns = convar_enabled(environment.shotgun_patterns_convar);
        samples = accuracy_spread_directions(parameters, read<std::uint32_t>(&frame, 0x7C), basis,
                                             environment.spread_tables, environment.random,
                                             environment.shotgun_pattern, trigonometry);
    }
    auto* weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
    auto query =
        rage::prepare_target_bullet(weapon, environment.targeting.offsets.weapon_identifier,
                                    hit && bool(profile.flags & 0x80u));
    BulletTraceFilter native_filter(environment.trace_filter_vtable);
    const auto filter = native_filter.binding();
    FilterBallistics engine(environment.ballistics);
    const auto source = std::uint32_t(reinterpret_cast<std::uintptr_t>(local_player));
    const auto trace = [&](const Vector& ray) {
        return rage::trace_target_direction(engine, source, frame.origin_dc, ray, filter, query) !=
               0;
    };
    const auto accepted = [&] {
        return !hit || trigger_hit_allowed_live(
                           profile,
                           {query.damage, std::uint32_t(query.trace.hitgroup),
                            reinterpret_cast<void*>(std::uintptr_t(query.trace.entity))},
                           environment.targets.data());
    };
    std::int32_t hits = 0;
    if (direct || (hit && profile.minimum_hit_chance == 0)) {
        if (trace(basis.center) && accepted())
            hits = 1;
    } else if (!hit || trace(basis.center)) {
        for (const auto& sample : samples)
            if (trace(sample) && accepted())
                ++hits;
    }
    if (hits < 1)
        return -1;
    if (hit) {
        profile.accuracy_boost = 0;
        if (!direct) {
            auto required =
                std::bit_cast<std::int32_t>(std::uint32_t(profile.minimum_hit_chance) << 5) / 100;
            if (required < 1)
                required = 1;
            if (hits < required)
                return -1;
        }
        *hit = true;
    }
    return selected == -1 ? -1
                          : add_tick(selected, lagcomp::time_to_tick(read<float>(&frame, 0x198),
                                                                     *environment.interval));
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: accuracy_history.cpp
// ============================================================================

namespace skeet::legit {
namespace {
/* [stripped helper] */
/* [dedup add] */
/* [dedup multiply] */
float distance_squared(const Vector& first, const Vector& second) {
    const auto y = first[1] - second[1], x = first[0] - second[0], z = first[2] - second[2];
    return (y * y + x * x) + z * z;
}
Matrix3x4 angle_matrix(const Vector& angles, const Vector& origin,
                       const lagcomp::RotationMath& math) {
    constexpr float radians = 0.01745329238474369f;
    const auto sy = math.sine(angles[1] * radians), cy = math.cosine(angles[1] * radians);
    const auto sp = math.sine(angles[0] * radians), cp = math.cosine(angles[0] * radians);
    const auto sr = math.sine(angles[2] * radians), cr = math.cosine(angles[2] * radians);
    const auto srcy = sr * cy, srsy = sr * sy, crcy = cr * cy, crsy = cr * sy;
    return {{{cp * cy, srcy * sp - crsy, crcy * sp + srsy, origin[0]},
             {cp * sy, srsy * sp + crcy, crsy * sp - srcy, origin[1]},
             {-sp, sr * cp, cr * cp, origin[2]}}};
}
Matrix3x4 concatenate(const Matrix3x4& left, const Matrix3x4& right) {
    Matrix3x4 result;
    for (unsigned row = 0; row != 3; ++row)
        for (unsigned column = 0; column != 4; ++column)
            result.rows[row][column] =
                add(add(add(multiply(left.rows[row][2], right.rows[2][column]),
                            multiply(left.rows[row][1], right.rows[1][column])),
                        multiply(left.rows[row][0], right.rows[0][column])),
                    column == 3 ? left.rows[row][3] : 0.0f);
    return result;
}
Matrix3x4 rebase_matrix(const lagcomp::LagRecordHeader& old, const Vector& origin,
                        const Vector& angles, AccuracyMatrixScratch& scratch,
                        const lagcomp::RotationMath& math) {
    const auto old_rotation = angle_matrix({0.0f, old.animation_yaw, 0.0f}, {}, math);
    Matrix3x4 inverse;
    for (unsigned row = 0; row != 3; ++row)
        for (unsigned column = 0; column != 3; ++column)
            inverse.rows[row][column] = old_rotation.rows[column][row];
    inverse.rows[0][3] = multiply(add(add(multiply(old.origin[0], inverse.rows[0][0]),
                                          multiply(old.origin[1], inverse.rows[0][1])),
                                      multiply(old.origin[2], inverse.rows[0][2])),
                                  -1.0f);
    inverse.rows[1][3] = multiply(
        add(add(multiply(old.origin[0],
                         engine::read_opaque_native_float(scratch.inverse_row1.data())),
                multiply(old.origin[1],
                         engine::read_opaque_native_float(scratch.inverse_row1.data() + 1))),
            multiply(old.origin[2],
                     engine::read_opaque_native_float(scratch.inverse_row1.data() + 2))),
        -1.0f);
    inverse.rows[2][3] = multiply(
        add(add(multiply(old.origin[0],
                         engine::read_opaque_native_float(scratch.inverse_row2.data())),
                multiply(old.origin[1],
                         engine::read_opaque_native_float(scratch.inverse_row2.data() + 1))),
            multiply(old.origin[2],
                     engine::read_opaque_native_float(scratch.inverse_row2.data() + 2))),
        -1.0f);
    for (unsigned axis = 0; axis != 3; ++axis) {
        scratch.inverse_row1[axis] = inverse.rows[1][axis];
        scratch.inverse_row2[axis] = inverse.rows[2][axis];
    }
    return concatenate(angle_matrix(angles, origin, math), inverse);
}
}
std::int32_t select_accuracy_hitbox(const lagcomp::RecordRing& ring, std::int32_t requested,
                                    std::uint32_t hitbox, Vector& origin, Matrix3x4& matrix,
                                    float interval, AccuracyMatrixScratch& scratch,
                                    const lagcomp::RotationMath& math) {
    const lagcomp::LagRecord* previous = nullptr;
    for (std::uint32_t ordinal = 0; ordinal != ring.count; ++ordinal) {
        const auto& record = ring.at(ordinal);
        const auto& header = record.header;
        if (previous && distance_squared(previous->header.origin, header.origin) > 4096.0f)
            return -1;
        if (header.simulation_tick <= requested) {
            if (!(header.flags & 4u))
                return -1;
            if (!previous || requested <= header.simulation_tick ||
                previous->header.simulation_tick <= header.simulation_tick) {
                origin = header.origin;
                matrix = record.matrices[hitbox];
            } else {
                const auto old_time = float(header.simulation_tick) * interval;
                const auto fraction =
                    (float(requested) * interval - old_time) /
                    (float(previous->header.simulation_tick) * interval - old_time);
                Vector angles{0.0f, header.animation_yaw, 0.0f};
                if (previous->header.animation_yaw != header.animation_yaw) {
                    const auto first = lagcomp::history_angle_quaternion(angles, math);
                    const auto second = lagcomp::history_angle_quaternion(
                        {0.0f, previous->header.animation_yaw, 0.0f}, math);
                    angles = lagcomp::history_quaternion_angles(
                        lagcomp::slerp_history_quaternion(
                            first, lagcomp::align_history_quaternion(first, second), fraction,
                            math),
                        math);
                }
                for (unsigned axis = 0; axis != 3; ++axis)
                    origin[axis] =
                        (previous->header.origin[axis] - header.origin[axis]) * fraction +
                        header.origin[axis];
                matrix = concatenate(rebase_matrix(header, origin, angles, scratch, math),
                                     record.matrices[hitbox]);
            }
            return requested;
        }
        if (header.flags & 0x200u)
            return -1;
        previous = &record;
    }
    return -1;
}
void search_accuracy_history(const lagcomp::RecordRing& ring, const AccuracyHistoryRay& ray,
                             const Vector& newest, const AcquisitionHitbox& descriptor,
                             std::uint32_t hitbox, std::int32_t first, std::int32_t last,
                             std::int32_t& best_tick, float& best_cosine, float interval,
                             AccuracyMatrixScratch& scratch, const lagcomp::RotationMath& math) {
    const auto range_squared = ray.maximum_origin_distance * ray.maximum_origin_distance;
    const Vector center{(descriptor.maximum[0] + descriptor.minimum[0]) * 0.5f,
                        (descriptor.maximum[1] + descriptor.minimum[1]) * 0.5f,
                        (descriptor.maximum[2] + descriptor.minimum[2]) * 0.5f};
    for (auto tick = last; tick >= first; --tick) {
        Vector origin;
        Matrix3x4 matrix;
        if (select_accuracy_hitbox(ring, tick, hitbox, origin, matrix, interval, scratch, math) ==
            -1)
            continue;
        Vector point;
        for (unsigned row = 0; row != 3; ++row) {
            const auto& m = matrix.rows[row];
            point[row] = ((m[1] * center[1] + m[0] * center[0]) + m[2] * center[2]) + m[3];
        }
        if (4096.0f > range_squared && distance_squared(origin, newest) > range_squared)
            continue;
        const auto x = point[0] - ray.origin[0], y = point[1] - ray.origin[1],
                   z = point[2] - ray.origin[2];
        const auto inverse = 1.0f / (std::sqrt((x * x + y * y) + z * z) + 1.1920928955078125e-7f);
        const auto cosine = ((inverse * y) * ray.direction[1] + (inverse * x) * ray.direction[0]) +
                            (inverse * z) * ray.direction[2];
        if (cosine > best_cosine) {
            best_tick = tick;
            best_cosine = cosine;
        }
    }
}
void search_target_accuracy_history(const void* target, const StudioOffsets& offsets,
                                    std::ptrdiff_t origin_offset, const lagcomp::RecordRing& ring,
                                    const AccuracyHistoryRay& ray, std::uint32_t hitbox,
                                    std::int32_t first, std::int32_t last, std::int32_t& best_tick,
                                    float& best_cosine, float interval,
                                    AccuracyMatrixScratch& scratch,
                                    const lagcomp::RotationMath& math) {
    const auto set = load_target_hitbox_set(target, offsets);
    if (!set.studio || !set.count)
        return;
    const auto* box = set.descriptors + std::ptrdiff_t(hitbox) * 0x44;
    const AcquisitionHitbox descriptor{read<std::int32_t>(box, 0), read<std::int32_t>(box, 4),
                                       read<Vector>(box, 8), read<Vector>(box, 0x14),
                                       read<float>(box, 0x30)};
    const auto newest = ring.count ? ring.at(0).header.origin : read<Vector>(target, origin_offset);
    search_accuracy_history(ring, ray, newest, descriptor, hitbox, first, last, best_tick,
                            best_cosine, interval, scratch, math);
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: acquisition.cpp
// ============================================================================

namespace skeet::legit {
namespace {
/* [stripped helper] */
void acquire(TargetTrack& track, const ProfileState& state,
             std::span<const AcquisitionHitbox> hitboxes, std::span<const Matrix3x4> matrices,
             const Vector& eye, const Vector& forward, float interval, BallisticEngine& engine,
             const BallisticFilter* supplied, const AcquisitionTraceContext* native,
             TriggerTraceBindings bindings, const std::byte* raw_descriptors = nullptr) {
    auto required = state.enabled ? state.aim_hitgroups : 0u;
    if (state.flags & 0x20u)
        required |= state.trigger_hitgroups;
    std::uint32_t visible = 0;
    if (required && (raw_descriptors || hitboxes.size() >= 7)) {
        float best = -1.0f;
        for (std::size_t i = 0; i != 7; ++i) {
            const auto* raw = raw_descriptors ? raw_descriptors + i * 0x44 : nullptr;
            const auto box =
                raw ? AcquisitionHitbox{read<std::int32_t>(raw, 0), read<std::int32_t>(raw, 4),
                                        read<Vector>(raw, 8), read<Vector>(raw, 0x14),
                                        read<float>(raw, 0x30)}
                    : hitboxes[i];
            if (box.bone < 0 || !(box.radius > 0.0f))
                continue;
            const auto group = 1u << (std::uint32_t(box.hitgroup) & 31u);
            if (!(group & required))
                continue;
            const Vector center{(box.maximum[0] + box.minimum[0]) * 0.5f,
                                (box.maximum[1] + box.minimum[1]) * 0.5f,
                                (box.maximum[2] + box.minimum[2]) * 0.5f};
            Vector point;
            for (unsigned row = 0; row != 3; ++row) {
                const auto& m = matrices[i].rows[row];
                point[row] = ((m[1] * center[1] + m[0] * center[0]) + m[2] * center[2]) + m[3];
            }
            if (!(state.flags & 2u) && bindings.through_smoke &&
                bindings.through_smoke(bindings.context, eye, point))
                continue;
            const Vector delta{point[0] - eye[0], point[1] - eye[1], point[2] - eye[2]};
            const auto squared = (delta[1] * delta[1] + delta[0] * delta[0]) + delta[2] * delta[2];
            BallisticTrace trace;
            if (native) {
                anti_aim::TargetTraceFilter owner(native->engine_filter_vtable, native->entities);
                const auto filter = owner.binding();
                engine.trace_ray({{eye, delta, squared != 0.0f},
                                  0x2004001,
                                  BallisticFilterKind::supplied,
                                  &filter,
                                  0,
                                  0},
                                 trace);
            } else
                engine.trace_ray({{eye, delta, squared != 0.0f},
                                  0x2004001,
                                  BallisticFilterKind::supplied,
                                  supplied,
                                  0,
                                  0},
                                 trace);
            if (1.0f > trace.fraction || trace.all_solid || trace.start_solid)
                continue;
            visible |= group;
            if (state.enabled && (state.aim_hitgroups & group)) {
                const auto length =
                    std::sqrt((delta[0] * delta[0] + delta[1] * delta[1]) + delta[2] * delta[2]);
                const auto inverse = 1.0f / (length + 1.1920928955078125e-7f);
                const auto cosine =
                    ((inverse * delta[1]) * forward[1] + (inverse * delta[0]) * forward[0]) +
                    (inverse * delta[2]) * forward[2];
                if (best < cosine) {
                    track.point = point;
                    best = cosine;
                }
            }
        }
    }
    track.visible_hitgroups = visible;
    if (visible)
        track.elapsed += interval;
    else {
        track.elapsed = 0;
        track.point = {};
        track.velocity_valid = 0;
        track.interval = interval;
    }
}
}
void acquire_visible_target(TargetTrack& track, const ProfileState& state,
                            std::span<const AcquisitionHitbox> hitboxes,
                            std::span<const Matrix3x4> matrices, const Vector& eye,
                            const Vector& forward, float interval, BallisticEngine& engine,
                            const BallisticFilter& filter, TriggerTraceBindings bindings) {
    acquire(track, state, hitboxes, matrices, eye, forward, interval, engine, &filter, nullptr,
            bindings);
}
void acquire_visible_target(TargetTrack& track, const ProfileState& state,
                            std::span<const AcquisitionHitbox> hitboxes,
                            std::span<const Matrix3x4> matrices, const Vector& eye,
                            const Vector& forward, float interval, BallisticEngine& engine,
                            const AcquisitionTraceContext& native, TriggerTraceBindings bindings) {
    acquire(track, state, hitboxes, matrices, eye, forward, interval, engine, nullptr, &native,
            bindings);
}
void acquire_visible_target_live(TargetTrack& track, const ProfileState& state, const void* target,
                                 const StudioOffsets& offsets, std::span<const Matrix3x4> matrices,
                                 const Vector& eye, const Vector& forward, float interval,
                                 BallisticEngine& engine, const AcquisitionTraceContext& native,
                                 TriggerTraceBindings bindings) {
    auto required = state.enabled ? state.aim_hitgroups : 0u;
    if (state.flags & 0x20u)
        required |= state.trigger_hitgroups;
    const std::byte* descriptors = nullptr;
    if (required) {
        const auto set = load_target_hitbox_set(target, offsets);
        if (set.studio && set.count >= 7)
            descriptors = set.descriptors;
    }
    acquire(track, state, {}, matrices, eye, forward, interval, engine, nullptr, &native, bindings,
            descriptors);
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: aim_step.cpp
// ============================================================================

namespace skeet::legit {
namespace {
float normalize_pitch(float angle) noexcept {
    if (std::isnan(angle))
        return 0.0f;
    if (std::fabs(angle) > 180.0f) {
        const auto turns = std::round(std::fabs(angle / 360.0f));
        const auto correction = turns * 360.0f;
        angle = angle < 0.0f ? angle + correction : angle - correction;
    }
    return angle;
}
}
Vector calculate_aim_step(const ProfileState& settings, const AimStepInput& input,
                          RandomBindings random) {
    auto pitch = normalize_pitch((input.yaw_delta * 0.15f) * input.curve + input.pitch_delta);
    auto yaw = input.yaw_delta;
    auto speed = settings.speed;
    if (speed == 0.0f)
        return {};
    const auto length = std::sqrt(pitch * pitch + yaw * yaw);
    if (0.022f > length)
        return {};
    const auto scale = settings.speed_scale_fov;
    if (1.0f > scale) {
        const auto fraction = 1.0f - input.target_fov / settings.maximum_fov;
        const auto inverse_scale = 1.0f - scale * scale;
        speed += ((fraction * speed) - speed) * inverse_scale;
    }
    if (input.target_distance > 500.0f) {
        auto fraction = (input.target_distance - 500.0f) * 0.001f;
        if (!(fraction > 0.5f))
            fraction = 0.5f;
        if (!(fraction < 1.0f))
            fraction = 1.0f;
        const auto multiplier = 1.5f - fraction;
        if (1.0f > multiplier)
            speed *= multiplier;
    }
    if (speed > 0.0f) {
        if (!random.seed || !random.uniform)
            throw std::invalid_argument(
                "Native aim step requires RandomSeed and RandomFloat bindings");
        const auto extent = minimum_number(speed, 0.1f);
        random.seed(random.context, input.command_number);
        speed += random.uniform(random.context, -extent, extent);
    }
    const auto maximum_step = input.time_scale * speed;
    if (0.022f > maximum_step)
        return {};
    if (length > maximum_step) {
        const auto fraction = maximum_step / length;
        yaw *= fraction;
        pitch *= fraction;
    }
    return {pitch, yaw, 0.0f};
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: command.cpp
// ============================================================================

namespace skeet::legit {
namespace {
/* [stripped helper] */
/* [stripped helper] */
std::int32_t difference(std::int32_t first, std::int32_t second) {
    auto value = std::uint32_t(first) - std::uint32_t(second);
    const auto sign = std::bit_cast<std::int32_t>(value) < 0 ? 0xffffffffu : 0u;
    return std::bit_cast<std::int32_t>((value ^ sign) - sign);
}
/* [dedup convar_enabled] */
void clear_track(TargetTrack& track, float interval) {
    track.elapsed = 0;
    track.visible_hitgroups = 0;
    track.point = {};
    track.velocity_valid = 0;
    track.interval = interval;
}
Vector reference_forward(const Vector& angles, const lagcomp::RotationMath& math) {
    constexpr float radians = 0.01745329238474369f;
    const auto yaw = angles[1] * radians;
    const auto sy = math.sine(yaw), cy = math.cosine(yaw);
    const auto pitch = angles[0] * radians;
    const auto sp = math.sine(pitch), cp = math.cosine(pitch);
    Vector result{cp * cy, cp * sy, -sp};
    const auto inverse =
        1.0f / (std::sqrt((result[0] * result[0] + result[1] * result[1]) + result[2] * result[2]) +
                1.1920928955078125e-7f);
    for (auto& component : result)
        component = inverse * component;
    return result;
}
AccuracyEnvironment accuracy_environment(const RuntimeBindings& bindings,
                                         std::span<const TargetTrack, 64> targets) {
    auto result = bindings.accuracy;
    result.time = bindings.times.world_time;
    result.interval = bindings.times.interval;
    result.interpolation_43476928 = bindings.times.interpolation;
    result.maximum_clients = bindings.times.maximum_clients;
    result.player_punch = bindings.offsets.player_punch_1bae1168;
    result.targets = targets;
    return result;
}
Vector aim_command(RuntimeState& state, const client::CommandFrame& frame, void* player,
                   client::Command& command, const RuntimeSettings& settings,
                   RuntimeBindings& bindings, const TargetTrackingBindings& tracking) {
    auto& profile = state.profile;
    Vector output{};
    if (!profile.enabled ||
        !hotkey_active(settings.legit.aim_keys[std::size_t(profile.profile)].value.binding,
                       bindings.input) ||
        state.block_until > *bindings.times.command_time)
        return output;
    auto* weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
    const auto release = [&] {
        release_target(state, *bindings.times.interval, bindings.random);
        return Vector{};
    };
    if (profile.limit_lock_time) {
        const auto ready = engine::invoke<bool>(weapon, 430);
        if (!(ready && (command.movement.buttons & 1u)) &&
            *bindings.times.command_time - state.manual_time > profile.maximum_lock_time)
            return release();
    }
    const auto reference = recoil_reference_angles(state, frame.previous_command.angles,
                                                   bindings.accuracy.recoil_scale);
    // The native owner computes this direction before the angular search even
    // though that search itself uses the two reference Euler components.
    (void)reference_forward(reference, bindings.accuracy.rotation);
    const auto target =
        select_aim_target(state,
                          {frame.origin_dc, reference, *bindings.times.maximum_clients,
                           *bindings.times.command_time, bindings.times.command_time},
                          {bindings.accuracy.targeting, tracking,
                           bindings.accuracy.rotation.arc_tangent2, bindings.random});
    if (!target.index)
        return output;
    if (!(profile.flags & 4u) && read<float>(player, bindings.offsets.blindness_1bc6d160) > 200.0f)
        return release();
    const auto& key = settings.legit.aim_keys[std::size_t(profile.profile)].value.binding;
    if ((key.packed & 0xfffffffcu) != 4u || key.mode == HotkeyMode::always) {
        const auto ready = engine::invoke<bool>(weapon, 430);
        if (!ready && (command.movement.buttons & 1u)) {
            const auto cycle = engine::invoke<float>(weapon, 433, 0);
            state.block_until = *bindings.times.command_time + cycle;
            return release();
        }
    }
    const auto frame_angles = frame.previous_command.angles;
    const auto ready =
        recoil_shot_ready(profile, weapon, command, bindings.offsets.ammunition_ca5ec698);
    const auto& recoil = profile.recoil_history[ready ? 1u : 0u];
    const auto pitch = normalize_movement_angle(target.angles[0] - (recoil[0] + frame_angles[0]));
    const auto yaw = normalize_movement_angle(target.angles[1] - (recoil[1] + frame_angles[1]));
    if (!(command.movement.buttons & 1u)) {
        const auto motion = std::sqrt(state.filtered_motion[0] * state.filtered_motion[0] +
                                      state.filtered_motion[1] * state.filtered_motion[1]);
        if (!(motion < 0.01f)) {
            const auto negative_yaw = -yaw;
            const auto inverse = 1.0f / (std::sqrt(pitch * pitch + negative_yaw * negative_yaw) +
                                         1.1920928955078125e-7f);
            const auto cosine = ((inverse * pitch) * state.filtered_motion[0] -
                                 (inverse * negative_yaw) * state.filtered_motion[1]) /
                                motion;
            if (cosine < -0.5f) {
                if (profile.secondary_flags & 2u) {
                    if (profile.secondary_flags & 4u)
                        state.block_until = *bindings.times.command_time + 0.1f;
                    return release();
                }
                if (target.field_of_view > std::sqrt(motion) * 10.0f)
                    return release();
            }
        }
    }
    output = calculate_aim_step(profile,
                                {pitch, yaw, target.field_of_view, target.distance,
                                 state.time_scale, state.curve, command.number},
                                bindings.random);
    if (recoil_shot_ready(profile, weapon, command, bindings.offsets.ammunition_ca5ec698)) {
        output[0] -= profile.recoil_history[2][0];
        output[1] -= profile.recoil_history[2][1];
        for (unsigned axis = 0; axis != 3; ++axis)
            profile.recoil_history[1][axis] += profile.recoil_history[2][axis];
    } else
        profile.recoil_history[1] = profile.recoil_history[0];
    if (profile.flags & 1u)
        bindings.scheduler_flags |= 8u;
    return output;
}
}
void capture_legit_view(RuntimeState& state, void* engine_client) {
    Vector angles;
    engine::invoke<void>(engine_client, 18, angles.data());
    state.previous_view_valid = state.current_view_valid;
    state.previous_view = state.current_view;
    state.current_view = angles;
    if (!state.current_view_valid)
        state.current_view_valid = 1;
}
void process_legit_command(RuntimeState& state, client::CommandFrame& frame, void* player,
                           client::Command& command, const RuntimeSettings& settings,
                           RuntimeBindings& bindings) {
    const auto& offsets = bindings.offsets;
    const auto shots = read<std::int32_t>(player, offsets.player_shots_8cfc633a);
    state.new_shot = state.profile.shots < shots;
    state.profile.shots = shots;
    if (settings.rage_enabled.value && hotkey_active(settings.rage_key, bindings.input))
        return;
    state.pending_step = {};
    state.pending_fraction = 0;
    state.time_scale = *bindings.times.frame_delta * 64.0f;
    if (!frame.weapon_address)
        return;
    const auto requested = std::int32_t(std::bit_cast<std::int8_t>(frame.weapon_kind_124));
    if (!select_weapon_profile(settings.legit, state.profile, requested, command.movement.buttons))
        return;
    const FirstShotFrame first_shot{read<std::uint32_t>(&frame, 0x44), frame.origin_dc,
                                    read<Vector>(&frame, 0x174), *bindings.times.maximum_clients};
    apply_first_shot_delay(settings.legit, state.profile, first_shot, bindings.accuracy.targeting,
                           bindings.input, command.movement.buttons);
    if ((read<std::uint32_t>(player, offsets.player_effects_38ca5228) & 8u) ||
        read<std::uint8_t>(player, offsets.player_byte_043fa575) !=
            read<std::uint8_t>(player, offsets.player_byte_334b460c) ||
        difference(command.number, state.last_command) > 1)
        reset_runtime_live(state, bindings.times.interval, bindings.random);
    update_manual_motion(state, command, frame.previous_command.angles,
                         *bindings.times.command_time, bindings.accuracy.rotation);
    const auto forward =
        reference_forward(recoil_reference_angles(state, frame.previous_command.angles,
                                                  bindings.accuracy.recoil_scale),
                          bindings.accuracy.rotation);
    const AcquisitionTraceContext visibility_filter{bindings.accuracy.trace_filter_vtable,
                                                    bindings.accuracy.trace_entities};
    auto& players = bindings.accuracy.selection.players;
    const auto maximum_clients = *bindings.times.maximum_clients;
    for (std::int32_t index = 1; index <= maximum_clients; ++index) {
        auto* target = engine::invoke<void*>(bindings.accuracy.targeting.entity_list, 3, index);
        auto& track = state.targets[std::uint32_t(index) & 63u];
        if (!rage::target_is_eligible(bindings.accuracy.targeting, target)) {
            clear_track(track, *bindings.times.interval);
            continue;
        }
        track.interval = *bindings.times.interval;
        auto& cached = *players.get(std::uint32_t(index));
        const auto& bone_runtime = bindings.accuracy.selection.attachments.bones;
        const LiveHistoricalBoneBindings bones{bindings.historical_offsets,
                                               cached.native,
                                               bone_runtime,
                                               bindings.times.interval,
                                               bindings.times.world_time,
                                               bindings.times.setup_bones_time,
                                               bindings.times.clock_tick};
        if (!prepare_target_bones_live(state.profile, cached.history, target, bindings.target_bones,
                                       bindings.bone_scratch, track.interval, bones)) {
            clear_track(track, *bindings.times.interval);
            continue;
        }
        const StudioOffsets studio{bone_runtime.fields.studio_wrapper_32e4daf1,
                                   bone_runtime.fields.hitbox_set_c2b6fb10};
        acquire_visible_target_live(track, state.profile, target, studio, bindings.target_bones,
                                    frame.origin_dc, forward, *bindings.times.interval,
                                    bindings.accuracy.ballistics, visibility_filter,
                                    bindings.accuracy.trace);
    }
    if (frame.flags & 0x10u)
        return;
    auto decay = bindings.recoil_decay;
    decay.interval = *bindings.times.interval;
    RecoilStateInput recoil{
        read<Vector>(player, offsets.player_punch_1bae1168),
        read<Vector>(&frame, 0x8C),
        read<Vector>(player, offsets.player_punch_velocity_48442721),
        read<float>(player, offsets.player_next_attack_de530a8b),
        read<float>(reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address)),
                    offsets.weapon_next_attack_f71ce331),
        *bindings.times.world_time,
        command.movement.buttons};
    update_recoil_state(state, recoil, bindings.accuracy.recoil_scale, decay, bindings.random);
    auto* weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
    if (!weapon ||
        !lagcomp::history_weapon_is_firearm(
            read<std::uint16_t>(weapon, bindings.accuracy.targeting.offsets.weapon_identifier)) ||
        read<std::int32_t>(weapon, offsets.ammunition_ca5ec698) < 1 ||
        read<std::uint8_t>(weapon, offsets.reloading_b8969256))
        return;
    if (!(state.profile.secondary_flags & 1u) && !state.profile.enabled)
        clear_recoil_history(state.profile);
    std::array<lagcomp::RecordRing, 64> ring_views;
    const TargetTrackingBindings tracking{ring_views,
                                          state.targets,
                                          bindings.historical_offsets.common.velocity,
                                          *bindings.times.history_command_tick,
                                          *bindings.times.interval,
                                          &players,
                                          bindings.times.history_command_tick,
                                          bindings.times.interval};
    auto output = aim_command(state, frame, player, command, settings, bindings, tracking);
    auto& profile = state.profile;
    if ((profile.secondary_flags & 1u) && !state.target && output[0] == 0.0f && output[1] == 0.0f) {
        if (recoil_shot_ready(profile, weapon, command, offsets.ammunition_ca5ec698)) {
            output[0] -= profile.recoil_history[2][0];
            output[1] -= profile.recoil_history[2][1];
            for (unsigned axis = 0; axis != 3; ++axis)
                profile.recoil_history[1][axis] += profile.recoil_history[2][axis];
        } else
            clear_recoil_history(profile);
    }
    if (output[0] != 0.0f || output[1] != 0.0f) {
        output = {normalize_movement_angle(output[0]), normalize_movement_angle(output[1]), 0.0f};
        if (std::sqrt(output[0] * output[0] + output[1] * output[1]) < 2.0f) {
            state.pending_step = output;
            state.pending_fraction = 1.0f;
            apply_pending_aim(state, command, read<float>(&frame, 0x1B4), *bindings.times.interval);
            client::set_command_view_angles(bindings.view_angles, command.angles);
            const auto saved = read<Vector>(&frame, 0x118);
            if (frame.previous_command.angles == saved)
                write(&frame, 0x118, command.angles);
        }
    }
    std::array<float, 64> target_times;
    for (unsigned index = 0; index != 64; ++index)
        target_times[index] = state.targets[index].elapsed;
    const TriggerFrame trigger_frame{frame.flags, std::uint32_t(frame.weapon_id_d8),
                                     *bindings.times.world_time,
                                     read<float>(player, offsets.blindness_1bc6d160)};
    TriggerCommand trigger_command{command.movement.buttons, command.tick};
    if (begin_trigger(settings.legit, profile, trigger_frame, bindings.input, trigger_command)) {
        command.movement.buttons = trigger_command.buttons;
        bool hit = false;
        auto selected_tick = command.tick;
        if (profile.accuracy_boost) {
            auto accuracy = accuracy_environment(bindings, state.targets);
            selected_tick = accuracy_tick(profile, frame, player, command, &hit, accuracy);
        } else {
            auto angles = command.angles;
            add_player_recoil(angles, read<Vector>(player, offsets.player_punch_1bae1168),
                              bindings.accuracy.recoil_scale,
                              bindings.accuracy.normalize_angles_434693ec);
            auto query = rage::prepare_target_bullet(
                weapon, bindings.accuracy.targeting.offsets.weapon_identifier,
                bool(profile.flags & 0x80u));
            const RageTrigonometry trigonometry{bindings.accuracy.rotation.sine,
                                                bindings.accuracy.rotation.cosine};
            const auto basis = spread_basis(angles, trigonometry);
            const rage::SpreadParameters parameters{
                std::uint32_t(frame.weapon_id_d8),
                std::bit_cast<std::int32_t>(frame.field_b0),
                read<float>(&frame, 0xD0),
                std::bit_cast<float>(frame.field_c4),
                std::bit_cast<float>(frame.field_100),
                1,
                convar_enabled(bindings.accuracy.shotgun_patterns_convar),
                frame.weapon_kind_124};
            const TriggerTraceInput trace_input{
                std::uint32_t(reinterpret_cast<std::uintptr_t>(player)),
                frame.origin_dc,
                basis,
                parameters,
                read<std::uint32_t>(&frame, 0x7C),
                target_times,
                state.targets.data()};
            BulletTraceFilter native_filter(bindings.accuracy.trace_filter_vtable);
            const auto filter = native_filter.binding();
            FilterBallistics engine(bindings.accuracy.ballistics);
            hit = trace_trigger(engine, profile, trace_input, filter, query,
                                bindings.accuracy.spread_tables, bindings.accuracy.random,
                                bindings.accuracy.shotgun_pattern, trigonometry,
                                bindings.accuracy.trace);
        }
        // The native continuation reads the command after the complete trace
        // or historical transaction, while selected_tick retains its saved
        // pre-trace value on the ordinary128-seed path.
        trigger_command = {command.movement.buttons, command.tick};
        finish_trigger_live(profile, &frame.flags, bindings.times.world_time, hit, selected_tick,
                            trigger_command, bindings.random);
    }
    command.movement.buttons = trigger_command.buttons;
    command.tick = trigger_command.tick;
    if (profile.accuracy_boost &&
        ((frame.flags & 0x100000u) || ((frame.flags & 0x400u) && (command.movement.buttons & 1u)) ||
         ((frame.flags & 2u) && (command.movement.buttons & 0x800u) &&
          (frame.weapon_id_d8 == 64 || !frame.weapon_kind_124)))) {
        auto accuracy = accuracy_environment(bindings, state.targets);
        const auto tick = accuracy_tick(profile, frame, player, command, nullptr, accuracy);
        if (tick != -1)
            command.tick = tick;
    }
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: historical_bones.cpp
// ============================================================================

namespace skeet::legit {
namespace {
/* [stripped helper] */
/* [stripped helper] */
std::int32_t subtract(std::int32_t a, std::int32_t b) {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) - std::uint32_t(b));
}
}
void refresh_hitbox_attachments(void* entity, const animation::BoneModel& model,
                                std::span<Matrix3x4, 19> hitboxes,
                                std::span<Matrix3x4, 256> scratch, std::ptrdiff_t accessor,
                                AttachmentUpdate update) {
    if (model.studio_header)
        for (std::size_t i = 0; i < std::min(std::size_t(19), model.hitbox_bones.size()); ++i)
            scratch[model.hitbox_bones[i]] = hitboxes[i];
    if (update) {
        const auto readable = read<std::uint32_t>(entity, accessor + 8),
                   writable = read<std::uint32_t>(entity, accessor + 12);
        write(entity, accessor + 8, 0xFFF00u);
        write(entity, accessor + 12, 0xFFF00u);
        update(entity, scratch.data(), 0x300);
        write(entity, accessor + 12, writable);
        write(entity, accessor + 8, readable);
    }
    animation::select_hitbox_bone_matrices(model, scratch, hitboxes);
}
std::int32_t historical_target_bones(const lagcomp::RecordRing& ring, void* entity,
                                     std::int32_t requested, std::span<Matrix3x4, 19> output,
                                     std::span<Matrix3x4, 256> scratch,
                                     const HistoricalBoneBindings& bindings) {
    const lagcomp::LagRecord *previous = nullptr, *selected = nullptr;
    for (std::uint32_t ordinal = 0; ordinal < ring.count; ++ordinal) {
        const auto& record = ring.at(ordinal);
        if (previous) {
            const auto& a = previous->header.origin;
            const auto& b = record.header.origin;
            const auto y = a[1] - b[1], x = a[0] - b[0], z = a[2] - b[2];
            if ((y * y + x * x) + z * z > 4096.0f)
                return -1;
        }
        if (record.header.simulation_tick <= requested) {
            selected = &record;
            break;
        }
        if (record.header.flags & 0x200u)
            return -1;
        previous = &record;
    }
    if (!selected || !(selected->header.flags & 4u))
        return -1;
    std::copy(selected->matrices.begin(), selected->matrices.end(), output.begin());
    auto result = requested;
    if (!previous || requested <= selected->header.simulation_tick ||
        previous->header.simulation_tick <= selected->header.simulation_tick)
        result = selected->header.simulation_tick;
    else {
        const auto old_time = float(selected->header.simulation_tick) * bindings.interval;
        const auto fraction =
            (float(requested) * bindings.interval - old_time) /
            (float(previous->header.simulation_tick) * bindings.interval - old_time);
        for (auto& matrix : output)
            for (unsigned axis = 0; axis != 3; ++axis)
                matrix.rows[axis][3] +=
                    (previous->header.origin[axis] - selected->header.origin[axis]) * fraction;
    }
    const auto& o = bindings.offsets;
    const auto& snapshot = bindings.snapshot;
    const auto old_432 = read<std::uint32_t>(entity, o.common.field_432b1f5b);
    write(entity, o.common.field_432b1f5b, snapshot.field_12c);
    const auto old_0d = read<std::uint32_t>(entity, o.common.field_0d560fac);
    write(entity, o.common.field_0d560fac, snapshot.field_148);
    const auto old_e23 = read<std::array<std::uint32_t, 2>>(entity, o.field_e23a9be1);
    write(entity, o.field_e23a9be1, snapshot.field_1a8);
    if (selected->header.collision_max_z != snapshot.field_130) {
        write(entity, o.common.field_432b1f5b, snapshot.field_144 + snapshot.field_130);
        write(entity, o.common.field_0d560fac, bindings.time);
    }
    auto* collision = engine::invoke<void*>(entity, 3);
    auto* maximum = engine::invoke<Vector*>(collision, 2);
    const auto old_height = (*maximum)[2];
    (*maximum)[2] = selected->header.collision_max_z;
    const auto old_duck = read<std::uint32_t>(entity, o.common.duck_amount);
    write(entity, o.common.duck_amount, snapshot.field_134);
    const auto old_6e = read<std::uint32_t>(entity, o.field_6e9852d6);
    const auto old_2a = read<Vector>(entity, o.field_2a671b42);
    write(entity, o.field_2a671b42, selected->header.origin);
    const auto old_4e = read<Vector>(entity, o.field_4e66bb34);
    write(entity, o.field_4e66bb34, selected->header.origin);
    const auto old_flags = read<std::uint32_t>(entity, o.field_83a6cf2b);
    write(entity, o.field_83a6cf2b, old_flags & 0xFFFFF7FFu);
    refresh_hitbox_attachments(entity, bindings.model, output, scratch, o.common.bone_accessor,
                               bindings.update_attachments);
    write(entity, o.field_83a6cf2b, old_flags);
    write(entity, o.field_4e66bb34, old_4e);
    write(entity, o.field_2a671b42, old_2a);
    write(entity, o.field_6e9852d6, old_6e);
    write(entity, o.common.duck_amount, old_duck);
    (*maximum)[2] = old_height;
    write(entity, o.field_e23a9be1, old_e23);
    write(entity, o.common.field_0d560fac, old_0d);
    write(entity, o.common.field_432b1f5b, old_432);
    return result;
}
bool prepare_target_bones(const ProfileState& state, const lagcomp::RecordRing& ring, void* entity,
                          std::span<Matrix3x4, 19> output, std::span<Matrix3x4, 256> scratch,
                          float& interval, const HistoricalBoneBindings& bindings) {
    if (state.reaction_time > 0.0f && ring.count) {
        const auto delay =
            _mm_cvttss_si32(_mm_set_ss(state.reaction_time / bindings.interval + 0.5f));
        const auto tick = historical_target_bones(
            ring, entity, subtract(bindings.current_tick, delay), output, scratch, bindings);
        if (tick != -1) {
            const auto age = float(subtract(bindings.current_tick, tick)) * bindings.interval;
            if (age > 0.0f && !(0.3f < age))
                interval = age;
            return true;
        }
    }
    const auto success = engine::invoke<bool>(static_cast<std::byte*>(entity) + 4, 13,
                                              scratch.data(), 256, 0xFFF00u, bindings.frame_time);
    if (!success)
        return false;
    animation::select_hitbox_bone_matrices(bindings.model, scratch, output);
    return true;
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: historical_bones_live.cpp
// ============================================================================

namespace skeet::legit {
namespace {
/* [stripped helper] */
/* [stripped helper] */
/* [dedup subtract_i32] */
}
std::int32_t historical_target_bones_live(const lagcomp::RecordRing& ring, void* target,
                                          std::int32_t requested, std::span<Matrix3x4, 19> output,
                                          std::span<Matrix3x4, 256> scratch,
                                          const LiveHistoricalBoneBindings& bindings) {
    const lagcomp::LagRecord *previous = nullptr, *selected = nullptr;
    for (std::uint32_t ordinal = 0; ordinal != ring.count; ++ordinal) {
        const auto& record = ring.at(ordinal);
        if (previous) {
            const auto y = previous->header.origin[1] - record.header.origin[1];
            const auto x = previous->header.origin[0] - record.header.origin[0];
            const auto z = previous->header.origin[2] - record.header.origin[2];
            if ((y * y + x * x) + z * z > 4096.0f)
                return -1;
        }
        if (record.header.simulation_tick <= requested) {
            selected = &record;
            break;
        }
        if (record.header.flags & 0x200u)
            return -1;
        previous = &record;
    }
    if (!selected || !(selected->header.flags & 4u))
        return -1;
    std::copy(selected->matrices.begin(), selected->matrices.end(), output.begin());
    auto result = requested;
    if (!previous || requested <= selected->header.simulation_tick ||
        previous->header.simulation_tick <= selected->header.simulation_tick)
        result = selected->header.simulation_tick;
    else {
        const auto old_time = float(selected->header.simulation_tick) * (*bindings.interval);
        const auto fraction =
            (float(requested) * (*bindings.interval) - old_time) /
            (float(previous->header.simulation_tick) * (*bindings.interval) - old_time);
        for (auto& matrix : output)
            for (unsigned axis = 0; axis != 3; ++axis)
                matrix.rows[axis][3] +=
                    (previous->header.origin[axis] - selected->header.origin[axis]) * fraction;
    }
    const auto& offsets = bindings.offsets;
    const auto& cached = bindings.cached;
    const auto old_432 = read<std::uint32_t>(target, offsets.common.field_432b1f5b);
    write(target, offsets.common.field_432b1f5b, read<std::uint32_t>(&cached, 0x12C));
    const auto old_0d = read<std::uint32_t>(target, offsets.common.field_0d560fac);
    write(target, offsets.common.field_0d560fac, read<std::uint32_t>(&cached, 0x148));
    const auto old_e23 = read<std::array<std::uint32_t, 2>>(target, offsets.field_e23a9be1);
    write(target, offsets.field_e23a9be1, read<std::array<std::uint32_t, 2>>(&cached, 0x1A8));
    if (selected->header.collision_max_z != read<float>(&cached, 0x130)) {
        write(target, offsets.common.field_432b1f5b,
              read<float>(&cached, 0x144) + read<float>(&cached, 0x130));
        write(target, offsets.common.field_0d560fac, *bindings.time);
    }
    auto* collision = engine::invoke<void*>(target, 3);
    auto* maximum = engine::invoke<Vector*>(collision, 2);
    const auto old_height = (*maximum)[2];
    (*maximum)[2] = selected->header.collision_max_z;
    const auto old_duck = read<std::uint32_t>(target, offsets.common.duck_amount);
    write(target, offsets.common.duck_amount, read<std::uint32_t>(&cached, 0x134));
    const auto old_6e = read<std::uint32_t>(target, offsets.field_6e9852d6);
    const auto old_2a = read<Vector>(target, offsets.field_2a671b42);
    write(target, offsets.field_2a671b42, selected->header.origin);
    const auto old_4e = read<Vector>(target, offsets.field_4e66bb34);
    write(target, offsets.field_4e66bb34, selected->header.origin);
    const auto old_flags = read<std::uint32_t>(target, offsets.field_83a6cf2b);
    write(target, offsets.field_83a6cf2b, old_flags & 0xFFFFF7FFu);
    bindings.bones.refresh_hitbox_attachments(target, output, scratch);
    write(target, offsets.field_83a6cf2b, old_flags);
    write(target, offsets.field_4e66bb34, old_4e);
    write(target, offsets.field_2a671b42, old_2a);
    write(target, offsets.field_6e9852d6, old_6e);
    write(target, offsets.common.duck_amount, old_duck);
    (*maximum)[2] = old_height;
    write(target, offsets.field_e23a9be1, old_e23);
    write(target, offsets.common.field_0d560fac, old_0d);
    write(target, offsets.common.field_432b1f5b, old_432);
    return result;
}
bool prepare_target_bones_live(const ProfileState& profile, const lagcomp::RecordRing& ring,
                               void* target, std::span<Matrix3x4, 19> output,
                               std::span<Matrix3x4, 256> scratch, float& interval,
                               const LiveHistoricalBoneBindings& bindings) {
    if (profile.reaction_time > 0.0f && ring.count) {
        const auto current = *bindings.current_tick;
        const auto delay =
            _mm_cvttss_si32(_mm_set_ss(profile.reaction_time / *bindings.interval + 0.5f));
        const auto selected = historical_target_bones_live(ring, target, subtract(current, delay),
                                                           output, scratch, bindings);
        if (selected != -1) {
            const auto age = float(subtract(current, selected)) * (*bindings.interval);
            if (age > 0.0f && !(0.3f < age))
                interval = age;
            return true;
        }
    }
    if (!engine::invoke<bool>(static_cast<std::byte*>(target) + 4, 13, scratch.data(), 256,
                              0xFFF00u, *bindings.frame_time))
        return false;
    bindings.bones.select_hitbox_bone_matrices(target, scratch, output);
    return true;
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: manual_motion.cpp
// ============================================================================

namespace skeet::legit {
namespace {
float normalize(float angle) {
    if (std::isnan(angle))
        return 0;
    if (std::fabs(angle) > 180.0f) {
        const auto correction = std::round(std::fabs(angle / 360.0f)) * 360.0f;
        angle = angle < 0.0f ? angle + correction : angle - correction;
    }
    return angle;
}
}
void update_manual_motion(RuntimeState& state, const client::Command& command,
                          const Vector& frame_angles, float time,
                          const lagcomp::RotationMath& math) {
    const auto base = state.previous_view_valid ? state.previous_view : frame_angles;
    const auto pitch = normalize((command.angles[0] - state.remainder[0]) - base[0]);
    const auto yaw = normalize((command.angles[1] - state.remainder[1]) - base[1]);
    state.raw_motion = {state.time_scale * pitch, state.time_scale * yaw, state.time_scale * 0.0f};
    state.remainder = {};
    const auto prior_length = std::sqrt(state.filtered_motion[0] * state.filtered_motion[0] +
                                        state.filtered_motion[1] * state.filtered_motion[1]);
    if (state.raw_motion != state.filtered_motion) {
        const auto old_quaternion = lagcomp::history_angle_quaternion(state.filtered_motion, math);
        const auto new_quaternion = lagcomp::history_angle_quaternion(state.raw_motion, math);
        const auto aligned = lagcomp::align_history_quaternion(old_quaternion, new_quaternion);
        const auto interpolated = lagcomp::slerp_history_quaternion(
            old_quaternion, aligned, state.time_scale * 0.06666667f, math);
        state.filtered_motion = lagcomp::history_quaternion_angles(interpolated, math);
    }
    const auto length = std::sqrt(state.filtered_motion[0] * state.filtered_motion[0] +
                                  state.filtered_motion[1] * state.filtered_motion[1]);
    const auto moving =
        std::fabs(state.raw_motion[0]) >= 0.022f || std::fabs(state.raw_motion[1]) >= 0.022f;
    const auto accelerating = length != 0.0f && length >= prior_length;
    state.profile.secondary_flags = std::uint8_t((state.profile.secondary_flags & 0xF9u) |
                                                 (moving ? 2u : 0u) | (accelerating ? 4u : 0u));
    const auto raw_length = std::sqrt(state.raw_motion[1] * state.raw_motion[1] +
                                      state.raw_motion[0] * state.raw_motion[0]);
    if (raw_length >= 0.022f)
        state.manual_time = time;
    state.last_command = command.number;
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: owner.cpp
// ============================================================================

namespace skeet::legit {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define LEGIT_MEMBER __thiscall
#else
#define LEGIT_MEMBER
#endif
using LifecycleEntry = void(LEGIT_MEMBER*)(void*);
void LEGIT_MEMBER destroy_entry(void*);
void LEGIT_MEMBER forward_reset_entry(void*);
void LEGIT_MEMBER reset_entry(void*);
void LEGIT_MEMBER unchanged_entry(void*) {} //433CCE8E
// Exact primary4346FA04. Slot5 forwards to slot7, in this direction.
const std::array<LifecycleEntry, 9> lifecycle_table{
    destroy_entry,       unchanged_entry, unchanged_entry, unchanged_entry, unchanged_entry,
    forward_reset_entry, unchanged_entry, reset_entry,     unchanged_entry};
void clear_low_byte(float& value) noexcept {
    std::memset(&value, 0, 1);
}
}

void initialize_runtime_state(RuntimeState& state) noexcept {
    state.target_offset = {};
    state.profile.profile = 0;
    clear_low_byte(state.profile.maximum_lock_time); // native byte+824
    state.profile.limit_lock_time = false;
    clear_low_byte(state.current_view[0]);
    state.current_view_valid = 0;
    clear_low_byte(state.previous_view[0]);
    state.previous_view_valid = 0;
    for (auto& target : state.targets) {
        target.elapsed = 0;
        target.visible_hitgroups = 0;
        target.point = {};
        // Native+14 is the first byte of velocity.x, not velocity_valid+20.
        clear_low_byte(target.velocity[0]);
        target.velocity_valid = 0;
    }
}

class LifecycleOwner {
public:
    const LifecycleEntry* vtable = nullptr;
    OwnerBindings bindings;
    client::NativeCommandPhaseReceiver command_receiver;

    explicit LifecycleOwner(OwnerBindings context) : bindings(context) {
        bindings.lifecycle.append(this); //434845D7 ->433676A0, primary null
        bindings.command_owners.append(command_receiver, &bindings.commands); //434845DF,+8
        bindings.empty_tick_owners.append(bindings.commands); //434845E7,+10
        vtable = lifecycle_table.data();
        bindings.command_owners.publish(command_receiver, client::CommandOwnerId::legit); //434845FA
        bindings.empty_tick_owners.publish(
            bindings.commands); //43484601, after primary/command tables
        register_settings(bindings.configuration, bindings.settings);
        initialize_runtime_state(bindings.commands.state);
        finish_settings_initialization(bindings.settings);
    }
};

namespace {
void LEGIT_MEMBER destroy_entry(void* object) {
    auto* owner = static_cast<LifecycleOwner*>(object);
    destroy_legit_owner(owner, owner->bindings.active);
}
void LEGIT_MEMBER forward_reset_entry(void* object) { //433B06E3
    auto* owner = static_cast<LifecycleOwner*>(object);
    owner->vtable[7](object);
}
void LEGIT_MEMBER reset_entry(void* object) { //433C8F71
    auto& command = static_cast<LifecycleOwner*>(object)->bindings.commands;
    command.reset();
}
}

std::size_t legit_owner_size() noexcept {
    return sizeof(LifecycleOwner);
}
LifecycleOwner& construct_legit_owner(void* storage, OwnerBindings bindings) {
    if (&bindings.commands.settings.legit != &bindings.settings)
        throw std::invalid_argument("LEGIT constructor requires its canonical command settings");
    return *::new (storage) LifecycleOwner(bindings);
}
LifecycleOwner* create_legit_owner(OwnerBindings bindings) { //4347FEBC
    static_assert(sizeof(LifecycleOwner) <= 0x1310);
    const auto memory = bindings.lifecycle.memory;
    auto* raw = memory.allocate(memory.process_heap, 8, 0x1330);
    if (!raw) {
        bindings.active = nullptr; //4347FED3 / recorded4347FED5 MOV[global],EAX
        return nullptr;
    }
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address);
    constexpr std::uint32_t allocation_size = 0x1330;
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    // HeapZeroMemory covered these fields inside the native allocation. The
    // canonical source storage is borrowed by both command interfaces.
    bindings.settings = Settings{};
    bindings.commands.clear_allocated_storage();
    auto& owner = construct_legit_owner(reinterpret_cast<void*>(header + 16), bindings);
    bindings.active = &owner; //mem_int3.h:4347FED5 MOV[4347588C],EAX
    return &owner;
}
void destroy_legit_owner(LifecycleOwner* owner, LifecycleOwner*& active) {
    active = nullptr;
    if (!owner)
        return;
    const auto memory = owner->bindings.lifecycle.memory;
    owner->~LifecycleOwner();
    auto* bytes = reinterpret_cast<std::byte*>(owner);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void* legit_lifecycle_object(LifecycleOwner& owner) noexcept {
    return &owner;
}
const Settings* legit_owner_settings(const LifecycleOwner* owner) noexcept {
    return owner ? &owner->bindings.settings : nullptr;
}
#undef LEGIT_MEMBER
} // namespace skeet::legit

// ============================================================================
// Subcomponent: profile.cpp
// ============================================================================

namespace skeet::legit {
bool recoil_compensation_enabled(const Settings* settings, std::uint32_t category,
                                 InputBitmap input) {
    return !settings ||
           (settings->enabled.value[category] &&
            hotkey_active(settings->aim_keys[category].value.binding, input)) ||
           settings->standalone_recoil.value[category];
}
void clear_recoil_history(ProfileState& state) noexcept {
    state.recoil_history = {};
}
bool select_weapon_profile(const Settings& s, ProfileState& state, std::int32_t requested,
                           std::uint32_t command_buttons) {
    if (requested < 0)
        throw std::invalid_argument("Native weapon profile must be a nonnegative signed byte");
    const auto i = std::size_t(requested < 9 ? requested : 0);
    if (requested < 9)
        state.profile = requested;
    state.accuracy_boost = s.accuracy_boost.value[i];
    state.accuracy_range = s.accuracy_range.value[i];
    state.secondary_flags = (state.secondary_flags & 0xFEu) | (s.standalone_recoil.value[i] & 1u);
    state.enabled = s.enabled.value[i];
    state.flags = (state.flags & 0xDFu) | ((s.trigger_enabled.value[i] << 5) & 0x20u);
    if (!state.accuracy_boost && !(state.secondary_flags & 1u) && !state.enabled &&
        !(state.flags & 0x20u)) {
        clear_recoil_history(state);
        return false;
    }
    state.speed =
        float((command_buttons & 1u) ? s.attack_speed.value[i] : s.speed.value[i]) / 500.0f;
    state.speed_scale_fov = float(s.speed_scale_fov.value[i]) * 0.01f;
    if (s.maximum_lock_time.value[i] == 1001)
        state.limit_lock_time = false;
    else {
        state.limit_lock_time = true;
        state.maximum_lock_time = float(s.maximum_lock_time.value[i]) * 0.001f;
    }
    state.reaction_time = float(s.reaction_time.value[i]) * 0.001f;
    state.maximum_fov = minimum_number(60.0f, float(s.maximum_fov.value[i]) * 0.1f);
    state.recoil_pitch = float(s.recoil_pitch.value[i]) * 0.01f;
    state.recoil_yaw = float(s.recoil_yaw.value[i]) * 0.01f;
    auto flag = [&](std::uint8_t value, unsigned bit) {
        const auto mask = 1u << bit;
        state.flags = std::uint8_t((state.flags & ~mask) | ((unsigned(value) << bit) & mask));
    };
    flag(s.quick_stop.value[i], 0);
    flag(s.through_smoke.value[i], 1);
    flag(s.while_blind.value[i], 2);
    flag(s.shoot_smoke.value[i], 4);
    flag(s.shoot_blind.value[i], 3);
    state.aim_hitgroups =
        (s.head.value[i] ? 2u : 0u) | (s.chest.value[i] ? 4u : 0u) | (s.stomach.value[i] ? 8u : 0u);
    state.first_shot_range = (s.aim_keys[i].value.binding.packed & 0xFFFFFFFCu) == 4
                                 ? float(s.first_shot_delay.value[i])
                                 : 0.0f;
    state.minimum_hit_chance = s.minimum_hit_chance.value[i];
    state.trigger_reaction = float(s.trigger_reaction.value[i]) * 0.001f;
    flag(s.burst_fire.value[i], 6);
    state.burst_time = float(s.burst_time.value[i]) * 0.001f;
    state.minimum_damage = s.minimum_damage.value[i];
    flag(s.penetration.value[i], 7);
    state.trigger_hitgroups = (s.trigger_head.value[i] ? 2u : 0u) |
                              (s.trigger_chest.value[i] ? 4u : 0u) |
                              (s.trigger_stomach.value[i] ? 8u : 0u);
    return true;
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: recoil.cpp
// ============================================================================

namespace skeet::legit {
namespace {
/* [stripped helper] */
float convar_float(const void* object) noexcept {
    for (;;) {
        const auto parent = field<const void*>(object, 0x1C);
        if (parent == object)
            break;
        object = parent;
    }
    return std::bit_cast<float>(field<std::uint32_t>(object, 0x2C) ^
                                std::uint32_t(reinterpret_cast<std::uintptr_t>(object)));
}
}
void decay_recoil(Vector& velocity, Vector& angle, const RecoilDecayBindings& bindings) {
    if (!bindings.exponential)
        throw std::invalid_argument("Native recoil decay requires its exponential boundary");
    const auto linear = convar_float(bindings.linear_decay) * bindings.interval;
    const auto decay =
        bindings.exponential(-(bindings.interval * convar_float(bindings.angle_decay)));
    auto x = angle[0] * decay, y = angle[1] * decay, z = angle[2] * decay;
    angle = {x, y, z};
    const auto length = std::sqrt((x * x + y * y) + z * z);
    if (length == 0.0f || !(length > linear))
        x = y = z = 0.0f;
    else {
        const auto fraction = 1.0f - linear / length;
        z *= fraction;
        y *= fraction;
        x *= fraction;
    }
    angle = {x, y, z};
    const auto interval = bindings.interval;
    angle[0] = (velocity[0] * interval) * 0.5f + x;
    angle[1] = (velocity[1] * interval) * 0.5f + y;
    angle[2] = (velocity[2] * interval) * 0.5f + z;
    const auto velocity_decay =
        bindings.exponential((-convar_float(bindings.velocity_decay)) * bindings.interval);
    const auto vx = velocity[0] * velocity_decay;
    const auto vy = velocity[1] * velocity_decay;
    const auto vz = velocity[2] * velocity_decay;
    velocity = {vx, vy, vz};
    angle[0] = (vx * interval) * 0.5f + angle[0];
    angle[1] = (vy * interval) * 0.5f + angle[1];
    angle[2] = (vz * interval) * 0.5f + angle[2];
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: recoil_state.cpp
// ============================================================================

namespace skeet::legit {
namespace {
/* [stripped helper] */
float scale_value(const void* convar) {
    for (;;) {
        const auto* parent = read<const void*>(convar, 0x1C);
        if (parent == convar)
            break;
        convar = parent;
    }
    return std::bit_cast<float>(read<std::uint32_t>(convar, 0x2C) ^
                                std::uint32_t(reinterpret_cast<std::uintptr_t>(convar)));
}
void normalize(Vector& vector) {
    for (auto& component : vector)
        component = normalize_movement_angle(component);
}
}
void update_recoil_state(RuntimeState& state, const RecoilStateInput& input,
                         const void* recoil_scale, const RecoilDecayBindings& decay,
                         RandomBindings random) {
    auto& profile = state.profile;
    if (!(profile.secondary_flags & 1u) && !profile.enabled)
        return;
    if (input.player_punch[0] == 0.0f)
        state.recoil_velocity[0] = 0.0f;
    if (input.player_punch[1] == 0.0f)
        state.recoil_velocity[1] = 0.0f;
    auto velocity = input.context_velocity;
    decay_recoil(velocity, state.recoil_velocity, decay);
    if ((input.command_buttons & 1u) && state.new_shot) {
        const auto next_attack = maximum_number(input.weapon_next_attack, input.player_next_attack);
        auto future_velocity = input.player_velocity;
        auto future_angle = state.recoil_velocity;
        auto time = input.time;
        std::int32_t iterations = 1;
        if (time < next_attack)
            do {
                decay_recoil(future_velocity, future_angle, decay);
                ++iterations;
                time += decay.interval;
            } while (time < next_attack);
        const auto scale = scale_value(recoil_scale);
        auto& delta = profile.recoil_history[2];
        const auto& accumulated = profile.recoil_history[1];
        delta[0] = (profile.recoil_pitch * scale) * future_angle[0] - accumulated[0];
        delta[1] = (profile.recoil_yaw * scale) * future_angle[1] - accumulated[1];
        delta[2] = future_angle[2] - accumulated[2];
        normalize(delta);
        if (iterations > 1) {
            const auto inverse = 1.0f / float(iterations);
            for (auto& component : delta)
                component *= inverse;
        }
        if (!random.uniform)
            throw std::invalid_argument("LEGIT recoil state requires native RandomFloat");
        const auto maximum = random.uniform(random.context, 0.2f, 0.3f) * state.time_scale;
        const auto length = std::sqrt(delta[0] * delta[0] + delta[1] * delta[1]);
        if (maximum < length) {
            const auto fraction = maximum / length;
            for (auto& component : delta)
                component *= fraction;
        }
    }
    const auto scale = scale_value(recoil_scale);
    profile.recoil_history[0][0] = (profile.recoil_pitch * scale) * state.recoil_velocity[0];
    profile.recoil_history[0][1] = (profile.recoil_yaw * scale) * state.recoil_velocity[1];
}
Vector recoil_reference_angles(const RuntimeState& state, const Vector& frame,
                               const void* recoil_scale) {
    const auto scale = scale_value(recoil_scale);
    return {state.recoil_velocity[0] * scale + frame[0],
            state.recoil_velocity[1] * scale + frame[1], 0.0f};
}
void add_player_recoil(Vector& angles, const Vector& punch, const void* recoil_scale,
                       bool normalized) {
    if (punch[0] == 0.0f && punch[1] == 0.0f)
        return;
    const auto scale = scale_value(recoil_scale);
    for (unsigned axis = 0; axis != 3; ++axis)
        angles[axis] = punch[axis] * scale + angles[axis];
    if (normalized)
        normalize(angles);
}
bool recoil_shot_ready(const ProfileState& profile, void* weapon, const client::Command& command,
                       std::ptrdiff_t ammunition) {
    if (!engine::invoke<bool>(weapon, 430))
        return false;
    if (!(command.movement.buttons & 1u) || profile.shots <= 1)
        return false;
    return read<std::int32_t>(weapon, ammunition) > 0;
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: runtime_owner.cpp
// ============================================================================

namespace skeet::legit {
namespace {
using client::ClientRuntimeStorage;
std::ptrdiff_t offset(ClientRuntimeStorage& s, std::uint32_t identity) {
    const auto& g = s.offsets().globals;
    for (const auto& r : engine::build_receive_offset_recipes())
        if (r.pointer_identity == identity)
            return engine::DecodedOffset(g, identity, r.xor_key_identity, r.initial_xor_key, 0);
    for (const auto& r : engine::build_data_map_offset_recipes())
        if (r.pointer_identity == identity)
            return engine::DecodedOffset(g, identity, r.xor_key_identity, r.initial_xor_key, 0);
    throw std::invalid_argument("LEGIT field has no proven receive/datamap recipe");
}
std::ptrdiff_t derived(ClientRuntimeStorage& s, std::uint32_t pointer, std::uint32_t key,
                       std::uint32_t salt) {
    return engine::DecodedOffset(s.offsets().globals, pointer, key, salt, 0);
}
/* [stripped helper] */
RuntimeNetvars runtime_fields(ClientRuntimeStorage& s) {
    return {offset(s, 0x4346A6DCu),
            offset(s, 0x43468B50u),
            derived(s, 0x4346757Cu, 0x4346707Cu, 0x043FA575u),
            offset(s, 0x4346934Cu),
            offset(s, 0x434682B8u),
            offset(s, 0x43467F74u),
            offset(s, 0x4346A80Cu),
            offset(s, 0x43467A50u),
            offset(s, 0x4346A60Cu),
            derived(s, 0x43467430u, 0x4346A718u, 0xB8969256u),
            derived(s, 0x43467434u, 0x43467FA8u, 0x1BC6D160u)};
}
HistoricalBoneOffsets bone_fields(ClientRuntimeStorage& s,
                                  const lagcomp::EntityStateRuntime& entity) {
    return {entity.offsets,         offset(s, 0x43468EA0u),
            offset(s, 0x43468F88u), derived(s, 0x43468D30u, 0x43468ED4u, 0x2A671B42u),
            offset(s, 0x434670E0u), offset(s, 0x43469224u)};
}
ClientRuntimeStorage& checked(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& engine,
                              client::ConfigurationRuntimeOwner& config,
                              client::HistoryRuntimeOwner& history,
                              engine::SceneQueryRuntimeOwner& scene, animation::RuntimeOwner& bones,
                              client::CommandCaptureRuntimeOwner& capture,
                              const RuntimeServices& services) {
    auto& data = s.data();
    if (&engine.variables.data() != &data || &config.storage() != &s || &history.storage() != &s ||
        &scene.storage() != &s || &scene.players() != &history.players() ||
        &bones.storage() != &s || &bones.players() != &history.players().cache() ||
        &capture.storage() != &s ||
        &capture.globals().engine_client != &data.native_cell<void*>(0x4346A514u))
        throw std::invalid_argument(
            "LEGIT requires the actual shared engine, configuration, history, scene, animation and capture owners");
    if (!services.random.seed || !services.random.uniform || !services.shotgun_pattern.get ||
        !services.rotation.sine || !services.rotation.cosine || !services.rotation.arc_cosine ||
        !services.rotation.arc_tangent2 || !services.exponential || !services.trace.entity_index ||
        !services.trace.through_smoke)
        throw std::invalid_argument(
            "LEGIT requires its genuine RNG, math, entity-index and smoke boundaries");
    for (const auto cell : {0x43467B04u, 0x43467EB8u, 0x4346A334u, 0x4346A338u, 0x43469300u,
                            0x43468D4Cu, 0x43475A60u, 0x43475B90u, 0x43469060u, 0x4346A82Cu})
        if (!data.native_cell<void*>(cell))
            throw std::invalid_argument(
                "LEGIT requires actual clock, bone, filter and smoke publications");
    for (const auto cell : {0x4346794Cu, 0x43468520u, 0x43468D8Cu, 0x43467BB4u, 0x4346A480u})
        if (!engine.variables.at(cell).get())
            throw std::invalid_argument("LEGIT requires its actual acquired ConVars");
    if (data.native_cell<LifecycleOwner*>(0x4347588Cu))
        throw std::logic_error("LEGIT source context must precede its actual feature factory");
    return s;
}
RuntimeServices production_services(ClientRuntimeStorage& s, animation::RuntimeOwner& bones,
                                    client::CrtRuntimeOwner& crt, esp::EspRuntimeOwner& esp) {
    if (&crt.storage() != &s || &esp.storage() != &s)
        throw std::invalid_argument(
            "LEGIT must borrow the same CRT spread and ESP shotgun-pattern owners");
    const auto numeric = lagcomp::native_history_numeric_bindings();
    const auto& trig = bones.trigonometry();
    return {crt.spread().tables(),
            crt.random(),
            esp.shotgun_pattern(),
            {trig.sine, trig.cosine, numeric.arccosine, animation::native_sse_atan2},
            animation::native_sse_exponential,
            engine::BuildLegitBindings{s.data()}.trigger_trace()};
}
}

struct RuntimeOwner::Data final : CommandSource {
    ClientRuntimeStorage& storage;
    const client::EngineRuntimeBindings& engine;
    client::ConfigurationRuntimeOwner& configuration;
    client::HistoryRuntimeOwner& history;
    engine::SceneQueryRuntimeOwner& scene;
    animation::RuntimeOwner& bones;
    client::CommandCaptureRuntimeOwner& capture;
    const CommandFrameInputs* explicit_inputs;
    const RuntimeServices services;
    Settings settings{};
    RuntimeState state{};
    RuntimeSettings runtime_settings;
    RuntimeNetvars offsets;
    HistoricalBoneOffsets historical;
    lagcomp::EntityPreparationLiveGlobals preparation_globals;
    lagcomp::HistoryTickLiveGlobals selection_globals;
    std::array<Matrix3x4, 19> target_bones;
    client::LegitCommandOwner commands;
    OwnerBindings factory;

    Data(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
         client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
         engine::SceneQueryRuntimeOwner& q, animation::RuntimeOwner& b,
         client::CommandCaptureRuntimeOwner& capture_owner, const CommandFrameInputs* inputs,
         RuntimeServices imported)
        : storage(checked(s, e, c, h, q, b, capture_owner, imported)), engine(e), configuration(c),
          history(h), scene(q), bones(b), capture(capture_owner), explicit_inputs(inputs),
          services(imported), runtime_settings{settings, c.rage_settings().global.enabled,
                                               c.rage_settings().global.activation.value.binding},
          offsets(runtime_fields(s)), historical(bone_fields(s, h.entity_runtime())),
          preparation_globals{s.data().native_cell<void*>(0x43467B04u),
                              s.data().native_cell<float*>(0x43467EB8u),
                              s.data().native_cell<float*>(0x4346A334u),
                              s.data().native_cell<lagcomp::TickTimingSamples>(0x43478C70u)},
          selection_globals{
              s.data().native_cell<float*>(0x43467EB8u), s.data().native_cell<float>(0x43476928u),
              s.data().native_cell<float*>(0x4346A338u), c.settings_menu().maximum_unlag},
          commands{state, runtime_settings, *this},
          factory{{s.owner_registries().at(0x43475B78u), s.memory()},
                  s.command_phases(),
                  s.empty_ticks(),
                  s.configuration(),
                  settings,
                  commands,
                  s.data().native_cell<LifecycleOwner*>(0x4347588Cu)} {}
    void require_state(RuntimeState& current) const {
        if (&current != &state)
            throw std::invalid_argument(
                "LEGIT command endpoint does not name this source owner's canonical state");
    }
    void clear_allocated_storage() override {
        // These nineteen matrices were inside the native zeroed allocation.
        // Keep their clearing at successful allocation, before constructor.
        target_bones = {};
    }
    void capture_view(RuntimeState& current) override {
        require_state(current);
        capture_legit_view(current, storage.data().native_cell<void*>(0x4346A514u));
    }
    void reset(RuntimeState& current) override {
        require_state(current);
        reset_runtime_live(
            current, storage.data().native_cell<float*>(0x4346A338u),
            {services.random.context, services.random.seed, services.random.uniform});
    }
    void pending(RuntimeState& current, client::Command& command, float sample_time) override {
        require_state(current);
        apply_pending_aim(current, command, sample_time,
                          *storage.data().native_cell<float*>(0x4346A338u));
    }
    void process(RuntimeState& current, const RuntimeSettings& selected,
                 client::CommandFrame& frame, void* player, client::Command& command) override {
        require_state(current);
        if (&selected != &runtime_settings || !player)
            throw std::invalid_argument(
                "LEGIT requires its canonical settings and this phase's actual local player");
        auto& data = storage.data();
        auto* globals = data.native_cell<std::byte*>(0x43467B04u);
        auto* interval = data.native_cell<float*>(0x4346A338u);
        const RuntimeTimes times{
            interval,
            data.native_cell<float*>(0x4346A334u),
            data.native_cell<float*>(0x43467EB8u),
            reinterpret_cast<float*>(globals + 0x20),
            reinterpret_cast<float*>(globals + 0x10),
            &data.native_cell<float>(0x43476928u),
            data.native_cell<std::int32_t*>(0x43468D4Cu),
            reinterpret_cast<std::int32_t*>(
                static_cast<std::byte*>(data.native_cell<void*>(0x43475A60u)) + 4),
            data.native_cell<std::int32_t*>(0x43469300u)};
        const rage::TargetBindings targeting{player,
                                             data.native_cell<void*>(0x4346A890u),
                                             history.game_type(),
                                             history.teammates_are_enemies(),
                                             history.players().adjustments(),
                                             history.target_offsets()};
        // Native43344C0C/433EEDFF allocate these locals per invocation. Keep
        // their opaque initial stack bytes; no persistent synthetic seeds.
        CommandFrameInputs workspace;
        if (explicit_inputs)
            std::memcpy(&workspace, explicit_inputs, sizeof workspace);
        auto& record_scratch = workspace.record_interpolation;
        auto& accuracy_scratch = workspace.accuracy_interpolation;
        auto& attachment_matrices = workspace.attachment_matrices;
        auto& saved_matrices = workspace.saved_matrices;
        const lagcomp::HistoryTickTiming timing{*times.world_time,
                                                field<float>(&frame, 0x198),
                                                *times.interpolation,
                                                *interval,
                                                configuration.settings_menu().maximum_unlag,
                                                std::bit_cast<std::int32_t>(frame.field_d4),
                                                std::bit_cast<std::int32_t>(frame.field_18c),
                                                &selection_globals,
                                                &frame.field_d4,
                                                &frame.field_18c};
        const lagcomp::EntityPreparationContext preparation{
            history.entity_runtime(),
            std::bit_cast<std::uint32_t>(*times.world_time),
            field<float>(globals, 0),
            *times.command_time,
            data.native_cell<lagcomp::TickTimingSamples>(0x43478C70u),
            nullptr,
            &preparation_globals};
        const lagcomp::RecordSelectionLiveContext selection{
            history.players().cache(),
            preparation,
            timing,
            services.rotation,
            record_scratch,
            {bones.bones(), attachment_matrices, historical.field_6e9852d6,
             historical.field_2a671b42, historical.field_4e66bb34, historical.field_83a6cf2b},
            data.native_cell<std::uint32_t*>(0x43469060u)};
        const anti_aim::RawTargetTraceEntities entities{
            targeting.entity_list, std::uint32_t(offset(storage, 0x434683C8u)),
            std::uint32_t(offset(storage, 0x43469344u) - 0x18)};
        AccuracyEnvironment accuracy{targeting,
                                     history.entity_runtime(),
                                     selection,
                                     offsets.player_punch_1bae1168,
                                     engine.variables.at(0x4346794Cu),
                                     configuration.rage_settings().global.remove_recoil.value,
                                     configuration.anti_untrusted().enabled.value,
                                     times.world_time,
                                     times.interpolation,
                                     interval,
                                     times.maximum_clients,
                                     saved_matrices,
                                     accuracy_scratch,
                                     scene.queries().ballistics(),
                                     services.spread_tables,
                                     services.random,
                                     services.shotgun_pattern,
                                     engine.variables.at(0x43468520u),
                                     services.rotation,
                                     state.targets,
                                     services.trace,
                                     data.native_cell<void**>(0x43475B90u),
                                     entities};
        const RecoilDecayBindings decay{
            engine.variables.at(0x43468D8Cu), engine.variables.at(0x43467BB4u),
            engine.variables.at(0x4346A480u), *interval, services.exponential};
        RuntimeBindings bindings{
            offsets,
            times,
            accuracy,
            historical,
            target_bones,
            attachment_matrices,
            decay,
            {services.random.context, services.random.seed, services.random.uniform},
            storage.input_runtime().bitmap(),
            capture.view_angles(),
            data.native_cell<std::uint32_t>(0x434759C0u)};
        process_legit_command(current, frame, player, command, selected, bindings);
    }
};

RuntimeOwner::RuntimeOwner(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
                           client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
                           engine::SceneQueryRuntimeOwner& q, animation::RuntimeOwner& b,
                           client::CommandCaptureRuntimeOwner& capture,
                           client::CrtRuntimeOwner& crt, esp::EspRuntimeOwner& esp)
    : RuntimeOwner(s, e, c, h, q, b, capture, production_services(s, b, crt, esp)) {}
RuntimeOwner::RuntimeOwner(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
                           client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
                           engine::SceneQueryRuntimeOwner& q, animation::RuntimeOwner& b,
                           client::CommandCaptureRuntimeOwner& capture,
                           client::CrtRuntimeOwner& crt, esp::EspRuntimeOwner& esp,
                           CommandFrameInputs& incoming)
    : RuntimeOwner(s, e, c, h, q, b, capture, incoming, production_services(s, b, crt, esp)) {}
RuntimeOwner::RuntimeOwner(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
                           client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
                           engine::SceneQueryRuntimeOwner& q, animation::RuntimeOwner& b,
                           client::CommandCaptureRuntimeOwner& capture,
                           CommandFrameInputs& incoming, RuntimeServices services)
    : data_(std::make_unique<Data>(s, e, c, h, q, b, capture, &incoming, services)) {}
RuntimeOwner::RuntimeOwner(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
                           client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
                           engine::SceneQueryRuntimeOwner& q, animation::RuntimeOwner& b,
                           client::CommandCaptureRuntimeOwner& capture, RuntimeServices services)
    : data_(std::make_unique<Data>(s, e, c, h, q, b, capture, nullptr, services)) {}
RuntimeOwner::~RuntimeOwner() = default;
ClientRuntimeStorage& RuntimeOwner::storage() const noexcept {
    return data_->storage;
}
Settings& RuntimeOwner::settings() noexcept {
    return data_->settings;
}
RuntimeState& RuntimeOwner::state() noexcept {
    return data_->state;
}
client::LegitCommandOwner& RuntimeOwner::commands() noexcept {
    return data_->commands;
}
OwnerBindings& RuntimeOwner::factory_bindings() noexcept {
    return data_->factory;
}
void RuntimeOwner::bind_initializer(client::NativeStartupSources& sources) {
    if (&sources.data() != &storage().data())
        throw std::invalid_argument("LEGIT initializer requires its canonical source data");
    sources.bind_initializer({0x4347FEBCu, data_.get(), [](void* value) {
                                  (void)create_legit_owner(static_cast<Data*>(value)->factory);
                              }});
}
} //namespace skeet::legit

// ============================================================================
// Subcomponent: state.cpp
// ============================================================================

namespace skeet::legit {
void release_target(RuntimeState& state, float interval, RandomBindings random) {
    if (!state.target)
        return;
    state.target = 0;
    if (!random.seed || !random.uniform)
        throw std::invalid_argument("Target release requires native RandomSeed/RandomFloat");
    const auto ticks = _mm_cvttss_si32(_mm_set_ss(1.0f / interval));
    if (!ticks)
        throw std::domain_error("Native target-release interval has no tick divisor");
    random.seed(random.context, state.last_command / ticks);
    state.curve = random.uniform(random.context, -1.5f, 1.5f);
}
void reset_runtime_live(RuntimeState& state, const float* interval, RandomBindings random) {
    state.pending_step = {};
    state.remainder = {};
    state.profile.previous_buttons = 0;
    state.new_shot = 0;
    state.profile.shots = 0;
    clear_recoil_history(state.profile);
    release_target(state, *interval, random);
    state.recoil_velocity = {};
    state.profile.burst_until = 0;
    state.block_until = 0;
    state.filtered_motion = {};
    state.manual_time = 0;
    state.profile.secondary_flags &= 0xF9u;
    state.pending_fraction = 0;
    state.target = state.previous_target = 0;
    state.current_view_valid = state.previous_view_valid = 0;
    for (auto& target : state.targets) {
        target.elapsed = 0;
        target.visible_hitgroups = 0;
        target.point = {};
        target.velocity_valid = 0;
        target.interval = *interval;
    }
}
void reset_runtime(RuntimeState& state, float interval, RandomBindings random) {
    reset_runtime_live(state, &interval, random);
}
void apply_pending_aim(RuntimeState& state, client::Command& command, float delta_time,
                       float interval) {
    if (state.pending_fraction <= 0.0f) {
        state.pending_fraction = 0;
        return;
    }
    const auto fraction = _mm_min_ss(
        _mm_max_ss(_mm_set_ss(delta_time / interval), _mm_setzero_ps()), _mm_set_ss(1.0f));
    const auto applied = _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(state.pending_fraction), fraction));
    const auto pitch = state.pending_step[0] * applied;
    const auto yaw = state.pending_step[1] * applied;
    command.angles[0] += pitch;
    command.angles[1] += yaw;
    state.pending_fraction -= applied;
    if (!command.tick) {
        state.remainder[0] = pitch + state.remainder[0];
        state.remainder[1] = yaw + state.remainder[1];
    }
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: studio.cpp
// ============================================================================

namespace skeet::legit {
namespace {
/* [stripped helper] */
}
StudioHitboxSet load_target_hitbox_set(const void* entity, const StudioOffsets& offsets) {
    const auto* wrapper = read<const void*>(entity, offsets.wrapper);
    if (!wrapper)
        return {nullptr, 0, false};
    const auto* header = read<const std::byte*>(wrapper, 0);
    if (!header)
        return {nullptr, 0, false};
    const auto* set = header + read<std::int32_t>(header, 0xB0) +
                      std::ptrdiff_t(read<std::int32_t>(entity, offsets.hitbox_set)) * 12;
    return {set + read<std::int32_t>(set, 8), read<std::int32_t>(set, 4), true};
}
TargetStudio load_target_studio(const void* entity, const StudioOffsets& offsets) {
    TargetStudio result;
    const auto set = load_target_hitbox_set(entity, offsets);
    result.ready = set.studio;
    result.count = std::max(0, std::min(19, set.count));
    for (std::int32_t i = 0; i < result.count; ++i) {
        const auto* box = set.descriptors + i * 0x44;
        result.bones[std::size_t(i)] = read<std::uint32_t>(box, 0);
        result.hitboxes[std::size_t(i)] = {read<std::int32_t>(box, 0), read<std::int32_t>(box, 4),
                                           read<Vector>(box, 8), read<Vector>(box, 0x14),
                                           read<float>(box, 0x30)};
    }
    return result;
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: target_selection.cpp
// ============================================================================

namespace skeet::legit {
namespace {
AimTarget target_angles(std::int32_t index, void* entity, RuntimeState& state,
                        const TargetSelectionInput& input,
                        const TargetSelectionBindings& bindings) {
    const auto& track = state.targets[std::uint32_t(index) & 63u];
    Vector point{track.point[0] + state.target_offset[0], state.target_offset[1] + track.point[1],
                 state.target_offset[2] + track.point[2]};
    predict_target_point(state.profile, bindings.tracking, entity, point);
    auto x = point[0] - input.eye_position[0], y = point[1] - input.eye_position[1],
         z = point[2] - input.eye_position[2];
    const auto distance = std::sqrt((x * x + y * y) + z * z);
    const auto inverse = 1.0f / (distance + 1.1920928955078125e-7f);
    x = inverse * x;
    y = inverse * y;
    z = inverse * z;
    Vector angles{};
    if (y == 0.0f && x == 0.0f)
        angles[0] = z <= 0.0f ? 90.0f : 270.0f;
    else {
        angles[1] = float(bindings.arc_tangent2(double(y), double(x))) * 57.2957763671875f;
        const auto horizontal = std::sqrt(x * x + y * y);
        angles[0] =
            float(bindings.arc_tangent2(double(-z), double(horizontal))) * 57.2957763671875f;
    }
    const auto pitch = normalize_movement_angle(angles[0] - input.reference_angles[0]);
    const auto yaw = normalize_movement_angle(angles[1] - input.reference_angles[1]);
    return {index, angles, std::sqrt(pitch * pitch + yaw * yaw), distance};
}
bool target_ready(const ProfileState& profile, const TargetTrack& track) {
    return !(track.elapsed < profile.reaction_time) &&
           bool(profile.aim_hitgroups & track.visible_hitgroups);
}
}
AimTarget select_aim_target(RuntimeState& state, const TargetSelectionInput& input,
                            const TargetSelectionBindings& bindings) {
    if (!bindings.arc_tangent2)
        throw std::invalid_argument("LEGIT selection requires native atan2 binding");
    AimTarget best;
    best.field_of_view = state.profile.maximum_fov;
    const auto release = [&] {
        release_target(state,
                       bindings.tracking.live_interval ? *bindings.tracking.live_interval
                                                       : bindings.tracking.interval,
                       bindings.random);
    };
    for (std::int32_t index = 1; index <= input.maximum_clients; ++index) {
        auto* entity = engine::invoke<void*>(bindings.entities.entity_list, 3, index);
        if (!target_ready(state.profile, state.targets[std::uint32_t(index) & 63u]))
            continue;
        auto candidate = target_angles(index, entity, state, input, bindings);
        if (candidate.distance == 0.0f)
            continue;
        const auto pitch =
            normalize_movement_angle(candidate.angles[0] - input.reference_angles[0]);
        const auto yaw = normalize_movement_angle(candidate.angles[1] - input.reference_angles[1]);
        const auto maximum = (1000.0f / candidate.distance) * state.profile.maximum_fov;
        if (maximum < std::fabs(pitch) || maximum < std::fabs(yaw) ||
            best.field_of_view <= candidate.field_of_view)
            continue;
        best = candidate;
    }
    const auto previous = state.target;
    if (previous) {
        auto* entity = engine::invoke<void*>(bindings.entities.entity_list, 3, previous);
        const auto& retained_track = state.targets[std::uint32_t(previous) & 63u];
        if (!rage::target_is_eligible(bindings.entities, entity) ||
            retained_track.elapsed < state.profile.reaction_time ||
            !(retained_track.visible_hitgroups & state.profile.aim_hitgroups)) {
            state.block_until = (input.live_time ? *input.live_time : input.time) + 0.4f;
            release();
            return {};
        }
        if (best.index != previous) {
            auto retained = target_angles(previous, entity, state, input, bindings);
            if (state.profile.maximum_fov <= retained.field_of_view ||
                2.0f < retained.field_of_view - best.field_of_view) {
                release();
                return {};
            }
            best = retained;
        }
    }
    if (best.index <= 0) {
        release();
        return {};
    }
    if (previous != best.index) {
        if (!bindings.random.seed || !bindings.random.uniform)
            throw std::invalid_argument(
                "LEGIT target change requires native RandomSeed/RandomFloat");
        state.target = best.index;
        if (best.index != state.previous_target) {
            state.target_offset[0] = bindings.random.uniform(bindings.random.context, -2.0f, 2.0f);
            state.target_offset[1] = bindings.random.uniform(bindings.random.context, -2.0f, 2.0f);
            state.target_offset[2] = bindings.random.uniform(bindings.random.context, -1.0f, 1.0f);
            state.previous_target = state.target;
        }
        const auto tick_count = _mm_cvttss_si32(
            _mm_set_ss(1.0f / (bindings.tracking.live_interval ? *bindings.tracking.live_interval
                                                               : bindings.tracking.interval)));
        if (!tick_count)
            throw std::domain_error("Native target selection interval has no tick divisor");
        bindings.random.seed(bindings.random.context, state.last_command / tick_count);
        state.curve = bindings.random.uniform(bindings.random.context, -1.5f, 1.5f);
    }
    return best;
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: tracking.cpp
// ============================================================================

namespace skeet::legit {
namespace {
std::int32_t index(void* entity) {
    const auto value = engine::invoke<std::int32_t>(static_cast<std::byte*>(entity) + 8, 10);
    if (value == 0)
        throw std::invalid_argument("LEGIT target requires a nonzero native networkable index");
    return value;
}
/* [dedup subtract_i32] */
/* [removed duplicate ticks] */
}
Vector estimate_target_velocity(const ProfileState& state, const TargetTrackingBindings& bindings,
                                void* entity) {
    const auto id = std::uint32_t(index(entity));
    const auto& ring =
        bindings.players ? bindings.players->get(id)->history : bindings.histories[id & 63u];
    if (ring.count && state.reaction_time != 0.0f) {
        const auto interval = bindings.live_interval ? *bindings.live_interval : bindings.interval;
        const auto clock =
            bindings.live_clock_tick ? *bindings.live_clock_tick : bindings.clock_tick;
        const auto latest = subtract(clock, ticks(state.reaction_time, interval));
        const auto width = maximum_number(0.02f, state.reaction_time * 0.5f);
        const auto earliest = subtract(latest, ticks(width, interval));
        Vector sum{};
        std::int32_t count = 0;
        for (std::uint32_t ordinal = 0; ordinal != ring.count; ++ordinal) {
            const auto& record = ring.at(ordinal).header;
            if (record.command_tick > latest)
                continue;
            if (record.command_tick < earliest)
                break;
            for (unsigned axis = 0; axis != 3; ++axis)
                sum[axis] += record.velocity[axis];
            ++count;
        }
        if (count) {
            const auto inverse = 1.0f / float(count);
            for (auto& component : sum)
                component *= inverse;
            return sum;
        }
    }
    Vector result;
    std::memcpy(result.data(), static_cast<std::byte*>(entity) + bindings.velocity_offset,
                sizeof(result));
    return result;
}
void predict_target_point(const ProfileState& state, const TargetTrackingBindings& bindings,
                          void* entity, Vector& point) {
    auto velocity = estimate_target_velocity(state, bindings, entity);
    velocity[2] *= 0.0001f;
    auto& track = bindings.tracks[std::uint32_t(index(entity)) & 63u];
    if (track.velocity_valid) {
        const Vector delta{velocity[0] - track.velocity[0], velocity[1] - track.velocity[1],
                           velocity[2] - track.velocity[2]};
        const auto length =
            std::sqrt((delta[0] * delta[0] + delta[1] * delta[1]) + delta[2] * delta[2]);
        const auto inverse = 1.0f / (length + 1.1920928955078125e-7f);
        const auto step =
            (bindings.live_interval ? *bindings.live_interval : bindings.interval) * 1500.0f;
        if (!(length > step)) {
            if (-step > length)
                for (unsigned axis = 0; axis != 3; ++axis)
                    velocity[axis] = track.velocity[axis] - (inverse * delta[axis]) * step;
        } else
            for (unsigned axis = 0; axis != 3; ++axis)
                velocity[axis] = (inverse * delta[axis]) * step + track.velocity[axis];
    }
    if (!track.velocity_valid)
        track.velocity_valid = 1;
    track.velocity = velocity;
    const auto speed = std::sqrt((velocity[0] * velocity[0] + velocity[1] * velocity[1]) +
                                 velocity[2] * velocity[2]);
    if (!(0.1f > speed) && !(speed > 320.0f))
        for (unsigned axis = 0; axis != 3; ++axis)
            point[axis] = track.interval * velocity[axis] + point[axis];
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: trigger.cpp
// ============================================================================

namespace skeet::legit {
bool trigger_hit_allowed_live(const ProfileState& state, const TriggerHit& hit,
                              const TargetTrack* targets) {
    const auto damage = _mm_cvttss_si32(_mm_set_ss(hit.damage));
    if (damage < state.minimum_damage || !(state.trigger_hitgroups & (1u << (hit.hitgroup & 31u))))
        return false;
    if (state.trigger_reaction != 0.0f && hit.entity) {
        const auto index =
            engine::invoke<std::int32_t>(static_cast<std::byte*>(hit.entity) + 8, 10);
        if (state.trigger_reaction > targets[std::uint32_t(index) & 63u].elapsed)
            return false;
    }
    return true;
}
bool trigger_hit_allowed(const ProfileState& state, const TriggerHit& hit,
                         std::span<const float, 64> target_times) {
    const auto damage = _mm_cvttss_si32(_mm_set_ss(hit.damage));
    if (damage < state.minimum_damage || !(state.trigger_hitgroups & (1u << (hit.hitgroup & 31u))))
        return false;
    if (state.trigger_reaction != 0.0f && hit.entity) {
        const auto index =
            engine::invoke<std::int32_t>(static_cast<std::byte*>(hit.entity) + 8, 10);
        if (state.trigger_reaction > target_times[std::uint32_t(index) & 63u])
            return false;
    }
    return true;
}
bool begin_trigger(const Settings& settings, ProfileState& state, const TriggerFrame& frame,
                   InputBitmap input, TriggerCommand& command) {
    if (!(state.flags & 0x20u) || (command.buttons & 1u))
        return false;
    if (!hotkey_active(settings.trigger_keys[std::size_t(state.profile)].value.binding, input))
        return false;
    if (state.burst_until > frame.time)
        command.buttons |= 1u;
    if (!(frame.context_flags & 0x400u) && frame.weapon_id != 64)
        return false;
    if (!(state.flags & 8u) && frame.blind_amount > 200.0f)
        return false;
    return true;
}
void finish_trigger_live(ProfileState& state, const std::uint32_t* context_flags, const float* time,
                         bool hit, std::int32_t selected_tick, TriggerCommand& command,
                         RandomBindings random) {
    if (*context_flags & 0x400u) {
        if (!hit && !(command.buttons & 1u))
            return;
        if (selected_tick != -1)
            command.tick = selected_tick;
    }
    if (!hit)
        return;
    command.buttons |= 1u;
    if (state.flags & 0x40u)
        state.burst_until = state.burst_time + *time;
    else {
        if (!random.uniform)
            throw std::invalid_argument("Native trigger timer requires RandomFloat");
        const auto duration = random.uniform(random.context, 0.03125f, 0.1f);
        state.burst_until = duration + *time;
    }
}
void finish_trigger(ProfileState& state, const TriggerFrame& frame, bool hit,
                    std::int32_t selected_tick, TriggerCommand& command, RandomBindings random) {
    finish_trigger_live(state, &frame.context_flags, &frame.time, hit, selected_tick, command,
                        random);
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: trigger_trace.cpp
// ============================================================================

namespace skeet::legit {
namespace {
constexpr std::array<std::uint8_t, 128> seeds{
    2,   3,   4,   10,  13,  15,  27,  29, 37, 38,  41,  46,  47,  49,  50,  52,  56,  57,  58,
    62,  66,  68,  70,  72,  76,  83,  87, 97, 108, 109, 110, 111, 113, 121, 122, 125, 135, 1,
    5,   6,   8,   17,  25,  26,  28,  31, 32, 36,  39,  44,  51,  61,  67,  71,  74,  77,  78,
    79,  80,  81,  84,  89,  90,  91,  92, 96, 98,  101, 102, 104, 107, 114, 115, 119, 9,   11,
    16,  19,  20,  22,  30,  48,  54,  59, 60, 65,  69,  73,  75,  82,  85,  86,  88,  94,  99,
    103, 105, 112, 116, 117, 118, 124, 0,  7,  12,  14,  18,  21,  23,  24,  33,  34,  35,  40,
    42,  43,  45,  53,  55,  63,  64,  93, 95, 100, 106, 120, 133, 137};
bool accepts(const ProfileState& settings, const BallisticQuery& query,
             const TriggerTraceInput& input, TriggerTraceBindings bindings) {
    if (input.live_targets)
        return trigger_hit_allowed_live(
            settings,
            {query.damage, std::uint32_t(query.trace.hitgroup),
             reinterpret_cast<void*>(std::uintptr_t(query.trace.entity))},
            input.live_targets);
    if (_mm_cvttss_si32(_mm_set_ss(query.damage)) < settings.minimum_damage ||
        !(settings.trigger_hitgroups & (1u << (std::uint32_t(query.trace.hitgroup) & 31u))))
        return false;
    if (settings.trigger_reaction != 0.0f && query.trace.entity) {
        if (!bindings.entity_index)
            throw std::invalid_argument(
                "Trigger reaction time requires the entity networkable index");
        const auto index = bindings.entity_index(bindings.context, query.trace.entity);
        if (settings.trigger_reaction > input.target_times[std::uint32_t(index) & 63u])
            return false;
    }
    return true;
}
}
bool evaluate_trigger_samples(BallisticEngine& engine, const ProfileState& settings,
                              const TriggerTraceInput& input, const BallisticFilter& filter,
                              BallisticQuery& query, const rage::SpreadTables& tables,
                              const rage::SpreadRandom& random, const rage::ShotgunPattern& pattern,
                              const RageTrigonometry& trigonometry, TriggerTraceBindings bindings) {
    const auto shifted = std::uint32_t(settings.minimum_hit_chance) << 7;
    auto required = std::bit_cast<std::int32_t>(shifted) / 100;
    if (required < 1)
        required = 1;
    auto parameters = input.spread;
    if (parameters.weapon_id == 64) {
        parameters.spread = 0.00052f;
        parameters.inaccuracy = (input.weapon_flags_7c & 6u) ? 0.003f : 0.0077f;
        parameters.fire_mode = 0;
    }
    parameters.pellet_count = 1;
    std::int32_t hits = 0;
    for (std::int32_t i = 0; i != 128; ++i) {
        const auto sample = rage::generate_spread_sample(parameters, seeds[std::size_t(i)], tables,
                                                         random, pattern, trigonometry);
        Vector direction;
        for (std::size_t axis = 0; axis != 3; ++axis) {
            const auto first = sample.first * input.basis.first[axis] + input.basis.center[axis];
            direction[axis] = first + sample.second * input.basis.second[axis];
        }
        const auto length = std::sqrt((direction[0] * direction[0] + direction[1] * direction[1]) +
                                      direction[2] * direction[2]);
        const auto inverse = 1.0f / (length + 1.1920928955078125e-7f);
        for (auto& component : direction)
            component *= inverse;
        if (rage::trace_target_direction(engine, input.source, input.origin, direction, filter,
                                         query) &&
            accepts(settings, query, input, bindings)) {
            ++hits;
            if (hits >= required)
                return true;
        }
        const auto possible = hits - i + 128;
        if (possible < required)
            return false;
    }
    return hits >= required;
}
bool trace_trigger(BallisticEngine& engine, const ProfileState& settings,
                   const TriggerTraceInput& input, const BallisticFilter& filter,
                   BallisticQuery& query, const rage::SpreadTables& tables,
                   const rage::SpreadRandom& random, const rage::ShotgunPattern& pattern,
                   const RageTrigonometry& trigonometry, TriggerTraceBindings bindings) {
    if (!rage::trace_target_direction(engine, input.source, input.origin, input.basis.center,
                                      filter, query))
        return false;
    if (!(settings.flags & 0x10u) && bindings.through_smoke &&
        bindings.through_smoke(bindings.context, input.origin, query.trace.end))
        return false;
    if (settings.minimum_hit_chance == 0)
        return true;
    return evaluate_trigger_samples(engine, settings, input, filter, query, tables, random, pattern,
                                    trigonometry, bindings);
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: acquisition_allocation.cpp
// ============================================================================

namespace skeet::rage {
inline RageVector subtract(const RageVector& a, const RageVector& b) noexcept {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}

inline std::uint32_t convar(const void* node, std::ptrdiff_t offset) noexcept {
    for (;;) {
        const auto parent = read<const void*>(node, 0x1c);
        if (parent == node)
            break;
        node = parent;
    }
    return read<std::uint32_t>(node, offset) ^ std::uint32_t(reinterpret_cast<std::uintptr_t>(node));
}
AcquisitionEvaluations::~AcquisitionEvaluations() {
    release();
}
void AcquisitionEvaluations::allocate(engine::JobMemory memory) { //434805C0..434805E3
    if (data_)
        throw std::logic_error("RAGE evaluation storage is already allocated");
    memory_ = memory;
    constexpr auto size = 80u * sizeof(AcquisitionEvaluation);
    auto* raw = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!raw)
        return;
    const auto base = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (base + 15) & ~std::uintptr_t(15);
    const auto displacement = static_cast<std::uint32_t>(header - base);
    const auto allocation_size = static_cast<std::uint32_t>(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    data_ = reinterpret_cast<AcquisitionEvaluation*>(header + 16);
    for (std::size_t index = 0; index < 80; ++index)
        ::new (static_cast<void*>(data_ + index)) AcquisitionEvaluation;
}
void AcquisitionEvaluations::release() noexcept { //43387719
    if (!data_)
        return;
    for (std::size_t index = 0; index < 80; ++index)
        std::destroy_at(data_ + index);
    auto* bytes = reinterpret_cast<std::byte*>(data_);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory_.free(memory_.process_heap, 0, bytes - 16 - displacement);
    data_ = nullptr;
}
}

// ============================================================================
// Subcomponent: acquisition_basics.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* [removed duplicate integer] */
/* [stripped helper] */
}
std::uint8_t AcquisitionTargetSettings::option(std::size_t native_offset) const noexcept {
    return std::uint8_t(words[(native_offset - 0xBCU) >> 2] >> ((native_offset & 3U) * 8U));
}
void AcquisitionTargetSettings::option(std::size_t native_offset, std::uint8_t value) noexcept {
    auto& word = words[(native_offset - 0xBCU) >> 2];
    const auto shift = (native_offset & 3U) * 8U;
    word = (word & ~(0xFFU << shift)) | (std::uint32_t(value) << shift);
}
ScanPolicy AcquisitionTargetSettings::scan_policy() const noexcept {
    return {{words[1], words[2]},
            std::bit_cast<float>(words[3]),
            words[4],
            integer(words[5]),
            std::bit_cast<float>(words[6]),
            0,
            option(0xDC) != 0,
            option(0xDD) != 0,
            option(0xE1) != 0};
}
void AcquisitionTargetSettings::scan_policy(const ScanPolicy& policy) noexcept {
    words[1] = policy.hitboxes.regular;
    words[2] = policy.hitboxes.safe;
    words[3] = std::bit_cast<std::uint32_t>(policy.point_scale);
    words[4] = policy.multipoint_mask;
    words[5] = std::uint32_t(policy.minimum_damage);
    words[6] = std::bit_cast<std::uint32_t>(policy.minimum_chance);
    option(0xDC, std::uint8_t(policy.prefer_safe));
    option(0xDD, std::uint8_t(policy.prefer_body));
    option(0xE1, std::uint8_t(policy.additional_hypotheses));
}
bool pending_target_damage(const ShotFeedbackState* state, PendingTargetDamage& output) noexcept {
    if (!state || state->pending.empty())
        return false;
    output.entity_index = integer(state->pending.front().state.words[1]);
    output.damage = integer(state->pending.front().state.words[10]);
    return true;
}
std::int32_t scan_target_health(std::int32_t health, std::uint32_t entity_index,
                                const PendingTargetDamage* pending) noexcept {
    health = health < 1 ? 1 : health > 100 ? 100 : health;
    if (pending && std::uint32_t(pending->entity_index) == entity_index) {
        health = integer(std::uint32_t(health) - std::uint32_t(pending->damage));
        if (health <= 0)
            health = 1000;
    }
    return health;
}
void reset_scan_point(TargetPoint& point) noexcept {
    point.position = {0, 0, 0};
    point.reference_position = {0, 0, 0};
    point.damage = 0;
    point.hitbox = -1;
    point.hitgroup = -1;
    point.flags = 0;
    point.chance = 0;
    point.descriptor_weight = 0;
    point.priority = 0;
}
bool target_history_is_usable(const client::NativePlayerStateLayout& state,
                              const lagcomp::RecordRing& history, const client::CommandFrame& frame,
                              std::uint32_t entity_index, float interval) noexcept {
    const auto index = entity_index & 63U;
    const auto mask = 1U << (index & 31U);
    if (!(read<std::uint32_t>(&frame, 0x1EC + (index >> 5) * 4U) & mask))
        return true;
    const auto tick = lagcomp::time_to_tick(read<float>(&state, 0x110), interval);
    const auto receive = read<std::uint32_t>(&state, 0x168);
    if (tick >= integer(receive - 1U))
        return true;
    if (read<std::int32_t>(&state, 0x1D4) <= 1)
        return false;
    return !history.count || tick >= history.at(0).header.simulation_tick;
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: acquisition_frame.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
lagcomp::HistoryTickTiming refresh_timing(lagcomp::HistoryTickTiming timing,
                                          const client::CommandFrame& frame, float clock) noexcept {
    timing.current_time = clock;
    timing.latency_198 = field<float>(&frame, 0x198);
    timing.tick_d4 = field<std::int32_t>(&frame, 0xD4);
    timing.tick_18c = std::bit_cast<std::int32_t>(frame.field_18c);
    return timing;
}
lagcomp::HistoryTickTiming preparation_timing(const lagcomp::EntityPreparationContext& seed,
                                              const RecordCandidateBindings& candidates,
                                              const client::CommandFrame& frame,
                                              float clock) noexcept {
    const lagcomp::HistoryTickTiming frame_timing{
        clock, 0, candidates.interpolation, candidates.interval, candidates.maximum_unlag_ms, 0, 0};
    return refresh_timing(seed.current_timing ? *seed.current_timing : frame_timing, frame, clock);
}
lagcomp::EntityPreparationContext refresh_preparation(const lagcomp::EntityPreparationContext& seed,
                                                      const lagcomp::HistoryTickTiming& timing,
                                                      float clock) noexcept {
    auto result = seed;
    result.global_frame = std::bit_cast<std::uint32_t>(clock);
    // A null original timing state selects native mean-latency clock logic.
    result.current_timing = seed.current_timing ? &timing : nullptr;
    return result;
}
RecordCandidateBindings refresh_candidates(RecordCandidateBindings seed, float clock) noexcept {
    seed.current_time = clock;
    return seed;
}
TargetBindings refresh_targeting(const TargetBindings& seed,
                                 const players::State& adjustments) noexcept {
    return {seed.local_player,          seed.entity_list, seed.game_type,
            seed.teammates_are_enemies, adjustments,      seed.offsets};
}
BodyAimProbeContext refresh_body(const BodyAimProbeContext& seed, PreambleState& state,
                                 const client::CommandFrame& frame,
                                 const GlobalSettings& settings) noexcept {
    //43353190 reads the current4346A3E0 descriptor before the probe query.
    return {seed.collision,     seed.ballistics,
            seed.source,        reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address)),
            seed.offsets,       state.flags,
            state.weapon_flags, settings.automatic_penetration.value,
            seed.trace_flag,    seed.trace_filter_vtable,
            seed.trace_entities};
}
ScanContext refresh_scanner(const ScanContext& seed, PreambleState& state,
                            const client::CommandFrame& frame,
                            const TargetPreparationBindings& preparation,
                            const lagcomp::EntityPreparationContext& entity,
                            const std::array<RageSpreadSample, 32>& samples) {
    return {seed.ballistics,
            preparation.targeting,
            seed.source,
            reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address)),
            seed.weapon_item_offset,
            preparation.global.automatic_penetration.value,
            frame.weapon_id_d8,
            frame.origin_dc,
            std::bit_cast<float>(frame.field_160),
            field<std::int32_t>(&frame, 0x19C),
            field<std::int32_t>(&frame, 0x1D0),
            preparation.weapon_settings.minimum_hit_chance.value,
            state.flags,
            state.previous_targets,
            samples,
            seed.model_offsets,
            seed.trigonometry,
            seed.box_clipper,
            seed.players,
            entity,
            seed.hypothesis_offsets,
            seed.update_attachments,
            seed.teammate_convars,
            seed.trace_filter_vtable,
            seed.trace_entities};
}
ScanContext refresh_projected_scanner(const ScanContext& seed, const client::CommandFrame& frame,
                                      const lagcomp::EntityPreparationContext& preparation) {
    return {seed.ballistics,
            seed.targeting,
            seed.source,
            reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address)),
            seed.weapon_item_offset,
            seed.allow_penetration,
            frame.weapon_id_d8,
            frame.origin_dc,
            std::bit_cast<float>(frame.field_160),
            frame.pellet_count_19c,
            std::bit_cast<std::int32_t>(frame.field_1d0),
            seed.accuracy_percent,
            seed.configuration_flags,
            seed.target_mask,
            seed.samples,
            seed.model_offsets,
            seed.trigonometry,
            seed.box_clipper,
            seed.players,
            preparation,
            seed.hypothesis_offsets,
            seed.update_attachments,
            seed.teammate_convars,
            seed.trace_filter_vtable,
            seed.trace_entities};
}
BodyAimProbeContext refresh_projected_body(const BodyAimProbeContext& seed,
                                           const ScanContext& scanner,
                                           const client::CommandFrame& frame) noexcept {
    return {seed.collision,     seed.ballistics,
            seed.source,        reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address)),
            seed.offsets,       scanner.configuration_flags,
            seed.weapon_flags,  scanner.allow_penetration,
            seed.trace_flag,    seed.trace_filter_vtable,
            seed.trace_entities};
}
}
AcquisitionBranchFrameBindings::AcquisitionBranchFrameBindings(
    PreambleState& state, const client::CommandFrame& frame, float clock, std::int32_t handle,
    std::uint32_t scheduler_flags, const std::array<RageSpreadSample, 32>& samples,
    const TargetPreparationBindings& preparation_seed, const BodyAimProbeContext& body_seed,
    const TargetPredictionBindings& prediction_seed, const RecordCandidateBindings& candidates_seed,
    const AcquisitionRecordBindings& records_seed, const players::State& adjustments,
    const ui::SettingsMenuValues& settings_menu)
    : targeting(refresh_targeting(preparation_seed.targeting, adjustments)),
      prediction_timing(refresh_timing(prediction_seed.record_selection.timing, frame, clock)),
      scanner_timing(
          preparation_timing(records_seed.scanner.preparation, candidates_seed, frame, clock)),
      extrapolation_timing(preparation_timing(records_seed.extrapolation.preparation,
                                              candidates_seed, frame, clock)),
      history_timing(preparation_timing(records_seed.history.extrapolation.preparation,
                                        candidates_seed, frame, clock)),
      prediction_preparation(refresh_preparation(prediction_seed.record_selection.preparation,
                                                 prediction_timing, clock)),
      scanner_preparation(
          refresh_preparation(records_seed.scanner.preparation, scanner_timing, clock)),
      extrapolation_preparation(
          refresh_preparation(records_seed.extrapolation.preparation, extrapolation_timing, clock)),
      history_preparation(refresh_preparation(records_seed.history.extrapolation.preparation,
                                              history_timing, clock)),
      prediction_records{prediction_seed.record_selection.players,
                         prediction_preparation,
                         prediction_timing,
                         prediction_seed.record_selection.rotation,
                         prediction_seed.record_selection.interpolation_scratch,
                         prediction_seed.record_selection.attachments,
                         prediction_seed.record_selection.current_bone_counter},
      extrapolation{
          records_seed.extrapolation.players,     extrapolation_preparation,
          records_seed.extrapolation.attachments, records_seed.extrapolation.bones,
          records_seed.extrapolation.movement,    records_seed.extrapolation.prediction_limit},
      history_extrapolation{records_seed.history.extrapolation.players,
                            history_preparation,
                            records_seed.history.extrapolation.attachments,
                            records_seed.history.extrapolation.bones,
                            records_seed.history.extrapolation.movement,
                            records_seed.history.extrapolation.prediction_limit},
      history{history_extrapolation,
              records_seed.history.alternative,
              records_seed.history.rotation,
              records_seed.history.matrix_scratch,
              clock,
              records_seed.history.interpolation,
              records_seed.history.maximum_unlag_ms},
      preparation{targeting,
                  preparation_seed.global,
                  preparation_seed.weapon_settings,
                  preparation_seed.input,
                  state,
                  handle,
                  preparation_seed.accuracy_convar,
                  preparation_seed.offsets},
      record_candidates(refresh_candidates(candidates_seed, clock)),
      scanner(refresh_scanner(records_seed.scanner, state, frame, preparation, scanner_preparation,
                              samples)),
      records{records_seed.players,
              records_seed.lag_compensation,
              records_seed.interval,
              handle,
              preparation.global.body_aim_disablers.value,
              records_seed.feedback_owner,
              history,
              extrapolation,
              scanner,
              settings_menu,
              records_seed.bone_runtime,
              records_seed.live_movement,
              records_seed.live_budget,
              records_seed.clear_safe_neck},
      body_probe(refresh_body(body_seed, state, frame, preparation.global)),
      prediction{body_probe,
                 prediction_records,
                 prediction_seed.engine_trace,
                 prediction_seed.local_maximum_speed,
                 scheduler_flags,
                 prediction_seed.command} {}

AcquisitionFrameBindings::AcquisitionFrameBindings(
    PreambleState& state, const PreambleResult& preamble, float clock, std::int32_t handle,
    std::uint32_t scheduler_flags, const SingleTargetBindings& single_seed,
    const MultipleTargetBindings& multiple_seed, const ui::SettingsMenuValues& settings_menu)
    : samples(hitchance_spread_samples(state.spread)),
      single_frame(state, *preamble.frame, clock, handle, scheduler_flags, samples,
                   single_seed.preparation, single_seed.prediction.trace, single_seed.prediction,
                   single_seed.record_candidates, single_seed.records, single_seed.adjustments,
                   settings_menu),
      multiple_frame(state, *preamble.frame, clock, handle, scheduler_flags, samples,
                     multiple_seed.preparation, multiple_seed.body_probe, multiple_seed.prediction,
                     multiple_seed.record_candidates, multiple_seed.records,
                     multiple_seed.adjustments, settings_menu),
      single{single_frame.preparation, single_frame.prediction, single_frame.record_candidates,
             single_frame.records,     single_seed.adjustments, single_seed.maximum_clients,
             single_seed.jobs},
      multiple{multiple_frame.preparation,    multiple_frame.body_probe,
               multiple_frame.prediction,     multiple_frame.record_candidates,
               multiple_frame.records,        multiple_seed.adjustments,
               multiple_seed.maximum_clients, multiple_seed.jobs} {}
TargetFrameBindings::TargetFrameBindings(const TargetPredictionBindings& prediction_seed,
                                         ScanContext& scanner_seed,
                                         const client::CommandFrame& frame, float clock,
                                         std::uint32_t scheduler_flags)
    : prediction_timing(refresh_timing(prediction_seed.record_selection.timing, frame, clock)),
      scanner_timing(refresh_timing(scanner_seed.preparation.current_timing
                                        ? *scanner_seed.preparation.current_timing
                                        : prediction_seed.record_selection.timing,
                                    frame, clock)),
      prediction_preparation(refresh_preparation(prediction_seed.record_selection.preparation,
                                                 prediction_timing, clock)),
      scanner_preparation(refresh_preparation(scanner_seed.preparation, scanner_timing, clock)),
      prediction_records{prediction_seed.record_selection.players,
                         prediction_preparation,
                         prediction_timing,
                         prediction_seed.record_selection.rotation,
                         prediction_seed.record_selection.interpolation_scratch,
                         prediction_seed.record_selection.attachments,
                         prediction_seed.record_selection.current_bone_counter},
      body_probe(refresh_projected_body(prediction_seed.trace, scanner_seed, frame)),
      prediction{body_probe,
                 prediction_records,
                 prediction_seed.engine_trace,
                 prediction_seed.local_maximum_speed,
                 scheduler_flags,
                 prediction_seed.command},
      scanner(refresh_projected_scanner(scanner_seed, frame, scanner_preparation)) {}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: acquisition_jobs.cpp
// ============================================================================

namespace skeet::rage {
namespace {
struct Work {
    PreambleState* state;
    ScanContext* context;
    ScanRequest* request;
};
static_assert(sizeof(Work) <= sizeof(AcquisitionWorkItem));
#if defined(_M_IX86) || defined(__i386__)
#define ACQUISITION_FASTCALL __fastcall
#else
#define ACQUISITION_FASTCALL
#endif
void ACQUISITION_FASTCALL scan_work(void* item) {
    Work work;
    std::memcpy(&work, item, sizeof(work));
    const auto index = work.request->evaluation.candidate.entity_index & 63U;
    if (!(work.state->previous_targets[index >> 5] & (1U << (index & 31U))))
        scan_candidate(*work.context, *work.request);
}
#undef ACQUISITION_FASTCALL
}
void submit_acquisition_scan(AcquisitionStorage& storage, PreambleState& state,
                             ScanContext& context, void* player, std::uint32_t ordinal,
                             std::uint32_t flags, const ui::SettingsMenuValues& settings_menu) {
    auto& slot = storage.evaluations[ordinal];
    auto& candidate = storage.candidates[ordinal];
    auto& request = storage.requests[ordinal];
    request.emplace(ScanRequest{player,
                                std::uint32_t(reinterpret_cast<std::uintptr_t>(player)),
                                {candidate, slot.record, slot.source_record,
                                 slot.settings.scan_policy(), slot.ballistic_evaluations},
                                flags,
                                state.completed,
                                ordinal});
    request->evaluation.policy.body_disabler_reasons = slot.body_disabler_reasons;
    if (settings_menu.multithreading && storage.queued.inline_storage &&
        storage.queued.count < 80) {
        const Work work{&state, &context, &*request};
        std::memcpy(storage.queued.items[storage.queued.count++].bytes.data(), &work, sizeof(work));
    } else {
        scan_candidate(context, *request);
    }
}
void dispatch_acquisition_scans(AcquisitionStorage& storage, engine::JobDispatcher& jobs,
                                const ui::SettingsMenuValues& settings_menu) {
    if (!settings_menu.multithreading)
        return;
    if (!storage.queued.count)
        return;
    auto* task = jobs.create_thirty_six(scan_work);
    jobs.dispatch_thirty_six(*task, storage.queued.items.data(), storage.queued.count);
    engine::release_job(*task);
    storage.queued.clear();
}
void finish_target_acquisition(PreambleState& state, AcquisitionStorage& storage,
                               engine::JobDispatcher& jobs, client::PlayerStateCache& players,
                               std::int32_t maximum_clients,
                               const ui::SettingsMenuValues& settings_menu) {
    dispatch_acquisition_scans(storage, jobs, settings_menu);
    if (state.flags & 9U)
        return;
    for (std::int32_t index = 1; index <= maximum_clients; ++index) {
        auto* player = players.get(std::uint32_t(index));
        const std::uint32_t zero = 0;
        std::memcpy(reinterpret_cast<std::byte*>(&player->native) + 0x194, &zero, sizeof(zero));
    }
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: acquisition_records.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
bool convar_nonzero(const void* object) noexcept {
    for (;;) {
        const auto* parent = field<const void*>(object, 0x1C);
        if (parent == object)
            break;
        object = parent;
    }
    return field<std::uint32_t>(object, 0x30) !=
           std::uint32_t(reinterpret_cast<std::uintptr_t>(object));
}
}
void prepare_and_submit_target_records(PreambleState& state, PreambleResult& preamble,
                                       AcquisitionStorage& storage, void* player,
                                       client::PlayerState& cached,
                                       std::span<const std::int32_t> ticks,
                                       const AcquisitionTargetSettings& settings,
                                       std::uint32_t initial_flags,
                                       const AcquisitionRecordBindings& bindings) {
    (void)cached; // Native loaders deliberately repeat their own canonical cache lookup.
    auto& frame = *preamble.frame;
    auto* network = static_cast<std::byte*>(player) + 8;
    for (std::uint32_t age = 0; age < ticks.size(); ++age) {
        const auto ordinal = state.evaluation_count;
        auto& candidate = storage.candidates[ordinal];
        auto& slot = storage.evaluations[ordinal];
        candidate.flags = 0;
        candidate.entity_index = engine::invoke<std::uint32_t>(network, 10);
        const auto health = engine::invoke<std::int32_t>(player, 122);
        PendingTargetDamage pending;
        const bool has_pending =
            pending_target_damage(feedback_owner_state(bindings.feedback_owner), pending);
        candidate.health =
            scan_target_health(health, candidate.entity_index, has_pending ? &pending : nullptr);
        reset_scan_point(candidate.primary);
        candidate.priority = 0;
        slot.requested_tick = ticks[age];
        slot.body_disabler_reasons = 0;
        slot.ballistic_evaluations = 0;
        slot.extrapolated = false;
        slot.extrapolation_ticks = 0;
        slot.target_handle = *engine::invoke<const std::uint32_t*>(player, 2);
        slot.settings = settings;
        std::uint32_t output_flags = 0;
        bool usable = false;
        if (convar_nonzero(bindings.lag_compensation)) {
            const auto* current = bindings.players.get(candidate.entity_index);
            usable = target_history_is_usable(current->native, current->history, frame,
                                              candidate.entity_index, bindings.interval);
        }
        if (!usable) {
            if (!lagcomp::capture_extrapolated_record(frame, player, slot.record, output_flags,
                                                      bindings.extrapolation, bindings.bone_runtime,
                                                      bindings.live_movement, bindings.live_budget,
                                                      &slot.extrapolation_ticks))
                continue;
            slot.requested_tick = slot.record.header.simulation_tick;
            candidate.flags |= 4U;
            slot.source_record = reinterpret_cast<const lagcomp::LagRecord*>(
                std::uintptr_t(slot.record.header.field_00));
        } else {
            std::uint32_t options = (state.flags & 1U) ? 1U : 5U;
            if (state.flags & 8U)
                options |= 8U;
            const auto handle = bindings.remembered_target_handle == -1
                                    ? 0x1FFFU
                                    : std::uint32_t(bindings.remembered_target_handle) & 0xFFFFU;
            if (!(frame.flags & 0x4000U) && engine::invoke<std::uint32_t>(network, 10) != handle)
                options &= ~1U;
            if (!lagcomp::select_target_history_record(
                    frame, player, slot.requested_tick, options, slot.record, output_flags,
                    bindings.history, bindings.bone_runtime, bindings.live_movement,
                    bindings.live_budget, &slot.source_record, &slot.extrapolation_ticks)) {
                slot.requested_tick = -1;
                continue;
            }
        }
        slot.extrapolated = slot.extrapolation_ticks > 0;
        auto scan = slot.settings.scan_policy();
        TargetRecordPolicy policy{{scan.hitboxes, slot.record.header.flags, 0, scan.prefer_body},
                                  initial_flags,
                                  scan.additional_hypotheses};
        if (!prepare_target_record_policy(policy, output_flags, age,
                                          std::uint8_t(bindings.body_disablers)))
            break;
        scan.hitboxes = policy.body.hitboxes;
        if (bindings.clear_safe_neck)
            scan.hitboxes.safe &= ~0x200U;
        scan.prefer_body = policy.body.context_ready;
        scan.additional_hypotheses = policy.delay_shot;
        slot.body_disabler_reasons = policy.body.disabler_reasons;
        slot.settings.words[1] = scan.hitboxes.regular;
        slot.settings.words[2] = scan.hitboxes.safe;
        if (output_flags & 0x40U)
            slot.settings.option(0xDD, 1);
        if (!scan.additional_hypotheses)
            slot.settings.option(0xE1, 0);
        candidate.prefer_body = scan.prefer_body;
        candidate.prefer_safe = scan.prefer_safe;
        candidate.weapon_priority_enabled = (slot.record.header.player_flags & 1U) != 0;
        candidate.target_state_flags = slot.record.header.flags;
        submit_acquisition_scan(storage, state, bindings.scanner, player, ordinal,
                                policy.evaluation_flags, bindings.settings_menu);
        ++state.evaluation_count;
    }
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: acquisition_state.cpp
// ============================================================================

namespace skeet::rage {
const lagcomp::EvaluationState& synchronize_evaluation_state(const TargetCandidate& candidate,
                                                             AcquisitionEvaluation& slot) noexcept {
    auto& words = slot.native.words;
    words[0] = candidate.flags;
    words[1] = candidate.entity_index;
    words[2] = std::uint32_t(candidate.health);
    std::memcpy(words.data() + 3, &candidate.primary, sizeof(TargetPoint));
    std::memcpy(words.data() + 17, &candidate.alternate_first, sizeof(TargetPoint));
    std::memcpy(words.data() + 31, &candidate.alternate_second, sizeof(TargetPoint));
    words[45] = std::uint32_t(slot.requested_tick);
    words[46] = slot.body_disabler_reasons;
    std::memcpy(words.data() + 47, slot.settings.words.data(), sizeof(slot.settings.words));
    // DC/DD retain their native stored bytes; the bool views do not turn
    // noncanonical true values into1 during publication.
    words[57] = candidate.priority;
    words[58] = slot.ballistic_evaluations;
    return slot.native;
}
void restore_evaluation_state(const lagcomp::EvaluationState& input, TargetCandidate& candidate,
                              AcquisitionEvaluation& slot) noexcept {
    if (&input != &slot.native)
        std::memcpy(&slot.native, &input, sizeof(input));
    const auto& words = slot.native.words;
    candidate.flags = words[0];
    candidate.entity_index = words[1];
    candidate.health = std::bit_cast<std::int32_t>(words[2]);
    std::memcpy(&candidate.primary, words.data() + 3, sizeof(TargetPoint));
    std::memcpy(&candidate.alternate_first, words.data() + 17, sizeof(TargetPoint));
    std::memcpy(&candidate.alternate_second, words.data() + 31, sizeof(TargetPoint));
    slot.requested_tick = std::bit_cast<std::int32_t>(words[45]);
    slot.body_disabler_reasons = words[46];
    std::memcpy(slot.settings.words.data(), words.data() + 47, sizeof(slot.settings.words));
    candidate.priority = words[57];
    slot.ballistic_evaluations = words[58];
    candidate.prefer_safe = slot.settings.option(0xDC) != 0;
    candidate.prefer_body = slot.settings.option(0xDD) != 0;
    candidate.weapon_priority_enabled = (slot.record.header.player_flags & 1U) != 0;
    candidate.target_state_flags = slot.record.header.flags;
    slot.source_record =
        reinterpret_cast<const lagcomp::LagRecord*>(std::uintptr_t(slot.record.header.field_00));
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: action_points.cpp
// ============================================================================

namespace skeet::rage::action_detail {
std::vector<Point> points(const client::CommandFrame& frame,
                          const AcquisitionTargetSettings& policy, const lagcomp::LagRecord& record,
                          const scan_detail::Model& model, const scan_detail::Geometry& geometry,
                          std::uint32_t index, bool safe, bool regular,
                          const ActionStateBindings& bindings) {
    const RagePointGeneration generation{frame.origin_dc,
                                         0xFFFFFFFFU,
                                         policy.words[4],
                                         std::bit_cast<float>(policy.words[3]),
                                         std::bit_cast<float>(frame.field_160),
                                         bindings.preamble.weapon_settings.minimum_hit_chance.value,
                                         (bindings.state.flags & 4U) != 0};
    std::vector<Point> result;
    if (safe) {
        const auto center = rage_point_detail::midpoint(geometry.primary[index].capsule);
        std::array<Matrix3x4, 3> matrices;
        for (std::size_t i = 0; i != 3; ++i)
            matrices[i] = record.alternatives[i].matrices[index];
        std::vector<RageAimPoint> descriptors;
        if (index == 0) {
            const RageHeadOcclusion occlusion{model.hitboxes[6],
                                              {record.alternatives[0].matrices[6],
                                               record.alternatives[1].matrices[6],
                                               record.alternatives[2].matrices[6]}};
            descriptors = generate_rage_head_safe_points(generation, center, record.header.origin,
                                                         model.hitboxes[0], matrices, &occlusion,
                                                         bindings.scan.trigonometry);
        } else {
            descriptors = generate_rage_body_safe_points(generation, center, record.header.origin,
                                                         index, model.hitboxes[index], matrices);
        }
        for (const auto& point : descriptors)
            result.push_back({point.position, point.flags, 2});
    }
    if (regular) {
        const auto descriptors = generate_rage_regular_hitbox_points(
            generation, index, model.hitboxes, geometry.primary);
        for (const auto& point : descriptors)
            result.push_back({point.position, point.flags, 1});
    }
    return result;
}
} // namespace skeet::rage::action_detail

// ============================================================================
// Subcomponent: action_settings.cpp
// ============================================================================

namespace skeet::rage::action_detail {
namespace {
/* [stripped helper] */
std::int32_t minimum_damage(const client::CommandFrame& frame, void* target, std::int32_t raw,
                            std::uint32_t flags) {
    if (raw >= 100)
        return add(engine::invoke<std::int32_t>(target, 122), add(raw, -100));
    if (raw != 0) {
        const auto limit = add(engine::invoke<std::int32_t>(target, 122), 5);
        return limit < raw ? limit : raw;
    }
    auto* weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
    const auto factor = engine::invoke<float>(weapon, 433, std::int32_t(0));
    const auto health = add(engine::invoke<std::int32_t>(target, 122), 1);
    float value = float(health > 10 ? health : 10) * factor;
    if (!(flags & 4U))
        value *= 1.25f;
    value = minimum_number(value, float(engine::invoke<std::int32_t>(weapon, 431)) - 1.0f);
    value = maximum_number(value, 1.0f);
    auto result = _mm_cvttss_si32(_mm_set_ss(value + 0.5f));
    const auto shots =
        rage_detail::shots_to_kill(engine::invoke<std::int32_t>(target, 122), result);
    if (shots > 1) {
        const auto quotient = engine::invoke<std::int32_t>(target, 122) / shots;
        const auto quartered = std::bit_cast<std::int32_t>(std::uint32_t(quotient) << 2);
        const auto reduced = add(quartered / 3, 1);
        if (reduced < result)
            result = reduced;
    }
    return result;
}
}
void prepare_settings(const client::CommandFrame& frame, void* target, std::uint32_t entity,
                      AcquisitionTargetSettings& settings, const ActionStateBindings& bindings) {
    const auto& b = bindings.target_preparation;
    const auto original_mode = std::bit_cast<std::int32_t>(settings.words[0]);
    if (settings.words[0]) {
        auto& player = *bindings.scan.players.get(entity);
        if (!player.history.count)
            settings.words[0] = 0;
        else {
            const auto handle = bindings.preamble.yaw_target.handle;
            const auto remembered =
                handle == -1 ? nullptr : engine::invoke<void*>(b.targeting.entity_list, 4, handle);
            if (remembered != target ||
                (bindings.state.previous_targets[(entity & 63U) >> 5] & (1U << (entity & 31U))))
                settings.words[0] = 1;
        }
    }
    settings.words[5] = std::uint32_t(minimum_damage(
        frame, target, std::bit_cast<std::int32_t>(settings.words[5]), bindings.state.flags));
    if (settings.words[4])
        settings.words[3] =
            std::bit_cast<std::uint32_t>(acquisition_point_scale(frame, target, settings, b));
    const auto target_origin = read<RageVector>(target, b.targeting.offsets.origin);
    const auto x = target_origin[0] - frame.origin_10c[0];
    const auto y = target_origin[1] - frame.origin_10c[1];
    const auto z = target_origin[2] - frame.origin_10c[2];
    const auto squared = (x * x + y * y) + z * z;
    if (squared > 4000000.0f) {
        settings.words[4] &= ~0x2F0U;
        if (settings.words[1] != 0x200U)
            settings.words[1] &= ~0x200U;
    }
    if (squared > 1000000.0f)
        settings.words[4] &= ~0x200U;
    // This hotkey is evaluated before the unconditional cache lookup.
    const auto body_key = hotkey_active(b.global.force_body_aim.value.binding, b.input);
    auto& player = *bindings.scan.players.get(entity);
    auto countdown = read<std::int32_t>(&player.native, 0xF8);
    if (countdown > 0) {
        settings.option(0xDD, 1);
        if (!(bindings.state.flags & 9U)) {
            --countdown;
            write(&player.native, 0xF8, countdown);
        }
    }
    const auto& adjustments = bindings.player_adjustments.records[entity].adjustments;
    if (adjustments.prefer_body_aim == 1)
        settings.option(0xDD, 0);
    else if (adjustments.prefer_body_aim == 2 || adjustments.prefer_body_aim == 3)
        settings.option(0xDD, 1);
    auto* weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
    const auto restricted = [&] {
        return (bindings.state.flags & 0x400U) &&
               read<std::uint16_t>(weapon, b.targeting.offsets.weapon_identifier) != 31;
    };
    if ((adjustments.prefer_body_aim == 3 || body_key) && !restricted()) {
        settings.words[1] &= 0xC;
        settings.words[2] &= 0xC;
        if (!(settings.words[1] | settings.words[2]))
            settings.words[1] = 0xC;
    }
    if (adjustments.safe_point == 2) {
        settings.words[2] |= settings.words[1];
        settings.words[1] = 0;
    }
    if (!settings.words[1] && settings.words[2])
        settings.words[2] &= ~0x200U;
    const auto correction = read<float>(&player.native, 0x51C);
    if (adjustments.correction_active && b.global.prefer_safe_point.value &&
        (settings.words[1] & 0xC) == 0xC && !(correction >= 0.5f) &&
        read<std::int32_t>(target, b.targeting.offsets.team) == 2 &&
        read<float>(target, b.targeting.offsets.duck_amount) >= 0.8f &&
        (read<std::uint8_t>(target, b.targeting.offsets.movement_flags) & 1U))
        settings.words[4] &= ~8U;
    if (restricted()) {
        settings.words[1] &= 2;
        settings.words[2] &= 2;
        settings.words[4] &= 2;
        settings.option(0xDD, 0);
        settings.option(0xDF, 0);
    }
    // Visualization deliberately admits only head/stomach/leg groups, and
    // folds accuracy modes above one back to one after the normal policy work.
    settings.words[0] = std::uint32_t(original_mode > 1 ? 1 : original_mode);
    settings.words[1] &= 0xCAU;
    settings.words[2] &= 0xCAU;
    settings.words[4] &= 0xCAU;
}
std::array<RageVector, 2> origins(const client::CommandFrame& frame, void* target,
                                  const ActionStateBindings& bindings) {
    using namespace rage_point_detail;
    const auto position =
        read<RageVector>(target, bindings.target_preparation.targeting.offsets.origin);
    const RageVector delta{position[0] - frame.origin_10c[0], position[1] - frame.origin_10c[1],
                           0.0f};
    const auto axis = normalize(delta, length(delta));
    const auto side = basis_from_axis(axis).side;
    std::array<RageVector, 2> result;
    for (std::size_t i = 0; i != 3; ++i) {
        const auto offset = side[i] * 36.0f;
        result[0][i] = frame.origin_dc[i] + offset;
        result[1][i] = frame.origin_dc[i] - offset;
    }
    for (auto& end : result) {
        MovementCollisionTrace trace;
        const auto& start = frame.origin_dc;
        const auto ray = make_movement_hull_ray(
            {start[0], start[1], start[2]}, {end[0], end[1], end[2]}, {-16, -16, -18}, {16, 16, 2});
        trace_movement_world(bindings.collision, ray, 1, trace, bindings.trace_flag);
        end = {trace.end.x, trace.end.y, trace.end.z};
    }
    return result;
}
} // namespace skeet::rage::action_detail

// ============================================================================
// Subcomponent: action_state.cpp
// ============================================================================

namespace skeet::rage {
namespace {
using action_detail::read;
struct ActionHistoryFrame {
    lagcomp::HistoryTickTiming timing;
    lagcomp::EntityPreparationContext preparation;
    lagcomp::ExtrapolatedRecordBindings extrapolation;
    lagcomp::HistorySelectionBindings history;
    const client::CommandBudgetBindings& live_budget;
    ActionHistoryFrame(const lagcomp::HistorySelectionBindings& seed,
                       const client::CommandFrame& frame, float clock,
                       const client::CommandBudgetBindings& budget)
        : timing{clock,
                 read<float>(&frame, 0x198),
                 seed.interpolation,
                 seed.extrapolation.preparation.entity.interval,
                 seed.maximum_unlag_ms,
                 read<std::int32_t>(&frame, 0xD4),
                 std::bit_cast<std::int32_t>(frame.field_18c)},
          preparation(seed.extrapolation.preparation),
          extrapolation{seed.extrapolation.players,     preparation,
                        seed.extrapolation.attachments, seed.extrapolation.bones,
                        seed.extrapolation.movement,    seed.extrapolation.prediction_limit},
          history{extrapolation, seed.alternative,   seed.rotation,        seed.matrix_scratch,
                  clock,         seed.interpolation, seed.maximum_unlag_ms},
          live_budget(budget) {
        preparation.global_frame = std::bit_cast<std::uint32_t>(clock);
        preparation.current_timing =
            seed.extrapolation.preparation.current_timing ? &timing : nullptr;
    }
    ActionHistoryFrame(const ActionHistoryFrame&) = delete;
    ActionHistoryFrame& operator=(const ActionHistoryFrame&) = delete;
};
void refresh_target(const client::CommandFrame& frame, client::Command& command,
                    const TargetBindings& targeting, const ActionStateBindings& bindings) {
    if (auto* live = std::get_if<std::reference_wrapper<const client::TargetRefreshRuntimeOwner>>(
            &bindings.preamble.target_refresh)) {
        live->get().refresh(bindings.preamble.yaw_target, targeting.local_player, command,
                            frame.previous_command.movement.buttons, read<float>(&frame, 0x11C));
        return;
    }
    const auto& refresh = std::get<std::reference_wrapper<const anti_aim::TargetRefreshContext>>(
                              bindings.preamble.target_refresh)
                              .get();
    const auto& base = refresh.scored;
    const auto& seed = base.common;
    const anti_aim::TargetSelectorContext common{targeting,
                                                 seed.engine_client,
                                                 seed.max_clients,
                                                 seed.histories,
                                                 seed.offsets,
                                                 seed.timing,
                                                 seed.prediction_limit,
                                                 seed.interval,
                                                 seed.receive_interval,
                                                 seed.collision,
                                                 seed.ballistics,
                                                 seed.trace_flag};
    const anti_aim::TargetScoredContext scored{
        {common, command, frame.previous_command.movement.buttons, read<float>(&frame, 0x11C),
         base.player_rules, base.dormant_health, base.offsets, base.geometry_offsets,
         base.local_pose_origin, base.local_pose, base.trigonometry, base.sequence,
         base.clock_owner, base.local_time_offset, base.trace_filter_vtable, base.trace_entities},
        bindings.live_movement.capture,
        bindings.live_movement.environment};
    anti_aim::refresh_yaw_target(bindings.preamble.yaw_target, {scored, refresh.selector_convar});
}
bool accepts_point(const action_detail::Point& point, const client::CommandFrame& frame,
                   const scan_detail::Geometry& geometry, std::int32_t requested_group,
                   std::int32_t minimum_damage, void* local, void* target,
                   const ActionStateBindings& bindings) {
    auto* weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
    auto query = prepare_target_bullet(weapon, bindings.scan.weapon_item_offset,
                                       bindings.preamble.settings.automatic_penetration.value != 0);
    const auto delta = rage_point_detail::subtract(point.position, frame.origin_dc);
    const auto direction = rage_point_detail::normalize(delta, rage_point_detail::length(delta));
    if (point.flags & 1U) {
        RageVector shot;
        for (std::size_t axis = 0; axis != 3; ++axis)
            shot[axis] = query.range * direction[axis];
        const auto multiplier =
            rage_detail::hitgroup_multipliers[std::uint32_t(requested_group) & 15U];
        if (!rage_point_safe({frame.origin_dc, shot}, requested_group, multiplier, geometry.views(),
                             bindings.scan.box_clipper))
            return false;
    }
    RageVector extended;
    for (std::size_t axis = 0; axis != 3; ++axis)
        extended[axis] = (direction[axis] * 120.0f + point.position[axis]) - frame.origin_dc[axis];
    const auto projection = intersect_scan_hitboxes({frame.origin_dc, extended}, geometry.primary,
                                                    bindings.scan.box_clipper);
    if (!projection.found || geometry.primary[projection.hitbox].hitgroup != requested_group)
        return false;
    // Native publishes this projected intersection into its temporary trace;
    // the ensuing point-to-point bullet call still receives the descriptor.
    anti_aim::TargetTraceFilter native_filter(bindings.scan.trace_filter_vtable,
                                              bindings.scan.trace_entities, 0x600400BU, 0x600400BU,
                                              1.0f);
    const auto filter = native_filter.binding();
    const auto source = BallisticEntity(reinterpret_cast<std::uintptr_t>(local));
    if (!trace_ballistic_bullet(bindings.scan.ballistics, source, frame.origin_dc, point.position,
                                filter, query))
        return false;
    const auto info = bindings.scan.ballistics.entity_info(
        BallisticEntity(reinterpret_cast<std::uintptr_t>(target)));
    const auto damage = final_bullet_damage(
        query.damage, query.armor_ratio, std::bit_cast<std::int32_t>(query.reserved),
        query.weapon_id, requested_group, info.armor, info.helmet);
    return damage >= minimum_damage && damage >= 0;
}
}
void update_action_state(esp::HitboxHighlightState& output, client::CommandFrame& frame,
                         void* local, client::Command& command,
                         const ActionStateBindings& bindings) {
    if (!bindings.active)
        return;
    for (auto& entry : output.entries)
        entry.flags = 0;
    output.target_handle = -1;
    if (!bindings.visualization.enabled.value && !bindings.visualization.safe_point.value)
        return;
    if (!frame.weapon_address || frame.weapon_id_d8 == 31)
        return;
    auto* weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
    const auto& seed = bindings.target_preparation.targeting;
    const TargetBindings targeting{
        seed.local_player,           seed.entity_list, seed.game_type, seed.teammates_are_enemies,
        bindings.player_adjustments, seed.offsets};
    if (!lagcomp::history_weapon_is_firearm(
            read<std::uint16_t>(weapon, targeting.offsets.weapon_identifier)))
        return;
    refresh_target(frame, command, targeting, bindings);
    const auto handle = bindings.preamble.yaw_target.handle;
    auto* target = handle == -1 ? nullptr : engine::invoke<void*>(targeting.entity_list, 4, handle);
    if (!target_is_eligible(targeting, target))
        return;
    const auto movement = read<std::uint8_t>(target, targeting.offsets.movement_type);
    if (movement == 8 || movement == 10 ||
        (read<std::uint8_t>(target, targeting.offsets.movement_flags) & 0x40U))
        return;
    const auto initial_model = scan_detail::model(target, bindings.scan.model_offsets);
    if (!initial_model.header || initial_model.hitboxes.empty())
        return;
    AcquisitionTargetSettings settings{
        prepare_rage_scan_template(bindings.state, frame, bindings.preamble)};
    const SpreadParameters spread{std::uint32_t(frame.weapon_id_d8),
                                  std::int32_t(frame.field_b0),
                                  read<float>(&frame, 0xD0),
                                  read<float>(&frame, 0xC4),
                                  std::bit_cast<float>(frame.field_100),
                                  read<std::int32_t>(&frame, 0x19C),
                                  bindings.preamble.shotgun_patterns,
                                  frame.weapon_kind_124};
    prepare_spread_cache(bindings.state.spread, spread, std::bit_cast<float>(frame.field_160),
                         std::uint32_t(frame.command_number_ac),
                         (bindings.state.flags & 0x10U) != 0, bindings.preamble.spread_tables,
                         bindings.preamble.random, bindings.preamble.pattern,
                         bindings.preamble.trigonometry);
    const auto networkable = static_cast<std::byte*>(target) + 8;
    const auto initial_index = engine::invoke<std::uint32_t>(networkable, 10);
    const auto health = engine::invoke<std::int32_t>(target, 122);
    PendingTargetDamage pending;
    const auto pending_present =
        pending_target_damage(feedback_owner_state(bindings.feedback_owner), pending);
    (void)scan_target_health(health, initial_index, pending_present ? &pending : nullptr);
    const auto index = engine::invoke<std::uint32_t>(networkable, 10);
    action_detail::prepare_settings(frame, target, index, settings, bindings);
    if (!(settings.words[1] | settings.words[2]))
        return;
    const auto shifted_origins = action_detail::origins(frame, target, bindings);
    const auto history_index = engine::invoke<std::uint32_t>(networkable, 10);
    auto& player = *bindings.scan.players.get(history_index);
    bindings.state.flags |= 1U;
    const auto requested_tick =
        player.history.count
            ? player.history.at(0).header.simulation_tick
            : lagcomp::time_to_tick(
                  read<float>(target, bindings.scan.preparation.entity.offsets.simulation_time),
                  bindings.preamble.interval);
    lagcomp::LagRecord record;
    record.header.field_00 = 0;
    std::uint32_t record_flags = 0;
    // The selected historical frame and readiness can both change the live
    // prediction clock after the acquisition bindings were constructed.
    // Own every refreshed reference until the complete history call returns.
    const ActionHistoryFrame selection(bindings.history, frame, bindings.preamble.prediction_time,
                                       bindings.live_budget);
    if (!lagcomp::select_target_history_record(
            frame, target, requested_tick, 8, record, record_flags, selection.history,
            bindings.bone_runtime, bindings.live_movement, selection.live_budget)) {
        bindings.state.flags &= ~1U;
        return;
    }
    // The original prepares one query before geometry and a fresh one for
    // every descriptor; its range is used by the three-hypothesis safety rays.
    (void)prepare_target_bullet(weapon, bindings.scan.weapon_item_offset,
                                bindings.preamble.settings.automatic_penetration.value != 0);
    const auto original_origin = frame.origin_dc;
    const auto safe_mask = bindings.visualization.safe_point.value && (record.header.flags & 0x80U)
                               ? settings.words[2]
                               : 0U;
    const auto model = scan_detail::model(target, bindings.scan.model_offsets);
    scan_detail::Geometry geometry;
    geometry.primary =
        prepare_rage_scan_hitboxes(model.hitboxes, record.matrices, bindings.scan.trigonometry);
    if (safe_mask)
        for (std::size_t i = 0; i != 3; ++i)
            geometry.alternatives[i] = prepare_rage_scan_hitboxes(
                model.hitboxes, record.alternatives[i].matrices, bindings.scan.trigonometry);
    for (auto& entry : output.entries) {
        if (entry.hitbox >= std::int32_t(initial_model.hitboxes.size()))
            continue;
        const auto index = std::uint32_t(entry.hitbox);
        const auto group = initial_model.hitboxes[index].hitgroup;
        const auto group_bit = 1U << (std::uint32_t(group) & 31U);
        if (!(group_bit & (safe_mask | settings.words[1])))
            continue;
        for (const auto& origin : shifted_origins) {
            frame.origin_dc = origin;
            const auto points = action_detail::points(
                frame, settings, record, model, geometry, index, (safe_mask & group_bit) != 0,
                bindings.visualization.enabled.value != 0, bindings);
            bool accepted = false;
            for (const auto& point : points) {
                if (!accepts_point(point, frame, geometry, group,
                                   std::bit_cast<std::int32_t>(settings.words[5]), local, target,
                                   bindings))
                    continue;
                entry.flags |= point.highlight;
                accepted = true;
                break;
            }
            if (accepted)
                break;
        }
        if (entry.flags) {
            const auto reference = engine::invoke<const std::uint32_t*>(target, 2);
            output.target_handle = std::bit_cast<std::int32_t>(*reference);
        }
    }
    frame.origin_dc = original_origin;
    bindings.state.flags &= ~1U;
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: body_aim_probe.cpp
// ============================================================================

namespace skeet::rage {
namespace {
constexpr float epsilon = std::numeric_limits<float>::epsilon();
/* deduplicated helper */
MovementVector movement(const RageVector& v) noexcept {
    return {v[0], v[1], v[2]};
}
float negate(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) ^ 0x80000000U);
}
MovementCollisionTrace empty_trace() noexcept {
    MovementCollisionTrace trace{};
    trace.fraction = 1.0f;
    trace.hitgroup = -1;
    trace.hitbox = -1;
    return trace;
}
RageVector transform(const Matrix3x4& bone, const RageVector& v) noexcept {
    RageVector out;
    for (std::size_t row = 0; row < 3; ++row)
        out[row] =
            ((bone.rows[row][1] * v[1] + bone.rows[row][0] * v[0]) + bone.rows[row][2] * v[2]) +
            bone.rows[row][3];
    return out;
}
void flatten(MovementVector& v) noexcept {
    if (v.z == 0.0f)
        return;
    const auto inverse = 1.0f / (std::sqrt((v.x * v.x + v.y * v.y) + 0.0f * 0.0f) + epsilon);
    v = {v.x * inverse, v.y * inverse, 0.0f * inverse};
}
bool gate(const client::CommandFrame& frame, const AcquisitionTargetSettings& settings,
          const BodyAimProbeContext& context) noexcept {
    if (!settings.option(0xDF))
        return false;
    const auto regular = settings.words[1], safe = settings.words[2], combined = regular | safe;
    if (!(combined & 8U) || (context.context_flags & 3U) || frame.weapon_id_d8 == 31)
        return false;
    if ((frame.previous_command.movement.buttons & 2U) || !(frame.flags & 0x40000U))
        return false;
    if (frame.weapon_id_d8 == 64 && !(context.weapon_flags & 1U))
        return false;
    if ((frame.flags & 0x102000U) || (!regular && safe) || !(combined & 0xFFFFFFF3U))
        return false;
    return true;
}
bool probe_origin(const client::CommandFrame& frame, void* target,
                  const BodyAimProbeContext& context, RageVector& origin) {
    auto center = field<RageVector>(target, context.offsets.network_origin);
    const auto* minimum = engine::invoke<const RageVector*>(target, 76);
    const auto* maximum = engine::invoke<const RageVector*>(target, 77);
    center[2] += ((*minimum)[2] + (*maximum)[2]) * 0.5f;
    MovementCollisionRay ray;
    ray.start = movement(frame.origin_dc);
    ray.delta = {center[0] - frame.origin_dc[0], center[1] - frame.origin_dc[1],
                 center[2] - frame.origin_dc[2]};
    ray.start_offset = ray.extents = {0, 0, 0};
    ray.is_ray = true;
    ray.swept =
        (ray.delta.y * ray.delta.y + ray.delta.x * ray.delta.x) + ray.delta.z * ray.delta.z != 0.0f;
    auto trace = empty_trace();
    trace_movement_world(context.collision, ray, 1, trace, context.trace_flag);
    if (!(1.0f > trace.fraction) && !trace.all_solid && !trace.start_solid)
        return false;
    origin = {frame.origin_10c[0], frame.origin_10c[1], frame.origin_10c[2] + 64.0f};
    const auto buttons = frame.previous_command.movement.buttons;
    if (!(frame.scalar_c0 > 2.0f)) {
        if (buttons & 4U)
            return false;
        const auto previous = field<float>(&frame, 0x98);
        return previous > 0.0f && field<float>(&frame, 0x1A4) > previous;
    }
    if (!(buttons & 0x618U))
        return false;
    const auto target_velocity = field<RageVector>(target, context.offsets.velocity);
    if (std::sqrt(target_velocity[0] * target_velocity[0] +
                  target_velocity[1] * target_velocity[1]) > 135.1999969482422f) {
        const auto target_inverse = 1.0f / (std::sqrt(target_velocity[0] * target_velocity[0] +
                                                      target_velocity[1] * target_velocity[1]) +
                                            epsilon);
        const auto local_inverse =
            1.0f / (std::sqrt(frame.velocity_150[0] * frame.velocity_150[0] +
                              frame.velocity_150[1] * frame.velocity_150[1]) +
                    epsilon);
        const auto dot =
            (local_inverse * frame.velocity_150[1]) * (target_inverse * target_velocity[1]) +
            (local_inverse * frame.velocity_150[0]) * (target_inverse * target_velocity[0]);
        if (!(dot >= -0.70710688829422f))
            return false;
    }
    MovementVector forward, right;
    const auto& angles = frame.previous_command.angles;
    movement_angle_vectors({angles[0], angles[1], angles[2]}, &forward, &right, nullptr);
    flatten(forward);
    flatten(right);
    const auto& command = frame.previous_command.movement;
    MovementVector displacement{right.x * command.side_move + forward.x * command.forward_move,
                                right.y * command.side_move + forward.y * command.forward_move,
                                right.z * command.side_move + forward.z * command.forward_move};
    const auto distance = frame.weapon_id_d8 == 9 || frame.weapon_id_d8 == 40 ? 24.0f : 18.0f;
    const auto scale =
        distance / (std::sqrt((displacement.z * displacement.z + displacement.y * displacement.y) +
                              displacement.x * displacement.x) +
                    epsilon);
    displacement = {displacement.x * scale, displacement.y * scale, displacement.z * scale};
    const MovementVector start{frame.origin_10c[0], frame.origin_10c[1],
                               frame.origin_10c[2] + 18.0f};
    const auto hull_minimum = field<MovementVector>(&frame, 0x1B8);
    auto hull_maximum = field<MovementVector>(&frame, 0x1C4);
    hull_maximum.z -= 18.0f;
    ray = make_movement_hull_ray(
        start, {start.x + displacement.x, start.y + displacement.y, start.z + displacement.z},
        hull_minimum, hull_maximum);
    trace = empty_trace();
    context.collision.trace_ray(
        ray, 1, {MovementCollisionFilterKind::skip_player, context.source, 8, 0}, trace);
    if (1.0f > trace.fraction && !trace.start_solid && !trace.all_solid) {
        origin = {(trace.end.x - start.x) + frame.origin_dc[0],
                  (trace.end.y - start.y) + frame.origin_dc[1],
                  (trace.end.z - start.z) + frame.origin_dc[2]};
    } else if (trace.fraction == 1.0f) {
        origin = {displacement.x + frame.origin_dc[0], frame.origin_dc[1] + displacement.y,
                  frame.origin_dc[2] + displacement.z};
    }
    return true;
}
}

std::array<RageVector, 2> body_probe_surface_points(const RageCapsule& capsule,
                                                    const RageVector& origin) noexcept {
    const auto& a = capsule.first;
    const auto& b = capsule.second;
    RageVector direction{(b[0] + a[0]) * 0.5f - origin[0], (b[1] + a[1]) * 0.5f - origin[1],
                         (b[2] + a[2]) * 0.5f - origin[2]};
    const auto inverse =
        1.0f / (std::sqrt((direction[0] * direction[0] + direction[1] * direction[1]) +
                          direction[2] * direction[2]) +
                epsilon);
    direction = {inverse * direction[0], direction[1] * inverse, direction[2] * inverse};
    RageVector perpendicular;
    if (direction[0] == 0.0f && direction[1] == 0.0f)
        perpendicular = {0.0f, -1.0f, 0.0f};
    else {
        const auto zero_z = direction[2] * 0.0f;
        perpendicular = {direction[1] - zero_z, zero_z - direction[0],
                         direction[0] * 0.0f - direction[1] * 0.0f};
        const auto normalize =
            1.0f /
            (std::sqrt((perpendicular[0] * perpendicular[0] + perpendicular[1] * perpendicular[1]) +
                       perpendicular[2] * perpendicular[2]) +
             epsilon);
        perpendicular = {perpendicular[0] * normalize, perpendicular[1] * normalize,
                         perpendicular[2] * normalize};
    }
    const RageVector axis{b[0] - a[0], b[1] - a[1], b[2] - a[2]};
    auto along = [&](const RageVector& p) {
        return (axis[1] * p[1] + axis[0] * p[0]) + axis[2] * p[2];
    };
    std::array<RageVector, 2> out;
    const auto& first = along(perpendicular) >= 0.0f ? b : a;
    const auto first_scale =
        capsule.radius /
        std::sqrt((perpendicular[2] * perpendicular[2] + perpendicular[1] * perpendicular[1]) +
                  perpendicular[0] * perpendicular[0]);
    out[0] = {first_scale * perpendicular[0] + first[0], first_scale * perpendicular[1] + first[1],
              first_scale * perpendicular[2] + first[2]};
    perpendicular = {negate(perpendicular[0]), negate(perpendicular[1]), negate(perpendicular[2])};
    const auto& second = along(perpendicular) >= 0.0f ? b : a;
    const auto second_scale =
        capsule.radius /
        std::sqrt((perpendicular[0] * perpendicular[0] + perpendicular[1] * perpendicular[1]) +
                  perpendicular[2] * perpendicular[2]);
    out[1] = {second_scale * perpendicular[0] + second[0],
              second_scale * perpendicular[1] + second[1],
              perpendicular[2] * second_scale + second[2]};
    return out;
}

bool probe_body_aim(client::CommandFrame& frame, AcquisitionTargetSettings& settings, void* target,
                    const BodyAimProbeContext& context) {
    if (!gate(frame, settings, context))
        return false;
    RageVector origin;
    if (!probe_origin(frame, target, context, origin))
        return false;
    auto query = prepare_target_bullet(context.weapon, context.offsets.weapon_item,
                                       context.allow_penetration);
    anti_aim::TargetTraceFilter filter(context.trace_filter_vtable, context.trace_entities);
    const auto binding = filter.binding();
    const auto wrapper = field<const void*>(target, context.offsets.studio_header);
    const std::byte* hitbox = nullptr;
    if (wrapper && field<const void*>(wrapper, 0)) {
        const auto studio = field<const std::byte*>(wrapper, 0);
        const auto set =
            studio + field<std::int32_t>(studio, 0xB0) +
            std::ptrdiff_t(field<std::int32_t>(target, context.offsets.hitbox_set)) * 12;
        if (set && field<std::int32_t>(set, 4))
            hitbox = set + field<std::int32_t>(set, 8) + 6 * 0x44;
    }
    const auto bones = field<const Matrix3x4*>(target, context.offsets.cached_bones);
    const auto& bone = bones[field<std::int32_t>(hitbox, 0)];
    const RageCapsule capsule{transform(bone, field<RageVector>(hitbox, 8)),
                              transform(bone, field<RageVector>(hitbox, 0x14)),
                              field<float>(hitbox, 0x30)};
    const auto points = body_probe_surface_points(capsule, origin);
    const auto target_entity = BallisticEntity(reinterpret_cast<std::uintptr_t>(target));
    std::int32_t maximum_damage = 0;
    for (const auto& point : points) {
        trace_ballistic_bullet(context.ballistics, context.source, origin, point, binding, query);
        if (!_mm_cvttss_si32(_mm_set_ss(query.damage)))
            continue;
        const auto hitgroup = field<std::int32_t>(hitbox, 4);
        const auto info = context.ballistics.entity_info(target_entity);
        const auto damage = final_bullet_damage(query.damage, query.armor_ratio,
                                                std::bit_cast<std::int32_t>(query.reserved),
                                                query.weapon_id, hitgroup, info.armor, info.helmet);
        if (maximum_damage < damage)
            maximum_damage = damage;
    }
    if (!maximum_damage)
        return false;
    auto health = engine::invoke<std::int32_t>(target, 122);
    if (frame.weapon_id_d8 != 40 && frame.field_1d4 == 1 && !(frame.flags & 0x8000000U))
        health = std::bit_cast<std::int32_t>(std::uint32_t(health) - std::uint32_t(maximum_damage));
    const auto minimum_damage = std::bit_cast<std::int32_t>(settings.words[5]);
    const auto required = minimum_damage < health ? health : minimum_damage;
    if (maximum_damage < required)
        return false;
    if (!(context.context_flags & 0x400U) || frame.weapon_id_d8 == 31) {
        settings.words[2] &= 0xCU;
        settings.words[1] &= 0xCU;
        if (!(settings.words[1] | settings.words[2]))
            settings.words[1] = 0xCU;
    }
    frame.flags |= 0x1000000U;
    return true;
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: command_owner.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
FiringState firing_state(const client::CommandFrame& frame, std::uint8_t& send_packet) noexcept {
    FiringState result;
    result.flags = frame.flags;
    result.weapon_id = frame.weapon_id_d8;
    result.marker = frame.command_number_ac;
    result.previous_tick = frame.tick_base_c8;
    result.current_tick = std::bit_cast<std::int32_t>(frame.field_1d8);
    result.field_fc = frame.choke_limit_fc;
    result.field_15c = frame.choked_15c;
    result.field_1d4 = std::bit_cast<std::int32_t>(frame.field_1d4);
    result.field_198 = field<float>(&frame, 0x198);
    result.reference_angles = {frame.previous_command.angles[0], frame.previous_command.angles[1],
                               frame.previous_command.angles[2]};
    result.recoil = frame.recoil_angles_9c;
    result.origin = frame.origin_dc;
    result.bone_origin = frame.origin_10c;
    result.field_84 = frame.view_height_84;
    result.field_a8 = frame.player_field_a8;
    result.field_164 = frame.height_164;
    result.special_threshold = frame.postpone_fire_1e4;
    result.angle_step = frame.maximum_angle_step_1e8;
    result.send_packet = &send_packet;
    return result;
}
void publish_firing_state(client::CommandFrame& frame, const FiringState& state) noexcept {
    // These are the frame members written by readiness/angles/commit. The
    // remaining projection fields are inputs and retain their native bytes.
    frame.flags = state.flags;
    frame.tick_base_c8 = state.previous_tick;
    frame.origin_dc = state.origin;
    frame.view_height_84 = state.field_84;
}
}
void process_rage_command_phase2(CommandOwner& owner, void* player, void* weapon,
                                 client::Command& command, std::array<float, 3>& incoming,
                                 std::uint8_t& send_packet) {
    process_rage_command_phase2(owner.state, player, weapon, command, incoming, send_packet,
                                owner.phase2);
}
void process_rage_phase6(CommandOwner& owner, const client::CommandFrame& frame,
                         const client::Command& command) {
    process_rage_phase6(frame, command, owner.phase6);
}
void capture_rage_command_frame(CommandOwner& owner, const client::CommandFrame& frame) noexcept {
    capture_rage_command_frame(owner.state, frame);
}
void process_rage_command(CommandOwner& owner, client::CommandFrame& original, void* player,
                          client::Command& command, std::uint8_t& send_packet) {
    auto& state = owner.state;
    auto& storage = owner.acquisition;
    auto preamble = prepare_rage_command(state, original, command, player, storage, owner.preamble);
    switch (preamble.continuation) {
    case PreambleContinuation::no_weapon:
        finish_rage_command(state, preamble, original, player, owner.tail,
                            PreambleTailEntry::return_only);
        return;
    case PreambleContinuation::inactive:
        finish_rage_command(state, preamble, original, player, owner.tail,
                            PreambleTailEntry::actions);
        return;
    case PreambleContinuation::ammunition:
        finish_rage_command(state, preamble, original, player, owner.tail,
                            PreambleTailEntry::after_firing);
        return;
    case PreambleContinuation::single_target:
    case PreambleContinuation::multiple_targets:
        break;
    }
    auto& frame = *preamble.frame;
    auto& selected_command = *preamble.command;
    AcquisitionFrameBindings current(state, preamble, owner.preamble.prediction_time,
                                     owner.preamble.yaw_target.handle,
                                     owner.preamble.scheduler_flags, owner.single_target,
                                     owner.multiple_targets, owner.settings_menu);
    const bool single = preamble.continuation == PreambleContinuation::single_target;
    bool acquired = true;
    if (single)
        acquired = acquire_single_target(state, preamble, storage, current.single);
    else
        acquire_multiple_targets(state, preamble, storage, current.multiple);
    auto& scanner = single ? current.single.records.scanner : current.multiple.records.scanner;
    TargetBatch batch;
    // A missing single selection jumps directly to54908, bypassing the
    // completion traversal, job drain and per-player cleanup.
    if (acquired)
        batch = rank_completed_target_candidates(storage.candidates, state.completed, state.flags,
                                                 state.target_mask, scanner.targeting);
    const FinalTargetContextBindings final_target{scanner, owner.final_target.current_bone_counter,
                                                  owner.settings_menu, owner.final_target.jobs};
    auto choice = select_acquired_target_for_shot(batch, storage, frame, state, final_target);
    RageShotCommitState commit{state.flags, owner.preamble.scheduler_flags,
                               owner.preamble.selected_marker, send_packet};
    TargetFiringResult firing{
        batch.count ? TargetFiringStatus::rejected_batch : TargetFiringStatus::empty_batch, choice};
    if (choice.candidate) {
        const auto& evaluation = acquisition_evaluation_for(storage, *choice.candidate);
        const TargetFiringTiming timing{evaluation.requested_tick,
                                        evaluation.record.header.simulation_tick};
        auto firing_frame = firing_state(frame, send_packet);
        RageShotState shot{
            frame.flags, frame.weapon_id_d8, frame.postpone_fire_1e4, {}, frame.command_number_ac};
        auto readiness = owner.readiness;
        readiness.tick_interval = owner.preamble.interval;
        readiness.correction_time = 0.5f;
        readiness.enabled = owner.preamble.settings.automatic_fire.value != 0;
        readiness.mode_enabled = owner.preamble.settings.silent_aim.value != 0;
        readiness.send_mode = owner.preamble.send_mode;
        readiness.committed_marker = owner.preamble.selected_marker;
        readiness.command_marker = owner.preamble.current_sequence;
        if (frame.weapon_id_d8 == 64)
            readiness.revolver_gate =
                field<float>(reinterpret_cast<const void*>(std::uintptr_t(frame.weapon_address)),
                             owner.phase2.offsets.fire_state.next_secondary);
        readiness.scheduler_flags = commit.context_flags;
        const FiringTargetData target{*choice.candidate, timing.field_b4, timing.field_13c};
        const auto ready =
            evaluate_firing_readiness(firing_frame, target, preamble.firing, owner.firing_shared,
                                      selected_command, readiness);
        if (ready == FiringReadiness::retained_target)
            firing = {TargetFiringStatus::retained_target, choice};
        else if (ready == FiringReadiness::stopped)
            firing = {TargetFiringStatus::readiness_stopped, choice};
        else {
            auto post = owner.post_angles;
            post.player = player;
            post.enabled = owner.preamble.settings.automatic_fire.value != 0;
            post.send_mode = owner.preamble.send_mode;
            post.recoil_option = owner.preamble.settings.remove_recoil.value != 0;
            post.compensate_recoil =
                legit::recoil_compensation_enabled(
                    owner.tail.legit_settings,
                    std::uint32_t(std::int32_t(std::int8_t(frame.weapon_kind_124))),
                    owner.preamble.input) ||
                !post.recoil_option;
            commit.command_flags = owner.preamble.scheduler_flags;
            firing = targeting_detail::aim_and_commit_target(
                choice, firing_frame, preamble.firing, selected_command, shot, commit, readiness,
                post,
                single ? current.single.records.bone_runtime
                       : current.multiple.records.bone_runtime);
        }
        publish_firing_state(frame, firing_frame);
        state.flags = commit.context_flags;
        if (firing.status == TargetFiringStatus::recorded) {
            std::memcpy(reinterpret_cast<std::byte*>(&frame) + 0xE8, shot.selected_aim_point.data(),
                        sizeof shot.selected_aim_point);
            owner.preamble.selected_marker = commit.last_committed_marker;
            owner.preamble.scheduler_flags = commit.command_flags;
        }
    }
    const auto& base = owner.post_firing;
    const auto& movement = base.movement;
    const PostFiringMovementBindings live_movement{owner.preamble.settings,
                                                   owner.preamble.input,
                                                   state.flags,
                                                   owner.preamble.scheduler_flags,
                                                   state.weapon_flags,
                                                   state.target_mask,
                                                   scanner.targeting,
                                                   movement.offsets,
                                                   scanner.ballistics,
                                                   scanner.source,
                                                   scanner.weapon_item_offset,
                                                   scanner.trace_filter_vtable,
                                                   scanner.trace_entities,
                                                   movement.prediction,
                                                   owner.preamble.prediction_time,
                                                   owner.preamble.yaw_target.handle};
    const auto& branch_prediction =
        single ? current.single.prediction : current.multiple.prediction;
    auto trace = branch_prediction.trace;
    trace.context_flags = state.flags;
    trace.weapon_flags = state.weapon_flags;
    trace.weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
    const TargetPredictionBindings prediction{trace,
                                              branch_prediction.record_selection,
                                              branch_prediction.engine_trace,
                                              branch_prediction.local_maximum_speed,
                                              owner.preamble.scheduler_flags,
                                              &selected_command};
    const PostFiringBindings continuation{owner.preamble,
                                          live_movement,
                                          base.players,
                                          base.command_angles,
                                          final_target,
                                          base.scripts,
                                          base.diagnostics,
                                          base.pending_shots,
                                          base.shot_models,
                                          base.shot_index,
                                          base.entity_limit,
                                          base.tick_correction,
                                          nullptr,
                                          base.stop,
                                          base.simulation_capture,
                                          base.collision,
                                          prediction,
                                          scanner,
                                          single ? current.single.records.bone_runtime
                                                 : current.multiple.records.bone_runtime};
    const auto tail =
        continue_after_target_firing(state, preamble, storage, batch, firing, player, continuation);
    finish_rage_command(state, preamble, original, player, owner.tail, tail);
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: command_phase2.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
bool accuracy_mode(const void* object) noexcept {
    for (;;) {
        const auto* parent = field<const void*>(object, 0x1C);
        if (parent == object)
            break;
        object = parent;
    }
    return field<std::uint32_t>(object, 0x30) !=
           std::uint32_t(reinterpret_cast<std::uintptr_t>(object));
}
bool rule_blocks_fire(const CommandPhase2Bindings& bindings) noexcept {
    return bindings.game_rules && *bindings.game_rules &&
           field<std::uint8_t>(*bindings.game_rules, bindings.offsets.fire_state.rule_d720aea6) !=
               0;
}
void prepare_revolver(PreambleState& state, void* player, void* weapon, client::Command& command,
                      const CommandPhase2Bindings& bindings) {
    auto buttons = command.movement.buttons;
    if (buttons & 0x2000u)
        return;
    const auto& offsets = bindings.offsets.fire_state;
    if (field<std::uint8_t>(player, offsets.player_block_bcb263a5))
        return;
    if (accuracy_mode(bindings.accuracy_convar) || rule_blocks_fire(bindings))
        return;
    if (field<std::uint8_t>(player, offsets.player_state_a15c3105))
        return;
    if (field<std::int32_t>(weapon, offsets.ammunition) == 0)
        return;
    const auto next_tick = std::bit_cast<std::int32_t>(
        std::uint32_t(field<std::int32_t>(player, bindings.offsets.tick_base)) + 1u);
    const float time = float(next_tick) * bindings.interval;
    // COMISS/JB rejects unordered player timing. The later JBE branches
    // also distinguish equality and unordered values from ordered greater.
    if (!(time >= field<float>(player, offsets.player_next_attack)))
        return;
    if (time >= field<float>(weapon, offsets.next_primary) &&
        field<std::int32_t>(weapon, offsets.weapon_state_9e2078a1) == 208 &&
        time > field<float>(weapon, offsets.postpone_fire)) {
        if (!(field<float>(weapon, offsets.next_secondary) > time))
            return;
        buttons |= 0x800u;
    } else
        buttons |= 1u;
    command.movement.buttons = buttons;
    state.weapon_flags |= 1u;
}
}
void process_rage_command_phase2(PreambleState& state, void* player, void* weapon,
                                 client::Command& command, std::array<float, 3>& incoming_angles,
                                 std::uint8_t& send_packet, const CommandPhase2Bindings& bindings) {
    (void)incoming_angles;
    (void)send_packet;
    state.weapon_flags = 0;
    if (!bindings.settings.enabled.value ||
        !hotkey_active(bindings.settings.activation.value.binding, bindings.input) || !weapon)
        return;
    const auto& offsets = bindings.offsets.fire_state;
    if (!bindings.field_4347c9ec && field<std::uint16_t>(weapon, offsets.weapon_definition) == 64)
        prepare_revolver(state, player, weapon, command, bindings);
    if (field<std::int32_t>(weapon, offsets.ammunition) == 0 &&
        field<std::int32_t>(weapon, bindings.offsets.reserve_ammunition) > 0)
        command.movement.buttons = (command.movement.buttons & ~1u) | 0x2000u;
}
} //namespace skeet::rage

// ============================================================================
// Subcomponent: command_phase6.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
bool rules_block_command(const CommandPhase6Bindings& bindings) noexcept { //433DF22C
    return bindings.game_rules && *bindings.game_rules &&
           field<std::uint8_t>(*bindings.game_rules, bindings.rule_guard) != 0;
}
}

void publish_command_animation_angles(std::array<animation::AnimationAngleSample, 2>& samples,
                                      std::int32_t number,
                                      const std::array<float, 3>& angles) noexcept {
    if (number != samples[0].frame)
        std::memcpy(&samples[1], &samples[0], sizeof samples[0]);
    samples[0].frame = number;
    std::memcpy(samples[0].angles.data(), angles.data(), sizeof samples[0].angles);
}

void process_rage_phase6(const client::CommandFrame& frame, const client::Command& command,
                         const CommandPhase6Bindings& bindings) noexcept {
    if (!frame.weapon_address || !frame.weapon_data_address)
        return;
    if (rules_block_command(bindings))
        return;
    const auto* weapon = reinterpret_cast<const void*>(std::uintptr_t(frame.weapon_address));
    if (!lagcomp::history_weapon_is_firearm(
            field<std::uint16_t>(weapon, bindings.weapon_identifier)))
        return;
    if (!(frame.flags & 0x100000U) &&
        !((frame.flags & 0x400U) && (command.movement.buttons & 1U)) &&
        !((frame.flags & 2U) && (command.movement.buttons & 0x800U) &&
          (frame.weapon_id_d8 == 64 || frame.weapon_kind_124 == 0)))
        return;
    publish_command_animation_angles(bindings.pending_angles, frame.command_number_ac,
                                     command.angles);
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: command_phase8.cpp
// ============================================================================

namespace skeet::rage {
void capture_rage_command_frame(PreambleState& state, const client::CommandFrame& frame) noexcept {
    if (state.frame_count != 16)
        state.frame_count = std::bit_cast<std::int32_t>(std::uint32_t(state.frame_count) + 1U);
    state.frame_first = (state.frame_first - 1U) & 15U;
    std::memcpy(&state.frames[state.frame_first], &frame, sizeof frame);
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: command_source.cpp
// ============================================================================

namespace skeet::rage {
PreambleState& CommandEndpoint::state() const noexcept {
    if (auto* value = std::get_if<CommandOwner*>(&source_))
        return (*value)->state;
    return std::get<CommandSource*>(source_)->state();
}
AcquisitionStorage& CommandEndpoint::acquisition() const noexcept {
    if (auto* value = std::get_if<CommandOwner*>(&source_))
        return (*value)->acquisition;
    return std::get<CommandSource*>(source_)->acquisition();
}
std::int32_t& CommandEndpoint::shot_index() const noexcept {
    if (auto* value = std::get_if<CommandOwner*>(&source_))
        return (*value)->post_firing.shot_index;
    return std::get<CommandSource*>(source_)->shot_index();
}
esp::HitboxHighlightState& CommandEndpoint::action_state() const noexcept {
    if (auto* value = std::get_if<CommandOwner*>(&source_))
        return (*value)->tail.action_state;
    return std::get<CommandSource*>(source_)->action_state();
}
std::array<animation::AnimationAngleSample, 2>& CommandEndpoint::pending_angles() const noexcept {
    if (auto* value = std::get_if<CommandOwner*>(&source_))
        return (*value)->phase6.pending_angles;
    return std::get<CommandSource*>(source_)->pending_angles();
}
PredictionMaskOwner* const& CommandEndpoint::prediction_masks() const noexcept {
    if (auto* value = std::get_if<CommandOwner*>(&source_))
        return (*value)->post_firing.movement.prediction.owner;
    return std::get<CommandSource*>(source_)->prediction_masks();
}
void CommandEndpoint::require_factory(const AimbotSettings& settings, const GlobalSettings& global,
                                      const esp::AimbotVisualizationConfig& visualization,
                                      const esp::HitboxHighlightState& highlight) const {
    if (auto* value = std::get_if<CommandOwner*>(&source_)) {
        const auto& c = **value;
        if (&c.preamble.weapon_settings != &settings || &c.preamble.settings != &global ||
            &c.tail.action_state != &highlight || &c.tail.action.visualization != &visualization ||
            &c.single_target.preparation.state != &c.state ||
            &c.multiple_targets.preparation.state != &c.state)
            throw std::invalid_argument(
                "RAGE constructor requires canonical command settings and state");
    } else
        std::get<CommandSource*>(source_)->require_factory(settings, global, visualization,
                                                           highlight);
}
void CommandEndpoint::phase2(void* player, void* weapon, client::Command& command,
                             std::array<float, 3>& angles, std::uint8_t& send) {
    if (auto* value = std::get_if<CommandOwner*>(&source_))
        process_rage_command_phase2(**value, player, weapon, command, angles, send);
    else
        std::get<CommandSource*>(source_)->phase2(player, weapon, command, angles, send);
}
void CommandEndpoint::process(client::CommandFrame& frame, void* player, client::Command& command,
                              std::uint8_t& send) {
    if (auto* value = std::get_if<CommandOwner*>(&source_))
        process_rage_command(**value, frame, player, command, send);
    else
        std::get<CommandSource*>(source_)->process(frame, player, command, send);
}
void CommandEndpoint::phase6(const client::CommandFrame& frame, const client::Command& command) {
    if (auto* value = std::get_if<CommandOwner*>(&source_))
        process_rage_phase6(**value, frame, command);
    else
        std::get<CommandSource*>(source_)->phase6(frame, command);
}
void CommandEndpoint::capture(const client::CommandFrame& frame) noexcept {
    if (auto* value = std::get_if<CommandOwner*>(&source_))
        capture_rage_command_frame(**value, frame);
    else
        std::get<CommandSource*>(source_)->capture(frame);
}
} //namespace skeet::rage

// ============================================================================
// Subcomponent: entity_update.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
std::uint32_t level_hash(const char* name) noexcept { //43359C70
    std::uint32_t result = 0x811C9DC5;
    for (auto* current = reinterpret_cast<const unsigned char*>(name); *current; ++current)
        result = (result ^ *current) * 0x1000193u;
    return result ^ 0x92CA5DF1u;
}
}
void update_rage_entities(const EntityUpdateBindings& bindings, void* local,
                          const client::EntityPredictionEntities& incoming) {
    if (!local)
        return;
    const auto* level = engine::invoke<const char*>(bindings.engine_client, 53);
    if (!level || level_hash(level) != 0xDC515205u)
        return;
    const auto maximum = *bindings.maximum_clients;
    for (std::int32_t index = 0; index < incoming.count; ++index) {
        const auto entity_index = incoming.entries[std::size_t(index)].entity_index;
        if (entity_index <= maximum || entity_index > 0x800 ||
            (std::uint32_t(entity_index) & 0xFFFF0000u))
            continue;
        auto* entity = engine::invoke<void*>(bindings.entity_list, 3, entity_index);
        if (!entity)
            continue;
        auto* actual_class = engine::invoke<void*>(static_cast<std::byte*>(entity) + 8, 2);
        if (actual_class != bindings.required_class)
            continue;
        if (field<std::uint8_t>(entity, bindings.offsets.movement_type) != 7)
            continue;
        auto* collision = engine::invoke<void*>(entity, 3);
        if (engine::invoke<std::int32_t>(collision, 11) != 1)
            continue;
        collision = engine::invoke<void*>(entity, 3);
        if (!(engine::invoke<std::uint32_t>(collision, 12) & 0x800u))
            continue;
        if (field<std::int32_t>(entity, bindings.offsets.owner_handle) != -1)
            continue;
        const auto model_index = field<std::int16_t>(entity, bindings.offsets.model_index);
        if (model_index <= 1)
            continue;
        auto* model = engine::invoke<void*>(bindings.model_info, 1, std::int32_t(model_index));
        if (!model)
            continue;
        const auto* name = engine::invoke<const char*>(bindings.model_info, 3, model);
        if (!name || *name != '*')
            continue;
        const std::uint32_t invalid = UINT32_MAX;
        std::memcpy(static_cast<std::byte*>(entity) + bindings.offsets.health, &invalid,
                    sizeof invalid);
    }
}
}

// ============================================================================
// Subcomponent: final_target_context.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
void* pointer(BallisticEntity entity) noexcept {
    return reinterpret_cast<void*>(std::uintptr_t(entity));
}

// Source owners43366346/433304C1, with the original filter prefix. Methods1..4
// are the genuine43475B90 engine methods. Each sample owns its own prefix;
// engine source/last-hit setters must never mutate a shared worker filter.
struct FinalTraceFilter {
    void** vtable;
    std::array<std::uint32_t, 5> fields_04{};
    std::uint32_t mask, exit_mask;
    float minimum_damage = 1.0f;
    std::int32_t team = 0;
    std::uint8_t teammates_are_enemies = 0;
    std::array<std::byte, 3> padding_29{};
    const FinalTargetContextBindings* bindings;
    std::array<void*, 5> methods;
    FinalTraceFilter(const FinalTargetContextBindings& b, bool teammate, BallisticEntity preferred,
                     std::int32_t own_team = 0, bool enemies = false)
        : vtable(nullptr), mask(0x4600400BU), exit_mask(0x4600400BU), team(own_team),
          teammates_are_enemies(enemies), bindings(&b) {
        fields_04[4] = preferred;
        const auto table = b.scan.trace_filter_vtable;
        methods = {teammate ? reinterpret_cast<void*>(teammate_hit)
                            : reinterpret_cast<void*>(regular_hit),
                   table[1], table[2], table[3], table[4]};
        vtable = methods.data();
    }
    BallisticFilter binding() noexcept {
        return {this, fields_04[4], mask, exit_mask, minimum_damage};
    }
    bool delegate(BallisticEntity entity, std::uint32_t contents) {
        const auto method =
            reinterpret_cast<engine::MemberFunction<bool, BallisticEntity, std::uint32_t>>(
                bindings->scan.trace_filter_vtable[0]);
        return method(this, entity, contents);
    }
#if defined(_M_IX86) || defined(__i386__)
    static bool __thiscall regular_hit(void* receiver, BallisticEntity entity,
                                       std::uint32_t contents) {
#else
    static bool regular_hit(void* receiver, BallisticEntity entity, std::uint32_t contents) {
#endif
        auto& self = *static_cast<FinalTraceFilter*>(receiver);
        const auto& entities = self.bindings->scan.trace_entities;
        if (entity && entity != self.fields_04[4]) {
            auto current = entities.base_entity(entity);
            while (current) {
                if (entities.collision_group(current) == 11 || entities.is_player(current))
                    return false;
                const auto handle = entities.parent_handle(current);
                if (handle == -1)
                    break;
                current = entities.by_handle(handle);
            }
        }
        return self.delegate(entity, contents);
    }
#if defined(_M_IX86) || defined(__i386__)
    static bool __thiscall teammate_hit(void* receiver, BallisticEntity entity,
                                        std::uint32_t contents){
#else
    static bool teammate_hit(void* receiver, BallisticEntity entity, std::uint32_t contents) {
#endif
        auto& self = *static_cast<FinalTraceFilter*>(receiver);
    const auto& scan = self.bindings->scan;
    const auto& entities = scan.trace_entities;
    if (entity) {
        auto current = entities.base_entity(entity);
        while (current) {
            if (entities.is_player(current)) {
                if (!self.fields_04[4])
                    return false;
                if (current != self.fields_04[4] &&
                    (self.teammates_are_enemies ||
                     read<std::int32_t>(pointer(current), scan.targeting.offsets.team) !=
                         self.team))
                    return false;
                break;
            }
            if (engine::invoke<bool>(pointer(current), 166) ||
                engine::invoke<bool>(pointer(current), 159) ||
                entities.collision_group(current) == 11)
                return false;
            const auto handle = entities.parent_handle(current);
            if (handle == -1)
                break;
            current = entities.by_handle(handle);
        }
    }
    return self.delegate(entity, contents);
}
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(offsetof(FinalTraceFilter, mask) == 0x18);
static_assert(offsetof(FinalTraceFilter, team) == 0x24);
static_assert(offsetof(FinalTraceFilter, teammates_are_enemies) == 0x28);
#endif
}

namespace final_target_detail {
void bind_teammate_policy(FinalHitchanceContext& context, const Producer& producer) {
    context.check_teammates = final_teammate_precheck_required(
        context.context_flags, producer.bindings.scan.teammate_convars);
    // That source policy has already read the terminal teammates ConVar only
    // when the native damage policy reaches it. A true result means zero.
    context.teammates_are_enemies = false;
}
bool teammate_hit(const Producer& producer, BallisticEntity entity) {
    return engine::invoke<bool>(pointer(entity), 158) &&
           read<std::int32_t>(pointer(entity), producer.bindings.scan.targeting.offsets.team) ==
               std::bit_cast<std::int32_t>(producer.frame.field_1d0);
}
bool select_studio_hitbox(FinalHitchanceContext& context, Producer& producer,
                          const TargetCandidate& candidate, const lagcomp::LagRecord& record) {
    const auto& offsets = producer.bindings.scan.model_offsets;
    const auto wrapper = read<const void*>(context.target_player, offsets.studio_header);
    const void* set = nullptr;
    if (wrapper) {
        const auto header = read<const void*>(wrapper, 0);
        if (header) {
            const auto index = read<std::int32_t>(context.target_player, offsets.hitbox_set);
            set = static_cast<const std::byte*>(header) + read<std::int32_t>(header, 0xB0) +
                  std::ptrdiff_t(index) * 12;
            if (!read<std::uint32_t>(set, 4))
                set = nullptr;
        }
    }
    // Native4341BB8D computes this descriptor even on regular points. A valid
    // selected studio set is its contract; a missing set is not substituted.
    const auto descriptor = static_cast<const std::byte*>(set) + read<std::int32_t>(set, 8) +
                            std::ptrdiff_t(candidate.primary.hitbox) * 0x44;
    producer.selected_descriptor = descriptor;
    return (candidate.primary.flags & 1U) && (record.header.flags & 0x80U) && descriptor &&
           read<float>(descriptor, 0x30) > 0.0f;
}
std::vector<RageScanHitbox> hitbox_geometry(const FinalHitchanceContext& context,
                                            const Producer& producer,
                                            const HitboxMatrices& matrices) {
    const auto model =
        scan_detail::model(context.target_player, producer.bindings.scan.model_offsets);
    return prepare_rage_scan_hitboxes(model.hitboxes, matrices, context.trigonometry);
}
void apply_record(const FinalHitchanceContext& context, const Producer& producer,
                  lagcomp::EntityRecordScope& scope, const lagcomp::LagRecord& record) {
    const auto& model = producer.bindings.scan.model_offsets;
    scope.apply_live_model(record, {model.studio_header, model.hitbox_set},
                           context.current_bone_counter);
}
void publish_failure(FinalHitchanceFailure& state, const Producer& producer,
                     std::uint32_t entity_index) {
    if (state.context_flags & 8U)
        return;
    if (requests_body_aim(state, &producer.frame)) {
        auto& native = producer.bindings.scan.players.get(entity_index)->native;
        const std::int32_t mode = 3;
        std::memcpy(reinterpret_cast<std::byte*>(&native) + 0xF8, &mode, sizeof mode);
    }
    state.context_flags |= 0x40U;
}
std::int32_t trace_direction(const FinalTargetContextBindings& bindings, BallisticEngine& engine,
                             const FinalHitchanceSample& input, const RageVector& direction,
                             BallisticQuery& query) {
    FinalTraceFilter filter(bindings, false, input.target);
    return trace_target_direction(engine, input.source, input.origin, direction, filter.binding(),
                                  query);
}
std::int32_t trace_projected(const FinalTargetContextBindings& bindings, BallisticEngine& engine,
                             const FinalHitchanceSample& input, const RageVector& point,
                             BallisticQuery& query) {
    anti_aim::TargetTraceFilter filter(bindings.scan.trace_filter_vtable,
                                       bindings.scan.trace_entities, 0x600400BU, 0x600400BU, 1.0f);
    return trace_ballistic_bullet(engine, input.source, input.origin, point, filter.binding(),
                                  query);
}
std::int32_t trace_teammate(const Producer& producer, FinalHitchanceContext& context,
                            const RageVector& direction, BallisticQuery& query) {
    FinalTraceFilter filter(producer.bindings, true, 0, context.team,
                            context.teammates_are_enemies);
    return trace_target_direction(context.ballistics, context.source, context.origin, direction,
                                  filter.binding(), query);
}
}

bool evaluate_final_target(client::CommandFrame& frame, PreambleState& state,
                           const TargetCandidate& candidate,
                           const AcquisitionEvaluation& evaluation, std::uint32_t& context_flags,
                           const FinalTargetContextBindings& bindings, float& chance) {
    if (frame.flags & 0x4000U) {
        chance = 1.0f;
        return true;
    }
    auto& scan = bindings.scan;
    auto* target = engine::invoke<void*>(scan.targeting.entity_list, 3, candidate.entity_index);
    if (!target)
        return false;
    final_target_detail::Producer producer{bindings, frame, state, {}, nullptr};
    FinalHitchanceContext context{
        .ballistics = scan.ballistics,
        .entity_runtime = scan.preparation.entity,
        .target_player = target,
        .source = scan.source,
        .target = BallisticEntity(reinterpret_cast<std::uintptr_t>(target)),
        .weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address)),
        .weapon_item_offset = scan.weapon_item_offset,
        .allow_penetration = scan.allow_penetration,
        .weapon_flags = frame.flags,
        .context_flags = context_flags,
        .previous_targets = {},
        .origin = frame.origin_dc,
        .velocity = {frame.velocity_150[0], frame.velocity_150[1]},
        .baseline = frame.inaccuracy_c4,
        .current = std::bit_cast<float>(frame.field_108),
        .spread = frame.spread_d0,
        .inaccuracy = frame.inaccuracy_c4,
        .check_teammates = false,
        .teammates_are_enemies = false,
        .team = std::bit_cast<std::int32_t>(frame.field_1d0),
        .target_aim_mode = nullptr,
        .local_hitboxes = {},
        .hitbox_bones = {},
        .current_bone_counter = bindings.current_bone_counter,
        .saved_bones = producer.saved_bones,
        .trigonometry = scan.trigonometry,
        .box_clipper = scan.box_clipper,
        .direction_filter = {},
        .projected_filter = {},
        .teammate_filter = {},
        .samples = state.spread.primary,
        .settings_menu = bindings.settings_menu,
        .jobs = bindings.jobs,
    };
    return evaluate_target_hitchance(context, candidate, evaluation.record,
                                     std::bit_cast<std::int32_t>(evaluation.settings.words[5]),
                                     std::bit_cast<float>(evaluation.settings.words[6]), chance,
                                     producer);
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: firing_angles.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
/* [stripped helper] */
MovementAngles angles(const client::Command& command) noexcept {
    return {command.angles[0], command.angles[1], command.angles[2]};
}
void angles(client::Command& command, const MovementAngles& value) noexcept {
    command.angles = {value.pitch, value.yaw, value.roll};
}
const MovementAngles* reference_angles(const FiringState& state, const client::Command& command,
                                       const FiringPostAngleBindings& bindings,
                                       MovementAngles& saved) {
    const auto player = bindings.player;
    const auto& offsets = bindings.offsets;
    if (!(field<std::uint32_t>(player, offsets.flags) & 8U) &&
        field<std::uint8_t>(player, offsets.first_byte) ==
            field<std::uint8_t>(player, offsets.second_byte)) {
        const auto previous = engine::invoke<const client::Command*>(
            bindings.input, 8, 0, std::bit_cast<std::int32_t>(std::uint32_t(command.number) - 1U));
        if (previous) {
            saved = angles(*previous);
            return &saved;
        }
    }
    return &state.reference_angles;
}
const void* studio_wrapper(const FiringPostAngleBindings& bindings) noexcept {
    const auto wrapper = field<const void*>(bindings.player, bindings.offsets.studio_header);
    return wrapper && field<const void*>(wrapper, 0) ? wrapper : nullptr;
}
struct SemanticBoneBuild {
    const FiringPostAngleBindings& bindings;
    bool build(void*, const animation::BoneRequest& request, std::span<Matrix3x4> matrices) const {
        return animation::build_bone_matrices(
            bindings.bone_state.decoded(), bindings.bone_model.decoded(), request, matrices,
            bindings.bone_engine.decoded(), bindings.trigonometry);
    }
};
struct LiveBoneBuild {
    const animation::BoneRuntime& runtime;
    bool build(void* player, const animation::BoneRequest& request,
               std::span<Matrix3x4> matrices) const {
        std::int32_t count = 0;
        return runtime.build_bone_matrices(player, request, matrices, &count);
    }
};
}
bool limit_firing_angle_step(MovementAngles& aim, const MovementAngles& reference,
                             float maximum_step) noexcept {
    aim.roll = 0;
    auto pitch = normalize_movement_angle(aim.pitch - reference.pitch);
    auto yaw = normalize_movement_angle(aim.yaw - reference.yaw);
    const auto distance = std::sqrt(pitch * pitch + yaw * yaw);
    if (maximum_step > distance)
        return false;
    const auto scale = maximum_step / distance;
    pitch *= scale;
    yaw *= scale;
    aim.pitch = pitch + reference.pitch;
    aim.yaw = yaw + reference.yaw;
    return true;
}
bool firing_animation_ready(const void* animation, std::uint8_t guard) noexcept {
    return field<std::uint8_t>(animation, 0x109) != 0 || field<float>(animation, 0xa4) != 0.0f ||
           !(guard & 1U);
}
float correct_firing_eye_height(const Matrix3x4& bone, float height) noexcept {
    const auto ceiling = bone.rows[2][3] + 1.7f;
    if (!(height > ceiling))
        return height;
    const auto raw = (std::fabs(height - ceiling) - 4.0f) / 6.0f;
    auto t = raw > 0.0f ? raw : 0.0f;
    t = t < 1.0f ? t : 1.0f;
    const auto square = t * t;
    const auto weight = (square * 3.0f - (square + square) * t) + 0.0f;
    return (ceiling - height) * weight + height;
}
template <class BoneBuild>
bool evaluate_post_angles(FiringState& state, const TargetCandidate& target,
                          FiringBranchState& branch, client::Command& command,
                          const FiringPostAngleBindings& bindings, const BoneBuild& bones) {
    const auto limit = [&] {
        MovementAngles saved;
        const auto reference = reference_angles(state, command, bindings, saved);
        auto aim = angles(command);
        const bool limited = limit_firing_angle_step(aim, *reference, state.angle_step);
        angles(command, aim);
        return limited;
    };
    if ((state.flags & 0x40U) && limit()) {
        if (state.weapon_id != 64 || state.special_threshold >= bindings.prediction_time)
            return false;
    }
    if (!bindings.enabled)
        return false;
    if (bindings.scheduler_flags & 9U)
        return true;
    branch.field_22 = true;
    if (!bindings.send_mode || (state.flags & 0x400000U))
        return true;
    const auto player = bindings.player;
    const auto& offsets = bindings.offsets;
    const auto animation_data = field<const void*>(player, offsets.animation_state);
    if (!animation_data)
        return true;
    if (!firing_animation_ready(animation_data,
                                field<std::uint8_t>(player, offsets.animation_guard)))
        return true;
    // Both original433FD30D calls are simple decoded-field reads.
    if (!studio_wrapper(bindings))
        return true;
    const auto wrapper = studio_wrapper(bindings);
    if (!wrapper)
        return true;
    const auto header = field<const std::byte*>(wrapper, 0);
    const auto set_index = field<std::int32_t>(player, offsets.hitbox_set);
    const auto set = header + field<std::int32_t>(header, 0xb0) + std::ptrdiff_t(set_index) * 12;
    if (!field<std::int32_t>(set, 4))
        return true;
    const auto boxes = set + field<std::int32_t>(set, 8);
    const auto bone_index = field<std::int32_t>(boxes, 0);
    if (bone_index == -1)
        return true;
    auto pitch = command.angles[0];
    if (!(state.flags & 0x10U))
        pitch -= state.recoil[0];
    pitch = normalize_movement_angle(pitch) + state.field_a8;
    if (pitch > 180.0f)
        pitch -= 360.0f;
    pitch = _mm_cvtss_f32(
        _mm_min_ss(_mm_max_ss(_mm_set_ss(pitch), _mm_set_ss(-90.0f)), _mm_set_ss(90.0f)));
    auto* pose = static_cast<std::byte*>(player) + offsets.pose_parameters + 0x30;
    const auto saved_pose = field<float>(pose, 0);
    if (state.origin[2] == state.bone_origin[2] + state.field_164 &&
        pitch <= saved_pose * 180.0f - 90.0f)
        return true;
    float normalized = pitch;
    bindings.convert_pose_parameter(wrapper, 12, pitch, normalized);
    write(pose, 0, normalized);
    std::array<Matrix3x4, animation::maximum_bones> matrices;
    const animation::BoneRequest request{{0, field<float>(animation_data, 0x80), 0},
                                         state.bone_origin,
                                         bindings.prediction_time,
                                         0x300U,
                                         bone_index,
                                         0,
                                         true};
    const auto built = bones.build(player, request, matrices);
    if (built) {
        const auto height = correct_firing_eye_height(matrices[std::size_t(bone_index)],
                                                      state.bone_origin[2] + state.field_164);
        if (height != state.origin[2]) {
            state.field_84 = height - state.bone_origin[2];
            state.origin[2] = height;
            branch.field_33 = bindings.recoil_option;
            const auto aim =
                target_aim_angles(target.primary.position, state.origin, state.recoil,
                                  bindings.compensate_recoil, state.flags, bindings.angle_math);
            angles(command, aim);
            branch.field_22 = true;
            if ((state.flags & 0x40U) && limit() &&
                (state.weapon_id != 64 || state.special_threshold >= bindings.prediction_time))
                branch.field_22 = false;
        }
    }
    write(pose, 0, saved_pose);
    return branch.field_22;
}
bool evaluate_firing_post_angles(FiringState& state, const TargetCandidate& target,
                                 FiringBranchState& branch, client::Command& command,
                                 const FiringPostAngleBindings& bindings) {
    return evaluate_post_angles(state, target, branch, command, bindings,
                                SemanticBoneBuild{bindings});
}
bool evaluate_firing_post_angles(FiringState& state, const TargetCandidate& target,
                                 FiringBranchState& branch, client::Command& command,
                                 const FiringPostAngleBindings& bindings,
                                 const animation::BoneRuntime& runtime) {
    return evaluate_post_angles(state, target, branch, command, bindings, LiveBoneBuild{runtime});
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: firing_readiness.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
/* [dedup convar_nonzero] */
/* [removed duplicate truncate] */
std::int32_t subtract_wrapped(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) - std::uint32_t(b));
}
}
std::int32_t firing_mode(bool mode_enabled, bool send_mode) noexcept {
    return mode_enabled ? (send_mode ? 2 : 1) : 0;
}
FiringReadiness evaluate_firing_readiness(FiringState& state, const FiringTargetData& target,
                                          FiringBranchState& branch, FiringSharedState& shared,
                                          client::Command& command,
                                          const FiringReadinessBindings& bindings) {
    const auto& candidate = target.candidate;
    const auto near_tick = [&](std::int32_t tick) {
        if (target.field_b4 <= 0)
            return true;
        const auto time = float(tick) * bindings.tick_interval;
        const auto cap = float(bindings.latency_milliseconds) * 0.001f;
        const auto corrected = minimum_number(state.field_198 + bindings.current_time, cap);
        const auto record_time = float(target.field_b4) * bindings.tick_interval;
        return std::fabs(corrected - (time - record_time)) <= 0.2f;
    };
    const auto remember = [&] {
        shared.selected_marker = state.marker;
        shared.selected_point = candidate.primary.position;
    };
    bool newly_ready = false;
    if (!branch.field_22) {
        if (!branch.timing_ready && !branch.field_83) {
            if (!branch.field_33 || !near_tick(state.current_tick)) {
                remember();
                return FiringReadiness::retained_target;
            }
            state.flags |= 0x400U;
            state.previous_tick = state.current_tick;
            branch.alternate_path = true;
            branch.timing_ready = true;
            newly_ready = true;
            bindings.prediction_time = float(state.current_tick) * bindings.tick_interval;
        } else if (candidate.primary.flags & 0x30U) {
            auto& player = *bindings.players.get(candidate.entity_index);
            const auto receive =
                player.history.count
                    ? player.history.records[player.history.first & 31U]->header.receive_tick
                    : field<std::int32_t>(&player.native, 0x168);
            const auto time = engine::invoke<float>(bindings.engine_client, 14);
            const auto age =
                truncate(float(static_cast<long double>(time) /
                                   static_cast<long double>(bindings.receive_interval) +
                               static_cast<long double>(bindings.correction_time)));
            if (subtract_wrapped(age, receive) > 2) {
                remember();
                return FiringReadiness::retained_target;
            }
        }
    }
    // The first successful timing transition jumps directly to43354FB5.
    if (!newly_ready && state.weapon_id != 64 && state.field_1d4 != 0 &&
        convar_nonzero(bindings.timing_convar) && state.previous_tick < state.current_tick &&
        target.field_b4 > 0 && !near_tick(state.previous_tick) && near_tick(state.current_tick)) {
        state.previous_tick = state.current_tick;
        branch.alternate_path = true;
        bindings.prediction_time = float(state.current_tick) * bindings.tick_interval;
    }
    branch.field_5b = false;
    branch.field_33 = false;
    if (!(state.flags & 0x40000000U) && !branch.alternate_path) {
        const auto mode = firing_mode(bindings.mode_enabled, bindings.send_mode);
        branch.field_5b =
            !branch.field_22 && bindings.enabled && mode == 1 && state.field_15c >= state.field_fc;
        if (branch.field_83)
            branch.field_33 = true;
        else if (candidate.primary.damage < candidate.health && !branch.field_22 &&
                 bindings.enabled && bindings.delay_option != 0 && mode == 2 &&
                 state.field_15c == 0 && !(bindings.scheduler_flags & 0x23U) &&
                 !(state.flags & 0x40U) && !(state.flags & 0x102000U) && state.field_1d4 != 2 &&
                 state.field_fc > 0) {
            const bool revolver_ok = state.weapon_id != 64 || !(state.flags & 0x8000U) ||
                                     (bindings.revolver_gate > bindings.prediction_time &&
                                      (command.movement.buttons & 0x800U));
            const bool marker_ok = bindings.committed_marker <= bindings.command_marker ||
                                   bindings.committed_marker >= state.marker;
            branch.field_33 =
                revolver_ok && marker_ok &&
                std::uint32_t(shared.last_delay_marker) + 1U != std::uint32_t(state.marker);
        }
    }
    auto& player = *bindings.players.get(candidate.entity_index);
    if (branch.field_5b || branch.field_33) {
        if (!branch.field_5b) {
            std::memcpy(reinterpret_cast<std::byte*>(&player.native) + 0x194, &target.field_13c, 4);
            *state.send_packet = 0;
            if (!branch.field_22)
                command.movement.buttons &= ~1U;
        }
        remember();
        shared.last_delay_marker = state.marker;
        return FiringReadiness::retained_target;
    }
    return branch.timing_ready || branch.field_22 ? FiringReadiness::angles
                                                  : FiringReadiness::stopped;
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: group_initializers.cpp
// ============================================================================

namespace skeet::rage {
namespace {
struct BlobInitializer {
    std::uint32_t callback, finalizer;
};
constexpr std::array<BlobInitializer, 12> blobs{{{0x43492965, 0x4349298E},
                                                 {0x434929A8, 0x434929D1},
                                                 {0x434929EB, 0x43492A14},
                                                 {0x43492A2E, 0x43492A57},
                                                 {0x43492A71, 0x43492A9A},
                                                 {0x43492AB4, 0x43492ADD},
                                                 {0x43492AF7, 0x43492B20},
                                                 {0x43492B3A, 0x43492B63},
                                                 {0x43492B7D, 0x43492BA6},
                                                 {0x43492BC0, 0x43492BE9},
                                                 {0x43492C03, 0x43492C2C},
                                                 {0x43492C46, 0x43492C6F}}};
constexpr std::array<std::uint32_t, 12> enabled{0x43492C89, 0x43492CAB, 0x43492CCD, 0x43492CEF,
                                                0x43492D11, 0x43492D33, 0x43492D55, 0x43492D77,
                                                0x43492D99, 0x43492DBB, 0x43492DDD, 0x43492DFF};
GroupInitializerBindings* active_groups = nullptr;

template <std::size_t Index> void finalize_blob() {
    active_groups->groups.finalize_blob(Index);
}
template <std::size_t... Index> constexpr auto finalizer_entries(std::index_sequence<Index...>) {
    return std::array<void (*)(), sizeof...(Index)>{finalize_blob<Index>...};
}
constexpr auto finalizers = finalizer_entries(std::make_index_sequence<12>{});
template <std::size_t Index> void initialize_blob(void* raw) {
    auto& source = *static_cast<GroupInitializerBindings*>(raw);
    source.groups.initialize_blob(Index, source.configuration.descriptors);
    (void)source.callbacks.append(source.source_finalizer(blobs[Index].finalizer));
}
template <std::size_t Index> void initialize_enabled(void* raw) {
    auto& source = *static_cast<GroupInitializerBindings*>(raw);
    source.groups.initialize_enabled(Index, source.configuration.descriptors);
}
template <std::size_t... Index>
void bind(client::NativeStartupSources& sources, GroupInitializerBindings& context,
          std::index_sequence<Index...>) {
    (sources.bind_initializer({blobs[Index].callback, &context, initialize_blob<Index>}), ...);
    (sources.bind_initializer({enabled[Index], &context, initialize_enabled<Index>}), ...);
}
}
void (*GroupInitializerBindings::source_finalizer(std::uint32_t address))() {
    for (std::size_t index = 0; index < blobs.size(); ++index) {
        if (blobs[index].finalizer != address)
            continue;
        if (active_groups && active_groups != this)
            throw std::logic_error("RAGE group exit callbacks require one stable source binding");
        active_groups = this;
        return finalizers[index];
    }
    throw std::out_of_range("Finalizer is not a RAGE weapon-group blob callback");
}
void bind_native_group_initializers(client::NativeStartupSources& sources,
                                    GroupInitializerBindings& context) {
    if (&context.groups.image() != &sources.data())
        throw std::invalid_argument(
            "RAGE groups require the startup scheduler's canonical data owner");
    if (active_groups && active_groups != &context)
        throw std::logic_error("RAGE group exit callbacks require one stable source binding");
    active_groups = &context;
    bind(sources, context, std::make_index_sequence<12>{});
}
}

// ============================================================================
// Subcomponent: group_storage.cpp
// ============================================================================

namespace skeet::rage {
namespace {
// Both orders are native pointer-table orders, established independently of
// their scattered global addresses and ordinary CRT callback order.
constexpr std::array<std::uint32_t, 12> blob_addresses{
    //43472B98
    0x43478F58, 0x43479018, 0x43478EF8, 0x43478FF8, 0x43478F78, 0x43478F38,
    0x43479058, 0x43479038, 0x43478F98, 0x43478FD8, 0x43478F18, 0x43478FB8};
constexpr std::array<std::uint32_t, 12> blob_hashes{0x210D3A72, 0x9599E10A, 0xADB1D494, 0xBDCCCC63,
                                                    0x2BB92C57, 0x935CA480, 0x2C3F27C1, 0x0D4168C1,
                                                    0x056F4E4F, 0x45F8FEC5, 0x85D3154A, 0x6B8A7FCE};
NativeBlobBytes bytes(engine::NativeDataImage& image, engine::JobMemory memory, std::size_t index) {
    return {image.native_cell<NativeBlobCells>(blob_addresses[index] + 0x10), memory,
            image.native_cell<std::byte>(0x434759AC)};
}
}
GroupStorage::GroupStorage(engine::PreservedInitialization& initialization,
                           engine::JobMemory memory)
    : GroupStorage(initialization.image_owner(), memory) {}
GroupStorage::GroupStorage(std::shared_ptr<engine::NativeDataImage> image, engine::JobMemory memory)
    : image_(image), native_{bytes(engine::require_native_data_image(image_), memory, 0),
                             bytes(*image_, memory, 1),
                             bytes(*image_, memory, 2),
                             bytes(*image_, memory, 3),
                             bytes(*image_, memory, 4),
                             bytes(*image_, memory, 5),
                             bytes(*image_, memory, 6),
                             bytes(*image_, memory, 7),
                             bytes(*image_, memory, 8),
                             bytes(*image_, memory, 9),
                             bytes(*image_, memory, 10),
                             bytes(*image_, memory, 11)},
      enabled_pointers_(enabled_.descriptors()) {
    for (std::size_t index = 0; index < blobs_.size(); ++index) {
        blobs_[index].value.bind_native(native_[index]);
        blob_pointers_[index] = &blobs_[index];
    }
}
void GroupStorage::initialize_blob(std::size_t index, ConfigLink& registry) {
    // The semantic view borrows native+10's pointer triple. Register its real
    // adjacent header, so serialization/menu lookup and terminal cleanup all
    // refer to the same object instead of treating ConfigBlob as raw storage.
    ConfigRegistries::for_descriptors(registry).bind_blob(
        image_->native_cell<ConfigDescriptor>(blob_addresses[index]), blobs_[index]);
    initialize_blob_config(registry, blobs_[index], blob_hashes[index]);
}
void GroupStorage::initialize_enabled(std::size_t index, ConfigLink& registry) noexcept {
    auto& descriptor = enabled_.values[index];
    append_config_link(registry, descriptor); //metadata already exists in the image
    descriptor.value = index == 0 ? 1 : 0;
}
void GroupStorage::finalize_blob(std::size_t index) {
    native_[index].terminal_release();
}
}

// ============================================================================
// Subcomponent: initialization.cpp
// ============================================================================

namespace skeet::rage {
namespace {
void initialize_prefix(const RageResetBindings& bindings) noexcept {
    bindings.field_174 = -1;
    bindings.highlight.target_handle = -1;
    bindings.state.spread.generation = UINT32_MAX;
    bindings.shot_index = 0;
    bindings.highlight.entries = {{{0, 0}, {3, 0}, {9, 0}, {10, 0}}};
    bindings.state.evaluation_count = 0;
}
void initialize_storage(const RageResetBindings& bindings) noexcept {
    for (std::size_t index = 0; index < bindings.acquisition.evaluations.size(); ++index) {
        auto& evaluation = bindings.acquisition.evaluations[index];
        //43387448 allocates this separate80-record block with HEAP_ZERO_MEMORY.
        // Keep its native payload and the existing typed candidate view equal.
        std::memset(&evaluation.native, 0, sizeof(evaluation.native));
        evaluation.record.header.field_00 = 0;
        evaluation.extrapolated = false;
        evaluation.extrapolation_ticks = 0;
        evaluation.target_handle = UINT32_MAX;
        restore_evaluation_state(evaluation.native, bindings.acquisition.candidates[index],
                                 evaluation);
    }
    bindings.state.frame_first = 0;
    bindings.acquisition.queued.count = 0;
    bindings.acquisition.queued.inline_storage = true;
    bindings.state.frame_count = 0;
    for (auto& frame : bindings.state.frames)
        client::initialize_command_frame(frame);
}
}
void initialize_rage_state(const RageResetBindings& bindings) noexcept {
    initialize_prefix(bindings);
    initialize_storage(bindings);
}
void initialize_rage_state(const RageResetBindings& bindings, engine::JobMemory memory) {
    initialize_prefix(bindings);
    bindings.acquisition.evaluations.allocate(memory);
    initialize_storage(bindings);
}
}

// ============================================================================
// Subcomponent: multiple_targets.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
void clear_target_bit(std::array<std::uint32_t, 2>& words, std::uint32_t entity) noexcept {
    const auto index = entity & 63U;
    words[index >> 5] &= ~(1U << (index & 31U));
}
}
void acquire_multiple_targets(PreambleState& state, PreambleResult& preamble,
                              AcquisitionStorage& storage, const MultipleTargetBindings& bindings) {
    auto& frame = *preamble.frame;
    const auto& preparation = bindings.preparation;
    auto& players = bindings.records.players;
    const TargetView view{frame.origin_dc, field<RageVector>(&frame, 0x1A8),
                          preparation.weapon_settings.maximum_fov.value,
                          std::bit_cast<float>(preamble.scan_template[7])};
    for (std::int32_t entity = std::int32_t(preamble.first_entity);
         entity <= bindings.maximum_clients; ++entity) {
        auto* cached = players.get(std::uint32_t(entity));
        auto* target = engine::invoke<void*>(preparation.targeting.entity_list, 3, entity);
        if (!(state.flags & 9U))
            clear_target_bit(state.target_mask, std::uint32_t(entity));
        if (!target_is_eligible(preparation.targeting, target))
            continue;
        if (!target_passes_scan_filter(preparation.targeting, target, view))
            continue;
        AcquisitionTargetSettings settings{preamble.scan_template};
        const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(target) + 8, 10);
        prepare_acquisition_target_settings(frame, target, *cached, index,
                                            bindings.adjustments.records[index].adjustments,
                                            settings, preparation);
        if (!(state.flags & 9U))
            clear_target_bit(state.previous_targets, std::uint32_t(entity));
        auto body = bindings.body_probe;
        body.context_flags = state.flags;
        body.weapon_flags = state.weapon_flags;
        body.weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
        probe_body_aim(frame, settings, target, body);
        const TargetPredictionBindings prediction{body,
                                                  bindings.prediction.record_selection,
                                                  bindings.prediction.engine_trace,
                                                  bindings.prediction.local_maximum_speed,
                                                  bindings.prediction.scheduler_flags,
                                                  bindings.prediction.command};
        refine_acquisition_minimum_damage(frame, target, *cached, settings, prediction,
                                          bindings.records.bone_runtime);
        auto ticks = acquisition_target_ticks(frame, target, *cached, settings, preparation,
                                              bindings.record_candidates);
        prepare_and_submit_target_records(state, preamble, storage, target, *cached, ticks.values,
                                          settings, ticks.descriptor_flags, bindings.records);
    }
    finish_target_acquisition(state, storage, bindings.jobs, players, bindings.maximum_clients,
                              bindings.records.settings_menu);
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: owner.cpp
// ============================================================================

namespace skeet::rage {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define RAGE_MEMBER __thiscall
#define RAGE_HUD __fastcall
#else
#define RAGE_MEMBER
#define RAGE_HUD
#endif
using LifecycleEntry = void(RAGE_MEMBER*)(void*);
void RAGE_MEMBER destroy_entry(void*);
void RAGE_MEMBER reset_entry(void*);
void RAGE_MEMBER forward_reset_entry(void*);
void RAGE_MEMBER unchanged_entry(void*) {} //433CCE8E
const std::array<LifecycleEntry, 9> lifecycle_table{
    destroy_entry, unchanged_entry, unchanged_entry,     unchanged_entry, unchanged_entry,
    reset_entry,   unchanged_entry, forward_reset_entry, unchanged_entry};
void RAGE_HUD draw_entry(void*, ui::RenderQueue*, void*, std::int32_t*);
const std::array<hud::HudFrameEntry, 1> hud_table{draw_entry};
struct HudEndpoint : hud::HudFrameCallback {
    LifecycleOwner* owner;
};

void* allocate(engine::JobMemory memory, std::size_t size) { //43387448
    auto* raw = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!raw)
        return nullptr;
    const auto base = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (base + 15) & ~std::uintptr_t(15);
    const auto displacement = static_cast<std::uint32_t>(header - base);
    const auto allocation_size = static_cast<std::uint32_t>(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return reinterpret_cast<void*>(header + 16);
}
void free(engine::JobMemory memory, void* object) {
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void require_canonical_bindings(const OwnerBindings& b) {
    const auto& c = b.commands;
    if (&b.hud.config != &b.aimbot || &b.hud.visualization != &b.visualization)
        throw std::invalid_argument(
            "RAGE constructor bindings must share their canonical settings and state");
    c.require_factory(b.aimbot, b.hud.global, b.visualization, b.hud.highlight);
    if (c.acquisition().evaluations.data())
        throw std::invalid_argument("RAGE constructor requires its unallocated evaluation owner");
}
}

class LifecycleOwner {
public:
    const LifecycleEntry* vtable = nullptr;
    OwnerBindings bindings;
    LifecycleOwner** active;
    client::NativeCommandPhaseReceiver command_receiver;
    HudEndpoint hud_endpoint;
    client::NativeEntityObserverReceiver entity_receiver;
    std::int32_t field_174; //the complete owner's retained constructor/reset marker

    LifecycleOwner(OwnerBindings b, LifecycleOwner*& current) : bindings(b), active(&current) {
        bindings.lifecycle.append(this); //433676A0, complete owner+0
        bindings.command_owners.append(command_receiver, &bindings.commands); //434803CE, +8
        hud_endpoint.owner = this;
        hud::register_hud_frame(bindings.hud_owners, &hud_endpoint); //433D20B5, +10
        bindings.entity_owners.append_rage(entity_receiver, bindings.entities); //434803DE, +18
        vtable = lifecycle_table.data(); //434803ED
        bindings.command_owners.publish(command_receiver, client::CommandOwnerId::rage); //434803F4
        hud_endpoint.vtable = hud_table.data(); //434803FB
        bindings.entity_owners.publish_rage(entity_receiver); //43480402
        owner_detail::initialize_settings(bindings.configuration, bindings.aimbot,
                                          bindings.visualization);
        initialize_rage_state(reset_bindings(), bindings.lifecycle.memory);
    }
    RageResetBindings reset_bindings() noexcept {
        auto& command = bindings.commands;
        return {command.state(), command.acquisition(),  command.shot_index(),
                field_174,       command.action_state(), command.pending_angles()};
    }
};

namespace {
void destroy(LifecycleOwner* owner, LifecycleOwner*& active) { //433F19F5/43387719
    active = nullptr;
    if (!owner)
        return;
    *owner->active = nullptr;
    const auto memory = owner->bindings.lifecycle.memory;
    owner->bindings.commands.acquisition().evaluations.release();
    std::destroy_at(owner);
    free(memory, owner);
}
void RAGE_MEMBER destroy_entry(void* object) {
    auto* owner = static_cast<LifecycleOwner*>(object);
    destroy(owner, *owner->active);
}
void RAGE_MEMBER reset_entry(void* object) {
    reset_rage_state(static_cast<LifecycleOwner*>(object)->reset_bindings());
}
void RAGE_MEMBER forward_reset_entry(void* object) { //433CF2F5
    engine::invoke<void>(object, 5);
}
void RAGE_HUD draw_entry(void* object, ui::RenderQueue* queue, void* player, std::int32_t* cursor) {
    static_cast<HudEndpoint*>(object)->owner->bindings.hud.draw(*queue, player, *cursor);
}
#undef RAGE_MEMBER
#undef RAGE_HUD
}

LifecycleOwner* construct_rage_owner(OwnerBindings bindings, LifecycleOwner*& active) { //4348DA25
    require_canonical_bindings(bindings);
    auto* memory = allocate(bindings.lifecycle.memory, sizeof(LifecycleOwner));
    if (!memory) {
        active = nullptr;
        return nullptr;
    }
    auto* owner = ::new (memory) LifecycleOwner(bindings, active);
    active = owner;
    return owner;
}
void destroy_rage_owner(LifecycleOwner*& active) {
    destroy(active, active);
}
void* rage_lifecycle_object(LifecycleOwner& owner) noexcept {
    return &owner;
}
}

// ============================================================================
// Subcomponent: owner_settings.cpp
// ============================================================================

namespace skeet::rage::owner_detail {
void initialize_settings(ConfigRegistries& r, AimbotSettings& a,
                         esp::AimbotVisualizationConfig& v) noexcept {
    auto& d = r.descriptors;
    initialize_integer_config(d, a.minimum_hit_chance, 0x3F5E5553, 50);
    initialize_integer_config(d, a.maximum_fov, 0xD6D22A5A, 180);
    initialize_integer_config(d, a.target_selection, 0x65E86322, 0);
    initialize_integer_config(d, a.minimum_damage, 0x18C0E6BE, 10);
    initialize_boolean_config(d, a.damage_override, 0xB80303B4);
    initialize_hotkey_config(r, a.damage_override_key, 0x7EB3A142, 0x1B, HotkeyMode::hold);
    initialize_integer_config(d, a.override_damage, 0x7DC413FE, 10);
    initialize_integer_config(d, a.multipoint_scale, 0xA25029BA, 90);
    initialize_integer_config(d, a.multipoint, 0xBFABB4A0, 0);
    initialize_integer_config(d, a.accuracy_boost, 0x4346EDB5, 1);
    initialize_boolean_config(d, a.delay_shot, 0xA09C0D52);
    initialize_boolean_config(d, v.enabled, 0x535095F5);
    initialize_color_config(d, v.color, 0x7451E155, 0xCEE55200);
    initialize_boolean_config(d, v.safe_point, 0xE9CC8B95);
    initialize_color_config(d, v.safe_point_color, 0xE20F6035, 0x920000FF);
    initialize_boolean_config(d, a.force_body_aim_on_peek, 0x9BBD48EA);
}
}

// ============================================================================
// Subcomponent: post_firing.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
/* deduplicated helper */
std::int32_t sum(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) + std::uint32_t(b));
}
float ray_distance(const RageVector& center, const RageVector& origin, const RageVector& axis,
                   float bound) noexcept {
    const auto projection =
        ((center[1] - origin[1]) * axis[1] + (center[0] - origin[0]) * axis[0]) +
        (center[2] - origin[2]) * axis[2];
    if (projection < 0.0f || projection > bound)
        return std::numeric_limits<float>::max();
    const auto x = center[0] - (axis[0] * projection + origin[0]);
    const auto y = center[1] - (axis[1] * projection + origin[1]);
    const auto z = center[2] - (axis[2] * projection + origin[2]);
    return std::sqrt((x * x + y * y) + z * z);
}
void publish_shot(PreambleState&, PreambleResult& preamble, AcquisitionStorage& storage,
                  TargetCandidate& selected, void* local, const PostFiringBindings& b) {
    auto& frame = *preamble.frame;
    auto& command = *preamble.command;
    auto& evaluation = storage.evaluations[std::size_t(&selected - storage.candidates.data())];
    auto* target =
        engine::invoke<void*>(b.movement.targeting.entity_list, 3, selected.entity_index);
    if (target) {
        if (!frame.field_1a0)
            capture_shot_diagnostics(frame, local, target, b.diagnostics);
        const auto index = b.shot_index % 150;
        b.shot_index = sum(b.shot_index, 1);
        dispatch_aim_fire_event(b.scripts, b.players, frame, selected, evaluation, index);
        publish_pending_shot(frame, synchronize_evaluation_state(selected, evaluation), index,
                             b.pending_shots,
                             evaluation.extrapolated ? evaluation.extrapolation_ticks : 0);
        publish_accepted_shot_model(frame, selected, evaluation.record, b.shot_models);
        if (frame.field_1d4 == 1 && selected.primary.damage < selected.health) {
            const RageVector delta{selected.primary.position[0] - frame.origin_dc[0],
                                   selected.primary.position[1] - frame.origin_dc[1],
                                   selected.primary.position[2] - frame.origin_dc[2]};
            const auto inverse =
                1.0f /
                (std::sqrt((delta[0] * delta[0] + delta[1] * delta[1]) + delta[2] * delta[2]) +
                 std::numeric_limits<float>::epsilon());
            const RageVector direction{inverse * delta[0], inverse * delta[1], inverse * delta[2]};
            const auto limit = b.entity_limit;
            for (std::int32_t index = 1; index < limit; ++index) {
                auto* other = engine::invoke<void*>(b.movement.targeting.entity_list, 3, index);
                if (std::uint32_t(index) == selected.entity_index ||
                    !target_is_eligible(b.movement.targeting, other))
                    continue;
                auto center = read<RageVector>(other, b.movement.offsets.network_origin);
                auto* collideable = engine::invoke<void*>(other, 3);
                center[2] =
                    engine::invoke<const RageVector*>(collideable, 2)->at(2) * 0.5f + center[2];
                if (100.0f >
                    ray_distance(center, frame.origin_dc, direction, read<float>(&frame, 0x130))) {
                    frame.field_1dc = 12;
                    break;
                }
            }
        }
    }
    if (evaluation.requested_tick > 0) {
        const auto time = maximum_number(read<float>(&frame, 0x198), b.tick_correction);
        const auto ticks = _mm_cvtt_ss2si(_mm_set_ss(time / b.preamble.interval + 0.5f));
        const auto corrected = sum(evaluation.requested_tick, ticks);
        if (corrected <=
            sum(std::bit_cast<std::int32_t>(frame.field_18c), read<std::int32_t>(&frame, 0x144)))
            command.tick = corrected;
    }
    auto& player = b.players.get(selected.entity_index)->native;
    write(&player, 0xA6, std::uint16_t(read<std::uint16_t>(&player, 0xA6) + 1u));
}
}
PreambleTailEntry continue_after_target_firing(PreambleState& state, PreambleResult& preamble,
                                               AcquisitionStorage& storage, TargetBatch& batch,
                                               const TargetFiringResult& firing, void* local,
                                               const PostFiringBindings& b) {
    auto& frame = *preamble.frame;
    auto& command = *preamble.command;
    const auto status = firing.status;
    if (status == TargetFiringStatus::recorded)
        publish_shot(state, preamble, storage, *firing.choice.candidate, local, b);
    const bool cleanup = status == TargetFiringStatus::recorded ||
                         status == TargetFiringStatus::post_angles_stopped ||
                         status == TargetFiringStatus::attack_button_only;
    if (cleanup && !b.preamble.settings.silent_aim.value)
        client::set_command_view_angles(b.command_angles, command.angles);
    if (status == TargetFiringStatus::recorded || status == TargetFiringStatus::retained_target ||
        status == TargetFiringStatus::readiness_stopped ||
        status == TargetFiringStatus::post_angles_stopped ||
        status == TargetFiringStatus::attack_button_only) {
        state.flags |= 0x80U;
        if (firing.choice.candidate->primary.chance != 1.0f)
            state.flags |= 0x100U;
    } else if (status == TargetFiringStatus::rejected_batch)
        state.flags |= 0x100U;
    const auto* leading =
        status == TargetFiringStatus::empty_batch ? b.prior_leading : batch.candidates[0];
    const auto* evaluation =
        leading ? &storage.evaluations[std::size_t(leading - storage.candidates.data())] : nullptr;
    if (b.preamble.settings.quick_stop.value &&
        hotkey_active(b.preamble.settings.quick_stop_key.value.binding, b.preamble.input) &&
        !(state.flags & 0x80U) && leading) {
        state.flags |= 8U;
        float chance = 0.0f;
        TargetFrameBindings current(b.prediction, b.scanner, frame, b.preamble.prediction_time,
                                    b.preamble.scheduler_flags);
        const FinalTargetContextBindings final{current.scanner, b.final_target.current_bone_counter,
                                               b.final_target.settings_menu, b.final_target.jobs};
        if (evaluate_final_target(frame, state, *leading, *evaluation, state.flags, final, chance))
            state.flags |= 0x180U;
        state.flags &= ~8U;
    }
    post_firing_detail::predict_stop(state, preamble, storage, local, b);
    finish_post_firing_movement(frame, command, leading, evaluation, true,
                                preamble.original_firing_bit, b.movement);
    return PreambleTailEntry::after_firing;
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: post_firing_movement.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
void* weapon(const client::CommandFrame& frame) noexcept {
    return reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
}
bool quick_stop_key(const PostFiringMovementBindings& b) {
    return b.settings.quick_stop.value &&
           hotkey_active(b.settings.quick_stop_key.value.binding, b.input);
}
std::uint32_t options(const client::CommandFrame& frame,
                      const PostFiringMovementBindings& b) noexcept {
    auto result = std::uint32_t(b.settings.quick_stop_options.value);
    if (frame.field_1d4 == 1)
        result |= std::uint32_t(b.settings.double_tap_quick_stop.value);
    return result;
}
bool publish_stop(const client::CommandFrame& frame, std::uint32_t policy,
                  const PostFiringMovementBindings& b) {
    if (frame.weapon_id_d8 == 31 && !(policy & 0x40U))
        return false;
    if ((policy & 8U) && !(frame.flags & 0x400U)) {
        if (frame.weapon_address && engine::invoke<float>(weapon(frame), 433, 0) > 1.0f)
            return false;
        if (read<float>(b.targeting.local_player, b.offsets.player_next_attack) - b.current_time >
            0.4f)
            return false;
    }
    b.scheduler_flags |= 8U;
    return true;
}
void damage_stop(const client::CommandFrame& frame, const TargetCandidate* leading,
                 const AcquisitionEvaluation* evaluation, const PostFiringMovementBindings& b) {
    if (!leading)
        return;
    const auto origin =
        RageVector{frame.origin_10c[0], frame.origin_10c[1], frame.origin_10c[2] + 46.0f};
    auto query = prepare_target_bullet(weapon(frame), b.weapon_item,
                                       b.settings.automatic_penetration.value != 0);
    anti_aim::TargetTraceFilter native_filter(
        b.trace_filter_vtable, b.trace_entities, 0x600400BU, 0x600400BU,
        float(std::bit_cast<std::int32_t>(evaluation->settings.words[5])) /
            rage_detail::hitgroup_multipliers[std::uint32_t(leading->primary.hitgroup) & 15U]);
    trace_ballistic_bullet(b.ballistics, b.source, origin, leading->primary.position,
                           native_filter.binding(), query);
    auto damage = _mm_cvtt_ss2si(_mm_set_ss(query.damage));
    if (damage) {
        auto* target = engine::invoke<void*>(b.targeting.entity_list, 3, leading->entity_index);
        if (target) {
            const auto info =
                b.ballistics.entity_info(BallisticEntity(reinterpret_cast<std::uintptr_t>(target)));
            damage = final_bullet_damage(
                query.damage, query.armor_ratio, std::bit_cast<std::int32_t>(query.reserved),
                query.weapon_id, leading->primary.hitgroup, info.armor, info.helmet);
        }
    }
    if (damage >= std::bit_cast<std::int32_t>(evaluation->settings.words[5]))
        b.scheduler_flags |= 0x11000U;
}
void between_shots(const client::CommandFrame& frame, const PostFiringMovementBindings& b) {
    if (!(b.context_flags & 0x40U) || !(frame.flags & 0x40000U))
        return;
    const auto time = (frame.player_flags_7c & 2U)
                          ? engine::invoke<float>(weapon(frame), 436, frame.field_b0)
                          : engine::invoke<float>(weapon(frame), 435, frame.field_b0);
    const auto difference = std::bit_cast<float>(
        std::bit_cast<std::uint32_t>(frame.inaccuracy_c4 - time) & 0x7FFFFFFFU);
    if (0.0001f > difference)
        b.scheduler_flags |= 0x10U;
}
}
bool post_firing_prediction_allowed(const PostFiringPredictionMasks& source) noexcept {
    const auto& masks = prediction_mask_state(*source.owner);
    if (masks.blocked[0] || masks.blocked[1])
        return false;
    return masks.allowed[0] || masks.allowed[1];
}
std::size_t select_post_firing_candidate(const PreambleState& state,
                                         const AcquisitionStorage& storage,
                                         std::int32_t remembered) noexcept {
    auto select = [&](bool match) {
        auto best = no_rage_point;
        for (std::size_t i = 0; i < state.evaluation_count; ++i) {
            if (match &&
                storage.candidates[i].entity_index != (std::uint32_t(remembered) & 0xFFFFU))
                continue;
            if (best == no_rage_point ||
                std::bit_cast<std::int32_t>(storage.evaluations[i].ballistic_evaluations) >
                    std::bit_cast<std::int32_t>(storage.evaluations[best].ballistic_evaluations))
                best = i;
        }
        return best;
    };
    const auto matching = remembered != -1 ? select(true) : no_rage_point;
    return matching != no_rage_point ? matching : select(false);
}
void finish_post_firing_movement(client::CommandFrame& frame, client::Command& command,
                                 const TargetCandidate* leading,
                                 const AcquisitionEvaluation* evaluation, bool ground_gate,
                                 bool restore_bit, const PostFiringMovementBindings& b) {
    if (b.settings.automatic_scope.value && !(frame.flags & 0x4000U) &&
        (b.context_flags & 0x100U)) {
        const bool grounded =
            (frame.flags & 0x40000U) &&
            (std::to_integer<std::uint8_t>(frame.fields_e8[0x10]) & std::uint32_t(ground_gate));
        const auto speed = frame.velocity_150[0] * frame.velocity_150[0] +
                           frame.velocity_150[1] * frame.velocity_150[1];
        if (grounded || (frame.weapon_id_d8 == 40 &&
                         !(frame.previous_command.movement.buttons & 0x618U) && 900.0f > speed)) {
            if (engine::invoke<std::int32_t>(weapon(frame), 443) > 0 &&
                !read<std::uint8_t>(b.targeting.local_player, b.offsets.scope_guard_cacaeecc) &&
                !read<std::uint8_t>(b.targeting.local_player, b.offsets.scope_guard_c07d1150) &&
                !(command.movement.buttons & 1U))
                command.movement.buttons |= 0x800U;
        }
    }
    if ((b.context_flags & 0x80U) && quick_stop_key(b) && frame.state_104 == 2) {
        const auto policy = options(frame, b);
        if (publish_stop(frame, policy, b) && leading) {
            if (policy & 2U)
                damage_stop(frame, leading, evaluation, b);
            between_shots(frame, b);
        }
    }
    if ((std::uint32_t(b.settings.quick_stop_options.value) & 0x20U) &&
        lagcomp::history_weapon_is_firearm(std::uint16_t(frame.weapon_id_d8)) &&
        frame.weapon_id_d8 != 31 && frame.state_104 == 2 && (frame.flags & 0x40000U) &&
        !(frame.previous_command.movement.buttons & 6U) && b.remembered_handle != -1) {
        auto* target = engine::invoke<void*>(b.targeting.entity_list, 4, b.remembered_handle);
        if (target && !engine::invoke<bool>(static_cast<std::byte*>(target) + 8, 9) &&
            post_firing_prediction_allowed(b.prediction))
            b.scheduler_flags |= 0x100U;
    }
    if ((frame.flags & 0x20000U) && !(b.scheduler_flags & 8U) && (b.scheduler_flags & 0x4000U)) {
        if ((b.target_mask[0] || b.target_mask[1]) && frame.state_104 == 2) {
            const auto policy = options(frame, b);
            if (publish_stop(frame, policy, b) && leading) {
                if (policy & 2U)
                    damage_stop(frame, leading, evaluation, b);
                between_shots(frame, b);
            }
        }
    }
    frame.flags = (frame.flags & ~0x400U) | (restore_bit ? 0x400U : 0U);
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: post_firing_prediction.cpp
// ============================================================================

namespace skeet::rage::post_firing_detail {
namespace {
struct SavedFrame {
    std::int32_t tick;
    RageVector origin, bone_origin, velocity;
    float duck_amount;
    std::uint32_t player_flags, flags;
};
/* [dedup movement] */
RageVector rage(const MovementVector& value) noexcept {
    return {value.x, value.y, value.z};
}
}
void predict_stop(PreambleState& state, PreambleResult& preamble, AcquisitionStorage& storage,
                  void* local, const PostFiringBindings& b) {
    auto& frame = *preamble.frame;
    auto& command = *preamble.command;
    const auto& settings = b.preamble.settings;
    if ((state.flags & 0x80U) || !settings.quick_stop.value ||
        !hotkey_active(settings.quick_stop_key.value.binding, b.preamble.input) ||
        !(std::uint32_t(settings.quick_stop_options.value) & 1U) || !frame.weapon_address ||
        !(frame.flags & 0x40000U) || frame.weapon_id_d8 == 31 || (state.flags & 3U))
        return;
    const auto slot = select_post_firing_candidate(state, storage, b.movement.remembered_handle);
    if (slot == no_rage_point)
        return;
    auto& candidate = storage.candidates[slot];
    auto& evaluation = storage.evaluations[slot];
    if (evaluation.requested_tick == -1)
        return;
    reset_scan_point(candidate.primary);
    evaluation.settings.words[0] = 1;
    if (frame.flags & 0x1000000U) {
        evaluation.settings.words[1] &= ~2U;
        evaluation.settings.words[2] &= ~2U;
    }
    if (!(evaluation.settings.words[1] | evaluation.settings.words[2]))
        return;
    state.flags |= 8U;
    auto* target =
        engine::invoke<void*>(b.movement.targeting.entity_list, 3, candidate.entity_index);
    b.players.get(candidate.entity_index); //native563EA, before frame/simulation reads
    const SavedFrame saved{frame.tick_base_c8, frame.origin_dc,      frame.origin_10c,
                           frame.velocity_150, frame.duck_amount_98, frame.player_flags_7c,
                           frame.flags};
    auto* weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
    const auto threshold = rage_stop_speed(weapon);
    MovementSimulation simulation;
    initialize_live_movement_simulation(
        simulation, local, movement(frame.origin_10c), movement(frame.velocity_150),
        frame.player_flags_7c, std::uint32_t(frame.tick_base_c8) + 1u, b.simulation_capture);
    std::int32_t ticks = 0;
    while (ticks < 14) {
        const auto squared = simulation.velocity.y * simulation.velocity.y +
                             simulation.velocity.x * simulation.velocity.x;
        if (threshold * threshold >= squared)
            break;
        auto copy = frame.previous_command;
        prepare_rage_stop_command(frame, local, copy, simulation, b.stop);
        simulate_live_movement_tick(simulation, b.collision, b.stop.environment, local, copy);
        ++ticks;
    }
    bool projected = false;
    if (ticks) {
        const auto x = simulation.position.x - frame.origin_10c[0];
        const auto y = simulation.position.y - frame.origin_10c[1];
        const auto z = simulation.position.z - frame.origin_10c[2];
        if (!(0.1f > ((y * y + x * x) + z * z))) {
            projected = true;
            frame.tick_base_c8 = std::bit_cast<std::int32_t>(simulation.mode);
            frame.origin_dc = {simulation.position.x, simulation.position.y,
                               simulation.view_offset_z + simulation.position.z};
            frame.origin_10c = rage(simulation.position);
            frame.velocity_150 = rage(simulation.velocity);
            frame.duck_amount_98 = simulation.duck_amount;
            frame.player_flags_7c = simulation.player_flags;
            frame.flags = (frame.flags & ~0x40000U) | ((simulation.player_flags & 1U) << 18);
            b.preamble.prediction_time = float(frame.tick_base_c8) * b.preamble.interval;
        }
    }
    if (evaluation.settings.option(0xDE) && (frame.flags & 0x400U) && !(state.flags & 3U) &&
        !(b.preamble.scheduler_flags & 8U)) {
        const auto network_index =
            engine::invoke<std::uint32_t>(static_cast<std::byte*>(target) + 8, 10);
        auto& player_state = *b.players.get(network_index);
        auto trace = b.prediction.trace;
        trace.context_flags = state.flags;
        trace.weapon_flags = state.weapon_flags;
        trace.weapon = weapon;
        const TargetPredictionBindings seed{trace,
                                            b.prediction.record_selection,
                                            b.prediction.engine_trace,
                                            b.prediction.local_maximum_speed,
                                            b.preamble.scheduler_flags,
                                            &command};
        TargetFrameBindings current(seed, b.scanner, frame, b.preamble.prediction_time,
                                    b.preamble.scheduler_flags);
        refine_acquisition_minimum_damage(frame, target, player_state, evaluation.settings,
                                          current.prediction, b.bone_runtime);
    }
    std::array<std::uint32_t, 3> completed{};
    TargetFrameBindings current(b.prediction, b.scanner, frame, b.preamble.prediction_time,
                                b.preamble.scheduler_flags);
    ScanEvaluation scan{candidate, evaluation.record, evaluation.source_record,
                        evaluation.settings.scan_policy(), evaluation.ballistic_evaluations};
    ScanRequest request{
        target, BallisticEntity(reinterpret_cast<std::uintptr_t>(target)), scan, 0, completed, 0};
    scan_candidate(current.scanner, request);
    if (completed[0]) {
        float chance = 1.0f;
        TargetFrameBindings after_scan(b.prediction, b.scanner, frame, b.preamble.prediction_time,
                                       b.preamble.scheduler_flags);
        const FinalTargetContextBindings final{after_scan.scanner,
                                               b.final_target.current_bone_counter,
                                               b.final_target.settings_menu, b.final_target.jobs};
        if (evaluate_final_target(frame, state, candidate, evaluation, state.flags, final,
                                  chance)) {
            state.flags |= 0x180U;
            frame.flags |= 0x10000U;
        }
    }
    state.flags &= ~8U;
    if (projected) {
        frame.tick_base_c8 = saved.tick;
        frame.origin_dc = saved.origin;
        frame.origin_10c = saved.bone_origin;
        frame.velocity_150 = saved.velocity;
        frame.duck_amount_98 = saved.duck_amount;
        frame.player_flags_7c = saved.player_flags;
        frame.flags = (frame.flags & ~0x40000U) | (saved.flags & 0x40000U);
        b.preamble.prediction_time = float(saved.tick) * b.preamble.interval;
    }
}
} // namespace skeet::rage::post_firing_detail

// ============================================================================
// Subcomponent: preamble.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
/* [stripped helper] */
void* weapon_of(const client::CommandFrame& frame) noexcept {
    return reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
}
/* [dedup convar_nonzero] */
std::int32_t sub(std::int32_t first, std::int32_t second) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(first) - std::uint32_t(second));
}
std::uint32_t hash_command(const client::Command& command) noexcept {
    std::uint32_t result = 0x811C9DC5;
    for (const auto byte : std::as_bytes(std::span(&command, 1)))
        result = (std::to_integer<std::uint8_t>(byte) ^ result) * 0x1000193;
    return result ^ 0x92CA5DF1;
}
void refresh_target(const PreambleBindings& bindings, const client::CommandFrame& original,
                    const client::Command& command, void* player) {
    if (auto* live = std::get_if<std::reference_wrapper<const client::TargetRefreshRuntimeOwner>>(
            &bindings.target_refresh)) {
        live->get().refresh(bindings.yaw_target, player, command,
                            original.previous_command.movement.buttons,
                            field<float>(&original, 0x11C));
        return;
    }
    const auto& refresh = std::get<std::reference_wrapper<const anti_aim::TargetRefreshContext>>(
                              bindings.target_refresh)
                              .get();
    const auto& base = refresh.scored;
    const anti_aim::TargetScoredContext scored{
        {base.common, command, original.previous_command.movement.buttons,
         field<float>(&original, 0x11C), base.player_rules, base.dormant_health, base.offsets,
         base.geometry_offsets, base.local_pose_origin, base.local_pose, base.trigonometry,
         base.sequence, base.clock_owner, base.local_time_offset, base.trace_filter_vtable,
         base.trace_entities},
        base.movement,
        base.movement_environment};
    anti_aim::refresh_yaw_target(bindings.yaw_target, {scored, refresh.selector_convar});
}
struct ScanTemplate {
    std::uint32_t mode;
    RageHitboxMasks hitboxes;
    float scale;
    std::uint32_t multipoint;
    std::int32_t damage;
    float chance, minimum_cosine;
    std::array<std::uint8_t, 8> options;
};
static_assert(sizeof(ScanTemplate) == 0x28);
PreambleScanTemplate prepare_template(const PreambleState& state, const client::CommandFrame& frame,
                                      const PreambleBindings& b) {
    const auto& g = b.settings;
    const auto& w = b.weapon_settings;
    ScanTemplate result{};
    result.mode = std::uint32_t(w.accuracy_boost.value ? w.accuracy_boost.value : 1);
    result.hitboxes = prepare_rage_hitboxes(
        std::uint32_t(g.target_hitboxes.value), g.prefer_safe_point.value != 0,
        hotkey_active(g.force_safe_point.value.binding, b.input),
        std::uint32_t(g.avoid_unsafe_hitboxes.value), state.flags, g.prefer_body_aim.value != 0,
        std::uint8_t(g.body_aim_disablers.value));
    result.scale = w.multipoint_scale.value == 24
                       ? 1.0f
                       : minimum_number(0.9f, float(w.multipoint_scale.value) * 0.01f - 0.01f);
    result.multipoint = hotkey_active(g.multipoint_key.value.binding, b.input)
                            ? std::uint32_t(w.multipoint.value)
                            : 0;
    result.damage = w.minimum_damage.value;
    result.chance = float((state.flags & 0x20U) ? b.settings.double_tap_hit_chance.value
                                                : w.minimum_hit_chance.value) *
                    0.01f;
    result.minimum_cosine =
        b.trigonometry.cosine(float(w.maximum_fov.value) * 0.01745329238474369049f);
    result.options[0] = g.prefer_safe_point.value;
    result.options[1] = g.prefer_body_aim.value;
    result.options[2] = w.delay_shot.value;
    result.options[3] = w.force_body_aim_on_peek.value;
    if (frame.weapon_id_d8 == 31) {
        if (result.damage < 105)
            result.damage = 105;
        result.options[1] = 1;
        if (field<float>(weapon_of(frame), b.offsets.taser_timing) > 0.05f)
            result.multipoint = 0;
    }
    if (frame.field_1a0 & 1U)
        result.mode = 1;
    if (frame.field_1a0 & 2U)
        result.multipoint &= ~0x200U;
    if (frame.field_1a0 & 0x10U)
        result.hitboxes.regular &= ~0x200U;
    if (frame.field_1a0 & 8U)
        result.multipoint &= ~0xC0U;
    if (frame.field_1a0 & 4U)
        result.multipoint &= ~0x30U;
    if ((state.flags & 0x400U) && frame.weapon_id_d8 != 31) {
        result.hitboxes.regular &= 2;
        result.hitboxes.safe &= 2;
        result.multipoint &= 2;
        result.options[1] = 0;
        result.options[3] = 0;
    }
    if (w.damage_override.value && hotkey_active(w.damage_override_key.value.binding, b.input)) {
        result.options[4] = 1;
        result.damage = w.override_damage.value;
        result.options[2] = 0;
    }
    return std::bit_cast<PreambleScanTemplate>(result);
}
void prepare_firing_branch(PreambleResult& out, const PreambleBindings& b, void* player) {
    auto& frame = *out.frame;
    auto& branch = out.firing;
    branch.field_22 = frame.weapon_id_d8 == 64 && (frame.flags & 0x40U);
    branch.alternate_path = false;
    branch.field_33 = false;
    branch.field_83 = false;
    branch.timing_ready = false;
    if (b.command_sequence_gate && frame.choked_15c != 0 &&
        b.selected_marker > b.current_sequence && b.selected_marker < frame.command_number_ac)
        return;
    branch.timing_ready =
        (frame.flags & 0x800U) ? (frame.flags & 2U) != 0 : (frame.flags & 0x400U) != 0;
    if (frame.field_1d4 == 2 && !(frame.flags & 0x40000000U)) {
        branch.timing_ready = false;
        if (frame.weapon_id_d8 != 64 && (frame.flags & 0x4000000U))
            branch.field_33 = true;
        return;
    }
    if (branch.timing_ready)
        return;
    branch.field_33 = frame.weapon_id_d8 != 64 && (frame.flags & 0x4000000U);
    if (!b.settings.silent_aim.value)
        return;
    const float attack =
        maximum_number(field<float>(player, b.offsets.player_next_attack),
                       field<float>(weapon_of(frame), b.offsets.weapon_next_attack));
    const float quotient = attack / b.interval;
    const auto rounded = _mm_cvtss_si32(_mm_set_ss(quotient));
    const std::uint32_t ceiling = std::uint32_t(rounded) + std::uint32_t(quotient > float(rounded));
    const auto current = _mm_cvttss_si32(_mm_set_ss(b.prediction_time / b.interval + 0.5f));
    branch.field_83 = sub(std::bit_cast<std::int32_t>(ceiling), current) == 1;
}
}
PreambleScanTemplate prepare_rage_scan_template(const PreambleState& state,
                                                const client::CommandFrame& frame,
                                                const PreambleBindings& bindings) {
    return prepare_template(state, frame, bindings);
}
PreambleResult prepare_rage_command(PreambleState& state, client::CommandFrame& original,
                                    client::Command& command, void* player,
                                    AcquisitionStorage& storage, const PreambleBindings& b) {
    PreambleResult out;
    out.frame = &original;
    out.command = &command;
    const auto previous = b.scheduler_flags;
    b.scheduler_flags = ((previous & 8U) << 11) | (previous & 0xFFFEAFE7U);
    auto* weapon = weapon_of(original);
    if (weapon && !field<std::uint8_t>(weapon, b.offsets.weapon_reload_guard)) {
        bool keep = false;
        if (command.movement.buttons & 0x2000U) {
            const auto ammunition = field<std::int32_t>(weapon, b.offsets.ammunition);
            if (ammunition != -1) {
                // Real weapon virtual+5DC, reached through the client ABI thunk.
                const auto maximum = engine::invoke<std::int32_t>(weapon, 375);
                keep = ammunition < maximum &&
                       field<std::int32_t>(weapon, b.offsets.reserve_ammunition) > 0;
            }
        }
        if (!keep)
            b.scheduler_flags &= ~0x100U;
    }
    state.flags = (original.flags & 0x8000000U) ? 0x20U : 0;
    if (original.flags & 0x4000U)
        state.flags |= 4;
    else if ((state.flags & 0x20U) ? b.settings.double_tap_hit_chance.value > 0
                                   : b.weapon_settings.minimum_hit_chance.value > 0)
        state.flags |= 0x10;
    if (original.flags & 8U)
        state.flags |= 0x200;
    if (convar_nonzero(b.restricted_convar))
        state.flags |= 0x400;
    if (!weapon)
        return out;
    bool active =
        b.settings.enabled.value && hotkey_active(b.settings.activation.value.binding, b.input);
    if (active) {
        active = !(b.game_rules && *b.game_rules &&
                   field<std::uint8_t>(*b.game_rules, b.offsets.local_guard));
        if (active)
            active = lagcomp::history_weapon_is_firearm(
                field<std::uint16_t>(weapon, b.offsets.weapon_identifier));
    }
    if (active && (b.scheduler_flags & 0x40U)) {
        if (original.choked_15c < original.choke_limit_fc || original.choked_15c <= 0)
            active = false;
        else {
            const auto count =
                state.frame_count < original.choked_15c ? state.frame_count : original.choked_15c;
            auto number = std::uint32_t(original.command_number_ac) - 1;
            for (std::int32_t i = 0; i < count; ++i, --number) {
                auto& candidate_command = b.command_ring[number % 150U];
                if (std::uint32_t(candidate_command.number) != number ||
                    std::uint32_t(candidate_command.tick) == 0x7F7FFFFFU)
                    continue;
                auto& candidate = state.frames[(state.frame_first + std::uint32_t(i)) & 15U];
                if (std::uint32_t(candidate.command_number_ac) == number &&
                    field<float>(&candidate, 0x84) > field<float>(out.frame, 0x84)) {
                    out.frame = &candidate;
                    out.command = &candidate_command;
                }
            }
            if (out.frame != &original) {
                auto& selected = *out.frame;
                out.prediction_clock_marker = 0;
                const auto number = std::uint32_t(selected.command_number_ac);
                if (const auto* saved_address = b.animation_history.at(number)) {
                    PreambleAnimationSnapshot saved;
                    std::memcpy(&saved, saved_address, sizeof saved);
                    if (saved.sequence == number) {
                        out.prediction_clock_marker =
                            std::uint32_t(reinterpret_cast<std::uintptr_t>(saved_address));
                        const auto* layers =
                            static_cast<std::byte*>(player) + b.offsets.animation_layers;
                        lagcomp::restore_animation(
                            saved.animation, field<void*>(layers, 0),
                            field<std::int32_t>(layers, 0xC),
                            reinterpret_cast<float*>(static_cast<std::byte*>(player) +
                                                     b.offsets.pose_parameters));
                        if (b.set_absolute_angles) {
                            const lagcomp::Vector angles{0.0f, saved.absolute_yaw, 0.0f};
                            b.set_absolute_angles(player, angles);
                        }
                    }
                }
                store(&selected, 0xD4, field<std::uint32_t>(&original, 0xD4));
                selected.field_18c = 0;
                store(&selected, 0x144, field<std::uint32_t>(&original, 0x144));
                out.previous_prediction_time = b.prediction_time;
                out.replaced_frame = true;
                b.prediction_time = float(field<std::int32_t>(&selected, 0xC8)) * b.interval;
                out.command_hash = hash_command(*out.command);
                state.flags |= 2;
            }
        }
    }
    if (b.settings.enabled.value)
        refresh_target(b, original, command, player);
    if (!active) {
        out.continuation = PreambleContinuation::inactive;
        return out;
    }
    auto& frame = *out.frame;
    out.tick_base = field<std::int32_t>(&frame, 0xC8);
    weapon = weapon_of(frame);
    out.point_scale_is_automatic = b.weapon_settings.multipoint_scale.value == 24;
    if (field<std::int32_t>(weapon, b.offsets.ammunition) == 0) {
        if (field<std::int32_t>(weapon, b.offsets.reserve_ammunition) > 0)
            out.command->movement.buttons = (out.command->movement.buttons & ~1U) | 0x2000U;
        out.continuation = PreambleContinuation::ammunition;
        return out;
    }
    if (*reinterpret_cast<const std::uint8_t*>(std::uintptr_t(frame.send_packet_address)) &&
        firing_mode(b.firing_mode_enabled, b.send_mode) == 1 && std::int32_t(frame.field_18c) < 1)
        frame.field_18c = 1;
    out.original_firing_bit = (frame.flags & 0x400U) != 0;
    prepare_firing_branch(out, b, player);
    out.scan_template = prepare_rage_scan_template(state, frame, b);
    const SpreadParameters spread{std::uint32_t(frame.weapon_id_d8),
                                  std::int32_t(frame.field_b0),
                                  field<float>(&frame, 0xD0),
                                  field<float>(&frame, 0xC4),
                                  std::bit_cast<float>(frame.field_100),
                                  field<std::int32_t>(&frame, 0x19C),
                                  b.shotgun_patterns,
                                  frame.weapon_kind_124};
    prepare_spread_cache(state.spread, spread, std::bit_cast<float>(frame.field_160),
                         std::uint32_t(frame.command_number_ac), (state.flags & 0x10U) != 0,
                         b.spread_tables, b.random, b.pattern, b.trigonometry);
    storage.queued.clear();
    state.completed.fill(0);
    state.evaluation_count = 0;
    out.continuation = b.weapon_settings.target_selection.value == 1
                           ? PreambleContinuation::single_target
                           : PreambleContinuation::multiple_targets;
    return out;
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: preamble_tail.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
/* [stripped helper] */
constexpr float degrees = 57.2957763671875f;
std::uint32_t command_hash(const client::Command& command) noexcept {
    std::uint32_t result = 0x811C9DC5;
    for (const auto byte : std::as_bytes(std::span(&command, 1)))
        result = (std::to_integer<std::uint8_t>(byte) ^ result) * 0x1000193;
    return result ^ 0x92CA5DF1;
}
SpreadParameters spread_parameters(const client::CommandFrame& frame, bool patterns) noexcept {
    return {std::uint32_t(frame.weapon_id_d8),
            std::int32_t(frame.field_b0),
            field<float>(&frame, 0xD0),
            field<float>(&frame, 0xC4),
            std::bit_cast<float>(frame.field_100),
            field<std::int32_t>(&frame, 0x19C),
            patterns,
            frame.weapon_kind_124};
}
MovementAngles command_angles(const client::Command& command) noexcept {
    return {command.angles[0], command.angles[1], command.angles[2]};
}
void command_angles(client::Command& command, const MovementAngles& angles) noexcept {
    command.angles = {angles.pitch, angles.yaw, angles.roll};
}
void subtract_recoil(client::Command& command, client::CommandFrame& frame, bool normalize) {
    const auto recoil = field<RageVector>(&frame, 0x9C);
    if ((recoil[0] * recoil[0] + recoil[1] * recoil[1]) + recoil[2] * recoil[2] != 0.0f) {
        for (unsigned i = 0; i < 3; ++i)
            command.angles[i] -= recoil[i];
        if (normalize) {
            auto angles = command_angles(command);
            normalize_movement_angles(angles);
            command_angles(command, angles);
        }
        frame.flags |= 0x10U;
    }
}
MovementAngles direction_angles(RageVector direction, const TargetAngleMath& math) {
    const auto squared =
        (direction[0] * direction[0] + direction[1] * direction[1]) + direction[2] * direction[2];
    const float inverse = 1.0f / (std::sqrt(squared) + 0x1p-23f);
    for (auto& component : direction)
        component *= inverse;
    if (direction[0] == 0.0f && direction[1] == 0.0f)
        return {direction[2] > 0.0f ? 270.0f : 90.0f, 0, 0};
    const auto yaw = float(math.atan2(double(direction[1]), double(direction[0]))) * degrees;
    const auto horizontal = std::sqrt(direction[0] * direction[0] + direction[1] * direction[1]);
    return {float(math.atan2(double(-direction[2]), double(horizontal))) * degrees, yaw, 0};
}
void remove_spread_vector(client::Command& command, RageSpreadSample spread,
                          const TargetAngleMath& math) {
    MovementVector forward, right, up;
    movement_angle_vectors(command_angles(command), &forward, &right, &up);
    RageVector direction{forward.x - (up.x * spread.second + right.x * spread.first),
                         forward.y - (up.y * spread.second + right.y * spread.first),
                         forward.z - (up.z * spread.second + right.z * spread.first)};
    auto angles = direction_angles(direction, math);
    normalize_movement_angles(angles);
    command_angles(command, angles);
}
void limit_step(client::Command& command, const client::CommandFrame& frame, void* player,
                const PreambleTailBindings& b) {
    auto reference = field<MovementAngles>(&frame, 0x20);
    if (!(field<std::uint32_t>(player, b.player_offsets.flags) & 8U) &&
        field<std::uint8_t>(player, b.player_offsets.first_byte) ==
            field<std::uint8_t>(player, b.player_offsets.second_byte)) {
        const auto number = std::bit_cast<std::int32_t>(std::uint32_t(command.number) - 1U);
        if (const auto* previous = engine::invoke<const client::Command*>(b.input, 8, 0, number))
            reference = command_angles(*previous);
    }
    auto angles = command_angles(command);
    limit_firing_angle_step(angles, reference, field<float>(&frame, 0x1E8));
    command_angles(command, angles);
}
// The air-side correction433578B3..43357E25. The entry pitch/roll changes
// precede these gates and remain observable when the prediction path is skipped.
bool correct_air_origin(PreambleState& state, PreambleResult& result, void* player,
                        const PreambleTailBindings& b) {
    auto& frame = *result.frame;
    auto& command = *result.command;
    if (!player || !b.shared.settings.enabled.value || (state.flags & 1U) ||
        (frame.flags & 0x40U) || command.movement.side_move == 0.0f || (frame.flags & 0x40000U) ||
        command.angles[2] == 0.0f)
        return false;
    command.movement.side_move = 0.0f;
    if (!(frame.flags & 0x1000U))
        return false;
    MovementSimulation simulation{};
    const auto simulate = [&](const MovementCaptureInput& capture,
                              const MovementEnvironmentBindings& environment,
                              MovementCollisionEngine& collision) {
        initialize_live_movement_simulation(
            simulation, player, field<MovementVector>(&frame, 0x168),
            field<MovementVector>(&frame, 0x180), field<std::uint32_t>(&frame, 0xF8),
            field<std::uint32_t>(&frame, 0xC8), capture);
        simulate_live_movement_tick(simulation, collision, environment, player, command);
    };
    if (const auto* decoded =
            std::get_if<std::reference_wrapper<const anti_aim::TargetRefreshContext>>(
                &b.shared.target_refresh)) {
        const auto& context = decoded->get().scored;
        simulate(context.movement, context.movement_environment, context.common.collision);
    } else {
        simulate(b.action.live_movement.capture, b.action.live_movement.environment,
                 b.action.collision);
    }
    const auto delta_x = simulation.position.x - frame.origin_10c[0];
    const auto delta_y = simulation.position.y - frame.origin_10c[1];
    const auto squared = delta_y * delta_y + delta_x * delta_x;
    if (!(squared > 0.01f && 4096.0f > squared))
        return false;
    frame.origin_10c[0] = simulation.position.x;
    frame.origin_10c[1] = simulation.position.y;
    frame.origin_dc[0] += delta_x;
    frame.origin_dc[1] += delta_y;
    const auto point = field<RageVector>(&frame, 0xE8);
    const auto recoil = field<RageVector>(&frame, 0x9C);
    const bool category_recoil = legit::recoil_compensation_enabled(
        b.legit_settings, std::uint32_t(std::int32_t(std::int8_t(frame.weapon_kind_124))),
        b.shared.input);
    auto angles = target_aim_angles(point, frame.origin_dc, recoil,
                                    category_recoil || !b.shared.settings.remove_recoil.value,
                                    frame.flags, b.angle_math);
    command_angles(command, angles);
    if (frame.flags & 0x40U)
        limit_step(command, frame, player, b);
    return true;
}
bool shot_path(const client::CommandFrame& frame, const client::Command& command) noexcept {
    if (frame.flags & 0x100000U)
        return true;
    if ((frame.flags & 0x400U) && (command.movement.buttons & 1U))
        return true;
    return (frame.flags & 2U) && (command.movement.buttons & 0x800U) &&
           (frame.weapon_id_d8 == 64 || frame.weapon_kind_124 == 0);
}
void correct_shot_angles(PreambleState& state, PreambleResult& result, void* player,
                         const PreambleTailBindings& b) {
    auto& frame = *result.frame;
    auto& command = *result.command;
    const auto& shared = b.shared;
    const auto parameters = spread_parameters(frame, shared.shotgun_patterns);
    const auto seed = std::uint32_t(command.random_seed) & 0xFFU;
    if (frame.flags & 0x4000U) {
        const auto spread =
            generate_spread_sample(parameters, seed, shared.spread_tables, shared.random,
                                   shared.pattern, shared.trigonometry);
        const float radius = std::sqrt(spread.first * spread.first + spread.second * spread.second);
        if (!(std::numeric_limits<float>::min() > radius)) {
            if (frame.flags & 8U) {
                const auto pitch = b.arctangent(radius) * degrees;
                command.angles[0] += pitch;
                command.angles[2] =
                    -(float(b.angle_math.atan2(double(spread.first), double(spread.second))) *
                      degrees);
                if (correct_air_origin(state, result, player, b)) {
                    command.angles[0] += b.arctangent(radius) * degrees;
                    command.angles[2] =
                        -(float(b.angle_math.atan2(double(spread.first), double(spread.second))) *
                          degrees);
                }
            } else
                remove_spread_vector(command, spread, b.angle_math);
        }
    }
    if (!shared.settings.remove_recoil.value)
        return;
    if (legit::recoil_compensation_enabled(
            b.legit_settings, std::uint32_t(std::int32_t(std::int8_t(frame.weapon_kind_124))),
            shared.input))
        return;
    if (shared.settings.pattern_recoil_correction.value && shared.shotgun_patterns &&
        shared.pattern.get && parameters.pellet_count > 1) {
        const auto spread = preamble_pattern_correction(parameters, seed, shared.random,
                                                        shared.pattern, shared.trigonometry);
        const auto radius = std::sqrt(spread.first * spread.first + spread.second * spread.second);
        if (!(std::numeric_limits<float>::min() > radius))
            remove_spread_vector(command, spread, b.angle_math);
    }
    subtract_recoil(command, frame, true);
}
bool publish_historical_command(PreambleResult& result, client::CommandFrame& original,
                                const PreambleTailBindings& b) {
    auto& frame = *result.frame;
    auto& command = *result.command;
    if (&frame == &original || command_hash(command) == result.command_hash)
        return false;
    auto& verified = b.verified_commands[std::uint32_t(frame.command_number_ac) % 150U];
    if (verified.command.number != frame.command_number_ac)
        return false;
    command.movement.forward_move = field<float>(&frame, 0x78);
    command.movement.side_move = field<float>(&frame, 0x80);
    client::reorient_command_movement(command, field<std::array<float, 3>>(&frame, 0x118),
                                      frame.state_104, (frame.flags & 0x40000U) != 0, true,
                                      b.button_policy, b.command_trigonometry);
    client::sanitize_command(command, (frame.flags & 8U) != 0, b.command_environment);
    client::save_final_command(command);
    client::publish_verified_command(verified, command, b.checksum);
    client::invalidate_command_prediction(b.prediction);
    return true;
}
}
RageSpreadSample preamble_pattern_correction(const SpreadParameters& parameters, std::uint32_t seed,
                                             const SpreadRandom& random,
                                             const ShotgunPattern& pattern,
                                             const RageTrigonometry& trig) {
    if (!pattern.get || !parameters.shotgun_patterns || parameters.pellet_count <= 1)
        return {0.0f, 0.0f};
    random.seed(random.context, std::bit_cast<std::int32_t>(seed + 1U));
    const auto recoil = _mm_cvttss_si32(_mm_set_ss(parameters.recoil_index));
    const auto first = std::uint32_t(recoil) * std::uint32_t(parameters.pellet_count);
    RageSpreadSample sum{0.0f, 0.0f};
    for (std::int32_t i = 0; i < parameters.pellet_count; ++i) {
        random.uniform(random.context, 0.0f, 1.0f);
        random.uniform(random.context, 0.0f, 6.283185482025146484375f);
        float angle = 0.0f, radius = 0.0f;
        pattern.get(pattern.context, parameters.weapon_id, parameters.fire_mode,
                    std::bit_cast<std::int32_t>(first + std::uint32_t(i)), angle, radius);
        const auto sine = trig.sine(angle);
        const auto cosine = trig.cosine(angle);
        const auto value = transform_spread_radius(parameters.weapon_id, parameters.fire_mode,
                                                   parameters.recoil_index, radius) *
                           parameters.spread;
        sum.second = value * sine + sum.second;
        sum.first = cosine * value + sum.first;
    }
    const float inverse = 1.0f / float(parameters.pellet_count);
    return {inverse * sum.first, sum.second * inverse};
}
PreambleTailResult finish_rage_command(PreambleState& state, PreambleResult& result,
                                       client::CommandFrame& original, void* player,
                                       const PreambleTailBindings& b, PreambleTailEntry entry) {
    PreambleTailResult tail;
    if (entry == PreambleTailEntry::return_only)
        return tail;
    auto& frame = *result.frame;
    if (entry == PreambleTailEntry::after_firing && &frame == &original &&
        field<std::int32_t>(&frame, 0xC8) != result.tick_base && !(state.flags & 0x800U)) {
        store(&frame, 0xC8, result.tick_base);
        b.shared.prediction_time = float(result.tick_base) * b.shared.interval;
    }
    if (entry == PreambleTailEntry::after_firing || entry == PreambleTailEntry::actions) {
        update_action_state(b.action_state, frame, player, *result.command, b.action);
        if (shot_path(frame, *result.command) && frame.weapon_address) {
            const auto weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
            if (lagcomp::history_weapon_is_firearm(
                    field<std::uint16_t>(weapon, b.shared.offsets.weapon_identifier))) {
                b.shared.selected_marker = frame.command_number_ac;
                tail.shot_marker_updated = true;
                correct_shot_angles(state, result, player, b);
            }
        }
    }
    if (entry != PreambleTailEntry::prediction_clock)
        tail.historical_command_published = publish_historical_command(result, original, b);
    // The native local at stack+120 starts at FLT_MAX and is overwritten by
    // 4340310B's returned snapshot pointer, including null, on frame selection.
    if (std::bit_cast<float>(result.prediction_clock_marker) != std::numeric_limits<float>::max())
        b.shared.prediction_time = 1.0f;
    return tail;
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: prediction_mask_owner.cpp
// ============================================================================

namespace skeet::rage {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_PREDICTION_MEMBER __thiscall
#define SKEET_PREDICTION_FAST __fastcall
#else
#define SKEET_PREDICTION_MEMBER
#define SKEET_PREDICTION_FAST
#endif
using Entry = void(SKEET_PREDICTION_MEMBER*)(void*);
void SKEET_PREDICTION_MEMBER prediction_mask_destroy_entry(void*);
void SKEET_PREDICTION_MEMBER prediction_mask_reset_entry(void*);
void SKEET_PREDICTION_MEMBER prediction_mask_forward_reset(void*);
void SKEET_PREDICTION_MEMBER prediction_mask_unchanged(void*) {}
void SKEET_PREDICTION_FAST update_entry(void*, std::uint8_t);
const std::array<Entry, 9> prediction_mask_primary{prediction_mask_destroy_entry, prediction_mask_unchanged,     prediction_mask_unchanged,
                                                   prediction_mask_unchanged,     prediction_mask_unchanged,     prediction_mask_reset_entry,
                                                   prediction_mask_unchanged,     prediction_mask_forward_reset, prediction_mask_unchanged}; //4347031C
const engine::FrameUpdateEntry secondary[]{update_entry}; //43470174
}

class PredictionMaskOwner {
public:
    const Entry* vtable = nullptr;
    std::uintptr_t preserved_second_word;
    engine::FrameUpdateEndpoint endpoint;
    PredictionMaskState masks;
    PredictionMaskFactoryBindings* bindings;

    explicit PredictionMaskOwner(PredictionMaskFactoryBindings& source) : bindings(&source) {
        source.lifecycle.append(this);
        engine::register_frame_update(source.frames, &endpoint);
        vtable = prediction_mask_primary.data();
        endpoint.vtable = secondary;
        // Constructor does not reset the masks;43387448 zeroed the allocation.
    }
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(PredictionMaskOwner, endpoint) == 8);
static_assert(offsetof(PredictionMaskOwner, masks) == 0xC);
static_assert(offsetof(PredictionMaskOwner, bindings) == 0x20);
#endif
namespace {
void SKEET_PREDICTION_MEMBER prediction_mask_destroy_entry(void* raw) {
    auto* owner = static_cast<PredictionMaskOwner*>(raw);
    auto& bindings = *owner->bindings;
    destroy_prediction_mask_owner(owner, bindings.active, bindings.lifecycle.memory);
}
void SKEET_PREDICTION_MEMBER prediction_mask_reset_entry(void* raw) {
    reset_prediction_masks(static_cast<PredictionMaskOwner*>(raw)->masks);
}
void SKEET_PREDICTION_MEMBER prediction_mask_forward_reset(void* raw) {
    engine::invoke<void>(raw, 5); //433CF2F5 calls the current slot5.
}
void SKEET_PREDICTION_FAST update_entry(void* raw, std::uint8_t update) {
    auto* owner = reinterpret_cast<PredictionMaskOwner*>(static_cast<std::byte*>(raw) -
                                                         offsetof(PredictionMaskOwner, endpoint));
    owner->bindings->runtime.update(owner->masks, update);
}
void initialize_entry(void* raw) {
    create_prediction_mask_owner(*static_cast<PredictionMaskFactoryBindings*>(raw));
}
#undef SKEET_PREDICTION_MEMBER
#undef SKEET_PREDICTION_FAST
}

PredictionMaskOwner* create_prediction_mask_owner(PredictionMaskFactoryBindings& source) {
    const auto memory = source.lifecycle.memory;
    auto* allocation = engine::allocate_owner_storage(memory, sizeof(PredictionMaskOwner));
    if (!allocation) {
        source.active = nullptr;
        return nullptr;
    }
    auto* owner = ::new (allocation) PredictionMaskOwner(source);
    source.active = owner;
    return owner;
}
void destroy_prediction_mask_owner(PredictionMaskOwner* owner, PredictionMaskOwner*& active,
                                   engine::JobMemory memory) {
    active = nullptr; //433BFE13 clears the singleton before433837D0's null test.
    if (!owner)
        return;
    owner->~PredictionMaskOwner();
    engine::release_owner_storage(memory, owner);
}
PredictionMaskState& prediction_mask_state(PredictionMaskOwner& owner) noexcept {
    return owner.masks;
}
engine::FrameUpdateEndpoint& prediction_mask_endpoint(PredictionMaskOwner& owner) noexcept {
    return owner.endpoint;
}
void bind_prediction_mask_factory(client::NativeStartupSources& sources,
                                  PredictionMaskFactoryBindings& bindings) {
    sources.bind_initializer({0x4347EA07u, &bindings, initialize_entry});
}
}

// ============================================================================
// Subcomponent: prediction_mask_runtime_owner.cpp
// ============================================================================

namespace skeet::rage {
namespace {
using client::ClientRuntimeStorage;
std::uint32_t prediction_field(ClientRuntimeStorage& s, std::uint32_t hash) {
    for (const auto& row : engine::build_receive_offset_recipes())
        if (row.initial_xor_key == hash)
            return std::uint32_t(std::ptrdiff_t(
                engine::DecodedOffset(s.offsets().globals, row.pointer_identity,
                                      row.xor_key_identity, row.initial_xor_key, 0)));
    throw std::invalid_argument("Prediction-mask field has no recovered receive recipe");
}
ClientRuntimeStorage& checked(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
                              client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
                              engine::SceneQueryRuntimeOwner& q, esp::EspRuntimeOwner& esp) {
    if (&e.variables.data() != &s.data() || &c.storage() != &s || &h.storage() != &s ||
        &q.storage() != &s || &q.players() != &h.players() || &esp.storage() != &s ||
        &esp.active() != &s.data().native_cell<esp::EspOwnerRuntime*>(0x434758DCu))
        throw std::invalid_argument(
            "Prediction masks require canonical engine, configuration, history, scene and late ESP owners");
    if (!s.data().native_cell<void*>(0x4346A514u) || !s.data().native_cell<void*>(0x4346A890u) ||
        !s.data().native_cell<void*>(0x43469300u) || !s.data().native_cell<void*>(0x43475B90u))
        throw std::invalid_argument(
            "Prediction masks require actual engine, entity-limit and filter publications");
    return s;
}
engine::OwnerRegistryBinding prediction_registry(ClientRuntimeStorage& s, std::uint32_t identity) {
    return {s.owner_registries().at(identity), s.memory()};
}
}
struct PredictionMaskRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    LiveTargetRelationships targeting;
    anti_aim::RawTargetTraceEntities entities;
    LivePredictionMaskBindings live;
    PredictionMaskFactoryBindings factory;
    Data(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
         client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
         engine::SceneQueryRuntimeOwner& q, esp::EspRuntimeOwner& esp)
        : storage(checked(s, e, c, h, q, esp)),
          targeting{s.data().native_cell<void*>(0x4346A890u), h.game_type(),
                    h.teammates_are_enemies(), h.players().adjustments(), h.target_offsets()},
          entities(s.data().native_cell<void*>(0x4346A890u), prediction_field(s, 0xE4E2D1A7u),
                   prediction_field(s, 0x49D93E62u) - 0x18u),
          live{s.data().native_cell<void*>(0x4346A514u),
               s.data().native_cell<void*>(0x43475808u),
               targeting,
               c.rage_settings().global.quick_stop.value,
               reinterpret_cast<const std::uint32_t&>(
                   c.rage_settings().global.quick_stop_options.value),
               s.data().native_cell<std::int32_t*>(0x43469300u),
               esp.active(),
               q.queries().ballistics(),
               s.data().native_cell<void**>(0x43475B90u),
               entities},
          factory{prediction_registry(s, 0x43475B78u), prediction_registry(s, 0x43475AD8u), live,
                  s.data().native_cell<PredictionMaskOwner*>(0x43475A58u)} {}
};
PredictionMaskRuntimeOwner::PredictionMaskRuntimeOwner(ClientRuntimeStorage& s,
                                                       const client::EngineRuntimeBindings& e,
                                                       client::ConfigurationRuntimeOwner& c,
                                                       client::HistoryRuntimeOwner& h,
                                                       engine::SceneQueryRuntimeOwner& q,
                                                       esp::EspRuntimeOwner& esp)
    : data_(std::make_unique<Data>(s, e, c, h, q, esp)) {}
PredictionMaskRuntimeOwner::~PredictionMaskRuntimeOwner() = default;
ClientRuntimeStorage& PredictionMaskRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
PredictionMaskFactoryBindings& PredictionMaskRuntimeOwner::factory_bindings() noexcept {
    return data_->factory;
}
void PredictionMaskRuntimeOwner::bind_initializer(client::NativeStartupSources& sources) {
    bind_prediction_mask_factory(sources, data_->factory);
}
} //namespace skeet::rage

// ============================================================================
// Subcomponent: prediction_masks.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* [stripped helper] */
bool incoming_shot_has_no_damage(void* shooter, const BallisticVector& origin,
                                 const BallisticVector& destination, PredictionMaskPlayers players,
                                 BallisticEngine& ballistics, void* const* trace_filter_vtable,
                                 const anti_aim::TargetTraceEntities& trace_entities) {
    auto& source = players.get();
    const auto metadata = esp::player_weapon_metadata(*source.identity(shooter), source);
    BallisticQuery query;
    query.allow_penetration = 1;
    reset_ballistic_query(query); //433D0015 ->433FCFD4
    query.weapon_id = metadata.definition;
    query.weapon_padding = metadata.padding;
    query.penetration = metadata.penetration;
    query.range = metadata.range;
    query.range_modifier = metadata.range_modifier;
    query.armor_ratio = metadata.armor_ratio;
    query.base_damage = metadata.damage;
    query.reserved = static_cast<std::uint32_t>(metadata.kind);
    anti_aim::TargetTraceFilter filter(trace_filter_vtable, trace_entities, 0x600400Bu, 0x600400Bu,
                                       1.0f);
    trace_ballistic_bullet(ballistics,
                           static_cast<BallisticEntity>(reinterpret_cast<std::uintptr_t>(shooter)),
                           origin, destination, filter.binding(), query);
    // Native CVTTSS2SI: fractional damage below1 sets the bit; NaN does not.
    return _mm_cvtt_ss2si(_mm_set_ss(query.damage)) == 0;
}
void assign_bit(PredictionMaskState& masks, bool blocked, std::uint32_t index, bool set) {
    const auto bit = 1u << (index & 31u);
    auto& word = index == 64 ? (blocked ? masks.trailing_word : masks.blocked[0])
                             : (blocked ? masks.blocked[index >> 5] : masks.allowed[index >> 5]);
    word = set ? word | bit : word & ~bit;
}
BallisticVector local_probe(void* player, float height) {
    const auto* origin = engine::invoke<const float*>(player, 10);
    return {origin[0] + 0.0f, origin[1] + 0.0f, origin[2] + height};
}
}

esp::EnginePlayers& PredictionMaskPlayers::get() const {
    if (const auto* value = std::get_if<std::reference_wrapper<esp::EnginePlayers>>(&source_))
        return value->get();
    auto* owner = std::get<std::reference_wrapper<esp::EspOwnerRuntime* const>>(source_).get();
    if (!owner)
        throw std::logic_error(
            "Prediction-mask weapon metadata requires the actual published ESP owner");
    return owner->player_engine();
}

void reset_prediction_masks(PredictionMaskState& masks) noexcept {
    masks.allowed.fill(0);
    masks.blocked.fill(0);
}
namespace {
template <class Limit>
void update_ready_masks(PredictionMaskState& masks, const TargetBindings& targeting,
                        const std::uint8_t& enabled, const std::uint32_t& flags, Limit entity_limit,
                        PredictionMaskPlayers players, BallisticEngine& ballistics,
                        void* const* trace_filter_vtable,
                        const anti_aim::TargetTraceEntities& trace_entities) {
    auto* local = targeting.local_player;
    const auto& offsets = targeting.offsets;
    if (!local || field<std::uint8_t>(local, offsets.rejection_bytes[0]) || !enabled ||
        !(flags & 0x20u)) {
        reset_prediction_masks(masks);
        return;
    }
    // The initial origin getter is observable even when the entity limit is0.
    engine::invoke<const float*>(local, 10);
    const auto limit = entity_limit();
    if (limit > 64)
        throw std::out_of_range(
            "Prediction-mask engine limit exceeds the recovered64-player object contract");
    for (std::int32_t index = 1; index <= limit; ++index) {
        auto* target = engine::invoke<void*>(targeting.entity_list, 3, index);
        if (!target_is_eligible(targeting, target)) {
            assign_bit(masks, false, index, false);
            assign_bit(masks, true, index, false);
            continue;
        }
        if (!(field<std::uint8_t>(target, offsets.movement_flags) & 1u))
            continue;
        const auto origin = field<BallisticVector>(target, offsets.origin);
        const BallisticVector start{origin[0] + 0.0f, origin[1] + 0.0f, origin[2] + 64.0f};
        assign_bit(masks, true, index,
                   incoming_shot_has_no_damage(target, start, local_probe(local, 48.0f), players,
                                               ballistics, trace_filter_vtable, trace_entities));
        assign_bit(masks, false, index,
                   incoming_shot_has_no_damage(target, start, local_probe(local, 68.0f), players,
                                               ballistics, trace_filter_vtable, trace_entities));
    }
}
}
void update_prediction_masks(PredictionMaskState& masks, std::uint8_t update,
                             const PredictionMaskBindings& b) {
    if (!engine::invoke<bool>(b.engine_client, 26) || !update)
        return;
    update_ready_masks(
        masks, b.targeting, b.enabled, b.context_flags, [&] { return b.entity_limit; }, b.players,
        b.ballistics, b.trace_filter_vtable, b.trace_entities);
}
void update_prediction_masks(PredictionMaskState& masks, std::uint8_t update,
                             const LivePredictionMaskBindings& b) {
    if (!engine::invoke<bool>(b.engine_client, 26) || !update)
        return;
    const auto targeting = b.targeting.capture(b.local_player);
    update_ready_masks(
        masks, targeting, b.enabled, b.context_flags, [&] { return *b.entity_limit; },
        PredictionMaskPlayers(b.players), b.ballistics, b.trace_filter_vtable, b.trace_entities);
}
void PredictionMaskRuntimeBinding::update(PredictionMaskState& masks, std::uint8_t update) const {
    std::visit([&](auto selected) { update_prediction_masks(masks, update, selected.get()); },
               source_);
}
}

// ============================================================================
// Subcomponent: record_candidates.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
/* [stripped helper] */
/* [dedup sub] */
std::int32_t absolute(std::int32_t value) noexcept {
    return value < 0 ? sub(0, value) : value;
}
/* deduplicated helper */
std::int32_t convar_integer(const void* object) noexcept {
    for (;;) {
        const auto parent = field<const void*>(object, 0x1c);
        if (parent == object)
            break;
        object = parent;
    }
    return integer(field<std::uint32_t>(object, 0x30) ^
                   std::uint32_t(reinterpret_cast<std::uintptr_t>(object)));
}
std::int32_t rounded(float value) noexcept {
    return _mm_cvtss_si32(_mm_set_ss(value));
}
float converted(std::int32_t value) noexcept {
    return _mm_cvtss_f32(_mm_cvtsi32_ss(_mm_setzero_ps(), value));
}
std::int32_t ceiling_tick(float value) noexcept {
    const auto result = rounded(value);
    return add(result, value > converted(result) ? 1 : 0);
}
std::int32_t floor_tick(float value) noexcept {
    const auto result = rounded(value);
    return sub(result, converted(result) > value ? 1 : 0);
}
std::int32_t cap_gap(std::int32_t value, std::int32_t limit) noexcept {
    if (limit < 1)
        return limit;
    return std::clamp(value, 1, limit);
}
lagcomp::LagTimeline timeline_view(const client::PlayerState& player) noexcept {
    lagcomp::LagTimeline out{};
    out.simulation_time = field<float>(&player.native, 0x110);
    out.last_update_tick = field<std::int32_t>(&player.native, 0x168);
    out.packet_head = field<std::uint32_t>(&player.native, 0x234);
    out.packet_count = field<std::uint32_t>(&player.native, 0x238);
    for (std::size_t i = 0; i < 6; ++i)
        out.packets[i] = field<lagcomp::PacketTimingSample>(&player.native, 0x23c + i * 0x20);
    out.record_head = player.history.first;
    out.record_count = player.history.count;
    for (std::size_t i = 0; i < 32; ++i)
        out.records[i] = player.history.records[i];
    return out;
}
struct Window {
    float low, high;
    std::int32_t corrected, upper;
};
Window candidate_window(const client::CommandFrame& frame,
                        const RecordCandidateBindings& bindings) noexcept {
    const float maximum = converted(bindings.maximum_unlag_ms) * 0.001F;
    const float delay =
        minimum_number(field<float>(&frame, 0x198) + bindings.interpolation, maximum);
    const float low = ((bindings.current_time - delay) - 0.2F) / bindings.interval;
    const float high = ((bindings.current_time - delay) + 0.2F) / bindings.interval;
    const lagcomp::HistoryTickTiming timing{
        bindings.current_time,   field<float>(&frame, 0x198), bindings.interpolation,
        bindings.interval,       bindings.maximum_unlag_ms,   field<std::int32_t>(&frame, 0xd4),
        integer(frame.field_18c)};
    return {low, high, lagcomp::history_tick_window(timing).corrected,
            add(field<std::int32_t>(&frame, 0x144), integer(frame.field_18c))};
}
std::int32_t extrapolated_tick(const client::CommandFrame& frame, void* target,
                               const client::PlayerState& player,
                               const RecordCandidateBindings& bindings, const Window& window) {
    const auto& ring = player.history;
    const auto& newest = ring.at(0).header;
    const auto now = add(field<std::int32_t>(&frame, 0xd4), integer(frame.field_18c));
    const auto arrival = lagcomp::receive_time_to_tick(
        engine::invoke<float>(bindings.engine_client, 14), bindings.receive_interval);
    auto receive_position = field<std::int32_t>(&player.native, 0x168);
    const auto arrival_gap = sub(arrival, receive_position);
    if (absolute(sub(now, newest.simulation_tick)) > 127)
        return -1;
    const auto timeline = timeline_view(player);
    const auto upper = std::min(window.upper, floor_tick(window.high));
    const auto lower = ceiling_tick(window.low);
    const auto simulation = lagcomp::time_to_tick(timeline.simulation_time, bindings.interval);
    bool simple = ring.count < 2 || timeline.packet_count < 2;
    if (!simple && simulation < sub(timeline.last_update_tick, 1) &&
        simulation < newest.simulation_tick)
        simple = true;
    if (!simple && lagcomp::packet_timing_discontinuity(timeline))
        simple = true;
    if (!simple && (newest.flags & 0x200U))
        simple = true;
    if (!simple) {
        const auto first = timeline.packet_head % 6U, next = (timeline.packet_head + 1U) % 6U;
        if (sub(timeline.packets[first].field_18, timeline.packets[next].field_18) < 4 &&
            sub(newest.receive_tick, ring.at(1).header.receive_tick) > 7)
            simple = true;
    }
    std::array<std::int32_t, 10> pattern{};
    std::size_t count = 0;
    if (simple)
        pattern[count++] = 2;
    else {
        const lagcomp::LagRecordHeader* previous = nullptr;
        for (std::uint32_t age = 1; age < ring.count; ++age) {
            const auto& record = ring.at(age).header;
            if (previous && !(previous->flags & 8U)) {
                pattern[count++] = cap_gap(
                    sub(previous->simulation_tick, record.simulation_tick),
                    (bindings.live_budget ? client::query_command_budget(*bindings.live_budget)
                                          : bindings.prediction_limit.value()));
                if (count == 10)
                    break;
            }
            if (record.flags & 0x200U)
                break;
            previous = &record;
        }
        const auto newest_gap =
            cap_gap(sub(newest.simulation_tick, ring.at(1).header.simulation_tick),
                    (bindings.live_budget ? client::query_command_budget(*bindings.live_budget)
                                          : bindings.prediction_limit.value()));
        const auto received = lagcomp::receive_time_to_tick(
            engine::invoke<float>(bindings.engine_client, 14), bindings.receive_interval);
        const auto receive_gap =
            cap_gap(add(sub(received, newest.receive_tick), 1),
                    (bindings.live_budget ? client::query_command_budget(*bindings.live_budget)
                                          : bindings.prediction_limit.value()));
        if (count < 10)
            pattern[count++] = std::max(newest_gap, receive_gap);
    }
    std::uint32_t ordinal = 0;
    auto result = newest.simulation_tick;
    while (result < window.corrected || result < lower || result > upper) {
        const auto step = pattern[ordinal % count];
        receive_position = add(receive_position, step);
        const auto proposed = add(result, step);
        if (receive_position > now)
            break;
        if (receive_position == now) {
            const auto index =
                engine::invoke<std::int32_t>(static_cast<std::byte*>(target) + 8, 10);
            if (engine::invoke<std::int32_t>(bindings.engine_client, 12) < index)
                break;
        }
        if (upper <= proposed)
            break;
        ++ordinal;
        result = proposed;
    }
    result = std::min(result, upper);
    if (newest.simulation_tick < result && arrival_gap > 2)
        return -1;
    return result;
}
void append_masked_ticks(const lagcomp::RecordRing& ring, std::uint32_t mask, std::uint32_t maximum,
                         std::int32_t preferred, std::vector<std::int32_t>& output) {
    const auto emit = [&](std::uint32_t age) {
        output.push_back(ring.at(age).header.simulation_tick);
        mask &= ~(1U << (age & 31U));
    };
    if (maximum > 3) {
        const auto original_count = std::popcount(mask);
        if (std::uint32_t(original_count) <= maximum) {
            for (std::uint32_t i = 0; i < ring.count; ++i)
                if (mask & (1U << (i & 31U)))
                    emit(i);
            return;
        }
        for (std::uint32_t i = 0; i < ring.count; ++i)
            if (mask & (1U << (i & 31U))) {
                emit(i);
                if (output.size() > 1)
                    break;
            }
        if (!mask || output.size() >= maximum)
            return;
        for (std::uint32_t i = ring.count; i > 0; --i)
            if (mask & (1U << ((i - 1U) & 31U))) {
                emit(i - 1U);
                if (output.size() > 3)
                    break;
            }
        if (!mask || output.size() >= maximum)
            return;
        for (std::uint32_t i = 0; i < ring.count; ++i)
            if ((mask & (1U << (i & 31U))) && (ring.at(i).header.flags & 8U)) {
                emit(i);
                break;
            }
        if (!mask || output.size() >= maximum)
            return;
        for (std::uint32_t i = 0; i < ring.count; ++i)
            if ((mask & (1U << (i & 31U))) && (ring.at(i).header.flags & 0x20U)) {
                emit(i);
                break;
            }
        if (!mask || output.size() >= maximum)
            return;
        std::uint32_t ordinal = 0;
        for (std::uint32_t i = 0; i < ring.count; ++i)
            if (mask & (1U << (i & 31U))) {
                if (++ordinal >= std::uint32_t(original_count >> 1)) {
                    emit(i);
                    return;
                }
            }
        return;
    }
    if (preferred != 0 && maximum == 1)
        for (std::uint32_t i = 0; i < ring.count; ++i)
            if ((mask & (1U << (i & 31U))) && ring.at(i).header.simulation_tick == preferred) {
                emit(i);
                return;
            }
    // The first eligible tick is emitted even for maximum==0.
    for (std::uint32_t i = 0; i < ring.count; ++i)
        if (mask & (1U << (i & 31U))) {
            emit(i);
            break;
        }
    if (!mask || output.size() >= maximum)
        return;
    for (std::uint32_t i = ring.count - 1U; i > 0; --i)
        if (mask & (1U << (i & 31U))) {
            emit(i);
            break;
        }
    if (!mask || output.size() >= maximum)
        return;
    for (std::uint32_t i = 0; i < ring.count; ++i)
        if ((mask & (1U << (i & 31U))) && (ring.at(i).header.flags & 0x20U)) {
            emit(i);
            return;
        }
}
}
void append_target_record_ticks(const client::CommandFrame& frame, void* target,
                                client::PlayerState& player, std::uint32_t maximum,
                                const RecordCandidateBindings& bindings,
                                std::vector<std::int32_t>& output) {
    const auto& ring = player.history;
    if (!ring.count)
        return;
    const auto entity_index = engine::invoke<std::int32_t>(static_cast<std::byte*>(target) + 8, 10);
    if (convar_integer(bindings.lag_compensation) == 0 ||
        !target_history_is_usable(player.native, ring, frame, std::uint32_t(entity_index),
                                  bindings.interval)) {
        output.push_back(ring.at(0).header.simulation_tick);
        return;
    }
    const auto window = candidate_window(frame, bindings);
    const auto raw_first = add(ceiling_tick(window.low), 1);
    const auto first = std::max(window.corrected, raw_first);
    const auto last = std::min(window.upper, sub(floor_tick(window.high), 1));
    std::uint32_t mask = 0;
    bool extrapolate = false;
    auto previous = player.native.cached_origin;
    for (std::uint32_t age = 0; age < ring.count; ++age) {
        const auto& record = ring.at(age).header;
        const auto& origin = record.origin;
        const float dx = origin[0] - previous[0], dy = origin[1] - previous[1],
                    dz = origin[2] - previous[2];
        if ((dy * dy + dx * dx) + dz * dz > 4096.0F) {
            if (!mask)
                mask = 1;
            break;
        }
        if (record.simulation_tick <= last) {
            if (record.simulation_tick < first) {
                if (age == 0 && raw_first <= record.simulation_tick)
                    extrapolate = true;
                break;
            }
            mask |= 1U << (age & 31U);
        }
        previous = origin;
    }
    if (!mask) {
        if (!extrapolate || ring.count < 2 || (ring.at(0).header.flags & 0x200U))
            return;
        const auto tick = extrapolated_tick(frame, target, player, bindings, window);
        if (tick != -1)
            output.push_back(tick);
        return;
    }
    if (frame.weapon_kind_124 == 0 || frame.weapon_id_d8 == 0x1f) {
        const lagcomp::LagRecordHeader* chosen = &ring.at(0).header;
        float best = std::numeric_limits<float>::max();
        for (std::uint32_t age = 0; age < ring.count; ++age)
            if (mask & (1U << (age & 31U))) {
                const auto& record = ring.at(age).header;
                const float dx = record.origin[0] - frame.origin_dc[0],
                            dy = record.origin[1] - frame.origin_dc[1];
                const float dz =
                    (record.collision_max_z * 0.5F + record.origin[2]) - frame.origin_dc[2];
                const float distance = (dy * dy + dx * dx) + dz * dz;
                if (distance < best) {
                    best = distance;
                    chosen = &record;
                }
            }
        output.push_back(chosen->simulation_tick);
        return;
    }
    append_masked_ticks(ring, mask, maximum, field<std::int32_t>(&player.native, 0x194), output);
}
std::vector<std::int32_t> collect_target_record_ticks(const client::CommandFrame& frame,
                                                      void* target, client::PlayerState& player,
                                                      std::uint32_t maximum,
                                                      const RecordCandidateBindings& bindings) {
    std::vector<std::int32_t> output;
    append_target_record_ticks(frame, target, player, maximum, bindings, output);
    return output;
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: reset.cpp
// ============================================================================

namespace skeet::rage {
void reset_rage_state(const RageResetBindings& bindings) noexcept { //4340BCDA
    bindings.field_174 = -1;
    bindings.highlight.target_handle = -1;
    bindings.shot_index = 0;
    bindings.state.target_mask.fill(0);
    bindings.state.previous_targets.fill(0);
    bindings.state.completed.fill(0);
    bindings.state.spread.generation = UINT32_MAX;
    bindings.acquisition.queued.clear();
    bindings.state.evaluation_count = 0;
    bindings.state.frame_count = 0;
    bindings.state.frame_first = 0;
    publish_command_animation_angles(bindings.pending_angles, -1, {0.0f, 0.0f, 0.0f});
}

void reset_rage_command_owner(CommandOwner& owner, std::int32_t& field_174) noexcept {
    reset_rage_state({owner.state, owner.acquisition, owner.post_firing.shot_index, field_174,
                      owner.tail.action_state, owner.phase6.pending_angles});
}
}

// ============================================================================
// Subcomponent: runtime_owner.cpp
// ============================================================================

namespace skeet::rage {
namespace {
using client::ClientRuntimeStorage;
/* [stripped helper] */
struct DerivedField {
    std::uint32_t hash, pointer, key;
};
constexpr std::array derived_fields{DerivedField{0xB8969256u, 0x43467430u, 0x4346A718u},
                                    DerivedField{0x3D52A1A7u, 0x43467F4Cu, 0x4346A8B0u},
                                    DerivedField{0x1CAAF0A8u, 0x43467D78u, 0x43467B80u},
                                    DerivedField{0x17978090u, 0x43469064u, 0x4346A484u},
                                    DerivedField{0x2A671B42u, 0x43468D30u, 0x43468ED4u},
                                    DerivedField{0x3B1F6ECCu, 0x43468368u, 0x43467FACu},
                                    DerivedField{0x54783141u, 0x43468E7Cu, 0x434674A8u}};
std::ptrdiff_t storage_field(ClientRuntimeStorage& s, std::uint32_t hash) {
    const auto& offsets = s.offsets().globals;
    for (const auto& row : derived_fields)
        if (row.hash == hash)
            return engine::DecodedOffset(offsets, row.pointer, row.key, row.hash, 0);
    for (const auto& row : engine::build_receive_offset_recipes())
        if (row.initial_xor_key == hash)
            return engine::DecodedOffset(offsets, row.pointer_identity, row.xor_key_identity,
                                         row.initial_xor_key, 0);
    for (const auto& row : engine::build_data_map_offset_recipes())
        if (row.initial_xor_key == hash)
            return engine::DecodedOffset(offsets, row.pointer_identity, row.xor_key_identity,
                                         row.initial_xor_key, 0);
    throw std::invalid_argument("RAGE field has no recovered receive/datamap/derived recipe");
}
template <class T> T& pointed(ClientRuntimeStorage& s, std::uint32_t identity) {
    auto* pointer = s.data().native_cell<T*>(identity);
    if (!pointer)
        throw std::invalid_argument("RAGE requires its actual published engine alias");
    return *pointer;
}
bool convar_boolean(const void* variable) {
    for (;;) {
        const auto parent = read<const void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    return read<std::uint32_t>(variable, 0x30) !=
           std::uint32_t(reinterpret_cast<std::uintptr_t>(variable));
}
engine::OwnerRegistryBinding owner_registry(ClientRuntimeStorage& s, std::uint32_t identity) {
    return {s.owner_registries().at(identity), s.memory()};
}
ClientRuntimeStorage&
checked(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
        client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
        engine::SceneQueryRuntimeOwner& q, animation::RuntimeOwner& b,
        client::CommandCaptureRuntimeOwner& capture, client::MovementHudRuntimeOwner& m,
        client::TargetRefreshRuntimeOwner& refresh, client::CrtRuntimeOwner& crt,
        esp::EspRuntimeOwner& esp, effects::EffectsRuntimeOwner& effects,
        client::VisualRuntimeOwner& visual, script::SourceScriptRuntimeOwner& scripts) {
    if (&e.variables.data() != &s.data() || &c.storage() != &s || &h.storage() != &s ||
        &q.storage() != &s || &q.players() != &h.players() || &b.storage() != &s ||
        &b.players() != &h.players().cache() || &capture.storage() != &s || &m.storage() != &s ||
        &refresh.storage() != &s || &crt.storage() != &s || &esp.storage() != &s ||
        &effects.storage() != &s || &visual.storage() != &s || &scripts.storage() != &s)
        throw std::invalid_argument(
            "RAGE must share the canonical engine, configuration, history, scene, animation, command, movement, CRT, ESP, effects, HUD and Lua owners");
    for (const auto identity : {0x4346A338u, 0x43467EB8u, 0x4346A334u, 0x43469300u, 0x43475B2Cu,
                                0x434757DCu, 0x43467B04u, 0x4346A894u, 0x43475B90u, 0x43469264u,
                                0x43467118u, 0x434685FCu, 0x4346A614u, 0x43469330u})
        if (!s.data().native_cell<void*>(identity))
            throw std::invalid_argument("RAGE engine/interface publication is incomplete");
    for (const auto identity :
         {0x43469028u, 0x43468EA4u, 0x43468520u, 0x43467A78u, 0x43467C3Cu, 0x43467084u, 0x43468C90u,
          0x434670D0u, 0x43467E4Cu, 0x43467A54u, 0x43467FA0u, 0x43467990u, 0x4346A88Cu, 0x43468FB4u,
          0x43468F4Cu, 0x434674E4u})
        if (!e.variables.at(identity).get())
            throw std::invalid_argument("RAGE requires its genuinely acquired ConVars");
    if (s.data().native_cell<LifecycleOwner*>(0x43475810u))
        throw std::logic_error("RAGE source context must precede its feature factory");
    // These adapters inspect the already loaded supported engine image; they
    // execute no engine function and publish no captured client address.
    engine::initialize_build_hitbox_clip_bindings(s.data());
    engine::initialize_build_pose_range_bindings(s.data());
    client::initialize_source_command_checksum(s.data());
    return s;
}
PreambleOffsets preamble_fields(ClientRuntimeStorage& s) {
    return {storage_field(s, 0xB8969256u), storage_field(s, 0xCA5EC698u), storage_field(s, 0x49C770CAu),
            storage_field(s, 0x5C9F0FBBu), storage_field(s, 0xF71CE331u), storage_field(s, 0xDE530A8Bu),
            storage_field(s, 0xD2563274u), storage_field(s, 0xD720AEA6u), storage_field(s, 0xB538BBA5u),
            storage_field(s, 0x1CAAF0A8u)};
}
FiringPlayerOffsets firing_fields(ClientRuntimeStorage& s) {
    return {storage_field(s, 0x38CA5228u),
            engine::DecodedOffset(s.offsets().globals, 0x4346757Cu, 0x4346707Cu, 0x043FA575u, 0),
            storage_field(s, 0x334B460Cu),
            storage_field(s, 0x3D52A1A7u),
            storage_field(s, 0x32E4DAF1u),
            storage_field(s, 0xC2B6FB10u),
            storage_field(s, 0x1CAAF0A8u),
            storage_field(s, 0xE2FBE695u)};
}
ShotModelOffsets shot_model_fields(ClientRuntimeStorage& s) {
    return {storage_field(s, 0x32E4DAF1u), storage_field(s, 0x47877DEBu), storage_field(s, 0x38CA5228u),
            storage_field(s, 0x8481EA04u), storage_field(s, 0x57B2D535u), storage_field(s, 0xC2B6FB10u),
            storage_field(s, 0x950F7B20u), storage_field(s, 0x5173A531u), storage_field(s, 0xA25D8271u),
            storage_field(s, 0x2A671B42u), storage_field(s, 0x3B1F6ECCu), storage_field(s, 0xFC2723F7u),
            storage_field(s, 0x688E0292u), storage_field(s, 0x4999428Du)};
}
}

struct RuntimeOwner::Data final : CommandSource {
    ClientRuntimeStorage& storage;
    const client::EngineRuntimeBindings& engine;
    client::ConfigurationRuntimeOwner& configuration;
    client::HistoryRuntimeOwner& history;
    engine::SceneQueryRuntimeOwner& scene;
    animation::RuntimeOwner& animation;
    client::CommandCaptureRuntimeOwner& capture_owner;
    client::MovementHudRuntimeOwner& movement;
    client::TargetRefreshRuntimeOwner& refresh;
    client::CrtRuntimeOwner& crt;
    esp::EspRuntimeOwner& esp;
    effects::EffectsRuntimeOwner& effects;
    client::VisualRuntimeOwner& visual;
    script::SourceScriptRuntimeOwner& scripts;
    const CommandFrameInputs* explicit_inputs;
    InputBitmap input;
    ui::SettingsMenuValues menu;
    PreambleState state_{};
    AcquisitionStorage acquisition_{};
    std::int32_t shot_index_ = 0;
    esp::HitboxHighlightState highlight_{};
    FiringSharedState shared;
    engine::JobDispatcher jobs;
    engine::EyePoseParameterRuntime eye_pose;
    engine::PoseRangeRuntime pose_range;
    RageBoxClipper clipper;
    lagcomp::RotationMath rotation;
    esp::HitboxHighlightEngineContext highlight_context;
    OwnerBindings factory;

    Data(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
         client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
         engine::SceneQueryRuntimeOwner& q, animation::RuntimeOwner& b,
         client::CommandCaptureRuntimeOwner& capture, client::MovementHudRuntimeOwner& m,
         client::TargetRefreshRuntimeOwner& target, client::CrtRuntimeOwner& process,
         esp::EspRuntimeOwner& esp_owner, effects::EffectsRuntimeOwner& effects_owner,
         client::VisualRuntimeOwner& visual_owner, script::SourceScriptRuntimeOwner& lua,
         const CommandFrameInputs* inputs, RuntimeImports imports)
        : storage(checked(s, e, c, h, q, b, capture, m, target, process, esp_owner, effects_owner,
                          visual_owner, lua)),
          engine(e), configuration(c), history(h), scene(q), animation(b), capture_owner(capture),
          movement(m), refresh(target), crt(process), esp(esp_owner), effects(effects_owner),
          visual(visual_owner), scripts(lua), explicit_inputs(inputs),
          input(s.input_runtime().bitmap()), menu(c.settings_menu().values()),
          shared(FiringSharedState::Live{}, s.data().native_cell<std::int32_t>(0x434759CCu),
                 s.data().native_cell<RageVector>(0x434759D0u),
                 s.data().native_cell<std::int32_t>(0x434759C8u)),
          jobs(imports.thread_pool, s.memory()), eye_pose(s.data()), pose_range(s.data()),
          clipper(engine::native_hitbox_clipper(s.data())),
          rotation{b.trigonometry().sine, b.trigonometry().cosine,
                   lagcomp::native_history_numeric_bindings().arccosine,
                   animation::native_sse_atan2},
          highlight_context{esp_owner.active(),
                            s.offsets(),
                            s.data().native_cell<std::uint8_t>(0x4346A65Cu),
                            visual_owner.projection(),
                            s.data().native_cell<RageVector>(0x43479CCCu),
                            b.trigonometry(),
                            h.players().adjustments()},
          factory{owner_registry(s, 0x43475B78u),
                  s.command_phases(),
                  owner_registry(s, 0x43475A9Cu),
                  s.entity_observers(),
                  s.configuration(),
                  c.rage_settings().aimbot,
                  c.player_esp_settings().aimbot_visualization,
                  *this,
                  {lua.bridge(), c.rage_settings().global, c.rage_settings().aimbot,
                   c.other_esp_settings().feature_indicators, input, visual_owner.indicators(),
                   c.player_esp_settings().aimbot_visualization, highlight_, highlight_context},
                  {s.data().native_cell<void*>(0x4346A514u),
                   s.data().native_cell<void*>(0x4346A890u),
                   s.data().native_cell<void*>(0x4346A614u),
                   s.data().native_cell<std::int32_t*>(0x43469300u),
                   s.data().native_cell<void*>(0x43467478u),
                   {storage_field(s, 0x7501CB91u), storage_field(s, 0x8D1BD8D0u), storage_field(s, 0x47877DEBu),
                    storage_field(s, 0x9BB80163u)}}} {}

    PreambleState& state() noexcept override {
        return state_;
    }
    AcquisitionStorage& acquisition() noexcept override {
        return acquisition_;
    }
    std::int32_t& shot_index() noexcept override {
        return shot_index_;
    }
    esp::HitboxHighlightState& action_state() noexcept override {
        return highlight_;
    }
    std::array<animation::AnimationAngleSample, 2>& pending_angles() noexcept override {
        return storage.data().native_cell<std::array<animation::AnimationAngleSample, 2>>(
            0x434759DCu);
    }
    PredictionMaskOwner* const& prediction_masks() const noexcept override {
        return storage.data().native_cell<PredictionMaskOwner*>(0x43475A58u);
    }
    void require_factory(const AimbotSettings& a, const GlobalSettings& g,
                         const esp::AimbotVisualizationConfig& v,
                         const esp::HitboxHighlightState& h) const override {
        if (&a != &configuration.rage_settings().aimbot ||
            &g != &configuration.rage_settings().global ||
            &v != &configuration.player_esp_settings().aimbot_visualization || &h != &highlight_)
            throw std::invalid_argument(
                "RAGE factory must retain this source owner's actual configuration and highlight state");
    }
    CommandPhase2Bindings phase2_bindings() const {
        auto& s = storage;
        auto& data = s.data();
        return {configuration.rage_settings().global,
                input,
                data.native_cell<std::int32_t>(0x4347C9ECu),
                engine.variables.at(0x43468EA4u),
                data.native_cell<void**>(0x4346A3A4u),
                pointed<float>(s, 0x4346A338u),
                {capture_owner.firing().offsets, storage_field(s, 0x3D10B94Cu), storage_field(s, 0x49C770CAu)}};
    }
    CommandPhase6Bindings phase6_bindings() {
        return {storage.data().native_cell<const void**>(0x4346A3A4u), storage_field(storage, 0xD720AEA6u),
                storage_field(storage, 0x5C9F0FBBu), pending_angles()};
    }
    void phase2(void* player, void* weapon, client::Command& command, std::array<float, 3>& angles,
                std::uint8_t& send) override {
        process_rage_command_phase2(state_, player, weapon, command, angles, send,
                                    phase2_bindings());
    }
    void phase6(const client::CommandFrame& frame, const client::Command& command) override {
        process_rage_phase6(frame, command, phase6_bindings());
    }
    void capture(const client::CommandFrame& frame) noexcept override {
        capture_rage_command_frame(state_, frame);
    }
    struct Frame;
    void process(client::CommandFrame&, void*, client::Command&, std::uint8_t&) override;
};

// All physical views below live on this invocation's stack. The acquisition
// owner synchronously drains its jobs before Frame is destroyed.
struct RuntimeOwner::Data::Frame {
    Data& owner;
    CommandFrameInputs inputs;
    lagcomp::EntityPreparationLiveGlobals preparation_globals;
    lagcomp::HistoryTickLiveGlobals selection_globals;
    lagcomp::HistoryTickTiming timing;
    lagcomp::EntityPreparationContext preparation;
    TargetBindings targeting;
    anti_aim::RawTargetTraceEntities trace_entities;
    lagcomp::RecordAttachmentContext attachments;
    lagcomp::RecordSelectionContext selected_records;
    MovementExtrapolationBindings movement_extrapolation;
    lagcomp::ExtrapolatedRecordBindings extrapolation;
    lagcomp::HistoryAlternativeBindings alternative;
    lagcomp::HistorySelectionBindings history;
    PreambleBindings preamble;
    TargetPreparationBindings target_preparation;
    RecordCandidateBindings candidates;
    std::array<RageSpreadSample, 32> samples;
    ScanContext scanner;
    BodyAimProbeContext body_probe;
    TargetPredictionBindings prediction;
    AcquisitionRecordBindings records;
    SingleTargetBindings single;
    MultipleTargetBindings multiple;
    FinalTargetContextBindings final_target;
    FiringReadinessBindings readiness;
    FiringPostAngleBindings post_angles;
    PostFiringMovementBindings post_movement;
    ShotDiagnosticBindings diagnostics;
    resolver_runtime::RawAttemptEngine attempts;
    PendingShotPublicationBindings pending;
    ShotModelPublicationBindings shot_models;
    RageStopBindings stop;
    PostFiringBindings post_firing;
    ActionStateBindings action;
    client::CommandButtonPolicy button_policy;
    PreambleTailBindings tail;
    CommandPhase2Bindings phase2;
    CommandPhase6Bindings phase6;
    CommandOwner command;

    Frame(Data&, client::CommandFrame&, void*, client::Command&);
    RuntimeFrameBindings views() const noexcept {
        return {targeting, candidates, selected_records, extrapolation};
    }
};

namespace {
void* shot_registration_receiver(void* entity_list) {
    const auto* table = read<const std::byte*>(entity_list, 0);
    const auto* locator = read<const std::byte*>(table, -4);
    return static_cast<std::byte*>(entity_list) - read<std::uint32_t>(locator, 4);
}
PreambleAnimationHistory animation_history(ClientRuntimeStorage& s) {
    auto* active = s.data().native_cell<client::PredictionHistoryOwner*>(0x434759B4u);
    return PreambleAnimationHistory(
        reinterpret_cast<const std::byte*>(client::prediction_history_snapshots(active)));
}
}

RuntimeOwner::Data::Frame::Frame(Data& d, client::CommandFrame& frame, void* player,
                                 client::Command&)
    : owner(d),
      preparation_globals{d.storage.data().native_cell<void*>(0x43467B04u),
                          d.storage.data().native_cell<float*>(0x43467EB8u),
                          d.storage.data().native_cell<float*>(0x4346A334u),
                          d.storage.data().native_cell<lagcomp::TickTimingSamples>(0x43478C70u)},
      selection_globals{d.storage.data().native_cell<float*>(0x43467EB8u),
                        d.storage.data().native_cell<float>(0x43476928u),
                        d.storage.data().native_cell<float*>(0x4346A338u), d.menu.maximum_unlag},
      timing{pointed<float>(d.storage, 0x43467EB8u),
             read<float>(&frame, 0x198),
             selection_globals.interpolation,
             pointed<float>(d.storage, 0x4346A338u),
             d.menu.maximum_unlag,
             std::bit_cast<std::int32_t>(frame.field_d4),
             std::bit_cast<std::int32_t>(frame.field_18c),
             &selection_globals,
             &frame.field_d4,
             &frame.field_18c},
      preparation{d.history.entity_runtime(),
                  std::bit_cast<std::uint32_t>(pointed<float>(d.storage, 0x43467EB8u)),
                  read<float>(d.storage.data().native_cell<void*>(0x43467B04u), 0),
                  pointed<float>(d.storage, 0x4346A334u),
                  d.storage.data().native_cell<lagcomp::TickTimingSamples>(0x43478C70u),
                  nullptr,
                  &preparation_globals},
      targeting{player,
                d.storage.data().native_cell<void*>(0x4346A890u),
                d.history.game_type(),
                d.history.teammates_are_enemies(),
                d.history.players().adjustments(),
                d.history.target_offsets()},
      trace_entities{targeting.entity_list, std::uint32_t(storage_field(d.storage, 0xE4E2D1A7u)),
                     std::uint32_t(storage_field(d.storage, 0x49D93E62u) - 0x18)},
      attachments{d.animation.bones(),
                  inputs.attachment_matrices,
                  d.animation.bones().update_attachments,
                  storage_field(d.storage, 0x6E9852D6u),
                  storage_field(d.storage, 0x2A671B42u),
                  storage_field(d.storage, 0x4E66BB34u),
                  storage_field(d.storage, 0x83A6CF2Bu)},
      selected_records{d.history.players().cache(),
                       preparation,
                       timing,
                       d.rotation,
                       inputs.record_interpolation,
                       attachments,
                       d.storage.data().native_cell<std::uint32_t*>(0x43469060u)},
      movement_extrapolation{d.history.entity_runtime().offsets,
                             std::uint32_t(storage_field(d.storage, 0x7A2D60C1u)),
                             std::uint32_t(storage_field(d.storage, 0x4E66BB34u)),
                             std::uint32_t(storage_field(d.storage, 0x2A671B42u)),
                             std::uint32_t(storage_field(d.storage, 0x7090D8DDu)),
                             std::uint32_t(storage_field(d.storage, 0x52899DF3u)),
                             std::uint32_t(storage_field(d.storage, 0xD3D4230Du)),
                             std::nullopt,
                             &d.history.players().cache(),
                             std::nullopt,
                             std::nullopt,
                             std::nullopt,
                             &d.scene.queries().movement(),
                             lagcomp::native_history_animation_engine(
                                 d.storage.data().native_cell<void*>(0x434693F0u)),
                             d.storage.data().native_cell<std::int32_t*>(0x434678F8u),
                             d.storage.data().native_cell<float*>(0x43467EB8u),
                             d.storage.data().native_cell<float*>(0x4346A338u),
                             d.storage.data().native_cell<std::uint8_t*>(0x43467B24u),
                             d.history.entity_runtime().set_absolute_angles,
                             d.storage.data().native_cell<void*>(0x43467204u)},
      extrapolation{d.history.players().cache(), preparation, attachments, d.animation.bones(),
                    movement_extrapolation,      std::nullopt},
      alternative{d.history.entity_runtime().offsets,
                  {std::uint32_t(storage_field(d.storage, 0x36375C23u)),
                   std::uint32_t(storage_field(d.storage, 0x4E66BB34u)),
                   std::uint32_t(storage_field(d.storage, 0x2A671B42u)),
                   std::uint32_t(storage_field(d.storage, 0x3D10B94Cu)),
                   std::uint32_t(storage_field(d.storage, 0xE2FBE695u)),
                   std::uint32_t(storage_field(d.storage, 0xE23A9BE1u)),
                   std::uint32_t(storage_field(d.storage, 0x3EB71843u)),
                   std::uint32_t(storage_field(d.storage, 0x0176A381u))},
                  {d.storage.data().native_cell<void**>(0x4346A3A4u),
                   std::uint32_t(storage_field(d.storage, 0xBF567619u)),
                   std::uint32_t(storage_field(d.storage, 0xDDA99AB5u)),
                   std::uint32_t(storage_field(d.storage, 0xB7EA8FDAu)),
                   &d.storage.data().native_cell<std::int32_t>(0x43469274u)},
                  {d.engine.variables.at(0x43467E4Cu), d.engine.variables.at(0x43467A54u),
                   d.engine.variables.at(0x43467FA0u), d.engine.variables.at(0x43467990u),
                   d.engine.variables.at(0x4346A88Cu), d.engine.variables.at(0x43468FB4u),
                   d.engine.variables.at(0x43468F4Cu), d.engine.variables.at(0x434674E4u)},
                  d.storage.data().native_cell<void*>(0x4346A514u),
                  targeting.entity_list,
                  d.storage.data().native_cell<float*>(0x4346A338u),
                  d.storage.data().native_cell<float*>(0x4346A338u),
                  d.storage.data().native_cell<float*>(0x43467EB8u),
                  d.storage.data().native_cell<std::int32_t*>(0x434678F8u),
                  d.storage.data().native_cell<std::uint8_t*>(0x43467B24u),
                  d.storage.data().native_cell<std::uint8_t>(0x4347CA58u),
                  d.scene.queries().movement(),
                  d.storage.data().native_cell<std::uint8_t*>(0x434683D0u),
                  movement_extrapolation.animation_engine,
                  d.animation.trigonometry(),
                  animation::native_sse_atan2,
                  d.history.entity_runtime().set_absolute_angles,
                  d.storage.data().native_cell<void*>(0x43467204u),
                  inputs.alternative_animation},
      history{extrapolation,
              alternative,
              d.rotation,
              inputs.history_interpolation,
              pointed<float>(d.storage, 0x43467EB8u),
              selection_globals.interpolation,
              d.menu.maximum_unlag},
      preamble{
          d.configuration.rage_settings().global,
          d.configuration.rage_settings().aimbot,
          d.input,
          d.storage.data().native_cell<std::uint32_t>(0x434759C0u),
          d.engine.variables.at(0x43469028u),
          d.storage.data().native_cell<const void**>(0x4346A3A4u),
          preamble_fields(d.storage),
          std::span<client::Command, 150>(
              read<client::Command*>(d.storage.data().native_cell<void*>(0x4346A894u), 0xF0), 150),
          animation_history(d.storage),
          pointed<float>(d.storage, 0x43467EB8u),
          pointed<float>(d.storage, 0x4346A338u),
          d.configuration.rage_settings().global.silent_aim.value != 0,
          d.menu.hold_aim != 0,
          d.menu.hold_aim != 0,
          pointed<std::int32_t>(d.storage, 0x43475B2Cu),
          d.storage.data().native_cell<std::int32_t>(0x434759C4u),
          d.configuration.rage_settings().global.double_tap_hit_chance.value,
          convar_boolean(d.engine.variables.at(0x43468520u)),
          d.crt.spread().tables(),
          d.crt.random(),
          d.esp.shotgun_pattern(),
          d.animation.trigonometry(),
          d.storage.data().native_cell<anti_aim::TargetState>(0x4346DB14u),
          std::cref(d.refresh),
          d.history.entity_runtime().set_absolute_angles},
      target_preparation{targeting,
                         preamble.settings,
                         preamble.weapon_settings,
                         d.input,
                         d.state_,
                         preamble.yaw_target.handle,
                         d.engine.variables.at(0x43468EA4u),
                         {storage_field(d.storage, 0x1CAAF0A8u), storage_field(d.storage, 0x3D52A1A7u),
                          storage_field(d.storage, 0x32E4DAF1u), storage_field(d.storage, 0xC2B6FB10u),
                          storage_field(d.storage, 0x17978090u), storage_field(d.storage, 0x36375C23u),
                          storage_field(d.storage, 0xB538BBA5u)}},
      candidates{d.engine.variables.at(0x43467A78u),
                 d.storage.data().native_cell<void*>(0x4346A514u),
                 preamble.prediction_time,
                 preamble.interval,
                 d.history.entity_runtime().receive_interval,
                 selection_globals.interpolation,
                 d.menu.maximum_unlag,
                 std::nullopt,
                 &d.capture_owner.budget()},
      samples(hitchance_spread_samples(d.state_.spread)),
      scanner{d.scene.queries().ballistics(),
              targeting,
              BallisticEntity(reinterpret_cast<std::uintptr_t>(player)),
              reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address)),
              storage_field(d.storage, 0x5C9F0FBBu),
              preamble.settings.automatic_penetration.value,
              frame.weapon_id_d8,
              frame.origin_dc,
              std::bit_cast<float>(frame.field_160),
              frame.pellet_count_19c,
              std::bit_cast<std::int32_t>(frame.field_1d0),
              preamble.weapon_settings.minimum_hit_chance.value,
              d.state_.flags,
              d.state_.previous_targets,
              samples,
              {storage_field(d.storage, 0x32E4DAF1u), storage_field(d.storage, 0xC2B6FB10u),
               storage_field(d.storage, 0x17978090u), storage_field(d.storage, 0xA9EE6062u)},
              d.animation.trigonometry(),
              d.clipper,
              d.history.players().cache(),
              preparation,
              {storage_field(d.storage, 0x6E9852D6u), storage_field(d.storage, 0x2A671B42u),
               storage_field(d.storage, 0x4E66BB34u), storage_field(d.storage, 0x83A6CF2Bu)},
              d.animation.bones().update_attachments,
              {d.engine.variables.at(0x43467C3Cu), d.engine.variables.at(0x43467084u),
               d.engine.variables.at(0x43468C90u), d.history.teammates_are_enemies()},
              d.storage.data().native_cell<void**>(0x43475B90u),
              trace_entities},
      body_probe{d.scene.queries().movement(),
                 scanner.ballistics,
                 scanner.source,
                 scanner.weapon,
                 {storage_field(d.storage, 0xA9EE6062u), storage_field(d.storage, 0x36375C23u),
                  storage_field(d.storage, 0x32E4DAF1u), storage_field(d.storage, 0xC2B6FB10u),
                  storage_field(d.storage, 0x17978090u), storage_field(d.storage, 0x5C9F0FBBu)},
                 d.state_.flags,
                 d.state_.weapon_flags,
                 preamble.settings.automatic_penetration.value,
                 d.storage.data().native_cell<std::uint8_t*>(0x434683D0u),
                 scanner.trace_filter_vtable,
                 trace_entities},
      prediction{body_probe,
                 selected_records,
                 d.storage.data().native_cell<void*>(0x43468304u),
                 storage_field(d.storage, 0x3EB71843u),
                 preamble.scheduler_flags,
                 nullptr},
      records{d.history.players().cache(),
              candidates.lag_compensation,
              preamble.interval,
              preamble.yaw_target.handle,
              preamble.settings.body_aim_disablers.value,
              d.storage.data().native_cell<FeedbackOwner*>(0x43475A7Cu),
              history,
              extrapolation,
              scanner,
              d.menu,
              d.animation.bones(),
              d.movement.live_movement(),
              d.capture_owner.budget()},
      single{target_preparation,
             prediction,
             candidates,
             records,
             d.history.players().adjustments(),
             pointed<std::int32_t>(d.storage, 0x43469300u),
             d.jobs},
      multiple{target_preparation,
               body_probe,
               prediction,
               candidates,
               records,
               d.history.players().adjustments(),
               pointed<std::int32_t>(d.storage, 0x43469300u),
               d.jobs},
      final_target{scanner, d.storage.data().native_cell<std::uint32_t*>(0x43469060u), d.menu,
                   &d.jobs},
      readiness{
          d.history.players().cache(),
          d.storage.data().native_cell<void*>(0x4346A514u),
          candidates.lag_compensation,
          preamble.interval,
          selection_globals.interpolation,
          preamble.prediction_time,
          d.menu.maximum_unlag,
          0.5f,
          d.history.entity_runtime().receive_interval,
          preamble.settings.automatic_fire.value != 0,
          preamble.settings.silent_aim.value != 0,
          preamble.send_mode,
          d.configuration.anti_aim_global().body_yaw.value,
          preamble.scheduler_flags,
          preamble.selected_marker,
          preamble.current_sequence,
          frame.weapon_id_d8 == 64
              ? read<float>(reinterpret_cast<const void*>(std::uintptr_t(frame.weapon_address)),
                            d.capture_owner.firing().offsets.next_secondary)
              : frame.postpone_fire_1e4},
      post_angles{player,
                  firing_fields(d.storage),
                  d.storage.data().native_cell<void*>(0x4346A894u),
                  d.animation.bones(),
                  d.animation.bones(),
                  d.animation.bones(),
                  d.animation.trigonometry(),
                  {animation::native_sse_atan2},
                  d.eye_pose,
                  legit::recoil_compensation_enabled(
                      legit::legit_owner_settings(
                          d.storage.data().native_cell<legit::LifecycleOwner*>(0x4347588Cu)),
                      std::uint32_t(std::int32_t(std::int8_t(frame.weapon_kind_124))), d.input) ||
                      !preamble.settings.remove_recoil.value,
                  preamble.settings.automatic_fire.value != 0,
                  preamble.send_mode,
                  preamble.settings.remove_recoil.value != 0,
                  preamble.scheduler_flags,
                  preamble.prediction_time},
      post_movement{preamble.settings,
                    d.input,
                    d.state_.flags,
                    preamble.scheduler_flags,
                    d.state_.weapon_flags,
                    d.state_.target_mask,
                    targeting,
                    {storage_field(d.storage, 0xA9EE6062u), storage_field(d.storage, 0xDE530A8Bu),
                     storage_field(d.storage, 0xCACAEECCu), storage_field(d.storage, 0xC07D1150u)},
                    scanner.ballistics,
                    scanner.source,
                    scanner.weapon_item_offset,
                    scanner.trace_filter_vtable,
                    trace_entities,
                    {d.prediction_masks()},
                    preamble.prediction_time,
                    preamble.yaw_target.handle},
      diagnostics{d.storage.data().native_cell<client::BufferedShotLifecycleOwner*>(0x43475968u),
                  d.history.players().cache(),
                  {storage_field(d.storage, 0xD5FD2E2Cu), storage_field(d.storage, 0xE2FBE695u),
                   storage_field(d.storage, 0x3309C0CFu), storage_field(d.storage, 0x7F5ED2D0u)},
                  d.storage.data().native_cell<std::uint32_t*>(0x4346A334u),
                  preamble.interval,
                  d.storage.data().native_cell<void*>(0x43467B54u)},
      attempts{d.storage.data().native_cell<void*>(0x4346A890u),
               d.storage.data().native_cell<float*>(0x4346A334u),
               storage_field(d.storage, 0x9D920E35u),
               storage_field(d.storage, 0x3D52A1A7u),
               storage_field(d.storage, 0x32E4DAF1u),
               storage_field(d.storage, 0x1CAAF0A8u),
               storage_field(d.storage, 0xC2B6FB10u),
               d.pose_range,
               d.clipper},
      pending{d.storage.data().native_cell<FeedbackOwner*>(0x43475A7Cu),
              d.configuration.rage_settings().global.anti_aim_correction.value,
              d.history.players().cache(),
              d.storage.data().native_cell<void*>(0x4346A514u),
              targeting.entity_list,
              d.storage.data().native_cell<void*>(0x43467B54u),
              d.engine.variables.at(0x434670D0u),
              {storage_field(d.storage, 0xCA5EC698u), storage_field(d.storage, 0x47877DEBu),
               storage_field(d.storage, 0xC2B6FB10u)},
              d.storage.data().native_cell<float*>(0x4346A334u),
              d.storage.data().native_cell<float*>(0x43467EB8u),
              d.storage.data().native_cell<float*>(0x4346A338u),
              &preamble.scheduler_flags,
              attempts},
      shot_models{d.configuration.colored_model_settings(),
                  targeting.entity_list,
                  d.storage.data().native_cell<void*>(0x4346A614u),
                  d.storage.data().native_cell<void*>(0x43469330u),
                  read<ShotEntityCreate>(d.storage.data().native_cell<void*>(0x434685FCu), 0),
                  d.storage.data().native_cell<RegisterShotEntity>(0x4346A884u),
                  d.storage.data().native_cell<UnregisterShotEntity>(0x43468E88u),
                  shot_registration_receiver(targeting.entity_list),
                  shot_model_fields(d.storage),
                  d.storage.data().native_cell<InvalidateShotTransform>(0x43468C10u),
                  &d.storage.data().native_cell<engine::RecvVarProxy>(0x43476B44u),
                  d.storage.data().native_cell<std::uint32_t>(0x4346D674u),
                  d.effects.shot_state(),
                  d.effects.shot_bones(),
                  d.history.players().cache(),
                  preparation,
                  timing,
                  d.storage.data().native_cell<std::uint32_t*>(0x43469060u),
                  d.animation.bones(),
                  d.storage.data().native_cell<void*>(0x43467118u)},
      stop{d.movement.movement_environment()},
      post_firing{preamble,
                  post_movement,
                  d.history.players().cache(),
                  d.capture_owner.view_angles(),
                  final_target,
                  d.scripts.bridge(),
                  diagnostics,
                  pending,
                  shot_models,
                  d.shot_index_,
                  pointed<std::int32_t>(d.storage, 0x43469300u),
                  d.storage.data().native_cell<float>(0x43469254u),
                  nullptr,
                  stop,
                  d.movement.movement_capture(),
                  d.scene.queries().movement(),
                  prediction,
                  scanner,
                  d.animation.bones()},
      action{d.storage.data().native_cell<std::uint8_t>(0x4346A65Cu) != 0,
             d.configuration.player_esp_settings().aimbot_visualization,
             d.state_,
             preamble,
             target_preparation,
             d.history.players().adjustments(),
             pending.active,
             history,
             scanner,
             d.scene.queries().movement(),
             body_probe.trace_flag,
             d.animation.bones(),
             d.movement.live_movement(),
             d.capture_owner.budget()},
      button_policy{d.storage.data().native_cell<std::array<std::uint32_t, 150>>(0x43476090u),
                    d.configuration.anti_aim_other().leg_movement.value,
                    pointed<std::int32_t>(d.storage, 0x434757DCu)},
      tail{preamble,
           d.highlight_,
           action,
           legit::legit_owner_settings(
               d.storage.data().native_cell<legit::LifecycleOwner*>(0x4347588Cu)),
           {animation::native_sse_atan2},
           animation::native_sse_arctangent,
           d.storage.data().native_cell<void*>(0x4346A894u),
           post_angles.offsets,
           button_policy,
           {d.animation.trigonometry().sine, d.animation.trigonometry().cosine},
           d.capture_owner.angle_environment(),
           std::span<client::VerifiedCommand, 150>(
               read<client::VerifiedCommand*>(d.storage.data().native_cell<void*>(0x4346A894u),
                                              0xF4),
               150),
           d.storage.data().native_cell<client::CommandChecksum>(0x43467C48u),
           d.storage.data().native_cell<void*>(0x43469264u)},
      phase2(d.phase2_bindings()), phase6(d.phase6_bindings()),
      command{d.state_,  d.acquisition_, preamble,    single, multiple, final_target, d.shared,
              readiness, post_angles,    post_firing, tail,   phase2,   phase6,       d.menu} {
    // Native43344C0C/4336C572/4333DB01 own their automatic workspaces.
    // Production retains actual unwritten local bytes where native does;
    // only the explicit differential profile supplies a prior byte image.
    if (d.explicit_inputs)
        std::memcpy(&inputs, d.explicit_inputs, sizeof inputs);
}

void RuntimeOwner::Data::process(client::CommandFrame& frame, void* player,
                                 client::Command& command, std::uint8_t& send) {
    if (!player)
        throw std::invalid_argument("RAGE requires the actual current command player");
    Frame physical(*this, frame, player, command);
    process_rage_command(physical.command, frame, player, command, send);
}

RuntimeOwner::RuntimeOwner(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
                           client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
                           engine::SceneQueryRuntimeOwner& q, animation::RuntimeOwner& b,
                           client::CommandCaptureRuntimeOwner& capture,
                           client::MovementHudRuntimeOwner& m,
                           client::TargetRefreshRuntimeOwner& refresh, client::CrtRuntimeOwner& crt,
                           esp::EspRuntimeOwner& esp, effects::EffectsRuntimeOwner& effects,
                           client::VisualRuntimeOwner& visual,
                           script::SourceScriptRuntimeOwner& scripts, CommandFrameInputs& incoming)
    : data_(std::make_unique<Data>(s, e, c, h, q, b, capture, m, refresh, crt, esp, effects, visual,
                                   scripts, &incoming,
                                   RuntimeImports{engine::native_thread_pool()})) {}
RuntimeOwner::RuntimeOwner(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
                           client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
                           engine::SceneQueryRuntimeOwner& q, animation::RuntimeOwner& b,
                           client::CommandCaptureRuntimeOwner& capture,
                           client::MovementHudRuntimeOwner& m,
                           client::TargetRefreshRuntimeOwner& refresh, client::CrtRuntimeOwner& crt,
                           esp::EspRuntimeOwner& esp, effects::EffectsRuntimeOwner& effects,
                           client::VisualRuntimeOwner& visual,
                           script::SourceScriptRuntimeOwner& scripts)
    : RuntimeOwner(s, e, c, h, q, b, capture, m, refresh, crt, esp, effects, visual, scripts,
                   RuntimeImports{engine::native_thread_pool()}) {}
RuntimeOwner::RuntimeOwner(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
                           client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
                           engine::SceneQueryRuntimeOwner& q, animation::RuntimeOwner& b,
                           client::CommandCaptureRuntimeOwner& capture,
                           client::MovementHudRuntimeOwner& m,
                           client::TargetRefreshRuntimeOwner& refresh, client::CrtRuntimeOwner& crt,
                           esp::EspRuntimeOwner& esp, effects::EffectsRuntimeOwner& effects,
                           client::VisualRuntimeOwner& visual,
                           script::SourceScriptRuntimeOwner& scripts, RuntimeImports imports)
    : data_(std::make_unique<Data>(s, e, c, h, q, b, capture, m, refresh, crt, esp, effects, visual,
                                   scripts, nullptr, imports)) {}
RuntimeOwner::~RuntimeOwner() = default;
ClientRuntimeStorage& RuntimeOwner::storage() const noexcept {
    return data_->storage;
}
PreambleState& RuntimeOwner::state() noexcept {
    return data_->state_;
}
AcquisitionStorage& RuntimeOwner::acquisition() noexcept {
    return data_->acquisition_;
}
CommandEndpoint& RuntimeOwner::commands() noexcept {
    return data_->factory.commands;
}
OwnerBindings& RuntimeOwner::factory_bindings() noexcept {
    return data_->factory;
}
void RuntimeOwner::with_frame(client::CommandFrame& frame, void* player, client::Command& command,
                              const std::function<void(const RuntimeFrameBindings&)>& consume) {
    if (!player || !consume)
        throw std::invalid_argument(
            "RAGE frame composition requires an actual player and synchronous consumer");
    Data::Frame physical(*data_, frame, player, command);
    consume(physical.views());
}
const lagcomp::EvaluationState* RuntimeOwner::repair_buffered(client::CommandFrame& frame,
                                                              void* player,
                                                              client::Command& command,
                                                              const ShotDiagnosticEntry& diagnostic,
                                                              BufferedShotRepairScratch& scratch) {
    if (!storage().data().native_cell<LifecycleOwner*>(0x43475810u) || (frame.flags & 0x20040u))
        return nullptr;
    if (!player)
        throw std::invalid_argument(
            "Buffered RAGE reconstruction requires this invocation's actual local player");
    Data::Frame physical(*data_, frame, player, command);
    return repair_buffered_shot(frame, player, command, diagnostic, scratch,
                                BufferedShotRepairBindings(&physical.command));
}
void RuntimeOwner::bind_initializer(client::NativeStartupSources& sources) {
    if (&sources.data() != &storage().data())
        throw std::invalid_argument("RAGE initializer requires its canonical source data");
    sources.bind_initializer({0x4348DA25u, data_.get(), [](void* raw) {
                                  auto& d = *static_cast<Data*>(raw);
                                  (void)construct_rage_owner(
                                      d.factory,
                                      d.storage.data().native_cell<LifecycleOwner*>(0x43475810u));
                              }});
}

} //namespace skeet::rage

// ============================================================================
// Subcomponent: scan_hypotheses.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
/* deduplicated helper */
/* [dedup negate] */
}
float mean_scan_history_yaw_change(const client::NativePlayerStateLayout& history) noexcept {
    const auto count = history.field_238;
    if (count < 2)
        return 0.0f;
    const auto first = read<std::uint32_t>(&history, 0x234);
    float total = 0.0f, differences = 0.0f;
    for (std::uint32_t ordinal = 1; ordinal < count; ++ordinal) {
        const auto previous = (first + ordinal - 1u) % 6u, current = (first + ordinal) % 6u;
        const auto delta = read<float>(&history, 0x258 + previous * 0x20) -
                           read<float>(&history, 0x258 + current * 0x20);
        auto difference = normalize_movement_angle(delta);
        if (difference < 0.0f)
            difference = negate(difference);
        total += difference;
        differences += 1.0f;
    }
    return total / differences;
}

void refresh_scan_hypothesis_sets(const ScanHypothesisContext& context, void* target,
                                  const lagcomp::LagRecord& selected,
                                  std::span<HitboxMatrices, 3> matrices,
                                  std::span<Matrix3x4, 256> scratch) {
    const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(target) + 8, 10);
    auto& history = context.players.get(std::uint32_t(index))->native;
    const auto& common = context.preparation.entity.offsets;
    const auto first = read<std::uint32_t>(target, common.field_432b1f5b);
    const auto second = read<std::uint32_t>(target, common.field_0d560fac);
    const auto eye = read<std::array<std::uint32_t, 2>>(target, common.eye_angles);
    (void)lagcomp::prepare_entity_angles(context.players, target, selected, false,
                                         context.preparation);
    auto* collision = engine::invoke<void*>(target, 3);
    auto* maximum = engine::invoke<lagcomp::Vector*>(collision, 2);
    const auto height = (*maximum)[2];
    (*maximum)[2] = selected.header.collision_max_z;
    const auto duck = read<std::uint32_t>(target, common.duck_amount);
    write(target, common.duck_amount, history.cached_duck_amount);
    const auto saved_6e = read<std::uint32_t>(target, context.offsets.field_6e9852d6);
    const auto saved_2a = read<lagcomp::Vector>(target, context.offsets.field_2a671b42);
    write(target, context.offsets.field_2a671b42, selected.header.origin);
    const auto saved_4e = read<lagcomp::Vector>(target, context.offsets.field_4e66bb34);
    write(target, context.offsets.field_4e66bb34, selected.header.origin);
    const auto flags = read<std::uint32_t>(target, context.offsets.field_83a6cf2b);
    write(target, context.offsets.field_83a6cf2b, flags & 0xFFFFF7FFu);
    for (auto& set : matrices)
        legit::refresh_hitbox_attachments(target, context.model, set, scratch, common.bone_accessor,
                                          context.update_attachments);
    write(target, context.offsets.field_83a6cf2b, flags);
    write(target, context.offsets.field_4e66bb34, saved_4e);
    write(target, context.offsets.field_2a671b42, saved_2a);
    write(target, context.offsets.field_6e9852d6, saved_6e);
    write(target, common.duck_amount, duck);
    (*maximum)[2] = height;
    write(target, common.eye_angles + 4, eye[1]);
    write(target, common.eye_angles, eye[0]);
    write(target, common.field_0d560fac, second);
    write(target, common.field_432b1f5b, first);
}

void build_scan_hypotheses(const ScanHypothesisContext& context, void* target,
                           const lagcomp::LagRecord& selected, const lagcomp::LagRecord& source,
                           ScanHypothesisMatrices& matrices, std::span<Matrix3x4, 256> scratch) {
    constexpr std::array<float, 3> offsets{-60.0f, 0.0f, 60.0f};
    const auto eye_yaw = context.preparation.entity.offsets.eye_angles + 4;
    for (std::size_t group = 0; group < offsets.size(); ++group) {
        const auto yaw = read<float>(target, eye_yaw);
        write(target, eye_yaw, offsets[group] + yaw);
        for (std::size_t variant = 0; variant < 3; ++variant)
            std::memcpy(&matrices[group][variant], &source.alternatives[variant].matrices,
                        sizeof(HitboxMatrices));
        refresh_scan_hypothesis_sets(context, target, selected, matrices[group], scratch);
        write(target, eye_yaw, yaw);
    }
}

bool build_optional_scan_hypotheses(const ScanHypothesisContext& context, void* target,
                                    std::uint32_t entity_index, const lagcomp::LagRecord& selected,
                                    const lagcomp::LagRecord* source, bool enabled,
                                    std::uint32_t safe_hitboxes, ScanHypothesisMatrices& matrices,
                                    std::span<Matrix3x4, 256> scratch) {
    if (!enabled || !safe_hitboxes || !source || !(source->header.flags & 0x80u))
        return false;
    const auto& history = context.players.get(entity_index)->native;
    if (!(mean_scan_history_yaw_change(history) > 30.0f))
        return false;
    build_scan_hypotheses(context, target, selected, *source, matrices, scratch);
    return true;
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: scanning.cpp
// ============================================================================

namespace skeet::rage {
namespace {
std::vector<RageAimPoint> descriptors(const ScanContext& context, const ScanPolicy& policy,
                                      const lagcomp::LagRecord& record,
                                      const scan_detail::Model& local,
                                      const scan_detail::Geometry& geometry,
                                      const ScanHypothesisMatrices* additional) {
    constexpr unsigned order[]{3, 4, 0, 6, 8, 7, 10, 9, 17, 15, 18, 16, 11, 12};
    const RagePointGeneration settings{context.origin,
                                       policy.hitboxes.regular,
                                       policy.multipoint_mask,
                                       policy.point_scale,
                                       context.spread,
                                       context.accuracy_percent,
                                       (context.configuration_flags & 4U) != 0};
    std::vector<RageAimPoint> result;
    for (const auto index : order) {
        const auto regular =
            generate_rage_regular_hitbox_points(settings, index, local.hitboxes, geometry.primary);
        result.insert(result.end(), regular.begin(), regular.end());
        const auto& hitbox = geometry.primary[index];
        const auto group = index == 11 || index == 12
                               ? 0x200U
                               : 1U << (std::uint32_t(local.hitboxes[index].hitgroup) & 31U);
        if (!(policy.hitboxes.safe & group) || !(hitbox.capsule.radius > 0.0f))
            continue;
        const auto center = rage_point_detail::midpoint(hitbox.capsule);
        std::array<Matrix3x4, 9> matrices;
        std::size_t count = 0;
        if (additional) {
            for (const auto& yaw : *additional)
                for (const auto& hypothesis : yaw)
                    matrices[count++] = hypothesis[index];
        } else {
            for (const auto& hypothesis : record.alternatives)
                matrices[count++] = hypothesis.matrices[index];
        }
        std::vector<RageAimPoint> safe;
        if (index != 0) {
            safe = generate_rage_body_safe_points(settings, center, record.header.origin, index,
                                                  local.hitboxes[index], {matrices.data(), count});
        } else if (additional) {
            //433FE9C0 returns before head/body occlusion when the optional
            //nine-set pointer is present. Bounds still consume all nine sets.
            auto bounds = rage_point_detail::build_safe_point_bounds(
                context.origin, center, local.hitboxes[0], {matrices.data(), count});
            safe = rage_point_detail::finish_safe_points(std::move(bounds.points), settings,
                                                         record.header.origin, 0,
                                                         local.hitboxes[0].hitgroup);
        } else {
            RageHeadOcclusion occlusion{local.hitboxes[6],
                                        {record.alternatives[0].matrices[6],
                                         record.alternatives[1].matrices[6],
                                         record.alternatives[2].matrices[6]}};
            safe = generate_rage_head_safe_points(settings, center, record.header.origin,
                                                  local.hitboxes[0],
                                                  std::span<const Matrix3x4, 3>(matrices.data(), 3),
                                                  &occlusion, context.trigonometry);
        }
        result.insert(result.end(), safe.begin(), safe.end());
    }
    return result;
}
float safe_multiplier(const ScanContext& context, const ScanPolicy& policy, std::int32_t health,
                      BallisticEntity target, const BallisticQuery& bullet,
                      const scan_detail::ProjectedPoint& point) {
    if (context.weapon_id == 31)
        return 0.0f;
    const auto delta = rage_point_detail::subtract(point.position, context.origin);
    const auto distance =
        std::sqrt((delta[0] * delta[0] + delta[1] * delta[1]) + delta[2] * delta[2]);
    auto damage = float(std::pow(double(bullet.range_modifier), double(distance * 0.002f)) *
                        double(bullet.base_damage));
    const auto info = context.ballistics.entity_info(target);
    if (hitgroup_armored(info.armor, info.helmet, point.hitgroup))
        damage = (bullet.armor_ratio * 0.5f) * damage;
    const auto integer = _mm_cvttss_si32(_mm_set_ss(damage));
    const auto shots = rage_detail::shots_to_kill(health, integer);
    const auto required =
        maximum_number(float(policy.minimum_damage), float(health) / float(shots));
    return damage == 0.0f ? rage_detail::hitgroup_multipliers[std::uint32_t(point.hitgroup) & 15U]
                          : required / damage;
}
}
namespace scan_detail {
RageSpreadBasis spread_basis(const RageVector& axis) noexcept {
    const auto basis = rage_point_detail::basis_from_axis(axis);
    return {axis, basis.side, basis.upper};
}
RageHitchance normal_chance(const ScanContext& context, void* player, const Geometry& geometry,
                            const RageSpreadBasis& basis, float range, std::int32_t hitgroup,
                            float minimum) {
    const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(player) + 8, 10);
    (void)context.players.get(std::uint32_t(index));
    return evaluate_rage_hitchance(
        {context.origin, basis, range, geometry.primary, context.record_count, hitgroup, minimum},
        context.samples, context.box_clipper);
}
ProjectedPoint project(const ScanContext& context, const Geometry& geometry,
                       const RageVector& point, float range) {
    ProjectedPoint result;
    const auto delta = rage_point_detail::subtract(point, context.origin);
    result.direction = rage_point_detail::normalize(
        delta, std::sqrt((delta[0] * delta[0] + delta[1] * delta[1]) + delta[2] * delta[2]));
    const auto scale = range / (rage_point_detail::length(result.direction) + 0x1p-23f);
    const RageVector extended{result.direction[0] * scale, result.direction[1] * scale,
                              result.direction[2] * scale};
    const auto intersection =
        intersect_scan_hitboxes({context.origin, extended}, geometry.primary, context.box_clipper);
    if (!intersection.found)
        return result;
    result.found = true;
    result.hitbox = std::int32_t(intersection.hitbox);
    result.hitgroup = geometry.primary[intersection.hitbox].hitgroup;
    for (std::size_t axis = 0; axis != 3; ++axis)
        result.position[axis] = extended[axis] * intersection.fraction + context.origin[axis];
    return result;
}
float minimum_multiplier(const ScanPolicy& policy, std::int32_t health, std::int32_t damage,
                         std::int32_t hitgroup, std::int32_t weapon_id) noexcept {
    if (weapon_id == 31)
        return 0.0f;
    const auto multiplier = rage_detail::hitgroup_multipliers[std::uint32_t(hitgroup) & 15U];
    const auto base = float(damage) / multiplier;
    const auto required =
        maximum_number(float(policy.minimum_damage),
                       float(health) / float(rage_detail::shots_to_kill(health, damage)));
    return base == 0.0f ? multiplier : required / base;
}
}
void scan_candidate(ScanContext& context, ScanRequest& request) {
    auto& evaluation = request.evaluation;
    auto& candidate = evaluation.candidate;
    const auto& policy = evaluation.policy;
    const auto local = scan_detail::model(request.target_player, context.model_offsets);
    scan_detail::Geometry geometry;
    geometry.primary = prepare_rage_scan_hitboxes(local.hitboxes, evaluation.record.matrices,
                                                  context.trigonometry);
    if (!geometry.primary.empty() && (evaluation.record.header.flags & 0x80U))
        for (std::size_t i = 0; i != 3; ++i)
            geometry.alternatives[i] = prepare_rage_scan_hitboxes(
                local.hitboxes, evaluation.record.alternatives[i].matrices, context.trigonometry);
    animation::BoneModel bone_model{};
    bone_model.studio_header = local.header != nullptr;
    bone_model.hitbox_bones = local.bones;
    const ScanHypothesisContext hypothesis_context{context.players, context.preparation,
                                                   context.hypothesis_offsets, bone_model,
                                                   context.update_attachments};
    ScanHypothesisMatrices additional;
    std::array<Matrix3x4, 256> attachment_scratch;
    const bool extra = build_optional_scan_hypotheses(
        hypothesis_context, request.target_player, candidate.entity_index, evaluation.record,
        evaluation.source_record, policy.additional_hypotheses, policy.hitboxes.safe, additional,
        attachment_scratch);
    // Valid scanner models provide the nineteen records referenced by the
    //native generation traversal, even when only a subset is enabled.
    const auto points = descriptors(context, policy, evaluation.record, local, geometry,
                                    extra ? &additional : nullptr);
    const auto target = request.target_entity;
    const RagePointRequirements requirements{candidate.health, policy.minimum_damage,
                                             policy.minimum_chance, policy.prefer_body};
    RagePointAdmissionState admission{{{}, 0},
                                      (policy.body_disabler_reasons & 6U) ? 0xffefffffU
                                                                          : 0xffffffffU,
                                      context.configuration_flags,
                                      policy.body_disabler_reasons};
    std::vector<TargetPoint> accepted;
    for (const auto& descriptor : points) {
        TargetPoint point{};
        point.hitbox = point.hitgroup = -1;
        auto bullet = prepare_target_bullet(context.weapon, context.weapon_item_offset,
                                            context.allow_penetration);
        const auto projected =
            scan_detail::project(context, geometry, descriptor.position, bullet.range);
        const auto flags = descriptor.flags | request.descriptor_flags;
        if (projected.found && projected.hitgroup == descriptor.hitgroup) {
            bool safe = true;
            if (flags & 1U) {
                const auto minimum =
                    safe_multiplier(context, policy, candidate.health, target, bullet, projected);
                RageVector direction;
                for (std::size_t axis = 0; axis != 3; ++axis)
                    direction[axis] = bullet.range * projected.direction[axis];
                safe = rage_point_safe({context.origin, direction}, projected.hitgroup, minimum,
                                       geometry.views(), context.box_clipper);
            }
            if (safe) {
                anti_aim::TargetTraceFilter native_filter(
                    context.trace_filter_vtable, context.trace_entities, 0x600400bU, 0x600400bU,
                    float(policy.minimum_damage) /
                        rage_detail::hitgroup_multipliers[std::uint32_t(projected.hitgroup) & 15U]);
                const auto filter = native_filter.binding();
                trace_ballistic_bullet(context.ballistics, context.source, context.origin,
                                       projected.position, filter, bullet);
                if (_mm_cvttss_si32(_mm_set_ss(bullet.damage)) > 0) {
                    const auto info = context.ballistics.entity_info(target);
                    point.position = descriptor.position;
                    point.damage = final_bullet_damage(bullet.damage, bullet.armor_ratio,
                                                       std::bit_cast<std::int32_t>(bullet.reserved),
                                                       bullet.weapon_id, projected.hitgroup,
                                                       info.armor, info.helmet);
                    point.hitbox = projected.hitbox;
                    point.hitgroup = projected.hitgroup;
                    point.flags = flags;
                    point.chance = 1.0f;
                    if (bullet.flags & 2U) {
                        const auto index = candidate.entity_index & 63U;
                        context.target_mask[index >> 5] |= 1U << (index & 31U);
                    }
                    ++evaluation.ballistic_evaluations;
                    if ((context.configuration_flags & 0x10U) &&
                        point.damage >= policy.minimum_damage) {
                        const auto minimum =
                            scan_detail::minimum_multiplier(policy, candidate.health, point.damage,
                                                            point.hitgroup, context.weapon_id);
                        const auto basis = scan_detail::spread_basis(projected.direction);
                        const auto chance =
                            (flags & 1U)
                                ? evaluate_scan_safe_hitchance({context.origin, basis, bullet.range,
                                                                point.hitgroup, minimum,
                                                                geometry.views()},
                                                               context.samples, context.box_clipper)
                                : scan_detail::normal_chance(context, request.target_player,
                                                             geometry, basis, bullet.range,
                                                             point.hitgroup, minimum);
                        //433604A7 writes evaluation+38. The temporary point's
                        //chance remains1 until selection/refinement copies it.
                        candidate.primary.chance = chance.fraction;
                    }
                    point.priority = rage_point_priority(
                        {point.damage, point.hitgroup, point.flags, point.chance, 0}, requirements);
                }
            }
        }
        RagePointScore score{point.damage, point.hitgroup, point.flags, point.chance,
                             point.priority};
        admission.evaluation_flags = context.configuration_flags;
        if (admit_rage_point(score, requirements, admission, descriptor.position,
                             descriptor.flags)) {
            point.reference_position = admission.reference.position;
            point.reference_damage = admission.reference.damage;
            point.descriptor_weight = descriptor.scale;
            accepted.push_back(point);
        }
        context.configuration_flags = admission.evaluation_flags;
    }
    candidate.prefer_body = policy.prefer_body;
    candidate.prefer_safe = policy.prefer_safe;
    candidate.target_state_flags = evaluation.record.header.flags;
    candidate.weapon_priority_enabled = (evaluation.record.header.player_flags & 1U) != 0;
    select_target_points(candidate, accepted, admission.priority_mask);
    if (candidate.primary.damage && (candidate.primary.flags & 2U)) {
        scan_detail::refine(context, policy, candidate.health, request.target_player, target,
                            geometry, candidate.primary);
        if (candidate.flags & 8U) {
            if (candidate.alternate_first.flags & 2U)
                scan_detail::refine(context, policy, candidate.health, request.target_player,
                                    target, geometry, candidate.alternate_first);
            if ((candidate.flags & 0x10U) && (candidate.alternate_second.flags & 2U))
                scan_detail::refine(context, policy, candidate.health, request.target_player,
                                    target, geometry, candidate.alternate_second);
        }
    }
    if (!candidate.primary.damage)
        return;
    if (scan_detail::teammate_blocks(context, candidate.primary.position)) {
        reset_scan_point(candidate.primary);
        return;
    }
    TargetBatch batch;
    batch.candidates[0] = &candidate;
    batch.count = 1;
    recalculate_target_priorities(batch, context.targeting);
    std::atomic_ref(request.completed[request.ordinal >> 5])
        .fetch_or(1U << (request.ordinal & 31U), std::memory_order_seq_cst);
}
}

// ============================================================================
// Subcomponent: scanning_chance.cpp
// ============================================================================

namespace skeet::rage {
RageHitchance evaluate_scan_safe_hitchance(const ScanSafeHitchanceInput& input,
                                           std::span<const RageSpreadSample, 32> samples,
                                           const RageBoxClipper& clipper) {
    std::uint32_t hits = 0;
    for (const auto& sample : samples) {
        RageVector direction;
        for (std::size_t axis = 0; axis != 3; ++axis)
            direction[axis] = (input.basis.first[axis] * sample.first + input.basis.center[axis]) +
                              input.basis.second[axis] * sample.second;
        const auto magnitude =
            std::sqrt((direction[0] * direction[0] + direction[1] * direction[1]) +
                      direction[2] * direction[2]);
        const auto inverse = 1.0f / (magnitude + 0x1p-23f);
        for (auto& value : direction) {
            value *= inverse;
            value *= input.range;
        }
        if (rage_point_safe({input.origin, direction}, input.hitgroup,
                            input.minimum_hitgroup_multiplier, input.hypotheses, clipper))
            ++hits;
    }
    return {hits, float(hits) * 0.03125f};
}
}

// ============================================================================
// Subcomponent: scanning_geometry.cpp
// ============================================================================

namespace skeet::rage {
ScanIntersection intersect_scan_hitboxes(const RageRay& ray,
                                         std::span<const RageScanHitbox> hitboxes,
                                         const RageBoxClipper& clipper) {
    constexpr std::uint32_t order[]{0, 2, 3, 4, 6, 15, 16, 17, 18, 7, 8, 9, 10, 11, 12};
    constexpr std::uint32_t rank[]{5, 0, 2, 1, 4, 4, 6, 6, 3, 5, 5, 5, 5, 5, 5, 5};
    ScanIntersection result;
    RageTrace trace;
    std::uint32_t limit = 0x7fffffffu;
    for (const auto index : order) {
        if (index >= hitboxes.size())
            break;
        const auto& hitbox = hitboxes[index];
        if (rank[static_cast<std::uint32_t>(hitbox.hitgroup) & 15u] > limit)
            break;
        trace.fraction = 1.0f;
        const int hit = hitbox.capsule.radius > 0.0f
                            ? clip_rage_ray_to_capsule(ray, hitbox.capsule, trace)
                            : clipper.clip(clipper.context, ray, hitbox, trace);
        if (hit < 0 || !(result.fraction > trace.fraction))
            continue;
        result = {index, trace.fraction, true};
        if (hitbox.hitgroup != 1)
            break;
        limit = 2;
    }
    return result;
}
}

// ============================================================================
// Subcomponent: scanning_model.cpp
// ============================================================================

namespace skeet::rage::scan_detail {
namespace {
/* deduplicated helper */
std::uint32_t convar(const void* object, std::ptrdiff_t offset) noexcept {
    for (;;) {
        const auto parent = read<const void*>(object, 0x1c);
        if (parent == object)
            break;
        object = parent;
    }
    return read<std::uint32_t>(object, offset) ^
           std::uint32_t(reinterpret_cast<std::uintptr_t>(object));
}
//4341CB0C consumes the supplied axis verbatim. This caller passes the selected
//point vector, not a newly normalized segment direction.
float teammate_distance(const RageVector& center, const RageVector& origin, const RageVector& axis,
                        float bound) noexcept {
    const auto projection =
        ((center[1] - origin[1]) * axis[1] + (center[0] - origin[0]) * axis[0]) +
        (center[2] - origin[2]) * axis[2];
    if (projection < 0.0f || projection > bound)
        return std::numeric_limits<float>::max();
    const auto x = center[0] - (axis[0] * projection + origin[0]);
    const auto y = center[1] - (axis[1] * projection + origin[1]);
    const auto z = center[2] - (axis[2] * projection + origin[2]);
    return std::sqrt((x * x + y * y) + z * z);
}
}
Model model(void* player, const ScanModelOffsets& offsets) {
    Model result;
    const auto wrapper = read<const void*>(player, offsets.studio_header);
    if (!wrapper)
        return result;
    result.header = read<const void*>(wrapper, 0);
    if (!result.header)
        return result;
    const auto index = read<std::int32_t>(player, offsets.hitbox_set);
    const auto set = static_cast<const std::byte*>(result.header) +
                     read<std::int32_t>(result.header, 0xb0) + std::ptrdiff_t(index) * 12;
    const auto count = read<std::uint32_t>(set, 4);
    const auto capped = count < 19 ? count : 19U;
    const auto descriptors = set + read<std::int32_t>(set, 8);
    result.hitboxes.reserve(capped);
    result.bones.reserve(capped);
    for (std::uint32_t i = 0; i < capped; ++i) {
        const auto descriptor = descriptors + std::ptrdiff_t(i) * 0x44;
        result.bones.push_back(read<std::uint32_t>(descriptor, 0));
        result.hitboxes.push_back(
            {read<std::int32_t>(descriptor, 4), read<RageVector>(descriptor, 8),
             read<RageVector>(descriptor, 0x14), read<RageVector>(descriptor, 0x24),
             read<float>(descriptor, 0x30), descriptor});
    }
    return result;
}
bool teammate_blocks(const ScanContext& context, const RageVector& point) {
    const auto& variables = context.teammate_convars;
    const auto first = convar(variables.mode_43467c3c, 0x30) != 0 &&
                       std::bit_cast<float>(convar(variables.coefficient_43467084, 0x2c)) > 0.0f;
    if (!first && std::bit_cast<float>(convar(variables.coefficient_43468c90, 0x2c)) > 0.0f)
        return false;
    const RageVector delta{point[0] - context.origin[0], point[1] - context.origin[1],
                           point[2] - context.origin[2]};
    const auto distance =
        std::sqrt((delta[0] * delta[0] + delta[1] * delta[1]) + delta[2] * delta[2]);
    const RageRay ray{context.origin, delta};
    const auto& target = context.targeting;
    for (std::int32_t index = 1; index <= 64; ++index) {
        const auto player = engine::invoke<void*>(target.entity_list, 3, index);
        if (!player || !engine::invoke<bool>(player, 158))
            continue;
        if (read<std::uint8_t>(player, target.offsets.rejection_bytes[0]))
            continue;
        if (engine::invoke<std::int32_t>(player, 122) <= 0)
            continue;
        if (engine::invoke<bool>(static_cast<std::byte*>(player) + 8, 9))
            continue;
        if (read<std::uint8_t>(player, target.offsets.rejection_bytes[1]))
            continue;
        if (read<std::int32_t>(player, target.offsets.team) != context.team)
            continue;
        auto center = read<RageVector>(player, context.model_offsets.network_origin);
        const auto minimum = engine::invoke<const RageVector*>(player, 76);
        const auto maximum = engine::invoke<const RageVector*>(player, 77);
        center[2] = ((*maximum)[2] + (*minimum)[2]) * 0.5f + center[2];
        if (!(60.0f > teammate_distance(center, context.origin, point, distance)))
            continue;
        const auto local = model(player, context.model_offsets);
        if (!local.header || local.hitboxes.empty())
            continue;
        const auto count = read<std::int32_t>(local.header, 0x9c);
        const auto matrices = read<const Matrix3x4*>(player, context.model_offsets.bone_cache);
        RageTrace trace;
        // Native43364580 traverses nineteen studio records regardless of the
        //reported positive hitbox count; valid player models supply all19.
        for (std::uint32_t box = 0; box != 19; ++box) {
            const auto bone = std::bit_cast<std::int32_t>(local.bones[box]);
            if (bone < 0 || bone >= count)
                continue;
            HitboxMatrices selected;
            selected[0] = matrices[std::size_t(bone)];
            const auto prepared = prepare_rage_scan_hitboxes(
                std::span<const RageLocalHitbox>(&local.hitboxes[box], 1), selected,
                context.trigonometry);
            if (context.box_clipper.clip(context.box_clipper.context, ray, prepared.front(),
                                         trace) >= 0)
                return true;
        }
    }
    return false;
}
}

// ============================================================================
// Subcomponent: scanning_refinement.cpp
// ============================================================================

namespace skeet::rage::scan_detail {
void refine(const ScanContext& context, const ScanPolicy& policy, std::int32_t health, void* player,
            BallisticEntity target, const Geometry& geometry, TargetPoint& point) {
    const auto original = point.position;
    const auto reference = point.reference_position;
    const auto damage = point.damage;
    const auto shots = rage_detail::shots_to_kill(health, damage);
    float fraction = 0.5f;
    auto reference_damage = point.reference_damage;
    bool interpolate = rage_detail::shots_to_kill(health, reference_damage) == shots;
    if (!interpolate) {
        float lower = 0.0f, upper = 1.0f;
        reference_damage = 0;
        auto bullet = prepare_target_bullet(context.weapon, context.weapon_item_offset,
                                            context.allow_penetration);
        const auto difference = rage_point_detail::subtract(original, reference);
        for (unsigned iteration = 0; iteration != 4; ++iteration) {
            const auto midpoint = (upper + lower) * 0.5f;
            const RageVector candidate{difference[0] * midpoint + reference[0],
                                       difference[1] * midpoint + reference[1],
                                       difference[2] * midpoint + reference[2]};
            const auto hit = project(context, geometry, candidate, bullet.range);
            bool keep = false;
            if (hit.found &&
                rage_detail::hitgroup_multipliers[std::uint32_t(hit.hitgroup) & 15U] >=
                    rage_detail::hitgroup_multipliers[std::uint32_t(point.hitgroup) & 15U]) {
                anti_aim::TargetTraceFilter native_filter(context.trace_filter_vtable,
                                                          context.trace_entities, 0x600400bU,
                                                          0x600400bU, 1.0f);
                const auto filter = native_filter.binding();
                trace_ballistic_bullet(context.ballistics, context.source, context.origin,
                                       hit.position, filter, bullet);
                if (_mm_cvttss_si32(_mm_set_ss(bullet.damage)) != 0) {
                    const auto info = context.ballistics.entity_info(target);
                    const auto trial = final_bullet_damage(
                        bullet.damage, bullet.armor_ratio,
                        std::bit_cast<std::int32_t>(bullet.reserved), bullet.weapon_id,
                        hit.hitgroup, info.armor, info.helmet);
                    if (trial != 0 && trial >= policy.minimum_damage &&
                        rage_detail::shots_to_kill(health, trial) <= shots) {
                        reference_damage = trial;
                        keep = true;
                    }
                }
            }
            if (keep)
                upper = midpoint;
            else
                lower = midpoint;
        }
        if (upper != 1.0f) {
            fraction = (upper + 1.0f) * 0.5f;
            interpolate = true;
        }
    }
    if (interpolate) {
        for (std::size_t axis = 0; axis != 3; ++axis)
            point.position[axis] = (original[axis] - reference[axis]) * fraction + reference[axis];
        point.descriptor_weight *= fraction;
        point.damage = _mm_cvttss_si32(_mm_set_ss(
            (float(damage) - float(reference_damage)) * fraction + float(reference_damage)));
    }
    const auto minimum =
        minimum_multiplier(policy, health, point.damage, point.hitgroup, context.weapon_id);
    const auto bullet = prepare_target_bullet(context.weapon, context.weapon_item_offset,
                                              context.allow_penetration);
    const auto delta = rage_point_detail::subtract(point.position, context.origin);
    const auto axis = rage_point_detail::normalize(
        delta, std::sqrt((delta[0] * delta[0] + delta[1] * delta[1]) + delta[2] * delta[2]));
    const auto basis = spread_basis(axis);
    const auto chance =
        (point.flags & 1U)
            ? evaluate_scan_safe_hitchance(
                  {context.origin, basis, bullet.range, point.hitgroup, minimum, geometry.views()},
                  context.samples, context.box_clipper)
            : normal_chance(context, player, geometry, basis, bullet.range, point.hitgroup,
                            minimum);
    point.chance = chance.fraction;
}
}

// ============================================================================
// Subcomponent: settings.cpp
// ============================================================================

namespace skeet::rage {
void initialize_global_settings(ConfigRegistries& r, GlobalSettings& c) noexcept {
    auto& d = r.descriptors;
    initialize_boolean_config(d, c.enabled, 0x23060EC5);
    initialize_hotkey_config(r, c.activation, 0x861E7F9A, 6, HotkeyMode::hold);
    initialize_integer_config(d, c.target_hitboxes, 0x5B900854, 2);
    initialize_hotkey_config(r, c.multipoint_key, 0x29E2C8ED, 0x1B, HotkeyMode::always);
    initialize_boolean_config(d, c.prefer_safe_point, 0xF5A00B3B);
    initialize_hotkey_config(r, c.force_safe_point, 0x333C1DBE, 0x1B, HotkeyMode::hold);
    initialize_integer_config(d, c.avoid_unsafe_hitboxes, 0xECBBD134, 0);
    initialize_boolean_config(d, c.prefer_body_aim, 0x2B54548D);
    initialize_integer_config(d, c.body_aim_disablers, 0x16C91A85, c.body_aim_disablers.value);
    initialize_hotkey_config(r, c.force_body_aim, 0x02F59674, 0x1B, HotkeyMode::hold);
    initialize_boolean_config(d, c.quick_stop, 0xC06FD8CC);
    initialize_hotkey_config(r, c.quick_stop_key, 0x1AC79342, 0x1B, HotkeyMode::always);
    initialize_integer_config(d, c.quick_stop_options, 0x11195AD9, 0);
    initialize_integer_config(d, c.double_tap_hit_chance, 0x3F69C17C, 50);
    initialize_integer_config(d, c.double_tap_quick_stop, 0x9FBDD0E6,
                              c.double_tap_quick_stop.value);
    initialize_boolean_config(d, c.automatic_scope, 0x9E2B2E76);
    initialize_boolean_config(d, c.anti_aim_correction, 0xA4D31376);
    initialize_boolean_config(d, c.automatic_fire, 0x59BD212A);
    initialize_boolean_config(d, c.automatic_penetration, 0x64715C40);
    initialize_boolean_config(d, c.silent_aim, 0x86BFE317);
    initialize_boolean_config(d, c.remove_recoil, 0x63660060);
    initialize_hotkey_config(r, c.duck_peek_assist, 0xD23ED2A7, 0x1B, HotkeyMode::hold);
    initialize_boolean_config(d, c.reduce_aim_step, 0x4034958C);
    initialize_integer_config(d, c.aim_step_limit, 0x20E889DE, 15);
    initialize_boolean_config(d, c.pattern_recoil_correction, 0xA434F7C7);
}
void initialize_aimbot_settings(ConfigRegistries& r, AimbotSettings& c) noexcept {
    auto& d = r.descriptors;
    initialize_integer_config(d, c.minimum_hit_chance, 0x3F5E5553, 50);
    initialize_integer_config(d, c.maximum_fov, 0xD6D22A5A, 180);
    initialize_integer_config(d, c.target_selection, 0x65E86322, 0);
    initialize_integer_config(d, c.minimum_damage, 0x18C0E6BE, 10);
    initialize_boolean_config(d, c.damage_override, 0xB80303B4);
    initialize_hotkey_config(r, c.damage_override_key, 0x7EB3A142, 0x1B, HotkeyMode::hold);
    initialize_integer_config(d, c.override_damage, 0x7DC413FE, 10);
    initialize_integer_config(d, c.multipoint_scale, 0xA25029BA, 90);
    initialize_integer_config(d, c.multipoint, 0xBFABB4A0, 0);
    initialize_integer_config(d, c.accuracy_boost, 0x4346EDB5, 1);
    initialize_boolean_config(d, c.delay_shot, 0xA09C0D52);
    initialize_boolean_config(d, c.force_body_aim_on_peek, 0x9BBD48EA);
}
void initialize_exploit_settings(ConfigRegistries& r, ExploitSettings& c) noexcept {
    auto& d = r.descriptors;
    initialize_boolean_config(d, c.on_shot_anti_aim, 0xB2467427);
    initialize_hotkey_config(r, c.on_shot_key, 0x8F4F3C91, 0x1B, HotkeyMode::always);
    initialize_integer_config(d, c.double_tap_fake_lag_limit, 0x2B56E048, 2);
    initialize_boolean_config(d, c.fake_peek, 0x8494FC0D);
    initialize_hotkey_config(r, c.fake_peek_key, 0xAE772257, 0x58, HotkeyMode::hold);
    initialize_boolean_config(d, c.double_tap, 0x0889939F);
    initialize_hotkey_config(r, c.double_tap_key, 0x67C9986D, 0x1B, HotkeyMode::hold);
    initialize_integer_config(d, c.double_tap_mode, 0x31E2D7DD, 0);
}
void initialize_shot_log_settings(ConfigRegistries& r, ShotLogSettings& c) noexcept {
    initialize_boolean_config(r.descriptors, c.spread_misses, 0xE82BE8DD);
    c.spread_misses.value = 1; // 0x4347F7BA, after registration
}
void initialize_performance_settings(ConfigRegistries& r, PerformanceSettings& c) noexcept {
    initialize_integer_config(r.descriptors, c.mitigations, 0x2B385DC7, c.mitigations.value);
    initialize_boolean_config(r.descriptors, c.warning, 0x740C32AE);
}
void initialize_settings(ConfigRegistries& r, Settings& c) noexcept {
    initialize_global_settings(r, c.global);
    initialize_aimbot_settings(r, c.aimbot);
    initialize_exploit_settings(r, c.exploits);
    initialize_shot_log_settings(r, c.shot_log);
    initialize_performance_settings(r, c.performance);
    initialize_quick_peek_config(r, c.quick_peek);
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: single_target.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
/* deduplicated helper */
float minimum(float a, float b) noexcept {
    return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
float maximum(float a, float b) noexcept {
    return _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(a), _mm_set_ss(b)));
}
/* [stripped helper] */
std::int32_t shots_to_kill(std::int32_t health, std::int32_t damage) noexcept {
    if (damage <= 0)
        return 0x7FFFFFFF;
    const auto shots = add(add(health, -1), damage) / damage;
    return shots > 1 ? shots : 1;
}
// Exact434701D0 filter:43382057 returnsfalse,433686FC returns1.
class PredictionWorldFilter {
public:
    virtual bool should_hit(void*, std::uint32_t) {
        return false;
    }
    virtual std::int32_t trace_type() {
        return 1;
    }
};
struct alignas(16) PredictionRay {
    std::array<float, 4> start, delta, start_offset, extents;
    std::uint32_t transform;
    bool is_ray, swept;
};
static_assert(sizeof(PredictionRay) == 0x50);
static_assert(offsetof(PredictionRay, is_ray) == 0x44);
/* [dedup convar_nonzero] */
/* [removed duplicate weapon] */
float point_scale(const client::CommandFrame& frame, void* target,
                  const AcquisitionTargetSettings& settings, const TargetPreparationBindings& b) {
    const auto& offsets = b.offsets;
    const auto* pose = static_cast<const std::byte*>(target) + offsets.pose_parameters;
    const float duck_pose = field<float>(pose, 0xC);
    const float limit = 0.9f - (duck_pose - 0.0f) * 0.199999988079071044921875f;
    if (!(b.state.flags & 4U))
        return minimum(limit, 0.9f);
    float scale;
    if (b.weapon_settings.multipoint_scale.value == 24) {
        const auto target_origin = field<RageVector>(target, b.targeting.offsets.origin);
        const auto* local_origin = engine::invoke<const RageVector*>(b.targeting.local_player, 10);
        const RageVector delta{target_origin[0] - (*local_origin)[0],
                               target_origin[1] - (*local_origin)[1],
                               target_origin[2] - (*local_origin)[2]};
        if ((b.state.flags & 0x200U) && convar_nonzero(b.accuracy_convar)) {
            const auto* wrapper = field<const std::byte*>(target, offsets.studio_header);
            const std::byte* hitbox = nullptr;
            if (wrapper) {
                const auto* header = field<const std::byte*>(wrapper, 0);
                const auto index = field<std::int32_t>(target, offsets.hitbox_set);
                const auto* set =
                    header + field<std::int32_t>(header, 0xB0) + std::ptrdiff_t(index) * 12;
                if (field<std::int32_t>(set, 4))
                    hitbox = set + field<std::int32_t>(set, 8) + 0x44;
            }
            const auto bone = hitbox ? field<std::int32_t>(hitbox, 0) : -1;
            const auto* cache =
                field<const std::byte*>(target, offsets.bone_cache) + std::ptrdiff_t(bone) * 0x30;
            const float x = field<float>(cache, 0), y = field<float>(cache, 0x10),
                        z = field<float>(cache, 0x20);
            const float planar = std::sqrt(x * x + y * y);
            float angle = float(std::atan2(double(-z), double(planar))) * 57.2957763671875f + 90.0f;
            angle = normalize_movement_angle(angle);
            float fraction = minimum(maximum(angle - 40.0f, 0.0f), 40.0f) / 40.0f;
            if (duck_pose != 1.0f)
                fraction += ((1.0f - duck_pose) - fraction) * 0.5f;
            if (delta[2] > 100.0f)
                fraction += (0.9700000286102294921875f - fraction) * 0.5f;
            scale = minimum(maximum(fraction, 0.1f), 0.9700000286102294921875f);
            if (std::isnan(scale))
                scale = 0.699999988079071044921875f;
        } else {
            const float length =
                std::sqrt((delta[0] * delta[0] + delta[1] * delta[1]) + delta[2] * delta[2]);
            scale = minimum(
                maximum(1.0f - (length * std::bit_cast<float>(frame.field_160)) / 5.0f, 0.3f),
                0.8f);
        }
    } else {
        scale = float(std::bit_cast<std::int32_t>(
                    std::uint32_t(b.weapon_settings.multipoint_scale.value) - 1U)) *
                0.01f;
        if (settings.words[1] != 2U && duck_pose >= 0.99f)
            scale = minimum(0.699999988079071044921875f, scale);
        else if (field<float>(target, offsets.velocity + 8) != 0.0f)
            scale *= 0.9f;
    }
    return minimum(scale, limit);
}
}
float acquisition_point_scale(const client::CommandFrame& frame, void* target,
                              const AcquisitionTargetSettings& settings,
                              const TargetPreparationBindings& bindings) {
    return point_scale(frame, target, settings, bindings);
}
void* select_single_target(const SingleTargetSelection& request) {
    void* selected = nullptr;
    float best = request.minimum_cosine;
    for (std::int32_t index = 1; index <= request.maximum_clients; ++index) {
        void* target = engine::invoke<void*>(request.targeting.entity_list, 3, index);
        if (!target_is_eligible(request.targeting, target))
            continue;
        const auto origin = field<RageVector>(target, request.targeting.offsets.origin);
        const auto* collision = engine::invoke<const RageVector*>(target, 77);
        const float x = origin[0] - request.origin[0], y = origin[1] - request.origin[1];
        const float z = (((*collision)[2] - 8.0f) + origin[2]) - request.origin[2];
        const float length = std::sqrt((x * x + y * y) + z * z);
        const float score =
            ((request.forward[1] * y + x * request.forward[0]) + request.forward[2] * z) /
            (length + 0.00000011920928955078125f);
        if (score > best) {
            selected = target;
            best = score;
        }
    }
    return selected;
}
bool target_history_yaw_changed(void* target, const client::PlayerState& player,
                                std::ptrdiff_t animation_state_offset) noexcept {
    const auto* animation = field<const std::byte*>(target, animation_state_offset);
    const float yaw = animation ? field<float>(animation, 0x80) : player.native.reference_yaw;
    const auto count = player.history.count < 3 ? player.history.count : 3;
    for (std::uint32_t n = 0; n < count; ++n) {
        const auto& record = player.history.at(n).header;
        if (record.flags & 0x200U)
            break;
        if (record.flags & 8U)
            continue;
        const float difference = yaw - record.animation_yaw;
        if (std::isnan(difference))
            continue;
        if (std::fabs(normalize_movement_angle(difference)) > 60.0f)
            return true;
    }
    return false;
}
void prepare_acquisition_target_settings(const client::CommandFrame& frame, void* target,
                                         client::PlayerState& player, std::uint32_t entity_index,
                                         const players::Adjustments& adjustments,
                                         AcquisitionTargetSettings& settings,
                                         const TargetPreparationBindings& b) {
    if (settings.words[0] && player.history.count) {
        bool retain = false;
        if (b.remembered_target_handle != -1) {
            const auto* remembered =
                engine::invoke<void*>(b.targeting.entity_list, 4, b.remembered_target_handle);
            retain = remembered == target && !(b.state.previous_targets[(entity_index & 63U) >> 5] &
                                               (1U << (entity_index & 31U)));
        }
        if (!retain)
            settings.words[0] = 1;
    } else
        settings.words[0] = 0;
    const auto health = engine::invoke<std::int32_t>(target, 122);
    const auto raw = std::bit_cast<std::int32_t>(settings.words[5]);
    float factor = 0;
    std::int32_t source_limit = 0;
    if (raw == 0) {
        factor = engine::invoke<float>(weapon(frame), 433, std::int32_t(0));
        source_limit = engine::invoke<std::int32_t>(weapon(frame), 431);
    }
    settings.words[5] =
        std::uint32_t(rage_minimum_damage(raw, health, b.state.flags, factor, source_limit));
    if (settings.words[4])
        settings.words[3] = std::bit_cast<std::uint32_t>(point_scale(frame, target, settings, b));
    const auto origin = field<RageVector>(target, b.targeting.offsets.origin);
    const float x = origin[0] - frame.origin_10c[0], y = origin[1] - frame.origin_10c[1],
                z = origin[2] - frame.origin_10c[2];
    const float distance = (y * y + x * x) + z * z;
    if (distance > 4000000.0f) {
        settings.words[4] &= ~0x2F0U;
        if (settings.words[1] != 0x200U)
            settings.words[1] &= ~0x200U;
    }
    if (distance > 1000000.0f)
        settings.words[4] &= ~0x200U;
    auto countdown = field<std::int32_t>(&player.native, 0xF8);
    if (countdown > 0) {
        settings.option(0xDD, 1);
        if (!(b.state.flags & 9U)) {
            --countdown;
            write(&player.native, 0xF8, countdown);
        }
    }
    if (adjustments.prefer_body_aim == 1)
        settings.option(0xDD, 0);
    else if (adjustments.prefer_body_aim == 2 || adjustments.prefer_body_aim == 3)
        settings.option(0xDD, 1);
    const bool force_body = adjustments.prefer_body_aim == 3 ||
                            hotkey_active(b.global.force_body_aim.value.binding, b.input);
    const bool restricted =
        (b.state.flags & 0x400U) &&
        field<std::uint16_t>(weapon(frame), b.targeting.offsets.weapon_identifier) != 31;
    if (force_body && !restricted) {
        settings.words[1] &= 0xC;
        settings.words[2] &= 0xC;
        if (!(settings.words[1] | settings.words[2]))
            settings.words[1] = 0xC;
    }
    if (adjustments.safe_point == 2) {
        settings.words[2] |= settings.words[1];
        settings.words[1] = 0;
    }
    if (!settings.words[1] && settings.words[2])
        settings.words[2] &= ~0x200U;
    if (adjustments.correction_active && b.global.prefer_safe_point.value &&
        (settings.words[1] & 0xC) == 0xC && 0.5f > field<float>(&player.native, 0x51C) &&
        field<std::int32_t>(target, b.targeting.offsets.team) == 2 &&
        field<float>(target, b.targeting.offsets.duck_amount) >= 0.8f &&
        (field<std::uint8_t>(target, b.targeting.offsets.movement_flags) & 1U))
        settings.words[4] &= ~8U;
    if (restricted) {
        settings.words[1] &= 2;
        settings.words[2] &= 2;
        settings.words[4] &= 2;
        settings.option(0xDD, 0);
        settings.option(0xDF, 0);
    }
}
AcquisitionTargetTicks acquisition_target_ticks(const client::CommandFrame& frame, void* target,
                                                client::PlayerState& player,
                                                AcquisitionTargetSettings& settings,
                                                const TargetPreparationBindings& b,
                                                const RecordCandidateBindings& timing) {
    constexpr std::array<std::uint32_t, 5> maximum_records{1, 1, 3, 2, 5}; //43474950
    const auto maximum = maximum_records[settings.words[0]];
    AcquisitionTargetTicks result;
    const auto window = lagcomp::history_tick_window(
        {timing.current_time, field<float>(&frame, 0x198), timing.interpolation, timing.interval,
         timing.maximum_unlag_ms, field<std::int32_t>(&frame, 0xD4),
         std::int32_t(frame.field_18c)});
    const auto clamp_newest = [&](std::int32_t first, std::int32_t last) {
        if (last >= first) {
            const auto newest = player.history.at(0).header.simulation_tick;
            if (newest < first)
                return first;
            return newest > last ? last : newest;
        }
        return last;
    };
    if (settings.words[0]) {
        append_target_record_ticks(frame, target, player, maximum, timing, result.values);
        if (!result.values.empty()) {
            if (result.values.size() >= 2)
                std::qsort(result.values.data(), result.values.size(), sizeof(std::int32_t),
                           [](const void* left, const void* right) {
                               return std::bit_cast<std::int32_t>(
                                   std::uint32_t(*static_cast<const std::int32_t*>(right)) -
                                   std::uint32_t(*static_cast<const std::int32_t*>(left)));
                           });
            result.descriptor_flags = 4;
        } else {
            if (!player.history.count)
                return result;
            const auto first = window.corrected > window.first ? window.corrected : window.first;
            std::int32_t stop_tick = player.history.at(0).header.simulation_tick;
            for (std::uint32_t n = 0; n < player.history.count; ++n) {
                const auto& record = player.history.at(n).header;
                stop_tick = record.simulation_tick;
                if (record.flags & 0x200U)
                    break;
            }
            if (window.last < stop_tick)
                return result;
            const auto* layers = field<const std::byte*>(target, b.offsets.animation_layers);
            if (0.00150000001303851604461669921875f > field<float>(layers, 0x178)) {
                if (target_history_yaw_changed(target, player, b.offsets.animation_state))
                    settings.option(0xDD, 1);
            } else {
                const auto receive_tick = player.history.at(0).header.receive_tick;
                const long double clock = engine::invoke<double>(timing.engine_client, 14);
                const float rounded =
                    float(clock / static_cast<long double>(timing.receive_interval) + 0.5L);
                const auto tick = _mm_cvttss_si32(_mm_set_ss(rounded));
                const auto difference =
                    std::bit_cast<std::int32_t>(std::uint32_t(tick) - std::uint32_t(receive_tick));
                if (difference > 2)
                    return result;
            }
            result.values.push_back(clamp_newest(first, window.last));
        }
    }
    if (result.values.empty()) {
        if ((b.state.flags & 1U) || !player.history.count)
            return result;
        result.values.push_back(clamp_newest(window.first, window.last));
    }
    if (result.values.size() > maximum)
        result.values.resize(maximum);
    return result;
}
namespace {
bool prediction_damage_gate(const client::CommandFrame& frame, const client::PlayerState& player,
                            const AcquisitionTargetSettings& settings,
                            const TargetPredictionBindings& b) {
    const auto& trace = b.trace;
    if (!settings.option(0xDE) || !(frame.flags & 0x400U) || (trace.context_flags & 3U) ||
        (b.scheduler_flags & 8U) || !player.history.count || (frame.flags & 0x8000000U))
        return false;
    if (frame.weapon_id_d8 == 64 && !(trace.weapon_flags & 1U))
        return false;
    if ((frame.flags & 0x102000U) || !(frame.flags & 0x40000U) || (frame.flags & 0x4000U))
        return false;
    return !(20.0f > frame.scalar_c0 && !(frame.previous_command.movement.buttons & 0x618U));
}
void refine_prediction_record(const client::CommandFrame& frame, void* target,
                              AcquisitionTargetSettings& settings,
                              const TargetPredictionBindings& b, const lagcomp::LagRecord& record) {
    const auto& trace = b.trace;
    auto query = prepare_target_bullet(trace.weapon, trace.offsets.weapon_item, true);
    anti_aim::TargetTraceFilter engine_filter(trace.trace_filter_vtable, trace.trace_entities);
    const auto filter = engine_filter.binding();
    RageVector aim = record.header.origin;
    aim[2] = record.header.collision_max_z * 0.5f + aim[2];
    trace_ballistic_bullet(trace.ballistics, trace.source, frame.origin_dc, aim, filter, query);
    if (!query.point_count)
        return;
    const auto target_entity = BallisticEntity(reinterpret_cast<std::uintptr_t>(target));
    const auto damage = [&] {
        const auto info = trace.ballistics.entity_info(target_entity);
        return final_bullet_damage(query.damage, query.armor_ratio,
                                   std::bit_cast<std::int32_t>(query.reserved), query.weapon_id, 3,
                                   info.armor, info.helmet);
    };
    const auto health = engine::invoke<std::int32_t>(target, 122);
    const auto present_damage = damage();
    const auto present_shots = shots_to_kill(health, present_damage);
    if (present_shots <= 1)
        return;
    const auto angles = field<MovementAngles>(&frame, 0x118);
    MovementVector forward, right;
    movement_angle_vectors(angles, &forward, &right, nullptr);
    const auto flatten = [](MovementVector& v) {
        const auto inverse =
            1.0f / (std::sqrt((v.x * v.x + v.y * v.y) + 0.0f * 0.0f) + 0.00000011920928955078125f);
        v = {v.x * inverse, v.y * inverse, 0.0f * inverse};
    };
    flatten(forward);
    flatten(right);
    const auto forward_move =
        b.command ? b.command->movement.forward_move : field<float>(&frame, 0x24);
    const auto side_move = b.command ? b.command->movement.side_move : field<float>(&frame, 0x28);
    const float x = side_move * right.x + forward_move * forward.x;
    const float y = side_move * right.y + forward_move * forward.y;
    void* local = reinterpret_cast<void*>(std::uintptr_t(trace.source));
    float speed;
    if (b.command) {
        // Post-firing4336961E calls player450 before reading3EB71843.
        const auto maximum = engine::invoke<float>(local, 276);
        speed = minimum(field<float>(local, b.local_maximum_speed), maximum) *
                    0.3400000035762786865234375f -
                1.0f;
    } else {
        speed =
            minimum(field<float>(local, b.local_maximum_speed), engine::invoke<float>(local, 276)) *
                0.3400000035762786865234375f -
            1.0f;
    }
    const auto inverse =
        speed / (std::sqrt((x * x + y * y) + 0.0f * 0.0f) + 0.00000011920928955078125f);
    const float time = engine::invoke<float>(trace.weapon, 433, std::int32_t(0)) * 0.75f;
    const RageVector projected{(x * inverse) * time + frame.origin_dc[0],
                               (y * inverse) * time + frame.origin_dc[1],
                               (0.0f * inverse) * time + frame.origin_dc[2]};
    PredictionRay ray{};
    ray.start = {frame.origin_dc[0] - 16.0f, frame.origin_dc[1] - 16.0f, frame.origin_dc[2] + 0.0f,
                 0};
    for (unsigned axis = 0; axis < 3; ++axis)
        ray.delta[axis] = projected[axis] - frame.origin_dc[axis];
    ray.start_offset = {16.0f, 16.0f, -0.0f, 0};
    ray.extents = {0, 0, 2.0f, 0};
    ray.is_ray = false;
    ray.swept = ((ray.delta[0] * ray.delta[0] + ray.delta[1] * ray.delta[1]) +
                 ray.delta[2] * ray.delta[2]) != 0.0f;
    BallisticTrace world_trace;
    PredictionWorldFilter world_filter;
    {
        MovementTraceFlagGuard guard(trace.trace_flag);
        engine::invoke<void>(b.engine_trace, 5, &ray, std::uint32_t(1), &world_filter,
                             &world_trace);
    }
    if (!trace_ballistic_bullet(trace.ballistics, trace.source, world_trace.end, aim, filter,
                                query))
        return;
    const auto future_damage = damage();
    if (future_damage <= 0)
        return;
    const auto future_shots = shots_to_kill(health, future_damage);
    if (present_shots <= future_shots)
        return;
    if (future_shots >= 2 && (b.scheduler_flags & 0x8000U) && frame.field_1d4 == 1)
        return;
    auto numerator = health;
    const auto remainder = health % future_shots;
    if (remainder)
        numerator = add(health, future_shots - remainder);
    auto required = numerator / future_shots;
    if (required > future_damage)
        required = future_damage;
    if (frame.field_1d4 == 1 && engine::invoke<bool>(trace.weapon, 430)) {
        if (add(present_damage, future_damage) > health)
            return;
        const auto remaining = add(health, -future_damage);
        required = remaining > 1 ? remaining : 1;
    }
    const auto old = std::bit_cast<std::int32_t>(settings.words[5]);
    if (required > old)
        settings.words[5] = std::uint32_t(required);
}
}
void refine_acquisition_minimum_damage(const client::CommandFrame& frame, void* target,
                                       client::PlayerState& player,
                                       AcquisitionTargetSettings& settings,
                                       const TargetPredictionBindings& b) {
    if (!prediction_damage_gate(frame, player, settings, b))
        return;
    lagcomp::LagRecord record;
    std::memcpy(&record, &player.history.at(0), sizeof record);
    const auto& selection = b.record_selection;
    lagcomp::refresh_record_attachments(selection.players, target, record, selection.preparation,
                                        selection.attachments);
    refine_prediction_record(frame, target, settings, b, record);
}
void refine_acquisition_minimum_damage(const client::CommandFrame& frame, void* target,
                                       client::PlayerState& player,
                                       AcquisitionTargetSettings& settings,
                                       const TargetPredictionBindings& b,
                                       const animation::BoneRuntime& runtime) {
    if (!prediction_damage_gate(frame, player, settings, b))
        return;
    lagcomp::LagRecord record;
    std::memcpy(&record, &player.history.at(0), sizeof record);
    const auto& selection = b.record_selection;
    lagcomp::refresh_record_attachments(selection.players, target, record, selection.preparation,
                                        selection.attachments, runtime);
    refine_prediction_record(frame, target, settings, b, record);
}
bool acquire_single_target(PreambleState& state, PreambleResult& preamble,
                           AcquisitionStorage& storage, const SingleTargetBindings& bindings) {
    if (!(state.flags & 9U)) {
        state.target_mask.fill(0);
        state.previous_targets.fill(0);
    }
    auto& frame = *preamble.frame;
    const auto& preparation = bindings.preparation;
    void* target = select_single_target({preparation.targeting, bindings.maximum_clients,
                                         frame.origin_dc, field<RageVector>(&frame, 0x1A8),
                                         std::bit_cast<float>(preamble.scan_template[7])});
    if (!target)
        return false;
    const auto entity = engine::invoke<std::uint32_t>(static_cast<std::byte*>(target) + 8, 10);
    auto& player = *bindings.records.players.get(entity);
    AcquisitionTargetSettings settings{preamble.scan_template};
    prepare_acquisition_target_settings(frame, target, player, entity,
                                        bindings.adjustments.records[entity].adjustments, settings,
                                        preparation);
    auto trace = bindings.prediction.trace;
    trace.context_flags = state.flags;
    trace.weapon_flags = state.weapon_flags;
    trace.weapon = weapon(frame);
    const TargetPredictionBindings prediction{trace,
                                              bindings.prediction.record_selection,
                                              bindings.prediction.engine_trace,
                                              bindings.prediction.local_maximum_speed,
                                              bindings.prediction.scheduler_flags,
                                              bindings.prediction.command};
    refine_acquisition_minimum_damage(frame, target, player, settings, prediction,
                                      bindings.records.bone_runtime);
    auto ticks = acquisition_target_ticks(frame, target, player, settings, preparation,
                                          bindings.record_candidates);
    prepare_and_submit_target_records(state, preamble, storage, target, player, ticks.values,
                                      settings, ticks.descriptor_flags, bindings.records);
    finish_target_acquisition(state, storage, bindings.jobs, bindings.records.players,
                              bindings.maximum_clients, bindings.records.settings_menu);
    return true;
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: targeting.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* deduplicated helper */
/* [dedup convar_integer] */
int compare_candidates(const void* left, const void* right) noexcept {
    const auto a = *static_cast<TargetCandidate* const*>(left);
    const auto b = *static_cast<TargetCandidate* const*>(right);
    return compare_rage_candidate_priorities(a->priority, b->priority);
}
}

bool target_is_enemy(const TargetBindings& bindings, const void* target) noexcept {
    const auto local = bindings.local_player;
    if (!local || !target || local == target)
        return false;
    if (convar_integer(bindings.game_type.get()) == 6) {
        const auto team = field<std::int32_t>(local, bindings.offsets.survival_team);
        return team == -1 || team != field<std::int32_t>(target, bindings.offsets.survival_team);
    }
    const auto team = field<std::int32_t>(local, bindings.offsets.team);
    const auto other_team = field<std::int32_t>(target, bindings.offsets.team);
    if (team == 2) {
        if (other_team == 3)
            return true;
    } else if (team == 3) {
        if (other_team == 2)
            return true;
    } else
        return false;
    return convar_integer(bindings.teammates_are_enemies.get()) != 0;
}

bool target_is_eligible(const TargetBindings& bindings, void* target) {
    if (!target || !engine::invoke<bool>(target, 158))
        return false;
    const auto& offsets = bindings.offsets;
    if (field<std::uint8_t>(target, offsets.rejection_bytes[0]) != 0)
        return false;
    if (engine::invoke<std::int32_t>(target, 122) <= 0)
        return false;
    const auto networkable = static_cast<std::byte*>(target) + 8;
    if (engine::invoke<bool>(networkable, 9))
        return false;
    if (field<std::uint8_t>(target, offsets.rejection_bytes[1]) != 0)
        return false;
    if (field<std::uint8_t>(target, offsets.rejection_bytes[2]) != 0)
        return false;
    const auto index = engine::invoke<std::int32_t>(networkable, 10);
    if (bindings.player_adjustments.records[std::uint32_t(index)].adjustments.whitelist != 0)
        return false;
    if (field<std::uint8_t>(target, offsets.rejection_bytes[3]) != 0)
        return false;
    return target_is_enemy(bindings, target);
}

RageVector target_eye_position(const TargetBindings& bindings, const void* target) noexcept {
    return target_eye_position(
        TargetEyePositionOffsets{bindings.offsets.origin, bindings.offsets.duck_amount}, target);
}
RageVector target_eye_position(TargetEyePositionOffsets offsets, const void* target) noexcept {
    auto result = field<RageVector>(target, offsets.origin);
    const auto duck = field<float>(target, offsets.duck_amount);
    const auto square = duck * duck;
    const auto cubic_twice = (square + square) * duck;
    const auto interpolation = 1.0f - (square * 3.0f - cubic_twice);
    result[2] = (interpolation * 18.0f + 46.0f) + result[2];
    return result;
}

bool target_passes_scan_filter(const TargetBindings& bindings, const void* target,
                               const TargetView& view) noexcept {
    const auto move_type = field<std::uint8_t>(target, bindings.offsets.movement_type);
    if (move_type == 8 || move_type == 10)
        return false;
    if (field<std::uint8_t>(target, bindings.offsets.movement_flags) & 0x40U)
        return false;
    if (view.field_of_view == 180)
        return true;
    const auto eye = target_eye_position(bindings, target);
    const auto x = eye[0] - view.origin[0];
    const auto y = eye[1] - view.origin[1];
    const auto z = eye[2] - view.origin[2];
    const auto length = std::sqrt((x * x + y * y) + z * z);
    const auto cosine =
        ((view.forward[1] * y + view.forward[0] * x) + view.forward[2] * z) / length;
    return !(view.minimum_cosine > cosine);
}

TargetBatch collect_target_candidates(std::span<TargetCandidate, 80> candidates,
                                      const std::array<std::uint32_t, 3>& completed,
                                      std::uint32_t context_flags,
                                      std::array<std::uint32_t, 2>& target_mask) noexcept {
    TargetBatch result;
    std::array<std::int8_t, 64> groups;
    groups.fill(-1);
    for (std::size_t i = 0; i != 80; ++i) {
        if (!(completed[i >> 5] & (1U << (i & 31))))
            continue;
        auto& candidate = candidates[i];
        const auto entity = candidate.entity_index & 63U;
        if (!(context_flags & 1U))
            target_mask[entity >> 5] |= 1U << (entity & 31);
        const auto slot = groups[entity];
        if (slot == -1) {
            if (result.count == result.candidates.size())
                continue;
            groups[entity] = std::int8_t(result.count);
            result.candidates[result.count++] = &candidate;
        } else if (candidate.priority > result.candidates[std::size_t(slot)]->priority)
            result.candidates[std::size_t(slot)] = &candidate;
    }
    return result;
}

void recalculate_target_priorities(TargetBatch& batch, const TargetBindings& bindings) {
    for (std::size_t i = 0; i != batch.count; ++i) {
        auto& candidate = *batch.candidates[i];
        bool taser = false;
        if (candidate.weapon_priority_enabled) {
            const auto entity =
                engine::invoke<void*>(bindings.entity_list, 3, candidate.entity_index);
            if (entity) {
                const auto handle = field<std::uint32_t>(entity, bindings.offsets.active_weapon);
                if (handle != 0xffffffffU) {
                    const auto weapon = engine::invoke<void*>(bindings.entity_list, 4, handle);
                    if (weapon)
                        taser =
                            field<std::uint16_t>(weapon, bindings.offsets.weapon_identifier) == 31;
                }
            }
        }
        RageCandidatePriority score;
        score.target_health = candidate.health;
        score.primary = {candidate.primary.damage, candidate.primary.hitgroup,
                         candidate.primary.flags, candidate.primary.chance,
                         candidate.primary.priority};
        score.prefer_body = candidate.prefer_body;
        score.prefer_safe = candidate.prefer_safe;
        score.target_weapon_priority_enabled = candidate.weapon_priority_enabled;
        score.target_weapon_is_taser = taser;
        score.target_priority =
            bindings.player_adjustments.records[candidate.entity_index].adjustments.high_priority !=
            0;
        score.target_state_flags = candidate.target_state_flags;
        candidate.priority = rage_candidate_priority(score);
    }
}

void sort_target_candidates(TargetBatch& batch) {
    std::qsort(batch.candidates.data(), batch.count, sizeof batch.candidates[0],
               compare_candidates);
}

TargetBatch rank_completed_target_candidates(std::span<TargetCandidate, 80> candidates,
                                             const std::array<std::uint32_t, 3>& completed,
                                             std::uint32_t context_flags,
                                             std::array<std::uint32_t, 2>& target_mask,
                                             const TargetBindings& bindings) {
    auto batch = collect_target_candidates(candidates, completed, context_flags, target_mask);
    recalculate_target_priorities(batch, bindings);
    if (batch.count > 1)
        sort_target_candidates(batch);
    return batch;
}

} // namespace skeet::rage

// ============================================================================
// Subcomponent: targeting_acquired.cpp
// ============================================================================

namespace skeet::rage {
const AcquisitionEvaluation& acquisition_evaluation_for(const AcquisitionStorage& storage,
                                                        const TargetCandidate& candidate) noexcept {
    const auto slot = &candidate - storage.candidates.data();
    return storage.evaluations[std::size_t(slot)];
}
TargetShotChoice select_acquired_target_for_shot(TargetBatch& batch, AcquisitionStorage& storage,
                                                 client::CommandFrame& frame, PreambleState& state,
                                                 const FinalTargetContextBindings& bindings) {
    auto& context_flags = state.flags;
    if (!batch.count)
        return {};
    if ((context_flags & 0x10U) && frame.weapon_id_d8 == 64 && (frame.flags & 0x40U)) {
        batch.candidates[0]->primary.chance = 1.0f;
        return {batch.candidates[0], 0};
    }
    for (std::size_t ordinal = 0; ordinal != batch.count; ++ordinal) {
        auto& candidate = *batch.candidates[ordinal];
        float chance = 1.0f;
        if (context_flags & 0x10U) {
            const auto& evaluation = acquisition_evaluation_for(storage, candidate);
            const auto evaluate = [&] {
                return evaluate_final_target(frame, state, candidate, evaluation, context_flags,
                                             bindings, chance);
            };
            if (!evaluate()) {
                if (!(frame.flags & 0x400U) || !(candidate.flags & 8U))
                    continue;
                candidate.flags &= ~8U;
                candidate.primary = candidate.alternate_first;
                if (!evaluate()) {
                    if (!(candidate.flags & 0x10U))
                        continue;
                    candidate.flags &= ~0x10U;
                    candidate.primary = candidate.alternate_second;
                    if (!evaluate())
                        continue;
                }
            }
        }
        // Stack2C starts at +0.0 and stack90 at0.5; neither is rewritten
        // between the complete main entry and the selection loop.
        const float effective = (candidate.primary.flags & 0x20U) ? chance * 0.5f : chance;
        if (!(0.0f >= effective)) {
            candidate.primary.chance = chance;
            return {&candidate, ordinal};
        }
    }
    return {};
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: targeting_commit.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* [stripped helper] */
/* [removed duplicate subtract] */
RageVector advance(const RageVector& a, const RageVector& d, float scale) {
    return {d[0] * scale + a[0], d[1] * scale + a[1], d[2] * scale + a[2]};
}
float length(const RageVector& v) {
    return std::sqrt((v[0] * v[0] + v[1] * v[1]) + v[2] * v[2]);
}
BallisticRay ray(const RageVector& a, const RageVector& b) {
    const auto delta = subtract(b, a);
    return {a, delta, (delta[1] * delta[1] + delta[0] * delta[0]) + delta[2] * delta[2] != 0.0f};
}
/* [dedup convar] */
}

bool final_teammate_precheck_required(std::uint32_t flags,
                                      const FinalTeammateConvars& variables) noexcept {
    if (flags & 8U)
        return false;
    const bool forced = convar(variables.mode_43467c3c, 0x30) != 0 &&
                        std::bit_cast<float>(convar(variables.coefficient_43467084, 0x2c)) > 0.0f;
    if (!forced && std::bit_cast<float>(convar(variables.coefficient_43468c90, 0x2c)) > 0.0f)
        return false;
    return convar(variables.teammates_are_enemies, 0x30) == 0;
}

BallisticQuery prepare_target_bullet(void* weapon, std::ptrdiff_t item_offset,
                                     std::uint8_t allow_penetration) {
    BallisticQuery query;
    query.allow_penetration = allow_penetration;
    reset_ballistic_query(query);
    const auto data = engine::invoke<const void*>(weapon, 461);
    query.weapon_id = read<std::uint16_t>(weapon, item_offset);
    query.penetration = read<float>(data, 0x100);
    query.range = read<float>(data, 0x10c);
    query.range_modifier = read<float>(data, 0x110);
    query.armor_ratio = engine::invoke<float>(weapon, 434);
    query.base_damage = float(engine::invoke<std::int32_t>(weapon, 431));
    query.reserved = read<std::uint32_t>(data, 0xfc);
    return query;
}

std::int32_t trace_target_direction(BallisticEngine& engine, BallisticEntity source,
                                    const RageVector& origin, const RageVector& direction,
                                    const BallisticFilter& filter, BallisticQuery& query) {
    reset_ballistic_query(query);
    engine.bind_source(filter, source);
    engine.bind_last_hit(filter, 0);
    float damage = query.base_damage;
    const auto minimum = maximum_number(1.0f, filter.minimum_damage - 5.0f);
    if (!(damage > minimum))
        return 0;
    auto position = origin;
    float travelled = 0.0f;
    for (;;) {
        const auto remaining = query.range - travelled;
        const auto end = advance(position, direction, remaining);
        engine.trace_ray(
            {ray(position, end), filter.mask, BallisticFilterKind::supplied, &filter, source, 0},
            query.trace);
        if (filter.mask & 0x40000000U) {
            const auto extended = advance(end, direction, 40.0f);
            const auto fraction =
                length(subtract(query.trace.end, position)) / length(subtract(position, extended));
            clip_ballistic_entities(engine, position, extended, 0.0f, fraction, filter,
                                    query.trace);
        }
        if (query.trace.fraction == 1.0f) {
            query.damage = 0;
            return 0;
        }
        query.points[query.point_count++] = query.trace.end;
        const auto surface = engine.surface(query.trace.surface_property);
        BallisticEntity hit = 0;
        if (query.trace.entity && engine.entity_info(query.trace.entity).virtual_278)
            hit = query.trace.entity;
        engine.bind_last_hit(filter, hit);
        travelled = remaining * query.trace.fraction + travelled;
        damage = float(std::pow(double(query.range_modifier), double(travelled * 0.002f)) *
                       double(damage));
        if (minimum > damage) {
            query.damage = 0;
            return 0;
        }
        if ((travelled > 3000.0f && query.penetration > 0.0f) ||
            double(surface.penetration_modifier) < 0.1)
            query.remaining_penetrations = 0;
        if (!(filter.mask & 0x40000000U))
            query.trace.hitgroup = 1;
        if (query.trace.entity && engine.entity_info(query.trace.entity).virtual_280) {
            if (!hit) {
                query.damage = 0;
                return 0;
            }
            const auto info = engine.entity_info(hit);
            if (std::uint32_t(query.trace.hitgroup) - 1U < 8U && !info.target_state) {
                if (info.target_index_blocked ||
                    (source && !ballistic_enemy(engine, source, hit))) {
                    query.damage = 0;
                    return 0;
                }
                if (filter.mask & 0x40000000U)
                    damage = hitgroup_damage_scale(info.armor, info.helmet, query.trace.hitgroup,
                                                   query.weapon_id, query.armor_ratio) *
                             damage;
                query.damage = damage;
                return _mm_cvttss_si32(_mm_set_ss(damage));
            }
        }
        if (query.remaining_penetrations < 1) {
            query.damage = 0;
            return 0;
        }
        if (handle_ballistic_penetration(engine, direction, position, source, filter, query,
                                         surface, damage)) {
            query.flags |= 4U;
            query.damage = 0;
            return 0;
        }
        if (!(damage > minimum)) {
            query.damage = 0;
            return 0;
        }
    }
}

FinalHitchancePlan
plan_final_hitchance(bool safe, float minimum_chance, std::uint32_t context_flags,
                     std::uint32_t entity_index,
                     const std::array<std::uint32_t, 2>& previous_targets) noexcept {
    const auto index = entity_index & 63U;
    const auto samples =
        (context_flags & 8U) || (previous_targets[index >> 5] & (1U << (index & 31U))) ? 32U : 64U;
    const auto total = std::int32_t(samples * (safe ? 3U : 1U));
    const auto required = _mm_cvtss_si32(_mm_set_ss(float(total) * minimum_chance));
    return {samples, 128U / samples,
            rage_detail::signed_word(std::uint32_t(total) - std::uint32_t(required) + 2U)};
}

RageSpreadBasis final_target_spread_basis(const RageVector& point,
                                          const RageVector& origin) noexcept {
    const auto delta = subtract(point, origin);
    const auto center = rage_point_detail::normalize(
        delta, std::sqrt((delta[2] * delta[2] + delta[0] * delta[0]) + delta[1] * delta[1]));
    const auto transverse = rage_point_detail::basis_from_axis(center);
    return {center, transverse.side, transverse.upper};
}

void evaluate_final_hitchance_sample(BallisticEngine& engine, const FinalHitchanceSample& input,
                                     RageSpreadSample sample, std::int32_t failure_budget,
                                     FinalHitchanceVotes& votes) {
    if (votes.stopped.load(std::memory_order_relaxed))
        return;
    auto query =
        prepare_target_bullet(input.weapon, input.weapon_item_offset, input.allow_penetration);
    RageVector direction;
    for (std::size_t i = 0; i != 3; ++i)
        direction[i] = (sample.first * input.basis.first[i] + input.basis.center[i]) +
                       sample.second * input.basis.second[i];
    const auto inverse = 1.0f / (length(direction) + 0x1p-23f);
    for (auto& value : direction)
        value *= inverse;
    const auto delta = subtract(input.point, input.origin);
    const auto projection =
        (delta[1] * direction[1] + delta[0] * direction[0]) + delta[2] * direction[2];
    float distance;
    if (projection < 0.0f || projection > query.range + 40.0f)
        distance = 3.4028234663852886e38f;
    else
        distance = length(subtract(input.point, advance(input.origin, direction, projection)));
    std::uint32_t successes = 0;
    if (60.0f > distance) {
        if (!input.safe) {
            auto filter = input.direction_filter;
            filter.preferred_entity = input.target;
            filter.mask = filter.exit_mask = 0x4600400bU;
            filter.minimum_damage = 1.0f;
            const auto damage = input.live_bindings
                                    ? final_target_detail::trace_direction(
                                          *input.live_bindings, engine, input, direction, query)
                                    : trace_target_direction(engine, input.source, input.origin,
                                                             direction, filter, query);
            const auto total = std::uint32_t(damage) * query.reserved;
            if (damage > 0 && std::bit_cast<std::int32_t>(total) >= input.minimum_damage)
                successes = 1;
        } else {
            const RageRay ray{input.origin,
                              {direction[0] * query.range, direction[1] * query.range,
                               direction[2] * query.range}};
            for (const auto& hitboxes : input.hypotheses) {
                bool hit = false;
                if (input.hitgroup == 1) {
                    RageTrace trace;
                    if (input.box_clipper.clip(input.box_clipper.context, ray, hitboxes.front(),
                                               trace) >= 0) {
                        hit = true;
                        const auto head_fraction = trace.fraction;
                        for (const auto& hitbox : hitboxes) {
                            if (!(hitbox.capsule.radius > 0.0f) ||
                                (hitbox.hitgroup != 2 && hitbox.hitgroup != 3))
                                continue;
                            trace.fraction = 1.0f;
                            if (input.box_clipper.clip(input.box_clipper.context, ray, hitbox,
                                                       trace) >= 0 &&
                                trace.fraction < head_fraction) {
                                hit = false;
                                break;
                            }
                        }
                    }
                } else {
                    const RageSegment shot{input.origin,
                                           advance(input.origin, direction, query.range)};
                    for (const auto& hitbox : hitboxes) {
                        if (!(hitbox.capsule.radius > 0.0f) ||
                            !(input.minimum_hitgroup_multiplier <=
                              rage_detail::hitgroup_multipliers[std::uint32_t(hitbox.hitgroup) &
                                                                15U]))
                            continue;
                        if (hitbox.capsule.radius >=
                            rage_segment_distance({hitbox.capsule.first, hitbox.capsule.second},
                                                  shot)) {
                            hit = true;
                            break;
                        }
                    }
                }
                if (hit)
                    ++successes;
            }
            if (successes) {
                auto filter = input.projected_filter;
                filter.mask = filter.exit_mask = 0x600400bU;
                filter.minimum_damage = 1.0f;
                const auto projected =
                    (input.point_delta[1] * direction[1] + input.point_delta[0] * direction[0]) +
                    input.point_delta[2] * direction[2];
                const auto point = advance(input.origin, direction, projected);
                const auto damage = input.live_bindings
                                        ? final_target_detail::trace_projected(
                                              *input.live_bindings, engine, input, point, query)
                                        : trace_ballistic_bullet(engine, input.source, input.origin,
                                                                 point, filter, query);
                const auto scaled = _mm_cvttss_si32(_mm_set_ss(input.base_damage * query.damage));
                if (damage <= 0 || scaled < input.minimum_damage)
                    successes = 0;
            }
        }
    }
    const auto increment = successes | ((input.safe ? 3U : 1U) << 16);
    const auto packed = votes.packed.fetch_add(increment, std::memory_order_relaxed) + increment;
    const auto failures = std::bit_cast<std::int32_t>(packed) >> 16;
    if (failures - std::int32_t(packed & 0xffffU) > failure_budget)
        votes.stopped.exchange(1, std::memory_order_relaxed);
}

RageHitchance final_hitchance_fraction(const FinalHitchanceVotes& votes,
                                       std::int32_t failure_budget) noexcept {
    const auto packed = votes.packed.load(std::memory_order_relaxed);
    auto hits = packed & 0xffffU;
    auto count = std::bit_cast<std::int32_t>(packed) >> 16;
    if (count - std::int32_t(hits) >= failure_budget ||
        votes.stopped.load(std::memory_order_relaxed)) {
        hits = 0;
        count = 0;
    }
    return {hits, count ? float(hits) / float(count) : 0.0f};
}

RageHitchance evaluate_final_hitchance_inline(BallisticEngine& engine,
                                              const FinalHitchanceSample& input,
                                              std::span<const RageSpreadSample, 128> samples,
                                              const FinalHitchancePlan& plan) {
    FinalHitchanceVotes votes;
    for (std::size_t i = 0; i != plan.samples; ++i)
        evaluate_final_hitchance_sample(engine, input, samples[i * plan.sample_stride],
                                        plan.failure_budget, votes);
    return final_hitchance_fraction(votes, plan.failure_budget);
}

namespace {
struct FinalWork {
    BallisticEngine* engine;
    const FinalHitchanceSample* input;
    RageSpreadSample sample;
    std::int32_t budget;
    FinalHitchanceVotes* votes;
};
struct FinalJobItem {
    FinalWork* work;
#if UINTPTR_MAX == UINT32_MAX
    std::uint32_t padding;
#endif
};
static_assert(sizeof(FinalJobItem) == 8);
#if defined(_M_IX86) || defined(__i386__)
void __fastcall final_sample_job(void* raw) {
#else
void final_sample_job(void* raw) {
#endif
    const auto& work = *static_cast<FinalJobItem*>(raw)->work;
    evaluate_final_hitchance_sample(*work.engine, *work.input, work.sample, work.budget,
                                    *work.votes);
}
}

RageHitchance evaluate_final_hitchance_parallel(BallisticEngine& engine,
                                                const FinalHitchanceSample& input,
                                                std::span<const RageSpreadSample, 128> samples,
                                                const FinalHitchancePlan& plan,
                                                engine::JobDispatcher& jobs) {
    FinalHitchanceVotes votes;
    std::array<FinalWork, 64> work;
    std::array<FinalJobItem, 64> items;
    for (std::size_t i = 0; i != plan.samples; ++i) {
        work[i] = {&engine, &input, samples[i * plan.sample_stride], plan.failure_budget, &votes};
        items[i].work = &work[i];
    }
    auto task = jobs.create_eight(final_sample_job);
    if (!task)
        throw std::bad_alloc();
    jobs.dispatch_eight(*task, items.data(), plan.samples);
    engine::release_job(*task);
    return final_hitchance_fraction(votes, plan.failure_budget);
}

bool final_target_detail::requests_body_aim(const FinalHitchanceFailure& state) {
    return requests_body_aim(state, nullptr);
}
bool final_target_detail::requests_body_aim(const FinalHitchanceFailure& state,
                                            const client::CommandFrame* frame) {
    const auto flags = frame ? frame->flags : state.weapon_flags;
    bool request_body = (flags & 0x40000U) && state.hitgroup == 1 && (flags & 0x400U);
    const auto weapon =
        frame ? reinterpret_cast<void*>(std::uintptr_t(frame->weapon_address)) : state.weapon;
    if (request_body && weapon) {
        const auto maximum = engine::invoke<float>(weapon, 442);
        if (maximum != 0.0f) {
            const auto low = maximum * 0.34f, high = maximum * 0.95f;
            const auto x = frame ? frame->velocity_150[0] : state.velocity[0];
            const auto y = frame ? frame->velocity_150[1] : state.velocity[1];
            const auto speed = std::sqrt(x * x + y * y);
            if (low == high)
                request_body = speed < high;
            else {
                auto fraction = (speed - low) / (high - low);
                fraction = minimum_number(maximum_number(fraction, 0.0f), 1.0f) + 0.0f;
                request_body = fraction == 0.0f;
            }
        }
    }
    if (request_body) {
        const auto delta = frame ? std::bit_cast<float>(frame->field_108) - frame->inaccuracy_c4
                                 : state.current - state.baseline;
        const auto damage = _mm_cvttss_si32(_mm_set_ss(float(state.point_damage) * 0.3125f + 0.5f));
        return delta >= 0.0f && 0.0001f > delta && damage >= state.minimum_damage;
    }
    return false;
}
void apply_final_hitchance_failure(FinalHitchanceFailure& state) {
    if (state.context_flags & 8U)
        return;
    if (final_target_detail::requests_body_aim(state))
        *state.target_aim_mode = 3;
    state.context_flags |= 0x40U;
}

namespace {
bool evaluate_target_hitchance_impl(FinalHitchanceContext& context,
                                    const TargetCandidate& candidate,
                                    const lagcomp::LagRecord& record, std::int32_t minimum_damage,
                                    float minimum_chance, float& chance,
                                    final_target_detail::Producer* producer) {
    if (context.weapon_flags & 0x4000U) {
        chance = 1.0f;
        return true;
    }
    if (!context.target_player)
        return false;
    auto bullet = prepare_target_bullet(context.weapon, context.weapon_item_offset,
                                        context.allow_penetration);
    const auto delta = subtract(candidate.primary.position, context.origin);
    const auto basis = final_target_spread_basis(candidate.primary.position, context.origin);
    if (producer)
        final_target_detail::bind_teammate_policy(context, *producer);
    if (!(context.context_flags & 8U) && context.check_teammates &&
        !context.teammates_are_enemies) {
        const auto scale = context.spread + minimum_number(1.0f, context.inaccuracy);
        constexpr std::array<RageSpreadSample, 4> offsets{{{1, 0}, {0, 1}, {-1, 0}, {0, -1}}};
        auto filter = context.teammate_filter;
        filter.mask = filter.exit_mask = 0x4600400bU;
        filter.minimum_damage = 1.0f;
        for (const auto& offset : offsets) {
            RageVector direction;
            for (std::size_t i = 0; i != 3; ++i)
                direction[i] = (basis.first[i] * (offset.first * scale) + basis.center[i]) +
                               (offset.second * scale) * basis.second[i];
            direction = rage_point_detail::normalize(direction, length(direction));
            if (producer)
                final_target_detail::trace_teammate(*producer, context, direction, bullet);
            else
                trace_target_direction(context.ballistics, context.source, context.origin,
                                       direction, filter, bullet);
            const auto& trace = bullet.trace;
            if ((trace.fraction < 1.0f || trace.start_solid || trace.all_solid) && trace.entity) {
                bool teammate;
                if (producer)
                    teammate = final_target_detail::teammate_hit(*producer, trace.entity);
                else {
                    const auto info = context.ballistics.entity_info(trace.entity);
                    teammate = info.virtual_278 && info.team == context.team;
                }
                if (teammate) {
                    chance = 0;
                    return false;
                }
            }
        }
    }
    const bool safe =
        producer ? final_target_detail::select_studio_hitbox(context, *producer, candidate, record)
                 : (candidate.primary.flags & 1U) && (record.header.flags & 0x80U) &&
                       context.local_hitboxes[std::size_t(candidate.primary.hitbox)].radius > 0.0f;
    const auto plan = plan_final_hitchance(
        safe, minimum_chance, context.context_flags, candidate.entity_index,
        producer ? producer->state.previous_targets : context.previous_targets);
    std::array<std::vector<RageScanHitbox>, 3> geometry;
    RageResolverHitboxes hypotheses;
    float minimum_multiplier = 0, base_damage = 0;
    lagcomp::EntityRecordScope scope(context.entity_runtime, context.target_player,
                                     context.saved_bones);
    if (safe) {
        if ((producer ? producer->frame.weapon_id_d8 : std::int32_t(bullet.weapon_id)) == 31)
            base_damage = 1;
        else {
            const auto info = context.ballistics.entity_info(context.target);
            base_damage = hitgroup_damage_scale(info.armor, info.helmet, candidate.primary.hitgroup,
                                                bullet.weapon_id, bullet.armor_ratio);
            minimum_multiplier =
                rage_detail::hitgroup_multipliers[std::uint32_t(candidate.primary.hitgroup) & 15U];
            const auto shots =
                rage_detail::shots_to_kill(candidate.health, candidate.primary.damage);
            const auto raw = float(candidate.primary.damage) / minimum_multiplier;
            constexpr float multipliers[]{.75f, 1, 1.25f, 4};
            for (const auto multiplier : multipliers) {
                const auto damage = _mm_cvttss_si32(_mm_set_ss(multiplier * raw));
                if (rage_detail::shots_to_kill(candidate.health, damage) <= shots) {
                    minimum_multiplier = multiplier;
                    break;
                }
            }
        }
        // The native consumer prepares the primary set before the three
        // resolver sets, retaining its engine descriptor and matrix ordering.
        const auto primary =
            producer ? final_target_detail::hitbox_geometry(context, *producer, record.matrices)
                     : prepare_rage_scan_hitboxes(context.local_hitboxes, record.matrices,
                                                  context.trigonometry);
        for (std::size_t i = 0; i != 3; ++i) {
            geometry[i] = producer ? final_target_detail::hitbox_geometry(
                                         context, *producer, record.alternatives[i].matrices)
                                   : prepare_rage_scan_hitboxes(context.local_hitboxes,
                                                                record.alternatives[i].matrices,
                                                                context.trigonometry);
            hypotheses[i] = geometry[i];
        }
    } else {
        if (producer)
            final_target_detail::apply_record(context, *producer, scope, record);
        else
            scope.apply(record, context.hitbox_bones, context.current_bone_counter);
    }
    const auto hitgroup = producer ? read<std::int32_t>(producer->selected_descriptor, 4)
                                   : candidate.primary.hitgroup;
    if (producer)
        context.samples = (context.context_flags & 8U) ? producer->state.spread.compensated
                                                       : producer->state.spread.primary;
    FinalHitchanceSample input{context.weapon,
                               context.weapon_item_offset,
                               context.allow_penetration,
                               context.source,
                               context.target,
                               context.origin,
                               candidate.primary.position,
                               delta,
                               basis,
                               minimum_damage,
                               hitgroup,
                               minimum_multiplier,
                               base_damage,
                               safe,
                               hypotheses,
                               context.box_clipper,
                               context.direction_filter,
                               context.projected_filter,
                               producer ? &producer->bindings : nullptr};
    const auto result =
        context.settings_menu.multithreading
            ? evaluate_final_hitchance_parallel(context.ballistics, input, context.samples, plan,
                                                *context.jobs)
            : evaluate_final_hitchance_inline(context.ballistics, input, context.samples, plan);
    chance = result.fraction;
    const bool accepted = !(minimum_chance > chance);
    if (!accepted) {
        FinalHitchanceFailure failure{context.context_flags, context.weapon_flags,
                                      context.weapon,        candidate.primary.hitgroup,
                                      context.velocity,      context.baseline,
                                      context.current,       candidate.primary.damage,
                                      minimum_damage,        context.target_aim_mode};
        if (producer)
            final_target_detail::publish_failure(failure, *producer, candidate.entity_index);
        else
            apply_final_hitchance_failure(failure);
        context.context_flags = failure.context_flags;
    }
    scope.restore();
    return accepted;
}
}
bool evaluate_target_hitchance(FinalHitchanceContext& context, const TargetCandidate& candidate,
                               const lagcomp::LagRecord& record, std::int32_t minimum_damage,
                               float minimum_chance, float& chance) {
    return evaluate_target_hitchance_impl(context, candidate, record, minimum_damage,
                                          minimum_chance, chance, nullptr);
}
bool evaluate_target_hitchance(FinalHitchanceContext& context, const TargetCandidate& candidate,
                               const lagcomp::LagRecord& record, std::int32_t minimum_damage,
                               float minimum_chance, float& chance,
                               final_target_detail::Producer& producer) {
    return evaluate_target_hitchance_impl(context, candidate, record, minimum_damage,
                                          minimum_chance, chance, &producer);
}

TargetShotChoice select_target_for_shot(TargetBatch& batch,
                                        std::span<const FinalTargetEvaluation> evaluations,
                                        std::uint32_t context_flags, std::uint32_t weapon_flags,
                                        std::int32_t weapon_id, float chance_floor,
                                        float point_penalty) {
    if (!batch.count)
        return {};
    if ((context_flags & 0x10U) && weapon_id == 64 && (weapon_flags & 0x40U)) {
        batch.candidates[0]->primary.chance = 1.0f;
        return {batch.candidates[0], 0};
    }
    for (std::size_t i = 0; i != batch.count; ++i) {
        auto& candidate = *batch.candidates[i];
        float chance = 1.0f;
        if (context_flags & 0x10U) {
            const auto& evaluation = evaluations[i];
            const auto evaluate = [&] {
                return evaluate_target_hitchance(*evaluation.context, candidate, *evaluation.record,
                                                 evaluation.minimum_damage,
                                                 evaluation.minimum_chance, chance);
            };
            if (!evaluate()) {
                if (!(weapon_flags & 0x400U) || !(candidate.flags & 8U))
                    continue;
                candidate.flags &= ~8U;
                candidate.primary = candidate.alternate_first;
                if (!evaluate()) {
                    if (!(candidate.flags & 0x10U))
                        continue;
                    candidate.flags &= ~0x10U;
                    candidate.primary = candidate.alternate_second;
                    if (!evaluate())
                        continue;
                }
            }
        }
        const auto effective = (candidate.primary.flags & 0x20U) ? chance * point_penalty : chance;
        if (!(chance_floor >= effective)) {
            candidate.primary.chance = chance;
            return {&candidate, i};
        }
    }
    return {};
}

MovementAngles target_aim_angles(const RageVector& point, const RageVector& origin,
                                 const RageVector& recoil, bool compensate_recoil,
                                 std::uint32_t& weapon_flags, const TargetAngleMath& math) {
    auto direction = subtract(point, origin);
    const auto inverse = 1.0f / (length(direction) + 0x1p-23f);
    for (auto& value : direction)
        value *= inverse;
    MovementAngles angles;
    if (direction[0] == 0.0f && direction[1] == 0.0f)
        angles = {direction[2] > 0.0f ? 270.0f : 90.0f, 0, 0};
    else {
        constexpr float degrees = 57.2957763671875f;
        const auto yaw = float(math.atan2(double(direction[1]), double(direction[0]))) * degrees;
        const auto horizontal =
            std::sqrt(direction[0] * direction[0] + direction[1] * direction[1]);
        const auto pitch = float(math.atan2(double(-direction[2]), double(horizontal))) * degrees;
        angles = {pitch, yaw, 0};
    }
    const auto squared = (recoil[0] * recoil[0] + recoil[1] * recoil[1]) + recoil[2] * recoil[2];
    if (compensate_recoil && squared != 0.0f) {
        angles.pitch -= recoil[0];
        angles.yaw -= recoil[1];
        angles.roll -= recoil[2];
        weapon_flags |= 0x10U;
    }
    normalize_movement_angles(angles);
    return angles;
}

RageShotCommit commit_target_choice(const TargetShotChoice& choice, RageShotState& state,
                                    RageShotCommitState& shared, UserCommand& command,
                                    float special_gate_value, bool alternate_path,
                                    bool send_mode) noexcept {
    const RageShotCandidate candidate{choice.candidate->flags, choice.candidate->primary.position};
    return commit_accepted_rage_shot(state, shared, command, candidate, special_gate_value,
                                     alternate_path, send_mode);
}
}

// ============================================================================
// Subcomponent: targeting_pipeline.cpp
// ============================================================================

namespace skeet::rage {
namespace {
void prepare_target_angles(TargetShotChoice choice, FiringState& state, FiringBranchState& branches,
                           client::Command& command, const FiringPostAngleBindings& post) {
    branches.field_33 = post.recoil_option;
    const auto angles =
        target_aim_angles(choice.candidate->primary.position, state.origin, state.recoil,
                          post.compensate_recoil, state.flags, post.angle_math);
    command.angles = {angles.pitch, angles.yaw, angles.roll};
}
TargetFiringResult commit_aimed_target(TargetShotChoice choice, FiringState& state,
                                       FiringBranchState& branches, client::Command& command,
                                       RageShotState& shot, RageShotCommitState& commit,
                                       const FiringReadinessBindings& readiness) {
    shot.flags = state.flags;
    shot.weapon_id = state.weapon_id;
    shot.special_gate_threshold = state.special_threshold;
    shot.marker = state.marker;
    commit.send_packet = *state.send_packet;
    const auto result =
        commit_target_choice(choice, shot, commit, command.movement, readiness.prediction_time,
                             branches.alternate_path, readiness.send_mode);
    state.flags = shot.flags;
    *state.send_packet = commit.send_packet;
    return {result == RageShotCommit::recorded ? TargetFiringStatus::recorded
                                               : TargetFiringStatus::attack_button_only,
            choice};
}
}
TargetFiringResult select_and_fire_target(
    TargetBatch& batch, std::span<const FinalTargetEvaluation> evaluations,
    std::span<const TargetFiringTiming> timing, FiringState& state, FiringBranchState& branches,
    FiringSharedState& shared, client::Command& command, RageShotState& shot,
    RageShotCommitState& commit, const FiringReadinessBindings& readiness,
    const FiringPostAngleBindings& post, float chance_floor, float point_penalty) {
    const auto choice =
        select_target_for_shot(batch, evaluations, commit.context_flags, state.flags,
                               state.weapon_id, chance_floor, point_penalty);
    if (!choice.candidate)
        return {batch.count ? TargetFiringStatus::rejected_batch : TargetFiringStatus::empty_batch,
                choice};
    const auto& selected_timing = timing[choice.ordinal];
    return fire_target_choice(choice, selected_timing, state, branches, shared, command, shot,
                              commit, readiness, post);
}
TargetFiringResult
fire_target_choice(TargetShotChoice choice, const TargetFiringTiming& selected_timing,
                   FiringState& state, FiringBranchState& branches, FiringSharedState& shared,
                   client::Command& command, RageShotState& shot, RageShotCommitState& commit,
                   const FiringReadinessBindings& readiness, const FiringPostAngleBindings& post) {
    const FiringTargetData target{*choice.candidate, selected_timing.field_b4,
                                  selected_timing.field_13c};
    auto current_readiness = readiness;
    current_readiness.scheduler_flags = commit.context_flags;
    const auto ready =
        evaluate_firing_readiness(state, target, branches, shared, command, current_readiness);
    if (ready == FiringReadiness::retained_target)
        return {TargetFiringStatus::retained_target, choice};
    if (ready == FiringReadiness::stopped)
        return {TargetFiringStatus::readiness_stopped, choice};
    return targeting_detail::aim_and_commit_target(choice, state, branches, command, shot, commit,
                                                   readiness, post);
}
TargetFiringResult targeting_detail::aim_and_commit_target(
    TargetShotChoice choice, FiringState& state, FiringBranchState& branches,
    client::Command& command, RageShotState& shot, RageShotCommitState& commit,
    const FiringReadinessBindings& readiness, const FiringPostAngleBindings& post) {
    prepare_target_angles(choice, state, branches, command, post);
    auto current_post = post;
    current_post.prediction_time = readiness.prediction_time;
    current_post.scheduler_flags = commit.context_flags;
    if (!evaluate_firing_post_angles(state, *choice.candidate, branches, command, current_post))
        return {TargetFiringStatus::post_angles_stopped, choice};
    return commit_aimed_target(choice, state, branches, command, shot, commit, readiness);
}
TargetFiringResult targeting_detail::aim_and_commit_target(
    TargetShotChoice choice, FiringState& state, FiringBranchState& branches,
    client::Command& command, RageShotState& shot, RageShotCommitState& commit,
    const FiringReadinessBindings& readiness, const FiringPostAngleBindings& post,
    const animation::BoneRuntime& bones) {
    prepare_target_angles(choice, state, branches, command, post);
    auto current_post = post;
    current_post.prediction_time = readiness.prediction_time;
    current_post.scheduler_flags = commit.context_flags;
    if (!evaluate_firing_post_angles(state, *choice.candidate, branches, command, current_post,
                                     bones))
        return {TargetFiringStatus::post_angles_stopped, choice};
    return commit_aimed_target(choice, state, branches, command, shot, commit, readiness);
}
}

// ============================================================================
// Subcomponent: targeting_policy.cpp
// ============================================================================

namespace skeet::rage {
void select_target_points(TargetCandidate& candidate, std::span<const TargetPoint> points,
                          std::uint32_t priority_mask) {
    std::vector<RagePointScore> scores;
    scores.reserve(points.size());
    for (const auto& point : points)
        scores.push_back({point.damage, point.hitgroup, point.flags, point.chance, point.priority});
    const auto selection = select_rage_points(scores, priority_mask, candidate.prefer_body,
                                              candidate.prefer_safe, candidate.flags);
    candidate.flags = selection.candidate_flags;
    if (selection.primary != no_rage_point)
        candidate.primary = points[selection.primary];
    if (selection.alternate_first != no_rage_point)
        candidate.alternate_first = points[selection.alternate_first];
    if (selection.alternate_second != no_rage_point)
        candidate.alternate_second = points[selection.alternate_second];
}

bool prepare_target_record_policy(TargetRecordPolicy& policy, std::uint32_t record_flags,
                                  std::uint32_t ordinal, std::uint8_t body_disablers) noexcept {
    if (record_flags & 0x40U) {
        if (ordinal)
            return false;
        policy.evaluation_flags |= 0x20U;
        policy.body.context_ready = true;
    }
    if (record_flags & 0x10U)
        policy.evaluation_flags |= 8U;
    if (record_flags & 0x20U)
        policy.evaluation_flags |= 0x10U;
    apply_rage_body_disablers(policy.body, body_disablers);
    policy.body.hitboxes.safe &= ~0x200U;
    if ((record_flags & 0x70U) || (policy.body.target_state_flags & 0x20U))
        policy.delay_shot = false;
    return true;
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: targeting_records.cpp
// ============================================================================

namespace skeet::rage {
TargetRecordChoice choose_target_record(const TargetRecordHistory& history,
                                        std::int32_t requested_tick, float interval) noexcept {
    if (!history.count)
        return {};
    auto origin = history.origin;
    const auto requested_time = float(requested_tick) * interval;
    TargetRecordChoice result;
    for (std::uint32_t i = 0; i != history.count; ++i) {
        const auto record = history.records[(history.first + i) & 31U];
        const auto x = record->origin[0] - origin[0];
        const auto y = record->origin[1] - origin[1];
        const auto z = record->origin[2] - origin[2];
        if ((y * y + x * x) + z * z > 4096.0f)
            return {};
        origin = record->origin;
        result = {record, i};
        // Native COMISS/JAE continues traversal for unordered values.
        if (requested_time >= float(record->simulation_tick) * interval)
            break;
    }
    return result;
}

TargetRecordCopy prepare_target_record_copy(TargetLagRecordHistory& history,
                                            std::int32_t requested_tick, float interval,
                                            std::uint32_t options, lagcomp::LagRecord& destination,
                                            std::uint32_t& output_flags) noexcept {
    output_flags = 0;
    destination.header.field_00 = 0;
    TargetRecordHistory traversal{history.origin, history.first, history.count, {}};
    for (std::size_t i = 0; i != history.records.size(); ++i)
        if (history.records[i])
            traversal.records[i] = &history.records[i]->header;
    const auto choice = choose_target_record(traversal, requested_tick, interval);
    if (!choice.record)
        return {};
    const auto source = history.records[(history.first + choice.ordinal) & 31U];
    resolver_runtime::advance_pending_attempt(*source, options);
    std::memcpy(&destination, source, sizeof destination);
    destination.header.field_00 = std::uint32_t(reinterpret_cast<std::uintptr_t>(source));
    return {source, choice.ordinal};
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: rage_config.cpp
// ============================================================================

namespace skeet {
namespace {
template <class T>
const T* setting(ConfigRegistries& registries, std::uint32_t hash, ConfigType type) {
    auto* link = registries.descriptors.next ? find_config(registries.descriptors, hash) : nullptr;
    if (!link)
        throw std::invalid_argument("RAGE runtime requires an existing configuration descriptor");
    auto& descriptor = static_cast<ConfigDescriptor&>(*link);
    if (descriptor.type != type)
        throw std::invalid_argument("RAGE runtime configuration has the wrong native type");
    return &static_cast<T&>(descriptor);
}
}
RageRuntimeConfigView::RageRuntimeConfigView(ConfigRegistries& registries)
    : minimum_chance_(setting<IntegerConfig>(registries, 0x3F5E5553, ConfigType::integer)),
      minimum_damage_(setting<IntegerConfig>(registries, 0x18C0E6BE, ConfigType::integer)),
      override_damage_(setting<IntegerConfig>(registries, 0x7DC413FE, ConfigType::integer)),
      double_tap_chance_(setting<IntegerConfig>(registries, 0x3F69C17C, ConfigType::integer)),
      override_enabled_(setting<BooleanConfig>(registries, 0xB80303B4, ConfigType::boolean)),
      delay_shot_(setting<BooleanConfig>(registries, 0xA09C0D52, ConfigType::boolean)),
      selected_hitboxes_(setting<IntegerConfig>(registries, 0x5B900854, ConfigType::integer)),
      avoid_unsafe_(setting<IntegerConfig>(registries, 0xECBBD134, ConfigType::integer)),
      body_disablers_(setting<IntegerConfig>(registries, 0x16C91A85, ConfigType::integer)),
      prefer_safe_(setting<BooleanConfig>(registries, 0xF5A00B3B, ConfigType::boolean)),
      prefer_body_(setting<BooleanConfig>(registries, 0x2B54548D, ConfigType::boolean)) {}
RageThresholdSettings RageRuntimeConfigView::threshold_settings() const noexcept {
    // Native owner 0x434803BC: value offsets +2C,+68,+7C,+B8,+108.
    return {minimum_chance_->value, minimum_damage_->value, override_enabled_->value != 0,
            override_damage_->value, delay_shot_->value != 0};
}
RageThresholds RageRuntimeConfigView::thresholds(std::uint16_t weapon_id,
                                                 std::uint32_t configuration_flags,
                                                 bool override_hotkey_active) const noexcept {
    // Config flag 0x20 selects native global 0x4346A4B0, which is the
    // value of descriptor 0x3F69C17C (Double tap hit chance).
    return prepare_rage_thresholds(threshold_settings(), weapon_id, configuration_flags,
                                   double_tap_chance_->value, override_hotkey_active);
}
RageHitboxMasks RageRuntimeConfigView::hitboxes(std::uint32_t configuration_flags,
                                                bool force_safe_active) const noexcept {
    // Native values 434691CC,43468F78,43467030,43467D70,4346A8D0,
    // consumed directly by 0x433C15E8..0x433C16CC.
    return prepare_rage_hitboxes(std::uint32_t(selected_hitboxes_->value), prefer_safe_->value != 0,
                                 force_safe_active, std::uint32_t(avoid_unsafe_->value),
                                 configuration_flags, prefer_body_->value != 0,
                                 std::uint8_t(body_disablers_->value));
}
void RageRuntimeConfigView::apply_body_disablers(RageBodyAimPolicy& policy) const noexcept {
    apply_rage_body_disablers(policy, std::uint8_t(body_disablers_->value));
}
} // namespace skeet

// ============================================================================
// Subcomponent: rage_damage.cpp
// ============================================================================

namespace skeet {
using namespace rage_detail;

bool hitgroup_armored(std::int32_t armor, std::uint8_t helmet, std::int32_t hitgroup) noexcept {
    if (armor <= 0)
        return false;
    const auto bit = 1U << (std::uint32_t(hitgroup) & 31U);
    if (bit & 0x3dU)
        return true;
    return (bit & 0x102U) && helmet != 0;
}

float hitgroup_damage_scale(std::int32_t armor, std::uint8_t helmet, std::int32_t hitgroup,
                            std::uint16_t weapon_id, float armor_ratio) noexcept {
    if (weapon_id == 0x1fU)
        return 1.0f;
    float scale = hitgroup_multipliers[std::uint32_t(hitgroup) & 15U];
    if (hitgroup_armored(armor, helmet, hitgroup))
        scale *= armor_ratio * 0.5f;
    return scale;
}

std::int32_t final_bullet_damage(float ballistic_damage, float armor_ratio, std::int32_t pellets,
                                 std::uint16_t weapon_id, std::int32_t hitgroup,
                                 std::int32_t target_armor, std::uint8_t target_helmet) noexcept {
    const float scale =
        hitgroup_damage_scale(target_armor, target_helmet, hitgroup, weapon_id, armor_ratio);
    const auto per_pellet = _mm_cvttss_si32(_mm_set_ss(ballistic_damage * scale));
    return signed_word(std::uint32_t(per_pellet) * std::uint32_t(pellets));
}

bool apply_surface_penetration(BulletPenetrationState& state,
                               const SurfacePenetration& surface) noexcept {
    const auto entry_material = surface.entry_surface.material;
    float combined, damage_modifier;
    if (entry_material == 0x47 || entry_material == 0x59) {
        combined = 3.0f;
        damage_modifier = 0.05f;
    } else {
        combined = surface.contents_bit_3 || surface.surface_flag_bit_7
                       ? 1.0f
                       : (surface.exit_surface.penetration_modifier +
                          surface.entry_surface.penetration_modifier) *
                             0.5f;
        damage_modifier = 0.16f;
    }
    if (entry_material == surface.exit_surface.material) {
        if (entry_material == 0x55 || entry_material == 0x57)
            combined = 3.0f;
        else if (entry_material == 0x4c)
            combined = 2.0f;
    }
    const float x = surface.exit[0] - surface.entry[0];
    const float y = surface.exit[1] - surface.entry[1];
    const float z = surface.exit[2] - surface.entry[2];
    // Native performs sqrt followed by squaring; keeping both preserves rounding.
    const float thickness = std::sqrt((x * x + y * y) + z * z);
    const float inverse = maximum_number(0.0f, 1.0f / combined);
    const float weapon_term =
        maximum_number(0.0f, float((3.0 / double(surface.weapon_penetration)) * 1.25));
    const float thickness_term = ((thickness * thickness) * inverse) / 24.0f;
    const float damage_term = state.damage * damage_modifier;
    const float combined_loss =
        float(double(weapon_term) * (double(inverse) * 3.0) + double(damage_term));
    const float lost = maximum_number(0.0f, combined_loss + thickness_term);
    if (!std::isfinite(lost)) {
        state.damage = 0.0f;
        return false;
    }
    state.damage -= lost;
    if (1.0f > state.damage) {
        state.damage = 0.0f;
        return false;
    }
    state.position = surface.exit;
    state.remaining_penetrations = add(state.remaining_penetrations, -1);
    return true;
}

std::int32_t rage_minimum_damage(std::int32_t raw, std::int32_t health, std::uint32_t flags,
                                 float source_factor, std::int32_t source_limit) noexcept {
    if (raw >= 100)
        return add(health, add(raw, -100));
    if (raw != 0)
        return std::min(raw, add(health, 5));
    float candidate = float(std::max(10, add(health, 1))) * source_factor;
    if (!(flags & 4U))
        candidate *= 1.25f;
    candidate = minimum_number(candidate, float(source_limit) - 1.0f);
    candidate = maximum_number(candidate, 1.0f);
    auto effective = _mm_cvttss_si32(_mm_set_ss(candidate + 0.5f));
    const auto shots = shots_to_kill(health, effective);
    if (shots > 1) {
        const auto quartered = signed_word(std::uint32_t(health / shots) << 2);
        effective = std::min(effective, add(quartered / 3, 1));
    }
    return effective;
}
} // namespace skeet

// ============================================================================
// Subcomponent: rage_geometry.cpp
// ============================================================================

namespace skeet {
namespace {
constexpr float epsilon = 0x1p-23f;
constexpr float maximum_float = std::numeric_limits<float>::max();
float dot_yxz(const RageVector& a, const RageVector& b) noexcept {
    return (a[1] * b[1] + a[0] * b[0]) + a[2] * b[2];
}
float length_yzx(const RageVector& value) noexcept {
    return std::sqrt((value[1] * value[1] + value[2] * value[2]) + value[0] * value[0]);
}
RageVector subtract(const RageVector& a, const RageVector& b) noexcept {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}
RageVector scale(const RageVector& value, float factor) noexcept {
    return {value[0] * factor, value[1] * factor, value[2] * factor};
}
RageVector cross(const RageVector& a, const RageVector& b) noexcept {
    return {a[1] * b[2] - a[2] * b[1], a[2] * b[0] - a[0] * b[2], a[0] * b[1] - a[1] * b[0]};
}
float clamp_sse(float value, float limit) noexcept {
    const auto low = _mm_max_ss(_mm_set_ss(value), _mm_setzero_ps());
    return _mm_cvtss_f32(_mm_min_ss(low, _mm_set_ss(limit)));
}
// 0x433C918F. The source builds and orthogonalizes this seed vector, including
// its zero component; replacing it with a conventional cross product differs.
RageVector perpendicular(const RageVector& axis) noexcept {
    const float seed_x = (1.0f - axis[2]) * (axis[1] * axis[1] - 0.0f) + axis[2];
    const float seed_z = -axis[0];
    const float inverse = 1.0f / (std::sqrt((seed_x * seed_x + 0.0f) + seed_z * seed_z) + epsilon);
    const RageVector seed{seed_x * inverse, inverse * 0.0f, seed_z * inverse};
    const float projection = (seed[1] * axis[1] + axis[0] * seed[0]) + seed[2] * axis[2];
    const RageVector rejected{seed[0] - axis[0] * projection, seed[1] - axis[1] * projection,
                              seed[2] - axis[2] * projection};
    const float normalized =
        1.0f / (std::sqrt((rejected[0] * rejected[0] + rejected[1] * rejected[1]) +
                          rejected[2] * rejected[2]) +
                epsilon);
    return scale(rejected, normalized);
}
}

CapsuleIntersection intersect_rage_capsule(const RageRay& ray,
                                           const RageCapsule& capsule) noexcept {
    CapsuleIntersection result;
    const auto segment = subtract(capsule.second, capsule.first);
    const float segment_length = length_yzx(segment);
    auto relative = subtract(ray.start, capsule.first);
    const float radius = capsule.radius;
    if (radius < 0.00001f)
        return result;
    if (0.001f > segment_length) {
        const float speed_squared = dot_yxz(ray.delta, ray.delta);
        const float offset_squared = dot_yxz(relative, relative);
        const float outside = offset_squared - radius * radius;
        if (!(0x1p-46f > speed_squared)) {
            const float along = dot_yxz(ray.delta, relative);
            const float discriminant = along * along - outside * speed_squared;
            if (!(0.0f > discriminant)) {
                const float fraction = (-along - std::sqrt(discriminant)) / speed_squared;
                if (!(0.0f > fraction)) {
                    result.fraction = fraction;
                    for (unsigned i = 0; i < 3; ++i) {
                        const float displacement = ray.delta[i] * fraction;
                        result.point[i] = ray.start[i] + displacement;
                        result.normal[i] = (displacement + relative[i]) * (1.0f / radius);
                    }
                    return result;
                }
                if (!(outside <= 0.0f)) {
                    result.fraction = maximum_float;
                    return result;
                }
            } else {
                result.fraction = maximum_float;
                return result;
            }
        } else if (!(outside <= 0.0f)) {
            result.fraction = maximum_float;
            return result;
        }
        result.fraction = 0.0f;
        result.point = ray.start;
        result.normal = offset_squared > epsilon ? scale(relative, 1.0f / std::sqrt(offset_squared))
                                                 : RageVector{0, 0, 1};
        return result;
    }

    const auto axis = scale(segment, 1.0f / segment_length);
    const float ray_length = length_yzx(ray.delta);
    if (!(ray_length > 0.0001f)) {
        const float along = dot_yxz(relative, axis);
        if (!(0.0f > along)) {
            if (!(along <= segment_length))
                relative = subtract(ray.start, capsule.second);
            else {
                // Native 0x43419186 uses the first endpoint, not the unit axis.
                for (unsigned i = 0; i < 3; ++i)
                    relative[i] -= capsule.first[i] * along;
            }
        }
        const float offset_squared = dot_yxz(relative, relative);
        // The native stationary path compares squared distance with radius.
        if (!(offset_squared <= radius))
            return result;
        result.fraction = 0.0f;
        result.start_solid = true;
        result.normal = offset_squared > 1.0e-8f ? scale(relative, 1.0f / std::sqrt(offset_squared))
                                                 : perpendicular(axis);
        result.point = ray.start;
        return result;
    }

    const auto direction = scale(ray.delta, 1.0f / ray_length);
    auto plane_normal = cross(axis, direction);
    const float normal_squared = dot_yxz(plane_normal, plane_normal);
    const float axis_motion = dot_yxz(ray.delta, axis);
    float fraction = maximum_float;
    float axis_position = 0;
    if (!(normal_squared > 0x1p-30f)) {
        const float projection = dot_yxz(relative, axis);
        RageVector radial;
        for (unsigned i = 0; i < 3; ++i)
            radial[i] = relative[i] - axis[i] * projection;
        const float radial_squared = dot_yxz(radial, radial);
        float extent = radius;
        bool has_extent = true;
        if (!(0x1p-46f > radial_squared)) {
            const auto radial_direction = scale(radial, 1.0f / std::sqrt(radial_squared));
            const auto normal = cross(radial_direction, axis);
            const float off_plane = dot_yxz(relative, normal);
            const float available = (radius * radius - radial_squared) - off_plane * off_plane;
            if (!(available > 0.0f)) {
                axis_position = radial_squared;
                has_extent = false;
            } else
                extent = std::sqrt(available);
        }
        if (has_extent) {
            if (0.0f > projection + extent) {
                const float distance = -(projection + extent);
                axis_position = 0.0f;
                if (!(axis_motion < distance))
                    fraction = distance / axis_motion;
            } else if (segment_length + extent > projection) {
                axis_position = clamp_sse(projection, segment_length);
                fraction = 0.0f;
            } else {
                const float distance = projection - (segment_length + extent);
                const float backward_motion = -axis_motion;
                axis_position = segment_length;
                if (!(backward_motion < distance))
                    fraction = distance / backward_motion;
            }
        }
    } else {
        plane_normal = scale(plane_normal, 1.0f / std::sqrt(normal_squared));
        const float off_plane = dot_yxz(relative, plane_normal);
        const float available = radius * radius - off_plane * off_plane;
        if (0.0f >= available) {
            result.fraction = maximum_float;
            return result;
        }
        const auto lateral_axis = cross(plane_normal, axis);
        float projection = dot_yxz(relative, axis);
        const float lateral = dot_yxz(relative, lateral_axis);
        const float lateral_motion = dot_yxz(ray.delta, lateral_axis);
        const float closest_axis_position = clamp_sse(projection, segment_length);
        const float beyond_axis = projection - closest_axis_position;
        if (available > beyond_axis * beyond_axis + lateral * lateral) {
            fraction = 0.0f;
            axis_position = closest_axis_position;
        } else {
            const float cylinder_fraction = (-std::sqrt(available) - lateral) / lateral_motion;
            const float inverse_motion =
                1.0f / (lateral_motion * lateral_motion + axis_motion * axis_motion);
            const float cylinder_axis_position =
                maximum_number(0.0f, cylinder_fraction) * axis_motion + projection;
            bool endpoint = true;
            if (!(0.0f > cylinder_axis_position)) {
                if (!(segment_length <= cylinder_axis_position)) {
                    axis_position = cylinder_axis_position;
                    if (!(cylinder_fraction < 0.0f))
                        fraction = cylinder_fraction;
                    endpoint = false;
                } else {
                    axis_position = segment_length;
                    projection -= segment_length;
                }
            }
            if (endpoint) {
                const float along =
                    (projection * axis_motion + lateral_motion * lateral) * inverse_motion;
                const float discriminant =
                    along * along -
                    ((projection * projection + lateral * lateral) - available) * inverse_motion;
                if (!(discriminant < 0.0f)) {
                    const float endpoint_fraction = -along - std::sqrt(discriminant);
                    if (!(endpoint_fraction < 0.0f))
                        fraction = endpoint_fraction;
                }
            }
        }
    }
    result.fraction = fraction;
    RageVector normal;
    for (unsigned i = 0; i < 3; ++i) {
        result.point[i] = fraction * ray.delta[i] + ray.start[i];
        normal[i] = result.point[i] - (axis_position * axis[i] + capsule.first[i]);
    }
    const float inverse_normal =
        1.0f / (std::sqrt((normal[0] * normal[0] + normal[1] * normal[1]) + normal[2] * normal[2]) +
                epsilon);
    result.normal = scale(normal, inverse_normal);
    return result;
}

int clip_rage_ray_to_capsule(const RageRay& ray, const RageCapsule& capsule,
                             RageTrace& trace) noexcept {
    RageRay shortened = ray;
    shortened.delta = scale(ray.delta, trace.fraction);
    const auto result = intersect_rage_capsule(shortened, capsule);
    if (!(1.0f > result.fraction))
        return -1;
    trace.fraction *= result.fraction;
    if (result.start_solid)
        trace.start_solid = true;
    trace.point = result.point;
    trace.normal = result.normal;
    return 0;
}
} // namespace skeet

// ============================================================================
// Subcomponent: rage_head_points.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace rage_point_detail;
bool head_obstructed(const RageVector& point, const RageVector& origin,
                     const std::array<RageCapsule, 3>& heads,
                     const std::array<RageCapsule, 3>& bodies) noexcept {
    for (unsigned i = 0; i < 3; ++i) {
        RageTrace trace{1.0f, point, {}, false};
        clip_rage_ray_to_capsule({origin, subtract(point, origin)}, heads[i], trace);
        if (rage_segment_distance({bodies[i].first, bodies[i].second}, {origin, trace.point}) <=
            bodies[i].radius)
            return true;
    }
    return false;
}
bool intersects_other_heads(const RageRay& ray, unsigned selected,
                            const std::array<RageCapsule, 3>& heads) noexcept {
    for (unsigned i = 0; i < 3; ++i) {
        if (i == selected)
            continue;
        const auto& capsule = heads[i];
        const auto closest = closest_rage_ray_segment(ray, {capsule.first, capsule.second});
        const float weight = 1.0f - closest.second_fraction;
        RageVector delta;
        delta[0] = (capsule.second[0] * closest.second_fraction + weight * capsule.first[0]) -
                   (ray.delta[0] * closest.first_fraction + ray.start[0]);
        delta[1] = (capsule.second[1] * closest.second_fraction + capsule.first[1] * weight) -
                   (ray.delta[1] * closest.first_fraction + ray.start[1]);
        delta[2] = (capsule.first[2] * weight + closest.second_fraction * capsule.second[2]) -
                   (ray.delta[2] * closest.first_fraction + ray.start[2]);
        // COMISS/JB also rejects unordered distances from a zero-length axis.
        if (!(capsule.radius >= length(delta)))
            return false;
    }
    return true;
}
std::vector<RageAimPoint> search_head_points(const RageVector& origin,
                                             const SafePointBounds& bounds,
                                             const std::array<RageCapsule, 3>& bodies,
                                             std::int32_t hitgroup, const RageTrigonometry& trig) {
    const auto& heads = bounds.last_hypotheses;
    const auto& side = bounds.basis.side;
    const auto& upper = bounds.basis.upper;
    const RageVector opposite{-side[0], -side[1], -side[2]};
    const RageVector axes[]{side, opposite, upper};
    std::array<float, 3> scores{-1.0f, -1.0f, -1.0f};
    std::array<RageVector, 3> selected{};
    float angle = 0.0f;
    for (unsigned step = 0; step < 15; ++step, angle += 0.10471975803375244f) {
        const float sine = trig.sine(angle);
        const float cosine = trig.cosine(angle);
        const RageVector vertical{upper[0] * cosine, upper[1] * cosine, upper[2] * cosine};
        for (unsigned sign = 0; sign < 2; ++sign) {
            const auto& lateral = axes[sign];
            const RageVector direction{lateral[0] * sine + vertical[0],
                                       lateral[1] * sine + vertical[1],
                                       lateral[2] * sine + vertical[2]};
            const auto normalized = normalize(direction, length(direction));
            const RageVector delta{normalized[0] * -1000.0f, normalized[1] * -1000.0f,
                                   normalized[2] * -1000.0f};
            for (unsigned i = 0; i < 3; ++i) {
                const auto center = midpoint(heads[i]);
                const RageRay cross_section{{center[0] - delta[0] * 0.5f,
                                             center[1] - delta[1] * 0.5f,
                                             center[2] - delta[2] * 0.5f},
                                            delta};
                const auto intersection = intersect_rage_capsule(cross_section, heads[i]);
                const auto to_point = subtract(intersection.point, origin);
                const RageRay ray{origin, normalize(to_point, length(to_point))};
                if (!intersects_other_heads(ray, i, heads) ||
                    head_obstructed(intersection.point, origin, heads, bodies))
                    continue;
                for (unsigned axis = 0; axis < 3; ++axis) {
                    const float score =
                        (axes[axis][1] * ray.delta[1] + axes[axis][0] * ray.delta[0]) +
                        axes[axis][2] * ray.delta[2];
                    if (score > scores[axis]) {
                        scores[axis] = score;
                        selected[axis] = intersection.point;
                    }
                }
            }
        }
    }
    if (scores[0] == -1.0f || scores[1] == -1.0f || scores[2] == -1.0f)
        return {};
    return {{selected[0], 1.0f, hitgroup, 3},
            {selected[1], 1.0f, hitgroup, 3},
            {selected[2], 1.0f, hitgroup, 3}};
}
}

std::vector<RageAimPoint>
generate_rage_head_safe_points(const RagePointGeneration& settings, const RageVector& center,
                               const RageVector& target_origin, const RageLocalHitbox& head,
                               std::span<const Matrix3x4, 3> hypotheses,
                               const RageHeadOcclusion* occlusion, const RageTrigonometry& trig) {
    using namespace rage_point_detail;
    auto bounds = build_safe_point_bounds(settings.origin, center, head, hypotheses);
    if (bounds.points.empty())
        return {};
    if (occlusion) {
        std::array<RageCapsule, 3> bodies;
        for (unsigned i = 0; i < 3; ++i)
            bodies[i] = transform_capsule(occlusion->hitbox, occlusion->matrices[i]);
        // The original checks the upper bound first, then the lower bound.
        if (head_obstructed(bounds.points[1].position, settings.origin, bounds.last_hypotheses,
                            bodies) ||
            head_obstructed(bounds.points[0].position, settings.origin, bounds.last_hypotheses,
                            bodies))
            bounds.points =
                search_head_points(settings.origin, bounds, bodies, head.hitgroup, trig);
    }
    return finish_safe_points(std::move(bounds.points), settings, target_origin, 0, head.hitgroup);
}
} // namespace skeet

// ============================================================================
// Subcomponent: rage_hitboxes.cpp
// ============================================================================

namespace skeet {
namespace {
Matrix3x4 hitbox_rotation(const RageVector& rotation, const RageTrigonometry& trig) {
    constexpr float radians = 0.01745329238474369f;
    const float sy = trig.sine(rotation[1] * radians);
    const float cy = trig.cosine(rotation[1] * radians);
    const float sp = trig.sine(rotation[0] * radians);
    const float cp = trig.cosine(rotation[0] * radians);
    const float sr = trig.sine(rotation[2] * radians);
    const float cr = trig.cosine(rotation[2] * radians);
    const float crcy = cr * cy;
    const float crsy = cr * sy;
    const float srcy = sr * cy;
    const float srsy = sr * sy;
    return {{{cp * cy, srcy * sp - crsy, crcy * sp + srsy, 0.0f},
             {cp * sy, srsy * sp + crcy, crsy * sp - srcy, 0.0f},
             {-sp, sr * cp, cr * cp, 0.0f}}};
}
Matrix3x4 compose_hitbox_matrix(const Matrix3x4& bone, const Matrix3x4& rotation) noexcept {
    Matrix3x4 result;
    for (unsigned row = 0; row < 3; ++row) {
        for (unsigned column = 0; column < 4; ++column) {
            const float combined = (bone.rows[row][2] * rotation.rows[2][column] +
                                    bone.rows[row][1] * rotation.rows[1][column]) +
                                   bone.rows[row][0] * rotation.rows[0][column];
            result.rows[row][column] = combined + (column == 3 ? bone.rows[row][3] : 0.0f);
        }
    }
    return result;
}
}

std::vector<RageScanHitbox> prepare_rage_scan_hitboxes(std::span<const RageLocalHitbox> local,
                                                       const HitboxMatrices& matrices,
                                                       const RageTrigonometry& trig) {
    std::vector<RageScanHitbox> result;
    const auto count = std::min(local.size(), matrices.size());
    result.reserve(count);
    for (std::size_t index = 0; index < count; ++index) {
        const auto& hitbox = local[index];
        const auto matrix =
            compose_hitbox_matrix(matrices[index], hitbox_rotation(hitbox.rotation, trig));
        RageCapsule capsule{{}, {}, hitbox.radius};
        for (unsigned row = 0; row < 3; ++row) {
            const auto& m = matrix.rows[row];
            capsule.first[row] =
                ((hitbox.minimum[1] * m[1] + m[0] * hitbox.minimum[0]) + m[2] * hitbox.minimum[2]) +
                m[3];
            const float pair = row == 2 ? m[1] * hitbox.maximum[1] + m[0] * hitbox.maximum[0]
                                        : hitbox.maximum[0] * m[0] + hitbox.maximum[1] * m[1];
            capsule.second[row] = (pair + m[2] * hitbox.maximum[2]) + m[3];
        }
        result.push_back({hitbox.hitgroup, capsule, hitbox.engine_hitbox, matrix});
    }
    return result;
}
} // namespace skeet

// ============================================================================
// Subcomponent: rage_hitchance.cpp
// ============================================================================

namespace skeet {
namespace {
RageRay sample_ray(const RageHitchanceInput& input, const RageSpreadSample& sample) noexcept {
    RageVector direction;
    for (unsigned i = 0; i < 3; ++i)
        direction[i] = (sample.first * input.basis.first[i] + input.basis.center[i]) +
                       sample.second * input.basis.second[i];
    const float length = std::sqrt((direction[0] * direction[0] + direction[1] * direction[1]) +
                                   direction[2] * direction[2]);
    const float inverse = 1.0f / (length + 0x1p-23f);
    for (auto& component : direction)
        component *= inverse;
    const float normalized_length = std::sqrt(
        (direction[0] * direction[0] + direction[1] * direction[1]) + direction[2] * direction[2]);
    const float scale = input.distance / (normalized_length + 0x1p-23f);
    for (auto& component : direction)
        component *= scale;
    return {input.origin, direction};
}
}

RageHitchance evaluate_rage_hitchance(const RageHitchanceInput& input,
                                      const std::array<RageSpreadSample, 32>& samples,
                                      const RageBoxClipper& box_clipper) {
    constexpr std::array<std::uint32_t, 15> hitbox_order{0,  2, 3, 4, 6,  15, 16, 17,
                                                         18, 7, 8, 9, 10, 11, 12};
    constexpr std::array<std::uint32_t, 16> hitgroup_rank{5, 0, 2, 1, 4, 4, 6, 6,
                                                          3, 5, 5, 5, 5, 5, 5, 5};
    std::uint32_t hits = 0;
    for (const auto& sample : samples) {
        const auto ray = sample_ray(input, sample);
        RageTrace trace;
        float nearest_fraction = 1.0f;
        std::uint32_t rank_limit = 0x7fffffffU;
        const RageScanHitbox* selected = nullptr;
        for (const auto index : hitbox_order) {
            if (index >= input.hitboxes.size())
                break;
            const auto& hitbox = input.hitboxes[index];
            if (hitgroup_rank[std::uint32_t(hitbox.hitgroup) & 15U] > rank_limit)
                break;
            trace.fraction = 1.0f;
            const int clipped = hitbox.capsule.radius > 0.0f
                                    ? clip_rage_ray_to_capsule(ray, hitbox.capsule, trace)
                                    : box_clipper.clip(box_clipper.context, ray, hitbox, trace);
            if (clipped < 0 || !(nearest_fraction > trace.fraction))
                continue;
            nearest_fraction = trace.fraction;
            selected = &hitbox;
            if (hitbox.hitgroup != 1)
                break;
            rank_limit = 2;
        }
        if (!selected)
            continue;
        const auto group = selected->hitgroup;
        if (input.record_count <= 1 && input.preferred_hitgroup != group &&
            !(rage_detail::hitgroup_multipliers[std::uint32_t(group) & 15U] >=
              input.minimum_hitgroup_multiplier))
            continue;
        ++hits;
    }
    return {hits, float(hits) * 0.03125f};
}
} // namespace skeet

// ============================================================================
// Subcomponent: rage_point_scaling.cpp
// ============================================================================

namespace skeet::rage_point_detail {
namespace {
struct PointSegmentDistance {
    float distance;
    float projection;
};
// 0x4338590E. Distances beyond either endpoint are negative, while projection
// remains the signed distance along the segment's epsilon-normalized axis.
PointSegmentDistance point_segment_distance(const RageVector& point, const RageVector& from,
                                            const RageVector& to) noexcept {
    const auto delta = subtract(to, from);
    const float magnitude = length(delta);
    const auto axis = normalize(delta, magnitude);
    const auto relative = subtract(point, from);
    const float projection =
        (axis[1] * relative[1] + axis[0] * relative[0]) + axis[2] * relative[2];
    if (projection < 0.0f)
        return {-length(relative), projection};
    if (projection > magnitude)
        return {-length(subtract(point, to)), projection};
    const RageVector closest{axis[0] * projection + from[0], axis[1] * projection + from[1],
                             axis[2] * projection + from[2]};
    return {length(subtract(point, closest)), projection};
}
} // namespace

void scale_points(std::vector<RageAimPoint>& points, const RageVector& center,
                  const RagePointGeneration& settings) {
    if (settings.fixed_scale) {
        for (auto& point : points) {
            point.position = interpolate(center, point.position, settings.point_scale);
            point.scale = settings.point_scale;
        }
        return;
    }
    const auto delta = subtract(center, settings.origin);
    const float distance = length(delta);
    const auto axis = normalize(delta, distance);
    const float accuracy = float(settings.accuracy_percent) * 0.01f;
    const float margin = ((distance * settings.spread) * accuracy) * accuracy;
    const RageVector extended{axis[0] * 60.0f + center[0], axis[1] * 60.0f + center[1],
                              axis[2] * 60.0f + center[2]};
    for (auto it = points.begin(); it != points.end();) {
        const auto projected = point_segment_distance(it->position, settings.origin, extended);
        const float remaining = projected.distance - margin;
        if (!(remaining > 0.0f)) {
            it = points.erase(it);
            continue;
        }
        const float axial = std::fabs(projected.projection - distance);
        const float radius = std::sqrt(remaining * remaining + axial * axial);
        const float ratio = radius / length(subtract(center, it->position));
        const float limited = minimum_number(ratio, settings.point_scale);
        const auto low = _mm_max_ss(_mm_set_ss(limited), _mm_set_ss(0.25f));
        const float fraction = _mm_cvtss_f32(_mm_min_ss(low, _mm_set_ss(0.9f)));
        it->position = interpolate(center, it->position, fraction);
        it->scale = fraction;
        ++it;
    }
}
} // namespace skeet::rage_point_detail

// ============================================================================
// Subcomponent: rage_points.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace rage_point_detail;
constexpr unsigned order[]{3, 4, 0, 6, 8, 7, 10, 9, 17, 15, 18, 16, 11, 12};
PointBasis point_basis(const RageVector& center, const RageVector& origin) noexcept {
    const auto delta = subtract(center, origin);
    const auto axis = normalize(
        delta, std::sqrt((delta[2] * delta[2] + delta[0] * delta[0]) + delta[1] * delta[1]));
    return basis_from_axis(axis);
}
bool obstructs_head(const RageCapsule& body, const RageVector& origin,
                    const RageVector& point) noexcept {
    return body.radius >= rage_segment_distance({body.first, body.second}, {origin, point});
}
void capsule_points(std::vector<RageAimPoint>& points, const RageScanHitbox& hitbox,
                    const RageCapsule& body, const RageVector& origin, const PointBasis& basis) {
    const auto& capsule = hitbox.capsule;
    const auto append = [&](const RageVector& position) {
        points.push_back({position, 1.0f, hitbox.hitgroup, 2});
    };
    if (hitbox.hitgroup >= 4 && hitbox.hitgroup <= 7) {
        const auto delta = subtract(capsule.second, capsule.first);
        const auto axis = normalize(delta, length(delta));
        append({capsule.radius * axis[0] + capsule.second[0],
                capsule.radius * axis[1] + capsule.second[1],
                capsule.radius * axis[2] + capsule.second[2]});
    } else if (hitbox.hitgroup == 3) {
        append(support(capsule, basis.side, true));
        append(support(capsule, {-basis.side[0], -basis.side[1], -basis.side[2]}));
    } else {
        const RageVector directions[]{
            basis.upper, basis.side, {-basis.side[0], -basis.side[1], -basis.side[2]}};
        for (auto direction : directions) {
            auto point = support(capsule, direction);
            if (hitbox.hitgroup == 1 && obstructs_head(body, origin, point)) {
                const RageVector sum{direction[0] + basis.upper[0], direction[1] + basis.upper[1],
                                     direction[2] + basis.upper[2]};
                direction = normalize(
                    sum, std::sqrt((sum[1] * sum[1] + sum[2] * sum[2]) + sum[0] * sum[0]));
                point = support(capsule, direction);
            }
            append(point);
        }
    }
}
void box_points(std::vector<RageAimPoint>& points, const RageLocalHitbox& local,
                const RageScanHitbox& hitbox, const RageVector& origin, const PointBasis& basis) {
    const float middle_y = (local.maximum[1] + local.minimum[1]) * 0.5f;
    const RageVector corners[]{{local.minimum[0], middle_y, local.minimum[2]},
                               {local.maximum[0], middle_y, local.maximum[2]},
                               {local.maximum[0], middle_y, local.minimum[2]},
                               {local.minimum[0], middle_y, local.maximum[2]}};
    float maximum = -std::numeric_limits<float>::max(), minimum = std::numeric_limits<float>::max();
    RageVector highest, lowest;
    for (const auto& corner : corners) {
        RageVector position;
        for (unsigned row = 0; row < 3; ++row) {
            const auto& matrix = hitbox.transform.rows[row];
            position[row] =
                ((corner[1] * matrix[1] + matrix[0] * corner[0]) + matrix[2] * corner[2]) +
                matrix[3];
        }
        const auto delta = subtract(position, origin);
        const float projection =
            ((delta[0] * basis.side[0] + delta[1] * basis.side[1]) + delta[2] * basis.side[2]) /
            length(delta);
        if (projection > maximum) {
            maximum = projection;
            highest = position;
        }
        if (minimum > projection) {
            minimum = projection;
            lowest = position;
        }
    }
    if (maximum != -std::numeric_limits<float>::max())
        points.push_back({highest, 1.0f, local.hitgroup, 2});
    if (minimum != std::numeric_limits<float>::max())
        points.push_back({lowest, 1.0f, local.hitgroup, 2});
}
}

std::vector<RageAimPoint>
generate_rage_regular_points(const RagePointGeneration& settings,
                             std::span<const RageLocalHitbox, 19> local,
                             std::span<const RageScanHitbox, 19> hitboxes) {
    std::vector<RageAimPoint> result;
    for (const auto index : order) {
        const auto points = generate_rage_regular_hitbox_points(settings, index, local, hitboxes);
        result.insert(result.end(), points.begin(), points.end());
    }
    return result;
}

std::vector<RageAimPoint>
generate_rage_regular_hitbox_points(const RagePointGeneration& settings, std::uint32_t index,
                                    std::span<const RageLocalHitbox> local,
                                    std::span<const RageScanHitbox> hitboxes) {
    std::vector<RageAimPoint> result;
    if (index >= local.size() || index >= hitboxes.size())
        return result;
    const auto group_bit =
        index == 11 || index == 12 ? 0x200u : 1u << (std::uint32_t(local[index].hitgroup) & 31u);
    if (!(settings.regular_mask & group_bit))
        return result;
    const auto& hitbox = hitboxes[index];
    const auto center = midpoint(hitbox.capsule);
    RageAimPoint primary{center, 0.0f, local[index].hitgroup, 0};
    if (hitbox.hitgroup == 1) {
        auto lowered = center;
        lowered[2] -= 1.0f;
        if (obstructs_head(hitboxes[6].capsule, settings.origin, lowered))
            primary.position =
                interpolate(center, support(hitbox.capsule, {0.0f, 0.0f, 1.0f}, true), 0.6f);
    }
    std::vector<RageAimPoint> extra;
    if (settings.multipoint_mask & group_bit) {
        const auto basis = point_basis(center, settings.origin);
        if (hitbox.capsule.radius > 0.0f)
            capsule_points(extra, hitbox, hitboxes[6].capsule, settings.origin, basis);
        else
            box_points(extra, local[index], hitbox, settings.origin, basis);
        scale_points(extra, center, settings);
    }
    if (hitbox.hitgroup == 3)
        primary.position = interpolate(primary.position, midpoint(hitboxes[2].capsule), 0.5f);
    result.push_back(primary);
    result.insert(result.end(), extra.begin(), extra.end());
    return result;
}
} // namespace skeet

// ============================================================================
// Subcomponent: rage_policy.cpp
// ============================================================================

namespace skeet {
using namespace rage_detail;

RageHitboxMasks prepare_rage_hitboxes(std::uint32_t selected, bool prefer_safe, bool force_safe,
                                      std::uint32_t avoid_unsafe, std::uint32_t configuration_flags,
                                      bool prefer_body, std::uint8_t body_disablers) noexcept {
    RageHitboxMasks result{selected, prefer_safe ? selected : 0U};
    if (force_safe) {
        result.safe |= result.regular;
        result.regular = 0;
    } else if (avoid_unsafe) {
        const auto moved = avoid_unsafe & result.regular;
        result.safe |= moved;
        result.regular &= ~moved;
        if (avoid_unsafe & 0x200U) {
            result.regular &= ~0x200U;
            result.safe &= ~0x200U;
        }
    }
    if (!(configuration_flags & 0x400U) && prefer_body && (body_disablers & 8U))
        result.safe |= 2U;
    return result;
}

void apply_rage_body_disablers(RageBodyAimPolicy& policy, std::uint8_t options) noexcept {
    std::uint32_t reasons = 0;
    if (policy.context_ready && ((policy.hitboxes.regular | policy.hitboxes.safe) & 2U)) {
        if ((options & 2U) && (policy.target_state_flags & 8U))
            reasons |= 2U;
        if ((options & 4U) && (policy.target_state_flags & 0x20U))
            reasons |= 4U;
        if ((options & 8U) && (policy.target_state_flags & 0x80U))
            reasons |= 8U;
        if (reasons & 8U)
            policy.hitboxes.safe |= 2U;
    }
    policy.disabler_reasons = reasons;
    if (!(policy.target_state_flags & 0x80U))
        policy.hitboxes.safe = 0;
}

RageThresholds prepare_rage_thresholds(const RageThresholdSettings& settings,
                                       std::uint16_t weapon_id, std::uint32_t configuration_flags,
                                       std::int32_t forced_chance,
                                       bool override_hotkey_active) noexcept {
    const auto chance = (configuration_flags & 0x20U) ? forced_chance : settings.minimum_chance;
    RageThresholds result{settings.minimum_damage, float(chance) * 0.01f,
                          settings.override_enabled && override_hotkey_active, settings.delay_shot};
    if (weapon_id == 0x1fU)
        result.raw_minimum_damage = std::max(result.raw_minimum_damage, 105);
    if (result.override_active) {
        result.raw_minimum_damage = settings.override_damage;
        result.delay_shot = false;
    }
    return result;
}

} // namespace skeet

// ============================================================================
// Subcomponent: rage_safe_points.cpp
// ============================================================================

namespace skeet {
namespace rage_point_detail {
RageCapsule transform_capsule(const RageLocalHitbox& local, const Matrix3x4& matrix) noexcept {
    RageCapsule capsule{{}, {}, local.radius};
    for (unsigned r = 0; r < 3; ++r) {
        const auto& m = matrix.rows[r];
        capsule.first[r] =
            ((m[1] * local.minimum[1] + m[0] * local.minimum[0]) + m[2] * local.minimum[2]) + m[3];
        capsule.second[r] =
            ((m[1] * local.maximum[1] + m[0] * local.maximum[0]) + m[2] * local.maximum[2]) + m[3];
    }
    return capsule;
}
namespace {
RageVector clamp_to_capsule(const RageVector& point, const RageCapsule& capsule) noexcept {
    float fraction;
    const auto closest =
        project_rage_point_to_segment({capsule.first, capsule.second}, point, fraction);
    const auto relative = subtract(closest, point);
    const float squared =
        (relative[1] * relative[1] + relative[0] * relative[0]) + relative[2] * relative[2];
    if (capsule.radius * capsule.radius >= squared)
        return point;
    const auto direction = subtract(point, closest);
    const float magnitude_squared =
        (direction[1] * direction[1] + direction[0] * direction[0]) + direction[2] * direction[2];
    // The tiny-distance branch keeps radius in the X component, as in the
    // original 433FE77C/433FE8CD branches, rather than inventing an axis.
    RageVector offset{capsule.radius, 0.0f, 0.0f};
    if (magnitude_squared >= 1.0e-6f) {
        const float factor = capsule.radius / std::sqrt(magnitude_squared);
        offset = {factor * direction[0], factor * direction[1], factor * direction[2]};
    }
    return {closest[0] + offset[0], offset[1] + closest[1], closest[2] + offset[2]};
}
}

SafePointBounds build_safe_point_bounds(const RageVector& origin, const RageVector& center,
                                        const RageLocalHitbox& local,
                                        std::span<const Matrix3x4> hypotheses) {
    if (hypotheses.empty() || !(local.radius > 0.0f))
        return {};
    const auto relative = subtract(center, origin);
    const auto basis = basis_from_axis(normalize(relative, length(relative)));
    const RageVector opposite{-basis.side[0], -basis.side[1], -basis.side[2]};
    float upper = 1.0f, lower = -1.0f;
    RageVector upper_point{}, lower_point{};
    std::array<RageCapsule, 3> last_hypotheses;
    for (std::size_t i = 0; i < hypotheses.size(); ++i) {
        const auto capsule = transform_capsule(local, hypotheses[i]);
        last_hypotheses[i % 3] = capsule;
        const auto positive = support(capsule, basis.side, true);
        const auto negative = support(capsule, opposite, true);
        const auto first = subtract(positive, origin);
        const auto second = subtract(negative, origin);
        const float high =
            ((basis.side[1] * first[1] + basis.side[0] * first[0]) + basis.side[2] * first[2]) /
            length(first);
        const float low =
            ((basis.side[1] * second[1] + basis.side[0] * second[0]) + basis.side[2] * second[2]) /
            length(second);
        if (upper > high) {
            upper = high;
            upper_point = positive;
        }
        if (low > lower) {
            lower = low;
            lower_point = negative;
        }
    }
    if (upper == 1.0f || lower == -1.0f || lower >= upper)
        return {};
    return {last_hypotheses,
            basis,
            {{lower_point, 1.0f, local.hitgroup, 3}, {upper_point, 1.0f, local.hitgroup, 3}}};
}

std::vector<RageAimPoint> finish_safe_points(std::vector<RageAimPoint> result,
                                             const RagePointGeneration& settings,
                                             const RageVector& target_origin,
                                             std::uint32_t hitbox_index, std::int32_t hitgroup) {
    if (result.empty())
        return result;
    RageVector average{};
    for (const auto& point : result)
        for (unsigned i = 0; i < 3; ++i)
            average[i] += point.position[i] - target_origin[i];
    const float inverse = 1.0f / float(result.size());
    for (unsigned i = 0; i < 3; ++i)
        average[i] = inverse * average[i] + target_origin[i];
    const auto group_bit =
        hitbox_index == 11 || hitbox_index == 12 ? 0x200u : 1u << (std::uint32_t(hitgroup) & 31u);
    const bool multipoint =
        ((settings.multipoint_mask & group_bit) & 0xeu) != 0 ||
        ((settings.multipoint_mask & 0xc0u) != 0 && (hitbox_index == 7 || hitbox_index == 8));
    if (multipoint)
        scale_points(result, average, settings);
    else
        result.clear();
    result.insert(result.begin(), {average, 0.0f, hitgroup, 1});
    return result;
}
} // namespace rage_point_detail

std::vector<RageAimPoint> generate_rage_body_safe_points(const RagePointGeneration& settings,
                                                         const RageVector& center,
                                                         const RageVector& target_origin,
                                                         std::uint32_t hitbox_index,
                                                         const RageLocalHitbox& local,
                                                         std::span<const Matrix3x4> hypotheses) {
    using namespace rage_point_detail;
    auto bounds = build_safe_point_bounds(settings.origin, center, local, hypotheses);
    if (bounds.points.empty())
        return {};
    if (hitbox_index == 7 || hitbox_index == 8) {
        for (const auto& capsule : bounds.last_hypotheses)
            for (auto& point : bounds.points)
                point.position = clamp_to_capsule(point.position, capsule);
    }
    return finish_safe_points(std::move(bounds.points), settings, target_origin, hitbox_index,
                              local.hitgroup);
}
} // namespace skeet

// ============================================================================
// Subcomponent: rage_safety.cpp
// ============================================================================

namespace skeet {
bool rage_point_safe(const RageRay& ray, std::int32_t requested_hitgroup, float minimum_multiplier,
                     const RageResolverHitboxes& hypotheses, const RageBoxClipper& clipper) {
    if (requested_hitgroup != 1) {
        const RageSegment shot{ray.start,
                               {ray.delta[0] + ray.start[0], ray.delta[1] + ray.start[1],
                                ray.delta[2] + ray.start[2]}};
        for (const auto& hitboxes : hypotheses) {
            bool hit = false;
            for (const auto& hitbox : hitboxes) {
                if (!(hitbox.capsule.radius > 0.0f))
                    continue;
                const float multiplier =
                    rage_detail::hitgroup_multipliers[std::uint32_t(hitbox.hitgroup) & 15U];
                if (!(multiplier >= minimum_multiplier))
                    continue;
                const RageSegment axis{hitbox.capsule.first, hitbox.capsule.second};
                if (hitbox.capsule.radius >= rage_segment_distance(axis, shot)) {
                    hit = true;
                    break;
                }
            }
            if (!hit)
                return false;
        }
        return true;
    }
    for (const auto& hitboxes : hypotheses) {
        RageTrace trace;
        if (clipper.clip(clipper.context, ray, hitboxes.front(), trace) < 0)
            return false;
        const float head_fraction = trace.fraction;
        for (const auto& hitbox : hitboxes) {
            if (!(hitbox.capsule.radius > 0.0f) || (hitbox.hitgroup != 2 && hitbox.hitgroup != 3))
                continue;
            trace.fraction = 1.0f;
            if (clipper.clip(clipper.context, ray, hitbox, trace) >= 0 &&
                head_fraction > trace.fraction)
                return false;
        }
    }
    return true;
}
} // namespace skeet

// ============================================================================
// Subcomponent: rage_segments.cpp
// ============================================================================

namespace skeet {
namespace {
RageVector difference(const RageVector& a, const RageVector& b) noexcept {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}
float dot_xyz(const RageVector& a, const RageVector& b) noexcept {
    return (a[0] * b[0] + a[1] * b[1]) + a[2] * b[2];
}
/* [dedup dot_yxz] */
bool outside(float value) noexcept {
    return !(value >= 0.0f && value <= 1.0f);
}
std::array<float, 2> line_parameters(const RageVector& origin, const RageVector& direction,
                                     const RageSegment& segment) noexcept {
    const auto b = difference(segment.second, segment.first);
    const auto relative = difference(origin, segment.first);
    const float along_b = dot_yxz(b, relative);
    const float product = dot_xyz(direction, b);
    const float b_squared = dot_xyz(b, b);
    const float determinant = dot_xyz(direction, direction) * b_squared - product * product;
    float s = 0.0f;
    if (determinant != 0.0f)
        s = (product * along_b - dot_yxz(direction, relative) * b_squared) / determinant;
    return {s, (s * product + along_b) / b_squared};
}
}
RageVector project_rage_point_to_segment(const RageSegment& segment, const RageVector& point,
                                         float& fraction) noexcept {
    const auto delta = difference(segment.second, segment.first);
    const auto offset = difference(point, segment.first);
    const float projected = dot_xyz(offset, delta) / dot_xyz(delta, delta);
    const auto low = _mm_max_ss(_mm_set_ss(projected), _mm_setzero_ps());
    fraction = _mm_cvtss_f32(_mm_min_ss(low, _mm_set_ss(1.0f)));
    return {delta[0] * fraction + segment.first[0], delta[1] * fraction + segment.first[1],
            delta[2] * fraction + segment.first[2]};
}

RageSegmentClosest closest_rage_segments(const RageSegment& first,
                                         const RageSegment& second) noexcept {
    const auto a = difference(first.second, first.first);
    auto [s, t] = line_parameters(first.first, a, second);
    RageVector first_point;
    if (!outside(s)) {
        if (outside(t)) {
            const auto& endpoint = t < 0.0f ? second.first : second.second;
            t = t < 0.0f ? 0.0f : 1.0f;
            first_point = project_rage_point_to_segment(first, endpoint, s);
        } else
            first_point = {s * a[0] + first.first[0], s * a[1] + first.first[1],
                           s * a[2] + first.first[2]};
    } else if (!outside(t)) {
        first_point = s < 0.0f ? first.first : first.second;
        s = s < 0.0f ? 0.0f : 1.0f;
        project_rage_point_to_segment(second, first_point, t);
    } else {
        const auto first_endpoint = s < 0.0f ? first.first : first.second;
        const auto second_endpoint = t < 0.0f ? second.first : second.second;
        s = s < 0.0f ? 0.0f : 1.0f;
        t = t < 0.0f ? 0.0f : 1.0f;
        float first_projection, second_projection;
        const auto first_candidate =
            project_rage_point_to_segment(first, second_endpoint, first_projection);
        const auto second_candidate =
            project_rage_point_to_segment(second, first_endpoint, second_projection);
        const auto first_delta = difference(first_candidate, second_endpoint);
        const auto second_delta = difference(second_candidate, first_endpoint);
        const float first_distance = dot_yxz(first_delta, first_delta);
        const float second_distance = dot_yxz(second_delta, second_delta);
        if (second_distance >= first_distance) {
            s = first_projection;
            first_point = first_candidate;
        } else {
            t = second_projection;
            first_point = first_endpoint;
        }
    }
    return {s, t, first_point};
}

float rage_segment_distance(const RageSegment& first, const RageSegment& second) noexcept {
    const auto parameters = closest_rage_segments(first, second);
    const float first_weight = 1.0f - parameters.first_fraction;
    const float second_weight = 1.0f - parameters.second_fraction;
    RageVector delta;
    for (unsigned i = 0; i < 3; ++i) {
        const float a = first.first[i] * first_weight + first.second[i] * parameters.first_fraction;
        const float b =
            i == 0
                ? second.second[i] * parameters.second_fraction + second.first[i] * second_weight
                : second.first[i] * second_weight + second.second[i] * parameters.second_fraction;
        delta[i] = a - b;
    }
    return std::sqrt(dot_xyz(delta, delta));
}

RageSegmentClosest closest_rage_ray_segment(const RageRay& ray,
                                            const RageSegment& segment) noexcept {
    auto [s, t] = line_parameters(ray.start, ray.delta, segment);
    const auto ray_point = [&](float distance) {
        return RageVector{ray.delta[0] * distance + ray.start[0],
                          ray.delta[1] * distance + ray.start[1],
                          ray.delta[2] * distance + ray.start[2]};
    };
    if (s < 0.0f) {
        s = 0.0f;
        if (!outside(t)) {
            project_rage_point_to_segment(segment, ray.start, t);
            return {s, t, ray.start};
        }
        const auto& endpoint = t < 0.0f ? segment.first : segment.second;
        const float endpoint_fraction = t < 0.0f ? 0.0f : 1.0f;
        s = maximum_number(0.0f, dot_yxz(difference(endpoint, ray.start), ray.delta));
        const auto first_candidate = ray_point(s);
        const auto second_candidate = project_rage_point_to_segment(segment, ray.start, t);
        const auto first_delta = difference(first_candidate, endpoint);
        const auto second_delta = difference(second_candidate, ray.start);
        if (dot_yxz(second_delta, second_delta) >= dot_yxz(first_delta, first_delta))
            return {s, endpoint_fraction, first_candidate};
        return {0.0f, t, ray.start};
    }
    if (t < 0.0f) {
        t = 0.0f;
        s = maximum_number(0.0f, dot_yxz(difference(segment.first, ray.start), ray.delta));
    } else if (t > 1.0f) {
        t = 1.0f;
        s = maximum_number(0.0f, dot_yxz(difference(segment.second, ray.start), ray.delta));
    }
    return {s, t, ray_point(s)};
}
} // namespace skeet

// ============================================================================
// Subcomponent: rage_selection.cpp
// ============================================================================

namespace skeet {
using namespace rage_detail;

bool admit_rage_point(RagePointScore& point, const RagePointRequirements& requirements,
                      RagePointAdmissionState& state,
                      const std::array<float, 3>& descriptor_position,
                      std::uint32_t descriptor_flags) noexcept {
    if (!(descriptor_flags & 2U))
        state.reference = {descriptor_position, point.damage};
    if (point.damage == 0)
        return false;
    if (point.damage < requirements.minimum_damage) {
        point.flags |= 0x80U;
        return false;
    }
    if (!(point.chance >= requirements.minimum_chance)) {
        if (!(state.evaluation_flags & 9U))
            state.evaluation_flags |= 0x100U;
        point.flags |= 0x80U;
        return false;
    }
    if (requirements.prefer_body && (state.priority_mask & 0x100000U) && point.hitgroup == 1 &&
        (point.flags & 1U) && (state.body_disabler_reasons & 8U))
        state.priority_mask &= ~0x100000U;
    return true;
}

std::uint32_t rage_point_priority(const RagePointScore& point,
                                  const RagePointRequirements& requirements) noexcept {
    auto result = damage_priority(requirements.target_health, point.damage);
    if (!(point.flags & 2U))
        result |= 0x80U;
    result |= chance_priority(point.chance) << 8;
    if (point.flags & 1U)
        result |= 0x8000U;
    const auto shots = shots_to_kill(requirements.target_health, point.damage);
    const auto inverse_shots = add(15, -shots);
    result |= std::uint32_t(std::clamp(inverse_shots, 0, 15)) << 16;
    if (requirements.prefer_body && (point.hitgroup == 2 || point.hitgroup == 3))
        result |= 0x100000U;
    if (point.chance >= requirements.minimum_chance)
        result |= 0x200000U;
    if (point.damage >= requirements.minimum_damage)
        result |= 0x400000U;
    return result;
}

RagePointSelection select_rage_points(std::span<const RagePointScore> points,
                                      std::uint32_t priority_mask, bool prefer_body,
                                      bool prefer_safe, std::uint32_t candidate_flags) noexcept {
    RagePointSelection result;
    result.candidate_flags = candidate_flags;
    if (points.empty())
        return result;
    result.primary = 0;
    for (std::size_t i = 1; i < points.size(); ++i)
        if ((points[i].priority & priority_mask) >
            (points[result.primary].priority & priority_mask))
            result.primary = i;
    std::size_t body = no_rage_point;
    if (prefer_body && points[result.primary].hitgroup == 1) {
        for (std::size_t i = 0; i < points.size(); ++i) {
            if (points[i].hitgroup != 2 && points[i].hitgroup != 3)
                continue;
            if (body == no_rage_point || points[i].priority > points[body].priority)
                body = i;
        }
        if (body != no_rage_point) {
            result.alternate_first = body;
            result.candidate_flags |= 8U;
        }
    }
    if (prefer_safe && (points[result.primary].flags & 1U)) {
        std::size_t regular = no_rage_point;
        for (std::size_t i = 0; i < points.size(); ++i) {
            if (points[i].flags & 1U)
                continue;
            if (regular == no_rage_point || points[i].priority > points[regular].priority)
                regular = i;
        }
        if (regular != no_rage_point && regular != body) {
            if (result.candidate_flags & 8U) {
                result.alternate_second = regular;
                result.candidate_flags |= 0x10U;
            } else {
                result.alternate_first = regular;
                result.candidate_flags |= 8U;
            }
        }
    }
    return result;
}

std::uint32_t rage_candidate_priority(const RageCandidatePriority& candidate) noexcept {
    const auto& point = candidate.primary;
    auto result = damage_priority(candidate.target_health, point.damage) |
                  (chance_priority(point.chance) << 9);
    if (candidate.prefer_body && (point.hitgroup == 2 || point.hitgroup == 3))
        result |= 0x100U;
    if (candidate.target_weapon_priority_enabled && !candidate.target_weapon_is_taser)
        result |= 0x80U;
    if (candidate.prefer_safe && (point.flags & 1U))
        result |= 0x10000U;
    if (candidate.target_priority)
        result |= 0x20000U;
    const auto shots = std::clamp(shots_to_kill(candidate.target_health, point.damage), 0, 15);
    result |= ((~std::uint32_t(shots)) & 15U) << 19;
    if (candidate.target_state_flags & 0x1800U)
        result |= 0x40000U;
    return result;
}

std::int32_t compare_rage_candidate_priorities(std::uint32_t left, std::uint32_t right) noexcept {
    return signed_word(right - left);
}

} // namespace skeet

// ============================================================================
// Subcomponent: fire_state.cpp
// ============================================================================

#include "skeet/weapon/fire_state.hpp"
#include "skeet/engine/interface_call.hpp"
#include <cstring>

namespace skeet::weapon {
namespace {
template <class T> T fire_state_field(const void* object, std::ptrdiff_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
bool rule_blocks_fire(const FireStateBindings& bindings) { // complete 433DF22C
    return bindings.game_rules && *bindings.game_rules &&
           fire_state_field<std::uint8_t>(*bindings.game_rules, bindings.offsets.rule_d720aea6) != 0;
}
}
FireState evaluate_fire_state(const FireStateBindings& bindings, void* player, void* weapon,
                              const client::Command& command) {
    FireState state;
    if (!weapon)
        return state;
    if (command.weapon_select &&
        command.weapon_select !=
            engine::invoke<std::int32_t>(static_cast<std::byte*>(weapon) + 8, 10))
        return state;
    const auto& offsets = bindings.offsets;
    const auto definition = fire_state_field<std::uint16_t>(weapon, offsets.weapon_definition);
    if ((definition == 4 || definition == 10) && fire_state_field<std::uint8_t>(weapon, offsets.burst_mode) &&
        fire_state_field<std::int32_t>(weapon, offsets.burst_remaining) > 0 &&
        *bindings.current_time >= fire_state_field<float>(weapon, offsets.next_burst)) {
        state.primary_ready = state.burst_ready = true;
        return state;
    }
    if (fire_state_field<std::uint8_t>(player, offsets.player_block_bcb263a5) || rule_blocks_fire(bindings))
        return state;
    if (fire_state_field<std::int32_t>(player, offsets.team) == 3 &&
        fire_state_field<std::uint8_t>(player, offsets.player_state_a15c3105))
        return state;
    if (fire_state_field<std::int32_t>(weapon, offsets.ammunition) == 0)
        return state;
    const auto primary_time = *bindings.current_time;
    if (fire_state_field<float>(player, offsets.player_next_attack) > primary_time)
        return state;
    if (!(fire_state_field<float>(weapon, offsets.next_primary) > primary_time)) {
        bool permitted = true;
        if (fire_state_field<std::int32_t>(player, offsets.shots_fired) > 0)
            permitted = engine::invoke<bool>(weapon, 430);
        if (permitted) {
            if (fire_state_field<std::uint16_t>(weapon, offsets.weapon_definition) != 64)
                state.primary_ready = true;
            else if (fire_state_field<std::int32_t>(weapon, offsets.weapon_state_9e2078a1) == 208 &&
                     !(fire_state_field<float>(weapon, offsets.postpone_fire) >= primary_time))
                state.primary_ready = true;
        }
    }
    if (fire_state_field<float>(weapon, offsets.next_secondary) > *bindings.current_time)
        return state;
    if (fire_state_field<std::uint16_t>(weapon, offsets.weapon_definition) == 64) {
        if (fire_state_field<std::int32_t>(player, offsets.shots_fired) > 0 ||
            fire_state_field<std::int32_t>(weapon, offsets.weapon_state_9e2078a1) == 208 ||
            (command.movement.buttons & 1))
            return state;
    }
    state.secondary_ready = true;
    return state;
}
} // namespace skeet::weapon
