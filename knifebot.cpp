// Knifebot & Zeusbot Subsystem

#include "util/address.h"
using skeet::field;
using skeet::store;
using skeet::add;
using skeet::subtract;

#include "skeet/misc/knife_aim.hpp"
#include "skeet/misc/knife_melee.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/rage/firing.hpp"
#include <bit>
#include <cstring>
#include <xmmintrin.h>
#include <cmath>
#include "skeet/misc/knife_owner.hpp"
#include "skeet/misc/command_owner.hpp"
#include <array>
#include <new>
#include "skeet/misc/knife_record.hpp"
#include "skeet/lagcomp/packet_capture.hpp"
#include "skeet/lagcomp/live_record_attachments.hpp"
#include "skeet/lagcomp/live_timeline.hpp"
#include "skeet/movement/extrapolation.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/misc/knife_runtime.hpp"
#include "skeet/rage/acquisition.hpp"
#include "skeet/client/command_orchestrator.hpp"
#include "skeet/lagcomp/record_selection_live.hpp"
#include "skeet/misc/zeus.hpp"
#include "skeet/rage/targeting_commit.hpp"
#include "skeet/misc/zeus_owner.hpp"


// ============================================================================
// Subcomponent: knife_aim.cpp
// ============================================================================

namespace skeet::misc {
namespace {
// field from util/address.h
float bound_component(float value, float minimum, float maximum) noexcept {
    return _mm_cvtss_f32(
        _mm_min_ss(_mm_max_ss(_mm_set_ss(value), _mm_set_ss(minimum)), _mm_set_ss(maximum)));
}
float add(float first, float second) noexcept {
    return _mm_cvtss_f32(_mm_add_ss(_mm_set_ss(first), _mm_set_ss(second)));
}
float subtract(float first, float second) noexcept {
    return _mm_cvtss_f32(_mm_sub_ss(_mm_set_ss(first), _mm_set_ss(second)));
}
float multiply(float first, float second) noexcept {
    return _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(first), _mm_set_ss(second)));
}
float square_root(float value) noexcept {
    return _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(value)));
}
float inverse_length(float length) noexcept {
    return _mm_cvtss_f32(_mm_div_ss(_mm_set_ss(1.0f), _mm_set_ss(add(length, 0x1p-23f))));
}
float squared_length(const RageVector& vector) noexcept {
    return add(add(multiply(vector[0], vector[0]), multiply(vector[1], vector[1])),
               multiply(vector[2], vector[2]));
}
MovementAngles command_angles(const client::Command& command) noexcept {
    return {command.angles[0], command.angles[1], command.angles[2]};
}
} // namespace

void evaluate_knife_direction(std::uint32_t modes, const client::CommandFrame& frame,
                              void* local_player, void* target, bool low_health,
                              RageVector& chosen_direction, std::uint32_t& best_score,
                              const KnifeMeleeBindings& bindings) {
    auto* collideable = engine::invoke<void*>(target, 3);
    const auto* maximum = engine::invoke<const RageVector*>(collideable, 2);
    collideable = engine::invoke<void*>(target, 3);
    const auto* minimum = engine::invoke<const RageVector*>(collideable, 1);
    const auto origin = field<RageVector>(target, bindings.target_origin);
    RageVector direction;
    for (unsigned axis = 0; axis < 3; ++axis) {
        const float nearest = bound_component(subtract(frame.origin_dc[axis], origin[axis]),
                                              (*minimum)[axis], (*maximum)[axis]);
        direction[axis] = subtract(add(origin[axis], nearest), frame.origin_dc[axis]);
    }
    const float distance = square_root(squared_length(direction));
    const float inverse = inverse_length(distance);
    for (auto& component : direction)
        component = multiply(inverse, component);
    if (!(distance <= 100.0f))
        return;
    if (!find_knife_trace_direction(local_player, frame.origin_dc, target, direction, bindings))
        return;

    const bool swing_enabled = (modes & 1U) != 0;
    const bool stab_enabled = (modes & 2U) != 0;
    auto swing_score =
        swing_enabled ? score_knife_attack(frame, local_player, direction, false, bindings) : 0U;
    const auto stab_score =
        stab_enabled ? score_knife_attack(frame, local_player, direction, true, bindings) : 0U;

    if ((frame.flags & 0x400U) && swing_enabled) {
        if (stab_enabled && stab_score == 0 && (swing_score & 4U) && !(swing_score & 0x10U)) {
            const float speed = square_root(squared_length(frame.velocity_150));
            const float velocity_inverse = inverse_length(speed);
            const float alignment =
                add(add(multiply(multiply(frame.velocity_150[1], velocity_inverse), direction[1]),
                        multiply(multiply(frame.velocity_150[0], velocity_inverse), direction[0])),
                    multiply(multiply(frame.velocity_150[2], velocity_inverse), direction[2]));
            if (alignment >= 0.08715574443340302f)
                swing_score = 0;
        }
        if (best_score < swing_score && (!stab_enabled || !low_health || (swing_score & 0x10U))) {
            best_score = swing_score;
            chosen_direction = direction;
            if (swing_score & 0x10U)
                return;
        }
    }
    if ((frame.flags & 2U) && stab_enabled && best_score < stab_score) {
        best_score = stab_score;
        chosen_direction = direction;
    }
}

