// Movement Subsystem Implementation
#include "skeet/client/player_physics_hooks.hpp"
#include "player_physics_hooks_detail.hpp"
#include "movement_math.hpp"
#include <bit>
#include <immintrin.h>
#include <stdexcept>
#include "skeet/movement/acceleration.hpp"
#include "skeet/movement/air_feedback.hpp"
#include <limits>
#include "skeet/movement/air_input.hpp"
#include "skeet/movement/air_steering.hpp"
#include "skeet/movement/air_strafe.hpp"
#include "skeet/movement/air_velocity.hpp"
#include "skeet/movement/bunny_hop.hpp"
#include "skeet/movement/air_wall_steering.hpp"
#include "skeet/movement/steering_math.hpp"
#include "skeet/engine/interface_call.hpp"
#include <cmath>
#include <cstring>
#include "skeet/movement/blockbot.hpp"
#include "skeet/movement/blockbot_owner.hpp"
#include "skeet/hud/composite_handlers.hpp"
#include "skeet/misc/command_owner.hpp"
#include <array>
#include <new>
#include "skeet/movement/collision.hpp"
#include "skeet/movement/command_air.hpp"
#include "skeet/movement/command_bindings.hpp"
#include "skeet/movement/normalized_air_final.hpp"
#include "skeet/movement/normalized_air_wall.hpp"
#include "skeet/movement/normalized_scout_gate.hpp"
#include "skeet/movement/pre_speed_live.hpp"
#include "skeet/movement/raw_scout_gate.hpp"
#include "skeet/movement/z_hop.hpp"
#include "skeet/movement/command_runtime.hpp"
#include "skeet/movement/easy_strafe.hpp"
#include "skeet/movement/ground_phase_live.hpp"
#include "skeet/movement/player_air_gate.hpp"
#include "skeet/movement/weapon_speed_phase.hpp"
#include "skeet/movement/command_state.hpp"
#include "skeet/movement/command_wrapper.hpp"
#include "skeet/movement_config.hpp"
#include "skeet/movement/duck_peek.hpp"
#include "skeet/core.hpp"
#include "skeet/movement/engine_collision.hpp"
#include "skeet/engine/trace.hpp"
#include "skeet/movement/extrapolation.hpp"
#include "skeet/movement/extrapolation_command.hpp"
#include "skeet/animation/state_copy.hpp"
#include "skeet/lagcomp/records.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/movement/simulation_environment.hpp"
#include "skeet/movement/simulation_velocity.hpp"
#include <algorithm>
#include "skeet/movement/global_config.hpp"
#include "skeet/movement/ground_contact.hpp"
#include "skeet/movement/ground_phase.hpp"
#include "skeet/movement/speed_control_live.hpp"
#include "skeet/movement/ground_speed.hpp"
#include "skeet/movement/initialization.hpp"
#include "skeet/movement/normalized_air_solver.hpp"
#include "skeet/movement/pre_speed_controller.hpp"
#include "skeet/movement/owner_lifecycle.hpp"
#include "skeet/movement/pre_speed.hpp"
#include "skeet/movement/pre_speed_probe.hpp"
#include "skeet/movement/prediction.hpp"
#include "skeet/movement/quick_peek.hpp"
#include "skeet/movement/speed_control.hpp"
#include "skeet/esp/projection.hpp"
#include "skeet/render_queue.hpp"
#include "skeet/movement/quick_stop.hpp"
#include "skeet/movement/rage_stop.hpp"
#include "skeet/movement/raw_air_solver.hpp"
#include "skeet/movement/release_command.hpp"
#include "skeet/movement/simulation.hpp"
#include "skeet/movement/simulation_capture.hpp"
#include "skeet/movement/simulation_collision.hpp"
#include "skeet/movement/simulation_duck.hpp"
#include "skeet/movement/simulation_step.hpp"
#include "skeet/movement/simulation_tick.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/movement/slow_motion.hpp"


// ============================================================================
// Subcomponent: absolute_velocity.cpp
// ============================================================================

extern "C" {
const std::uint32_t* skeet_player_velocity_return_cell = nullptr;
}

namespace skeet::client {
namespace player_physics_detail {
void bind_velocity_continuation(const std::uint32_t& continuation) noexcept {
    skeet_player_velocity_return_cell = &continuation;
}
}
namespace {
using namespace player_physics_detail;

void clamp_absolute_velocity(std::array<float, 3>& velocity, const void* variable) {
    // Complete433E4973. ConVar parent traversal precedes all vector reads.
    for (;;) {
        const auto* parent = field<const void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    const auto bits = field<std::uint32_t>(variable, 0x2C) ^
                      std::uint32_t(reinterpret_cast<std::uintptr_t>(variable));
    const auto maximum = _mm_castsi128_ps(_mm_cvtsi32_si128(std::bit_cast<std::int32_t>(bits)));
    const auto minimum = _mm_xor_ps(
        maximum, _mm_castsi128_ps(_mm_cvtsi32_si128(std::bit_cast<std::int32_t>(0x80000000U))));
    for (std::size_t index = 0; index < 3; ++index) {
        auto* destination = reinterpret_cast<std::byte*>(velocity.data()) + index * 4;
        const auto value = _mm_castsi128_ps(
            _mm_cvtsi32_si128(std::bit_cast<std::int32_t>(field<std::uint32_t>(destination, 0))));
        if (_mm_movemask_ps(_mm_cmpunord_ss(value, value)) & 1)
            store(destination, 0, std::uint32_t(0));
        else if (_mm_comigt_ss(value, maximum))
            store(destination, 0, bits);
        else if (_mm_comigt_ss(minimum, value))
            store(destination, 0, bits ^ 0x80000000U);
    }
}

// Complete43326986 continuation. The caller's stack vector is loaded into
// the same XMM registers before jumping to the live engine continuation.
// The scratch EAX is restored, and ESP has its original value at the jump.
#if defined(__i386__) && (defined(__GNUC__) || defined(__clang__))
#define SKEET_VELOCITY_STRING_IMPL(value) #value
#define SKEET_VELOCITY_STRING(value) SKEET_VELOCITY_STRING_IMPL(value)
__attribute__((naked, noinline)) void absolute_velocity_return_gate() {
    __asm__ __volatile__("movss 0x28(%esp), %xmm1\n\t"
                         "movss 0x24(%esp), %xmm3\n\t"
                         "movss 0x20(%esp), %xmm4\n\t"
                         "movaps %xmm4, %xmm0\n\t"
                         "movaps %xmm3, %xmm2\n\t"
                         "mulss %xmm4, %xmm0\n\t"
                         "mulss %xmm3, %xmm2\n\t"
                         "addss %xmm0, %xmm2\n\t"
                         "movaps %xmm1, %xmm0\n\t"
                         "mulss %xmm1, %xmm0\n\t"
                         "addss %xmm0, %xmm2\n\t"
                         "pushl %eax\n\t"
                         "movl " SKEET_VELOCITY_STRING(
                             __USER_LABEL_PREFIX__) "skeet_player_velocity_return_cell, %eax\n\t"
                                                    "movl (%eax), %eax\n\t"
                                                    "xchgl %eax, (%esp)\n\t"
                                                    "retl");
}
#undef SKEET_VELOCITY_STRING
#undef SKEET_VELOCITY_STRING_IMPL
#elif defined(_M_IX86) && defined(_MSC_VER)
__declspec(naked) void absolute_velocity_return_gate() {
    __asm {
        movss xmm1,dword ptr [esp+28h]
        movss xmm3,dword ptr [esp+24h]
        movss xmm4,dword ptr [esp+20h]
        movaps xmm0,xmm4
        movaps xmm2,xmm3
        mulss xmm0,xmm4
        mulss xmm2,xmm3
        addss xmm2,xmm0
        movaps xmm0,xmm1
        mulss xmm0,xmm1
        addss xmm2,xmm0
        push eax
        mov eax,skeet_player_velocity_return_cell
        mov eax,dword ptr [eax]
        xchg eax,dword ptr [esp]
        ret
    }
}
#endif
}

void PlayerPhysicsHooks::estimate_absolute_velocity(void* player, std::array<float, 3>& output,
                                                    std::uintptr_t& caller_return) {
    using namespace player_physics_detail;
    if (!bindings.command_mode_43467224 && !bindings.alternate_mode_4346a740) {
        original<void>(player, 145, &output);
        return;
    }
    // The original copies the words sequentially, including output aliases.
    for (std::size_t index = 0; index < 3; ++index)
        store(output.data(), index * 4,
              field<std::uint32_t>(player, bindings.fields.absolute_velocity_36375c23 + index * 4));
    const auto* variable = reinterpret_cast<const void*>(
        std::uintptr_t(bindings.maximum_velocity_convar_43468c2c ^ 0x3FE3AE0CU));
    clamp_absolute_velocity(output, variable);
    if (bindings.velocity_return_site_43467f0c && bindings.velocity_continuation_434757c4 &&
        caller_return == bindings.velocity_return_site_43467f0c) {
#if defined(__i386__) || defined(_M_IX86)
        caller_return = reinterpret_cast<std::uintptr_t>(&absolute_velocity_return_gate);
#else
        throw std::logic_error("Player velocity return continuation requires the x86 hook ABI");
#endif
    }
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: acceleration.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;

// 0x4339E713..0x4339E79B, including the tail call to minimum_number.
float air_acceleration_delta(MovementVector velocity, MovementVector direction, float wish_speed,
                             float acceleration, float interval) noexcept {
    const float capped_speed = minimum_number(wish_speed, 30.0f);
    const float additional = capped_speed - dot(velocity, direction);
    if (0.0f >= additional)
        return 0.0f;
    return minimum_number(additional, (wish_speed * acceleration) * interval);
}

// 0x433AC9C0..0x433ACDBA. The two out-of-line blocks at 0x433ACD94/DAF
// retain the initial 250-unit acceleration basis under their respective gates.
GroundAcceleration ground_acceleration(const MovementPhysics& physics, const UserCommand& command,
                                       MovementVector velocity, MovementVector direction,
                                       float wish_speed) noexcept {
    GroundAcceleration result{0.0f, 250.0f, physics.accelerate};
    const float current = dot(velocity, direction);
    const float additional = wish_speed - current;
    if (0.0f >= additional)
        return result;
    const bool duck =
        (command.buttons & duck_button) || physics.ducking || (physics.player_flags & 2);
    const bool walk = (command.buttons & speed_button) && !duck;
    float acceleration_speed = 250.0f;
    bool exception = false;
    if (physics.weapon_speed_enabled && physics.has_weapon) {
        exception = weapon_slowdown_exception(physics);
        const float scale = physics.weapon_max_speed * 0.004f;
        if (1.0f >= scale)
            result.wish_speed *= scale;
        if ((!duck && !walk) || exception) {
            if (1.0f >= scale)
                acceleration_speed = scale * 250.0f;
        }
    }
    if (duck) {
        if (!exception)
            acceleration_speed *= 0.34f;
        result.wish_speed *= 0.34f;
    }
    if (walk) {
        if (!exception)
            acceleration_speed *= 0.52f;
        result.wish_speed *= 0.52f;
        result.acceleration = walking_acceleration(result.acceleration, current, result.wish_speed);
    }
    result.amount = (physics.interval_per_tick * result.acceleration) * acceleration_speed;
    if (result.amount > additional)
        result.amount = additional;
    result.wish_speed = acceleration_speed;
    return result;
}

// 0x433EE5FC..0x433EEA5E. Predicts the next duck state before speed scaling.
GroundAccelerationCapacity ground_acceleration_capacity(const MovementPhysics& physics,
                                                        const UserCommand& command,
                                                        MovementVector velocity,
                                                        MovementVector direction) noexcept {
    bool duck = (command.buttons & duck_button) != 0;
    if (!duck) {
        bool transitioning = physics.ducking || physics.duck_amount > 0.0f;
        auto flags = physics.player_flags;
        if (transitioning || physics.duck_amount >= 1.0f) {
            const float duck_speed = physics.duck_speed >= 1.5f ? physics.duck_speed : 2.0f;
            const float remaining =
                sse_max(0.0f, physics.duck_amount - duck_speed * physics.interval_per_tick);
            transitioning = remaining > 0.0f && transitioning;
            if (!(remaining > 0.75f))
                flags = 0xfffffffdu;
        }
        duck = transitioning || (flags & 2);
    }
    const bool walk = (command.buttons & speed_button) && !duck;
    float limit = 250.0f;
    float acceleration_speed = 250.0f;
    bool exception = false;
    if (physics.weapon_speed_enabled && physics.has_weapon) {
        exception = weapon_slowdown_exception(physics);
        const float raw_scale = physics.weapon_max_speed * 0.004f;
        const float scale = 1.0f >= raw_scale ? raw_scale : 1.0f;
        float basis = 250.0f;
        if ((!duck && !walk) || exception) {
            acceleration_speed = scale * 250.0f;
            if (1.0f >= scale)
                basis = acceleration_speed;
        }
        limit = scale * basis;
    }
    if (duck) {
        if (!exception)
            acceleration_speed *= 0.34f;
        limit *= 0.34f;
    }
    float acceleration = physics.accelerate;
    if (walk) {
        if (!exception)
            acceleration_speed *= 0.52f;
        limit *= 0.52f;
        acceleration = walking_acceleration(acceleration, dot(velocity, direction), limit);
    }
    return {(acceleration * physics.interval_per_tick) * acceleration_speed, duck};
}

} // namespace skeet

// ============================================================================
// Subcomponent: air_feedback.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;

void apply_guarded_pre_speed_heading(float speed, float velocity_yaw, float previous_heading,
                                     float maximum_angle, MovementAngleView angles) noexcept {
    if (!(speed > 80.0f))
        return;
    if (previous_heading != std::numeric_limits<float>::max())
        angles.yaw = previous_heading;
    const float difference = velocity_yaw - angles.yaw;
    if (std::isnan(difference) || absolute(normalize_movement_angle(difference)) < 5.0f)
        angles.yaw += maximum_angle;
}

bool resolve_pre_speed_feedback(float previous_heading, float ideal_step, float turn_limit,
                                float side_magnitude, MovementFeedback& feedback,
                                UserCommand& command, ConstMovementAngleView angles) noexcept {
    if (previous_heading == std::numeric_limits<float>::max())
        return false;
    const float difference = normalize_movement_angle(angles.yaw - previous_heading);
    const float magnitude = absolute(difference);
    if (!(magnitude > ideal_step && magnitude <= turn_limit))
        return false;
    feedback.primary_heading = normalize_movement_angle(angles.yaw);
    command.side_move = difference > 0.0f ? negate(side_magnitude) : side_magnitude;
    return true;
}

} // namespace skeet

// ============================================================================
// Subcomponent: air_input.cpp
// ============================================================================

namespace skeet {

bool prepare_air_strafe_heading(const AirStrafeSelection& selection, MovementVector velocity,
                                bool normalized_basis, UserCommand& command,
                                MovementAngleView movement_angles) noexcept {
    if (!selection.continue_solver)
        return false;
    const float forward = command.forward_move;
    const float side = command.side_move;
    command.forward_move = 0.0f;
    const float speed = movement_detail::horizontal_length(velocity);
    if (speed == 0.0f && movement_detail::absolute(velocity.z) == 0.0f)
        return false;
    const bool movement_keys = (selection.movement_key_mode & 0xffu) != 0;
    if (selection.view_angles && !movement_keys && !selection.pre_speed_requested &&
        ((command.buttons & back_button) || side != 0.0f))
        return false;
    if (normalized_basis)
        command.side_move = 0.0f;
    if (movement_keys) {
        if (normalized_basis)
            project_movement_heading(forward, side, movement_angles);
        else
            project_raw_movement_heading(forward, side, movement_angles);
    }
    return true;
}

} // namespace skeet

// ============================================================================
// Subcomponent: air_steering.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;

void write_air_strafe_step(const AirSteeringInput& input, AirSteeringState& state,
                           UserCommand& command, MovementAngleView movement_angles) noexcept {
    state.feedback_heading = normalize_movement_angle(movement_angles.yaw);
    const float delta = normalize_movement_angle(movement_angles.yaw - input.velocity_yaw);
    const auto smoothing =
        std::bit_cast<std::int32_t>(100u - static_cast<std::uint32_t>(input.smoothing));
    float correction = static_cast<float>(smoothing) * 0.02f;
    correction *= input.ideal_step + input.ideal_step;
    if ((input.large_turn && absolute(delta) > 170.0f && input.speed > 80.0f) ||
        (delta > correction && input.speed > 80.0f)) {
        movement_angles.yaw = input.velocity_yaw + correction;
        command.side_move = negate(input.side_magnitude);
    } else if (negate(correction) > delta && input.speed > 80.0f) {
        movement_angles.yaw = input.velocity_yaw - correction;
        command.side_move = input.side_magnitude;
    } else if (state.side_flip) {
        movement_angles.yaw -= input.ideal_step;
        command.side_move = negate(input.side_magnitude);
    } else {
        movement_angles.yaw += input.ideal_step;
        command.side_move = input.side_magnitude;
    }
    state.side_flip = !state.side_flip;
    normalize_movement_angles(movement_angles);
}

} // namespace skeet

// ============================================================================
// Subcomponent: air_strafe.cpp
// ============================================================================

namespace skeet {

AirStrafeSelection select_air_strafe_modes(const AirStrafeSettings& settings, InputBitmap input,
                                           bool pre_speed_guard_enabled,
                                           const UserCommand& command) {
    const auto direction = static_cast<std::uint32_t>(settings.direction);
    AirStrafeSelection result{true, (direction & 1u) != 0, (direction >> 1) & 0xffffff01u, false,
                              false};
    result.pre_speed_requested =
        settings.pre_speed && hotkey_active(settings.pre_speed_hotkey, input);
    result.pre_speed_effective = result.pre_speed_requested && pre_speed_guard_enabled;
    const bool no_input = command.side_move == 0.0f && command.forward_move == 0.0f;
    if ((result.movement_key_mode & 0xffu) && !result.view_angles && !result.pre_speed_requested &&
        no_input) {
        result.continue_solver = false;
        return result;
    }
    if (no_input)
        result.movement_key_mode &= 0xffffff00u;
    return result;
}

} // namespace skeet

// ============================================================================
// Subcomponent: air_velocity.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;

AirVelocityPreparation
prepare_air_strafe_velocity(MovementVector velocity, float stamina, const MovementPhysics& physics,
                            const MovementSimulationRules& rules, bool movement_keys,
                            bool bunny_hop_enabled, bool jump_speed_cap_enabled,
                            float side_magnitude, UserCommand& command) noexcept {
    velocity.z -= (rules.gravity * 0.5f) * rules.interval_per_tick;
    if (!(physics.player_flags & on_ground))
        return {velocity, stamina, true};
    if (!(command.buttons & jump_button))
        return {velocity, stamina, false};
    const float squared = velocity.y * velocity.y + velocity.x * velocity.x;
    if (4.0f > squared) {
        recover_bunny_hop_low_speed(bunny_hop_enabled, {1.0f, false, false}, velocity, true,
                                    movement_keys, command, side_magnitude);
        return {velocity, stamina, false};
    }
    if (stamina > 0.0f)
        stamina = sse_max(0.0f, stamina - rules.stamina_recovery * rules.interval_per_tick);
    if (jump_speed_cap_enabled) {
        const float limit = physics.player_speed_limit * 1.1f;
        const float speed = std::sqrt((velocity.x * velocity.x + velocity.y * velocity.y) +
                                      velocity.z * velocity.z);
        if (speed > limit) {
            const float scale = limit / speed;
            velocity.z *= scale;
            velocity.x *= scale;
            velocity.y *= scale;
        }
    }
    if (physics.ducking || (physics.player_flags & 2u))
        velocity.z = rules.jump_impulse;
    else
        velocity.z += rules.jump_impulse;
    if (stamina > 0.0f) {
        const float scale = sse_min(sse_max(1.0f - stamina / 100.0f, 0.0f), 1.0f);
        velocity.z *= scale;
    }
    velocity.z -= (rules.gravity * 0.5f) * rules.interval_per_tick;
    return {velocity, stamina, true};
}

} // namespace skeet

// ============================================================================
// Subcomponent: air_wall_steering.cpp
// ============================================================================

namespace skeet {
namespace {
template <class T> T field(const void* object, std::uint32_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}

std::int32_t truncate_tick_count(float value) noexcept {
    // CVTTSS2SI returns the integer-indefinite value outside the signed range.
    if (!(value >= -2147483648.0f && value < 2147483648.0f))
        return std::numeric_limits<std::int32_t>::min();
    return static_cast<std::int32_t>(value);
}

MovementCollisionTrace initial_trace() noexcept {
    MovementCollisionTrace trace{};
    trace.fraction = 1.0f;
    trace.hitgroup = -1;
    trace.hitbox = -1;
    return trace;
}

bool surface_blocks_steering(void* surface_properties, std::int16_t property) {
    const auto* surface =
        engine::invoke<const std::uint8_t*>(surface_properties, 5, std::int32_t(property));
    return surface && surface[0x62] != 0;
}

void write_wall_heading(MovementVector direction, MovementVector forward,
                        const MovementCollisionTrace& trace,
                        MovementAngleView movement_angles) noexcept {
    using namespace movement_detail;
    const MovementVector basis =
        !(dot(direction, trace.plane_normal) >= -0.8660252690315247f) ? forward : direction;
    MovementVector tangent;
    if (trace.plane_normal.x == 0.0f && trace.plane_normal.y == 0.0f) {
        tangent = {0.0f, -1.0f, 0.0f};
    } else {
        tangent = {trace.plane_normal.y - trace.plane_normal.z * 0.0f,
                   trace.plane_normal.z * 0.0f - trace.plane_normal.x,
                   trace.plane_normal.x * 0.0f - trace.plane_normal.y * 0.0f};
        const float reciprocal =
            1.0f /
            (std::sqrt((tangent.x * tangent.x + tangent.y * tangent.y) + tangent.z * tangent.z) +
             epsilon);
        tangent = {tangent.x * reciprocal, tangent.y * reciprocal, tangent.z * reciprocal};
    }
    if (!(dot(tangent, basis) >= 0.0f)) {
        tangent.x = negate(tangent.x);
        tangent.y = negate(tangent.y);
    }
    const float weight = 1.0f - trace.fraction;
    const MovementVector blended{(tangent.x - forward.x) * weight + forward.x,
                                 (tangent.y - forward.y) * weight + forward.y, 0.0f};
    movement_angles.yaw = planar_movement_yaw(blended);
}
}

bool steer_air_wall(const AirWallSteeringInput& input, const MovementFeedback& feedback,
                    const client::Command& command, MovementAngleView movement_angles,
                    const AirWallSteeringBindings& bindings) {
    using namespace movement_detail;
    if (feedback.prediction_blocked || !input.enabled ||
        (feedback.original_buttons & 0x20026u) != 2u)
        return false;

    const float squared =
        (input.velocity.y * input.velocity.y + 0.0f) + input.velocity.x * input.velocity.x;
    const float inverse = 1.0f / (std::sqrt(squared) + epsilon);
    const MovementVector direction{input.velocity.x * inverse, input.velocity.y * inverse,
                                   0.0f * inverse};
    const float radians = movement_angles.yaw * degrees_to_radians;
    const MovementVector forward{std::cos(radians), std::sin(radians), 0.0f};
    // COMISS/JB rejects both a negative dot and an unordered comparison.
    if (!(dot(forward, direction) >= 0.0f))
        return false;

    const float seconds = static_cast<float>(input.prediction_milliseconds) * 0.001f;
    const auto ticks = truncate_tick_count(seconds / bindings.rules.interval_per_tick + 0.5f);
    MovementSimulation simulation{};
    initialize_movement_simulation(simulation, input.player, input.position, input.velocity,
                                   input.player_flags, 0, bindings.snapshot, bindings.rules);
    auto simulated_command = command;
    simulated_command.angles = {movement_angles.pitch, movement_angles.yaw, movement_angles.roll};
    const float turn = minimum_number(
        input.maximum_angle, input.angle_scale / (horizontal_length(input.velocity) + epsilon));
    if (input.weapon_handle != -1)
        engine::invoke<std::uint32_t>(bindings.entity_list, 4, std::uint32_t(input.weapon_handle));
    bool side_flip = feedback.side_flip != 0;
    for (std::int32_t tick = 0; tick < ticks; ++tick) {
        simulated_command.angles[1] =
            side_flip ? movement_angles.yaw - turn : movement_angles.yaw + turn;
        simulated_command.movement.side_move =
            side_flip ? negate(input.side_magnitude) : input.side_magnitude;
        side_flip = !side_flip;
        simulate_movement_tick(simulation, bindings.collision, bindings.environment, input.player,
                               simulated_command.movement,
                               {simulated_command.angles[0], simulated_command.angles[1],
                                simulated_command.angles[2]});
        if (simulation.simulation_flags & 2u)
            break;
        simulated_command.number =
            std::bit_cast<std::int32_t>(std::uint32_t(simulated_command.number) + 1u);
    }
    if (!(simulation.simulation_flags & 2u))
        return true;

    MovementTraceFlagGuard trace_flag(bindings.environment.engine_trace_flag);
    auto trace = initial_trace();
    const MovementVector start{input.position.x, input.position.y, input.position.z + 18.0f};
    const MovementVector extended{forward.x * 320.0f, forward.y * 320.0f, forward.z * 320.0f};
    const MovementVector maximum{bindings.snapshot.hull_maximum.x, bindings.snapshot.hull_maximum.y,
                                 24.0f};
    const MovementCollisionFilter filter{MovementCollisionFilterKind::reject_entities, 0, 0, 0};
    auto ray = make_movement_hull_ray(
        start,
        {extended.x * 0.5f + start.x, extended.y * 0.5f + start.y, extended.z * 0.5f + start.z},
        bindings.snapshot.hull_minimum, maximum);
    bindings.collision.trace_ray(ray, 0x200400bu, filter, trace);
    if (trace.plane_normal.z != 1.0f &&
        ((trace.contents & 0x20000000u) ||
         surface_blocks_steering(bindings.surface_properties, trace.surface_property)))
        return true;

    ray = make_movement_hull_ray(start,
                                 {extended.x * seconds + start.x, extended.y * seconds + start.y,
                                  extended.z * seconds + start.z},
                                 bindings.snapshot.hull_minimum, maximum);
    bindings.collision.trace_ray(ray, bindings.snapshot.collision_mask & 0xfffeffffu, filter,
                                 trace);
    if (!(trace.fraction < 1.0f || trace.all_solid || trace.start_solid) ||
        !(absolute(trace.plane_normal.z) < 0.1f) ||
        ballistic_breakable(bindings.ballistics, trace.entity))
        return true;

    write_wall_heading(direction, forward, trace, movement_angles);
    return true;
}

bool steer_live_air_wall(const AirWallSteeringInput& input, const MovementFeedback& feedback,
                         const client::Command& command, MovementAngleView movement_angles,
                         const AirWallSteeringLiveBindings& bindings) {
    using namespace movement_detail;
    if (feedback.prediction_blocked || !input.enabled ||
        (feedback.original_buttons & 0x20026u) != 2u)
        return false;

    const float squared =
        (input.velocity.y * input.velocity.y + 0.0f) + input.velocity.x * input.velocity.x;
    const float inverse = 1.0f / (std::sqrt(squared) + epsilon);
    const MovementVector direction{input.velocity.x * inverse, input.velocity.y * inverse,
                                   0.0f * inverse};
    const float radians = movement_angles.yaw * degrees_to_radians;
    const MovementVector forward{std::cos(radians), std::sin(radians), 0.0f};
    if (!(dot(forward, direction) >= 0.0f))
        return false;

    // Native 43379DB8 and 43379DFA precede the constructor's own hull reads.
    // The minimum is retained as a pointer and dereferenced at each trace;
    // mutations made by subsequent engine getters/traces remain observable.
    const auto* minimum = engine::invoke<const MovementVector*>(bindings.player, 0x130u / 4u);
    const MovementVector trace_position = bindings.origin;
    const MovementVector start{trace_position.x, trace_position.y, trace_position.z + 18.0f};
    const MovementVector maximum_snapshot =
        *engine::invoke<const MovementVector*>(bindings.player, 0x134u / 4u);
    const MovementVector maximum{maximum_snapshot.x, maximum_snapshot.y, 24.0f};
    const MovementVector extended{forward.x * 320.0f, forward.y * 320.0f, forward.z * 320.0f};
    const float seconds = static_cast<float>(bindings.tuning.prediction_milliseconds) * 0.001f;
    const auto flags =
        field<std::uint32_t>(bindings.player, bindings.environment.fields.player_flags_e2fbe695);
    const MovementVector constructor_velocity = bindings.velocity;
    const MovementVector constructor_position = bindings.origin;
    MovementSimulation simulation{};
    initialize_live_movement_simulation(simulation, bindings.player, constructor_position,
                                        constructor_velocity, flags, 0, bindings.capture);
    const auto ticks = truncate_tick_count(seconds / (*bindings.environment.interval) + 0.5f);
    auto simulated_command = command;
    simulated_command.angles = {movement_angles.pitch, movement_angles.yaw, movement_angles.roll};
    const float turn =
        minimum_number(bindings.tuning.maximum_angle,
                       bindings.tuning.angle_scale / (horizontal_length(input.velocity) + epsilon));
    const auto weapon_handle =
        field<std::int32_t>(bindings.player, bindings.environment.fields.weapon_handle_cd812654);
    if (weapon_handle != -1)
        engine::invoke<std::uint32_t>(bindings.environment.entity_list, 4,
                                      std::uint32_t(weapon_handle));
    bool side_flip = feedback.side_flip != 0;
    for (std::int32_t tick = 0; tick < ticks; ++tick) {
        simulated_command.angles[1] =
            side_flip ? movement_angles.yaw - turn : movement_angles.yaw + turn;
        simulated_command.movement.side_move =
            side_flip ? negate(bindings.tuning.side_magnitude) : bindings.tuning.side_magnitude;
        side_flip = !side_flip;
        simulate_live_movement_tick(simulation, bindings.collision, bindings.environment,
                                    bindings.player, simulated_command);
        if (simulation.simulation_flags & 2u)
            break;
        simulated_command.number =
            std::bit_cast<std::int32_t>(std::uint32_t(simulated_command.number) + 1u);
    }
    if (!(simulation.simulation_flags & 2u))
        return true;

    MovementTraceFlagGuard trace_flag(bindings.environment.engine_trace_flag);
    auto trace = initial_trace();
    const MovementCollisionFilter filter{MovementCollisionFilterKind::reject_entities, 0, 0, 0};
    bindings.collision.trace_ray(
        make_movement_hull_ray(
            start,
            {extended.x * 0.5f + start.x, extended.y * 0.5f + start.y, extended.z * 0.5f + start.z},
            *minimum, maximum),
        0x200400bu, filter, trace);
    if (trace.plane_normal.z != 1.0f &&
        ((trace.contents & 0x20000000u) ||
         surface_blocks_steering(bindings.surface_properties, trace.surface_property)))
        return true;

    const auto mask = engine::invoke<std::uint32_t>(bindings.player, 0x260u / 4u) & 0xfffeffffu;
    bindings.collision.trace_ray(
        make_movement_hull_ray(start,
                               {extended.x * seconds + start.x, extended.y * seconds + start.y,
                                extended.z * seconds + start.z},
                               *minimum, maximum),
        mask, filter, trace);
    if (!(trace.fraction < 1.0f || trace.all_solid || trace.start_solid) ||
        !(absolute(trace.plane_normal.z) < 0.1f) ||
        ballistic_breakable(bindings.ballistics, trace.entity))
        return true;

    write_wall_heading(direction, forward, trace, movement_angles);
    return true;
}

} // namespace skeet

// ============================================================================
// Subcomponent: blockbot.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;
template <class T> T field(const void* object, std::ptrdiff_t offset) {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
MovementVector origin(void* player) {
    return *engine::invoke<const MovementVector*>(player, 10);
}
MovementVector center(void* player) { // complete433FB98A
    auto result = origin(player);
    const auto* minimum = engine::invoke<const MovementVector*>(player, 76);
    const auto* maximum = engine::invoke<const MovementVector*>(player, 77);
    result.z += (maximum->z + minimum->z) * 0.5f;
    return result;
}
float target_score(void* player, std::int32_t team, MovementVector eye, MovementVector forward,
                   const BlockbotOffsets& offsets) { // complete433EA084
    constexpr auto rejected = std::numeric_limits<float>::max();
    if (!player || !engine::invoke<bool>(player, 158) ||
        field<std::uint8_t>(player, offsets.rejected_9d920e35) ||
        engine::invoke<std::int32_t>(player, 122) <= 0 ||
        engine::invoke<bool>(static_cast<std::byte*>(player) + 8, 9) ||
        field<std::uint8_t>(player, offsets.rejected_cde290d2) ||
        field<std::uint8_t>(player, offsets.rejected_d070684c) ||
        field<std::int32_t>(player, offsets.team) != team)
        return rejected;
    const auto point = center(player);
    const auto along = ((point.y - eye.y) * forward.y + (point.x - eye.x) * forward.x) +
                       (point.z - eye.z) * forward.z;
    if (along < 0.0f || along > 400.0f)
        return rejected;
    const auto dx = point.x - (forward.x * along + eye.x);
    const auto dy = point.y - (forward.y * along + eye.y);
    const auto dz = point.z - (forward.z * along + eye.z);
    return std::sqrt((dx * dx + dy * dy) + dz * dz);
}
struct PositionSample {
    float time;
    MovementVector position;
};
bool history_velocity(void* player, std::ptrdiff_t offset, MovementVector& newest,
                      MovementVector& older) { // complete valid-player433B171A
    const auto* history = static_cast<const std::byte*>(player) + offset;
    if (field<std::uint16_t>(history, 0x10) < 3)
        return false;
    const auto first = field<std::uint16_t>(history, 0xC);
    const auto head = field<std::uint16_t>(history, 0xE);
    const auto* samples = field<const PositionSample*>(history, 8);
    const auto index = [first](std::uint32_t sequence) {
        return sequence >= first ? sequence - first : sequence;
    };
    const auto& a = samples[index(head)];
    const auto& b = samples[index(std::uint32_t(head) + 1u)];
    const auto& c = samples[index(std::uint32_t(head) + 2u)];
    const auto dt = a.time - b.time;
    if (!(dt > 0.0f))
        return false;
    const auto inverse = 1.0f / dt;
    newest = {(a.position.x - b.position.x) * inverse, (a.position.y - b.position.y) * inverse,
              (a.position.z - b.position.z) * inverse};
    const auto previous_dt = b.time - c.time;
    if (!(previous_dt > 0.0f))
        return false;
    const auto previous_inverse = 1.0f / previous_dt;
    older = {(b.position.x - c.position.x) * previous_inverse,
             (b.position.y - c.position.y) * previous_inverse,
             (b.position.z - c.position.z) * previous_inverse};
    return true;
}
MovementAngles perpendicular_angles(MovementVector direction) {
    // Cross with (0,0,1), retaining native signed-zero products.
    const auto x = direction.y - direction.z * 0.0f;
    const auto y = direction.z * 0.0f - direction.x;
    const auto z = direction.x * 0.0f - direction.y * 0.0f;
    if (y == 0.0f && x == 0.0f)
        return {z > 0.0f ? 270.0f : 90.0f, 0.0f, 0.0f};
    const auto yaw =
        static_cast<float>(std::atan2(static_cast<double>(y), static_cast<double>(x))) *
        radians_to_degrees;
    const auto planar = std::sqrt(x * x + y * y);
    const auto pitch = static_cast<float>(std::atan2(static_cast<double>(negate(z)),
                                                     static_cast<double>(planar))) *
                       radians_to_degrees;
    return {pitch, yaw, 0.0f};
}
}
void initialize_blockbot_config(ConfigRegistries& registries, BlockbotConfig& config) noexcept {
    initialize_boolean_config(registries.descriptors, config.enabled, 0xB2CDBB7Du);
    initialize_hotkey_config(registries, config.hotkey, 0xE2FFCCF0u, 0x1Bu, HotkeyMode::hold);
}
void initialize_blockbot_state(BlockbotState& state) noexcept {
    state.target_handle = -1;
    state.flags &= 0xFEu;
    state.locked_right.x =
        std::bit_cast<float>(std::bit_cast<std::uint32_t>(state.locked_right.x) & 0xFFFFFF00u);
    state.activated = 0;
}
void process_blockbot(BlockbotState& state, bool enabled, const HotkeyBinding& hotkey,
                      InputBitmap input, void* player, client::Command& command,
                      MovementAngleView movement_angles, const BlockbotBindings& bindings) {
    state.flags &= 0xFEu;
    const auto& offsets = bindings.offsets;
    const auto deactivate = [&state] { state.activated = 0; };
    if (!enabled || !hotkey_active(hotkey, input)) {
        deactivate();
        return;
    }
    if (field<std::uint8_t>(player, offsets.move_type) != 2 ||
        !(field<std::uint8_t>(player, offsets.ground_flags) & 1u)) {
        deactivate();
        return;
    }
    const auto& move = command.movement;
    if (move.forward_move * move.forward_move + move.side_move * move.side_move != 0.0f ||
        (bindings.game_rules && *bindings.game_rules &&
         field<std::uint8_t>(*bindings.game_rules, offsets.rule_d720aea6))) {
        deactivate();
        return;
    }

    MovementAngles view;
    engine::invoke<void>(bindings.engine_client, 18, &view);
    MovementVector forward;
    movement_angle_vectors({view.pitch, view.yaw, 0.0f}, &forward, nullptr, nullptr);
    const auto team = field<std::int32_t>(player, offsets.team);
    const auto absolute = origin(player);
    const auto eye_offset = field<MovementVector>(player, offsets.view_offset);
    const MovementVector eye{absolute.x + eye_offset.x, absolute.y + eye_offset.y,
                             absolute.z + eye_offset.z};
    void* target = nullptr;
    if (state.target_handle != -1) {
        target = engine::invoke<void*>(bindings.entity_list, 4, state.target_handle);
        if (target && !(target_score(target, team, eye, forward, offsets) <
                        std::numeric_limits<float>::max())) {
            state.target_handle = -1;
            target = nullptr;
        }
    }
    if (!target) {
        auto best = std::numeric_limits<float>::max();
        const auto count = *bindings.max_clients;
        for (std::int32_t i = 1; i <= count; ++i) {
            auto* candidate = engine::invoke<void*>(bindings.entity_list, 3, i);
            if (!candidate)
                continue;
            const auto score = target_score(candidate, team, eye, forward, offsets);
            if (best > score) {
                best = score;
                target = candidate;
                state.target_handle = std::bit_cast<std::int32_t>(
                    *engine::invoke<const std::uint32_t*>(candidate, 2));
            }
        }
    }
    if (!target)
        return;
    if (!state.activated) {
        movement_angle_vectors(view, nullptr, &state.locked_right, nullptr);
        state.activated = 1;
    }
    MovementVector velocity, older;
    if (!history_velocity(target, offsets.position_history, velocity, older))
        return;
    velocity.z = older.z = 0.0f;
    const MovementVector acceleration{velocity.x - older.x, velocity.y - older.y, 0.0f};
    auto position = field<MovementVector>(target, offsets.network_origin);
    const auto ticks = lagcomp::mean_history_timing_ticks(*bindings.timing);
    if (ticks < 100) {
        MovementTraceFlagGuard guard(bindings.trace_flag);
        for (std::int32_t tick = 0; tick < ticks; ++tick) {
            velocity = {velocity.x + acceleration.x, velocity.y + acceleration.y,
                        velocity.z + acceleration.z};
            const auto speed = std::sqrt(velocity.x * velocity.x + velocity.y * velocity.y);
            const auto maximum_speed = field<float>(target, offsets.maximum_speed);
            if (speed > maximum_speed) {
                const auto factor = maximum_speed / speed;
                velocity = {velocity.x * factor, velocity.y * factor, velocity.z * factor};
            }
            const auto interval = *bindings.interval;
            const auto* maximum = engine::invoke<const MovementVector*>(target, 77);
            const auto* minimum = engine::invoke<const MovementVector*>(target, 76);
            const MovementVector destination{velocity.x * interval + position.x,
                                             velocity.y * interval + position.y,
                                             velocity.z * interval + position.z};
            const auto ray = make_movement_hull_ray(position, destination, *minimum, *maximum);
            MovementCollisionTrace trace{};
            trace.fraction = 1.0f;
            trace.hitgroup = -1;
            trace.hitbox = -1;
            bindings.collision->trace_ray(
                ray, 1,
                {MovementCollisionFilterKind::skip_player,
                 static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(target)), 0, 0},
                trace);
            position = trace.end;
            if (trace.fraction != 1.0f)
                break;
        }
    }
    state.flags |= 1u;
    const auto local_origin = origin(player);
    const auto dx = position.x - local_origin.x, dy = position.y - local_origin.y;
    const auto inverse = 1.0f / (std::sqrt((dx * dx + dy * dy) + 0.0f * 0.0f) + epsilon);
    const auto dot =
        ((inverse * dx) * state.locked_right.x + (inverse * dy) * state.locked_right.y) +
        (inverse * 0.0f) * state.locked_right.z;
    if (std::fabs(dot) < 0.001f)
        return;
    auto side = (dot * -450.0f) * 4.0f;
    if (side > 450.0f)
        side = 450.0f;
    else if (-450.0f > side)
        side = -450.0f;
    command.movement.side_move = side;
    movement_angles = perpendicular_angles(state.locked_right);
}
} // namespace skeet

