#include "util/address.h"
// Lag Compensation Subsystem Implementation
#include "skeet/lagcomp/angle_publication.hpp"
#include "skeet/resolver.hpp"
#include <bit>
#include <cmath>
#include <cstring>
#include <xmmintrin.h>
#include "skeet/lagcomp/animation_capture.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/animation/layer_state.hpp"
#include "skeet/lagcomp/animation_math.hpp"
#include <cstdint>
#include <immintrin.h>
#include "skeet/lagcomp/capture_speed.hpp"
#include "skeet/lagcomp/entity_preparation.hpp"
#include "skeet/engine/interface_call.hpp"
#include <limits>
#include "skeet/lagcomp/entity_state.hpp"
#include "skeet/engine/opaque_native_word.hpp"
#include "skeet/lagcomp/receive_timing.hpp"
#include <stdexcept>
#include "skeet/lagcomp/extrapolated_record.hpp"
#include "skeet/lagcomp/packet_capture.hpp"
#include "skeet/lagcomp/live_record_attachments.hpp"
#include "skeet/lagcomp/live_timeline.hpp"
#include "skeet/movement/extrapolation.hpp"
#include "skeet/movement/extrapolation_model.hpp"
#include "skeet/animation/record_pose_scope.hpp"
#include "skeet/lagcomp/history.hpp"
#include <algorithm>
#include "skeet/lagcomp/history_activation.hpp"
#include "skeet/legit/owner.hpp"
#include "skeet/lagcomp/history_alternative.hpp"
#include "skeet/movement/steering_math.hpp"
#include <array>
#include "skeet/lagcomp/history_animation_publish.hpp"
#include "skeet/resolver_runtime/movement_fit.hpp"
#include "skeet/lagcomp/history_build.hpp"
#include "skeet/lagcomp/history_environment.hpp"
#include "skeet/lagcomp/history_jobs.hpp"
#include <new>
#include <windows.h>
#include "skeet/lagcomp/history_owner.hpp"
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/resolver_runtime/environment_entry.hpp"
#include "skeet/lagcomp/history_receiver.hpp"
#include "skeet/lagcomp/history_resolver_update.hpp"
#include "skeet/lagcomp/prediction_scope.hpp"
#include "skeet/lagcomp/history_runtime.hpp"
#include "skeet/lagcomp/packet_direction.hpp"
#include "skeet/lagcomp/resolver_detection.hpp"
#include "skeet/anti_aim/body_yaw.hpp"
#include "skeet/resolver_runtime/build_validation.hpp"
#include "skeet/menu/misc_menu.hpp"
#include "skeet/lagcomp/record_validity.hpp"
#include "skeet/lagcomp/history_selection.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/client/command_orchestrator.hpp"
#include "skeet/lagcomp/history_velocity_capture.hpp"
#include "skeet/lagcomp/velocity.hpp"
#include "skeet/lagcomp/velocity_validation.hpp"
#include "skeet/lagcomp/history_velocity_reconstruction.hpp"
#include "skeet/lagcomp/history_yaw_side.hpp"
#include "skeet/lagcomp/interpolation.hpp"
#include "skeet/animation/bones.hpp"
#include "skeet/lagcomp/matrix_interpolation.hpp"
#include "skeet/lagcomp/native_history_boundaries.hpp"
#include "skeet/animation/native_trigonometry.hpp"
#include "skeet/lagcomp/record_selection.hpp"
#include "skeet/lagcomp/record_selection_live.hpp"
#include "skeet/lagcomp/records.hpp"
#include "skeet/lagcomp/timeline.hpp"
#include "skeet/lagcomp/update_capture.hpp"
#include "skeet/lagcomp/update_timing.hpp"


// ============================================================================
// Subcomponent: angle_publication.cpp
// ============================================================================

namespace skeet::lagcomp {

using skeet::read;
using skeet::write;
using skeet::field;
using skeet::ticks;
using skeet::truncate;

inline float normalize(float value) noexcept {
    if (std::isnan(value))
        return 0.0f;
    if (std::fabs(value) > 180.0f) {
        const float correction = round_half_away_from_zero(std::fabs(value / 360.0f)) * 360.0f;
        value = value < 0.0f ? value + correction : value - correction;
    }
    return value;
}

inline float absolute(float value) noexcept {
    return std::fabs(value);
}

inline std::int32_t absolute(std::int32_t value) noexcept {
    return std::abs(value);
}

inline float negative(float value) noexcept {
    return -value;
}

inline float negate(float value) noexcept {
    return -value;
}

inline std::int32_t sub(std::int32_t a, std::int32_t b) noexcept {
    return a - b;
}

inline std::int32_t subtract(std::int32_t a, std::int32_t b) noexcept {
    return a - b;
}

inline std::int32_t cap(std::int32_t value, std::int32_t limit) noexcept {
    if (limit < 1)
        return limit;
    if (value < 1)
        return 1;
    return value > limit ? limit : value;
}

inline float clamp_unit(float value) noexcept {
    return value < 0.0f ? 0.0f : (value > 1.0f ? 1.0f : value);
}

inline constexpr float radians = 0.01745329238474369f;
inline constexpr float degrees = 57.2957763671875f;
inline constexpr float epsilon = 0x1p-23f;


namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
/* [removed duplicate normalize] */
}
std::uint32_t history_angle_entry_index(std::uint16_t capacity, std::uint16_t first,
                                        std::int32_t ordinal) noexcept {
    const auto index = std::uint32_t(first) + std::uint32_t(ordinal);
    return std::bit_cast<std::int32_t>(index) < std::int32_t(capacity) ? index : index - capacity;
}
void publish_history_final_angles(void* player, const Vector& previous,
                                  const EntityStateRuntime& entity, std::uint32_t state_offset) {
    const auto pitch = normalize(read<float>(player, entity.offsets.eye_angles));
    const auto clamped = _mm_cvtss_f32(
        _mm_min_ss(_mm_max_ss(_mm_set_ss(pitch), _mm_set_ss(-90.0f)), _mm_set_ss(90.0f)));
    write(player, entity.offsets.eye_angles, clamped);
    if (clamped == previous[0] && read<float>(player, entity.offsets.eye_angles + 4) == previous[1])
        return;
    auto* state = static_cast<std::byte*>(player) + state_offset;
    if (read<std::uint16_t>(state, 0x10)) {
        const auto index = history_angle_entry_index(read<std::uint16_t>(state, 0xC),
                                                     read<std::uint16_t>(state, 0xE), 0);
        auto* entry = read<std::byte*>(state, 8) + std::size_t(index) * 16u;
        write(entry, 4, read<std::uint32_t>(player, entity.offsets.eye_angles));
        write(entry, 8, read<std::uint32_t>(player, entity.offsets.eye_angles + 4));
    }
    if (auto* target = read<std::byte*>(state, 0x14)) {
        write(target, 0, read<std::uint32_t>(player, entity.offsets.eye_angles));
        target = read<std::byte*>(state, 0x14);
        write(target, 4, read<std::uint32_t>(player, entity.offsets.eye_angles + 4));
    }
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: animation_capture.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
/* [removed duplicate normalize] */
}
HistoryAnimationSeed prepare_history_animation(client::PlayerState& owner, void* player,
                                               void* animation_state, std::int32_t simulation_ticks,
                                               std::uint32_t update_flags,
                                               const EntityStateRuntime& entity,
                                               std::uint32_t flags_offset, bool strict_bounds) {
    const auto reference = owner.native.reference_yaw;
    const auto stationary = read<float>(&owner.native, 0x118);
    const auto elapsed = float(simulation_ticks) * entity.interval;
    const auto target = normalize(reference);
    const AnimationYawMotion motion{
        {read<float>(animation_state, 0xA4), read<float>(animation_state, 0xF8),
         read<float>(animation_state, 0xFC), read<float>(animation_state, 0x11C),
         read<float>(animation_state, 0x334), read<float>(animation_state, 0x338)},
        read<float>(animation_state, 0xEC),
        read<float>(animation_state, 0xF0)};
    owner.native.capture_field_180 = std::bit_cast<std::uint32_t>(advance_animation_yaw(
        motion, target, stationary, elapsed, std::bit_cast<float>(owner.native.capture_field_180),
        strict_bounds));
    if ((update_flags & 1u) && !owner.history.count &&
        (read<std::uint8_t>(player, flags_offset) & 1u)) {
        write(animation_state, 0x108, std::uint8_t(1));
        write(animation_state, 0x109, std::uint8_t(0));
    }
    HistoryAnimationSeed seed;
    seed.previous_update_time = read<float>(animation_state, 0x6C);
    seed.velocity = read<Vector>(player, entity.offsets.velocity);
    seed.active_air_layer =
        !(read<std::uint8_t>(player, flags_offset) & 1u) &&
        animation::layer_is_active(reinterpret_cast<const std::byte*>(&owner.native) + 0x4D8);
    return seed;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: animation_math.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate absolute] */
/* [removed duplicate negative] */
/* [removed duplicate normalize] */
float angle_modulo(float value) noexcept {
    const float scale = std::bit_cast<float>(0x43360B61u);
    const auto integer = _mm_cvttss_si32(_mm_set_ss(value * scale));
    return float(std::uint32_t(integer) & 0xFFFFu) * std::bit_cast<float>(0x3BB40000u);
}
}

float advance_animation_yaw(const AnimationYawMotion& motion, float target_yaw,
                            float stationary_yaw, float elapsed, float current_yaw,
                            bool strict_bounds) noexcept {
    current_yaw = _mm_cvtss_f32(
        _mm_min_ss(_mm_max_ss(_mm_set_ss(current_yaw), _mm_set_ss(-360.0f)), _mm_set_ss(360.0f)));
    float delta = float(std::fmod(double(target_yaw - current_yaw), 360.0));
    if (target_yaw > current_yaw) {
        if (delta >= 180.0f)
            delta -= 360.0f;
    } else if (delta <= -180.0f) {
        delta += 360.0f;
    }
    float minimum, maximum;
    anti_aim::animation_yaw_limits(motion.bounds, minimum, maximum);
    if (delta > maximum)
        current_yaw = target_yaw - maximum;
    else if (delta < (strict_bounds ? minimum : negative(maximum)))
        // The server clamps the lower and upper model yaw limits independently.
        current_yaw = strict_bounds ? target_yaw - minimum : maximum + target_yaw;
    current_yaw = normalize(current_yaw);

    float rate;
    // Native JB treats unordered speed comparisons as the moving branch.
    if (!(motion.horizontal_speed_ec <= 0.1f) || !(absolute(motion.vertical_speed_f0) <= 100.0f)) {
        rate = motion.bounds.speed_fraction_11c * 20.0f + 30.0f;
    } else {
        target_yaw = stationary_yaw;
        rate = 100.0f;
    }
    float step = rate * elapsed;
    if (step < 0.0f)
        step = negative(step);
    const float target = angle_modulo(target_yaw);
    const float source = angle_modulo(current_yaw);
    delta = normalize(target - source);
    if (delta > step)
        return source + step;
    if (delta < negative(step))
        return source - step;
    return target;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: capture_speed.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
bool zero_integer(void* object) noexcept {
    for (;;) {
        auto* parent = read<void*>(object, 0x1C);
        if (object == parent)
            break;
        object = parent;
    }
    return read<std::uint32_t>(object, 0x30) ==
           std::uint32_t(reinterpret_cast<std::uintptr_t>(object));
}
}
bool apply_history_capture_speed_limit(const RecordRing& history, void* player,
                                       const HistoryCaptureSpeedBindings& bindings) {
    if (!history.count)
        return false;
    const auto flags = read<std::uint8_t>(player, bindings.flags_e2fbe695);
    if (!(flags & 1u))
        return false;
    const bool grounded_before = (history.at(0).header.player_flags & 1u) != 0;
    float limit;
    if (grounded_before) {
        const auto maximum = engine::invoke<float>(player, 276);
        limit = minimum_number(read<float>(player, bindings.maximum_speed_3eb71843), maximum);
        if (read<std::uint8_t>(player, bindings.mode_7090d8dd))
            limit *= 0.5199999809265137f;
        else if (read<std::uint8_t>(player, bindings.flags_e2fbe695) & 2u) {
            const auto duck = read<float>(player, bindings.entity.offsets.duck_amount);
            limit *= (duck * 0.3400000035762787f + 1.0f) - duck;
        }
    } else {
        if (!zero_integer(bindings.convar_4b52a9bf))
            return false;
        limit = read<float>(player, bindings.maximum_speed_3eb71843) * 1.100000023841858f;
        if (!(limit > 0.0f))
            return false;
    }
    const auto velocity = read<Vector>(player, bindings.entity.offsets.velocity);
    const auto speed = std::sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1]);
    if (!(speed > limit))
        return false;
    const auto scale = limit / speed;
    write(player, bindings.entity.offsets.velocity, velocity[0] * scale);
    write(player, bindings.entity.offsets.velocity + 4, velocity[1] * scale);
    return true;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: entity_preparation.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
/* [removed duplicate add] */
/* [removed duplicate subtract] */
/* [removed duplicate absolute] */
/* [removed duplicate normalize] */
std::int32_t target_index(void* target) {
    return engine::invoke<std::int32_t>(static_cast<std::byte*>(target) + 8, 10);
}
bool after_local_player(void* target, void* engine) {
    const auto index = target_index(target);
    return engine::invoke<std::int32_t>(engine, 12) < index;
}
}
std::uint32_t prepare_entity_angles(client::PlayerStateCache& players, void* target,
                                    const LagRecord& source, bool copy_yaw,
                                    const EntityPreparationContext& context) {
    auto& cached = players.get(std::uint32_t(target_index(target)))->native;
    const auto& offsets = context.entity.offsets;
    write(target, offsets.field_432b1f5b, read<std::uint32_t>(&cached, 0x12C));
    write(target, offsets.field_0d560fac, read<std::uint32_t>(&cached, 0x148));
    write(target, offsets.eye_angles, read<std::uint32_t>(&cached, 0x1A8));
    if (copy_yaw)
        write(target, offsets.eye_angles + 4, read<std::uint32_t>(&cached, 0x1AC));
    if (source.header.collision_max_z != read<float>(&cached, 0x130)) {
        write(target, offsets.field_432b1f5b,
              read<float>(&cached, 0x144) + read<float>(&cached, 0x130));
        write(target, offsets.field_0d560fac, context.frame_value());
    }
    const auto flags = cached.flags;
    const float reference = read<float>(&cached, 0xF0);
    if (!(flags & 0x30U) && reference != std::numeric_limits<float>::max() &&
        read<float>(&cached, 0x51C) < 0.75f &&
        absolute(read<float>(&cached, 0xF4) - context.comparison_value()) < 5.0f &&
        normalize(read<float>(&cached, 0x1A8)) > 45.0f &&
        absolute(normalize(read<float>(&cached, 0x190) - cached.reference_yaw)) > 45.0f) {
        const float body_distance = absolute(normalize(reference - read<float>(&cached, 0x190)));
        const float eye_distance = absolute(normalize(reference - cached.reference_yaw));
        if (body_distance < eye_distance && eye_distance > 60.0f) {
            if (read<std::uint8_t>(&cached, 0xA8) & 1U)
                write(target, offsets.eye_angles + 4, normalize(reference + 90.0f));
            write(&cached, 0xF4, context.current_value());
            return 1;
        }
    }
    const auto count = cached.field_238;
    if (count < 2)
        return 2;
    const auto first = read<std::uint32_t>(&cached, 0x234);
    const auto sample_offset = [&](std::uint32_t ordinal) {
        return 0x23Cu + ((first + ordinal) % 6U) * 0x20U;
    };
    auto step = subtract(read<std::int32_t>(&cached, sample_offset(0)),
                         read<std::int32_t>(&cached, sample_offset(1)));
    if (step < 1)
        step = 1;
    if (step > 16)
        step = 16;
    const auto* timing = context.current_timing;
    auto now = [&] {
        if (timing) {
            const auto current = resolve_history_tick_timing(*timing);
            return add(current.tick_d4, current.tick_18c);
        }
        return add(mean_history_timing_ticks(context.latency_values()),
                   corrected_command_tick(context.entity.command_tick,
                                          context.entity.previous_command,
                                          context.entity.current_command));
    }();
    const auto next = add(read<std::int32_t>(&cached, 0x168), step);
    if (now < next)
        return 3;
    if (now == next && after_local_player(target, context.entity.engine))
        return 4;
    if (flags & 0x20U) {
        write(target, offsets.eye_angles, read<std::uint32_t>(&cached, 0x18C));
        write(target, offsets.eye_angles + 4, read<std::uint32_t>(&cached, 0x190));
        return 5;
    }
    float variation = 0.0f;
    for (std::uint32_t ordinal = 1; ordinal < count; ++ordinal) {
        const float difference = read<float>(&cached, sample_offset(ordinal - 1) + 0x1C) -
                                 read<float>(&cached, sample_offset(ordinal) + 0x1C);
        variation += absolute(normalize(difference));
    }
    if (variation <= 60.0f)
        return 6;
    std::array<float, 6> angles{};
    std::uint32_t angle_first = 0, angle_count = 0;
    for (std::uint32_t ordinal = 0; ordinal < count; ++ordinal) {
        angle_first = (angle_first + 5) % 6;
        angles[angle_first] = normalize(read<float>(&cached, sample_offset(ordinal) + 0x1C));
        ++angle_count;
        if (angle_count == 6)
            break;
    }
    std::uint32_t selected = 0;
    float minimum = std::numeric_limits<float>::max();
    for (std::uint32_t ordinal = 0; ordinal < angle_count - 1; ++ordinal) {
        const float distance =
            absolute(normalize(angles[(angle_first + ordinal) % 6] - cached.reference_yaw));
        if (distance < minimum) {
            minimum = distance;
            selected = ordinal;
        }
    }
    if (after_local_player(target, context.entity.engine))
        now = add(now, -1);
    for (auto tick = next; tick <= now; tick = add(tick, step)) {
        if (tick == now && after_local_player(target, context.entity.engine))
            break;
        ++selected;
    }
    write(target, offsets.eye_angles + 4, angles[(selected % angle_count + angle_first) % 6]);
    return 7;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: entity_state.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
void capture_player_animation(AnimationState& destination, void* player,
                              const EntityNetvars& offsets) {
    const auto* descriptor = static_cast<std::byte*>(player) + offsets.animation_layers;
    capture_animation(destination, read<void*>(descriptor, 0), read<std::int32_t>(descriptor, 0xC),
                      static_cast<float*>(static_cast<void*>(static_cast<std::byte*>(player) +
                                                             offsets.pose_parameters)));
}
void restore_player_animation(const AnimationState& source, void* player,
                              const EntityNetvars& offsets) {
    auto* descriptor = static_cast<std::byte*>(player) + offsets.animation_layers;
    restore_animation(source, read<void*>(descriptor, 0), read<std::int32_t>(descriptor, 0xC),
                      static_cast<float*>(static_cast<void*>(static_cast<std::byte*>(player) +
                                                             offsets.pose_parameters)));
}
void set_transform(void* player, const EntityNetvars& offsets, SetAbsoluteVector function,
                   const Vector& value) {
    if (function) {
        const auto guard_offset = offsets.transform_guard_49d93e62 - 0x14u;
        const auto guard = read<std::uint32_t>(player, guard_offset);
        write(player, guard_offset, UINT32_MAX);
        function(player, value);
        write(player, guard_offset, guard);
    }
}
void scatter_live_hitbox_matrices(const HitboxMatrices& matrices, void* player,
                                  const EntityHitboxModelOffsets& offsets,
                                  Matrix3x4* bones) noexcept {
    const auto wrapper = read<const void*>(player, std::uint32_t(offsets.studio_header));
    if (!wrapper)
        return;
    const auto header = read<const void*>(wrapper, 0);
    if (!header)
        return;
    const auto index = read<std::int32_t>(player, std::uint32_t(offsets.hitbox_set));
    const auto set = static_cast<const std::byte*>(header) + read<std::int32_t>(header, 0xB0) +
                     std::ptrdiff_t(index) * 12;
    const auto native_count = read<std::int32_t>(set, 4);
    const auto count = native_count < 19 ? native_count : 19;
    for (std::int32_t i = 0; i < count; ++i) {
        const auto descriptor = set + read<std::int32_t>(set, 8) + std::ptrdiff_t(i) * 0x44;
        bones[read<std::int32_t>(descriptor, 0)] = matrices[std::size_t(i)];
    }
}
}
float animation_speed_factor(const void* state) noexcept {
    const auto clamp = [](float value) {
        return _mm_cvtss_f32(
            _mm_min_ss(_mm_max_ss(_mm_set_ss(value), _mm_setzero_ps()), _mm_set_ss(1.0f)));
    };
    //4333DB01 may supply its own unwritten null-animation local state. Its
    // native scalar observations remain actual opaque stack-word loads.
    const auto scalar = [&](std::size_t offset) {
        return engine::read_opaque_native_float(static_cast<const std::byte*>(state) + offset);
    };
    const float running = scalar(0x11C) * -0.3f - 0.2f;
    float factor = running * clamp(scalar(0xF8)) + 1.0f;
    const float duck = scalar(0xA4);
    if (duck > 0.0f) {
        const float speed = duck * clamp(scalar(0xFC));
        factor = speed * (0.5f - factor) + factor;
    }
    return factor;
}
void scatter_record_hitbox_matrices(const HitboxMatrices& matrices,
                                    std::span<const std::uint32_t> hitbox_bones,
                                    Matrix3x4* bones) noexcept {
    for (std::size_t i = 0; i < hitbox_bones.size() && i < 19; ++i)
        bones[hitbox_bones[i]] = matrices[i];
}
namespace {
struct CaptureHistoryView {
    const void* field_1ac;
    const void* field_180;
    const std::uint8_t* index;
    const std::uint8_t* state;
    const std::byte* modes;
    std::uint32_t mode_stride;
};
void capture_record_fields(const EntityStateRuntime& runtime, LagRecord& record, void* player,
                           const CaptureHistoryView& history) {
    const auto& offsets = runtime.offsets;
    const auto& interval = runtime.interval;
    auto& out = record.header;
    out.flags = 0;
    out.eye_pitch = read<float>(player, offsets.eye_angles);
    out.player_history_1ac = read<std::uint32_t>(history.field_1ac, 0);
    out.player_history_180 = read<std::uint32_t>(history.field_180, 0);
    out.simulation_tick = time_to_tick(read<float>(player, offsets.simulation_time), interval);
    if (out.simulation_tick ==
        time_to_tick(read<float>(player, offsets.old_simulation_time), interval))
        out.flags = 0x100;
    out.field_a2a8247f = read<float>(player, offsets.field_a2a8247f);
    out.origin = read<Vector>(player, offsets.origin);
    out.collision_max_z = engine::invoke<const Vector*>(player, 77)->at(2);
    out.player_flags = read<std::uint32_t>(player, offsets.player_flags);
    out.velocity = read<Vector>(player, offsets.velocity);
    out.field_fb7e3404 = read<std::uint32_t>(player, offsets.field_fb7e3404);
    out.duck_amount = read<float>(player, offsets.duck_amount);
    const auto handle = read<std::uint32_t>(player, offsets.weapon_handle);
    void* weapon =
        handle == UINT32_MAX ? nullptr : engine::invoke<void*>(runtime.entity_list, 4, handle);
    out.weapon_field_6777c9ce = weapon ? read<float>(weapon, offsets.weapon_field_6777c9ce) : 0.0f;
    out.receive_tick = sample_receive_tick(runtime.engine, runtime.receive_interval);
    out.command_tick = corrected_command_tick(runtime.command_tick, runtime.previous_command,
                                              runtime.current_command);
    const auto* animation = read<void*>(player, offsets.animation_state);
    out.animation_yaw =
        animation ? read<float>(animation, 0x80) : engine::invoke<const Vector*>(player, 11)->at(1);
    out.resolver_index = *history.index;
    out.resolver_mode =
        read<std::uint8_t>(history.modes, std::uint32_t(*history.index) * history.mode_stride);
    out.resolver_state = *history.state;
    out.animation_speed_factor = animation ? animation_speed_factor(animation) : 1.0f;
    capture_player_animation(record.animation, player, offsets);
}
}
void EntityStateRuntime::capture_record(LagRecord& record, void* player,
                                        const PlayerCaptureState& history) const {
    // Separate semantic entry: its caller supplies the proven history view.
    // As in the original, a valid player has a nonzero networkable index.
    (void)engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    capture_record_fields(*this, record, player,
                          {&history.field_1ac, &history.field_180, &history.resolver_index,
                           &history.resolver_state,
                           reinterpret_cast<const std::byte*>(history.resolver_modes.data()), 1});
}
void EntityStateRuntime::capture_record(LagRecord& record, void* player,
                                        client::PlayerStateCache& players) const {
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    auto& owner = *players.get(index);
    capture_record_fields(*this, record, player,
                          {&owner.native.reference_yaw, &owner.native.capture_field_180,
                           &owner.native.resolver_index, &owner.native.resolver_status,
                           reinterpret_cast<const std::byte*>(owner.native.resolver_slots.data()),
                           0x24});
}
void EntityStateRuntime::capture_snapshot(EntitySnapshot& snapshot, void* player) const {
    auto& out = snapshot.state;
    out.capture_frame = frame;
    out.handle = *engine::invoke<const std::uint32_t*>(player, 2);
    out.player_flags = read<std::uint32_t>(player, offsets.player_flags);
    out.simulation_time = read<float>(player, offsets.simulation_time);
    out.absolute_yaw = engine::invoke<const Vector*>(player, 11)->at(1);
    out.collision_max_z = engine::invoke<const Vector*>(player, 77)->at(2);
    out.velocity = read<Vector>(player, offsets.velocity);
    out.origin = read<Vector>(player, offsets.origin);
    out.absolute_origin = *engine::invoke<const Vector*>(player, 10);
    capture_player_animation(out.animation, player, offsets);
    auto* cache = static_cast<std::byte*>(player) + offsets.bone_cache;
    out.readable_bones = read<std::uint32_t>(player, offsets.bone_accessor + 8);
    out.writable_bones = read<std::uint32_t>(player, offsets.bone_accessor + 0xC);
    out.recent_bone_counter = read<std::uint32_t>(player, offsets.recent_bone_counter);
    out.bone_count = read<std::int32_t>(cache, 0xC);
    if (out.bone_count > std::int32_t(snapshot.bones.size()))
        throw std::length_error("Snapshot bone storage is smaller than the live cache");
    const auto* bones = read<const Matrix3x4*>(cache, 0);
    for (std::int32_t i = 0; i < out.bone_count; ++i)
        snapshot.bones[std::size_t(i)] = bones[i];
}
void EntityStateRuntime::restore_snapshot(const EntitySnapshot& snapshot) const {
    const auto& saved = snapshot.state;
    void* player =
        saved.handle == UINT32_MAX ? nullptr : engine::invoke<void*>(entity_list, 4, saved.handle);
    if (!player)
        throw std::logic_error("Snapshot recorded entity must remain valid");
    std::uint8_t prior_override = 0;
    if (transform_override) {
        prior_override = *transform_override;
        *transform_override = 1;
    }
    write(player, offsets.player_flags, saved.player_flags);
    write(player, offsets.simulation_time, saved.simulation_time);
    set_transform(player, offsets, set_absolute_angles, Vector{0.0f, saved.absolute_yaw, 0.0f});
    auto* collideable = engine::invoke<void*>(player, 3);
    Vector maximum = *engine::invoke<const Vector*>(collideable, 2);
    if (maximum[2] != saved.collision_max_z) {
        const auto first = read<std::uint32_t>(player, offsets.field_432b1f5b);
        const auto second = read<std::uint32_t>(player, offsets.field_0d560fac);
        maximum[2] = saved.collision_max_z;
        const auto* minimum = engine::invoke<const Vector*>(player, 76);
        if (set_collision_bounds) {
            auto* current = engine::invoke<void*>(player, 3);
            set_collision_bounds(current, *minimum, maximum);
        }
        collideable = engine::invoke<void*>(player, 3);
        auto* current_maximum = const_cast<Vector*>(engine::invoke<const Vector*>(collideable, 2));
        (*current_maximum)[2] = saved.collision_max_z;
        write(player, offsets.field_0d560fac, second);
        write(player, offsets.field_432b1f5b, first);
    }
    write(player, offsets.velocity, saved.velocity);
    set_transform(player, offsets, set_absolute_origin, saved.absolute_origin);
    write(player, offsets.origin, saved.origin);
    restore_player_animation(saved.animation, player, offsets);
    auto* cache = static_cast<std::byte*>(player) + offsets.bone_cache;
    if (!saved.bone_count || read<std::int32_t>(cache, 0xC) < saved.bone_count) {
        write(player, offsets.bone_accessor + 8, std::uint32_t(0));
        write(player, offsets.recent_bone_counter, std::uint32_t(0));
        write(player, offsets.last_bone_setup_time, std::uint32_t(0xFF7FFFFF));
        write(player, offsets.field_0471e13c, std::uint32_t(0));
        write(player, offsets.effects_bb78dc43,
              read<std::uint32_t>(player, offsets.effects_bb78dc43) & 0xFFFFFFFDu);
    } else {
        write(player, offsets.bone_accessor + 8, saved.readable_bones);
        write(player, offsets.bone_accessor + 0xC, saved.writable_bones);
        write(player, offsets.recent_bone_counter, saved.recent_bone_counter);
        auto* bones = read<Matrix3x4*>(cache, 0);
        for (std::int32_t i = 0; i < saved.bone_count; ++i)
            bones[i] = snapshot.bones[std::size_t(i)];
    }
    if (transform_override)
        *transform_override = prior_override;
}
void EntityStateRuntime::apply_record(void* player, const LagRecord& record,
                                      std::span<const std::uint32_t> hitbox_bones,
                                      const std::uint32_t* bone_counter) const {
    apply_record_inputs(player, record, hitbox_bones, bone_counter, nullptr);
}
void EntityStateRuntime::apply_record_live_model(void* player, const LagRecord& record,
                                                 const EntityHitboxModelOffsets& model,
                                                 const std::uint32_t* bone_counter) const {
    apply_record_inputs(player, record, {}, bone_counter, &model);
}
void EntityStateRuntime::apply_record_inputs(void* player, const LagRecord& record,
                                             std::span<const std::uint32_t> hitbox_bones,
                                             const std::uint32_t* bone_counter,
                                             const EntityHitboxModelOffsets* model) const {
    const auto& saved = record.header;
    std::uint8_t prior_override = 0;
    if (transform_override) {
        prior_override = *transform_override;
        *transform_override = 1;
    }
    write(player, offsets.player_flags, saved.player_flags);
    write(player, offsets.simulation_time, float(saved.simulation_tick) * interval);
    set_transform(player, offsets, set_absolute_angles, Vector{0.0f, saved.animation_yaw, 0.0f});
    auto* collideable = engine::invoke<void*>(player, 3);
    auto* maximum = const_cast<Vector*>(engine::invoke<const Vector*>(collideable, 2));
    if ((*maximum)[2] != saved.collision_max_z) {
        collideable = engine::invoke<void*>(player, 3);
        maximum = const_cast<Vector*>(engine::invoke<const Vector*>(collideable, 2));
        (*maximum)[2] = saved.collision_max_z;
    }
    write(player, offsets.velocity, saved.velocity);
    set_transform(player, offsets, set_absolute_origin, saved.origin);
    write(player, offsets.origin, saved.origin);
    restore_player_animation(record.animation, player, offsets);
    auto* cache = static_cast<std::byte*>(player) + offsets.bone_cache;
    if (!(saved.flags & 4u) || !read<std::int32_t>(cache, 0xC)) {
        write(player, offsets.bone_accessor + 8, std::uint32_t(0));
        write(player, offsets.recent_bone_counter, std::uint32_t(0));
        write(player, offsets.last_bone_setup_time, std::uint32_t(0xFF7FFFFF));
        write(player, offsets.field_0471e13c, std::uint32_t(0));
        write(player, offsets.effects_bb78dc43,
              read<std::uint32_t>(player, offsets.effects_bb78dc43) & 0xFFFFFFFDu);
    } else {
        auto* bones = read<Matrix3x4*>(cache, 0);
        if (model)
            scatter_live_hitbox_matrices(record.matrices, player, *model, bones);
        else
            scatter_record_hitbox_matrices(record.matrices, hitbox_bones, bones);
        write(player, offsets.bone_accessor + 8, std::uint32_t(0xFFF00));
        write(player, offsets.bone_accessor + 0xC, std::uint32_t(0xFFF00));
        if (bone_counter)
            write(player, offsets.recent_bone_counter, *bone_counter);
    }
    if (transform_override)
        *transform_override = prior_override;
}
EntityRecordScope::EntityRecordScope(const EntityStateRuntime& owner, void* entity,
                                     std::span<Matrix3x4> saved_bones) noexcept
    : player(entity), flags(0), snapshot{EntitySnapshotHeader{}, saved_bones}, runtime(owner) {
    record.header.field_00 = 0;
    snapshot.state.capture_frame = -1;
    snapshot.state.handle = UINT32_MAX;
}
void EntityRecordScope::apply(const LagRecord& value, std::span<const std::uint32_t> hitbox_bones,
                              const std::uint32_t* bone_counter) {
    if (snapshot.state.capture_frame != runtime.frame)
        runtime.capture_snapshot(snapshot, player);
    std::memcpy(&record, &value, sizeof(record));
    runtime.apply_record(player, record, hitbox_bones, bone_counter);
}
void EntityRecordScope::apply_live_model(const LagRecord& value,
                                         const EntityHitboxModelOffsets& model,
                                         const std::uint32_t* bone_counter) {
    if (snapshot.state.capture_frame != runtime.frame)
        runtime.capture_snapshot(snapshot, player);
    std::memcpy(&record, &value, sizeof record);
    runtime.apply_record_live_model(player, record, model, bone_counter);
}
void EntityRecordScope::restore() {
    if (snapshot.state.capture_frame == runtime.frame) {
        runtime.restore_snapshot(snapshot);
        snapshot.state.capture_frame = -1;
    }
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: extrapolated_record.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
std::int32_t decrement(std::int32_t value) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(value) - 1u);
}
struct RecordCaptureResult {
    client::PlayerState& owner;
    HistoryTickTiming timing;
    std::int32_t changed;
    std::int32_t ticks;
};
template <bool Live>
RecordCaptureResult capture_extrapolation_source(client::CommandFrame& frame, void* player,
                                                 LagRecord& destination,
                                                 const ExtrapolatedRecordBindings& bindings) {
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    auto& owner = *bindings.players.get(index);
    const auto timeline = history_timeline(owner);
    const auto& entity = bindings.preparation.entity;
    bool capture = timeline.record_count == 0;
    if (!capture) {
        const auto tick = time_to_tick(timeline.simulation_time, entity.interval);
        capture = tick < decrement(timeline.last_update_tick) &&
                  tick < timeline.at(0).header.simulation_tick;
    }
    std::int32_t changed = 0;
    if (capture) {
        if constexpr (Live)
            entity.capture_record(destination, player, bindings.players);
        else
            entity.capture_record(destination, player, owner.capture_state());
        changed = 1;
    } else
        std::memcpy(&destination, &timeline.at(0), sizeof destination);
    HistoryTickTiming current{};
    current.tick_d4 = read<std::int32_t>(&frame, 0xD4);
    current.tick_18c = std::bit_cast<std::int32_t>(frame.field_18c);
    return {owner, current, changed, 0};
}
MissingTickTiming missing_tick_timing(const RecordCaptureResult& captured, void* player,
                                      const ExtrapolatedRecordBindings& bindings,
                                      std::int32_t limit) noexcept {
    const auto& entity = bindings.preparation.entity;
    return {captured.timing.tick_d4,
            captured.timing.tick_18c,
            entity.interval,
            entity.receive_interval,
            limit,
            entity.engine,
            player};
}
RecordCaptureResult prepare_extrapolated_record(client::CommandFrame& frame, void* player,
                                                LagRecord& destination,
                                                const ExtrapolatedRecordBindings& bindings) {
    auto captured = capture_extrapolation_source<false>(frame, player, destination, bindings);
    captured.ticks = estimate_missing_ticks(
        history_timeline(captured.owner),
        missing_tick_timing(captured, player, bindings, bindings.prediction_limit.value()));
    return captured;
}
RecordCaptureResult prepare_extrapolated_record(client::CommandFrame& frame, void* player,
                                                LagRecord& destination,
                                                const ExtrapolatedRecordBindings& bindings,
                                                const client::CommandBudgetBindings& budget) {
    auto captured = capture_extrapolation_source<true>(frame, player, destination, bindings);
    if (bindings.rebuild_pose) {
        if (captured.changed)
            return captured;
        const auto& entity = bindings.preparation.entity;
        const auto receive_tick = sample_receive_tick(entity.engine, entity.receive_interval);
        const auto elapsed = std::int64_t(receive_tick) - destination.header.receive_tick;
        const double ticks = double(elapsed) * entity.receive_interval / entity.interval;
        if (!std::isfinite(ticks) || ticks < 0.0 || ticks >= maximum_pose_extrapolation_ticks + 0.5) {
            captured.changed = -1;
            return captured;
        }
        captured.ticks = std::int32_t(std::round(ticks));
        if (captured.ticks > 0 && captured.ticks > client::query_command_budget(budget)) {
            captured.ticks = 0;
            captured.changed = -1;
        }
        return captured;
    }
    captured.ticks =
        estimate_missing_ticks(history_timeline(captured.owner),
                               missing_tick_timing(captured, player, bindings, 0), budget);
    return captured;
}
}
void build_extrapolated_record_matrices(LagRecord& record,
                                        resolver_runtime::HypothesisBuildContext& context,
                                        std::span<Matrix3x4, 256> scratch) {
    record.header.flags &= ~4U;
    const animation::BoneRequest request{{0.0f, record.header.animation_yaw, 0.0f},
                                         record.header.origin,
                                         0.0f,
                                         0x300u,
                                         -1,
                                         0,
                                         false};
    if (animation::build_bone_matrices(context.entity, context.model, request, scratch,
                                       context.engine, context.trigonometry)) {
        animation::select_hitbox_bone_matrices(context.model, scratch, record.matrices);
        record.header.flags = (record.header.flags & ~0x20u) | 4u;
    }
}