bool apply_knife_aim(const client::CommandFrame& frame, void* local_player,
                     client::Command& command, const RageVector& direction,
                     std::uint8_t& send_packet, const KnifeAimBindings& bindings) {
    constexpr float degrees = 57.2957763671875f;
    if (direction[0] == 0.0f && direction[1] == 0.0f) {
        command.angles = {direction[2] > 0.0f ? 270.0f : 90.0f, 0.0f, 0.0f};
    } else {
        const float yaw = multiply(
            float(bindings.arc_tangent2(double(direction[1]), double(direction[0]))), degrees);
        const float horizontal = square_root(
            add(multiply(direction[0], direction[0]), multiply(direction[1], direction[1])));
        const float pitch = multiply(
            float(bindings.arc_tangent2(double(-direction[2]), double(horizontal))), degrees);
        command.angles = {pitch, yaw, 0.0f};
    }
    if (frame.flags & 0x40U) {
        auto reference = command_angles(frame.previous_command);
        if (!(field<std::uint32_t>(local_player, bindings.player_flags) & 8U) &&
            field<std::uint8_t>(local_player, bindings.first_player_byte) ==
                field<std::uint8_t>(local_player, bindings.second_player_byte)) {
            const auto number = std::bit_cast<std::int32_t>(std::uint32_t(command.number) - 1U);
            if (const auto* previous =
                    engine::invoke<const client::Command*>(bindings.input, 8, 0, number))
                reference = command_angles(*previous);
        }
        auto aim = command_angles(command);
        const bool limited =
            rage::limit_firing_angle_step(aim, reference, frame.maximum_angle_step_1e8);
        command.angles = {aim.pitch, aim.yaw, aim.roll};
        if (limited)
            return false;
    }
    send_packet = 0;
    return true;
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: knife_melee.cpp
// ============================================================================

namespace skeet::misc {
namespace {
// field from util/address.h
/* deduplicated add */
/* deduplicated subtract */
/* deduplicated multiply */
/* deduplicated inverse_length */
bool greater(float a, float b) noexcept {
    return _mm_comigt_ss(_mm_set_ss(a), _mm_set_ss(b));
}
bool greater_equal(float a, float b) noexcept {
    return _mm_comige_ss(_mm_set_ss(a), _mm_set_ss(b));
}

// The four native 4346EDC0 slots are only jumps into this genuine engine
// table. Supplying the table directly retains that external boundary and the
// exact filter prefix (skip entity, collision group zero, additional word zero).
struct KnifeTraceFilter {
    void* const* vtable;
    void* skip;
    std::uint32_t collision_group = 0, additional = 0;
};
struct alignas(16) KnifeEngineRay {
    std::array<float, 4> start{}, delta{}, start_offset{}, extents{};
    const void* world_axis_transform = nullptr;
    std::uint8_t is_ray = 1, swept = 0;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(offsetof(KnifeTraceFilter, skip) == 4);
static_assert(offsetof(KnifeTraceFilter, collision_group) == 8);
static_assert(offsetof(KnifeEngineRay, world_axis_transform) == 0x40);
static_assert(offsetof(KnifeEngineRay, is_ray) == 0x44);
#endif
KnifeEngineRay make_ray(const RageVector& start, const RageVector& end, bool hull) noexcept {
    KnifeEngineRay ray;
    for (unsigned axis = 0; axis != 3; ++axis) {
        ray.delta[axis] = subtract(end[axis], start[axis]);
        ray.start[axis] = hull ? add(start[axis], 0.0f) : start[axis];
        if (hull)
            ray.start_offset[axis] = -0.0f;
    }
    const auto squared =
        add(add(multiply(ray.delta[1], ray.delta[1]), multiply(ray.delta[0], ray.delta[0])),
            multiply(ray.delta[2], ray.delta[2]));
    ray.swept = squared != 0.0f;
    if (hull) {
        ray.extents = {16.0f, 16.0f, 18.0f, 0.0f};
        ray.is_ray = 0;
    }
    return ray;
}
RageVector endpoint(const RageVector& origin, const RageVector& direction, float reach) noexcept {
    return {add(multiply(direction[0], reach), origin[0]),
            add(multiply(direction[1], reach), origin[1]),
            add(multiply(direction[2], reach), origin[2])};
}
bool hit(const BallisticTrace& trace) noexcept {
    return greater(1.0f, trace.fraction) || trace.all_solid || trace.start_solid;
}
void* entity_pointer(BallisticEntity entity) noexcept {
    return reinterpret_cast<void*>(std::uintptr_t(entity));
}
long double armor_ratio(void* weapon) {
#if defined(_M_IX86) || defined(__i386__)
    // The getter returns ST0. Native 4333CD68 multiplies that value directly,
    // retaining its extended precision until the following single float store.
    return engine::invoke<long double>(weapon, 434);
#else
    return engine::invoke<float>(weapon, 434);
#endif
}
}

void trace_knife_melee(void* local_player, const RageVector& start, const RageVector& end,
                       BallisticTrace& result, const KnifeMeleeBindings& bindings) {
    KnifeTraceFilter filter{bindings.trace_filter_vtable, local_player};
    auto ray = make_ray(start, end, false);
    engine::invoke<void>(bindings.engine_trace, 5, &ray, 0x200400Bu, &filter, &result);
    if (greater_equal(result.fraction, 1.0f)) {
        KnifeTraceFilter hull_filter{bindings.trace_filter_vtable, local_player};
        ray = make_ray(start, end, true);
        engine::invoke<void>(bindings.engine_trace, 5, &ray, 0x200400Bu, &hull_filter, &result);
    }
}

bool find_knife_trace_direction(void* local_player, const RageVector& origin, void* target,
                                RageVector& direction, const KnifeMeleeBindings& bindings) {
    BallisticTrace trace;
    auto end = endpoint(origin, direction, 48.0f);
    trace_knife_melee(local_player, origin, end, trace, bindings);
    if (!hit(trace))
        return false;
    if (entity_pointer(trace.entity) == target)
        return true;

    float yaw = 0.0f;
    if (direction[1] != 0.0f || direction[0] != 0.0f)
        yaw = float(bindings.arc_tangent2(double(direction[1]), double(direction[0])));
    constexpr float angle_step = 0.0872664600610733f;
    float offset = angle_step;
    for (unsigned step = 0; step != 7; ++step) {
        for (unsigned side = 0; side != 2; ++side) {
            const auto angle = side == 0 ? add(offset, yaw) : subtract(yaw, offset);
            const auto sine = bindings.trigonometry.sine(angle);
            const auto cosine = bindings.trigonometry.cosine(angle);
            const auto vertical = direction[2];
            const auto inverse = inverse_length(add(
                add(multiply(sine, sine), multiply(cosine, cosine)), multiply(vertical, vertical)));
            const RageVector candidate{multiply(inverse, cosine), multiply(inverse, sine),
                                       multiply(vertical, inverse)};
            end = endpoint(origin, candidate, 48.0f);
            trace_knife_melee(local_player, origin, end, trace, bindings);
            if (hit(trace) && entity_pointer(trace.entity) == target) {
                direction = candidate;
                return true;
            }
        }
        offset = add(offset, angle_step);
    }
    return false;
}

std::uint32_t score_knife_attack(const client::CommandFrame& frame, void* local_player,
                                 const RageVector& direction, bool stab,
                                 const KnifeMeleeBindings& bindings) {
    BallisticTrace trace;
    const auto end = endpoint(frame.origin_dc, direction, stab ? 32.0f : 48.0f);
    trace_knife_melee(local_player, frame.origin_dc, end, trace, bindings);
    if (greater_equal(trace.fraction, 1.0f))
        return 0;
    auto* target = entity_pointer(trace.entity);
    if (!rage::target_is_eligible(bindings.targeting, target))
        return 0;

    const auto* angles = engine::invoke<const RageVector*>(target, 11);
    constexpr float radians = 0.01745329238474369f;
    const auto yaw = multiply((*angles)[1], radians);
    const auto sine_yaw = bindings.trigonometry.sine(yaw);
    const auto cosine_yaw = bindings.trigonometry.cosine(yaw);
    const auto pitch = multiply((*angles)[0], radians);
    bindings.trigonometry.sine(pitch);
    const auto cosine_pitch = bindings.trigonometry.cosine(pitch);
    const auto forward_x = multiply(cosine_pitch, cosine_yaw);
    const auto forward_y = multiply(cosine_pitch, sine_yaw);

    const auto x = subtract(field<float>(target, bindings.target_origin), frame.origin_10c[0]);
    const auto y = subtract(field<float>(target, bindings.target_origin + 4), frame.origin_10c[1]);
    const auto inverse = inverse_length(add(multiply(y, y), multiply(x, x)));
    const auto unit_x = multiply(x, inverse), unit_y = multiply(y, inverse);
    std::uint32_t score = 2;
    if (stab)
        score = engine::invoke<std::int32_t>(target, 122) > 100 ? 9 : 1;

    auto* weapon = entity_pointer(frame.weapon_address);
    std::int32_t damage;
    const auto facing = add(multiply(unit_y, forward_y), multiply(forward_x, unit_x));
    if (greater(facing, 0.4749999940395355f)) {
        score |= 4;
        damage = stab ? 180 : 90;
    } else if (stab)
        damage = 65;
    else {
        const auto next_attack =
            add(field<float>(weapon, bindings.weapon_next_primary), 0.4000000059604645f);
        const auto now = *bindings.current_time;
        damage = greater(now, next_attack) ? 40 : 25;
    }
    if (field<std::int32_t>(target, bindings.target_armor) > 0) {
        const auto ratio = armor_ratio(weapon);
        const float scaled = float((ratio * static_cast<long double>(damage)) * 0.5L);
        damage = _mm_cvttss_si32(_mm_set_ss(scaled));
    }
    if (damage >= engine::invoke<std::int32_t>(target, 122))
        score |= 0x10;
    return score;
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: knife_owner.cpp
// ============================================================================

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define KNIFE_MEMBER __thiscall
#else
#define KNIFE_MEMBER
#endif
using OwnerEntry = void(KNIFE_MEMBER*)(void*);
void KNIFE_MEMBER destroy_knife(void*);
void KNIFE_MEMBER unchanged_knife(void*) {} //433CCE8E RET
const std::array<OwnerEntry, 9> knife_lifecycle_table{destroy_knife,   unchanged_knife, unchanged_knife,
                                                unchanged_knife, unchanged_knife, unchanged_knife,
                                                unchanged_knife, unchanged_knife, unchanged_knife};
}

class KnifeLifecycleOwner {
public:
    const OwnerEntry* vtable = nullptr;
    engine::JobMemory memory;
    CommandOwner command;
    client::NativeCommandPhaseReceiver command_receiver{};

    KnifeLifecycleOwner(engine::OwnerRegistryBinding lifecycle,
                        client::CommandPhaseRegistry& commands, ConfigRegistries& registries,
                        KnifeConfig& settings, KnifeCommandBindings bindings,
                        std::uint32_t& scheduler_flags)
        : memory(lifecycle.memory),
          command(KnifeCommandOwner{settings, bindings, scheduler_flags}) {
        lifecycle.append(this); //434802E9 ->433676A0, before vtable assignment
        commands.append(command_receiver, &command); //434802F1 ->43345EBC
        vtable = knife_lifecycle_table.data(); //434802F9, native4346F358
        commands.publish(command_receiver, client::CommandOwnerId::knife); //43480301
        initialize_knife_config(registries, settings); //43480317 /43480326
    }
};

namespace {
void KNIFE_MEMBER destroy_knife(void* object) {
    // Native433B1A7A jumps directly to433837D0. Config and command ranges are
    // released by the common shutdown; this slot does not unlink descriptors.
    if (!object)
        return;
    auto* owner = static_cast<KnifeLifecycleOwner*>(object);
    const auto memory = owner->memory;
    owner->~KnifeLifecycleOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
}
std::size_t knife_owner_size() noexcept {
    return sizeof(KnifeLifecycleOwner);
}
KnifeLifecycleOwner& construct_knife(void* storage, engine::OwnerRegistryBinding lifecycle,
                                     client::CommandPhaseRegistry& commands,
                                     ConfigRegistries& registries, KnifeConfig& settings,
                                     KnifeCommandBindings bindings,
                                     std::uint32_t& scheduler_flags) {
    return *::new (storage)
        KnifeLifecycleOwner(lifecycle, commands, registries, settings, bindings, scheduler_flags);
}
#undef KNIFE_MEMBER
} // namespace skeet::misc

// ============================================================================
// Subcomponent: knife_record.cpp
// ============================================================================

namespace skeet::misc {
namespace {
// field from util/address.h
// store from util/address.h
std::int32_t decrement(std::int32_t value) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(value) - 1U);
}
void set_transform(void* player, const lagcomp::EntityStateRuntime& entity,
                   lagcomp::SetAbsoluteVector function, const lagcomp::Vector& value) {
    if (!function)
        return;
    const auto offset = entity.offsets.transform_guard_49d93e62 - 0x14U;
    const auto saved = field<std::uint32_t>(player, offset);
    store(player, offset, UINT32_MAX);
    function(player, value);
    store(player, offset, saved);
}
void publish_bone_cache(lagcomp::EntityRecordScope& scope,
                        const lagcomp::ExtrapolatedRecordBindings& bindings,
                        const std::uint32_t* current_bone_counter,
                        const animation::BoneRuntime& bones) {
    const auto& entity = bindings.preparation.entity;
    const auto& offsets = entity.offsets;
    const auto cache = field<Matrix3x4*>(scope.player, offsets.bone_cache);
    bones.scatter_hitbox_bone_matrices(scope.player, scope.record.matrices, {cache, 256});
    store(scope.player, offsets.bone_accessor + 8, 0xFFF00U);
    store(scope.player, offsets.bone_accessor + 12, 0xFFF00U);
    if (current_bone_counter)
        store(scope.player, offsets.recent_bone_counter, *current_bone_counter);
}
void invalidate_bone_cache(void* player, const lagcomp::EntityNetvars& offsets) noexcept {
    store(player, offsets.bone_accessor + 8, 0U);
    store(player, offsets.recent_bone_counter, 0U);
    store(player, offsets.last_bone_setup_time, 0xFF7FFFFFU);
    store(player, offsets.field_0471e13c, 0U);
    store(player, offsets.effects_bb78dc43,
          field<std::uint32_t>(player, offsets.effects_bb78dc43) & ~2U);
}
}

void apply_extrapolated_knife_record(lagcomp::EntityRecordScope& scope, client::CommandFrame& frame,
                                     const lagcomp::ExtrapolatedRecordBindings& bindings,
                                     const std::uint32_t* current_bone_counter,
                                     const animation::BoneRuntime& bones,
                                     const MovementExtrapolationLiveBindings& live_movement,
                                     const client::CommandBudgetBindings& budget) {
    const auto& entity = bindings.preparation.entity;
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(scope.player) + 8, 10);
    auto& owner = *bindings.players.get(index);
    if (scope.snapshot.state.capture_frame != entity.frame)
        entity.capture_snapshot(scope.snapshot, scope.player);
    const auto timeline = lagcomp::history_timeline(owner);
    bool capture = timeline.record_count == 0;
    if (!capture) {
        const auto tick = lagcomp::time_to_tick(timeline.simulation_time, entity.interval);
        capture = tick < decrement(timeline.last_update_tick) &&
                  tick < timeline.at(0).header.simulation_tick;
    }
    if (capture) {
        entity.capture_record(scope.record, scope.player, bindings.players);
        scope.flags |= 0x20U;
    } else
        std::memcpy(&scope.record, &timeline.at(0), sizeof scope.record);

    lagcomp::HistoryTickTiming current{};
    current.tick_d4 = field<std::int32_t>(&frame, 0xD4);
    current.tick_18c = std::bit_cast<std::int32_t>(frame.field_18c);
    const lagcomp::MissingTickTiming timing{
        current.tick_d4, current.tick_18c, entity.interval, entity.receive_interval, 0,
        entity.engine,   scope.player};
    const auto ticks =
        lagcomp::estimate_missing_ticks(lagcomp::history_timeline(owner), timing, budget);
    std::int32_t changed = 0;
    if (ticks > 0) {
        changed = extrapolate_lag_record(scope.player, owner, scope.record, ticks,
                                         bindings.movement, live_movement);
        if (changed < 0)
            return;
        if (changed > 0)
            scope.flags |= 0x20U;
    }

    const auto applied_flags = scope.flags & 0x60U;
    const auto& offsets = entity.offsets;
    const auto& record = scope.record;
    const auto& saved = record.header;
    std::uint8_t prior_override = 0;
    if (entity.transform_override) {
        prior_override = *entity.transform_override;
        *entity.transform_override = 1;
    }
    store(scope.player, offsets.player_flags, saved.player_flags);
    store(scope.player, offsets.simulation_time, float(saved.simulation_tick) * entity.interval);
    set_transform(scope.player, entity, entity.set_absolute_angles,
                  {0.0f, saved.animation_yaw, 0.0f});
    auto* collideable = engine::invoke<void*>(scope.player, 3);
    auto* maximum =
        const_cast<lagcomp::Vector*>(engine::invoke<const lagcomp::Vector*>(collideable, 2));
    if ((*maximum)[2] != saved.collision_max_z) {
        collideable = engine::invoke<void*>(scope.player, 3);
        maximum =
            const_cast<lagcomp::Vector*>(engine::invoke<const lagcomp::Vector*>(collideable, 2));
        (*maximum)[2] = saved.collision_max_z;
    }
    store(scope.player, offsets.velocity, saved.velocity);
    set_transform(scope.player, entity, entity.set_absolute_origin, saved.origin);
    store(scope.player, offsets.origin, saved.origin);
    auto* layers = static_cast<std::byte*>(scope.player) + offsets.animation_layers;
    auto* poses = static_cast<float*>(
        static_cast<void*>(static_cast<std::byte*>(scope.player) + offsets.pose_parameters));
    lagcomp::restore_animation(record.animation, field<void*>(layers, 0),
                               field<std::int32_t>(layers, 12), poses);
    if (!applied_flags) {
        if ((saved.flags & 4U) && field<std::int32_t>(scope.player, offsets.bone_cache + 12))
            publish_bone_cache(scope, bindings, current_bone_counter, bones);
        else
            invalidate_bone_cache(scope.player, offsets);
    }
    if (entity.transform_override)
        *entity.transform_override = prior_override;

    if (changed > 0) {
        lagcomp::build_extrapolated_record_matrices(scope.player, scope.record, bones,
                                                    bindings.attachments.scratch);
        if (!(scope.flags & 0x40U)) {
            const auto input =
                lagcomp::capture_record_hypothesis_input(scope.player, owner, scope.record, entity);
            resolver_runtime::build_hypotheses(scope.record, input, scope.player, bones);
        }
    }
    auto preparation = bindings.preparation;
    preparation.current_timing = &current;
    lagcomp::refresh_record_attachments(bindings.players, scope.player, scope.record, preparation,
                                        bindings.attachments, bones);
    if (scope.record.header.flags & 4U)
        publish_bone_cache(scope, bindings, current_bone_counter, bones);
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: knife_runtime.cpp
// ============================================================================

namespace skeet::misc {
namespace {
// field from util/address.h
std::int32_t add(std::int32_t first, std::int32_t second) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(first) + std::uint32_t(second));
}
std::int32_t subtract(std::int32_t first, std::int32_t second) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(first) - std::uint32_t(second));
}
bool convar_nonzero(const void* object) noexcept {
    for (;;) {
        const auto parent = field<const void*>(object, 0x1C);
        if (parent == object)
            break;
        object = parent;
    }
    return field<std::uint32_t>(object, 0x30) !=
           std::uint32_t(reinterpret_cast<std::uintptr_t>(object));
}
void add_attack_button(client::Command& command, std::uint32_t score) noexcept {
    command.movement.buttons |= (score & 2U) ? 1U : ((score & 1U) << 11);
}
std::int32_t interpolation_ticks(const client::CommandFrame& frame, float interval) noexcept {
    return lagcomp::time_to_tick(field<float>(&frame, 0x198), interval);
}
std::int32_t target_attack_tick(const client::CommandFrame& frame, const void* target,
                                const KnifeRuntimeBindings& bindings) noexcept {
    const auto& entity = bindings.records.preparation.entity;
    return add(lagcomp::time_to_tick(field<float>(target, entity.offsets.simulation_time),
                                     entity.interval),
               interpolation_ticks(frame, entity.interval));
}
std::int32_t knife_history_age(const lagcomp::RecordRing& ring,
                               const KnifeRuntimeBindings& bindings) noexcept { // complete4340B19C
    if (!ring.count)
        return 1;
    const auto& entity = bindings.records.preparation.entity;
    const auto limit = client::query_command_budget(bindings.budget);
    const auto current = lagcomp::corrected_command_tick(
        entity.command_tick, entity.previous_command, entity.current_command);
    const auto age = subtract(current, ring.at(0).header.command_tick);
    if (limit < 0)
        return limit;
    if (age < 0)
        return 0;
    return age > limit ? limit : age;
}