// ============================================================================
// Subcomponent: blockbot_owner.cpp
// ============================================================================

namespace skeet {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define BLOCK_MEMBER __thiscall
#define BLOCK_FAST __fastcall
#else
#define BLOCK_MEMBER
#define BLOCK_FAST
#endif
using OwnerEntry = void(BLOCK_MEMBER*)(void*);
void BLOCK_MEMBER destroy(void*);
void BLOCK_MEMBER unchanged(void*) {} //433CCE8E
void BLOCK_MEMBER reset(void*);
void BLOCK_MEMBER forward_reset(void*);
struct HudEndpoint : hud::HudFrameCallback {
    hud::BlockbotHudBinding binding;
};
void BLOCK_FAST draw(void* endpoint, ui::RenderQueue* queue, void* player, std::int32_t* cursor) {
    static_cast<HudEndpoint*>(static_cast<hud::HudFrameCallback*>(endpoint))
        ->binding.draw(*queue, player, *cursor);
}
const std::array<hud::HudFrameEntry, 1> hud_table{draw}; //43472498
const std::array<OwnerEntry, 9> lifecycle_table{//434728AC
                                                destroy,   unchanged,     unchanged,
                                                unchanged, unchanged,     reset,
                                                unchanged, forward_reset, unchanged};
}
class BlockbotOwner {
public:
    const OwnerEntry* vtable = nullptr;
    engine::JobMemory memory;
    BlockbotState& state;
    misc::CommandOwner command;
    client::NativeCommandPhaseReceiver command_receiver;
    HudEndpoint hud_endpoint;
    BlockbotOwner(engine::OwnerRegistryBinding lifecycle, client::CommandPhaseRegistry& commands,
                  engine::OwnerRegistryBinding callbacks, ConfigRegistries& registries,
                  BlockbotConfig& settings, BlockbotState& shared_state,
                  const BlockbotBindings& bindings, const esp::Projection& projection,
                  const std::uint32_t& life_state_offset)
        : memory(lifecycle.memory), state(shared_state),
          command(misc::BlockbotCommandOwner{state, settings.enabled, settings.hotkey, bindings}),
          hud_endpoint{{nullptr}, {state, projection, life_state_offset}} {
        lifecycle.append(this);
        commands.append(command_receiver, &command);
        hud::register_hud_frame(callbacks, &hud_endpoint);
        vtable = lifecycle_table.data();
        commands.publish(command_receiver, client::CommandOwnerId::blockbot); //43480289
        hud_endpoint.vtable = hud_table.data();
        initialize_blockbot_config(registries, settings);
        initialize_blockbot_state(state);
    }
};
namespace {
void BLOCK_MEMBER destroy(void* object) { //433B1A7A tail433837D0
    if (!object)
        return;
    auto* owner = static_cast<BlockbotOwner*>(object);
    const auto memory = owner->memory;
    owner->~BlockbotOwner();
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void BLOCK_MEMBER reset(void* object) { //4332057F, preserves direction and upper flag bits
    auto& state = static_cast<BlockbotOwner*>(object)->state;
    if (state.activated)
        state.activated = 0;
    state.target_handle = -1;
    state.flags &= 0xFEu;
}
void BLOCK_MEMBER forward_reset(void* object) { //433CF2F5
    static_cast<BlockbotOwner*>(object)->vtable[5](object);
}
#undef BLOCK_MEMBER
#undef BLOCK_FAST
}
std::size_t blockbot_owner_size() noexcept {
    return sizeof(BlockbotOwner);
}
BlockbotOwner& construct_blockbot_owner(void* storage, engine::OwnerRegistryBinding lifecycle,
                                        client::CommandPhaseRegistry& commands,
                                        engine::OwnerRegistryBinding callbacks,
                                        ConfigRegistries& registries, BlockbotConfig& settings,
                                        BlockbotState& state, const BlockbotBindings& bindings,
                                        const esp::Projection& projection,
                                        const std::uint32_t& life_state_offset) {
    return *::new (storage) BlockbotOwner(lifecycle, commands, callbacks, registries, settings,
                                          state, bindings, projection, life_state_offset);
}
} //namespace skeet

// ============================================================================
// Subcomponent: bunny_hop.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;
bool BunnyHopGuard::releases_jump() const noexcept {
    if (const auto* semantic = std::get_if<const BunnyHopGuardNode*>(&source_)) {
        auto* node = *semantic;
        while (node->next != node)
            node = node->next;
        return node->release_marker == node;
    }
    const void* node = std::get<engine::NativeConVarReference>(source_).get();
    for (;;) {
        const void* parent;
        std::memcpy(&parent, static_cast<const std::byte*>(node) + 0x1C, sizeof(parent));
        if (parent == node)
            break;
        node = parent;
    }
    std::uint32_t encoded;
    std::memcpy(&encoded, static_cast<const std::byte*>(node) + 0x30, sizeof(encoded));
    return encoded == std::uint32_t(reinterpret_cast<std::uintptr_t>(node));
}
void suppress_airborne_jump(bool enabled, std::uint32_t flags, const BunnyHopGuard& guard,
                            UserCommand& command) noexcept {
    if (enabled && !(flags & on_ground) && guard.releases_jump())
        command.buttons &= ~jump_button;
}

// 0x43372CCB ground test, then 0x43372D3A..0x43372D68.
// The grounded edge at 0x43372CE3 belongs to a separate weapon/stop path.
void suppress_airborne_jump(bool enabled, std::uint32_t player_flags,
                            const BunnyHopGuardNode& guard, UserCommand& command) noexcept {
    if (!enabled || (player_flags & on_ground))
        return;
    const BunnyHopGuardNode* node = &guard;
    while (node->next != node)
        node = node->next;
    if (node->release_marker == node)
        command.buttons &= ~jump_button;
}

// Consumer 0x4337392E..0x43373A61. The intervening unrelated vertical-velocity
// update is performed by the shared solver, not by this command mutation.
void recover_bunny_hop_low_speed(bool enabled, const BunnyHopTraceResult& trace,
                                 MovementVector velocity, bool required_state, bool blocked,
                                 UserCommand& command, float forced_move) noexcept {
    if (trace.fraction < 1.0f || trace.flag_a || trace.flag_b || !required_state ||
        !(command.buttons & jump_button))
        return;
    const float squared_speed = velocity.y * velocity.y + velocity.x * velocity.x;
    if (!(4.0f > squared_speed) || blocked || !enabled)
        return;
    command.forward_move = forced_move;
}

// Consumer 0x433794E2..0x43379546, after the shared trace has completed.
void recover_bunny_hop_trace(bool enabled, const BunnyHopTraceResult& trace, bool blocked,
                             UserCommand& command, float forced_move) noexcept {
    if (trace.fraction < 1.0f || trace.flag_a || trace.flag_b || blocked || !enabled)
        return;
    command.forward_move = forced_move;
}

} // namespace skeet

// ============================================================================
// Subcomponent: collision.cpp
// ============================================================================

namespace skeet {

MovementTraceFlagGuard::MovementTraceFlagGuard(std::uint8_t* flag) noexcept : flag_(flag) {
    if (flag_) {
        saved_ = *flag_;
        *flag_ = 0;
    }
}

MovementTraceFlagGuard::~MovementTraceFlagGuard() {
    restore();
}

void MovementTraceFlagGuard::restore() noexcept {
    if (flag_) {
        *flag_ = saved_;
        flag_ = nullptr;
    }
}

void trace_movement_world(MovementCollisionEngine& engine, const MovementCollisionRay& ray,
                          std::uint32_t mask, MovementCollisionTrace& trace,
                          std::uint8_t* engine_trace_flag) {
    MovementTraceFlagGuard guard(engine_trace_flag);
    engine.trace_ray(ray, mask, {MovementCollisionFilterKind::reject_entities, 0, 0, 0}, trace);
}

MovementCollisionRay make_movement_hull_ray(MovementVector start, MovementVector end,
                                            MovementVector minimum,
                                            MovementVector maximum) noexcept {
    MovementCollisionRay ray;
    ray.delta = {end.x - start.x, end.y - start.y, end.z - start.z};
    ray.swept =
        (ray.delta.y * ray.delta.y + ray.delta.x * ray.delta.x) + ray.delta.z * ray.delta.z != 0.0f;
    ray.extents = {(maximum.x - minimum.x) * 0.5f, (maximum.y - minimum.y) * 0.5f,
                   (maximum.z - minimum.z) * 0.5f};
    const float squared = (ray.extents.y * ray.extents.y + ray.extents.x * ray.extents.x) +
                          ray.extents.z * ray.extents.z;
    ray.is_ray = static_cast<double>(squared) < 1.0e-6;
    const MovementVector center{(minimum.x + maximum.x) * 0.5f, (minimum.y + maximum.y) * 0.5f,
                                (minimum.z + maximum.z) * 0.5f};
    ray.start = {start.x + center.x, start.y + center.y, start.z + center.z};
    ray.start_offset = {center.x * -1.0f, center.y * -1.0f, center.z * -1.0f};
    return ray;
}

} // namespace skeet

// ============================================================================
// Subcomponent: command_air.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;
/* deduplicated helper */
bool convar_self_integer(const void* object) noexcept {
    for (;;) {
        const auto* parent = field<const void*>(object, 0x1c);
        if (parent == object)
            break;
        object = parent;
    }
    return field<std::uint32_t>(object, 0x30) ==
           static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(object));
}
ZHopSettings z_hop_settings(const MovementCommandOwner& owner) noexcept {
    return {owner.config.z_hop.value != 0, owner.config.config_2b499e77.value.binding,
            owner.config.config_3027dc11.value, owner.config.config_3127de82.value};
}
AirStrafeSelection select(MovementCommandOwner& owner, const client::Command& command,
                          const MovementCommandBindings& b, bool normalized_basis) {
    const AirStrafeSettings settings{owner.config.air_strafe_direction.value,
                                     owner.config.pre_speed.value != 0,
                                     owner.config.config_712c1936.value.binding};
    auto selection = select_air_strafe_modes(settings, b.input, false, command.movement);
    if (normalized_basis && selection.pre_speed_requested)
        selection.pre_speed_effective = !convar_self_integer(b.pre_speed_guard);
    return selection;
}

AirVelocityPreparation prepare_velocity(MovementCommandOwner& owner, void* player,
                                        MovementVector velocity, std::uint32_t initial_flags,
                                        const AirStrafeSelection& selection, UserCommand& command,
                                        const MovementCommandBindings& b) {
    const float stamina = field<float>(player, b.fields.stamina_2f33c681);
    MovementSimulationRules rules{};
    rules.gravity = movement_command_convar_float(b.simulation.convars.gravity);
    rules.interval_per_tick = *b.environment.interval;
    MovementPhysics physics{};
    physics.player_flags = initial_flags;
    bool cap = false;
    const float squared = velocity.y * velocity.y + velocity.x * velocity.x;
    if ((initial_flags & on_ground) && (command.buttons & jump_button) && !(4.0f > squared)) {
        if (stamina > 0.0f)
            rules.stamina_recovery =
                movement_command_convar_float(b.simulation.convars.stamina_recovery);
        cap = convar_self_integer(owner.jump_speed_cap);
        if (cap)
            physics.player_speed_limit =
                field<float>(player, b.environment.fields.speed_limit_3eb71843);
        physics.ducking = field<std::uint8_t>(player, b.environment.fields.ducking_6b1a59b5) != 0;
        if (!physics.ducking) {
            const auto flags = movement_command_player_flags(player, b);
            physics.player_flags = (initial_flags & ~2u) | (flags & 2u);
        }
        rules.jump_impulse = movement_command_convar_float(b.simulation.convars.jump_impulse);
    }
    return prepare_air_strafe_velocity(
        velocity, stamina, physics, rules, (selection.movement_key_mode & 0xffu) != 0,
        owner.config.bunny_hop.value != 0, cap, owner.side_magnitude, command);
}

bool pre_speed(MovementCommandOwner& owner, void* player, const MovementVector& origin,
               MovementVector velocity, const MovementVector& forecast_velocity, float speed,
               const MovementFeedbackHistory& history, MovementFeedback& feedback,
               client::Command& command, MovementAngleView angles,
               const MovementCommandBindings& b) {
    const LivePreSpeedInput input{owner.limits, velocity, forecast_velocity, origin,
                                  history.primary_heading};
    const LivePreSpeedBindings bindings{player, b.environment, b.simulation,
                                        b.fields.origin_a9ee6062, b.collision};
    angles.yaw = choose_live_pre_speed_heading(input, angles.yaw, bindings);
    const float ideal =
        minimum_number(owner.limits.maximum_angle, owner.limits.angle_scale / (speed + epsilon));
    return resolve_pre_speed_feedback(history.primary_heading, ideal, owner.brake_threshold,
                                      owner.side_magnitude, feedback, command.movement, angles);
}

void raw(MovementCommandOwner& owner, void* player, void* weapon, client::Command& command,
         MovementAngleView angles, const MovementFeedbackHistory& history,
         MovementFeedback& feedback, const MovementCommandBindings& b) {
    // The raw preamble retains the velocity address, then obtains the origin
    // reference before loading velocity/ground state and selecting the modes.
    const auto& live_velocity = *reinterpret_cast<const MovementVector*>(
        static_cast<const std::byte*>(player) + b.fields.velocity_36375c23);
    const auto& origin = movement_command_position_ref(player);
    const auto flags = movement_command_player_flags(player, b);
    if (feedback.original_buttons & speed_button)
        return;
    const auto selection = select(owner, command, b, false);
    if (!selection.continue_solver)
        return;
    auto velocity = live_velocity;
    const auto definition = weapon ? movement_command_weapon_definition(weapon, b) : 0u;
    const RawScoutGateFrame scout{velocity, flags, weapon != nullptr, std::int32_t(definition),
                                  origin};
    const RawScoutGateBindings scout_bindings{player, b.engine_client, b.collision,
                                              b.environment.engine_trace_flag};
    if (process_raw_scout_gate({owner.config.bunny_hop.value != 0, owner.side_magnitude}, selection,
                               scout, feedback, command.movement,
                               scout_bindings) != RawScoutGateResult::continue_air)
        return;
    // Native79B59 rereads only Z after the scout traces when cached XY speed
    // is zero. The XY values saved before those traces remain the heading seed.
    if (horizontal_length(velocity) == 0.0f)
        velocity.z = live_velocity.z;
    if (!prepare_air_strafe_heading(selection, velocity, false, command.movement, angles))
        return;
    const AirWallSteeringInput wall{
        owner.config.avoid_collisions.value != 0,
        owner.config.config_af9ae891.value,
        owner.limits.angle_scale,
        owner.limits.maximum_angle,
        owner.side_magnitude,
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(player)),
        origin,
        velocity,
        flags,
        0};
    const AirWallSteeringLiveBindings wall_bindings{
        player,
        b.environment,
        b.simulation,
        b.collision,
        b.ballistics,
        b.surface_properties,
        origin,
        live_velocity,
        {owner.config.config_af9ae891.value, owner.config.air_strafe_smoothing.value,
         owner.limits.angle_scale, owner.limits.maximum_angle, owner.side_magnitude,
         owner.brake_threshold, owner.angle_step}};
    steer_live_air_wall(wall, feedback, command, angles, wall_bindings);
    const float speed = horizontal_length(velocity);
    const float ideal =
        minimum_number(owner.limits.maximum_angle, owner.limits.angle_scale / (speed + epsilon));
    const float velocity_yaw = planar_movement_yaw(velocity);
    auto smoothing = owner.config.air_strafe_smoothing.value;
    if (selection.pre_speed_requested) {
        if (!convar_self_integer(b.pre_speed_guard)) {
            if (pre_speed(owner, player, origin, velocity, live_velocity, speed, history, feedback,
                          command, angles, b))
                return;
            smoothing = 0;
        } else {
            smoothing = 100;
            apply_guarded_pre_speed_heading(speed, velocity_yaw, history.primary_heading,
                                            owner.limits.maximum_angle, angles);
        }
    } else if (apply_z_hop_heading(z_hop_settings(owner), b.input, b.engine_time, angles))
        smoothing = 0;
    AirSteeringState state{feedback.side_flip != 0, feedback.primary_heading};
    write_air_strafe_step({velocity_yaw, ideal, speed, owner.side_magnitude, smoothing,
                           (selection.movement_key_mode & 0xffu) != 0},
                          state, command.movement, angles);
    feedback.side_flip = std::uint8_t(state.side_flip);
    feedback.primary_heading = state.feedback_heading;
}

void normalized(MovementCommandOwner& owner, void* player, void* weapon, client::Command& command,
                MovementAngleView angles, const MovementFeedbackHistory& history,
                MovementFeedback& feedback, const MovementCommandBindings& b) {
    // Unlike the raw path, native7322A copies all three velocity components
    // before the player origin getter. Its ground byte is loaded afterwards.
    const auto original_velocity = movement_command_velocity(player, b);
    const auto& origin = movement_command_position_ref(player);
    const auto flags = movement_command_player_flags(player, b);
    if (feedback.original_buttons & speed_button)
        return;
    const auto selection = select(owner, command, b, true);
    if (!selection.continue_solver)
        return;
    const auto definition = weapon ? movement_command_weapon_definition(weapon, b) : 0u;
    if (!pass_normalized_scout_gate({player, b.engine_client, original_velocity, origin,
                                     weapon != nullptr, static_cast<std::uint16_t>(definition)},
                                    command, b.collision, b.environment.engine_trace_flag))
        return;
    const auto prepared =
        prepare_velocity(owner, player, original_velocity, flags, selection, command.movement, b);
    if (!prepared.continue_solver ||
        !prepare_air_strafe_heading(selection, prepared.velocity, true, command.movement, angles))
        return;
    const NormalizedAirWallInput wall{
        owner.config.avoid_collisions.value != 0,
        owner.config.config_af9ae891.value,
        owner.config.air_strafe_smoothing.value,
        owner.limits.maximum_angle,
        owner.side_magnitude,
        owner.brake_threshold,
        owner.angle_step,
        0.0f,
        selection.pre_speed_effective,
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(player)),
        origin,
        prepared.velocity,
        flags,
        0};
    const AirWallSteeringLiveBindings wall_bindings{
        player,
        b.environment,
        b.simulation,
        b.collision,
        b.ballistics,
        b.surface_properties,
        origin,
        prepared.velocity,
        {owner.config.config_af9ae891.value, owner.config.air_strafe_smoothing.value,
         owner.limits.angle_scale, owner.limits.maximum_angle, owner.side_magnitude,
         owner.brake_threshold, owner.angle_step}};
    const float velocity_yaw = steer_live_normalized_air_wall(wall, feedback, command, angles,
                                                              wall_bindings, owner.air_accelerate);
    const float speed = horizontal_length(prepared.velocity);
    if (selection.pre_speed_requested) {
        if (selection.pre_speed_effective &&
            pre_speed(owner, player, origin, prepared.velocity, prepared.velocity, speed, history,
                      feedback, command, angles, b))
            return;
    } else
        apply_z_hop_heading(z_hop_settings(owner), b.input, b.engine_time, angles);
    const NormalizedAirFinalSettings settings{owner.config.air_strafe_smoothing.value,
                                              owner.limits.maximum_angle, owner.side_magnitude,
                                              owner.brake_threshold, owner.angle_step};
    const NormalizedAirFinalInput input{origin, prepared.velocity, prepared.stamina, velocity_yaw,
                                        speed,  selection};
    const NormalizedAirFinalBindings bindings{
        player,       b.environment,        b.simulation,        b.collision,
        b.ballistics, b.surface_properties, owner.air_accelerate};
    finish_normalized_air_strafe(settings, input, history, feedback, command, angles, bindings);
}
}

void process_live_movement_air(MovementCommandOwner& owner, void* player, void* weapon,
                               client::Command& command, MovementAngleView angles,
                               const MovementFeedbackHistory& history, MovementFeedback& feedback,
                               const MovementCommandBindings& bindings) {
    if (owner.config.config_ad536de0.value)
        normalized(owner, player, weapon, command, angles, history, feedback, bindings);
    else
        raw(owner, player, weapon, command, angles, history, feedback, bindings);
}

} // namespace skeet

// ============================================================================
// Subcomponent: command_bindings.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;
/* deduplicated helper */
const void* terminal(const void* object) noexcept {
    for (;;) {
        const auto* parent = field<const void*>(object, 0x1Cu);
        if (parent == object)
            return object;
        object = parent;
    }
}
bool self_integer(const void* object) noexcept {
    object = terminal(object);
    return field<std::uint32_t>(object, 0x30u) ==
           static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(object));
}
}

float movement_command_convar_float(const void* object) noexcept {
    object = terminal(object);
    return std::bit_cast<float>(
        field<std::uint32_t>(object, 0x2Cu) ^
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(object)));
}
std::uint32_t movement_command_player_flags(void* player,
                                            const MovementCommandBindings& b) noexcept {
    return field<std::uint32_t>(player, b.environment.fields.player_flags_e2fbe695);
}
std::uint8_t movement_command_player_type(void* player, const MovementCommandBindings& b) noexcept {
    return field<std::uint8_t>(player, b.fields.movement_type_7501cb91);
}
MovementVector movement_command_velocity(void* player, const MovementCommandBindings& b) noexcept {
    return field<MovementVector>(player, b.fields.velocity_36375c23);
}
MovementVector movement_command_position(void* player) {
    return *engine::invoke<const MovementVector*>(player, 0x28u / 4u);
}
const MovementVector& movement_command_position_ref(void* player) {
    return *engine::invoke<const MovementVector*>(player, 0x28u / 4u);
}
std::uint32_t movement_command_trace_mask(void* player) {
    return engine::invoke<std::uint32_t>(player, 0x260u / 4u);
}
PlayerAirGateBindings capture_player_air_gate_bindings(const MovementCommandBindings& b) noexcept {
    return {{b.fields.movement_type_7501cb91, b.fields.player_state_ac7f2913,
             b.fields.velocity_36375c23, b.fields.weapon_definition_5c9f0fbb,
             b.fields.weapon_blocked_b8969256, b.fields.player_next_attack_de530a8b,
             b.fields.weapon_next_attack_f71ce331},
            b.environment,
            &b.weapon_policy,
            &b.current_time,
            b.bunny_hop_guard};
}
bool movement_command_weapon_blocked(void* weapon, const MovementCommandBindings& b) noexcept {
    return field<std::uint8_t>(weapon, b.fields.weapon_blocked_b8969256) != 0;
}
float movement_command_player_attack(void* player, const MovementCommandBindings& b) noexcept {
    return field<float>(player, b.fields.player_next_attack_de530a8b);
}
float movement_command_weapon_attack(void* weapon, const MovementCommandBindings& b) noexcept {
    return field<float>(weapon, b.fields.weapon_next_attack_f71ce331);
}
float movement_command_attack_guard(void* player, const MovementCommandBindings& b) noexcept {
    return field<float>(player, b.fields.attack_guard_8815d240);
}
std::uint16_t movement_command_weapon_definition(void* weapon,
                                                 const MovementCommandBindings& b) noexcept {
    return field<std::uint16_t>(weapon, b.fields.weapon_definition_5c9f0fbb);
}
MovementCommandFriction
capture_movement_command_friction(const MovementCommandBindings& b) noexcept {
    MovementCommandFriction result;
    result.stop_speed = movement_command_convar_float(b.environment.convars.stop_speed);
    result.friction = movement_command_convar_float(b.environment.convars.friction);
    result.interval = *b.environment.interval;
    return result;
}

void process_live_duck_peek(MovementCommandOwner& owner, void* player, void* weapon,
                            UserCommand& command, std::uint8_t& send,
                            const MovementCommandBindings& b) {
    auto& state = owner.duck_peek;
    auto& flags = b.scheduler_flags;
    const DuckPeekSettings settings{b.duck_peek_hotkey, b.infinite_duck != 0};
    DuckPeekFrame frame{};
    frame.physics.player_flags = movement_command_player_flags(player, b);
    if (!(frame.physics.player_flags & on_ground) || (command.buttons & jump_button) ||
        (!hotkey_active(settings.hotkey, b.input) && !(flags & 0x100u))) {
        process_duck_peek(settings, b.input, state, frame, command, flags, send);
        return;
    }
    if (!settings.crouch_cycle) {
        const auto velocity = movement_command_velocity(player, b);
        const float squared =
            (velocity.x * velocity.x + velocity.y * velocity.y) + velocity.z * velocity.z;
        if (!(squared <= 10.0f))
            return;
        command.buttons |= duck_button;
        if (weapon) {
            const float elapsed =
                b.current_time - field<float>(weapon, b.fields.weapon_simulation_time_0176a381);
            const float weapon_delay = engine::invoke<float>(weapon, 0x6C4u / 4u);
            const float cvar_delay = movement_command_convar_float(b.duck_delay);
            if (elapsed > maximum_number(cvar_delay, weapon_delay) + 0.75f)
                command.buttons &= ~duck_button;
        }
        return;
    }
    const bool separate = (state.original_buttons & duck_button) && state.last_key != 27 &&
                          std::uint32_t(state.last_key) != (settings.hotkey.packed >> 2);
    if (separate) {
        if ((!(state.previous_buttons & duck_button) || state.release_latch) && state.choked < 14)
            frame.physics.duck_amount =
                field<float>(player, b.environment.fields.duck_amount_3309c0cf);
    } else {
        frame.duck_delay = movement_command_convar_float(b.duck_delay);
        frame.player_duck_time = field<float>(player, b.fields.duck_time_04b798c6);
        frame.current_time = b.current_time;
        if (!(frame.player_duck_time + frame.duck_delay > frame.current_time)) {
            frame.physics.duck_speed =
                field<float>(player, b.environment.fields.duck_speed_fb7e3404);
            if (!(6.0f > frame.physics.duck_speed)) {
                frame.command_budget = client::query_command_budget(b.budget);
                const auto budget =
                    std::bit_cast<std::int32_t>(std::uint32_t(frame.command_budget) - 1u);
                if ((budget < *state.configured_limit ? budget : *state.configured_limit) >= 3) {
                    frame.physics.interval_per_tick = *b.environment.interval;
                    frame.physics.duck_amount =
                        field<float>(player, b.environment.fields.duck_amount_3309c0cf);
                }
            }
        }
    }
    process_duck_peek(settings, b.input, state, frame, command, flags, send);
}

MovementCommandHull capture_quick_peek_origin_hull(void* player) {
    MovementCommandHull result;
    result.mask = engine::invoke<std::uint32_t>(player, 0x260u / 4u);
    auto* collideable = engine::invoke<void*>(player, 0xCu / 4u);
    const auto* maximum = engine::invoke<const MovementVector*>(collideable, 2);
    collideable = engine::invoke<void*>(player, 0xCu / 4u);
    const auto* minimum = engine::invoke<const MovementVector*>(collideable, 1);
    result.maximum = *maximum;
    result.minimum = *minimum;
    return result;
}
MovementCommandHull capture_quick_peek_limit_hull(void* player) {
    MovementCommandHull result;
    const auto* minimum = engine::invoke<const MovementVector*>(player, 0x130u / 4u);
    const auto* maximum = engine::invoke<const MovementVector*>(player, 0x134u / 4u);
    result.minimum = *minimum;
    result.maximum = *maximum;
    result.mask = engine::invoke<std::uint32_t>(player, 0x260u / 4u);
    return result;
}
QuickPeekHistory capture_quick_peek_history(void* player, std::int32_t number,
                                            const MovementCommandBindings& b) {
    QuickPeekHistory result;
    result.command_number = number;
    result.position = movement_command_position(player);
    result.unavailable = b.history_command != -1 && b.history_available != 0;
    result.grounded = (movement_command_player_flags(player, b) & on_ground) != 0;
    return result;
}
bool movement_command_quick_peek_shot(const QuickPeekState& state, void* player,
                                      const MovementCommandBindings& b) noexcept {
    std::int32_t previous = -1;
    for (int i = 0; i < 16; ++i) {
        const auto number =
            std::bit_cast<std::int32_t>(std::uint32_t(b.last_acknowledged) - std::uint32_t(i));
        const auto* record =
            prediction::CommandHistoryView::from_owner(b.history_owner).find(number);
        if (!record)
            break;
        if ((record->flags & 1u) && record->shot_time != 0.0f && record->fired) {
            previous = number;
            break;
        }
    }
    if (previous != -1) {
        for (int i = 1; i < 16; ++i) {
            const auto number =
                std::bit_cast<std::int32_t>(std::uint32_t(previous) + std::uint32_t(i));
            if (number > b.last_acknowledged)
                break;
            const auto* record =
                prediction::CommandHistoryView::from_owner(b.history_owner).find(number);
            if (!record)
                break;
            if ((record->flags & 1u) && record->shot_time == 0.0f && !record->fired)
                return true;
        }
    }
    return field<std::int32_t>(player, b.fields.shot_counter_8cfc633a) > state.shot_counter;
}
MovementPhysics capture_movement_command_speed(void* player, const MovementCommandBindings& b) {
    return capture_movement_command_physics(player, b.environment);
}

GroundAcceleration movement_command_ground_acceleration(void* player, const UserCommand& command,
                                                        MovementVector velocity,
                                                        MovementVector direction, float wish_speed,
                                                        const MovementCommandBindings& b) {
    MovementPhysics physics{};
    physics.accelerate = movement_command_convar_float(b.environment.convars.accelerate);
    if (0.0f >= wish_speed - dot(velocity, direction))
        return {0.0f, 250.0f, physics.accelerate};
    UserCommand copied{};
    copied.buttons = command.buttons;
    if (!(copied.buttons & duck_button)) {
        physics.ducking = field<std::uint8_t>(player, b.environment.fields.ducking_6b1a59b5) != 0;
        if (!physics.ducking)
            physics.player_flags = movement_command_player_flags(player, b);
    }
    physics.weapon_speed_enabled = !self_integer(b.environment.convars.accelerate_weapon_speed);
    if (physics.weapon_speed_enabled) {
        const auto handle =
            field<std::int32_t>(player, b.environment.fields.weapon_handle_cd812654);
        if (handle != -1) {
            auto* weapon = engine::invoke<void*>(b.environment.entity_list, 4, handle);
            if (weapon) {
                physics.has_weapon = true;
                physics.weapon_max_speed = engine::invoke<float>(weapon, 0x6E8u / 4u);
                physics.weapon_mode_count = engine::invoke<std::int32_t>(weapon, 0x738u / 4u);
                if (physics.weapon_mode_count > 0)
                    physics.weapon_mode = engine::invoke<std::int32_t>(weapon, 0x6ECu / 4u);
            }
        }
    }
    physics.interval_per_tick = *b.environment.interval;
    return ground_acceleration(physics, copied, velocity, direction, wish_speed);
}
bool write_live_movement_counter(void* player, UserCommand& command, MovementVector velocity,
                                 MovementAngleView angles, const MovementCommandBindings& b) {
    command.buttons &= ~0x61au;
    command.forward_move = 0.0f;
    command.side_move = 0.0f;
    const float speed = horizontal_length(velocity);
    if (!(speed >= 0.1f))
        return false;
    const auto friction = capture_movement_command_friction(b);
    const float remaining =
        maximum_number(speed - (maximum_number(friction.stop_speed, speed) * friction.friction) *
                                   friction.interval,
                       0.0f);
    if (!(remaining >= 0.1f))
        return false;
    const float factor = remaining / speed;
    const MovementVector decayed{velocity.x * factor, velocity.y * factor, factor * 0.0f};
    const MovementVector reverse{negate(velocity.x), negate(velocity.y), 0.0f};
    angles = horizontal_angles(reverse, false);
    command.forward_move = 450.0f;
    const auto physics = capture_movement_command_speed(player, b);
    const float maximum = minimum_number(physics.player_speed_limit, physics.player_max_speed);
    const auto acceleration = movement_command_ground_acceleration(
        player, command, decayed, normalize_horizontal(reverse), maximum, b);
    if (acceleration.amount > remaining)
        command.forward_move = 0.001f;
    return true;
}
} // namespace skeet

// ============================================================================
// Subcomponent: command_phase_one.cpp
// ============================================================================

namespace skeet {

void process_movement_phase_one(MovementCommandOwner& owner, client::Command& command,
                                bool infinite_duck) noexcept {
    EasyStrafeState state{owner.previous_buttons_1e0, owner.suppressed_buttons_1e4,
                          owner.duck_peek.previous_buttons};
    process_easy_strafe(state, command.movement, owner.config.easy_strafe.value, infinite_duck,
                        owner.side_magnitude);
    owner.previous_buttons_1e0 = state.previous_buttons;
    owner.suppressed_buttons_1e4 = state.suppressed_buttons;
    owner.duck_peek.previous_buttons = state.saved_buttons;
}

} // namespace skeet

// ============================================================================
// Subcomponent: command_runtime.cpp
// ============================================================================

namespace skeet {
namespace {
void process(MovementCommandOwner& owner, void* player, void* weapon, client::Command& command,
             MovementAngleView angles, std::uint8_t& send_packet, MovementFeedback& feedback,
             const MovementCommandBindings& bindings) {
    const auto history = begin_movement_command(
        feedback, command.movement, owner.duck_peek.original_buttons, bindings.scheduler_flags);
    process_live_duck_peek(owner, player, weapon, command.movement, send_packet, bindings);
    if (weapon && process_live_quick_peek(owner.quick_peek_config, bindings.input, owner.quick_peek,
                                          player, weapon, command, feedback.original_buttons,
                                          angles, bindings.scheduler_flags,
                                          bindings) == QuickPeekContinuation::complete)
        return;

    const auto ground = process_live_ground_phase(owner, player, weapon, command, angles, bindings);
    if (ground == GroundPhaseContinuation::complete)
        return;
    if (ground == GroundPhaseContinuation::weapon_speed_features &&
        process_live_weapon_speed_phase(player, weapon, command, angles, bindings) ==
            WeaponSpeedPhaseContinuation::complete)
        return;

    const PlayerAirGateSettings settings{owner.config.air_strafe.value != 0,
                                         owner.config.bunny_hop.value != 0,
                                         owner.side_magnitude,
                                         owner.brake_reduction_limit,
                                         owner.brake_threshold,
                                         owner.air_accelerate};
    const PlayerAirGateFrame frame{player, weapon, &feedback};
    const auto player_bindings = capture_player_air_gate_bindings(bindings);
    const auto gate = process_player_air_gate(settings, frame, player_bindings,
                                              bindings.scheduler_flags, command.movement, angles);
    if (gate.continuation == PlayerAirGateContinuation::complete)
        return;
    if (gate.continuation == PlayerAirGateContinuation::air_solver && gate.air_strafe_enabled)
        process_live_movement_air(owner, player, weapon, command, angles, history, feedback,
                                  bindings);
    process_live_ground_speed_tail(owner, player, command, angles, bindings);
}
}

void process_movement_command(MovementCommandOwner& owner, void* player, void* weapon,
                              client::Command& command, MovementAngles& angles,
                              std::uint8_t& send_packet, MovementFeedback& feedback,
                              const MovementCommandBindings& bindings) {
    process(owner, player, weapon, command, MovementAngleView{angles}, send_packet, feedback,
            bindings);
}

void process_movement_command(MovementCommandOwner& owner, void* player, void* weapon,
                              client::Command& command, std::array<float, 3>& angles,
                              std::uint8_t& send_packet, MovementFeedback& feedback,
                              const MovementCommandBindings& bindings) {
    process(owner, player, weapon, command, MovementAngleView{angles}, send_packet, feedback,
            bindings);
}

} // namespace skeet