void build_extrapolated_record_matrices(void* player, LagRecord& record,
                                        const animation::BoneRuntime& bones,
                                        std::span<Matrix3x4, 256> scratch, bool strict, float interval) {
    record.header.flags &= ~4U;
    if (strict) {
        const auto count = animation::build_captured_hitbox_matrices(
            player, record, bones, scratch, record.matrices, 0x300u,
            float(record.header.simulation_tick) * interval);
        if (resolver_runtime::complete_hitbox_matrices(count, record.matrices))
            record.header.flags = (record.header.flags & ~0x20u) | 4u;
        return;
    }
    const animation::BoneRequest request{{0.0f, record.header.animation_yaw, 0.0f},
                                         record.header.origin,
                                         0.0f,
                                         0x300u,
                                         -1,
                                         0,
                                         false};
    if (bones.build_bone_matrices(player, request, scratch)) {
        bones.select_hitbox_bone_matrices(player, scratch, record.matrices);
        record.header.flags = (record.header.flags & ~0x20u) | 4u;
    }
}

resolver_runtime::HypothesisInput
capture_record_hypothesis_input(void* player, const client::PlayerState& owner,
                                const LagRecord& record,
                                const EntityStateRuntime& entity, bool use_record_yaw) noexcept {
    const auto* animation = read<const void*>(player, entity.offsets.animation_state);
    if (!animation && use_record_yaw)
        return {};
    const auto velocity = read<Vector>(player, entity.offsets.velocity);
    resolver_runtime::HypothesisInput result;
    result.speed_fraction = animation ? read<float>(animation, 0x11C) : 0.0f;
    result.feet_yaw = read<float>(animation, 0x84);
    result.movement_yaw = read<float>(animation, 0x88);
    result.movement_yaw_target = read<float>(animation, 0x8C);
    result.duck_amount = read<float>(animation, 0xA4);
    result.running_fraction = read<float>(animation, 0xF8);
    result.ducking_fraction = read<float>(animation, 0xFC);
    result.minimum_yaw = read<float>(animation, 0x334);
    result.maximum_yaw = read<float>(animation, 0x338);
    result.eye_yaw = use_record_yaw ? std::bit_cast<float>(record.header.player_history_1ac)
                                    : owner.native.reference_yaw;
    result.animation_fraction = read<float>(&owner.native, 0x538);
    result.velocity_x = velocity[0];
    result.velocity_y = velocity[1];
    result.velocity_z = velocity[2];
    result.lower_body_yaw = read<float>(player, entity.offsets.field_a2a8247f);
    result.interval = entity.interval;
    result.record_flags = record.header.flags;
    result.moving = read<std::uint8_t>(animation, 0x108);
    return result;
}

bool capture_extrapolated_record(client::CommandFrame& frame, void* player, LagRecord& destination,
                                 std::uint32_t& output_flags,
                                 const ExtrapolatedRecordBindings& bindings) {
    auto captured = prepare_extrapolated_record(frame, player, destination, bindings);
    if (captured.ticks > 0)
        captured.changed = extrapolate_lag_record(player, captured.owner, destination,
                                                  captured.ticks, bindings.movement);
    auto& owner = captured.owner;
    const auto& entity = bindings.preparation.entity;
    if (captured.changed < 0)
        return false;
    if (captured.changed > 0) {
        output_flags |= 0x20u;
        build_extrapolated_record_matrices(destination, bindings.bones.decoded(),
                                           bindings.attachments.scratch);
        if (!(output_flags & 0x40u)) {
            const auto input = capture_record_hypothesis_input(player, owner, destination, entity);
            const auto origin = read<Vector>(player, entity.offsets.origin);
            resolver_runtime::HypothesisBuildContext context{
                bindings.bones.decoded().entity,
                bindings.bones.decoded().model,
                origin,
                bindings.bones.decoded().pose_parameters,
                bindings.bones.decoded().engine,
                bindings.bones.decoded().trigonometry};
            resolver_runtime::build_hypotheses(destination, input, context);
        }
    }
    auto preparation = bindings.preparation;
    preparation.current_timing = &captured.timing;
    refresh_record_attachments(bindings.players, player, destination, preparation,
                               bindings.attachments);
    return true;
}

namespace {
bool finish_live_record_capture(const RecordCaptureResult& captured, void* player,
                                LagRecord& destination, std::uint32_t& output_flags,
                                const ExtrapolatedRecordBindings& bindings,
                                const animation::BoneRuntime& bones) {
    if (captured.changed < 0)
        return false;
    const bool rebuild_pose = bool(bindings.rebuild_pose);
    if (captured.changed > 0) {
        if (!rebuild_pose || captured.ticks > 0)
            output_flags |= 0x20u;
        if (rebuild_pose)
            destination.header.field_00 = 0;
        build_extrapolated_record_matrices(player, destination, bones,
                                           bindings.attachments.scratch, rebuild_pose,
                                           bindings.preparation.entity.interval);
        if (rebuild_pose) {
            if (!(destination.header.flags & 4u))
                return false;
        } else if (!(output_flags & 0x40u)) {
            const auto input = capture_record_hypothesis_input(
                player, captured.owner, destination, bindings.preparation.entity);
            resolver_runtime::build_hypotheses(destination, input, player, bones);
        }
    }
    auto preparation = bindings.preparation;
    preparation.current_timing = &captured.timing;
    refresh_record_attachments(bindings.players, player, destination, preparation,
                               bindings.attachments, bones, rebuild_pose && captured.changed > 0);
    return !rebuild_pose || captured.changed == 0 || record_geometry_valid(destination);
}
}