void process_current_knife(const KnifeConfig& config, client::CommandFrame& frame,
                           client::Command& command, void* player, void* target,
                           const KnifeRuntimeBindings& bindings) {
    const auto& entity = bindings.records.preparation.entity;
    const auto absolute = *engine::invoke<const RageVector*>(target, 10);
    const auto origin = field<RageVector>(target, bindings.targeting.offsets.origin);
    const bool moved =
        origin[0] != absolute[0] || origin[1] != absolute[1] || origin[2] != absolute[2];
    if (moved && entity.set_absolute_origin)
        entity.set_absolute_origin(target, origin);
    std::uint32_t swing = 0;
    bool finished = false;
    const auto modes = std::uint32_t(config.mode.value);
    const auto direction = field<RageVector>(&frame, 0x1A8);
    if (modes & 1U) {
        swing = score_knife_attack(frame, player, direction, false, bindings.melee);
        if (!swing)
            finished = true; // Native swing miss exits before the stab query.
    }
    if (!finished) {
        const auto stab =
            (modes & 2U) ? score_knife_attack(frame, player, direction, true, bindings.melee) : 0U;
        if (stab > swing) {
            command.movement.buttons |= 0x800U;
            if (frame.flags & 2U)
                command.tick = target_attack_tick(frame, target, bindings);
        } else if (swing) {
            command.movement.buttons |= 1U;
            if (frame.flags & 0x400U)
                command.tick = target_attack_tick(frame, target, bindings);
        }
    }
    if (moved && entity.set_absolute_origin)
        entity.set_absolute_origin(target, absolute);
}
}