// ============================================================================
// Subcomponent: command_state.cpp
// ============================================================================

namespace skeet {

MovementFeedbackHistory begin_movement_command(MovementFeedback& feedback,
                                               const UserCommand& command,
                                               std::uint32_t& original_buttons,
                                               std::uint32_t& scheduler_flags) noexcept {
    const MovementFeedbackHistory previous{feedback.primary_heading, feedback.turn_direction,
                                           feedback.secondary_heading};
    feedback.primary_heading = std::numeric_limits<float>::max();
    feedback.turn_direction = 0.0f;
    feedback.secondary_heading = std::numeric_limits<float>::max();
    feedback.original_buttons = command.buttons;
    scheduler_flags &= 0xfffd5f3cu;
    original_buttons = feedback.original_buttons;
    return previous;
}

} // namespace skeet

// ============================================================================
// Subcomponent: command_wrapper.cpp
// ============================================================================

namespace skeet {
namespace {
/* deduplicated helper */
}

void movement_owner_phase1(MovementCommandPhaseOwner& owner, client::Command& command) {
    process_movement_phase_one(owner.state, command, owner.bindings.command.infinite_duck != 0);
}

void movement_owner_phase2(MovementCommandPhaseOwner& owner, void* player, void* weapon,
                           client::Command& command, std::array<float, 3>& angles,
                           std::uint8_t& send_packet) {
    auto& state = owner.state;
    const auto& bindings = owner.bindings;
    const auto& live = bindings.command;
    const auto* const count = bindings.choked_commands;
    state.callback_flags_1dc &= ~1u;
    if (*count == 0) {
        // The guard reads this field even though both valid-player branches
        // join at the same origin copy. Its other branch is invalid code.
        [[maybe_unused]] volatile auto movement_flags =
            field<std::uint32_t>(player, bindings.movement_flags_38ca5228);
        state.previous_packet_origin = field<MovementVector>(player, live.fields.origin_a9ee6062);
    }
    state.duck_peek.choked = *count;
    process_movement_command(state, player, weapon, command, angles, send_packet, state.feedback,
                             live);
    state.feedback.was_grounded =
        static_cast<std::uint8_t>(movement_command_player_flags(player, live) & 1u);
    update_quick_peek_shot_counter(state.quick_peek,
                                   field<std::int32_t>(player, live.fields.shot_counter_8cfc633a));
}

void movement_owner_phase4(MovementCommandPhaseOwner& owner, client::CommandFrame& frame,
                           client::Command& command) {
    MovementFrame projected{frame.flags, frame.player_flags_f8};
    const MovementSettings settings{owner.state.config.jump_at_edge.value != 0,
                                    owner.state.callback_flags_1dc};
    post_prediction(settings, projected, command.movement, owner.bindings.jump_at_edge_key,
                    owner.bindings.command.input);
    frame.flags = projected.packed_flags;
}

} //namespace skeet

// ============================================================================
// Subcomponent: config.cpp
// ============================================================================

namespace skeet {

// Descriptor registration order in 0x4348FADB..0x4348FC96 is observable through
// configuration traversal and through the two independently linked hotkeys.
void initialize_movement_config(ConfigRegistries& registries, MovementConfig& config) noexcept {
    auto& descriptors = registries.descriptors;
    initialize_boolean_config(descriptors, config.air_strafe, 0xF2EB89E9u);
    initialize_boolean_config(descriptors, config.config_ad536de0, 0xAD536DE0u);
    initialize_integer_config(descriptors, config.air_strafe_direction, 0x6F6ACC9Bu, 1);
    initialize_integer_config(descriptors, config.air_strafe_smoothing, 0xC94C5F0Bu, 70);
    initialize_boolean_config(descriptors, config.z_hop, 0x3FDF2566u);
    initialize_integer_config(descriptors, config.config_3027dc11, 0x3027DC11u, 175);
    initialize_integer_config(descriptors, config.config_3127de82, 0x3127DE82u, 200);
    initialize_boolean_config(descriptors, config.pre_speed, 0xCA7FC107u);
    initialize_boolean_config(descriptors, config.fast_walk, 0xB5E64AD1u);
    initialize_hotkey_config(registries, config.config_712c1936, 0x712C1936u, 0x46,
                             HotkeyMode::hold);
    initialize_hotkey_config(registries, config.config_2b499e77, 0x2B499E77u, 0x54,
                             HotkeyMode::hold);
    initialize_boolean_config(descriptors, config.easy_strafe, 0xDB2172A1u);
    initialize_boolean_config(descriptors, config.standalone_quick_stop, 0x4CD278CAu);
    initialize_boolean_config(descriptors, config.avoid_collisions, 0x3095C574u);
    initialize_integer_config(descriptors, config.config_af9ae891, 0xAF9AE891u, 200);
    initialize_boolean_config(descriptors, config.slow_motion, 0x4DF394FFu);
    initialize_boolean_config(descriptors, config.bunny_hop, 0xFE3FDAABu);
    initialize_boolean_config(descriptors, config.jump_at_edge, 0x8C13C109u);
}

// 0x43402969..0x434029C5. Runtime state initialization starts at 0x434029C6.
void initialize_quick_peek_config(ConfigRegistries& registries, QuickPeekConfig& config) noexcept {
    initialize_boolean_config(registries.descriptors, config.enabled, 0x141C9037u);
    initialize_hotkey_config(registries, config.hotkey, 0xEC178029u, 0x1B, HotkeyMode::hold);
    initialize_integer_config(registries.descriptors, config.mode, 0x4F67FC33u, 3);
    initialize_color_config(registries.descriptors, config.color, 0xC9FCBE4Bu, 0x4D0000FFu);
    initialize_integer_config(registries.descriptors, config.distance, 0x4CB8E4AAu, 201);
}

} // namespace skeet

// ============================================================================
// Subcomponent: duck_peek.cpp
// ============================================================================

namespace skeet {
namespace {
std::int32_t decrement(std::int32_t value) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(value) - 1u);
}
std::int32_t truncate(float value) noexcept {
    return _mm_cvttss_si32(_mm_set_ss(value));
}
void force_send(std::uint32_t& flags) noexcept {
    flags = (flags & 0xfffffbffu) | 0x200u;
}
// Actual helper 4331E7F5, including the native cubic's operation order.
float duck_height(float amount) noexcept {
    const float square = amount * amount;
    const float doubled = square + square;
    const float smooth = square * 3.0f - doubled * amount;
    return (1.0f - smooth) * 18.0f + 46.0f;
}

void short_crouch_cycle(DuckPeekState& state, const DuckPeekFrame& frame, std::int32_t limit,
                        UserCommand& command, std::uint32_t& flags,
                        std::uint8_t& send_packet) noexcept {
    command.buttons |= duck_button;
    const float interval = frame.physics.interval_per_tick;
    const float amount = frame.physics.duck_amount;
    const float crouch_step = (interval * 0.8f) * 8.0f;
    if (state.phase == 0) {
        const float threshold = 1.0f - interval * 8.0f;
        if (!(absolute((crouch_step + amount) - threshold) <= 0.001f)) {
            if (absolute(amount - threshold) <= 0.001f) {
                const auto total = limit + 1;
                const auto release = total / 2;
                const auto crouch = total - release;
                state.release_commands = release;
                state.crouch_commands = crouch;
                const float release_height = (interval * 8.0f) * static_cast<float>(release);
                const float crouch_height = ((interval * 0.8f) * 8.0f) * static_cast<float>(crouch);
                state.delay = 0;
                float best_difference = 0.0f;
                for (std::int32_t delay = 0; delay < 10; ++delay) {
                    const float target =
                        (1.0f - (interval * 8.0f) * static_cast<float>(delay + 1)) - release_height;
                    const float first = duck_height(target);
                    const float second = duck_height(target + crouch_height);
                    const float difference = first - second;
                    if (best_difference >= difference)
                        break;
                    state.delay = delay;
                    state.target = target;
                    best_difference = difference;
                }
                force_send(flags);
                state.phase = 1;
            } else if (amount > threshold) {
                send_packet = 0;
                command.buttons &= ~duck_button;
            }
        }
    }

    // A completed phase enters its successor during this same command.
    if (state.phase == 1) {
        if (state.delay > 0) {
            state.delay = decrement(state.delay);
            command.buttons &= ~duck_button;
            force_send(flags);
        } else
            state.phase = 2;
    }
    if (state.phase == 2) {
        if (state.release_commands > 0) {
            state.release_commands = decrement(state.release_commands);
            if (state.choked < limit) {
                command.buttons &= ~duck_button;
                send_packet = 0;
            }
        } else
            state.phase = 3;
    }
    if (state.phase == 3) {
        if (state.crouch_commands == 0)
            state.phase = 0;
        else {
            state.crouch_commands = decrement(state.crouch_commands);
            if (state.choked < limit)
                send_packet = 0;
        }
    }
    if ((command.buttons & duck_button) && crouch_step + amount > 0.99f)
        command.buttons &= ~duck_button;
}

void long_crouch_cycle(DuckPeekState& state, const DuckPeekFrame& frame, std::int32_t limit,
                       UserCommand& command, std::uint8_t& send_packet) noexcept {
    state.target = 0.0f;
    command.buttons |= duck_button;
    const float interval = frame.physics.interval_per_tick;
    const float release_step = interval * 8.0f;
    const float crouch_step = (interval * 0.8f) * 8.0f;
    const float product = release_step * crouch_step;
    const float sum = release_step + crouch_step;
    float target = static_cast<float>(limit) / (sum / product);
    // MAXSS with zero, then MINSS with one: unordered selects operand two.
    if (!(target > 0.0f))
        target = 0.0f;
    if (!(target < 1.0f))
        target = 1.0f;
    const float amount = frame.physics.duck_amount;
    bool advance = state.crouch_active;
    if (amount >= target) {
        state.crouch_active = true;
        advance = true;
    }
    if (advance && state.choked < limit) {
        const auto crouch_ticks = truncate(target / crouch_step + 0.5f);
        const auto remaining =
            std::bit_cast<std::int32_t>(std::uint32_t(limit) - std::uint32_t(state.choked));
        if (remaining >= crouch_ticks || amount > target)
            command.buttons &= ~duck_button;
        send_packet = 0;
    }
    if ((command.buttons & duck_button) && amount + crouch_step > 0.99f)
        command.buttons &= ~duck_button;
}
} // namespace

void process_duck_peek(const DuckPeekSettings& settings, InputBitmap input, DuckPeekState& state,
                       const DuckPeekFrame& frame, UserCommand& command, std::uint32_t& flags,
                       std::uint8_t& send_packet) {
    const bool original_duck = (state.original_buttons & duck_button) != 0;
    const bool separate_duck = original_duck && state.last_key != 27 &&
                               std::uint32_t(state.last_key) != (settings.hotkey.packed >> 2);
    if (!(frame.physics.player_flags & on_ground) || (command.buttons & jump_button) ||
        (!hotkey_active(settings.hotkey, input) && !(flags & 0x100u))) {
        state.crouch_active = false;
        state.release_latch = false;
        state.target = 0.0f;
        if (separate_duck)
            flags |= 2u;
        return;
    }
    if (!settings.crouch_cycle) {
        const auto velocity = frame.velocity;
        const float speed_squared =
            (velocity.x * velocity.x + velocity.y * velocity.y) + velocity.z * velocity.z;
        if (!(speed_squared <= 10.0f))
            return;
        command.buttons |= duck_button;
        if (frame.has_weapon) {
            const float elapsed = frame.current_time - frame.weapon_simulation_time;
            const float delay = maximum_number(frame.duck_delay, frame.weapon_duck_delay) + 0.75f;
            if (elapsed > delay)
                command.buttons &= ~duck_button;
        }
        return;
    }
    if (separate_duck) {
        flags |= 0x42u;
        if (!(state.previous_buttons & duck_button))
            state.release_latch = true;
        else if (!state.release_latch)
            return;
        if (state.choked < 14 && frame.physics.duck_amount < 1.0f) {
            send_packet = 0;
            return;
        }
        state.release_latch = false;
        return;
    }
    if (frame.player_duck_time + frame.duck_delay > frame.current_time) {
        flags |= 0x42u;
        command.buttons |= duck_button;
        state.release_latch = false;
        return;
    }
    state.release_latch = false;
    // The native JA only excludes ordered rates below six.
    if (6.0f > frame.physics.duck_speed)
        return;
    const auto budget_limit = decrement(frame.command_budget);
    const auto configured = *state.configured_limit;
    const auto limit = budget_limit < configured ? budget_limit : configured;
    if (limit < 3) {
        state.target = 0.0f;
        return;
    }
    flags |= 0x40u;
    if (limit <= 9)
        short_crouch_cycle(state, frame, limit, command, flags, send_packet);
    else
        long_crouch_cycle(state, frame, limit, command, send_packet);
}

} // namespace skeet

// ============================================================================
// Subcomponent: easy_strafe.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;

// 0x4333F746..0x4333F8E2, secondary interface receives owner + 8.
void process_easy_strafe(EasyStrafeState& state, UserCommand& command, bool enabled,
                         bool force_bullrush, float default_move) noexcept {
    const auto original = command.buttons;
    if (force_bullrush)
        command.buttons |= 0x400000;
    if (enabled) {
        const auto current = command.buttons & 0x618u;
        const auto pressed = (state.previous_buttons ^ current) & current;
        auto suppressed = state.suppressed_buttons;
        const auto update = [&](std::uint32_t key, std::uint32_t opposite) {
            if (pressed & key)
                suppressed = (suppressed & ~key) | opposite;
            else if (!(current & key))
                suppressed &= ~opposite;
        };
        update(move_left_button, move_right_button);
        update(move_right_button, move_left_button);
        update(forward_button, back_button);
        update(back_button, forward_button);
        state.suppressed_buttons = suppressed;
        command.buttons &= ~suppressed;
        if ((command.buttons & 0x600) != 0x600) {
            if (command.buttons & move_left_button) {
                float magnitude = absolute(command.side_move);
                if (magnitude == 0.0f)
                    magnitude = default_move;
                command.side_move = negate(magnitude);
            } else if (command.buttons & move_right_button) {
                float magnitude = absolute(command.side_move);
                command.side_move = magnitude == 0.0f ? default_move : magnitude;
            }
        }
        if ((command.buttons & 0x18) != 0x18) {
            if (command.buttons & forward_button) {
                float magnitude = absolute(command.forward_move);
                command.forward_move = magnitude == 0.0f ? default_move : magnitude;
            } else if (command.buttons & back_button) {
                float magnitude = absolute(command.forward_move);
                if (magnitude == 0.0f)
                    magnitude = default_move;
                command.forward_move = negate(magnitude);
            }
        }
    } else {
        state.suppressed_buttons = 0;
    }
    state.previous_buttons = original;
    state.saved_buttons = original;
}

} // namespace skeet

// ============================================================================
// Subcomponent: edge_jump.cpp
// ============================================================================

namespace skeet {
// Complete callback 0x433D1F99..0x433D1FF4, including its shared tail.
void post_prediction(const MovementSettings& settings, MovementFrame& frame, UserCommand& command,
                     const HotkeyBinding& binding, InputBitmap input) {
    if (settings.jump_at_edge && hotkey_active(binding, input)) {
        const bool held = (command.buttons & jump_button) != 0;
        const bool was_grounded = (frame.saved_player_flags & on_ground) != 0;
        const bool now_grounded = (frame.packed_flags & frame_on_ground) != 0;
        if (!held && was_grounded && !now_grounded)
            command.buttons |= jump_button;
    }
    if ((settings.shared_callback_flags & 1) != 0)
        frame.packed_flags |= frame_shared_flag;
}
} // namespace skeet

// ============================================================================
// Subcomponent: engine_collision.cpp
// ============================================================================

namespace skeet {
namespace {
engine::TraceRay native_ray(const MovementCollisionRay& source) noexcept {
    engine::TraceRay ray;
    ray.start = {source.start.x, source.start.y, source.start.z, 0.0f};
    ray.delta = {source.delta.x, source.delta.y, source.delta.z, 0.0f};
    ray.start_offset = {source.start_offset.x, source.start_offset.y, source.start_offset.z, 0.0f};
    ray.extents = {source.extents.x, source.extents.y, source.extents.z, 0.0f};
    ray.is_ray = source.is_ray;
    ray.swept = source.swept;
    return ray;
}

// Native4346EDC0 methods read434758B4 at every invocation. Keep the current
// table cell, rather than copying its entries before the engine TraceRay call.
struct LiveSkipFilter {
    engine::TraceSkipFilterPrefix prefix;
    const MovementEngineCollisionBindings* bindings;
#if defined(_M_IX86) || defined(__i386__)
    static bool __thiscall should_hit(void* receiver, void* entity, std::uint32_t mask) {
#else
    static bool should_hit(void* receiver, void* entity, std::uint32_t mask) {
#endif
        const auto& self = *static_cast<LiveSkipFilter*>(receiver);
        const auto table = self.bindings->skip_filter_vtable;
        if (!table)
            throw std::logic_error("Required movement trace filter table is not bound");
        return reinterpret_cast<engine::MemberFunction<bool, void*, std::uint32_t>>(table[0])(
            receiver, entity, mask);
    }
#if defined(_M_IX86) || defined(__i386__)
    static std::int32_t __thiscall trace_type(void* receiver){
#else
    static std::int32_t trace_type(void* receiver) {
#endif
        const auto& self = *static_cast<LiveSkipFilter*>(receiver);
    const auto table = self.bindings->skip_filter_vtable;
    if (!table)
        throw std::logic_error("Required movement trace filter table is not bound");
    return reinterpret_cast<engine::MemberFunction<std::int32_t>>(table[1])(receiver);
} LiveSkipFilter(MovementCollisionFilter filter, const MovementEngineCollisionBindings& source)
    : prefix{table(), reinterpret_cast<void*>(std::uintptr_t(filter.player)),
             filter.collision_group, filter.extra},
      bindings(&source) {
}

private:
static void* const* table() noexcept {
    static void* const methods[]{reinterpret_cast<void*>(should_hit),
                                 reinterpret_cast<void*>(trace_type)};
    return methods;
}
};
static_assert(offsetof(LiveSkipFilter, prefix) == 0);
}

void EngineMovementCollision::trace_ray(const MovementCollisionRay& source, std::uint32_t mask,
                                        MovementCollisionFilter filter,
                                        MovementCollisionTrace& result) {
    auto ray = native_ray(source);
    switch (filter.kind) {
    case MovementCollisionFilterKind::reject_entities: {
        engine::TraceRejectEntitiesFilter native_filter;
        engine::invoke<void>(bindings_.engine_trace, 5, &ray, mask, &native_filter, &result);
        return;
    }
    case MovementCollisionFilterKind::skip_player: {
        LiveSkipFilter native_filter(filter, bindings_);
        engine::invoke<void>(bindings_.engine_trace, 5, &ray, mask, &native_filter, &result);
        return;
    }
    case MovementCollisionFilterKind::world_only: {
        engine::TraceWorldOnlyFilter native_filter;
        engine::invoke<void>(bindings_.engine_trace, 5, &ray, mask, &native_filter, &result);
        return;
    }
    }
    throw std::logic_error("Invalid movement collision filter kind");
}

float EngineMovementCollision::surface_friction(std::int16_t property) {
    // Every recovered categorization initializes its friction output to one
    // before tracing/calling SurfacePhysics; preserve that initial storage.
    float friction = 1.0f;
    engine::invoke<void>(bindings_.surface_properties, 4, std::int32_t(property),
                         static_cast<float*>(nullptr), static_cast<float*>(nullptr), &friction,
                         static_cast<float*>(nullptr));
    return friction;
}

} //namespace skeet

// ============================================================================
// Subcomponent: extrapolation.cpp
// ============================================================================

namespace skeet {
namespace {
template <class T> T read(const void* object, std::uint32_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}
template <class T> void write(void* object, std::uint32_t offset, const T& value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}
std::int32_t add(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) + std::uint32_t(b));
}
MovementVector movement_vector(const lagcomp::Vector& value) noexcept {
    return {value[0], value[1], value[2]};
}
}

static std::int32_t extrapolate_record(void* player, client::PlayerState& history,
                                       lagcomp::LagRecord& record, std::int32_t ticks,
                                       const MovementExtrapolationBindings& bindings,
                                       const MovementExtrapolationLiveBindings* live,
                                       bool rebuild_pose = false) {
    if (ticks <= 0)
        return 0;
    if (ticks > 32)
        return 0;
    void* const original_state = read<void*>(player, bindings.offsets.animation_state);
    if (!original_state)
        return 0;

    lagcomp::AnimationState saved_animation;
    animation::capture_entity_animation(saved_animation, player, bindings.offsets);
    const auto saved_angles = *engine::invoke<const lagcomp::Vector*>(player, 11);
    alignas(8) std::array<std::byte, animation::copied_animation_state_size> animation_state;
    animation::initialize_animation_state_vector(animation_state.data());
    std::memcpy(animation_state.data(), original_state, animation_state.size());
    write(player, bindings.offsets.animation_state, static_cast<void*>(animation_state.data()));

    //43385AFD independently obtains the actual player's cache. Its raw path
    //must not reuse the owner or snapshot supplied for a different target.
    client::Command command{};
    bool inferred;
    if (live) {
        const MovementExtrapolationCommandOffsets fields{
            bindings.offsets.velocity, bindings.offsets.duck_amount, bindings.offsets.player_flags,
            bindings.field_7090d8dd,   bindings.field_52899df3,      bindings.field_d3d4230d};
        inferred = reconstruct_live_extrapolation_command(command, player, *bindings.players,
                                                          fields, live->environment);
    } else {
        const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(player) + 8, 10);
        if (index == 0)
            throw std::logic_error("Movement extrapolation requires a player index");
        auto& command_history = *bindings.players->get(std::uint32_t(index));
        const MovementExtrapolationCommandPlayer command_player{
            read<MovementVector>(player, bindings.offsets.velocity),
            read<float>(player, bindings.offsets.duck_amount),
            read<std::uint32_t>(player, bindings.offsets.player_flags),
            read<std::uint8_t>(player, bindings.field_7090d8dd),
            read<std::int32_t>(player, bindings.field_52899df3),
            read<float>(player, bindings.field_d3d4230d)};
        inferred = reconstruct_extrapolation_command(
            command, command_history.history, read<float>(&command_history.native, 0x110),
            read<std::int32_t>(&command_history.native, 0x168), command_player,
            bindings.environment.value().physics);
    }
    std::int32_t result = 0;
    if (inferred) {
        MovementSimulation simulation{};
        if (live)
            initialize_live_movement_simulation(
                simulation, player, movement_vector(record.header.origin),
                movement_vector(record.header.velocity), record.header.player_flags,
                std::uint32_t(record.header.simulation_tick) + 1u, live->capture);
        else
            initialize_movement_simulation(
                simulation, bindings.player_handle.value(), movement_vector(record.header.origin),
                movement_vector(record.header.velocity), record.header.player_flags,
                std::uint32_t(record.header.simulation_tick) + 1u, bindings.snapshot.value(),
                bindings.rules.value());
        lagcomp::Vector angles{read<float>(&history.native, 0x1a8), history.native.reference_yaw,
                               0.0f};
        float intermediate_yaw = engine::invoke<const lagcomp::Vector*>(player, 11)->at(1);
        if ((history.native.flags & 0x20u) && history.history.count >= 2) {
            const auto& previous = history.history.at(1).header;
            angles[0] = previous.eye_pitch;
            angles[1] = std::bit_cast<float>(previous.player_history_1ac);
            intermediate_yaw = previous.animation_yaw;
        }
        for (std::int32_t i = 0; i < ticks; ++i) {
            float* const clock = bindings.current_time;
            const float saved_time = *clock;
            *clock = static_cast<float>(std::bit_cast<std::int32_t>(simulation.mode)) *
                     (*bindings.interval);
            if (live)
                simulate_live_movement_tick(simulation, *bindings.collision, live->environment,
                                            player, command);
            else
                simulate_movement_tick(simulation, *bindings.collision,
                                       bindings.environment.value(), bindings.player_handle.value(),
                                       command.movement,
                                       {command.angles[0], command.angles[1], command.angles[2]});
            auto update_angles = angles;
            if (i != ticks - 1)
                update_angles[1] = intermediate_yaw;

            const auto velocity = read<MovementVector>(player, bindings.offsets.velocity);
            write(player, bindings.offsets.velocity, simulation.velocity);
            const auto velocity_7a = read<MovementVector>(player, bindings.velocity_7a2d60c1);
            write(player, bindings.velocity_7a2d60c1, simulation.velocity);
            const auto guard_offset = bindings.offsets.transform_guard_49d93e62 - 0x14u;
            const auto guard = read<std::uint32_t>(player, guard_offset);
            const auto origin_4e = read<MovementVector>(player, bindings.origin_4e66bb34);
            write(player, bindings.origin_4e66bb34, simulation.position);
            const auto origin_2a = read<MovementVector>(player, bindings.origin_2a671b42);
            write(player, bindings.origin_2a671b42, simulation.position);
            const auto flags = read<std::uint32_t>(player, bindings.offsets.player_flags);
            write(player, bindings.offsets.player_flags, simulation.player_flags);
            write(player, guard_offset, UINT32_MAX);
            update_angles[1] = normalize_movement_angle(update_angles[1]);
            const animation::AnimationUpdateTiming timing{
                *bindings.animation_frame, *clock, *bindings.interval, bindings.animation_reset};
            animation::update_animation_state(animation_state.data(), update_angles, timing,
                                              bindings.animation_engine);
            write(player, bindings.offsets.player_flags, flags);
            write(player, bindings.origin_2a671b42, origin_2a);
            write(player, bindings.origin_4e66bb34, origin_4e);
            write(player, guard_offset, guard);
            write(player, bindings.velocity_7a2d60c1, velocity_7a);
            write(player, bindings.offsets.velocity, velocity);
            *clock = saved_time;
        }
        record.header.simulation_tick = add(record.header.simulation_tick, ticks);
        record.header.receive_tick = add(record.header.receive_tick, ticks);
        record.header.origin = {simulation.position.x, simulation.position.y,
                                simulation.position.z};
        record.header.duck_amount = simulation.duck_amount;
        record.header.player_flags = simulation.player_flags;
        record.header.collision_max_z = (simulation.player_flags & 2u) ? 54.0f : 72.0f;
        record.header.velocity = {simulation.velocity.x, simulation.velocity.y,
                                  simulation.velocity.z};
        record.header.animation_speed_factor =
            lagcomp::animation_speed_factor(animation_state.data());
        animation::capture_entity_animation(record.animation, player, bindings.offsets);
        record.header.flags &= 0xffffff7bu;
        animation::restore_entity_animation(saved_animation, player, bindings.offsets);
        if (bindings.set_absolute_angles)
            bindings.set_absolute_angles(player, saved_angles);
        result = ticks;
    }
    write(player, bindings.offsets.animation_state, original_state);
    animation::release_animation_state_vector(animation_state.data(), bindings.allocator);
    return result;
}

std::int32_t extrapolate_lag_record(void* player, client::PlayerState& history,
                                    lagcomp::LagRecord& record, std::int32_t ticks,
                                    const MovementExtrapolationBindings& bindings) {
    return extrapolate_record(player, history, record, ticks, bindings, nullptr, false);
}
std::int32_t extrapolate_lag_record(void* player, client::PlayerState& history,
                                    lagcomp::LagRecord& record, std::int32_t ticks,
                                    const MovementExtrapolationBindings& bindings,
                                    const MovementExtrapolationLiveBindings& live,
                                    bool rebuild_pose) {
    return extrapolate_record(player, history, record, ticks, bindings, &live, rebuild_pose);
}
} // namespace skeet

// ============================================================================
// Subcomponent: extrapolation_command.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;
namespace {
/* deduplicated helper */
struct LiveCommandPlayer {
    void* player;
    const MovementExtrapolationCommandOffsets& offsets;
    const MovementEnvironmentBindings& environment;
};
const void* convar_parent(const void* object) noexcept {
    for (;;) {
        const auto parent = field<const void*>(object, 0x1c);
        if (parent == object)
            return object;
        object = parent;
    }
}
float convar_float(const void* object) noexcept {
    object = convar_parent(object);
    return std::bit_cast<float>(field<std::uint32_t>(object, 0x2c) ^
                                std::uint32_t(reinterpret_cast<std::uintptr_t>(object)));
}
bool convar_integer(const void* object) noexcept {
    object = convar_parent(object);
    return field<std::uint32_t>(object, 0x30) !=
           std::uint32_t(reinterpret_cast<std::uintptr_t>(object));
}
std::int32_t subtract(std::int32_t a, std::int32_t b) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(a) - std::uint32_t(b));
}
bool walking(const MovementExtrapolationCommandPlayer& player) noexcept {
    return player.field_7090d8dd != 0 || player.field_52899df3 == 1;
}
bool walking(const LiveCommandPlayer& live) noexcept {
    return field<std::uint8_t>(live.player, live.offsets.field_7090d8dd) != 0 ||
           field<std::int32_t>(live.player, live.offsets.field_52899df3) == 1;
}
void infer_walk_button(client::Command& command,
                       const MovementExtrapolationCommandPlayer& player) noexcept {
    if (walking(player))
        command.movement.buttons |= speed_button;
}

// 433F3DC7 uses player flags 2/4 and the two decoded walking fields, rather
// than command buttons or the predicted duck transition used by 433AC9C0.
float player_ground_acceleration(const MovementExtrapolationCommandPlayer& player,
                                 const MovementPhysics& physics, MovementVector velocity,
                                 MovementVector direction, float wish_speed,
                                 const LiveCommandPlayer* live = nullptr) {
    MovementPhysics current_physics = physics;
    float acceleration =
        live ? convar_float(live->environment.convars.accelerate) : physics.accelerate;
    const float current = dot(velocity, direction);
    const float additional = wish_speed - current;
    if (0.0f >= additional)
        return 0.0f;
    bool duck, walk;
    if (live) {
        //433F3E7B captures duck flags before the state-first walking test.
        duck = (field<std::uint32_t>(live->player, live->offsets.player_flags) & 6U) != 0;
        const auto moving = field<std::int32_t>(live->player, live->offsets.field_52899df3) == 1 ||
                            field<std::uint8_t>(live->player, live->offsets.field_7090d8dd) != 0;
        walk = moving && !duck;
        current_physics.weapon_speed_enabled =
            convar_integer(live->environment.convars.accelerate_weapon_speed);
        current_physics.has_weapon = false;
        if (current_physics.weapon_speed_enabled) {
            const auto handle =
                field<std::int32_t>(live->player, live->environment.fields.weapon_handle_cd812654);
            if (handle != -1) {
                const auto weapon = engine::invoke<void*>(live->environment.entity_list, 4, handle);
                if (weapon) {
                    current_physics.has_weapon = true;
                    current_physics.weapon_max_speed = engine::invoke<float>(weapon, 0x6e8U / 4U);
                    current_physics.weapon_mode_count =
                        engine::invoke<std::int32_t>(weapon, 0x738U / 4U);
                    if (current_physics.weapon_mode_count > 0)
                        current_physics.weapon_mode =
                            engine::invoke<std::int32_t>(weapon, 0x6ecU / 4U);
                }
            }
        }
    } else {
        duck = (player.player_flags & 6u) != 0;
        walk = walking(player) && !duck;
    }
    float limit = 250.0f, basis = 250.0f;
    bool exception = false;
    if (current_physics.weapon_speed_enabled && current_physics.has_weapon) {
        exception = weapon_slowdown_exception(current_physics);
        const float scale = current_physics.weapon_max_speed / 250.0f;
        if (1.0f >= scale)
            limit = scale * 250.0f;
        if ((!duck && !walk) || exception) {
            if (1.0f >= scale)
                basis *= scale;
        }
    }
    if (duck) {
        if (!exception)
            basis *= 0.34f;
        limit *= 0.34f;
    }
    if (walk) {
        if (!exception)
            basis *= 0.52f;
        limit *= 0.52f;
        acceleration = walking_acceleration(acceleration, current, limit);
    }
    const auto interval = live ? *live->environment.interval : physics.interval_per_tick;
    const float amount = (interval * acceleration) * basis;
    return sse_min(additional, amount);
}
bool infer_history_command(client::Command& command, const lagcomp::RecordRing& history,
                           float history_time, const MovementExtrapolationCommandPlayer& player,
                           const MovementPhysics& physics, const LiveCommandPlayer* live) {
    if (history.count < 1)
        return false;
    const auto& newest = history.at(0).header;
    const MovementVector velocity{newest.velocity[0], newest.velocity[1], newest.velocity[2]};
    command.angles[1] = planar_movement_yaw(velocity);
    if (history.count < 2)
        return false;
    const auto& previous = history.at(1).header;
    if (newest.duck_amount >= 1.0f || newest.duck_amount > previous.duck_amount)
        command.movement.buttons |= duck_button;
    if (!(newest.player_flags & on_ground))
        return true;
    if (live) {
        if (walking(*live))
            command.movement.buttons |= speed_button;
    } else
        infer_walk_button(command, player);
    const auto interval = live ? *live->environment.interval : physics.interval_per_tick;
    const auto player_time =
        live ? field<float>(live->player, live->offsets.field_d3d4230d) : player.field_d3d4230d;
    if (lagcomp::time_to_tick(player_time, interval) !=
        lagcomp::time_to_tick(history_time, interval))
        return true;

    const float speed = horizontal_length(velocity);
    const float previous_speed = std::sqrt(previous.velocity[0] * previous.velocity[0] +
                                           previous.velocity[1] * previous.velocity[1]);
    const float difference = speed - previous_speed;
    if (5.0f > absolute(difference)) {
        command.movement.forward_move = speed;
        return true;
    }
    float old_x = previous.velocity[0], old_y = previous.velocity[1];
    float decayed = previous_speed;
    if (decayed > 0.1f) {
        const auto delta = subtract(newest.simulation_tick, previous.simulation_tick);
        const auto ticks = delta > 16 ? 16 : delta;
        const auto friction =
            live ? convar_float(live->environment.convars.friction) : physics.friction;
        const float decay_scale = interval * friction;
        for (std::int32_t i = 0; i < ticks; ++i) {
            const auto stop =
                live ? convar_float(live->environment.convars.stop_speed) : physics.stop_speed;
            const auto control = maximum_number(stop, decayed);
            const auto drop = control * decay_scale;
            decayed = maximum_number(decayed - drop, 0.0f);
            if (0.1f > decayed) {
                decayed = 0.0f;
                break;
            }
        }
        if (decayed != previous_speed) {
            const float scale = decayed / previous_speed;
            old_y *= scale;
            old_x *= scale;
        }
    } else
        decayed = 0.0f;
    if (5.0f >= speed - decayed)
        return true;
    const float dx = velocity.x - old_x, dy = velocity.y - old_y;
    const float squared = (dx * dx + dy * dy) + 0.0f * 0.0f;
    const float reciprocal = 1.0f / (std::sqrt(squared) + epsilon);
    const MovementVector direction{reciprocal * dx, reciprocal * dy, reciprocal * 0.0f};
    const MovementVector acceleration_velocity =
        live ? MovementVector{newest.velocity[0], newest.velocity[1], newest.velocity[2]}
             : velocity;
    const auto speed_physics =
        live ? capture_movement_command_physics(live->player, live->environment) : physics;
    const float limit =
        minimum_number(speed_physics.player_speed_limit, speed_physics.player_max_speed);
    const float acceleration =
        player_ground_acceleration(player, physics, acceleration_velocity, direction, limit, live);
    command.movement.forward_move = difference >= acceleration * 0.95f ? limit : speed;
    return true;
}
}

bool infer_history_movement_command(client::Command& command, const lagcomp::RecordRing& history,
                                    float history_time,
                                    const MovementExtrapolationCommandPlayer& player,
                                    const MovementPhysics& physics) noexcept {
    return infer_history_command(command, history, history_time, player, physics, nullptr);
}

bool reconstruct_extrapolation_command(client::Command& command, const lagcomp::RecordRing& history,
                                       float history_time, std::int32_t history_tick,
                                       const MovementExtrapolationCommandPlayer& player,
                                       const MovementPhysics& physics) noexcept {
    std::memset(&command, 0, sizeof command);
    const auto tick = lagcomp::time_to_tick(history_time, physics.interval_per_tick);
    const bool recent = history.count >= 2 && !(tick < subtract(history_tick, 1) &&
                                                tick < history.at(0).header.simulation_tick);
    if (recent)
        return infer_history_movement_command(command, history, history_time, player, physics);
    command.angles[1] = planar_movement_yaw(player.velocity);
    if (player.duck_amount >= 1.0f)
        command.movement.buttons |= duck_button;
    if (player.player_flags & on_ground) {
        infer_walk_button(command, player);
        if (lagcomp::time_to_tick(player.field_d3d4230d, physics.interval_per_tick) == tick)
            command.movement.forward_move = horizontal_length(player.velocity);
    }
    return true;
}