bool capture_extrapolated_record(client::CommandFrame& frame, void* player, LagRecord& destination,
                                 std::uint32_t& output_flags,
                                 const ExtrapolatedRecordBindings& bindings,
                                 const animation::BoneRuntime& bones,
                                 const MovementExtrapolationLiveBindings& movement,
                                 const client::CommandBudgetBindings& budget,
                                 std::int32_t* predicted_ticks) {
    if (predicted_ticks)
        *predicted_ticks = 0;
    auto captured = prepare_extrapolated_record(frame, player, destination, bindings, budget);
    const auto source_tick = destination.header.simulation_tick;
    if (captured.ticks > 0)
        captured.changed = extrapolate_lag_record(player, captured.owner, destination,
                                                  captured.ticks, bindings.movement, movement,
                                                  bool(bindings.rebuild_pose));
    if (!finish_live_record_capture(captured, player, destination, output_flags, bindings, bones))
        return false;
    if (predicted_ticks && captured.changed > 0)
        *predicted_ticks = destination.header.simulation_tick - source_tick;
    return true;
}

} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate sub] */
/* [removed duplicate add] */
/* [removed duplicate absolute] */
float field(const LagRecord& record, std::size_t offset) noexcept {
    float value;
    std::memcpy(&value, reinterpret_cast<const std::byte*>(&record) + offset, 4);
    return value;
}
float origin_distance(const LagRecord& first, const LagRecord& second) noexcept {
    const auto& a = first.header.origin;
    const auto& b = second.header.origin;
    const float x = a[0] - b[0], y = a[1] - b[1], z = a[2] - b[2];
    return (y * y + x * x) + z * z;
}
std::int32_t capped_gap(std::int32_t value, std::int32_t limit) noexcept {
    if (limit < 1)
        return limit;
    if (value < 1)
        return 1;
    return value > limit ? limit : value;
}
}
LagRecord& RecordPool::acquire() {
    LagRecord* record;
    if (free.empty()) {
        allocated.push_back(std::make_unique<LagRecord>());
        record = allocated.back().get();
    } else {
        record = free.back();
        free.pop_back();
        record->header.field_00 = 0;
    }
    return *record;
}
void RecordPool::release(LagRecord& record) {
    record.header.field_00 = std::uint32_t(reinterpret_cast<std::uintptr_t>(free_head()));
    free.push_back(&record);
}
void RecordPool::finalize_free_records() {
    while (!free.empty()) {
        auto* const record = free.back();
        const auto owner =
            std::find_if(allocated.begin(), allocated.end(),
                         [record](const auto& candidate) { return candidate.get() == record; });
        if (owner == allocated.end())
            throw std::logic_error(
                "History terminal cleanup requires the canonical owning record pool");
        free.pop_back();
        record->header.field_00 = 0;
        owner->reset();
    }
    allocated.erase(std::remove_if(allocated.begin(), allocated.end(),
                                   [](const auto& record) { return !record; }),
                    allocated.end());
    std::vector<LagRecord*>{}.swap(free);
    if (allocated.empty())
        std::vector<std::unique_ptr<LagRecord>>{}.swap(allocated);
}
LagRecord& push_history_record(RecordRing& ring, RecordPool& pool) {
    LagRecord* record;
    if (ring.count == 32)
        record = ring.records[(ring.first - 1u) & 31u];
    else {
        record = &pool.acquire();
        ++ring.count;
    }
    ring.first = (ring.first - 1u) & 31u;
    ring.records[ring.first] = record;
    std::memset(record, 0, sizeof(*record));
    return *record;
}
void pop_history_record(RecordRing& ring, RecordPool& pool) {
    auto* record = ring.records[(ring.first - 1u + ring.count) & 31u];
    if (record)
        pool.release(*record);
    if (ring.count)
        --ring.count;
}
void clear_history_records(RecordRing& ring, RecordPool& pool) {
    while (ring.count)
        pop_history_record(ring, pool);
}
void prune_history_records(RecordRing& ring, RecordPool& pool, std::int32_t clock, float interval) {
    const auto maximum_age = time_to_tick(1.0f, interval);
    while (ring.count &&
           absolute(sub(ring.at(ring.count - 1u).header.receive_tick, clock)) >= maximum_age)
        pop_history_record(ring, pool);
}
void prune_packet_samples(LagTimeline& timeline, std::int32_t clock, float interval) noexcept {
    const auto maximum_age = time_to_tick(1.0f, interval);
    while (timeline.packet_count) {
        const auto index = (timeline.packet_head - 1u + timeline.packet_count) % 6u;
        if (absolute(sub(timeline.packets[index].field_18, clock)) < maximum_age)
            break;
        --timeline.packet_count;
    }
}
ExtrapolationPlan plan_history_extrapolation(const RecordRing& ring, std::uint32_t ordinal,
                                             std::uint32_t options, float interval,
                                             std::int32_t prediction_limit) noexcept {
    ExtrapolationPlan result;
    if (!(options & 1u) || ordinal || ring.count < 2)
        return result;
    const auto& newest = ring.at(0);
    const auto& previous = ring.at(1);
    if (newest.header.flags & 0x200u)
        return result;
    if (newest.header.player_flags & 1u) {
        if (previous.header.player_flags & 1u)
            return result;
        const auto weight = field(newest, 0xFC);
        const auto rate = field(newest, 0x104);
        const auto rate_bits = std::bit_cast<std::uint32_t>(rate) & 0x7FFFFFFFu;
        if (weight > 0.0f && rate_bits && time_to_tick(field(newest, 0x108) / rate, interval) > 1)
            return result;
    }
    const LagRecord* first = &newest;
    const LagRecord* second = &previous;
    if (!(origin_distance(newest, previous) > 4096.0f)) {
        if (ring.count == 2 || (previous.header.flags & 0x200u))
            return result;
        result.inspected_third = true;
        const auto& third = ring.at(2);
        if (!(origin_distance(previous, third) > 4096.0f))
            return result;
        const auto& a = newest.header.velocity;
        const auto& b = previous.header.velocity;
        const auto speed_a = std::sqrt(a[0] * a[0] + a[1] * a[1]);
        const auto speed_b = std::sqrt(b[0] * b[0] + b[1] * b[1]);
        if (speed_b - speed_a > 50.0f)
            return result;
        first = &previous;
        second = &third;
    }
    result.ticks = capped_gap(sub(first->header.simulation_tick, second->header.simulation_tick),
                              prediction_limit);
    return result;
}
HistoryTickTiming resolve_history_tick_timing(const HistoryTickTiming& source) noexcept {
    auto result = source;
    if (source.live_globals) {
        result.current_time = *source.live_globals->current_time;
        result.interpolation = source.live_globals->interpolation;
        result.interval = *source.live_globals->interval;
        result.maximum_unlag_ms = source.live_globals->maximum_unlag_ms;
    }
    if (source.live_tick_d4)
        result.tick_d4 = std::bit_cast<std::int32_t>(*source.live_tick_d4);
    if (source.live_tick_18c)
        result.tick_18c = std::bit_cast<std::int32_t>(*source.live_tick_18c);
    result.live_globals = nullptr;
    result.live_tick_d4 = nullptr;
    result.live_tick_18c = nullptr;
    return result;
}
HistoryTickWindow history_tick_window(const HistoryTickTiming& source) noexcept {
    const auto timing = resolve_history_tick_timing(source);
    const float maximum = float(timing.maximum_unlag_ms) * 0.001f;
    const float delay = _mm_cvtss_f32(
        _mm_min_ss(_mm_set_ss(timing.latency_198 + timing.interpolation), _mm_set_ss(maximum)));
    const float centered = timing.current_time - delay;
    const auto first = add(time_to_tick(centered - 0.2f, timing.interval), 1);
    const auto last = sub(time_to_tick(centered + 0.2f, timing.interval), 1);
    const auto seconds = _mm_cvtt_ss2si(
        _mm_set_ss(float(add(timing.tick_d4, timing.tick_18c)) * timing.interval - maximum));
    return {first, last, time_to_tick(float(seconds), timing.interval)};
}
std::int32_t mean_history_timing_ticks(const TickTimingSamples& samples) noexcept {
    if (samples.count <= 0)
        return 1;
    auto sum = _mm_setzero_ps();
    for (std::int32_t i = 0; i < samples.count; ++i)
        sum = _mm_add_ss(sum, _mm_set_ss(samples.values[(samples.first + std::uint32_t(i)) & 15u]));
    return _mm_cvt_ss2si(_mm_div_ss(sum, _mm_cvtsi32_ss(_mm_setzero_ps(), samples.count)));
}
void publish_record_flags(LagRecord& record, const LagRecord* previous, bool compare_layer,
                          std::uint32_t update_flags, bool flag_43468d7c) noexcept {
    auto& flags = record.header.flags;
    if (previous && compare_layer && !(flags & 0x200u) && !(previous->header.flags & 0x200u) &&
        field(*previous, 0x118) == 0.0f && field(record, 0x118) > 0.0f)
        flags |= 0x400u;
    if (update_flags & 1u)
        flags |= 0x200u;
    if (update_flags & 4u) {
        flags |= 8u;
        if (flag_43468d7c)
            flags = (flags & 0xFFFFEFFFu) | 0x800u;
    }
    if (update_flags & 0x10u)
        flags |= 2u;
    if (update_flags & 0x20u)
        flags |= 0x20u;
    if (update_flags & 8u)
        flags |= 0x10u;
}
HistoryCaptureSlot prepare_history_capture(HistoryRecordingState& state, RecordPool& pool,
                                           bool enabled, bool suppressed,
                                           std::int32_t simulation_tick, bool refresh_fallback) {
    if (!enabled || suppressed) {
        clear_history_records(state.ring, pool);
        state.resolver_index = 0;
        return {nullptr, HistoryCaptureKind::none};
    }
    if (!state.ring.count || simulation_tick > state.ring.at(0).header.simulation_tick)
        return {&push_history_record(state.ring, pool), HistoryCaptureKind::inserted};
    if (refresh_fallback) {
        std::memset(&state.fallback, 0, sizeof(state.fallback));
        return {&state.fallback, HistoryCaptureKind::fallback};
    }
    return {nullptr, HistoryCaptureKind::none};
}
bool publish_history_fallback(HistoryRecordingState& state, bool captured_now, void* engine,
                              float interval, float receive_interval,
                              std::int32_t maximum_unlag_ms) {
    if (!state.ring.count ||
        state.fallback.header.receive_tick <= state.ring.at(0).header.receive_tick)
        return false;
    const auto receive_tick =
        receive_time_to_tick(engine::invoke<float>(engine, 14), receive_interval);
    if (!captured_now) {
        const float seconds = float(receive_tick) * interval - float(maximum_unlag_ms) * 0.001f;
        const auto truncated = _mm_cvtt_ss2si(_mm_set_ss(seconds));
        if (float(truncated) > float(state.ring.at(0).header.simulation_tick) * interval) {
            std::memcpy(&state.ring.at(0), &state.fallback, sizeof(LagRecord));
            state.fallback.header.receive_tick = -1;
            return true;
        }
    }
    state.flags |= 0x100u;
    return false;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_activation.cpp
// ============================================================================

namespace skeet::lagcomp {
bool history_prediction_available(const HistoryActivationBindings& bindings) noexcept {
    if (*bindings.prediction_suppressed != 0)
        return false;
    const auto shared_player = *bindings.local_player;
    if (!shared_player || !*shared_player)
        return false;
    return *(static_cast<const std::byte*>(*shared_player) + bindings.player_prediction_disabled) ==
           std::byte{0};
}

bool history_configuration_active(const HistoryActivationBindings& bindings) noexcept {
    const auto* configuration = legit::legit_owner_settings(*bindings.configuration);
    if (!configuration)
        return false;
    for (std::size_t index = 0; index != 9; ++index) {
        if (configuration->enabled.value[index] ||
            (configuration->trigger_enabled.value[index] &&
             configuration->trigger_reaction.value[index]) ||
            configuration->accuracy_boost.value[index])
            return true;
    }
    return false;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_alternative.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
/* [removed duplicate add] */
/* [removed duplicate sub] */
std::int32_t wrapped_absolute(std::int32_t value) noexcept {
    const auto sign = value < 0 ? UINT32_MAX : 0u;
    return std::bit_cast<std::int32_t>((std::uint32_t(value) ^ sign) - sign);
}
/* [removed duplicate absolute] */
/* [removed duplicate negate] */
/* [removed duplicate truncate] */
const void* terminal(const void* variable) noexcept {
    for (;;) {
        const auto parent = read<const void*>(variable, 0x1c);
        if (parent == variable)
            return variable;
        variable = parent;
    }
}
float variable_float(const void* variable) noexcept {
    variable = terminal(variable);
    return std::bit_cast<float>(read<std::uint32_t>(variable, 0x2c) ^
                                std::uint32_t(reinterpret_cast<std::uintptr_t>(variable)));
}
std::int32_t variable_integer(const void* variable) noexcept {
    variable = terminal(variable);
    return std::bit_cast<std::int32_t>(read<std::uint32_t>(variable, 0x30) ^
                                       std::uint32_t(reinterpret_cast<std::uintptr_t>(variable)));
}
std::int32_t prediction_limit(const HistoryAlternativePredictionLimit& bindings) noexcept {
    void* const local = bindings.local_player ? *bindings.local_player : nullptr;
    if (local) {
        if (!read<std::uint8_t>(local, bindings.field_bf567619))
            return *bindings.configured;
        if (read<std::uint8_t>(local, bindings.field_dda99ab5) &&
            read<std::int32_t>(local, bindings.field_b7ea8fda) == 1)
            return 3;
    }
    return 7;
}
/* [removed duplicate cap] */
float stamina_clamp(float value, float maximum) noexcept {
    return _mm_cvtss_f32(
        _mm_min_ss(_mm_max_ss(_mm_set_ss(value), _mm_setzero_ps()), _mm_set_ss(maximum)));
}
float planar_yaw(const Vector& velocity, const HistoryAlternativeBindings& bindings) {
    if (velocity[1] == 0.0f && velocity[0] == 0.0f)
        return 0.0f;
    return static_cast<float>(bindings.atan2_import(double(velocity[1]), double(velocity[0]))) *
           57.2957763671875f;
}
struct Motion {
    std::int32_t tick;
    std::uint32_t flags;
    Vector origin, velocity;
    float cosine, sine, heading, stamina;
};
static_assert(sizeof(Motion) == 48);
float squared_distance(const Vector& a, const Vector& b) noexcept {
    const float y = a[1] - b[1], x = a[0] - b[0], z = a[2] - b[2];
    return (y * y + x * x) + z * z;
}
MovementVector vector(const Vector& value) noexcept {
    return {value[0], value[1], value[2]};
}
Vector vector(MovementVector value) noexcept {
    return {value.x, value.y, value.z};
}
Vector advance(const Vector& origin, const Vector& velocity, float time) noexcept {
    return {velocity[0] * time + origin[0], velocity[1] * time + origin[1],
            velocity[2] * time + origin[2]};
}
std::size_t make_pattern(std::array<std::int32_t, 10>& pattern, const LagTimeline& timeline,
                         const HistoryAlternativeBindings& bindings) {
    std::size_t count = 0;
    bool simple = timeline.record_count < 2 || timeline.packet_count < 2;
    const auto simulation = time_to_tick(timeline.simulation_time, *bindings.interval);
    if (!simple && simulation < sub(timeline.last_update_tick, 1) &&
        simulation < timeline.at(0).header.simulation_tick)
        simple = true;
    if (!simple && packet_timing_discontinuity(timeline))
        simple = true;
    if (!simple && (timeline.at(0).header.flags & 0x200u))
        simple = true;
    if (!simple) {
        const auto head = timeline.packet_head % 6u, next = (timeline.packet_head + 1u) % 6u;
        if (sub(timeline.packets[head].field_18, timeline.packets[next].field_18) < 4 &&
            sub(timeline.at(0).header.receive_tick, timeline.at(1).header.receive_tick) > 7)
            simple = true;
    }
    if (simple)
        pattern[count++] = 2;
    else {
        const LagRecord* previous = nullptr;
        for (std::uint32_t age = 1; age < timeline.record_count; ++age) {
            const auto& record = timeline.at(age);
            if (previous && !(previous->header.flags & 8u)) {
                const auto limit = prediction_limit(bindings.prediction);
                pattern[count++] = cap(
                    sub(previous->header.simulation_tick, record.header.simulation_tick), limit);
                if (count == pattern.size())
                    break;
            }
            if (record.header.flags & 0x200u)
                break;
            previous = &record;
        }
        const auto limit = prediction_limit(bindings.prediction);
        const auto latest =
            cap(sub(timeline.at(0).header.simulation_tick, timeline.at(1).header.simulation_tick),
                limit);
        const auto arrival_limit = prediction_limit(bindings.prediction);
        const auto received_tick = sample_receive_tick(bindings.engine, *bindings.receive_interval);
        const auto arrival =
            cap(add(sub(received_tick, timeline.at(0).header.receive_tick), 1), arrival_limit);
        if (count < pattern.size())
            pattern[count++] = latest >= arrival ? latest : arrival;
    }
    return count;
}
} // namespace

std::int32_t model_alternative_history_record(client::CommandFrame& context, void* player,
                                              client::PlayerState& owner, LagRecord& destination,
                                              std::int32_t ticks,
                                              const HistoryAlternativeBindings& bindings) {
    const auto& newest = owner.history.at(0).header;
    const auto initial_tick = newest.simulation_tick;
    const auto now = add(std::bit_cast<std::int32_t>(context.field_d4),
                         std::bit_cast<std::int32_t>(context.field_18c));
    (void)engine::invoke<float>(bindings.engine, 14);
    (void)engine::invoke<float>(bindings.engine, 14);
    const auto timeline = history_timeline(owner);
    if (wrapped_absolute(sub(now, initial_tick)) >= 128)
        return 0;

    Motion motion;
    motion.tick = initial_tick;
    if (!(owner.native.flags & 0x100u)) {
        motion.origin = newest.origin;
        motion.velocity = newest.velocity;
        motion.flags = newest.player_flags;
    } else {
        const auto origin = owner.native.cached_origin;
        motion.origin = origin;
        motion.velocity = read<Vector>(player, bindings.fields.velocity_36375c23);
        motion.flags = read<std::uint32_t>(player, bindings.fields.player_flags_e2fbe695);
    }
    float turn = 0.0f;
    if (owner.history.count >= 2) {
        const auto& previous = owner.history.at(1).header;
        const float elapsed =
            float(sub(newest.simulation_tick, previous.simulation_tick)) * (*bindings.interval);
        const float newest_yaw = planar_yaw(newest.velocity, bindings);
        const float previous_yaw = planar_yaw(previous.velocity, bindings);
        turn =
            (normalize_movement_angle(newest_yaw - previous_yaw) / elapsed) * (*bindings.interval);
    }
    if (absolute(turn) > 6.0f)
        turn = 0.0f;
    motion.heading = planar_yaw(motion.velocity, bindings);
    const float heading_radians = motion.heading * 0.01745329238474369f;
    motion.cosine = bindings.trigonometry.cosine(heading_radians);
    motion.sine = bindings.trigonometry.sine(heading_radians);
    motion.stamina = 0.0f;

    auto* const scratch = bindings.animation_scratch.data();
    animation::initialize_animation_state_vector(scratch);
    void* const original_state = read<void*>(player, bindings.offsets.animation_state);
    if (original_state)
        std::memcpy(scratch, original_state, bindings.animation_scratch.size());
    write(player, bindings.offsets.animation_state, static_cast<void*>(scratch));
    const auto saved_angles = *engine::invoke<const Vector*>(player, 11);
    const bool owner_mode = ((owner.native.flags >> 1) & 1u) != 0;
    const float turn_radians = turn * 0.01745329238474369f;
    const float turn_cosine = bindings.trigonometry.cosine(turn_radians);
    const float turn_sine = bindings.trigonometry.sine(turn_radians);
    std::array<std::int32_t, 10> pattern;
    const auto pattern_count = make_pattern(pattern, timeline, bindings);

    bool has_cutoff = false;
    std::int32_t cutoff = 0;
    if ((context.flags & 0x4000u) && (owner.native.flags & 1u)) {
        const auto handle = read<std::uint32_t>(player, bindings.offsets.weapon_handle);
        if (handle != UINT32_MAX) {
            if (void* const weapon = engine::invoke<void*>(bindings.entity_list, 4, handle)) {
                const float shot_tick = read<float>(weapon, bindings.fields.weapon_time_0176a381) /
                                            (*bindings.interval) +
                                        0.5f;
                const float duration = engine::invoke<float>(weapon, 433, std::int32_t(0));
                const float quotient = duration / (*bindings.interval);
                const auto rounded = _mm_cvtss_si32(_mm_set_ss(quotient));
                cutoff = add(add(truncate(shot_tick), 1),
                             add(rounded, float(rounded) < quotient ? 1 : 0));
                has_cutoff = cutoff > initial_tick;
            }
        }
    }
    auto receive_base = timeline.last_update_tick;
    std::uint32_t pattern_index = 0;
    auto step = pattern[0];
    auto schedule = add(receive_base, step);
    while (schedule <= now) {
        if (schedule == now) {
            const auto index =
                engine::invoke<std::int32_t>(static_cast<std::byte*>(player) + 8, 10);
            if (engine::invoke<std::int32_t>(bindings.engine, 12) < index)
                break;
        }
        if (has_cutoff && cutoff > initial_tick && schedule > cutoff && add(schedule, ticks) >= now)
            break;
        const Motion batch_start = motion;
        for (std::int32_t i = 0; i < step; i = add(i, 1)) {
            const Motion before = motion;
            const float speed = std::sqrt(motion.velocity[0] * motion.velocity[0] +
                                          motion.velocity[1] * motion.velocity[1]);
            motion.velocity[0] = speed * motion.cosine;
            motion.velocity[1] = speed * motion.sine;
            if (bindings.variables.stamina_recovery && motion.stamina > 0.0f) {
                motion.stamina =
                    motion.stamina -
                    variable_float(bindings.variables.stamina_recovery) * (*bindings.interval);
                if (motion.stamina < 0.0f)
                    motion.stamina = 0.0f;
            }
            const float half_gravity =
                (variable_float(bindings.variables.gravity) * (*bindings.interval)) * 0.5f;
            motion.velocity[2] = motion.velocity[2] - half_gravity;
            if (motion.flags & 1u) {
                if (variable_integer(bindings.variables.bunnyhop) == 0) {
                    const float maximum =
                        read<float>(player, bindings.fields.maximum_speed_3eb71843) * 1.1f;
                    if (maximum > 0.0f) {
                        const float length = std::sqrt((motion.velocity[2] * motion.velocity[2] +
                                                        motion.velocity[0] * motion.velocity[0]) +
                                                       motion.velocity[1] * motion.velocity[1]);
                        if (maximum < length) {
                            const float scale = maximum / length;
                            motion.velocity[2] *= scale;
                            motion.velocity[0] *= scale;
                            motion.velocity[1] *= scale;
                        }
                    }
                }
                const float old_z = motion.velocity[2];
                const float impulse = variable_float(bindings.variables.jump_impulse);
                float jumping = (motion.flags & 2u) ? impulse : old_z + impulse;
                if (motion.stamina > 0.0f) {
                    const float fraction = stamina_clamp(1.0f - motion.stamina / 100.0f, 1.0f);
                    jumping *= fraction;
                }
                motion.velocity[2] = jumping - half_gravity;
                const float maximum = variable_float(bindings.variables.stamina_maximum);
                const float jump_scale = variable_float(bindings.variables.stamina_jump);
                motion.stamina = stamina_clamp(
                    (motion.velocity[2] - old_z) * jump_scale + motion.stamina, maximum);
            } else {
                // Air steering4333E3FF..E7BD, preserving the global alternator.
                const float ideal = 845.5f / std::sqrt(motion.velocity[0] * motion.velocity[0] +
                                                       motion.velocity[1] * motion.velocity[1]);
                if (!std::isnan(ideal)) {
                    float side_x, side_y;
                    if (motion.velocity[0] == 0.0f && motion.velocity[1] == 0.0f) {
                        side_x = 0.0f;
                        side_y = -1.0f;
                    } else {
                        const float x = motion.velocity[1] - motion.velocity[2] * 0.0f;
                        const float y = motion.velocity[2] * 0.0f - motion.velocity[0];
                        const float z = motion.velocity[0] * 0.0f - motion.velocity[1] * 0.0f;
                        const float inverse =
                            1.0f / (std::sqrt((x * x + y * y) + z * z) + 0x1p-23f);
                        side_x = inverse * x;
                        side_y = inverse * y;
                        const float cx =
                            motion.velocity[2] * side_y - inverse * z * motion.velocity[1];
                        const float cy =
                            inverse * z * motion.velocity[0] - motion.velocity[2] * side_x;
                        const float cz = side_x * motion.velocity[1] - side_y * motion.velocity[0];
                        [[maybe_unused]] volatile float magnitude =
                            std::sqrt((cx * cx + cy * cy) + cz * cz);
                    }
                    const float side_inverse =
                        1.0f /
                        (std::sqrt((side_x * side_x + side_y * side_y) + 0.0f * 0.0f) + 0x1p-23f);
                    float amount;
                    if (!(absolute(turn) > ideal) || !(30.0f > absolute(turn))) {
                        if (!bindings.alternate_strafe) {
                            motion.heading = ideal + motion.heading;
                            amount = 450.0f;
                        } else {
                            motion.heading = motion.heading - ideal;
                            amount = -450.0f;
                        }
                    } else
                        amount = turn > 0.0f ? -450.0f : 450.0f;
                    bindings.alternate_strafe = bindings.alternate_strafe == 0;
                    const float x = amount * (side_x * side_inverse),
                                y = amount * (side_y * side_inverse);
                    float wish_speed = std::sqrt((x * x + y * y) + 0.0f * 0.0f);
                    const float inverse = 1.0f / (wish_speed + 0x1p-23f);
                    const Vector direction{inverse * x, inverse * y, inverse * 0.0f};
                    const float maximum =
                        read<float>(player, bindings.fields.maximum_speed_3eb71843);
                    if (wish_speed != 0.0f && maximum <= wish_speed && wish_speed != maximum)
                        wish_speed = maximum;
                    const float acceleration = variable_float(bindings.variables.air_acceleration);
                    const float additional =
                        minimum_number(30.0f, wish_speed) -
                        ((direction[1] * motion.velocity[1] + direction[0] * motion.velocity[0]) +
                         direction[2] * motion.velocity[2]);
                    if (!(0.0f >= additional)) {
                        const float delta = minimum_number(additional, (wish_speed * acceleration) *
                                                                           (*bindings.interval));
                        motion.velocity[0] = direction[0] * delta + motion.velocity[0];
                        motion.velocity[1] = direction[1] * delta + motion.velocity[1];
                        motion.velocity[2] = direction[2] * delta + motion.velocity[2];
                    }
                }
            }
            const MovementVector minimum{-16.0f, -16.0f, 0.0f};
            const MovementVector maximum{16.0f, 16.0f, (motion.flags & 2u) ? 54.0f : 72.0f};
            MovementCollisionTrace trace{};
            trace.fraction = 1.0f;
            trace.entity = 0;
            trace.hitgroup = -1;
            trace.hitbox = -1;
            trace.displacement_flags = 0;
            auto trace_to = [&](const Vector& start, const Vector& end) {
                const auto ray =
                    make_movement_hull_ray(vector(start), vector(end), minimum, maximum);
                trace_movement_world(bindings.collision, ray, 1u, trace,
                                     bindings.engine_trace_flag);
            };
            trace_to(motion.origin, advance(motion.origin, motion.velocity, *bindings.interval));
            if (trace.fraction != 1.0f) {
                for (int bump = 0; bump < 2; ++bump) {
                    const auto normal = vector(trace.plane_normal);
                    float dot = (normal[1] * motion.velocity[1] + normal[0] * motion.velocity[0]) +
                                normal[2] * motion.velocity[2];
                    for (unsigned axis = 0; axis < 3; ++axis)
                        motion.velocity[axis] = motion.velocity[axis] - normal[axis] * dot;
                    dot = (normal[1] * motion.velocity[1] + normal[0] * motion.velocity[0]) +
                          normal[2] * motion.velocity[2];
                    if (dot < 0.0f)
                        for (unsigned axis = 0; axis < 3; ++axis)
                            motion.velocity[axis] = motion.velocity[axis] - normal[axis] * dot;
                    const float remaining = (1.0f - trace.fraction) * (*bindings.interval);
                    const auto start = vector(trace.end);
                    trace_to(start, advance(start, motion.velocity, remaining));
                    if (trace.fraction == 1.0f)
                        break;
                }
            }
            motion.origin = vector(trace.end);
            auto floor_end = motion.origin;
            floor_end[2] = floor_end[2] - 2.0f;
            trace_to(motion.origin, floor_end);
            if (trace.fraction != 1.0f && trace.plane_normal.z >= 0.7f)
                motion.flags |= 1u;
            else
                motion.flags &= 0xfffffffeu;
            motion.velocity[2] = motion.velocity[2] - half_gravity;
            const float falling = negate(motion.velocity[2]);
            if (motion.flags & 1u) {
                motion.velocity[2] = 0.0f;
                if (falling > 0.0f) {
                    const float maximum_stamina =
                        variable_float(bindings.variables.stamina_maximum);
                    const float land = variable_float(bindings.variables.stamina_land);
                    motion.stamina =
                        stamina_clamp(falling * land + motion.stamina, maximum_stamina);
                }
            }
            if ((before.flags & 1u) && !(motion.flags & 1u)) {
                write(scratch, 0x121, std::uint8_t(1));
                auto* const layers = read<void*>(player, bindings.offsets.animation_layers);
                write(layers, 0x10c, std::uint32_t(0));
                write(layers, 0x100, std::uint32_t(0));
                if (owner_mode && (motion.flags & 6u) == 6u)
                    motion.origin[2] = motion.origin[2] + 9.0f;
            }
            if (turn != 0.0f)
                motion.heading = normalize_movement_angle(motion.heading + turn);

            const float saved_time = *bindings.current_time;
            *bindings.current_time = float(motion.tick) * (*bindings.interval);
            const auto velocity = read<Vector>(player, bindings.fields.velocity_36375c23);
            write(player, bindings.fields.velocity_36375c23, motion.velocity);
            const auto origin4e = read<Vector>(player, bindings.fields.origin_4e66bb34);
            write(player, bindings.fields.origin_4e66bb34, motion.origin);
            const auto origin2a = read<Vector>(player, bindings.fields.origin_2a671b42);
            write(player, bindings.fields.origin_2a671b42, motion.origin);
            const auto tick = read<std::int32_t>(player, bindings.fields.tick_3d10b94c);
            write(player, bindings.fields.tick_3d10b94c, motion.tick);
            const auto flags = read<std::uint32_t>(player, bindings.fields.player_flags_e2fbe695);
            write(player, bindings.fields.player_flags_e2fbe695, motion.flags);
            auto angles = read<RageVector>(player, bindings.fields.angles_e23a9be1);
            if (i != sub(step, 1))
                angles[1] = newest.animation_yaw;
            const auto guard_offset = bindings.offsets.transform_guard_49d93e62 - 0x14u;
            const auto guard = read<std::uint32_t>(player, guard_offset);
            write(player, guard_offset, UINT32_MAX);
            animation::update_animation_state(scratch, angles,
                                              {*bindings.animation_frame, *bindings.current_time,
                                               *bindings.interval, bindings.animation_reset},
                                              bindings.animation_engine);
            write(player, guard_offset, guard);
            write(player, bindings.fields.player_flags_e2fbe695, flags);
            motion.tick = add(motion.tick, 1);
            write(player, bindings.fields.tick_3d10b94c, tick);
            write(player, bindings.fields.origin_2a671b42, origin2a);
            write(player, bindings.fields.origin_4e66bb34, origin4e);
            write(player, bindings.fields.velocity_36375c23, velocity);
            *bindings.current_time = saved_time;
            const float sine_term = turn_sine * motion.cosine;
            motion.cosine = turn_cosine * motion.cosine - turn_sine * motion.sine;
            motion.sine = turn_cosine * motion.sine + sine_term;
        }
        if (!(squared_distance(batch_start.origin, motion.origin) < 4096.0f) ||
            add(schedule, ticks) < now) {
            receive_base = add(receive_base, sub(motion.tick, batch_start.tick));
            ++pattern_index;
            step = pattern[pattern_index % pattern_count];
            schedule = add(receive_base, step);
        } else {
            motion = batch_start;
            break;
        }
    }
    const float factor = animation_speed_factor(scratch);
    std::int32_t result = 0;
    if ((motion.tick > initial_tick || (owner.native.flags & 0x100u)) &&
        !(squared_distance(destination.header.origin, motion.origin) < 4096.0f)) {
        destination.header.origin = motion.origin;
        destination.header.simulation_tick = motion.tick;
        destination.header.collision_max_z = (motion.flags & 2u) ? 54.0f : 72.0f;
        destination.header.velocity = motion.velocity;
        destination.header.player_flags = motion.flags;
        destination.header.animation_yaw = engine::read_opaque_native_float(scratch + 0x80);
        destination.header.animation_speed_factor = factor;
        animation::capture_entity_animation(destination.animation, player, bindings.offsets);
        destination.header.flags &= 0xffffff7bu;
        result = sub(motion.tick, initial_tick);
    }
    if (bindings.set_absolute_angles)
        bindings.set_absolute_angles(player, saved_angles);
    write(player, bindings.offsets.animation_state, original_state);
    animation::release_animation_state_vector(scratch, bindings.allocator);
    return result;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_animation_publish.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
/* [removed duplicate clamp_unit] */
/* [removed duplicate negate] */
/* [removed duplicate normalize] */
float quantize_yaw(float value) noexcept {
    const auto scaled = value * 182.0444488525390625f;
    const auto integer = _mm_cvttss_si32(_mm_set_ss(scaled));
    return float(std::uint32_t(integer) & 0xFFFFu) * 0.0054931640625f;
}
//433FD134 with the actual fixed inputs ECX=8,XMM1=0,XMM2=1. Its scale is
// exactly255 and no scale-retry arm is entered. FISTP obeys the x87 rounding
// mode; nearbyint preserves the active native floating-point rounding mode.
float quantize_weight(float value) noexcept {
    if (value < 0.0f)
        return 0.0f;
    if (value > 1.0f)
        return 1.0f;
    const auto scaled = value * 255.0f;
    return float(std::nearbyint(static_cast<double>(scaled))) / 255.0f;
}
float length2(const Vector& value) noexcept {
    return std::sqrt(value[0] * value[0] + value[1] * value[1]);
}
float length3(const Vector& value) noexcept {
    return std::sqrt((value[0] * value[0] + value[1] * value[1]) + value[2] * value[2]);
}
void* studio(void* player, const HistoryAnimationPublishBindings& bindings) noexcept {
    auto* wrapper = read<void*>(player, bindings.fields.studio_header);
    return wrapper && read<void*>(wrapper, 0) ? wrapper : nullptr;
}
float local_time(const HistoryAnimationPublishBindings& bindings) noexcept {
    if (!bindings.local_time_entity || !*bindings.local_time_entity)
        return 0.0f;
    return read<float>(*bindings.local_time_entity, bindings.fields.local_time);
}
//Complete433D716B. Pose writes are mirrored into both live interpolation
// destinations selected by the native16-bit cyclic watcher indices.
void publish_pose(void* poses, std::uint32_t index) noexcept {
    const auto first = read<std::uint16_t>(poses, 0x6C);
    const auto last = read<std::uint16_t>(poses, 0x6E);
    const auto relative = last < first ? std::uint32_t(last) : std::uint32_t(last - first);
    const auto* entries = read<const std::byte*>(poses, 0x68);
    auto* current = read<void*>(entries, std::size_t(relative) * 0xC + 8);
    const auto value = read<std::uint32_t>(poses, index * 4u);
    if (current)
        write(current, index * 4u, value);
    if (auto* destination = read<void*>(poses, 0x74))
        write(destination, index * 4u, value);
}
struct MovementCandidate {
    std::uint32_t ordinal;
    std::uint32_t seed;
    float movement_yaw;
    float feet_yaw;
    float pose_yaw;
    float score;
};
bool movement_eligible(client::PlayerState& owner, void* player, void* state,
                       std::int32_t simulation_tick, std::int32_t simulation_ticks,
                       std::uint32_t update_flags,
                       const HistoryAnimationPublishBindings& bindings) {
    const auto* native = &owner.native;
    if (!*bindings.configuration.resolver_enabled ||
        !bindings.player_table[bindings.player_table_offset + 6] || !owner.history.count)
        return false;
    const auto resolver = owner.native.resolver_index;
    if (resolver < 1 || resolver > 3)
        return false;
    const auto sequence = read<std::uint32_t>(native, 0x3A8);
    if (sequence != read<std::uint32_t>(native, 0x514) &&
        (sequence || !bindings.allow_zero_sequence))
        return false;
    if (0.000099999997473787516f >= read<float>(native, 0x51C))
        return false;
    if (animation::layer_is_active(reinterpret_cast<const std::byte*>(native) + 0x4F4) &&
        read<float>(native, 0x500) != 0.0f)
        return false;
    if (local_time(bindings) == owner.native.reference_time &&
        std::fabs(*bindings.now - read<float>(native, 0xCC)) < 5.0f)
        return false;
    if (!bindings.movement.displacement || !state ||
        !(read<std::uint8_t>(player, bindings.fields.player_flags) & 1u))
        return false;
    const auto receive = sample_receive_tick(bindings.clock, bindings.receive_interval);
    if (read<std::int32_t>(native, 0x1DC) != receive &&
        ((update_flags & 1u) || 0.0005000000237487257f >= read<float>(native, 0x524)))
        return false;
    const auto& previous = owner.history.at(0).header;
    if (!(previous.player_flags & 1u))
        return false;
    const auto blend = read<float>(native, 0x5A8);
    if ((update_flags & 8u) || simulation_ticks < 2 || (blend > 0.0f && blend < 1.0f))
        return true;
    const auto interval = bindings.entity.interval;
    if (time_to_tick(read<float>(player, bindings.fields.old_simulation_time), interval) !=
        simulation_tick)
        return false;
    if (read<float>(native, 0x3B0) - read<float>(native, 0x51C) > interval + interval)
        return false;
    const auto live = read<Vector>(player, bindings.fields.absolute_velocity);
    if (std::fabs(length2(previous.velocity) - length2(live)) >= interval * 2000.0f)
        return false;
    if (read<float>(native, 0x5C4) > 0.009999999776482582f) {
        const auto old_length = length3(previous.velocity);
        const auto live_length = length3(live);
        const auto old_scale = 1.0f / (old_length + std::numeric_limits<float>::epsilon());
        const auto live_scale = 1.0f / (live_length + std::numeric_limits<float>::epsilon());
        if (std::fabs(old_length - live_length) > 5.0f)
            return false;
        const float x = previous.velocity[0] * old_scale;
        const float y = previous.velocity[1] * old_scale;
        const float z = previous.velocity[2] * old_scale;
        const auto dot =
            (live[1] * live_scale * y + live[0] * live_scale * x) + live[2] * live_scale * z;
        if (!(dot >= 0.9993908405303955f))
            return false;
    }
    return true;
}
void resolve_movement(client::PlayerState& owner, void* player, void* state,
                      std::int32_t simulation_tick, float speed_fraction,
                      std::uint32_t& update_flags,
                      const HistoryAnimationPublishBindings& bindings) {
    if (!studio(player, bindings) || !studio(player, bindings))
        return;
    const bool reworked = bindings.configuration.reworked();
    auto* native = &owner.native;
    const auto running = read<float>(state, 0xF8);
    const auto ducking = read<float>(state, 0xFC);
    const auto animation_fraction = read<float>(state, 0x11C);
    const auto horizontal_speed = read<float>(state, 0xEC);
    const auto duck = read<float>(state, 0xA4);
    auto weight = read<float>(native, 0x51C);
    if (reworked)
        for (float value : std::array{running, ducking, animation_fraction, horizontal_speed, duck,
                                      weight, speed_fraction})
            if (!std::isfinite(value))
                return;
    bool angled = false;
    float cosine = 0.0f;
    if (weight < 1.0f && read<float>(native, 0x524) != 0.0f) {
        const auto layer_weight = read<float>(native, 0x500);
        if (layer_weight > 0.0f && read<float>(state, 0x124) > 0.0f)
            weight = (weight / maximum_number(1.0f - layer_weight, 0.550000011920929f)) /
                     read<float>(state, 0x124);
        const auto run = clamp_unit(running);
        const auto blend = (clamp_unit(ducking) - run) * duck + run;
        if (reworked && !(blend > 0.0f))
            return;
        cosine = clamp_unit(bindings.power(weight / blend, bindings.movement_exponent));
        angled = cosine < 0.9902681112289429f;
    }
    if (angled != bool(read<std::uint8_t>(native, 0xE4)) || (update_flags & 1u)) {
        write(native, 0xE4, std::uint8_t(angled));
        write(native, 0xE8, UINT32_MAX);
    }
    std::array<float, 4> yaws;
    std::size_t yaw_count = 0;
    if (angled) {
        const auto first = bindings.arccosine(cosine) * 57.2957763671875f;
        const auto second = bindings.arccosine(negate(cosine)) * 57.2957763671875f;
        yaws[yaw_count++] = first;
        yaws[yaw_count++] = negate(first);
        yaws[yaw_count++] = second;
        yaws[yaw_count++] = negate(second);
    } else
        yaws[yaw_count++] = 0.0f;
    auto* poses = static_cast<std::byte*>(player) + bindings.fields.poses;
    const auto saved_pose = read<float>(poses, 0x1C);
    const auto sequence = read<std::int32_t>(native, 0x514);
    float expected_speed = 0.0f;
    if (!reworked && read<float>(native, 0x524) > 0.0f) {
        const auto playback =
            engine::invoke<float>(player, 222, studio(player, bindings), sequence);
        const auto cycle = ((read<float>(native, 0x524) / bindings.entity.interval) /
                            (1.0f - animation_fraction * 0.15000000596046448f)) /
                           playback;
        expected_speed = (horizontal_speed / cycle) * (1.0f / playback);
    }
    if (reworked) {
        const auto rate = read<float>(native, 0x524);
        if (!(rate > 0.0f) || !std::isfinite(rate) || !std::isfinite(horizontal_speed) ||
            !std::isfinite(animation_fraction))
            return;
        // Playback appears once in the numerator and once in the denominator.
        // Cancel it before evaluation rather than producing 0/0 for a paused sequence.
        expected_speed = horizontal_speed * (1.0f - animation_fraction * 0.15f) *
                         bindings.entity.interval / rate;
        if (!(expected_speed > 0.0f) || !std::isfinite(expected_speed))
            return;
    }
    //The second native slot222 call is unconditional even though its returned
    //playback value is discarded.
    (void)engine::invoke<float>(player, 222, studio(player, bindings), sequence);
    float modifier = ((0.800000011920929f - speed_fraction * 0.30000001192092896f) - 1.0f) *
                         clamp_unit(running) +
                     1.0f;
    if (duck > 0.0f)
        modifier += clamp_unit(ducking) * duck * (0.5f - modifier);
    const auto maximum_yaw = read<float>(state, 0x338) * modifier;
    const auto velocity = read<Vector>(player, bindings.fields.absolute_velocity);
    const auto backward_x = negate(velocity[0]);
    const auto backward_y = negate(velocity[1]);
    const auto direction =
        backward_x == 0.0f && backward_y == 0.0f
            ? 0.0f
            : float(bindings.atan2(double(backward_y), double(backward_x))) * 57.2957763671875f;
    const std::array<float, 3> seeds{negate(maximum_yaw), 0.0f, maximum_yaw};
    MovementCandidate best{};
    best.score = 10.0f;
    std::uint32_t ordinal = 0;
    bool retain = false;
    if (reworked) {
        const auto minimum_yaw = read<float>(state, 0x334) * modifier;
        const auto candidate = [&](float pose_yaw, unsigned branch) {
            const auto feet_yaw = normalize(quantize_yaw(owner.native.reference_yaw - pose_yaw));
            const auto movement_yaw = normalize(direction - feet_yaw - yaws[branch]);
            const auto seed = pose_yaw < -maximum_yaw * 0.5f  ? 0u
                              : pose_yaw > maximum_yaw * 0.5f ? 2u
                                                              : 1u;
            return MovementCandidate{UINT32_MAX, seed, movement_yaw, feet_yaw, pose_yaw, 0.0f};
        };
        const auto fit = resolver_runtime::fit_movement_pose(
            minimum_yaw, maximum_yaw, unsigned(yaw_count), 0.25f, [&](float pose, unsigned branch) {
                const auto value = candidate(pose, branch);
                write(poses, 0x1C, clamp_unit((value.movement_yaw + 180.0f) / 360.0f));
                Vector displacement{};
                bindings.movement.displacement(bindings.movement.context, studio(player, bindings),
                                               sequence, reinterpret_cast<const float*>(poses),
                                               displacement);
                return std::fabs(length3(displacement) - expected_speed);
            });
        if (fit.identified) {
            best = candidate(fit.pose, fit.branch);
            best.score = fit.error;
        }
    } else
        for (std::uint32_t seed = 0; seed < 3 && !retain; ++seed) {
            auto pose_yaw = seeds[seed];
            if (pose_yaw != 0.0f) {
                const auto step =
                    std::fabs((speed_fraction * 20.0f + 30.0f) * bindings.entity.interval);
                const auto delta = normalize(0.0f - pose_yaw);
                if (delta > step)
                    pose_yaw += step;
                else if (delta < negate(step))
                    pose_yaw -= step;
                else
                    pose_yaw = 0.0f;
            }
            const auto feet_yaw = normalize(quantize_yaw(owner.native.reference_yaw - pose_yaw));
            const auto move_delta = direction - feet_yaw;
            for (auto* cursor = yaws.data(); cursor != yaws.data() + yaw_count;
                 ++cursor, ++ordinal) {
                const auto movement_yaw = normalize(move_delta - *cursor);
                write(poses, 0x1C, clamp_unit((movement_yaw - -180.0f) / 360.0f + 0.0f));
                //43347018->433E065C: concrete displacement-length calculation.
                Vector displacement{};
                bindings.movement.displacement(bindings.movement.context, studio(player, bindings),
                                               sequence, reinterpret_cast<const float*>(poses),
                                               displacement);
                const auto score = std::fabs(length3(displacement) - expected_speed);
                const MovementCandidate candidate{ordinal,  seed,     movement_yaw,
                                                  feet_yaw, pose_yaw, score};
                retain = owner.native.resolver_status == 2 &&
                         ordinal == read<std::uint32_t>(native, 0xE8) && score < 3.0f;
                if (retain || score < best.score)
                    best = candidate;
                if (retain)
                    break;
            }
        }
    write(poses, 0x1C, saved_pose);
    if (best.score < 10.0f) {
        //PUSH7 at43416218 moves the stack before DIVSS[ESP+34]; the source
        //operand is the enclosing frame's360 constant at stack+30.
        write(poses, 0x1C, clamp_unit((best.movement_yaw - -180.0f) / 360.0f + 0.0f));
        write(poses, 0x2C, clamp_unit((best.pose_yaw - -60.0f) / 120.0f + 0.0f));
        publish_pose(poses, 7);
        publish_pose(poses, 11);
        if (bindings.entity.set_absolute_angles)
            bindings.entity.set_absolute_angles(player, Vector{0.0f, best.feet_yaw, 0.0f});
        write(state, 0x80, best.feet_yaw);
        write(state, 0x88, best.movement_yaw);
        update_flags |= 0x20u;
        owner.native.reference_time = local_time(bindings);
        owner.native.resolver_status = 2;
        owner.native.reference_origin = read<Vector>(player, bindings.fields.player_origin);
        owner.native.resolver_slots[owner.native.resolver_index].attempt =
            bindings.candidate_attempts[best.seed];
        write(native, 0xE8, best.ordinal);
        write(native, 0xEC, simulation_tick);
    } else
        write(native, 0xE8, UINT32_MAX);
    //433CCE8E is the empty fixed-capacity-list destructor.
}
}
void publish_history_animation(client::PlayerState& owner, void* player, void* state,
                               std::int32_t simulation_tick, std::int32_t simulation_ticks,
                               float speed_fraction, float layer_weight,
                               std::uint32_t& update_flags,
                               const HistoryAnimationPublishSaved& saved,
                               const HistoryAnimationPublishBindings& bindings) {
    const auto& entity = bindings.entity;
    if (bindings.player_table[bindings.player_table_offset + 5])
        write(
            state, 0x80,
            owner.native.reference_yaw -
                float(read<std::int32_t>(bindings.player_table, bindings.player_table_offset + 8)));
    *saved.current_time = float(simulation_tick) * entity.interval;
    const auto previous = read<std::uint8_t>(player, bindings.fields.animation_enabled);
    write(player, bindings.fields.animation_enabled, read<std::uint8_t>(&owner.native, 0x17C));
    write(state, 0x70,
          std::bit_cast<std::int32_t>(std::bit_cast<std::uint32_t>(entity.frame) - 1u));
    if (read<float>(state, 0x6C) == *saved.current_time)
        write(state, 0x6C, entity.interval + *saved.current_time);
    engine::invoke<void>(player, 224);
    if (bindings.reset_byte)
        *bindings.reset_byte = 0;
    write(player, bindings.fields.animation_enabled, previous);
    const auto weight = read<float>(state, 0x9C);
    const auto blended =
        (maximum_number(1.0f - layer_weight, 0.550000011920929f) * read<float>(state, 0x124)) *
        weight;
    if (std::fabs(quantize_weight(clamp_unit(blended)) - read<float>(&owner.native, 0x51C)) >
        0.000099999997473787516f)
        write(state, 0x9C, read<float>(&owner.native, 0x51C));
    const auto* descriptor =
        static_cast<const std::byte*>(player) + entity.offsets.animation_layers;
    auto* layers = read<std::byte*>(descriptor, 0);
    auto count = read<std::int32_t>(descriptor, 0xC);
    if (count > 13)
        count = 13;
    for (std::int32_t index = 0; index < count; ++index)
        std::memcpy(layers + std::size_t(index) * 0x38 + 0x14,
                    reinterpret_cast<const std::byte*>(&owner.native) + 0x468 +
                        std::size_t(index) * 0x1C,
                    0x1C);
    write(state, 0x98, read<std::uint32_t>(&owner.native, 0x528));
    write(state, 0x19C, read<std::uint32_t>(&owner.native, 0x530));
    write(state, 0x190, read<std::uint32_t>(&owner.native, 0x538));
    write(state, 0x198, read<std::uint32_t>(&owner.native, 0x544));
    write(state, 0x180, read<std::uint32_t>(&owner.native, 0x5C4));
    if (movement_eligible(owner, player, state, simulation_tick, simulation_ticks, update_flags,
                          bindings))
        resolve_movement(owner, player, state, simulation_tick, speed_fraction, update_flags,
                         bindings);
    write(player, bindings.fields.invalidation_counter, std::uint32_t(0));
    write(player, entity.offsets.last_bone_setup_time, std::uint32_t(0xFF7FFFFF));
    write(player, entity.offsets.field_0471e13c, std::uint32_t(0));
    write(player, entity.offsets.effects_bb78dc43, std::uint32_t(4));
    *saved.frame_time = saved.saved_frame_time;
    *saved.current_time = saved.saved_current_time;
    publish_history_animation_origin(owner, player, saved.absolute_origin, bindings);
}
void publish_history_animation_origin(client::PlayerState& owner, void* player,
                                      const Vector& saved_absolute_origin,
                                      const HistoryAnimationPublishBindings& bindings) {
    const auto origin = *engine::invoke<const Vector*>(static_cast<std::byte*>(player) + 4, 2);
    write(player, bindings.fields.render_origin, origin);
    const auto& entity = bindings.entity;
    if (entity.set_absolute_origin) {
        const auto offset = entity.offsets.transform_guard_49d93e62 - 0x14u;
        const auto guard = read<std::uint32_t>(player, offset);
        write(player, offset, UINT32_MAX);
        entity.set_absolute_origin(player, saved_absolute_origin);
        write(player, offset, guard);
    }
    if (read<float>(static_cast<const std::byte*>(player) + bindings.fields.poses, 0x30) >= 0.75f)
        write(&owner.native, 0x160, *bindings.now);
}
} //namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_build.cpp
// ============================================================================

namespace skeet::lagcomp {
bool should_build_history_hypotheses(const HistoryHypothesisPolicy& policy, void* player) {
    if (!policy.enabled)
        return false;
    if (policy.game_type == 6) {
        if (policy.local_handle == UINT32_MAX ||
            engine::invoke<void*>(policy.entity_list, 4, policy.local_handle) != player)
            return false;
    }
    if (policy.force)
        return true;
    auto* networkable = static_cast<std::byte*>(player) + 8;
    const auto index = engine::invoke<std::uint32_t>(networkable, 10);
    std::uint32_t priority;
    std::memcpy(&priority, policy.player_table + std::size_t(index) * 0x20 + 0x14, 4);
    if (priority)
        return priority == 2;
    if (hotkey_active(policy.hotkey, policy.keys))
        return true;
    if (policy.settings_mask & policy.active_mode_mask)
        return true;
    return policy.physical_override && (policy.input_flags & 8u);
}
void build_deferred_history_record(LagRecord& record, void* player, float speed_fraction,
                                   const HistoryHypothesisPolicy& policy,
                                   const resolver_runtime::HypothesisInput& input,
                                   resolver_runtime::HypothesisBuildContext& context) {
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    if (!index)
        throw std::logic_error("Deferred history building requires a player index");
    context.entity.entity_flags |= 8u;
    context.entity.ik_mask = UINT32_MAX;
    const auto count = animation::build_resolver_hitbox_matrices(
        context.entity, context.model, context.origin, record.header.animation_yaw, record.matrices,
        context.engine, context.trigonometry);
    if (count)
        record.header.flags |= 4u;
    if (should_build_history_hypotheses(policy, player)) {
        auto parameters = input;
        parameters.speed_fraction = speed_fraction;
        resolver_runtime::build_hypotheses(record, parameters, context);
    }
    context.entity.entity_flags &= ~8u;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_environment.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
struct RestoreSuppression {
    std::uint8_t* flag;
    std::uint8_t saved = 0;
    explicit RestoreSuppression(std::uint8_t* value) : flag(value) {
        if (flag) {
            saved = *flag;
            *flag = 0;
        }
    }
    ~RestoreSuppression() {
        if (flag)
            *flag = saved;
    }
};
bool large_yaw_change(float difference) noexcept {
    // The native NaN check bypasses normalization and the timestamp write.
    if (std::isnan(difference))
        return false;
    if (std::fabs(difference) > 180.0f) {
        const float turns = round_half_away_from_zero(std::fabs(difference / 360.0f)) * 360.0f;
        difference = difference < 0.0f ? difference + turns : difference - turns;
    }
    return std::fabs(difference) > 45.0f;
}
}

HistoryEnvironmentUpdate update_history_environment(client::PlayerState& owner, const void* target,
                                                    HistoryEnvironmentScratch& scratch,
                                                    const HistoryEnvironmentInput& input,
                                                    const HistoryEnvironmentEngine& engine) {
    RageVector target_origin;
    std::memcpy(&target_origin, static_cast<const std::byte*>(target) + engine.origin_offset,
                sizeof(target_origin));
    // Native433AC5BD remains active through the angle import and timestamp
    // policy. The concrete probe's nested guards restore this zero, not the
    // caller's original suppression byte, until4341873D at the end.
    RestoreSuppression suppression(engine.probe.suppression_flag);
    const auto probe =
        resolver_runtime::probe_environment(input.local_origin, target_origin, engine.probe);
    auto& state = owner.native;
    HistoryEnvironmentUpdate result{probe.selected, false};
    if (!probe.selected) {
        state.environment_yaw = std::numeric_limits<float>::max();
        return result;
    }

    RageVector direction = probe.direction;
    if (probe.damage[0] == 0 && probe.damage[1] == 0) {
        //4340CEAF replaced stack+1F0 with the native ring address; stack+1F4
        // retains the preceding direction.x.4340E366 reuses those two words.
        // This deliberately differs from resolver43348FFC's local-eye reuse.
        direction[0] = engine::read_opaque_native_float(scratch.direction.data());
        direction[1] = std::bit_cast<float>(input.native_history_address);
    } else {
        scratch.direction[0] = direction[0];
        scratch.direction[1] = direction[1];
        // The sign-flipped side writes only x/y at4340E301..E31C.
        if (probe.damage[1] != 0)
            scratch.direction[2] = direction[2];
    }
    float yaw = 0.0f;
    if (direction[0] != 0.0f || direction[1] != 0.0f) {
        // Native converts atan2's double result to float before multiplying.
        yaw = static_cast<float>(engine.atan2(double(direction[1]), double(direction[0]))) *
              57.2957763671875f;
    }
    state.environment_yaw = yaw;
    if (yaw == std::numeric_limits<float>::max() || !(state.field_51c < 0.75f) ||
        owner.history.count < 2u || input.simulation_ticks > 2)
        return result;
    const auto& newest = owner.history.at(0).header;
    const float newest_yaw = std::bit_cast<float>(newest.player_history_1ac);
    if (!large_yaw_change(state.reference_yaw - newest_yaw))
        return result;
    const auto& previous = owner.history.at(1).header;
    const float previous_yaw = std::bit_cast<float>(previous.player_history_1ac);
    if (large_yaw_change(newest_yaw - previous_yaw)) {
        state.environment_yaw_change_time = input.current_time;
        result.timestamp_updated = true;
    }
    return result;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_jobs.cpp
// ============================================================================
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif

namespace skeet::lagcomp {
namespace {
std::uint32_t add_word(std::uint32_t& target, std::uint32_t value) {
#if defined(_WIN32)
    return static_cast<std::uint32_t>(InterlockedExchangeAdd(
        reinterpret_cast<volatile long*>(&target), static_cast<long>(value)));
#else
    return __atomic_fetch_add(&target, value, __ATOMIC_SEQ_CST);
#endif
}
std::uint32_t load_word(std::uint32_t& target) {
#if defined(_WIN32)
    return static_cast<std::uint32_t>(
        InterlockedCompareExchange(reinterpret_cast<volatile long*>(&target), 0, 0));
#else
    return __atomic_load_n(&target, __ATOMIC_SEQ_CST);
#endif
}
std::uintptr_t add_pointer(std::uintptr_t& target, std::uintptr_t value) {
#if defined(_WIN64)
    return static_cast<std::uintptr_t>(InterlockedExchangeAdd64(
        reinterpret_cast<volatile long long*>(&target), static_cast<long long>(value)));
#elif defined(_WIN32)
    return static_cast<std::uintptr_t>(InterlockedExchangeAdd(
        reinterpret_cast<volatile long*>(&target), static_cast<long>(value)));
#else
    return __atomic_fetch_add(&target, value, __ATOMIC_SEQ_CST);
#endif
}
std::uintptr_t load_pointer(std::uintptr_t& target) {
#if defined(_WIN32)
    return reinterpret_cast<std::uintptr_t>(InterlockedCompareExchangePointer(
        reinterpret_cast<void* volatile*>(&target), nullptr, nullptr));
#else
    return __atomic_load_n(&target, __ATOMIC_SEQ_CST);
#endif
}
void store_pointer(std::uintptr_t& target, std::uintptr_t value) {
#if defined(_WIN32)
    InterlockedExchangePointer(reinterpret_cast<void* volatile*>(&target),
                               reinterpret_cast<void*>(value));
#else
    __atomic_exchange_n(&target, value, __ATOMIC_SEQ_CST);
#endif
}
#if defined(_M_IX86) || defined(__i386__)
#define HISTORY_JOB_MEMBER __thiscall
#else
#define HISTORY_JOB_MEMBER
#endif
std::int32_t HISTORY_JOB_MEMBER retain_entry(void* task) {
    return engine::retain_job(*static_cast<engine::JobTask*>(task));
}
std::int32_t HISTORY_JOB_MEMBER release_entry(void* task) {
    return engine::release_job(*static_cast<engine::JobTask*>(task));
}
void HISTORY_JOB_MEMBER consume_entry(void* task) {
    consume_history_jobs(*static_cast<engine::JobTask*>(task));
}
}

void initialize_history_job_queue(HistoryJobQueue& queue) noexcept {
    queue.begin = queue.items.data();
    queue.end = queue.items.data();
    queue.limit = queue.items.data() + queue.items.size();
}
std::uint32_t history_job_count(const HistoryJobQueue& queue) noexcept {
    const auto difference = std::bit_cast<std::int32_t>(
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(queue.end) -
                                   reinterpret_cast<std::uintptr_t>(queue.begin)));
    return static_cast<std::uint32_t>(difference / 12);
}
bool append_history_job(HistoryJobQueue& queue, bool asynchronous, void* player,
                        float speed_fraction, LagRecord& record) noexcept {
    if (!asynchronous || history_job_count(queue) >= 64 || queue.begin != queue.items.data())
        return false;
    // Complete4341F0AD reserves12 bytes and ignores its stack argument.
    auto* item = queue.end++;
    item->player = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(player));
    item->speed_fraction = speed_fraction;
    item->record = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(&record));
    return true;
}

void consume_history_jobs(engine::JobTask& task) {
    const auto end = task.end;
    auto position = load_pointer(task.cursor);
    while (position < end) {
        position = add_pointer(task.cursor, 12);
        const auto next = position + 12;
        if (position < std::min(end, next)) {
            task.work(reinterpret_cast<void*>(position));
            add_word(task.completed, 1);
            position = next;
        }
    }
    engine::invoke<std::int32_t>(&task, 1);
}

HistoryJobDispatcher::HistoryJobDispatcher(void* thread_pool, engine::JobMemory memory)
    : table_{}, destroyed_{}, pool_(thread_pool), memory_(memory) {
    if (!pool_ || !memory_.allocate || !memory_.free || !memory_.purecall)
        throw std::invalid_argument("History jobs require the actual thread pool and heap exports");
    table_ = {
        {{reinterpret_cast<void*>(&retain_entry), reinterpret_cast<void*>(&release_entry),
          reinterpret_cast<void*>(&destruction_entry), reinterpret_cast<void*>(&consume_entry)}},
        this};
    destroyed_ = {memory_.purecall, memory_.purecall, reinterpret_cast<void*>(&destruction_entry),
                  memory_.purecall};
}
engine::JobTask* HistoryJobDispatcher::create() {
    const auto size = sizeof(engine::JobTask);
    auto* allocation = memory_.allocate(memory_.process_heap, 8, size + 0x20);
    if (!allocation)
        return nullptr;
    const auto raw = reinterpret_cast<std::uintptr_t>(allocation);
    const auto header = (raw + 15) & ~std::uintptr_t(15);
    const auto displacement = static_cast<std::uint32_t>(header - raw);
    const auto allocation_size = static_cast<std::uint32_t>(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    // Complete4341F142: the supplied native argument is ignored; the worker
    // is always4336A5B3, represented by the canonical concrete source entry.
    return new (reinterpret_cast<void*>(header + 16))
        engine::JobTask{table_.methods.data(), 0, 1, &execute_history_record_job, 0, 0, 0};
}
engine::JobTask* HistoryJobDispatcher::destroy(engine::JobTask& task, std::uint8_t flags) {
    const auto* table = reinterpret_cast<TaskTable*>(task.vtable);
    auto& owner = *table->owner;
    task.vtable = owner.destroyed_.data();
    if (flags & 1) {
        auto* object = reinterpret_cast<std::byte*>(&task);
        std::uint32_t displacement;
        std::memcpy(&displacement, object - 8, 4);
        owner.memory_.free(owner.memory_.process_heap, 0, object - 16 - displacement);
    }
    return &task;
}
engine::JobTask* HISTORY_JOB_MEMBER HistoryJobDispatcher::destruction_entry(void* object,
                                                                            std::uint8_t flags) {
    return destroy(*static_cast<engine::JobTask*>(object), flags);
}

void HistoryJobDispatcher::dispatch(engine::JobTask& task, HistoryJobItem* items,
                                    std::uint32_t count) {
    if (!count)
        return;
    const auto first = reinterpret_cast<std::uintptr_t>(items);
    store_pointer(task.cursor, first);
    task.end = first + static_cast<std::uintptr_t>(count) * 12;
    const auto available = engine::invoke<std::int32_t>(pool_, 8);
    auto workers = std::min(static_cast<std::int32_t>(count - 1), available);
    if (workers <= 0) {
        engine::invoke<std::int32_t>(&task, 0);
        consume_history_jobs(task);
        return;
    }
    workers = std::min(workers, 32);
    std::array<void*, 32> handles{};
    for (std::int32_t i = 0; i < workers; ++i) {
        void* handle = nullptr;
        engine::invoke<void, engine::JobTask*, void**, int, int>(pool_, 15, &task, &handle, 0, 8);
        if (handle) {
            engine::invoke<std::int32_t>(&task, 0);
            handles[static_cast<std::size_t>(i)] = handle;
        }
    }
    engine::invoke<std::int32_t>(&task, 0);
    consume_history_jobs(task);
    for (std::int32_t i = 0; i < workers; ++i)
        if (auto* handle = handles[static_cast<std::size_t>(i)])
            engine::invoke<void>(handle, 1);
    while (load_word(task.completed) != count) {
#if defined(_WIN32)
        _mm_pause();
#else
        __builtin_ia32_pause();
#endif
    }
}
void dispatch_history_jobs(HistoryJobQueue& queue, HistoryJobDispatcher& dispatcher) {
    if (queue.begin == queue.end)
        return;
    auto* task = dispatcher.create();
    dispatcher.dispatch(*task, queue.begin, history_job_count(queue));
    engine::release_job(*task);
}

#undef HISTORY_JOB_MEMBER
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_owner.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
void* allocate(engine::JobMemory memory, std::size_t size) { //43387448
    if (!size)
        return nullptr;
    auto* raw = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address);
    const auto total = std::uint32_t(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &total, 4);
    return reinterpret_cast<void*>(header + 16);
}
void release(engine::JobMemory memory, void* object) { //433837D0
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
HistoryAnimationPublishBindings
publication_bindings(const HistoryAnimationPublishBindings& source,
                     const HistoryRecordingConfiguration& configuration, const float& exponent) {
    return {source.entity,
            source.fields,
            source.clock,
            source.local_time_entity,
            source.now,
            source.receive_interval,
            source.reset_byte,
            configuration,
            source.player_table,
            source.player_table_offset,
            source.allow_zero_sequence,
            exponent,
            source.candidate_attempts,
            source.movement,
            source.power,
            source.arccosine,
            source.atan2};
}
HistoryRecordingBindings recording_bindings(const HistoryRecordingBindings& source,
                                            HistoryRecordingConfiguration& configuration,
                                            const HistoryAnimationPublishBindings& publication,
                                            const HistoryVelocityCaptureBindings& velocity) {
    return {source.players,    source.records,       source.entity,
            configuration,     source.activation,    source.targeting,
            source.prelude,    source.fields,        source.globals,
            source.hypotheses, source.yaw_side_fmod, source.bones,
            source.jobs,       source.environment,   velocity,
            source.resolver,   publication,          source.maximum_unlag_ms};
}
}

HistoryOwnerRuntime::HistoryOwnerRuntime(HistoryOwnerRuntime*& active,
                                         HistoryOwnerBindings bindings)
    : bindings_(bindings), active_(&active), configuration_(bindings.recording.configuration),
      physics_(bindings.recording.velocity.reconstruction),
      velocity_{bindings.recording.velocity.entity, physics_,
                bindings.recording.velocity.maximum_velocity_convar,
                bindings.recording.velocity.landing_limit_convar},
      publication_(
          publication_bindings(bindings.recording.publication, configuration_, movement_exponent_)),
      recording_(recording_bindings(bindings.recording, configuration_, publication_, velocity_)),
      runtime_(recording_) {
    auto& registries = bindings_.registries;
    registries.lifecycle.append(this); //4347E5EF, before publishing any table
    if (bindings_.scratch)
        receiver_.emplace(runtime_, *bindings_.scratch);
    else
        receiver_.emplace(runtime_);
    registries.entities.append(*receiver_); //4347E5F7, physical owner-local receiver
    registries.packets.append(&packet_endpoint_); //4347E5FF
    engine::register_frame_update(registries.frame_update, &frame_endpoint_); //4347E607
    registries.entities.publish(*receiver_); //4347E60E, after all four registrations
    packet_endpoint_.vtable = packet_; //4347E617
    frame_endpoint_.vtable = frame_; //4347E61E
    vtable = primary_; //4347E625

    initialize_boolean_config(registries.configuration.descriptors, bindings_.field_ed048f3d,
                              0xED048F3Du); //4347E631..4347E63F; stored value/reserved byte survive
    configuration_.target_hitboxes = nullptr;
    configuration_.recording_enabled = recording_.activation.recording_enabled;
    configuration_.resolver_enabled = &recording_.globals.transition_enabled;
    configuration_.prediction_available = 0;
    configuration_.engine = recording_.entity.engine; //captured4346A514, full+40
    first_activity_ = 0x3D3;
    second_activity_ = 0x3D4;

    //Complete433B2213->433C1360 for the decoded seven-byte literal. The
    //native valid allocation path owns eight bytes including the terminator.
    crouch_.begin = nullptr;
    crouch_.end = nullptr;
    crouch_.limit = nullptr;
    auto* text = static_cast<char*>(allocate(registries.lifecycle.memory, 8));
    crouch_.begin = text;
    crouch_.end = text;
    crouch_.limit = text + 8;
    std::memmove(text, "_crouch", 7);
    crouch_.end = text + 7;
    text[7] = 0;
    physics_.jump_sequence_name = crouch_.begin; //both actual43475AC0+4C consumers
    movement_exponent_ = std::bit_cast<float>(0x3EDC8172u); //full+5C, actual powf exponent
    registries.events.add({0x9D2A2EF7u, this, event}); //4347E6C6 ECX is the subscription hash
    // The source worker for4336A5B3 reads this same receiver runtime. Publish
    // it only after the complete constructor has established its owned data.
    bind_history_recording_runtime(runtime_);
}
HistoryOwnerRuntime::~HistoryOwnerRuntime() {
    //Complete4336B1E9. Signed native capacity comparison, then free through
    //the same allocation header. Its three string words are not cleared.
    const auto capacity =
        std::bit_cast<std::int32_t>(std::uint32_t(reinterpret_cast<std::uintptr_t>(crouch_.limit)) -
                                    std::uint32_t(reinterpret_cast<std::uintptr_t>(crouch_.begin)));
    if (capacity >= 2 && crouch_.begin)
        release(bindings_.registries.lifecycle.memory, crouch_.begin);
}
void HistoryOwnerRuntime::bind_configuration() {
    auto* descriptor = find_config(bindings_.registries.configuration.descriptors, 0x5B900854u);
    configuration_.target_hitboxes = &static_cast<IntegerConfig*>(descriptor)->value;
}
void HistoryOwnerRuntime::observe_packet(const void* packet_value) {
    if (!*configuration_.recording_enabled || !*configuration_.resolver_enabled ||
        !bindings_.local_player)
        return;
    //Valid native packet contract: the decoded zero-based index addresses a
    //player. No null-packet/failure substitute replaces the guarded body.
    const auto original = read<std::uint32_t>(packet_value, bindings_.fields.packet_player_index);
    const auto index = std::bit_cast<std::int32_t>(original + 1u);
    if (index > 64)
        return;
    if (!engine::invoke<void*>(recording_.entity.entity_list, 3, index))
        return;
    auto& player = *recording_.players.get(std::uint32_t(index));
    write(&player.native, 0x1C4, read<std::uint32_t>(packet_value, bindings_.fields.packet_timing));
    write(&player.native, 0x1BC,
          read<std::uint32_t>(packet_value, bindings_.fields.packet_timing + 4u));
}
void HistoryOwnerRuntime::on_event(events::GameEventReader& reader) {
    events::FieldValue value;
    value.integer = UINT32_MAX;
    if (reader.read(0x8361150Eu, events::FieldType::integer, value) || value.integer == UINT32_MAX)
        return;
    const auto player = engine::invoke<std::int32_t>(configuration_.engine, 9, value.integer);
    if (player != engine::invoke<std::int32_t>(configuration_.engine, 12))
        return;
    for (std::uint32_t index = 1; index <= 64; ++index) {
        auto& state = recording_.players.get(index)->native;
        state.probe_count = 0;
        state.resolver_status = 0;
    }
}

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_HISTORY_MEMBER __thiscall
#define SKEET_HISTORY_FAST __fastcall
#else
#define SKEET_HISTORY_MEMBER
#define SKEET_HISTORY_FAST
#endif
void SKEET_HISTORY_MEMBER HistoryOwnerRuntime::destroy(void* object) {
    auto* owner = static_cast<HistoryOwnerRuntime*>(object);
    destroy_history_owner(owner, *owner->active_);
}
void SKEET_HISTORY_MEMBER HistoryOwnerRuntime::initialize(void* object) {
    static_cast<HistoryOwnerRuntime*>(object)->bind_configuration();
}
void SKEET_HISTORY_MEMBER HistoryOwnerRuntime::unchanged(void*) {} //433CCE8E
void SKEET_HISTORY_FAST HistoryOwnerRuntime::packet(void* endpoint, const void* value) {
    static_cast<PacketEndpoint*>(endpoint)->owner->observe_packet(value);
}
void SKEET_HISTORY_FAST HistoryOwnerRuntime::frame(void* endpoint, std::uint8_t) {
    static_cast<FrameEndpoint*>(static_cast<engine::FrameUpdateEndpoint*>(endpoint))
        ->owner->update_environment();
}
void HistoryOwnerRuntime::event(void* owner, events::GameEventReader& reader,
                                events::EventPlayerIndices&) {
    static_cast<HistoryOwnerRuntime*>(owner)->on_event(reader);
}
#undef SKEET_HISTORY_MEMBER
#undef SKEET_HISTORY_FAST

std::size_t history_owner_size() noexcept {
    return sizeof(HistoryOwnerRuntime);
}
HistoryOwnerRuntime& construct_history_owner(void* storage, HistoryOwnerRuntime*& active,
                                             HistoryOwnerBindings bindings) {
    return *::new (storage) HistoryOwnerRuntime(active, bindings);
}
void destroy_history_owner(void* storage, HistoryOwnerRuntime*& active) {
    active = nullptr;
    if (!storage)
        return;
    auto* owner = static_cast<HistoryOwnerRuntime*>(storage);
    const auto memory = owner->bindings_.registries.lifecycle.memory;
    owner->~HistoryOwnerRuntime();
    release(memory, storage);
}
} //namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_owner_environment.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
//Complete43407DE2. Its COMISS policy admits a NaN decoded value and rejects
//either signed zero; the same required ConVar object is read at this gate.
bool environment_enabled(const void* object) noexcept {
    for (;;) {
        const auto* parent = read<const void*>(object, 0x1C);
        if (parent == object)
            break;
        object = parent;
    }
    const auto encoded = read<std::uint32_t>(object, 0x2C);
    return std::bit_cast<float>(encoded ^
                                std::uint32_t(reinterpret_cast<std::uintptr_t>(object))) != 0.0f;
}
}
void HistoryOwnerRuntime::update_environment() {
    const auto& bindings = recording_;
    if (!engine::invoke<bool>(configuration_.engine, 26))
        return;
    if (!environment_enabled(bindings_.environment_convar))
        return;
    if (!bindings_.local_player)
        return;
    if (read<std::uint8_t>(bindings_.local_player, bindings_.fields.local_life_state))
        return;
    if (bindings.hypotheses.selected_handle == -1)
        return;
    auto* target =
        engine::invoke<void*>(bindings.entity.entity_list, 4, bindings.hypotheses.selected_handle);
    if (!target)
        return;
    if (engine::invoke<bool>(static_cast<std::byte*>(target) + 8, 9))
        return;
    if (read<std::uint32_t>(target, bindings.fields.effects_38ca5228) & 8u)
        return;
    if (read<std::uint8_t>(target, bindings.fields.field_043fa575) !=
        read<std::uint8_t>(target, bindings.fields.field_334b460c))
        return;
    //433279A4 reloads the selected handle after target/networkable calls.
    const auto index = std::uint32_t(bindings.hypotheses.selected_handle) & 0xFFFFu;
    if (resolver_runtime::has_pending_environment_shot(bindings_.feedback.pending, index))
        return;
    if (!bindings.globals.adjustments.settings_base()[std::size_t(index) * 0x20 + 6])
        return;
    if (!index)
        return;
    auto& owner = *bindings.players.get(index);
    const resolver_runtime::EnvironmentLiveUpdateInput input{
        target,
        bindings.bones,
        bindings.resolver.local_time_entity,
        bindings.resolver.fields.local_time_9f8e7f71,
        bindings.globals.current_time,
        bindings_.command_tick,
        bindings.entity.previous_command,
        bindings.entity.current_command,
        bindings.globals.local_origin,
        bindings.environment.origin_offset,
        bindings.configuration.resolver_reworked()};
    //The common live implementation retains the canonical owner/ring. Its
    //world probe precedes the fresh studio/set/hitbox0/3 reads.
    resolver_runtime::update_environment_entry(owner, input, bindings.resolver.environment);
}
} //namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_receiver.cpp
// ============================================================================