void* nearest_knife_target(const RageVector& origin, const rage::TargetBindings& bindings,
                           std::int32_t maximum_clients) {
    void* selected = nullptr;
    float nearest = 250000.0f;
    for (std::int32_t index = 1; index <= maximum_clients; index = add(index, 1)) {
        auto* target = engine::invoke<void*>(bindings.entity_list, 3, index);
        if (!rage::target_is_eligible(bindings, target) ||
            field<std::uint8_t>(target, bindings.offsets.movement_type) == 8)
            continue;
        const auto position = field<RageVector>(target, bindings.offsets.origin);
        const float dy = origin[1] - position[1], dx = origin[0] - position[0],
                    dz = origin[2] - position[2];
        const float distance = (dy * dy + dx * dx) + dz * dz;
        if (distance < nearest) {
            nearest = distance;
            selected = target;
        }
    }
    return selected;
}

void process_knife_command(const KnifeConfig& config, client::CommandFrame& frame,
                           client::Command& command, void* player, std::uint8_t& send_packet,
                           std::uint32_t& scheduler_flags, const KnifeRuntimeBindings& bindings) {
    if (!knife_command_enabled(config, frame, command, bindings.rage_enabled))
        return;
    process_enabled_knife_command(config, frame, command, player, send_packet, scheduler_flags,
                                  bindings);
}
bool knife_command_enabled(const KnifeConfig& config, const client::CommandFrame& frame,
                           const client::Command& command,
                           const std::uint8_t* rage_enabled) noexcept {
    if (!config.enabled.value || !frame.weapon_address || frame.weapon_kind_124 ||
        (command.movement.buttons & 0x801U))
        return false;
    return !*rage_enabled || (frame.flags & 0x402U);
}
void process_enabled_knife_command(const KnifeConfig& config, client::CommandFrame& frame,
                                   client::Command& command, void* player,
                                   std::uint8_t& send_packet, std::uint32_t& scheduler_flags,
                                   const KnifeRuntimeBindings& bindings) {
    auto* target =
        nearest_knife_target(frame.origin_10c, bindings.targeting, *bindings.maximum_clients);
    if (!target)
        return;
    if (!*bindings.rage_enabled) {
        process_current_knife(config, frame, command, player, target, bindings);
        return;
    }
    if (frame.choked_15c >= frame.choke_limit_fc && frame.choked_15c != 0)
        return;

    auto* networkable = static_cast<std::byte*>(target) + 8;
    const auto index = engine::invoke<std::uint32_t>(networkable, 10);
    auto& cached = *bindings.records.players.get(index);
    const auto health = engine::invoke<std::int32_t>(target, 122);
    const bool low_health = health <= (field<std::int32_t>(target, bindings.armor) > 0 ? 55 : 65);
    const auto second_index = engine::invoke<std::uint32_t>(networkable, 10);
    const auto& entity = bindings.records.preparation.entity;
    bool history_usable = false;
    if (convar_nonzero(bindings.candidates.lag_compensation)) {
        const auto& current = *bindings.records.players.get(second_index);
        history_usable = rage::target_history_is_usable(current.native, current.history, frame,
                                                        second_index, entity.interval);
    }
    const auto modes = std::uint32_t(config.mode.value);
    RageVector chosen;
    std::uint32_t best = 0;
    if (history_usable) {
        auto ticks =
            rage::collect_target_record_ticks(frame, target, cached, 1, bindings.candidates);
        if (ticks.empty() && cached.history.count) {
            const auto& newest = cached.history.at(0).header;
            if (!(frame.flags & 0x40U) && knife_history_age(cached.history, bindings) > 3) {
                const auto& velocity = newest.velocity;
                const float speed_squared =
                    (velocity[0] * velocity[0] + velocity[1] * velocity[1]) +
                    velocity[2] * velocity[2];
                if (speed_squared > 10.0f)
                    return;
            }
            ticks.push_back(newest.simulation_tick);
        }
        if (ticks.empty()) {
            evaluate_knife_direction(modes, frame, player, target, low_health, chosen, best,
                                     bindings.melee);
            if (!best ||
                !apply_knife_aim(frame, player, command, chosen, send_packet, bindings.aim))
                return;
            add_attack_button(command, best);
            command.tick = target_attack_tick(frame, target, bindings);
            return;
        }

        std::array<Matrix3x4, 256> saved_bones;
        lagcomp::EntityRecordScope scope(entity, target, saved_bones);
        std::int32_t chosen_tick = command.tick;
        for (const auto tick : ticks) {
            if (!lagcomp::select_and_apply_history_record(scope, bindings.records, tick, 0,
                                                          bindings.bone_runtime))
                break;
            const auto previous = best;
            evaluate_knife_direction(modes, frame, player, target, low_health, chosen, best,
                                     bindings.melee);
            if (best > previous) {
                chosen_tick = tick;
                if ((best & 0x12U) == 0x12U)
                    break;
            }
            if (scope.flags & 0x20U)
                break;
        }
        if (best && apply_knife_aim(frame, player, command, chosen, send_packet, bindings.aim)) {
            add_attack_button(command, best);
            command.tick = add(chosen_tick, interpolation_ticks(frame, entity.interval));
            send_packet = 0;
            scheduler_flags = (scheduler_flags & ~0x400U) | 0x200U;
        }
        return;
    }

    std::array<Matrix3x4, 256> saved_bones;
    lagcomp::EntityRecordScope scope(entity, target, saved_bones);
    apply_extrapolated_knife_record(scope, frame, bindings.extrapolation,
                                    bindings.records.current_bone_counter, bindings.bone_runtime,
                                    bindings.live_movement, bindings.budget);
    evaluate_knife_direction(modes, frame, player, target, low_health, chosen, best,
                             bindings.melee);
    if (best && apply_knife_aim(frame, player, command, chosen, send_packet, bindings.aim)) {
        add_attack_button(command, best);
        frame.flags |= 0x2000000U;
    }
}
} // namespace skeet::misc