bool reconstruct_live_extrapolation_command(client::Command& command, void* player,
                                            client::PlayerStateCache& players,
                                            const MovementExtrapolationCommandOffsets& offsets,
                                            const MovementEnvironmentBindings& environment) {
    const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(player) + 8, 10);
    if (index == 0)
        throw std::logic_error("Movement extrapolation requires a player index");
    auto& owner = *players.get(std::uint32_t(index));
    std::memset(&command, 0, sizeof command);
    const auto& history = owner.history;
    bool recent = false;
    if (history.count >= 2) {
        const auto tick =
            lagcomp::time_to_tick(field<float>(&owner.native, 0x110), *environment.interval);
        recent = !(tick < subtract(field<std::int32_t>(&owner.native, 0x168), 1) &&
                   tick < history.at(0).header.simulation_tick);
    }
    const LiveCommandPlayer live{player, offsets, environment};
    if (recent)
        return infer_history_command(command, history, field<float>(&owner.native, 0x110), {},
                                     MovementPhysics{}, &live);
    const auto velocity = field<MovementVector>(player, offsets.velocity);
    command.angles[1] = planar_movement_yaw(velocity);
    if (field<float>(player, offsets.duck_amount) >= 1.0f)
        command.movement.buttons |= duck_button;
    if (field<std::uint32_t>(player, offsets.player_flags) & on_ground) {
        if (walking(live))
            command.movement.buttons |= speed_button;
        const auto interval = *environment.interval;
        if (lagcomp::time_to_tick(field<float>(player, offsets.field_d3d4230d), interval) ==
            lagcomp::time_to_tick(field<float>(&owner.native, 0x110), interval))
            command.movement.forward_move =
                horizontal_length(field<MovementVector>(player, offsets.velocity));
    }
    return true;
}

} // namespace skeet

// ============================================================================
// Subcomponent: global_config.cpp
// ============================================================================

namespace skeet {
void initialize_infinite_duck_config(ConfigRegistries& registries,
                                     BooleanConfig& setting) noexcept {
    append_config_link(registries.descriptors, setting);
}
void initialize_air_duck_config(ConfigRegistries& registries, IntegerConfig& setting) noexcept {
    append_config_link(registries.descriptors, setting);
    setting.value = 0;
}
void initialize_edge_jump_hotkey(ConfigRegistries& registries, HotkeyConfig& setting) noexcept {
    append_config_link(registries.descriptors, setting);
    auto& hotkey = setting.value;
    append_hotkey(registries.hotkeys, hotkey);
    hotkey.callback_object = nullptr;
    hotkey.callback = nullptr;
    hotkey.binding.packed = (hotkey.binding.packed & 1u) | 0x18u;
    hotkey.binding.mode = HotkeyMode::hold;
}
} //namespace skeet

// ============================================================================
// Subcomponent: ground_contact.cpp
// ============================================================================

namespace skeet {
namespace {
bool valid_plane(const MovementCollisionTrace& trace) noexcept {
    return trace.entity && trace.plane_normal.z >= 0.7f;
}
void trace_hull(MovementSimulation& state, MovementCollisionEngine& engine, MovementVector end,
                MovementVector minimum, MovementVector maximum, MovementCollisionTrace& trace) {
    engine.trace_ray(make_movement_hull_ray(state.position, end, minimum, maximum),
                     state.collision_mask,
                     {MovementCollisionFilterKind::skip_player, state.player, 8, 0}, trace);
}
void trace_ground(MovementSimulation& state, MovementCollisionEngine& engine, MovementVector end,
                  MovementCollisionTrace& trace) {
    using namespace movement_detail;
    trace_hull(state, engine, end, state.hull_minimum, state.hull_maximum, trace);
    if ((trace.fraction < 1.0f || trace.all_solid || trace.start_solid) && valid_plane(trace))
        return;
    const float fraction = trace.fraction;
    const MovementVector original_end = trace.end;
    const auto minimum = state.hull_minimum;
    const auto maximum = state.hull_maximum;
    MovementVector low_maximum{minimum_number(maximum.x, 0.0f), minimum_number(maximum.y, 0.0f),
                               maximum.z};
    MovementVector high_minimum{maximum_number(minimum.x, 0.0f), maximum_number(minimum.y, 0.0f),
                                minimum.z};
    trace_hull(state, engine, end, minimum, low_maximum, trace);
    if (!valid_plane(trace)) {
        trace_hull(state, engine, end, high_minimum, maximum, trace);
        if (!valid_plane(trace)) {
            trace_hull(state, engine, end, {minimum.x, high_minimum.y, minimum.z},
                       {low_maximum.x, maximum.y, maximum.z}, trace);
            if (!valid_plane(trace))
                trace_hull(state, engine, end, {high_minimum.x, minimum.y, minimum.z},
                           {maximum.x, low_maximum.y, maximum.z}, trace);
        }
    }
    trace.fraction = fraction;
    trace.end = original_end;
}
}

bool categorize_movement_ground(MovementSimulation& state, MovementCollisionEngine& engine,
                                bool extend_grounded_drop, bool mark_hard_landing) {
    state.surface_friction = 1.0f;
    const bool was_grounded = (state.player_flags & on_ground) != 0;
    const float vertical = state.velocity.z;
    bool moving_up = vertical > 140.0f;
    if (moving_up && was_grounded)
        moving_up = vertical - vertical > 140.0f;
    const bool extend = extend_grounded_drop && state.movement_type == 2 && was_grounded;
    MovementVector end = state.position;
    end.z -= 2.0f;
    if (extend)
        end.z -= state.rules.step_size;
    if (moving_up || (vertical > 0.0f && state.movement_type == 9)) {
        state.player_flags &= ~on_ground;
        return false;
    }
    MovementCollisionTrace trace{};
    trace.fraction = 1.0f;
    trace.hitgroup = trace.hitbox = -1;
    trace_ground(state, engine, end, trace);
    const bool grounded =
        (trace.fraction < 1.0f || trace.all_solid || trace.start_solid) && valid_plane(trace);
    if (!grounded) {
        state.player_flags &= ~on_ground;
        if (vertical > 0.0f && state.movement_type != 8)
            state.surface_friction = 0.25f;
        return false;
    }
    if (mark_hard_landing && vertical < -580.0f)
        state.simulation_flags |= 0x10u;
    state.player_flags |= on_ground;
    state.surface_friction = engine.surface_friction(trace.surface_property) * 1.25f;
    if (state.surface_friction > 1.0f)
        state.surface_friction = 1.0f;
    if (state.movement_type != 8)
        state.velocity.z = 0.0f;
    if (extend && !trace.start_solid && trace.fraction > 0.0f && trace.fraction < 1.0f)
        state.position = trace.end;
    return true;
}

void prepare_movement_simulation_ground(MovementSimulation& state,
                                        MovementCollisionEngine& engine) {
    if (state.movement_type == 2) {
        if (state.velocity.z > 250.0f)
            state.player_flags &= ~on_ground;
        return;
    }
    categorize_movement_ground(state, engine, false);
}

} // namespace skeet

// ============================================================================
// Subcomponent: ground_phase.cpp
// ============================================================================

namespace skeet {

GroundPhaseContinuation
process_ground_phase(const GroundPhaseSettings& settings, InputBitmap input,
                     const MovementPhysics& physics, bool has_weapon, std::int32_t weapon_state,
                     bool weapon_blocked, bool& crouch_cycle_active, std::uint32_t& scheduler_flags,
                     std::int32_t command_number, UserCommand& command, MovementVector velocity,
                     MovementAngles command_angles, MovementAngleView movement_angles) {
    if (!(physics.player_flags & on_ground) || (command.buttons & jump_button)) {
        crouch_cycle_active = false;
        return GroundPhaseContinuation::player_movement;
    }
    const auto stop = process_quick_stop_stage(settings.quick_stop, input, physics, has_weapon,
                                               scheduler_flags, command, velocity, movement_angles);
    if (stop == QuickStopContinuation::player_movement)
        return GroundPhaseContinuation::player_movement;
    if (stop == QuickStopContinuation::complete)
        return GroundPhaseContinuation::complete;
    if (process_slow_motion_stage(settings.slow_motion, input, physics, weapon_state,
                                  weapon_blocked, scheduler_flags, command_number, command,
                                  velocity, command_angles, movement_angles))
        return GroundPhaseContinuation::complete;
    return GroundPhaseContinuation::weapon_speed_features;
}

} // namespace skeet

// ============================================================================
// Subcomponent: ground_phase_live.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;
namespace {
/* deduplicated helper */
float periodic_wave(const client::Command& command, float interval, float rate,
                    float amplitude) noexcept {
    const float phase = (static_cast<float>(command.number % 1000) * interval) * rate;
    float wave = phase - static_cast<float>(std::floor(static_cast<double>(phase)));
    wave = absolute(wave - 0.5f);
    wave += wave;
    return wave * amplitude;
}
}

GroundPhaseContinuation process_live_ground_phase(MovementCommandOwner& owner, void* player,
                                                  void* weapon, client::Command& command,
                                                  MovementAngleView angles,
                                                  const MovementCommandBindings& b) {
    if (!(movement_command_player_flags(player, b) & on_ground) ||
        (command.movement.buttons & jump_button)) {
        owner.duck_peek.crouch_active = false;
        return GroundPhaseContinuation::player_movement;
    }
    if (!weapon)
        return GroundPhaseContinuation::player_movement;
    const bool quick_stop = owner.config.standalone_quick_stop.value ||
                            (owner.quick_peek_config.enabled.value &&
                             hotkey_active(owner.quick_peek_config.hotkey.value.binding, b.input));
    if (quick_stop) {
        const auto velocity = movement_command_velocity(player, b);
        const float squared =
            (velocity.x * velocity.x + velocity.y * velocity.y) + velocity.z * velocity.z;
        const float input = command.movement.forward_move * command.movement.forward_move +
                            command.movement.side_move * command.movement.side_move;
        if (squared > 4.0f && 0.01f > input && !(command.movement.buttons & 0x61au)) {
            if (movement_command_player_flags(player, b) & on_ground) {
                if (write_live_movement_counter(player, command.movement,
                                                movement_command_velocity(player, b), angles, b))
                    b.scheduler_flags |= 1u;
            }
            return GroundPhaseContinuation::complete;
        }
    }
    const bool slow =
        owner.config.slow_motion.value && hotkey_active(b.slow_motion_hotkey, b.input);
    if (!slow && !(b.scheduler_flags & 0x40u))
        return GroundPhaseContinuation::weapon_speed_features;
    if ((b.scheduler_flags & 8u) && b.weapon_state != -1 && !(b.weapon_state & 4))
        return GroundPhaseContinuation::weapon_speed_features;
    b.scheduler_flags =
        (b.scheduler_flags & 0xfffedfe7u) | ((b.scheduler_flags & 8u) << 10) | 0x80u;
    if (movement_command_weapon_blocked(weapon, b))
        return GroundPhaseContinuation::complete;
    const auto maximum = capture_movement_command_speed(player, b);
    const float ceiling =
        (minimum_number(maximum.player_speed_limit, maximum.player_max_speed) - 1.0f) * 0.34f;
    const float wave = periodic_wave(command, *b.environment.interval, 7.0f, 4.0f);
    regulate_live_movement_speed(player, command.movement, movement_command_velocity(player, b),
                                 command.angles, angles, ceiling - wave, true, b);
    return GroundPhaseContinuation::complete;
}

void process_live_ground_speed_tail(const MovementCommandOwner& owner, void* player,
                                    client::Command& command, MovementAngleView angles,
                                    const MovementCommandBindings& b) {
    if (!(movement_command_player_flags(player, b) & on_ground) ||
        field<std::uint8_t>(player, b.fields.player_state_ac7f2913) >= 2 ||
        (command.movement.buttons & 0x2002u))
        return;
    if (b.periodic_gate_4346a740 && b.periodic_gate_43467f60 && b.periodic_gate_434685b8) {
        const auto maximum = capture_movement_command_speed(player, b);
        const float ceiling =
            minimum_number(maximum.player_speed_limit, maximum.player_max_speed) - 1.0f;
        const float wave = periodic_wave(command, *b.environment.interval, 10.0f, 10.0f);
        regulate_live_movement_speed(player, command.movement, movement_command_velocity(player, b),
                                     command.angles, angles, ceiling - wave, false, b);
    }
    if (owner.config.fast_walk.value && !(command.movement.buttons & duck_button) &&
        (command.movement.buttons & speed_button))
        regulate_live_movement_speed(player, command.movement, movement_command_velocity(player, b),
                                     command.angles, angles, 134.0f, true, b);
}

} // namespace skeet

// ============================================================================
// Subcomponent: ground_speed.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;

void process_ground_speed_tail(const GroundSpeedTailSettings& settings,
                               const MovementPhysics& physics, std::uint8_t player_state,
                               std::int32_t command_number, UserCommand& command,
                               MovementVector velocity, MovementAngles command_angles,
                               MovementAngleView movement_angles) noexcept {
    if (!(physics.player_flags & on_ground) || player_state >= 2 || (command.buttons & 0x2002u))
        return;

    if (settings.periodic_speed_enabled && settings.periodic_context_a &&
        settings.periodic_context_b) {
        const float ceiling =
            minimum_number(physics.player_speed_limit, physics.player_max_speed) - 1.0f;
        const float phase =
            (static_cast<float>(command_number % 1000) * physics.interval_per_tick) * 10.0f;
        float wave = phase - static_cast<float>(std::floor(static_cast<double>(phase)));
        wave = absolute(wave - 0.5f);
        wave += wave;
        wave *= 10.0f;
        regulate_movement_speed(physics, command, velocity, command_angles, movement_angles,
                                ceiling - wave, false);
    }

    // This gate reads the command after the preceding regulator has run.
    if (settings.fast_walk && !(command.buttons & duck_button) &&
        (command.buttons & speed_button)) {
        regulate_movement_speed(physics, command, velocity, command_angles, movement_angles, 134.0f,
                                true);
    }
}

} // namespace skeet

// ============================================================================
// Subcomponent: initialization.cpp
// ============================================================================

namespace skeet {

void initialize_movement_state(MovementCommandOwner& state, ConfigRegistries& registry,
                               const void* air_accelerate, const void* jump_speed_cap) {
    state.air_accelerate = air_accelerate;
    state.jump_speed_cap = jump_speed_cap;
    initialize_movement_config(registry, state.config);
    state.previous_buttons_1e0 = 0;
    state.suppressed_buttons_1e4 = 0;
    state.feedback.primary_heading = (std::numeric_limits<float>::max)();
    state.feedback.secondary_heading = (std::numeric_limits<float>::max)();
    state.limits = {845.5f, 90.0f};
    state.side_magnitude = 450.0f;
    state.brake_reduction_limit = 15.0f;
    state.brake_threshold = 30.0f;
    state.angle_step = 60.0f;
    state.field_1d8 = 286.0f;
    initialize_quick_peek_config(registry, state.quick_peek_config);
    initialize_quick_peek_state(state.quick_peek);

    auto& duck = state.duck_peek;
    duck.target = 0.0f;
    duck.field_530 = -1;
    duck.phase = 0;
    duck.choked = 0;
    duck.original_buttons = 0;
    duck.previous_buttons = 0;
    duck.last_key = 27;
    duck.crouch_active = false;
    duck.release_latch = false;
    duck.configured_limit = nullptr;
}

void finish_movement_state_initialization(MovementCommandOwner& state) noexcept {
    state.callback_flags_1dc = 0;
    state.feedback.turn_direction = 0.0f;
    state.feedback.secondary_heading = 0.0f;
    state.feedback.was_grounded = 1;
    state.feedback.prediction_blocked = 0;
}

void reset_movement_state(MovementCommandOwner& state) noexcept {
    state.feedback.turn_direction = 0.0f;
    state.feedback.primary_heading = (std::numeric_limits<float>::max)();
    state.feedback.secondary_heading = (std::numeric_limits<float>::max)();
    state.callback_flags_1dc = 0;
    state.feedback.was_grounded = 1;
    state.previous_buttons_1e0 = 0;
    state.suppressed_buttons_1e4 = 0;
    reset_quick_peek_state(state.quick_peek);
    state.duck_peek.target = 0.0f;
    state.duck_peek.phase = 0;
    state.duck_peek.crouch_active = false;
    state.duck_peek.last_key = 27;
}

void bind_movement_settings(MovementCommandOwner& state, ConfigRegistries& registry) {
    state.duck_peek.configured_limit =
        &static_cast<IntegerConfig*>(find_config(registry.descriptors, 0xF17FEF11u))->value;
}

} //namespace skeet

// ============================================================================
// Subcomponent: normalized_air_final.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;

/* deduplicated helper */

std::int32_t truncate_integer(float value) noexcept {
    if (!(value >= -2147483648.0f && value < 2147483648.0f))
        return std::numeric_limits<std::int32_t>::min();
    return static_cast<std::int32_t>(value);
}

float effective_maximum_speed(float stamina, const UserCommand& command,
                              const NormalizedAirFinalBindings& bindings) {
    // The virtual getter precedes all property reads at every invocation.
    const float maximum = engine::invoke<float>(bindings.player, 0x450u / 4u);
    float result = minimum_number(
        field<float>(bindings.player, bindings.environment.fields.speed_limit_3eb71843), maximum);
    if (stamina > 0.0f) {
        const float scale = sse_min(sse_max(1.0f - stamina / 100.0f, 0.0f), 1.0f);
        result *= scale * scale;
    }
    const auto& fields = bindings.environment.fields;
    if ((command.buttons & duck_button) ||
        field<std::uint8_t>(bindings.player, fields.ducking_6b1a59b5) ||
        (field<std::uint8_t>(bindings.player, fields.player_flags_e2fbe695) & 2u)) {
        const float amount = field<float>(bindings.player, fields.duck_amount_3309c0cf);
        const float scale =
            (amount * 0.34f + 1.0f) - field<float>(bindings.player, fields.duck_amount_3309c0cf);
        result *= scale;
    }
    return result;
}

struct SteeringAngles {
    float minimum, wide;
};

SteeringAngles steering_angles(const NormalizedAirFinalSettings& settings, float speed,
                               float maximum, const MovementSimulation& simulation,
                               const NormalizedAirFinalBindings& bindings) {
    const float first_friction = simulation.surface_friction;
    const float first_basis = maximum_number(settings.acceleration_limit, speed);
    const float first_acceleration =
        ((convar_float(bindings.air_acceleration) * maximum) * *bindings.environment.interval) *
        first_friction;
    const float first_cosine = sse_min(
        sse_max((settings.acceleration_limit - first_acceleration) / first_basis, -1.0f), 1.0f);
    const float minimum =
        minimum_number(std::acos(first_cosine) * radians_to_degrees, settings.maximum_angle);
    const float second_friction = simulation.surface_friction;
    const float second_basis = maximum_number(settings.acceleration_limit, speed);
    const float second_acceleration =
        ((convar_float(bindings.air_acceleration) * maximum) * *bindings.environment.interval) *
        second_friction;
    const float second_cosine =
        sse_min(sse_max(negate(minimum_number(settings.angle_step, second_acceleration) /
                               (second_basis * 2.0f)),
                        -1.0f),
                1.0f);
    return {minimum, std::acos(second_cosine) * radians_to_degrees};
}

SteeringAngles live_steering_angles(const NormalizedAirFinalSettings& settings, float speed,
                                    float maximum, const NormalizedAirFinalBindings& bindings) {
    // The two reads bracket the first acos/minimum operation in the native block.
    const float first_friction =
        field<float>(bindings.player, bindings.capture.offsets.field_f3c10e80 + 0x14u);
    const float first_basis = maximum_number(settings.acceleration_limit, speed);
    const float first_acceleration =
        ((convar_float(bindings.air_acceleration) * maximum) * *bindings.environment.interval) *
        first_friction;
    const float first_cosine = sse_min(
        sse_max((settings.acceleration_limit - first_acceleration) / first_basis, -1.0f), 1.0f);
    const float minimum =
        minimum_number(std::acos(first_cosine) * radians_to_degrees, settings.maximum_angle);
    const float second_friction =
        field<float>(bindings.player, bindings.capture.offsets.field_f3c10e80 + 0x14u);
    const float second_basis = maximum_number(settings.acceleration_limit, speed);
    const float second_acceleration =
        ((convar_float(bindings.air_acceleration) * maximum) * *bindings.environment.interval) *
        second_friction;
    const float second_cosine =
        sse_min(sse_max(negate(minimum_number(settings.angle_step, second_acceleration) /
                               (second_basis * 2.0f)),
                        -1.0f),
                1.0f);
    return {minimum, std::acos(second_cosine) * radians_to_degrees};
}

bool forecast_collision(const MovementSimulation& simulation, MovementVector previous_position,
                        const NormalizedAirFinalBindings& bindings) {
    MovementTraceFlagGuard guard(bindings.environment.engine_trace_flag);
    const MovementVector displacement{simulation.contact_position.x - previous_position.x,
                                      simulation.contact_position.y - previous_position.y,
                                      simulation.contact_position.z - previous_position.z};
    const float distance =
        std::sqrt((displacement.z * displacement.z + displacement.y * displacement.y) +
                  displacement.x * displacement.x);
    const float scale = (distance + 24.0f) / (distance + epsilon);
    const MovementVector extended{displacement.x * scale, displacement.y * scale,
                                  displacement.z * scale};
    const MovementVector maximum =
        *engine::invoke<const MovementVector*>(bindings.player, 0x134u / 4u);
    const MovementVector start{previous_position.x, previous_position.y,
                               previous_position.z + 18.0f};
    const MovementVector end{extended.x + previous_position.x, extended.y + previous_position.y,
                             extended.z + start.z};
    const MovementVector minimum =
        *engine::invoke<const MovementVector*>(bindings.player, 0x130u / 4u);
    const MovementVector top{maximum.x, maximum.y, 24.0f};
    const MovementCollisionFilter filter{MovementCollisionFilterKind::reject_entities, 0, 0, 0};
    MovementCollisionTrace trace{};
    trace.fraction = 1.0f;
    trace.hitgroup = trace.hitbox = -1;
    bindings.collision.trace_ray(make_movement_hull_ray(start, end, minimum, top), 0x200400bu,
                                 filter, trace);
    if (trace.plane_normal.z != 1.0f &&
        ((trace.contents & 0x20000000u) ||
         surface_blocks_steering(bindings.surface_properties, trace.surface_property)))
        return false;
    const auto mask = engine::invoke<std::uint32_t>(bindings.player, 0x260u / 4u) & 0xfffeffffu;
    const MovementVector second_minimum =
        *engine::invoke<const MovementVector*>(bindings.player, 0x130u / 4u);
    bindings.collision.trace_ray(make_movement_hull_ray(start, end, second_minimum, top), mask,
                                 filter, trace);
    return (1.0f > trace.fraction || trace.all_solid || trace.start_solid) &&
           0.1f > absolute(trace.plane_normal.z) &&
           !ballistic_breakable(bindings.ballistics, trace.entity);
}

bool forecast_turn(const NormalizedAirFinalSettings& settings, const NormalizedAirFinalInput& input,
                   const client::Command& command, ConstMovementAngleView movement_angles,
                   std::int32_t ticks, bool positive, const NormalizedAirFinalBindings& bindings) {
    const auto flags =
        field<std::uint32_t>(bindings.player, bindings.environment.fields.player_flags_e2fbe695);
    MovementSimulation simulation{};
    initialize_live_movement_simulation(simulation, bindings.player, input.position, input.velocity,
                                        flags, 0, bindings.capture);
    auto copy = command;
    copy.angles = {movement_angles.pitch, movement_angles.yaw, movement_angles.roll};
    for (std::int32_t tick = 0; tick < ticks; ++tick) {
        const auto previous_position = simulation.position;
        const float speed = horizontal_length(simulation.velocity);
        const float yaw = planar_movement_yaw(simulation.velocity);
        static_cast<void>(normalize_movement_angle(movement_angles.yaw - yaw));
        const float smoothing =
            sse_min(sse_max(1.0f - static_cast<float>(settings.smoothing) * 0.01f, 0.0f), 1.0f);
        const float maximum = effective_maximum_speed(simulation.stamina, copy.movement, bindings);
        const auto angles = steering_angles(settings, speed, maximum, simulation, bindings);
        const float step = (angles.wide - angles.minimum) * smoothing + angles.minimum;
        copy.angles[1] = positive ? yaw + step : yaw - step;
        copy.movement.forward_move = settings.forward_magnitude;
        simulate_live_movement_tick(simulation, bindings.collision, bindings.environment,
                                    bindings.player, copy);
        if (simulation.simulation_flags & 2u)
            return forecast_collision(simulation, previous_position, bindings);
        copy.number = std::bit_cast<std::int32_t>(std::uint32_t(copy.number) + 1u);
    }
    return false;
}

} // namespace

void finish_normalized_air_strafe(const NormalizedAirFinalSettings& settings,
                                  const NormalizedAirFinalInput& input,
                                  const MovementFeedbackHistory& history,
                                  MovementFeedback& feedback, client::Command& command,
                                  MovementAngleView movement_angles,
                                  const NormalizedAirFinalBindings& bindings) {
    using namespace movement_detail;
    const float difference =
        absolute(normalize_movement_angle(movement_angles.yaw - input.velocity_yaw));
    const float maximum = effective_maximum_speed(input.stamina, command.movement, bindings);
    const auto angles = live_steering_angles(settings, input.speed, maximum, bindings);
    float ideal = angles.minimum;
    if (input.speed >= settings.maximum_angle) {
        const float smoothing =
            sse_min(sse_max(static_cast<float>(settings.smoothing) * 0.01f, 0.0f), 1.0f);
        ideal = (angles.minimum - angles.wide) * smoothing + angles.wide;
    }
    feedback.primary_heading = normalize_movement_angle(movement_angles.yaw);
    float selected = std::numeric_limits<float>::max();
    if (input.selection.pre_speed_requested && !input.selection.pre_speed_effective &&
        input.speed > settings.maximum_angle) {
        selected = angles.minimum;
    } else if ((input.selection.movement_key_mode & 0xffu) && difference > settings.maximum_angle &&
               input.speed > settings.maximum_angle) {
        if (history.secondary_heading != std::numeric_limits<float>::max() &&
            settings.acceleration_limit > absolute(normalize_movement_angle(
                                              history.secondary_heading - movement_angles.yaw))) {
            feedback.secondary_heading = history.secondary_heading;
            feedback.turn_direction = history.turn_direction;
            selected = ideal * history.turn_direction;
        } else {
            const auto ticks = truncate_integer(0.5f / (*bindings.environment.interval) + 0.5f);
            const bool negative_blocked =
                forecast_turn(settings, input, command, movement_angles, ticks, false, bindings);
            const bool positive_blocked =
                forecast_turn(settings, input, command, movement_angles, ticks, true, bindings);
            if (negative_blocked != positive_blocked) {
                feedback.secondary_heading = movement_angles.yaw;
                feedback.turn_direction = negative_blocked ? 1.0f : -1.0f;
                selected = ideal * feedback.turn_direction;
            } else if (difference > 160.0f) {
                selected = ideal;
            }
        }
    }
    // The native UCOMISS gate accepts unordered candidates and rejects only
    // the ordered FLT_MAX sentinel before considering the nearest heading.
    if (selected == std::numeric_limits<float>::max()) {
        selected = difference > 20.0f ? ideal : angles.minimum;
        const float negative = absolute(
            normalize_movement_angle((input.velocity_yaw - selected) - movement_angles.yaw));
        const float positive = absolute(
            normalize_movement_angle((selected + input.velocity_yaw) - movement_angles.yaw));
        if (positive > negative)
            selected = negate(selected);
    }
    movement_angles.yaw = input.velocity_yaw + selected;
    command.movement.forward_move = settings.forward_magnitude;
    movement_angles.yaw = normalize_movement_angle(movement_angles.yaw);
}

} // namespace skeet

// ============================================================================
// Subcomponent: normalized_air_solver.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;

bool process_normalized_air_strafe(const NormalizedAirStrafeSettings& settings,
                                   const AirStrafeSelection& selection, InputBitmap input,
                                   const NormalizedAirStrafeFrame& frame,
                                   const MovementFeedbackHistory& history,
                                   MovementFeedback& feedback, client::Command& command,
                                   MovementAngleView angles,
                                   const NormalizedAirStrafeBindings& bindings) {
    const auto& steering = settings.steering;
    const auto velocity = frame.collision.velocity;
    if (!prepare_air_strafe_heading(selection, velocity, true, command.movement, angles))
        return false;
    auto wall = frame.collision;
    wall.maximum_angle = steering.limits.maximum_angle;
    wall.side_magnitude = steering.side_magnitude;
    wall.smoothing = steering.smoothing;
    wall.acceleration_limit = steering.feedback_turn_limit;
    wall.angle_step = settings.angle_step;
    wall.pre_speed_effective = selection.pre_speed_effective;
    const float velocity_yaw =
        steer_normalized_air_wall(wall, feedback, command, angles, bindings.wall);
    const float speed = horizontal_length(velocity);
    if (selection.pre_speed_requested) {
        if (selection.pre_speed_effective) {
            const auto& snapshot = bindings.wall.snapshot;
            const auto& rules = bindings.wall.rules;
            const PreSpeedProbeInput forecast{frame.forecast_position,
                                              velocity,
                                              frame.forecast_player_flags,
                                              rules.gravity,
                                              rules.jump_impulse,
                                              rules.interval_per_tick,
                                              0.0f,
                                              0.0f,
                                              snapshot.hull_minimum,
                                              snapshot.hull_maximum,
                                              snapshot.collision_mask,
                                              wall.player};
            const RawPreSpeedInput pre_speed{steering.limits, forecast, wall.position,
                                             bindings.wall.environment.physics.player_speed_limit,
                                             history.primary_heading};
            angles.yaw =
                choose_raw_pre_speed_heading(pre_speed, angles.yaw, bindings.wall.collision,
                                             bindings.wall.environment.engine_trace_flag);
            const float ideal_step = minimum_number(
                steering.limits.maximum_angle, steering.limits.angle_scale / (speed + epsilon));
            if (resolve_pre_speed_feedback(history.primary_heading, ideal_step,
                                           steering.feedback_turn_limit, steering.side_magnitude,
                                           feedback, command.movement, angles))
                return true;
        }
    } else
        apply_z_hop_heading(steering.z_hop, input, frame.engine_time, angles);
    const NormalizedAirFinalSettings final_settings{
        steering.smoothing, steering.limits.maximum_angle, steering.side_magnitude,
        steering.feedback_turn_limit, settings.angle_step};
    const NormalizedAirFinalInput final_input{wall.position, velocity, frame.stamina,
                                              velocity_yaw,  speed,    selection};
    finish_normalized_air_strafe(final_settings, final_input, history, feedback, command, angles,
                                 bindings.final);
    return true;
}

} // namespace skeet

// ============================================================================
// Subcomponent: normalized_air_wall.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;

/* deduplicated helper */

float choose_prediction_heading(float velocity_yaw, float step, float previous_yaw) noexcept {
    const float negative = velocity_yaw - step;
    const float positive = velocity_yaw + step;
    const float negative_difference = absolute(normalize_movement_angle(negative - previous_yaw));
    const float positive_difference = absolute(normalize_movement_angle(positive - previous_yaw));
    // COMISS/JBE selects the positive candidate on ties and unordered values.
    return positive_difference > negative_difference ? negative : positive;
}

float live_prediction_yaw(const MovementSimulation& simulation, const client::Command& command,
                          const AirWallSteeringLiveBindings& bindings,
                          const void* const& air_acceleration) {
    const float velocity_yaw = planar_movement_yaw(simulation.velocity);
    const float previous_yaw = command.angles[1];
    const float difference = normalize_movement_angle(previous_yaw - velocity_yaw);
    const float speed = horizontal_length(simulation.velocity);
    const float smoothing =
        sse_min(sse_max(1.0f - static_cast<float>(bindings.tuning.smoothing) * 0.01f, 0.0f), 1.0f);
    // Stamina is saved before the live virtual450 call in the native block.
    const float stamina = simulation.stamina;
    const float player_maximum = engine::invoke<float>(bindings.player, 0x450u / 4u);
    const auto& fields = bindings.environment.fields;
    float maximum =
        minimum_number(field<float>(bindings.player, fields.speed_limit_3eb71843), player_maximum);
    if (stamina > 0.0f) {
        const float scale = sse_min(sse_max(1.0f - stamina / 100.0f, 0.0f), 1.0f);
        maximum *= scale * scale;
    }
    if ((command.movement.buttons & duck_button) ||
        field<std::uint8_t>(bindings.player, fields.ducking_6b1a59b5) ||
        (field<std::uint8_t>(bindings.player, fields.player_flags_e2fbe695) & 2u)) {
        const float amount = field<float>(bindings.player, fields.duck_amount_3309c0cf);
        maximum *=
            ((amount * 0.34f + 1.0f) - field<float>(bindings.player, fields.duck_amount_3309c0cf));
    }
    const float first_friction = simulation.surface_friction;
    const float first_basis = maximum_number(bindings.tuning.acceleration_limit, speed);
    const float first_acceleration =
        ((convar_float(air_acceleration) * maximum) * *bindings.environment.interval) *
        first_friction;
    const float cosine = sse_min(
        sse_max((bindings.tuning.acceleration_limit - first_acceleration) / first_basis, -1.0f),
        1.0f);
    const float minimum =
        minimum_number(std::acos(cosine) * radians_to_degrees, bindings.tuning.maximum_angle);
    const float second_friction = simulation.surface_friction;
    const float second_basis = maximum_number(bindings.tuning.acceleration_limit, speed);
    const float second_acceleration =
        ((convar_float(air_acceleration) * maximum) * *bindings.environment.interval) *
        second_friction;
    const float alternate =
        sse_min(sse_max(negate(minimum_number(bindings.tuning.angle_step, second_acceleration) /
                               (second_basis * 2.0f)),
                        -1.0f),
                1.0f);
    const float target = std::acos(alternate) * radians_to_degrees;
    const float step =
        absolute(difference) > 10.0f ? (target - minimum) * smoothing + minimum : minimum;
    // The getter may mutate the copied command. The initial difference above
    // is already saved, while the nearest-candidate comparisons read it anew.
    return choose_prediction_heading(velocity_yaw, step, command.angles[1]);
}

}

float normalized_air_prediction_yaw(const NormalizedAirWallInput& input,
                                    const MovementSimulation& simulation,
                                    const UserCommand& command, float previous_yaw,
                                    const MovementPhysics& physics) noexcept {
    using namespace movement_detail;
    const float velocity_yaw = planar_movement_yaw(simulation.velocity);
    const float difference = normalize_movement_angle(previous_yaw - velocity_yaw);
    const float speed = horizontal_length(simulation.velocity);
    const float smoothing =
        sse_min(sse_max(1.0f - static_cast<float>(input.smoothing) * 0.01f, 0.0f), 1.0f);
    float maximum = minimum_number(physics.player_speed_limit, physics.player_max_speed);
    if (simulation.stamina > 0.0f) {
        const float scale = sse_min(sse_max(1.0f - simulation.stamina / 100.0f, 0.0f), 1.0f);
        maximum *= scale * scale;
    }
    if ((command.buttons & duck_button) || physics.ducking || (physics.player_flags & 2u))
        maximum *= ((physics.duck_amount * 0.34f + 1.0f) - physics.duck_amount);
    const float basis = maximum_number(input.acceleration_limit, speed);
    const float acceleration =
        ((input.air_acceleration * maximum) * simulation.rules.interval_per_tick) *
        simulation.surface_friction;
    const float cosine =
        sse_min(sse_max((input.acceleration_limit - acceleration) / basis, -1.0f), 1.0f);
    const float minimum =
        minimum_number(std::acos(cosine) * radians_to_degrees, input.maximum_angle);
    const float alternate = sse_min(
        sse_max(negate(minimum_number(input.angle_step, acceleration) / (basis * 2.0f)), -1.0f),
        1.0f);
    const float target = std::acos(alternate) * radians_to_degrees;
    const float step =
        absolute(difference) > 10.0f ? (target - minimum) * smoothing + minimum : minimum;
    return choose_prediction_heading(velocity_yaw, step, previous_yaw);
}