namespace skeet::lagcomp {
HistoryRecordingReceiver::HistoryRecordingReceiver(HistoryRecordingRuntime& value) noexcept
    : runtime(&value), scratch(nullptr) {}
HistoryRecordingReceiver::HistoryRecordingReceiver(HistoryRecordingRuntime& value,
                                                   HistoryRecordingScratch& incoming) noexcept
    : runtime(&value), scratch(&incoming) {}
void HistoryRecordingReceiver::record_players(void* local) const {
    if (scratch)
        runtime->record_players(local, *scratch);
    else
        runtime->record_players(local);
}
} //namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_resolver_update.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
/* [removed duplicate absolute] */
/* [removed duplicate negative] */
/* [removed duplicate normalize] */
/* [removed duplicate truncate] */
/* [removed duplicate add] */
/* [removed duplicate subtract] */
/* [removed duplicate clamp_unit] */
float convar(const void* value) noexcept {
    return std::bit_cast<float>(read_prediction_convar(const_cast<void*>(value)));
}
float local_time(const HistoryResolverUpdateBindings& bindings) noexcept {
    if (!bindings.local_time_entity || !*bindings.local_time_entity)
        return 0.0f;
    return read<float>(*bindings.local_time_entity, bindings.fields.local_time_9f8e7f71);
}
struct Suppression {
    std::uint8_t* target;
    std::uint8_t saved = 0;
    explicit Suppression(std::uint8_t* value) : target(value) {
        if (target) {
            saved = *target;
            *target = 0;
        }
    }
    void restore() {
        if (target) {
            *target = saved;
            target = nullptr;
        }
    }
    ~Suppression() {
        restore();
    }
};
resolver_runtime::LagRecordHistory history_view(client::PlayerState& owner) noexcept {
    return {owner.history.records, owner.history.first, owner.history.count};
}
//43412A95..434137FE: this caller uses model descriptors0/3 without the
// nineteen-hitbox cap used by the independent aiming-model reader.
bool feedback_from_environment(client::PlayerState& owner, void* player, std::uint8_t mode,
                               std::uint8_t previous_attempt, float clock,
                               HistoryResolverScratch& scratch,
                               const HistoryResolverUpdateBindings& bindings) {
    auto& state = owner.native;
    Suppression suppression(bindings.environment.suppression_flag);
    const auto origin = read<RageVector>(player, bindings.entity.offsets.origin);
    const auto probe =
        resolver_runtime::probe_environment(bindings.local_origin, origin, bindings.environment);
    if (!probe.selected)
        return false;
    RageVector direction;
    if (probe.damage[0] == 0 && probe.damage[1] == 0)
        for (unsigned axis = 0; axis < 3; ++axis)
            direction[axis] =
                engine::read_opaque_native_float(scratch.probe_direction.data() + axis);
    else {
        direction = probe.direction;
        scratch.probe_direction = direction;
    }
    const auto* wrapper = read<const void*>(player, bindings.fields.studio_wrapper);
    if (!wrapper)
        return false;
    const auto* header = read<const std::byte*>(wrapper, 0);
    if (!header)
        return false;
    const auto* set =
        header + read<std::int32_t>(header, 0xB0) +
        std::ptrdiff_t(read<std::int32_t>(player, bindings.fields.hitbox_set_c2b6fb10)) * 12;
    if (read<std::int32_t>(set, 4) < (bindings.configuration.resolver_reworked() ? 4 : 1))
        return false;
    const auto* descriptors = set + read<std::int32_t>(set, 8);
    std::array<resolver_runtime::EnvironmentCapsule, 2> hitboxes;
    constexpr std::size_t indices[]{0, 3};
    for (std::size_t i = 0; i < 2; ++i) {
        const auto* box = descriptors + indices[i] * 0x44;
        const auto radius = read<float>(box, 0x30);
        if (!(radius > 0.0f))
            return false;
        hitboxes[i] = {std::uint32_t(indices[i]), read<RageVector>(box, 8),
                       read<RageVector>(box, 0x14), radius};
    }
    const auto& source = owner.history.at(0);
    const float yaw_delta =
        state.reference_yaw - std::bit_cast<float>(source.header.player_history_1ac);
    const auto& old_origin = source.header.origin;
    const bool rebase = yaw_delta != 0.0f || old_origin[0] != origin[0] ||
                        old_origin[1] != origin[1] || old_origin[2] != origin[2];
    for (auto& axis : direction)
        axis = negative(axis);
    const RageVector eye{bindings.local_origin[0], bindings.local_origin[1],
                         bindings.local_origin[2] + 64.0f};
    const auto selection =
        rebase ? resolver_runtime::score_environment_hypotheses(
                     source, hitboxes, direction, eye, {yaw_delta, old_origin, origin},
                     bindings.environment.trigonometry, bindings.configuration.resolver_reworked())
               : resolver_runtime::score_environment_hypotheses(source, hitboxes, direction, eye,
                                                                bindings.configuration.resolver_reworked());
    if (!selection.valid())
        return false;
    const auto attempt = resolver_runtime::classify_winning_pose(selection.pose_yaw);
    state.resolver_slots[mode].attempt = attempt;
    state.probe_time = clock;
    state.last_update_tick =
        corrected_command_tick(bindings.entity.command_tick, bindings.entity.previous_command,
                               bindings.entity.current_command);
    suppression.restore();
    state.resolver_status = 1;
    if (mode == 1)
        state.resolver_slots[2].attempt = attempt;
    else if (mode == 2)
        state.resolver_slots[1].attempt = attempt;
    const bool strict = bindings.configuration.resolver_reworked();
    const auto history_attempt = strict ? source.header.resolver_mode : previous_attempt;
    if ((!strict || source.header.resolver_index == mode) && history_attempt != attempt) {
        auto history = history_view(owner);
        resolver_runtime::propagate_attempt(history, mode, history_attempt, attempt);
    }
    return true;
}
//43411CBD..434138D9, before the selected mode's animation-yaw policy.
void update_inline_feedback(client::PlayerState& owner, void* player, std::uint8_t& mode,
                            SimulationTickSpan span, std::uint32_t& flags,
                            HistoryResolverScratch& scratch,
                            const HistoryResolverUpdateBindings& bindings) {
    auto& state = owner.native;
    const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(player) + 8, 10);
    if (!bindings.player_table[std::size_t(index) * 0x20 + 6])
        return;
    if (!(state.flags & 0x40u) && read<float>(&state, 4) == local_time(bindings)) {
        state.flags |= 0x40u;
        const auto delta = normalize(state.reference_yaw - read<float>(&state, 8));
        if (absolute(delta) < 60.0f) {
            mode = std::uint8_t(!(state.flags & 4u)) + 1;
            state.resolver_slots[mode].attempt = resolver_runtime::classify_winning_pose(delta);
            state.resolver_status = 3;
            return;
        }
    }
    if (state.resolver_status == 3 || (flags & 1u))
        return;
    const auto selected = bindings.selected_entity;
    if (selected == -1 || engine::invoke<void*>(bindings.entity.entity_list, 4, selected) != player)
        return;
    if (span.count <= 1 &&
        (owner.history.count <= 1 || subtract(owner.history.at(0).header.simulation_tick,
                                              owner.history.at(1).header.simulation_tick) <= 1))
        return;
    const auto selected_index = selected == -1 ? 0x1FFFu : std::uint32_t(std::uint16_t(selected));
    const auto pending_shots = std::span<const HistoryEvaluation>(bindings.pending_shots);
    if (resolver_runtime::has_pending_environment_shot(pending_shots, selected_index))
        return;
    if (mode != 1 && mode != 2)
        return;
    const auto previous = state.resolver_slots[mode].attempt;
    const auto clock = local_time(bindings);
    if (clock == state.probe_time &&
        (state.probe_count >= 2 || absolute(bindings.current_time - state.probe_stamp) < 5.0f))
        return;
    if (!owner.history.count)
        return;
    if (clock == state.reference_time && state.resolver_status == 2) {
        const auto origin = read<RageVector>(player, bindings.entity.offsets.origin);
        const float y = state.reference_origin[1] - origin[1],
                    x = state.reference_origin[0] - origin[0],
                    z = state.reference_origin[2] - origin[2];
        if (10000.0f > (y * y + x * x) + z * z)
            return;
    }
    if (!(owner.history.at(0).header.flags & 0x80u))
        return;
    if (feedback_from_environment(owner, player, mode, previous, clock, scratch, bindings))
        flags |= 0x10u;
}
bool recent_shot_matches_pose(const client::NativePlayerStateLayout& state, std::uint8_t mode,
                             const resolver_runtime::JitterPoseEstimate& estimate,
                             float maximum_yaw, float now) {
    const auto offset = std::size_t(mode) * 0x24;
    const float fired = read<float>(&state, 0x14 + offset);
    const float fired_yaw = read<float>(&state, 0x18 + offset);
    const float age = now - fired;
    const float yaw_change = std::fabs(std::remainder(state.reference_yaw - fired_yaw, 360.0f));
    if (!(fired > 0 && age >= 0 && age <= 0.25f && yaw_change <= 4.0f))
        return false;

    const auto attempt = state.resolver_slots[mode].attempt % 3u;
    const float delta = attempt == 1 ? -maximum_yaw : attempt == 2 ? maximum_yaw : 0.0f;
    const float old_error = std::fabs(std::remainder(
        state.reference_yaw + delta - estimate.anchor, 360.0f));
    // Keep fresh evidence only while the current observation fits that pose.
    // A body-yaw change can invalidate it even when the eye yaw repeats.
    return old_error - estimate.error <= std::max(3.0f, maximum_yaw * 0.15f);
}
void choose_resolver_yaw(client::PlayerState& owner, void* player, void* animation_state,
                         SimulationTickSpan span, std::int32_t shot_tick, std::uint32_t& flags,
                         const HistoryResolverInput& input, HistoryResolverScratch& scratch,
                         const HistoryResolverUpdateBindings& bindings) {
    auto& state = owner.native;
    const auto angles = bindings.entity.offsets.eye_angles;
    const auto* table = bindings.player_table + input.player_table_offset;
    const float previous_animation_yaw = read<float>(animation_state, 0x80);
    using resolver_runtime::JitterDecision;
    auto& evidence = owner.jitter.current;
    const bool evaluate_jitter = bindings.configuration.jitter_enabled() &&
                                 evidence.tick == span.last && span.count >= 1 && span.count <= 16 &&
                                 !(flags & 5u);
    if (evaluate_jitter) {
        evidence.estimate = resolver_runtime::estimate_jitter_pose(
            state.reference_yaw, std::bit_cast<float>(state.capture_field_180), input.maximum_yaw);
        if (!evidence.estimate.valid())
            evidence.decision = JitterDecision::ambiguous_pose;
    }
    const bool use_prior = evaluate_jitter && evidence.estimate.valid();
    if (table[5]) {
        write(animation_state, 0x80, state.reference_yaw - float(read<std::int32_t>(table, 8)));
        evidence.decision = JitterDecision::forced_yaw;
    } else {
        if (table[6] || use_prior) {
            if (table[6] && !table[4] && bindings.configuration.prediction_available) {
                float pitch = read<float>(player, angles);
                const auto delta =
                    absolute(normalize((read<float>(player, bindings.fields.recoil_pitch_66146802) +
                                        read<float>(&state, 0x1A8)) -
                                       180.0f));
                if (delta < 0.1f) {
                    pitch = -180.0f;
                    if (read<std::uint8_t>(&state, 0xA4) &&
                        absolute(read<float>(&state, 0x108)) == 180.0f)
                        pitch = read<float>(&state, 0x108);
                    else if (read<std::uint16_t>(&state, 0xA6) % 11 == 10)
                        pitch = 180.0f;
                }
                write(player, angles, pitch);
            }
            std::uint8_t mode = 0, attempt = 0;
            const bool stock_selects_pose = table[6] &&
                (span.count > 1 || read<std::uint8_t>(&state, 0xA3) > 1);
            if (stock_selects_pose || use_prior)
                mode = (flags & 4u) && bindings.shot_angles_active
                           ? 3
                           : std::uint8_t(!(state.flags & 4u)) + 1;
            update_inline_feedback(owner, player, mode, span, flags, scratch, bindings);
            if (use_prior && mode > 0 && mode < 3) {
                auto& decision = evidence.decision;
                if (state.resolver_status != 0) {
                    decision = state.resolver_status == 1   ? JitterDecision::environment
                               : state.resolver_status == 2 ? JitterDecision::movement
                                                            : JitterDecision::shared;
                } else if (recent_shot_matches_pose(state, mode, evidence.estimate,
                                                    input.maximum_yaw, bindings.current_time)) {
                    decision = JitterDecision::shot_feedback;
                } else {
                    auto& chosen = state.resolver_slots[mode].attempt;
                    const bool changed = !stock_selects_pose || chosen % 3u != evidence.estimate.attempt;
                    if (changed)
                        chosen = evidence.estimate.attempt;
                    decision = changed ? JitterDecision::animation_yaw : JitterDecision::unchanged_pose;
                }
            } else if (flags & 4u) {
                evidence.decision = JitterDecision::shot_record;
            }
            if (mode == 0) {
                mode = input.previous_resolver_index;
                attempt = read<std::uint8_t>(&state, 0xA2);
            } else if (mode < 3) {
                const auto index =
                    engine::invoke<std::int32_t>(static_cast<std::byte*>(player) + 8, 10);
                attempt = state.resolver_slots[mode].attempt % 3u;
                if (attempt == 0) {
                    if (evidence.applied() || bindings.player_table[std::size_t(index) * 0x20 + 6] ||
                        read<std::int32_t>(&state, std::size_t(mode) * 0x24 + 0x20) > 1 ||
                        absolute(bindings.current_time -
                                 read<float>(&state, std::size_t(mode) * 0x24 + 0x1C)) <= 2.0f)
                        write(animation_state, 0x80, state.reference_yaw);
                    else if (input.previous_resolver_index != mode ||
                             read<std::uint8_t>(&state, 0xA2) != 0)
                        write(animation_state, 0x80, std::bit_cast<float>(state.capture_field_180));
                } else if (attempt == 1) {
                    write(animation_state, 0x80, state.reference_yaw - input.maximum_yaw);
                    write(player, angles + 4, normalize(state.reference_yaw - 120.0f));
                } else if (attempt == 2) {
                    write(animation_state, 0x80, state.reference_yaw + input.maximum_yaw);
                    write(player, angles + 4, normalize(state.reference_yaw + 120.0f));
                }
            } else if (mode == 3) {
                bool retain_center = false;
                if (span.count == 1 || (bindings.shot_angles_active && (flags & 4u) &&
                                        shot_tick == add(span.first, 1))) {
                    const auto difference = state.reference_yaw - read<float>(&state, 0x114);
                    retain_center =
                        std::isnan(difference) || absolute(normalize(difference)) < 120.0f;
                }
                if (!retain_center) {
                    const auto delta = normalize(state.reference_yaw -
                                                 std::bit_cast<float>(state.capture_field_180));
                    attempt = read<std::uint8_t>(&state, 0x7C) % 3u;
                    if (attempt == 2)
                        write(animation_state, 0x80, state.reference_yaw);
                    else if ((attempt == 0 && delta > 0.0f) || (attempt == 1 && !(delta > 0.0f))) {
                        write(animation_state, 0x80, state.reference_yaw + input.maximum_yaw);
                        write(player, angles + 4, normalize(state.reference_yaw + 120.0f));
                    } else {
                        write(animation_state, 0x80, state.reference_yaw - input.maximum_yaw);
                        write(player, angles + 4, normalize(state.reference_yaw - 120.0f));
                    }
                }
            }
            write(&state, 0xA2, attempt);
            write(&state, 0xA3, std::uint8_t(span.count));
            state.resolver_index = mode;
        }
        if (read<float>(&state, 0x524) > 0.0f)
            write(player, bindings.entity.offsets.field_a2a8247f, read<float>(player, angles + 4));
    }
    const auto yaw = read<float>(animation_state, 0x80);
    if (previous_animation_yaw != yaw)
        write(animation_state, 0x80, normalize(yaw));
}

struct StepFields {
    std::uint32_t flags;
    RageVector angles;
    float recoil, lower_body_yaw, duck;
    Vector velocity, absolute_origin;
};
StepFields capture_step_fields(void* player,
                               const HistoryResolverUpdateBindings& bindings) noexcept {
    const auto& o = bindings.entity.offsets;
    return {read<std::uint32_t>(player, o.player_flags),
            read<RageVector>(player, o.eye_angles),
            read<float>(player, bindings.fields.recoil_pitch_66146802),
            read<float>(player, o.field_a2a8247f),
            read<float>(player, o.duck_amount),
            read<Vector>(player, o.velocity),
            read<Vector>(player, bindings.fields.absolute_origin_4e66bb34)};
}
void publish_step_fields(void* player, const StepFields& value,
                         const HistoryResolverUpdateBindings& bindings) noexcept {
    const auto& o = bindings.entity.offsets;
    write(player, o.player_flags, value.flags);
    write(player, o.eye_angles, value.angles);
    write(player, bindings.fields.recoil_pitch_66146802, value.recoil);
    write(player, o.field_a2a8247f, value.lower_body_yaw);
    write(player, o.duck_amount, value.duck);
    write(player, o.velocity, value.velocity);
    write(player, bindings.fields.absolute_origin_4e66bb34, value.absolute_origin);
}
std::int32_t animation_step_count(const void* state, float interval) noexcept {
    const auto current = read<float>(state, 0x5B4);
    auto cycle = read<float>(state, 0x448), rate = read<float>(state, 0x444);
    const float delta = current - cycle;
    std::int32_t ticks = 0;
    // COMISS zero,delta / JBE includes an unordered delta in this arm.
    if (!(delta < 0.0f))
        ticks = truncate((delta / rate) / interval + 0.5f);
    else
        for (std::int32_t i = 0; i < 16; ++i) {
            ticks = add(ticks, 1);
            if (rate * interval + cycle >= 0x1.fffcp-1f)
                rate = read<float>(state, 0x5B0);
            cycle = rate * interval + cycle;
            const auto integer = truncate(cycle);
            if (integer > 0) {
                const auto rest = truncate(
                    ((current - (cycle - float(integer))) / read<float>(state, 0x5B0)) / interval +
                    0.5f);
                ticks = add(ticks, rest);
                break;
            }
        }
    return ticks < 1 ? 1 : ticks > 16 ? 16 : ticks;
}
void step_stock_animation(client::PlayerState& owner, void* player, void* animation_state,
                            SimulationTickSpan span, std::int32_t shot_tick, std::uint32_t flags,
                            HistoryResolverUpdateResult& result,
                            const HistoryResolverUpdateBindings& bindings) {
    auto& state = owner.native;
    const auto& o = bindings.entity.offsets;
    auto* descriptor = static_cast<std::byte*>(player) + o.animation_layers;
    const auto count = read<std::int32_t>(descriptor, 0xC);
    const auto copy_count = count > 13 ? 13 : count;
    for (std::int32_t i = 0; i < copy_count; ++i)
        std::memcpy(static_cast<std::byte*>(read<void*>(descriptor, 0)) + std::size_t(i) * 0x38 +
                        0x14,
                    reinterpret_cast<const std::byte*>(&state) + 0x3A4 +
                        std::size_t(i) * 0x1C,
                    0x1C);
    constexpr std::size_t animation_offsets[]{0x98, 0x19C, 0x190, 0x198, 0x180};
    constexpr std::size_t owner_offsets[]{0x3BC, 0x3C4, 0x3CC, 0x3D8, 0x458};
    for (std::size_t i = 0; i < 5; ++i)
        write(animation_state, animation_offsets[i], read<std::uint32_t>(&state, owner_offsets[i]));
    for (std::int32_t i = 0; i < copy_count; ++i)
        write(read<void*>(descriptor, 0), std::size_t(i) * 0x38 + 0x18,
              read<std::uint32_t>(&state, 0x46C + std::size_t(i) * 0x1C));
    const auto saved = capture_step_fields(player, bindings);
    auto current = saved;
    current.recoil = read<float>(&state, 0x150);
    current.duck = read<float>(&state, 0x104);
    if (read<std::uint32_t>(&state, 0x238)) {
        const auto sample = (read<std::uint32_t>(&state, 0x234) % 6u) * 0x20;
        current.flags = read<std::uint32_t>(&state, 0x250 + sample);
        current.absolute_origin = read<Vector>(&state, 0x240 + sample);
    }
    const auto ticks = animation_step_count(&state, bindings.entity.interval);
    std::int32_t jump_tick = -1, land_tick = -1, pre_shot_tick = -1, duck_full_tick = -1,
                 duck_zero_tick = -1;
    auto shot_cutoff = INT32_MAX;
    bool interpolate_duck = current.duck != saved.duck;
    const auto duck_delta = (saved.duck - current.duck) / float(ticks);
    const float inverse = 1.0f / float(ticks);
    const Vector velocity_delta{(saved.velocity[0] - current.velocity[0]) * inverse,
                                (saved.velocity[1] - current.velocity[1]) * inverse,
                                (saved.velocity[2] - current.velocity[2]) * inverse};
    if (shot_tick != -1 && (flags & 4u)) {
        if (bindings.shot_angles_active)
            shot_cutoff = shot_tick;
        else
            pre_shot_tick = shot_tick;
    }
    bool physical_air = false;
    const auto event_tick = [&](float cycle, float rate) {
        const auto age = truncate((cycle / rate) / bindings.entity.interval + 0.5f);
        return add(span.last, subtract(1, age));
    };
    if (!(saved.flags & 1u)) {
        const auto cycle = read<float>(&state, 0x4F0), old_cycle = read<float>(&state, 0x384);
        if (!((cycle == old_cycle &&
               read<std::int32_t>(&state, 0x4DC) == read<std::int32_t>(&state, 0x370)) ||
              old_cycle <= cycle || !bindings.velocity.classify)) {
            if (bindings.velocity.classify(player, read<std::int32_t>(&state, 0x4DC)) == 0x3D9) {
                jump_tick = event_tick(cycle, read<float>(&state, 0x4EC));
                const char* name =
                    bindings.velocity.sequence_name
                        ? bindings.velocity.sequence_name(player, read<std::int32_t>(&state, 0x4DC))
                        : "";
                const auto comparison = bindings.velocity.compare_sequence_name(
                    name, bindings.velocity.jump_sequence_name);
                if (comparison == 0 && saved.duck >= 1.0f)
                    duck_full_tick = add(jump_tick, 1);
                else if (comparison != 0 && !(saved.duck > 0.0f))
                    duck_zero_tick = add(jump_tick, 1);
                interpolate_duck = false;
            }
            physical_air = jump_tick != -1 && owner.history.count != 0;
        }
    } else if (owner.history.count && !(owner.history.at(0).header.player_flags & 1u)) {
        if (read<float>(&state, 0x500) != 0.0f && read<float>(&state, 0x394) == 0.0f &&
            bindings.velocity.classify) {
            const auto activity =
                bindings.velocity.classify(player, read<std::int32_t>(&state, 0x4F8));
            if (activity == 0x3DC || activity == 0x3DD) {
                land_tick = event_tick(read<float>(&state, 0x50C), read<float>(&state, 0x508));
                physical_air = land_tick != -1 && owner.history.count != 0;
            }
        }
    } else
        physical_air = jump_tick != -1 && owner.history.count != 0;
    bool override_animation =
        read<std::uint8_t>(player, bindings.fields.animation_override_00a5bd68) == 0 &&
        read<float>(&state, 0x538) > 0.5f &&
        read<float>(&state, 0x538) == read<float>(&state, 0x3CC);
    const auto first = subtract(span.last, ticks);
    write(animation_state, 0x6C, float(first) * bindings.entity.interval);
    auto vertical = current.velocity[2];
    for (auto tick = add(first, 1); tick < span.last; tick = add(tick, 1)) {
        bindings.animation_time = float(tick) * bindings.entity.interval;
        if (physical_air) {
            if (!(current.flags & 1u)) {
                const auto half_gravity =
                    convar(bindings.velocity.cvars.gravity) * bindings.entity.interval * 0.5f;
                const auto half_velocity = vertical - half_gravity;
                vertical = half_velocity - half_gravity;
                current.absolute_origin[2] =
                    half_velocity * bindings.entity.interval + current.absolute_origin[2];
                current.velocity[2] = vertical;
            } else {
                vertical = 0.0f;
                current.velocity[2] = 0.0f;
            }
        }
        if (tick == land_tick) {
            current.flags |= 1u;
            auto* live = read<void*>(descriptor, 0);
            write(live, 0x144, std::uint32_t(0));
            write(live, 0x140, read<std::uint32_t>(&state, 0x508));
            land_tick = -1;
        }
        if (tick == subtract(jump_tick, 1))
            current.flags |= 1u;
        if (tick == jump_tick) {
            const auto previous_flags = current.flags;
            current.flags &= ~1u;
            auto* live = read<void*>(descriptor, 0);
            write(live, 0x10C, std::uint32_t(0));
            write(live, 0x108, read<std::uint32_t>(&state, 0x4EC));
            if (physical_air) {
                const auto impulse = convar(bindings.velocity.cvars.jump_impulse);
                vertical = previous_flags & 2u ? impulse : vertical + impulse;
                current.absolute_origin[2] =
                    vertical * bindings.entity.interval + current.absolute_origin[2];
                vertical -=
                    convar(bindings.velocity.cvars.gravity) * bindings.entity.interval * 0.5f;
                current.velocity[2] = vertical;
            }
        }
        if (tick >= shot_cutoff) {
            current.angles[0] = read<float>(&state, 0x1A8);
            current.lower_body_yaw = read<float>(&state, 0x118);
            current.angles[1] = read<float>(&state, 0x1AC);
        } else if (tick == shot_tick && tick == pre_shot_tick) {
            const auto old_pitch =
                read<float>(&state, 0x1C4) -
                convar(bindings.recoil_scale_convar) * read<float>(&state, 0x150);
            const auto pitch_delta = read<float>(&state, 0x1A8) - old_pitch;
            bool replace = !std::isnan(pitch_delta) && absolute(normalize(pitch_delta)) > 10.0f;
            if (!replace) {
                const auto yaw_delta = read<float>(&state, 0x1AC) - read<float>(&state, 0x1BC);
                replace = !std::isnan(yaw_delta) && absolute(normalize(yaw_delta)) > 10.0f;
            }
            if (replace) {
                current.angles[0] = old_pitch;
                current.angles[1] = read<float>(&state, 0x1BC);
            }
        }
        if (tick == duck_full_tick || tick == duck_zero_tick)
            current.duck = saved.duck;
        if (interpolate_duck)
            current.duck = clamp_unit(duck_delta + current.duck);
        current.velocity[0] = velocity_delta[0] + current.velocity[0];
        current.velocity[1] = velocity_delta[1] + current.velocity[1];
        vertical = velocity_delta[2] + vertical;
        current.velocity[2] = vertical;
        publish_step_fields(player, current, bindings);
        if (override_animation)
            write(player, bindings.fields.animation_override_00a5bd68, std::uint8_t(1));
        animation::update_animation_state(animation_state, current.angles,
                                          {bindings.animation_frame, bindings.animation_time,
                                           bindings.entity.interval, bindings.animation_reset_byte},
                                          bindings.animation);
        if (override_animation) {
            override_animation = false;
            write(player, bindings.fields.animation_override_00a5bd68, std::uint8_t(0));
        }
        result.speed_fraction = read<float>(animation_state, 0x11C);
        if (tick == shot_tick)
            current.recoil = saved.recoil;
        current.lower_body_yaw = read<float>(&state, 0x118);
    }
    publish_step_fields(player, saved, bindings);
    result.animation_publish_layer_weight = read<float>(read<void*>(descriptor, 0), 0x138);
}