// ============================================================================
// Subcomponent: zeus.cpp
// ============================================================================

namespace skeet::misc {
namespace {
// field from util/address.h

// Native table 4346DCEC contains only tail forwards to the five actual engine
// methods at 43475B90. No client should-hit predicate replaces slot 0 here.
struct ZeusTraceFilter {
    void* const* vtable;
    std::array<std::uint32_t, 5> fields_04{};
    std::uint32_t mask = 0x4600400Bu;
    std::uint32_t exit_mask = 0x4600400Bu;
    float minimum_damage = 1.0f;

    BallisticFilter binding() noexcept {
        return {this, 0, mask, exit_mask, minimum_damage, BallisticFilterKind::supplied};
    }
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(ZeusTraceFilter) == 0x24);
static_assert(offsetof(ZeusTraceFilter, mask) == 0x18);
static_assert(offsetof(ZeusTraceFilter, minimum_damage) == 0x20);
#endif
}

void bind_zeus_configuration(ZeusState& state, ConfigLink& descriptors) noexcept {
    const auto* descriptor = static_cast<BooleanConfig*>(find_config(descriptors, 0x23060EC5u));
    state.rage_enabled = &descriptor->value;
}

void process_zeus(ZeusState& state, const ZeusConfig& config, const ZeusBindings& bindings,
                  const client::CommandFrame& frame, void* player, client::Command& command) {
    if (!config.enabled.value || *state.rage_enabled != 0 || frame.weapon_id_d8 != 31 ||
        !(frame.flags & 0x400u) || (command.movement.buttons & 1u))
        return;

    auto* weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
    auto query = rage::prepare_target_bullet(weapon, bindings.weapon_definition, false);
    ZeusTraceFilter native_filter{bindings.trace_filter_vtable};
    const auto filter = native_filter.binding();
    const auto direction = field<RageVector>(&frame, 0x174);
    const auto source = static_cast<BallisticEntity>(reinterpret_cast<std::uintptr_t>(player));
    const auto result = rage::trace_target_direction(bindings.ballistics, source, frame.origin_dc,
                                                     direction, filter, query);
    if (result == 0 || _mm_cvttss_si32(_mm_set_ss(query.damage)) < 110)
        return;

    // A nonzero 43364CEC return already guarantees a nonnull hit entity.
    // Native's contradictory-null arm is the overlapped null-write trap at
    // 433A13A5; it is not an alternate no-target decision.
    const auto* target = reinterpret_cast<const void*>(std::uintptr_t(query.trace.entity));
    if (field<std::uint8_t>(target, bindings.target_immunity) == 0)
        command.movement.buttons |= 1u;
}

} // namespace skeet::misc