float steer_normalized_air_wall(const NormalizedAirWallInput& input,
                                const MovementFeedback& feedback, const client::Command& command,
                                MovementAngleView movement_angles,
                                const AirWallSteeringBindings& bindings) {
    using namespace movement_detail;
    const float velocity_yaw = planar_movement_yaw(input.velocity);
    if (input.pre_speed_effective || feedback.prediction_blocked || !input.enabled ||
        (feedback.original_buttons & 0x20026u) != 2u)
        return velocity_yaw;
    const float squared =
        (input.velocity.y * input.velocity.y + 0.0f) + input.velocity.x * input.velocity.x;
    const float reciprocal = 1.0f / (std::sqrt(squared) + epsilon);
    const MovementVector direction{input.velocity.x * reciprocal, input.velocity.y * reciprocal,
                                   0.0f * reciprocal};
    const float radians = movement_angles.yaw * degrees_to_radians;
    const MovementVector forward{std::cos(radians), std::sin(radians), 0.0f};
    if (!(dot(forward, direction) >= 0.0f))
        return velocity_yaw;
    const float seconds = static_cast<float>(input.prediction_milliseconds) * 0.001f;
    const auto ticks = truncate_tick_count(seconds / bindings.rules.interval_per_tick + 0.5f);
    MovementSimulation simulation{};
    initialize_movement_simulation(simulation, input.player, input.position, input.velocity,
                                   input.player_flags, 0, bindings.snapshot, bindings.rules);
    auto simulated_command = command;
    simulated_command.angles = {movement_angles.pitch, movement_angles.yaw, movement_angles.roll};
    if (input.weapon_handle != -1)
        engine::invoke<std::uint32_t>(bindings.entity_list, 4, std::uint32_t(input.weapon_handle));
    for (std::int32_t tick = 0; tick < ticks; ++tick) {
        simulated_command.angles[1] = normalized_air_prediction_yaw(
            input, simulation, simulated_command.movement, simulated_command.angles[1],
            bindings.environment.physics);
        simulated_command.movement.side_move = input.side_magnitude;
        simulate_movement_tick(simulation, bindings.collision, bindings.environment, input.player,
                               simulated_command.movement,
                               {simulated_command.angles[0], simulated_command.angles[1],
                                simulated_command.angles[2]});
        if (simulation.simulation_flags & 2u)
            break;
        simulated_command.number =
            std::bit_cast<std::int32_t>(std::uint32_t(simulated_command.number) + 1u);
    }
    if (!(simulation.simulation_flags & 2u))
        return velocity_yaw;
    MovementTraceFlagGuard trace_flag(bindings.environment.engine_trace_flag);
    auto trace = initial_trace();
    const MovementVector start{input.position.x, input.position.y, input.position.z + 18.0f};
    const MovementVector extended{forward.x * 320.0f, forward.y * 320.0f, forward.z * 320.0f};
    const MovementVector maximum{bindings.snapshot.hull_maximum.x, bindings.snapshot.hull_maximum.y,
                                 24.0f};
    const MovementCollisionFilter filter{MovementCollisionFilterKind::reject_entities, 0, 0, 0};
    auto ray = make_movement_hull_ray(
        start,
        {extended.x * 0.5f + start.x, extended.y * 0.5f + start.y, extended.z * 0.5f + start.z},
        bindings.snapshot.hull_minimum, maximum);
    bindings.collision.trace_ray(ray, 0x200400bu, filter, trace);
    if (trace.plane_normal.z != 1.0f &&
        ((trace.contents & 0x20000000u) ||
         surface_blocks_steering(bindings.surface_properties, trace.surface_property)))
        return velocity_yaw;
    ray = make_movement_hull_ray(start,
                                 {extended.x * seconds + start.x, extended.y * seconds + start.y,
                                  extended.z * seconds + start.z},
                                 bindings.snapshot.hull_minimum, maximum);
    bindings.collision.trace_ray(ray, bindings.snapshot.collision_mask & 0xfffeffffu, filter,
                                 trace);
    if (!(trace.fraction < 1.0f || trace.all_solid || trace.start_solid) ||
        !(absolute(trace.plane_normal.z) < 0.1f) ||
        ballistic_breakable(bindings.ballistics, trace.entity))
        return velocity_yaw;
    write_wall_heading(direction, forward, trace, movement_angles);
    return velocity_yaw;
}

float steer_live_normalized_air_wall(const NormalizedAirWallInput& input,
                                     const MovementFeedback& feedback,
                                     const client::Command& command,
                                     MovementAngleView movement_angles,
                                     const AirWallSteeringLiveBindings& bindings,
                                     const void* const& air_acceleration) {
    using namespace movement_detail;
    const float velocity_yaw = planar_movement_yaw(input.velocity);
    if (input.pre_speed_effective || feedback.prediction_blocked || !input.enabled ||
        (feedback.original_buttons & 0x20026u) != 2u)
        return velocity_yaw;
    const float squared =
        (input.velocity.y * input.velocity.y + 0.0f) + input.velocity.x * input.velocity.x;
    const float reciprocal = 1.0f / (std::sqrt(squared) + epsilon);
    const MovementVector direction{input.velocity.x * reciprocal, input.velocity.y * reciprocal,
                                   0.0f * reciprocal};
    const float radians = movement_angles.yaw * degrees_to_radians;
    const MovementVector forward{std::cos(radians), std::sin(radians), 0.0f};
    if (!(dot(forward, direction) >= 0.0f))
        return velocity_yaw;

    // These getters precede the constructor's own 130/134/260 captures. The
    // first minimum pointer stays live through both traces; maximum is copied.
    const auto* minimum = engine::invoke<const MovementVector*>(bindings.player, 0x130u / 4u);
    const MovementVector trace_position = bindings.origin;
    const MovementVector maximum_snapshot =
        *engine::invoke<const MovementVector*>(bindings.player, 0x134u / 4u);
    const MovementVector start{trace_position.x, trace_position.y, trace_position.z + 18.0f};
    const MovementVector maximum{maximum_snapshot.x, maximum_snapshot.y, 24.0f};
    const MovementVector extended{forward.x * 320.0f, forward.y * 320.0f, forward.z * 320.0f};
    const float seconds = static_cast<float>(bindings.tuning.prediction_milliseconds) * 0.001f;
    const auto flags =
        field<std::uint32_t>(bindings.player, bindings.environment.fields.player_flags_e2fbe695);
    const MovementVector constructor_velocity = bindings.velocity;
    const MovementVector constructor_position = bindings.origin;
    MovementSimulation simulation{};
    initialize_live_movement_simulation(simulation, bindings.player, constructor_position,
                                        constructor_velocity, flags, 0, bindings.capture);
    const auto ticks = truncate_tick_count(seconds / (*bindings.environment.interval) + 0.5f);
    auto simulated_command = command;
    simulated_command.angles = {movement_angles.pitch, movement_angles.yaw, movement_angles.roll};
    const auto weapon_handle =
        field<std::int32_t>(bindings.player, bindings.environment.fields.weapon_handle_cd812654);
    if (weapon_handle != -1)
        engine::invoke<std::uint32_t>(bindings.environment.entity_list, 4,
                                      std::uint32_t(weapon_handle));
    for (std::int32_t tick = 0; tick < ticks; ++tick) {
        simulated_command.angles[1] =
            live_prediction_yaw(simulation, simulated_command, bindings, air_acceleration);
        simulated_command.movement.side_move = bindings.tuning.side_magnitude;
        simulate_live_movement_tick(simulation, bindings.collision, bindings.environment,
                                    bindings.player, simulated_command);
        if (simulation.simulation_flags & 2u)
            break;
        simulated_command.number =
            std::bit_cast<std::int32_t>(std::uint32_t(simulated_command.number) + 1u);
    }
    if (!(simulation.simulation_flags & 2u))
        return velocity_yaw;

    MovementTraceFlagGuard trace_flag(bindings.environment.engine_trace_flag);
    auto trace = initial_trace();
    const MovementCollisionFilter filter{MovementCollisionFilterKind::reject_entities, 0, 0, 0};
    bindings.collision.trace_ray(
        make_movement_hull_ray(
            start,
            {extended.x * 0.5f + start.x, extended.y * 0.5f + start.y, extended.z * 0.5f + start.z},
            *minimum, maximum),
        0x200400bu, filter, trace);
    if (trace.plane_normal.z != 1.0f &&
        ((trace.contents & 0x20000000u) ||
         surface_blocks_steering(bindings.surface_properties, trace.surface_property)))
        return velocity_yaw;
    const auto mask = engine::invoke<std::uint32_t>(bindings.player, 0x260u / 4u) & 0xfffeffffu;
    bindings.collision.trace_ray(
        make_movement_hull_ray(start,
                               {extended.x * seconds + start.x, extended.y * seconds + start.y,
                                extended.z * seconds + start.z},
                               *minimum, maximum),
        mask, filter, trace);
    if (!(trace.fraction < 1.0f || trace.all_solid || trace.start_solid) ||
        !(absolute(trace.plane_normal.z) < 0.1f) ||
        ballistic_breakable(bindings.ballistics, trace.entity))
        return velocity_yaw;
    write_wall_heading(direction, forward, trace, movement_angles);
    return velocity_yaw;
}

} // namespace skeet

// ============================================================================
// Subcomponent: normalized_scout_gate.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;

MovementCollisionRay scout_hull_ray(MovementVector origin, MovementVector delta,
                                    float horizontal_delta_squared, const MovementVector& minimum,
                                    const MovementVector& maximum, bool elevated) noexcept {
    MovementCollisionRay ray;
    ray.delta = delta;
    ray.swept = delta.z * delta.z + horizontal_delta_squared != 0.0f;
    ray.extents = {(maximum.x - minimum.x) * 0.5f, (maximum.y - minimum.y) * 0.5f,
                   (maximum.z - minimum.z) * 0.5f};
    const float squared = (ray.extents.y * ray.extents.y + ray.extents.x * ray.extents.x) +
                          ray.extents.z * ray.extents.z;
    ray.is_ray = static_cast<double>(squared) < 1.0e-6;
    const MovementVector center{(maximum.x + minimum.x) * 0.5f, (minimum.y + maximum.y) * 0.5f,
                                (minimum.z + maximum.z) * 0.5f};
    ray.start = {center.x + origin.x, origin.y + center.y,
                 elevated ? center.z + origin.z : origin.z + center.z};
    ray.start_offset = {center.x * -1.0f, center.y * -1.0f, center.z * -1.0f};
    return ray;
}

bool blocked(const MovementCollisionTrace& trace) noexcept {
    return 1.0f > trace.fraction || trace.all_solid != 0 || trace.start_solid != 0;
}
}

bool pass_normalized_scout_gate(const NormalizedScoutGateFrame& frame,
                                const client::Command& command, MovementCollisionEngine& collision,
                                std::uint8_t* engine_trace_flag) {
    if (!frame.has_weapon || frame.weapon_definition != 0x28u)
        return true;
    const auto& velocity = frame.original_velocity;
    if (!(4.0f > velocity.y * velocity.y + velocity.x * velocity.x))
        return true;
    const auto& movement = command.movement;
    if (movement.buttons & 0x618u)
        return true;
    if (!(4.0f >
          movement.forward_move * movement.forward_move + movement.side_move * movement.side_move))
        return true;

    auto* collideable = engine::invoke<void*>(frame.player, 3);
    const auto* maximum = engine::invoke<const MovementVector*>(collideable, 2);
    collideable = engine::invoke<void*>(frame.player, 3);
    const auto* minimum = engine::invoke<const MovementVector*>(collideable, 1);
    MovementAngles view;
    engine::invoke<void>(frame.engine_client, 18, &view);

    const float yaw = view.yaw * degrees_to_radians;
    const float sine_yaw = std::sin(yaw);
    const float cosine_yaw = std::cos(yaw);
    const float cosine_pitch = std::cos(view.pitch * degrees_to_radians);
    const float x = cosine_pitch * cosine_yaw;
    const float y = cosine_pitch * sine_yaw;
    const float reciprocal = 1.0f / (std::sqrt((x * x + y * y) + 0.0f) + epsilon);
    const auto& origin = frame.origin;
    const float end_z = (reciprocal * 0.0f) * 64.0f + origin.z;
    MovementVector delta{((reciprocal * x) * 64.0f + origin.x) - origin.x,
                         ((reciprocal * y) * 64.0f + origin.y) - origin.y, end_z - origin.z};
    const float horizontal_delta_squared = delta.y * delta.y + delta.x * delta.x;
    MovementCollisionTrace trace{};
    trace.fraction = 1.0f;
    trace.hitgroup = -1;
    trace.hitbox = -1;
    auto ray = scout_hull_ray(origin, delta, horizontal_delta_squared, *minimum, *maximum, false);
    trace_movement_world(collision, ray, 0x1400bu, trace, engine_trace_flag);
    if (!blocked(trace))
        return true;

    MovementVector elevated_origin{origin.x, origin.y, origin.z + 64.0f};
    delta.z = (end_z + 64.0f) - elevated_origin.z;
    ray =
        scout_hull_ray(elevated_origin, delta, horizontal_delta_squared, *minimum, *maximum, true);
    trace_movement_world(collision, ray, 0x1400bu, trace, engine_trace_flag);
    return !blocked(trace);
}

} // namespace skeet

// ============================================================================
// Subcomponent: owner_lifecycle.cpp
// ============================================================================

namespace skeet {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define MOVEMENT_MEMBER __thiscall
#define MOVEMENT_FAST __fastcall
#else
#define MOVEMENT_MEMBER
#define MOVEMENT_FAST
#endif
using MovementOwnerEntry = void(MOVEMENT_MEMBER*)(void*);
struct MovementInstance;
struct MovementHudEndpoint : hud::HudFrameCallback {
    MovementInstance* owner;
};
struct OwnedText {
    char* begin = nullptr;
    char* end = nullptr;
    char* limit = nullptr;
};
void* allocate(engine::JobMemory memory, std::size_t size) {
    if (!size)
        return nullptr;
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
void construct_text(OwnedText& text, const char* value, std::size_t length,
                    engine::JobMemory memory) {
    // Whole433B2213 ->433C1360 for the two nonempty native literals. The
    // allocation includes the trailing zero, and end excludes that zero.
    text.begin = nullptr;
    text.end = nullptr;
    text.limit = nullptr;
    auto* const bytes = static_cast<char*>(allocate(memory, length + 1));
    text.begin = bytes;
    text.end = bytes;
    text.limit = bytes + length + 1;
    std::memmove(bytes, value, length);
    text.end = bytes + length;
    bytes[length] = 0;
}
void destroy_text(OwnedText& text, engine::JobMemory memory) {
    const auto capacity =
        std::bit_cast<std::int32_t>(std::uint32_t(reinterpret_cast<std::uintptr_t>(text.limit)) -
                                    std::uint32_t(reinterpret_cast<std::uintptr_t>(text.begin)));
    if (capacity >= 2 && text.begin)
        free(memory, text.begin);
    //4336B1E9 does not clear the three string words after freeing.
}
void MOVEMENT_MEMBER destruction_entry(void*);
void MOVEMENT_MEMBER binding_entry(void*);
void MOVEMENT_MEMBER reset_entry(void*);
void MOVEMENT_MEMBER refresh_entry(void*);
void MOVEMENT_FAST hud_entry(void*, ui::RenderQueue*, void*, std::int32_t*);
const std::array<MovementOwnerEntry, 9> owner_table{destruction_entry, binding_entry, unchanged,
                                                    unchanged,         unchanged,     reset_entry,
                                                    unchanged,         refresh_entry, unchanged};
const std::array<hud::HudFrameEntry, 1> movement_hud_table{hud_entry};

struct MovementInstance {
    const MovementOwnerEntry* vtable = nullptr;
    client::NativeCommandPhaseReceiver command_receiver;
    MovementHudEndpoint hud{{nullptr}, this};
    MovementOwnerBindings bindings;
    ConfigRegistries& config;
    engine::JobMemory memory;
    OwnedText command_text, label_text; //native full+54C,full+55C

    MovementInstance(const MovementOwnerRegistries& registries, ConfigRegistries& configurations,
                     MovementOwnerBindings context)
        : bindings(context), config(configurations), memory(registries.lifecycle.memory) {
        registries.lifecycle.append(this);
        registries.commands.append(command_receiver, &bindings.command);
        hud::register_hud_frame(registries.hud, &hud);
        vtable = owner_table.data();
        registries.commands.publish(command_receiver, client::CommandOwnerId::movement); //4348FAB6
        hud.vtable = movement_hud_table.data();
        auto& state = bindings.command.state;
        initialize_movement_state(state, config, bindings.air_accelerate, bindings.jump_speed_cap);
        construct_text(command_text, "+duck", 5, memory);
        construct_text(label_text, "DUCK", 4, memory);
        finish_movement_state_initialization(state);
    }
};
void MOVEMENT_MEMBER destruction_entry(void* storage) {
    auto* const owner = static_cast<MovementInstance*>(storage);
    destroy_movement_owner(storage, owner->bindings.active);
}
void MOVEMENT_MEMBER binding_entry(void* storage) {
    auto& owner = *static_cast<MovementInstance*>(storage);
    bind_movement_settings(owner.bindings.command.state, owner.config);
}
void MOVEMENT_MEMBER reset_entry(void* storage) {
    reset_movement_state(static_cast<MovementInstance*>(storage)->bindings.command.state);
}
void MOVEMENT_MEMBER refresh_entry(void* storage) {
    refresh_movement_owner_key(storage);
}
void MOVEMENT_FAST hud_entry(void* callback, ui::RenderQueue* queue, void* player,
                             std::int32_t* cursor) {
    auto& owner = *static_cast<MovementHudEndpoint*>(static_cast<hud::HudFrameCallback*>(callback))->owner;
    draw_movement_owner_hud(owner.bindings.command, owner.bindings.hud, *queue, player, *cursor);
}
#undef MOVEMENT_MEMBER
#undef MOVEMENT_FAST
} // namespace

std::size_t movement_owner_size() noexcept {
    return sizeof(MovementInstance);
}
void construct_movement_owner(void* storage, const MovementOwnerRegistries& registries,
                              ConfigRegistries& config, MovementOwnerBindings bindings) {
    ::new (storage) MovementInstance(registries, config, bindings);
    bindings.active = &bindings.command.state;
}
void* create_movement_owner(const MovementOwnerRegistries& registries, ConfigRegistries& config,
                            MovementOwnerBindings bindings) {
    void* const storage = allocate(registries.lifecycle.memory, movement_owner_size());
    if (storage)
        construct_movement_owner(storage, registries, config, bindings);
    else
        bindings.active = nullptr;
    return storage;
}
void destroy_movement_owner(void* storage, MovementCommandOwner*& active) {
    active = nullptr;
    if (!storage)
        return;
    auto* const owner = static_cast<MovementInstance*>(storage);
    const auto memory = owner->memory;
    destroy_text(owner->label_text, memory);
    destroy_text(owner->command_text, memory);
    owner->~MovementInstance();
    free(memory, storage);
}
void refresh_movement_owner_key(void* storage) {
    auto& owner = *static_cast<MovementInstance*>(storage);
    owner.vtable[5](storage);
    const auto command = engine::invoke<std::int32_t>(
        owner.bindings.engine_client, 170, static_cast<const char*>(owner.command_text.begin),
        std::int32_t(-1), std::int32_t(0), std::int32_t(-1));
    if (command != 0 && command != -1)
        owner.bindings.command.state.duck_peek.last_key =
            engine::invoke<std::int32_t>(owner.bindings.input_system, 46, command);
}
void draw_movement_owner_hud(MovementCommandPhaseOwner& command,
                             const MovementOwnerHudBindings& bindings, ui::RenderQueue& queue,
                             void* player, std::int32_t& cursor) {
    esp::RuntimePlayer local;
    local.engine_entity = player;
    std::memcpy(&local.life_state,
                static_cast<const std::byte*>(player) + bindings.life_state_9d920e35,
                sizeof local.life_state);
    auto& state = command.state;
    auto& duck = state.duck_peek;
    hud::MovementIndicatorState render_state{state.quick_peek, duck.phase, duck.target,
                                             duck.last_key, duck.crouch_active};
    hud::draw_movement_indicators(
        queue, bindings.scripts, bindings.feature_indicators, bindings.infinite_duck, bindings.rage,
        bindings.command_flags, state.quick_peek_config, render_state, local,
        command.bindings.command.input, cursor, bindings.layout, bindings.projection);
}
} // namespace skeet

// ============================================================================
// Subcomponent: player_air_gate.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;

/* deduplicated helper */

float air_accelerate(const PlayerAirGateSettings& settings) noexcept {
    const void* node = settings.air_accelerate;
    for (;;) {
        const void* next = field<const void*>(node, 0x1Cu);
        if (node == next)
            break;
        node = next;
    }
    const auto encoded = field<std::uint32_t>(node, 0x2Cu);
    const auto address = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(node));
    return std::bit_cast<float>(encoded ^ address);
}

MovementAngles counter_angles(MovementVector direction) noexcept {
    if (direction.x == 0.0f && direction.y == 0.0f)
        return {direction.z > 0.0f ? 270.0f : 90.0f, 0.0f, 0.0f};
    const auto yaw = static_cast<float>(
        std::atan2(static_cast<double>(direction.y), static_cast<double>(direction.x)));
    const float horizontal = std::sqrt(direction.x * direction.x + direction.y * direction.y);
    const auto pitch = static_cast<float>(
        std::atan2(static_cast<double>(negate(direction.z)), static_cast<double>(horizontal)));
    return {pitch * radians_to_degrees, yaw * radians_to_degrees, 0.0f};
}
}

PlayerAirGateResult process_player_air_gate(const PlayerAirGateSettings& settings,
                                            const PlayerAirGateFrame& frame,
                                            const PlayerAirGateBindings& bindings,
                                            std::uint32_t& scheduler_flags, UserCommand& command,
                                            MovementAngleView movement_angles) {
    using Continuation = PlayerAirGateContinuation;
    PlayerAirGateResult result{Continuation::complete, false};
    const auto& fields = bindings.fields;
    auto* const player = frame.player;
    if (field<std::uint8_t>(player, fields.movement_type_7501cb91) != 2u)
        return result;

    result.air_strafe_enabled = settings.air_strafe_enabled;
    result.continuation = Continuation::air_solver;
    // 433BA215 returns the unsigned byte comparison >=2.
    if (field<std::uint8_t>(player, fields.water_state_ac7f2913) >= 2u)
        return result;

    const auto player_flags =
        field<std::uint8_t>(player, bindings.environment.fields.player_flags_e2fbe695);
    auto* const weapon = frame.weapon;
    if (player_flags & on_ground) {
        if ((scheduler_flags & 8u) && weapon &&
            field<std::uint16_t>(weapon, fields.weapon_definition_5c9f0fbb) == 31u &&
            (command.buttons & jump_button)) {
            scheduler_flags &= ~8u;
            command.buttons &= ~jump_button;
            result.continuation = Continuation::complete;
        }
        return result;
    }

    if (settings.bunny_hop_enabled)
        suppress_airborne_jump(true, player_flags, bindings.bunny_hop_guard, command);

    if (!(scheduler_flags & 8u) || !(*bindings.speed_policy_flags & 0x80u) ||
        !(frame.feedback->original_buttons & speed_button) || !weapon ||
        field<std::uint16_t>(weapon, fields.weapon_definition_5c9f0fbb) != 40u ||
        field<std::uint8_t>(weapon, fields.weapon_blocked_b8969256) != 0u)
        return result;
    const auto vertical = field<float>(player, fields.velocity_36375c23 + 8u);
    if (-80.0f > vertical)
        return result;
    const auto weapon_attack = field<float>(weapon, fields.weapon_next_attack_f71ce331);
    const auto player_attack = field<float>(player, fields.player_next_attack_de530a8b);
    if (maximum_number(player_attack, weapon_attack) > *bindings.current_time)
        return result;

    command.buttons &= ~0x618u;
    command.forward_move = 0.0f;
    command.side_move = 0.0f;
    result.continuation = Continuation::ground_speed_tail;

    const auto velocity = field<MovementVector>(player, fields.velocity_36375c23);
    const float speed = std::sqrt((velocity.x * velocity.x + velocity.y * velocity.y) + 0.0f);
    const float reciprocal = 1.0f / (speed + epsilon);
    MovementVector direction{velocity.x * reciprocal, velocity.y * reciprocal, 0.0f * reciprocal};

    const float first_acceleration = air_accelerate(settings);
    const float reduction = minimum_number(settings.brake_reduction_limit,
                                           *bindings.environment.interval * first_acceleration);
    const auto physics = capture_movement_command_physics(player, bindings.environment);
    const float threshold = maximum_number(
        settings.brake_threshold,
        minimum_number(physics.player_speed_limit, physics.player_max_speed) * 0.34f - reduction);
    // COMISS speed,threshold / JB also exits for unordered values.
    if (!(speed >= threshold))
        return result;

    direction = {negate(direction.x), negate(direction.y), negate(direction.z)};
    movement_angles = counter_angles(direction);
    command.forward_move = settings.maximum_move;
    const float second_acceleration = air_accelerate(settings);
    const float bounded_acceleration =
        minimum_number(settings.brake_threshold, second_acceleration);
    if (!(bounded_acceleration > 0.0f))
        return result;

    command.forward_move = sse_min(settings.maximum_move,
                                   speed / (*bindings.environment.interval * bounded_acceleration));
    const float probe_acceleration = air_accelerate(settings);
    const auto probe_velocity = field<MovementVector>(player, fields.velocity_36375c23);
    const float amount = air_acceleration_delta(probe_velocity, direction, speed,
                                                probe_acceleration, *bindings.environment.interval);
    const auto current_velocity = field<MovementVector>(player, fields.velocity_36375c23);
    const float next_y = direction.y * amount + current_velocity.y;
    const float next_x = direction.x * amount + current_velocity.x;
    if (std::sqrt(next_y * next_y + next_x * next_x) > speed)
        command.forward_move = 0.001f;
    return result;
}

} // namespace skeet

// ============================================================================
// Subcomponent: player_collision.cpp
// ============================================================================

namespace skeet::client {
bool PlayerPhysicsHooks::should_collide(void* player, std::int32_t group, std::uint32_t mask) {
    if (group == 8) {
        const auto contents = engine::invoke<std::uint32_t>(player, 152) & 0x1800U;
        if (contents && (mask & 0x1800U) == contents)
            return false;
    }
    return player_physics_detail::original<bool>(player, 173, group, mask);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: pre_speed.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;

PreSpeedTurn prepare_pre_speed_turn(PreSpeedLimits limits, float interval_per_tick,
                                    float player_speed_limit, MovementVector velocity) noexcept {
    const float speed = horizontal_length(velocity);
    const float tick_scale = interval_per_tick * 128.0f;
    const float minimum_step = tick_scale * 2.25f;
    const float maximum_step = tick_scale * 5.0f;
    const float angle =
        minimum_number(limits.maximum_angle, limits.angle_scale / (speed + epsilon)) * 2.0f;
    const float turn = sse_min(sse_max(angle, minimum_step), maximum_step);
    return {speed, minimum_step, maximum_step, turn, player_speed_limit >= speed};
}

} // namespace skeet

// ============================================================================
// Subcomponent: pre_speed_controller.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;
namespace {
struct RadialWall {
    bool found;
    MovementVector normal;
};
RadialWall find_pre_speed_wall(const RawPreSpeedInput& input, MovementCollisionEngine& engine,
                               std::uint8_t* trace_flag) {
    MovementTraceFlagGuard guard(trace_flag);
    float nearest = 1.0f;
    MovementVector normal{};
    MovementCollisionTrace trace{};
    trace.hitgroup = trace.hitbox = -1;
    trace.fraction = 1.0f;
    float radians = 0.0f;
    do {
        const MovementVector destination{input.radial_origin.x + std::cos(radians) * 64.0f,
                                         input.radial_origin.y + std::sin(radians) * 64.0f,
                                         input.radial_origin.z};
        engine.trace_ray(make_movement_hull_ray(input.radial_origin, destination,
                                                input.forecast.hull_minimum,
                                                input.forecast.hull_maximum),
                         1u, {MovementCollisionFilterKind::reject_entities, 0, 0, 0}, trace);
        // Native COMISS/JAE retains a new unordered fraction as well.
        if (!(trace.fraction >= nearest)) {
            nearest = trace.fraction;
            normal = trace.plane_normal;
        }
        radians += 0.31415927410125732421875f;
    } while (radians < 6.283185482025146484375f);
    return {nearest != 1.0f && !(normal.z >= 0.1f), normal};
}
float wall_or_turn(const RawPreSpeedInput& input, MovementCollisionEngine& engine,
                   std::uint8_t* trace_flag, float base, float turn) {
    const auto wall = find_pre_speed_wall(input, engine, trace_flag);
    if (!wall.found)
        return base + turn;
    const float wall_heading = planar_movement_yaw(wall.normal);
    const float raw_difference = wall_heading - base;
    if (std::isnan(raw_difference))
        return wall_heading;
    const float difference = normalize_movement_angle(raw_difference);
    if (difference > 2.5f)
        return base + 2.5f;
    if (-2.5f > difference)
        return base - 2.5f;
    return wall_heading;
}
}

float choose_raw_pre_speed_heading(const RawPreSpeedInput& input, float movement_heading,
                                   MovementCollisionEngine& engine,
                                   std::uint8_t* engine_trace_flag) {
    const float base = input.previous_heading != std::numeric_limits<float>::max()
                           ? input.previous_heading
                           : movement_heading;
    MovementTraceFlagGuard guard(engine_trace_flag);
    const auto prepared = prepare_pre_speed_turn(input.limits, input.forecast.interval_per_tick,
                                                 input.player_speed_limit, input.forecast.velocity);
    float turn = prepared.turn_step;
    if (prepared.within_speed_limit)
        return wall_or_turn(input, engine, engine_trace_flag, base, turn);
    auto trial = input.forecast;
    trial.horizon = sse_min(sse_max(320.0f / prepared.horizontal_speed, 0.35f), 1.0f);
    for (;;) {
        trial.turn_step = turn;
        if (!forecast_pre_speed_circle(trial, engine).blocked)
            break;
        if (prepared.maximum_step <= turn)
            break;
        turn += 0.2f;
    }
    if (!(turn >= prepared.maximum_step))
        return base + (turn + 0.2f);
    turn = prepared.minimum_step;
    for (;;) {
        trial.turn_step = turn;
        if (!forecast_pre_speed_circle(trial, engine).blocked)
            break;
        if (turn <= negate(prepared.minimum_step))
            break;
        turn -= 0.2f;
    }
    if (!(negate(prepared.minimum_step) >= turn))
        return base + (turn - 0.2f);
    return wall_or_turn(input, engine, engine_trace_flag, base, turn);
}

} // namespace skeet

// ============================================================================
// Subcomponent: pre_speed_live.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;

/* deduplicated helper */
std::int32_t tick_count(float value) noexcept {
    if (!(value >= -2147483648.0f && value < 2147483648.0f))
        return std::numeric_limits<std::int32_t>::min();
    return static_cast<std::int32_t>(value);
}
void trace_forecast(const LivePreSpeedBindings& bindings, MovementVector start, MovementVector end,
                    MovementCollisionTrace& trace) {
    const auto mask = engine::invoke<std::uint32_t>(bindings.player, 0x260u / 4u);
    const auto* maximum = engine::invoke<const MovementVector*>(bindings.player, 0x134u / 4u);
    const auto* minimum = engine::invoke<const MovementVector*>(bindings.player, 0x130u / 4u);
    bindings.collision.trace_ray(
        make_movement_hull_ray(start, end, *minimum, *maximum), mask,
        {MovementCollisionFilterKind::skip_player,
         static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(bindings.player)), 8, 0},
        trace);
}
bool forecast_circle(const LivePreSpeedInput& input, float turn, float horizon,
                     const LivePreSpeedBindings& bindings) {
    auto velocity = input.forecast_velocity;
    float yaw = planar_movement_yaw(velocity);
    const float gravity_step =
        convar_float(bindings.capture.convars.gravity) * (*bindings.environment.interval);
    const float jump_impulse = convar_float(bindings.capture.convars.jump_impulse);
    const auto count = tick_count(horizon / (*bindings.environment.interval) + 0.5f);
    bool ground =
        (field<std::uint8_t>(bindings.player, bindings.environment.fields.player_flags_e2fbe695) &
         on_ground) != 0;
    auto position = field<MovementVector>(bindings.player, bindings.forecast_position_a9ee6062);
    MovementCollisionTrace trace{};
    trace.fraction = 1.0f;
    trace.hitgroup = trace.hitbox = -1;
    for (std::int32_t tick = 0; tick < count; ++tick) {
        if (ground)
            velocity.z = jump_impulse;
        else
            velocity.z -= gravity_step;
        yaw += turn;
        const float radians = yaw * degrees_to_radians;
        const float sine = std::sin(radians), cosine = std::cos(radians);
        const float speed = horizontal_length(velocity);
        velocity.x = speed * cosine;
        velocity.y = speed * sine;
        const float interval = *bindings.environment.interval;
        const MovementVector destination{interval * velocity.x + position.x,
                                         interval * velocity.y + position.y,
                                         interval * velocity.z + position.z};
        trace_forecast(bindings, position, destination, trace);
        if ((trace.fraction != 1.0f && trace.plane_normal.z <= 0.9f) || trace.start_solid ||
            trace.all_solid)
            return true;
        position = trace.end;
        trace_forecast(bindings, position, {position.x, position.y, position.z - 2.0f}, trace);
        ground = (1.0f > trace.fraction || trace.start_solid || trace.all_solid) &&
                 trace.plane_normal.z >= 0.7f;
    }
    return false;
}
float radial_heading(const LivePreSpeedInput& input, float base, float turn,
                     const LivePreSpeedBindings& bindings) {
    MovementTraceFlagGuard guard(bindings.environment.engine_trace_flag);
    const auto* minimum = engine::invoke<const MovementVector*>(bindings.player, 0x130u / 4u);
    const auto* maximum = engine::invoke<const MovementVector*>(bindings.player, 0x134u / 4u);
    float nearest = 1.0f;
    MovementVector normal{};
    MovementCollisionTrace trace{};
    trace.fraction = 1.0f;
    trace.hitgroup = trace.hitbox = -1;
    float radians = 0.0f;
    do {
        const float dx = std::cos(radians) * 64.0f;
        const float z = input.radial_origin.z;
        const float dy = std::sin(radians) * 64.0f;
        const auto origin = input.radial_origin;
        const MovementVector end{origin.x + dx, origin.y + dy, z};
        bindings.collision.trace_ray(make_movement_hull_ray(origin, end, *minimum, *maximum), 1u,
                                     {MovementCollisionFilterKind::reject_entities, 0, 0, 0},
                                     trace);
        if (!(trace.fraction >= nearest)) {
            nearest = trace.fraction;
            normal = trace.plane_normal;
        }
        radians += 0.31415927410125732421875f;
    } while (radians < 6.283185482025146484375f);
    if (nearest == 1.0f || normal.z >= 0.1f)
        return base + turn;
    const float wall = planar_movement_yaw(normal);
    const float raw = wall - base;
    if (std::isnan(raw))
        return wall;
    const float difference = normalize_movement_angle(raw);
    if (difference > 2.5f)
        return base + 2.5f;
    if (-2.5f > difference)
        return base - 2.5f;
    return wall;
}
}

float choose_live_pre_speed_heading(const LivePreSpeedInput& input, float movement_heading,
                                    const LivePreSpeedBindings& bindings) {
    using namespace movement_detail;
    const float base = input.previous_heading != std::numeric_limits<float>::max()
                           ? input.previous_heading
                           : movement_heading;
    MovementTraceFlagGuard guard(bindings.environment.engine_trace_flag);
    const auto prepared = prepare_pre_speed_turn(
        input.limits, *bindings.environment.interval,
        field<float>(bindings.player, bindings.environment.fields.speed_limit_3eb71843),
        input.velocity);
    float turn = prepared.turn_step;
    if (prepared.within_speed_limit)
        return radial_heading(input, base, turn, bindings);
    const float horizon = sse_min(sse_max(320.0f / prepared.horizontal_speed, 0.35f), 1.0f);
    for (;;) {
        if (!forecast_circle(input, turn, horizon, bindings))
            break;
        if (prepared.maximum_step <= turn)
            break;
        turn += 0.2f;
    }
    if (!(turn >= prepared.maximum_step))
        return base + (turn + 0.2f);
    turn = prepared.minimum_step;
    for (;;) {
        if (!forecast_circle(input, turn, horizon, bindings))
            break;
        if (turn <= negate(prepared.minimum_step))
            break;
        turn -= 0.2f;
    }
    if (!(negate(prepared.minimum_step) >= turn))
        return base + (turn - 0.2f);
    return radial_heading(input, base, turn, bindings);
}

} // namespace skeet

// ============================================================================
// Subcomponent: pre_speed_probe.cpp
// ============================================================================

namespace skeet {
namespace {

std::int32_t forecast_tick_count(float horizon, float interval) noexcept {
    const float count = horizon / interval + 0.5f;
    // CVTTSS2SI produces the indefinite integer for NaN and overflow.
    if (!(count >= -2147483648.0f && count < 2147483648.0f))
        return std::numeric_limits<std::int32_t>::min();
    return static_cast<std::int32_t>(count);
}

void trace_pre_speed_hull(const PreSpeedProbeInput& input, MovementCollisionEngine& engine,
                          MovementVector start, MovementVector end, MovementCollisionTrace& trace) {
    engine.trace_ray(make_movement_hull_ray(start, end, input.hull_minimum, input.hull_maximum),
                     input.collision_mask,
                     {MovementCollisionFilterKind::skip_player, input.player, 8, 0}, trace);
}

} // namespace

PreSpeedProbeResult forecast_pre_speed_circle(const PreSpeedProbeInput& input,
                                              MovementCollisionEngine& engine) {
    using namespace movement_detail;
    PreSpeedProbeResult result{false, input.position, input.velocity,
                               (input.player_flags & skeet::on_ground) != 0, 0};
    float yaw = planar_movement_yaw(result.velocity);
    const float gravity_step = input.gravity * input.interval_per_tick;
    const auto count = forecast_tick_count(input.horizon, input.interval_per_tick);
    MovementCollisionTrace trace{};
    trace.fraction = 1.0f;
    trace.hitgroup = trace.hitbox = -1;
    for (std::int32_t tick = 0; tick < count; ++tick) {
        if (result.on_ground)
            result.velocity.z = input.jump_impulse;
        else
            result.velocity.z -= gravity_step;

        yaw += input.turn_step;
        const float radians = yaw * degrees_to_radians;
        const float sine = std::sin(radians), cosine = std::cos(radians);
        const float speed = std::sqrt(result.velocity.x * result.velocity.x +
                                      result.velocity.y * result.velocity.y);
        result.velocity.x = speed * cosine;
        result.velocity.y = speed * sine;
        const MovementVector destination{
            input.interval_per_tick * result.velocity.x + result.position.x,
            input.interval_per_tick * result.velocity.y + result.position.y,
            input.interval_per_tick * result.velocity.z + result.position.z};
        trace_pre_speed_hull(input, engine, result.position, destination, trace);
        if ((trace.fraction != 1.0f && trace.plane_normal.z <= 0.9f) || trace.start_solid ||
            trace.all_solid) {
            result.blocked = true;
            return result;
        }

        result.position = trace.end;
        const MovementVector below{result.position.x, result.position.y, result.position.z - 2.0f};
        trace_pre_speed_hull(input, engine, result.position, below, trace);
        result.on_ground = (1.0f > trace.fraction || trace.start_solid || trace.all_solid) &&
                           trace.plane_normal.z >= 0.7f;
        ++result.completed_ticks;
    }
    return result;
}

} // namespace skeet

// ============================================================================
// Subcomponent: prediction_local.cpp
// ============================================================================