struct MotionAxis {
    double start, linear = 0, quadratic = 0, cubic = 0;

    MotionAxis(float from, float to, float initial_speed, float final_speed, double duration)
        : start(from) {
        const double displacement = double(to) - from;
        double first = initial_speed * duration, last = final_speed * duration;
        // Opposite endpoint velocities can describe a reversal. Keep both
        // slopes when the integrated constant acceleration agrees with the
        // observed displacement to within one sixteenth of a map unit.
        const bool reversal = first * last < 0.0 &&
                              std::fabs(displacement - 0.5 * (first + last)) <= 0.0625;
        if (!reversal) {
            if (displacement == 0.0)
                return;
            // These tangent limits make the Hermite segment monotone.
            first = std::clamp(first / displacement, 0.0, 3.0) * displacement;
            last = std::clamp(last / displacement, 0.0, 3.0) * displacement;
        }
        linear = first;
        quadratic = 3.0 * displacement - 2.0 * first - last;
        cubic = first + last - 2.0 * displacement;
    }
    float position(double t) const noexcept {
        return float(start + t * (linear + t * (quadratic + t * cubic)));
    }
    float speed(double t, double duration) const noexcept {
        return float((linear + t * (2.0 * quadratic + 3.0 * t * cubic)) / duration);
    }
};

constexpr unsigned jump_layer = 4, land_layer = 5;
constexpr std::uint32_t on_ground = 1u;
struct LayerTransition {
    int step = 0;
    bool jump = false;
};
LayerTransition layer_transition(const AnimationLayerState& previous,
                                 const AnimationLayerState& received, unsigned index,
                                 void* player, SimulationTickSpan span,
                                 const HistoryResolverUpdateBindings& bindings) {
    const auto cycle = std::bit_cast<float>(received.words[6]);
    const auto rate = std::bit_cast<float>(received.words[5]);
    const auto sequence = std::bit_cast<std::int32_t>(received.words[1]);
    if (!bindings.velocity.classify || sequence <= 0 || !std::isfinite(cycle) ||
        cycle < 0.0f || cycle >= 1.0f || !std::isfinite(rate) || rate <= 0.0f)
        return {};
    const auto old_cycle = std::bit_cast<float>(previous.words[6]);
    const auto old_rate = std::bit_cast<float>(previous.words[5]);
    if (previous.words[1] == received.words[1] && std::isfinite(old_cycle) &&
        std::isfinite(old_rate) && old_rate > 0.0f && cycle >= old_cycle)
        return {};
    const auto activity = bindings.velocity.classify(player, sequence);
    if (index == jump_layer ? activity != 0x3D9 && activity != 0x3DA
                            : activity != 0x3DC && activity != 0x3DD)
        return {};
    const double age = double(cycle) / rate / bindings.entity.interval;
    if (!(age < double(span.count) + 0.5))
        return {};
    const auto ticks = std::max(1, int(std::floor(age + 0.5)));
    return {span.count + 1 - ticks, activity == 0x3D9};
}

struct FlightSegment {
    int from, until;
    double height, launch_speed, gravity;
    float position(double time) const noexcept {
        return float(height + time * (launch_speed - 0.5 * gravity * time));
    }
    float speed(double time) const noexcept {
        return float(launch_speed - gravity * time);
    }
};
std::optional<FlightSegment> fit_flight(int ticks, bool grounded_before, bool grounded_after,
                                      LayerTransition takeoff, LayerTransition landing,
                                      const StepFields& start, const StepFields& end,
                                      const HistoryResolverUpdateBindings& bindings) {
    const int from = grounded_before ? takeoff.step - 1 : 0;
    const int until = grounded_after ? landing.step : ticks;
    const auto gravity_object = bindings.velocity.cvars.gravity.get();
    if (from < 0 || until <= from || !gravity_object)
        return {};
    const double gravity = convar(gravity_object);
    const double interval = bindings.entity.interval;
    if (!std::isfinite(gravity) || gravity < 0.0)
        return {};
    const double duration = (until - from) * interval;
    const double displacement = double(end.absolute_origin[2]) - start.absolute_origin[2];
    const double launch = displacement / duration + 0.5 * gravity * duration;
    const double arrival = launch - gravity * duration;
    // Endpoint velocities are quantized and event cycles resolve to whole ticks.
    const double tolerance = std::max(8.0, 2.0 * gravity * interval);
    if ((!grounded_before && std::fabs(launch - start.velocity[2]) > tolerance) ||
        (!grounded_after && std::fabs(arrival - end.velocity[2]) > tolerance) ||
        (grounded_after && arrival > 0.0))
        return {};
    if (grounded_before && takeoff.jump) {
        const auto impulse_object = bindings.velocity.cvars.jump_impulse.get();
        if (!impulse_object)
            return {};
        const auto impulse = convar(impulse_object);
        if (!std::isfinite(impulse) || impulse <= 0.0f || launch <= 0.0 ||
            launch > impulse + tolerance)
            return {};
    } else if (grounded_before && std::fabs(launch) > tolerance) {
        return {};
    }
    return FlightSegment{from, until, start.absolute_origin[2], launch, gravity};
}

bool finite_vector(const Vector& value) noexcept {
    return std::ranges::all_of(value, [](float component) { return std::isfinite(component); });
}
Vector packet_origin(const PacketTimingSample& packet) noexcept {
    return {std::bit_cast<float>(packet.fields_04[0]), std::bit_cast<float>(packet.fields_04[1]),
            std::bit_cast<float>(packet.fields_04[2])};
}
void begin_replayed_layer(std::byte* layers, unsigned index,
                         const AnimationLayerState& received) noexcept {
    auto* layer = layers + index * 0x38;
    write(layer, 0x18, received.words[1]);
    write(layer, 0x28, received.words[5]);
    write(layer, 0x2C, std::uint32_t(0));
}

void rebuild_history_animation(client::PlayerState& owner, void* player, void* state,
                               SimulationTickSpan span, std::int32_t shot_tick, std::uint32_t flags,
                               HistoryResolverUpdateResult& result,
                               const HistoryResolverUpdateBindings& b, float previous_yaw) {
    const double interval = b.entity.interval;
    const auto ticks = span.count;
    if (!b.animation.update_state || ticks < 2 || ticks > 16 || !std::isfinite(interval) ||
        interval <= 0.0 || span.last <= span.first || std::int64_t(span.last) - span.first != ticks)
        return;
    const auto timeline = history_timeline(owner);
    if (timeline.packet_count < 2 || timeline.packet_count > timeline.packets.size())
        return;
    const auto& received = timeline.packets[timeline.packet_head % timeline.packets.size()];
    const auto& previous = timeline.packets[(timeline.packet_head + 1u) % timeline.packets.size()];
    if (previous.field_00 != span.first || received.field_00 != span.last)
        return;
    const auto first_time = float(span.first) * b.entity.interval;
    const auto last_update = read<float>(state, 0x6C);
    if (!std::isfinite(last_update) || std::fabs(last_update - first_time) > interval ||
        !(float(span.first + 1) * b.entity.interval > first_time))
        return;
    const auto& native = owner.native;
    const auto saved = capture_step_fields(player, b);
    auto start = saved;
    auto end = saved;
    start.absolute_origin = packet_origin(previous);
    end.absolute_origin = packet_origin(received);
    start.flags = previous.fields_04[4];
    end.flags = received.fields_04[4];
    start.velocity = read<Vector>(&native, 0x120);
    start.duck = read<float>(&native, 0x104);
    start.recoil = read<float>(&native, 0x150);
    start.lower_body_yaw = read<float>(&native, 0x184);
    start.angles[0] = read<float>(&native, 0x188);
    start.angles[1] = read<float>(&native, 0x114);
    if (!finite_vector(start.absolute_origin) || !finite_vector(end.absolute_origin) ||
        !finite_vector(start.velocity) || !finite_vector(end.velocity) ||
        !finite_vector(start.angles) || !finite_vector(end.angles) ||
        !std::isfinite(start.duck) || !std::isfinite(end.duck) || !std::isfinite(start.recoil) ||
        !std::isfinite(start.lower_body_yaw) || !std::isfinite(previous_yaw))
        return;
    double distance_squared = 0.0;
    for (unsigned axis = 0; axis < 3; ++axis) {
        const auto delta = double(end.absolute_origin[axis]) - start.absolute_origin[axis];
        distance_squared += delta * delta;
    }
    if (distance_squared > 4096.0 || ((saved.flags ^ end.flags) & on_ground))
        return;
    const auto* descriptor = static_cast<const std::byte*>(player) + b.entity.offsets.animation_layers;
    auto* layers = read<std::byte*>(descriptor, 0);
    const auto layer_count = read<std::int32_t>(descriptor, 0xC);
    if (!layers || layer_count <= int(land_layer) || layer_count > 13)
        return;
    const auto old_layers = read<std::array<AnimationLayerState, 13>>(&native, 0x2FC);
    const auto new_layers = read<std::array<AnimationLayerState, 13>>(&native, 0x468);
    const bool grounded_before = (start.flags & on_ground) != 0;
    const bool grounded_after = (end.flags & on_ground) != 0;
    const auto takeoff = layer_transition(old_layers[jump_layer], new_layers[jump_layer],
                                          jump_layer, player, span, b);
    const auto landing = layer_transition(old_layers[land_layer], new_layers[land_layer],
                                          land_layer, player, span, b);
    const bool complete_hop = grounded_before && grounded_after && takeoff.step > 0 &&
                              landing.step > takeoff.step;
    std::optional<FlightSegment> flight;
    if (!grounded_before || !grounded_after || complete_hop) {
        if ((!grounded_before && !grounded_after && landing.step > 0) ||
            (grounded_before && !takeoff.step) || (grounded_after && !landing.step))
            return;
        flight = fit_flight(ticks, grounded_before, grounded_after, takeoff, landing, start, end, b);
        if (!flight)
            return;
    }
    const double duration = ticks * interval;
    const MotionAxis x(start.absolute_origin[0], end.absolute_origin[0],
                       start.velocity[0], end.velocity[0], duration);
    const MotionAxis y(start.absolute_origin[1], end.absolute_origin[1],
                       start.velocity[1], end.velocity[1], duration);
    const MotionAxis ground_z(start.absolute_origin[2], end.absolute_origin[2], 0, 0, duration);
    const auto abs_velocity = b.fields.absolute_velocity_7a2d60c1;
    const auto saved_abs_velocity = abs_velocity ? read<Vector>(player, abs_velocity) : Vector{};
    const auto network_origin = b.entity.offsets.origin;
    const auto saved_network_origin = network_origin ? read<Vector>(player, network_origin) : Vector{};
    const auto selected_yaw = read<float>(state, 0x80);
    for (int i = 0; i < layer_count; ++i)
        std::memcpy(layers + i * 0x38 + 0x14, &old_layers[i], sizeof(AnimationLayerState));
    constexpr std::size_t animation_offsets[]{0x98, 0x19C, 0x190, 0x198, 0x180};
    constexpr std::size_t owner_offsets[]{0x3BC, 0x3C4, 0x3CC, 0x3D8, 0x458};
    for (unsigned i = 0; i < std::size(animation_offsets); ++i)
        write(state, animation_offsets[i], read<std::uint32_t>(&native, owner_offsets[i]));
    write(state, 0x80, previous_yaw);
    write(state, 0x6C, first_time);
    auto current = start;
    for (int step = 1; step < ticks; ++step) {
        const auto tick = span.first + step;
        const double t = double(step) / ticks;
        const bool airborne = flight && step > flight->from &&
                              (step < flight->until || !grounded_after);
        current.flags = (current.flags & ~on_ground) | (airborne ? 0u : on_ground);
        current.duck = clamp_unit(std::lerp(start.duck, end.duck, float(t)));
        current.absolute_origin = {x.position(t), y.position(t), ground_z.position(t)};
        current.velocity = {x.speed(t, duration), y.speed(t, duration), 0.0f};
        if (flight) {
            if (airborne) {
                const double air_time = (step - flight->from) * interval;
                current.absolute_origin[2] = flight->position(air_time);
                current.velocity[2] = flight->speed(air_time);
            } else {
                current.absolute_origin[2] = step <= flight->from ? start.absolute_origin[2]
                                                               : end.absolute_origin[2];
            }
            if (grounded_before && step == takeoff.step)
                begin_replayed_layer(layers, jump_layer, new_layers[jump_layer]);
            if (grounded_after && step == landing.step)
                begin_replayed_layer(layers, land_layer, new_layers[land_layer]);
        }
        if ((flags & 4u) && tick == shot_tick) {
            current.recoil = saved.recoil;
            current.lower_body_yaw = saved.lower_body_yaw;
            if (b.shot_angles_active)
                current.angles = saved.angles;
        }
        publish_step_fields(player, current, b);
        if (network_origin)
            write(player, network_origin, current.absolute_origin);
        if (abs_velocity)
            write(player, abs_velocity, current.velocity);
        b.animation_time = float(tick) * b.entity.interval;
        animation::update_animation_state(state, current.angles,
            {b.animation_frame, b.animation_time, b.entity.interval, b.animation_reset_byte},
            b.animation);
    }
    if (selected_yaw != previous_yaw)
        write(state, 0x80, selected_yaw);
    publish_step_fields(player, saved, b);
    if (network_origin)
        write(player, network_origin, saved_network_origin);
    if (abs_velocity)
        write(player, abs_velocity, saved_abs_velocity);
    result.speed_fraction = read<float>(state, 0x11C);
    result.animation_publish_layer_weight = read<float>(layers, 0x138);
}

void step_history_animation(client::PlayerState& owner, void* player, void* state,
                            SimulationTickSpan span, std::int32_t shot_tick, std::uint32_t flags,
                            HistoryResolverUpdateResult& result,
                            const HistoryResolverUpdateBindings& bindings, float previous_yaw) {
    if (bindings.configuration.reworked())
        rebuild_history_animation(owner, player, state, span, shot_tick, flags, result, bindings,
                                  previous_yaw);
    else
        step_stock_animation(owner, player, state, span, shot_tick, flags, result, bindings);
}
} // namespace

HistoryResolverUpdateResult
update_history_resolver(client::PlayerState& owner, void* player, void* animation_state,
                        const HistoryAnimationSeed& seed, SimulationTickSpan span,
                        std::int32_t shot_tick, std::uint32_t& flags,
                        const HistoryResolverInput& input, HistoryResolverScratch& scratch,
                        const HistoryResolverUpdateBindings& bindings) {
    HistoryResolverUpdateResult result{read<float>(&owner.native, 0x394),
                                       input.initial_speed_fraction};
    const bool resolve = *bindings.configuration.resolver_enabled != 0;
    const bool rebuild = bindings.configuration.reworked();
    const auto angles = bindings.entity.offsets.eye_angles;
    const auto saved_yaw = read<float>(player, angles + 4);
    const auto saved_lower_body = read<float>(player, bindings.entity.offsets.field_a2a8247f);
    const auto previous_yaw = read<float>(animation_state, 0x80);
    if (resolve) {
        choose_resolver_yaw(owner, player, animation_state, span, shot_tick, flags, input, scratch,
                            bindings);
        const auto yaw = read<float>(player, angles + 4);
        if (saved_yaw != yaw)
            write(player, angles + 4, normalize(yaw));
    }
    if ((resolve || rebuild) && !(flags & 1u) && span.count > 1)
        step_history_animation(owner, player, animation_state, span, shot_tick, flags, result,
                               bindings, previous_yaw);
    if (resolve) {
        std::optional<std::int32_t> air_age;
        if (seed.active_air_layer && (!rebuild || (flags & 1u))) {
            const auto rate = read<float>(&owner.native, 0x4EC);
            const auto cycle = read<float>(&owner.native, 0x4F0);
            const auto interval = bindings.entity.interval;
            if (!rebuild)
                air_age = truncate((cycle / rate) / interval + 0.5f);
            else if (std::isfinite(rate) && rate > 0.0f && std::isfinite(interval) &&
                     interval > 0.0f && cycle >= 0.0f && cycle < 1.0f) {
                const double ticks = double(cycle) / rate / interval;
                if (ticks + 0.5 < double(INT32_MAX))
                    air_age = std::int32_t(ticks + 0.5);
            }
        }
        if (air_age) {
            const auto age = *air_age;
            const auto age_time = float(age) * bindings.entity.interval;
            const auto start = float(span.last) * bindings.entity.interval - age_time;
            if ((flags & 1u) && seed.previous_update_time < start) {
                write(animation_state, 0x108, std::uint16_t(0));
                write(player, bindings.fields.pose_parameters_1caaf0a8 + 0x18, std::uint32_t(0));
                write(animation_state, 0x110, std::uint32_t(0));
                write(animation_state, 0x6C, start);
            }
            write(animation_state, 0x110, age_time - bindings.entity.interval);
        }
        write(player, bindings.entity.offsets.field_a2a8247f, saved_lower_body);
        write(player, angles + 4, saved_yaw);
    }
    write(player, bindings.entity.offsets.velocity, seed.velocity);
    return result;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_runtime.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
HistoryRecordingRuntime* installed_runtime;
/* [removed duplicate read] */
/* [removed duplicate write] */
/* [removed duplicate subtract] */
std::uint32_t convar_integer(const void* object) noexcept {
    for (;;) {
        const auto* parent = read<const void*>(object, 0x1C);
        if (parent == object)
            break;
        object = parent;
    }
    return read<std::uint32_t>(object, 0x30) ^
           std::uint32_t(reinterpret_cast<std::uintptr_t>(object));
}
void clear_packets(client::PlayerState& owner) noexcept {
    write(&owner.native, 0x234, std::uint32_t(0));
    owner.native.field_238 = 0;
}
void set_absolute_origin(void* player, const Vector& value, const EntityStateRuntime& entity) {
    if (!entity.set_absolute_origin)
        return;
    const auto offset = entity.offsets.transform_guard_49d93e62 - 0x14u;
    const auto saved = read<std::uint32_t>(player, offset);
    write(player, offset, UINT32_MAX);
    entity.set_absolute_origin(player, value);
    write(player, offset, saved);
}
float maximum_yaw(const void* state) noexcept {
    if (!state)
        return 58.0f;
    const anti_aim::AnimationYawInput input{read<float>(state, 0xA4),  read<float>(state, 0xF8),
                                            read<float>(state, 0xFC),  read<float>(state, 0x11C),
                                            read<float>(state, 0x334), read<float>(state, 0x338)};
    float minimum, maximum;
    anti_aim::animation_yaw_limits(input, minimum, maximum);
    return maximum;
}
struct LastLayerRestore {
    std::byte* target = nullptr;
    std::uint32_t saved;
    LastLayerRestore(void* player, const EntityNetvars& offsets) {
        const auto* descriptor = static_cast<const std::byte*>(player) + offsets.animation_layers;
        if (read<std::int32_t>(descriptor, 0xC) > 12) {
            target = read<std::byte*>(descriptor, 0) + 0x2C0;
            saved = read<std::uint32_t>(target, 0);
            write(target, 0, std::uint32_t(0));
        }
    }
    ~LastLayerRestore() {
        if (target)
            write(target, 0, saved);
    }
};
void publish_live_fallback(client::PlayerState& owner, bool captured,
                           const HistoryRecordingBindings& bindings) {
    if (!owner.history.count ||
        owner.fallback.header.receive_tick <= owner.history.at(0).header.receive_tick)
        return;
    const auto& entity = bindings.entity;
    const auto receive_tick = sample_receive_tick(entity.engine, entity.receive_interval);
    // Native1677E reloads the current ring head after engine14;167AD reads
    // the live maximum-unlag word. No pre-call recording-state copy publishes
    // over changes made at that genuine engine boundary.
    auto& newest = owner.history.at(0);
    if (!captured) {
        const float seconds =
            float(receive_tick) * entity.interval - float(bindings.maximum_unlag_ms) * 0.001f;
        const auto truncated = _mm_cvtt_ss2si(_mm_set_ss(seconds));
        if (float(truncated) > float(newest.header.simulation_tick) * entity.interval) {
            std::memcpy(&newest, &owner.fallback, sizeof(LagRecord));
            owner.jitter.remember(owner.jitter.fallback);
            owner.fallback.header.receive_tick = -1;
            return;
        }
    }
    owner.native.flags |= 0x100u;
}
}
void bind_history_recording_runtime(HistoryRecordingRuntime& runtime) noexcept {
    installed_runtime = &runtime;
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall execute_history_record_job(void* item) {
#else
void execute_history_record_job(void* item) {
#endif
    installed_runtime->execute_job(*static_cast<HistoryJobItem*>(item));
}
void HistoryRecordingRuntime::execute_job(const HistoryJobItem& item) {
    auto* player = reinterpret_cast<void*>(std::uintptr_t(item.player));
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    auto& owner = *bindings_.players.get(index);
    auto& record = *reinterpret_cast<LagRecord*>(std::uintptr_t(item.record));
    build_record(owner, player, item.speed_fraction, record, true);
}
bool HistoryRecordingRuntime::hypotheses_enabled(void* player) const {
    const auto& b = bindings_;
    if (!*b.configuration.recording_enabled)
        return false;
    if (convar_integer(b.hypotheses.game_type) == 6u) {
        const auto handle = std::uint32_t(b.hypotheses.selected_handle);
        if (handle == UINT32_MAX ||
            engine::invoke<void*>(b.entity.entity_list, 4, handle) != player)
            return false;
    }
    if (*b.configuration.resolver_enabled)
        return true;
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    const auto safe_point = read<std::uint32_t>(b.globals.adjustments.settings_base(),
                                                std::size_t(index) * 0x20 + 0x14);
    if (safe_point)
        return safe_point == 2u;
    if (hotkey_active(b.hypotheses.hotkey, b.hypotheses.keys))
        return true;
    if (std::uint32_t(*b.configuration.target_hitboxes) & b.hypotheses.active_mode_mask)
        return true;
    return b.hypotheses.physical_override && (b.hypotheses.input_flags & 8u);
}
void HistoryRecordingRuntime::build_live_hypotheses(void* player, client::PlayerState& owner,
                                                    float fraction, LagRecord& record) {
    const auto& b = bindings_;
    auto input = capture_record_hypothesis_input(player, owner, record, b.entity,
                                                  b.configuration.reworked());
    input.speed_fraction = fraction;
    resolver_runtime::build_hypotheses(record, input, player, b.bones, b.configuration.reworked());
}
void HistoryRecordingRuntime::build_record(client::PlayerState& owner, void* player,
                                           const float& fraction, LagRecord& record,
                                           bool deferred) {
    const auto& b = bindings_;
    if (deferred) {
        const auto offset = b.bones.fields.entity_flags_83a6cf2b;
        write(player, offset, read<std::uint32_t>(player, offset) | 8u);
        write(player, b.bones.fields.ik_mask_01d0bacc, UINT32_MAX);
    } else
        engine::invoke<const Vector*>(player, 11);
    if (b.configuration.reworked())
        record.header.flags &= ~(4u | 0x80u);
    std::int32_t count;
    if (b.configuration.reworked()) {
        std::array<Matrix3x4, animation::maximum_bones> scratch;
        count = animation::build_captured_hitbox_matrices(player, record, b.bones, scratch,
                                                         record.matrices, 0x80300u);
    } else
        count = b.bones.build_resolver_hitbox_matrices(player, record.header.animation_yaw,
                                                       record.matrices);
    if (count > 0 && (!b.configuration.reworked() ||
                      resolver_runtime::complete_hitbox_matrices(count, record.matrices)))
        record.header.flags |= 4u;
    if (hypotheses_enabled(player))
        build_live_hypotheses(player, owner, fraction, record);
    if (deferred) {
        const auto offset = b.bones.fields.entity_flags_83a6cf2b;
        write(player, offset, read<std::uint32_t>(player, offset) & ~8u);
    }
}
void HistoryRecordingRuntime::publish_record(client::PlayerState& owner, void* player,
                                             const CachedHistoryFrame& frame, bool active,
                                             bool enemy, bool forced_yaw, bool changed, std::uint32_t flags,
                                             float fraction, HistoryJobQueue& queue) {
    const auto& b = bindings_;
    auto state = owner.recording_state();
    const auto slot =
        prepare_history_capture(state, b.records, active, !enemy, frame.ticks.last, changed);
    owner.recording_state(state);
    if (!active || !enemy)
        return;
    const bool fallback = slot.kind == HistoryCaptureKind::fallback;
    auto* record = fallback ? &owner.fallback : slot.record;
    if (record) {
        LastLayerRestore layer(player, b.entity.offsets);
        b.entity.capture_record(*record, player, b.players);
        if (fallback)
            owner.jitter.fallback = {};
        if (b.configuration.jitter_fix || forced_yaw) {
            auto evidence = owner.jitter.current;
            if (evidence.tick != record->header.simulation_tick)
                evidence = {};
            evidence.tick = record->header.simulation_tick;
            evidence.receive_tick = record->header.receive_tick;
            evidence.observed_yaw = std::bit_cast<float>(record->header.player_history_1ac);
            if (forced_yaw)
                evidence.decision = resolver_runtime::JitterDecision::forced_yaw;
            if ((flags & 0x20u) && record->header.resolver_state == 2 && evidence.applied())
                evidence.decision = resolver_runtime::JitterDecision::movement;
            if (evidence.applied() && evidence.estimate.attempt != record->header.resolver_mode)
                evidence.decision = resolver_runtime::JitterDecision::not_applied;
            if (fallback)
                owner.jitter.fallback = evidence;
            else
                owner.jitter.remember(evidence);
        }
        const auto* previous = owner.history.count > 1 ? &owner.history.at(1) : nullptr;
        publish_record_flags(*record, previous, *b.configuration.recording_enabled, flags,
                             b.globals.shot_angles_active);
        if (!append_history_job(queue, b.globals.settings_menu.multithreading, player, fraction,
                                *record))
            build_record(owner, player, fraction, *record, false);
    }
    publish_live_fallback(owner, fallback, b);
    if (b.configuration.jitter_enabled() && !forced_yaw)
        resolver_runtime::correct_jitter_history(owner.history, owner.jitter);
}
void HistoryRecordingRuntime::record_players(void* local) {
    // Native4340C971 allocates both direction regions on this invocation's
    // stack, reusing them across its player loop, never as process state.
    HistoryRecordingScratch scratch;
    record_players(local, scratch);
}
void HistoryRecordingRuntime::record_players(void* local, HistoryRecordingScratch& scratch) {
    const auto& b = bindings_;
    const auto& entity = b.entity;
    auto* const player_table = b.globals.adjustments.settings_base();
    if (b.globals.animation_reset)
        *b.globals.animation_reset = 0;
    b.configuration.prediction_available = std::uint8_t(history_prediction_available(b.activation));
    bool active = false;
    if (!engine::invoke<bool>(entity.engine, 82) && !engine::invoke<bool>(entity.engine, 93))
        active = *b.activation.recording_enabled || history_configuration_active(b.activation);
    const auto local_index = engine::invoke<std::int32_t>(static_cast<std::byte*>(local) + 8, 10);
    const auto maximum = b.globals.maximum_players;
    HistoryJobQueue queue;
    initialize_history_job_queue(queue);
    for (std::int32_t index = 1; index <= maximum;
         index = std::bit_cast<std::int32_t>(std::uint32_t(index) + 1u)) {
        auto& owner = *b.players.get(std::uint32_t(index));
        auto* player = engine::invoke<void*>(entity.entity_list, 3, index);
        prune_history_records(owner.history, b.records, b.globals.history_clock, entity.interval);
        owner.publish_history();
        auto timeline = history_timeline(owner);
        prune_packet_samples(timeline, b.globals.history_clock, entity.interval);
        owner.native.field_238 = timeline.packet_count;
        std::array<std::byte, 0x158> information;
        if (!player || !engine::invoke<bool>(player, 158) ||
            !engine::invoke<bool>(b.configuration.engine, 8, index, information.data()) ||
            std::to_integer<unsigned>(information[0x13D]) ||
            engine::invoke<bool>(static_cast<std::byte*>(player) + 8, 9)) {
            clear_packets(owner);
            continue;
        }
        auto frame = prepare_cached_history_frame(owner, b.records, player, index == local_index,
                                                  entity, b.prelude);
        if (!frame.ready)
            continue;
        const auto max_yaw = maximum_yaw(frame.animation_state);
        const auto old_index = owner.native.resolver_index;
        auto receive = sample_receive_tick(entity.engine, entity.receive_interval);
        std::uint32_t flags = read<std::int32_t>(&owner.native, 0x1A4) == receive ||
                              (read<std::uint32_t>(player, b.fields.effects_38ca5228) & 8u) ||
                              read<std::uint8_t>(player, b.fields.field_043fa575) !=
                                  read<std::uint8_t>(player, b.fields.field_334b460c);
        receive = sample_receive_tick(entity.engine, entity.receive_interval);
        if (read<std::int32_t>(&owner.native, 0x1DC) == receive)
            flags |= 8u;
        std::int32_t shot = -1;
        bool accepted_shot = false;
        if (frame.ticks.count > 0) {
            owner.native.flags =
                (owner.native.flags & ~0x10u) | ((owner.native.flags >> 1) & 0x10u);
            const auto handle = read<std::uint32_t>(player, entity.offsets.weapon_handle);
            auto* weapon = handle == UINT32_MAX
                               ? nullptr
                               : engine::invoke<void*>(entity.entity_list, 4, handle);
            if (weapon) {
                const auto tick = time_to_tick(
                    read<float>(weapon, b.fields.weapon_shot_time_0176a381), entity.interval);
                owner.native.flags = (owner.native.flags & ~1u) |
                                     std::uint32_t(tick == subtract(frame.ticks.last, 1) ||
                                                   tick == frame.ticks.last);
                if (history_weapon_is_firearm(
                        read<std::uint16_t>(weapon, b.prelude.weapon_definition_5c9f0fbb)) &&
                    tick > frame.ticks.first && tick <= frame.ticks.last) {
                    owner.native.flags |= 0x20u;
                    flags |= 4u;
                    shot = tick;
                    accepted_shot = true;
                }
            }
            if (!accepted_shot)
                owner.native.flags &= ~0x20u;
        }
        if (read<std::int32_t>(&owner.native, 0x1D4) > 1 && frame.ticks.count) {
            const auto height = engine::invoke<const Vector*>(player, 77)->at(2);
            if (read<float>(&owner.native, 0x11C) != height) {
                write(&owner.native, 0x12C,
                      read<float>(player, entity.offsets.origin + 8) +
                          read<float>(&owner.native, 0x11C));
                write(&owner.native, 0x148,
                      read<float>(player, b.prelude.previous_time_1599541f) + entity.interval);
            }
        }
        if (flags & 1u)
            clear_packets(owner);
        //4340D50C supplies this invocation's saved local player to4339EF8C.
        const rage::TargetBindings targeting{local,
                                             entity.entity_list,
                                             b.targeting.game_type,
                                             b.targeting.teammates_are_enemies,
                                             b.globals.adjustments,
                                             b.targeting.offsets};
        const bool enemy = rage::target_is_enemy(targeting, player);
        const auto current_layers = read<std::array<AnimationLayerState, 13>>(&owner.native, 0x2FC);
        const auto previous_layers =
            read<std::array<AnimationLayerState, 13>>(&owner.native, 0x468);
        const bool changed = history_animation_layers_changed(current_layers, previous_layers);
        const auto animation_enabled =
            read<std::uint8_t>(player, b.fields.animation_enabled_8481ea04);
        const auto cached_enabled = read<std::uint8_t>(&owner.native, 0x17C);
        const auto previous_angles = read<Vector>(player, entity.offsets.eye_angles);
        const auto table_offset = std::uint32_t(index) << 5;
        owner.jitter.current = {};
        if (enemy && b.configuration.jitter_fix) {
            auto& evidence = owner.jitter.current;
            evidence.tick = frame.ticks.last;
            evidence.observed_yaw = owner.native.reference_yaw;
            if (!b.configuration.jitter_enabled())
                evidence.decision = resolver_runtime::JitterDecision::correction_disabled;
        }
        if (enemy && frame.ticks.count) {
            if (owner.history.count) {
                const auto tick = time_to_tick(read<float>(&owner.native, 0x110), entity.interval);
                if (tick < subtract(read<std::int32_t>(&owner.native, 0x168), 1) &&
                    tick < owner.history.at(0).header.simulation_tick) {
                    owner.native.flags |= 0x200u;
                    write(&owner.native, 0x1D8,
                          sample_receive_tick(entity.engine, entity.receive_interval));
                }
            }
            ResolverDetectionState detection{bool(player_table[table_offset + 6]),
                                             read<float>(&owner.native, 0x1C0),
                                             read<float>(&owner.native, 0x178),
                                             read<std::uint16_t>(&owner.native, 0xA6),
                                             {}};
            for (unsigned mode = 0; mode < 4; ++mode)
                detection.selection.attempt_by_mode[mode] =
                    owner.native.resolver_slots[mode].attempt;
            detection.selection.status = owner.native.resolver_status;
            const auto packets = history_timeline(owner);
            update_resolver_detection(
                detection, {frame.ticks.count, entity.interval, read<float>(&owner.native, 0x118),
                            owner.native.reference_yaw,
                            read<float>(player, entity.offsets.eye_angles), packets});
            player_table[table_offset + 6] = std::uint8_t(detection.enabled);
            write(&owner.native, 0x1C0, detection.yaw_duration_1c0);
            write(&owner.native, 0x178, detection.pitch_duration_178);
            for (unsigned mode = 0; mode < 4; ++mode)
                owner.native.resolver_slots[mode].attempt =
                    detection.selection.attempt_by_mode[mode];
            owner.native.resolver_status = detection.selection.status;
            if (detection.enabled)
                update_history_environment(
                    owner, player, scratch.environment,
                    {b.globals.local_origin, frame.ticks.count,
                     std::uint32_t(reinterpret_cast<std::uintptr_t>(&owner.native) + 0x5D4),
                     b.globals.current_time},
                    b.environment);
        }
        if (enemy)
            publish_history_eye_angles(owner, player, frame.ticks.count != 0,
                                       {bool(player_table[table_offset + 4]),
                                        read<std::int32_t>(player_table, table_offset + 0xC)},
                                       entity);
        capture_history_packet(owner, changed, player, entity,
                               b.velocity.reconstruction.fields.player_flags);
        if (index == local_index) {
            clear_history_records(owner.history, b.records);
            owner.publish_history();
            continue;
        }
        float fraction = frame.animation_speed_fraction;
        if (animation_enabled != cached_enabled && changed) {
            const auto saved_origin = *engine::invoke<const Vector*>(player, 10);
            set_absolute_origin(player, read<Vector>(player, entity.offsets.origin), entity);
            update_history_yaw_side(owner, enemy, flags, b.globals.transition_enabled,
                                    b.yaw_side_fmod);
            const auto& publication_source = b.publication;
            const HistoryAnimationPublishBindings publication{
                entity,
                publication_source.fields,
                entity.engine,
                publication_source.local_time_entity,
                &b.globals.current_time,
                entity.receive_interval,
                b.globals.animation_reset,
                b.configuration,
                player_table,
                table_offset,
                publication_source.allow_zero_sequence,
                publication_source.movement_exponent,
                publication_source.candidate_attempts,
                publication_source.movement,
                publication_source.power,
                publication_source.arccosine,
                publication_source.atan2};
            if (!frame.animation_state) {
                const auto previous =
                    read<std::uint8_t>(player, b.fields.animation_enabled_8481ea04);
                write(player, b.fields.animation_enabled_8481ea04,
                      read<std::uint8_t>(&owner.native, 0x17C));
                engine::invoke<void>(player, 224);
                if (b.globals.animation_reset)
                    *b.globals.animation_reset = 0;
                write(player, b.fields.animation_enabled_8481ea04, previous);
                publish_history_animation_origin(owner, player, saved_origin, publication);
            } else {
                const HistoryAnimationPublishSaved saved{
                    &b.globals.animation_time, b.globals.animation_time, &b.globals.frame_time,
                    b.globals.frame_time, saved_origin};
                b.globals.frame_time = entity.interval;
                float layer_weight = read<float>(&owner.native, 0x394);
                if (enemy) {
                    update_history_packet_direction(owner, flags, frame.ticks.last,
                                                    entity.interval);
                    const auto& p = b.velocity.reconstruction;
                    const HistoryVelocityReconstructionBindings physics{b.players,
                                                                        p.trace,
                                                                        entity.entity_list,
                                                                        entity.interval,
                                                                        p.fields,
                                                                        p.cvars,
                                                                        p.classify,
                                                                        p.sequence_name,
                                                                        p.compare_sequence_name,
                                                                        p.jump_sequence_name,
                                                                        p.atan2,
                                                                        p.sine,
                                                                        p.cosine,
                                                                        p.trace_flag,
                                                                        flags};
                    capture_history_velocity(owner, player,
                                             {entity, physics, b.velocity.maximum_velocity_convar,
                                              b.velocity.landing_limit_convar});
                    const auto seed = prepare_history_animation(
                        owner, player, frame.animation_state, frame.ticks.count, flags, entity,
                        p.fields.player_flags, b.configuration.reworked());
                    const auto& resolver_source = b.resolver;
                    const HistoryResolverUpdateBindings resolver{
                        entity,
                        b.configuration,
                        player_table,
                        b.hypotheses.selected_handle,
                        resolver_source.local_time_entity,
                        b.globals.current_time,
                        b.globals.animation_time,
                        b.globals.animation_frame,
                        b.globals.shot_angles_active,
                        b.globals.animation_reset,
                        b.globals.local_origin,
                        resolver_source.pending_shots,
                        resolver_source.fields,
                        resolver_source.recoil_scale_convar,
                        physics,
                        resolver_source.animation,
                        resolver_source.environment};
                    const auto result = update_history_resolver(
                        owner, player, frame.animation_state, seed, frame.ticks, shot, flags,
                        {old_index, max_yaw, fraction, table_offset}, scratch.resolver, resolver);
                    layer_weight = result.animation_publish_layer_weight;
                    fraction = result.speed_fraction;
                }
                publish_history_animation(owner, player, frame.animation_state, frame.ticks.last,
                                          frame.ticks.count, fraction, layer_weight, flags, saved,
                                          publication);
            }
        }
        publish_record(owner, player, frame, active, enemy, player_table[table_offset + 5] != 0,
                       changed, flags, fraction, queue);
        publish_history_final_angles(player, previous_angles, entity,
                                     b.fields.angle_interpolation_fac21eaf);
    }
    dispatch_history_jobs(queue, b.jobs);
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_selection.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate subtract] */
}
bool select_target_history_record(client::CommandFrame& frame, void* player,
                                  std::int32_t requested_tick, std::uint32_t options,
                                  LagRecord& destination, std::uint32_t& output_flags,
                                  const HistorySelectionBindings& bindings,
                                  const LagRecord** selected_source) {
    const auto& extrapolation = bindings.extrapolation;
    const auto& entity = extrapolation.preparation.entity;
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    auto& owner = *extrapolation.players.get(index);
    if (selected_source)
        *selected_source = nullptr;
    rage::TargetLagRecordHistory traversal{owner.native.cached_origin, owner.history.first,
                                           owner.history.count, owner.history.records};
    const auto choice = rage::prepare_target_record_copy(traversal, requested_tick, entity.interval,
                                                         options, destination, output_flags);
    if (!choice.source)
        return false;
    if (extrapolation.validate_record &&
        (!record_geometry_valid(*choice.source) ||
         record_precedes_discontinuity(owner.history, choice.source->header.simulation_tick)))
        return false;
    const auto plan =
        plan_history_extrapolation(owner.history, choice.ordinal, options, entity.interval,
                                   extrapolation.prediction_limit.value());
    const HistoryTickTiming timing{bindings.current_time,
                                   read<float>(&frame, 0x198),
                                   bindings.interpolation,
                                   entity.interval,
                                   bindings.maximum_unlag_ms,
                                   read<std::int32_t>(&frame, 0xD4),
                                   std::bit_cast<std::int32_t>(frame.field_18c)};
    const auto window = history_tick_window(timing);
    auto effective_tick = requested_tick;
    if (const auto* older = history_interpolation_source(
            owner.history, choice.ordinal, *choice.source, requested_tick, plan.ticks, window)) {
        interpolate_history_record(
            destination, *choice.source, *older,
            float(requested_tick) * entity.interval,
            entity.interval, bindings.matrix_scratch, output_flags, bindings.rotation,
            bindings.use_captured_endpoint_pose());
        effective_tick = window.last;
    }
    const bool had_alternatives = (choice.source->header.flags & 0x80u) != 0;
    if ((options & 1u) && !choice.ordinal && owner.history.count > 1) {
        if (plan.ticks > 0) {
            const auto result = model_alternative_history_record(frame, player, owner, destination,
                                                                 plan.ticks, bindings.alternative);
            if (result < 0)
                return false;
            if (result > 0 || (owner.native.flags & 0x100u))
                output_flags |= 0x40u;
        } else {
            const auto newest_tick = owner.history.at(0).header.simulation_tick;
            const auto ticks = subtract(effective_tick, newest_tick);
            if (effective_tick > newest_tick && ticks > 0) {
                const auto result = extrapolate_lag_record(player, owner, destination, ticks,
                                                           extrapolation.movement);
                if (result < 0)
                    return false;
                if (result > 0)
                    output_flags |= 0x20u;
            }
        }
    }
    if (output_flags & 0x60u) {
        destination.header.field_00 = 0;
        build_extrapolated_record_matrices(destination, extrapolation.bones.decoded(),
                                           extrapolation.attachments.scratch);
        if (had_alternatives && !(output_flags & 0x40u)) {
            const auto input = capture_record_hypothesis_input(player, owner, destination, entity);
            const auto origin = read<Vector>(player, entity.offsets.origin);
            resolver_runtime::HypothesisBuildContext context{
                extrapolation.bones.decoded().entity,
                extrapolation.bones.decoded().model,
                origin,
                extrapolation.bones.decoded().pose_parameters,
                extrapolation.bones.decoded().engine,
                extrapolation.bones.decoded().trigonometry};
            resolver_runtime::build_hypotheses(destination, input, context);
        }
    }
    auto preparation = extrapolation.preparation;
    preparation.current_timing = &timing;
    refresh_record_attachments(extrapolation.players, player, destination, preparation,
                               extrapolation.attachments);
    if (extrapolation.validate_record &&
        (!record_time_valid(destination, timing) || !record_geometry_valid(destination)))
        return false;
    if (selected_source && destination.header.field_00)
        *selected_source = choice.source;
    return true;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_selection_live.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate subtract] */
float distance(const LagRecord& first, const LagRecord& second) noexcept {
    const auto& a = first.header.origin;
    const auto& b = second.header.origin;
    const float x = a[0] - b[0], y = a[1] - b[1], z = a[2] - b[2];
    return (y * y + x * x) + z * z;
}
ExtrapolationPlan live_plan(const RecordRing& ring, std::uint32_t ordinal, std::uint32_t options,
                            float interval, const client::CommandBudgetBindings& budget) noexcept {
    ExtrapolationPlan result;
    if (!(options & 1u) || ordinal || ring.count < 2)
        return result;
    const auto& newest = ring.at(0);
    const auto& previous = ring.at(1);
    if (newest.header.flags & 0x200u)
        return result;
    if (newest.header.player_flags & 1u) {
        if (previous.header.player_flags & 1u)
            return result;
        const auto weight = read<float>(&newest, 0xFC), rate = read<float>(&newest, 0x104);
        if (weight > 0.0f && (std::bit_cast<std::uint32_t>(rate) & 0x7FFFFFFFu) &&
            time_to_tick(read<float>(&newest, 0x108) / rate, interval) > 1)
            return result;
    }
    const auto* first = &newest;
    const auto* second = &previous;
    if (!(distance(newest, previous) > 4096.0f)) {
        if (ring.count == 2 || (previous.header.flags & 0x200u))
            return result;
        result.inspected_third = true;
        const auto& third = ring.at(2);
        if (!(distance(previous, third) > 4096.0f))
            return result;
        const auto& a = newest.header.velocity;
        const auto& b = previous.header.velocity;
        if (std::sqrt(b[0] * b[0] + b[1] * b[1]) - std::sqrt(a[0] * a[0] + a[1] * a[1]) > 50.0f)
            return result;
        first = &previous;
        second = &third;
    }
    const auto limit = client::query_command_budget(budget);
    const auto gap = subtract(first->header.simulation_tick, second->header.simulation_tick);
    result.ticks = limit < 1 ? limit : gap < 1 ? 1 : gap > limit ? limit : gap;
    return result;
}
}
bool select_target_history_record(
    client::CommandFrame& frame, void* player, std::int32_t requested_tick, std::uint32_t options,
    LagRecord& destination, std::uint32_t& output_flags, const HistorySelectionBindings& bindings,
    const animation::BoneRuntime& bones, const MovementExtrapolationLiveBindings& movement,
    const client::CommandBudgetBindings& budget, const LagRecord** selected_source,
    std::int32_t* predicted_ticks) {
    if (predicted_ticks)
        *predicted_ticks = 0;
    const auto& extrapolation = bindings.extrapolation;
    const auto& entity = extrapolation.preparation.entity;
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    auto& owner = *extrapolation.players.get(index);
    if (selected_source)
        *selected_source = nullptr;
    rage::TargetLagRecordHistory traversal{owner.native.cached_origin, owner.history.first,
                                           owner.history.count, owner.history.records};
    const auto choice = rage::prepare_target_record_copy(traversal, requested_tick, entity.interval,
                                                         options, destination, output_flags);
    if (!choice.source)
        return false;
    if (extrapolation.validate_record &&
        (!record_geometry_valid(*choice.source) ||
         record_precedes_discontinuity(owner.history, choice.source->header.simulation_tick)))
        return false;
    const bool rebuild_pose = bool(extrapolation.rebuild_pose);
    const auto plan = rebuild_pose ? ExtrapolationPlan{}
                                   : live_plan(owner.history, choice.ordinal, options, entity.interval, budget);
    const HistoryTickTiming timing{bindings.current_time,
                                   read<float>(&frame, 0x198),
                                   bindings.interpolation,
                                   entity.interval,
                                   bindings.maximum_unlag_ms,
                                   read<std::int32_t>(&frame, 0xD4),
                                   std::bit_cast<std::int32_t>(frame.field_18c)};
    const auto window = history_tick_window(timing);
    auto effective_tick = requested_tick;
    if (const auto* older = history_interpolation_source(
            owner.history, choice.ordinal, *choice.source, requested_tick, plan.ticks, window)) {
        interpolate_history_record(
            destination, *choice.source, *older,
            float(requested_tick) * entity.interval,
            entity.interval, bindings.matrix_scratch, output_flags, bindings.rotation,
            bindings.use_captured_endpoint_pose());
        effective_tick = window.last;
    }
    const bool had_alternatives = (choice.source->header.flags & 0x80u) != 0;
    if (rebuild_pose && effective_tick > choice.source->header.simulation_tick) {
        const auto ticks = std::int64_t(effective_tick) - choice.source->header.simulation_tick;
        if (!(options & 1u) || choice.ordinal || ticks > maximum_pose_extrapolation_ticks ||
            ticks > client::query_command_budget(budget) ||
            extrapolate_lag_record(player, owner, destination, std::int32_t(ticks),
                                    extrapolation.movement, movement, true) != ticks)
            return false;
        output_flags |= 0x20u;
    } else if (!rebuild_pose && (options & 1u) && !choice.ordinal && owner.history.count > 1) {
        if (plan.ticks > 0) {
            const auto result = model_alternative_history_record(frame, player, owner, destination,
                                                                 plan.ticks, bindings.alternative);
            if (result < 0)
                return false;
            if (result > 0 || (owner.native.flags & 0x100u))
                output_flags |= 0x40u;
        } else {
            const auto newest_tick = owner.history.at(0).header.simulation_tick;
            const auto ticks = subtract(effective_tick, newest_tick);
            if (effective_tick > newest_tick && ticks > 0) {
                const auto result = extrapolate_lag_record(player, owner, destination, ticks,
                                                           extrapolation.movement, movement);
                if (result < 0)
                    return false;
                if (predicted_ticks && result > 0)
                    *predicted_ticks = result;
                if (result > 0)
                    output_flags |= 0x20u;
            }
        }
    }
    if (output_flags & 0x60u) {
        destination.header.field_00 = 0;
        build_extrapolated_record_matrices(player, destination, bones,
                                           extrapolation.attachments.scratch,
                                           rebuild_pose, entity.interval);
        if (rebuild_pose) {
            if (!(destination.header.flags & 4u))
                return false;
        } else if (had_alternatives && !(output_flags & 0x40u)) {
            const auto input = capture_record_hypothesis_input(player, owner, destination, entity);
            resolver_runtime::build_hypotheses(destination, input, player, bones);
        }
    }
    auto preparation = extrapolation.preparation;
    preparation.current_timing = &timing;
    refresh_record_attachments(extrapolation.players, player, destination, preparation,
                               extrapolation.attachments, bones, rebuild_pose && (output_flags & 0x20u));
    if (rebuild_pose && (output_flags & 0x20u)) {
        if (!record_geometry_valid(destination))
            return false;
        if (predicted_ticks)
            *predicted_ticks = subtract(destination.header.simulation_tick, choice.source->header.simulation_tick);
    }
    if (extrapolation.validate_record &&
        (!record_time_valid(destination, timing) || !record_geometry_valid(destination)))
        return false;
    if (selected_source && destination.header.field_00)
        *selected_source = choice.source;
    return true;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_velocity_capture.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
// Actual433AC5BD/4341873D guard. The saved pointer is retained even if an
// engine call changes the global which originally supplied the binding.
class TransformOverrideScope {
public:
    explicit TransformOverrideScope(std::uint8_t* value) noexcept : target(value) {
        if (target) {
            saved = *target;
            *target = 0;
        }
    }
    ~TransformOverrideScope() {
        if (target)
            *target = saved;
    }
    TransformOverrideScope(const TransformOverrideScope&) = delete;

private:
    std::uint8_t* target;
    std::uint8_t saved;
};
float capture_convar_float(const void* object) noexcept {
    return std::bit_cast<float>(read_prediction_convar(const_cast<void*>(object)));
}
}
void capture_history_velocity(client::PlayerState& owner, void* player,
                              const HistoryVelocityCaptureBindings& bindings) {
    const auto& entity = bindings.entity;
    const auto& physics = bindings.reconstruction;
    auto velocity = read<Vector>(player, entity.offsets.velocity);
    const auto receive = sample_receive_tick(entity.engine, entity.receive_interval);
    bool publish = true;
    if (read<std::int32_t>(&owner.native, 0x1DC) != receive &&
        read<float>(&owner.native, 0x524) != 0.0f) {
        if (physics.update_flags & 1u)
            velocity = {};
        else {
            TransformOverrideScope guard(entity.transform_override);
            const auto timeline = history_timeline(owner);
            if (timeline.packet_count >= 3) {
                const auto& newest = timeline.packets[timeline.packet_head % 6u];
                const auto& previous = timeline.packets[(timeline.packet_head + 1u) % 6u];
                const float x = std::bit_cast<float>(newest.fields_04[0]) -
                                std::bit_cast<float>(previous.fields_04[0]);
                const float y = std::bit_cast<float>(newest.fields_04[1]) -
                                std::bit_cast<float>(previous.fields_04[1]);
                if (y * y + x * x != 0.0f)
                    publish = reconstruct_history_velocity(owner, player, velocity, physics) ==
                              HistoryVelocityReconstructionExit::publish;
                else
                    velocity = {};
            }
        }
    }
    if (publish)
        write(player, entity.offsets.velocity, velocity);
    const auto next_receive = sample_receive_tick(entity.engine, entity.receive_interval);
    if (read<std::int32_t>(&owner.native, 0x1DC) == next_receive)
        return;
    velocity = read<Vector>(player, entity.offsets.velocity);
    validate_velocity(velocity, bindings.maximum_velocity_convar);
    write(player, entity.offsets.velocity, velocity);
    if (read<float>(&owner.native, 0x5B0) == read<float>(&owner.native, 0x444)) {
        const auto blend = read<float>(&owner.native, 0x5A8);
        const float factor = ((1.0f - blend) - 0.0f) * std::bit_cast<float>(0x3EB33332u) +
                             std::bit_cast<float>(0x3F0CCCCDu);
        float maximum = 260.0f;
        const auto handle = read<std::uint32_t>(player, entity.offsets.weapon_handle);
        if (handle != UINT32_MAX) {
            if (auto* weapon = engine::invoke<void*>(entity.entity_list, 4, handle))
                maximum =
                    maximum_number(engine::invoke<float>(weapon, 442), 0.0010000000474974513f);
        }
        velocity = read<Vector>(player, entity.offsets.velocity);
        adjust_capture_velocity(velocity, read<float>(&owner.native, 0x5A8), maximum, factor);
        write(player, entity.offsets.velocity, velocity);
    }
    apply_history_capture_speed_limit(owner.history, player,
                                      {entity, physics.fields.player_flags,
                                       physics.fields.speed_limit, physics.fields.walking_override,
                                       bindings.landing_limit_convar});
    const auto jump = capture_convar_float(physics.cvars.jump_impulse);
    const auto gravity = capture_convar_float(physics.cvars.gravity);
    const float limit = jump - (entity.interval * gravity) * 0.5f;
    if (read<float>(player, entity.offsets.velocity + 8) > limit &&
        animation::layer_is_active(reinterpret_cast<const std::byte*>(&owner.native) + 0x4D8) &&
        physics.classify &&
        physics.classify(player, read<std::int32_t>(&owner.native, 0x4DC)) == 0x3D9)
        write(player, entity.offsets.velocity + 8, limit);
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_velocity_reconstruction.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
using Exit = HistoryVelocityReconstructionExit;
/* [removed duplicate epsilon] */

/* [removed duplicate read] */
/* [removed duplicate negative] */
/* [removed duplicate absolute] */
std::int32_t difference(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) - std::uint32_t(b));
}
std::int32_t add_tick(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) + std::uint32_t(b));
}
float sse_min(float first, float second) noexcept {
    return first < second ? first : second;
}
float sse_max(float first, float second) noexcept {
    return first > second ? first : second;
}
const void* convar_parent(const void* object) noexcept {
    for (;;) {
        const auto* parent = read<const void*>(object, 0x1C);
        if (parent == object)
            return object;
        object = parent;
    }
}
std::uint32_t convar_bits(const void* object, std::size_t offset) noexcept {
    object = convar_parent(object);
    return read<std::uint32_t>(object, offset) ^
           std::uint32_t(reinterpret_cast<std::uintptr_t>(object));
}
float convar_float(const void* object) noexcept {
    return std::bit_cast<float>(convar_bits(object, 0x2C));
}
Vector position(const PacketTimingSample& packet) noexcept {
    return {std::bit_cast<float>(packet.fields_04[0]), std::bit_cast<float>(packet.fields_04[1]),
            std::bit_cast<float>(packet.fields_04[2])};
}
float squared_xy(Vector value) noexcept {
    return value[0] * value[0] + value[1] * value[1];
}
float length_xy(Vector value) noexcept {
    return std::sqrt(squared_xy(value));
}
Vector normalized(Vector value) noexcept {
    const float reciprocal =
        1.0f /
        (std::sqrt((value[0] * value[0] + value[1] * value[1]) + value[2] * value[2]) + epsilon);
    return {value[0] * reciprocal, value[1] * reciprocal, value[2] * reciprocal};
}
Vector perpendicular(Vector value) noexcept {
    if (value[0] == 0.0f && value[1] == 0.0f)
        return {0.0f, -1.0f, 0.0f};
    const float zero_product = value[2] * 0.0f;
    return normalized(
        {value[1] - zero_product, zero_product - value[0], value[0] * 0.0f - value[1] * 0.0f});
}
float dot(Vector a, Vector b) noexcept {
    return (a[0] * b[0] + a[1] * b[1]) + a[2] * b[2];
}
float normalize_yaw(float value) noexcept {
    if (std::isnan(value))
        return 0.0f;
    if (absolute(value) > 180.0f) {
        const float correction = round_half_away_from_zero(absolute(value / 360.0f)) * 360.0f;
        value = value < 0.0f ? value + correction : value - correction;
    }
    return value;
}
float yaw(Vector value, const HistoryVelocityReconstructionBindings& bindings) {
    if (value[0] == 0.0f && value[1] == 0.0f)
        return 0.0f;
    return float(bindings.atan2(double(value[1]), double(value[0]))) * 57.2957763671875f;
}

