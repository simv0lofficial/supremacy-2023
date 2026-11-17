// HvH / Anti-Aim Subsystem Implementation
#include "skeet/anti_aim/angles.hpp"
#include "skeet/anti_aim/body_freestanding.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/movement/steering_math.hpp"
#include <algorithm>
#include <bit>
#include <cmath>
#include <cstring>
#include <limits>
#include "skeet/anti_aim/body_yaw.hpp"
#include <immintrin.h>
#include "skeet/anti_aim/command_owner.hpp"
#include "skeet/movement/simulation_environment.hpp"
#include "skeet/client/target_refresh_runtime_owner.hpp"
#include "skeet/lagcomp/native_history_boundaries.hpp"
#include "skeet/anti_aim/command_phase.hpp"
#include "skeet/anti_aim/config.hpp"
#include "skeet/anti_aim/edge.hpp"
#include "skeet/anti_aim/eligibility.hpp"
#include "skeet/anti_aim/finisher.hpp"
#include "skeet/anti_aim/freestanding.hpp"
#include "skeet/anti_aim/jitter.hpp"
#include "skeet/anti_aim/menu_dependencies.hpp"
#include "skeet/anti_aim/movement_capture.hpp"
#include "skeet/anti_aim/owner_lifecycle.hpp"
#include "skeet/animation/state_copy.hpp"
#include <array>
#include <new>
#include "skeet/anti_aim/pitch.hpp"
#include "skeet/anti_aim/precalculation.hpp"
#include <type_traits>
#include "skeet/anti_aim/proximity.hpp"
#include "skeet/anti_aim/runtime.hpp"
#include "skeet/anti_aim/runtime_owner.hpp"
#include "skeet/client/runtime_storage.hpp"
#include "skeet/client/runtime_composition.hpp"
#include "skeet/client/history_runtime_owner.hpp"
#include "skeet/client/player_runtime_owner.hpp"
#include "skeet/client/movement_hud_runtime_owner.hpp"
#include "skeet/client/command_capture_runtime_owner.hpp"
#include "skeet/client/visual_runtime_owner.hpp"
#include "skeet/client/combat_runtime_factories.hpp"
#include "skeet/client/native_startup.hpp"
#include "skeet/script/source_runtime_owner.hpp"
#include "skeet/engine/scene_runtime_owner.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/engine/netvar_initialization.hpp"
#include "skeet/engine/build_hitbox_clip_bindings.hpp"
#include "skeet/animation/runtime_owner.hpp"
#include "skeet/animation/native_trigonometry.hpp"
#include "skeet/rage/settings.hpp"
#include "skeet/esp/other_config.hpp"
#include "skeet/misc/config.hpp"
#include <stdexcept>
#include <windows.h>
#include "skeet/anti_aim/scheduler.hpp"
#include "skeet/anti_aim/startup_initializers.hpp"
#include "skeet/anti_aim/state.hpp"
#include "skeet/anti_aim/target_geometry.hpp"
#include "skeet/anti_aim/targets.hpp"
#include "skeet/esp/player_draw.hpp"
#include "skeet/esp/entity_overlay.hpp"
#include "skeet/esp/player_engine.hpp"
#include "skeet/rage/point_penetration.hpp"
#include "skeet/rage_segments.hpp"
#include "skeet/anti_aim/yaw.hpp"


// ============================================================================
// Subcomponent: angles.cpp
// ============================================================================