namespace skeet {
MovementVector advance_movement_origin(std::uint32_t player, MovementVector position,
                                       MovementVector velocity, std::uint32_t player_flags,
                                       const MovementSimulationPlayer& snapshot,
                                       const MovementSimulationRules& rules,
                                       const MovementSimulationEnvironment& environment,
                                       const client::Command& command, std::uint32_t input_buttons,
                                       float yaw, MovementCollisionEngine& collision) {
    const float squared_input = command.movement.forward_move * command.movement.forward_move +
                                command.movement.side_move * command.movement.side_move;
    const float squared_velocity =
        (velocity.x * velocity.x + velocity.y * velocity.y) + velocity.z * velocity.z;
    if (!((input_buttons & 0x618u) && command.tick != 0x7f7fffff && squared_input > 10.0f) &&
        !(squared_velocity > 4.0f))
        return position;
    auto copy = command;
    copy.angles[1] = yaw;
    MovementSimulation simulation{};
    initialize_movement_simulation(simulation, player, position, velocity, player_flags, 0,
                                   snapshot, rules);
    const MovementAngles angles{copy.angles[0], copy.angles[1], copy.angles[2]};
    for (int i = 0; i < 2; ++i)
        simulate_movement_tick(simulation, collision, environment, player, copy.movement, angles);
    return simulation.position;
}
} // namespace skeet

// ============================================================================
// Subcomponent: prediction_origin.cpp
// ============================================================================

namespace skeet {
void predict_movement_origin(MovementVector& origin, MovementVector velocity,
                             MovementVector minimum, MovementVector maximum, std::int32_t ticks,
                             float interval, MovementCollisionEngine& collision,
                             std::uint8_t* trace_flag) {
    const float speed_squared = velocity.x * velocity.x + velocity.y * velocity.y;
    if (!(speed_squared > 25.0f) || !(115600.0f > speed_squared))
        return;
    maximum.z -= 18.0f;
    MovementVector start = origin;
    start.z += 18.0f;
    const float duration = minimum_number(0.2f, static_cast<float>(ticks) * interval);
    const MovementVector end{velocity.x * duration + origin.x, velocity.y * duration + origin.y,
                             duration * 0.0f + start.z};
    auto trace = MovementCollisionTrace{};
    trace.fraction = 1;
    trace.hitgroup = -1;
    trace.hitbox = -1;
    trace_movement_world(collision, make_movement_hull_ray(start, end, minimum, maximum), 1, trace,
                         trace_flag);
    origin.x = trace.end.x;
    origin.y = trace.end.y;
    const auto next = trace.end;
    trace.fraction = 1;
    trace.all_solid = 0;
    trace.start_solid = 0;
    trace.entity = 0;
    trace.hitgroup = -1;
    trace.hitbox = -1;
    trace_movement_world(
        collision, make_movement_hull_ray(next, {next.x, next.y, next.z - 18.0f}, minimum, maximum),
        1, trace, trace_flag);
    origin.z = trace.end.z;
}
} // namespace skeet

// ============================================================================
// Subcomponent: quick_peek.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;
namespace {
struct LiveQuickPeek {
    void* player;
    void* weapon;
    client::Command& command;
    const MovementCommandBindings& bindings;
};
constexpr std::uint32_t movement_buttons = 0x61a;
float length(MovementVector v) noexcept {
    return std::sqrt((v.x * v.x + v.y * v.y) + v.z * v.z);
}
MovementVector add(MovementVector a, MovementVector b) {
    return {a.x + b.x, a.y + b.y, a.z + b.z};
}
MovementVector sub(MovementVector a, MovementVector b) {
    return {a.x - b.x, a.y - b.y, a.z - b.z};
}
MovementVector mul(MovementVector a, float b) {
    return {a.x * b, a.y * b, a.z * b};
}
MovementVector unit(MovementVector a) {
    return mul(a, 1.0f / (length(a) + epsilon));
}
bool hit(const MovementCollisionTrace& t) {
    return 1.0f > t.fraction || t.all_solid || t.start_solid;
}
MovementCollisionRay hull(MovementVector start, MovementVector end, MovementVector minimum,
                          MovementVector maximum) {
    return make_movement_hull_ray(start, end, minimum, maximum);
}
MovementCollisionTrace peek_trace(MovementCollisionEngine& engine, MovementCollisionRay ray,
                                  const QuickPeekFrame& f, std::uint8_t* flag,
                                  bool guard_flag = true, const LiveQuickPeek* live = nullptr) {
    MovementCollisionTrace result{};
    result.fraction = 1.0f;
    result.surface_property = -1;
    result.hitbox = -1;
    const auto mask = live ? movement_command_trace_mask(live->player) : f.trace_mask;
    MovementTraceFlagGuard guard(
        guard_flag ? (live ? live->bindings.environment.engine_trace_flag : flag) : nullptr);
    engine.trace_ray(ray, mask, {MovementCollisionFilterKind::skip_player, f.player_entity, 8, 0},
                     result);
    return result;
}
const QuickPeekShotRecord* record(const QuickPeekFrame& f, std::int32_t command) {
    for (const auto& item : f.shot_records)
        if (item.command_number == command)
            return &item;
    return nullptr;
}
bool shot_triggered(const QuickPeekState& s, const QuickPeekFrame& f) {
    int previous = -1;
    for (int i = 0; i < 16; ++i) {
        const auto* r = record(f, f.last_acknowledged_command - i);
        if (!r)
            break;
        if ((r->flags & 1) && r->shot_time != 0.0f && r->fired) {
            previous = f.last_acknowledged_command - i;
            break;
        }
    }
    if (previous != -1) {
        for (int i = 1, c = previous + 1; i < 16 && c <= f.last_acknowledged_command; ++i, ++c) {
            const auto* r = record(f, c);
            if (!r)
                break;
            if ((r->flags & 1) && r->shot_time == 0.0f && !r->fired)
                return true;
        }
    }
    return f.shot_counter > s.shot_counter;
}
float retreat_heading(QuickPeekState& s, const QuickPeekFrame& f,
                      MovementCollisionEngine& collision, std::uint8_t* flag,
                      const LiveQuickPeek* live = nullptr,
                      const MovementVector* retained_origin = nullptr) {
    MovementVector delta = sub(retained_origin ? *retained_origin : s.origin.value, f.position);
    delta.z = 0;
    const auto direction = unit(delta);
    float yaw = planar_movement_yaw(direction);
    const float distance = length(delta);
    const auto check = add(f.position, mul(direction, minimum_number(100.0f, distance)));
    auto ray = hull({f.position.x, f.position.y, f.position.z + 20},
                    {check.x, check.y, f.position.z + 20}, {-16, -16, 0}, {16, 16, 48});
    const auto first = peek_trace(collision, ray, f, flag, true, live);
    if (!hit(first))
        return yaw;
    MovementVector perpendicular;
    if (first.plane_normal.x == 0 && first.plane_normal.y == 0)
        perpendicular = {0, -1, 0};
    else
        perpendicular = unit({first.plane_normal.y, -first.plane_normal.x, 0});
    for (int i = 0; i < 16; ++i) {
        auto offset = mul(perpendicular, 4.0f * static_cast<float>(i + 1));
        for (int side = 0; side < 2; ++side) {
            const MovementVector origin{f.position.x + offset.x, f.position.y + offset.y,
                                        f.position.z + 20 + offset.z};
            MovementVector toward{s.origin.value.x - origin.x, s.origin.value.y - origin.y, 0};
            const float distance2 = length(toward);
            if (distance2 > 100.0f)
                toward = mul(toward, 100.0f / distance2);
            const auto tested = peek_trace(
                collision, hull(origin, add(origin, toward), {-16, -16, 0}, {16, 16, 48}), f, flag,
                true, live);
            if (!hit(tested)) {
                const auto alternate = unit(offset);
                const MovementVector blended{
                    (direction.x - alternate.x) * first.fraction + alternate.x,
                    (direction.y - alternate.y) * first.fraction + alternate.y, 0};
                return planar_movement_yaw(blended);
            }
            offset = mul(offset, -1.0f);
        }
    }
    return yaw;
}
MovementVector stopping_position(const QuickPeekFrame& f, const UserCommand& command,
                                 const LiveQuickPeek* live = nullptr) {
    MovementVector position = f.position, velocity = f.velocity;
    float speed = length(velocity);
    if (!(speed > 1.0f))
        return position;
    const auto copied_command = command;
    const auto& acceleration_command = live ? copied_command : command;
    const auto speed_physics =
        live ? capture_movement_command_speed(live->player, live->bindings) : f.physics;
    const float maximum =
        minimum_number(speed_physics.player_speed_limit, speed_physics.player_max_speed);
    for (int i = 0; i < 15; ++i) {
        const auto friction =
            live ? capture_movement_command_friction(live->bindings)
                 : MovementCommandFriction{f.physics.stop_speed, f.physics.friction,
                                           f.physics.interval_per_tick};
        const float decreased = maximum_number(
            speed - (maximum_number(friction.stop_speed, speed) * friction.friction) *
                        friction.interval,
            0.0f);
        if (0.1f > decreased)
            break;
        auto opposite = unit(mul(velocity, -1.0f));
        velocity = mul(velocity, decreased / speed);
        if (opposite.z != 0.0f)
            opposite = normalize_horizontal(opposite);
        auto capacity =
            live ? movement_command_ground_acceleration(live->player, acceleration_command,
                                                        velocity, opposite, maximum, live->bindings)
                 : ground_acceleration(f.physics, command, velocity, opposite, maximum);
        if (capacity.amount > decreased && capacity.acceleration != 0.0f)
            capacity = live ? movement_command_ground_acceleration(live->player,
                                                                   acceleration_command, velocity,
                                                                   opposite, 0.01f, live->bindings)
                            : ground_acceleration(f.physics, command, velocity, opposite, 0.01f);
        if (capacity.amount > 0)
            velocity = add(velocity, mul(opposite, capacity.amount));
        position = add(position, mul(velocity, live ? *live->bindings.environment.interval
                                                    : f.physics.interval_per_tick));
        speed = length(velocity);
        if (1.0f > speed)
            break;
    }
    return position;
}
}

void initialize_quick_peek_state(QuickPeekState& s) noexcept {
    s.phase = 0;
    s.saved_buttons = 0;
    s.origin.value.x =
        std::bit_cast<float>(std::bit_cast<std::uint32_t>(s.origin.value.x) & 0xffffff00u);
    s.origin.valid = false;
    s.shot_counter = 0;
}
void reset_quick_peek_state(QuickPeekState& s) noexcept {
    s.phase = 0;
    s.origin.valid = false;
    s.stop_time = 0;
    s.shot_counter = 0;
}
void update_quick_peek_shot_counter(QuickPeekState& s, std::int32_t counter) noexcept {
    s.shot_counter = counter;
}

namespace {
QuickPeekContinuation process_peek(const QuickPeekConfig& config, InputBitmap input,
                                   QuickPeekState& s, QuickPeekFrame f, UserCommand& command,
                                   MovementAngleView angles, std::uint32_t& flags,
                                   std::span<std::uint32_t, 150> marked,
                                   MovementCollisionEngine& collision, std::uint8_t* trace_flag,
                                   const LiveQuickPeek* live) {
    s.distance = config.distance.value == 201 ? 1000.0f : static_cast<float>(config.distance.value);
    s.saved_buttons = f.original_buttons;
    auto& history = s.history[static_cast<std::uint32_t>(f.command_number) % 32];
    if (live) {
        history.command_number = f.command_number;
        const auto captured =
            capture_quick_peek_history(live->player, f.command_number, live->bindings);
        history.position = captured.position;
        history.unavailable = captured.unavailable;
        history.grounded = captured.grounded;
    } else
        history = {f.command_number, f.position, f.history_unavailable,
                   (f.physics.player_flags & 1) != 0};
    if (live && s.stop_time > 0)
        f.physics.interval_per_tick = *live->bindings.environment.interval;
    if (s.stop_time > 0)
        s.stop_time = maximum_number(s.stop_time - f.physics.interval_per_tick, 0.0f);
    if (!config.enabled.value || !hotkey_active(config.hotkey.value.binding, input)) {
        reset_quick_peek_state(s);
        return QuickPeekContinuation::movement_features;
    }
    if (s.stop_time > 0) {
        if (live)
            f.physics.player_flags = movement_command_player_flags(live->player, live->bindings);
        if ((f.physics.player_flags & 1) &&
            !((s.retreat_buttons ^ s.saved_buttons) & movement_buttons)) {
            const auto velocity =
                live ? movement_command_velocity(live->player, live->bindings) : f.velocity;
            const bool wrote = live ? write_live_movement_counter(live->player, command, velocity,
                                                                  angles, live->bindings)
                                    : write_counter_movement(f.physics, command, velocity, angles);
            if (wrote)
                flags |= 1;
            return QuickPeekContinuation::complete;
        }
        s.stop_time = 0;
    }
    if (s.phase == 0) {
        if (!s.origin.valid) {
            auto position = live ? movement_command_position(live->player) : f.position;
            const MovementVector* selected_position = nullptr;
            if (live) {
                f.physics.player_flags =
                    movement_command_player_flags(live->player, live->bindings);
                f.command_number = live->command.number;
            }
            if (f.physics.player_flags & 1) {
                if (live)
                    f.physics.interval_per_tick = *live->bindings.environment.interval;
                const int ticks = static_cast<int>(0.2f / f.physics.interval_per_tick + 0.5f);
                bool grounded = true;
                for (int i = 1; i <= ticks; ++i) {
                    const int number = f.command_number - i;
                    const auto& h = s.history[static_cast<std::uint32_t>(number) % 32];
                    if (h.command_number == number && !h.grounded) {
                        grounded = false;
                        break;
                    }
                }
                const int number = f.command_number - ticks;
                const auto& h = s.history[static_cast<std::uint32_t>(number) % 32];
                if (grounded && h.command_number == number && !h.unavailable) {
                    position = h.position;
                    selected_position = &h.position;
                }
            }
            const auto end = MovementVector{position.x, position.y, position.z - 200.0f};
            if (live) {
                const auto bounds = capture_quick_peek_origin_hull(live->player);
                f.hull_minimum = bounds.minimum;
                f.hull_maximum = bounds.maximum;
                f.trace_mask = bounds.mask;
                if (selected_position)
                    position = *selected_position;
            }
            auto trace = MovementCollisionTrace{};
            trace.fraction = 1;
            trace.surface_property = -1;
            trace.hitbox = -1;
            trace_movement_world(collision, hull(position, end, f.hull_minimum, f.hull_maximum),
                                 f.trace_mask, trace,
                                 live ? live->bindings.environment.engine_trace_flag : trace_flag);
            if (hit(trace))
                s.origin = {trace.end, true};
        }
        if (s.origin.valid) {
            s.start_time = live ? live->bindings.current_time : f.current_time;
            s.phase = 1;
        }
    }
    if (s.phase == 1 && s.saved_buttons & movement_buttons)
        s.phase = 2;
    if (s.phase == 2) {
        flags |= 0x20000;
        if (live) {
            f.command_number = live->command.number;
            f.mark_command = hotkey_active(live->bindings.command_mark_hotkey, input);
        }
        if (f.mark_command)
            marked[static_cast<std::uint32_t>(f.command_number) % 150] = f.command_number;
        s.shot = (config.mode.value & 1) &&
                 (live ? movement_command_quick_peek_shot(s, live->player, live->bindings)
                       : shot_triggered(s, f));
        if (((config.mode.value & 2) && !(s.saved_buttons & movement_buttons)) || s.shot) {
            s.phase = 3;
            s.retreat_start = live ? movement_command_position(live->player) : f.position;
            s.released = false;
            s.retreat_buttons = s.saved_buttons;
            s.start_time = live ? live->bindings.current_time : f.current_time;
        } else {
            if (live && s.origin.valid)
                f.physics.player_flags =
                    movement_command_player_flags(live->player, live->bindings);
            if (s.origin.valid && (f.physics.player_flags & 1)) {
                const auto first_position =
                    live ? movement_command_position(live->player) : f.position;
                const auto delta = sub(s.origin.value, first_position);
                const float distance = std::sqrt(delta.x * delta.x + delta.y * delta.y);
                if (distance > s.distance) {
                    const auto second_position =
                        live ? movement_command_position(live->player) : f.position;
                    const auto center =
                        add(second_position, mul(delta, s.distance / (distance + epsilon)));
                    if (live) {
                        const auto bounds = capture_quick_peek_limit_hull(live->player);
                        f.hull_minimum = bounds.minimum;
                        f.hull_maximum = bounds.maximum;
                        f.trace_mask = bounds.mask;
                    }
                    const auto t = peek_trace(collision,
                                              hull({center.x, center.y, center.z + 18},
                                                   {center.x, center.y, center.z - 128},
                                                   f.hull_minimum, f.hull_maximum),
                                              f, trace_flag, false);
                    if (!t.all_solid && !t.start_solid)
                        s.origin = {t.end, true};
                }
            }
        }
    }
    if (s.phase == 3) {
        flags |= 0x20000;
        if (live) {
            f.command_number = live->command.number;
            f.mark_command = hotkey_active(live->bindings.command_mark_hotkey, input);
        }
        if (f.mark_command)
            marked[static_cast<std::uint32_t>(f.command_number) % 150] = f.command_number;
        if (!(s.saved_buttons & movement_buttons))
            s.released = true;
        if (s.released) {
            if (live && s.shot) {
                f.weapon_next_attack = movement_command_weapon_attack(live->weapon, live->bindings);
                f.player_next_attack = movement_command_player_attack(live->player, live->bindings);
                f.current_time = live->bindings.current_time;
            }
            const bool may_cancel =
                ((config.mode.value & 2) && !s.shot) ||
                (s.shot &&
                 f.current_time >= maximum_number(f.player_next_attack, f.weapon_next_attack));
            if (may_cancel && (s.saved_buttons & movement_buttons)) {
                s.phase = 2;
                flags |= 0x8000;
                return QuickPeekContinuation::movement_features;
            }
        }
        const auto retained_origin = s.origin.value;
        if (live)
            f.position = movement_command_position(live->player);
        const float heading =
            retreat_heading(s, f, collision, trace_flag, live, live ? &retained_origin : nullptr);
        if (live)
            f.physics.player_flags = movement_command_player_flags(live->player, live->bindings);
        if (!(f.physics.player_flags & 1)) {
            angles.yaw = heading;
            return QuickPeekContinuation::movement_features;
        }
        if (live) {
            f.position = movement_command_position(live->player);
            f.velocity = movement_command_velocity(live->player, live->bindings);
        }
        const auto predicted = stopping_position(f, command, live);
        auto toward = sub(predicted, s.origin.value);
        toward.z = 0;
        const float distance = length(toward);
        toward = unit(toward);
        auto start = sub(s.retreat_start, s.origin.value);
        start.z = 0;
        start = unit(start);
        if (distance > 0.01f && start.x * toward.x + start.y * toward.y >= 0.0f) {
            angles.yaw = heading;
            command.buttons &= ~speed_button;
            command.forward_move = 450;
            command.side_move = 0;
            flags |= 0x8000;
            return QuickPeekContinuation::complete;
        }
        s.phase = 0;
        if (live) {
            f.current_time = live->bindings.current_time;
            f.physics.interval_per_tick = *live->bindings.environment.interval;
            f.weapon_next_attack = movement_command_weapon_attack(live->weapon, live->bindings);
            f.player_next_attack = movement_command_player_attack(live->player, live->bindings);
        }
        const float elapsed =
            sse_min(sse_max(f.current_time - s.start_time, f.physics.interval_per_tick), 2.0f);
        const float next = sse_min(
            sse_max(maximum_number(f.player_next_attack, f.weapon_next_attack) - f.current_time,
                    0.0f),
            2.0f);
        s.stop_time = sse_min(sse_max(next - elapsed, 0.0f), 0.75f) + 0.2f;
        flags |= 0x200000;
    }
    if (s.phase)
        flags |= 0x8000;
    return QuickPeekContinuation::movement_features;
}
}

QuickPeekContinuation process_quick_peek(const QuickPeekConfig& config, InputBitmap input,
                                         QuickPeekState& state, const QuickPeekFrame& frame,
                                         UserCommand& command, MovementAngleView angles,
                                         std::uint32_t& flags, std::span<std::uint32_t, 150> marked,
                                         MovementCollisionEngine& collision,
                                         std::uint8_t* trace_flag) {
    return process_peek(config, input, state, frame, command, angles, flags, marked, collision,
                        trace_flag, nullptr);
}
QuickPeekContinuation process_live_quick_peek(const QuickPeekConfig& config, InputBitmap input,
                                              QuickPeekState& state, void* player, void* weapon,
                                              client::Command& command,
                                              std::uint32_t original_buttons,
                                              MovementAngleView angles, std::uint32_t& flags,
                                              const MovementCommandBindings& bindings) {
    QuickPeekFrame frame{};
    frame.original_buttons = original_buttons;
    frame.command_number = command.number;
    frame.player_entity = static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(player));
    const LiveQuickPeek live{player, weapon, command, bindings};
    return process_peek(config, input, state, frame, command.movement, angles, flags,
                        bindings.marked_commands, bindings.collision,
                        bindings.environment.engine_trace_flag, &live);
}
} // namespace skeet

// ============================================================================
// Subcomponent: quick_peek_visual.cpp
// ============================================================================

namespace skeet {
namespace {
struct Vertex {
    float x, y;
    std::uint32_t color;
};
bool project(const esp::Projection& p, MovementVector position, Vertex& output) {
    const auto& m = p.matrix;
    const float w = ((position.y * m[13] + position.x * m[12]) + position.z * m[14]) + m[15];
    if (0.001f > w)
        return false;
    const float reciprocal = 1.0f / w;
    const float x = ((position.y * m[1] + position.x * m[0]) + position.z * m[2]) + m[3];
    const float y = ((position.y * m[5] + position.x * m[4]) + position.z * m[6]) + m[7];
    output.x = (x * reciprocal) * p.horizontal_scale + p.horizontal_scale;
    output.y = p.vertical_scale - (y * reciprocal) * p.vertical_scale;
    return true;
}
}
void draw_quick_peek(const QuickPeekConfig& config, InputBitmap input, const QuickPeekState& state,
                     const esp::Projection& projection, ui::RenderQueue& queue) {
    if (!config.enabled.value || !hotkey_active(config.hotkey.value.binding, input) ||
        !state.origin.valid)
        return;
    // Native 4337DCA0 computes both tangent vectors even for this fixed plane.
    const float pitch = -90.0f * movement_detail::degrees_to_radians;
    const float sp = std::sin(pitch), cp = std::cos(pitch);
    const float sy = std::sin(0.0f), cy = std::cos(0.0f);
    const float sr = std::sin(0.0f), cr = std::cos(0.0f);
    const MovementVector first{(cr * cy) * sp + sr * sy, (cr * sy) * sp - sr * cy, cr * cp};
    const MovementVector second{(sr * cy) * sp - cr * sy, (sr * sy) * sp + cr * cy, sr * cp};
    const auto origin = state.origin.value;
    MovementVector previous{first.x * 24.0f + origin.x, first.y * 24.0f + origin.y,
                            first.z * 24.0f + origin.z};
    Vertex center{0, 0, config.color.value};
    if (!project(projection, origin, center))
        return;
    std::array<Vertex, 48> vertices;
    for (int i = 1; i <= 16; ++i) {
        const float angle = static_cast<float>(i) * 0.3926990926265716552734375f;
        const float sine = std::sin(angle), cosine = std::cos(angle);
        const MovementVector next{
            ((first.x * cosine) * 24.0f + origin.x) + (second.x * sine) * 24.0f,
            ((first.y * cosine) * 24.0f + origin.y) + (second.y * sine) * 24.0f,
            ((first.z * cosine) * 24.0f + origin.z) + (second.z * sine) * 24.0f};
        auto* triangle = vertices.data() + 3 * (i - 1);
        triangle[0] = center;
        triangle[1].color = config.color.value & 0x00ffffffu;
        triangle[2].color = config.color.value & 0x00ffffffu;
        if (!project(projection, previous, triangle[1]) || !project(projection, next, triangle[2]))
            return;
        previous = next;
    }
    const std::uint32_t count = 48;
    auto* payload = queue.append(7, count * sizeof(Vertex) + 16);
    std::memcpy(payload, &count, 4);
    std::memcpy(payload + 4, vertices.data(), sizeof(vertices));
}
} // namespace skeet

// ============================================================================
// Subcomponent: quick_stop.cpp
// ============================================================================

namespace skeet {

QuickStopContinuation process_quick_stop_stage(const QuickStopSettings& settings, InputBitmap input,
                                               const MovementPhysics& physics, bool has_weapon,
                                               std::uint32_t& scheduler_flags, UserCommand& command,
                                               MovementVector velocity,
                                               MovementAngleView movement_angles) {
    if (!has_weapon)
        return QuickStopContinuation::player_movement;
    if (!settings.standalone_quick_stop &&
        !(settings.quick_peek_enabled && hotkey_active(settings.quick_peek_hotkey, input)))
        return QuickStopContinuation::speed_features;

    const float squared_speed =
        (velocity.x * velocity.x + velocity.y * velocity.y) + velocity.z * velocity.z;
    const float squared_input =
        command.forward_move * command.forward_move + command.side_move * command.side_move;
    if (!(squared_speed > 4.0f) || !(0.01f > squared_input) || (command.buttons & 0x61au))
        return QuickStopContinuation::speed_features;

    if (physics.player_flags & on_ground) {
        if (write_counter_movement(physics, command, velocity, movement_angles))
            scheduler_flags |= 1u;
    }
    return QuickStopContinuation::complete;
}

} // namespace skeet

// ============================================================================
// Subcomponent: rage_stop.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;

/* deduplicated helper */

// Live433AC9C0. The acceleration cvar is read even when additional speed is
// nonpositive. Player/weapon reads follow that gate; interval follows all
// engine calls. The command button word is retained across those calls.
float counter_acceleration(void* player, UserCommand& command, MovementVector velocity,
                           MovementVector direction, float wish_speed,
                           const MovementEnvironmentBindings& bindings) {
    MovementPhysics physics{};
    physics.accelerate = convar_float(bindings.convars.accelerate);
    const float additional = wish_speed - dot(velocity, direction);
    if (0.0f >= additional)
        return 0.0f;

    UserCommand acceleration_command{};
    acceleration_command.buttons = command.buttons;
    if (!(acceleration_command.buttons & duck_button)) {
        physics.ducking = field<std::uint8_t>(player, bindings.fields.ducking_6b1a59b5) != 0;
        if (!physics.ducking)
            physics.player_flags =
                field<std::uint32_t>(player, bindings.fields.player_flags_e2fbe695);
    }
    physics.weapon_speed_enabled = !convar_self_integer(bindings.convars.accelerate_weapon_speed);
    if (physics.weapon_speed_enabled) {
        const auto handle = field<std::int32_t>(player, bindings.fields.weapon_handle_cd812654);
        if (handle != -1) {
            auto* const weapon = engine::invoke<void*>(bindings.entity_list, 4, handle);
            if (weapon) {
                physics.has_weapon = true;
                physics.weapon_max_speed = engine::invoke<float>(weapon, 0x6E8u / 4u);
                physics.weapon_mode_count = engine::invoke<std::int32_t>(weapon, 0x738u / 4u);
                if (physics.weapon_mode_count > 0)
                    physics.weapon_mode = engine::invoke<std::int32_t>(weapon, 0x6ECu / 4u);
            }
        }
    }
    physics.interval_per_tick = *bindings.interval;
    return ground_acceleration(physics, acceleration_command, velocity, direction, wish_speed)
        .amount;
}

// Live43388405. Friction/player/weapon getters are intentionally below the
// same speed gates as the original. This cannot use one entry-time snapshot.
void write_rage_counter_movement(void* player, client::Command& command, MovementVector velocity,
                                 const MovementEnvironmentBindings& bindings) {
    auto& movement = command.movement;
    movement.buttons &= ~0x61au;
    movement.forward_move = 0.0f;
    movement.side_move = 0.0f;
    const float speed = horizontal_length(velocity);
    if (!(speed >= 0.1f))
        return;

    const float stop_speed = convar_float(bindings.convars.stop_speed);
    const float friction = convar_float(bindings.convars.friction);
    const float control = maximum_number(stop_speed, speed);
    const float drop = (control * friction) * (*bindings.interval);
    const float after_friction = maximum_number(speed - drop, 0.0f);
    if (!(after_friction >= 0.1f))
        return;
    const float fraction = after_friction / speed;
    const MovementVector friction_velocity{velocity.x * fraction, velocity.y * fraction,
                                           fraction * 0.0f};
    const MovementVector reverse{negate(velocity.x), negate(velocity.y), 0.0f};
    const auto angles = horizontal_angles(reverse, false);
    command.angles[1] = angles.yaw;
    command.angles[0] = angles.pitch;
    command.angles[2] = 0.0f;
    movement.forward_move = 450.0f;
    const auto direction = normalize_horizontal(reverse);

    const auto command_physics = capture_movement_command_physics(player, bindings);
    const float maximum =
        minimum_number(command_physics.player_speed_limit, command_physics.player_max_speed);
    const float acceleration =
        counter_acceleration(player, movement, friction_velocity, direction, maximum, bindings);
    if (acceleration > after_friction)
        movement.forward_move = 0.001f;
}
}

float rage_stop_speed(void* weapon) {
    float maximum = engine::invoke<float>(weapon, 0x6E8u / 4u);
    if (maximum == 0.0f)
        maximum = 250.0f;
    return maximum * 0.34f - 1.0f;
}

float rage_move_speed(void* player, const RageStopBindings& bindings) {
    const auto physics = capture_movement_command_physics(player, bindings.environment);
    return minimum_number(physics.player_speed_limit, physics.player_max_speed) * 0.34f - 1.0f;
}

void prepare_rage_stop_command(const client::CommandFrame&, void* player, client::Command& command,
                               const MovementSimulation& simulation,
                               const RageStopBindings& bindings) {
    if (simulation.player_flags & on_ground)
        write_rage_counter_movement(player, command, simulation.velocity, bindings.environment);
}

} // namespace skeet

// ============================================================================
// Subcomponent: raw_air_solver.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;

bool process_raw_air_strafe(const RawAirStrafeSettings& settings,
                            const AirStrafeSelection& selection, InputBitmap input,
                            const RawAirStrafeFrame& frame, MovementFeedback& feedback,
                            client::Command& command, MovementAngleView angles,
                            const AirWallSteeringBindings& bindings) {
    const auto velocity = frame.collision.velocity;
    if (!prepare_air_strafe_heading(selection, velocity, false, command.movement, angles))
        return false;
    auto wall = frame.collision;
    wall.angle_scale = settings.limits.angle_scale;
    wall.maximum_angle = settings.limits.maximum_angle;
    wall.side_magnitude = settings.side_magnitude;
    steer_air_wall(wall, feedback, command, angles, bindings);
    const float speed = horizontal_length(velocity);
    const float ideal_step = minimum_number(settings.limits.maximum_angle,
                                            settings.limits.angle_scale / (speed + epsilon));
    const float velocity_yaw = planar_movement_yaw(velocity);
    auto smoothing = settings.smoothing;
    if (selection.pre_speed_requested) {
        if (!selection.pre_speed_effective) {
            smoothing = 100;
            apply_guarded_pre_speed_heading(speed, velocity_yaw, frame.previous_heading,
                                            settings.limits.maximum_angle, angles);
        } else {
            const auto& snapshot = bindings.snapshot;
            const auto& rules = bindings.rules;
            const PreSpeedProbeInput forecast{frame.forecast_position,
                                              velocity,
                                              frame.forecast_player_flags,
                                              rules.gravity,
                                              rules.jump_impulse,
                                              rules.interval_per_tick,
                                              0.0f,
                                              0.0f,
                                              snapshot.hull_minimum,
                                              snapshot.hull_maximum,
                                              snapshot.collision_mask,
                                              wall.player};
            const RawPreSpeedInput pre_speed{settings.limits, forecast, wall.position,
                                             bindings.environment.physics.player_speed_limit,
                                             frame.previous_heading};
            angles.yaw = choose_raw_pre_speed_heading(pre_speed, angles.yaw, bindings.collision,
                                                      bindings.environment.engine_trace_flag);
            if (resolve_pre_speed_feedback(frame.previous_heading, ideal_step,
                                           settings.feedback_turn_limit, settings.side_magnitude,
                                           feedback, command.movement, angles))
                return true;
            smoothing = 0;
        }
    } else if (apply_z_hop_heading(settings.z_hop, input, frame.engine_time, angles))
        smoothing = 0;
    const AirSteeringInput steering{velocity_yaw, ideal_step,
                                    speed,        settings.side_magnitude,
                                    smoothing,    (selection.movement_key_mode & 0xffu) != 0};
    AirSteeringState state{feedback.side_flip != 0, feedback.primary_heading};
    write_air_strafe_step(steering, state, command.movement, angles);
    feedback.side_flip = std::uint8_t(state.side_flip);
    feedback.primary_heading = state.feedback_heading;
    return true;
}

} // namespace skeet

// ============================================================================
// Subcomponent: raw_scout_gate.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;

bool obstructed(const MovementCollisionTrace& trace) noexcept {
    // The native COMISS/JA edge also treats an unordered fraction as clear.
    return trace.fraction < 1.0f || trace.all_solid || trace.start_solid;
}

bool clear_scout_path(const RawScoutGateFrame& frame, const RawScoutGateBindings& bindings,
                      MovementCollisionTrace& trace) {
    auto* collideable = engine::invoke<void*>(bindings.player, 3);
    const auto* maximum = engine::invoke<const MovementVector*>(collideable, 2);
    collideable = engine::invoke<void*>(bindings.player, 3);
    const auto* minimum = engine::invoke<const MovementVector*>(collideable, 1);
    MovementAngles view;
    // Complete boundary wrapper 0x4339FB19: engine-client virtual +0x48.
    engine::invoke<void>(bindings.engine_client, 18, &view);

    const float yaw = view.yaw * degrees_to_radians;
    const float sine_yaw = std::sin(yaw);
    const float cosine_yaw = std::cos(yaw);
    const float pitch = view.pitch * degrees_to_radians;
    // The original also evaluates sin(pitch), but discards its return value.
    (void)std::sin(pitch);
    const float cosine_pitch = std::cos(pitch);
    const float x = cosine_pitch * cosine_yaw;
    const float y = cosine_pitch * sine_yaw;
    const float inverse = 1.0f / (std::sqrt((x * x + y * y) + 0.0f * 0.0f) + epsilon);
    const MovementVector position = frame.position;
    const MovementVector end{(inverse * x) * 64.0f + position.x, (inverse * y) * 64.0f + position.y,
                             (inverse * 0.0f) * 64.0f + position.z};
    auto ray = make_movement_hull_ray(position, end, *minimum, *maximum);
    trace_movement_world(bindings.collision, ray, 0x1400bu, trace, bindings.engine_trace_flag);
    if (!obstructed(trace))
        return true;

    // Bounds remain live references across the first actual engine trace.
    const MovementVector raised{position.x, position.y, position.z + 64.0f};
    const MovementVector raised_end{end.x, end.y, end.z + 64.0f};
    ray = make_movement_hull_ray(raised, raised_end, *minimum, *maximum);
    trace_movement_world(bindings.collision, ray, 0x1400bu, trace, bindings.engine_trace_flag);
    return !obstructed(trace);
}
}

RawScoutGateResult process_raw_scout_gate(const RawScoutGateSettings& settings,
                                          const AirStrafeSelection& selection,
                                          const RawScoutGateFrame& frame,
                                          const MovementFeedback& feedback, UserCommand& command,
                                          const RawScoutGateBindings& bindings) {
    const float speed = movement_detail::horizontal_length(frame.velocity);
    const bool grounded = (frame.player_flags & on_ground) != 0;
    if (grounded && (!(command.buttons & jump_button) || !(2.0f > speed))) {
        return !feedback.was_grounded && (command.buttons & jump_button)
                   ? RawScoutGateResult::continue_air
                   : RawScoutGateResult::ground_speed_tail;
    }

    MovementCollisionTrace trace{};
    trace.fraction = 1.0f;
    trace.hitgroup = -1;
    trace.hitbox = -1;
    const float squared_input =
        command.forward_move * command.forward_move + command.side_move * command.side_move;
    if (frame.has_weapon && frame.weapon_id == 0x28 && 2.0f > speed &&
        !(command.buttons & 0x618u) && 4.0f > squared_input &&
        !clear_scout_path(frame, bindings, trace))
        return RawScoutGateResult::ground_speed_tail;

    if (!grounded)
        return RawScoutGateResult::continue_air;
    recover_bunny_hop_trace(
        settings.bunny_hop_enabled, {trace.fraction, trace.all_solid != 0, trace.start_solid != 0},
        (selection.movement_key_mode & 0xffu) != 0, command, settings.forced_forward_move);
    return RawScoutGateResult::ground_speed_tail;
}

} // namespace skeet

// ============================================================================
// Subcomponent: release_command.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;