// Complete43336987: retain the vertical/horizontal special case and both
// slope divisions. A determinant replacement changes native singular cases.
bool circle_center(Vector first, Vector second, Vector third, Vector& center) noexcept {
    const float first_dx = first[0] - second[0];
    const float other_dy = third[1] - first[1];
    if (absolute(first_dx) <= 0.00001f && absolute(other_dy) <= 0.00001f) {
        center = {(third[0] + first[0]) * 0.5f, (first[1] + second[1]) * 0.5f, 0.0f};
        return true;
    }
    const float first_slope = (first[1] - second[1]) / first_dx;
    const float other_slope = other_dy / (third[0] - first[0]);
    if (absolute(first_slope - other_slope) <= 0.000001f)
        return false;
    const float mean_x = (second[0] + first[0]) * 0.5f;
    const float mean_y = (first[1] + second[1]) * 0.5f;
    const float product = other_slope * first_slope;
    const float numerator =
        ((second[1] - third[1]) * product + (second[0] + first[0]) * other_slope) -
        (third[0] + first[0]) * first_slope;
    const float slope_delta = other_slope - first_slope;
    center[0] = numerator / (slope_delta + slope_delta);
    center[1] = mean_y - (center[0] - mean_x) / first_slope;
    center[2] = 0.0f;
    return true;
}

struct FlightWorkspace {
    float duration, gravity, jump_impulse;
    Vector midpoint, end, maximum, minimum;
    MovementCollisionRay ray;
    MovementCollisionTrace trace;
};
void reconstruct_flight(Vector& velocity, bool jump_activity, bool previous_grounded, float delta_z,
                        float height_correction, float packet_duration, std::size_t newest_offset,
                        std::size_t previous_offset, const client::PlayerState& owner, void* player,
                        const HistoryVelocityReconstructionBindings& bindings) {
    if (!jump_activity || previous_grounded) {
        float duration = packet_duration;
        if (jump_activity) {
            const float cycle = read<float>(&owner.native, 0x4F0);
            const float rate = read<float>(&owner.native, 0x4EC);
            duration = maximum_number(bindings.interval,
                                      float(time_to_tick(cycle / rate, bindings.interval)) *
                                          bindings.interval);
        }
        const float gravity_step = negative(convar_float(bindings.cvars.gravity)) * duration;
        const float inferred = (delta_z / duration - gravity_step * 0.5f) + gravity_step;
        velocity[2] = minimum_number(inferred, convar_float(bindings.cvars.jump_impulse));
        return;
    }

    FlightWorkspace work;
    const float cycle = read<float>(&owner.native, 0x4F0);
    const float rate = read<float>(&owner.native, 0x4EC);
    auto ticks = time_to_tick(cycle / rate, bindings.interval);
    if (ticks < 1)
        ticks = 1;
    work.duration = float(ticks) * bindings.interval;
    work.gravity = negative(convar_float(bindings.cvars.gravity));
    work.jump_impulse = convar_float(bindings.cvars.jump_impulse);
    const auto newest = read<Vector>(&owner.native, newest_offset + 4u);
    const auto previous = read<Vector>(&owner.native, previous_offset + 4u);
    work.midpoint = {(previous[0] + newest[0]) * 0.5f, (previous[1] + newest[1]) * 0.5f,
                     (previous[2] + newest[2]) * 0.5f};
    work.end = work.midpoint;
    work.end[2] -= absolute(((work.gravity * 0.5f * work.duration) * work.duration) +
                            work.jump_impulse * work.duration);
    // Native invokes77 before76 and copies their vectors only after both calls.
    const auto* maximum = engine::invoke<const Vector*>(player, 77);
    const auto* minimum = engine::invoke<const Vector*>(player, 76);
    work.maximum = *maximum;
    work.minimum = *minimum;
    work.ray = make_movement_hull_ray({work.midpoint[0], work.midpoint[1], work.midpoint[2]},
                                      {work.end[0], work.end[1], work.end[2]},
                                      {work.minimum[0], work.minimum[1], work.minimum[2]},
                                      {work.maximum[0], work.maximum[1], work.maximum[2]});
    work.trace = {};
    work.trace.fraction = 1.0f;
    work.trace.surface_property = -1;
    work.trace.hitbox = -1;
    // Complete433F545E:434701D0 returns trace type1, unlike434727C0's
    // entity-rejecting type0 filter. Restore the nested guard before policy.
    {
        MovementTraceFlagGuard guard(bindings.trace_flag);
        bindings.trace.trace_ray(work.ray, 1u, {MovementCollisionFilterKind::world_only, 0, 0, 0},
                                 work.trace);
    }
    const bool clear =
        !(work.trace.fraction < 1.0f) && work.trace.all_solid == 0 && work.trace.start_solid == 0;
    if (clear || !(work.trace.plane_normal.z >= 0.7f)) {
        velocity[2] = work.gravity * work.duration + work.jump_impulse;
    } else {
        const float step = work.gravity * work.duration;
        const float current_z = read<float>(&owner.native, newest_offset + 12u);
        const float from_ground =
            ((current_z - work.trace.end.z) - height_correction) / work.duration - step * 0.5f;
        velocity[2] = sse_min(from_ground, step + from_ground);
    }
}

void friction_velocity(Vector& velocity, float elapsed,
                       const HistoryVelocityReconstructionBindings& bindings,
                       bool initial) noexcept {
    const float speed = length_xy(velocity);
    if (initial && !(speed > 0.1f)) {
        velocity = {};
        return;
    }
    const float control = maximum_number(convar_float(bindings.cvars.stop_speed), speed);
    const float drop = (control * convar_float(bindings.cvars.friction)) * elapsed;
    const float reduced = maximum_number(speed - drop, 0.0f);
    if (initial) {
        if (reduced != speed) {
            const float scale = reduced / speed;
            velocity[1] = scale * velocity[1];
            velocity[0] = scale * velocity[0];
        }
    } else {
        const float denominator =
            std::sqrt((velocity[2] * velocity[2] + velocity[1] * velocity[1]) +
                      velocity[0] * velocity[0]) +
            epsilon;
        const float scale = reduced / denominator;
        velocity[0] *= scale;
        velocity[1] *= scale;
        velocity[2] = 0.0f;
    }
}

float accelerate_step(void* player, float projection, float missing,
                      const HistoryVelocityReconstructionBindings& bindings) {
    if (missing <= 0.0f)
        return 0.0f;
    const float positive_projection = sse_max(0.0f, projection);
    const auto flags = (read<std::uint32_t>(player, bindings.fields.player_flags) >> 1) & 1u;
    const bool walking = (read<std::int32_t>(player, bindings.fields.walking) == 1 ||
                          read<std::uint8_t>(player, bindings.fields.walking_override) != 0) &&
                         flags == 0;
    bool exception = false;
    float acceleration_basis = 250.0f;
    float speed_basis = 250.0f;
    if (convar_bits(bindings.cvars.accelerate_weapon_speed, 0x30) != 0u) {
        const auto handle = read<std::uint32_t>(player, bindings.fields.weapon_handle);
        void* weapon =
            handle == UINT32_MAX ? nullptr : engine::invoke<void*>(bindings.entity_list, 4, handle);
        if (weapon) {
            const float maximum_speed = engine::invoke<float>(weapon, 442);
            const auto mode_count = engine::invoke<std::int32_t>(weapon, 462);
            if (mode_count > 0 && engine::invoke<std::int32_t>(weapon, 443) > 1)
                exception = maximum_speed * 0.52f < 110.0f;
            const float ratio = maximum_speed / 250.0f;
            speed_basis = ratio <= 1.0f ? ratio * 250.0f : 250.0f;
            if ((!flags && !walking) || exception)
                acceleration_basis = ratio <= 1.0f ? ratio * 250.0f : 250.0f;
        }
    }
    if (flags) {
        if (!exception)
            acceleration_basis *= 0.34f;
        speed_basis *= 0.34f;
    }
    float acceleration = convar_float(bindings.cvars.accelerate);
    if (walking) {
        if (!exception)
            acceleration_basis *= 0.52f;
        speed_basis *= 0.52f;
        if (positive_projection - speed_basis >= -5.0f) {
            float scale = ((speed_basis * 0.2f - (1.0f - positive_projection * -0.2f)) - -1.0f);
            scale = sse_min(sse_max(scale, 0.0f), 1.0f);
            acceleration = scale * acceleration;
        }
    }
    return sse_min(missing, (bindings.interval * acceleration) * acceleration_basis);
}

struct GroundCandidateWorkspace {
    Vector reference, right, decayed, target_direction;
    Vector direction, velocity, origin, best_velocity;
    std::array<float, 2> speeds;
    float best_distance;
};
void reconstruct_ground_velocity(client::PlayerState& owner, void* player, Vector& velocity,
                                 const HistoryVelocityReconstructionBindings& bindings) {
    if (read<std::uint8_t>(player, bindings.fields.dormant) != 0 || owner.history.count == 0u ||
        length_xy(owner.history.at(0).header.velocity) > 250.0f)
        return;
    const auto old_tick =
        time_to_tick(read<float>(player, bindings.fields.old_simulation_time), bindings.interval);
    const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(player) + 8, 10);
    auto& canonical = *bindings.players.get(std::uint32_t(index));
    if (read<float>(player, bindings.fields.simulation_time) -
            read<float>(&canonical.native, 0x19C) >
        10.0f)
        return;

    GroundCandidateWorkspace work;
    work.reference = read<Vector>(&canonical.native, 0x16C);
    work.right = perpendicular(work.reference);
    const float virtual_speed = engine::invoke<float>(player, 276);
    work.speeds[0] =
        minimum_number(read<float>(player, bindings.fields.speed_limit), virtual_speed);
    work.speeds[1] = length_xy(velocity);
    work.best_velocity = velocity;
    work.best_distance = std::numeric_limits<float>::max();
    const auto& initial_record = canonical.history.at(0).header;
    work.decayed = initial_record.velocity;
    float elapsed = read<float>(player, bindings.fields.simulation_time) -
                    float(initial_record.simulation_tick) * bindings.interval;
    elapsed = sse_min(sse_max(elapsed, bindings.interval), bindings.interval * 16.0f);
    friction_velocity(work.decayed, elapsed, bindings, true);
    work.target_direction =
        normalized({velocity[0] - work.decayed[0], velocity[1] - work.decayed[1], 0.0f});
    constexpr std::array<std::array<float, 2>, 8> combinations{{{1.0f, 0.0f},
                                                                {1.0f, -1.0f},
                                                                {0.0f, -1.0f},
                                                                {-1.0f, -1.0f},
                                                                {-1.0f, 0.0f},
                                                                {-1.0f, 1.0f},
                                                                {0.0f, 1.0f},
                                                                {1.0f, 1.0f}}};
    bool close = false;
    for (float speed_limit : work.speeds) {
        for (const auto& combination : combinations) {
            work.direction = normalized(
                {combination[0] * work.right[0] + combination[1] * work.reference[0],
                 combination[0] * work.right[1] + combination[1] * work.reference[1], 0.0f});
            if (!(dot(work.direction, work.target_direction) >= -0.5f))
                continue;
            const auto& record = canonical.history.at(0).header;
            work.velocity = record.velocity;
            // The native scalar z accumulator starts at zero independently
            // of the three words copied from the record at100DF..100E6.
            work.velocity[2] = 0.0f;
            work.origin = record.origin;
            const auto final_tick = time_to_tick(
                read<float>(player, bindings.fields.simulation_time), bindings.interval);
            auto first_tick = add_tick(final_tick, -16);
            if (final_tick < first_tick)
                first_tick = final_tick;
            else if (first_tick <= record.simulation_tick)
                first_tick =
                    final_tick < record.simulation_tick ? final_tick : record.simulation_tick;
            for (auto tick = add_tick(first_tick, 1); tick <= final_tick;
                 tick = add_tick(tick, 1)) {
                friction_velocity(work.velocity, bindings.interval, bindings, false);
                if (tick <= old_tick) {
                    const float projection = dot(work.direction, work.velocity);
                    const float step =
                        accelerate_step(player, projection, speed_limit - projection, bindings);
                    work.velocity[0] += work.direction[0] * step;
                    work.velocity[1] = work.direction[1] * step + work.velocity[1];
                }
                if (length_xy(work.velocity) > speed_limit) {
                    const float reciprocal =
                        speed_limit / (std::sqrt((work.velocity[2] * work.velocity[2] +
                                                  work.velocity[1] * work.velocity[1]) +
                                                 work.velocity[0] * work.velocity[0]) +
                                       epsilon);
                    work.velocity[0] *= reciprocal;
                    work.velocity[1] = reciprocal * work.velocity[1];
                    work.velocity[2] = reciprocal * 0.0f;
                }
                work.origin[0] = bindings.interval * work.velocity[0] + work.origin[0];
                work.origin[1] = bindings.interval * work.velocity[1] + work.origin[1];
                if (read<float>(&canonical.native, 0x3B8) < 0.001f || tick >= final_tick) {
                    const auto live = read<Vector>(player, bindings.fields.origin);
                    const float dx = work.origin[0] - live[0];
                    const float dy = work.origin[1] - live[1];
                    const float distance = dy * dy + dx * dx;
                    if (distance < work.best_distance) {
                        work.best_velocity = work.velocity;
                        work.best_distance = distance;
                        if (distance < 0.0006250000442378223f) {
                            close = true;
                            break;
                        }
                    }
                }
            }
            if (close || work.best_distance < 0.0006250000442378223f) {
                close = true;
                break;
            }
        }
        if (close)
            break;
    }
    if (work.best_distance <= 0.0006250000442378223f) {
        bindings.update_flags |= 8u;
        velocity[0] = work.best_velocity[0];
        velocity[1] = work.best_velocity[1];
    }
}
} // namespace