namespace skeet::anti_aim {
template <class Bindings>
void angle_stage(const GlobalConfig& global, const Config& config,
                 const AngleParameters& parameters, AngleStageState& state, AngleFrame& frame,
                 const AnglePlayer& player, ActionGateGlobals& globals, client::Command& command,
                 std::uint8_t& send_packet, const Bindings& bindings) {
    ActionGateState gates{state.flags_130, state.value_158};
    const auto path = evaluate_action_gates(
        {global.enabled.value != 0, global.pitch.value, config.yaw_base.value}, frame.gates,
        player.gates, globals, gates, command, send_packet, bindings.proximity);
    state.flags_130 = gates.flags_130;
    state.value_158 = gates.sequence_158;
    if (path == ActionGatePath::stop)
        return;
    if (path == ActionGatePath::cached_yaw) {
        CommandYawState tail{state.flags_130, state.cached_yaw_18c};
        apply_cached_command_yaw(command.angles[1], tail);
        state.cached_yaw_18c = tail.cached_yaw_18c;
        return;
    }
    apply_pitch(
        command.angles[0],
        {global.pitch.value, config.custom_pitch.value, parameters.value_138, parameters.value_150},
        {frame.gates.flags, command.movement.buttons, player.gates.movement_type_8b5586f6,
         command.number, frame.angle_9c, player.gates.has_weapon_3d52a1a7, player.weapon_pitch_340},
        bindings.pitch);
    if ((!config.freestanding.value || (state.value_1ac & 0x618u)) && !(state.flags_130 & 8u)) {
        state.value_1ac = 0;
        state.value_1a8 = std::numeric_limits<float>::max();
    }
    const bool edge = solve_edge_yaw({config.edge_yaw.value, frame.scalar_c0,
                                      player.edge_scalar_3eb71843, player.state_7501cb91},
                                     {parameters.value_148, parameters.value_14c}, bindings.edge,
                                     bindings.trace_flag, command.angles[1]);
    if (edge) {
        state.value_1a8 = std::numeric_limits<float>::max();
        frame.gates.flags |= 0x80000u;
    } else if (global.yaw.value != 0 || global.body_yaw.value != 0) {
        if (global.yaw.value != 0)
            apply_target_yaw_base(config.yaw_base.value,
                                  {globals.target_handle, bindings.freestanding.target[0],
                                   bindings.freestanding.target[1]},
                                  frame.origin_dc[0], frame.origin_dc[1], globals.entities,
                                  command.angles[1]);
        auto free_context = bindings.freestanding;
        free_context.flags = globals.command_flags;
        free_context.target_handle = globals.target_handle;
        free_context.entities = globals.entities;
        FreestandingState free_state{state.flags_130, state.value_1a8, state.value_1ac};
        const auto free = apply_freestanding(
            {config.freestanding.value != 0, config.freestanding_hotkey.value.binding},
            {frame.scalar_c0, frame.scalar_38, frame.height_164, command.movement.forward_move,
             frame.origin_10c, frame.origin_168},
            free_context, free_state, command.angles[1], bindings.ballistics,
            bindings.body.workspace.freestanding);
        state.value_1a8 = free_state.yaw_1a8;
        state.value_1ac = free_state.value_1ac;
        if (!free)
            command.angles[1] = transform_yaw(
                command.angles[1], float(config.yaw_offsets.value[global.yaw.value]),
                global.yaw.value, state.saved_yaw_198, {parameters.value_138, parameters.value_148},
                {frame.angle_b8, frame.origin_dc[0], frame.origin_dc[1], frame.interpolation_158},
                {globals.target_handle, free_context.target[0], free_context.target[1]},
                globals.entities);
    }
    if (frame.gates.choked_15c == 0)
        send_packet = 0;
    if (!(state.flags_130 & 8u)) {
        apply_yaw_jitter(command.angles[1], global.yaw.value,
                         {config.yaw_jitter.value, config.yaw_jitter_offset.value,
                          parameters.value_14c, parameters.value_148, parameters.pattern_15c},
                         state.value_190, command.number, bindings.random);
        if (!(frame.gates.flags & 0x40u) &&
            lag_step_pending(frame.gates.flags, frame.gates.choke_limit_fc,
                             frame.gates.choked_15c) &&
            command.number != bindings.last_sent_command && global.body_yaw.value != 0) {
            // Working 4332E8C3 skips the shot-position correction.
            const auto base_yaw = command.angles[1];
            float minimum, maximum;
            animation_yaw_limits(player.animation, minimum, maximum);
            BodyYawState body_state{state.value_190, state.value_1a0, state.reference_yaw_258};
            const auto offset =
                solve_body_yaw({global.body_yaw.value, config.yaw_jitter.value,
                                config.body_yaw_offsets.value[2], config.body_yaw_offsets.value[3],
                                parameters.value_134, parameters.value_144, parameters.pattern_15c},
                               body_state,
                               {config.freestanding_body_yaw.value != 0, frame.command_ac,
                                bindings.last_body_command, bindings.shot_command, frame.origin_dc,
                                command.angles[0], base_yaw, maximum},
                               bindings.body, bindings.random);
            state.value_1a0 = body_state.cached_opposite_1a0;
            blend_body_yaw({parameters.value_134, parameters.value_138, parameters.value_13c,
                            parameters.value_144, parameters.value_154},
                           {offset, minimum, maximum, base_yaw, player.feet_yaw_80,
                            frame.gates.choked_15c, frame.gates.choke_limit_fc},
                           command.angles[1], send_packet);
        }
    }
    CommandYawState tail{state.flags_130, state.cached_yaw_18c};
    finalize_command_yaw(command.angles[1], frame.gates.yaw_24, tail);
    state.cached_yaw_18c = tail.cached_yaw_18c;
}
void process_angle_stage(const GlobalConfig& global, const Config& config,
                         const AngleParameters& parameters, AngleStageState& state,
                         AngleFrame& frame, const AnglePlayer& player, ActionGateGlobals& globals,
                         client::Command& command, std::uint8_t& send_packet,
                         const AngleStageBindings& bindings) {
    angle_stage(global, config, parameters, state, frame, player, globals, command, send_packet,
                bindings);
}
void process_angle_stage(const GlobalConfig& global, const Config& config,
                         const AngleParameters& parameters, AngleStageState& state,
                         AngleFrame& frame, const AnglePlayer& player, ActionGateGlobals& globals,
                         client::Command& command, std::uint8_t& send_packet,
                         const LiveAngleStageBindings& bindings) {
    angle_stage(global, config, parameters, state, frame, player, globals, command, send_packet,
                bindings);
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: body_freestanding.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
float neg(float x) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(x) ^ 0x80000000u);
}
float length(const RageVector& x) noexcept {
    return std::sqrt((x[0] * x[0] + x[1] * x[1]) + x[2] * x[2]);
}
RageVector subtract(RageVector a, RageVector b) {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}
RageVector normalized(RageVector a) {
    const float inverse = 1.0f / (length(a) + std::numeric_limits<float>::epsilon());
    return {inverse * a[0], inverse * a[1], inverse * a[2]};
}
RageVector angles(RageVector direction, const BodyFreestandingMath& math) {
    if (direction[0] == 0.0f && direction[1] == 0.0f)
        return {direction[2] > 0.0f ? 270.0f : 90.0f, 0.0f, 0.0f};
    constexpr float degrees = 57.2957763671875f;
    const float yaw =
        static_cast<float>(math.atan2(double(direction[1]), double(direction[0]))) * degrees;
    const float horizontal = std::sqrt(direction[0] * direction[0] + direction[1] * direction[1]);
    const float pitch =
        static_cast<float>(math.atan2(double(neg(direction[2])), double(horizontal))) * degrees;
    return {pitch, yaw, 0.0f};
}
RageVector transform(const Matrix3x4& matrix, const RageVector& point) noexcept {
    RageVector result;
    for (unsigned i = 0; i != 3; ++i)
        result[i] = ((matrix.rows[i][0] * point[0] + matrix.rows[i][1] * point[1]) +
                     matrix.rows[i][2] * point[2]) +
                    matrix.rows[i][3];
    return result;
}
template <class T> T read(const void* object, std::ptrdiff_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
bool build(BodyFreestandingBones& bones, const animation::BoneRequest& request,
           std::span<Matrix3x4> output, const RageTrigonometry& trig) {
    return animation::build_bone_matrices(*bones.state, *bones.model, request, output,
                                          *bones.engine, trig);
}
bool build(LiveBodyFreestandingBones& bones, const animation::BoneRequest& request,
           std::span<Matrix3x4> output, const RageTrigonometry&) {
    return bones.runtime.build_bone_matrices(bones.player, request, output);
}
std::uint32_t hitbox_bone(const BodyFreestandingBones& bones, unsigned index) noexcept {
    return bones.model->hitbox_bones[index];
}
std::uint32_t hitbox_bone(const LiveBodyFreestandingBones& bones, unsigned index) noexcept {
    return bones.hitbox_bones[index];
}
}
LiveBodyFreestandingModel capture_body_freestanding_model(const animation::BoneRuntime& runtime,
                                                          const void* player) {
    LiveBodyFreestandingModel result{};
    const auto* wrapper = read<const void*>(player, runtime.fields.studio_wrapper_32e4daf1);
    if (!wrapper)
        return result;
    const auto* header = read<const std::byte*>(wrapper, 0);
    if (!header)
        return result;
    const auto relative =
        std::uint32_t(read<std::int32_t>(player, runtime.fields.hitbox_set_c2b6fb10)) * 12u +
        std::uint32_t(read<std::int32_t>(header, 0xB0));
    const auto* set = header + std::bit_cast<std::int32_t>(relative);
    result.hitbox_set_present = true;
    result.hitbox_count = std::min(19, read<std::int32_t>(set, 4));
    if (result.hitbox_count <= 0)
        return result;
    const auto* boxes = set + read<std::int32_t>(set, 8);
    for (std::int32_t i = 0; i < result.hitbox_count; ++i) {
        const auto* box = boxes + std::ptrdiff_t(i) * 0x44;
        result.hitbox_bones[std::size_t(i)] = read<std::uint32_t>(box, 0);
        result.hitboxes[std::size_t(i)] = {read<std::int32_t>(box, 4),  read<RageVector>(box, 8),
                                           read<RageVector>(box, 0x14), read<RageVector>(box, 0x24),
                                           read<float>(box, 0x30),      box};
    }
    return result;
}
LiveBodyFreestandingBones bind_body_freestanding_bones(const animation::BoneRuntime& runtime,
                                                       void* player,
                                                       const LiveBodyFreestandingModel& model,
                                                       std::uint32_t eye_angles,
                                                       std::uint32_t pose_parameters) noexcept {
    auto* const bytes = static_cast<std::byte*>(player);
    auto* const pitch = reinterpret_cast<float*>(bytes + eye_angles);
    const auto count = std::size_t(std::max(0, model.hitbox_count));
    return {player,
            runtime,
            pitch,
            pitch + 1,
            reinterpret_cast<float*>(bytes + pose_parameters + 0x2C),
            model.hitbox_set_present,
            model.hitbox_count,
            std::int32_t(model.hitbox_bones[0]),
            model.hitboxes[0].minimum,
            model.hitboxes[0].maximum,
            {model.hitboxes.data(), count},
            {model.hitbox_bones.data(), count}};
}
namespace {
struct DecodedModelProjection {};
DecodedModelProjection capture_model(const BodyFreestandingBones&) noexcept {
    return {};
}
BodyFreestandingBones bind_model(const BodyFreestandingBones& bones,
                                 DecodedModelProjection) noexcept {
    return bones;
}
LiveBodyFreestandingModel capture_model(const LiveBodyFreestandingSource& source) {
    return capture_body_freestanding_model(source.runtime, source.player);
}
LiveBodyFreestandingBones bind_model(const LiveBodyFreestandingSource& source,
                                     const LiveBodyFreestandingModel& model) noexcept {
    return bind_body_freestanding_bones(source.runtime, source.player, model, source.eye_angles,
                                        source.pose_parameters);
}
}
template <class Bones>
bool solve_animation_body(const RageVector& frame_origin, const BodyFreestandingContext& context,
                          Bones& source, float maximum_yaw, float& result,
                          const BodyFreestandingMath& math, BodyFreestandingWorkspace& work) {
    if (context.target_handle == -1 ||
        !context.entities.by_handle(context.entities.object, context.target_handle))
        return false;
    const auto look = angles(normalized(subtract(context.shot_position, frame_origin)), math);
    const auto model = capture_model(source);
    auto bones = bind_model(source, model);
    if (!bones.hitbox_set_present || bones.hitbox_count == 0)
        return false;
    const float saved_pitch = *bones.eye_pitch, saved_yaw = *bones.eye_yaw,
                saved_pose = *bones.pose_parameter_11;
    *bones.eye_pitch = look[0];
    *bones.eye_yaw = look[1];
    const std::array<float, 3> offsets{maximum_yaw, 0.0f, neg(maximum_yaw)};
    for (unsigned i = 0; i != 3; ++i) {
        float pose = (offsets[i] - (-60.0f)) / 120.0f + 0.0f;
        pose = pose > 0.0f ? pose : 0.0f;
        pose = pose < 1.0f ? pose : 1.0f;
        *bones.pose_parameter_11 = pose;
        const auto origin = *engine::invoke<const RageVector*>(bones.player, 10);
        const animation::BoneRequest request{
            {0.0f, look[1] - offsets[i], 0.0f}, origin, 0.0f, 0x300, -1, 0, true};
        if (build(bones, request, work.matrices, math.trigonometry)) {
            const auto& matrix = work.matrices[unsigned(bones.first_hitbox_bone)];
            const auto minimum = transform(matrix, bones.first_hitbox_minimum);
            const auto maximum = transform(matrix, bones.first_hitbox_maximum);
            for (unsigned axis = 0; axis != 3; ++axis)
                work.centers[i][axis] = (minimum[axis] + maximum[axis]) * 0.5f;
        }
    }
    auto target = context.target_origin;
    target[2] += 64.0f;
    const auto forward = normalized(subtract(work.centers[1], target));
    RageVector side;
    if (forward[0] == 0.0f && forward[1] == 0.0f)
        side = {0.0f, -1.0f, 0.0f};
    else {
        const float common = forward[2] * 0.0f;
        side = normalized(
            {forward[1] - common, common - forward[0], forward[0] * 0.0f - forward[1] * 0.0f});
    }
    const auto displacement = [&](const RageVector& point) {
        const auto delta = subtract(point, work.centers[1]);
        const float projection = (delta[1] * side[1] + delta[0] * side[0]) + delta[2] * side[2];
        RageVector projected;
        for (unsigned i = 0; i != 3; ++i)
            projected[i] = (side[i] * projection + work.centers[1][i]) - work.centers[1][i];
        return length(projected);
    };
    result = displacement(work.centers[0]) > displacement(work.centers[2]) ? maximum_yaw
                                                                           : neg(maximum_yaw);
    *bones.pose_parameter_11 = saved_pose;
    *bones.eye_yaw = saved_yaw;
    *bones.eye_pitch = saved_pitch;
    return true;
}
template <class Bones>
bool occludes_head(const RageRay& ray, const Bones& bones, std::span<const Matrix3x4> matrices,
                   const RageBoxClipper& clipper) {
    const auto clip = [&](unsigned index, RageTrace& trace) {
        const auto& h = bones.hitboxes[index];
        const RageScanHitbox hitbox{
            h.hitgroup, {{}, {}, h.radius}, h.engine_hitbox, matrices[hitbox_bone(bones, index)]};
        return clipper.clip(clipper.context, ray, hitbox, trace);
    };
    RageTrace trace;
    if (clip(0, trace) < 0)
        return false;
    const float head_fraction = trace.fraction;
    for (unsigned i = 0; i != bones.hitboxes.size(); ++i) {
        const auto group = bones.hitboxes[i].hitgroup;
        if (group != 2 && group != 3)
            continue;
        trace.fraction = 1.0f;
        if (clip(i, trace) >= 0 && head_fraction > trace.fraction)
            return true;
    }
    return false;
}
template <class Bones>
bool solve_covered_body(const BodyFreestandingSettings& settings, const RageVector& frame_origin,
                        const BodyFreestandingContext& context, Bones& source, float command_pitch,
                        float base_yaw, float maximum_yaw, float& result,
                        const BodyFreestandingMath& math, const RageBoxClipper& clipper,
                        BodyFreestandingWorkspace& work) {
    if (context.target_handle == -1 ||
        !context.entities.by_handle(context.entities.object, context.target_handle))
        return false;
    if (settings.yaw_mode_2c != 1)
        return false;
    const float dx = frame_origin[0] - context.target_origin[0],
                dy = frame_origin[1] - context.target_origin[1];
    float yaw = 0.0f;
    if (dy != 0.0f || dx != 0.0f)
        yaw = static_cast<float>(math.atan2(double(dy), double(dx))) * 57.2957763671875f;
    float difference = normalize_movement_angle(yaw - base_yaw);
    if (difference < 0.0f)
        difference = neg(difference);
    if (difference > settings.maximum_angle_148)
        return false;
    const auto model = capture_model(source);
    auto bones = bind_model(source, model);
    if (!bones.hitbox_set_present || bones.hitbox_count == 0)
        return false;
    const float saved_pitch = *bones.eye_pitch, saved_yaw = *bones.eye_yaw,
                saved_pose = *bones.pose_parameter_11;
    *bones.eye_pitch = command_pitch;
    *bones.eye_yaw = base_yaw;
    const std::array<float, 3> offsets{maximum_yaw, 0.0f, neg(maximum_yaw)};
    auto target = context.target_origin;
    target[2] += 64.0f;
    unsigned maximum = 0, minimum = 0xFFFFFFFFu;
    for (unsigned hypothesis = 0; hypothesis != 3; ++hypothesis) {
        float pose = (offsets[hypothesis] - (-60.0f)) / 120.0f + 0.0f;
        pose = pose > 0.0f ? pose : 0.0f;
        pose = pose < 1.0f ? pose : 1.0f;
        *bones.pose_parameter_11 = pose;
        const auto origin = *engine::invoke<const RageVector*>(bones.player, 10);
        const animation::BoneRequest request{
            {0.0f, base_yaw - offsets[hypothesis], 0.0f}, origin, 0.0f, 0x300, -1, 0, true};
        if (!build(bones, request, work.matrices, math.trigonometry))
            continue;
        const auto& matrix = work.matrices[unsigned(bones.first_hitbox_bone)];
        const auto low = transform(matrix, bones.first_hitbox_minimum),
                   high = transform(matrix, bones.first_hitbox_maximum);
        RageVector center;
        for (unsigned i = 0; i != 3; ++i)
            center[i] = (high[i] + low[i]) * 0.5f;
        const auto forward = normalized(subtract(center, target));
        RageVector side, up;
        if (forward[0] == 0.0f && forward[1] == 0.0f) {
            side = {0.0f, -1.0f, 0.0f};
            up = {neg(forward[2]), 0.0f, 0.0f};
        } else {
            const float common = forward[2] * 0.0f;
            side = normalized(
                {forward[1] - common, common - forward[0], forward[0] * 0.0f - forward[1] * 0.0f});
            up = normalized({side[1] * forward[2] - side[2] * forward[1],
                             side[2] * forward[0] - side[0] * forward[2],
                             side[0] * forward[1] - side[1] * forward[0]});
        }
        constexpr std::array<float, 5> side_weights{-1.0f, -0.5f, 0.0f, 0.5f, 1.0f};
        constexpr std::array<float, 5> up_weights{0.25f, 0.75f, 1.0f, 0.75f, 0.25f};
        const float radius = bones.hitboxes[0].radius * 0.7f;
        const auto axis = subtract(high, low);
        unsigned coverage = 0;
        for (unsigned sample = 0; sample != 5; ++sample) {
            RageVector direction;
            for (unsigned i = 0; i != 3; ++i)
                direction[i] = side[i] * side_weights[sample] + up[i] * up_weights[sample];
            direction = normalized(direction);
            const float dot =
                (axis[1] * direction[1] + axis[0] * direction[0]) + axis[2] * direction[2];
            const auto& endpoint = dot < 0.0f ? low : high;
            const float distance = radius / length(direction);
            RageVector point;
            for (unsigned i = 0; i != 3; ++i)
                point[i] = distance * direction[i] + endpoint[i];
            const auto to_point = subtract(point, target);
            const float extension =
                40.0f / (length(to_point) + std::numeric_limits<float>::epsilon());
            RageRay ray{target, {}};
            for (unsigned i = 0; i != 3; ++i)
                ray.delta[i] = (extension * to_point[i] + point[i]) - target[i];
            if (occludes_head(ray, bones, work.matrices, clipper))
                ++coverage;
        }
        if (coverage > maximum) {
            result = offsets[hypothesis];
            maximum = coverage;
        }
        if (coverage < minimum)
            minimum = coverage;
    }
    *bones.pose_parameter_11 = saved_pose;
    *bones.eye_yaw = saved_yaw;
    *bones.eye_pitch = saved_pitch;
    return minimum < maximum;
}
bool solve_animation_body_freestanding(const RageVector& origin,
                                       const BodyFreestandingContext& context,
                                       BodyFreestandingBones& bones, float yaw, float& result,
                                       const BodyFreestandingMath& math,
                                       BodyFreestandingWorkspace& work) {
    return solve_animation_body(origin, context, bones, yaw, result, math, work);
}
bool solve_animation_body_freestanding(const RageVector& origin,
                                       const BodyFreestandingContext& context,
                                       LiveBodyFreestandingSource& bones, float yaw, float& result,
                                       const BodyFreestandingMath& math,
                                       BodyFreestandingWorkspace& work) {
    return solve_animation_body(origin, context, bones, yaw, result, math, work);
}
bool body_occludes_head(const RageRay& ray, const BodyFreestandingBones& bones,
                        std::span<const Matrix3x4> matrices, const RageBoxClipper& clipper) {
    return occludes_head(ray, bones, matrices, clipper);
}
bool body_occludes_head(const RageRay& ray, const LiveBodyFreestandingBones& bones,
                        std::span<const Matrix3x4> matrices, const RageBoxClipper& clipper) {
    return occludes_head(ray, bones, matrices, clipper);
}
bool solve_covered_body_freestanding(const BodyFreestandingSettings& settings,
                                     const RageVector& origin,
                                     const BodyFreestandingContext& context,
                                     BodyFreestandingBones& bones, float pitch, float yaw,
                                     float maximum, float& result, const BodyFreestandingMath& math,
                                     const RageBoxClipper& clipper,
                                     BodyFreestandingWorkspace& work) {
    return solve_covered_body(settings, origin, context, bones, pitch, yaw, maximum, result, math,
                              clipper, work);
}
bool solve_covered_body_freestanding(const BodyFreestandingSettings& settings,
                                     const RageVector& origin,
                                     const BodyFreestandingContext& context,
                                     LiveBodyFreestandingSource& bones, float pitch, float yaw,
                                     float maximum, float& result, const BodyFreestandingMath& math,
                                     const RageBoxClipper& clipper,
                                     BodyFreestandingWorkspace& work) {
    return solve_covered_body(settings, origin, context, bones, pitch, yaw, maximum, result, math,
                              clipper, work);
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: body_yaw.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
float negate(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) ^ 0x80000000u);
}
float clamp_unit(float value) noexcept {
    return _mm_cvtss_f32(
        _mm_min_ss(_mm_max_ss(_mm_set_ss(value), _mm_setzero_ps()), _mm_set_ss(1.0f)));
}
float clamp(float value, float lower, float upper) noexcept {
    return _mm_cvtss_f32(
        _mm_min_ss(_mm_max_ss(_mm_set_ss(value), _mm_set_ss(lower)), _mm_set_ss(upper)));
}
float absolute(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) & 0x7FFFFFFFu);
}
float normalized(float value) noexcept {
    if (std::isnan(value))
        return 0.0f;
    if (absolute(value) > 180.0f) {
        const auto correction = round_movement_step(absolute(value / 360.0f)) * 360.0f;
        if (value < 0.0f)
            value += correction;
        else
            value -= correction;
    }
    return value;
}
float angle_difference(float destination, float source) noexcept {
    auto delta = static_cast<float>(std::fmod(static_cast<double>(destination - source), 360.0));
    if (destination > source) {
        if (delta >= 180.0f)
            delta -= 360.0f;
    } else if (delta <= -180.0f)
        delta += 360.0f;
    return delta;
}
}
void animation_yaw_limits(const AnimationYawInput& input, float& minimum, float& maximum) noexcept {
    auto modifier = (input.speed_fraction_11c * std::bit_cast<float>(0xBE99999Au) -
                     std::bit_cast<float>(0x3E4CCCCDu)) *
                        clamp_unit(input.running_fraction_f8) +
                    1.0f;
    if (input.duck_amount_a4 > 0.0f) {
        const auto blend = input.duck_amount_a4 * clamp_unit(input.ducking_fraction_fc);
        modifier += blend * (0.5f - modifier);
    }
    maximum = input.maximum_yaw_338 * modifier;
    minimum = input.minimum_yaw_334 * modifier;
}
float regular_body_yaw(const BodyYawSettings& settings, BodyYawState& state,
                       std::int32_t frame_command, std::int32_t last_command, float base_yaw,
                       const JitterRandom& random) {
    const bool opposite =
        (last_command == frame_command && (settings.mode != 3 || settings.static_offset_9c != 0)) ||
        settings.mode == 1;
    if (opposite) {
        auto delta = static_cast<float>(
            std::fmod(static_cast<double>(base_yaw - state.reference_yaw_258), 360.0));
        if (base_yaw > state.reference_yaw_258) {
            if (delta >= 180.0f)
                delta -= 360.0f;
        } else if (delta <= -180.0f)
            delta += 360.0f;
        const auto absolute =
            std::bit_cast<float>(std::bit_cast<std::uint32_t>(delta) & 0x7FFFFFFFu);
        if (absolute > 5.0f)
            state.cached_opposite_1a0 =
                delta > 0.0f ? negate(settings.opposite_angle_134) : settings.opposite_angle_134;
        return state.cached_opposite_1a0;
    }
    if (settings.mode == 3)
        return _mm_cvtss_f32(_mm_cvtsi32_ss(_mm_setzero_ps(), settings.static_offset_9c));
    if (settings.jitter_mode == 4)
        return settings.skitter[state.phase_190 % 12u] * settings.opposite_angle_134;
    float first, second;
    if (settings.jitter_seed_98 == -180) {
        first = negate(settings.jitter_maximum_144);
        second = 0.0f;
    } else if (settings.jitter_seed_98 == 0) {
        first = negate(settings.jitter_maximum_144);
        second = settings.jitter_maximum_144;
    } else if (settings.jitter_seed_98 == 180) {
        first = 0.0f;
        second = settings.jitter_maximum_144;
    } else {
        random.seed(random.context, settings.jitter_seed_98);
        first = random.range(random.context, negate(settings.jitter_maximum_144), 0.0f);
        second = random.range(random.context, 0.0f, settings.jitter_maximum_144);
    }
    return state.phase_190 & 1u ? second : first;
}
void blend_body_yaw(const BodyBlendSettings& settings, const BodyBlendInput& input,
                    float& command_yaw, std::uint8_t& send_packet) noexcept {
    const auto desired = clamp(input.offset, input.minimum_yaw, input.maximum_yaw);
    const auto goal = input.base_yaw - desired;
    auto delta = angle_difference(goal, input.feet_yaw);
    if (_mm_cvttss_si32(_mm_set_ss(input.offset)) != 0) {
        const auto direction = normalized(input.base_yaw - input.feet_yaw);
        if (!send_packet && input.choked_commands < input.choke_limit) {
            const auto amplitude = clamp(absolute(input.offset), settings.minimum_magnitude_134,
                                         settings.maximum_magnitude_138);
            auto expanded = settings.scale_154 * input.maximum_yaw + settings.expanded_angle_144;
            if (settings.minimum_magnitude_134 == settings.maximum_magnitude_138) {
                if (amplitude < settings.maximum_magnitude_138)
                    expanded = settings.base_angle_13c;
            } else
                expanded = ((expanded - settings.base_angle_13c) *
                            (amplitude - settings.minimum_magnitude_134)) /
                               (settings.maximum_magnitude_138 - settings.minimum_magnitude_134) +
                           settings.base_angle_13c;
            if (input.offset > 0.0f) {
                command_yaw = input.base_yaw - settings.base_angle_13c;
                if (direction > 0.0f)
                    command_yaw = input.base_yaw - expanded;
            } else {
                command_yaw = input.base_yaw + settings.base_angle_13c;
                if (direction < 0.0f)
                    command_yaw = expanded + input.base_yaw;
            }
        } else if (input.offset > 0.0f) {
            if (direction < 0.0f && input.choked_commands < input.choke_limit) {
                command_yaw = input.base_yaw - settings.base_angle_13c;
                send_packet = 0;
            }
        } else if (direction > 0.0f && input.choked_commands < input.choke_limit) {
            command_yaw = settings.base_angle_13c + input.base_yaw;
            send_packet = 0;
        }
        return;
    }
    if (!(absolute(delta) >= 5.0f) || input.choked_commands >= input.choke_limit) {
        if (!send_packet)
            command_yaw = goal;
        return;
    }
    send_packet = 0;
    if (delta > 0.0f)
        delta += input.maximum_yaw;
    else
        delta -= input.maximum_yaw;
    if (absolute(delta) <= settings.maximum_magnitude_138)
        command_yaw = input.feet_yaw + delta;
    else {
        const auto width = settings.expanded_angle_144 - input.maximum_yaw;
        const auto plus = width + input.feet_yaw;
        const auto minus = input.feet_yaw - width;
        const auto positive =
            clamp(angle_difference(input.base_yaw, plus), input.minimum_yaw, input.maximum_yaw);
        const auto negative =
            clamp(angle_difference(input.base_yaw, minus), input.minimum_yaw, input.maximum_yaw);
        if (absolute(positive - desired) > absolute(negative - desired))
            command_yaw = input.feet_yaw - settings.expanded_angle_144;
        else
            command_yaw = input.feet_yaw + settings.expanded_angle_144;
    }
}
template <class Special>
float solve_body(const BodyYawSettings& settings, BodyYawState& state,
                 const BodyYawRequest& request, const Special& special,
                 const JitterRandom& random) {
    float result = 0.0f;
    if (request.freestanding_body_yaw && request.last_command != request.frame_command &&
        solve_covered_body_freestanding(special.settings, request.frame_origin_dc, special.context,
                                        special.bones, request.command_pitch, request.base_yaw,
                                        request.maximum_yaw, result, special.math, special.clipper,
                                        special.workspace))
        return result;
    if (request.freestanding_body_yaw && request.last_command == request.frame_command &&
        request.shot_command == request.frame_command &&
        solve_animation_body_freestanding(request.frame_origin_dc, special.context, special.bones,
                                          request.maximum_yaw, result, special.math,
                                          special.workspace))
        return result;
    return regular_body_yaw(settings, state, request.frame_command, request.last_command,
                            request.base_yaw, random);
}
float solve_body_yaw(const BodyYawSettings& settings, BodyYawState& state,
                     const BodyYawRequest& request, const BodyYawFreestandingBindings& special,
                     const JitterRandom& random) {
    return solve_body(settings, state, request, special, random);
}
float solve_body_yaw(const BodyYawSettings& settings, BodyYawState& state,
                     const BodyYawRequest& request, const LiveBodyYawFreestandingBindings& special,
                     const JitterRandom& random) {
    return solve_body(settings, state, request, special, random);
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: command_owner.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
template <class T> T field(const void* object, std::uint32_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
template <class T> T& slot(void* object, std::uint32_t offset) noexcept {
    return *reinterpret_cast<T*>(static_cast<std::byte*>(object) + offset);
}
const void* shared_player(const CommandOwner& owner) noexcept {
    return owner.globals.shared_player ? *owner.globals.shared_player : nullptr;
}
bool shared_guard(const CommandOwner& owner) noexcept {
    const auto* shared = shared_player(owner);
    return shared && field<std::uint8_t>(shared, owner.offsets.guard_d720aea6) != 0;
}
const void* entity_by_handle(void* object, std::int32_t handle) {
    return engine::invoke<const void*>(object, 4, handle);
}
class PlayerEdge final : public EdgeEngine {
    void* player_;
    CommandOwner& owner_;

public:
    PlayerEdge(void* player, CommandOwner& owner) : player_(player), owner_(owner) {}
    const MovementVector& hull_minimum() override {
        return *engine::invoke<const MovementVector*>(player_, 76);
    }
    const MovementVector& hull_maximum() override {
        return *engine::invoke<const MovementVector*>(player_, 77);
    }
    const MovementVector& origin() override {
        return *engine::invoke<const MovementVector*>(player_, 10);
    }
    MovementAngles view_angles() override {
        MovementAngles result;
        engine::invoke<void>(owner_.engine_client, 18, &result);
        return result;
    }
    std::uint32_t point_contents_world(const MovementVector& point, std::uint32_t mask) override {
        return engine::invoke<std::uint32_t>(owner_.engine_trace, 1, &point, mask);
    }
    void trace_ray(const MovementCollisionRay& ray, std::uint32_t mask,
                   MovementCollisionFilter filter, MovementCollisionTrace& trace) override {
        owner_.phase2.collision.trace_ray(ray, mask, filter, trace);
    }
    float surface_friction(std::int16_t property) override {
        return owner_.phase2.collision.surface_friction(property);
    }
};
struct PlayerPitch {
    void* player;
    const CommandOwnerPitchRandomBindings& imports;
    static void seed(void* context, std::int32_t seed_value) {
        const auto& self = *static_cast<PlayerPitch*>(context);
        self.imports.seed(self.imports.context, seed_value);
    }
    static std::int32_t integer(void* context, std::int32_t minimum, std::int32_t maximum) {
        const auto& self = *static_cast<PlayerPitch*>(context);
        return self.imports.integer(self.imports.context, minimum, maximum);
    }
    static std::int32_t code(void* context) {
        return engine::invoke<std::int32_t>(static_cast<PlayerPitch*>(context)->player, 122);
    }
};
AnglePlayer capture_angle_player(const CommandOwner& owner, const client::CommandFrame& frame,
                                 void* player) {
    const auto& o = owner.offsets;
    const auto& entity = owner.phase6.entity_offsets;
    const auto* animation = field<const void*>(player, entity.animation_state);
    AnglePlayer result{};
    if (frame.weapon_kind_124 == 9 || *owner.globals.mode == owner.state.value_158) {
        const auto* weapon = reinterpret_cast<const void*>(std::uintptr_t(frame.weapon_address));
        result.gates.weapon_state_e1df7e9b =
            field<std::uint8_t>(weapon, o.weapon_state_e1df7e9b) != 0;
        result.gates.weapon_time_3b93667b = field<float>(weapon, o.weapon_time_3b93667b);
    }
    result.gates.tick_base_3d10b94c = field<std::int32_t>(player, o.tick_base_3d10b94c);
    result.gates.movement_type_8b5586f6 = field<std::int32_t>(player, o.movement_type_8b5586f6);
    result.gates.field_7fcb7381 = field<std::uint8_t>(player, o.field_7fcb7381) != 0;
    result.gates.has_weapon_3d52a1a7 = animation != nullptr;
    result.gates.entity = reinterpret_cast<ProximityEntity>(player);
    result.edge_scalar_3eb71843 = field<float>(player, o.edge_3eb71843);
    result.state_7501cb91 = field<std::uint8_t>(player, o.movement_type_7501cb91);
    if (animation) {
        result.weapon_pitch_340 = field<float>(animation, 0x340);
        result.animation = {field<float>(animation, 0xA4),  field<float>(animation, 0xF8),
                            field<float>(animation, 0xFC),  field<float>(animation, 0x11C),
                            field<float>(animation, 0x334), field<float>(animation, 0x338)};
        result.feet_yaw_80 = field<float>(animation, 0x80);
    }
    return result;
}
}
AngleFrame capture_angle_frame(const client::CommandFrame& frame) noexcept {
    return {{frame.flags, frame.previous_command.movement.buttons, frame.choke_limit_fc,
             frame.choked_15c, frame.weapon_id_d8, frame.state_104, frame.weapon_kind_124,
             frame.previous_command.angles[0], frame.previous_command.angles[1]},
            frame.scalar_c0,
            frame.previous_command.movement.forward_move,
            frame.height_164,
            field<float>(&frame, 0x9C),
            frame.angle_b8,
            field<float>(&frame, 0xA0),
            frame.velocity_150[2],
            frame.origin_10c,
            frame.origin_168,
            frame.origin_dc,
            frame.command_number_ac};
}
void command_owner_phase2(CommandOwner& owner, void* player, void* weapon, client::Command& command,
                          const std::array<float, 3>& incoming_angles, std::uint8_t& send_packet) {
    (void)weapon; // native phase2's first stack argument is unused
    if (!owner.global.enabled.value) {
        owner.state.flags_130 &= ~2u;
        return;
    }
    const auto& o = owner.offsets;
    const auto* animation = field<const void*>(player, owner.phase6.entity_offsets.animation_state);
    const PrecalculationPlayer snapshot{
        field<std::uint32_t>(player, o.player_flags_e2fbe695),
        animation != nullptr,
        animation && *owner.globals.choked == 0 ? field<float>(animation, 0x80) : 0.0f,
        shared_guard(owner),
        field<std::uint8_t>(player, o.movement_type_7501cb91),
        field<MovementVector>(player, o.velocity_36375c23),
        field<float>(player, o.duck_3309c0cf),
        field<float>(player, o.rate_fb7e3404),
        field<std::uint8_t>(player, o.ducking_6b1a59b5) != 0};
    const auto& base = owner.phase2;
    auto live_environment = owner.environment;
    live_environment.interval = owner.globals.interval;
    live_environment.entity_list = owner.entity_list;
    live_environment.engine_trace_flag = owner.globals.trace_flag;
    const MovementCaptureInput movement{owner.movement.offsets, owner.movement.convars,
                                        *owner.globals.interval, &live_environment};
    auto roll_guard = base.roll_guard;
    roll_guard.shared_player = owner.globals.shared_player;
    roll_guard.player_guard = owner.offsets.rule_bf567619;
    const PrecalculationLiveFields live{o.duck_3309c0cf,       o.rate_fb7e3404,
                                        o.ducking_6b1a59b5,    o.player_flags_e2fbe695,
                                        &owner.globals.choked, &owner.globals.interval};
    const LivePrecalculationBindings bindings{
        player, movement, live_environment, base.collision, base.workspace, roll_guard, live};
    precalculate_live_command_angles(owner.global, owner.config, owner.precalculation, snapshot,
                                     *owner.globals.choked, command, incoming_angles, send_packet,
                                     bindings);
}
void command_owner_phase4(CommandOwner& owner, client::CommandFrame& frame, void* player,
                          client::Command& command, std::uint8_t& send_packet) {
    if (!owner.global.enabled.value || (frame.secondary_flags & 4u) ||
        (!owner.global.pitch.value && !owner.global.yaw.value && !owner.global.body_yaw.value)) {
        update_command_flags(frame.flags, owner.state.flags_130);
        return;
    }
    const auto* shared = shared_player(owner);
    const bool guard = shared_guard(owner);
    const EligibilityGuard eligibility_guard{
        shared != nullptr, guard, guard ? field<float>(shared, owner.offsets.clock_9f8e7f71) : 0.0f,
        *owner.globals.current_time};
    const auto& eligibility_source = owner.phase4.eligibility;
    const EligibilityBindings eligibility{
        eligibility_source.input_system, eligibility_source.engine_client,
        eligibility_source.hud_context, eligibility_source.find_hud};
    if (!command_eligible(
            {true, owner.global.pitch.value, owner.global.yaw.value, owner.global.body_yaw.value},
            {frame.flags, frame.secondary_flags}, eligibility_guard, owner.state.value_1c4,
            eligibility, owner.state.flags_130))
        return;
    const auto cached = command_uses_cached_angles(
        field<std::uint8_t>(player, owner.offsets.cached_a15c3105) != 0,
        field<std::int32_t>(player, owner.offsets.movement_type_8b5586f6), frame.flags,
        command.movement.buttons, owner.phase4.angles.proximity,
        reinterpret_cast<ProximityEntity>(player));
    AngleCycleState cycle{owner.state.flags_130, owner.state.value_190, owner.state.saved_yaw_198};
    begin_angle_cycle(cached, frame.choked_15c, owner.config.yaw_offsets.value[3], cycle);
    owner.state.flags_130 = cycle.flags_130;
    owner.state.value_190 = cycle.phase_190;
    owner.state.saved_yaw_198 = cycle.saved_yaw_198;

    if (owner.config.yaw_base.value == 1 || owner.config.freestanding.value ||
        owner.config.freestanding_body_yaw.value)
        owner.phase4.target_refresh.refresh(owner.globals.target, player, command,
                                            frame.previous_command.movement.buttons,
                                            field<float>(&frame, 0x11C));
    if (frame.state_104 == 8)
        return;
    owner.state.reference_yaw_258 = field<float>(owner.finisher.animation_1d8.data(), 0x80);

    auto angle_frame = capture_angle_frame(frame);
    const auto angle_player = capture_angle_player(owner, frame, player);
    const auto& base = owner.phase4.angles;
    const FreestandingContext freestanding{
        owner.globals.scheduler_flags, owner.globals.height_hotkey,
        owner.globals.input,           owner.globals.target.handle,
        owner.globals.target.position, {owner.entity_list, entity_by_handle}};
    const BodyFreestandingContext body_context{owner.globals.target.handle,
                                               owner.globals.target.position,
                                               owner.globals.shot_position, freestanding.entities};
    LiveBodyFreestandingSource body_bones{player, owner.phase6.bones,
                                          owner.phase6.entity_offsets.eye_angles,
                                          owner.phase6.entity_offsets.pose_parameters};
    const LiveBodyYawFreestandingBindings body{
        {owner.config.yaw_base.value, owner.parameters.value_148},
        body_context,
        body_bones,
        base.body_math,
        base.clipper,
        base.workspace};
    PlayerEdge edge(player, owner);
    PlayerPitch pitch_context{player, base.pitch};
    const PitchInterfaces pitch{&pitch_context, PlayerPitch::seed, PlayerPitch::integer,
                                PlayerPitch::code};
    const LiveAngleStageBindings angles{base.proximity,
                                        edge,
                                        base.ballistics,
                                        freestanding,
                                        body,
                                        pitch,
                                        base.random,
                                        owner.globals.trace_flag,
                                        owner.globals.last_body_command,
                                        owner.globals.shot_command,
                                        owner.globals.last_sent_command,
                                        owner.globals.shot_correction != 0};
    ActionGateGlobals globals{*owner.globals.mode,         *owner.globals.choked,
                              *owner.globals.interval,     owner.globals.scheduler_flags,
                              owner.globals.target.handle, freestanding.entities};
    process_angle_stage(owner.global, owner.config, owner.parameters, owner.state, angle_frame,
                        angle_player, globals, command, send_packet, angles);
    frame.flags = angle_frame.gates.flags;
    owner.globals.scheduler_flags = globals.command_flags;
}
void command_owner_phase6(CommandOwner& owner, client::CommandFrame& frame, void* player,
                          client::Command& command, std::uint8_t& send_packet) {
    if (!owner.global.enabled.value || !owner.global.body_yaw.value || !send_packet)
        return;
    const auto& base = owner.phase6;
    if (!field<const void*>(player, base.entity_offsets.animation_state))
        return;
    const auto& o = owner.offsets;
    const animation::AnimationAngleHistory history{owner.globals.pending_angles_active != 0,
                                                   *owner.globals.mode,
                                                   owner.globals.pending_angles};
    const animation::AnimationUpdateTiming timing{
        *owner.globals.sequence, *owner.globals.current_time, *owner.globals.interval, nullptr,
        &base.reset_byte};
    const auto update = lagcomp::native_history_animation_engine(base.animation_update);
    const LiveFinisherBindings bindings{player,
                                        slot<void*>(player, base.entity_offsets.animation_state),
                                        slot<float>(player, base.entity_offsets.field_a2a8247f),
                                        base.simulation_time,
                                        owner.state.build_bones_12c,
                                        slot<RageVector>(player, base.entity_offsets.eye_angles),
                                        base.final_angles,
                                        base.entity_offsets,
                                        history,
                                        timing,
                                        update,
                                        base.set_absolute_angles,
                                        base.bones};
    const FinisherPlayer snapshot{
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(player)),
        field<std::uint32_t>(player, o.flags_38ca5228),
        field<std::uint8_t>(player, o.field_043fa575),
        field<std::uint8_t>(player, o.field_334b460c), field<float>(player, o.field_86a5f8b3 - 4u)};
    finish_command_angles(owner.global.enabled.value != 0, owner.global.body_yaw.value,
                          owner.finisher, snapshot, frame.command_number_ac, command, send_packet,
                          bindings);
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: command_phase.cpp
// ============================================================================

namespace skeet::anti_aim {
void process_command_angles(const GlobalConfig& global, const Config& config,
                            const AngleParameters& parameters, AngleStageState& state,
                            AngleFrame& frame, const AnglePlayer& player,
                            ActionGateGlobals& globals, client::Command& command,
                            std::uint8_t& send_packet, TargetState& target,
                            const CommandPhaseInput& input, const CommandPhaseBindings& bindings) {
    if (!command_eligible({global.enabled.value != 0, global.pitch.value, global.yaw.value,
                           global.body_yaw.value},
                          {frame.gates.flags, input.secondary_frame_flags}, input.guard,
                          state.value_1c4, bindings.eligibility, state.flags_130))
        return;

    const auto cached = command_uses_cached_angles(
        input.player_a15c3105, player.gates.movement_type_8b5586f6, frame.gates.flags,
        command.movement.buttons, bindings.angles.proximity, player.gates.entity);
    AngleCycleState cycle{state.flags_130, state.value_190, state.saved_yaw_198};
    begin_angle_cycle(cached, frame.gates.choked_15c, config.yaw_offsets.value[3], cycle);
    state.flags_130 = cycle.flags_130;
    state.value_190 = cycle.phase_190;
    state.saved_yaw_198 = cycle.saved_yaw_198;

    if (config.yaw_base.value == 1 || config.freestanding.value ||
        config.freestanding_body_yaw.value)
        refresh_yaw_target(target, bindings.target_refresh);
    globals.target_handle = target.handle;

    // All three native consumers read the same target publication. Retain the
    // shot position from434759D0 independently of the target's current origin.
    auto freestanding = bindings.angles.freestanding;
    freestanding.target_handle = target.handle;
    freestanding.target = target.position;
    auto body_context = bindings.angles.body.context;
    body_context.target_handle = target.handle;
    body_context.target_origin = target.position;
    body_context.entities = globals.entities;
    const BodyYawFreestandingBindings body{{config.yaw_base.value, parameters.value_148},
                                           body_context,
                                           bindings.angles.body.bones,
                                           bindings.angles.body.math,
                                           bindings.angles.body.clipper,
                                           bindings.angles.body.workspace};
    const AngleStageBindings stage{bindings.angles.proximity,
                                   bindings.angles.edge,
                                   bindings.angles.ballistics,
                                   freestanding,
                                   body,
                                   bindings.angles.pitch,
                                   bindings.angles.random,
                                   bindings.angles.trace_flag,
                                   bindings.angles.last_body_command,
                                   bindings.angles.shot_command,
                                   bindings.angles.last_sent_command,
                                   bindings.angles.shot_correction};
    process_angle_stage(global, config, parameters, state, frame, player, globals, command,
                        send_packet, stage);
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: config.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
template <class Value>
void register_descriptor(ConfigLink& registry, ConfigValue<Value>& item, std::uint32_t hash,
                         ConfigType type) noexcept {
    item.hash = hash;
    item.serialized_size = sizeof(Value);
    item.type = type;
    append_config_link(registry, item);
}
template <std::size_t N>
void initialize_offsets(ConfigLink& registry, ConfigValue<std::array<std::int32_t, N>>& item,
                        std::uint32_t hash) noexcept {
    register_descriptor(registry, item, hash, ConfigType::integer_array);
    item.value.fill(0);
}
}
void register_global_config(ConfigRegistries& registries, GlobalConfig& config) noexcept {
    register_descriptor(registries.descriptors, config.enabled, 0x96C0EF29u, ConfigType::boolean);
    register_descriptor(registries.descriptors, config.pitch, 0x6D5F1172u, ConfigType::integer);
    register_descriptor(registries.descriptors, config.yaw, 0xF2D408F3u, ConfigType::integer);
    register_descriptor(registries.descriptors, config.body_yaw, 0x4D6D88C6u, ConfigType::integer);
}
void initialize_owner_config(ConfigRegistries& registries, Config& config) noexcept {
    auto& registry = registries.descriptors;
    initialize_integer_config(registry, config.yaw_base, 0x1D43759Bu, 0);
    initialize_integer_config(registry, config.yaw_jitter, 0x177D7470u, 0);
    initialize_integer_config(registry, config.yaw_jitter_offset, 0xDCA4B692u, 0);
    initialize_boolean_config(registry, config.edge_yaw, 0x8C526C4Fu);
    initialize_boolean_config(registry, config.freestanding, 0x86C04DC4u);
    initialize_offsets(registry, config.body_yaw_offsets, 0x8DFD54BDu);
    initialize_offsets(registry, config.yaw_offsets, 0xFE49974Du);
    initialize_hotkey_config(registries, config.freestanding_hotkey, 0x697E8050u, 0x1Bu,
                             HotkeyMode::always);
    initialize_boolean_config(registry, config.freestanding_body_yaw, 0x2716923Fu);
    initialize_integer_config(registry, config.roll, 0xA1DFC38Du, 0);
    initialize_integer_config(registry, config.custom_pitch, 0x01DE265Eu, 0);
}
void finish_owner_config(Config& config) noexcept {
    config.yaw_offsets.value[1] = -12;
    config.yaw_offsets.value[3] = 80;
}
void initialize_config(ConfigRegistries& registries, Config& config) noexcept {
    initialize_owner_config(registries, config);
    finish_owner_config(config);
}
void initialize_fake_lag_enabled(ConfigRegistries& registries, FakeLagSettings& settings) noexcept {
    initialize_boolean_config(registries.descriptors, settings.enabled, 0xF9BBEC7Bu);
}
void initialize_fake_lag_hotkey(ConfigRegistries& registries, FakeLagSettings& settings) noexcept {
    initialize_hotkey_config(registries, settings.hotkey, 0x27D21BD4u, 0x1Bu, HotkeyMode::always);
}
void initialize_fake_lag_settings(ConfigRegistries& registries,
                                  FakeLagSettings& settings) noexcept {
    initialize_fake_lag_enabled(registries, settings);
    initialize_fake_lag_hotkey(registries, settings);
    initialize_integer_config(registries.descriptors, settings.amount, 0x2A71D5FCu, 0);
    initialize_integer_config(registries.descriptors, settings.variance, 0xCA9768E6u, 0);
    initialize_integer_config(registries.descriptors, settings.limit, 0xF17FEF11u, 13);
}
void initialize_other_global_settings(ConfigRegistries& registries,
                                      OtherGlobalSettings& settings) noexcept {
    initialize_hotkey_config(registries, settings.slow_motion_key, 0x7293DE2Au, 0x14u,
                             HotkeyMode::hold);
    initialize_integer_config(registries.descriptors, settings.leg_movement, 0x8F8A4366u, 0);
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: edge.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
bool blocked(const MovementCollisionTrace& trace) noexcept {
    return trace.fraction < 1.0f || trace.all_solid || trace.start_solid;
}
void initial_view_basis(MovementAngles angles) {
    constexpr float radians = 0.01745329238474369049072265625f;
    const auto yaw = angles.yaw * radians;
    const auto sy = std::sin(yaw), cy = std::cos(yaw);
    const auto pitch = angles.pitch * radians;
    const volatile auto sp = std::sin(pitch);
    (void)sp;
    const auto cp = std::cos(pitch);
    const auto x = cp * cy, y = cp * sy;
    const auto inverse =
        1.0f / (std::sqrt((x * x + y * y) + 0.0f) + std::numeric_limits<float>::epsilon());
    const auto forward_x = x * inverse, forward_y = y * inverse;
    const auto forward_z = inverse * 0.0f;
    if (forward_x != 0.0f || forward_y != 0.0f) {
        const auto common = forward_z * 0.0f;
        const auto side_x = forward_y - common, side_y = common - forward_x;
        const auto side_z = forward_x * 0.0f - forward_y * 0.0f;
        // The original stores this temporary before the radial loop reuses it.
        const volatile auto side_length =
            std::sqrt((side_x * side_x + side_y * side_y) + side_z * side_z);
        (void)side_length;
    }
}
}

bool solve_edge_yaw(const EdgeInput& input, const EdgeSettings& settings, EdgeEngine& engine,
                    std::uint8_t* trace_flag, float& command_yaw) {
    if (!input.enabled || input.frame_scalar_c0 >= input.player_scalar || input.movement_type == 9)
        return false;
    const auto& player_minimum = engine.hull_minimum();
    const auto& player_maximum = engine.hull_maximum();
    const auto padding = settings.hull_padding;
    const MovementVector minimum{player_minimum.x - padding, player_minimum.y - padding, -8.0f};
    const MovementVector maximum{player_maximum.x + padding, player_maximum.y + padding, 8.0f};
    auto origin = engine.origin();
    origin.z += 56.0f;
    MovementCollisionTrace trace{};
    trace.fraction = 1.0f;
    trace_movement_world(engine, make_movement_hull_ray(origin, origin, minimum, maximum), 1, trace,
                         trace_flag);
    if (!blocked(trace))
        return false;
    const auto first_view = engine.view_angles();
    initial_view_basis(first_view);

    std::int32_t sample_count = 0, normal_count = 0;
    MovementVector total{}, normals{};
    for (float angle = 0.0f; angle < std::bit_cast<float>(0x40C90FDBu);
         angle += std::bit_cast<float>(0x3EA0D97Cu)) {
        const auto sine = std::sin(angle), cosine = std::cos(angle); // complete 0x433C84EF
        const MovementVector direction{cosine, sine, 0.0f};
        const MovementVector point{direction.x * 48.0f + origin.x, direction.y * 48.0f + origin.y,
                                   direction.z * 48.0f + origin.z};
        const MovementVector end{direction.x * 30.0f + origin.x, direction.y * 30.0f + origin.y,
                                 direction.z * 30.0f + origin.z};
        trace_movement_world(engine, make_movement_hull_ray(origin, end, {-1, -1, -8}, {1, 1, 8}),
                             1, trace, trace_flag);
        if (!engine.point_contents_world(point, 1))
            continue;
        total.x += direction.x;
        total.y += direction.y;
        total.z += direction.z;
        ++sample_count;
        if (!blocked(trace) || trace.plane_normal.z >= 0.1f)
            continue;
        total.x += direction.x;
        total.y += direction.y;
        total.z += direction.z;
        ++sample_count;
        const MovementVector normal{negate(trace.plane_normal.x), negate(trace.plane_normal.y),
                                    negate(trace.plane_normal.z)};
        if ((direction.y * normal.y + direction.x * normal.x) + direction.z * normal.z >= 0.5f) {
            normals.x += normal.x;
            normals.y += normal.y;
            normals.z += normal.z;
            ++normal_count;
        }
    }
    if (sample_count <= 1 || !normal_count)
        return false;
    const auto sample_inverse = 1.0f / static_cast<float>(sample_count);
    total.x *= sample_inverse;
    total.y *= sample_inverse;
    total.z *= sample_inverse;
    const auto normal_inverse = 1.0f / static_cast<float>(normal_count);
    normals.x *= normal_inverse;
    normals.y *= normal_inverse;
    const auto inverse = 1.0f / (std::sqrt((normals.x * normals.x + normals.y * normals.y) + 0.0f) +
                                 std::numeric_limits<float>::epsilon());
    normals.x *= inverse;
    normals.y *= inverse;
    auto yaw = planar_movement_yaw(total);
    const auto second_view = engine.view_angles();
    auto difference = second_view.yaw - yaw;
    if (!std::isnan(difference)) {
        if (absolute(difference) > 180.0f) {
            const auto turns = round_movement_step(absolute(difference / 360.0f)) * 360.0f;
            if (difference < 0.0f)
                difference += turns;
            else
                difference -= turns;
        }
        if (difference < 0.0f)
            difference = negate(difference);
        if (difference > 120.0f)
            return false;
    }
    const MovementVector point{total.x * 30.0f + origin.x, total.y * 30.0f + origin.y,
                               total.z * 30.0f + origin.z};
    if (engine.point_contents_world(point, 1) & 1u) {
        MovementVector above = point;
        above.z += 8.0f;
        const MovementVector delta{point.x - above.x, point.y - above.y, point.z - above.z};
        const auto swept = (delta.y * delta.y + delta.x * delta.x) + delta.z * delta.z != 0.0f;
        // The line-ray constructor stores positive zero offsets, unlike the
        // negated hull-center zeros used by the two hull trace constructors.
        const MovementCollisionRay line{above, delta, {0, 0, 0}, {0, 0, 0}, true, swept};
        trace_movement_world(engine, line, 1, trace, trace_flag);
        if (trace.fraction < 1.0f && !trace.start_solid && trace.plane_normal.z > 0.7f) {
            command_yaw = yaw;
            return true;
        }
    }
    const auto correction = clamp(angle_difference(first_view.yaw, yaw) * 1.5f,
                                  negate(settings.maximum_angle), settings.maximum_angle);
    if (absolute(correction) > settings.hull_padding)
        yaw -= correction;
    const auto normal_yaw = planar_movement_yaw(normals);
    difference = angle_difference(yaw, normal_yaw);
    if (difference > settings.maximum_angle)
        yaw = normal_yaw + settings.maximum_angle;
    else if (difference < negate(settings.maximum_angle))
        yaw = normal_yaw - settings.maximum_angle;
    command_yaw = yaw;
    return true;
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: eligibility.cpp
// ============================================================================

namespace skeet::anti_aim {
bool hud_chat_active(const EligibilityBindings& bindings) {
    if (!bindings.find_hud)
        return false;
    auto* chat = bindings.find_hud(bindings.hud_context, "CCSGO_HudChat");
    return chat && engine::invoke<bool>(chat, 13);
}
bool command_eligible(const EligibilitySettings& settings, const EligibilityFrame& frame,
                      const EligibilityGuard& guard, std::int32_t command_handle,
                      const EligibilityBindings& bindings, std::uint32_t& flags) {
    flags = (flags & ~4u) | ((frame.flags >> 4) & 4u);
    if (!settings.enabled || (frame.secondary_flags & 4u) ||
        (settings.pitch == 0 && settings.yaw == 0 && settings.body_yaw == 0))
        return false;
    if (guard.shared_player_available && guard.guard &&
        guard.current_time - guard.reference_time > 0.5f)
        return false;
    if (command_handle != -1 && engine::invoke<bool>(bindings.input_system, 15, command_handle) &&
        !engine::invoke<bool>(bindings.engine_client, 11) && !hud_chat_active(bindings))
        return false;
    return true;
}
bool command_uses_cached_angles(bool player_state, std::int32_t movement_type,
                                std::uint32_t frame_flags, std::uint32_t buttons,
                                ProximityEngine& proximity, ProximityEntity local) {
    if (player_state)
        return true;
    return (frame_flags & 0x200000u) && (buttons & 0x20u) && movement_type == 3 &&
           any_nearby_entity(proximity, local);
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: finisher.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
/* deduplicated helper */
bool model_ready(const animation::BoneModel& model) noexcept {
    return model.renderable_model && model.model_index != -1 && model.studio_header &&
           (!model.virtual_model_required || model.virtual_model);
}
bool build_finisher_bones(FinisherState& state, const animation::BoneRequest& request,
                          const FinisherBindings& bindings) {
    state.common.value_1b0 = 0;
    if (model_ready(bindings.model))
        state.common.value_1b0 = static_cast<std::uint32_t>(bindings.model.parents.size());
    return animation::build_bone_matrices(bindings.bone_state, bindings.model, request,
                                          state.matrices_520, bindings.bones,
                                          bindings.trigonometry);
}
bool build_finisher_bones(FinisherState& state, const animation::BoneRequest& request,
                          const LiveFinisherBindings& bindings) {
    // Corresponding signed/unsigned integer types may alias. The canonical
    // count is published before pose accumulation, as the native owner does.
    return bindings.bones.build_bone_matrices(
        bindings.player, request, state.matrices_520,
        reinterpret_cast<std::int32_t*>(&state.common.value_1b0));
}
}
template <class Bindings>
void finish_angles(bool enabled, std::int32_t body_yaw, FinisherState& state,
                   const FinisherPlayer& player, std::int32_t frame_command,
                   const client::Command& command, std::uint8_t send_packet,
                   const Bindings& bindings) {
    if (!enabled || body_yaw == 0 || !send_packet || !bindings.animation_state)
        return;
    auto* const original_state = bindings.animation_state;
    if ((player.flags_38ca5228 & 8u) || player.field_043fa575 != player.field_334b460c ||
        read<float>(original_state, 0x6C) == player.field_86a5f8b3_minus4 ||
        read<std::uint32_t>(state.animation_1d8.data(), 0x60) != player.identity) {
        std::memcpy(state.animation_1d8.data(), original_state, 0x348);
        std::memset(state.animation_1d8.data() + 0x13C, 0, 0x14);
        state.common.value_1b4 = -1;
    }
    bindings.animation_state = state.animation_1d8.data();
    const auto scalar = bindings.scalar_a2a8247f;
    bindings.scalar_a2a8247f = bindings.simulation_time;
    lagcomp::AnimationState snapshot;
    animation::capture_entity_animation(snapshot, bindings.player, bindings.entity_offsets);
    const auto absolute = *engine::invoke<const RageVector*>(bindings.player, 11);
    auto angles = command.angles;
    animation::select_animation_angles(bindings.angle_history, frame_command, angles);
    animation::update_animation_state(state.animation_1d8.data(), angles, bindings.timing,
                                      bindings.update);
    if (*bindings.build_bones_12c) {
        state.absolute_angles_1b8 = *engine::invoke<const RageVector*>(bindings.player, 11);
        const auto saved_eye = bindings.eye_angles;
        bindings.eye_angles = bindings.final_angles;
        // Native43388720 initializes the optional count before its model gate,
        // then publishes the studio count before engine pose accumulation.
        const animation::BoneRequest request{
            state.absolute_angles_1b8, {0.0f, 0.0f, 0.0f}, 0.0f, 0xFFF00u, -1, 0, true};
        if (build_finisher_bones(state, request, bindings))
            state.common.value_1b4 = frame_command;
        bindings.eye_angles = saved_eye;
    }
    animation::restore_entity_animation(snapshot, bindings.player, bindings.entity_offsets);
    if (bindings.set_absolute_angles)
        bindings.set_absolute_angles(bindings.player, absolute);
    bindings.scalar_a2a8247f = scalar;
    bindings.animation_state = original_state;
    state.common.reference_yaw_258 = read<float>(state.animation_1d8.data(), 0x80);
}
void finish_command_angles(bool enabled, std::int32_t body_yaw, FinisherState& state,
                           const FinisherPlayer& player, std::int32_t frame_command,
                           const client::Command& command, std::uint8_t send_packet,
                           const FinisherBindings& bindings) {
    finish_angles(enabled, body_yaw, state, player, frame_command, command, send_packet, bindings);
}
void finish_command_angles(bool enabled, std::int32_t body_yaw, FinisherState& state,
                           const FinisherPlayer& player, std::int32_t frame_command,
                           const client::Command& command, std::uint8_t send_packet,
                           const LiveFinisherBindings& bindings) {
    finish_angles(enabled, body_yaw, state, player, frame_command, command, send_packet, bindings);
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: freestanding.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
BallisticVector advance(const BallisticVector& a, const BallisticVector& b, float distance) {
    return {b[0] * distance + a[0], b[1] * distance + a[1], b[2] * distance + a[2]};
}
std::int32_t add(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) + std::uint32_t(b));
}
std::int32_t difference(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) - std::uint32_t(b));
}
// Local filter vtable 4346EEBC: accepts=false, type=0; source/last-hit
// setters forward to the registered filter implementation at 43475B90.
class FreestandingFilterEngine final : public BallisticEngine {
    BallisticEngine& engine_;

public:
    explicit FreestandingFilterEngine(BallisticEngine& engine) : engine_(engine) {}
    std::int32_t filter_type(const BallisticFilter&) override {
        return 0;
    }
    bool accepts(const BallisticFilter&, BallisticEntity, std::uint32_t) override {
        return false;
    }
    void bind_source(const BallisticFilter& f, BallisticEntity e) override {
        engine_.bind_source(f, e);
    }
    void bind_last_hit(const BallisticFilter& f, BallisticEntity e) override {
        engine_.bind_last_hit(f, e);
    }
    std::uint32_t point_contents(const BallisticVector& p, std::uint32_t mask,
                                 bool world) override {
        return engine_.point_contents(p, mask, world);
    }
    void trace_ray(const BallisticTraceRequest& r, BallisticTrace& t) override {
        engine_.trace_ray(r, t);
    }
    void clip_ray_to_entity(const BallisticRay& r, std::uint32_t m, BallisticEntity e,
                            BallisticTrace& t) override {
        engine_.clip_ray_to_entity(r, m, e, t);
    }
    PenetrationSurface surface(std::int16_t p) override {
        return engine_.surface(p);
    }
    BallisticEntity entity(std::int32_t i) override {
        return engine_.entity(i);
    }
    std::int32_t maximum_clients() override {
        return engine_.maximum_clients();
    }
    BallisticEntityInfo entity_info(BallisticEntity e) override {
        return engine_.entity_info(e);
    }
    BallisticEntityInfo clip_entity_info(BallisticEntity e) override {
        return engine_.clip_entity_info(e);
    }
    BallisticBreakableInfo breakable_info(BallisticEntity e) override {
        return engine_.breakable_info(e);
    }
    BallisticModelInfo model_info(std::int32_t i) override {
        return engine_.model_info(i);
    }
    std::uint8_t damage_mode(BallisticEntity e) override {
        return engine_.damage_mode(e);
    }
    void set_damage_mode(BallisticEntity e, std::uint8_t m) override {
        engine_.set_damage_mode(e, m);
    }
    bool has_engine_breakable_callback() override {
        return engine_.has_engine_breakable_callback();
    }
    bool engine_breakable(BallisticEntity e) override {
        return engine_.engine_breakable(e);
    }
    BallisticRules rules() override {
        return engine_.rules();
    }
};
}
void build_freestanding_rays(std::array<FreestandingRay, 12>& rays,
                             const std::array<FreestandingScore, 3>& scores,
                             const BallisticVector& start, const BallisticVector& target,
                             const BallisticVector& side) {
    const auto left12 = advance(start, scores[0].direction, 12.0f);
    const auto right12 = advance(start, scores[1].direction, 12.0f);
    const auto left22 = advance(start, scores[0].direction, 22.0f);
    const auto right22 = advance(start, scores[1].direction, 22.0f);
    BallisticVector offset{side[0] * 50.0f, side[1] * 50.0f, side[2] * 50.0f};
    const BallisticVector positive{offset[0] + target[0], offset[1] + target[1],
                                   offset[2] + target[2]};
    const BallisticVector negative{target[0] - offset[0], target[1] - offset[1],
                                   target[2] - offset[2]};
    rays[0] = {true, left12, target};
    rays[1] = {true, right12, target};
    rays[2] = {true, start, target};
    rays[3] = {true, left22, target};
    rays[4] = {true, right22, target};
    rays[5].enabled = false;
    rays[6] = {true, left22, positive};
    rays[7] = {true, right22, positive};
    rays[8] = {true, start, positive};
    rays[9] = {true, left22, negative};
    rays[10] = {true, right22, negative};
    rays[11] = {true, start, negative};
}
bool freestanding_difference(const std::array<FreestandingScore, 3>& scores) noexcept {
    if (scores[1].damage == scores[0].damage &&
        scores[1].maximum_distance == scores[0].maximum_distance)
        return false;
    auto minimum_damage = std::numeric_limits<std::int32_t>::max(),
         maximum_damage = std::numeric_limits<std::int32_t>::min();
    auto minimum_distance = minimum_damage, maximum_distance = maximum_damage;
    for (const auto& score : scores) {
        minimum_damage = std::min(minimum_damage, score.damage);
        maximum_damage = std::max(maximum_damage, score.damage);
        minimum_distance = std::min(minimum_distance, score.maximum_distance);
        maximum_distance = std::max(maximum_distance, score.maximum_distance);
    }
    return (minimum_damage == 0 && maximum_damage != 0) ||
           (minimum_distance < 50 && (difference(maximum_damage, minimum_damage) > 5 ||
                                      difference(maximum_distance, minimum_distance) > 25));
}
std::int32_t select_freestanding_side(const std::array<FreestandingScore, 3>& scores) noexcept {
    if (!scores[2].damage) {
        if (!scores[0].damage && scores[1].damage)
            return 0;
        if (scores[0].damage && !scores[1].damage)
            return 1;
    }
    std::int32_t selected = 0;
    for (std::int32_t i = 1; i < 3; ++i)
        if (scores[i].damage < scores[selected].damage ||
            (scores[i].damage == scores[selected].damage &&
             scores[i].total_distance < scores[selected].total_distance))
            selected = i;
    return selected;
}
std::int32_t score_freestanding_ray(BallisticEngine& engine, BallisticQuery& query,
                                    FreestandingScore& score, const BallisticVector& start,
                                    const BallisticVector& target) {
    BallisticFilter filter{nullptr,     0,    0x0600400Bu,
                           0x0600400Bu, 1.0f, BallisticFilterKind::reject_entities};
    FreestandingFilterEngine filtered_engine(engine);
    query.trace.fraction = 0.0f;
    trace_ballistic_bullet(filtered_engine, 0, start, target, filter, query);
    const auto damage = _mm_cvttss_si32(_mm_set_ss(query.damage));
    if (damage > 0)
        score.damage = add(score.damage, damage);
    const auto travelled = length(subtract(query.trace.end, start));
    const auto requested = length(subtract(target, start));
    auto distance = (travelled / requested) * 100.0f / 25.0f;
    distance = round_movement_step(distance) * 25.0f + 0.5f;
    const auto quantized = _mm_cvttss_si32(_mm_set_ss(distance));
    score.total_distance = add(score.total_distance, quantized);
    score.maximum_distance = std::max(score.maximum_distance, quantized);
    return damage;
}
bool find_freestanding_direction(BallisticEngine& engine, const BallisticVector& start,
                                 const BallisticVector& target, BallisticVector& direction,
                                 std::int32_t* total_damage, FreestandingWorkspace& work) {
    const auto dx = target[0] - start[0], dy = target[1] - start[1];
    const auto reciprocal =
        1.0f / (std::sqrt((dx * dx + dy * dy) + 0.0f) + std::numeric_limits<float>::epsilon());
    const BallisticVector forward{reciprocal * dx, reciprocal * dy, reciprocal * 0.0f};
    BallisticVector side;
    if (forward[0] == 0.0f && forward[1] == 0.0f)
        side = {0, -1, 0};
    else {
        const auto common = forward[2] * 0.0f;
        side = {forward[1] - common, common - forward[0], forward[0] * 0.0f - forward[1] * 0.0f};
        const auto inverse = 1.0f / (length(side) + std::numeric_limits<float>::epsilon());
        for (auto& value : side)
            value *= inverse;
    }
    auto& query = work.query;
    query.allow_penetration = 1;
    query.weapon_id = 0;
    query.penetration = 2.5f;
    query.range = 8192.0f;
    query.range_modifier = std::bit_cast<float>(0x3F7AE148u);
    query.armor_ratio = std::bit_cast<float>(0x3FD33333u);
    query.base_damage = 80.0f;
    query.reserved = 1;
    work.scores = {};
    work.scores[0].direction = side;
    work.scores[1].direction = {neg(side[0]), neg(side[1]), neg(side[2])};
    work.scores[2].direction = {neg(forward[0]), neg(forward[1]), neg(forward[2])};
    build_freestanding_rays(work.rays, work.scores, start, target, side);
    for (std::size_t i = 0; i < work.rays.size(); ++i) {
        auto ray = work.rays[i];
        auto& score = work.scores[i % 3];
        if (engine.point_contents(ray.start, 1, true) & 1u) {
            if (i < 6 || i % 3 == 2 || score.total_distance == 0)
                continue;
            ray.start = advance(start, score.direction, 10.0f);
        }
        score_freestanding_ray(engine, query, score, ray.start, ray.end);
    }
    if (total_damage) {
        *total_damage = 0;
        for (const auto& score : work.scores)
            *total_damage = add(*total_damage, score.damage);
    }
    if (!freestanding_difference(work.scores))
        return false;
    const auto selected = select_freestanding_side(work.scores);
    if (selected == -1)
        return false;
    direction = work.scores[selected].direction;
    return true;
}
bool apply_freestanding(const FreestandingSettings& settings, const FreestandingFrame& frame,
                        const FreestandingContext& context, FreestandingState& state, float& yaw,
                        BallisticEngine& engine, FreestandingWorkspace& workspace) {
    if (!settings.enabled || !hotkey_active(settings.hotkey, context.input))
        return false;
    if (!(state.flags_130 & 8u)) {
        if (context.target_handle == -1 ||
            !context.entities.by_handle(context.entities.object, context.target_handle))
            return false;
        auto start = frame.origin_10c;
        const auto absolute38 =
            std::bit_cast<float>(std::bit_cast<std::uint32_t>(frame.scalar_38) & 0x7FFFFFFFu);
        if (!(frame.scalar_c0 > 2.0f) && absolute38 == 0.0f && frame.forward_move > 0.0f &&
            frame.forward_move < 10.0f)
            start = frame.origin_168;
        bool standing = (context.flags & 0x40u) != 0;
        if (!standing)
            standing = hotkey_active(context.height_hotkey, context.input) ||
                       (context.flags & 0x100u) != 0;
        start[2] += standing ? 68.0f : frame.height_164 + 4.0f;
        auto target = context.target;
        target[2] += 64.0f;
        BallisticVector selected{};
        std::int32_t total_damage = 0;
        if (find_freestanding_direction(engine, start, target, selected, &total_damage, workspace))
            state.yaw_1a8 = planar_movement_yaw({selected[0], selected[1], selected[2]});
        else if (total_damage > 0) {
            state.value_1ac = 0;
            state.yaw_1a8 = std::numeric_limits<float>::max();
        }
    }
    if (state.yaw_1a8 == std::numeric_limits<float>::max())
        return false;
    yaw = state.yaw_1a8;
    return true;
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: jitter.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
float scalar(std::int32_t value) noexcept {
    return _mm_cvtss_f32(_mm_cvtsi32_ss(_mm_setzero_ps(), value));
}
}
void apply_yaw_jitter(float& yaw, std::int32_t yaw_mode, const JitterSettings& settings,
                      std::uint32_t phase, std::int32_t command_number,
                      const JitterRandom& random) {
    if (yaw_mode == 0)
        return;
    float delta = 0.0f;
    switch (settings.mode) {
    case 1:
        if (phase & 1u)
            delta = scalar(settings.offset);
        break;
    case 2:
        delta = scalar(settings.offset) * 0.5f;
        if (!(phase & 1u))
            delta = negate(delta);
        break;
    case 3: {
        const auto half = absolute(scalar(settings.offset)) * 0.5f;
        random.seed(random.context, command_number);
        delta = random.range(random.context, negate(half), half);
        break;
    }
    case 4: {
        auto amplitude = _mm_set_ss(absolute(scalar(settings.offset)));
        amplitude = _mm_max_ss(amplitude, _mm_set_ss(settings.minimum_skitter));
        amplitude = _mm_min_ss(amplitude, _mm_set_ss(settings.maximum_skitter));
        delta = settings.skitter[phase % 12u] * _mm_cvtss_f32(amplitude);
        break;
    }
    default:
        break;
    }
    yaw += delta;
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: menu_dependencies.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
std::int32_t current(const ui::ComboBox& source) noexcept {
    return source.value ? std::bit_cast<std::int32_t>(*source.value) : 0;
}
}
void pitch_changed(MenuDependencies& widgets, const ui::ComboBox& source) {
    const auto pitch = current(source);
    if (pitch > 6)
        return;
    widgets.custom_pitch.set_visible(pitch == 6);
}
void yaw_changed(MenuDependencies& widgets, const ui::ComboBox& source) {
    const auto yaw = current(source);
    if (yaw >= 6)
        return;
    if (yaw) {
        widgets.yaw_offset.value = &widgets.yaw_offsets[yaw];
        widgets.yaw_offset.value_changed();
    }
    widgets.yaw_offset.set_visible(yaw != 0);
    widgets.yaw_jitter.set_visible(yaw != 0);
    widgets.yaw_jitter_offset.set_visible(yaw != 0 && current(widgets.yaw_jitter) != 0);
}
void jitter_changed(MenuDependencies& widgets, const ui::ComboBox& source,
                    const ui::ComboBox& yaw) {
    const auto jitter = current(source);
    if (source.value && jitter >= 5)
        return;
    widgets.yaw_jitter_offset.set_visible(jitter != 0 && current(yaw) != 0);
}
void body_yaw_changed(MenuDependencies& widgets, const ui::ComboBox& source) {
    const auto body_yaw = current(source);
    if (body_yaw >= 4)
        return;
    widgets.freestanding_body_yaw.set_visible(body_yaw != 0);
    if (body_yaw != 0 && body_yaw != 1) {
        widgets.body_yaw_offset.value = &widgets.body_yaw_offsets[body_yaw];
        widgets.body_yaw_offset.value_changed();
    }
    widgets.body_yaw_offset.set_visible(body_yaw != 0 && body_yaw != 1);
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: movement_capture.cpp
// ============================================================================

namespace skeet::anti_aim {
MovementVector advance_live_movement_origin(void* player, MovementVector position,
                                            MovementVector velocity, std::uint32_t player_flags,
                                            const MovementCaptureInput& input,
                                            const MovementEnvironmentBindings& environment,
                                            const client::Command& command,
                                            std::uint32_t input_buttons, float yaw,
                                            MovementCollisionEngine& collision) {
    const auto movement_squared = command.movement.forward_move * command.movement.forward_move +
                                  command.movement.side_move * command.movement.side_move;
    const auto velocity_squared =
        (velocity.x * velocity.x + velocity.y * velocity.y) + velocity.z * velocity.z;
    if (!((input_buttons & 0x618u) && command.tick != 0x7FFFFFFF && movement_squared > 10.0f) &&
        !(velocity_squared > 4.0f))
        return position;
    auto copy = command;
    copy.angles[1] = yaw;
    MovementSimulation simulation{};
    initialize_live_movement_simulation(simulation, player, position, velocity, player_flags, 0,
                                        input);
    for (int tick = 0; tick != 2; ++tick)
        simulate_live_movement_tick(simulation, collision, environment, player, copy);
    return simulation.position;
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: owner_lifecycle.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define AA_MEMBER __thiscall
#define AA_FAST __fastcall
#else
#define AA_MEMBER
#define AA_FAST
#endif
using PrimaryEntry = void(AA_MEMBER*)(void*);
struct Instance;
struct HudEndpoint : hud::HudFrameCallback {
    Instance* owner;
};
struct Text {
    char* begin = nullptr;
    char* end = nullptr;
    char* limit = nullptr;
};
void AA_MEMBER destroy_entry(void*);
void AA_MEMBER bind_entry(void*);
void AA_MEMBER unchanged(void*) {} //433CCE8E
void AA_MEMBER reset_entry(void*);
void AA_MEMBER refresh_entry(void*);
void AA_FAST hud_entry(void*, ui::RenderQueue*, void*, std::int32_t*);
void AA_FAST frame_entry(void*, std::uint8_t) {} //43471F08 ->433CCE8E
const std::array<PrimaryEntry, 9> primary_table{destroy_entry, bind_entry,    unchanged,
                                                unchanged,     unchanged,     reset_entry,
                                                unchanged,     refresh_entry, unchanged};
const std::array<hud::HudFrameEntry, 1> hud_table{hud_entry};
const std::array<engine::FrameUpdateEntry, 1> frame_table{frame_entry};
void* allocate(engine::JobMemory memory, std::size_t size) {
    auto* const raw = memory.allocate(memory.process_heap, 8, size + 0x20);
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
void free(engine::JobMemory memory, void* object) {
    if (!object)
        return;
    auto* const bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void construct_text(Text& text, engine::JobMemory memory) {
    //433B2213 ->433C1360, including its trailing zero and12-byte capacity.
    text.begin = nullptr;
    text.end = nullptr;
    text.limit = nullptr;
    auto* const bytes = static_cast<char*>(allocate(memory, 12));
    text.begin = bytes;
    text.end = bytes;
    text.limit = bytes + 12;
    std::memmove(bytes, "+spray_menu", 11);
    text.end = bytes + 11;
    bytes[11] = 0;
}
void destroy_text(Text& text, engine::JobMemory memory) {
    const auto capacity =
        std::bit_cast<std::int32_t>(std::uint32_t(reinterpret_cast<std::uintptr_t>(text.limit)) -
                                    std::uint32_t(reinterpret_cast<std::uintptr_t>(text.begin)));
    if (capacity >= 2)
        free(memory, text.begin);
}
struct Instance {
    const PrimaryEntry* vtable = nullptr;
    engine::JobMemory memory;
    ConfigRegistries& config;
    OwnerBindings bindings;
    client::NativeCommandPhaseReceiver command_receiver;
    HudEndpoint hud{{nullptr}, this};
    engine::FrameUpdateEndpoint frame;
    Text command_text;

    Instance(const OwnerRegistries& registries, ConfigRegistries& configurations,
             OwnerBindings context)
        : memory(registries.primary.memory), config(configurations), bindings(context) {
        registries.primary.append(this); //433676A0, complete+0
        registries.commands.append(command_receiver, &bindings.command); //43490448,+8
        hud::register_hud_frame(registries.hud, &hud); //433D20B5,+10
        engine::register_frame_update(registries.frames, &frame); //433FCB12,+18
        registries.commands.publish(command_receiver, client::CommandOwnerId::anti_aim); //4349045F
        hud.vtable = hud_table.data(); //43472654
        vtable = primary_table.data(); //4346F834
        frame.vtable = frame_table.data(); //43471F08
        auto& command = bindings.command;
        initialize_owner_config(config, command.config);
        initialize_command_state_prefix(command.state, command.precalculation, command.parameters);
        construct_text(command_text, memory);
        initialize_command_animation_state(command.state, command.finisher);
        finish_owner_config(command.config);
        initialize_command_pattern(command.parameters);
    }
};
void AA_MEMBER destroy_entry(void* storage) {
    auto* const owner = static_cast<Instance*>(storage);
    destroy_owner(storage, owner->bindings.active);
}
void AA_MEMBER bind_entry(void* storage) {
    bind_owner_settings(storage);
}
void AA_MEMBER reset_entry(void* storage) {
    reset_state(static_cast<Instance*>(storage)->bindings.command.state);
}
void AA_MEMBER refresh_entry(void* storage) {
    refresh_owner_command(storage);
}
void AA_FAST hud_entry(void* endpoint, ui::RenderQueue* queue, void* player, std::int32_t* cursor) {
    auto& owner = *static_cast<HudEndpoint*>(static_cast<hud::HudFrameCallback*>(endpoint))->owner;
    draw_owner_hud(owner.bindings.command, owner.bindings.hud, *queue, player, *cursor);
}
}
std::size_t owner_size() noexcept {
    return sizeof(Instance);
}
void construct_owner(void* storage, const OwnerRegistries& registries, ConfigRegistries& config,
                     OwnerBindings bindings) {
    ::new (storage) Instance(registries, config, bindings);
}
void* create_owner(const OwnerRegistries& registries, ConfigRegistries& config,
                   OwnerBindings bindings) {
    void* const storage = allocate(registries.primary.memory, owner_size());
    auto& command = bindings.command;
    command.config = Config{};
    command.state = AngleStageState{};
    command.state.cached_yaw_18c = 0.0f;
    command.parameters = {0, 0, 0, 0, 0, 0, 0, 0, 0, {}};
    command.precalculation.feet_yaw_194 = 0.0f;
    command.precalculation.micro_move_19c = 0.0f;
    command.precalculation.previous_duck_1a4 = 0.0f;
    std::fill(command.finisher.animation_1d8.begin(), command.finisher.animation_1d8.end(),
              std::byte{0});
    command.finisher.absolute_angles_1b8 = {};
    std::fill(command.finisher.matrices_520.begin(), command.finisher.matrices_520.end(),
              Matrix3x4{});
    construct_owner(storage, registries, config, bindings);
    return storage;
}
void destroy_owner(void* storage, CommandOwner*& active) {
    active = nullptr;
    if (!storage)
        return;
    auto* const owner = static_cast<Instance*>(storage);
    const auto memory = owner->memory;
    animation::release_animation_state_vector(owner->bindings.command.finisher.animation_1d8.data(),
                                              owner->bindings.allocator);
    destroy_text(owner->command_text, memory);
    owner->~Instance();
    free(memory, storage);
}
void bind_owner_settings(void* storage) {
    auto& owner = *static_cast<Instance*>(storage);
    owner.bindings.command.state.build_bones_12c =
        &static_cast<BooleanConfig*>(find_config(owner.config.descriptors, 0x96DB2E1Cu))->value;
}
void refresh_owner_command(void* storage) {
    auto& owner = *static_cast<Instance*>(storage);
    owner.vtable[5](storage);
    const auto command =
        engine::invoke<std::int32_t>(owner.bindings.command.engine_client, 170,
                                     static_cast<const char*>(owner.command_text.begin),
                                     std::int32_t(-1), std::int32_t(0), std::int32_t(-1));
    owner.bindings.command.state.value_1c4 = command == 0 ? -1 : command;
}
void draw_owner_hud(CommandOwner& command, const OwnerHudBindings& bindings, ui::RenderQueue& queue,
                    void* player, std::int32_t& cursor) {
    if (!command.global.enabled.value)
        return;
    esp::RuntimePlayer local;
    local.engine_entity = player;
    std::memcpy(&local.life_state,
                static_cast<const std::byte*>(player) + bindings.life_state_9d920e35,
                sizeof local.life_state);
    hud::draw_freestanding_indicator(queue, bindings.scripts, command.global, command.config,
                                     command.state, bindings.indicators, local,
                                     command.globals.input, cursor, bindings.layout);
}
#undef AA_MEMBER
#undef AA_FAST
} //namespace skeet::anti_aim

// ============================================================================
// Subcomponent: pitch.cpp
// ============================================================================

namespace skeet::anti_aim {
void apply_pitch(float& pitch, const PitchSettings& settings, const PitchFrame& frame,
                 const PitchInterfaces& external) {
    auto mode = settings.mode;
    if (!mode)
        return;
    if ((frame.flags & 0x200000u) && (frame.buttons & 0x20u) && frame.movement_type == 3)
        mode = 1;
    if (mode == 4) {
        external.random_seed(external.context, frame.command_number);
        mode = external.random_integer(external.context, 1, 5);
    }
    switch (mode) {
    case 1:
        pitch = settings.maximum_pitch;
        break;
    case 2:
        if ((frame.flags & 8u) && external.player_code(external.context) == 100)
            pitch = negate(settings.special_angle) - frame.angle_9c;
        else
            pitch = settings.maximum_pitch;
        break;
    case 3:
        if (frame.has_weapon) {
            auto value = _mm_set_ss(frame.weapon_pitch_340);
            value = _mm_max_ss(value, _mm_set_ss(negate(settings.maximum_pitch)));
            value = _mm_min_ss(value, _mm_set_ss(settings.maximum_pitch));
            pitch = _mm_cvtss_f32(value);
            if ((std::bit_cast<std::uint32_t>(pitch) & 0x7F800000u) == 0x7F800000u)
                pitch = settings.maximum_pitch;
        }
        break;
    case 5:
        pitch = (frame.flags & 8u) ? settings.special_angle - frame.angle_9c
                                   : negate(settings.maximum_pitch);
        break;
    case 6:
        pitch = _mm_cvtss_f32(_mm_cvtsi32_ss(_mm_setzero_ps(), settings.custom_pitch));
        break;
    default:
        break;
    }
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: precalculation.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
template <class T> T live_field(const void* object, std::uint32_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
}
void apply_micro_movement(PrecalculationState& state, const PrecalculationPlayer& player,
                          float interval, client::Command& command) noexcept {
    const auto original = state.micro_move_19c;
    auto amount = original;
    auto duck = player.duck_3309c0cf;
    bool scale;
    if (command.movement.buttons & 4u) {
        const auto advanced = (interval * 0.8f) * player.rate_fb7e3404 + duck;
        duck = 1.0f < advanced ? 1.0f : advanced; // native MINSS selects second on unordered
        scale = true;
    } else {
        if (duck != 1.0f || duck != state.previous_duck_1a4) {
            const auto advanced = duck - maximum_number(1.5f, player.rate_fb7e3404) * interval;
            duck = 0.0f > advanced ? 0.0f : advanced; // native MAXSS, including signed zero
        }
        scale = player.field_6b1a59b5 || (player.flags_e2fbe695 & 2u);
    }
    if (scale)
        amount /= ((duck * 0.34f + 1.0f) - duck);
    state.micro_move_19c =
        std::bit_cast<float>(std::bit_cast<std::uint32_t>(original) ^ 0x80000000u);
    const auto forward = command.movement.forward_move;
    if (forward != 0.0f) {
        if (forward < 0.0f)
            amount = forward - amount;
        else
            amount += forward;
    }
    command.movement.forward_move = amount;
}
template <class Bindings>
void precalculate_angles(const GlobalConfig& global, const Config& config,
                         PrecalculationState& state, const PrecalculationPlayer& player,
                         std::int32_t choked_commands, client::Command& command,
                         const std::array<float, 3>& incoming_angles, std::uint8_t& send_packet,
                         const Bindings& bindings) {
    constexpr bool live = std::is_same_v<Bindings, LivePrecalculationBindings>;
    auto& common = state.common;
    common.flags_130 &= ~2u;
    if (!global.enabled.value)
        return;
    common.flags_130 = (common.flags_130 & ~1u) | (player.flags_e2fbe695 & 1u);
    if (choked_commands == 0 && player.has_animation_state)
        state.feet_yaw_194 = player.feet_yaw_80;
    if ((global.body_yaw.value != 0 || config.roll.value != 0) && !player.guard_d720aea6 &&
        !(common.flags_130 & 4u) && !(command.movement.buttons & 0x63Au) &&
        player.movement_type_7501cb91 == 2) {
        const auto origin = *engine::invoke<const MovementVector*>(bindings.player, 10);
        if constexpr (live) {
            initialize_live_movement_simulation(bindings.workspace, bindings.player, origin,
                                                player.velocity_36375c23, player.flags_e2fbe695, 0,
                                                bindings.movement);
        } else
            initialize_movement_simulation(bindings.workspace, bindings.player_value, origin,
                                           player.velocity_36375c23, player.flags_e2fbe695, 0,
                                           bindings.movement_player, bindings.movement_rules);
        const auto saved = command.angles;
        command.angles = incoming_angles;
        if constexpr (live)
            simulate_live_movement_tick(bindings.workspace, bindings.collision,
                                        bindings.environment, bindings.player, command);
        else
            simulate_movement_tick(bindings.workspace, bindings.collision,
                                   bindings.movement_environment, bindings.player_value,
                                   command.movement,
                                   {command.angles[0], command.angles[1], command.angles[2]});
        command.angles = saved;
        const auto& velocity = bindings.workspace.velocity;
        const auto speed = std::sqrt((velocity.x * velocity.x + velocity.z * velocity.z) +
                                     velocity.y * velocity.y);
        if (speed < 2.0f) {
            auto choked = choked_commands;
            if constexpr (live)
                choked = **bindings.fields.choked;
            if (choked == 0)
                send_packet = 0;
            if (global.body_yaw.value != 0) {
                auto current = player;
                float interval;
                if constexpr (live) {
                    const auto& offsets = bindings.fields;
                    current.duck_3309c0cf =
                        live_field<float>(bindings.player, offsets.duck_3309c0cf);
                    current.rate_fb7e3404 =
                        live_field<float>(bindings.player, offsets.rate_fb7e3404);
                    current.field_6b1a59b5 =
                        live_field<std::uint8_t>(bindings.player, offsets.field_6b1a59b5) != 0;
                    current.flags_e2fbe695 =
                        live_field<std::uint32_t>(bindings.player, offsets.flags_e2fbe695);
                    interval = **offsets.interval;
                } else
                    interval = bindings.movement_rules.interval_per_tick;
                apply_micro_movement(state, current, interval, command);
            }
            if (config.roll.value != 0 && !client::command_roll_blocked(bindings.roll_guard) &&
                !(command.movement.buttons & 0x801u))
                command.angles[2] = static_cast<float>(config.roll.value);
        }
    }
    if (common.value_1a8 != std::numeric_limits<float>::max())
        common.value_1ac |= command.movement.buttons;
    if constexpr (live)
        state.previous_duck_1a4 = live_field<float>(bindings.player, bindings.fields.duck_3309c0cf);
    else
        state.previous_duck_1a4 = player.duck_3309c0cf;
}
void precalculate_command_angles(const GlobalConfig& global, const Config& config,
                                 PrecalculationState& state, const PrecalculationPlayer& player,
                                 std::int32_t choked, client::Command& command,
                                 const std::array<float, 3>& incoming, std::uint8_t& send,
                                 const PrecalculationBindings& bindings) {
    precalculate_angles(global, config, state, player, choked, command, incoming, send, bindings);
}
void precalculate_live_command_angles(const GlobalConfig& global, const Config& config,
                                      PrecalculationState& state,
                                      const PrecalculationPlayer& player, std::int32_t choked,
                                      client::Command& command,
                                      const std::array<float, 3>& incoming, std::uint8_t& send,
                                      const LivePrecalculationBindings& bindings) {
    precalculate_angles(global, config, state, player, choked, command, incoming, send, bindings);
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: proximity.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
bool nearby(ProximityEngine& engine, ProximityEntity candidate, ProximityEntity local) {
    const auto first = engine.origin(candidate);
    const auto second = engine.origin(local);
    const auto y = first[1] - second[1], x = first[0] - second[0], z = first[2] - second[2];
    return (y * y + x * x) + z * z < 10000.0f;
}
}
bool any_nearby_entity(ProximityEngine& engine, ProximityEntity local) {
    const auto* list = engine.list();
    if (!list || list->count <= 0)
        return false;
    for (std::int32_t index = 0; index < list->count; ++index) {
        const auto candidate = list->entities[index];
        if (candidate && engine.active(candidate)) {
            if (nearby(engine, candidate, local))
                return true;
            list = engine.list();
        }
    }
    return false;
}
bool first_nearby_entity(ProximityEngine& engine, ProximityEntity local) {
    const auto* list = engine.list();
    if (!list || list->count == 0)
        return false;
    const auto first = list->entities[0];
    return first && engine.active(first) && nearby(engine, first, local);
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: runtime.cpp
// ============================================================================

namespace skeet::anti_aim {
void initialize_command_state_prefix(AngleStageState& state, PrecalculationState& precalculation,
                                     AngleParameters& parameters) noexcept {
    state.saved_yaw_198 = 0.0f;
    state.value_1b4 = -1;
    state.value_1c4 = -1;
    parameters.value_134 = 60.0f;
    parameters.value_138 = 180.0f;
    parameters.value_13c = 118.0f;
    parameters.value_140 = 58.0f;
    parameters.value_144 = 179.0f;
    parameters.value_148 = 90.0f;
    parameters.value_14c = 20.0f;
    parameters.value_150 = 89.0f;
    parameters.value_154 = 0.75f;
    state.cached_yaw_18c = std::numeric_limits<float>::max();
    state.value_190 = 0;
    precalculation.micro_move_19c = 1.01f;
    state.value_1a8 = std::numeric_limits<float>::max();
    state.value_1ac = 0;
    state.value_1b0 = 0;
}
void initialize_command_animation_state(AngleStageState& state, FinisherState& finisher) noexcept {
    animation::initialize_animation_state(finisher.animation_1d8.data());
    state.flags_130 |= 1u;
    // The logical source view mirrors the same native field as the retained
    // animation buffer; this copy does not initialize or alter the raw field.
    std::memcpy(&state.reference_yaw_258, finisher.animation_1d8.data() + 0x80, sizeof(float));
}
void initialize_command_pattern(AngleParameters& parameters) noexcept {
    parameters.pattern_15c = {-1, 1, 0, -1, 1, -1, 0, 1, -1, 0, 1, 0};
}
void initialize_command_state(AngleStageState& state, PrecalculationState& precalculation,
                              FinisherState& finisher, AngleParameters& parameters) noexcept {
    initialize_command_state_prefix(state, precalculation, parameters);
    initialize_command_animation_state(state, finisher);
    initialize_command_pattern(parameters);
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: runtime_owner.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::anti_aim {
namespace {
std::uint32_t field(client::ClientRuntimeStorage& storage, std::uint32_t salt) {
    const auto& globals = storage.offsets().globals;
    for (const auto& row : engine::build_receive_offset_recipes())
        if (row.initial_xor_key == salt)
            return std::uint32_t(std::ptrdiff_t(engine::DecodedOffset(
                globals, row.pointer_identity, row.xor_key_identity, salt, 0)));
    for (const auto& row : engine::build_data_map_offset_recipes())
        if (row.initial_xor_key == salt)
            return std::uint32_t(std::ptrdiff_t(engine::DecodedOffset(
                globals, row.pointer_identity, row.xor_key_identity, salt, 0)));
    if (salt == 0x043FA575u)
        return std::uint32_t(
            std::ptrdiff_t(engine::DecodedOffset(globals, 0x4346757Cu, 0x4346707Cu, salt, 0)));
    throw std::invalid_argument("Anti-aim field has no recovered canonical offset recipe");
}
CommandOwnerOffsets command_fields(client::ClientRuntimeStorage& storage) {
    const auto f = [&](std::uint32_t salt) { return field(storage, salt); };
    return {f(0xE2FBE695), f(0x36375C23), f(0x3309C0CF), f(0xFB7E3404), f(0x6B1A59B5),
            f(0x7501CB91), f(0x8B5586F6), f(0xA15C3105), f(0x3D10B94C), f(0x7FCB7381),
            f(0x3EB71843), f(0xD720AEA6), f(0x9F8E7F71), f(0xBF567619), f(0xDDA99AB5),
            f(0xB7EA8FDA), f(0xE1DF7E9B), f(0x3B93667B), f(0x38CA5228), f(0x043FA575),
            f(0x334B460C), f(0x86A5F8B3)};
}
/* deduplicated helper */
engine::OwnerRegistryBinding registry(client::ClientRuntimeStorage& storage,
                                      std::uint32_t identity) {
    return {storage.owner_registries().at(identity), storage.memory()};
}
class RuntimeProximity final : public ProximityEngine {
    void* const& current_;
    const std::uint32_t ticking_;
    ProximityList projection_;

public:
    RuntimeProximity(void* const& current, std::uint32_t ticking)
        : current_(current), ticking_(ticking) {}
    const ProximityList* list() override {
        if (!current_)
            return nullptr;
        projection_ = {read<const ProximityEntity*>(current_, 0), read<std::int32_t>(current_, 0xC)};
        return &projection_;
    }
    bool active(ProximityEntity entity) override {
        return read<std::uint8_t>(reinterpret_cast<const void*>(entity), ticking_) != 0;
    }
    BallisticVector origin(ProximityEntity entity) override {
        return *engine::invoke<const BallisticVector*>(reinterpret_cast<void*>(entity), 10);
    }
};
client::ClientRuntimeStorage&
checked(client::ClientRuntimeStorage& storage, const client::EngineRuntimeBindings& engine,
        client::ConfigurationRuntimeOwner& config, engine::SceneQueryRuntimeOwner& scene,
        animation::RuntimeOwner& animation, client::MovementHudRuntimeOwner& movement,
        client::CommandCaptureRuntimeOwner& capture, client::HistoryRuntimeOwner& history,
        client::TargetRefreshRuntimeOwner& target, client::VisualRuntimeOwner& visual,
        script::SourceScriptRuntimeOwner& scripts, const RuntimeImports& imports) {
    auto& data = storage.data();
    if (&engine.variables.data() != &data || &config.storage() != &storage ||
        &scene.storage() != &storage || &animation.storage() != &storage ||
        &movement.storage() != &storage || &capture.storage() != &storage ||
        &history.storage() != &storage || &target.storage() != &storage ||
        &visual.storage() != &storage || &scripts.storage() != &storage ||
        &scene.players() != &history.players() ||
        &animation.players() != &history.players().cache() ||
        capture.angle_environment().angle_guard != capture.view_angles().environment.angle_guard ||
        capture.angle_environment().angle_guard != &config.anti_untrusted().enabled.value ||
        &capture.budget().game_rules != &data.native_cell<void**>(0x4346A3A4u) ||
        &capture.budget().configured != &data.native_cell<std::int32_t>(0x43469274u) ||
        &scripts.bridge().name_storage != &storage.event_names())
        throw std::invalid_argument(
            "Anti-aim requires the canonical configuration, scene, animation, movement, capture, history and Lua owners");
    const auto core = engine.interfaces.core();
    const auto* globals = data.native_cell<const std::byte*>(0x43467B04u);
    if (!globals || !core.engine_client || !core.entity_list ||
        core.engine_client != data.native_cell<void*>(0x4346A514u) ||
        core.entity_list != data.native_cell<void*>(0x4346A890u) ||
        data.native_cell<void*>(0x4346A338u) != globals + 0x20 ||
        data.native_cell<void*>(0x43467EB8u) != globals + 0x10 ||
        data.native_cell<void*>(0x434678F8u) != globals + 4 ||
        !data.native_cell<void*>(0x43475B2Cu) || !data.native_cell<void*>(0x434757DCu) ||
        !data.native_cell<void*>(0x4346A89Cu) || !data.native_cell<void*>(0x43468304u) ||
        !data.native_cell<void**>(0x4346A3A4u) ||
        animation.bones().entity_list != core.entity_list ||
        history.entity_runtime().entity_list != core.entity_list ||
        history.entity_runtime().offsets.animation_state !=
            std::uint32_t(animation.bones().fields.animation_state_3d52a1a7) ||
        history.entity_runtime().offsets.pose_parameters !=
            std::uint32_t(animation.bones().fields.pose_parameters))
        throw std::invalid_argument(
            "Anti-aim requires its actual published engine objects and clock aliases");
    if (!imports.seed || !imports.integer || !imports.range || !imports.trigonometry.sine ||
        !imports.trigonometry.cosine || !imports.atan2 || !imports.clipper.clip)
        throw std::invalid_argument(
            "Anti-aim requires genuine RNG, CRT and hitbox clipping boundaries");
    return storage;
}
}
RuntimeImports native_runtime_imports(client::ClientRuntimeStorage& storage,
                                      const client::EngineRuntimeBindings& engine) {
    if (&engine.variables.data() != &storage.data())
        throw std::invalid_argument("Anti-aim imports require canonical engine modules");
    const auto module = GetModuleHandleW(L"vstdlib.dll");
    if (!module || module != engine.modules.at(u"vstdlib.dll") ||
        module != storage.data().native_cell<void*>(0x43468A7Cu))
        throw std::invalid_argument("Anti-aim requires its actual already loaded vstdlib module");
    const auto seed = GetProcAddress(module, "RandomSeed"),
               integer = GetProcAddress(module, "RandomInt"),
               range = GetProcAddress(module, "RandomFloat");
    if (!seed || !integer || !range)
        throw std::runtime_error(
            "Anti-aim requires vstdlib RandomSeed, RandomInt and RandomFloat exports");
    engine::initialize_build_hitbox_clip_bindings(storage.data());
    return {reinterpret_cast<RuntimeImports::Seed>(seed),
            reinterpret_cast<RuntimeImports::Integer>(integer),
            reinterpret_cast<RuntimeImports::Range>(range),
            animation::native_sse_trigonometry(),
            animation::native_sse_atan2,
            engine::native_hitbox_clipper(storage.data())};
}
struct RuntimeOwner::Data {
    client::ClientRuntimeStorage& storage;
    RuntimeImports imports;
    InputBitmap input;
    AngleStageState state;
    AngleParameters parameters;
    PrecalculationState precalculation;
    std::array<std::byte, 0x348> animation_state;
    std::array<Matrix3x4, animation::maximum_bones> matrices;
    FinisherState finisher;
    MovementSimulation movement_workspace;
    BodyFreestandingWorkspace body_workspace;
    RuntimeProximity proximity;
    CommandOwnerPitchRandomBindings pitch;
    JitterRandom random;
    BodyFreestandingMath body_math;
    CommandOwnerPhase2Bindings phase2;
    CommandOwnerEligibilityBindings eligibility;
    CommandOwnerAngleBindings angles;
    CommandOwnerPhase4Bindings phase4;
    CommandOwnerPhase6Bindings phase6;
    CommandOwner command;
    OwnerHudBindings hud;
    client::AntiAimRuntimeFactoryBindings factory;
    static void seed(void* raw, std::int32_t value) {
        static_cast<Data*>(raw)->imports.seed(value);
    }
    static std::int32_t integer(void* raw, std::int32_t first, std::int32_t last) {
        return static_cast<Data*>(raw)->imports.integer(first, last);
    }
    static float range(void* raw, float first, float last) {
        return static_cast<Data*>(raw)->imports.range(first, last);
    }
    static void initialize(void* raw) {
        auto& self = *static_cast<Data*>(raw);
        client::create_anti_aim_runtime_owner(self.factory, self.storage.configuration());
    }
    Data(client::ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
         client::ConfigurationRuntimeOwner& c, engine::SceneQueryRuntimeOwner& scene,
         animation::RuntimeOwner& animation, client::MovementHudRuntimeOwner& movement,
         client::CommandCaptureRuntimeOwner& capture, client::HistoryRuntimeOwner& history,
         client::TargetRefreshRuntimeOwner& target, client::VisualRuntimeOwner& visual,
         script::SourceScriptRuntimeOwner& scripts, RuntimeImports i)
        : storage(checked(s, e, c, scene, animation, movement, capture, history, target, visual,
                          scripts, i)),
          imports(i), input(s.input_runtime().bitmap()), precalculation{state, 0, 0, 0},
          finisher{state, animation_state, {}, matrices},
          proximity(s.data().native_cell<void*>(0x434688C0u), field(s, 0x0E5BECE5u)),
          pitch{this, seed, integer}, random{this, seed, range},
          body_math{imports.trigonometry, imports.atan2},
          phase2{scene.queries().movement(), movement_workspace, capture.angle_environment()},
          eligibility{
              s.data().native_cell<void*>(0x4346A89Cu), s.data().native_cell<void*>(0x4346A514u),
              s.data().native_cell<void*>(0x43468994u),
              s.data().native_cell<engine::MemberFunction<void*, const char*>>(0x4346A374u)},
          angles{proximity,      scene.queries().ballistics(),
                 body_math,      imports.clipper,
                 body_workspace, pitch,
                 random},
          phase4{eligibility, target, angles},
          phase6{s.data().native_cell<float>(0x43476B40u),
                 s.data().native_cell<RageVector>(0x4347A4F0u),
                 history.entity_runtime().offsets,
                 s.data().native_cell<std::uint8_t*>(0x43467B24u),
                 s.data().native_cell<void*>(0x434693F0u),
                 s.data().native_cell<lagcomp::SetAbsoluteVector>(0x434688C8u),
                 animation.bones()},
          command{
              c.anti_aim_global(),
              c.anti_aim_settings(),
              parameters,
              state,
              precalculation,
              finisher,
              phase2,
              phase4,
              phase6,
              movement.movement_capture(),
              movement.movement_environment(),
              command_fields(s),
              {s.data().native_cell<std::int32_t*>(0x43475B2Cu),
               s.data().native_cell<std::int32_t*>(0x434757DCu),
               s.data().native_cell<float*>(0x4346A338u),
               s.data().native_cell<float*>(0x43467EB8u),
               s.data().native_cell<void**>(0x4346A3A4u),
               s.data().native_cell<std::int32_t*>(0x434678F8u),
               s.data().native_cell<std::int32_t*>(0x43469300u),
               s.data().native_cell<void*>(0x43467B04u),
               s.data().native_cell<std::int32_t>(0x43469274u),
               s.data().native_cell<std::uint32_t>(0x434759C0u),
               s.data().native_cell<TargetState>(0x4346DB14u),
               s.data().native_cell<std::int32_t>(0x434759C8u),
               s.data().native_cell<std::int32_t>(0x434759CCu),
               s.data().native_cell<std::int32_t>(0x434759F4u),
               s.data().native_cell<std::uint8_t>(0x43468410u),
               s.data().native_cell<RageVector>(0x434759D0u),
               c.rage_settings().global.duck_peek_assist.value.binding,
               input,
               s.data().native_cell<std::uint8_t*>(0x434683D0u),
               s.data().native_cell<std::uint8_t>(0x43468D7Cu),
               s.data().native_cell<std::array<animation::AnimationAngleSample, 2>>(0x434759DCu)},
              s.data().native_cell<void*>(0x4346A514u),
              s.data().native_cell<void*>(0x43468304u),
              s.data().native_cell<void*>(0x4346A890u)},
          hud{scripts.bridge(), c.other_esp_settings().feature_indicators, visual.indicators(),
              field(s, 0x9D920E35u)},
          factory{{registry(s, 0x43475B78u), s.command_phases(), registry(s, 0x43475A9Cu),
                   registry(s, 0x43475AD8u)},
                  {command, hud, s.data().native_cell<void*>(0x43467204u),
                   s.data().native_cell<CommandOwner*>(0x43475940u)}} {}
};
RuntimeOwner::RuntimeOwner(
    client::ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
    client::ConfigurationRuntimeOwner& c, engine::SceneQueryRuntimeOwner& scene,
    animation::RuntimeOwner& animation, client::MovementHudRuntimeOwner& movement,
    client::CommandCaptureRuntimeOwner& capture, client::HistoryRuntimeOwner& history,
    client::TargetRefreshRuntimeOwner& target, client::VisualRuntimeOwner& visual,
    script::SourceScriptRuntimeOwner& scripts)
    : RuntimeOwner(s, e, c, scene, animation, movement, capture, history, target, visual, scripts,
                   native_runtime_imports(s, e)) {}
RuntimeOwner::RuntimeOwner(
    client::ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
    client::ConfigurationRuntimeOwner& c, engine::SceneQueryRuntimeOwner& scene,
    animation::RuntimeOwner& animation, client::MovementHudRuntimeOwner& movement,
    client::CommandCaptureRuntimeOwner& capture, client::HistoryRuntimeOwner& history,
    client::TargetRefreshRuntimeOwner& target, client::VisualRuntimeOwner& visual,
    script::SourceScriptRuntimeOwner& scripts, RuntimeImports imports)
    : data_(std::make_unique<Data>(s, e, c, scene, animation, movement, capture, history, target,
                                   visual, scripts, imports)) {}
RuntimeOwner::~RuntimeOwner() = default;
client::ClientRuntimeStorage& RuntimeOwner::storage() const noexcept {
    return data_->storage;
}
CommandOwner& RuntimeOwner::command() noexcept {
    return data_->command;
}
AngleStageState& RuntimeOwner::state() noexcept {
    return data_->state;
}
PrecalculationState& RuntimeOwner::precalculation() noexcept {
    return data_->precalculation;
}
FinisherState& RuntimeOwner::finisher() noexcept {
    return data_->finisher;
}
client::AntiAimRuntimeFactoryBindings& RuntimeOwner::factory_bindings() noexcept {
    return data_->factory;
}
void RuntimeOwner::bind_initializers(client::NativeStartupSources& sources) {
    if (&sources.data() != &storage().data())
        throw std::invalid_argument("Anti-aim factory requires its canonical startup scheduler");
    sources.bind_initializer({client::anti_aim_runtime_factory, data_.get(), Data::initialize});
}
} //namespace skeet::anti_aim

// ============================================================================
// Subcomponent: scheduler.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
float normalize_yaw(float yaw) noexcept {
    if (std::isnan(yaw))
        return 0.0f;
    const auto absolute = std::bit_cast<float>(std::bit_cast<std::uint32_t>(yaw) & 0x7FFFFFFFu);
    if (absolute > 180.0f) {
        const auto turns =
            std::bit_cast<float>(std::bit_cast<std::uint32_t>(yaw / 360.0f) & 0x7FFFFFFFu);
        const auto correction = round_movement_step(turns) * 360.0f;
        if (yaw < 0.0f)
            yaw += correction;
        else
            yaw -= correction;
    }
    return yaw;
}
}
void finalize_command_yaw(float& command_yaw, float frame_yaw_24, CommandYawState& state) noexcept {
    auto current = command_yaw;
    if (frame_yaw_24 != current) {
        current = normalize_yaw(current);
        command_yaw = current;
    }
    apply_cached_command_yaw(command_yaw, state);
}
void apply_cached_command_yaw(float& command_yaw, CommandYawState& state) noexcept {
    if (!(state.flags_130 & 8u))
        return;
    if (state.cached_yaw_18c != std::numeric_limits<float>::max())
        command_yaw = state.cached_yaw_18c;
    else
        state.cached_yaw_18c = normalize_yaw(command_yaw);
}
void update_command_flags(std::uint32_t frame_flags, std::uint32_t& flags_130) noexcept {
    flags_130 = (flags_130 & ~4u) | ((frame_flags >> 4) & 4u);
}
void begin_angle_cycle(bool cached_command, std::int32_t choked_commands, std::int32_t spin_offset,
                       AngleCycleState& state) noexcept {
    state.flags_130 = (state.flags_130 & ~8u) | (cached_command ? 8u : 0u);
    if (choked_commands != 0 || cached_command)
        return;
    const auto speed = static_cast<float>(spin_offset);
    const auto updated = normalize_yaw(speed + state.saved_yaw_198);
    ++state.phase_190;
    state.saved_yaw_198 = updated;
}
ActionGatePath evaluate_action_gates(const ActionGateSettings& settings,
                                     const ActionGateFrame& frame, const ActionGatePlayer& player,
                                     ActionGateGlobals& globals, ActionGateState& state,
                                     client::Command& command, std::uint8_t& send_packet,
                                     ProximityEngine& proximity) {
    const auto stop = ActionGatePath::stop;
    if (frame.state_104 == 8)
        return stop;
    const auto kind = frame.weapon_kind_124;
    auto& buttons = command.movement.buttons;
    if (kind == 9 || globals.mode_43475b2c == state.sequence_158) {
        if (!player.weapon_state_e1df7e9b && player.weapon_time_3b93667b > 0.0f &&
            static_cast<float>(player.tick_base_3d10b94c) * globals.interval_per_tick >
                player.weapon_time_3b93667b) {
            if (globals.choked_434757dc == 0)
                send_packet = 0;
            state.sequence_158 = globals.mode_43475b2c;
            return stop;
        }
    } else if (kind != 7 &&
               ((frame.flags & 0x100000u) || ((frame.flags & 0x400u) && (buttons & 1u)) ||
                ((frame.flags & 2u) && (buttons & 0x800u) &&
                 (frame.weapon_id_d8 == 0x40 || kind == 0)))) {
        if (kind == 0 && frame.choked_15c < frame.choke_limit_fc) {
            send_packet = 0;
            globals.command_flags = (globals.command_flags & ~0x400u) | 0x200u;
        }
        return stop;
    }
    if (frame.state_104 == 9 && (buttons & 0x618u)) {
        if (frame.choke_limit_fc <= 6 || !settings.enabled || !settings.pitch_mode ||
            (globals.command_flags & 0x600u))
            return stop;
        if (frame.choked_15c < frame.choke_limit_fc) {
            send_packet = 0;
            return stop;
        }
        buttons &= ~0x618u;
    }
    if (settings.yaw_base == 1) {
        const bool found =
            globals.target_handle != -1 &&
            globals.entities.by_handle(globals.entities.object, globals.target_handle);
        if (!found && (frame.flags & 8u))
            return stop;
    }
    if ((frame.flags & 0x40u) && kind != 7 && (frame.previous_buttons_44 & 0x801u)) {
        if ((frame.previous_buttons_44 & 0x801u) != 0x800u || kind == 0 ||
            frame.weapon_id_d8 == 0x40)
            return stop;
    }
    if (buttons & 0x20u) {
        if (player.movement_type_8b5586f6 == 3) {
            if (!first_nearby_entity(proximity, player.entity))
                return stop;
        } else if (frame.weapon_id_d8 != 0x31 || !player.field_7fcb7381)
            return stop;
    }
    if (command.angles[0] != frame.pitch_20 || command.angles[1] != frame.yaw_24)
        return stop;
    if (!player.has_weapon_3d52a1a7)
        return ActionGatePath::cached_yaw;
    state.flags_130 |= 2u;
    return ActionGatePath::angles;
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: startup_initializers.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
void initialize_enabled(void* context) { //434931DE
    auto& source = *static_cast<StartupInitializerBindings*>(context);
    append_config_link(source.configuration.descriptors, source.fake_lag.enabled);
}
void initialize_hotkey(void* context) { //434931F8
    auto& source = *static_cast<StartupInitializerBindings*>(context);
    append_config_link(source.configuration.descriptors, source.fake_lag.hotkey);
    skeet::initialize_hotkey(source.configuration.hotkeys, source.fake_lag.hotkey.value, 0x1BU,
                             HotkeyMode::always);
}
void initialize_slow_motion_key(void* context) { //43493456
    auto& source = *static_cast<StartupInitializerBindings*>(context);
    append_config_link(source.configuration.descriptors, source.other.slow_motion_key);
    skeet::initialize_hotkey(source.configuration.hotkeys, source.other.slow_motion_key.value,
                             0x14U, HotkeyMode::hold);
}
void initialize_leg_movement(void* context) { //43490A8D
    auto& source = *static_cast<StartupInitializerBindings*>(context);
    append_config_link(source.configuration.descriptors, source.other.leg_movement);
    source.other.leg_movement.value = 0;
}
}

void bind_native_startup_initializers(client::NativeStartupSources& sources,
                                      StartupInitializerBindings& bindings) {
    sources.bind_initializer({0x434931DEU, &bindings, initialize_enabled});
    sources.bind_initializer({0x434931F8U, &bindings, initialize_hotkey});
    sources.bind_initializer({0x43493456U, &bindings, initialize_slow_motion_key});
    sources.bind_initializer({0x43490A8DU, &bindings, initialize_leg_movement});
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: state.cpp
// ============================================================================

namespace skeet::anti_aim {
void reset_state(State& state) noexcept {
    state.flags_130 |= 1;
    state.value_1ac = 0;
    state.value_1b4 = -1;
    state.value_1c4 = -1;
    state.value_158 = -1;
    state.value_190 = 0;
    state.value_1a8 = std::bit_cast<float>(0x7F7FFFFFu);
    state.value_1a0 = 58.0f;
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: target_geometry.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
constexpr std::array<std::uint32_t, 5> selected_hitboxes{0, 3, 6, 11, 12}; // 43474A30
constexpr float epsilon = 1.1920928955078125e-7f;
float sign_flip(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) ^ 0x80000000u);
}
RageVector normalize(const RageVector& v) noexcept {
    const auto inverse = 1.0f / (length(v) + epsilon);
    return {inverse * v[0], inverse * v[1], inverse * v[2]};
}
Matrix3x4 rotated_matrix(const Matrix3x4& bone, const RageVector& angles,
                         const RageTrigonometry& trig) {
    constexpr float radians = 0.01745329238474369f;
    const auto sy = trig.sine(angles[1] * radians), cy = trig.cosine(angles[1] * radians);
    const auto sp = trig.sine(angles[0] * radians), cp = trig.cosine(angles[0] * radians);
    const auto sr = trig.sine(angles[2] * radians), cr = trig.cosine(angles[2] * radians);
    const auto crcy = cr * cy, crsy = cr * sy, srcy = sr * cy, srsy = sr * sy;
    const Matrix3x4 rotation{{{cp * cy, srcy * sp - crsy, crcy * sp + srsy, 0.0f},
                              {cp * sy, srsy * sp + crcy, crsy * sp - srcy, 0.0f},
                              {sign_flip(sp), sr * cp, cr * cp, 0.0f}}};
    Matrix3x4 output;
    for (unsigned row = 0; row < 3; ++row)
        for (unsigned column = 0; column < 4; ++column) {
            const auto combined = (bone.rows[row][2] * rotation.rows[2][column] +
                                   bone.rows[row][1] * rotation.rows[1][column]) +
                                  bone.rows[row][0] * rotation.rows[0][column];
            output.rows[row][column] = combined + (column == 3 ? bone.rows[row][3] : 0.0f);
        }
    return output;
}
RageVector capsule_support(const RageVector& first, const RageVector& second,
                           const RageVector& direction, float radius) noexcept {
    const auto projection =
        ((second[0] - first[0]) * direction[0] + (second[1] - first[1]) * direction[1]) +
        (second[2] - first[2]) * direction[2];
    const auto& endpoint = projection < 0.0f ? first : second;
    const auto scale = radius / length(direction);
    return {direction[0] * scale + endpoint[0], direction[1] * scale + endpoint[1],
            direction[2] * scale + endpoint[2]};
}
/* deduplicated helper */
}

TargetPoints build_target_points(const RageVector& target_origin, float view_offset_z,
                                 const RageVector& shoot_origin,
                                 std::span<const RageLocalHitbox> hitboxes,
                                 const HitboxMatrices& matrices,
                                 const RageTrigonometry& trig) { // 433F2CC1
    TargetPoints output;
    const RageVector center{target_origin[0], target_origin[1],
                            (view_offset_z - 2.0f) + target_origin[2]};
    output.points.fill(center);
    if (hitboxes.empty())
        return output;
    if (!trig.sine || !trig.cosine)
        throw std::logic_error("Target geometry requires native trigonometry bindings");
    const auto forward = normalize(
        {center[0] - shoot_origin[0], center[1] - shoot_origin[1], center[2] - shoot_origin[2]});
    RageVector right, up;
    if (forward[0] == 0.0f && forward[1] == 0.0f) {
        right = {0.0f, -1.0f, 0.0f};
        up = {sign_flip(forward[2]), 0.0f, 0.0f};
    } else {
        right = normalize({forward[1] - forward[2] * 0.0f, forward[2] * 0.0f - forward[0],
                           forward[0] * 0.0f - forward[1] * 0.0f});
        up = normalize({right[1] * forward[2] - right[2] * forward[1],
                        right[2] * forward[0] - right[0] * forward[2],
                        right[0] * forward[1] - right[1] * forward[0]});
    }
    float minimum = 1.0f, maximum = -1.0f;
    std::uint32_t band = 0;
    for (const auto index : selected_hitboxes) {
        if (index >= hitboxes.size())
            continue;
        const auto& hitbox = hitboxes[index];
        const auto matrix = rotated_matrix(matrices[index], hitbox.rotation, trig);
        std::array<RageVector, 4> candidates;
        std::uint32_t count;
        auto group = hitbox.hitgroup;
        if (hitbox.radius > 0.0f) {
            const auto first = transform(matrix, hitbox.minimum),
                       second = transform(matrix, hitbox.maximum);
            candidates[0] = capsule_support(first, second, right, hitbox.radius);
            candidates[1] = capsule_support(
                first, second, {sign_flip(right[0]), sign_flip(right[1]), sign_flip(right[2])},
                hitbox.radius);
            count = 2;
            if (index == 0 || index == 6) {
                const auto slot = index == 0 ? 4 : 5;
                output.points[slot] = capsule_support(first, second, up, hitbox.radius);
                output.hitgroups[slot] = group = 3;
            }
        } else {
            if (index == 11) {
                minimum = 1.0f;
                maximum = -1.0f;
            }
            // The native pair stays2/3 after any box. Only hitbox11 resets its scores.
            band = 2;
            count = 4;
            // This is the max-Y face, with coordinates halved before transformation.
            const std::array<RageVector, 4> local{
                {{hitbox.minimum[0], hitbox.maximum[1], hitbox.minimum[2]},
                 {hitbox.maximum[0], hitbox.maximum[1], hitbox.maximum[2]},
                 {hitbox.maximum[0], hitbox.maximum[1], hitbox.minimum[2]},
                 {hitbox.minimum[0], hitbox.maximum[1], hitbox.maximum[2]}}};
            for (unsigned i = 0; i < count; ++i)
                candidates[i] =
                    transform(matrix, {local[i][0] * 0.5f, local[i][1] * 0.5f, local[i][2] * 0.5f});
        }
        if (!group)
            continue;
        for (std::uint32_t i = 0; i < count; ++i) {
            const auto& point = candidates[i];
            const auto projection =
                ((point[1] - center[1]) * right[1] + (point[0] - center[0]) * right[0]) +
                (point[2] - point[2]) * right[2];
            const RageVector delta{(right[0] * projection + center[0]) - shoot_origin[0],
                                   (right[1] * projection + center[1]) - shoot_origin[1],
                                   (right[2] * projection + point[2]) - shoot_origin[2]};
            const auto inverse = 1.0f / (length(delta) + epsilon);
            const auto score = ((delta[1] * inverse) * right[1] + (delta[0] * inverse) * right[0]) +
                               (inverse * delta[2]) * right[2];
            if (score < minimum) {
                output.points[band] = point;
                output.hitgroups[band] = group;
                minimum = score;
            }
            if (score > maximum) {
                output.points[band + 1] = point;
                output.hitgroups[band + 1] = group;
                maximum = score;
            }
        }
    }
    return output;
}

TargetPoints target_points_from_entity(void* entity, const TargetGeometryOffsets& offsets,
                                       const RageVector& target_origin,
                                       const RageVector& shoot_origin,
                                       const HitboxMatrices& matrices,
                                       const RageTrigonometry& trig) {
    const auto* view = engine::invoke<const RageVector*>(entity, 77);
    const auto height = (*view)[2];
    const auto* handle = field<const void*>(entity, offsets.studio_header);
    const auto* header = handle ? field<const void*>(handle, 0) : nullptr;
    if (!header)
        return build_target_points(target_origin, height, shoot_origin, {}, matrices, trig);
    const auto index = field<std::int32_t>(entity, offsets.hitbox_set);
    const auto* set = static_cast<const std::byte*>(header) + field<std::int32_t>(header, 0xb0) +
                      std::ptrdiff_t(index) * 12;
    const auto count = field<std::int32_t>(set, 4);
    if (count <= 0)
        return build_target_points(target_origin, height, shoot_origin, {}, matrices, trig);
    const auto* hitboxes = set + field<std::int32_t>(set, 8);
    std::array<RageLocalHitbox, 13> selected{};
    for (const auto hitbox_index : selected_hitboxes) {
        if (hitbox_index >= std::uint32_t(count))
            continue;
        const auto raw =
            field<TargetStudioHitbox>(hitboxes, hitbox_index * sizeof(TargetStudioHitbox));
        selected[hitbox_index] = {
            raw.hitgroup, raw.minimum, raw.maximum,
            raw.rotation, raw.radius,  hitboxes + hitbox_index * sizeof(TargetStudioHitbox)};
    }
    return build_target_points(
        target_origin, height, shoot_origin,
        std::span(selected.data(), std::min<std::size_t>(std::size_t(count), selected.size())),
        matrices, trig);
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: targets.cpp
// ============================================================================

namespace skeet::anti_aim {
std::uint32_t TargetPlayerHistoryView::record_count() const noexcept {
    return source_ ? source_->history.count : decoded_->timeline.record_count;
}
const lagcomp::LagRecord& TargetPlayerHistoryView::at(std::uint32_t age) const noexcept {
    return source_ ? source_->history.at(age) : decoded_->timeline.at(age);
}
const esp::PlayerGeometry& TargetPlayerHistoryView::geometry() const noexcept {
    return source_ ? source_->geometry : decoded_->geometry;
}
float TargetPlayerHistoryView::prediction_field() const noexcept {
    return source_ ? source_->native.target_prediction_524 : decoded_->field_524;
}
void TargetPlayerHistoryView::last_damage_tick(std::int32_t value) const noexcept {
    if (source_)
        source_->native.last_target_damage_tick = value;
    else
        decoded_->last_damage_tick = value;
}
TargetPlayerHistoryView TargetPlayerHistories::operator[](std::uint32_t index) const {
    if (source_)
        return TargetPlayerHistoryView(*source_->get(index));
    return TargetPlayerHistoryView(decoded_[index & 63u]);
}
namespace {
std::int32_t sub(std::int32_t a, std::int32_t b) {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) - std::uint32_t(b));
}
esp::PlayerWeaponMetadata weapon_metadata(void* weapon, const TargetSelectorContext& context) {
    const auto* info = engine::invoke<const std::byte*>(weapon, 461);
    esp::PlayerWeaponMetadata result;
    result.definition =
        field<std::uint16_t>(weapon, std::uint32_t(context.targeting.offsets.weapon_identifier));
    result.penetration = field<float>(info, 0x100);
    result.range = field<float>(info, 0x10c);
    result.range_modifier = field<float>(info, 0x110);
    result.armor_ratio = engine::invoke<float>(weapon, 434);
    result.damage = float(engine::invoke<std::int32_t>(weapon, 431));
    result.kind = field<std::int32_t>(info, 0xfc);
    return result;
}
esp::PlayerWeaponMetadata player_metadata(void* player, const TargetSelectorContext& c) {
    const auto get_weapon = [&](std::int32_t handle) -> void* {
        return engine::invoke<void*>(c.targeting.entity_list, 4, handle);
    };
    const auto is_ammo = [&](void* weapon) {
        return esp::ammunition_weapon(
            field<std::uint16_t>(weapon, std::uint32_t(c.targeting.offsets.weapon_identifier)));
    };
    const auto active =
        field<std::int32_t>(player, std::uint32_t(c.targeting.offsets.active_weapon));
    if (active != -1)
        if (void* weapon = get_weapon(active))
            if (is_ammo(weapon))
                return weapon_metadata(weapon, c);
    if (field<std::uint8_t>(player, std::uint32_t(c.targeting.offsets.rejection_bytes[2])))
        for (unsigned i = 0; i < 2; ++i) {
            const auto handle = field<std::int32_t>(player, c.offsets.inventory + i * 4);
            if (handle == -1)
                continue;
            if (void* weapon = get_weapon(handle))
                if (engine::invoke<bool>(weapon, 166) && is_ammo(weapon))
                    return weapon_metadata(weapon, c);
        }
    return {};
}
BallisticQuery bullet_query(const esp::PlayerWeaponMetadata& m) {
    BallisticQuery query;
    query.allow_penetration = 1;
    query.weapon_id = m.definition;
    query.penetration = m.penetration;
    query.range = m.range;
    query.range_modifier = m.range_modifier;
    query.armor_ratio = m.armor_ratio;
    query.base_damage = m.damage;
    query.reserved = std::uint32_t(m.kind);
    return query;
}
std::int32_t target_prediction_ticks(TargetPlayerHistoryView history,
                                     const TargetSelectorContext& c) {
    if (!history.record_count())
        return 1;
    const auto& p = c.prediction_limit;
    const auto limit = lagcomp::prediction_tick_limit(
        p.local_available, p.field_bf567619, p.field_dda99ab5, p.field_b7ea8fda, p.configured);
    const auto received = engine::invoke<float>(c.engine_client, 14);
    const auto elapsed = sub(lagcomp::receive_time_to_tick(received, c.receive_interval),
                             history.at(0).header.receive_tick);
    if (limit < 0)
        return limit;
    return elapsed < 0 ? 0 : elapsed > limit ? limit : elapsed;
}
template <class Scored> float clock_time(const Scored& c) {
    return c.clock_owner && *c.clock_owner ? field<float>(*c.clock_owner, c.local_time_offset)
                                           : 0.0f;
}
std::int32_t integer_convar(const void* object) {
    for (;;) {
        const auto p = field<const void*>(object, 0x1c);
        if (p == object)
            break;
        object = p;
    }
    return std::bit_cast<std::int32_t>(field<std::uint32_t>(object, 0x30) ^
                                       std::uint32_t(reinterpret_cast<std::uintptr_t>(object)));
}
std::int32_t player_index(void*, const esp::RuntimePlayer& player) {
    return engine::invoke<std::int32_t>(static_cast<std::byte*>(player.engine_entity) + 8, 10);
}
esp::RuntimePlayer rule_player(void* entity, const TargetSelectorContext& c) {
    esp::RuntimePlayer result;
    result.engine_entity = entity;
    result.team = field<std::int32_t>(entity, std::uint32_t(c.targeting.offsets.team));
    result.squad = field<std::int32_t>(entity, std::uint32_t(c.targeting.offsets.survival_team));
    return result;
}
template <class Scored> bool special_crouch(void* entity, const Scored& c) {
    const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(entity) + 8, 10);
    const auto history = c.common.histories[std::uint32_t(index) & 63];
    if (history.record_count() > 1 &&
        sub(history.at(0).header.simulation_tick, history.at(1).header.simulation_tick) > 2) {
        const auto duck =
            field<float>(entity, std::uint32_t(c.common.targeting.offsets.duck_amount));
        if (duck > 0.0f && duck < 1.0f && field<float>(entity, c.offsets.field_fb7e3404) == 8.0f)
            return true;
    }
    return false;
}
std::int32_t selector_health(void* entity, const TargetScoredInputs& c) {
    if (!engine::invoke<bool>(static_cast<std::byte*>(entity) + 8, 9))
        return engine::invoke<std::int32_t>(entity, 122);
    if (!c.player_rules.present)
        return 0;
    const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(entity) + 8, 10);
    return c.player_rules.players[std::uint32_t(index)].connected
               ? c.dormant_health[std::uint32_t(index)]
               : 0;
}
std::int32_t selector_health(void* entity, const LiveTargetScoredContext& c) {
    if (!engine::invoke<bool>(static_cast<std::byte*>(entity) + 8, 9))
        return engine::invoke<std::int32_t>(entity, 122);
    auto* const resource = c.players.resource();
    if (!resource)
        return 0;
    const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(entity) + 8, 10);
    return c.players.connected(resource, index) ? c.players.resource_health(resource, index) : 0;
}
bool dormant_target(void* local, void* player, const TargetScoredInputs& c) {
    const auto a = rule_player(local, c.common), b = rule_player(player, c.common);
    esp::PlayerRuntimeEngine engine{};
    engine.index = player_index;
    return esp::dormant_enemy(&a, &b, engine, c.player_rules) &&
           esp::dormant_alive(&b, engine, c.player_rules);
}
bool dormant_target(void* local, void* player, const LiveTargetScoredContext& c) {
    auto* const a = c.players.identity(local);
    auto* const b = c.players.identity(player);
    return esp::dormant_enemy(a, b, c.players) && esp::dormant_alive(b, c.players);
}
}
#if defined(_M_IX86) || defined(__i386__)
bool __thiscall TargetTraceFilter::should_hit(void* receiver, BallisticEntity entity,
                                              std::uint32_t mask) {
#else
bool TargetTraceFilter::should_hit(void* receiver, BallisticEntity entity, std::uint32_t mask) {
#endif
    auto& self = *static_cast<TargetTraceFilter*>(receiver);
    const auto& entities = *self.entities;
    if (entity) {
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
    const auto delegate =
        reinterpret_cast<engine::MemberFunction<bool, BallisticEntity, std::uint32_t>>(
            self.engine_vtable[0]);
    return delegate(receiver, entity, mask);
}
TargetTraceFilter::TargetTraceFilter(void* const* base, const TargetTraceEntities& e,
                                     std::uint32_t entry_mask, std::uint32_t leave_mask,
                                     float minimum)
    : vtable(nullptr), mask(entry_mask), exit_mask(leave_mask), minimum_damage(minimum),
      engine_vtable(base), entities(&e) {
    if (!base)
        throw std::logic_error("Required engine trace filter vtable is not bound");
    methods = {reinterpret_cast<void*>(should_hit), base[1], base[2], base[3], base[4]};
    vtable = methods.data();
}
BallisticFilter TargetTraceFilter::binding() noexcept {
    return {this, fields_04[4], mask, exit_mask, minimum_damage};
}
BallisticEntity RawTargetTraceEntities::base_entity(BallisticEntity entity) const {
    return std::uint32_t(reinterpret_cast<std::uintptr_t>(
        engine::invoke<void*>(reinterpret_cast<void*>(std::uintptr_t(entity)), 7)));
}
std::int32_t RawTargetTraceEntities::collision_group(BallisticEntity entity) const {
    return field<std::int32_t>(reinterpret_cast<void*>(std::uintptr_t(entity)),
                               offsets_.collision_group);
}
bool RawTargetTraceEntities::is_player(BallisticEntity entity) const {
    return engine::invoke<bool>(reinterpret_cast<void*>(std::uintptr_t(entity)), 158);
}
std::int32_t RawTargetTraceEntities::parent_handle(BallisticEntity entity) const {
    return field<std::int32_t>(reinterpret_cast<void*>(std::uintptr_t(entity)),
                               offsets_.move_parent_handle);
}
BallisticEntity RawTargetTraceEntities::by_handle(std::int32_t handle) const {
    return std::uint32_t(reinterpret_cast<std::uintptr_t>(engine::invoke<void*>(list_, 4, handle)));
}
void clear_target_selection(TargetState& state) noexcept {
    state.handle = -1;
    state.fields_18 = {0, 0, 0x7FFFFFFFu, 0};
}
void insert_target_candidate(std::span<TargetCandidateOrder> candidates, std::uint32_t candidate,
                             std::uint32_t& head) noexcept {
    auto* link = &head;
    for (auto current = head;
         current != UINT32_MAX && !(candidates[current].score > candidates[candidate].score);
         current = candidates[current].next)
        link = &candidates[current].next;
    candidates[candidate].next = *link;
    *link = candidate;
}
void select_penetrable_yaw_target(TargetState& state, const TargetSelectorContext& c) {
    void* local = c.targeting.local_player;
    const auto* local_origin = engine::invoke<const RageVector*>(local, 10);
    const auto view = field<RageVector>(local, c.offsets.view_offset);
    const float eye_x = (*local_origin)[0] + view[0], eye_y = (*local_origin)[1] + view[1];
    engine::invoke<const RageVector*>(local, 77);
    const auto latency = lagcomp::mean_history_timing_ticks(c.timing);
    const auto* minimum = engine::invoke<const RageVector*>(local, 76);
    const auto* maximum = engine::invoke<const RageVector*>(local, 77);
    const std::array<RageVector, 4> corners{{{(*minimum)[0], (*minimum)[1], (*minimum)[2]},
                                             {(*minimum)[0], (*maximum)[1], (*minimum)[2]},
                                             {(*maximum)[0], (*maximum)[1], (*maximum)[2]},
                                             {(*maximum)[0], (*minimum)[1], (*maximum)[2]}}};
    float nearest = std::numeric_limits<float>::max();
    void* selected = nullptr;
    RageVector selected_position{};
    const auto limit = *c.max_clients;
    for (std::int32_t index = 1; index <= limit; ++index) {
        void* player = engine::invoke<void*>(c.targeting.entity_list, 3, index);
        if (!rage::target_is_eligible(c.targeting, player))
            continue;
        auto position = field<RageVector>(player, c.offsets.network_origin);
        if (latency) {
            const auto ticks =
                add(target_prediction_ticks(c.histories[std::uint32_t(index) & 63], c), latency);
            const auto velocity = field<MovementVector>(player, c.offsets.velocity);
            // The native predictor does not query hull virtuals outside this speed gate.
            const auto speed = velocity.x * velocity.x + velocity.y * velocity.y;
            if (speed > 25.0f && speed < 115600.0f) {
                const auto minimum_hull = *engine::invoke<const MovementVector*>(player, 76);
                const auto maximum_hull = *engine::invoke<const MovementVector*>(player, 77);
                MovementVector predicted{position[0], position[1], position[2]};
                predict_movement_origin(predicted, velocity, minimum_hull, maximum_hull, ticks,
                                        c.interval, c.collision, c.trace_flag);
                position = {predicted.x, predicted.y, predicted.z};
            }
        }
        const RageVector start{position[0], position[1], position[2] + 64.0f};
        auto query = bullet_query(player_metadata(player, c));
        const auto x = eye_x - start[0], y = eye_y - start[1];
        const auto inverse =
            1.0f / (std::sqrt(y * y + x * x) + std::numeric_limits<float>::epsilon());
        const auto dx = x * inverse, dy = y * inverse;
        bool penetrable = false;
        for (const auto& corner : corners) {
            const RageVector point{(dx * corner[0] - dy * corner[1]) + (*local_origin)[0],
                                   (dy * corner[0] + dx * corner[1]) + (*local_origin)[1],
                                   (*local_origin)[2] + corner[2]};
            if (trace_point_penetration(c.ballistics, start, point, 1, query, c.trace_flag) > 0) {
                penetrable = true;
                break;
            }
        }
        if (!penetrable)
            continue;
        const auto distance_y = position[1] - (*local_origin)[1],
                   distance_x = position[0] - (*local_origin)[0],
                   distance_z = position[2] - (*local_origin)[2];
        const auto distance =
            (distance_y * distance_y + distance_x * distance_x) + distance_z * distance_z;
        if (distance < nearest) {
            nearest = distance;
            selected = player;
            selected_position = position;
        }
    }
    if (selected) {
        state.handle = *engine::invoke<const std::int32_t*>(selected, 2);
        state.position = selected_position;
    }
}
template <class Scored> static void select_scored_target(TargetState& state, const Scored& scored) {
    const auto& c = scored.common;
    void* local = c.targeting.local_player;
    bool close_weapon = false;
    const auto weapon_handle =
        field<std::int32_t>(local, std::uint32_t(c.targeting.offsets.active_weapon));
    if (weapon_handle != -1)
        if (void* weapon = engine::invoke<void*>(c.targeting.entity_list, 4, weapon_handle)) {
            const auto kind = engine::invoke<std::int32_t>(weapon, 455);
            if (kind == 0)
                close_weapon = true;
            else if (kind == 8)
                close_weapon = field<std::int32_t>(weapon, scored.offsets.weapon_ammunition) > 0;
        }
    const auto latency = lagcomp::mean_history_timing_ticks(c.timing);
    const auto cap = std::int32_t(0.2f / c.interval + 0.5f);
    const auto prediction = latency < cap ? latency : cap;
    RageVector angles;
    engine::invoke<void>(c.engine_client, 18, angles.data());
    const auto yaw = angles[1] * std::bit_cast<float>(0x3C8EFA35u);
    const auto sy = scored.trigonometry.sine(yaw), cy = scored.trigonometry.cosine(yaw);
    const auto pitch = angles[0] * std::bit_cast<float>(0x3C8EFA35u);
    const auto sp = scored.trigonometry.sine(pitch), cp = scored.trigonometry.cosine(pitch);
    const RageVector direction{
        cp * cy, cp * sy, std::bit_cast<float>(std::bit_cast<std::uint32_t>(sp) ^ 0x80000000u)};
    const auto* current = engine::invoke<const MovementVector*>(local, 10);
    const auto velocity = field<MovementVector>(local, c.offsets.velocity);
    const auto flags =
        field<std::uint32_t>(local, std::uint32_t(c.targeting.offsets.movement_flags));
    MovementVector position;
    if constexpr (std::is_same_v<Scored, TargetScoredContext> ||
                  std::is_same_v<Scored, LiveTargetScoredContext>)
        position = advance_live_movement_origin(local, *current, velocity, flags, scored.movement,
                                                scored.movement_environment, scored.command,
                                                scored.input_buttons, scored.yaw, c.collision);
    else
        position = advance_movement_origin(
            std::uint32_t(reinterpret_cast<std::uintptr_t>(local)), *current, velocity, flags,
            scored.movement_player, scored.movement_rules, scored.movement_environment,
            scored.command, scored.input_buttons, scored.yaw, c.collision);
    const RageVector origin{position.x, position.y, position.z};
    const RageVector eye{origin[0], origin[1], origin[2] + 64.0f};
    struct Candidate {
        std::int32_t index;
        bool dormant;
        RageVector origin;
        float distance;
    };
    std::array<Candidate, 64> candidates{};
    std::array<TargetCandidateOrder, 64> ordering{};
    std::uint32_t count = 0, head = UINT32_MAX;
    bool use_distance = false;
    const auto limit = *c.max_clients;
    for (std::int32_t index = 1; index <= limit; ++index) {
        void* player = engine::invoke<void*>(c.targeting.entity_list, 3, index);
        if (!player || !engine::invoke<bool>(player, 158) ||
            field<std::uint8_t>(player, std::uint32_t(c.targeting.offsets.rejection_bytes[1])) ||
            field<std::uint8_t>(player, std::uint32_t(c.targeting.offsets.rejection_bytes[2])) ||
            c.targeting.player_adjustments.records[std::uint32_t(index)].adjustments.whitelist)
            continue;
        // Engine slot 8 writes the full 0x158-byte player_info_t. A 0x148-byte
        // buffer overwrites the first candidate's index, dormant flag and XY.
        std::array<std::byte, 0x158> info{};
        if (!engine::invoke<bool>(c.engine_client, 8, index, info.data()) ||
            info[0x13D] != std::byte{})
            continue;
        const auto history = c.histories[std::uint32_t(index) & 63];
        const bool dormant = engine::invoke<bool>(static_cast<std::byte*>(player) + 8, 9);
        if (dormant) {
            if (history.geometry().alpha <= 0.0f)
                continue;
            if (!dormant_target(local, player, scored))
                continue;
        } else if (field<std::uint8_t>(player,
                                       std::uint32_t(c.targeting.offsets.rejection_bytes[0])) ||
                   !rage::target_is_enemy(c.targeting, player))
            continue;
        const RageVector point =
            dormant ? RageVector{history.geometry().origin.x, history.geometry().origin.y,
                                 history.geometry().origin.z}
                    : field<RageVector>(player, c.offsets.network_origin);
        const float x = origin[0] - point[0], y = origin[1] - point[1], z = origin[2] - point[2];
        const float distance = std::sqrt((x * x + y * y) + z * z);
        if (close_weapon && distance < 500.0f)
            use_distance = true;
        const float dz = (point[2] + 36.0f) - eye[2], dy = point[1] - eye[1],
                    dx = point[0] - eye[0];
        const float projection = (dy * direction[1] + dx * direction[0]) + dz * direction[2];
        float score = std::numeric_limits<float>::max();
        if (!(projection < 0.0f) && !(5000.0f < projection)) {
            const auto px = point[0] - (direction[0] * projection + eye[0]);
            const auto py = point[1] - (direction[1] * projection + eye[1]);
            const auto pz = (point[2] + 36.0f) - (direction[2] * projection + eye[2]);
            score = std::sqrt((px * px + py * py) + pz * pz);
        }
        if (score == std::numeric_limits<float>::max()) {
            const float x2 = point[0] - eye[0], y2 = point[1] - eye[1], z2 = point[2] - eye[2];
            const auto length = std::sqrt((x2 * x2 + y2 * y2) + z2 * z2);
            score =
                2002.0f - ((direction[1] * y2 + direction[0] * x2) + direction[2] * z2) / length;
        } else {
            const RageSegment segment{point, {point[0], point[1], point[2] + 72.0f}};
            const auto nearest = closest_rage_ray_segment({eye, direction}, segment);
            const auto fraction = nearest.second_fraction, complement = 1.0f - fraction;
            const auto x2 = nearest.first_point[0] -
                            (segment.first[0] * complement + segment.second[0] * fraction);
            const auto y2 = nearest.first_point[1] -
                            (segment.first[1] * complement + segment.second[1] * fraction);
            const auto z2 = nearest.first_point[2] -
                            (segment.second[2] * fraction + segment.first[2] * complement);
            score = std::sqrt((x2 * x2 + y2 * y2) + z2 * z2);
        }
        candidates[count] = {index, dormant, point, distance};
        ordering[count].score = score;
        insert_target_candidate(ordering, count, head);
        ++count;
    }
    RageVector delta{origin[0] - scored.local_pose_origin[0],
                     origin[1] - scored.local_pose_origin[1],
                     origin[2] - scored.local_pose_origin[2]};
    if ((delta[1] * delta[1] + delta[0] * delta[0]) + delta[2] * delta[2] > 122500.0f)
        delta = {};
    TargetState best;
    best.handle = -1;
    float best_distance = std::numeric_limits<float>::max();
    bool dormant_processed = false;
    for (auto cursor = head; cursor != UINT32_MAX; cursor = ordering[cursor].next) {
        const auto& candidate = candidates[cursor];
        const auto history = c.histories[std::uint32_t(candidate.index) & 63];
        void* player = engine::invoke<void*>(c.targeting.entity_list, 3, candidate.index);
        if (!use_distance && ordering[cursor].score > 2000.0f && best.handle != -1)
            break;
        if (candidate.dormant) {
            if (dormant_processed)
                continue;
            dormant_processed = true;
        }
        auto point = candidate.origin;
        if (prediction && !candidate.dormant && history.prediction_field() != 0.0f &&
            history.record_count() >= 2) {
            const auto& previous = history.at(0).header.velocity;
            if (previous[0] * previous[0] + previous[1] * previous[1] > 6400.0f) {
                const auto movement = field<MovementVector>(player, c.offsets.velocity);
                const auto squared = movement.x * movement.x + movement.y * movement.y;
                if (squared > 25.0f && squared < 115600.0f) {
                    const auto minimum = *engine::invoke<const MovementVector*>(player, 76),
                               maximum = *engine::invoke<const MovementVector*>(player, 77);
                    MovementVector predicted{point[0], point[1], point[2]};
                    predict_movement_origin(predicted, movement, minimum, maximum, prediction,
                                            c.interval, c.collision, c.trace_flag);
                    point = {predicted.x, predicted.y, predicted.z};
                }
            }
        }
        const bool dormant = engine::invoke<bool>(static_cast<std::byte*>(player) + 8, 9);
        const bool standing =
            dormant || special_crouch(player, scored) ||
            !(field<float>(player, std::uint32_t(c.targeting.offsets.duck_amount)) >= 1.0f);
        const RageVector shooter{point[0], point[1], point[2] + (standing ? 64.0f : 46.0f)};
        const auto metadata = player_metadata(player, c);
        auto query = bullet_query(metadata);
        // Prediction changes the firing eye only. Native publication copies
        // the candidate-list origin saved before43359415, not its result.
        TargetState selected{*scored.sequence,
                             clock_time(scored),
                             *engine::invoke<const std::int32_t*>(player, 2),
                             candidate.origin,
                             {0, 0, 0x7FFFFFFFu, metadata.definition}};
        auto points =
            target_points_from_entity(local, scored.geometry_offsets, scored.local_pose_origin,
                                      shooter, scored.local_pose, scored.trigonometry);
        TargetTraceFilter engine_filter(scored.trace_filter_vtable, scored.trace_entities);
        const auto filter = engine_filter.binding();
        std::int32_t maximum_damage = 0;
        for (unsigned i = 0; i < 6; ++i) {
            for (unsigned j = 0; j < 3; ++j)
                points.points[i][j] += delta[j];
            trace_ballistic_bullet(c.ballistics,
                                   std::uint32_t(reinterpret_cast<std::uintptr_t>(player)), shooter,
                                   points.points[i], filter, query);
            const auto damage = final_bullet_damage(
                query.damage, query.armor_ratio, std::int32_t(query.reserved), query.weapon_id,
                points.hitgroups[i], field<std::int32_t>(local, scored.offsets.local_armor),
                field<std::uint8_t>(local, scored.offsets.local_helmet));
            if (damage > 0) {
                const auto time = engine::invoke<float>(c.engine_client, 14);
                selected.fields_18[0] = 4;
                if (damage > maximum_damage)
                    maximum_damage = damage;
                selected.fields_18[1] = std::uint32_t(maximum_damage);
                history.last_damage_tick(lagcomp::receive_time_to_tick(time, c.receive_interval));
            }
        }
        if (use_distance) {
            if (best_distance > candidate.distance) {
                best_distance = candidate.distance;
                best = selected;
            }
        } else {
            std::int32_t shots = INT32_MAX;
            if (maximum_damage > 0)
                shots =
                    add(sub(selector_health(player, scored), 1), maximum_damage) / maximum_damage;
            selected.fields_18[2] = std::uint32_t(shots);
            if (shots < std::bit_cast<std::int32_t>(best.fields_18[2]))
                best = selected;
        }
        if (best.handle == -1)
            best = selected;
    }
    if (best.handle != -1)
        state = best;
    else
        clear_target_selection(state);
}
void select_scored_yaw_target(TargetState& state, const TargetScoredContext& scored) {
    select_scored_target(state, scored);
}
void select_decoded_scored_yaw_target(TargetState& state,
                                      const DecodedTargetScoredContext& scored) {
    select_scored_target(state, scored);
}
template <class Scored>
static void refresh_target(TargetState& state, const Scored& scored, const void* selector_convar) {
    if (*scored.sequence == state.sequence)
        return;
    if (integer_convar(selector_convar) == 0)
        select_scored_target(state, scored);
    else
        select_penetrable_yaw_target(state, scored.common);
    state.sequence = *scored.sequence;
    if (state.handle == -1)
        clear_target_selection(state);
    else
        state.timestamp = clock_time(scored);
}
void refresh_yaw_target(TargetState& state, const TargetRefreshContext& c) {
    refresh_target(state, c.scored, c.selector_convar);
}
void refresh_live_yaw_target(TargetState& state, const LiveTargetScoredContext& c,
                             const void* selector_convar) {
    refresh_target(state, c, selector_convar);
}
void refresh_decoded_yaw_target(TargetState& state, const DecodedTargetScoredContext& scored,
                                const void* selector_convar) {
    refresh_target(state, scored, selector_convar);
}
} // namespace skeet::anti_aim

// ============================================================================
// Subcomponent: yaw.cpp
// ============================================================================

namespace skeet::anti_aim {
namespace {
float clamp_native(float value, float lower, float upper) noexcept {
    return _mm_cvtss_f32(
        _mm_min_ss(_mm_max_ss(_mm_set_ss(value), _mm_set_ss(lower)), _mm_set_ss(upper)));
}
float normalize(float angle) noexcept {
    if (std::isnan(angle))
        return 0.0f;
    const auto absolute = std::bit_cast<float>(std::bit_cast<std::uint32_t>(angle) & 0x7FFFFFFFu);
    if (absolute > 180.0f) {
        const auto quotient =
            std::bit_cast<float>(std::bit_cast<std::uint32_t>(angle / 360.0f) & 0x7FFFFFFFu);
        const auto revolutions = round_movement_step(quotient) * 360.0f;
        if (angle < 0.0f)
            angle += revolutions;
        else
            angle -= revolutions;
    }
    return angle;
}
}
float transform_yaw(float base, float offset, std::int32_t mode, const float& saved_angle,
                    const YawLimits& limits, const YawFrame& frame, const YawTarget& target,
                    const EntityLookup& entities) {
    switch (mode) {
    case 0:
        return base;
    case 1:
        return base - (limits.backward_angle - offset);
    case 2: {
        const auto fraction =
            clamp_native((frame.interpolation_158 - -300.0f) / 600.0f, 0.0f, 1.0f);
        const auto initial = base - limits.maximum_angle;
        const auto interpolated = initial - limits.backward_angle * fraction;
        return interpolated + offset;
    }
    case 3:
        return saved_angle;
    case 4:
        if (target.handle == -1 || !entities.by_handle(entities.object, target.handle))
            return base - (limits.backward_angle - offset);
        {
            const auto direction = planar_movement_yaw(
                {target.x - frame.origin_x_dc, target.y - frame.origin_y_e0, 0.0f});
            const auto delta = normalize(direction - frame.angle_b8);
            const auto correction =
                clamp_native(delta * 10.0f, negate(limits.maximum_angle), limits.maximum_angle);
            return correction + (base - limits.backward_angle);
        }
    case 5:
        return offset;
    default:
        return base;
    }
}
bool lag_step_pending(std::uint32_t flags, std::int32_t limit_fc,
                      std::int32_t current_15c) noexcept {
    if ((flags & 0x80u) && limit_fc)
        limit_fc = std::bit_cast<std::int32_t>(std::uint32_t(limit_fc) - 1u);
    return current_15c < limit_fc;
}
bool apply_target_yaw_base(std::int32_t yaw_base, const YawTarget& target, float origin_x,
                           float origin_y, const EntityLookup& entities, float& yaw) {
    if (yaw_base != 1 || target.handle == -1 || !entities.by_handle(entities.object, target.handle))
        return false;
    auto quantized = [](float value) {
        const auto scaled = _mm_div_ss(_mm_set_ss(value), _mm_set_ss(0.25f));
        const auto integer = _mm_cvtss_si32(scaled);
        return _mm_cvtss_f32(
            _mm_mul_ss(_mm_cvtsi32_ss(_mm_setzero_ps(), integer), _mm_set_ss(0.25f)));
    };
    const auto x = quantized(target.x) - quantized(origin_x);
    const auto y = quantized(target.y) - quantized(origin_y);
    yaw = planar_movement_yaw({x, y, 0.0f});
    return true;
}
} // namespace skeet::anti_aim