/* deduplicated helper */
PreSpeedLimits limits(const MovementReleaseRuntime& runtime) noexcept {
    return {runtime.tuning.angle_scale, runtime.tuning.maximum_angle};
}
ZHopSettings z_hop(const MovementReleaseRuntime& runtime) noexcept {
    const auto& config = runtime.config;
    return {config.z_hop.value != 0, config.config_2b499e77.value.binding,
            config.config_3027dc11.value, config.config_3127de82.value};
}
AirStrafeSelection select_modes(const MovementReleaseRuntime& runtime, bool normalized,
                                const UserCommand& command) {
    const auto direction = static_cast<std::uint32_t>(runtime.config.air_strafe_direction.value);
    AirStrafeSelection result{true, (direction & 1u) != 0, (direction >> 1) & 0xFFFFFF01u, false,
                              false};
    result.pre_speed_requested =
        runtime.config.pre_speed.value &&
        hotkey_active(runtime.config.config_712c1936.value.binding, runtime.input);
    if (normalized && result.pre_speed_requested)
        result.pre_speed_effective = !convar_self_integer(runtime.pre_speed_guard);
    const bool no_input = command.side_move == 0.0f && command.forward_move == 0.0f;
    if ((result.movement_key_mode & 0xFFu) && !result.view_angles && !result.pre_speed_requested &&
        no_input)
        result.continue_solver = false;
    if (no_input)
        result.movement_key_mode &= 0xFFFFFF00u;
    return result;
}
AirVelocityPreparation prepare_live_velocity(MovementReleaseRuntime& runtime, void* player,
                                             const MovementSimulation& simulation,
                                             const AirStrafeSelection& selection,
                                             UserCommand& command) {
    auto velocity = simulation.velocity;
    float stamina = field<float>(player, runtime.capture.offsets.stamina_2f33c681);
    velocity.z -=
        (convar_float(runtime.capture.convars.gravity) * 0.5f) * (*runtime.environment.interval);
    if (!(simulation.player_flags & on_ground))
        return {velocity, stamina, true};
    if (!(command.buttons & jump_button))
        return {velocity, stamina, false};
    if (4.0f > velocity.y * velocity.y + velocity.x * velocity.x) {
        if (!(selection.movement_key_mode & 0xFFu) && runtime.config.bunny_hop.value)
            command.forward_move = runtime.tuning.side_magnitude;
        return {velocity, stamina, false};
    }
    if (stamina > 0.0f)
        stamina = sse_max(0.0f, stamina - convar_float(runtime.capture.convars.stamina_recovery) *
                                              (*runtime.environment.interval));
    if (convar_self_integer(runtime.jump_speed_cap)) {
        const float limit =
            field<float>(player, runtime.environment.fields.speed_limit_3eb71843) * 1.1f;
        const float speed = std::sqrt((velocity.x * velocity.x + velocity.y * velocity.y) +
                                      velocity.z * velocity.z);
        if (speed > limit) {
            const float scale = limit / speed;
            velocity.z *= scale;
            velocity.x *= scale;
            velocity.y *= scale;
        }
    }
    const auto& fields = runtime.environment.fields;
    const bool duck = field<std::uint8_t>(player, fields.ducking_6b1a59b5) != 0 ||
                      (field<std::uint8_t>(player, fields.player_flags_e2fbe695) & 2u);
    const float impulse = convar_float(runtime.capture.convars.jump_impulse);
    if (duck)
        velocity.z = impulse;
    else
        velocity.z += impulse;
    if (stamina > 0.0f)
        velocity.z *= sse_min(sse_max(1.0f - stamina / 100.0f, 0.0f), 1.0f);
    velocity.z -=
        (convar_float(runtime.capture.convars.gravity) * 0.5f) * (*runtime.environment.interval);
    return {velocity, stamina, true};
}
AirWallSteeringLiveBindings wall_bindings(MovementReleaseRuntime& runtime, void* player,
                                          const MovementVector& origin,
                                          const MovementVector& velocity) {
    return {player,
            runtime.environment,
            runtime.capture,
            runtime.collision,
            runtime.ballistics,
            runtime.surface_properties,
            origin,
            velocity,
            {runtime.config.config_af9ae891.value, runtime.config.air_strafe_smoothing.value,
             runtime.tuning.angle_scale, runtime.tuning.maximum_angle,
             runtime.tuning.side_magnitude, runtime.tuning.feedback_turn_limit,
             runtime.tuning.normalized_angle_step}};
}
float pre_speed_heading(MovementReleaseRuntime& runtime, void* player,
                        const MovementSimulation& simulation, MovementVector velocity,
                        const MovementVector& forecast_velocity, float previous, float heading) {
    const LivePreSpeedInput input{limits(runtime), velocity, forecast_velocity, simulation.position,
                                  previous};
    const LivePreSpeedBindings bindings{player, runtime.environment, runtime.capture,
                                        runtime.offsets.forecast_position_a9ee6062,
                                        runtime.collision};
    return choose_live_pre_speed_heading(input, heading, bindings);
}

void synthesize_normalized(MovementReleaseRuntime& runtime, void* player, void* weapon,
                           const MovementSimulation& simulation,
                           const MovementFeedbackHistory& history,
                           const AirStrafeSelection& selection, MovementFeedback& feedback,
                           client::Command& command, MovementAngleView angles) {
    const auto definition =
        weapon ? field<std::uint16_t>(weapon, runtime.offsets.weapon_definition_5c9f0fbb)
               : std::uint16_t(0);
    if (!pass_normalized_scout_gate({player, runtime.engine_client, simulation.velocity,
                                     simulation.position, weapon != nullptr, definition},
                                    command, runtime.collision,
                                    runtime.environment.engine_trace_flag))
        return;
    const auto prepared =
        prepare_live_velocity(runtime, player, simulation, selection, command.movement);
    if (!prepared.continue_solver ||
        !prepare_air_strafe_heading(selection, prepared.velocity, true, command.movement, angles))
        return;
    const auto& tuning = runtime.tuning;
    const NormalizedAirWallInput wall{
        runtime.config.avoid_collisions.value != 0,
        runtime.config.config_af9ae891.value,
        runtime.config.air_strafe_smoothing.value,
        tuning.maximum_angle,
        tuning.side_magnitude,
        tuning.feedback_turn_limit,
        tuning.normalized_angle_step,
        0.0f,
        selection.pre_speed_effective,
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(player)),
        simulation.position,
        prepared.velocity,
        0,
        -1};
    const float velocity_yaw = steer_live_normalized_air_wall(
        wall, feedback, command, angles,
        wall_bindings(runtime, player, simulation.position, prepared.velocity),
        runtime.air_acceleration);
    const float speed = horizontal_length(prepared.velocity);
    if (selection.pre_speed_requested) {
        if (selection.pre_speed_effective) {
            angles.yaw = pre_speed_heading(runtime, player, simulation, prepared.velocity,
                                           prepared.velocity, history.primary_heading, angles.yaw);
            const float ideal_step =
                minimum_number(tuning.maximum_angle, tuning.angle_scale / (speed + epsilon));
            if (resolve_pre_speed_feedback(history.primary_heading, ideal_step,
                                           tuning.feedback_turn_limit, tuning.side_magnitude,
                                           feedback, command.movement, angles))
                return;
        }
    } else {
        apply_z_hop_heading(z_hop(runtime), runtime.input, runtime.engine_time, angles);
    }
    // Advanced steering always retains its configured smoothing. The raw
    // solver's 100/0 overrides and guarded-heading mutation do not occur here.
    const NormalizedAirFinalSettings final_settings{
        runtime.config.air_strafe_smoothing.value, tuning.maximum_angle, tuning.side_magnitude,
        tuning.feedback_turn_limit, tuning.normalized_angle_step};
    const NormalizedAirFinalInput final_input{
        simulation.position, prepared.velocity, prepared.stamina, velocity_yaw, speed, selection};
    const NormalizedAirFinalBindings final_bindings{player,
                                                    runtime.environment,
                                                    runtime.capture,
                                                    runtime.collision,
                                                    runtime.ballistics,
                                                    runtime.surface_properties,
                                                    runtime.air_acceleration};
    finish_normalized_air_strafe(final_settings, final_input, history, feedback, command, angles,
                                 final_bindings);
}

void synthesize_raw(MovementReleaseRuntime& runtime, void* player, void* weapon,
                    const MovementSimulation& simulation, const MovementFeedbackHistory& history,
                    AirStrafeSelection selection, MovementFeedback& feedback,
                    client::Command& command, MovementAngleView angles) {
    auto velocity = simulation.velocity;
    const float speed = horizontal_length(velocity);
    const bool scout_reached = !(simulation.player_flags & on_ground) ||
                               ((command.movement.buttons & jump_button) && 2.0f > speed);
    const auto definition =
        scout_reached && weapon && 2.0f > speed
            ? field<std::uint16_t>(weapon, runtime.offsets.weapon_definition_5c9f0fbb)
            : std::uint16_t(0);
    const RawScoutGateFrame scout{velocity, simulation.player_flags, weapon != nullptr, definition,
                                  simulation.position};
    const RawScoutGateBindings scout_bindings{player, runtime.engine_client, runtime.collision,
                                              runtime.environment.engine_trace_flag};
    if (process_raw_scout_gate({runtime.config.bunny_hop.value != 0, runtime.tuning.side_magnitude},
                               selection, scout, feedback, command.movement,
                               scout_bindings) != RawScoutGateResult::continue_air)
        return;
    if (speed == 0.0f)
        velocity.z = simulation.velocity.z;
    if (!prepare_air_strafe_heading(selection, velocity, false, command.movement, angles))
        return;
    const auto& tuning = runtime.tuning;
    const AirWallSteeringInput wall{
        runtime.config.avoid_collisions.value != 0,
        runtime.config.config_af9ae891.value,
        tuning.angle_scale,
        tuning.maximum_angle,
        tuning.side_magnitude,
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(player)),
        simulation.position,
        velocity,
        0,
        -1};
    steer_live_air_wall(wall, feedback, command, angles,
                        wall_bindings(runtime, player, simulation.position, simulation.velocity));
    const float ideal_step =
        minimum_number(tuning.maximum_angle, tuning.angle_scale / (speed + epsilon));
    const float velocity_yaw = planar_movement_yaw(velocity);
    auto smoothing = runtime.config.air_strafe_smoothing.value;
    if (selection.pre_speed_requested) {
        selection.pre_speed_effective = !convar_self_integer(runtime.pre_speed_guard);
        if (selection.pre_speed_effective) {
            angles.yaw =
                pre_speed_heading(runtime, player, simulation, velocity, simulation.velocity,
                                  history.primary_heading, angles.yaw);
            if (resolve_pre_speed_feedback(history.primary_heading, ideal_step,
                                           tuning.feedback_turn_limit, tuning.side_magnitude,
                                           feedback, command.movement, angles))
                return;
            smoothing = 0;
        } else {
            smoothing = 100;
            apply_guarded_pre_speed_heading(speed, velocity_yaw, history.primary_heading,
                                            tuning.maximum_angle, angles);
        }
    } else if (apply_z_hop_heading(z_hop(runtime), runtime.input, runtime.engine_time, angles)) {
        smoothing = 0;
    }
    const AirSteeringInput steering{velocity_yaw, ideal_step,
                                    speed,        tuning.side_magnitude,
                                    smoothing,    (selection.movement_key_mode & 0xFFu) != 0};
    AirSteeringState state{feedback.side_flip != 0, feedback.primary_heading};
    write_air_strafe_step(steering, state, command.movement, angles);
    feedback.side_flip = static_cast<std::uint8_t>(state.side_flip);
    feedback.primary_heading = state.feedback_heading;
}
}

void synthesize_release_command(MovementReleaseRuntime& runtime, void* player, void* weapon,
                                client::Command& command, MovementSimulation& simulation,
                                MovementFeedback& feedback) {
    const MovementFeedbackHistory history{feedback.primary_heading, feedback.turn_direction,
                                          feedback.secondary_heading};
    feedback.primary_heading = std::numeric_limits<float>::max();
    feedback.turn_direction = 0.0f;
    feedback.secondary_heading = std::numeric_limits<float>::max();
    feedback.original_buttons = command.movement.buttons;
    if (simulation.movement_type == 2 &&
        field<std::uint8_t>(player, runtime.offsets.player_state_ac7f2913) < 2) {
        if (!(simulation.player_flags & on_ground)) {
            if (runtime.config.bunny_hop.value && convar_self_integer(runtime.bunny_hop_guard))
                command.movement.buttons &= ~jump_button;
            if (runtime.force_duck_mode == 1 && !(command.movement.buttons & back_button))
                command.movement.buttons |= duck_button;
        }
        if (runtime.config.air_strafe.value && !(feedback.original_buttons & speed_button)) {
            const bool normalized = runtime.config.config_ad536de0.value != 0;
            const auto selection = select_modes(runtime, normalized, command.movement);
            if (selection.continue_solver) {
                MovementAngleView angles{command.angles};
                if (normalized)
                    synthesize_normalized(runtime, player, weapon, simulation, history, selection,
                                          feedback, command, angles);
                else
                    synthesize_raw(runtime, player, weapon, simulation, history, selection,
                                   feedback, command, angles);
            }
        }
    }
    feedback.was_grounded = static_cast<std::uint8_t>((simulation.player_flags & on_ground) != 0);
}

} // namespace skeet

// ============================================================================
// Subcomponent: simulation.cpp
// ============================================================================

namespace skeet {

void initialize_movement_simulation(MovementSimulation& state, std::uint32_t player,
                                    MovementVector position, MovementVector velocity,
                                    std::uint32_t player_flags, std::uint32_t mode,
                                    const MovementSimulationPlayer& snapshot,
                                    const MovementSimulationRules& rules) noexcept {
    state.trace.hitgroup = -1;
    state.trace.hitbox = -1;
    state.player = player;
    state.trace.all_solid = 0;
    state.trace.start_solid = 0;
    state.trace.entity = 0;
    state.trace.fraction = 1.0f;
    state.rules = rules;
    state.position = position;
    state.velocity = velocity;
    state.player_flags = player_flags;
    state.mode = mode;
    state.speed_scale = snapshot.speed_scale;
    state.duck_speed = snapshot.duck_speed;
    state.duck_amount = snapshot.duck_amount;
    state.view_offset_z = snapshot.view_offset_z;
    state.stamina = snapshot.stamina;
    state.hull_minimum = snapshot.hull_minimum;
    state.hull_maximum = snapshot.hull_maximum;
    state.ducking = snapshot.ducking;
    state.fall_velocity = snapshot.fall_velocity;
    state.surface_friction = snapshot.surface_friction;
    state.movement_type = snapshot.movement_type;
    state.collision_mask = snapshot.collision_mask;
    state.simulation_flags =
        (state.simulation_flags & 0xbfu) | ((snapshot.field_f3c10e80_00 & 1u) << 6);
}

} // namespace skeet

// ============================================================================
// Subcomponent: simulation_capture.cpp
// ============================================================================

namespace skeet {
MovementSimulationPlayer capture_movement_player(void* player,
                                                 const MovementCaptureOffsets& offsets) {
    MovementSimulationPlayer result;
    result.speed_scale = field<float>(player, offsets.speed_scale_7f5ed2d0);
    result.duck_speed = field<float>(player, offsets.duck_speed_fb7e3404);
    result.duck_amount = field<float>(player, offsets.duck_amount_3309c0cf);
    result.view_offset_z = field<float>(player, offsets.view_offset_c246315e + 8u);
    result.stamina = field<float>(player, offsets.stamina_2f33c681);
    result.hull_minimum = *engine::invoke<const MovementVector*>(player, 76);
    result.hull_maximum = *engine::invoke<const MovementVector*>(player, 77);
    result.ducking = field<std::uint8_t>(player, offsets.ducking_6b1a59b5);
    result.fall_velocity = field<float>(player, offsets.fall_velocity_7e9ba7bc);
    result.surface_friction = field<float>(player, offsets.field_f3c10e80 + 0x14u);
    result.movement_type = field<std::uint8_t>(player, offsets.movement_type_7501cb91);
    result.collision_mask = engine::invoke<std::uint32_t>(player, 152);
    result.field_f3c10e80_00 = field<std::uint8_t>(player, offsets.field_f3c10e80);
    return result;
}
MovementSimulationRules capture_movement_rules(const MovementCaptureConvars& convars,
                                               float interval) noexcept {
    MovementSimulationRules result;
    result.gravity = convar_float(convars.gravity);
    result.jump_impulse = convar_float(convars.jump_impulse);
    result.step_size = convar_float(convars.step_size);
    result.stamina_recovery = convar_float(convars.stamina_recovery);
    result.stamina_land = convar_float(convars.stamina_land);
    result.stamina_jump = convar_float(convars.stamina_jump);
    result.stamina_maximum = convar_float(convars.stamina_maximum);
    result.stop_speed = convar_float(convars.stop_speed);
    result.friction = convar_float(convars.friction);
    result.accelerate = convar_float(convars.accelerate);
    result.cvar_dceaeac8 = convar_float(convars.cvar_dceaeac8);
    result.interval_per_tick = interval;
    return result;
}
void initialize_live_movement_simulation(MovementSimulation& state, void* player,
                                         MovementVector position, MovementVector velocity,
                                         std::uint32_t player_flags, std::uint32_t mode,
                                         const MovementCaptureInput& input) {
    const auto rules = capture_movement_rules(input.convars, input.interval);
    const auto snapshot = capture_movement_player(player, input.offsets);
    initialize_movement_simulation(
        state, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(player)), position,
        velocity, player_flags, mode, snapshot, rules);
}
} // namespace skeet

// ============================================================================
// Subcomponent: simulation_collision.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;

void trace_slide(MovementSimulation& state, MovementCollisionEngine& engine, MovementVector start,
                 MovementVector end, MovementCollisionTrace& trace) {
    engine.trace_ray(make_movement_hull_ray(start, end, state.hull_minimum, state.hull_maximum),
                     state.collision_mask,
                     {MovementCollisionFilterKind::skip_player, state.player, 8, 0}, trace);
}

void record_plane(MovementSimulation& state, MovementVector normal) noexcept {
    const auto original = state.simulation_flags;
    state.simulation_flags = original | 8u;
    state.contact_position = state.position;
    if (normal.z >= 0.7f)
        state.simulation_flags = original | 0xcu;
    else if (0.1f > absolute(normal.z))
        state.simulation_flags = original | 0xau;
    else
        return;
    state.contact_normal = normal;
}

MovementVector clip_velocity(MovementVector velocity, MovementVector normal) noexcept {
    const float amount = (normal.x * velocity.x + normal.y * velocity.y) + velocity.z * normal.z;
    MovementVector result{velocity.x - normal.x * amount, velocity.y - normal.y * amount,
                          velocity.z - normal.z * amount};
    const float residual = (normal.y * result.y + normal.x * result.x) + result.z * normal.z;
    if (!(residual >= 0.0f)) {
        const float correction = minimum_number(residual, -0.03125f);
        result.x -= normal.x * correction;
        result.y -= normal.y * correction;
        result.z -= correction * normal.z;
    }
    return result;
}

MovementVector crease_velocity(MovementVector velocity, MovementVector a,
                               MovementVector b) noexcept {
    const MovementVector direction{a.y * b.z - a.z * b.y, b.x * a.z - a.x * b.z,
                                   a.x * b.y - b.x * a.y};
    const float length = std::sqrt((direction.x * direction.x + direction.y * direction.y) +
                                   direction.z * direction.z);
    const float factor = 1.0f / (length + epsilon);
    const MovementVector normalized{factor * direction.x, factor * direction.y,
                                    factor * direction.z};
    const float along =
        (velocity.y * normalized.y + normalized.x * velocity.x) + velocity.z * normalized.z;
    return {along * normalized.x, normalized.y * along, normalized.z * along};
}
}

void slide_movement_simulation(MovementSimulation& state, MovementCollisionEngine& engine,
                               const MovementVector* first_destination) {
    using namespace movement_detail;
    const MovementVector original_velocity = state.velocity;
    MovementVector anchor = state.velocity;
    const MovementVector reuse = first_destination ? *first_destination : MovementVector{};
    std::array<MovementVector, 5> planes;
    unsigned count = 0;
    float remaining = state.rules.interval_per_tick;
    float fraction_sum = 0.0f;
    for (unsigned bump = 0; bump < 4; ++bump) {
        const float squared =
            (state.velocity.x * state.velocity.x + state.velocity.y * state.velocity.y) +
            state.velocity.z * state.velocity.z;
        if (squared == 0.0f)
            break;
        const MovementVector end{remaining * state.velocity.x + state.position.x,
                                 remaining * state.velocity.y + state.position.y,
                                 remaining * state.velocity.z + state.position.z};
        if (!(first_destination && end.x == reuse.x && end.y == reuse.y && end.z == reuse.z))
            trace_slide(state, engine, state.position, end, state.trace);
        float fraction = state.trace.fraction;
        if (fraction > 0.0f && fraction < 0.0001f)
            state.trace.fraction = fraction = 0.0f;
        fraction_sum += fraction;
        if (state.trace.all_solid) {
            state.velocity = {};
            break;
        }
        if (fraction > 0.0f) {
            if (fraction == 1.0f) {
                MovementCollisionTrace probe{};
                probe.fraction = 1.0f;
                probe.hitgroup = probe.hitbox = -1;
                trace_slide(state, engine, state.trace.end, state.trace.end, probe);
                if (probe.start_solid || probe.fraction != 1.0f) {
                    state.velocity = {};
                    break;
                }
            }
            state.position = state.trace.end;
            anchor = state.velocity;
            count = 0;
        }
        if (fraction == 1.0f)
            break;
        if (!(absolute(state.trace.plane_normal.z) >= 0.0001f))
            state.trace.plane_normal.z = 0.0f;
        remaining -= remaining * fraction;
        if (count > 4) {
            state.velocity = {};
            break;
        }
        planes[count++] = state.trace.plane_normal;
        if (count == 1 && state.movement_type == 2 && !(state.player_flags & on_ground)) {
            record_plane(state, planes[0]);
            anchor = clip_velocity(anchor, planes[0]);
            state.velocity = anchor;
        } else {
            unsigned selected = 0;
            for (; selected < count; ++selected) {
                record_plane(state, planes[selected]);
                state.velocity = clip_velocity(anchor, planes[selected]);
                unsigned other = 0;
                for (; other < count; ++other)
                    if (other != selected && dot(state.velocity, planes[other]) < 0.0f)
                        break;
                if (other == count)
                    break;
            }
            if (selected == count) {
                if (count != 2) {
                    state.velocity = {};
                    break;
                }
                state.velocity = crease_velocity(state.velocity, planes[0], planes[1]);
            }
            if (!(dot(state.velocity, original_velocity) > 0.0f)) {
                state.velocity = {};
                break;
            }
        }
    }
    if (fraction_sum == 0.0f)
        state.velocity = {};
}

} // namespace skeet

// ============================================================================
// Subcomponent: simulation_command.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;

namespace {
void command_prefix(MovementSimulation& state, std::uint32_t player,
                    const UserCommand& command) noexcept {
    state.player = player;
    state.saved_vertical_velocity = state.velocity.z;
    state.buttons = command.buttons;
    state.command_move = {command.forward_move, command.side_move, command.up_move};
    state.state_5c = 0.0f;
    if (!(state.player_flags & on_ground))
        state.fall_velocity = negate(state.velocity.z);
    state.simulation_flags &= 0xc0u;
}
void command_speed(MovementSimulation& state, float player_speed_limit,
                   float player_max_speed) noexcept {
    float maximum = minimum_number(player_speed_limit, player_max_speed);
    if (state.buttons & speed_button) {
        const float walking_maximum = maximum * 0.52f;
        const float speed =
            std::sqrt((state.velocity.y * state.velocity.y + state.velocity.z * state.velocity.z) +
                      state.velocity.x * state.velocity.x);
        if (walking_maximum + 25.0f > speed)
            maximum = walking_maximum;
    }
    if (state.player_flags & on_ground)
        maximum *= state.speed_scale;
    if (state.stamina > 0.0f) {
        const float factor = sse_min(sse_max(1.0f - state.stamina / 100.0f, 0.0f), 1.0f);
        maximum = (factor * factor) * maximum;
    }
    state.maximum_speed = maximum;

    const float squared_input = (state.command_move.x * state.command_move.x +
                                 state.command_move.y * state.command_move.y) +
                                state.command_move.z * state.command_move.z;
    if (squared_input != 0.0f && squared_input > maximum * maximum) {
        const float scale = maximum / std::sqrt(squared_input);
        state.command_move.x *= scale;
        state.command_move.y *= scale;
        state.command_move.z *= scale;
    }
    if (state.stamina > 0.0f) {
        state.stamina -= state.rules.interval_per_tick * state.rules.stamina_recovery;
        if (0.0f > state.stamina)
            state.stamina = 0.0f;
    }
}
}

void prepare_movement_simulation_command(MovementSimulation& state, std::uint32_t player,
                                         float player_speed_limit, float player_max_speed,
                                         const UserCommand& command) noexcept {
    command_prefix(state, player, command);
    command_speed(state, player_speed_limit, player_max_speed);
}

void prepare_live_movement_simulation_command(MovementSimulation& state, void* player,
                                              const UserCommand& command,
                                              const MovementEnvironmentBindings& bindings) {
    command_prefix(state, static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(player)),
                   command);
    const auto physics = capture_movement_command_physics(player, bindings);
    command_speed(state, physics.player_speed_limit, physics.player_max_speed);
}

} // namespace skeet

// ============================================================================
// Subcomponent: simulation_duck.cpp
// ============================================================================

namespace skeet {
namespace {
float approach(float target, float value, float step) noexcept {
    const float difference = target - value;
    if (difference > step)
        return value + step;
    if (movement_detail::negate(step) > difference)
        return value - step;
    return target;
}
void update_duck(MovementSimulation& state, MovementCollisionEngine& engine, bool field_a15c3105,
                 void* player, const MovementEnvironmentBindings* live) {
    const bool grounded = (state.player_flags & 1u) != 0;
    state.duck_speed = approach(8.0f, state.duck_speed, state.rules.interval_per_tick * 3.0f);
    if ((state.buttons & 4u) == 0) {
        if (state.duck_amount > 0.0f)
            state.ducking = 1;
        else if (!state.ducking)
            return;
        if (grounded)
            return;
        float rate = maximum_number(1.5f, state.duck_speed);
        if (live ? movement_player_duck_slowdown(player, *live) : field_a15c3105)
            rate *= 0.4f;
        state.duck_amount = approach(0.0f, state.duck_amount, state.rules.interval_per_tick * rate);
        if ((state.player_flags & 1u) == 0 && state.movement_type != 9)
            state.position.z -= 9.0f;
        state.player_flags &= ~6u;
        state.ducking = 0;
        state.view_offset_z = 64.0f;
        categorize_movement_ground(state, engine, true, true);
        state.duck_amount = 0.0f;
        state.player_flags &= ~6u;
        return;
    }
    if (1.0f > state.duck_amount)
        state.ducking = 1;
    if (!state.ducking)
        return;
    float rate = state.duck_speed * 0.8f;
    if (live ? movement_player_duck_slowdown(player, *live) : field_a15c3105)
        rate *= 0.4f;
    state.duck_amount = approach(1.0f, state.duck_amount, rate * state.rules.interval_per_tick);
    if (state.duck_amount < 1.0f && grounded) {
        const float square = state.duck_amount * state.duck_amount;
        const float twice = square + square;
        const float eased = square * 3.0f - twice * state.duck_amount;
        state.view_offset_z = (1.0f - eased) * 18.0f + 46.0f;
    } else {
        if (!grounded && state.movement_type != 9)
            state.position.z += 9.0f;
        state.view_offset_z = 46.0f;
        state.ducking = 0;
        state.player_flags |= 6u;
        categorize_movement_ground(state, engine, true, true);
        state.duck_amount = 1.0f;
    }
    if (state.duck_amount >= 0.1f)
        state.player_flags |= 4u;
}
}
void update_movement_simulation_duck(MovementSimulation& state, MovementCollisionEngine& engine,
                                     bool field_a15c3105) {
    update_duck(state, engine, field_a15c3105, nullptr, nullptr);
}
void update_live_movement_simulation_duck(MovementSimulation& state,
                                          MovementCollisionEngine& engine, void* player,
                                          const MovementEnvironmentBindings& bindings) {
    update_duck(state, engine, false, player, &bindings);
}
} // namespace skeet

// ============================================================================
// Subcomponent: simulation_environment.cpp
// ============================================================================

namespace skeet {

MovementPhysics capture_movement_command_physics(void* player,
                                                 const MovementEnvironmentBindings& bindings) {
    MovementPhysics result{};
    result.player_max_speed = engine::invoke<float>(player, 0x450u / 4u);
    result.player_speed_limit = field<float>(player, bindings.fields.speed_limit_3eb71843);
    return result;
}

MovementGroundWeaponPhysics
capture_movement_ground_weapon_physics(void* player, const UserCommand& command,
                                       std::uint32_t simulation_player_flags,
                                       const MovementEnvironmentBindings& bindings) {
    MovementGroundWeaponPhysics result{};
    auto& physics = result.physics;
    physics.player_flags = simulation_player_flags;
    if (!(command.buttons & duck_button))
        physics.ducking = field<std::uint8_t>(player, bindings.fields.ducking_6b1a59b5) != 0;
    result.duck =
        (command.buttons & duck_button) || physics.ducking || (simulation_player_flags & 6u);
    result.walk = (command.buttons & speed_button) && !result.duck;
    physics.weapon_speed_enabled = !convar_self_integer(bindings.convars.accelerate_weapon_speed);
    if (!physics.weapon_speed_enabled)
        return result;
    const auto handle = field<std::int32_t>(player, bindings.fields.weapon_handle_cd812654);
    if (handle == -1)
        return result;
    auto* const weapon = engine::invoke<void*>(bindings.entity_list, 4, handle);
    if (!weapon)
        return result;
    physics.has_weapon = true;
    physics.weapon_max_speed = engine::invoke<float>(weapon, 0x6E8u / 4u);
    physics.weapon_mode_count = engine::invoke<std::int32_t>(weapon, 0x738u / 4u);
    if (physics.weapon_mode_count > 0)
        physics.weapon_mode = engine::invoke<std::int32_t>(weapon, 0x6ECu / 4u);
    return result;
}

bool movement_player_duck_slowdown(void* player,
                                   const MovementEnvironmentBindings& bindings) noexcept {
    return field<std::uint8_t>(player, bindings.fields.field_a15c3105) != 0;
}
bool movement_jump_speed_cap(const MovementEnvironmentBindings& bindings) noexcept {
    return convar_self_integer(bindings.convars.jump_speed_cap);
}
bool movement_landing_stamina_present(const MovementEnvironmentBindings& bindings) noexcept {
    return bindings.convars.stamina_land != nullptr;
}

} // namespace skeet

// ============================================================================
// Subcomponent: simulation_finish.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;

void finish_movement_simulation_tick(MovementSimulation& state, bool landing_stamina_cvar_present,
                                     bool advance_vertical) noexcept {
    if (advance_vertical && !(state.player_flags & on_ground)) {
        state.velocity.z -= (state.rules.interval_per_tick * state.rules.gravity) * 0.5f;
        if (-580.0f > state.saved_vertical_velocity && state.velocity.z >= -580.0f)
            state.simulation_flags |= 0x10u;
    } else if (advance_vertical) {
        state.velocity.z = 0.0f;
        // COMISS zero, fall followed by JAE also enters on unordered input.
        if (!(0.0f >= state.fall_velocity)) {
            if (landing_stamina_cvar_present) {
                const float stamina =
                    state.rules.stamina_land * state.fall_velocity + state.stamina;
                state.stamina = sse_min(sse_max(stamina, 0.0f), state.rules.stamina_maximum);
            }
            state.fall_velocity = 0.0f;
        }
    }
    if (state.player_flags & on_ground) {
        if (1.0f > state.speed_scale) {
            const float scale = state.speed_scale - state.rules.interval_per_tick * -0.4f;
            state.speed_scale = sse_min(sse_max(scale, 0.0f), 1.0f);
        }
        state.fall_velocity = 0.0f;
    }
    ++state.mode;
}

} // namespace skeet

// ============================================================================
// Subcomponent: simulation_step.cpp
// ============================================================================

namespace skeet {
namespace {
void trace_step(MovementSimulation& state, MovementCollisionEngine& engine, MovementVector start,
                MovementVector end, MovementCollisionTrace& trace) {
    engine.trace_ray(make_movement_hull_ray(start, end, state.hull_minimum, state.hull_maximum),
                     state.collision_mask,
                     {MovementCollisionFilterKind::skip_player, state.player, 8, 0}, trace);
}

void retain_ground(MovementSimulation& state, MovementCollisionEngine& engine) {
    MovementVector upper = state.position, lower = state.position;
    upper.z += 2.0f;
    lower.z -= state.rules.step_size;
    MovementCollisionTrace trace{};
    trace.fraction = 1.0f;
    trace.hitgroup = trace.hitbox = -1;
    trace_step(state, engine, state.position, upper, trace);
    upper = trace.end;
    trace_step(state, engine, upper, lower, trace);
    if (trace.fraction > 0.0f && trace.fraction < 1.0f && !trace.start_solid &&
        static_cast<double>(trace.plane_normal.z) >= 0.7 &&
        movement_detail::absolute(state.position.z - trace.end.z) > 0.015625f)
        state.position = trace.end;
}

void step_move(MovementSimulation& state, MovementCollisionEngine& engine,
               MovementVector first_destination) {
    const auto original_position = state.position;
    const auto original_velocity = state.velocity;
    slide_movement_simulation(state, engine, &first_destination);
    const auto flat_position = state.position;
    const auto flat_velocity = state.velocity;
    state.position = original_position;
    state.velocity = original_velocity;
    MovementVector upper = state.position;
    upper.z = static_cast<float>((static_cast<double>(state.rules.step_size) + 0.03125) +
                                 static_cast<double>(upper.z));
    trace_step(state, engine, state.position, upper, state.trace);
    if (!state.trace.start_solid && !state.trace.all_solid)
        state.position = state.trace.end;
    slide_movement_simulation(state, engine);
    MovementVector lower = state.position;
    lower.z = static_cast<float>(static_cast<double>(lower.z) -
                                 (static_cast<double>(state.rules.step_size) + 0.03125));
    trace_step(state, engine, state.position, lower, state.trace);
    if (static_cast<double>(state.trace.plane_normal.z) < 0.7) {
        state.position = flat_position;
        state.velocity = flat_velocity;
        return;
    }
    if (!state.trace.start_solid && !state.trace.all_solid)
        state.position = state.trace.end;
    const float flat_x = flat_position.x - original_position.x;
    const float flat_y = flat_position.y - original_position.y;
    const float step_x = state.position.x - original_position.x;
    const float step_y = state.position.y - original_position.y;
    if (flat_y * flat_y + flat_x * flat_x > step_x * step_x + step_y * step_y) {
        state.position = flat_position;
        state.velocity = flat_velocity;
    } else {
        state.velocity.z = flat_velocity.z;
    }
}
}

void move_movement_simulation_ground(MovementSimulation& state, MovementCollisionEngine& engine) {
    const auto destination = trace_movement_simulation_ground(state, engine);
    if (state.trace.fraction == 1.0f) {
        state.position = state.trace.end;
    } else {
        if (!(state.player_flags & on_ground)) {
            finish_movement_simulation_ground(state);
            return;
        }
        step_move(state, engine, destination);
    }
    retain_ground(state, engine);
    finish_movement_simulation_ground(state);
}

} // namespace skeet

// ============================================================================
// Subcomponent: simulation_tick.cpp
// ============================================================================

namespace skeet {

void simulate_movement_tick(MovementSimulation& state, MovementCollisionEngine& engine,
                            const MovementSimulationEnvironment& environment, std::uint32_t player,
                            const UserCommand& command, MovementAngles command_angles) {
    MovementTraceFlagGuard trace_flag(environment.engine_trace_flag);
    prepare_movement_simulation_command(state, player, environment.physics.player_speed_limit,
                                        environment.physics.player_max_speed, command);
    prepare_movement_simulation_ground(state, engine);
    if (!(state.player_flags & on_ground))
        state.fall_velocity = movement_detail::negate(state.velocity.z);
    update_movement_simulation_duck(state, engine, environment.player_field_a15c3105);
    const auto path = prepare_movement_simulation_velocity(
        state, environment.physics, command, command_angles, environment.jump_speed_cap_enabled);
    if (path == MovementVelocityPath::skip_walk) {
        finish_movement_simulation_tick(state, environment.landing_stamina_cvar_present, false);
        return;
    }
    if (path == MovementVelocityPath::ground)
        move_movement_simulation_ground(state, engine);
    else if (path == MovementVelocityPath::ground_stopped)
        finish_movement_simulation_ground(state);
    else
        slide_movement_simulation(state, engine);
    categorize_movement_ground(state, engine, true);
    finish_movement_simulation_tick(state, environment.landing_stamina_cvar_present);
}

namespace {
void live_tick(MovementSimulation& state, MovementCollisionEngine& engine,
               const MovementEnvironmentBindings& bindings, void* player,
               const UserCommand& command, MovementAngles angles,
               const client::Command* live_command) {
    MovementTraceFlagGuard trace_flag(bindings.engine_trace_flag);
    prepare_live_movement_simulation_command(state, player, command, bindings);
    prepare_movement_simulation_ground(state, engine);
    if (!(state.player_flags & on_ground))
        state.fall_velocity = movement_detail::negate(state.velocity.z);
    update_live_movement_simulation_duck(state, engine, player, bindings);
    const auto path =
        live_command
            ? prepare_live_movement_simulation_velocity(state, *live_command, player, bindings)
            : prepare_live_movement_simulation_velocity(state, command, angles, player, bindings);
    if (path == MovementVelocityPath::skip_walk) {
        finish_movement_simulation_tick(state, false, false);
        return;
    }
    if (path == MovementVelocityPath::ground)
        move_movement_simulation_ground(state, engine);
    else if (path == MovementVelocityPath::ground_stopped)
        finish_movement_simulation_ground(state);
    else
        slide_movement_simulation(state, engine);
    categorize_movement_ground(state, engine, true);
    bool landing_stamina_present = false;
    if ((state.player_flags & on_ground) && !(0.0f >= state.fall_velocity))
        landing_stamina_present = movement_landing_stamina_present(bindings);
    finish_movement_simulation_tick(state, landing_stamina_present);
}
}

void simulate_live_movement_tick(MovementSimulation& state, MovementCollisionEngine& engine,
                                 const MovementEnvironmentBindings& bindings, void* player,
                                 const UserCommand& command, MovementAngles angles) {
    live_tick(state, engine, bindings, player, command, angles, nullptr);
}

void simulate_live_movement_tick(MovementSimulation& state, MovementCollisionEngine& engine,
                                 const MovementEnvironmentBindings& bindings, void* player,
                                 const client::Command& command) {
    live_tick(state, engine, bindings, player, command.movement, {}, &command);
}

} // namespace skeet