HistoryVelocityReconstructionExit
reconstruct_history_velocity(client::PlayerState& owner, void* player, Vector& velocity,
                             const HistoryVelocityReconstructionBindings& bindings) {
    const auto timeline = history_timeline(owner);
    const auto& a = timeline.packets[timeline.packet_head % 6u];
    const auto& b = timeline.packets[(timeline.packet_head + 1u) % 6u];
    const auto newest_offset = std::size_t(0x23C) + (timeline.packet_head % 6u) * 32u;
    const auto previous_offset = std::size_t(0x23C) + ((timeline.packet_head + 1u) % 6u) * 32u;
    const auto oldest_offset = std::size_t(0x23C) + ((timeline.packet_head + 2u) % 6u) * 32u;
    const auto newest = position(a), previous = position(b);
    const Vector displacement{newest[0] - previous[0], newest[1] - previous[1],
                              newest[2] - previous[2]};
    if (squared_xy(displacement) > 1000000.0f)
        return Exit::continue_capture;

    bool jump_activity = false, special_sequence = false;
    const float current_cycle = read<float>(&owner.native, 0x4F0);
    const float previous_cycle = read<float>(&owner.native, 0x384);
    const auto sequence = read<std::int32_t>(&owner.native, 0x4DC);
    if (!((current_cycle == previous_cycle &&
           sequence == read<std::int32_t>(&owner.native, 0x370)) ||
          current_cycle >= previous_cycle) &&
        bindings.classify) {
        jump_activity = bindings.classify(player, sequence) == 0x3D9;
        if (jump_activity) {
            const char* name =
                bindings.sequence_name ? bindings.sequence_name(player, sequence) : "";
            special_sequence =
                bindings.compare_sequence_name(name, bindings.jump_sequence_name) != 0;
        }
    }
    const auto flags = a.fields_04[4], previous_flags = b.fields_04[4];
    const bool grounded = (flags & 1u) != 0u;
    float height_correction = 0.0f;
    if (!grounded) {
        const float duck = absolute(read<float>(player, bindings.fields.duck_amount));
        if (duck != read<float>(&owner.native, 0x104) && (duck == 0.0f || duck == 1.0f)) {
            if ((previous_flags & 1u) == 0u) {
                if (((previous_flags ^ flags) & 2u) != 0u)
                    height_correction = (flags & 2u) != 0u ? 9.0f : -9.0f;
            } else if (jump_activity) {
                if (special_sequence && duck == 0.0f)
                    height_correction = -9.0f;
                else if (!special_sequence && duck == 1.0f)
                    height_correction = 9.0f;
            }
        }
    }
    const float delta_z = displacement[2] - height_correction;
    // Native retains the packet addresses across engine classification but
    // rereads these timestamps afterwards; callback changes remain visible.
    const auto newest_tick = read<std::int32_t>(&owner.native, newest_offset);
    const auto previous_tick = read<std::int32_t>(&owner.native, previous_offset);
    const bool old_receive_gap =
        previous_tick < add_tick(read<std::int32_t>(&owner.native, previous_offset + 0x18u), -1);
    const bool new_receive_gap =
        newest_tick < add_tick(read<std::int32_t>(&owner.native, newest_offset + 0x18u), -1);
    const bool receive_transition = old_receive_gap != new_receive_gap;
    float duration = float(difference(newest_tick, previous_tick)) * bindings.interval;
    const Vector old_velocity = read<Vector>(&owner.native, 0x120);
    if (receive_transition) {
        const float speed = length_xy(old_velocity);
        if (speed > 5.0f) {
            const float estimated =
                float(time_to_tick(length_xy(displacement) / speed, bindings.interval)) *
                bindings.interval;
            const auto current_position = read<Vector>(&owner.native, newest_offset + 4u);
            const auto previous_position = read<Vector>(&owner.native, previous_offset + 4u);
            const float dx =
                current_position[0] - (old_velocity[0] * estimated + previous_position[0]);
            const float dy =
                current_position[1] - (old_velocity[1] * estimated + previous_position[1]);
            if (std::sqrt(dy * dy + dx * dx) < 0.1f)
                duration = estimated;
        }
    }
    const auto ticks = time_to_tick(duration, bindings.interval);
    const float reciprocal = 1.0f / duration;
    velocity = {reciprocal * displacement[0], reciprocal * displacement[1], reciprocal * delta_z};
    const float speed_squared = velocity[1] * velocity[1] + velocity[0] * velocity[0];
    if (speed_squared > 2.0f && squared_xy(old_velocity) > 2.0f) {
        const float new_reciprocal = 1.0f / (std::sqrt(speed_squared) + epsilon);
        const float old_reciprocal = 1.0f / (length_xy(old_velocity) + epsilon);
        const float similarity =
            ((old_reciprocal * old_velocity[1]) * (new_reciprocal * velocity[1])) +
            ((new_reciprocal * velocity[0]) * (old_reciprocal * old_velocity[0]));
        if (similarity < 0.9961947202682495f && similarity >= 0.258819043636322f) {
            const float new_yaw = yaw(velocity, bindings);
            const float old_yaw = yaw(old_velocity, bindings);
            const float angle =
                (normalize_yaw(new_yaw - old_yaw) * 0.5f + new_yaw) * 0.01745329238474369f;
            if (!grounded)
                velocity[2] -= convar_float(bindings.cvars.gravity) * duration * 0.5f;
            const float sine = bindings.sine(angle);
            const float cosine = bindings.cosine(angle);
            const float speed = std::sqrt(speed_squared);
            velocity[0] = speed * cosine;
            velocity[1] = speed * sine;
        }
    }
    if (ticks == 1 && !receive_transition)
        return Exit::publish;
    if (ticks <= 0) {
        if (old_receive_gap || !new_receive_gap || jump_activity ||
            ((previous_flags | flags) & 1u) != 0u)
            return Exit::continue_capture;
        if (owner.history.count != 0u) {
            const auto& record = owner.history.at(0).header;
            const float negative_gravity = negative(convar_float(bindings.cvars.gravity));
            velocity = record.velocity;
            const float height =
                read<float>(player, bindings.fields.origin + 8u) - record.origin[2];
            velocity[2] =
                std::sqrt(old_velocity[2] * old_velocity[2] - (height * negative_gravity) * -2.0f);
        }
        return Exit::publish;
    }
    if (ticks > 20)
        return Exit::continue_capture;
    if (grounded) {
        if ((previous_flags & 1u) != 0u)
            reconstruct_ground_velocity(owner, player, velocity, bindings);
        return Exit::publish;
    }

    reconstruct_flight(velocity, jump_activity, (previous_flags & 1u) != 0u, delta_z,
                       height_correction, duration, newest_offset, previous_offset, owner, player,
                       bindings);
    const float reconstructed_squared = squared_xy(velocity);
    if (reconstructed_squared < 400.0f)
        return Exit::publish;
    const auto curve_newest = read<Vector>(&owner.native, newest_offset + 4u);
    const auto curve_previous = read<Vector>(&owner.native, previous_offset + 4u);
    const auto oldest = read<Vector>(&owner.native, oldest_offset + 4u);
    const Vector previous_delta{curve_previous[0] - oldest[0], curve_previous[1] - oldest[1],
                                curve_previous[2] - oldest[2]};
    const float previous_squared =
        previous_delta[1] * previous_delta[1] + previous_delta[0] * previous_delta[0];
    if (previous_squared < 1.0f)
        return Exit::publish;
    if (difference(read<std::int32_t>(&owner.native, previous_offset),
                   read<std::int32_t>(&owner.native, oldest_offset)) > 20 ||
        previous_delta[2] * previous_delta[2] + previous_squared > 100000.0f)
        return Exit::continue_capture;
    Vector center;
    if (!circle_center({curve_previous[0], curve_previous[1], 0.0f},
                       {curve_newest[0], curve_newest[1], 0.0f}, {oldest[0], oldest[1], 0.0f},
                       center))
        return Exit::publish;
    if (!std::isfinite(center[0]) || !std::isfinite(center[1]))
        return Exit::continue_capture;
    const auto delta_direction = normalized(
        {curve_newest[0] - curve_previous[0], curve_newest[1] - curve_previous[1], 0.0f});
    const auto radial_direction =
        normalized({curve_newest[0] - center[0], curve_newest[1] - center[1], 0.0f});
    auto tangent = perpendicular(radial_direction);
    if (!(dot(delta_direction, tangent) >= 0.0f))
        tangent = {negative(tangent[0]), negative(tangent[1]), negative(tangent[2])};
    const float speed = std::sqrt(reconstructed_squared);
    velocity[0] = speed * tangent[0];
    velocity[1] = speed * tangent[1];
    return std::isnan(velocity[0]) || std::isnan(velocity[1]) || std::isnan(velocity[2])
               ? Exit::continue_capture
               : Exit::publish;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: history_yaw_side.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
float owner_float(const client::NativePlayerStateLayout& owner, std::size_t offset) noexcept {
    float value;
    std::memcpy(&value, reinterpret_cast<const std::byte*>(&owner) + offset, sizeof value);
    return value;
}
/* [removed duplicate absolute] */
float normalized_distance(float value) noexcept {
    if (std::isnan(value))
        return 0.0f;
    //4341AD12 is the concrete recovered half-away rounding operation. The
    // initial NaN test occurs before normalization; infinity can consequently
    // become NaN during the subtraction and must remain unordered afterward.
    if (absolute(value) > 180.0f) {
        const float correction = round_half_away_from_zero(absolute(value / 360.0f)) * 360.0f;
        value = value < 0.0f ? value + correction : value - correction;
    }
    return value < 0.0f ? std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) ^ 0x80000000u)
                        : value;
}
}

void update_history_yaw_side(client::PlayerState& owner, bool enemy, std::uint32_t update_flags,
                             const std::uint8_t& enabled, HistoryYawSideFmod fmod) {
    if (!enemy || !enabled || (update_flags & 1u) || !owner.history.count)
        return;
    auto& state = owner.native;
    const auto& newest = owner.history.at(0).header;
    float current = state.reference_yaw;
    float previous = std::bit_cast<float>(newest.player_history_1ac);
    if (update_flags & 4u)
        current = previous;
    else if (newest.flags & 8u) {
        if (owner.history.count < 2u)
            return;
        previous = std::bit_cast<float>(owner.history.at(1).header.player_history_1ac);
    }
    // Native UCOMISS equality exits only for ordered, equal values.
    if (previous == current)
        return;

    const float first_distance = normalized_distance(current - owner_float(state, 0x3C));
    const float second_distance = normalized_distance(current - owner_float(state, 0x60));
    // Prologue:4346FF70=15,43472C88=165,4346EEB0=20,4347222C=-20.
    if (first_distance < 15.0f) {
        state.flags |= 4u;
        return;
    }
    if (second_distance < 15.0f) {
        state.flags &= ~4u;
        return;
    }

    // SUBSS precedes CVTPS2PD: preserve the single-precision difference before
    // the actual double CRT remainder and the final single-precision store.
    const float difference = current - previous;
    float delta = static_cast<float>(fmod(double(difference), 360.0));
    if (current > previous) {
        if (delta >= 180.0f)
            delta -= 360.0f;
    } else if (delta <= -180.0f)
        delta += 360.0f;

    //4340EAA1..EAAA simplifies to XOR4, including when it was already set.
    // Unordered COMISS takes the following bounded-angle arm instead.
    if (absolute(delta) > 165.0f)
        state.flags ^= 4u;
    else if (delta > 20.0f)
        state.flags |= 4u;
    else if (delta < -20.0f)
        state.flags &= ~4u;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: interpolation.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate radians] */
/* [removed duplicate degrees] */
/* [removed duplicate negate] */
Vector interpolated_angles(float first, float second, float fraction, const RotationMath& math) {
    if (first == second)
        return {0.0f, first, 0.0f};
    const auto a = history_angle_quaternion({0.0f, first, 0.0f}, math);
    const auto b = history_angle_quaternion({0.0f, second, 0.0f}, math);
    return history_quaternion_angles(
        slerp_history_quaternion(a, align_history_quaternion(a, b), fraction, math), math);
}
}
Quaternion history_angle_quaternion(const Vector& angles, const RotationMath& math) {
    const float yaw = (angles[1] * radians) * 0.5f;
    const float sy = math.sine(yaw), cy = math.cosine(yaw);
    const float pitch = (angles[0] * radians) * 0.5f;
    const float sp = math.sine(pitch), cp = math.cosine(pitch);
    const float roll = (angles[2] * radians) * 0.5f;
    const float sr = math.sine(roll), cr = math.cosine(roll);
    const float srcp = sr * cp, crsp = cr * sp, srsp = sr * sp, crcp = cr * cp;
    return {srcp * cy - crsp * sy, srcp * sy + crsp * cy, crcp * sy - srsp * cy,
            srsp * sy + crcp * cy};
}
Quaternion align_history_quaternion(const Quaternion& first, const Quaternion& second) noexcept {
    float difference = 0, sum = 0;
    for (unsigned i = 0; i != 4; ++i) {
        const auto minus = first[i] - second[i], plus = first[i] + second[i];
        difference += minus * minus;
        sum += plus * plus;
    }
    auto result = second;
    if (!(difference <= sum))
        for (auto& value : result)
            value = negate(value);
    return result;
}
Quaternion slerp_history_quaternion(const Quaternion& first, const Quaternion& second,
                                    float fraction, const RotationMath& math) {
    const float cosine = ((first[1] * second[1] + first[0] * second[0]) + first[2] * second[2]) +
                         first[3] * second[3];
    float inverse = 1.0f - fraction;
    Quaternion result;
    if (!(cosine + 1.0f > 0.000001f)) {
        result = {negate(second[1]), second[0], negate(second[3]), second[2]};
        inverse = math.sine(float(double(inverse) * 1.5707963267948966));
        fraction = math.sine(float(double(fraction) * 1.5707963267948966));
        for (unsigned i = 0; i != 3; ++i)
            result[i] = first[i] * inverse + result[i] * fraction;
    } else {
        if (1.0f - cosine > 0.000001f) {
            const float angle = math.arc_cosine(cosine);
            const float sine = math.sine(angle);
            inverse = math.sine(inverse * angle) / sine;
            fraction = math.sine(angle * fraction) / sine;
        }
        for (unsigned i = 0; i != 4; ++i)
            result[i] = second[i] * fraction + first[i] * inverse;
    }
    return result;
}
Vector history_quaternion_angles(const Quaternion& quaternion, const RotationMath& math) {
    const auto matrix = animation::bone_local_matrix({}, quaternion);
    const float horizontal =
        std::sqrt(matrix.rows[0][0] * matrix.rows[0][0] + matrix.rows[1][0] * matrix.rows[1][0]);
    Vector result;
    if (horizontal > 0.001f) {
        result[1] = float(math.arc_tangent2(double(matrix.rows[1][0]), double(matrix.rows[0][0]))) *
                    degrees;
        result[0] =
            float(math.arc_tangent2(double(negate(matrix.rows[2][0])), double(horizontal))) *
            degrees;
        result[2] = float(math.arc_tangent2(double(matrix.rows[2][1]), double(matrix.rows[2][2]))) *
                    degrees;
    } else {
        result[1] =
            float(math.arc_tangent2(double(negate(matrix.rows[0][1])), double(matrix.rows[1][1]))) *
            degrees;
        result[0] =
            float(math.arc_tangent2(double(negate(matrix.rows[2][0])), double(horizontal))) *
            degrees;
        result[2] = 0.0f;
    }
    return result;
}
const LagRecord* history_interpolation_source(const RecordRing& ring, std::uint32_t ordinal,
                                              const LagRecord& selected,
                                              std::int32_t requested_tick,
                                              std::int32_t planned_ticks,
                                              const HistoryTickWindow& window) noexcept {
    if (requested_tick != selected.header.simulation_tick || planned_ticks ||
        requested_tick <= window.last || window.last < window.corrected ||
        ordinal + 1u >= ring.count)
        return nullptr;
    const auto& older = ring.at(ordinal + 1u);
    if (older.header.simulation_tick >= window.last)
        return nullptr;
    const auto& a = selected.header.origin;
    const auto& b = older.header.origin;
    const float x = a[0] - b[0], y = a[1] - b[1], z = a[2] - b[2];
    if ((y * y + x * x) + z * z > 4096.0f)
        return nullptr;
    return &older;
}
float interpolate_history_fields(LagRecord& destination, const LagRecord& newer,
                                 const LagRecord& older, float requested_time, float interval,
                                 const RotationMath& math, Vector* interpolated_rotation) {
    destination.header.simulation_tick = time_to_tick(requested_time, interval);
    const float old_time = float(older.header.simulation_tick) * interval;
    const float difference = float(newer.header.simulation_tick) * interval - old_time;
    const float fraction = _mm_cvtss_f32(_mm_min_ss(
        _mm_max_ss(_mm_set_ss((requested_time - old_time) / difference), _mm_setzero_ps()),
        _mm_set_ss(1.0f)));
    const auto angles =
        interpolated_angles(older.header.animation_yaw, newer.header.animation_yaw, fraction, math);
    if (interpolated_rotation)
        *interpolated_rotation = angles;
    destination.header.animation_yaw = angles[1];
    destination.header.player_flags = newer.header.player_flags;
    for (unsigned i = 0; i != 3; ++i) {
        destination.header.origin[i] =
            (newer.header.origin[i] - older.header.origin[i]) * fraction + older.header.origin[i];
        destination.header.velocity[i] =
            (newer.header.velocity[i] - older.header.velocity[i]) * fraction +
            older.header.velocity[i];
    }
    return fraction;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: live_record_attachments.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
}
void refresh_record_attachments(client::PlayerStateCache& players, void* target, LagRecord& record,
                                const EntityPreparationContext& preparation,
                                const LiveRecordAttachmentContext& attachments, bool projected_pose) {
    const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(target) + 8, 10);
    auto& cached = players.get(std::uint32_t(index))->native;
    const auto& offsets = preparation.entity.offsets;
    const auto old_432 = read<std::uint32_t>(target, offsets.field_432b1f5b);
    const auto old_0d = read<std::uint32_t>(target, offsets.field_0d560fac);
    const auto old_angles = read<std::array<std::uint32_t, 2>>(target, offsets.eye_angles);
    if (projected_pose) {
        write(target, offsets.eye_angles,
              std::array{record.header.eye_pitch, std::bit_cast<float>(record.header.player_history_1ac)});
    } else {
        prepare_entity_angles(players, target, record, true, preparation);
    }
    record.header.field_50 = read<float>(target, offsets.eye_angles + 4);
    auto* collision = engine::invoke<void*>(target, 3);
    auto* maximum = engine::invoke<Vector*>(collision, 2);
    const auto old_height = (*maximum)[2];
    (*maximum)[2] = record.header.collision_max_z;
    const auto old_duck = read<std::uint32_t>(target, offsets.duck_amount);
    write(target, offsets.duck_amount,
          projected_pose ? record.header.duck_amount : cached.cached_duck_amount);
    const auto old_6e = read<std::uint32_t>(target, attachments.field_6e9852d6);
    const auto old_2a = read<Vector>(target, attachments.field_2a671b42);
    write(target, attachments.field_2a671b42, record.header.origin);
    const auto old_4e = read<Vector>(target, attachments.field_4e66bb34);
    write(target, attachments.field_4e66bb34, record.header.origin);
    const auto old_flags = read<std::uint32_t>(target, attachments.field_83a6cf2b);
    write(target, attachments.field_83a6cf2b, old_flags & 0xFFFFF7FFu);
    if (record.header.flags & 4u)
        attachments.bones.refresh_hitbox_attachments(target, record.matrices, attachments.scratch);
    if (record.header.flags & 0x80u)
        for (auto& alternative : record.alternatives)
            attachments.bones.refresh_hitbox_attachments(target, alternative.matrices,
                                                         attachments.scratch);
    write(target, attachments.field_83a6cf2b, old_flags);
    write(target, attachments.field_4e66bb34, old_4e);
    write(target, attachments.field_2a671b42, old_2a);
    write(target, attachments.field_6e9852d6, old_6e);
    write(target, offsets.duck_amount, old_duck);
    (*maximum)[2] = old_height;
    write(target, offsets.eye_angles, old_angles);
    write(target, offsets.field_0d560fac, old_0d);
    write(target, offsets.field_432b1f5b, old_432);
}
void refresh_record_attachments(client::PlayerStateCache& players, void* target, LagRecord& record,
                                const EntityPreparationContext& preparation,
                                const RecordAttachmentContext& attachments,
                                const animation::BoneRuntime& bones, bool projected_pose) {
    const LiveRecordAttachmentContext live{bones,
                                           attachments.scratch,
                                           attachments.field_6e9852d6,
                                           attachments.field_2a671b42,
                                           attachments.field_4e66bb34,
                                           attachments.field_83a6cf2b};
    refresh_record_attachments(players, target, record, preparation, live, projected_pose);
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: live_timeline.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate add] */
/* [removed duplicate subtract] */
/* [removed duplicate cap] */
}

std::int32_t estimate_missing_ticks(const LagTimeline& timeline, const MissingTickTiming& timing,
                                    const client::CommandBudgetBindings& budget) {
    const auto now = add(timing.tick_d4, timing.tick_18c);
    const auto simulation = time_to_tick(timeline.simulation_time, timing.interval);
    const auto distance = subtract(now, simulation);
    const auto sign = distance < 0 ? UINT32_MAX : 0u;
    const auto absolute = std::bit_cast<std::int32_t>((std::uint32_t(distance) ^ sign) - sign);
    if (absolute >= 128)
        return 0;
    std::array<std::int32_t, 10> pattern;
    std::size_t count = 0;
    bool simple = timeline.record_count < 2 || timeline.packet_count < 2;
    if (!simple && simulation < subtract(timeline.last_update_tick, 1) &&
        simulation < timeline.at(0).header.simulation_tick)
        simple = true;
    if (!simple && packet_timing_discontinuity(timeline))
        simple = true;
    if (!simple && (timeline.at(0).header.flags & 0x200u))
        simple = true;
    if (!simple) {
        const auto head = timeline.packet_head % 6u, next = (timeline.packet_head + 1u) % 6u;
        if (subtract(timeline.packets[head].field_18, timeline.packets[next].field_18) < 4 &&
            subtract(timeline.at(0).header.receive_tick, timeline.at(1).header.receive_tick) > 7)
            simple = true;
    }
    if (simple)
        pattern[count++] = 2;
    else {
        const LagRecord* previous = nullptr;
        for (std::uint32_t age = 1; age < timeline.record_count; ++age) {
            const auto& record = timeline.at(age);
            if (previous && !(previous->header.flags & 8u)) {
                const auto limit = client::query_command_budget(budget); //433F9602
                pattern[count++] =
                    cap(subtract(previous->header.simulation_tick, record.header.simulation_tick),
                        limit);
                if (count == 10)
                    break;
            }
            if (record.header.flags & 0x200u)
                break;
            previous = &record;
        }
        const auto latest_limit = client::query_command_budget(budget); //433F9669
        const auto latest_gap = cap(
            subtract(timeline.at(0).header.simulation_tick, timeline.at(1).header.simulation_tick),
            latest_limit);
        const auto arrival_limit = client::query_command_budget(budget); //433F969A, before engine14
        const auto received = sample_receive_tick(timing.engine, timing.receive_interval);
        const auto arrival_gap =
            cap(add(subtract(received, timeline.at(0).header.receive_tick), 1), arrival_limit);
        if (count < 10)
            pattern[count++] = latest_gap >= arrival_gap ? latest_gap : arrival_gap;
    }
    std::uint32_t index = 0;
    auto step = pattern[0];
    auto position = add(timeline.last_update_tick, step);
    std::int32_t total = 0;
    while (position <= now) {
        if (position == now) {
            const auto player_index =
                engine::invoke<std::int32_t>(static_cast<std::byte*>(timing.player) + 8, 10);
            if (engine::invoke<std::int32_t>(timing.engine, 12) < player_index)
                break;
        }
        total = add(total, step);
        ++index;
        step = pattern[index % count];
        position = add(position, step);
    }
    return total;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: matrix_interpolation.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate radians] */
float add(float first, float second) noexcept {
    // Commuting SSE operands changes the selected NaN payload. Keep the native
    // destination operand explicit even when the compiler vectorizes this loop.
    __asm__ __volatile__("addss %1, %0" : "+x"(first) : "x"(second));
    return first;
}
float multiply(float first, float second) noexcept {
    __asm__ __volatile__("mulss %1, %0" : "+x"(first) : "x"(second));
    return first;
}
Matrix3x4 rotation_matrix(const Vector& angles, const Vector& origin, const RotationMath& math) {
    const float sy = math.sine(angles[1] * radians), cy = math.cosine(angles[1] * radians);
    const float sp = math.sine(angles[0] * radians), cp = math.cosine(angles[0] * radians);
    const float sr = math.sine(angles[2] * radians), cr = math.cosine(angles[2] * radians);
    const float crcy = cr * cy, crsy = cr * sy, srcy = sr * cy, srsy = sr * sy;
    return {{{cp * cy, srcy * sp - crsy, crcy * sp + srsy, origin[0]},
             {cp * sy, srsy * sp + crcy, crsy * sp - srcy, origin[1]},
             {-sp, sr * cp, cr * cp, origin[2]}}};
}
Matrix3x4 concatenate(const Matrix3x4& parent, const Matrix3x4& local) noexcept {
    Matrix3x4 result;
    for (unsigned row = 0; row != 3; ++row)
        for (unsigned column = 0; column != 4; ++column)
            result.rows[row][column] =
                add(add(add(multiply(parent.rows[row][2], local.rows[2][column]),
                            multiply(parent.rows[row][1], local.rows[1][column])),
                        multiply(parent.rows[row][0], local.rows[0][column])),
                    column == 3 ? parent.rows[row][3] : 0.0f);
    return result;
}
Matrix3x4 inverse_rotation(float yaw, const Vector& origin, HistoryMatrixScratch& scratch,
                           bool primary, const RotationMath& math) {
    const auto rotation = rotation_matrix({0.0f, yaw, 0.0f}, {}, math);
    Matrix3x4 inverse;
    for (unsigned row = 0; row != 3; ++row)
        for (unsigned column = 0; column != 3; ++column)
            inverse.rows[row][column] = rotation.rows[column][row];
    (void)primary;
    // R^-1 * -origin must use this rotation, never prior scratch contents.
    for (unsigned row = 0; row < 3; ++row)
        inverse.rows[row][3] =
            -(inverse.rows[row][0] * origin[0] + inverse.rows[row][1] * origin[1] +
              inverse.rows[row][2] * origin[2]);
    for (unsigned i = 0; i != 3; ++i) {
        scratch.inverse_row1[i] = inverse.rows[1][i];
        scratch.inverse_row2[i] = inverse.rows[2][i];
    }
    return inverse;
}
/* [removed duplicate interpolated_angles] */
void transform(HitboxMatrices& destination, const HitboxMatrices& source,
               const Matrix3x4& relative) noexcept {
    for (unsigned i = 0; i != 19; ++i)
        destination[i] = concatenate(relative, source[i]);
}
}
float interpolate_history_record(LagRecord& destination, const LagRecord& newer,
                                 const LagRecord& older, float requested_time, float interval,
                                 const HistoryMatrixScratch& incoming, std::uint32_t& output_flags,
                                 const RotationMath& math, bool use_captured_endpoint_pose) {
    Vector angles;
    const float fraction = interpolate_history_fields(destination, newer, older, requested_time,
                                                      interval, math, &angles);
    if (use_captured_endpoint_pose && fraction == 1.0f) {
        // Accuracy boost can reach this path at the exact newer tick.
        destination.header.animation_yaw = newer.header.animation_yaw;
        destination.header.flags = (destination.header.flags & ~0x84u) | (newer.header.flags & 0x84u);
        destination.animation = newer.animation;
        destination.matrices = newer.matrices;
        if (newer.header.flags & 0x80u)
            destination.alternatives = newer.alternatives;
    } else if (older.header.flags & 4u) {
        HistoryMatrixScratch scratch;
        std::memcpy(&scratch, &incoming, sizeof scratch);
        const auto inverse =
            inverse_rotation(older.header.animation_yaw, older.header.origin, scratch, true, math);
        transform(destination.matrices, older.matrices,
                  concatenate(rotation_matrix(angles, destination.header.origin, math), inverse));
        if ((older.header.flags & newer.header.flags & 0x80u) != 0) {
            for (unsigned i = 0; i != 3; ++i) {
                const float old_yaw = std::bit_cast<float>(older.alternatives[i].state[0]);
                const float new_yaw = std::bit_cast<float>(newer.alternatives[i].state[0]);
                const auto alternative_angles =
                    interpolated_angles(old_yaw, new_yaw, fraction, math);
                const auto alternative_inverse =
                    inverse_rotation(old_yaw, older.header.origin, scratch, false, math);
                transform(destination.alternatives[i].matrices, older.alternatives[i].matrices,
                          concatenate(
                              rotation_matrix(alternative_angles, destination.header.origin, math),
                              alternative_inverse));
            }
        } else
            destination.header.flags &= ~0x80u;
    }
    output_flags |= 0x10u;
    destination.header.field_00 = 0;
    return fraction;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: native_history_boundaries.cpp
// ============================================================================
#if defined(_WIN32) && (defined(_M_IX86) || defined(__i386__))
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::lagcomp {
namespace {
void* named(const char* name) {
    const auto module = GetModuleHandleW(L"msvcrt.dll");
    if (!module)
        throw std::runtime_error("History math requires the loaded msvcrt.dll");
    auto* const result = reinterpret_cast<void*>(GetProcAddress(module, name));
    if (!result)
        throw std::runtime_error("Loaded msvcrt.dll lacks a required history import");
    return result;
}
struct Entries {
    void* fmod;
    void* power;
    void* arccosine;
    void* substring;
};
const Entries& entries() {
    static const Entries result{named("_CIfmod"), named("__libm_sse2_powf"),
                                named("__libm_sse2_acosf"), named("strstr")};
    return result;
}
double remainder(double numerator, double denominator) {
    double result;
    __asm__ __volatile__("fldl %1\n\tfldl %2\n\tcall *%3\n\tfstpl %0"
                         : "=m"(result)
                         : "m"(numerator), "m"(denominator), "r"(entries().fmod)
                         : "eax", "ecx", "edx", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5",
                           "xmm6", "xmm7", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)",
                           "st(6)", "st(7)", "memory", "cc");
    return result;
}
float power(float base, float exponent) {
    register __m128 first __asm__("xmm0") = _mm_set_ss(base);
    register __m128 second __asm__("xmm1") = _mm_set_ss(exponent);
    __asm__ __volatile__("call *%2"
                         : "+x"(first), "+x"(second)
                         : "r"(entries().power)
                         : "eax", "ecx", "edx", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                           "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)",
                           "memory", "cc");
    return _mm_cvtss_f32(first);
}
float arccosine(float value) {
    register __m128 first __asm__("xmm0") = _mm_set_ss(value);
    __asm__ __volatile__("call *%1"
                         : "+x"(first)
                         : "r"(entries().arccosine)
                         : "eax", "ecx", "edx", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6",
                           "xmm7", "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)",
                           "st(7)", "memory", "cc");
    return _mm_cvtss_f32(first);
}
int substring(const char* text, const char* part) {
    using Entry = char*(__cdecl*)(const char*, const char*);
    return reinterpret_cast<Entry>(entries().substring)(text, part) != nullptr;
}
double __vectorcall environment_atan2(double y, double x) {
    return animation::native_sse_atan2(y, x);
}
void displacement(void* entry, void* studio, std::int32_t sequence, const float* poses,
                  Vector& output) {
    const void* arguments[]{poses, &output, entry};
    //433E0681's ADD ESP,8 is required: client+19D600 returns with plain RET.
    __asm__ __volatile__("pushl 4(%2)\n\tpushl (%2)\n\tcall *8(%2)\n\taddl $8,%%esp"
                         : "+c"(studio), "+d"(sequence)
                         : "S"(arguments)
                         : "eax", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7",
                           "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)",
                           "memory", "cc");
}
void update_animation(void* entry, void* state, float zero, float yaw, float pitch,
                      std::int32_t force) {
    const std::uintptr_t arguments[]{std::uintptr_t(force),
                                     reinterpret_cast<std::uintptr_t>(entry)};
    register __m128 first __asm__("xmm0") = _mm_set_ss(zero);
    register __m128 second __asm__("xmm1") = _mm_set_ss(yaw);
    register __m128 third __asm__("xmm2") = _mm_set_ss(pitch);
    std::uint32_t zero_register = 0;
    __asm__ __volatile__("pushl (%5)\n\tcall *4(%5)"
                         : "+c"(state), "+d"(zero_register), "+x"(first), "+x"(second), "+x"(third)
                         : "S"(arguments)
                         : "eax", "xmm3", "xmm4", "xmm5", "xmm6", "xmm7", "st", "st(1)", "st(2)",
                           "st(3)", "st(4)", "st(5)", "st(6)", "st(7)", "memory", "cc");
}
}
HistoryNumericBindings native_history_numeric_bindings() {
    (void)entries();
    return {remainder, environment_atan2, animation::native_sse_atan2, power, arccosine, substring};
}
HistoryAnimationMovementEngine native_history_movement_engine(void* entry) {
    return {entry, entry ? displacement : nullptr};
}
animation::AnimationUpdateEngine native_history_animation_engine(void* entry) {
    return {entry, entry ? update_animation : nullptr};
}
} //namespace skeet::lagcomp
#else
namespace skeet::lagcomp {
HistoryNumericBindings native_history_numeric_bindings() {
    throw std::logic_error("Native history boundaries require Windows x86");
}
HistoryAnimationMovementEngine native_history_movement_engine(void*) {
    throw std::logic_error("Native history movement requires Windows x86");
}
animation::AnimationUpdateEngine native_history_animation_engine(void*) {
    throw std::logic_error("Native history animation requires Windows x86");
}
}
#endif