// ============================================================================
// Subcomponent: zeus_owner.cpp
// ============================================================================

namespace skeet::misc {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define ZEUS_MEMBER __thiscall
#else
#define ZEUS_MEMBER
#endif
using OwnerEntry = void(ZEUS_MEMBER*)(void*);
void ZEUS_MEMBER destroy_zeus(void*);
void ZEUS_MEMBER initialize_zeus(void*);
void ZEUS_MEMBER unchanged_zeus(void*) {} //433CCE8E RET
// Native4346FEE0. Slot1 is initialization, not an eager constructor action.
const std::array<OwnerEntry, 9> zeus_lifecycle_table{destroy_zeus,   initialize_zeus, unchanged_zeus,
                                                unchanged_zeus, unchanged_zeus,  unchanged_zeus,
                                                unchanged_zeus, unchanged_zeus,  unchanged_zeus};
}

class ZeusLifecycleOwner {
public:
    const OwnerEntry* vtable = nullptr;
    engine::JobMemory memory;
    ConfigLink& descriptors;
    ZeusState& state;
    CommandOwner command;
    client::NativeCommandPhaseReceiver command_receiver{};

    ZeusLifecycleOwner(engine::OwnerRegistryBinding lifecycle,
                       client::CommandPhaseRegistry& commands, ConfigRegistries& registries,
                       ZeusConfig& settings, ZeusState& shared_state, const ZeusBindings& bindings)
        : memory(lifecycle.memory), descriptors(registries.descriptors), state(shared_state),
          command(ZeusCommandOwner{shared_state, settings, bindings}) {
        lifecycle.append(this); //43485D53 ->433676A0, primary still null
        commands.append(command_receiver, &command); //43485D5B ->43345EBC
        vtable = zeus_lifecycle_table.data(); //43485D63, table4346FEE0
        commands.publish(command_receiver, client::CommandOwnerId::zeus); //43485D6B
        initialize_zeus_config(registries, settings); //2E314CE2/size1/type1; value untouched
        state.rage_enabled = nullptr; //43485D86, after descriptor registration
    }
};

namespace {
void ZEUS_MEMBER initialize_zeus(void* object) { //434824A7
    auto& owner = *static_cast<ZeusLifecycleOwner*>(object);
    bind_zeus_configuration(owner.state, owner.descriptors);
}
void ZEUS_MEMBER destroy_zeus(void* object) { //433B1A7A ->433837D0
    if (!object)
        return;
    auto* owner = static_cast<ZeusLifecycleOwner*>(object);
    const auto memory = owner->memory;
    owner->~ZeusLifecycleOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
    // Native free does not remove borrowed descriptor/command/primary links.
}
}

std::size_t zeus_owner_size() noexcept {
    return sizeof(ZeusLifecycleOwner);
}
ZeusLifecycleOwner& construct_zeus(void* storage, engine::OwnerRegistryBinding lifecycle,
                                   client::CommandPhaseRegistry& commands,
                                   ConfigRegistries& registries, ZeusConfig& settings,
                                   ZeusState& state, const ZeusBindings& bindings) {
    return *::new (storage)
        ZeusLifecycleOwner(lifecycle, commands, registries, settings, state, bindings);
}
#undef ZEUS_MEMBER
} //namespace skeet::misc