// ============================================================================
// Subcomponent: simulation_velocity.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;
void scale(MovementVector& value, float factor) noexcept {
    value.x *= factor;
    value.y *= factor;
    value.z *= factor;
}
MovementVector horizontal_normalize(MovementVector value) noexcept {
    value.z = 0.0f;
    const float inverse = 1.0f / (length(value) + epsilon);
    scale(value, inverse);
    return value;
}
float dot_native(MovementVector left, MovementVector right) noexcept {
    return (left.x * right.x + left.y * right.y) + left.z * right.z;
}
void add_acceleration(MovementVector& velocity, MovementVector direction, float amount) noexcept {
    velocity.x = direction.x * amount + velocity.x;
    velocity.y = direction.y * amount + velocity.y;
    velocity.z = direction.z * amount + velocity.z;
}
MovementVelocityPath ground_velocity(MovementSimulation& state, const MovementPhysics& snapshot,
                                     const UserCommand& command, MovementAngles angles,
                                     void* player, const MovementEnvironmentBindings* live,
                                     const client::Command* live_command) {
    state.velocity.z = 0.0f;
    state.fall_velocity = 0.0f;
    const float speed = length(state.velocity);
    if (speed >= 0.1f) {
        const float control = maximum_number(state.rules.stop_speed, speed);
        const float drop = (state.rules.friction * control) * state.rules.interval_per_tick;
        const float remaining = maximum_number(speed - drop, 0.0f);
        if (remaining != speed)
            scale(state.velocity, remaining / speed);
    }
    MovementVector forward, right;
    if (live_command)
        angles = {live_command->angles[0], live_command->angles[1], live_command->angles[2]};
    movement_angle_vectors(angles, &forward, &right, nullptr);
    if (forward.z != 0.0f)
        forward = horizontal_normalize(forward);
    if (right.z != 0.0f)
        right = horizontal_normalize(right);
    MovementVector direction{state.command_move.x * forward.x + state.command_move.y * right.x,
                             state.command_move.x * forward.y + state.command_move.y * right.y,
                             0.0f};
    float wish_speed = length(direction);
    const float inverse = 1.0f / (wish_speed + epsilon);
    scale(direction, inverse);
    if (wish_speed != 0.0f)
        wish_speed = sse_min(state.maximum_speed, wish_speed);
    state.velocity.z = 0.0f;
    const float current = dot_native(state.velocity, direction);
    const float additional = wish_speed - current;
    if (!(0.0f >= additional)) {
        float acceleration = state.rules.accelerate;
        MovementPhysics physics;
        bool duck, walk;
        if (live) {
            const auto captured =
                capture_movement_ground_weapon_physics(player, command, state.player_flags, *live);
            physics = captured.physics;
            duck = captured.duck;
            walk = captured.walk;
        } else {
            physics = snapshot;
            duck = (command.buttons & duck_button) || physics.ducking || (state.player_flags & 6);
            walk = (command.buttons & speed_button) && !duck;
        }
        const bool exception = physics.weapon_speed_enabled && physics.has_weapon &&
                               weapon_slowdown_exception(physics);
        const float basis = maximum_number(wish_speed, 250.0f);
        float walking_limit = basis;
        float acceleration_basis = basis;
        if (physics.weapon_speed_enabled && physics.has_weapon) {
            const float weapon_scale = physics.weapon_max_speed / 250.0f;
            if (1.0f > weapon_scale)
                walking_limit = weapon_scale * basis;
            if ((!duck && !walk) || exception) {
                if (1.0f >= weapon_scale)
                    acceleration_basis = weapon_scale * basis;
            }
        }
        if (duck) {
            if (!exception)
                acceleration_basis *= 0.34f;
            walking_limit *= 0.34f;
        }
        if (walk) {
            if (!exception)
                acceleration_basis *= 0.52f;
            walking_limit *= 0.52f;
            acceleration = walking_acceleration(acceleration, current, walking_limit);
        }
        const float amount = minimum_number(
            additional, ((state.rules.interval_per_tick * acceleration) * acceleration_basis) *
                            state.surface_friction);
        add_acceleration(state.velocity, direction, amount);
    }
    state.velocity.z = 0.0f;
    const float squared =
        (state.velocity.x * state.velocity.x + state.velocity.y * state.velocity.y) +
        state.velocity.z * state.velocity.z;
    if (squared > state.maximum_speed * state.maximum_speed) {
        const float divisor =
            std::sqrt((state.velocity.z * state.velocity.z + state.velocity.x * state.velocity.x) +
                      state.velocity.y * state.velocity.y);
        scale(state.velocity, state.maximum_speed / divisor);
    }
    if (1.0f > length(state.velocity)) {
        state.velocity = {0.0f, 0.0f, 0.0f};
        return MovementVelocityPath::ground_stopped;
    }
    return MovementVelocityPath::ground;
}
void air_velocity(MovementSimulation& state, const UserCommand& command, MovementAngles angles,
                  const client::Command* live_command) noexcept {
    MovementVector forward, right;
    if (live_command)
        angles = {live_command->angles[0], live_command->angles[1], live_command->angles[2]};
    movement_angle_vectors(angles, &forward, &right, nullptr);
    forward = horizontal_normalize(forward);
    right = horizontal_normalize(right);
    MovementVector direction{command.forward_move * forward.x + command.side_move * right.x,
                             command.forward_move * forward.y + command.side_move * right.y, 0.0f};
    float wish_speed = length(direction);
    const float inverse = 1.0f / (wish_speed + epsilon);
    scale(direction, inverse);
    if (wish_speed != 0.0f)
        wish_speed = sse_min(state.maximum_speed, wish_speed);
    const float capped = minimum_number(30.0f, wish_speed);
    const float additional = capped - dot_native(state.velocity, direction);
    if (!(0.0f >= additional)) {
        const float amount = minimum_number(
            additional, ((state.rules.cvar_dceaeac8 * wish_speed) * state.rules.interval_per_tick) *
                            state.surface_friction);
        add_acceleration(state.velocity, direction, amount);
    }
}
}

namespace {
MovementVelocityPath prepare_velocity(MovementSimulation& state, const MovementPhysics& physics,
                                      const UserCommand& command, MovementAngles angles,
                                      bool jump_speed_cap_enabled, void* player,
                                      const MovementEnvironmentBindings* live,
                                      const client::Command* live_command) {
    using namespace movement_detail;
    if (0.0f >= state.duck_amount)
        state.player_flags &= ~4u;
    if (!(state.simulation_flags & 1)) {
        state.simulation_flags |= 1;
        if ((state.buttons & duck_button) || state.ducking || (state.player_flags & 2)) {
            const float factor = (state.duck_amount * 0.34f + 1.0f) - state.duck_amount;
            scale(state.command_move, factor);
            state.maximum_speed *= factor;
            state.simulation_flags |= 1;
        }
    }
    if (state.movement_type != 2)
        return MovementVelocityPath::skip_walk;
    const float half_gravity = (state.rules.interval_per_tick * state.rules.gravity) * 0.5f;
    state.velocity.z -= half_gravity;
    if (state.buttons & jump_button) {
        state.simulation_flags &= ~0x40u;
        if (state.player_flags & on_ground) {
            if (live ? movement_jump_speed_cap(*live) : jump_speed_cap_enabled) {
                const float maximum = state.maximum_speed * 1.1f;
                if (0.0f < maximum) {
                    const float speed = length(state.velocity);
                    if (speed > maximum)
                        scale(state.velocity, maximum / speed);
                }
            }
            state.player_flags &= ~on_ground;
            if (!state.ducking && !(state.player_flags & 2))
                state.velocity.z += state.rules.jump_impulse;
            else
                state.velocity.z = state.rules.jump_impulse;
            const float previous_stamina = state.stamina;
            if (previous_stamina > 0.0f) {
                const float factor = sse_min(sse_max(1.0f - previous_stamina / 100.0f, 0.0f), 1.0f);
                state.velocity.z *= factor;
            }
            state.velocity.z -= half_gravity;
            state.stamina = sse_min(
                sse_max(state.rules.stamina_jump * state.rules.jump_impulse + previous_stamina,
                        0.0f),
                state.rules.stamina_maximum);
        }
    }
    if (state.player_flags & on_ground)
        return ground_velocity(state, physics, command, angles, player, live, live_command);
    air_velocity(state, command, angles, live_command);
    return MovementVelocityPath::air;
}
}

MovementVelocityPath prepare_movement_simulation_velocity(MovementSimulation& state,
                                                          const MovementPhysics& physics,
                                                          const UserCommand& command,
                                                          MovementAngles angles,
                                                          bool jump_speed_cap_enabled) noexcept {
    return prepare_velocity(state, physics, command, angles, jump_speed_cap_enabled, nullptr,
                            nullptr, nullptr);
}

MovementVelocityPath
prepare_live_movement_simulation_velocity(MovementSimulation& state, const UserCommand& command,
                                          MovementAngles angles, void* player,
                                          const MovementEnvironmentBindings& bindings) {
    return prepare_velocity(state, MovementPhysics{}, command, angles, false, player, &bindings,
                            nullptr);
}

MovementVelocityPath
prepare_live_movement_simulation_velocity(MovementSimulation& state, const client::Command& command,
                                          void* player,
                                          const MovementEnvironmentBindings& bindings) {
    return prepare_velocity(state, MovementPhysics{}, command.movement, {}, false, player,
                            &bindings, &command);
}

MovementVector trace_movement_simulation_ground(MovementSimulation& state,
                                                MovementCollisionEngine& engine) {
    const MovementVector end{state.velocity.x * state.rules.interval_per_tick + state.position.x,
                             state.velocity.y * state.rules.interval_per_tick + state.position.y,
                             state.position.z};
    engine.trace_ray(
        make_movement_hull_ray(state.position, end, state.hull_minimum, state.hull_maximum),
        state.collision_mask, {MovementCollisionFilterKind::skip_player, state.player, 8, 0},
        state.trace);
    return end;
}

void finish_movement_simulation_ground(MovementSimulation& state) noexcept {
    state.simulation_flags |= 0x40;
}
} // namespace skeet

// ============================================================================
// Subcomponent: slow_motion.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;

bool process_slow_motion_stage(const SlowMotionSettings& settings, InputBitmap input,
                               const MovementPhysics& physics, std::int32_t weapon_state,
                               bool weapon_blocked, std::uint32_t& scheduler_flags,
                               std::int32_t command_number, UserCommand& command,
                               MovementVector velocity, MovementAngles command_angles,
                               MovementAngleView movement_angles) {
    if (!(settings.enabled && hotkey_active(settings.hotkey, input)) && !(scheduler_flags & 0x40u))
        return false;
    if ((scheduler_flags & 8u) && weapon_state != -1 && !(weapon_state & 4))
        return false;

    scheduler_flags = (scheduler_flags & 0xfffedfe7u) | ((scheduler_flags & 8u) << 10) | 0x80u;
    if (weapon_blocked)
        return true;

    const float ceiling =
        (minimum_number(physics.player_speed_limit, physics.player_max_speed) - 1.0f) * 0.34f;
    const float phase =
        (static_cast<float>(command_number % 1000) * physics.interval_per_tick) * 7.0f;
    float wave = phase - static_cast<float>(std::floor(static_cast<double>(phase)));
    wave = absolute(wave - 0.5f);
    wave += wave;
    wave *= 4.0f;
    regulate_movement_speed(physics, command, velocity, command_angles, movement_angles,
                            ceiling - wave, true);
    return true;
}

} // namespace skeet

// ============================================================================
// Subcomponent: speed_control.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;

// 0x43340ACB..0x433410AC. Both braking and acceleration feed the same
// acceleration-capacity limiter; braking is not an early-return shortcut.
void regulate_movement_speed(const MovementPhysics& physics, UserCommand& command,
                             MovementVector velocity, MovementAngles command_angles,
                             MovementAngleView movement_angles, float target_speed,
                             bool clear_speed_on_acceleration) noexcept {
    if ((command.buttons & jump_button) || command_length(command) < 38.0f)
        return;
    const float speed = horizontal_length(velocity);
    if (speed < 28.0f)
        return;
    const float after_friction = friction_speed(physics, speed);
    const float fraction = after_friction / speed;
    const MovementVector friction_velocity{velocity.x * fraction, velocity.y * fraction,
                                           fraction * 0.0f};
    float difference = target_speed - after_friction;
    if (absolute(difference) < 0.5f) {
        command.forward_move = 0.0f;
        command.side_move = 0.0f;
        return;
    }
    MovementVector direction;
    if (difference < 0.0f) {
        direction = normalize_horizontal({negate(velocity.x), negate(velocity.y), 0.0f});
        movement_angles = horizontal_angles(direction, true);
        command.side_move = 0.0f;
        command.forward_move = 450.0f;
        difference = negate(difference);
    } else {
        if (clear_speed_on_acceleration)
            command.buttons &= ~speed_button;
        MovementVector forward, right;
        command_basis(command_angles, forward, right);
        if (forward.z != 0.0f)
            forward = normalize_horizontal(forward);
        if (right.z != 0.0f)
            right = normalize_horizontal(right);
        direction = normalize_horizontal(
            {command.forward_move * forward.x + command.side_move * right.x,
             command.side_move * right.y + command.forward_move * forward.y, 0.0f});
    }
    const auto capacity =
        ground_acceleration_capacity(physics, command, friction_velocity, direction);
    const float current = dot(friction_velocity, direction);
    float available =
        minimum_number(physics.player_speed_limit, physics.player_max_speed) - current;
    if (0.0f >= available)
        available = 0.0f;
    else
        available = sse_min(available, capacity.amount);
    if (!(available > difference + 0.5f))
        return;
    float desired = difference + current;
    if (capacity.ducking && physics.duck_amount == 1.0f)
        desired /= 0.34f;
    const float scale = desired / (command_length(command) + epsilon);
    command.forward_move *= scale;
    command.side_move *= scale;
    command.up_move *= scale;
}

// 0x43388405..0x43388706. The final comparison uses the acceleration amount
// returned through the helper's output parameter, not a simulated speed norm.
bool write_counter_movement(const MovementPhysics& physics, UserCommand& command,
                            MovementVector velocity, MovementAngleView movement_angles) noexcept {
    command.buttons &= ~0x61au;
    command.forward_move = 0.0f;
    command.side_move = 0.0f;
    const float speed = horizontal_length(velocity);
    if (!(speed >= 0.1f))
        return false;
    const float after_friction = friction_speed(physics, speed);
    if (!(after_friction >= 0.1f))
        return false;
    const float fraction = after_friction / speed;
    const MovementVector friction_velocity{velocity.x * fraction, velocity.y * fraction,
                                           fraction * 0.0f};
    const MovementVector reverse{negate(velocity.x), negate(velocity.y), 0.0f};
    movement_angles = horizontal_angles(reverse, false);
    command.forward_move = 450.0f;
    const MovementVector direction = normalize_horizontal(reverse);
    const float wish_speed = minimum_number(physics.player_speed_limit, physics.player_max_speed);
    const auto acceleration =
        ground_acceleration(physics, command, friction_velocity, direction, wish_speed);
    if (acceleration.amount > after_friction)
        command.forward_move = 0.001f;
    return true;
}
} // namespace skeet

// ============================================================================
// Subcomponent: speed_control_live.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;

/* deduplicated helper */

//433EE7D7..433EE994. Duck/walk decisions have already been retained before
// the entity and weapon methods; those methods can change live player data.
void capture_capacity_weapon(void* player, MovementPhysics& physics,
                             const MovementEnvironmentBindings& bindings) {
    physics.weapon_speed_enabled = !self_integer(bindings.convars.accelerate_weapon_speed);
    if (!physics.weapon_speed_enabled)
        return;
    const auto handle = field<std::int32_t>(player, bindings.fields.weapon_handle_cd812654);
    if (handle == -1)
        return;
    auto* const weapon = engine::invoke<void*>(bindings.entity_list, 4, handle);
    if (!weapon)
        return;
    physics.has_weapon = true;
    physics.weapon_max_speed = engine::invoke<float>(weapon, 0x6E8u / 4u);
    physics.weapon_mode_count = engine::invoke<std::int32_t>(weapon, 0x738u / 4u);
    if (physics.weapon_mode_count > 0)
        physics.weapon_mode = engine::invoke<std::int32_t>(weapon, 0x6ECu / 4u);
}
}

GroundAccelerationCapacity
live_movement_acceleration_capacity(void* player, const UserCommand& command,
                                    MovementVector velocity, MovementVector direction,
                                    const MovementCommandBindings& bindings) {
    const auto& environment = bindings.environment;
    // These three reads are unconditional, including for command IN_DUCK.
    const bool raw_duck = field<std::uint8_t>(player, environment.fields.ducking_6b1a59b5) != 0;
    const float amount = field<float>(player, environment.fields.duck_amount_3309c0cf);
    auto flags = field<std::uint32_t>(player, environment.fields.player_flags_e2fbe695);
    bool duck = (command.buttons & duck_button) != 0;
    if (!duck) {
        bool transitioning = raw_duck || amount > 0.0f;
        if (transitioning || amount >= 1.0f) {
            const float raw_speed = field<float>(player, environment.fields.duck_speed_fb7e3404);
            const float duck_speed = raw_speed >= 1.5f ? raw_speed : 2.0f;
            const float remaining = sse_max(0.0f, amount - duck_speed * (*environment.interval));
            const bool positive_or_unordered = !(remaining <= 0.0f);
            transitioning = positive_or_unordered && transitioning;
            if (!positive_or_unordered || remaining <= 0.75f)
                flags = 0xfffffffdu;
        }
        duck = transitioning || (flags & 2u);
    }
    const bool walk = (command.buttons & speed_button) && !duck;

    MovementPhysics physics{};
    capture_capacity_weapon(player, physics, environment);
    physics.accelerate = movement_command_convar_float(environment.convars.accelerate);
    physics.interval_per_tick = *environment.interval;

    // The numerical helper receives the already decided native booleans.
    // It must not predict duck a second time using the later interval.
    UserCommand numerical_command{};
    numerical_command.buttons = (duck ? duck_button : 0u) | (walk ? speed_button : 0u);
    return ground_acceleration_capacity(physics, numerical_command, velocity, direction);
}

void regulate_live_movement_speed(void* player, UserCommand& command, MovementVector velocity,
                                  ConstMovementAngleView command_angles,
                                  MovementAngleView movement_angles, float target_speed,
                                  bool clear_speed_on_acceleration,
                                  const MovementCommandBindings& bindings) {
    const auto initial_buttons = command.buttons;
    if ((initial_buttons & jump_button) || command_length(command) < 38.0f)
        return;
    const float speed = horizontal_length(velocity);
    if (speed < 28.0f)
        return;
    const auto friction = capture_movement_command_friction(bindings);
    const float remaining =
        maximum_number(speed - (maximum_number(friction.stop_speed, speed) * friction.friction) *
                                   friction.interval,
                       0.0f);
    const float fraction = remaining / speed;
    const MovementVector friction_velocity{velocity.x * fraction, velocity.y * fraction,
                                           fraction * 0.0f};
    float difference = target_speed - remaining;
    if (absolute(difference) < 0.5f) {
        command.side_move = 0.0f;
        command.forward_move = 0.0f;
        return;
    }

    MovementVector direction;
    if (difference < 0.0f) {
        direction = normalize_horizontal({negate(velocity.x), negate(velocity.y), 0.0f});
        const auto angles = horizontal_angles(direction, true);
        movement_angles.yaw = angles.yaw;
        movement_angles.pitch = angles.pitch;
        movement_angles.roll = 0.0f;
        difference = negate(difference);
        command.side_move = 0.0f;
        command.forward_move = 450.0f;
    } else {
        if (clear_speed_on_acceleration)
            command.buttons = initial_buttons & ~speed_button;
        MovementVector forward, right;
        command_basis(static_cast<MovementAngles>(command_angles), forward, right);
        if (forward.z != 0.0f)
            forward = normalize_horizontal(forward);
        if (right.z != 0.0f)
            right = normalize_horizontal(right);
        direction = normalize_horizontal(
            {command.forward_move * forward.x + command.side_move * right.x,
             command.side_move * right.y + command.forward_move * forward.y, 0.0f});
    }

    //43340F68/87: speed getter and its following property read precede the
    // whole capacity call, and survive mutations made by its weapon getters.
    const auto speed_physics = capture_movement_command_speed(player, bindings);
    const auto capacity = live_movement_acceleration_capacity(player, command, friction_velocity,
                                                              direction, bindings);
    const float current = dot(friction_velocity, direction);
    float available =
        minimum_number(speed_physics.player_speed_limit, speed_physics.player_max_speed) - current;
    if (0.0f >= available)
        available = 0.0f;
    else
        available = sse_min(available, capacity.amount);
    if (!(available > difference + 0.5f))
        return;

    float desired = difference + current;
    if (capacity.ducking &&
        field<float>(player, bindings.environment.fields.duck_amount_3309c0cf) == 1.0f)
        desired /= 0.34f;
    const float scale = desired / (command_length(command) + epsilon);
    command.forward_move *= scale;
    command.side_move *= scale;
    command.up_move *= scale;
}

} // namespace skeet

// ============================================================================
// Subcomponent: steering_math.cpp
// ============================================================================

namespace skeet {
using namespace movement_detail;

float round_movement_step(float value) noexcept {
    const bool negative = !(value >= 0.0f);
    float rounded =
        static_cast<float>(std::ceil(static_cast<double>(negative ? negate(value) : value)));
    const float gap = negative ? rounded + value : rounded - value;
    if (gap > 0.5f)
        rounded -= 1.0f;
    return negative ? negate(rounded) : rounded;
}

float planar_movement_yaw(MovementVector direction) noexcept {
    if (direction.x == 0.0f && direction.y == 0.0f)
        return 0.0f;
    const float radians = static_cast<float>(
        std::atan2(static_cast<double>(direction.y), static_cast<double>(direction.x)));
    return radians * radians_to_degrees;
}

void movement_angle_vectors(MovementAngles angles, MovementVector* forward, MovementVector* right,
                            MovementVector* up) noexcept {
    const float pitch = angles.pitch * degrees_to_radians;
    const float yaw = angles.yaw * degrees_to_radians;
    const float roll = angles.roll * degrees_to_radians;
    const float sp = std::sin(pitch), cp = std::cos(pitch);
    const float sy = std::sin(yaw), cy = std::cos(yaw);
    const float sr = std::sin(roll), cr = std::cos(roll);
    if (forward) {
        forward->x = cy * cp;
        forward->y = sy * cp;
        forward->z = negate(sp);
    }
    if (right) {
        const float minus_cr = cr * -1.0f;
        const float sr_sp = sr * sp;
        right->y = minus_cr * cy - sr_sp * sy;
        right->x = minus_cr * negate(sy) - sr_sp * cy;
        right->z = (sr * -1.0f) * cp;
    }
    if (up) {
        const float cr_sp = cr * sp;
        up->z = cr * cp;
        up->x = cr_sp * cy - negate(sy) * sr;
        up->y = cr_sp * sy - sr * cy;
    }
}

void project_movement_heading(float forward_move, float side_move,
                              MovementAngleView movement_angles) noexcept {
    MovementVector forward, right;
    movement_angle_vectors(movement_angles, &forward, &right, nullptr);
    if (forward.z != 0.0f)
        forward = normalize_horizontal(forward);
    if (right.z != 0.0f)
        right = normalize_horizontal(right);
    const MovementVector desired{side_move * right.x + forward_move * forward.x,
                                 side_move * right.y + forward_move * forward.y, 0.0f};
    movement_angles.yaw = planar_movement_yaw(normalize_horizontal(desired));
}

void project_raw_movement_heading(float forward_move, float side_move,
                                  MovementAngleView movement_angles) noexcept {
    MovementVector forward, right;
    movement_angle_vectors(movement_angles, &forward, &right, nullptr);
    movement_angles.yaw =
        planar_movement_yaw({side_move * right.x + forward.x * forward_move,
                             side_move * right.y + forward.y * forward_move, 0.0f});
}

float normalize_movement_angle(float value) noexcept {
    if (std::isnan(value))
        return 0.0f;
    if (absolute(value) > 180.0f) {
        const float turns = round_movement_step(absolute(value / 360.0f)) * 360.0f;
        if (value < 0.0f)
            value += turns;
        else
            value -= turns;
    }
    return value;
}

void normalize_movement_angles(MovementAngleView angles) noexcept {
    angles.pitch = normalize_movement_angle(angles.pitch);
    angles.yaw = normalize_movement_angle(angles.yaw);
    angles.roll = normalize_movement_angle(angles.roll);
}

} // namespace skeet

// ============================================================================
// Subcomponent: weapon_speed_phase.cpp
// ============================================================================

namespace skeet {
namespace {
using namespace movement_detail;

std::int32_t truncate_tick(float value) noexcept {
    // CVTTSS2SI produces the integer-indefinite value for NaN/overflow.
    if (!(value >= -2147483648.0f && value < 2147483648.0f))
        return std::numeric_limits<std::int32_t>::min();
    return static_cast<std::int32_t>(value);
}

/* deduplicated helper */

void add_duck(UserCommand& command, std::uint32_t scheduler_flags) noexcept {
    if ((scheduler_flags & 0x10040u) == 0x10000u)
        command.buttons |= duck_button;
}

struct SnapshotWeaponSpeed {
    const WeaponSpeedPhaseSettings& settings;
    const WeaponSpeedPhaseFrame& frame;
    std::uint32_t& flags;
    MovementCollisionEngine& collision;
    MovementAngles command_angles;

    bool blocked() const noexcept {
        return frame.weapon_blocked;
    }
    std::int32_t weapon_state() const noexcept {
        return frame.weapon_state;
    }
    std::uint32_t policy() const noexcept {
        return settings.policy_flags;
    }
    float guard() const noexcept {
        return frame.attack_guard_8815d240;
    }
    float current_time() const noexcept {
        return frame.current_time;
    }
    std::uint32_t player_flags() const noexcept {
        return frame.player.physics.player_flags;
    }
    std::uint16_t definition() const noexcept {
        return frame.weapon_definition;
    }
    float player_attack() const noexcept {
        return frame.player_next_attack;
    }
    float weapon_attack() const noexcept {
        return frame.weapon_next_attack;
    }
    float interval() const noexcept {
        return frame.player.physics.interval_per_tick;
    }
    MovementPhysics speed() const noexcept {
        return frame.player.physics;
    }
    MovementVector velocity() const noexcept {
        return frame.player.velocity;
    }
    MovementCommandFriction friction() const noexcept {
        return {frame.player.physics.stop_speed, frame.player.physics.friction, interval()};
    }
    void initialize(MovementSimulation& state, MovementVector velocity) const noexcept {
        initialize_movement_simulation(state, frame.player.entity, frame.position, velocity,
                                       frame.player.physics.player_flags, frame.simulation_mode,
                                       frame.snapshot, frame.rules);
    }
    void simulate(MovementSimulation& state, const client::Command& command) const {
        simulate_movement_tick(state, collision, frame.simulation_environment, frame.player.entity,
                               command.movement,
                               {command.angles[0], command.angles[1], command.angles[2]});
    }
    GroundAcceleration accelerate(const UserCommand& command, MovementVector velocity,
                                  MovementVector direction, float maximum) const noexcept {
        return ground_acceleration(frame.player.physics, command, velocity, direction, maximum);
    }
    void apply_regulator(client::Command& command, MovementAngleView angles,
                         float target) const noexcept {
        regulate_movement_speed(frame.player.physics, command.movement, velocity(), command_angles,
                                angles, target, true);
    }
    void counter(UserCommand& command, MovementAngleView angles) const noexcept {
        write_counter_movement(frame.player.physics, command, velocity(), angles);
    }
};

struct LiveWeaponSpeed {
    void* player;
    void* weapon;
    const MovementCommandBindings& bindings;
    std::uint32_t& flags;

    bool blocked() const noexcept {
        return movement_command_weapon_blocked(weapon, bindings);
    }
    std::int32_t weapon_state() const noexcept {
        return bindings.weapon_state;
    }
    std::uint32_t policy() const noexcept {
        return bindings.weapon_policy;
    }
    float guard() const noexcept {
        return movement_command_attack_guard(player, bindings);
    }
    float current_time() const noexcept {
        return bindings.current_time;
    }
    std::uint32_t player_flags() const noexcept {
        return movement_command_player_flags(player, bindings);
    }
    std::uint16_t definition() const noexcept {
        return movement_command_weapon_definition(weapon, bindings);
    }
    float player_attack() const noexcept {
        return movement_command_player_attack(player, bindings);
    }
    float weapon_attack() const noexcept {
        return movement_command_weapon_attack(weapon, bindings);
    }
    float interval() const noexcept {
        return *bindings.environment.interval;
    }
    MovementPhysics speed() const {
        return capture_movement_command_speed(player, bindings);
    }
    MovementVector velocity() const noexcept {
        return movement_command_velocity(player, bindings);
    }
    MovementCommandFriction friction() const noexcept {
        return capture_movement_command_friction(bindings);
    }
    void initialize(MovementSimulation& state, MovementVector velocity) const {
        // 4337241B/440/47C: mode, flags, then direct decoded origin; the
        // constructor owns its following cvar reads and 130/134/260 getters.
        const auto mode = field<std::uint32_t>(player, bindings.fields.simulation_mode_3d10b94c);
        const auto flags = player_flags();
        const auto position = field<MovementVector>(player, bindings.fields.origin_a9ee6062);
        initialize_live_movement_simulation(state, player, position, velocity, flags, mode,
                                            bindings.simulation);
    }
    void simulate(MovementSimulation& state, const client::Command& command) const {
        simulate_live_movement_tick(state, bindings.collision, bindings.environment, player,
                                    command);
    }
    GroundAcceleration accelerate(const UserCommand& command, MovementVector velocity,
                                  MovementVector direction, float maximum) const {
        return movement_command_ground_acceleration(player, command, velocity, direction, maximum,
                                                    bindings);
    }
    void apply_regulator(client::Command& command, MovementAngleView angles, float target) const {
        const auto current_velocity = velocity();
        regulate_live_movement_speed(player, command.movement, current_velocity,
                                     ConstMovementAngleView(command.angles), angles, target, true,
                                     bindings);
    }
    void counter(UserCommand& command, MovementAngleView angles) const {
        const auto current_velocity = velocity();
        write_live_movement_counter(player, command, current_velocity, angles, bindings);
    }
};

template <class Source>
void regulate(Source& source, client::Command& command, MovementAngleView movement_angles,
              bool crouch) {
    const auto physics = source.speed();
    float ceiling = minimum_number(physics.player_speed_limit, physics.player_max_speed) - 1.0f;
    if (crouch)
        ceiling *= 0.34f;
    const auto remainder = command.number % 1000;
    const float phase =
        (static_cast<float>(remainder) * source.interval()) * (crouch ? 7.0f : 10.0f);
    float wave = phase - static_cast<float>(std::floor(static_cast<double>(phase)));
    wave = absolute(wave - 0.5f);
    wave += wave;
    wave *= crouch ? 4.0f : 10.0f;
    source.apply_regulator(command, movement_angles, ceiling - wave);
}

// Returns true for the direct regulator branch at 43372AE5.
// Both input sources execute this single prediction/braking algorithm.
template <class Source>
bool should_regulate_before_attack(Source& source, std::uint32_t policy,
                                   const client::Command& command) {
    // 43372207/212/21C: player attack, interval, weapon attack.
    const float player_attack = source.player_attack();
    const float interval = source.interval();
    const float attack = maximum_number(source.weapon_attack(), player_attack);
    const float attack_tick = static_cast<float>(std::ceil(static_cast<double>(attack / interval)));
    const auto remaining = std::bit_cast<std::int32_t>(
        std::uint32_t(truncate_tick(attack_tick + 0.5f)) -
        std::uint32_t(truncate_tick(source.current_time() / interval + 0.5f)));
    float threshold = 0.0f;
    if (policy & 4u) {
        const auto physics = source.speed();
        threshold =
            (minimum_number(physics.player_speed_limit, physics.player_max_speed) - 1.0f) * 0.34f -
            2.0f;
    }
    if (remaining < 1)
        return false;

    const auto initial_velocity = source.velocity();
    auto copy = command;
    const float scale = 260.0f / (command_length(copy.movement) + epsilon);
    copy.movement.forward_move *= scale;
    copy.movement.side_move *= scale;
    copy.movement.up_move *= scale;
    MovementSimulation simulation{};
    source.initialize(simulation, initial_velocity);
    source.simulate(simulation, copy);
    auto velocity = simulation.velocity;
    // The first native norm adds z, y, x; later loop norms add x, y, z.
    float speed =
        std::sqrt((velocity.z * velocity.z + velocity.y * velocity.y) + velocity.x * velocity.x);
    if (speed < 1.0f)
        return true;
    const auto physics = source.speed();
    const float maximum = minimum_number(physics.player_speed_limit, physics.player_max_speed);
    const float friction_threshold = threshold + 0.1f;

    for (std::int32_t tick = 0; tick < remaining; ++tick) {
        const auto friction = source.friction();
        const float control = maximum_number(friction.stop_speed, speed);
        const float after_friction =
            maximum_number(speed - friction.interval * (control * friction.friction), 0.0f);
        if (after_friction < friction_threshold)
            return true;
        const float fraction = after_friction / speed;
        MovementVector friction_velocity{fraction * velocity.x, fraction * velocity.y,
                                         fraction * velocity.z};
        const MovementVector reverse{negate(velocity.x), negate(velocity.y), negate(velocity.z)};
        const float reciprocal = 1.0f / (length(reverse) + epsilon);
        MovementVector direction{reciprocal * reverse.x, reciprocal * reverse.y,
                                 reciprocal * reverse.z};
        if (direction.z != 0.0f) {
            const float horizontal =
                1.0f / (std::sqrt((direction.x * direction.x + direction.y * direction.y) + 0.0f) +
                        epsilon);
            direction = {direction.x * horizontal, direction.y * horizontal, horizontal * 0.0f};
        }

        auto acceleration = source.accelerate(copy.movement, friction_velocity, direction, maximum);
        float retry_speed = maximum;
        if (acceleration.amount > after_friction && acceleration.acceleration != 0.0f)
            retry_speed = 0.01f;
        if (retry_speed != maximum) {
            // 43372898/9E/B7 use ESP after one/one/three PUSHes: their
            // adjusted offsets still select velocity.x, copy, and player.
            acceleration =
                source.accelerate(copy.movement, friction_velocity, direction, retry_speed);
        }
        if (acceleration.amount > 0.0f) {
            friction_velocity.x = acceleration.amount * direction.x + friction_velocity.x;
            friction_velocity.y = acceleration.amount * direction.y + friction_velocity.y;
            friction_velocity.z = acceleration.amount * direction.z + friction_velocity.z;
        }
        velocity = friction_velocity;
        speed = length(velocity);
        if (speed < threshold + 1.0f)
            return true;
    }
    return false;
}

template <class Source>
WeaponSpeedPhaseContinuation process_phase(Source& source, client::Command& command,
                                           MovementAngleView movement_angles) {
    using Result = WeaponSpeedPhaseContinuation;
    auto& scheduler_flags = source.flags;
    if (source.blocked() || !(scheduler_flags & 8u))
        return Result::player_movement;
    const auto weapon_state = source.weapon_state();
    auto policy = source.policy();
    if (weapon_state != -1) {
        policy = (policy & 0xfffffff1u) | std::uint32_t(weapon_state);
        if ((policy & 2u) && (scheduler_flags & 0x1000u) && !(scheduler_flags & 0x40u)) {
            scheduler_flags |= 0x10000u;
            command.movement.buttons |= duck_button;
        }
    }
    if (!(policy & 0x10u)) {
        const float guard = source.guard();
        if (guard != 0.0f && absolute(source.current_time() - guard) < 1.0f)
            return Result::player_movement;
    }
    if ((scheduler_flags & 0x10u) && (source.player_flags() & on_ground)) {
        add_duck(command.movement, scheduler_flags);
        policy |= 4u;
    }

    if (policy & 8u) {
        if (weapon_state != -1 || (source.definition() != 0x40u &&
                                   should_regulate_before_attack(source, policy, command))) {
            regulate(source, command, movement_angles, false);
            return Result::complete;
        }
    }
    if (policy & 4u) {
        if (source.blocked())
            return Result::complete;
        add_duck(command.movement, scheduler_flags);
        regulate(source, command, movement_angles, true);
    } else if (source.player_flags() & on_ground) {
        add_duck(command.movement, scheduler_flags);
        source.counter(command.movement, movement_angles);
    }
    return Result::complete;
}

} // namespace

WeaponSpeedPhaseContinuation
process_weapon_speed_phase(const WeaponSpeedPhaseSettings& settings,
                           const WeaponSpeedPhaseFrame& frame, client::Command& command,
                           MovementAngles command_angles, MovementAngleView movement_angles,
                           std::uint32_t& scheduler_flags, MovementCollisionEngine& collision) {
    SnapshotWeaponSpeed source{settings, frame, scheduler_flags, collision, command_angles};
    return process_phase(source, command, movement_angles);
}

WeaponSpeedPhaseContinuation
process_live_weapon_speed_phase(void* player, void* weapon, client::Command& command,
                                MovementAngleView movement_angles,
                                const MovementCommandBindings& bindings) {
    LiveWeaponSpeed source{player, weapon, bindings, bindings.scheduler_flags};
    return process_phase(source, command, movement_angles);
}

} // namespace skeet

// ============================================================================
// Subcomponent: z_hop.cpp
// ============================================================================

namespace skeet {

bool apply_z_hop_heading(const ZHopSettings& settings, InputBitmap input, float engine_time,
                         MovementAngleView movement_angles) {
    if (!settings.enabled || !hotkey_active(settings.hotkey, input))
        return false;
    const float amplitude = static_cast<float>(settings.amplitude) * 0.1f;
    float phase = static_cast<float>(settings.frequency) * 0.1f;
    phase *= engine_time;
    movement_angles.yaw += std::sin(phase) * amplitude;
    return true;
}

} // namespace skeet