// ============================================================================
// Subcomponent: packet_capture.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
/* [removed duplicate normalize] */
}
LagTimeline history_timeline(const client::PlayerState& owner) noexcept {
    LagTimeline result;
    result.simulation_time = read<float>(&owner.native, 0x110);
    result.last_update_tick = read<std::int32_t>(&owner.native, 0x168);
    result.packet_head = read<std::uint32_t>(&owner.native, 0x234);
    result.packet_count = owner.native.field_238;
    std::memcpy(result.packets.data(), reinterpret_cast<const std::byte*>(&owner.native) + 0x23C,
                sizeof(result.packets));
    result.record_head = owner.history.first;
    result.record_count = owner.history.count;
    for (unsigned i = 0; i != 32; ++i)
        result.records[i] = owner.history.records[i];
    return result;
}
void publish_history_eye_angles(client::PlayerState& owner, void* player, bool resolver_branch,
                                HistoryPitchOverride override, const EntityStateRuntime& runtime) {
    if (resolver_branch) {
        owner.native.resolver_index = 0;
        if (!override.enabled)
            write(player, runtime.offsets.eye_angles, normalize(read<float>(&owner.native, 0x1A8)));
        write(player, runtime.offsets.eye_angles + 4, normalize(owner.native.reference_yaw));
    }
    if (override.enabled)
        write(player, runtime.offsets.eye_angles, float(override.pitch));
}
void capture_history_packet(client::PlayerState& owner, bool changed, void* player,
                            const EntityStateRuntime& runtime, std::uint32_t flags_offset) {
    if (!changed)
        return;
    write(&owner.native, 0x18C, read<std::uint32_t>(&owner.native, 0x188));
    write(&owner.native, 0x190, read<std::uint32_t>(&owner.native, 0x114));
    owner.native.flags &= ~0x100u;
    if (owner.native.field_238 != 6u)
        ++owner.native.field_238;
    const auto first = (read<std::uint32_t>(&owner.native, 0x234) + 5u) % 6u;
    write(&owner.native, 0x234, first);
    const auto offset = std::size_t(0x23C) + first * 0x20u;
    write(&owner.native, offset, time_to_tick(read<float>(&owner.native, 0x110), runtime.interval));
    write(&owner.native, offset + 4, read<Vector>(player, runtime.offsets.origin));
    write(&owner.native, offset + 0x10, read<std::uint32_t>(&owner.native, 0x524));
    write(&owner.native, offset + 0x14, read<std::uint32_t>(player, flags_offset));
    const auto receive = sample_receive_tick(runtime.engine, runtime.receive_interval);
    write(&owner.native, offset + 0x18, receive);
    write(&owner.native, offset + 0x1C, owner.native.reference_yaw);
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: packet_direction.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate epsilon] */
float component(const PacketTimingSample& sample, unsigned index) noexcept {
    return std::bit_cast<float>(sample.fields_04[index]);
}
Vector horizontal_difference(const PacketTimingSample& newer,
                             const PacketTimingSample& older) noexcept {
    return {component(newer, 0) - component(older, 0), component(newer, 1) - component(older, 1),
            0.0f};
}
Vector normalize(Vector value) noexcept {
    const float length =
        std::sqrt((value[0] * value[0] + value[1] * value[1]) + value[2] * value[2]);
    const float scale = 1.0f / (length + epsilon);
    for (auto& item : value)
        item = scale * item;
    return value;
}
/* [removed duplicate dot] */
}
bool update_history_packet_direction(HistoryPacketDirection& state, const LagTimeline& timeline,
                                     RecordRing& records, std::uint32_t flags,
                                     std::int32_t last_tick, float interval) noexcept {
    if ((flags & 1u) || timeline.packet_count < 2)
        return false;
    const auto first = timeline.packet_head % 6u;
    const auto second = (timeline.packet_head + 1u) % 6u;
    const auto third = (timeline.packet_head + 2u) % 6u;
    const auto& newest = timeline.packets[first];
    const auto& previous = timeline.packets[second];
    if (!(component(newest, 3) > 0.0005000000237487257f))
        return false;
    const bool older_stopped = timeline.packet_count >= 3 &&
                               0.0010000000474974513f > component(timeline.packets[third], 3);
    const bool previous_stopped = 0.0010000000474974513f > component(previous, 3);
    if (!previous_stopped && !older_stopped) {
        if (timeline.packet_count < 3)
            return false;
        const auto before = normalize(horizontal_difference(previous, timeline.packets[third]));
        const auto current = normalize(horizontal_difference(newest, previous));
        if (!(std::fabs(dot(before, current)) >= 0.9999904632568359f))
            return false;
        state.direction = current;
        state.time = float(last_tick) * interval;
        return true;
    }
    state.direction = normalize(horizontal_difference(newest, previous));
    state.time = float(last_tick) * interval;
    if (older_stopped && records.count) {
        auto& record = *records.records[records.first & 31u];
        if (!(record.header.flags & 0x10u)) {
            const auto& velocity = record.header.velocity;
            const auto direction = normalize({velocity[0], velocity[1], 0.0f});
            if (dot(direction, state.direction) >= 0.9961947202682495f) {
                const float speed =
                    std::sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1]);
                record.header.velocity[0] = speed * state.direction[0];
                record.header.velocity[1] = speed * state.direction[1];
            }
        }
    }
    return true;
}
bool update_history_packet_direction(client::PlayerState& owner, std::uint32_t flags,
                                     std::int32_t last_tick, float interval) noexcept {
    HistoryPacketDirection state;
    auto* native = reinterpret_cast<std::byte*>(&owner.native);
    std::memcpy(state.direction.data(), native + 0x16C, sizeof(Vector));
    std::memcpy(&state.time, native + 0x19C, sizeof(float));
    if (!update_history_packet_direction(state, history_timeline(owner), owner.history, flags,
                                         last_tick, interval))
        return false;
    std::memcpy(native + 0x16C, state.direction.data(), sizeof(Vector));
    std::memcpy(native + 0x19C, &state.time, sizeof(float));
    return true;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: prediction_scope.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
void* terminal(void* object) noexcept {
    for (;;) {
        void* parent;
        std::memcpy(&parent, static_cast<std::byte*>(object) + 0x1C, sizeof(parent));
        if (object == parent)
            return object;
        object = parent;
    }
}
std::uint32_t pointer_word(const void* cell) noexcept {
    std::uint32_t value;
    std::memcpy(&value, cell, sizeof value);
    return value;
}
void pointer_word(void* cell, std::uint32_t value) noexcept {
    std::memcpy(cell, &value, sizeof value);
}
}
std::uint32_t read_prediction_convar(void* object) noexcept {
    object = terminal(object);
    std::uint32_t result;
    std::memcpy(&result, static_cast<std::byte*>(object) + 0x2C, 4);
    return result ^ std::uint32_t(reinterpret_cast<std::uintptr_t>(object));
}
void write_prediction_convar(void* object, std::uint32_t bits) noexcept {
    object = terminal(object);
    bits ^= std::uint32_t(reinterpret_cast<std::uintptr_t>(object));
    std::memcpy(static_cast<std::byte*>(object) + 0x2C, &bits, 4);
    std::memcpy(static_cast<std::byte*>(object) + 0x3C, &bits, 4);
}
PredictionScope::PredictionScope(const PredictionTargets& input, std::uint32_t player,
                                 std::uint32_t command, std::uint32_t seed) noexcept
    : targets(input) {
    bytes[0] = *targets.in_prediction;
    *targets.in_prediction = 1;
    bytes[1] = *targets.first_time_predicted;
    *targets.first_time_predicted = 0;
    bytes[2] = *targets.command_predicted;
    *targets.command_predicted = 1;
    words[0] = *targets.random_seed;
    *targets.random_seed = seed;
    // These are actual engine pointer cells. Copy their32-bit representation
    // without accessing a C++ pointer object through an integer lvalue.
    words[1] = pointer_word(targets.prediction_player);
    pointer_word(targets.prediction_player, player);
    words[2] = pointer_word(targets.player_current_command);
    pointer_word(targets.player_current_command, command);
    words[3] = read_prediction_convar(targets.convar_aaa77b8a);
    write_prediction_convar(targets.convar_aaa77b8a, 0);
    words[4] = read_prediction_convar(targets.convar_de58ce34);
    write_prediction_convar(targets.convar_de58ce34, 0x7F7FFFFF);
    words[5] = *targets.field_a7f8_08;
    vector = *targets.player_field_6610746e;
}
void PredictionScope::restore() noexcept {
    if (targets.player_field_6610746e) {
        *targets.player_field_6610746e = vector;
        targets.player_field_6610746e = nullptr;
    }
    if (targets.field_a7f8_08) {
        *targets.field_a7f8_08 = words[5];
        targets.field_a7f8_08 = nullptr;
    }
    if (targets.convar_de58ce34) {
        write_prediction_convar(targets.convar_de58ce34, words[4]);
        targets.convar_de58ce34 = nullptr;
    }
    if (targets.convar_aaa77b8a) {
        write_prediction_convar(targets.convar_aaa77b8a, words[3]);
        targets.convar_aaa77b8a = nullptr;
    }
    if (targets.player_current_command) {
        pointer_word(targets.player_current_command, words[2]);
        targets.player_current_command = nullptr;
    }
    if (targets.prediction_player) {
        pointer_word(targets.prediction_player, words[1]);
        targets.prediction_player = nullptr;
    }
    if (targets.random_seed) {
        *targets.random_seed = words[0];
        targets.random_seed = nullptr;
    }
    if (targets.command_predicted) {
        *targets.command_predicted = bytes[2];
        targets.command_predicted = nullptr;
    }
    if (targets.first_time_predicted) {
        *targets.first_time_predicted = bytes[1];
        targets.first_time_predicted = nullptr;
    }
    if (targets.in_prediction) {
        *targets.in_prediction = bytes[0];
        targets.in_prediction = nullptr;
    }
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: receive_timing.cpp
// ============================================================================

namespace skeet::lagcomp {
std::int32_t sample_receive_tick(void* engine, const float& interval) {
#if defined(_M_IX86) || defined(__i386__)
    const auto received = engine::invoke<long double>(engine, 14);
#else
    const auto received = static_cast<long double>(engine::invoke<float>(engine, 14));
#endif
    const float rounded = static_cast<float>(received / static_cast<long double>(interval) + 0.5L);
    return _mm_cvtt_ss2si(_mm_set_ss(rounded));
}
std::int32_t sample_receive_tick(void* engine, std::byte* const& globals) {
#if defined(_M_IX86) || defined(__i386__)
    const auto received = engine::invoke<long double>(engine, 14);
#else
    const auto received = static_cast<long double>(engine::invoke<float>(engine, 14));
#endif
    float interval;
    std::memcpy(&interval, globals + 0x20, sizeof(interval));
    const float rounded = static_cast<float>(received / static_cast<long double>(interval) + 0.5L);
    return _mm_cvtt_ss2si(_mm_set_ss(rounded));
}
} //namespace skeet::lagcomp

// ============================================================================
// Subcomponent: record_selection.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
/* [removed duplicate add] */
/* [removed duplicate multiply] */
float distance_squared(const Vector& first, const Vector& second) noexcept {
    const float y = first[1] - second[1], x = first[0] - second[0], z = first[2] - second[2];
    return add(add(multiply(y, y), multiply(x, x)), multiply(z, z));
}
Vector angles_between(float older, float newer, float fraction, const RotationMath& math) {
    if (older == newer)
        return {0.0f, older, 0.0f};
    const auto first = history_angle_quaternion({0.0f, older, 0.0f}, math);
    const auto second = history_angle_quaternion({0.0f, newer, 0.0f}, math);
    const auto aligned = align_history_quaternion(first, second);
    return history_quaternion_angles(slerp_history_quaternion(first, aligned, fraction, math),
                                     math);
}
Matrix3x4 angle_matrix(const Vector& angles, const Vector& origin, const RotationMath& math) {
    /* [removed duplicate radians] */
    const float sy = math.sine(angles[1] * radians), cy = math.cosine(angles[1] * radians);
    const float sp = math.sine(angles[0] * radians), cp = math.cosine(angles[0] * radians);
    const float sr = math.sine(angles[2] * radians), cr = math.cosine(angles[2] * radians);
    const float srcy = sr * cy, srsy = sr * sy, crcy = cr * cy, crsy = cr * sy;
    return {{{cp * cy, srcy * sp - crsy, crcy * sp + srsy, origin[0]},
             {cp * sy, srsy * sp + crcy, crsy * sp - srcy, origin[1]},
             {-sp, sr * cp, cr * cp, origin[2]}}};
}
/* [removed duplicate concatenate] */
Matrix3x4 correction_matrix(float old_yaw, const Vector& old_origin, const Vector& angles,
                            const Vector& origin, const RotationMath& math,
                            RecordInterpolationScratch& scratch, bool alternative) {
    const auto old = angle_matrix({0.0f, old_yaw, 0.0f}, old_origin, math);
    Matrix3x4 inverse;
    for (unsigned row = 0; row != 3; ++row)
        for (unsigned column = 0; column != 3; ++column)
            inverse.rows[row][column] = old.rows[column][row];
    (void)alternative;
    // Translation belongs to the freshly transposed rotation.
    for (unsigned row = 0; row < 3; ++row)
        inverse.rows[row][3] =
            -(inverse.rows[row][0] * old_origin[0] + inverse.rows[row][1] * old_origin[1] +
              inverse.rows[row][2] * old_origin[2]);
    for (unsigned axis = 0; axis != 3; ++axis) {
        scratch.inverse_y[axis] = std::bit_cast<std::uint32_t>(inverse.rows[1][axis]);
        scratch.inverse_z[axis] = std::bit_cast<std::uint32_t>(inverse.rows[2][axis]);
    }
    return concatenate(angle_matrix(angles, origin, math), inverse);
}
void rebase(HitboxMatrices& destination, const HitboxMatrices& source,
            const Matrix3x4& correction) {
    for (unsigned index = 0; index != 19; ++index)
        destination[index] = concatenate(correction, source[index]);
}
}
RecordChoice choose_history_record(const RecordRing& ring, const Vector& cached_origin,
                                   std::int32_t requested_tick, float interval) noexcept {
    RecordChoice choice;
    auto previous = cached_origin;
    const float requested_time = float(requested_tick) * interval;
    for (std::uint32_t ordinal = 0; ordinal != ring.count; ++ordinal) {
        const auto& record = ring.at(ordinal);
        if (distance_squared(record.header.origin, previous) > 4096.0f)
            return {};
        previous = record.header.origin;
        choice = {&record, ordinal};
        if (requested_time >= float(record.header.simulation_tick) * interval)
            break;
    }
    return choice;
}
RecordInterpolation interpolate_selected_record(LagRecord& destination, const RecordRing& ring,
                                                const RecordChoice& choice,
                                                std::int32_t requested_tick,
                                                const HistoryTickWindow& window, float interval,
                                                const RotationMath& math,
                                                RecordInterpolationScratch& scratch) {
    const auto* older = history_interpolation_source(ring, choice.ordinal, *choice.record,
                                                     requested_tick, 0, window);
    if (older == nullptr)
        return {};
    const auto& newer = *choice.record;
    const float requested_time = float(window.last) * interval;
    destination.header.simulation_tick = time_to_tick(requested_time, interval);
    const float old_time = float(older->header.simulation_tick) * interval;
    const float fraction = _mm_cvtss_f32(_mm_min_ss(
        _mm_max_ss(_mm_set_ss((requested_time - old_time) /
                              (float(newer.header.simulation_tick) * interval - old_time)),
                   _mm_setzero_ps()),
        _mm_set_ss(1.0f)));
    const auto angles =
        angles_between(older->header.animation_yaw, newer.header.animation_yaw, fraction, math);
    destination.header.animation_yaw = angles[1];
    destination.header.player_flags = newer.header.player_flags;
    for (unsigned axis = 0; axis != 3; ++axis) {
        destination.header.origin[axis] =
            (newer.header.origin[axis] - older->header.origin[axis]) * fraction +
            older->header.origin[axis];
        destination.header.velocity[axis] =
            (newer.header.velocity[axis] - older->header.velocity[axis]) * fraction +
            older->header.velocity[axis];
    }
    if ((older->header.flags & 4U) != 0) {
        rebase(destination.matrices, older->matrices,
               correction_matrix(older->header.animation_yaw, older->header.origin, angles,
                                 destination.header.origin, math, scratch, false));
        if ((older->header.flags & newer.header.flags & 0x80U) == 0)
            destination.header.flags &= ~0x80U;
        else {
            for (unsigned variant = 0; variant != 3; ++variant) {
                const auto old_yaw = std::bit_cast<float>(older->alternatives[variant].state[0]);
                const auto new_yaw = std::bit_cast<float>(newer.alternatives[variant].state[0]);
                const auto variant_angles = angles_between(old_yaw, new_yaw, fraction, math);
                rebase(destination.alternatives[variant].matrices,
                       older->alternatives[variant].matrices,
                       correction_matrix(old_yaw, older->header.origin, variant_angles,
                                         destination.header.origin, math, scratch, true));
            }
        }
    }
    return {true, fraction, angles};
}
void refresh_record_attachments(client::PlayerStateCache& players, void* target, LagRecord& record,
                                const EntityPreparationContext& preparation,
                                const RecordAttachmentContext& attachments) {
    const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(target) + 8, 10);
    auto& cached = players.get(std::uint32_t(index))->native;
    const auto& offsets = preparation.entity.offsets;
    const auto old_432 = read<std::uint32_t>(target, offsets.field_432b1f5b);
    const auto old_0d = read<std::uint32_t>(target, offsets.field_0d560fac);
    const auto old_angles = read<std::array<std::uint32_t, 2>>(target, offsets.eye_angles);
    prepare_entity_angles(players, target, record, true, preparation);
    record.header.field_50 = read<float>(target, offsets.eye_angles + 4);
    auto* collision = engine::invoke<void*>(target, 3);
    auto* maximum = engine::invoke<Vector*>(collision, 2);
    const auto old_height = (*maximum)[2];
    (*maximum)[2] = record.header.collision_max_z;
    const auto old_duck = read<std::uint32_t>(target, offsets.duck_amount);
    write(target, offsets.duck_amount, cached.cached_duck_amount);
    const auto old_6e = read<std::uint32_t>(target, attachments.field_6e9852d6);
    const auto old_2a = read<Vector>(target, attachments.field_2a671b42);
    write(target, attachments.field_2a671b42, record.header.origin);
    const auto old_4e = read<Vector>(target, attachments.field_4e66bb34);
    write(target, attachments.field_4e66bb34, record.header.origin);
    const auto old_flags = read<std::uint32_t>(target, attachments.field_83a6cf2b);
    write(target, attachments.field_83a6cf2b, old_flags & 0xFFFFF7FFu);
    if (record.header.flags & 4u)
        legit::refresh_hitbox_attachments(target, attachments.model.decoded(), record.matrices,
                                          attachments.scratch, offsets.bone_accessor,
                                          attachments.update);
    if (record.header.flags & 0x80u)
        for (auto& alternative : record.alternatives)
            legit::refresh_hitbox_attachments(target, attachments.model.decoded(),
                                              alternative.matrices, attachments.scratch,
                                              offsets.bone_accessor, attachments.update);
    write(target, attachments.field_83a6cf2b, old_flags);
    write(target, attachments.field_4e66bb34, old_4e);
    write(target, attachments.field_2a671b42, old_2a);
    write(target, attachments.field_6e9852d6, old_6e);
    write(target, offsets.duck_amount, old_duck);
    (*maximum)[2] = old_height;
    write(target, offsets.eye_angles, old_angles);
    write(target, offsets.field_0d560fac, old_0d);
    write(target, offsets.field_432b1f5b, old_432);
}
bool select_and_apply_history_record(EntityRecordScope& scope,
                                     const RecordSelectionContext& context,
                                     std::int32_t requested_tick, std::uint32_t unused_options) {
    (void)unused_options; // No read of EBP+10 in the original entry.
    const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(scope.player) + 8, 10);
    auto& cached = *context.players.get(std::uint32_t(index));
    scope.flags = 0;
    const auto choice = choose_history_record(cached.history, cached.native.cached_origin,
                                              requested_tick, context.preparation.entity.interval);
    if (!choice.record)
        return false;
    const auto& runtime = context.preparation.entity;
    if (scope.snapshot.state.capture_frame != runtime.frame)
        runtime.capture_snapshot(scope.snapshot, scope.player);
    std::memcpy(&scope.record, choice.record, sizeof scope.record);
    const auto interpolated = interpolate_selected_record(
        scope.record, cached.history, choice, requested_tick, history_tick_window(context.timing),
        runtime.interval, context.rotation, context.interpolation_scratch);
    if (interpolated.used)
        scope.flags |= 0x10u;
    const auto& model = context.attachments.model.decoded();
    const auto hitboxes =
        model.studio_header ? model.hitbox_bones : std::span<const std::uint32_t>{};
    runtime.apply_record(scope.player, scope.record, hitboxes, context.current_bone_counter);
    // Generic43344C0C always has the nonnull native timing context forwarded
    // through43327659 to4341DDA0.
    auto preparation = context.preparation;
    preparation.current_timing = &context.timing;
    refresh_record_attachments(context.players, scope.player, scope.record, preparation,
                               context.attachments);
    if (scope.record.header.flags & 4u) {
        const auto& offsets = runtime.offsets;
        auto* bones = read<Matrix3x4*>(scope.player, offsets.bone_cache);
        scatter_record_hitbox_matrices(scope.record.matrices, hitboxes, bones);
        write(scope.player, offsets.bone_accessor + 8, 0xFFF00u);
        write(scope.player, offsets.bone_accessor + 12, 0xFFF00u);
        if (context.current_bone_counter)
            write(scope.player, offsets.recent_bone_counter, *context.current_bone_counter);
    }
    return true;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: record_selection_live.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
}
RecordSelectionLiveContext live_record_selection_context(const RecordSelectionContext& source,
                                                         const animation::BoneRuntime& bones) {
    const auto& old = source.attachments;
    return {source.players,
            source.preparation,
            source.timing,
            source.rotation,
            source.interpolation_scratch,
            {bones, old.scratch, old.field_6e9852d6, old.field_2a671b42, old.field_4e66bb34,
             old.field_83a6cf2b},
            source.current_bone_counter};
}
bool select_and_apply_history_record_live(EntityRecordScope& scope,
                                          const RecordSelectionLiveContext& context,
                                          std::int32_t requested, std::uint32_t unused_options) {
    (void)unused_options; // Native never reads its third stack argument.
    const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(scope.player) + 8, 10);
    auto& cached = *context.players.get(std::uint32_t(index));
    scope.flags = 0;
    const auto& runtime = context.preparation.entity;
    const auto choice = choose_history_record(cached.history, cached.native.cached_origin,
                                              requested, runtime.interval);
    if (!choice.record)
        return false;
    if (scope.snapshot.state.capture_frame != runtime.frame)
        runtime.capture_snapshot(scope.snapshot, scope.player);
    std::memcpy(&scope.record, choice.record, sizeof scope.record);
    const auto interpolated = interpolate_selected_record(
        scope.record, cached.history, choice, requested, history_tick_window(context.timing),
        runtime.interval, context.rotation, context.interpolation_scratch);
    if (interpolated.used)
        scope.flags |= 0x10u;
    const auto& bones = context.attachments.bones;
    const EntityHitboxModelOffsets model{bones.fields.studio_wrapper_32e4daf1,
                                         bones.fields.hitbox_set_c2b6fb10};
    runtime.apply_record_live_model(scope.player, scope.record, model,
                                    context.current_bone_counter);
    // Entry clears flags; interpolation only sets10. Thus the native60 rebuild
    // arm is unreachable here, regardless of the unused caller options.
    auto preparation = context.preparation;
    preparation.current_timing = &context.timing;
    refresh_record_attachments(context.players, scope.player, scope.record, preparation,
                               context.attachments);
    if (scope.record.header.flags & 4u) {
        const auto& offsets = runtime.offsets;
        auto* cache = read<Matrix3x4*>(scope.player, offsets.bone_cache);
        bones.scatter_hitbox_bone_matrices(scope.player, scope.record.matrices, cache);
        write(scope.player, offsets.bone_accessor + 8, 0xFFF00u);
        write(scope.player, offsets.bone_accessor + 12, 0xFFF00u);
        if (context.current_bone_counter)
            write(scope.player, offsets.recent_bone_counter, *context.current_bone_counter);
    }
    return true;
}
bool select_and_apply_history_record(EntityRecordScope& scope,
                                     const RecordSelectionContext& context, std::int32_t requested,
                                     std::uint32_t unused_options,
                                     const animation::BoneRuntime& bones) {
    const auto live = live_record_selection_context(context, bones);
    return select_and_apply_history_record_live(scope, live, requested, unused_options);
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: records.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
void copy_words(void* destination, const void* source, std::size_t count) noexcept {
    auto* out = static_cast<std::byte*>(destination);
    const auto* in = static_cast<const std::byte*>(source);
    for (std::size_t i = 0; i != count; ++i) {
        std::uint32_t word;
        std::memcpy(&word, in + i * 4, 4);
        std::memcpy(out + i * 4, &word, 4);
    }
}
void copy_matrices(HitboxMatrices& destination, const HitboxMatrices& source) noexcept {
    for (std::size_t i = 0; i != 19; ++i) {
        // The native loop loads all three rows before storing this matrix.
        const auto a = _mm_loadu_ps(source[i].rows[0]);
        const auto b = _mm_loadu_ps(source[i].rows[1]);
        const auto c = _mm_loadu_ps(source[i].rows[2]);
        _mm_storeu_ps(destination[i].rows[0], a);
        _mm_storeu_ps(destination[i].rows[1], b);
        _mm_storeu_ps(destination[i].rows[2], c);
    }
}
}
void copy_lag_record(LagRecord& destination, const LagRecord& source) noexcept {
    copy_words(&destination.header, &source.header, 0x16);
    destination.header.resolver_index = source.header.resolver_index;
    destination.header.resolver_mode = source.header.resolver_mode;
    destination.header.resolver_state = source.header.resolver_state;
    destination.header.flags = source.header.flags;
    copy_words(&destination.animation, &source.animation, 0x74);
    copy_matrices(destination.matrices, source.matrices);
    for (std::size_t i = 0; i != 3; ++i) {
        destination.alternatives[i].state[0] = source.alternatives[i].state[0];
        destination.alternatives[i].state[1] = source.alternatives[i].state[1];
        copy_matrices(destination.alternatives[i].matrices, source.alternatives[i].matrices);
    }
}
void assign_evaluation(HistoryEvaluation& destination, const HistoryEvaluation& source) noexcept {
    copy_words(destination.words.data(), source.words.data(), 0x22);
    copy_words(destination.state.words.data(), source.state.words.data(), 0x3B);
    copy_lag_record(destination.state.record, source.state.record);
}
void copy_construct_evaluation(HistoryEvaluation& destination,
                               const HistoryEvaluation& source) noexcept {
    copy_words(destination.words.data(), source.words.data(), 0x22);
    std::memcpy(&destination.state, &source.state, sizeof(EvaluationState));
}
void capture_animation(AnimationState& destination, const void* layers, std::int32_t count,
                       const float* pose_parameters) noexcept {
    destination.layer_count = count < 13 ? count : 13;
    const auto* source = static_cast<const std::byte*>(layers);
    for (std::int32_t i = 0; i < destination.layer_count; ++i)
        copy_words(&destination.layers[std::size_t(i)], source + std::size_t(i) * 0x38 + 0x14, 7);
    copy_words(destination.pose_parameters.data(), pose_parameters, 24);
}
void restore_animation(const AnimationState& source, void* layers, std::int32_t count,
                       float* pose_parameters) noexcept {
    if (source.layer_count > 0 && source.layer_count == count) {
        auto* destination = static_cast<std::byte*>(layers);
        for (std::int32_t i = 0; i < source.layer_count; ++i)
            copy_words(destination + std::size_t(i) * 0x38 + 0x14, &source.layers[std::size_t(i)],
                       7);
    }
    copy_words(pose_parameters, source.pose_parameters.data(), 24);
}
std::int32_t time_to_tick(float time, float interval) noexcept {
    return _mm_cvtt_ss2si(
        _mm_add_ss(_mm_div_ss(_mm_set_ss(time), _mm_set_ss(interval)), _mm_set_ss(0.5f)));
}
std::int32_t receive_time_to_tick(float time, float interval) noexcept {
    const float rounded = static_cast<float>(
        static_cast<long double>(time) / static_cast<long double>(interval) + 0.5L);
    return _mm_cvtt_ss2si(_mm_set_ss(rounded));
}
std::int32_t corrected_command_tick(std::int32_t tick, const std::int32_t* previous,
                                    const std::int32_t* current) noexcept {
    if (previous && current) {
        const auto delta = std::uint32_t(*current) - std::uint32_t(*previous);
        if (delta - 1u < 9u)
            tick = std::bit_cast<std::int32_t>(std::uint32_t(tick) + delta);
    }
    return tick;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: resolver_detection.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate normalize] */
}
bool newest_packet_timing_discontinuity(const LagTimeline& timeline) noexcept {
    if (!timeline.packet_count)
        return false;
    const auto& packet = timeline.packets[timeline.packet_head % 6u];
    const auto last = std::bit_cast<std::int32_t>(std::uint32_t(packet.field_18) - 1u);
    return packet.field_00 < last;
}
bool update_resolver_detection(ResolverDetectionState& state,
                               const ResolverDetectionInput& input) noexcept {
    if (state.enabled)
        return true;
    if (newest_packet_timing_discontinuity(input.packets)) {
        state.enabled = true;
        return true;
    }
    if (input.simulation_ticks > 1) {
        const auto delta = normalize(input.previous_yaw_118 - input.observed_yaw_1ac);
        const float elapsed = float(input.simulation_ticks) * input.interval;
        if (std::fabs(delta) > 40.0f)
            state.yaw_duration_1c0 += elapsed;
        else
            state.yaw_duration_1c0 = 0.0f;
        if (input.pitch > 70.0f)
            state.pitch_duration_178 += elapsed;
        else
            state.pitch_duration_178 = 0.0f;
    } else
        state.yaw_duration_1c0 = state.pitch_duration_178 = 0.0f;
    if (state.yaw_duration_1c0 > 1.2f || state.pitch_duration_178 > 1.2f) {
        state.yaw_duration_1c0 = state.pitch_duration_178 = 0.0f;
        state.enabled = true;
        if (!state.field_a6) {
            const auto delta = normalize(input.previous_yaw_118 - input.observed_yaw_1ac);
            const std::uint8_t attempt = delta < -30.0f ? 2 : 1;
            state.selection.attempt_by_mode[1] = state.selection.attempt_by_mode[2] = attempt;
            state.selection.status = 0;
        }
    }
    return state.enabled;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: timeline.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate add] */
/* [removed duplicate sub] */
/* [removed duplicate cap] */
}
bool packet_timing_discontinuity(const LagTimeline& timeline) noexcept {
    for (std::uint32_t i = 0; i != timeline.packet_count; ++i) {
        const auto& sample = timeline.packets[(timeline.packet_head + i) % 6u];
        if (sample.field_00 < sub(sample.field_18, 1))
            return true;
    }
    return false;
}
std::int32_t prediction_tick_limit(bool available, bool first, bool second, std::int32_t mode,
                                   std::int32_t configured) noexcept {
    if (available) {
        if (!first)
            return configured;
        if (second && mode == 1)
            return 3;
    }
    return 7;
}
std::int32_t estimate_missing_ticks(const LagTimeline& timeline, const MissingTickTiming& timing) {
    const auto now = add(timing.tick_d4, timing.tick_18c);
    const auto simulation = time_to_tick(timeline.simulation_time, timing.interval);
    const auto distance = sub(now, simulation);
    const auto sign = distance < 0 ? UINT32_MAX : 0u;
    const auto absolute = std::bit_cast<std::int32_t>((std::uint32_t(distance) ^ sign) - sign);
    if (absolute >= 128)
        return 0;
    std::array<std::int32_t, 10> pattern{};
    std::size_t count = 0;
    bool simple = timeline.record_count < 2 || timeline.packet_count < 2;
    if (!simple && simulation < sub(timeline.last_update_tick, 1) &&
        simulation < timeline.at(0).header.simulation_tick)
        simple = true;
    if (!simple && packet_timing_discontinuity(timeline))
        simple = true;
    if (!simple && (timeline.at(0).header.flags & 0x200u))
        simple = true;
    if (!simple) {
        const auto head = timeline.packet_head % 6u;
        const auto next = (timeline.packet_head + 1u) % 6u;
        if (sub(timeline.packets[head].field_18, timeline.packets[next].field_18) < 4 &&
            sub(timeline.at(0).header.receive_tick, timeline.at(1).header.receive_tick) > 7)
            simple = true;
    }
    if (simple)
        pattern[count++] = 2;
    else {
        const LagRecord* previous = nullptr;
        for (std::uint32_t age = 1; age < timeline.record_count; ++age) {
            const auto& record = timeline.at(age);
            if (previous && !(previous->header.flags & 8u)) {
                pattern[count++] =
                    cap(sub(previous->header.simulation_tick, record.header.simulation_tick),
                        timing.prediction_limit);
                if (count == 10)
                    break;
            }
            if (record.header.flags & 0x200u)
                break;
            previous = &record;
        }
        const auto latest_gap =
            cap(sub(timeline.at(0).header.simulation_tick, timeline.at(1).header.simulation_tick),
                timing.prediction_limit);
        const float received = engine::invoke<float>(timing.engine, 14);
        const auto receive_tick = receive_time_to_tick(received, timing.receive_interval);
        const auto arrival_gap = cap(add(sub(receive_tick, timeline.at(0).header.receive_tick), 1),
                                     timing.prediction_limit);
        if (count < 10)
            pattern[count++] = latest_gap >= arrival_gap ? latest_gap : arrival_gap;
    }
    std::uint32_t index = 0;
    auto step = pattern[0];
    auto position = add(timeline.last_update_tick, step);
    std::int32_t total = 0;
    while (position <= now) {
        if (position == now) {
            auto* networkable = static_cast<std::byte*>(timing.player) + 8;
            const auto player_index = engine::invoke<std::int32_t>(networkable, 10);
            if (engine::invoke<std::int32_t>(timing.engine, 12) < player_index)
                break;
        }
        total = add(total, step);
        ++index;
        step = pattern[index % count];
        position = add(position, step);
    }
    return total;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: update_capture.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
/* [removed duplicate write] */
}
void capture_history_display_metadata(client::PlayerState& owner, void* player,
                                      const EntityStateRuntime& runtime,
                                      const HistoryPreludeOffsets& offsets) {
    owner.geometry.last_live_tick = sample_receive_tick(runtime.engine, runtime.receive_interval);
    if (read<std::uint8_t>(player, offsets.life_state_9d920e35))
        return;
    const auto handle = read<std::uint32_t>(player, runtime.offsets.weapon_handle);
    if (handle == UINT32_MAX)
        return;
    if (auto* weapon = engine::invoke<void*>(runtime.entity_list, 4, handle))
        owner.cached_definition = read<std::uint16_t>(weapon, offsets.weapon_definition_5c9f0fbb);
}
void cache_history_entity_fields(client::PlayerState& owner, void* player,
                                 const EntityStateRuntime& runtime) {
    write(&owner.native, 0x13C, read<Vector>(player, runtime.offsets.origin));
    auto* collideable = engine::invoke<void*>(player, 3);
    owner.cached_target_height = (*engine::invoke<const Vector*>(collideable, 2))[2];
    write(&owner.native, 0x134, read<float>(player, runtime.offsets.duck_amount));
    write(&owner.native, 0x138, read<std::uint32_t>(player, runtime.offsets.field_fb7e3404));
}
CachedHistoryFrame prepare_cached_history_frame(client::PlayerState& owner, RecordPool& pool,
                                                void* player, bool is_local,
                                                const EntityStateRuntime& runtime,
                                                const HistoryPreludeOffsets& offsets) {
    cache_history_entity_fields(owner, player, runtime);
    if (read<std::uint8_t>(player, offsets.life_state_9d920e35) ||
        read<std::uint8_t>(player, offsets.suppressed_cde290d2)) {
        owner.native.field_238 = 0;
        write(&owner.native, 0x234, std::uint32_t(0));
        clear_history_records(owner.history, pool);
        owner.publish_history();
        return {};
    }
    const auto receive = sample_receive_tick(runtime.engine, runtime.receive_interval);
    if (!is_local) {
        const SameTickHistoryState saved{
            read<std::int32_t>(&owner.native, 0x1E4), read<std::int32_t>(&owner.native, 0x1E0),
            read<std::int32_t>(&owner.native, 0x1DC), read<Vector>(&owner.native, 0x1C8)};
        float history = read<float>(&owner.native, 0x110),
              simulation = read<float>(player, runtime.offsets.simulation_time);
        auto velocity = read<Vector>(player, runtime.offsets.velocity);
        const auto writes = restore_same_tick_history_fields(saved, receive, runtime.interval,
                                                             history, simulation, velocity);
        if (writes.simulation) {
            write(&owner.native, 0x110, history);
            write(player, runtime.offsets.simulation_time, simulation);
        }
        if (writes.velocity)
            write(player, runtime.offsets.velocity, velocity);
        capture_history_display_metadata(owner, player, runtime, offsets);
    }
    auto* animation = read<void*>(player, runtime.offsets.animation_state);
    const float fraction = animation ? read<float>(animation, 0x11C) : 1.0f;
    const auto ticks = simulation_tick_span(read<float>(player, offsets.previous_time_1599541f),
                                            read<float>(player, runtime.offsets.simulation_time),
                                            runtime.interval);
    return {true, receive, animation, fraction, ticks};
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: update_timing.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate add] */
/* [removed duplicate sub] */
/* [removed duplicate absolute] */
}
SimulationTickSpan simulation_tick_span(float previous, float current, float interval) noexcept {
    auto first = time_to_tick(previous, interval);
    const auto last = time_to_tick(current, interval);
    auto count = sub(last, first);
    if (count < 0)
        count = 0;
    else if (count > 16)
        count = 16;
    if (add(first, count) != last)
        first = sub(last, count);
    return {first, last, count};
}
SameTickRestoreWrites restore_same_tick_history_fields(const SameTickHistoryState& state,
                                                       std::int32_t receive, float interval,
                                                       float& history_time, float& player_time,
                                                       Vector& velocity) noexcept {
    SameTickRestoreWrites writes;
    if (state.simulation_override_receive_tick == receive &&
        absolute(sub(state.simulation_override_tick, receive)) <= 20) {
        const float time = float(sub(state.simulation_override_tick, 1)) * interval;
        history_time = time;
        player_time = time;
        writes.simulation = true;
    }
    if (state.velocity_receive_tick == receive) {
        velocity = state.velocity;
        writes.velocity = true;
    }
    return writes;
}
std::uint32_t initial_history_update_flags(const UpdateDiscontinuity& input) noexcept {
    std::uint32_t flags = input.saved_receive_tick_1a4 == input.receive_tick ||
                          (input.entity_effects_38ca5228 & 8u) ||
                          input.field_043fa575 != input.field_334b460c;
    if (input.velocity_receive_tick_1dc == input.next_receive_tick)
        flags |= 8u;
    return flags;
}
bool history_weapon_is_firearm(std::uint16_t identifier) noexcept {
    constexpr std::array<std::uint32_t, 3> mask{0xFF8B6F9Eu, 0xB00001DFu, 1};
    return identifier < 65u && (mask[identifier >> 5] & (1u << (identifier & 31u)));
}
std::int32_t update_history_shot_flags(std::uint32_t& owner, std::uint32_t& update,
                                       SimulationTickSpan span, HistoryWeaponShot weapon,
                                       float interval) noexcept {
    if (span.count <= 0)
        return -1;
    owner = (owner & 0xFFFFFFEFu) | ((owner >> 1) & 0x10u);
    if (weapon.available) {
        const auto tick = time_to_tick(weapon.time, interval);
        owner =
            (owner & 0xFFFFFFFEu) | std::uint32_t(tick == sub(span.last, 1) || tick == span.last);
        if (history_weapon_is_firearm(weapon.identifier) && tick > span.first &&
            tick <= span.last) {
            owner |= 0x20u;
            update |= 4u;
            return tick;
        }
    }
    owner &= 0xFFFFFFDFu;
    return -1;
}
void update_history_collision_transition(CollisionTransition& state, SimulationTickSpan span,
                                         float current_height, float origin_z, float previous_time,
                                         float interval) noexcept {
    if (state.received_updates_1d4 > 1 && span.count &&
        state.saved_collision_height_11c != current_height) {
        state.field_12c = origin_z + state.saved_collision_height_11c;
        state.field_148 = previous_time + interval;
    }
}
bool history_animation_layers_changed(
    const std::array<AnimationLayerState, 13>& current,
    const std::array<AnimationLayerState, 13>& previous) noexcept {
    for (std::size_t i = 0; i != 13; ++i)
        for (std::size_t j = 0; j != 7; ++j)
            if (current[i].words[j] != previous[i].words[j])
                return true;
    return false;
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: velocity.cpp
// ============================================================================

namespace skeet::lagcomp {
void adjust_capture_velocity(Vector& velocity, float blend, float maximum_speed,
                             float interpolated_factor) noexcept {
    /* [removed duplicate epsilon] */
    float limit;
    if (blend <= 0.0f) {
        limit = maximum_speed * std::bit_cast<float>(0x3F666666u);
        const float speed = std::sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1]);
        if (!(limit > speed))
            return;
        const float denominator =
            std::sqrt(velocity[1] * velocity[1] + velocity[0] * velocity[0]) + epsilon;
        const float scale = limit / denominator;
        velocity[0] *= scale;
        velocity[1] = scale * velocity[1];
    } else if (blend >= 1.0f) {
        limit = maximum_speed * std::bit_cast<float>(0x3F0CCCCDu);
        const float speed = std::sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1]);
        if (!(speed > limit))
            return;
        const float denominator =
            std::sqrt(velocity[1] * velocity[1] + velocity[0] * velocity[0]) + epsilon;
        const float scale = limit / denominator;
        velocity[0] *= scale;
        velocity[1] *= scale;
    } else {
        limit = maximum_speed * interpolated_factor;
        const float denominator =
            std::sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1]) + epsilon;
        const float scale = limit / denominator;
        velocity[0] *= scale;
        velocity[1] *= scale;
    }
}
} // namespace skeet::lagcomp

// ============================================================================
// Subcomponent: velocity_validation.cpp
// ============================================================================

namespace skeet::lagcomp {
namespace {
/* [removed duplicate read] */
}

bool validate_velocity_component(float& value, float maximum) noexcept {
    const auto component = _mm_load_ss(&value);
    // The original UCOMISS self-test is quiet for a quiet NaN. CMPUNORDSS has
    // the same NaN/denormal handling, including the signaling-NaN exception.
    if (_mm_movemask_ps(_mm_cmpunord_ss(component, component)) & 1) {
        value = 0.0f;
        return false;
    }

    const auto upper = _mm_set_ss(maximum);
    if (_mm_comigt_ss(component, upper)) {
        _mm_store_ss(&value, upper);
        return false;
    }

    // XORPS at 433E4FA5 flips the sign, including signed zero, infinity and
    // NaN payloads. Neither comparison accepts an unordered operand as greater.
    const auto lower = _mm_xor_ps(upper, _mm_set_ss(-0.0f));
    if (_mm_comigt_ss(lower, component)) {
        _mm_store_ss(&value, lower);
        return false;
    }
    return true;
}

bool validate_velocity(Vector& velocity, float maximum) noexcept {
    const bool x_valid = validate_velocity_component(velocity[0], maximum);
    const bool y_valid = validate_velocity_component(velocity[1], maximum);
    const bool z_valid = validate_velocity_component(velocity[2], maximum);
    return x_valid & y_valid & z_valid;
}

bool validate_velocity(Vector& velocity, const void* maximum_convar) noexcept {
    for (;;) {
        const auto* parent = read<const void*>(maximum_convar, 0x1C);
        if (parent == maximum_convar)
            break;
        maximum_convar = parent;
    }
    const auto encoded = read<std::uint32_t>(maximum_convar, 0x2C);
    const auto address = std::uint32_t(reinterpret_cast<std::uintptr_t>(maximum_convar));
    return validate_velocity(velocity, std::bit_cast<float>(encoded ^ address));
}
} // namespace skeet::lagcomp
