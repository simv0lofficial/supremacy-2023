#include "util/address.h"
// Client Core Implementation
#include "skeet/client/buffered_shot_command.hpp"
#include "skeet/rage/buffered_shot_repair.hpp"
#include "skeet/rage/command_phase6.hpp"
#include "skeet/rage/feedback_owner.hpp"
#include "skeet/lagcomp/prediction_scope.hpp"
#include "skeet/movement/steering_math.hpp"
#include <bit>
#include <cmath>
#include <cstring>
#include <limits>
#include <xmmintrin.h>
#include "skeet/client/buffered_shot_lifecycle.hpp"
#include "skeet/client/native_startup.hpp"
#include "skeet/engine/native_owner_registry_storage.hpp"
#include "skeet/engine/unload.hpp"
#include <array>
#include <new>
#include <stdexcept>
#include "skeet/client/buffered_shot_runtime_owner.hpp"
#include "skeet/client/runtime_storage.hpp"
#include "skeet/client/runtime_composition.hpp"
#include "skeet/client/history_runtime_owner.hpp"
#include "skeet/client/player_runtime_owner.hpp"
#include "skeet/client/command_capture_runtime_owner.hpp"
#include "skeet/client/command_checksum.hpp"
#include "skeet/animation/runtime_owner.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/engine/netvar_initialization.hpp"
#include "skeet/esp/runtime_owner.hpp"
#include "skeet/rage/runtime_owner.hpp"
#include "skeet/rage/feedback_runtime_owner.hpp"
#include "skeet/rage/spread_storage.hpp"
#include "skeet/client/combat_runtime_factories.hpp"
#include "skeet/engine/owner_allocation.hpp"
#include "skeet/network/exploit_phase7.hpp"
#include <algorithm>
#include "skeet/client/commands.hpp"
#include "skeet/engine/hooks.hpp"
#include "../script/detail.hpp"
#include "skeet/client/command_angle_retention.hpp"
#include "skeet/client/command_capture.hpp"
#include "skeet/client/startup_reset.hpp"
#include "skeet/animation/native_trigonometry.hpp"
#include "skeet/engine/build_eye_adjustment_bindings.hpp"
#include "skeet/misc/config.hpp"
#include "skeet/rage/settings.hpp"
#include "skeet/anti_aim/config.hpp"
#include "skeet/client/command_empty_tick.hpp"
#include "skeet/client/command_owner_dispatch.hpp"
#include "skeet/misc/grenade_command.hpp"
#include "skeet/effects/bullet_impact_command_owner.hpp"
#include "skeet/client/prediction_history_owner.hpp"
#include "skeet/client/command_eye_adjustment.hpp"
#include "skeet/rage/firing.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/network/packet_control.hpp"
#include <type_traits>
#include "skeet/client/command_orchestrator.hpp"
#include "skeet/client/target_refresh_runtime_owner.hpp"
#include "skeet/effects/free_look_owner.hpp"
#include "skeet/anti_aim/command_owner.hpp"
#include "skeet/network/command_owner.hpp"
#include "skeet/network/exploit_command_owner.hpp"
#include "skeet/misc/command_owner.hpp"
#include "skeet/misc/viewmodel_lag.hpp"
#include "skeet/misc/no_fall_damage.hpp"
#include "skeet/misc/low_fps_runtime.hpp"
#include "skeet/rage/command_source.hpp"
#include "skeet/rage/feedback_command_owner.hpp"
#include "skeet/client/entity_command_owner.hpp"
#include "skeet/script/command_owner.hpp"
#include "skeet/skins/command.hpp"
#include "skeet/client/target_mask_owner.hpp"
#include "skeet/movement/command_wrapper.hpp"
#include "skeet/client/command_packet_angle_guard.hpp"
#include "skeet/client/command_pipeline.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/client/command_packet_transition.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/client/command_prediction_cache.hpp"
#include "skeet/client/command_recapture.hpp"
#include "skeet/rage/targeting_commit.hpp"
#include "skeet/client/command_runtime_owner.hpp"
#include "skeet/client/local_animation_runtime_owner.hpp"
#include "skeet/client/shared_esp_runtime_owner.hpp"
#include "skeet/engine/hook_runtime_owner.hpp"
#include "skeet/client/command_transform.hpp"
#include "skeet/client/command_weapon_postprocess.hpp"
#include <emmintrin.h>
#include "skeet/client/config_initializers.hpp"
#include "skeet/esp/config.hpp"
#include "skeet/esp/effects_config.hpp"
#include "skeet/esp/other_config.hpp"
#include "skeet/client/crt_initialization.hpp"
#include "skeet/client/crt_startup_bindings.hpp"
#include "skeet/client/static_crt_initialization.hpp"
#include "skeet/client/source_terminal_cleanup.hpp"
#include "skeet/animation/fix_animation_backup_cleanup.hpp"
#include "skeet/script/source_runtime_owner.hpp"
#include "skeet/script/startup_initializers.hpp"
#include "skeet/engine/first_startup_services.hpp"
#include "skeet/engine/startup_vector_cleanup.hpp"
#include "skeet/engine/source_initialization_imports.hpp"
#include "skeet/engine/build_window_bindings.hpp"
#include "skeet/engine/build_game_state_bindings.hpp"
#include "skeet/engine/build_client_state_aliases.hpp"
#include "skeet/engine/build_script_bindings.hpp"
#include "skeet/engine/build_misc_engine_bindings.hpp"
#include "skeet/engine/build_command_bindings.hpp"
#include "skeet/engine/build_legit_bindings.hpp"
#include "skeet/engine/build_entity_animation_bindings.hpp"
#include "skeet/engine/build_visual_interface_bindings.hpp"
#include "skeet/engine/build_esp_bindings.hpp"
#include "skeet/engine/build_material_bindings.hpp"
#include "skeet/engine/build_world_bindings.hpp"
#include "skeet/engine/build_packet_bindings.hpp"
#include "skeet/engine/build_shared_esp_bindings.hpp"
#include "skeet/engine/build_scene_bindings.hpp"
#include "skeet/engine/build_loader_inspection_bindings.hpp"
#include "skeet/engine/build_runtime_type_bindings.hpp"
#include "skeet/engine/tier0_startup_exports.hpp"
#include "skeet/engine/animation_startup_patterns.hpp"
#include "skeet/client/restricted_player_command.hpp"
#include "skeet/client/entity_command_lifecycle.hpp"
#include "skeet/animation/entity_update.hpp"
#include "skeet/esp/map_entity_parser.hpp"
#include "skeet/misc/native_text.hpp"
#include "entity_command_map.hpp"
#include "entity_command_memory.hpp"
#include "entity_command_scratch.hpp"
#include "skeet/engine/netvars.hpp"
#include "skeet/engine/hash_growth.hpp"
#include <cstdlib>
#include "skeet/client/verified_command.hpp"
#include "skeet/events/game_events.hpp"
#include "skeet/prediction/observer_registry.hpp"
#include "skeet/prediction/player_simulation.hpp"
#include "skeet/client/entity_command_runtime_owner.hpp"
#include "skeet/client/local_player_animation_hooks.hpp"
#include "skeet/engine/build_prediction_scratch_bindings.hpp"
#include "skeet/legit/runtime.hpp"
#include <functional>
#include <windows.h>
#include "skeet/client/player_state.hpp"
#include <immintrin.h>
#include "skeet/rage/shot_feedback.hpp"
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/lagcomp/history_receiver.hpp"
#include "skeet/misc/clan_tag.hpp"
#include "skeet/prediction/entity_update.hpp"
#include "skeet/esp/sound_overlay.hpp"
#include "skeet/skins/native_entity_cosmetics.hpp"
#include "skeet/skins/native_inventory.hpp"
#include "skeet/skins/native_sequence_update.hpp"
#include "skeet/skins/sequence_owner.hpp"
#include "skeet/script/entity_observer.hpp"
#include "skeet/rage/entity_update.hpp"
#include "skeet/rage/feedback_entity_owner.hpp"
#include "skeet/client/entity_update_dispatch.hpp"
#include "skeet/menu/skins_menu.hpp"
#include "skeet/client/exit_registration.hpp"
#include "skeet/client/shutdown.hpp"
#include "skeet/client/static_cleanup.hpp"
#include <cstdint>
#include "skeet/client/exploit_runtime_owner.hpp"
#include "skeet/client/generated_command_owners.hpp"
#include "skeet/client/movement_hud_runtime_owner.hpp"
#include "skeet/client/visual_runtime_owner.hpp"
#include "skeet/engine/scene_runtime_owner.hpp"
#include "skeet/network/exploit_phase1.hpp"
#include "skeet/movement/rage_stop.hpp"
#include "skeet/client/feature_runtime_owner.hpp"
#include "skeet/skins/runtime_owner.hpp"
#include "skeet/legit/runtime_owner.hpp"
#include "skeet/script/api_runtime_owner.hpp"
#include "skeet/client/final_menu_startup.hpp"
#include "skeet/menu/startup_window.hpp"
#include "skeet/menu/root_menu.hpp"
#include "skeet/misc/settings_runtime_owner.hpp"
#include "skeet/render/startup_renderer.hpp"
#include "skeet/render/backend.hpp"
#include "skeet/hotkey_widget.hpp"
#include "skeet/client/final_patch_startup.hpp"
#include "skeet/client/player_receive_owner.hpp"
#include "skeet/engine/client_class_factory_hooks.hpp"
#include "skeet/engine/hook_descriptor_storage.hpp"
#include "skeet/engine/build_final_startup_bindings.hpp"
#include "skeet/network/incoming_update.hpp"
#include "skeet/network/startup_user_messages.hpp"
#include "skeet/network/receive_float_proxy.hpp"
#include "skeet/effects/visual_receive_owner.hpp"
#include "skeet/skins/agent_receive.hpp"
#include <atomic>
#include <vector>
#include "skeet/client/game_window_proxy_factory.hpp"
#include "skeet/movement/command_runtime.hpp"
#include "skeet/client/global_config_initializers.hpp"
#include "skeet/esp/model_color_config.hpp"
#include "skeet/menu/settings_menu_storage.hpp"
#include "skeet/client/history_runtime_factories.hpp"
#include "skeet/engine/native_data_image.hpp"
#include "skeet/engine/build_history_bindings.hpp"
#include "skeet/engine/trace.hpp"
#include "skeet/lagcomp/native_history_boundaries.hpp"
#include "skeet/client/hitboxes.hpp"
#include "skeet/lagcomp/entity_state.hpp"
#include "skeet/client/input_commands.hpp"
#include "skeet/client/player_command_queries.hpp"
#include "skeet/client/player_physics_hooks.hpp"
#include "skeet/client/player_command_hooks.hpp"
#include "skeet/effects/runtime_owner.hpp"
#include "skeet/engine/build_local_animation_bindings.hpp"
#include "skeet/movement/global_config.hpp"
#include "skeet/movement/collision.hpp"
#include "skeet/client/misc_runtime_factories.hpp"
#include "skeet/engine/scene_queries.hpp"
#include "skeet/client/misc_runtime_owner.hpp"
#include "skeet/misc/runtime.hpp"
#include "skeet/misc/knife_aim.hpp"
#include "skeet/misc/knife_melee.hpp"
#include "skeet/movement/extrapolation.hpp"
#include "skeet/client/movement_hud_factories.hpp"
#include "skeet/movement/command_bindings.hpp"
#include "skeet/client/runtime_owner_bindings.hpp"
#include "skeet/client/runtime_owner_initialization.hpp"
#include "skeet/events/event_name_storage.hpp"
#include "skeet/events/registry.hpp"
#include "skeet/engine/startup_container_cleanup.hpp"
#include "skeet/client/skin_runtime_factories.hpp"
#include "skeet/client/world_runtime_factories.hpp"
#include "skeet/rage/prediction_mask_owner.hpp"
#include "skeet/rage/post_firing.hpp"
#include "skeet/engine/map_game_system_owner.hpp"
#include "skeet/anti_aim/startup_initializers.hpp"
#include "skeet/client/hook_descriptor_cleanup.hpp"
#include "skeet/client/prediction_history_lifecycle.hpp"
#include "skeet/client/target_mask_lifecycle.hpp"
#include "skeet/client/shared_esp_owner.hpp"
#include "skeet/engine/preserved_initialization.hpp"
#include "skeet/script/ui_type_storage.hpp"
#include "skeet/input/runtime.hpp"
#include "skeet/rage/group_storage.hpp"
#include "skeet/misc/viewmodel_lag_owner.hpp"
#include "skeet/hud/simple_owners.hpp"
#include "skeet/esp/flag_table_initializer.hpp"
#include <sstream>
#include "skeet/client/packet_runtime_owner.hpp"
#include <intrin.h>
#include "skeet/client/player_data_update.hpp"
#include "skeet/engine/offset_storage.hpp"
#include "player_physics_hooks_detail.hpp"
#include "skeet/client/player_receive.hpp"
#include "skeet/lagcomp/receive_timing.hpp"
#include "skeet/animation/fix_runtime_owner.hpp"
#include "skeet/animation/fix_animations.hpp"
#include "skeet/animation/state_copy.hpp"
#include "skeet/animation/local_prediction_pose.hpp"
#include "skeet/client/prediction_history_runtime_owner.hpp"
#include "skeet/client/rejected_player_update.hpp"
#include <utility>
#include "restricted_player_command_internal.hpp"
#include "skeet/render/renderer_assets.hpp"
#include "skeet/script/panorama_api.hpp"
#include "skeet/client/runtime_feature_bindings.hpp"
#include "skeet/anti_aim/runtime_owner.hpp"
#include "skeet/script/client_api.hpp"
#include "skeet/engine/source_data_initialization.hpp"
#include "skeet/misc/network_console_handlers.hpp"
#include "skeet/rage/target_relationships.hpp"
#include "skeet/misc/unload.hpp"
#include "skeet/render/backend_container.hpp"
#include "skeet/client/shutdown_runtime_owner.hpp"
#include "skeet/config_storage/owner.hpp"
#include "skeet/network/data_update_observers.hpp"
#include <initializer_list>
#include "skeet/client/source_application.hpp"
#include "skeet/client/source_feature_runtime.hpp"
#include "skeet/client/source_startup_cleanup.hpp"
#include "skeet/client/startup_bindings.hpp"
#include "skeet/script/source_executable_image.hpp"
#include "skeet/script/required_module_storage.hpp"
#include "skeet/menu/startup_config_path.hpp"
#include "skeet/engine/build_hook_initialization.hpp"
#include "skeet/engine/build_skin_bindings.hpp"
#include <optional>
#include <string>
#include "skeet/network/user_message_runtime.hpp"
#include "skeet/client/source_feature_runtime_imports.hpp"
#include "skeet/client/target_mask_runtime_owner.hpp"
#include "skeet/client/world_runtime_owner.hpp"
#include "skeet/engine/player_hook_family.hpp"
#include "skeet/effects/camera_runtime_owner.hpp"
#include "skeet/effects/player_opacity_owner.hpp"
#include "skeet/effects/free_look_runtime_owner.hpp"
#include "skeet/effects/player_auxiliary_hooks.hpp"
#include "skeet/effects/harmless_molotov.hpp"
#include "skeet/rage/prediction_mask_runtime_owner.hpp"
#include "skeet/misc/viewmodel_runtime_owner.hpp"
#include "skeet/hud/simple_runtime_owner.hpp"
#include "skeet/players/menu_bindings.hpp"
#include "skeet/network/packet_dispatch.hpp"
#include "skeet/network/client_state_runtime_owner.hpp"
#include "skeet/lagcomp/history_runtime.hpp"
#include "skeet/script/initialize.hpp"
#include "skeet/script/modules.hpp"
#include "skeet/script/registry.hpp"
#include "skeet/script/shutdown.hpp"
#include "skeet/script/ui.hpp"
#include "skeet/client/startup.hpp"
#include "skeet/config_storage/preserved.hpp"
#include "skeet/input/window_hook.hpp"
#include "skeet/render/device_listener.hpp"
#include "skeet/render/startup_icons.hpp"
#include "skeet/script/owner.hpp"
#include "skeet/network/exploit_observers.hpp"
#include "skeet/render/icon_cache.hpp"
#include "skeet/hud/composite_runtime.hpp"
#include "skeet/lagcomp/records.hpp"
#include "skeet/anti_aim/targets.hpp"
#include "skeet/esp/runtime_bindings.hpp"
#include "skeet/engine/pe_exports.hpp"


// ============================================================================
// Subcomponent: buffered_shot_command.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* [stripped helper] */
/* [stripped helper] */
/* [stripped helper] */
/* [stripped helper] */
/* [stripped truncate] */
/* [stripped ticks] */
float convar_float(void* variable) noexcept {
    return std::bit_cast<float>(lagcomp::read_prediction_convar(variable));
}
std::uint32_t convar_integer(void* variable) noexcept {
    for (;;) {
        auto* parent = read<void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    return read<std::uint32_t>(variable, 0x30) ^
           std::uint32_t(reinterpret_cast<std::uintptr_t>(variable));
}
float interpolation_time(const BufferedShotCommandBindings& b) noexcept {
    const auto& c = b.convars;
    const auto rate = convar_float(c.interpolation_rate);
    const auto minimum = convar_float(c.interpolation_minimum);
    const auto ratio = convar_float(c.interpolation_ratio);
    return maximum_number(b.interpolation_floor, maximum_number(minimum, ratio / rate));
}
float squared_distance(const RageVector& first, const RageVector& second) noexcept {
    const float y = first[1] - second[1], x = first[0] - second[0], z = first[2] - second[2];
    return (y * y + x * x) + z * z;
}
bool needs_repair(const rage::ShotDiagnosticEntry& entry, const CommandFrame& current, void* player,
                  const BufferedShotCommandBindings& b) {
    if (entry.local_tick != ticks(read<float>(player, b.offsets.simulation_time), b.interval)) {
        const auto number = engine::invoke<std::int32_t>(b.engine_client, 205);
        const auto* history =
            prediction::CommandHistoryView::from_owner(b.history_owner).find(number);
        if (history && (history->flags & 2u))
            return true;
    }
    if (std::bit_cast<float>(entry.local_field_7f5ed2d0) >
        read<float>(player, b.offsets.local_field_7f5ed2d0))
        return true;

    // The actual engine slot returns the +8 networkable subobject.
    const auto* networkable =
        engine::invoke<const std::byte*>(b.entity_list, 1, entry.target_handle);
    if (!networkable)
        return true;
    const auto* target = networkable - 8;
    if (read<std::uint8_t>(target, b.offsets.target_rejection))
        return true;
    const auto index =
        entry.target_handle == UINT32_MAX ? 0x1FFFu : std::uint16_t(entry.target_handle);
    const auto& cached = *b.players.get(index);
    if (integer(entry.target_field_168) < read<std::int32_t>(&cached.native, 0x168))
        return false;
    if (cached.history.count >= 2 && squared_distance(cached.history.at(0).header.origin,
                                                      cached.history.at(1).header.origin) > 4096.0f)
        return true;
    if ((read<std::uint32_t>(target, b.offsets.target_flags) ^ entry.target_flags) & 1u)
        return true;
    if (!(cached.native.cached_target_height == entry.target_height))
        return true;
    if (!(read<float>(target, b.offsets.target_duck) == entry.target_duck))
        return true;
    if (!(cached.native.reference_yaw == entry.target_reference_yaw))
        return true;

    // 4337F00E's intentional seconds truncation, repeated inline here.
    const auto base = integer(current.field_18c + current.field_d4);
    const auto seconds = truncate(float(base) * b.interval - float(b.maximum_unlag_ms) * 0.001f);
    const auto corrected = ticks(float(seconds), b.interval);
    return add(entry.target_tick, ticks(interpolation_time(b), b.interval)) < corrected;
}

void refresh_prediction_frame(CommandFrame& frame, std::int32_t number,
                              const BufferedShotCommandBindings& b) noexcept {
    const auto* history = prediction::CommandHistoryView::from_owner(b.history_owner).find(number);
    if (!history)
        return;
    const bool preserve_height = frame.view_height_84 == frame.height_164;
    const auto origin = read<RageVector>(history, 0x2C);
    const RageVector delta{origin[0] - frame.origin_10c[0], origin[1] - frame.origin_10c[1],
                           origin[2] - frame.origin_10c[2]};
    const float square = (delta[1] * delta[1] + delta[0] * delta[0]) + delta[2] * delta[2];
    if (square > 0.010000000707805157f) {
        for (std::size_t axis = 0; axis < 3; ++axis)
            frame.origin_dc[axis] += delta[axis];
        frame.origin_10c = origin;
    }
    const auto height = read<float>(history, 0x28);
    if (preserve_height) {
        const float difference = height - frame.height_164;
        if (difference * difference > 0.010000000707805157f) {
            frame.height_164 = frame.view_height_84 = height;
            frame.origin_dc[2] = origin[2] + height;
        }
    }
    const auto scale = convar_float(b.convars.recoil_scale);
    const auto& raw = history->recoil_angles;
    const RageVector recoil{raw[0] * scale, raw[1] * scale, raw[2] * scale};
    if (squared_distance(recoil, frame.recoil_angles_9c) > 0.010000000707805157f)
        frame.recoil_angles_9c = recoil;
    const auto latency = network::average_latency_ticks(b.latency);
    write(&frame, 0x14C, latency);
    const auto& timing = b.timing;
    const auto command_tick = network::adjusted_command_tick(
        read<std::int32_t>(timing.command_clock, 4), timing.acknowledged != nullptr,
        timing.acknowledged ? *timing.acknowledged : 0, timing.sent != nullptr,
        timing.sent ? *timing.sent : 0);
    frame.field_d4 = std::uint32_t(latency) + std::uint32_t(command_tick);
    frame.field_18c = 0;
    write(&frame, 0x144, frame.field_d4 + convar_integer(b.convars.command_tick_offset));
}

std::array<float, 3> direction_angles(RageVector direction, const rage::TargetAngleMath& math) {
    const float square =
        (direction[0] * direction[0] + direction[1] * direction[1]) + direction[2] * direction[2];
    const float inverse = 1.0f / (std::sqrt(square) + 0x1p-23f);
    for (auto& component : direction)
        component *= inverse;
    if (direction[0] == 0.0f && direction[1] == 0.0f)
        return {direction[2] > 0.0f ? 270.0f : 90.0f, 0.0f, 0.0f};
    constexpr float degrees = 57.2957763671875f;
    const auto yaw = float(math.atan2(double(direction[1]), double(direction[0]))) * degrees;
    const auto horizontal = std::sqrt(direction[0] * direction[0] + direction[1] * direction[1]);
    return {float(math.atan2(double(-direction[2]), double(horizontal))) * degrees, yaw, 0.0f};
}
void normalize(std::array<float, 3>& angles) noexcept {
    normalize_movement_angles(angles);
}
void correct_spread(Command& command, const CommandFrame& frame,
                    const BufferedShotCommandBindings& b) {
    if (!(frame.flags & 0x4000u))
        return;
    const rage::SpreadParameters parameters{std::uint32_t(frame.weapon_id_d8),
                                            integer(frame.field_b0),
                                            frame.spread_d0,
                                            frame.inaccuracy_c4,
                                            frame.weapon_field_100,
                                            frame.pellet_count_19c,
                                            convar_integer(b.convars.shotgun_patterns) != 0,
                                            frame.weapon_kind_124};
    const auto spread =
        rage::generate_spread_sample(parameters, std::uint8_t(command.random_seed), b.spread_tables,
                                     b.random, b.shotgun_pattern, b.spread_trigonometry);
    const float radius = std::sqrt(spread.first * spread.first + spread.second * spread.second);
    if (std::numeric_limits<float>::min() > radius)
        return;
    constexpr float degrees = 57.2957763671875f;
    if (frame.flags & 8u) {
        command.angles[0] += b.arctangent(radius) * degrees;
        command.angles[2] =
            -(float(b.angle_math.atan2(double(spread.first), double(spread.second))) * degrees);
        return;
    }
    MovementVector forward, right, up;
    movement_angle_vectors({command.angles[0], command.angles[1], command.angles[2]}, &forward,
                           &right, &up);
    const RageVector direction{forward.x - (up.x * spread.second + right.x * spread.first),
                               forward.y - (up.y * spread.second + right.y * spread.first),
                               forward.z - (up.z * spread.second + right.z * spread.first)};
    command.angles = direction_angles(direction, b.angle_math);
    normalize(command.angles);
}
void refresh_pending_shot(const CommandFrame& frame, const lagcomp::EvaluationState& selected,
                          const BufferedShotCommandBindings& b) {
    auto* feedback = rage::feedback_owner_state(b.pending_shots.active);
    if (!feedback || feedback->pending.empty())
        return;
    auto& shot = feedback->pending.back();
    if (shot.words[1] != std::uint32_t(frame.command_number_ac))
        return;
    rage::shot_capture_target(shot, selected, frame.origin_dc, 1, b.pending_shots);
}
struct PredictionTimeRestore {
    float& time;
    float saved;
    explicit PredictionTimeRestore(float& value) noexcept : time(value), saved(value) {}
    ~PredictionTimeRestore() {
        time = saved;
    }
};
}

BufferedShotCommandStorage::BufferedShotCommandStorage() noexcept {
    diagnostics.count = 0;
    for (auto& entry : diagnostics.entries)
        entry.target_handle = UINT32_MAX;
    for (auto& frame : frames)
        initialize_command_frame(frame);
}
void prepare_buffered_shot_commands(BufferedShotCommandOwner& owner) noexcept {
    if (auto* source = owner.live()) {
        source->prepare(owner.state);
        return;
    }
    if (owner.decoded()->choked == 0)
        owner.state.diagnostics.count = 0;
}
void repair_buffered_shot_commands(BufferedShotCommandOwner& owner, CommandFrame& current,
                                   void* player, std::uint8_t send_packet) {
    if (auto* source = owner.live()) {
        source->repair(owner.state, current, player, send_packet);
        return;
    }
    const auto& b = *owner.decoded();
    auto& slot = owner.state.frames[std::uint32_t(current.command_number_ac) % 150u];
    if (b.enabled && current.field_1a0 == 0)
        std::memcpy(&slot, &current, sizeof current);
    else
        slot.command_number_ac = -1;
    auto& diagnostics = owner.state.diagnostics;
    if (!b.checksum || diagnostics.count == 0 || !b.enabled || (current.flags & 0x40u) ||
        !b.firing_mode_enabled || command_roll_blocked(b.command_environment) ||
        current.choked_15c <= 0 || (!send_packet && current.choked_15c < current.choke_limit_fc))
        return;

    const auto first = add(b.last_outgoing, 1);
    const auto number = current.command_number_ac;
    const auto count = diagnostics.count;
    for (std::uint32_t ordinal = 0; ordinal < count; ++ordinal) {
        auto& entry = diagnostics.entries[ordinal];
        const auto age_bits =
            std::bit_cast<std::uint32_t>(b.clock_time - std::bit_cast<float>(entry.clock));
        if (std::bit_cast<float>(age_bits & 0x7FFFFFFFu) > 1.0f)
            break;
        if (entry.command < first || entry.command >= number ||
            !needs_repair(entry, current, player, b))
            continue;
        const auto index = std::uint32_t(entry.command) % 150u;
        auto& raw = b.commands[index];
        auto& verified = b.verified[index];
        if (raw.number != entry.command || verified.command.number != entry.command ||
            !(verified.command.movement.buttons & 1u))
            continue;
        Command command = verified.command;
        auto& frame = owner.state.frames[index];
        if (frame.command_number_ac != entry.command || frame.weapon_id_d8 == 64 ||
            frame.weapon_id_d8 != current.weapon_id_d8 ||
            frame.weapon_address != current.weapon_address)
            continue;

        PredictionTimeRestore restore(b.prediction_time);
        b.prediction_time = float(frame.tick_base_c8) * b.interval;
        refresh_prediction_frame(frame, entry.command, b);
        rage::BufferedShotRepairScratch scratch;
        const auto* selected =
            rage::repair_buffered_shot(frame, player, command, entry, scratch, b.repair);
        if (!selected)
            continue;
        refresh_pending_shot(frame, *selected, b);
        const RageVector point{std::bit_cast<float>(selected->words[3]),
                               std::bit_cast<float>(selected->words[4]),
                               std::bit_cast<float>(selected->words[5])};
        command.angles =
            direction_angles({point[0] - frame.origin_dc[0], point[1] - frame.origin_dc[1],
                              point[2] - frame.origin_dc[2]},
                             b.angle_math);
        for (std::size_t axis = 0; axis < 3; ++axis)
            command.angles[axis] -= frame.recoil_angles_9c[axis];
        normalize(command.angles);
        correct_spread(command, frame, b);
        command.tick = add(integer(selected->words[0xB4 / 4]),
                           ticks(read<float>(&current, 0x198), b.interval));
        if ((frame.flags & 0x400008u) == 0x400008u)
            command.angles[0] = normalize_movement_angle(command.angles[0]) - 720.0f;
        command.movement.forward_move = frame.input_forward_78;
        command.movement.side_move = frame.input_side_80;
        reorient_command_movement(command, frame.angles_118, frame.state_104,
                                  (frame.flags & 0x40000u) != 0, true, b.button_policy,
                                  b.trigonometry);
        sanitize_command(command, (frame.flags & 8u) != 0, b.command_environment);
        if (b.send_mode)
            rage::publish_command_animation_angles(b.animation_angles, command.number,
                                                   command.angles);
        save_final_command(command);
        publish_verified_command(verified, command, b.checksum);
        const auto predicted = raw.predicted;
        std::memcpy(&raw, &command, sizeof command);
        raw.predicted = predicted;
    }
    diagnostics.count = 0;
}

} // namespace skeet::client

// ============================================================================
// Subcomponent: buffered_shot_lifecycle.cpp
// ============================================================================

namespace skeet::client {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define BUFFERED_SHOT_MEMBER __thiscall
#else
#define BUFFERED_SHOT_MEMBER
#endif
using PrimaryEntry = void(BUFFERED_SHOT_MEMBER*)(void*);
void BUFFERED_SHOT_MEMBER buffered_shot_destruction_entry(void*);
void BUFFERED_SHOT_MEMBER buffered_shot_forward_buffered_shot_reset_entry(void*);
void BUFFERED_SHOT_MEMBER buffered_shot_reset_entry(void*);
void BUFFERED_SHOT_MEMBER buffered_shot_unchanged_entry(void*) {} //actual433CCE8E RET

// Exact primary4346FC00. Slot5 forwards to the currently installed slot7.
const std::array<PrimaryEntry, 9> buffered_shot_primary_table{
    buffered_shot_destruction_entry,   buffered_shot_unchanged_entry, buffered_shot_unchanged_entry, buffered_shot_unchanged_entry, buffered_shot_unchanged_entry,
    buffered_shot_forward_buffered_shot_reset_entry, buffered_shot_unchanged_entry, buffered_shot_reset_entry,     buffered_shot_unchanged_entry};
}

class BufferedShotLifecycleOwner {
public:
    const PrimaryEntry* vtable = nullptr;
    NativeCommandPhaseReceiver command_receiver;
    BufferedShotLifecycleBindings bindings;

    explicit BufferedShotLifecycleOwner(BufferedShotLifecycleBindings source) : bindings(source) {
        bindings.primary.append(this); //4347FE5A ->433676A0, primary still null
        bindings.commands.append(command_receiver, &bindings.command); //4347FE62
        vtable = buffered_shot_primary_table.data(); //4347FE69, primary4346FC00
        bindings.command.state.diagnostics.count = 0; //4347FE72, before the secondary table write
        bindings.commands.publish(command_receiver, CommandOwnerId::owner_4346f01c); //4347FE76
        // Reuse sparse4347FE72..FEA4 construction on the existing state: count,
        // six handle sentinels and all150 sparse43366497 frame constructors.
        ::new (static_cast<void*>(&bindings.command.state)) BufferedShotCommandStorage;
    }
};

namespace {
void BUFFERED_SHOT_MEMBER buffered_shot_destruction_entry(void* object) {
    auto* owner = static_cast<BufferedShotLifecycleOwner*>(object);
    destroy_buffered_shot_lifecycle(owner, owner->bindings.active);
}
void BUFFERED_SHOT_MEMBER buffered_shot_forward_buffered_shot_reset_entry(void* object) {
    refresh_buffered_shot_lifecycle(*static_cast<BufferedShotLifecycleOwner*>(object));
}
void BUFFERED_SHOT_MEMBER buffered_shot_reset_entry(void* object) {
    reset_buffered_shot_lifecycle(*static_cast<BufferedShotLifecycleOwner*>(object));
}
void buffered_shot_factory_entry(void* context) {
    const auto& source = *static_cast<BufferedShotFactoryBindings*>(context);
    create_buffered_shot_owner(source.lifecycle);
}
}

std::size_t buffered_shot_lifecycle_size() noexcept {
    return sizeof(BufferedShotLifecycleOwner);
}

BufferedShotLifecycleOwner&
construct_buffered_shot_lifecycle(void* storage, BufferedShotLifecycleBindings bindings) {
    return *::new (storage) BufferedShotLifecycleOwner(bindings);
}

BufferedShotLifecycleOwner* create_buffered_shot_owner(BufferedShotLifecycleBindings bindings) {
    static_assert(sizeof(BufferedShotCommandStorage) == buffered_shot_native_size);
    static_assert(sizeof(BufferedShotLifecycleOwner) <= buffered_shot_native_size);
    const auto memory = bindings.primary.memory;
    constexpr auto allocation_size = buffered_shot_native_size + 0x20u;
    auto* const raw = memory.allocate(memory.process_heap, 8, allocation_size);
    if (!raw) {
        //43485BFA XOR EAX,EAX followed by the recorded43485BFC MOV.
        bindings.active = nullptr;
        return nullptr;
    }
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15u) & ~std::uintptr_t(15u);
    const auto displacement = static_cast<std::uint32_t>(header - address);
    const auto size = static_cast<std::uint32_t>(allocation_size);
    std::memcpy(reinterpret_cast<void*>(header + 8u), &displacement, sizeof displacement);
    std::memcpy(reinterpret_cast<void*>(header + 12u), &size, sizeof size);

    // The portable lifecycle borrows the native diagnostics/frame projection.
    // HEAP_ZERO_MEMORY covered that whole126B8 state before the constructor;
    // on failure the borrowed state is left unchanged, as in the native path.
    std::memset(&bindings.command.state, 0, sizeof bindings.command.state);
    auto& owner =
        construct_buffered_shot_lifecycle(reinterpret_cast<void*>(header + 16u), bindings);
    //mem_int3.h:560 records this MOV at43485BFC with size5; execution then
    //continues at the unmodified43485C01 RET. No saved-return substitution.
    bindings.active = &owner;
    return &owner;
}

void destroy_buffered_shot_lifecycle(BufferedShotLifecycleOwner* owner,
                                     BufferedShotLifecycleOwner*& active) {
    active = nullptr; //4334D8FE, before433837D0's null guard
    if (!owner)
        return;
    const auto memory = owner->bindings.primary.memory;
    owner->~BufferedShotLifecycleOwner();
    engine::UnloadSystemFunctions system;
    system.free_heap = memory.free;
    system.process_heap = memory.process_heap;
    engine::free_native_allocation(owner, system);
}

void reset_buffered_shot_lifecycle(BufferedShotLifecycleOwner& owner) noexcept {
    auto& state = owner.bindings.command.state;
    state.diagnostics.count = 0;
    for (auto& frame : state.frames)
        frame.command_number_ac = -1;
}

void refresh_buffered_shot_lifecycle(BufferedShotLifecycleOwner& owner) {
    owner.vtable[7](&owner);
}
void* buffered_shot_lifecycle_object(BufferedShotLifecycleOwner& owner) noexcept {
    return &owner;
}
BufferedShotCommandOwner* buffered_shot_command_owner(BufferedShotLifecycleOwner* owner) noexcept {
    return owner ? &owner->bindings.command : nullptr;
}
BufferedShotCommandStorage*
buffered_shot_command_storage(BufferedShotLifecycleOwner* owner) noexcept {
    const auto* command = buffered_shot_command_owner(owner);
    return command ? &command->state : nullptr;
}
rage::ShotDiagnosticHistory* buffered_shot_diagnostics(BufferedShotLifecycleOwner* owner) noexcept {
    auto* state = buffered_shot_command_storage(owner);
    return state ? &state->diagnostics : nullptr;
}

void bind_buffered_shot_factory(NativeStartupSources& sources,
                                BufferedShotFactoryBindings& source) {
    const auto& image = sources.data();
    if (&source.registries.image() != &image)
        throw std::invalid_argument(
            "Buffered-shot factory must share the startup initialization image");
    if (&source.lifecycle.primary.registry != &source.registries.at(0x43475B78u))
        throw std::invalid_argument(
            "Buffered-shot factory requires the canonical43475B78 lifecycle registry");
    if (&source.lifecycle.active != &image.native_cell<BufferedShotLifecycleOwner*>(0x43475968u))
        throw std::invalid_argument(
            "Buffered-shot factory requires the canonical43475968 full-owner slot");
    sources.bind_initializer({buffered_shot_factory, &source, buffered_shot_factory_entry});
}

#undef BUFFERED_SHOT_MEMBER
}

// ============================================================================
// Subcomponent: buffered_shot_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
std::ptrdiff_t buffered_shot_field(ClientRuntimeStorage& s, std::uint32_t hash) {
    for (const auto& recipe : engine::build_receive_offset_recipes())
        if (recipe.initial_xor_key == hash)
            return engine::DecodedOffset(s.offsets().globals, recipe.pointer_identity,
                                         recipe.xor_key_identity, recipe.initial_xor_key, 0);
    throw std::invalid_argument("Buffered-shot field has no recovered receive recipe");
}
ClientRuntimeStorage& checked(ClientRuntimeStorage& s, const EngineRuntimeBindings& e,
                              ConfigurationRuntimeOwner& c, HistoryRuntimeOwner& h,
                              CommandCaptureRuntimeOwner& capture, animation::RuntimeOwner& b,
                              CrtRuntimeOwner& crt, esp::EspRuntimeOwner& esp,
                              rage::RuntimeOwner& rage, rage::FeedbackRuntimeOwner& feedback) {
    if (&e.variables.data() != &s.data() || &c.storage() != &s || &h.storage() != &s ||
        &capture.storage() != &s || &b.storage() != &s || &b.players() != &h.players().cache() ||
        &crt.storage() != &s || &esp.storage() != &s || &rage.storage() != &s ||
        &feedback.storage() != &s || &feedback.runtime().players != &h.players().cache())
        throw std::invalid_argument(
            "Buffered-shot runtime requires its actual shared engine, history, capture, animation, CRT, ESP, RAGE and feedback owners");
    if (!s.data().native_cell<void*>(0x4346A894u) || !s.data().native_cell<void*>(0x43475A60u) ||
        !s.data().native_cell<void*>(0x434757DCu) || !s.data().native_cell<void*>(0x43475B2Cu))
        throw std::invalid_argument(
            "Buffered-shot runtime requires genuine input and command-clock publications");
    if (s.data().native_cell<BufferedShotLifecycleOwner*>(0x43475968u))
        throw std::logic_error("Buffered-shot context must precede its actual feature factory");
    initialize_source_command_checksum(s.data());
    return s;
}
}
struct BufferedShotRuntimeOwner::Data final : BufferedShotCommandSource {
    ClientRuntimeStorage& storage;
    const EngineRuntimeBindings& engine;
    ConfigurationRuntimeOwner& configuration;
    HistoryRuntimeOwner& history;
    CommandCaptureRuntimeOwner& capture;
    animation::RuntimeOwner& animation;
    CrtRuntimeOwner& crt;
    esp::EspRuntimeOwner& esp;
    rage::RuntimeOwner& rage;
    rage::FeedbackRuntimeOwner& feedback;
    BufferedShotCommandStorage state_;
    BufferedShotCommandOwner command_;
    BufferedShotFactoryBindings factory;
    Data(ClientRuntimeStorage& s, const EngineRuntimeBindings& e, ConfigurationRuntimeOwner& c,
         HistoryRuntimeOwner& h, CommandCaptureRuntimeOwner& cap, animation::RuntimeOwner& b,
         CrtRuntimeOwner& process, esp::EspRuntimeOwner& esp_owner, rage::RuntimeOwner& rage_owner,
         rage::FeedbackRuntimeOwner& feedback_owner)
        : storage(checked(s, e, c, h, cap, b, process, esp_owner, rage_owner, feedback_owner)),
          engine(e), configuration(c), history(h), capture(cap), animation(b), crt(process),
          esp(esp_owner), rage(rage_owner), feedback(feedback_owner), command_(state_, *this),
          factory(s.owner_registries(),
                  {{s.owner_registries().at(0x43475B78u), s.memory()},
                   s.command_phases(),
                   command_,
                   s.data().native_cell<BufferedShotLifecycleOwner*>(0x43475968u)}) {}
    void prepare(BufferedShotCommandStorage& state) noexcept override {
        if (*storage.data().native_cell<std::int32_t*>(0x434757DCu) == 0)
            state.diagnostics.count = 0;
    }
    void repair(BufferedShotCommandStorage& state, CommandFrame& frame, void* player,
                std::uint8_t send) override {
        if (&state != &state_)
            throw std::invalid_argument(
                "Buffered-shot source endpoint must use its canonical frame/diagnostic state");
        auto& data = storage.data();
        const BufferedShotCommandTiming timing{data.native_cell<void*>(0x43475A60u),
                                               data.native_cell<std::int32_t*>(0x43468F9Cu),
                                               data.native_cell<std::int32_t*>(0x434670A8u)};
        const CommandButtonPolicy policy{
            data.native_cell<std::array<std::uint32_t, 150>>(0x43476090u),
            configuration.anti_aim_other().leg_movement.value,
            *data.native_cell<std::int32_t*>(0x434757DCu)};
        const CommandTrigonometry trig{animation.trigonometry().sine,
                                       animation.trigonometry().cosine};
        const rage::BufferedShotRepairBindings reconstruction{rage};
        auto* input = static_cast<std::byte*>(data.native_cell<void*>(0x4346A894u));
        const BufferedShotCommandBindings bindings{
            configuration.rage_settings().global.enabled.value,
            configuration.rage_settings().global.silent_aim.value,
            configuration.settings_menu().hold_aim,
            *data.native_cell<std::int32_t*>(0x434757DCu),
            *data.native_cell<std::int32_t*>(0x43475B2Cu),
            configuration.settings_menu().maximum_unlag,
            *data.native_cell<float*>(0x4346A334u),
            *data.native_cell<float*>(0x4346A338u),
            data.native_cell<float>(0x43469254u),
            *data.native_cell<float*>(0x43467EB8u),
            data.native_cell<CommandChecksum>(0x43467C48u),
            *reinterpret_cast<Command**>(input + 0xF0),
            *reinterpret_cast<VerifiedCommand**>(input + 0xF4),
            data.native_cell<EntityCommandOwner*>(0x43475898u),
            data.native_cell<void*>(0x4346A514u),
            data.native_cell<void*>(0x4346A890u),
            history.players().cache(),
            {buffered_shot_field(storage, 0xD5FD2E2Cu), buffered_shot_field(storage, 0x7F5ED2D0u), buffered_shot_field(storage, 0x9D920E35u),
             buffered_shot_field(storage, 0xE2FBE695u), buffered_shot_field(storage, 0x3309C0CFu)},
            {engine.variables.at(0x4346747Cu), engine.variables.at(0x4346A748u),
             engine.variables.at(0x4346A330u), engine.variables.at(0x4346794Cu),
             engine.variables.at(0x43468528u), engine.variables.at(0x43468520u)},
            capture.angle_environment(),
            data.native_cell<network::LatencySamples>(0x43478C70u),
            timing,
            policy,
            trig,
            crt.spread().tables(),
            crt.random(),
            esp.shotgun_pattern(),
            animation.trigonometry(),
            {animation::native_sse_atan2},
            animation::native_sse_arctangent,
            data.native_cell<std::array<animation::AnimationAngleSample, 2>>(0x434759DCu),
            feedback.pending_shots(),
            reconstruction};
        BufferedShotCommandOwner current(state, bindings);
        repair_buffered_shot_commands(current, frame, player, send);
    }
};
BufferedShotRuntimeOwner::BufferedShotRuntimeOwner(
    ClientRuntimeStorage& s, const EngineRuntimeBindings& e, ConfigurationRuntimeOwner& c,
    HistoryRuntimeOwner& h, CommandCaptureRuntimeOwner& capture, animation::RuntimeOwner& b,
    CrtRuntimeOwner& crt, esp::EspRuntimeOwner& esp, rage::RuntimeOwner& rage,
    rage::FeedbackRuntimeOwner& feedback)
    : data_(std::make_unique<Data>(s, e, c, h, capture, b, crt, esp, rage, feedback)) {}
BufferedShotRuntimeOwner::~BufferedShotRuntimeOwner() = default;
ClientRuntimeStorage& BufferedShotRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
BufferedShotCommandOwner& BufferedShotRuntimeOwner::command() noexcept {
    return data_->command_;
}
BufferedShotCommandStorage& BufferedShotRuntimeOwner::state() noexcept {
    return data_->state_;
}
BufferedShotFactoryBindings& BufferedShotRuntimeOwner::factory_bindings() noexcept {
    return data_->factory;
}
void BufferedShotRuntimeOwner::bind_initializer(NativeStartupSources& sources) {
    bind_buffered_shot_factory(sources, data_->factory);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: combat_runtime_factories.cpp
// ============================================================================

namespace skeet::client {
namespace {
void* allocate(engine::JobMemory memory, std::size_t native_size, std::size_t source_size) {
    return engine::allocate_owner_storage(memory, std::max(native_size, source_size));
}
void require_unlinked(const ConfigDescriptor& descriptor) {
    if (descriptor.next || descriptor.previous)
        throw std::invalid_argument("Combat factory configuration was already registered");
}
void require_packet_state(const PacketRuntimeFactoryBindings& source) {
    const auto& state = source.command.state;
    if (&source.command.settings != &source.settings ||
        &state.mode != &source.settings.amount.value ||
        &state.variance != &source.settings.variance.value ||
        &state.configured_cap != &source.settings.limit.value)
        throw std::invalid_argument("Packet factory must share its registered fake-lag controls");
    require_unlinked(source.settings.amount);
    require_unlinked(source.settings.variance);
    require_unlinked(source.settings.limit);
}
void require_exploit_state(const ExploitRuntimeFactoryBindings& source) {
    const auto& command = source.owner.command;
    if (&command.controls != &command.settings.controls)
        throw std::invalid_argument("Exploit factory must share its canonical runtime controls");
    const auto& config = command.controls.settings;
    for (const auto* descriptor : std::array<const ConfigDescriptor*, 8>{
             &config.on_shot_anti_aim, &config.on_shot_key, &config.double_tap_fake_lag_limit,
             &config.fake_peek, &config.fake_peek_key, &config.double_tap, &config.double_tap_key,
             &config.double_tap_mode})
        require_unlinked(*descriptor);
}
void require_anti_aim_state(const AntiAimRuntimeFactoryBindings& source) {
    const auto& command = source.owner.command;
    if (&command.precalculation.common != static_cast<const anti_aim::State*>(&command.state) ||
        &command.finisher.common != &command.state || command.finisher.matrices_520.size() != 256u)
        throw std::invalid_argument(
            "Anti-aim factory requires its canonical state and 256 bone matrices");
    const auto& config = command.config;
    for (const auto* descriptor : std::array<const ConfigDescriptor*, 11>{
             &config.yaw_base, &config.yaw_jitter, &config.yaw_jitter_offset, &config.edge_yaw,
             &config.freestanding, &config.body_yaw_offsets, &config.yaw_offsets,
             &config.freestanding_hotkey, &config.freestanding_body_yaw, &config.roll,
             &config.custom_pitch})
        require_unlinked(*descriptor);
}
void require_effects_state(const EffectsRuntimeFactoryBindings& source) {
    const auto& owner = source.owner;
    const auto& command = owner.command;
    const auto& lifecycle = owner.lifecycle;
    const auto& updates = owner.updates;
    if (&lifecycle.models != &command.models || &lifecycle.world != &command.world ||
        &lifecycle.colors != &command.models.settings || &updates.models != &command.models ||
        &updates.world != &command.world || &updates.lifecycle != &lifecycle.state ||
        &updates.view.settings != &command.world.settings ||
        &command.camera.force_dead != &command.world.settings.third_person_dead.value ||
        &command.camera.alive_hotkey != &command.world.settings.third_person_key.value)
        throw std::invalid_argument(
            "Effects factory requires its canonical camera, world and model views");
    if (command.models.solid_material || command.models.shadow_material ||
        command.models.base_texture || command.models.metallic_environment ||
        command.models.glow_environment || command.models.fullscreen_texture ||
        lifecycle.state.allocation_760)
        throw std::invalid_argument("Effects factory requires its unallocated resource state");
    for (const auto& material : command.models.materials)
        if (material.material)
            throw std::invalid_argument("Effects materials were already initialized");
    // These owner descriptors must be fresh. Independent CRT descriptors in
    // EffectsSettings/ColoredModelsSettings are deliberately left untouched.
    const auto& effects = command.world.settings;
    for (const auto* descriptor : std::array<const ConfigDescriptor*, 10>{
             &effects.impact_duration, &effects.wall_transparency, &effects.prop_transparency,
             &effects.brightness, &effects.night_color, &effects.remove_fog,
             &effects.disable_post_processing, &effects.instant_scope, &effects.bullet_impacts,
             &effects.remove_sky})
        require_unlinked(*descriptor);
    const auto& colors = lifecycle.colors;
    for (const auto* group : std::array<const esp::ModelColorGroup*, 8>{
             &colors.player, &colors.teammate, &colors.local_player, &colors.local_fake,
             &colors.on_shot, &colors.hands, &colors.weapon_viewmodel, &colors.weapons}) {
        require_unlinked(group->enabled);
        require_unlinked(group->primary_color);
        require_unlinked(group->material);
        require_unlinked(group->secondary_color);
    }
    for (const auto* descriptor : std::array<const ConfigDescriptor*, 10>{
             &colors.props, &colors.props_color, &colors.player.behind_wall,
             &colors.player.behind_wall_color, &colors.teammate.behind_wall,
             &colors.teammate.behind_wall_color, &colors.local_transparency,
             &colors.on_shot_duration, &colors.shadow, &colors.shadow_color})
        require_unlinked(*descriptor);
}
void zero_packet_state(network::FakeLagState& state) {
    // HEAP_ZERO_MEMORY covers these owner words. The leading references keep
    // pointing to the same three independently registered descriptor payloads.
    state.flags = 0;
    state.target_trigger_time = 0.0f;
    state.cycle = 0;
    state.target = 0;
    state.selected = 0;
    state.recorded_choked = 0;
    state.value_58 = 0;
    state.last_trigger_time = 0.0f;
    state.tracked_target = 0;
    state.previous_selected = 0;
    state.last_origin = {};
    state.saved_duck_amount_bits = 0;
    state.previous_speed_delta = 0.0f;
    state.value_7c = 0;
    state.acceleration = {};
    state.value_8c = 0;
}
void zero_anti_aim_state(anti_aim::CommandOwner& command) {
    // Same fresh-allocation projection as anti_aim::create_owner; this caller
    // reserves the native3520-byte minimum before using the complete constructor.
    command.config = anti_aim::Config{};
    command.state = anti_aim::AngleStageState{};
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
}
void zero_effects_values(EffectsRuntimeFactoryBindings& source) {
    auto& effects = source.owner.command.world.settings;
    effects.brightness.value = 0;
    for (auto* value : std::array<BooleanConfig*, 5>{
             &effects.remove_fog, &effects.disable_post_processing, &effects.instant_scope,
             &effects.bullet_impacts, &effects.remove_sky})
        value->value = 0;
    auto& colors = source.owner.lifecycle.colors;
    for (auto* group : std::array<esp::ModelColorGroup*, 8>{
             &colors.player, &colors.teammate, &colors.local_player, &colors.local_fake,
             &colors.on_shot, &colors.hands, &colors.weapon_viewmodel, &colors.weapons})
        group->enabled.value = 0;
    colors.props.value = 0;
    colors.player.behind_wall.value = 0;
    colors.teammate.behind_wall.value = 0;
    colors.shadow.value = 0;
}
void packet_entry(void* context) {
    create_packet_runtime_owner(*static_cast<CombatRuntimeFactoryBindings*>(context));
}
void exploit_entry(void* context) {
    create_exploit_runtime_owner(*static_cast<CombatRuntimeFactoryBindings*>(context));
}
void anti_aim_entry(void* context) {
    create_anti_aim_runtime_owner(*static_cast<CombatRuntimeFactoryBindings*>(context));
}
void effects_entry(void* context) {
    create_effects_runtime_owner(*static_cast<CombatRuntimeFactoryBindings*>(context));
}
void script_entry(void* context) {
    create_script_runtime_owner(*static_cast<CombatRuntimeFactoryBindings*>(context));
}
}

void* create_packet_runtime_owner(CombatRuntimeFactoryBindings& source) {
    return create_packet_runtime_owner(source.packet, source.configuration);
}
void* create_packet_runtime_owner(PacketRuntimeFactoryBindings& packet,
                                  ConfigRegistries& configuration) {
    require_packet_state(packet);
    auto* storage = allocate(packet.lifecycle.memory, 0x98u, network::packet_owner_size());
    // Native null edge4347DA99 is INT1 followed by unresolved junk. This is
    // the source allocation-failure policy, not a claimed native continuation.
    if (!storage)
        throw std::bad_alloc();
    zero_packet_state(packet.command.state);
    network::construct_packet_owner(storage, packet.lifecycle, packet.commands, configuration,
                                    packet.settings, packet.command);
    return storage;
}
void* create_exploit_runtime_owner(CombatRuntimeFactoryBindings& source) {
    auto& exploit = source.exploit;
    require_exploit_state(exploit);
    auto* storage =
        allocate(exploit.registries.primary.memory, network::native_exploit_owner_allocation_size,
                 network::exploit_owner_size());
    // Native null edge4347FF44 is INT1/junk; no owner is registered or state
    // projected on that path. Allocation failure is explicit in the source.
    if (!storage)
        throw std::bad_alloc();
    network::construct_allocated_exploit_owner(storage, exploit.registries, source.configuration,
                                               exploit.owner);
    return storage;
}
void* create_anti_aim_runtime_owner(CombatRuntimeFactoryBindings& source) {
    return create_anti_aim_runtime_owner(source.anti_aim, source.configuration);
}
void* create_anti_aim_runtime_owner(AntiAimRuntimeFactoryBindings& aa,
                                    ConfigRegistries& configuration) {
    require_anti_aim_state(aa);
    auto* storage = allocate(aa.registries.primary.memory, 0x3520u, anti_aim::owner_size());
    if (!storage) {
        aa.owner.active = nullptr;
        return nullptr;
    } //43485CE3..43485CE5
    zero_anti_aim_state(aa.owner.command);
    anti_aim::construct_owner(storage, aa.registries, configuration, aa.owner);
    aa.owner.active = &aa.owner.command; //43485CE5, canonical source view
    return storage;
}
void* create_effects_runtime_owner(CombatRuntimeFactoryBindings& source) {
    return create_effects_runtime_owner(source.effects, source.configuration);
}
void* create_effects_runtime_owner(EffectsRuntimeFactoryBindings& effects,
                                   ConfigRegistries& configuration) {
    require_effects_state(effects);
    auto& image = effects.owner.data;
    if (&configuration.descriptors != &image.native_cell<ConfigLink>(0x434798BCu) ||
        &configuration.hotkeys != &image.native_cell<HotkeyLink>(0x434798C4u) ||
        &effects.owner.active != &image.native_cell<effects::EffectsCommandOwner*>(0x43475A1Cu))
        throw std::invalid_argument(
            "Effects factory requires its canonical data, configuration and singleton cells");
    auto* storage =
        allocate(effects.registries.lifecycle.memory, 0x768u, effects::model_effects_owner_size());
    if (!storage) {
        effects.owner.active = nullptr;
        return nullptr;
    } //43484FAA..43484FAC
    zero_effects_values(effects);
    effects::construct_model_effects_owner(storage, effects.registries, configuration,
                                           effects.owner);
    // construct_model_effects_owner publishes the same source view on success.
    return storage;
}
script::ScriptOwner* create_script_runtime_owner(CombatRuntimeFactoryBindings& source) {
    return create_script_runtime_owner(source.scripting);
}
script::ScriptOwner* create_script_runtime_owner(ScriptRuntimeFactoryBindings& scripting) {
    script::ScriptModulesOwner::validate_construction(scripting.runtime, scripting.modules);
    if (!scripting.owner.system.add_handler || !scripting.owner.system.remove_handler ||
        !scripting.owner.system.set_bit || !scripting.owner.executable.bitmap.buffer)
        throw std::invalid_argument(
            "Script factory requires its existing exception imports and executable bitmap");
    const auto memory = scripting.owner.primary.memory;
    auto* storage = allocate(memory, 0x288u, sizeof(script::ScriptOwner));
    if (!storage) {
        scripting.owner.current_owner = nullptr;
        return nullptr;
    } //4348E61F..4348E621
    return &script::construct_script_owner({storage, memory}, scripting.runtime, scripting.owner,
                                           scripting.modules);
}

void bind_combat_runtime_factories(NativeStartupSources& startup,
                                   CombatRuntimeFactoryBindings& source) {
    const auto& image = engine::require_native_data_image(source.data);
    if (&image != &startup.data() || &source.registries.image() != &image ||
        &source.configuration.descriptors != &image.native_cell<ConfigLink>(0x434798BCu) ||
        &source.configuration.hotkeys != &image.native_cell<HotkeyLink>(0x434798C4u))
        throw std::invalid_argument(
            "Combat factories require the common startup image and configuration sentinels");
    const auto memory = source.packet.lifecycle.memory;
    if (!memory.allocate || !memory.free || !memory.process_heap)
        throw std::invalid_argument("Combat factories require the canonical process heap imports");
    const auto registry = [&](engine::OwnerRegistryBinding binding, std::uint32_t address) {
        if (&binding.registry != &source.registries.at(address) ||
            binding.memory.allocate != memory.allocate || binding.memory.free != memory.free ||
            binding.memory.process_heap != memory.process_heap)
            throw std::invalid_argument(
                "Combat factories must share their native registry cells and heap");
    };
    auto& packet = source.packet;
    auto& exploit = source.exploit;
    auto& aa = source.anti_aim;
    auto& effects = source.effects;
    auto& scripting = source.scripting;
    auto& modules = scripting.modules;
    registry(packet.lifecycle, 0x43475B78u);
    registry(exploit.registries.primary, 0x43475B78u);
    registry(exploit.registries.hud, 0x43475A9Cu);
    registry(exploit.registries.messages, 0x43475B94u);
    registry(aa.registries.primary, 0x43475B78u);
    registry(aa.registries.hud, 0x43475A9Cu);
    registry(aa.registries.frames, 0x43475AD8u);
    registry(effects.registries.lifecycle, 0x43475B78u);
    registry(effects.registries.hud, 0x43475A9Cu);
    registry(effects.registries.updates, 0x43475988u);
    registry(effects.registries.frames, 0x43475AD8u);
    registry(scripting.owner.primary, 0x43475B78u);
    registry(modules.hud_callbacks, 0x43475A9Cu);
    registry(modules.network_callbacks, 0x43475B94u);
    registry(modules.frame_callbacks, 0x43475AD8u);
    registry(modules.update_callbacks, 0x43475988u);
    registry(modules.event_callbacks, 0x434758E0u);
    if (&packet.commands.image() != &image || &exploit.registries.commands != &packet.commands ||
        &aa.registries.commands != &packet.commands ||
        &effects.registries.commands != &packet.commands ||
        &modules.command_callbacks != &packet.commands ||
        &effects.registries.entities != &modules.entity_callbacks ||
        &effects.registries.entities.image() != &image ||
        &effects.registries.empty_ticks.image() != &image ||
        &exploit.registries.data_updates.image() != &image ||
        &exploit.registries.prediction.image() != &image ||
        &modules.prediction_callbacks != &exploit.registries.prediction ||
        &modules.config.configuration != &source.configuration)
        throw std::invalid_argument(
            "Combat factories require shared typed registries and module configuration");
    if (&aa.owner.active != &image.native_cell<anti_aim::CommandOwner*>(0x43475940u) ||
        &effects.owner.active != &image.native_cell<effects::EffectsCommandOwner*>(0x43475A1Cu) ||
        &scripting.owner.current_owner != &image.native_cell<script::ScriptOwner*>(0x43475B00u))
        throw std::invalid_argument("Combat factories require their canonical singleton cells");
    if (&packet.command.target != &aa.owner.command.globals.target ||
        &exploit.owner.command.phase7.target != &packet.command.target ||
        &aa.owner.command.globals.scheduler_flags != &packet.commands.services().scheduler_flags ||
        &exploit.owner.command.command_flags != &packet.commands.services().scheduler_flags ||
        &modules.command.scheduler_flags != &packet.commands.services().scheduler_flags ||
        &aa.owner.hud.scripts != &scripting.runtime.bridge ||
        &exploit.owner.hud.scripts != &scripting.runtime.bridge)
        throw std::invalid_argument(
            "Combat commands must share targeting, scheduler flags, Lua bridge and exploit controls");
    require_packet_state(packet);
    require_exploit_state(exploit);
    require_anti_aim_state(aa);
    require_effects_state(effects);
    startup.bind_initializer({packet_runtime_factory, &source, packet_entry});
    startup.bind_initializer({exploit_runtime_factory, &source, exploit_entry});
    startup.bind_initializer({anti_aim_runtime_factory, &source, anti_aim_entry});
    startup.bind_initializer({effects_runtime_factory, &source, effects_entry});
    startup.bind_initializer({script_runtime_factory, &source, script_entry});
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: commands.cpp
// ============================================================================

namespace skeet::client {

void restore_channel_hook(void* engine_client, ChannelHookState& state) {
    void* const channel = engine::invoke<void*>(engine_client, 78);
    if (channel && state.replacement_table) {
        auto& table = *static_cast<void***>(channel);
        if (table == state.expected_table) {
            table = state.replacement_table;
            state.installed_channel = channel;
        }
    }
}

namespace {
using OriginalCreateMove = engine::MemberFunction<std::uint8_t, float, Command*>;
CommandEntry enter_after_channel_restore(const CommandEntryBindings& bindings, void* player,
                                         float sample_time, Command& command,
                                         OriginalCreateMove original) {
    const std::uint8_t result = original(player, sample_time, &command);
    if (engine::invoke<bool>(bindings.engine_client, 82))
        return {result, CommandPath::engine_return};
    if (engine::invoke<bool>(bindings.engine_client, 93))
        return {result, CommandPath::engine_return};
    if (command.tick != 0)
        return {result, CommandPath::active_command};
    const auto bytes = static_cast<const std::byte*>(player);
    if (bytes[bindings.rejection_bytes[0]] != std::byte{0})
        return {result, CommandPath::engine_return};
    if (bytes[bindings.rejection_bytes[1]] != std::byte{0})
        return {result, CommandPath::engine_return};
    return {result, CommandPath::empty_tick};
}
}
CommandEntry enter_create_move(const CommandEntryBindings& bindings, void* player,
                               float sample_time, Command& command) {
    restore_channel_hook(bindings.engine_client, *bindings.channel_hook);
    return enter_after_channel_restore(
        bindings, player, sample_time, command,
        reinterpret_cast<OriginalCreateMove>(bindings.original_player_table[289]));
}
CommandEntry enter_hooked_create_move(const CommandEntryBindings& bindings, void* player,
                                      float sample_time, Command& command) {
    restore_channel_hook(bindings.engine_client, *bindings.channel_hook);
    auto** current_table = *static_cast<void***>(player);
    const auto* metadata = static_cast<const engine::VtableHookMetadata*>(current_table[-2]);
    return enter_after_channel_restore(
        bindings, player, sample_time, command,
        reinterpret_cast<OriginalCreateMove>(metadata->original_table[289]));
}

void save_final_command(Command& command) noexcept {
    command.saved_buttons = command.movement.buttons;
    for (std::size_t index = 0; index != 3; ++index)
        std::memcpy(&command.saved_angles[index], &command.angles[index], sizeof(float));
}

bool command_roll_blocked(const ScriptCommandEnvironment& environment) noexcept {
    if (!environment.shared_player || !*environment.shared_player)
        return true;
    const auto* player = static_cast<const std::byte*>(*environment.shared_player);
    return player[environment.player_guard] != std::byte{0};
}

bool command_angles_unrestricted(const ScriptCommandEnvironment& environment) noexcept {
    return *environment.angle_guard == 0 && !command_roll_blocked(environment);
}

} // namespace skeet::client

// ============================================================================
// Subcomponent: commands_entities.cpp
// ============================================================================

namespace skeet::client {

void save_command_entity_properties(const CommandEntityBindings& bindings,
                                    CommandEntitySnapshot& saved) {
    saved.fill(0);
    for (std::uint32_t index = 1; index <= 64; ++index) {
        void* const entity = engine::invoke<void*>(bindings.entity_list, 3, index);
        if (entity && engine::invoke<bool>(entity, 158)) {
            auto* property = static_cast<std::byte*>(entity) + bindings.overridden_property;
            std::memcpy(&saved[index & 63], property, sizeof(std::uint32_t));
            std::memcpy(property, bindings.replacement_value, sizeof(std::uint32_t));
        }
    }
}

void restore_command_entity_properties(const CommandEntityBindings& bindings,
                                       const CommandEntitySnapshot& saved) {
    for (std::uint32_t index = 1; index <= 64; ++index) {
        if (saved[index & 63] == 0)
            continue;
        void* const entity = engine::invoke<void*>(bindings.entity_list, 3, index);
        if (entity && engine::invoke<bool>(entity, 158)) {
            auto* property = static_cast<std::byte*>(entity) + bindings.overridden_property;
            std::memcpy(property, &saved[index & 63], sizeof(std::uint32_t));
        }
    }
}

} // namespace skeet::client

// ============================================================================
// Subcomponent: commands_free_look.cpp
// ============================================================================

namespace skeet::client {

void set_command_view_angles(const CommandAngleBindings& bindings, std::array<float, 3>& angles) {
    for (float& angle : angles)
        if (std::isnan(angle))
            angle = 0.0f;
    if (!command_angles_unrestricted(bindings.environment)) {
        MovementAngles value{angles[0], angles[1], angles[2]};
        normalize_movement_angles(value);
        angles = {value.pitch, value.yaw, value.roll};
    }
    angles[2] = 0.0f;
    engine::invoke<void>(bindings.engine_client, 19, angles.data());
}

void process_command_free_look(FreeLookState& state, Command& command, const HotkeyBinding& hotkey,
                               InputBitmap keys, const CommandAngleBindings& bindings) {
    if (!hotkey_active(hotkey, keys)) {
        state.active = 0;
        state.delta = {0.0f, 0.0f, 0.0f};
        return;
    }
    if (state.active == 0) {
        state.base_angles = command.angles;
        state.active = 1;
    }
    const float base_pitch = state.base_angles[0];
    const float base_yaw = state.base_angles[1];
    const float yaw = state.delta[1] + (command.angles[1] - base_yaw);
    state.delta[0] = (command.angles[0] - base_pitch) + state.delta[0];
    const float maximum_pitch = 89.0f - base_pitch;
    const float minimum_pitch = -89.0f - base_pitch;
    if (state.delta[0] > maximum_pitch)
        state.delta[0] = maximum_pitch;
    else if (minimum_pitch > state.delta[0])
        state.delta[0] = minimum_pitch;
    const float normalized_yaw = normalize_movement_angle(yaw);
    const float lower_yaw = normalized_yaw > -170.0f ? normalized_yaw : -170.0f;
    state.delta[1] = lower_yaw < 170.0f ? lower_yaw : 170.0f;
    command.angles[0] = base_pitch;
    command.angles[1] = base_yaw;
    set_command_view_angles(bindings, command.angles);
    if (command.tick == 0)
        engine::invoke<void>(bindings.prediction, 13, command.angles.data());
}

} // namespace skeet::client

// ============================================================================
// Subcomponent: commands_script.cpp
// ============================================================================

namespace skeet::client {
namespace {
int marshal_command(script::Bridge& bridge, void* context) {
    const auto& bindings = *static_cast<const ScriptCommandBindings*>(context);
    const auto& api = *bridge.api;
    void* const state = bridge.state;
    api.rawgeti(state, -10000, bridge.runtime_ref);
    api.rawgeti(state, -1, 1);
    api.remove(state, -2);
    bindings.push_light_userdata(state, bindings.snapshot);
    *bindings.marshalling = 1;
    const int result = script::detail::callback_pcall(state, 1, 1);
    *bindings.marshalling = 0;
    if (result == 0)
        return 1;
    script::detail::report_callback_error(bridge, state);
    return -1;
}
}

int dispatch_command_script(const ScriptCommandBindings& bindings, Command& command,
                            float& move_yaw, std::uint8_t& send_packet,
                            std::uint32_t& scheduler_flags) {
    auto& bridge = *bindings.bridge;
    const std::lock_guard lock(bridge.vm_mutex);
    if (!bridge.state || bridge.runtime_ref == -1)
        return 0;
    const auto active = bridge.callback_active.find(script::callback_hash(bindings.event_name));
    if (active != bridge.callback_active.end() && active->second)
        return 0;
    if (!bindings.snapshot) {
        script::detail::report_callback_failure(bridge, "Lua setup_command snapshot is unavailable");
        return 0;
    }
    const bool unrestricted = command_angles_unrestricted(bindings.environment);
    const auto baseline = capture_script_command(*bindings.snapshot, command, move_yaw, send_packet,
                                                 *bindings.choked_commands, scheduler_flags);
    const int completed = script::dispatch_callbacks(bridge, bindings.event_name, marshal_command,
                                                     const_cast<ScriptCommandBindings*>(&bindings));
    if (completed != 0)
        apply_script_command(*bindings.snapshot, command, move_yaw, send_packet, scheduler_flags,
                             baseline, unrestricted, command_roll_blocked(bindings.environment));
    return completed;
}

} // namespace skeet::client

// ============================================================================
// Subcomponent: commands_script_state.cpp
// ============================================================================

namespace skeet::client {
namespace {
float wrapped(float angle) noexcept {
    if (std::abs(angle) > 180.0f) {
        const float turns = std::round(std::abs(angle / 360.0f));
        const float rotation = turns * 360.0f;
        angle = angle < 0.0f ? angle + rotation : angle - rotation;
    }
    return angle;
}
float clamped(float value, float lower, float upper) noexcept {
    // MAXSS/MINSS choose their second input on unordered comparisons.
    value = value > lower ? value : lower;
    return value < upper ? value : upper;
}
bool changed(float proposed, float previous) noexcept {
    return proposed != previous && !std::isnan(proposed);
}
}

ScriptCommandBaseline capture_script_command(ScriptCommand& state, const Command& command,
                                             float move_yaw, std::uint8_t send_packet,
                                             std::int32_t choked_commands,
                                             std::uint32_t flags) noexcept {
    state.choked_commands = choked_commands;
    state.command_number = command.number;
    state.angles = command.angles;
    state.forward_move = command.movement.forward_move;
    state.side_move = command.movement.side_move;
    state.send_packet = send_packet != 0;
    state.force_defensive = 0;
    state.buttons = command.movement.buttons;
    state.weapon_select = command.weapon_select;
    state.weapon_subtype = command.weapon_subtype;
    state.move_yaw = move_yaw;
    state.exploit_state = (flags & 0x600) != 0;
    state.state_bit_3 = (flags >> 3) & 1;
    state.state_bit_18 = (flags >> 18) & 1;
    state.state_bit_20 = 0;
    return {state.send_packet, state.exploit_state, state.state_bit_18};
}

void apply_script_command(ScriptCommand& state, Command& command, float& move_yaw,
                          std::uint8_t& send_packet, std::uint32_t& flags,
                          ScriptCommandBaseline baseline, bool unrestricted_angles,
                          bool block_roll) noexcept {
    if (changed(state.angles[0], command.angles[0])) {
        if (!unrestricted_angles)
            state.angles[0] = clamped(wrapped(state.angles[0]), -89.0f, 89.0f);
        command.angles[0] = state.angles[0];
    }
    if (changed(state.angles[1], command.angles[1])) {
        if (!unrestricted_angles)
            state.angles[1] = wrapped(state.angles[1]);
        command.angles[1] = state.angles[1];
    }
    if (!block_roll && changed(state.angles[2], command.angles[2])) {
        if (!unrestricted_angles)
            state.angles[2] = clamped(wrapped(state.angles[2]), -50.0f, 50.0f);
        command.angles[2] = state.angles[2];
    }
    if (changed(state.forward_move, command.movement.forward_move)) {
        if (!unrestricted_angles)
            state.forward_move = clamped(state.forward_move, -450.0f, 450.0f);
        command.movement.forward_move = state.forward_move;
    }
    if (changed(state.side_move, command.movement.side_move)) {
        if (!unrestricted_angles)
            state.side_move = clamped(state.side_move, -450.0f, 450.0f);
        command.movement.side_move = state.side_move;
    }
    if (state.send_packet != baseline.send_packet)
        send_packet = state.send_packet != 0;
    command.movement.buttons = state.buttons;
    command.weapon_select = state.weapon_select;
    command.weapon_subtype = state.weapon_subtype;
    if (changed(state.move_yaw, move_yaw))
        move_yaw = wrapped(state.move_yaw);
    if (state.force_defensive != 0)
        flags |= 0x80000;
    if (state.exploit_state != baseline.exploit_state)
        flags ^= ((std::uint32_t(state.exploit_state) << 9) ^ flags) & 0x600;
    if (state.state_bit_18 != baseline.state_bit_18)
        flags ^= ((std::uint32_t(state.state_bit_18) << 18) ^ flags) & 0x40000;
    if (state.state_bit_20 != 0)
        flags |= 0x100000;
}

} // namespace skeet::client

// ============================================================================
// Subcomponent: command_angle_retention.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* [stripped helper] */
float attack_angle_difference(float current, float previous) noexcept {
    const float difference = current - previous;
    float result = static_cast<float>(std::fmod(static_cast<double>(difference), 360.0));
    if (current > previous) {
        if (result >= 180.0f)
            result -= 360.0f;
    } else if (result <= -180.0f)
        result += 360.0f;
    return result;
}
}

void retain_command_attack_angles(const CommandFrame& frame, Command& command, const void* player,
                                  const CommandMouseOffsets& offsets, const void* input) noexcept {
    if (!(frame.flags & 0x40U))
        return;
    if (field<std::uint32_t>(player, offsets.player_flags) & 8U)
        return;
    if (field<std::uint8_t>(player, offsets.player_state_043fa575) !=
        field<std::uint8_t>(player, offsets.player_state_334b460c))
        return;
    if (frame.weapon_id_d8 == 49)
        return;

    const auto buttons = command.movement.buttons;
    std::uint32_t retained_buttons;
    if (buttons & 1U)
        retained_buttons = buttons & ~1U;
    else {
        if (!(buttons & 0x800U) || frame.weapon_id_d8 != 64)
            return;
        retained_buttons = buttons & ~0x800U;
    }
    const auto previous_number = std::uint32_t(command.number) - 1U;
    const auto* commands = field<const Command*>(input, 0xF0);
    const auto& previous = commands[previous_number % 150U];
    if (std::bit_cast<std::uint32_t>(previous.number) != previous_number)
        return;

    const float pitch = attack_angle_difference(command.angles[0], previous.angles[0]);
    const float yaw = attack_angle_difference(command.angles[1], previous.angles[1]);
    const float yaw_square = yaw * yaw;
    const float pitch_square = pitch * pitch;
    const float distance = std::sqrt(pitch_square + yaw_square);
    const float maximum_step = frame.maximum_angle_step_1e8;
    if (!(distance >= maximum_step))
        return;
    const float scale = maximum_step / distance;
    const float scaled_yaw = scale * yaw;
    const float scaled_pitch = scale * pitch;
    command.angles[0] = scaled_pitch + previous.angles[0];
    const float limited_yaw = scaled_yaw + previous.angles[1];
    command.angles[2] = 0.0f;
    command.movement.buttons = retained_buttons;
    command.angles[1] = limited_yaw;
}

bool can_predict_command_copy(const Command& command, const void* variable) noexcept {
    if (variable) {
        for (;;) {
            const auto* parent = field<const void*>(variable, 0x1C);
            if (parent == variable)
                break;
            variable = parent;
        }
        const auto word = field<std::uint32_t>(variable, 0x30) ^
                          static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(variable));
        if (word & 0x80000000U)
            return false;
    }
    return std::bit_cast<std::uint32_t>(command.tick) != 0x7F7FFFFFU;
}

CommandCopyOverrideScope::CommandCopyOverrideScope(std::uint8_t* input) noexcept
    : target(input), saved(0) {
    if (target) {
        saved = *target;
        *target = 1;
    }
}
void CommandCopyOverrideScope::restore() noexcept {
    if (target) {
        *target = saved;
        target = nullptr;
    }
}

void prepare_prediction_command_copy(const CommandFrame& frame, const Command& command,
                                     Command& copy, std::uint8_t send_packet,
                                     const CommandButtonPolicy& policy,
                                     const CommandTrigonometry& trigonometry,
                                     const ScriptCommandEnvironment& environment) {
    const auto original_angles = frame.angles_118;
    const auto movement_type = frame.state_104;
    std::memcpy(&copy, &command, sizeof copy);
    reorient_command_movement(copy, original_angles, movement_type, true, send_packet == 0, policy,
                              trigonometry);
    sanitize_command(copy, (frame.flags & 8U) != 0, environment);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: command_capture.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */
/* deduplicated helper */
/* deduplicated asm float math helpers */
std::uint32_t address(const void* value) noexcept {
    return std::uint32_t(reinterpret_cast<std::uintptr_t>(value));
}
void* terminal_convar(void* object) noexcept {
    for (;;) {
        auto* parent = read<void*>(object, 0x1C);
        if (object == parent)
            break;
        object = parent;
    }
    return object;
}
std::uint32_t integer_convar(void* object) noexcept {
    object = terminal_convar(object);
    return read<std::uint32_t>(object, 0x30) ^ address(object);
}
// Complete433E3CFB. Mode0 additionally requires the secondary ConVar to be0;
// modes1 and6 bypass that secondary test.
bool randomized_angle_limit(const CommandCaptureContext& context) noexcept {
    if (*context.angle_override)
        return true;
    if (!command_roll_blocked(context.angle_environment))
        return false;
    const auto& c = context.convars;
    if (c.optional_angle_override.get() && integer_convar(c.optional_angle_override.get()))
        return true;
    const auto mode = integer_convar(c.angle_mode.get());
    const auto* secondary = terminal_convar(c.angle_secondary.get());
    return mode == 0 ? read<std::uint32_t>(secondary, 0x30) == address(secondary)
                     : (mode == 1 || mode == 6);
}
[[gnu::always_inline]] inline float
interpolation_time(const CommandCaptureContext& context) noexcept {
    const auto& c = context.convars;
    const auto rate =
        std::bit_cast<float>(lagcomp::read_prediction_convar(c.interpolation_rate.get()));
    const auto minimum =
        std::bit_cast<float>(lagcomp::read_prediction_convar(c.interpolation_minimum.get()));
    const auto ratio =
        std::bit_cast<float>(lagcomp::read_prediction_convar(c.interpolation_ratio.get()));
    return maximum_number(*context.interpolation_floor,
                          maximum_number(minimum, (ratio / rate)));
}
std::array<float, 3> view_forward(const std::array<float, 3>& angles,
                                  const RageTrigonometry& trig) {
    constexpr float radians = 0.01745329238474369f;
    const auto yaw = (angles[1] * radians);
    const auto sy = trig.sine(yaw), cy = trig.cosine(yaw);
    const auto pitch = (angles[0] * radians);
    const auto sp = trig.sine(pitch), cp = trig.cosine(pitch);
    return {(cp * cy), (cp * sy),
            std::bit_cast<float>(std::bit_cast<std::uint32_t>(sp) ^ 0x80000000u)};
}
}

bool command_target_valid(const CommandTargetValidation& context) {
    auto& state = context.target;
    if (*context.sequence == state.sequence)
        return true;
    float current = 0;
    if (context.clock_owner && *context.clock_owner)
        current = read<float>(*context.clock_owner, context.clock_offset);
    if (current != state.timestamp || state.handle == -1)
        return false;
    auto* target = engine::invoke<void*>(context.entity_list, 4, state.handle);
    if (!target || !engine::invoke<bool>(target, 158) ||
        read<std::uint8_t>(target, context.rejection_byte))
        return false;
    std::array<float, 3> origin;
    if (!engine::invoke<bool>(static_cast<std::byte*>(target) + 8, 9))
        origin = read<std::array<float, 3>>(target, context.network_origin);
    else {
        if (context.dormant_owner && *context.dormant_owner) {
            const auto index = state.handle == -1 ? 0x1FFFu : std::uint16_t(state.handle);
            auto* owner = *context.dormant_owner;
            if (!read<std::uint8_t>(owner, context.dormant_present + index) ||
                !read<std::uint8_t>(owner, context.dormant_valid + index))
                return false;
        }
        const auto index = state.handle == -1 ? 0x1FFFu : std::uint16_t(state.handle);
        const auto* cached = context.players.get(index);
        if (cached->geometry.alpha == 0.0f)
            return false;
        const auto& p = cached->geometry.origin;
        origin = {p.x, p.y, p.z};
    }
    const auto y = (origin[1] - state.position[1]);
    const auto x = (origin[0] - state.position[0]);
    const auto z = (origin[2] - state.position[2]);
    const auto square = ((y * y + x * x) + z * z);
    // COMISS/JBE also accepts unordered results in the native helper.
    return !(square > 122500.0f);
}

CommandCapturedState capture_initial_command_frame(CommandFrame& frame, Command& command,
                                                   std::uint8_t& send_packet,
                                                   std::uint32_t& global_flags, float sample_time,
                                                   const std::array<float, 3>& saved_angles,
                                                   std::vector<Matrix3x4>& bone_storage,
                                                   const CommandCaptureContext& context) {
    const auto& physical = context.physical;
    auto* player = physical.player;
    const auto& offsets = context.offsets;
    const auto& common = physical.offsets.common;
    frame.flags = frame.secondary_flags = 0;
    if (command_angles_unrestricted(context.angle_environment))
        frame.flags |= 8u;
    auto* const* rules = context.angle_environment.shared_player;
    if (rules && *rules && read<std::uint8_t>(*rules, offsets.rules_guard))
        frame.flags |= 0x200000u;
    if ((read<std::uint32_t>(player, offsets.mouse.player_flags) & 8u) ||
        read<std::uint8_t>(player, offsets.mouse.player_state_043fa575) !=
            read<std::uint8_t>(player, offsets.mouse.player_state_334b460c))
        frame.flags |= 0x200u;
    if (integer_convar(context.convars.target_selector.get()))
        frame.flags |= 0x4000u;
    if (hotkey_active(context.movement_key, context.input) || (global_flags & 0x100u)) {
        frame.flags |= 0x2000u;
        if (read<std::uint32_t>(player, common.player_flags) & 1u)
            frame.field_18c = std::uint32_t(frame.choke_limit_fc) - std::uint32_t(frame.choked_15c);
    }
    if (randomized_angle_limit(context)) {
        frame.flags |= 0x40u;
        const auto magnitude =
            command.number < 0 ? 0u - std::uint32_t(command.number) : std::uint32_t(command.number);
        network::ShuffleRandom random{std::bit_cast<std::int32_t>(0u - magnitude), 0, {}};
        const auto upper =
            _mm_cvtss_f32(_mm_cvtepi32_ps(_mm_cvtsi32_si128(*context.maximum_angle)));
        const auto value = network::next_shuffle_random(random);
        // Native converts the scaled double to float before its upper clamp.
        auto unit = static_cast<float>(static_cast<double>(value) * 4.656612875245797e-10);
        if (static_cast<double>(unit) > 0.99999988)
            unit = 0.9999998807907104f;
        frame.maximum_angle_step_1e8 = (upper - (unit + 0.0f));
    }
    const auto latency = network::average_latency_ticks(context.latency);
    write(&frame, 0x14C, latency);
    const auto& timing = context.timing;
    const auto tick = network::adjusted_command_tick(
        timing.base_tick, timing.have_ack, timing.acknowledged, timing.have_sent, timing.sent);
    const auto command_tick = std::uint32_t(latency) + std::uint32_t(tick);
    frame.field_d4 = command_tick;
    write(&frame, 0x144, command_tick + integer_convar(context.convars.command_tick_offset.get()));
    write(&frame, 0x198, interpolation_time(context));
    engine::invoke<void>(context.engine_client, 18,
                         static_cast<void*>(reinterpret_cast<std::byte*>(&frame) + 0xB4));
    write(&frame, 0x1A8,
          view_forward(read<std::array<float, 3>>(&frame, 0xB4), physical.trigonometry));
    frame.command_number_ac = command.number;
    frame.angles_118 = command.angles;
    frame.input_side_80 = command.movement.side_move;
    frame.input_forward_78 = command.movement.forward_move;
    write(&frame, 0xF8, read<std::uint32_t>(player, common.player_flags));
    const auto tick_base = read<std::int32_t>(player, offsets.tick_base);
    write(&frame, 0x88, tick_base);
    frame.tick_base_c8 = tick_base;
    const auto duck = read<float>(player, common.duck_amount);
    write(&frame, 0x1A4, duck);
    frame.duck_amount_98 = duck;
    write(&frame, 0x8C, read<std::array<float, 3>>(player, offsets.vector_48442721));
    const auto vy = read<float>(player, common.velocity + 4),
               vx = read<float>(player, common.velocity);
    const auto planar_speed =
        _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(add(multiply(vx, vx), multiply(vy, vy)))));
    frame.previous_speed_190 = planar_speed;
    frame.origin_168 = *engine::invoke<const std::array<float, 3>*>(player, 10);
    frame.previous_velocity_180 = read<std::array<float, 3>>(player, common.velocity);
    const auto handle = read<std::uint32_t>(player, physical.offsets.active_weapon);
    auto* weapon =
        handle == UINT32_MAX ? nullptr : engine::invoke<void*>(physical.entity_list, 4, handle);
    frame.weapon_address = address(weapon);
    if (weapon) {
        write(&frame, 0x194, read<float>(weapon, common.weapon_field_6777c9ce));
        write(&frame, 0xF4, read<float>(weapon, offsets.weapon_field_d2563274));
    }
    frame.flags &= ~0x400000u;
    write(&frame, 0x1B4, sample_time);
    capture_command_physical_frame(frame, command, physical, CommandCapturePass::initial);
    frame.field_108 = frame.field_c4;
    frame.field_1ec = frame.field_1f0 = 0;
    frame.field_1d0 = read<std::uint32_t>(player, offsets.team);

    auto* cache = static_cast<std::byte*>(player) + common.bone_cache;
    const auto count = read<std::int32_t>(cache, 0xC);
    if (bone_storage.size() != std::size_t(count))
        bone_storage.resize(std::size_t(count));
    if (!bone_storage.empty())
        std::memcpy(bone_storage.data(), read<const Matrix3x4*>(cache, 0),
                    bone_storage.size() * sizeof(Matrix3x4));
    auto& target = context.target_validation.target;
    if (target.handle != -1 && engine::invoke<void*>(physical.entity_list, 4, target.handle) &&
        !command_target_valid(context.target_validation))
        reset_target_state(
            {target.handle, target.fields_18[0], target.fields_18[1], target.fields_18[3]});
    frame.angles_118 = saved_angles;
    enforce_command_choke_limit(send_packet, frame.choked_15c, frame.choke_limit_fc);
    return {{bone_storage.data(), count}, planar_speed, frame.scalar_c0};
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: command_capture_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
std::ptrdiff_t offset(ClientRuntimeStorage& storage, std::uint32_t identity) {
    const auto& globals = storage.offsets().globals;
    for (const auto& row : engine::build_receive_offset_recipes())
        if (row.pointer_identity == identity)
            return engine::DecodedOffset(globals, identity, row.xor_key_identity,
                                         row.initial_xor_key, 0);
    for (const auto& row : engine::build_data_map_offset_recipes())
        if (row.pointer_identity == identity)
            return engine::DecodedOffset(globals, identity, row.xor_key_identity,
                                         row.initial_xor_key, 0);
    throw std::invalid_argument("Command capture field has no recovered receive/datamap recipe");
}
template <class T> T& pointed(engine::NativeDataImage& data, std::uint32_t identity) {
    auto* const object = data.native_cell<T*>(identity);
    if (!object)
        throw std::invalid_argument(
            "Command capture requires an initialized canonical engine alias");
    return *object;
}
CommandMouseOffsets mouse_fields(ClientRuntimeStorage& storage) {
    //043FA575 is the separately published +1 derivative of334B460C.
    return {
        offset(storage, 0x43468B50u),
        engine::DecodedOffset(storage.offsets().globals, 0x4346757Cu, 0x4346707Cu, 0x043FA575u, 0),
        offset(storage, 0x4346934Cu)};
}
CommandPlayerGates player_fields(ClientRuntimeStorage& storage) {
    return {{offset(storage, 0x43468E78u), offset(storage, 0x4346A5DCu)},
            offset(storage, 0x4346A6CCu),
            offset(storage, 0x43468BD4u),
            offset(storage, 0x434688D0u),
            offset(storage, 0x434693D8u),
            offset(storage, 0x43467A74u)};
}
weapon::FireStateOffsets fire_fields(ClientRuntimeStorage& storage) {
    //These two weapon datamap results are published separately at4348D65D,
    //before the ordinary derived-offset list in bindings_offset_storage.cpp.
    return {
        offset(storage, 0x4346908Cu),
        offset(storage, 0x4346A768u),
        offset(storage, 0x43467D40u),
        engine::DecodedOffset(storage.offsets().globals, 0x43468D68u, 0x43467558u, 0x836CB5DBu, 0),
        offset(storage, 0x4346799Cu),
        offset(storage, 0x434688B0u),
        offset(storage, 0x4346A7E0u),
        offset(storage, 0x4346A60Cu),
        offset(storage, 0x4346A80Cu),
        offset(storage, 0x43467A50u),
        offset(storage, 0x4346A6DCu),
        engine::DecodedOffset(storage.offsets().globals, 0x4346828Cu, 0x43468D2Cu, 0x9E2078A1u, 0),
        offset(storage, 0x43468FD8u),
        offset(storage, 0x434670A4u),
        offset(storage, 0x43467910u)};
}
CommandRecaptureOffsets recapture_fields(ClientRuntimeStorage& storage,
                                         const lagcomp::EntityStateRuntime& history) {
    return {history.offsets,
            offset(storage, 0x43467A74u),
            offset(storage, 0x43468BD4u),
            offset(storage, 0x43468A34u),
            offset(storage, 0x434682B8u),
            offset(storage, 0x43468354u),
            offset(storage, 0x4346A390u),
            offset(storage, 0x4346796Cu)};
}
CommandCaptureOffsets capture_fields(ClientRuntimeStorage& storage,
                                     const CommandMouseOffsets& mouse) {
    return {mouse,
            offset(storage, 0x4346746Cu),
            offset(storage, 0x434693D8u),
            offset(storage, 0x43467F74u),
            offset(storage, 0x4346A620u),
            offset(storage, 0x434688B0u)};
}
CommandEyeAdjustmentOffsets eye_fields(ClientRuntimeStorage& storage,
                                       const lagcomp::EntityStateRuntime& history) {
    return {history.offsets,
            offset(storage, 0x43468A34u),
            offset(storage, 0x4346A808u),
            offset(storage, 0x434688D0u),
            offset(storage, 0x4346908Cu),
            offset(storage, 0x43467E8Cu),
            offset(storage, 0x43468240u),
            offset(storage, 0x4346A390u),
            offset(storage, 0x434670CCu)};
}
ClientRuntimeStorage&
require_shared(ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
               ConfigurationRuntimeOwner& configuration, PlayerRuntimeOwner& players,
               animation::RuntimeOwner& animation, const lagcomp::EntityStateRuntime& history) {
    auto& data = storage.data();
    const auto* command_clock = data.native_cell<const std::byte*>(0x43475A60u);
    if (&engine.variables.data() != &data || &configuration.storage() != &storage ||
        &players.storage() != &storage || &animation.storage() != &storage ||
        &animation.players() != &players.cache() ||
        history.engine != data.native_cell<void*>(0x4346A514u) ||
        history.entity_list != data.native_cell<void*>(0x4346A890u) ||
        &history.interval != &pointed<float>(data, 0x4346A338u) ||
        &history.receive_interval != &pointed<float>(data, 0x4346A338u) ||
        &history.frame != &pointed<std::int32_t>(data, 0x434678F8u) || !command_clock ||
        &history.command_tick != reinterpret_cast<const std::int32_t*>(command_clock + 4) ||
        &history.previous_command != &data.native_cell<const std::int32_t*>(0x43468F9Cu) ||
        &history.current_command != &data.native_cell<const std::int32_t*>(0x434670A8u))
        throw std::invalid_argument(
            "Command capture requires the shared canonical configuration, player, animation and history owners");
    const auto core = engine.interfaces.core();
    auto* const globals = data.native_cell<std::byte*>(0x43467B04u);
    if (!globals || core.engine_client != data.native_cell<void*>(0x4346A514u) ||
        core.entity_list != data.native_cell<void*>(0x4346A890u) ||
        core.prediction != data.native_cell<void*>(0x43469264u) || !core.engine_client ||
        !core.entity_list || !core.prediction ||
        reinterpret_cast<const std::byte*>(data.native_cell<float*>(0x4346A338u)) !=
            globals + 0x20 ||
        reinterpret_cast<const std::byte*>(data.native_cell<float*>(0x43467EB8u)) !=
            globals + 0x10 ||
        !data.native_cell<void*>(0x4346A894u) || !data.native_cell<void**>(0x4346A3A4u) ||
        !data.native_cell<void**>(0x43468FF0u) || !data.native_cell<void*>(0x43475A60u))
        throw std::invalid_argument(
            "Command capture requires the initialized engine, CInput, global clock and client-state aliases");
    (void)pointed<std::int32_t>(data, 0x434757DCu);
    (void)pointed<std::int32_t>(data, 0x43475B2Cu);
    for (const auto cell : {0x43468EA4u, 0x43467A8Cu, 0x4346A42Cu, 0x43468528u, 0x4346747Cu,
                            0x4346A748u, 0x4346A330u, 0x4346794Cu})
        if (!engine.variables.at(cell).get())
            throw std::invalid_argument("Command capture requires its actual acquired ConVars");
    return storage;
}
}

struct CommandCaptureRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    ConfigurationRuntimeOwner& configuration;
    PlayerRuntimeOwner& players;
    animation::RuntimeOwner& animation;
    const lagcomp::EntityStateRuntime& history;
    CommandMasterGlobals globals;
    CommandPlayerGates gates;
    CommandMouseOffsets mouse;
    ScriptCommandEnvironment environment;
    CommandEntityBindings entities;
    CommandAngleBindings view;
    CommandPacketCatchupBindings catchup;
    CommandBudgetBindings budget;
    weapon::FireStateBindings fire;
    CommandRecaptureOffsets physical_fields;
    CommandCaptureOffsets capture_fields;
    CommandCaptureConvars convars;
    engine::NativeConVarReference recoil, copy_gate;
    CommandTargetValidation validation;
    CommandSentTimingBindings sent;
    SentCommandHistoryView history_view;
    engine::EyePoseParameterRuntime pose;
    CommandEyeAdjustmentSourceContext eye;
    std::vector<Matrix3x4> bones;

    Data(ClientRuntimeStorage& source, const EngineRuntimeBindings& engine,
         ConfigurationRuntimeOwner& config, PlayerRuntimeOwner& player_owner,
         animation::RuntimeOwner& animation_owner,
         const lagcomp::EntityStateRuntime& history_runtime)
        : storage(require_shared(source, engine, config, player_owner, animation_owner,
                                 history_runtime)),
          configuration(config), players(player_owner), animation(animation_owner),
          history(history_runtime), globals{source.data().native_cell<void*>(0x4346A514u),
                                            source.data().native_cell<void*>(0x4346A890u),
                                            source.data().native_cell<void*>(0x43469264u),
                                            source.data().native_cell<void*>(0x43467B04u),
                                            source.data().native_cell<float*>(0x4346A338u),
                                            source.data().native_cell<float*>(0x43467EB8u),
                                            source.data().native_cell<std::int32_t*>(0x434757DCu),
                                            source.data().native_cell<std::int32_t*>(0x43475B2Cu),
                                            source.data().native_cell<std::uint32_t>(0x434759C0u),
                                            source.data().native_cell<std::int32_t>(0x434759F8u)},
          gates(player_fields(source)), mouse(mouse_fields(source)),
          environment{&config.anti_untrusted().enabled.value,
                      source.data().native_cell<void**>(0x4346A3A4u), offset(source, 0x43468378u)},
          entities{source.data().native_cell<void*>(0x4346A890u), offset(source, 0x4346A808u),
                   source.data().native_cell<std::uint32_t*>(0x43469060u)},
          view{source.data().native_cell<void*>(0x4346A514u),
               source.data().native_cell<void*>(0x43469264u), environment},
          catchup{source.data().native_cell<void*>(0x4346A514u),
                  source.data().native_cell<void*>(0x43468E44u),
                  pointed<float>(source.data(), 0x4346A338u)},
          budget{source.data().native_cell<void**>(0x4346A3A4u), offset(source, 0x43468378u),
                 offset(source, 0x4346836Cu), offset(source, 0x43468BF4u),
                 source.data().native_cell<std::int32_t>(0x43469274u)},
          fire{source.data().native_cell<void**>(0x4346A3A4u),
               &pointed<float>(source.data(), 0x43467EB8u), fire_fields(source)},
          physical_fields(recapture_fields(source, history_runtime)),
          capture_fields(client::capture_fields(source, mouse)),
          convars{engine.variables.at(0x43468EA4u), engine.variables.at(0x43468254u),
                  engine.variables.at(0x43467A8Cu), engine.variables.at(0x4346A42Cu),
                  engine.variables.at(0x43468528u), engine.variables.at(0x4346747Cu),
                  engine.variables.at(0x4346A748u), engine.variables.at(0x4346A330u)},
          recoil(engine.variables.at(0x4346794Cu)), copy_gate(engine.variables.at(0x43468E94u)),
          validation{source.data().native_cell<anti_aim::TargetState>(0x4346DB14u),
                     player_owner.cache(),
                     source.data().native_cell<void*>(0x4346A890u),
                     &pointed<std::int32_t>(source.data(), 0x434678F8u),
                     source.data().native_cell<void**>(0x4346A3A4u),
                     offset(source, 0x43468D6Cu),
                     offset(source, 0x43468E78u),
                     offset(source, 0x434670CCu),
                     source.data().native_cell<void**>(0x43468FF0u),
                     offset(source, 0x43467CB8u),
                     offset(source, 0x4346A724u)},
          sent{source.data().native_cell<void*>(0x43475A60u),
               source.data().native_cell<std::int32_t*>(0x43468F9Cu),
               source.data().native_cell<std::int32_t*>(0x434670A8u)},
          history_view{source.data().native_cell<std::array<float, 3>>(0x43476414u),
                       source.data().native_cell<std::array<SentCommandRecord, 150>>(0x43476450u)},
          pose(source.data()),
          eye{eye_fields(source, history_runtime),
              &config.rage_settings().global.enabled.value,
              &config.anti_aim_global().enabled.value,
              reinterpret_cast<const std::uint32_t*>(&config.anti_aim_global().pitch.value),
              &source.data().native_cell<std::int32_t>(0x434759CCu),
              &source.data().native_cell<std::array<float, 3>>(0x434759D0u),
              source.data().native_cell<const std::uint32_t*>(0x43469060u),
              source.data().native_cell<std::array<std::uint32_t, 3>>(0x43474580u),
              animation_owner.bones(),
              animation_owner.trigonometry(),
              animation::native_sse_atan2,
              pose} {}
};

CommandCaptureRuntimeOwner::CommandCaptureRuntimeOwner(ClientRuntimeStorage& storage,
                                                       const EngineRuntimeBindings& engine,
                                                       ConfigurationRuntimeOwner& configuration,
                                                       PlayerRuntimeOwner& players,
                                                       animation::RuntimeOwner& animation,
                                                       const lagcomp::EntityStateRuntime& history)
    : data_(std::make_unique<Data>(storage, engine, configuration, players, animation, history)) {}
CommandCaptureRuntimeOwner::~CommandCaptureRuntimeOwner() = default;
ClientRuntimeStorage& CommandCaptureRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
const CommandMasterGlobals& CommandCaptureRuntimeOwner::globals() const noexcept {
    return data_->globals;
}
const CommandPlayerGates& CommandCaptureRuntimeOwner::player_gates() const noexcept {
    return data_->gates;
}
const CommandMouseOffsets& CommandCaptureRuntimeOwner::mouse_offsets() const noexcept {
    return data_->mouse;
}
const CommandRecaptureOffsets& CommandCaptureRuntimeOwner::physical_offsets() const noexcept {
    return data_->physical_fields;
}
const CommandCaptureOffsets& CommandCaptureRuntimeOwner::capture_offsets() const noexcept {
    return data_->capture_fields;
}
const weapon::FireStateBindings& CommandCaptureRuntimeOwner::firing() const noexcept {
    return data_->fire;
}
const CommandEntityBindings& CommandCaptureRuntimeOwner::entities() const noexcept {
    return data_->entities;
}
const ScriptCommandEnvironment& CommandCaptureRuntimeOwner::angle_environment() const noexcept {
    return data_->environment;
}
const CommandAngleBindings& CommandCaptureRuntimeOwner::view_angles() const noexcept {
    return data_->view;
}
const CommandPacketCatchupBindings& CommandCaptureRuntimeOwner::packet_catchup() const noexcept {
    return data_->catchup;
}
const CommandBudgetBindings& CommandCaptureRuntimeOwner::budget() const noexcept {
    return data_->budget;
}
const CommandTargetValidation& CommandCaptureRuntimeOwner::target_validation() const noexcept {
    return data_->validation;
}
const CommandSentTimingBindings& CommandCaptureRuntimeOwner::sent_timing_bindings() const noexcept {
    return data_->sent;
}
const CommandEyeAdjustmentSourceContext&
CommandCaptureRuntimeOwner::eye_adjustment() const noexcept {
    return data_->eye;
}
engine::NativeConVarReference CommandCaptureRuntimeOwner::copy_gate() const noexcept {
    return data_->copy_gate;
}
std::vector<Matrix3x4>& CommandCaptureRuntimeOwner::bone_storage() noexcept {
    return data_->bones;
}
SentCommandHistoryView& CommandCaptureRuntimeOwner::sent_history() noexcept {
    return data_->history_view;
}
CommandRecaptureContext CommandCaptureRuntimeOwner::recapture(void* player) const {
    if (!player)
        throw std::invalid_argument("Command physical capture requires the actual accepted player");
    auto& state = *data_;
    return {player,
            state.globals.entity_list,
            state.physical_fields,
            state.fire,
            state.recoil,
            &state.storage.data().native_cell<std::uint8_t>(0x434693D4u),
            state.animation.trigonometry(),
            state.storage.data().native_cell<std::array<float, 3>>(0x43475BD4u)};
}
CommandCaptureContext CommandCaptureRuntimeOwner::capture(const CommandRecaptureContext& physical,
                                                          const SentCommandTiming& timing) const {
    auto& state = *data_;
    if (!physical.player || &physical.fire != &state.fire ||
        &physical.trigonometry != &state.animation.trigonometry() ||
        physical.entity_list != state.globals.entity_list ||
        &physical.published_origin !=
            &state.storage.data().native_cell<std::array<float, 3>>(0x43475BD4u))
        throw std::invalid_argument(
            "Command capture requires this owner's current canonical physical view");
    const auto& settings = state.configuration.rage_settings().global;
    return {physical,
            state.environment,
            state.capture_fields,
            state.convars,
            state.globals.engine_client,
            settings.duck_peek_assist.value.binding,
            state.storage.input_runtime().bitmap(),
            &settings.reduce_aim_step.value,
            &settings.aim_step_limit.value,
            &state.storage.data().native_cell<float>(0x43469254u),
            state.storage.data().native_cell<network::LatencySamples>(0x43478C70u),
            timing,
            state.validation};
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: command_checksum.cpp
// ============================================================================

namespace skeet::client {
namespace {
constexpr auto crc_table() noexcept {
    std::array<std::uint32_t, 256> table{};
    for (std::uint32_t i = 0; i < table.size(); ++i) {
        auto value = i;
        for (unsigned bit = 0; bit < 8; ++bit)
            value = (value >> 1) ^ ((value & 1u) ? 0xEDB88320u : 0u);
        table[i] = value;
    }
    return table;
}
constexpr auto table = crc_table();
struct Field {
    std::uint32_t offset, size;
};
// Actual successive EDI lengths / LEA ESI,[EBX+offset] in307260..308465.
constexpr std::array fields{Field{4, 4},    Field{8, 4},    Field{0xC, 0xC}, Field{0x18, 0xC},
                            Field{0x24, 4}, Field{0x28, 4}, Field{0x2C, 4},  Field{0x30, 4},
                            Field{0x34, 1}, Field{0x38, 4}, Field{0x3C, 4},  Field{0x40, 4},
                            Field{0x44, 2}, Field{0x46, 2}};
#if defined(_M_IX86) || defined(__i386__)
std::uint32_t __thiscall checksum_entry(void* command) noexcept {
#else
std::uint32_t checksum_entry(void* command) noexcept {
#endif
    return source_command_checksum(*static_cast<const Command*>(command));
}
}
std::uint32_t source_command_checksum(const Command& command) noexcept {
    const auto* bytes = reinterpret_cast<const unsigned char*>(&command);
    std::uint32_t crc = UINT32_MAX;
    for (const auto field : fields)
        for (std::uint32_t i = 0; i < field.size; ++i)
            crc = (crc >> 8) ^ table[(crc ^ bytes[field.offset + i]) & 255u];
    return ~crc;
}
CommandChecksum source_command_checksum_entry() noexcept {
    return checksum_entry;
}
void initialize_source_command_checksum(engine::NativeDataImage& data) {
    auto& cell = data.native_cell<CommandChecksum>(0x43467C48u);
    if (cell && cell != checksum_entry)
        throw std::invalid_argument("Command checksum has a conflicting publication");
    cell = checksum_entry;
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: command_empty_tick.cpp
// ============================================================================

#if defined(_M_IX86) || defined(__i386__)
#define EMPTY_TICK_ENTRY __vectorcall
#else
#define EMPTY_TICK_ENTRY
#endif

namespace skeet::client {
namespace {
void EMPTY_TICK_ENTRY prediction_entry(NativeCommandEmptyTickReceiver* receiver, void* player,
                                       float, Command*, std::uint8_t*) {
    apply_empty_prediction_history(*static_cast<PredictionHistoryOwner*>(receiver->source), player);
}
void EMPTY_TICK_ENTRY legit_entry(NativeCommandEmptyTickReceiver* receiver, void*,
                                  float sample_time, Command* command, std::uint8_t*) {
    auto& owner = *static_cast<LegitCommandOwner*>(receiver->source);
    owner.pending(*command, sample_time);
}
void EMPTY_TICK_ENTRY timer_entry(NativeCommandEmptyTickReceiver* receiver, void*,
                                  float sample_time, Command*, std::uint8_t*) {
    auto& timer = *static_cast<CommandEmptyTickTimer*>(receiver->source);
    timer.remaining = maximum_number(timer.remaining - sample_time, 0.0f);
}
void EMPTY_TICK_ENTRY grenade_entry(NativeCommandEmptyTickReceiver* receiver, void* player, float,
                                    Command* command, std::uint8_t*) {
    static_cast<misc::GrenadeCommandRuntime*>(receiver->source)
        ->observer0(player, *command, *receiver->input);
}
void EMPTY_TICK_ENTRY camera_entry(NativeCommandEmptyTickReceiver* receiver, void* player, float,
                                   Command*, std::uint8_t*) {
    static_cast<effects::EffectsCommandOwner*>(receiver->source)->camera.update_observer(player);
}
const NativeCommandEmptyTickVtable empty_tick_prediction_table{prediction_entry}; //43470470
const NativeCommandEmptyTickVtable legit_table{legit_entry}; //434704B4
const NativeCommandEmptyTickVtable timer_table{timer_entry}; //4346F3B0
const NativeCommandEmptyTickVtable empty_tick_grenade_table{grenade_entry}; //434704BC
const NativeCommandEmptyTickVtable camera_table{camera_entry}; //4347270C
NativeCommandEmptyTickReceiver& empty_tick_source_receiver(void* object) {
    auto* receiver = static_cast<NativeCommandEmptyTickReceiver*>(object);
    if (!receiver || !receiver->source ||
        (receiver->vtable != &empty_tick_prediction_table && receiver->vtable != &legit_table &&
         receiver->vtable != &timer_table && receiver->vtable != &empty_tick_grenade_table &&
         receiver->vtable != &camera_table))
        throw std::logic_error("Native empty-tick entry has no concrete source receiver binding");
    return *receiver;
}
}
CommandEmptyTickRegistry::CommandEmptyTickRegistry(engine::PreservedInitialization& initialization,
                                                   engine::JobMemory memory,
                                                   const InputBitmap& input)
    : CommandEmptyTickRegistry(initialization.image_owner(), memory, input) {}
CommandEmptyTickRegistry::CommandEmptyTickRegistry(std::shared_ptr<engine::NativeDataImage> image,
                                                   engine::JobMemory memory,
                                                   const InputBitmap& input)
    : image_(image),
      owners_(engine::require_native_data_image(image_).native_cell<engine::OwnerRegistry>(
          0x43475978u)),
      memory_(memory), input_(input) {
    if (!memory_.allocate || !memory_.free || !memory_.process_heap)
        throw std::invalid_argument(
            "Native empty-tick storage requires the canonical process heap");
}
void CommandEmptyTickRegistry::append_receiver(NativeCommandEmptyTickReceiver& receiver,
                                               void* source) {
    receiver.source = source;
    receiver.input = &input_;
    //433F10DB shares the -4 sentinel and433F1665/4334DC99 append/growth.
    //Final table publication belongs to the actual later constructor cut.
    engine::register_owner(owners_, &receiver, memory_);
}
void CommandEmptyTickRegistry::append(PredictionHistoryOwner& owner) {
    append_receiver(owner.empty_tick_receiver, &owner);
}
void CommandEmptyTickRegistry::append(LegitCommandOwner& owner) {
    append_receiver(owner.empty_tick_receiver, &owner);
}
void CommandEmptyTickRegistry::append(CommandEmptyTickTimer& owner) {
    append_receiver(owner.empty_tick_receiver, &owner);
}
void CommandEmptyTickRegistry::append_grenade(NativeCommandEmptyTickReceiver& receiver,
                                              misc::GrenadeCommandRuntime* owner) {
    append_receiver(receiver, owner);
}
void CommandEmptyTickRegistry::append(effects::EffectsCommandOwner& owner) {
    append_receiver(owner.empty_tick_receiver, &owner);
}
void CommandEmptyTickRegistry::publish(PredictionHistoryOwner& owner) const noexcept {
    owner.empty_tick_receiver.vtable = &empty_tick_prediction_table;
}
void CommandEmptyTickRegistry::publish(LegitCommandOwner& owner) const noexcept {
    owner.empty_tick_receiver.vtable = &legit_table;
}
void CommandEmptyTickRegistry::publish(CommandEmptyTickTimer& owner) const noexcept {
    owner.empty_tick_receiver.vtable = &timer_table;
}
void CommandEmptyTickRegistry::publish_grenade(
    NativeCommandEmptyTickReceiver& receiver) const noexcept {
    receiver.vtable = &empty_tick_grenade_table;
}
void CommandEmptyTickRegistry::publish(effects::EffectsCommandOwner& owner) const noexcept {
    owner.empty_tick_receiver.vtable = &camera_table;
}
void CommandEmptyTickRegistry::require_sources() const {
    for (auto* object : owners_.entries())
        (void)empty_tick_source_receiver(object);
}
void CommandEmptyTickRegistry::dispatch(void* player, float sample_time, Command& command,
                                        std::uint8_t& result) const {
    //433D2F59 loads end, then433D2F5F loads begin, once before any callback.
    auto** const end = owners_.end;
    auto** cursor = owners_.begin;
    for (; cursor != end; ++cursor) {
        auto& receiver = empty_tick_source_receiver(*cursor);
        receiver.vtable->invoke(&receiver, player, sample_time, &command, &result);
    }
}
void CommandEmptyTickRegistry::finalize_storage() const {
    engine::UnloadSystemFunctions memory;
    memory.free_heap = memory_.free;
    memory.process_heap = memory_.process_heap;
    engine::free_native_allocation(owners_.begin, memory); //43349ECB, no cell reset
}
void require_empty_tick_sources(const CommandEmptyTickRegistry& registry) {
    registry.require_sources();
}
void dispatch_empty_tick_command(const CommandEmptyTickRegistry& registry, void* player,
                                 float sample_time, Command& command, std::uint8_t& result) {
    registry.dispatch(player, sample_time, command, result);
}
} //namespace skeet::client

#undef EMPTY_TICK_ENTRY

// ============================================================================
// Subcomponent: command_eye_adjustment.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */
/* deduplicated helper */
float negate(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) ^ 0x80000000U);
}
bool recent_selected_command(std::int32_t current, std::int32_t previous) noexcept {
    const auto difference = std::uint32_t(current) - std::uint32_t(previous);
    const auto sign = (difference & 0x80000000U) ? UINT32_MAX : 0U;
    return std::bit_cast<std::int32_t>((difference ^ sign) - sign) < 10;
}
template <class Context>
float selected_point_pitch(const CommandFrame& frame, const Context& context) {
    const auto& selected = *context.selected_point;
    const float x = selected[0] - frame.origin_dc[0];
    const float y = selected[1] - frame.origin_dc[1];
    const float z = selected[2] - frame.origin_dc[2];
    const float x_square = x * x, y_square = y * y, z_square = z * z;
    const float inverse =
        1.0f / (std::sqrt((x_square + y_square) + z_square) + 0.00000011920928955078125f);
    const float normalized_x = inverse * x, normalized_y = inverse * y, normalized_z = inverse * z;
    float pitch;
    if (normalized_y == 0.0f && normalized_x == 0.0f)
        pitch = normalized_z > 0.0f ? -90.0f : 90.0f;
    else {
        const float horizontal =
            std::sqrt(normalized_x * normalized_x + normalized_y * normalized_y);
        pitch = static_cast<float>(context.atan2(static_cast<double>(negate(normalized_z)),
                                                 static_cast<double>(horizontal))) *
                57.2957763671875f;
    }
    return pitch - frame.recoil_angles_9c[0];
}
bool build_eye_bones(void* player, const CommandEyeAdjustmentContext& context) {
    // Complete 433F8EEA: the renderable's slot 2 supplies angles in EDX;
    // decoded A9EE6062 supplies the origin in XMM1/XMM2/XMM3.
    const auto angles = *engine::invoke<const RageVector*>(static_cast<std::byte*>(player) + 4, 2);
    const auto origin = read<RageVector>(player, context.offsets.bone_origin);
    auto* matrices = read<Matrix3x4*>(player, context.offsets.common.bone_cache);
    const animation::BoneRequest request{angles, origin, 0.0f, 0x300U, -1, 0U, true};
    return animation::build_bone_matrices(
        context.bone_state, context.bone_model, request,
        std::span<Matrix3x4>(matrices, context.bone_model.parents.size()), context.bone_engine,
        context.trigonometry);
}
bool build_eye_bones(void* player, const CommandEyeAdjustmentSourceContext& context) {
    const auto angles = *engine::invoke<const RageVector*>(static_cast<std::byte*>(player) + 4, 2);
    const auto origin = read<RageVector>(player, context.offsets.bone_origin);
    auto* matrices = read<Matrix3x4*>(player, context.offsets.common.bone_cache);
    const animation::BoneRequest request{angles, origin, 0.0f, 0x300U, -1, 0U, true};
    return context.bones.build_bone_matrices(player, request, matrices);
}
template <class Context>
bool correct_head_height(CommandFrame& frame, void* player, const Context& context) {
    const auto& offsets = context.offsets;
    const auto* wrapper = read<const void*>(player, offsets.studio_header);
    if (!wrapper)
        return false;
    const auto* header = read<const std::byte*>(wrapper, 0);
    if (!header)
        return false;
    const auto set_index = read<std::int32_t>(player, offsets.hitbox_set);
    const auto set_offset = read<std::int32_t>(header, 0xB0);
    const auto relative = std::uint32_t(set_index) * 12U + std::uint32_t(set_offset);
    const auto* set = header + std::bit_cast<std::int32_t>(relative);
    if (!set || !read<std::int32_t>(set, 4))
        return false;
    const auto* boxes = set + read<std::int32_t>(set, 8);
    if (!boxes)
        return false;
    const auto bone = read<std::int32_t>(boxes, 0);
    if (bone < 0)
        return false;
    const auto& common = offsets.common;
    if (bone < read<std::int32_t>(player, common.bone_cache + 0xC)) {
        const auto* matrices = read<const Matrix3x4*>(player, common.bone_cache);
        frame.origin_dc[2] = rage::correct_firing_eye_height(matrices[bone], frame.origin_dc[2]);
    }
    return true;
}
}

template <class Context>
void adjust_eye(CommandFrame& frame, void* player, const Context& context) {
    if (!*context.enabled && !*context.alternate_enabled)
        return;
    const auto& offsets = context.offsets;
    const auto& common = offsets.common;
    const auto saved_pose = read<std::uint32_t>(player, common.pose_parameters + 0x30);
    const auto origin = *engine::invoke<const std::array<float, 3>*>(player, 10);
    const auto view = read<std::array<float, 3>>(player, offsets.view_offset);
    frame.origin_dc = {origin[0] + view[0], origin[1] + view[1], origin[2] + view[2]};

    write(player, common.bone_accessor + 8, std::uint32_t(0));
    write(player, offsets.overridden_property, std::uint32_t(0));
    write(player, common.last_bone_setup_time, std::uint32_t(0xFF7FFFFF));
    write(player, common.field_0471e13c, std::uint32_t(0));
    const auto setup_flags = read<std::uint32_t>(player, common.effects_bb78dc43) & ~2U;
    write(player, common.effects_bb78dc43, setup_flags);
    if constexpr (std::is_same_v<Context, CommandEyeAdjustmentContext>) {
        // The standalone model mirrors these native writes. The production
        // BoneRuntime reads the same fields directly from the actual player.
        context.bone_state.readable_bones = 0;
        context.bone_state.setup_flags = setup_flags;
    }

    const auto* animation = read<const void*>(player, common.animation_state);
    const bool animation_ready =
        animation && rage::firing_animation_ready(
                         animation, read<std::uint8_t>(player, offsets.animation_guard));
    frame.flags &= ~0x400000U;
    if (frame.weapon_address && !(frame.flags & 0x40U) && *context.enabled &&
        *context.pitch_enabled && !(frame.flags & 0x200U)) {
        const auto* weapon = reinterpret_cast<const void*>(std::uintptr_t(frame.weapon_address));
        if (network::classified_packet_weapon(
                read<std::uint16_t>(weapon, offsets.weapon_identifier),
                context.weapon_classification)) {
            float pitch;
            if ((frame.flags & 8U) && animation_ready) {
                pitch = -90.0f;
                frame.flags |= 0x400000U;
            } else if (recent_selected_command(frame.command_number_ac, *context.selected_command))
                pitch = selected_point_pitch(frame, context);
            else
                pitch = std::bit_cast<float>(frame.field_b4);
            pitch = normalize_movement_angle(pitch) +
                    read<float>(player, offsets.player_pitch_adjustment);
            if (pitch > 180.0f)
                pitch -= 360.0f;
            pitch = _mm_cvtss_f32(
                _mm_min_ss(_mm_max_ss(_mm_set_ss(pitch), _mm_set_ss(-90.0f)), _mm_set_ss(90.0f)));
            const auto* wrapper = read<const void*>(player, offsets.studio_header);
            if (wrapper && read<const void*>(wrapper, 0)) {
                float normalized = pitch;
                if constexpr (std::is_same_v<Context, CommandEyeAdjustmentContext>)
                    context.convert_pose_parameter(wrapper, 12, pitch, normalized);
                else
                    context.pose.convert(wrapper, 12, pitch, normalized);
                write(player, common.pose_parameters + 0x30, normalized);
            }
        }
    }

    const bool built = build_eye_bones(player, context);
    bool update_height = true;
    if (built) {
        frame.flags |= 0x20U;
        write(player, common.bone_accessor + 8, std::uint32_t(0xFFF00));
        write(player, common.bone_accessor + 0xC, std::uint32_t(0xFFF00));
        if constexpr (std::is_same_v<Context, CommandEyeAdjustmentContext>)
            context.bone_state.readable_bones = context.bone_state.writable_bones = 0xFFF00U;
        if (context.replacement_property)
            write(player, offsets.overridden_property, *context.replacement_property);
        if (animation_ready)
            update_height = correct_head_height(frame, player, context);
    }
    // Native invalid studio/hitbox paths skip this frame write entirely.
    if (update_height)
        frame.view_height_84 = frame.origin_dc[2] - frame.origin_10c[2];
    write(player, common.pose_parameters + 0x30, saved_pose);
}
void adjust_command_eye(CommandFrame& frame, void* player, Command&,
                        const CommandEyeAdjustmentContext& context) {
    adjust_eye(frame, player, context);
}
void adjust_command_eye(CommandFrame& frame, void* player, Command&,
                        const CommandEyeAdjustmentSourceContext& context) {
    adjust_eye(frame, player, context);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: command_orchestrator.cpp
// ============================================================================

namespace skeet::client {
namespace {
CommandCreateMoveRuntime* create_move_runtime;
/* deduplicated helper */
}
bool command_player_rejected(const void* player, const CommandPlayerGates& gates) noexcept {
    return field<std::uint8_t>(player, gates.rejection_bytes[0]) != 0 ||
           field<std::uint8_t>(player, gates.rejection_bytes[1]) != 0;
}
bool command_player_restricted(const void* player, const CommandPlayerGates& gates) noexcept {
    if (field<std::int32_t>(player, gates.restricted_state) != 0)
        return true;
    const auto type = field<std::uint8_t>(player, gates.movement_type);
    return type == 10 || type == 8 ||
           (field<std::uint32_t>(player, gates.player_flags) & 0x40u) != 0;
}
void catch_up_command_packet(std::uint8_t& send, const std::uint8_t& packet_context,
                             const CommandPacketCatchupBindings& bindings) {
    if (!packet_context || send)
        return;
    auto* channel = engine::invoke<void*>(bindings.engine_client, 78);
    //Native captures the network-state pointer before calling the channel.
    const auto* state = bindings.network_state;
    if (!channel)
        return;
#if defined(_M_IX86) || defined(__i386__)
    //The native caller consumes ST0 without first rounding it to float32.
    const auto channel_time = engine::invoke<long double>(channel, 2);
#else
    const auto channel_time = static_cast<long double>(engine::invoke<float>(channel, 2));
#endif
    const auto difference =
        static_cast<double>(static_cast<long double>(field<double>(state, 0x108)) - channel_time);
    const float maximum = bindings.interval * 0.25f;
    if (difference > 0.0 && difference < static_cast<double>(maximum))
        send = 1;
}
std::int32_t query_command_budget(const CommandBudgetBindings& bindings) noexcept {
    if (bindings.game_rules && *bindings.game_rules) {
        const auto* rules = *bindings.game_rules;
        if (!field<std::uint8_t>(rules, bindings.rule_bf567619))
            return bindings.configured;
        if (field<std::uint8_t>(rules, bindings.rule_dda99ab5) &&
            field<std::int32_t>(rules, bindings.rule_b7ea8fda) == 1)
            return 3;
    }
    return 7;
}
void forward_original_command_angles(void* const& engine_client, void* const& prediction,
                                     Command& command, bool empty_tick) {
    engine::invoke<void>(engine_client, 19, &command.angles);
    if (empty_tick)
        engine::invoke<void>(prediction, 13, &command.angles);
}
void finalize_command_movement(CommandFrame& frame, Command& command, std::uint8_t& send,
                               const CommandButtonPolicy& policy,
                               const CommandTrigonometry& trigonometry,
                               const ScriptCommandEnvironment& environment,
                               std::int32_t last_outgoing, std::int32_t& last_shot) {
    frame.input_side_80 = command.movement.side_move;
    frame.input_forward_78 = command.movement.forward_move;
    reorient_command_movement(command, frame.angles_118, frame.state_104,
                              (frame.flags & 0x40000u) != 0, send == 0, policy, trigonometry);
    sanitize_command(command, (frame.flags & 8u) != 0, environment);
    enforce_command_choke_limit(send, frame.choked_15c, frame.choke_limit_fc);
    sanitize_final_command_angles(command);
    record_command_shot(frame.flags, command, last_outgoing, last_shot);
}
void finish_command_transaction(Command& command, std::uint8_t& engine_send,
                                std::uint8_t requested_packet,
                                const CommandPacketTransitionBindings& transition,
                                std::uint32_t& scheduler_flags,
                                const CommandEntityBindings& entities,
                                const CommandEntitySnapshot& saved_entities) {
    save_final_command(command);
    apply_command_packet_transition(requested_packet, engine_send, *(&engine_send + 1), transition);
    scheduler_flags &= ~0x20u;
    restore_command_entity_properties(entities, saved_entities);
}
namespace {
prediction::TimeTargets time_targets(const CommandMasterGlobals& globals) noexcept {
    return {reinterpret_cast<float*>(static_cast<std::byte*>(globals.engine_globals) + 0x14),
            globals.current_time, static_cast<std::uint8_t*>(globals.prediction) + 8};
}
std::int32_t player_tick_base(void* player, const CommandPlayerGates& offsets) noexcept {
    return field<std::int32_t>(player, offsets.tick_base);
}
bool prediction_paused(const CommandMasterGlobals& globals) noexcept {
    return field<std::uint8_t>(globals.prediction, 10) != 0;
}
void* command_weapon(void* player, const CommandAcceptedBindings& bindings) {
    const auto handle = field<std::uint32_t>(player, bindings.player_gates.active_weapon);
    return handle == UINT32_MAX ? nullptr
                                : engine::invoke<void*>(bindings.globals.entity_list, 4, handle);
}
SentCommandTiming sent_timing(const CommandAcceptedBindings& bindings) noexcept {
    const auto& timing = bindings.sent_timing;
    return {*bindings.globals.last_outgoing, field<std::int32_t>(timing.command_clock, 4),
            timing.acknowledged != nullptr,  timing.acknowledged ? *timing.acknowledged : 0,
            timing.sent != nullptr,          timing.sent ? *timing.sent : 0};
}
void refresh_sentinel_target(const CommandAcceptedBindings& bindings, void* player,
                             Command& command) {
    const auto& validation = bindings.capture.target_validation();
    auto& target = validation.target;
    //The outer lookup precedes the actual433ED5A8 validator's own lookup.
    if (target.handle != -1 &&
        engine::invoke<void*>(bindings.globals.entity_list, 4, target.handle) &&
        !command_target_valid(validation))
        anti_aim::clear_target_selection(target);
    bindings.sentinel_refresh.refresh(target, player, command, command.movement.buttons,
                                      command.angles[1]);
}
}
std::uint8_t run_accepted_command_pipeline(const CommandAcceptedBindings& bindings,
                                           CommandFrame& frame, void* player, float sample_time,
                                           Command& command, std::uint8_t& engine_send,
                                           std::uint8_t original_result) {
    bindings.order.require_sources();
    const auto& globals = bindings.globals;
    globals.scheduler_flags |= 0x20u;
    catch_up_command_packet(engine_send, *(&engine_send + 1), bindings.catchup);
    if (bindings.free_look) {
        const CommandAngleBindings view{globals.engine_client, globals.prediction,
                                        bindings.view_angles.environment};
        process_command_free_look(effects::free_look_owner_state(*bindings.free_look).command,
                                  command, bindings.free_look_key, bindings.order.services().input,
                                  view);
    }
    if (original_result)
        forward_original_command_angles(globals.engine_client, globals.prediction, command, false);
    advance_command_cycle_flags(globals.scheduler_flags, *globals.choked);
    CommandEntitySnapshot saved_entities;
    const CommandEntityBindings entities{globals.entity_list, bindings.entities.overridden_property,
                                         bindings.entities.replacement_value};
    save_command_entity_properties(entities, saved_entities);
    // Native ESP+1A is a separate mutable decision byte. Phases1/2 and the
    // frame's send pointer all address it; the saved engine EBX byte stays
    // unchanged until433D5888 commits that decision and reconciles packets.
    std::uint8_t send = engine_send;
    auto finish = [&] {
        finish_command_transaction(command, engine_send, send, bindings.packet_transition,
                                   globals.scheduler_flags, entities, saved_entities);
    };
    if (command_player_restricted(player, bindings.player_gates)) {
        prediction::TimeScope restricted_time(time_targets(globals), prediction_paused(globals),
                                              *globals.interval,
                                              player_tick_base(player, bindings.player_gates));
        process_restricted_player_command(prediction_history_shared(bindings.restricted_owner),
                                          player, &command, &send, true, bindings.restricted);
        restricted_time.restore();
        finish();
        return false;
    }
    //Native retains this first lookup's effects although it discards the value.
    command_weapon(player, bindings);
    prediction::TimeScope command_time(time_targets(globals), prediction_paused(globals),
                                       *globals.interval,
                                       player_tick_base(player, bindings.player_gates));
    auto* weapon = command_weapon(player, bindings);
    auto incoming_angles = command.angles;
    bindings.order.phase1(player, weapon, command, send);
    const auto saved_command = command;
    *globals.current_time =
        static_cast<float>(player_tick_base(player, bindings.player_gates)) * *globals.interval;
    if (std::bit_cast<std::uint32_t>(command.tick) == 0x7F7FFFFFu) {
        refresh_sentinel_target(bindings, player, command);
        command_time.restore();
        finish();
        return false;
    }
    incoming_angles = command.angles;
    weapon = command_weapon(player, bindings);
    {
        prediction::TimeScope phase2_time(time_targets(globals), prediction_paused(globals),
                                          *globals.interval,
                                          player_tick_base(player, bindings.player_gates));
        prediction::update_command_seed(command, bindings.seed_generator);
        bindings.order.phase2(player, weapon, command, incoming_angles, send);
        phase2_time.restore();
    }
    prepare_command_frame(frame, saved_command,
                          std::uint32_t(reinterpret_cast<std::uintptr_t>(&send)), *globals.choked,
                          command.number);
    const auto initial_limit =
        std::bit_cast<std::int32_t>(std::uint32_t(query_command_budget(bindings.budget)) - 1u);
    frame.choke_limit_fc = bindings.order.query_limit(initial_limit);
    const auto physical = bindings.capture.recapture(player);
    const auto timing = sent_timing(bindings);
    const auto capture = bindings.capture.capture(physical, timing);
    const auto captured =
        capture_initial_command_frame(frame, command, send, globals.scheduler_flags, sample_time,
                                      incoming_angles, bindings.bone_storage, capture);
    retain_command_attack_angles(frame, command, player, bindings.mouse, bindings.input);
    if (can_predict_command_copy(command, bindings.copy_gate_convar.get())) {
        CommandCopyOverrideScope outer(bindings.transform_override);
        Command predicted_command;
        const CommandButtonPolicy policy{bindings.inverted_commands,
                                         bindings.anti_aim_other.leg_movement.value,
                                         *globals.choked};
        prepare_prediction_command_copy(frame, command, predicted_command, send, policy,
                                        bindings.trigonometry, bindings.environment);
        const auto prediction_bindings = prediction::bind_command_prediction(
            bindings.prediction_bindings, globals.prediction, player, predicted_command,
            bindings.prediction_player_offsets);
        prediction::CommandPrediction prediction(prediction_bindings, player, &predicted_command,
                                                 std::uint32_t(predicted_command.random_seed));
        recapture_command_prediction(frame, command, prediction, outer, physical, bindings.eye);
    } else
        frame.flags |= 1u; //Native433D4612 skips the entire second prediction.
    auto current_weapon = [&] {
        return reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
    };
    bindings.order.frame_phase(3, frame, player, command);
    bindings.order.frame_phase(4, frame, player, command);
    guard_changed_shot_packet(frame, command, send, current_weapon(), bindings.packet_angles);
    bindings.order.frame_phase(5, frame, player, command);
    guard_packet_angles(frame, command, send, player, current_weapon(), bindings.packet_angles);
    const CommandButtonPolicy policy{bindings.inverted_commands,
                                     bindings.anti_aim_other.leg_movement.value, *globals.choked};
    finalize_command_movement(frame, command, send, policy, bindings.trigonometry,
                              bindings.environment, *globals.last_outgoing, globals.last_shot);
    bindings.order.frame_phase(6, frame, player, command);
    record_sent_command_history(bindings.sent_history, command, send, sent_timing(bindings));
    bindings.order.frame_phase(7, frame, player, command);
    postprocess_command_after_phase7(bindings.postprocess, frame, player, command, send);
    bindings.order.frame_phase(8, frame, player, command);
    restore_command_prediction_cache(frame.flags, player, physical.offsets.common, captured.bones);
    command_time.restore();
    finish();
    return false;
}
std::uint8_t process_create_move_command(const CommandCreateMoveBindings& bindings,
                                         CommandFrame& frame, void* player, float sample_time,
                                         Command& command, std::uint8_t& send) {
    const auto& accepted = bindings.accepted;
    accepted.order.require_sources();
    require_empty_tick_sources(bindings.empty_tick_order);
    const CommandEntryBindings entry_bindings{
        accepted.globals.engine_client, bindings.entry.original_player_table,
        bindings.entry.channel_hook, bindings.entry.rejection_bytes};
    auto entry = enter_hooked_create_move(entry_bindings, player, sample_time, command);
    if (entry.path == CommandPath::engine_return)
        return entry.original_result;
    if (entry.path == CommandPath::empty_tick) {
        if (accepted.free_look) {
            const CommandAngleBindings view{accepted.globals.engine_client,
                                            accepted.globals.prediction,
                                            accepted.view_angles.environment};
            process_command_free_look(effects::free_look_owner_state(*accepted.free_look).command,
                                      command, accepted.free_look_key,
                                      accepted.order.services().input, view);
        }
        if (entry.original_result)
            forward_original_command_angles(accepted.globals.engine_client,
                                            accepted.globals.prediction, command, true);
        dispatch_empty_tick_command(bindings.empty_tick_order, player, sample_time, command,
                                    entry.original_result);
        return entry.original_result;
    }
    accepted.order.phase0(player, command);
    if (command_player_rejected(player, accepted.player_gates)) {
        if (command.number % 3 == 0)
            update_rejected_player(player, bindings.rejected_update);
        return entry.original_result;
    }
    return run_accepted_command_pipeline(accepted, frame, player, sample_time, command, send,
                                         entry.original_result);
}
std::uint8_t CommandCreateMoveRuntime::process(void* player, std::uint8_t* send, float sample_time,
                                               Command* command) {
    CommandFrame frame;
    return process_create_move_command(bindings, frame, player, sample_time, *command, *send);
}
void bind_create_move_runtime(CommandCreateMoveRuntime& runtime) noexcept {
    create_move_runtime = &runtime;
}
void unbind_create_move_runtime(CommandCreateMoveRuntime& runtime) noexcept {
    if (create_move_runtime == &runtime)
        create_move_runtime = nullptr;
}
#if defined(_M_IX86) || defined(__i386__)
std::uint8_t __fastcall create_move_command_hook(void* player, std::uint8_t* send,
                                                 float sample_time, Command* command) {
#else
std::uint8_t create_move_command_hook(void* player, std::uint8_t* send, float sample_time,
                                      Command* command) {
#endif
    return create_move_runtime->process(player, send, sample_time, command);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: command_owner_dispatch.cpp
// ============================================================================

namespace skeet::client {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define COMMAND_FAST __fastcall
#else
#define COMMAND_FAST
#endif
template <class T> T& source(NativeCommandPhaseReceiver* receiver) {
    return *static_cast<T*>(receiver->source);
}
std::uint8_t& send_packet(CommandFrame& frame) {
    return *reinterpret_cast<std::uint8_t*>(std::uintptr_t(frame.send_packet_address));
}
void* weapon_pointer(const CommandFrame& frame) {
    return reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
}
void COMMAND_FAST noop_phase0(NativeCommandPhaseReceiver*, void*, Command*) {} //433C95B7 RET4
void COMMAND_FAST noop_phase1(NativeCommandPhaseReceiver*, void*, void*, Command*, std::uint8_t*) {
} //4337D653 RET0C
void COMMAND_FAST noop_phase2(NativeCommandPhaseReceiver*, void*, void*, Command*,
                              std::array<float, 3>*, std::uint8_t*) {} //43337B12 RET10
void COMMAND_FAST noop_frame_phase(NativeCommandPhaseReceiver*, CommandFrame*, void*, Command*) {
} //4334A269 RET8
std::int32_t COMMAND_FAST unchanged_limit(NativeCommandPhaseReceiver*, std::int32_t incoming) {
    return incoming;
} //433D2AAC
void COMMAND_FAST legit_capture(NativeCommandPhaseReceiver* receiver, void*, Command*) { //4332CDEA
    auto& owner = source<LegitCommandOwner>(receiver);
    owner.capture_view();
}

void COMMAND_FAST viewmodel_capture(NativeCommandPhaseReceiver* receiver, void*,
                                    Command* command) { //433FA91C
    misc::record_viewmodel_command(source<misc::ViewmodelLagState>(receiver), *command);
}

void COMMAND_FAST viewmodel_prepare(NativeCommandPhaseReceiver* receiver, void*, void*,
                                    Command* command, std::uint8_t*) { //433AFBCC
    misc::record_viewmodel_command(source<misc::ViewmodelLagState>(receiver), *command);
}

void COMMAND_FAST exploit_prepare(NativeCommandPhaseReceiver* receiver, void* player, void* weapon,
                                  Command* command, std::uint8_t* send) { //433ABC86
    network::exploit_owner_phase1(source<network::ExploitCommandOwner>(receiver), player, weapon,
                                  *command, *send);
}

void COMMAND_FAST performance_prepare(NativeCommandPhaseReceiver* receiver, void*, void*, Command*,
                                      std::uint8_t*) { //433654D5
    source<misc::LowFpsRuntime>(receiver).phase1(receiver->services->input);
}

void COMMAND_FAST entity_prepare(NativeCommandPhaseReceiver* receiver, void* player, void*,
                                 Command* command, std::uint8_t*) { //4334BB84
    process_entity_command_phase1(source<EntityCommandOwner>(receiver), player, *command);
}

void COMMAND_FAST grenade_prepare(NativeCommandPhaseReceiver* receiver, void* player, void*,
                                  Command* command, std::uint8_t*) { //433B7F7E
    source<misc::GrenadeCommandRuntime>(receiver).phase1(player, *command);
}

void COMMAND_FAST movement_prepare(NativeCommandPhaseReceiver* receiver, void*, void*,
                                   Command* command, std::uint8_t*) { //4333F746
    movement_owner_phase1(source<MovementCommandPhaseOwner>(receiver), *command);
}

void COMMAND_FAST prediction_prepare(NativeCommandPhaseReceiver* receiver, void* player, void*,
                                     Command* command, std::array<float, 3>*,
                                     std::uint8_t*) { //433B383E
    prepare_prediction_history(source<PredictionHistoryOwner>(receiver), player, *command);
}

void COMMAND_FAST quick_switch_prepare(NativeCommandPhaseReceiver* receiver, void* player,
                                       void* weapon, Command* command, std::array<float, 3>*,
                                       std::uint8_t* send) { //4341704A
    std::get<misc::QuickSwitchRuntime*>(source<misc::CommandOwner>(receiver))
        ->process_phase2(player, weapon, *command, *send);
}

void COMMAND_FAST exploit_preprocess(NativeCommandPhaseReceiver* receiver, void* player,
                                     void* weapon, Command* command, std::array<float, 3>* angles,
                                     std::uint8_t* send) { //433D6A11
    network::exploit_owner_phase2(source<network::ExploitCommandOwner>(receiver), player, weapon,
                                  *command, *send, *angles);
}

void COMMAND_FAST buffered_prepare(NativeCommandPhaseReceiver* receiver, void*, void*, Command*,
                                   std::array<float, 3>*, std::uint8_t*) { //433E4CB6
    prepare_buffered_shot_commands(source<BufferedShotCommandOwner>(receiver));
}

void COMMAND_FAST script_prepare(NativeCommandPhaseReceiver* receiver, void*, void*,
                                 Command* command, std::array<float, 3>* angles,
                                 std::uint8_t* send) { //433E41AD
    source<script::ScriptCommandOwner>(receiver).phase2(*command, *angles, *send);
}

void COMMAND_FAST blockbot_prepare(NativeCommandPhaseReceiver* receiver, void* player, void*,
                                   Command* command, std::array<float, 3>* angles,
                                   std::uint8_t*) { //433B92DC
    auto& owner = std::get<misc::BlockbotCommandOwner>(source<misc::CommandOwner>(receiver));
    MovementAngles incoming{(*angles)[0], (*angles)[1], (*angles)[2]};
    process_blockbot(owner.state, owner.enabled.value != 0, owner.hotkey.value.binding,
                     receiver->services->input, player, *command, incoming, owner.bindings);
    *angles = {incoming.pitch, incoming.yaw, incoming.roll};
}

void COMMAND_FAST rage_prepare(NativeCommandPhaseReceiver* receiver, void* player, void* weapon,
                               Command* command, std::array<float, 3>* angles,
                               std::uint8_t* send) { //43358716
    source<rage::CommandEndpoint>(receiver).phase2(player, weapon, *command, *angles, *send);
}

void COMMAND_FAST rebuy_prepare(NativeCommandPhaseReceiver* receiver, void* player, void*,
                                Command* command, std::array<float, 3>*, std::uint8_t*) { //43407336
    auto* owner = std::get<misc::RebuyFixRuntime*>(source<misc::CommandOwner>(receiver));
    owner->local_player = player;
    owner->update(command->movement);
}

void COMMAND_FAST grenade_preprocess(NativeCommandPhaseReceiver* receiver, void* player,
                                     void* weapon, Command* command, std::array<float, 3>* angles,
                                     std::uint8_t*) { //43383139
    source<misc::GrenadeCommandRuntime>(receiver).phase2(player, weapon, *command, *angles,
                                                         receiver->services->input);
}

void COMMAND_FAST no_fall_prepare(NativeCommandPhaseReceiver* receiver, void* player, void*,
                                  Command* command, std::array<float, 3>*,
                                  std::uint8_t*) { //43401BDB
    source<misc::NoFallDamageRuntime>(receiver).phase2(player, *command);
}

void COMMAND_FAST movement_preprocess(NativeCommandPhaseReceiver* receiver, void* player,
                                      void* weapon, Command* command, std::array<float, 3>* angles,
                                      std::uint8_t* send) { //4334659B
    movement_owner_phase2(source<MovementCommandPhaseOwner>(receiver), player, weapon, *command,
                          *angles, *send);
}

void COMMAND_FAST packet_prepare(NativeCommandPhaseReceiver* receiver, void* player, void* weapon,
                                 Command* command, std::array<float, 3>* angles,
                                 std::uint8_t* send) { //433EA911
    network::process_packet_phase2(source<network::PacketCommandOwner>(receiver), player, weapon,
                                   *command, *angles, *send, receiver->services->scheduler_flags);
}

void COMMAND_FAST anti_aim_prepare(NativeCommandPhaseReceiver* receiver, void* player, void* weapon,
                                   Command* command, std::array<float, 3>* angles,
                                   std::uint8_t* send) { //4339F3BF
    anti_aim::command_owner_phase2(source<anti_aim::CommandOwner>(receiver), player, weapon,
                                   *command, *angles, *send);
}

void COMMAND_FAST exploit_frame_prepare(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                        void* player, Command* command) { //433B56ED
    network::exploit_owner_phase3(source<network::ExploitCommandOwner>(receiver), *frame, player,
                                  *command, send_packet(*frame));
}

void COMMAND_FAST automatic_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                    void* player, Command* command) { //433EA2AA
    auto& owner =
        std::get<misc::AutomaticWeaponsCommandOwner>(source<misc::CommandOwner>(receiver));
    process_automatic_weapons(owner.state, owner.settings, owner.bindings, *frame, player, *command,
                              receiver->services->input);
}

void COMMAND_FAST exploit_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                  void* player, Command* command) { //433CE4BC
    network::exploit_owner_phase4(source<network::ExploitCommandOwner>(receiver), *frame, player,
                                  *command, send_packet(*frame));
}

void COMMAND_FAST performance_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                      void*, Command*) { //433229CE
    source<misc::LowFpsRuntime>(receiver).phase4(*frame);
}

void COMMAND_FAST feedback_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame, void*,
                                   Command*) { //4340C927
    rage::process_feedback_command_phase4(source<rage::FeedbackCommandOwner>(receiver), *frame);
}

void COMMAND_FAST script_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame, void*,
                                 Command* command) { //4334A2C8
    source<script::ScriptCommandOwner>(receiver).phase4(*frame, *command);
}

void COMMAND_FAST knife_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                void* player, Command* command) { //433257DA
    auto& owner = std::get<misc::KnifeCommandOwner>(source<misc::CommandOwner>(receiver));
    owner.bindings.process(owner.settings, *frame, *command, player, send_packet(*frame),
                           owner.scheduler_flags);
}

void COMMAND_FAST rage_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                               void* player, Command* command) { //4334F55F
    source<rage::CommandEndpoint>(receiver).process(*frame, player, *command, send_packet(*frame));
}

void COMMAND_FAST target_mask_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                      void*, Command*) { //4340029E
    build_command_target_mask(source<TargetMaskOwner>(receiver), *frame);
}

void COMMAND_FAST penetration_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                      void* player, Command*) { //4334B1DA
    auto& owner = source<PenetrationReticleCommandOwner>(receiver);
    auto targeting = owner.bindings.targeting;
    targeting.local_player = player;
    const auto& base = owner.bindings;
    const hud::PenetrationReticleEnvironment context{targeting,
                                                     base.maximum_clients,
                                                     base.ballistics,
                                                     base.target_tick,
                                                     base.predicted_tick,
                                                     base.collision_group,
                                                     base.move_parent_handle,
                                                     base.point_contents,
                                                     base.trace_filter_vtable};
    const auto& origin = frame->origin_dc;
    const auto& direction = frame->direction_1a8;
    const hud::PenetrationReticleFrame snapshot{weapon_pointer(*frame),
                                                player,
                                                {origin[0], origin[1], origin[2]},
                                                {direction[0], direction[1], direction[2]}};
    hud::update_penetration_reticle(owner.state, owner.settings, snapshot, context);
}

void COMMAND_FAST legit_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                void* player, Command* command) { //43399A38
    auto& owner = source<LegitCommandOwner>(receiver);
    owner.process(*frame, player, *command);
}

void COMMAND_FAST zeus_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                               void* player, Command* command) { //433A12BD
    auto& owner = std::get<misc::ZeusCommandOwner>(source<misc::CommandOwner>(receiver));
    process_zeus(owner.state, owner.settings, owner.bindings, *frame, player, *command);
}

void COMMAND_FAST grenade_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                  void* player, Command* command) { //4338AD80
    source<misc::GrenadeCommandRuntime>(receiver).phase4(*frame, player, *command,
                                                         receiver->services->input);
}

void COMMAND_FAST screen_overlay_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                         void* player, Command* command) { //433361BA
    auto& owner = source<ScreenOverlayCommandOwner>(receiver);
    const auto snapshot = hud::screen_overlay_command_frame(frame);
    owner.publisher.publish(owner.state, owner.settings, snapshot, player, *command, owner.offsets,
                            owner.bindings);
}

void COMMAND_FAST movement_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame, void*,
                                   Command* command) { //433D1F99
    movement_owner_phase4(source<MovementCommandPhaseOwner>(receiver), *frame, *command);
}

void COMMAND_FAST packet_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                 void* player, Command* command) { //433A069B
    network::process_packet_phase4(source<network::PacketCommandOwner>(receiver), *frame, player,
                                   *command, send_packet(*frame),
                                   receiver->services->scheduler_flags);
}

void COMMAND_FAST inventory_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                    void* player, Command*) { //43344A6C
    source<skins::CommandRuntime>(receiver).phase4(*frame, player);
}

void COMMAND_FAST anti_aim_process(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                   void* player, Command* command) { //4332D1C6
    anti_aim::command_owner_phase4(source<anti_aim::CommandOwner>(receiver), *frame, player,
                                   *command, send_packet(*frame));
}

void COMMAND_FAST exploit_generate(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                   void* player, Command* command) { //4336663F
    network::exploit_owner_phase5(source<network::ExploitCommandOwner>(receiver), *frame, player,
                                  *command, send_packet(*frame));
}

void COMMAND_FAST prediction_finish(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                    void* player, Command* command) { //4339FB7D
    finish_prediction_history(source<PredictionHistoryOwner>(receiver), *frame, player, *command,
                              send_packet(*frame));
}

void COMMAND_FAST feedback_finish(NativeCommandPhaseReceiver* receiver, CommandFrame* frame, void*,
                                  Command*) { //4331E7AC
    rage::process_feedback_command_phase6(source<rage::FeedbackCommandOwner>(receiver), *frame);
}

void COMMAND_FAST entity_finish(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                void* player, Command* command) { //433E2400
    process_entity_command_phase6(source<EntityCommandOwner>(receiver), *frame, player,
                                  weapon_pointer(*frame), *command, send_packet(*frame));
}

void COMMAND_FAST buffered_finish(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                  void* player, Command*) { //43323284
    repair_buffered_shot_commands(source<BufferedShotCommandOwner>(receiver), *frame, player,
                                  send_packet(*frame));
}

void COMMAND_FAST script_finish(NativeCommandPhaseReceiver* receiver, CommandFrame* frame, void*,
                                Command* command) { //4339F05D
    source<script::ScriptCommandOwner>(receiver).phase6(*frame, *command);
}

void COMMAND_FAST rage_finish(NativeCommandPhaseReceiver* receiver, CommandFrame* frame, void*,
                              Command* command) { //4335CBFB
    source<rage::CommandEndpoint>(receiver).phase6(*frame, *command);
}

void COMMAND_FAST shared_esp_finish(NativeCommandPhaseReceiver* receiver, CommandFrame*,
                                    void* player, Command*) { //433E3C96
    update_rejected_player(player, source<SharedEspCommandOwner>(receiver).bindings);
}

void COMMAND_FAST effects_finish(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                 void* player, Command* command) { //4339EC1B
    effects::process_bullet_impacts(source<effects::EffectsCommandOwner>(receiver), *frame, player,
                                    *command);
}

void COMMAND_FAST packet_finish(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                void* player, Command* command) { //433A0EFE
    network::process_packet_phase6(source<network::PacketCommandOwner>(receiver), *frame, player,
                                   *command, send_packet(*frame),
                                   receiver->services->scheduler_flags);
}

void COMMAND_FAST anti_aim_finish(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                  void* player, Command* command) { //433470E4
    anti_aim::command_owner_phase6(source<anti_aim::CommandOwner>(receiver), *frame, player,
                                   *command, send_packet(*frame));
}

void COMMAND_FAST prediction_complete(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                      void* player, Command* command) { //43406B4B
    complete_prediction_history(source<PredictionHistoryOwner>(receiver), *frame, player, *command,
                                send_packet(*frame));
}

void COMMAND_FAST exploit_complete(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                   void* player, Command* command) { //43359E37
    network::exploit_owner_phase7(source<network::ExploitCommandOwner>(receiver), *frame, player,
                                  *command, send_packet(*frame));
}

void COMMAND_FAST performance_complete(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                       void*, Command*) { //433B22CB
    source<misc::LowFpsRuntime>(receiver).phase7(*frame, receiver->services->input);
}

void COMMAND_FAST target_mask_complete(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                       void*, Command* command) { //43386E3E
    mark_target_mask_attack(source<TargetMaskOwner>(receiver), *frame, *command);
}

void COMMAND_FAST grenade_complete(NativeCommandPhaseReceiver* receiver, CommandFrame* frame,
                                   void* player, Command*) { //4339C567
    source<misc::GrenadeCommandRuntime>(receiver).phase7(*frame, player);
}

void COMMAND_FAST rage_capture(NativeCommandPhaseReceiver* receiver, CommandFrame* frame, void*,
                               Command*) { //433B76DE
    source<rage::CommandEndpoint>(receiver).capture(*frame);
}

std::int32_t COMMAND_FAST packet_limit(NativeCommandPhaseReceiver* receiver,
                                       std::int32_t limit) { //4340354C
    return network::query_packet_command_limit(source<network::PacketCommandOwner>(receiver),
                                               limit);
}

std::int32_t COMMAND_FAST exploit_limit(NativeCommandPhaseReceiver* receiver,
                                        std::int32_t limit) { //4340165F
    return network::cap_exploit_commands(
        source<network::ExploitCommandOwner>(receiver).runtime.network, limit);
}

struct SourceTable {
    CommandOwnerId id;
    NativeCommandPhaseVtable entries;
};
const std::array<SourceTable, 28> source_tables{{
    {static_cast<CommandOwnerId>(0x4346FF8Cu),
     {noop_phase0, noop_phase1, prediction_prepare, noop_frame_phase, noop_frame_phase,
      noop_frame_phase, prediction_finish, prediction_complete, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x4346F244u),
     {noop_phase0, noop_phase1, noop_phase2, noop_frame_phase, automatic_process, noop_frame_phase,
      noop_frame_phase, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x4346FC24u),
     {noop_phase0, noop_phase1, quick_switch_prepare, noop_frame_phase, noop_frame_phase,
      noop_frame_phase, noop_frame_phase, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x4346EDD4u),
     {noop_phase0, exploit_prepare, exploit_preprocess, exploit_frame_prepare, exploit_process,
      exploit_generate, noop_frame_phase, exploit_complete, noop_frame_phase, exploit_limit}},
    {static_cast<CommandOwnerId>(0x43472148u),
     {noop_phase0, performance_prepare, noop_phase2, noop_frame_phase, performance_process,
      noop_frame_phase, noop_frame_phase, performance_complete, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x4346EED8u),
     {noop_phase0, noop_phase1, noop_phase2, noop_frame_phase, noop_frame_phase, noop_frame_phase,
      noop_frame_phase, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x4346DC2Cu),
     {noop_phase0, noop_phase1, noop_phase2, noop_frame_phase, feedback_process, noop_frame_phase,
      feedback_finish, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x43470200u),
     {noop_phase0, entity_prepare, noop_phase2, noop_frame_phase, noop_frame_phase,
      noop_frame_phase, entity_finish, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x4346F01Cu),
     {noop_phase0, noop_phase1, buffered_prepare, noop_frame_phase, noop_frame_phase,
      noop_frame_phase, buffered_finish, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x43470834u),
     {noop_phase0, noop_phase1, script_prepare, noop_frame_phase, script_process, noop_frame_phase,
      script_finish, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x43471E50u),
     {noop_phase0, noop_phase1, blockbot_prepare, noop_frame_phase, noop_frame_phase,
      noop_frame_phase, noop_frame_phase, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x43471EB8u),
     {noop_phase0, noop_phase1, noop_phase2, noop_frame_phase, knife_process, noop_frame_phase,
      noop_frame_phase, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x43472A00u),
     {noop_phase0, noop_phase1, rage_prepare, noop_frame_phase, rage_process, noop_frame_phase,
      rage_finish, noop_frame_phase, rage_capture, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x43470478u),
     {noop_phase0, noop_phase1, noop_phase2, noop_frame_phase, target_mask_process,
      noop_frame_phase, noop_frame_phase, target_mask_complete, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x43471F0Cu),
     {noop_phase0, noop_phase1, rebuy_prepare, noop_frame_phase, noop_frame_phase, noop_frame_phase,
      noop_frame_phase, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x4346F798u),
     {noop_phase0, noop_phase1, noop_phase2, noop_frame_phase, penetration_process,
      noop_frame_phase, noop_frame_phase, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x4346F5B0u),
     {legit_capture, noop_phase1, noop_phase2, noop_frame_phase, legit_process, noop_frame_phase,
      noop_frame_phase, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x434701A0u),
     {noop_phase0, noop_phase1, noop_phase2, noop_frame_phase, zeus_process, noop_frame_phase,
      noop_frame_phase, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x4346FE30u),
     {noop_phase0, noop_phase1, noop_phase2, noop_frame_phase, noop_frame_phase, noop_frame_phase,
      shared_esp_finish, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x4346F0ACu),
     {noop_phase0, grenade_prepare, grenade_preprocess, noop_frame_phase, grenade_process,
      noop_frame_phase, noop_frame_phase, grenade_complete, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x4346F048u),
     {viewmodel_capture, viewmodel_prepare, noop_phase2, noop_frame_phase, noop_frame_phase,
      noop_frame_phase, noop_frame_phase, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x4346DCC0u),
     {noop_phase0, noop_phase1, noop_phase2, noop_frame_phase, screen_overlay_process,
      noop_frame_phase, noop_frame_phase, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x43472998u),
     {noop_phase0, noop_phase1, no_fall_prepare, noop_frame_phase, noop_frame_phase,
      noop_frame_phase, noop_frame_phase, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x434700E4u),
     {noop_phase0, noop_phase1, noop_phase2, noop_frame_phase, noop_frame_phase, noop_frame_phase,
      effects_finish, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x43470770u),
     {noop_phase0, movement_prepare, movement_preprocess, noop_frame_phase, movement_process,
      noop_frame_phase, noop_frame_phase, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x43470390u),
     {noop_phase0, noop_phase1, packet_prepare, noop_frame_phase, packet_process, noop_frame_phase,
      packet_finish, noop_frame_phase, noop_frame_phase, packet_limit}},
    {static_cast<CommandOwnerId>(0x4346F1F4u),
     {noop_phase0, noop_phase1, noop_phase2, noop_frame_phase, inventory_process, noop_frame_phase,
      noop_frame_phase, noop_frame_phase, noop_frame_phase, unchanged_limit}},
    {static_cast<CommandOwnerId>(0x4346F2E0u),
     {noop_phase0, noop_phase1, anti_aim_prepare, noop_frame_phase, anti_aim_process,
      noop_frame_phase, anti_aim_finish, noop_frame_phase, noop_frame_phase, unchanged_limit}},
}};
#undef COMMAND_FAST
}
const NativeCommandPhaseVtable& source_command_phase_table(CommandOwnerId id) {
    for (const auto& table : source_tables)
        if (table.id == id)
            return table.entries;
    throw std::invalid_argument("Unknown native command source table");
}
bool is_source_command_phase_table(const NativeCommandPhaseVtable* table) noexcept {
    for (const auto& source : source_tables)
        if (table == &source.entries)
            return true;
    return false;
}
bool command_phase_source_bound(CommandOwnerId id, std::uint32_t phase) {
    if (phase >= 10)
        throw std::invalid_argument("Invalid CreateMove phase");
    (void)source_command_phase_table(id);
    return true;
}
void LegitCommandOwner::capture_view() {
    if (auto* source = std::get_if<Standalone>(&source_))
        legit::capture_legit_view(state, source->engine);
    else
        std::get<legit::CommandSource*>(source_)->capture_view(state);
}
void LegitCommandOwner::clear_allocated_storage() {
    state = legit::RuntimeState{};
    if (auto* source = std::get_if<legit::CommandSource*>(&source_))
        (*source)->clear_allocated_storage();
}
void LegitCommandOwner::process(CommandFrame& frame, void* player, Command& command) {
    if (auto* source = std::get_if<Standalone>(&source_))
        legit::process_legit_command(state, frame, player, command, settings, source->bindings);
    else
        std::get<legit::CommandSource*>(source_)->process(state, settings, frame, player, command);
}
void LegitCommandOwner::reset() {
    if (auto* source = std::get_if<Standalone>(&source_))
        legit::reset_runtime_live(state, source->bindings.times.interval, source->bindings.random);
    else
        std::get<legit::CommandSource*>(source_)->reset(state);
}
void LegitCommandOwner::pending(Command& command, float sample_time) {
    if (auto* source = std::get_if<Standalone>(&source_))
        legit::apply_pending_aim(state, command, sample_time, *source->bindings.times.interval);
    else
        std::get<legit::CommandSource*>(source_)->pending(state, command, sample_time);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: command_packet_angle_guard.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */

bool firing_command(const CommandFrame& frame, const Command& command) noexcept {
    const auto flags = frame.flags;
    const auto buttons = command.movement.buttons;
    return (flags & 0x100000U) || ((flags & 0x400U) && (buttons & 1U)) ||
           ((flags & 2U) && (buttons & 0x800U) &&
            (frame.weapon_id_d8 == 64 || frame.weapon_kind_124 == 0));
}

// The inline limiter uses fmod followed by directional inclusive endpoint
// correction. At +180/-180 it differs from normalize_movement_angle.
float step_difference(float current, float reference) noexcept {
    const float difference = current - reference;
    float wrapped = static_cast<float>(std::fmod(static_cast<double>(difference), 360.0));
    if (current > reference) {
        if (!(wrapped < 180.0f))
            wrapped -= 360.0f;
    } else if (!(-180.0f < wrapped))
        wrapped += 360.0f;
    return wrapped;
}
}

void guard_changed_shot_packet(CommandFrame& frame, const Command& command,
                               std::uint8_t& send_packet, const void* weapon,
                               const CommandPacketAngleBindings& bindings) noexcept {
    if (frame.choked_15c < frame.choke_limit_fc && *bindings.enabled &&
        *bindings.firing_mode_enabled && !*bindings.send_mode && firing_command(frame, command) &&
        weapon &&
        network::classified_packet_weapon(field<std::uint16_t>(weapon, bindings.weapon_identifier),
                                          bindings.weapon_classification)) {
        const auto& previous = frame.previous_command.angles;
        if (command.angles[0] != previous[0] || command.angles[1] != previous[1] ||
            command.angles[2] != previous[2])
            send_packet = 0;
    }
    enforce_command_choke_limit(send_packet, frame.choked_15c, frame.choke_limit_fc);
}

void guard_packet_angles(CommandFrame& frame, Command& command, std::uint8_t& send_packet,
                         void* player, const void* weapon,
                         const CommandPacketAngleBindings& bindings) {
    enforce_command_choke_limit(send_packet, frame.choked_15c, frame.choke_limit_fc);
    if ((frame.flags & 0x400008U) == 0x400008U && frame.choked_15c < frame.choke_limit_fc &&
        send_packet == 0 && weapon && firing_command(frame, command))
        command.angles[0] = normalize_movement_angle(command.angles[0]) - 720.0f;

    if (!(frame.flags & 0x40U))
        return;
    const std::array<float, 3>* reference = &frame.previous_command.angles;
    if (!(field<std::uint32_t>(player, bindings.player_flags) & 8U) &&
        field<std::uint8_t>(player, bindings.player_first_byte) ==
            field<std::uint8_t>(player, bindings.player_second_byte)) {
        const auto previous_number =
            std::bit_cast<std::int32_t>(std::uint32_t(command.number) - 1U);
        const auto previous = engine::invoke<const Command*>(bindings.input, 8, 0, previous_number);
        if (previous)
            reference = &previous->angles;
    }

    const float pitch = step_difference(command.angles[0], (*reference)[0]);
    const float yaw = step_difference(command.angles[1], (*reference)[1]);
    const float yaw_square = yaw * yaw;
    const float pitch_square = pitch * pitch;
    const float distance = std::sqrt(pitch_square + yaw_square);
    const float maximum_step = field<float>(&frame, 0x1E8);
    if (!(distance >= maximum_step))
        return;
    const float scale = maximum_step / distance;
    const float scaled_yaw = scale * yaw;
    const float scaled_pitch = scale * pitch;
    command.angles[0] = scaled_pitch + (*reference)[0];
    const float limited_yaw = scaled_yaw + (*reference)[1];
    command.angles[2] = 0.0f;
    command.angles[1] = limited_yaw;
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: command_packet_transition.cpp
// ============================================================================

namespace skeet::client {
namespace {
template <class T> T read_field(const void* object, std::ptrdiff_t offset) noexcept {
    T value;
    std::memcpy(&value, static_cast<const std::byte*>(object) + offset, sizeof value);
    return value;
}

void decrement_word(void* object, std::ptrdiff_t offset) noexcept {
    const auto value = read_field<std::uint32_t>(object, offset) - 1U;
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
}

// Exact 433240A6: both holder levels and the encoded guard offset are read
// after engine slot 26, so mutations made by that query remain observable.
bool player_blocks_transition(const CommandPacketTransitionBindings& bindings) noexcept {
    const auto* holder = bindings.shared_player;
    if (!holder || !*holder)
        return true;
    const auto offset = std::bit_cast<std::int32_t>(*bindings.player_guard_word ^
                                                    bindings.player_guard_key ^ 0xBF567619U);
    return read_field<std::uint8_t>(*holder, offset) != 0;
}
}

void apply_command_packet_transition(std::uint8_t requested_packet,
                                     std::uint8_t& engine_send_packet,
                                     const std::uint8_t& packet_context,
                                     const CommandPacketTransitionBindings& bindings) {
    if (requested_packet == engine_send_packet)
        return;

    if (requested_packet == 0 && engine_send_packet != 0 && packet_context != 0 &&
        bindings.enabled != 0 && bindings.transition_enabled != 0 &&
        engine::invoke<std::uint8_t>(bindings.engine_client, 26) != 0 &&
        !player_blocks_transition(bindings)) {
        void* const channel = engine::invoke<void*>(bindings.engine_client, 78);
        if (channel) {
            // Native IDIV uses the signed sequence and the retained divisor
            // 150 from stack +30. Each record is the startup reset's 8 bytes.
            const auto sequence = read_field<std::int32_t>(channel, 0x18);
            const auto outgoing = *bindings.last_outgoing;
            auto& record = bindings.acknowledgements[sequence % 150];
            record[0] = std::bit_cast<std::uint32_t>(sequence);
            record[1] = outgoing;

            engine::invoke<std::int32_t>(channel, 46, static_cast<void*>(nullptr));
            decrement_word(channel, 0x2C);
            decrement_word(channel, 0x18);
        }
    }
    engine_send_packet = requested_packet;
}

} // namespace skeet::client

// ============================================================================
// Subcomponent: command_phase_registry.cpp
// ============================================================================

namespace skeet::client {
namespace {
constexpr std::array<CommandPhaseTable, 28> command_phase_tables{{
    {CommandOwnerId::owner_4346ff8c,
     {{0x433C95B7u, 0x4337D653u, 0x433B383Eu, 0x4334A269u, 0x4334A269u, 0x4334A269u, 0x4339FB7Du,
       0x43406B4Bu, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::automatic_weapons,
     {{0x433C95B7u, 0x4337D653u, 0x43337B12u, 0x4334A269u, 0x433EA2AAu, 0x4334A269u, 0x4334A269u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::quick_switch,
     {{0x433C95B7u, 0x4337D653u, 0x4341704Au, 0x4334A269u, 0x4334A269u, 0x4334A269u, 0x4334A269u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::exploit,
     {{0x433C95B7u, 0x433ABC86u, 0x433D6A11u, 0x433B56EDu, 0x433CE4BCu, 0x4336663Fu, 0x4334A269u,
       0x43359E37u, 0x4334A269u, 0x4340165Fu}}},
    {CommandOwnerId::owner_43472148,
     {{0x433C95B7u, 0x433654D5u, 0x43337B12u, 0x4334A269u, 0x433229CEu, 0x4334A269u, 0x4334A269u,
       0x433B22CBu, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::owner_4346eed8,
     {{0x433C95B7u, 0x4337D653u, 0x43337B12u, 0x4334A269u, 0x4334A269u, 0x4334A269u, 0x4334A269u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::owner_4346dc2c,
     {{0x433C95B7u, 0x4337D653u, 0x43337B12u, 0x4334A269u, 0x4340C927u, 0x4334A269u, 0x4331E7ACu,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::owner_43470200,
     {{0x433C95B7u, 0x4334BB84u, 0x43337B12u, 0x4334A269u, 0x4334A269u, 0x4334A269u, 0x433E2400u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::owner_4346f01c,
     {{0x433C95B7u, 0x4337D653u, 0x433E4CB6u, 0x4334A269u, 0x4334A269u, 0x4334A269u, 0x43323284u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::owner_43470834,
     {{0x433C95B7u, 0x4337D653u, 0x433E41ADu, 0x4334A269u, 0x4334A2C8u, 0x4334A269u, 0x4339F05Du,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::blockbot,
     {{0x433C95B7u, 0x4337D653u, 0x433B92DCu, 0x4334A269u, 0x4334A269u, 0x4334A269u, 0x4334A269u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::knife,
     {{0x433C95B7u, 0x4337D653u, 0x43337B12u, 0x4334A269u, 0x433257DAu, 0x4334A269u, 0x4334A269u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::rage,
     {{0x433C95B7u, 0x4337D653u, 0x43358716u, 0x4334A269u, 0x4334F55Fu, 0x4334A269u, 0x4335CBFBu,
       0x4334A269u, 0x433B76DEu, 0x433D2AACu}}},
    {CommandOwnerId::owner_43470478,
     {{0x433C95B7u, 0x4337D653u, 0x43337B12u, 0x4334A269u, 0x4340029Eu, 0x4334A269u, 0x4334A269u,
       0x43386E3Eu, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::rebuy,
     {{0x433C95B7u, 0x4337D653u, 0x43407336u, 0x4334A269u, 0x4334A269u, 0x4334A269u, 0x4334A269u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::penetration_reticle,
     {{0x433C95B7u, 0x4337D653u, 0x43337B12u, 0x4334A269u, 0x4334B1DAu, 0x4334A269u, 0x4334A269u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::legit,
     {{0x4332CDEAu, 0x4337D653u, 0x43337B12u, 0x4334A269u, 0x43399A38u, 0x4334A269u, 0x4334A269u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::zeus,
     {{0x433C95B7u, 0x4337D653u, 0x43337B12u, 0x4334A269u, 0x433A12BDu, 0x4334A269u, 0x4334A269u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::owner_4346fe30,
     {{0x433C95B7u, 0x4337D653u, 0x43337B12u, 0x4334A269u, 0x4334A269u, 0x4334A269u, 0x433E3C96u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::owner_4346f0ac,
     {{0x433C95B7u, 0x433B7F7Eu, 0x43383139u, 0x4334A269u, 0x4338AD80u, 0x4334A269u, 0x4334A269u,
       0x4339C567u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::owner_4346f048,
     {{0x433FA91Cu, 0x433AFBCCu, 0x43337B12u, 0x4334A269u, 0x4334A269u, 0x4334A269u, 0x4334A269u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::screen_overlay,
     {{0x433C95B7u, 0x4337D653u, 0x43337B12u, 0x4334A269u, 0x433361BAu, 0x4334A269u, 0x4334A269u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::owner_43472998,
     {{0x433C95B7u, 0x4337D653u, 0x43401BDBu, 0x4334A269u, 0x4334A269u, 0x4334A269u, 0x4334A269u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::owner_434700e4,
     {{0x433C95B7u, 0x4337D653u, 0x43337B12u, 0x4334A269u, 0x4334A269u, 0x4334A269u, 0x4339EC1Bu,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::movement,
     {{0x433C95B7u, 0x4333F746u, 0x4334659Bu, 0x4334A269u, 0x433D1F99u, 0x4334A269u, 0x4334A269u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::fake_lag,
     {{0x433C95B7u, 0x4337D653u, 0x433EA911u, 0x4334A269u, 0x433A069Bu, 0x4334A269u, 0x433A0EFEu,
       0x4334A269u, 0x4334A269u, 0x4340354Cu}}},
    {CommandOwnerId::owner_4346f1f4,
     {{0x433C95B7u, 0x4337D653u, 0x43337B12u, 0x4334A269u, 0x43344A6Cu, 0x4334A269u, 0x4334A269u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
    {CommandOwnerId::anti_aim,
     {{0x433C95B7u, 0x4337D653u, 0x4339F3BFu, 0x4334A269u, 0x4332D1C6u, 0x4334A269u, 0x433470E4u,
       0x4334A269u, 0x4334A269u, 0x433D2AACu}}},
}};
}
std::span<const CommandPhaseTable> native_command_phase_tables() noexcept {
    return command_phase_tables;
}
std::optional<CommandOwnerId> identify_command_phase_owner(std::uint32_t vtable) noexcept {
    for (const auto& table : command_phase_tables)
        if (static_cast<std::uint32_t>(table.id) == vtable)
            return table.id;
    return std::nullopt;
}
const std::array<std::uint32_t, 10>& command_phase_entries(CommandOwnerId id) {
    for (const auto& table : command_phase_tables)
        if (table.id == id)
            return table.entries;
    throw std::invalid_argument("Unknown native CreateMove phase owner");
}
bool command_phase_is_noop(std::uint32_t entry) noexcept {
    return entry == 0x433C95B7u || entry == 0x4337D653u || entry == 0x43337B12u ||
           entry == 0x4334A269u;
}

namespace {
NativeCommandPhaseReceiver& phase_source_receiver(void* object) {
    auto* const receiver = static_cast<NativeCommandPhaseReceiver*>(object);
    if (!receiver || !receiver->source || !receiver->services ||
        !is_source_command_phase_table(receiver->vtable))
        throw std::logic_error("Native command entry has no concrete source receiver binding");
    return *receiver;
}
}
CommandPhaseRegistry::CommandPhaseRegistry(engine::PreservedInitialization& initialization,
                                           engine::JobMemory memory, CommandOwnerDispatch services)
    : CommandPhaseRegistry(initialization.image_owner(), memory, services) {}
CommandPhaseRegistry::CommandPhaseRegistry(std::shared_ptr<engine::NativeDataImage> image,
                                           engine::JobMemory memory, CommandOwnerDispatch services)
    : image_(image),
      owners_(engine::require_native_data_image(image_).native_cell<engine::OwnerRegistry>(
          0x434757A0u)),
      memory_(memory), services_(services) {
    if (!memory_.allocate || !memory_.free || !memory_.process_heap)
        throw std::invalid_argument("Native command storage requires the canonical process heap");
}
void CommandPhaseRegistry::append(NativeCommandPhaseReceiver& receiver, void* source) {
    receiver.source = source;
    receiver.services = &services_;
    //43345EBC uses the same -4 sentinel, append and growth as433676A0.
    //The receiver's final table remains unpublished until its later write.
    engine::register_owner(owners_, &receiver, memory_);
}
void CommandPhaseRegistry::publish(NativeCommandPhaseReceiver& receiver, CommandOwnerId id) const {
    receiver.vtable = &source_command_phase_table(id);
}
void CommandPhaseRegistry::require_sources() const {
    for (auto* object : owners_.entries())
        (void)phase_source_receiver(object);
}
void CommandPhaseRegistry::phase0(void* player, Command& command) const {
    auto** const end = owners_.end; //433D2F95 end precedes433D2F9B begin
    auto** cursor = owners_.begin;
    for (; cursor != end; ++cursor) {
        auto& receiver = phase_source_receiver(*cursor);
        receiver.vtable->phase0(&receiver, player, &command);
    }
}
void CommandPhaseRegistry::phase1(void* player, void* weapon, Command& command,
                                  std::uint8_t& send) const {
    auto** const end = owners_.end; //433D32EB/32F1
    auto** cursor = owners_.begin;
    for (; cursor != end; ++cursor) {
        auto& receiver = phase_source_receiver(*cursor);
        receiver.vtable->phase1(&receiver, player, weapon, &command, &send);
    }
}
void CommandPhaseRegistry::phase2(void* player, void* weapon, Command& command,
                                  std::array<float, 3>& angles, std::uint8_t& send) const {
    auto** const end = owners_.end; //433D3470/3476
    auto** cursor = owners_.begin;
    for (; cursor != end; ++cursor) {
        auto& receiver = phase_source_receiver(*cursor);
        receiver.vtable->phase2(&receiver, player, weapon, &command, &angles, &send);
    }
}
void CommandPhaseRegistry::frame_phase(std::uint32_t phase, CommandFrame& frame, void* player,
                                       Command& command) const {
    if (phase < 3 || phase > 8)
        throw std::invalid_argument("Native frame phase must be3..8");
    auto** const end = owners_.end; //Each of the six loops loads end before begin.
    auto** cursor = owners_.begin;
    for (; cursor != end; ++cursor) {
        auto& receiver = phase_source_receiver(*cursor);
        CommandFramePhaseEntry entry;
        switch (phase) {
        case 3:
            entry = receiver.vtable->phase3;
            break;
        case 4:
            entry = receiver.vtable->phase4;
            break;
        case 5:
            entry = receiver.vtable->phase5;
            break;
        case 6:
            entry = receiver.vtable->phase6;
            break;
        case 7:
            entry = receiver.vtable->phase7;
            break;
        case 8:
            entry = receiver.vtable->phase8;
            break;
        default:
            throw std::logic_error("Invalid native frame phase");
        }
        entry(&receiver, &frame, player, &command);
    }
}
std::int32_t CommandPhaseRegistry::query_limit(std::int32_t limit) const {
    auto** cursor = owners_.begin; //433D3527 begin precedes433D3530 end
    auto** const end = owners_.end;
    for (; cursor != end; ++cursor) {
        auto& receiver = phase_source_receiver(*cursor);
        limit = receiver.vtable->query_limit(&receiver, limit);
    }
    return limit;
}
void CommandPhaseRegistry::finalize_storage() const {
    engine::UnloadSystemFunctions memory;
    memory.free_heap = memory_.free;
    memory.process_heap = memory_.process_heap;
    engine::free_native_allocation(owners_.begin, memory); //433E4A38; cells survive.
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: command_pipeline.cpp
// ============================================================================

namespace skeet::client {
void* register_command_phase_owner(std::vector<void*>& owners, void* owner) {
    owners.push_back(
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(owner)) == 0xFFFFFFFCu ? nullptr
                                                                                           : owner);
    return owner;
}
void initialize_command_frame(CommandFrame& frame) noexcept {
    frame.weapon_address = frame.weapon_data_address = 0;
    frame.previous_command = {};
    frame.field_1ec = frame.field_1f0 = 0;
}
void prepare_command_frame(CommandFrame& frame, const Command& saved, std::uint32_t send,
                           std::int32_t choked, std::int32_t number) noexcept {
    frame.send_packet_address = send;
    frame.weapon_address = frame.weapon_data_address = 0;
    frame.previous_command = saved;
    frame.choked_15c = choked;
    frame.field_18c = frame.field_1a0 = frame.field_1d4 = frame.field_1dc = frame.field_1ec =
        frame.field_1f0 = 0;
    frame.command_number_ac = number;
}
void advance_command_cycle_flags(std::uint32_t& flags, std::int32_t choked) noexcept {
    if (choked != 0)
        return;
    const auto state = (flags >> 9) & 3u;
    if (state)
        flags ^= (((state - 1u) << 9) ^ flags) & 0x600u;
}
void enforce_command_choke_limit(std::uint8_t& send_packet, std::int32_t choked,
                                 std::int32_t limit) noexcept {
    if (!send_packet && choked >= limit)
        send_packet = 1;
}
void sanitize_final_command_angles(Command& command) noexcept {
    for (float angle : command.angles)
        if ((std::bit_cast<std::uint32_t>(angle) & 0x7F800000u) == 0x7F800000u) {
            command.angles = {0.0f, 0.0f, 0.0f};
            return;
        }
}
void record_command_shot(std::uint32_t flags, const Command& command, std::int32_t last_outgoing,
                         std::int32_t& last_shot) noexcept {
    if (((flags & 0x400u) && (command.movement.buttons & 1u)) ||
        ((flags & 2u) && (command.movement.buttons & 0x800u)))
        last_shot = last_outgoing;
}
void record_sent_command_history(SentCommandHistory& history, const Command& command,
                                 std::uint8_t send_packet,
                                 const SentCommandTiming& timing) noexcept {
    record_sent_command_history(SentCommandHistoryView(history), command, send_packet, timing);
}
void record_sent_command_history(SentCommandHistoryView history, const Command& command,
                                 std::uint8_t send_packet,
                                 const SentCommandTiming& timing) noexcept {
    if (!send_packet)
        return;
    history.angles = command.angles;
    for (float& angle : history.angles)
        angle = normalize_movement_angle(angle);
    const auto first = std::bit_cast<std::int32_t>(std::uint32_t(timing.last_outgoing) + 1u);
    if (command.number < first || std::uint32_t(command.number) - std::uint32_t(first) >= 150u)
        return;
    const auto tick = network::adjusted_command_tick(
        timing.base_tick, timing.have_ack, timing.acknowledged, timing.have_sent, timing.sent);
    for (auto number = first;; number = std::bit_cast<std::int32_t>(std::uint32_t(number) + 1u)) {
        history.records[unsigned(number % 150)] = {number, tick};
        if (number == command.number)
            break;
    }
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: command_prediction_cache.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* [stripped helper] */
/* [stripped helper] */
}
void restore_command_prediction_cache(std::uint32_t flags, void* player,
                                      const lagcomp::EntityNetvars& offsets,
                                      const CommandBoneBackup& backup) {
    if (!(flags & 0x20u))
        return;
    write(player, offsets.bone_accessor + 8, std::uint32_t(0));
    write(player, offsets.recent_bone_counter, std::uint32_t(0));
    write(player, offsets.last_bone_setup_time, std::uint32_t(0xFF7FFFFF));
    write(player, offsets.field_0471e13c, std::uint32_t(0));
    write(player, offsets.effects_bb78dc43,
          read<std::uint32_t>(player, offsets.effects_bb78dc43) & 0xFFFFFFFDu);
    if (!backup.count)
        return;
    auto* cache = static_cast<std::byte*>(player) + offsets.bone_cache;
    const auto live = read<std::int32_t>(cache, 0xC);
    if (live < backup.count)
        return;
    std::memcpy(read<Matrix3x4*>(cache, 0), backup.storage, std::size_t(live) * sizeof(Matrix3x4));
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: command_recapture.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */
/* deduplicated helper */
/* [dedup multiply_float] */
/* [dedup add_float] */
/* [dedup address] */
void* active_weapon(const CommandRecaptureContext& context) {
    const auto handle = read<std::uint32_t>(context.player, context.offsets.active_weapon);
    return handle == UINT32_MAX ? nullptr : engine::invoke<void*>(context.entity_list, 4, handle);
}
}
std::array<float, 3> capture_command_recoil(void* player, std::ptrdiff_t offset, void* convar) {
    const auto scale = std::bit_cast<float>(lagcomp::read_prediction_convar(convar));
    const auto recoil = read<std::array<float, 3>>(player, offset);
    return {multiply(recoil[0], scale), multiply(recoil[1], scale), multiply(recoil[2], scale)};
}
void update_predicted_command_weapon(const CommandRecaptureContext& context) {
    if (auto* weapon = active_weapon(context)) {
        if (engine::invoke<std::int32_t>(weapon, 455) == 9)
            return;
        const auto& o = context.fire.offsets;
        const auto definition = read<std::uint16_t>(weapon, o.weapon_definition);
        bool update = definition != 49;
        if (update && (definition == 4 || definition == 10) &&
            read<std::int32_t>(weapon, o.burst_remaining) > 0 &&
            *context.fire.current_time >= read<float>(weapon, o.next_burst) &&
            read<std::int32_t>(weapon, o.ammunition) > 0)
            update = false;
        if (update)
            engine::invoke<void>(weapon, 484);
    }
    const auto scale =
        std::bit_cast<float>(lagcomp::read_prediction_convar(context.recoil_scale_convar.get()));
    write(context.player, context.offsets.field_66146802,
          multiply(scale, read<float>(context.player, context.offsets.recoil_angles)));
}
void capture_predicted_command_frame(CommandFrame& frame, Command& command,
                                     const CommandRecaptureContext& context) {
    frame.flags |= 1;
    capture_command_physical_frame(frame, command, context, CommandCapturePass::predicted);
}
void capture_command_physical_frame(CommandFrame& frame, Command& command,
                                    const CommandRecaptureContext& context,
                                    CommandCapturePass pass) {
    const auto& o = context.offsets;
    frame.duck_amount_98 = read<float>(context.player, o.common.duck_amount);
    const auto player_flags = read<std::uint32_t>(context.player, o.common.player_flags);
    frame.player_flags_7c = player_flags;
    frame.flags ^= ((player_flags << 18) ^ frame.flags) & 0x40000u;
    frame.state_104 = read<std::uint8_t>(context.player, o.byte_7501cb91);
    frame.origin_10c = *engine::invoke<const std::array<float, 3>*>(context.player, 10);
    frame.height_164 = read<float>(context.player, o.view_offset + 8);
    frame.velocity_150 = read<std::array<float, 3>>(context.player, o.common.velocity);
    const auto& v = frame.velocity_150;
    const auto square = add(add(multiply(v[0], v[0]), multiply(v[1], v[1])), multiply(v[2], v[2]));
    frame.scalar_c0 = _mm_cvtss_f32(_mm_sqrt_ss(_mm_set_ss(square)));
    frame.origin_dc = frame.origin_10c;
    frame.origin_dc[2] = add(frame.origin_dc[2], frame.height_164);
    frame.view_height_84 = frame.height_164;
    frame.field_1e4 = 0x7F7FFFFFu;
    auto* weapon = active_weapon(context);
    frame.weapon_address = address(weapon);
    if (weapon) {
        frame.weapon_id_d8 = read<std::uint16_t>(weapon, context.fire.offsets.weapon_definition);
        const auto* data = engine::invoke<const void*>(weapon, 461);
        frame.weapon_data_address = address(data);
        frame.weapon_kind_124 = std::uint8_t(engine::invoke<std::int32_t>(weapon, 455));
        frame.field_b0 = read<std::uint32_t>(weapon, o.weapon_field_572aa59f);
        frame.field_100 = read<std::uint32_t>(weapon, o.common.weapon_field_6777c9ce);
        if (frame.weapon_id_d8 == 64) {
            frame.flags ^= ((frame.flags >> 3) ^ frame.flags) & 0x800u;
            frame.flags ^= (~(frame.flags << 4) ^ frame.flags) & 0x8000u;
            frame.postpone_fire_1e4 = read<float>(weapon, context.fire.offsets.postpone_fire);
        }
        const auto ready =
            skeet::weapon::evaluate_fire_state(context.fire, context.player, weapon, command);
        if (ready.primary_ready)
            frame.flags |= 0x400u;
        if (ready.secondary_ready)
            frame.flags |= 2u;
        if (ready.burst_ready) {
            frame.flags |= 0x100000u;
            command.movement.buttons |= 1u;
        }
        const auto spread = engine::invoke<float>(weapon, 453);
        frame.spread_d0 = spread;
        frame.inaccuracy_c4 = engine::invoke<float>(weapon, 483);
        if (data) {
            frame.pellet_count_19c = read<std::int32_t>(data, 0xFC);
            const auto bullet =
                rage::prepare_target_bullet(weapon, context.fire.offsets.weapon_definition, false);
            frame.bullet_128.weapon_id = std::uint16_t(bullet.weapon_id);
            frame.bullet_128.penetration = bullet.penetration;
            frame.bullet_128.range = bullet.range;
            frame.bullet_128.range_modifier = bullet.range_modifier;
            frame.bullet_128.armor_ratio = bullet.armor_ratio;
            frame.bullet_128.damage = bullet.base_damage;
            frame.bullet_128.pellet_count = bullet.reserved;
        } else {
            frame.pellet_count_19c = 1;
            std::memset(&frame.bullet_128, 0, sizeof frame.bullet_128);
        }
        if (frame.weapon_id_d8 == 64)
            frame.field_160 = (read<std::uint32_t>(context.player, o.common.player_flags) & 6u)
                                  ? 0x3B252696u
                                  : 0x3BC1FC90u;
        else {
            frame.adjusted_spread_160 = spread;
            bool alternate = false;
            if (engine::invoke<std::int32_t>(weapon, 443))
                alternate = *context.secondary_override != 0 ||
                            engine::invoke<std::int32_t>(weapon, 462) != 0;
            const auto adjustment =
                (read<std::uint32_t>(context.player, o.common.player_flags) & 6u)
                    ? engine::invoke<float>(weapon, 436, alternate)
                    : engine::invoke<float>(weapon, 435, alternate);
            frame.adjusted_spread_160 = add(adjustment, frame.adjusted_spread_160);
        }
    } else {
        frame.field_100 = 0;
        frame.spread_d0 = 0.0f;
        frame.field_c4 = 0;
        frame.flags &= ~0x402u;
        frame.weapon_id_d8 = 0;
        frame.weapon_kind_124 = 0;
        frame.weapon_data_address = 0;
        frame.pellet_count_19c = 0;
        std::memset(&frame.bullet_128, 0, sizeof frame.bullet_128);
    }
    const auto recoil =
        capture_command_recoil(context.player, o.recoil_angles, context.recoil_scale_convar.get());
    frame.recoil_angles_9c = recoil;
    constexpr float radians = 0.01745329238474369f;
    const auto pitch_sum = pass == CommandCapturePass::initial ? add(recoil[0], command.angles[0])
                                                               : add(command.angles[0], recoil[0]);
    const auto yaw = multiply(add(command.angles[1], recoil[1]), radians);
    const auto sy = context.trigonometry.sine(yaw), cy = context.trigonometry.cosine(yaw);
    const auto pitch = multiply(pitch_sum, radians);
    const auto sp = context.trigonometry.sine(pitch), cp = context.trigonometry.cosine(pitch);
    const std::array<float, 3> forward{
        multiply(cp, cy), multiply(cp, sy),
        std::bit_cast<float>(std::bit_cast<std::uint32_t>(sp) ^ 0x80000000u)};
    frame.forward_174 = forward;
    frame.player_field_a8 = read<float>(context.player, o.field_5ae32976);
    frame.hull_minimum_1b8 = *engine::invoke<const std::array<float, 3>*>(context.player, 76);
    frame.hull_maximum_1c4 = *engine::invoke<const std::array<float, 3>*>(context.player, 77);
}
template <class Eye>
void recapture_prediction(CommandFrame& frame, Command& command,
                          prediction::CommandPrediction& active, CommandCopyOverrideScope& outer,
                          const CommandRecaptureContext& context, const Eye& eye) {
    update_predicted_command_weapon(context);
    active.restore();
    capture_predicted_command_frame(frame, command, context);
    adjust_command_eye(frame, context.player, command, eye);
    context.published_origin = *engine::invoke<const std::array<float, 3>*>(context.player, 10);
    outer.restore();
}
void recapture_command_prediction(CommandFrame& frame, Command& command,
                                  prediction::CommandPrediction& active,
                                  CommandCopyOverrideScope& outer,
                                  const CommandRecaptureContext& context,
                                  const CommandEyeAdjustmentContext& eye) {
    recapture_prediction(frame, command, active, outer, context, eye);
}
void recapture_command_prediction(CommandFrame& frame, Command& command,
                                  prediction::CommandPrediction& active,
                                  CommandCopyOverrideScope& outer,
                                  const CommandRecaptureContext& context,
                                  const CommandEyeAdjustmentSourceContext& eye) {
    recapture_prediction(frame, command, active, outer, context, eye);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: command_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* [dedup offset_storage] */
ClientRuntimeStorage&
require_shared(ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
               ConfigurationRuntimeOwner& config, HistoryRuntimeOwner& history,
               CommandCaptureRuntimeOwner& capture, animation::RuntimeOwner& animation) {
    auto& data = storage.data();
    const auto& globals = capture.globals();
    if (&engine.variables.data() != &data || &config.storage() != &storage ||
        &history.storage() != &storage || &capture.storage() != &storage ||
        &animation.storage() != &storage || &animation.players() != &history.players().cache() ||
        &globals.engine_client != &data.native_cell<void*>(0x4346A514u) ||
        &globals.entity_list != &data.native_cell<void*>(0x4346A890u) ||
        &globals.prediction != &data.native_cell<void*>(0x43469264u) ||
        &globals.interval != &data.native_cell<float*>(0x4346A338u) ||
        &globals.last_outgoing != &data.native_cell<std::int32_t*>(0x43475B2Cu) ||
        !globals.interval || &history.entity_runtime().interval != globals.interval)
        throw std::invalid_argument(
            "Command runtime requires the shared canonical engine, configuration, history, capture and animation owners");
    const auto& command = engine.command;
    if (!command.seed_generator || !command.move_helper || !command.random_seed ||
        !command.prediction_player || !command.flag_head ||
        command.seed_generator != data.native_cell<prediction::SeedGenerator>(0x434679ACu) ||
        command.move_helper != data.native_cell<void*>(0x43468EC8u) ||
        command.random_seed != data.native_cell<std::int32_t*>(0x43468328u) ||
        command.prediction_player != data.native_cell<void**>(0x434693A4u) ||
        command.flag_head != data.native_cell<prediction::PredictionFlagNode**>(0x43467C58u) ||
        !data.native_cell<void*>(0x4346A7F8u) || !data.native_cell<void*>(0x4346A894u))
        throw std::invalid_argument(
            "Command runtime requires the actual published command engine bindings");
    for (const auto cell : {0x43469008u, 0x434685A0u, 0x43467950u, 0x43467988u})
        if (!engine.variables.at(cell).get())
            throw std::invalid_argument(
                "Command runtime requires its actual acquired prediction and mouse ConVars");
    return storage;
}
}

struct CommandRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    ConfigurationRuntimeOwner& configuration;
    HistoryRuntimeOwner& history;
    CommandCaptureRuntimeOwner& capture;
    prediction::CommandPredictionConVars prediction_variables;
    prediction::CommandPlayerOffsets prediction_fields;
    prediction::CommandBindings prediction;
    CommandPacketAngleBindings packet_angles;
    CommandWeaponPostprocessBindings postprocess;
    CommandPacketTransitionBindings transition;
    CommandTrigonometry trigonometry;

    struct Complete {
        CommandEntryBindings entry;
        CommandAcceptedBindings accepted;
        CommandCreateMoveBindings bindings;
        CommandCreateMoveRuntime runtime;

        Complete(Data& state, ChannelHookState& channel,
                 const RestrictedPlayerCommandBindings& restricted,
                 TargetRefreshRuntimeOwner& refresh, const RejectedPlayerUpdateContext& rejected)
            // The production framing resolves virtual289 from this call's
            // receiver metadata after channel restoration. The original-table
            // field belongs exclusively to enter_create_move's standalone API.
            : entry{state.capture.globals().engine_client, nullptr, &channel,
                    state.capture.player_gates().rejection_bytes},
              accepted{
                  state.capture.globals(),
                  state.capture.player_gates(),
                  state.capture.entities(),
                  state.capture.packet_catchup(),
                  state.storage.data().native_cell<effects::FreeLookOwner*>(0x43475A64u),
                  state.configuration.free_look().key.value.binding,
                  state.capture.view_angles(),
                  state.capture.budget(),
                  state.storage.command_phases(),
                  state.capture,
                  state.capture.eye_adjustment(),
                  state.capture.bone_storage(),
                  state.capture.mouse_offsets(),
                  state.storage.data().native_cell<void*>(0x4346A894u),
                  state.capture.copy_gate(),
                  state.storage.data().native_cell<std::uint8_t*>(0x434683D0u),
                  state.storage.data().native_cell<std::array<std::uint32_t, 150>>(0x43476090u),
                  state.configuration.anti_aim_other(),
                  state.trigonometry,
                  state.capture.angle_environment(),
                  state.prediction,
                  state.prediction_fields,
                  state.storage.data().native_cell<prediction::SeedGenerator>(0x434679ACu),
                  state.packet_angles,
                  state.capture.sent_history(),
                  state.capture.sent_timing_bindings(),
                  state.postprocess,
                  state.transition,
                  state.storage.data().native_cell<PredictionHistoryOwner*>(0x434759B4u),
                  restricted,
                  refresh},
              bindings{entry, accepted, state.storage.empty_ticks(), rejected}, runtime{bindings} {}
    };
    std::unique_ptr<Complete> completed;

    Data(ClientRuntimeStorage& source, const EngineRuntimeBindings& engine,
         ConfigurationRuntimeOwner& config, HistoryRuntimeOwner& history_owner,
         CommandCaptureRuntimeOwner& capture_owner, animation::RuntimeOwner& animation)
        : storage(require_shared(source, engine, config, history_owner, capture_owner, animation)),
          configuration(config), history(history_owner), capture(capture_owner),
          prediction_variables{engine.variables.at(0x43469008u), engine.variables.at(0x434685A0u)},
          prediction_fields{engine::DecodedOffset(source.offsets().globals, 0x43468FD0u,
                                                  0x43467C70u, 0xF58E6118u, 0),
                            offset(source, 0x434671C8u)},
          prediction{source.data().native_cell<void*>(0x43469264u),
                     source.data().native_cell<void*>(0x43468EC8u),
                     source.data().native_cell<void*>(0x4346A7F8u),
                     &source.data().native_cell<std::byte>(0x43478CD8u),
                     engine.command.flag_head,
                     // The six player/command/engine-local targets are bound by the
                     // existing bind_command_prediction at each actual scope entry.
                     {nullptr, nullptr, nullptr,
                      reinterpret_cast<std::uint32_t*>(engine.command.random_seed),
                      reinterpret_cast<std::uint32_t*>(engine.command.prediction_player), nullptr,
                      nullptr, nullptr, nullptr, nullptr},
                     &prediction_variables},
          packet_angles{&config.rage_settings().global.enabled.value,
                        &config.rage_settings().global.silent_aim.value,
                        &config.settings_menu().hold_aim,
                        offset(source, 0x4346908Cu),
                        capture_owner.mouse_offsets().player_flags,
                        capture_owner.mouse_offsets().player_state_043fa575,
                        capture_owner.mouse_offsets().player_state_334b460c,
                        source.data().native_cell<std::array<std::uint32_t, 3>>(0x43474580u),
                        source.data().native_cell<void*>(0x4346A894u)},
          postprocess{source.data().native_cell<void*>(0x4346A894u),
                      engine.variables.at(0x43467950u), engine.variables.at(0x43467988u),
                      &source.data().native_cell<std::array<float, 3>>(0x4347A4F0u),
                      capture_owner.mouse_offsets()},
          transition{
              config.rage_settings().global.enabled.value,
              config.rage_settings().global.anti_aim_correction.value,
              source.data().native_cell<void*>(0x4346A514u),
              source.data().native_cell<void**>(0x4346A3A4u),
              source.offsets().globals.pointers.at(0x43468378u),
              source.offsets().globals.keys.at(0x43468244u),
              source.data().native_cell<std::uint32_t*>(0x43475B2Cu),
              source.data().native_cell<std::array<StartupAcknowledgement, 150>>(0x43475BE0u)},
          trigonometry{animation.trigonometry().sine, animation.trigonometry().cosine} {}
};

CommandRuntimeOwner::CommandRuntimeOwner(ClientRuntimeStorage& storage,
                                         const EngineRuntimeBindings& engine,
                                         ConfigurationRuntimeOwner& config,
                                         HistoryRuntimeOwner& history,
                                         CommandCaptureRuntimeOwner& capture,
                                         animation::RuntimeOwner& animation)
    : data_(std::make_unique<Data>(storage, engine, config, history, capture, animation)) {}
CommandRuntimeOwner::~CommandRuntimeOwner() {
    if (data_->completed)
        unbind_create_move_runtime(data_->completed->runtime);
}
ClientRuntimeStorage& CommandRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
CommandCaptureRuntimeOwner& CommandRuntimeOwner::capture() const noexcept {
    return data_->capture;
}
HistoryRuntimeOwner& CommandRuntimeOwner::history() const noexcept {
    return data_->history;
}
const prediction::CommandBindings& CommandRuntimeOwner::prediction() const noexcept {
    return data_->prediction;
}
const prediction::CommandPlayerOffsets&
CommandRuntimeOwner::prediction_player_offsets() const noexcept {
    return data_->prediction_fields;
}
const CommandPacketAngleBindings& CommandRuntimeOwner::packet_angles() const noexcept {
    return data_->packet_angles;
}
const CommandWeaponPostprocessBindings& CommandRuntimeOwner::postprocess() const noexcept {
    return data_->postprocess;
}
const CommandPacketTransitionBindings& CommandRuntimeOwner::packet_transition() const noexcept {
    return data_->transition;
}
const CommandTrigonometry& CommandRuntimeOwner::trigonometry() const noexcept {
    return data_->trigonometry;
}
bool CommandRuntimeOwner::complete() const noexcept {
    return bool(data_->completed);
}
void CommandRuntimeOwner::complete(engine::HookRuntimeOwner& hooks,
                                   LocalAnimationRuntimeOwner& local_animation,
                                   TargetRefreshRuntimeOwner& refresh,
                                   SharedEspRuntimeOwner& shared_esp) {
    auto& state = *data_;
    auto& data = state.storage.data();
    const auto& restricted = local_animation.restricted();
    const auto& rejected = shared_esp.rejected_player_update();
    if (state.completed)
        throw std::logic_error("Command runtime is already complete");
    if (&hooks.storage() != &state.storage || &refresh.storage() != &state.storage ||
        &shared_esp.storage() != &state.storage || &local_animation.storage() != &state.storage ||
        &local_animation.history() != &state.history ||
        &restricted.players != &state.history.players().cache() ||
        restricted.interval != state.capture.globals().interval ||
        restricted.enabled_43467224 != &state.configuration.rage_settings().global.enabled.value ||
        restricted.entity_list != state.capture.globals().entity_list ||
        &rejected.owner != &data.native_cell<RejectedPlayerUpdateOwner*>(0x434758D0u) ||
        &rejected.cache != &state.history.players().cache() ||
        &rejected.players != &state.history.players().adjustments() ||
        &rejected.entity_list != &data.native_cell<void*>(0x4346A890u) ||
        &rejected.game_rules != &data.native_cell<void**>(0x4346A3A4u) ||
        &rejected.blocked != &data.native_cell<const std::int32_t*>(0x434757DCu) ||
        &rejected.tick_interval != state.capture.globals().interval)
        throw std::invalid_argument(
            "Command completion requires the actual shared hook, restricted-animation, target-refresh and SharedESP contexts");
    auto& channel = hooks.channel_state(); //throws if its real producer is absent
    if (&channel.expected_table != &data.native_cell<void**>(0x43475B0Cu) ||
        &channel.replacement_table != &data.native_cell<void**>(0x43475B14u) ||
        &channel.installed_channel != &data.native_cell<void*>(0x43475B08u))
        throw std::invalid_argument(
            "Command entry requires the canonical process channel hook state");
    state.completed =
        std::make_unique<Data::Complete>(state, channel, restricted, refresh, rejected);
}
CommandCreateMoveRuntime& CommandRuntimeOwner::runtime() const {
    if (!data_->completed)
        throw std::logic_error("Command runtime is missing required late source contexts");
    return data_->completed->runtime;
}
void CommandRuntimeOwner::bind_callback() {
    bind_create_move_runtime(runtime());
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: command_transform.cpp
// ============================================================================

namespace skeet::client {
namespace {
using Vector = std::array<float, 3>;
float neg(float f) {
    return std::bit_cast<float>(std::bit_cast<unsigned>(f) ^ 0x80000000u);
}
float absolute(float f) {
    return std::bit_cast<float>(std::bit_cast<unsigned>(f) & 0x7FFFFFFFu);
}
void basis(Vector angles, Vector& forward, Vector& right, Vector& up,
           const CommandTrigonometry& trig) {
    constexpr float radians = 0.01745329238474369f;
    const float sp = trig.sine(angles[0] * radians), cp = trig.cosine(angles[0] * radians);
    const float sy = trig.sine(angles[1] * radians), cy = trig.cosine(angles[1] * radians);
    const float sr = trig.sine(angles[2] * radians), cr = trig.cosine(angles[2] * radians);
    const float minus_sr = sr * -1.0f, minus_cr = cr * -1.0f;
    forward = {cy * cp, sy * cp, neg(sp)};
    right = {minus_cr * neg(sy) - sr * sp * cy, minus_cr * cy - sr * sp * sy, minus_sr * cp};
    up = {cr * sp * cy - neg(sy) * sr, cr * sp * sy - sr * cy, cr * cp};
}
void horizontal(Vector& vector) {
    if (vector[2] == 0.0f)
        return;
    vector[2] = 0.0f;
    const float length = std::sqrt(vector[0] * vector[0] + vector[1] * vector[1]);
    if (length >= std::numeric_limits<float>::epsilon()) {
        const float inverse = 1.0f / length;
        vector[0] *= inverse;
        vector[1] = inverse * vector[1];
    } else
        vector[0] = vector[1] = 0.0f;
}
float clamp(float f, float low, float high) {
    f = f > low ? f : low;
    return f < high ? f : high;
}
}
void rebuild_command_direction_buttons(Command& command, bool force,
                                       const CommandButtonPolicy& policy) noexcept {
    auto buttons = command.movement.buttons & ~0x618u;
    auto mode = policy.mode;
    const auto number = std::uint32_t(command.number);
    if (policy.inverted_commands[number % 150u] == number)
        mode = 1;
    if (mode == 1 && policy.choked != 0 && !force) {
        command.movement.buttons = buttons;
        return;
    }
    const float forward = command.movement.forward_move, side = command.movement.side_move;
    if (mode == 0) {
        if (forward > 0.0f)
            buttons |= 8u;
        else if (forward < 0.0f)
            buttons |= 0x10u;
        if (side > 0.0f)
            buttons |= 0x400u;
        else if (side < 0.0f)
            buttons |= 0x200u;
    } else if (mode == 1) {
        if (forward > 0.0f)
            buttons |= 0x10u;
        else if (forward < 0.0f)
            buttons |= 8u;
        if (side > 0.0f)
            buttons |= 0x200u;
        else if (side < 0.0f)
            buttons |= 0x400u;
    }
    command.movement.buttons = buttons;
}
void reorient_command_movement(Command& command, const Vector& original, std::uint8_t type,
                               bool grounded, bool force, const CommandButtonPolicy& policy,
                               const CommandTrigonometry& trig) {
    auto& move = command.movement;
    if (type == 8 || type == 9)
        return;
    if (move.forward_move * move.forward_move + move.side_move * move.side_move == 0.0f)
        return;
    if (original != command.angles) {
        Vector old_forward, old_right, old_up, new_forward, new_right, new_up;
        basis({original[0], original[1], 0.0f}, old_forward, old_right, old_up, trig);
        horizontal(old_forward);
        horizontal(old_right);
        const float old_vertical = old_up[2] >= std::numeric_limits<float>::epsilon() ? 1.0f : 0.0f;
        if (!grounded && command.angles[2] == 180.0f)
            move.forward_move = neg(move.forward_move);
        basis(command.angles, new_forward, new_right, new_up, trig);
        horizontal(new_forward);
        horizontal(new_right);
        const float new_vertical = new_up[2] >= std::numeric_limits<float>::epsilon() ? 1.0f : 0.0f;
        for (unsigned i = 0; i != 3; ++i) {
            old_forward[i] *= move.forward_move;
            old_right[i] *= move.side_move;
        }
        const float vertical = old_vertical * move.up_move, zero = move.up_move * 0.0f;
        if (absolute(command.angles[2]) == 0.0f) {
            const auto project = [&](const Vector& target) {
                return ((((((((old_right[1] * target[1] + old_right[0] * target[0]) +
                              old_right[2] * target[2]) +
                             old_forward[1] * target[1]) +
                            old_forward[0] * target[0]) +
                           old_forward[2] * target[2]) +
                          zero * target[1]) +
                         zero * target[0]) +
                        vertical * target[2]);
            };
            move.forward_move = project(new_forward);
            move.side_move = project(new_right);
            move.up_move =
                (((((((old_right[1] * 0.0f + old_right[0] * 0.0f) + old_right[2] * new_vertical) +
                     old_forward[1] * 0.0f) +
                    old_forward[0] * 0.0f) +
                   old_forward[2] * new_vertical) +
                  vertical * new_vertical) +
                 zero * 0.0f) +
                zero * 0.0f;
        } else {
            const float first = new_right[0] * new_forward[1],
                        second = new_right[1] * new_forward[0];
            if (first != second && first != 0.0f) {
                move.side_move = ((old_right[1] + old_forward[1]) * new_forward[0] -
                                  (old_right[0] + old_forward[0]) * new_forward[1]) /
                                 (second - first);
                move.forward_move = ((old_right[1] + old_forward[1]) * new_right[0] -
                                     (old_right[0] + old_forward[0]) * new_right[1]) /
                                    (first - second);
                const float side = absolute(move.side_move), forward = absolute(move.forward_move);
                float scale = 1.0f;
                if (side > forward) {
                    if (side > 450.0f)
                        scale = 450.0f / side;
                } else if (forward > side && forward > 450.0f)
                    scale = 450.0f / forward;
                if (scale != 1.0f) {
                    move.side_move *= scale;
                    move.forward_move *= scale;
                    move.up_move *= scale;
                }
            }
        }
    }
    rebuild_command_direction_buttons(command, force, policy);
}
void sanitize_command(Command& command, bool unrestricted,
                      const ScriptCommandEnvironment& environment) noexcept {
    if (!unrestricted) {
        command.angles[0] = clamp(normalize_movement_angle(command.angles[0]), -89.0f, 89.0f);
        command.angles[1] = clamp(normalize_movement_angle(command.angles[1]), -180.0f, 180.0f);
        if (!command_roll_blocked(environment))
            command.angles[2] = clamp(normalize_movement_angle(command.angles[2]), -45.0f, 45.0f);
        else
            command.angles[2] = 0.0f;
    } else
        command.angles[1] = clamp(normalize_movement_angle(command.angles[1]), -180.0f, 180.0f);
    for (float& angle : command.angles)
        if (std::isnan(angle))
            angle = 0.0f;
    auto& move = command.movement;
    if (std::isnan(move.side_move))
        move.side_move = 0.0f;
    if (std::isnan(move.forward_move))
        move.forward_move = 0.0f;
    if (std::isnan(move.up_move))
        move.up_move = 0.0f;
    move.forward_move = clamp(move.forward_move, -450.0f, 450.0f);
    move.side_move = clamp(move.side_move, -450.0f, 450.0f);
    move.up_move = clamp(move.up_move, -320.0f, 320.0f);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: command_weapon_postprocess.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */
float mouse_scale(const void* variable) noexcept {
    for (;;) {
        const auto* parent = field<const void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    return std::bit_cast<float>(
        field<std::uint32_t>(variable, 0x2C) ^
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(variable)));
}
std::int16_t mouse_count(float angle, float scale) noexcept {
    // CVTTSS2SI returns INT32_MIN for NaNs and overflow. Keeping AX gives
    // zero for that result, and preserves ordinary signed 16-bit wrapping.
    const auto count = _mm_cvttss_si32(_mm_div_ss(_mm_set_ss(angle), _mm_set_ss(scale)));
    return std::bit_cast<std::int16_t>(static_cast<std::uint16_t>(count));
}
}
void postprocess_command_after_phase7(const CommandWeaponPostprocessBindings& bindings,
                                      const CommandFrame& frame, void* player, Command& command,
                                      std::uint8_t send_packet) noexcept {
    // 4341999F reads tick as an integer word, not as a float timestamp.
    if (std::bit_cast<std::uint32_t>(command.tick) != 0x7F7FFFFFu && send_packet) {
        (*bindings.sent_angles)[0] = command.angles[0];
        (*bindings.sent_angles)[1] = command.angles[1];
        (*bindings.sent_angles)[2] = command.angles[2];
        (*bindings.sent_angles)[2] = 0.0f;
    }
    const auto& offsets = bindings.offsets;
    if (field<std::uint32_t>(player, offsets.player_flags) & 8u)
        return;
    if (field<std::uint8_t>(player, offsets.player_state_043fa575) !=
        field<std::uint8_t>(player, offsets.player_state_334b460c))
        return;
    if (command.angles[0] == frame.previous_command.angles[0] &&
        command.angles[1] == frame.previous_command.angles[1] &&
        command.angles[2] == frame.previous_command.angles[2])
        return;

    const auto number = std::uint32_t(command.number) - 1u;
    const auto* commands = field<const Command*>(bindings.input, 0xF0);
    const auto& previous = commands[number % 150u];
    if (std::bit_cast<std::uint32_t>(previous.number) != number)
        return;
    // Native copies all previous angles before it follows either ConVar's
    // parent chain. The ring entry can alias the mutable command.
    const auto previous_angles = previous.angles;
    const auto yaw_scale = mouse_scale(bindings.yaw_scale);
    if (yaw_scale != 0.0f)
        command.mouse_x = mouse_count(
            normalize_movement_angle(previous_angles[1] - command.angles[1]), yaw_scale);
    const auto pitch_scale = mouse_scale(bindings.pitch_scale);
    if (pitch_scale != 0.0f)
        command.mouse_y = mouse_count(
            normalize_movement_angle(command.angles[0] - previous_angles[0]), pitch_scale);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: config_initializers.cpp
// ============================================================================

namespace skeet::client {
namespace {
ConfigInitializerBindings& bindings(void* context) noexcept {
    return *static_cast<ConfigInitializerBindings*>(context);
}
void append(ConfigInitializerBindings& source, ConfigDescriptor& descriptor) noexcept {
    //43385AE4 ->433AFD92 changes only the descriptor/registry links.
    append_config_link(source.configuration.descriptors, descriptor);
}
void feature_indicators(void* context) { //434909F8, descriptor43468B84
    auto& source = bindings(context);
    append(source, source.other_esp.feature_indicators);
    source.other_esp.feature_indicators.value = 0;
}
void anti_aim_correction(void* context) { //43490A1A, descriptor4346753C
    auto& source = bindings(context);
    append(source, source.rage.anti_aim_correction);
}
void visual_recoil(void* context) { //43490A34, descriptor4346744C
    auto& source = bindings(context);
    append(source, source.effects.visual_recoil);
}
void reduce_aim_step(void* context) { //43490A4E, descriptor434670B0
    auto& source = bindings(context);
    append(source, source.rage.reduce_aim_step);
}
void aim_step_limit(void* context) { //43490A68, descriptor4346708C
    auto& source = bindings(context);
    append(source, source.rage.aim_step_limit);
    source.rage.aim_step_limit.value = 15;
}
void ping_enabled(void* context) { //43490AAF, descriptor434691EC
    auto& source = bindings(context);
    append(source, source.ping_spike.enabled);
}
void ping_key(void* context) { //43490AC9, descriptor43469378
    auto& source = bindings(context);
    auto& key = source.ping_spike.key.value;
    append(source, source.ping_spike.key);
    append_hotkey(source.configuration.hotkeys, key); //4333CA03 ->433AFD92
    const auto packed = key.binding.packed;
    key.callback_object = nullptr;
    key.callback = nullptr;
    key.binding.mode = HotkeyMode::hold;
    key.binding.packed = (packed & 1u) | 0x6cu;
}
void ping_milliseconds(void* context) { //43490B14, descriptor43467ECC
    auto& source = bindings(context);
    append(source, source.ping_spike.milliseconds);
    source.ping_spike.milliseconds.value = 100;
}
void crosshair(void* context) { //43490B5B, descriptor4346A754
    auto& source = bindings(context);
    append(source, source.other_esp.overlays.crosshair);
}
void money(void* context) { //43490B75, descriptor4346906C
    auto& source = bindings(context);
    append(source, source.player_esp.money);
}
void grenade_entities(void* context) { //43490B8F, descriptor43467CC4
    auto& source = bindings(context);
    append(source, source.other_esp.grenades.entities);
}
void grenade_color(void* context) { //43490BA9, descriptor43468F50
    auto& source = bindings(context);
    append(source, source.other_esp.grenades.entity_color);
    source.other_esp.grenades.entity_color.value = 0xff0047ffu;
}
void anti_aim_enabled(void* context) { //43490BCE, descriptor4346A730
    auto& source = bindings(context);
    append(source, source.anti_aim.enabled);
}
void anti_aim_pitch(void* context) { //43490BE8, descriptor43467F50
    auto& source = bindings(context);
    append(source, source.anti_aim.pitch);
    source.anti_aim.pitch.value = 0;
}
void anti_aim_yaw(void* context) { //43490C0A, descriptor434682EC
    auto& source = bindings(context);
    append(source, source.anti_aim.yaw);
    source.anti_aim.yaw.value = 0;
}
void anti_aim_body_yaw(void* context) { //43490C2C, descriptor434685A8
    auto& source = bindings(context);
    append(source, source.anti_aim.body_yaw);
    source.anti_aim.body_yaw.value = 0;
}
void double_tap_quick_stop(void* context) { //43490C4E, descriptor4346A664
    auto& source = bindings(context);
    append(source, source.rage.double_tap_quick_stop);
}
void target_mask(void* context) { //43490C68, descriptor43467974
    auto& source = bindings(context);
    append(source, source.target_mask);
}
}

void bind_native_config_initializers(NativeStartupSources& sources,
                                     ConfigInitializerBindings& source) {
    sources.bind_initializer({0x434909f8u, &source, feature_indicators});
    sources.bind_initializer({0x43490a1au, &source, anti_aim_correction});
    sources.bind_initializer({0x43490a34u, &source, visual_recoil});
    sources.bind_initializer({0x43490a4eu, &source, reduce_aim_step});
    sources.bind_initializer({0x43490a68u, &source, aim_step_limit});
    sources.bind_initializer({0x43490aafu, &source, ping_enabled});
    sources.bind_initializer({0x43490ac9u, &source, ping_key});
    sources.bind_initializer({0x43490b14u, &source, ping_milliseconds});
    sources.bind_initializer({0x43490b5bu, &source, crosshair});
    sources.bind_initializer({0x43490b75u, &source, money});
    sources.bind_initializer({0x43490b8fu, &source, grenade_entities});
    sources.bind_initializer({0x43490ba9u, &source, grenade_color});
    sources.bind_initializer({0x43490bceu, &source, anti_aim_enabled});
    sources.bind_initializer({0x43490be8u, &source, anti_aim_pitch});
    sources.bind_initializer({0x43490c0au, &source, anti_aim_yaw});
    sources.bind_initializer({0x43490c2cu, &source, anti_aim_body_yaw});
    sources.bind_initializer({0x43490c4eu, &source, double_tap_quick_stop});
    sources.bind_initializer({0x43490c68u, &source, target_mask});
}

} // namespace skeet::client

// ============================================================================
// Subcomponent: crt_initialization.cpp
// ============================================================================

namespace skeet::client {
namespace {
// Supplied ctx.h sha256=fd89587eb8e1319f189a73e7686a3b6c264323efc413be0315e3a7b29095bdb3
// Context fields current_rip/rax/rip/rsi establish resolver-return records.
// Identifiers are never converted into host executable pointers.
constexpr std::array<CrtInitializationSlot, 182> crt_initialization_slots{{
    {0x000u, 0x00000000u}, {0x004u, 0x434822C8u}, {0x008u, 0x4339E627u}, {0x00Cu, 0x4341738Bu},
    {0x010u, 0x433B22B6u}, {0x014u, 0x4337DA78u}, {0x018u, 0x4340BF0Bu}, {0x01Cu, 0x433F9001u},
    {0x020u, 0x433B7724u}, {0x024u, 0x4337F0AFu}, {0x028u, 0x433C084Fu}, {0x02Cu, 0x4339FA2Bu},
    {0x030u, 0x433B261Du}, {0x034u, 0x4340B872u}, {0x038u, 0x433881D7u}, {0x03Cu, 0x433AB3C9u},
    {0x040u, 0x433A165Du}, {0x044u, 0x4332798Cu}, {0x048u, 0x4340B25Au}, {0x04Cu, 0x433E1038u},
    {0x050u, 0x433B4B44u}, {0x054u, 0x434909F8u}, {0x058u, 0x43490A1Au}, {0x05Cu, 0x43490A34u},
    {0x060u, 0x43490A4Eu}, {0x064u, 0x43490A68u}, {0x068u, 0x43490A8Du}, {0x06Cu, 0x43490AAFu},
    {0x070u, 0x43490AC9u}, {0x074u, 0x43490B14u}, {0x078u, 0x43490B39u}, {0x07Cu, 0x43490B5Bu},
    {0x080u, 0x43490B75u}, {0x084u, 0x43490B8Fu}, {0x088u, 0x43490BA9u}, {0x08Cu, 0x43490BCEu},
    {0x090u, 0x43490BE8u}, {0x094u, 0x43490C0Au}, {0x098u, 0x43490C2Cu}, {0x09Cu, 0x43490C4Eu},
    {0x0A0u, 0x43490C68u}, {0x0A4u, 0x433B86B4u}, {0x0A8u, 0x43393926u}, {0x0ACu, 0x4337E843u},
    {0x0B0u, 0x434928A2u}, {0x0B4u, 0x4331FDB2u}, {0x0B8u, 0x434928E5u}, {0x0BCu, 0x43492907u},
    {0x0C0u, 0x43320100u}, {0x0C4u, 0x4341F48Cu}, {0x0C8u, 0x43400941u}, {0x0CCu, 0x4335939Cu},
    {0x0D0u, 0x433E87AAu}, {0x0D4u, 0x433ACF51u}, {0x0D8u, 0x4340C133u}, {0x0DCu, 0x433F97B6u},
    {0x0E0u, 0x433CD6C5u}, {0x0E4u, 0x4341F401u}, {0x0E8u, 0x4336A58Bu}, {0x0ECu, 0x43388E1Cu},
    {0x0F0u, 0x433B6E1Bu}, {0x0F4u, 0x4336A322u}, {0x0F8u, 0x4340C482u}, {0x0FCu, 0x43336F65u},
    {0x100u, 0x4339D11Fu}, {0x104u, 0x4341CBCFu}, {0x108u, 0x4337CAB9u}, {0x10Cu, 0x433A17AEu},
    {0x110u, 0x4334C748u}, {0x114u, 0x4333BF21u}, {0x118u, 0x433E90FDu}, {0x11Cu, 0x4335BA76u},
    {0x120u, 0x4341875Bu}, {0x124u, 0x4338A373u}, {0x128u, 0x433D0E9Du}, {0x12Cu, 0x433B37A8u},
    {0x130u, 0x43492921u}, {0x134u, 0x43492943u}, {0x138u, 0x433689B6u}, {0x13Cu, 0x43492965u},
    {0x140u, 0x434929A8u}, {0x144u, 0x434929EBu}, {0x148u, 0x43492A2Eu}, {0x14Cu, 0x43492A71u},
    {0x150u, 0x43492AB4u}, {0x154u, 0x43492AF7u}, {0x158u, 0x43492B3Au}, {0x15Cu, 0x43492B7Du},
    {0x160u, 0x43492BC0u}, {0x164u, 0x43492C03u}, {0x168u, 0x43492C46u}, {0x16Cu, 0x43492C89u},
    {0x170u, 0x43492CABu}, {0x174u, 0x43492CCDu}, {0x178u, 0x43492CEFu}, {0x17Cu, 0x43492D11u},
    {0x180u, 0x43492D33u}, {0x184u, 0x43492D55u}, {0x188u, 0x43492D77u}, {0x18Cu, 0x43492D99u},
    {0x190u, 0x43492DBBu}, {0x194u, 0x43492DDDu}, {0x198u, 0x43492DFFu}, {0x19Cu, 0x43492E21u},
    {0x1A0u, 0x433C9DFEu}, {0x1A4u, 0x4337FFB5u}, {0x1A8u, 0x43407D58u}, {0x1ACu, 0x433B0BCEu},
    {0x1B0u, 0x434931DEu}, {0x1B4u, 0x434931F8u}, {0x1B8u, 0x43493241u}, {0x1BCu, 0x433848D6u},
    {0x1C0u, 0x4339791Au}, {0x1C4u, 0x43398F0Cu}, {0x1C8u, 0x433999F0u}, {0x1CCu, 0x4332BB3Au},
    {0x1D0u, 0x4334B57Fu}, {0x1D4u, 0x4337EE46u}, {0x1D8u, 0x433FCAFCu}, {0x1DCu, 0x433A13D2u},
    {0x1E0u, 0x4337F097u}, {0x1E4u, 0x433FD92Fu}, {0x1E8u, 0x433800E5u}, {0x1ECu, 0x433C7FAEu},
    {0x1F0u, 0x4334C970u}, {0x1F4u, 0x4331EDECu}, {0x1F8u, 0x433DF100u}, {0x1FCu, 0x43396E08u},
    {0x200u, 0x433654BDu}, {0x204u, 0x43325025u}, {0x208u, 0x433E1321u}, {0x20Cu, 0x433F47E6u},
    {0x210u, 0x4340BCC1u}, {0x214u, 0x433E3DE2u}, {0x218u, 0x43368710u}, {0x21Cu, 0x433E42ACu},
    {0x220u, 0x433CC92Fu}, {0x224u, 0x433EB701u}, {0x228u, 0x433412AFu}, {0x22Cu, 0x43400269u},
    {0x230u, 0x433E495Au}, {0x234u, 0x433FC811u}, {0x238u, 0x433412F8u}, {0x23Cu, 0x434933A6u},
    {0x240u, 0x434933C0u}, {0x244u, 0x4349340Bu}, {0x248u, 0x43493456u}, {0x24Cu, 0x434934A1u},
    {0x250u, 0x434934BBu}, {0x254u, 0x434934D5u}, {0x258u, 0x434934EFu}, {0x25Cu, 0x43493509u},
    {0x260u, 0x43493552u}, {0x264u, 0x4349356Cu}, {0x268u, 0x43493586u}, {0x26Cu, 0x434935A0u},
    {0x270u, 0x434935C2u}, {0x274u, 0x433A12A4u}, {0x278u, 0x434935DCu}, {0x27Cu, 0x434935F6u},
    {0x280u, 0x43493610u}, {0x284u, 0x4349362Au}, {0x288u, 0x4349364Fu}, {0x28Cu, 0x43493671u},
    {0x290u, 0x4349368Bu}, {0x294u, 0x434936D6u}, {0x298u, 0x4349371Fu}, {0x29Cu, 0x4349376Au},
    {0x2A0u, 0x434937B3u}, {0x2A4u, 0x434937CDu}, {0x2A8u, 0x434937E7u}, {0x2ACu, 0x43493801u},
    {0x2B0u, 0x4349381Bu}, {0x2B4u, 0x4349383Du}, {0x2B8u, 0x43493857u}, {0x2BCu, 0x43493871u},
    {0x2C0u, 0x434938BCu}, {0x2C4u, 0x434938E1u}, {0x2C8u, 0x43349E5Au}, {0x2CCu, 0x4334115Au},
    {0x2D0u, 0x433FB71Eu}, {0x2D4u, 0x43484F06u},
}};
static_assert(crt_initialization_slots.front().operand == 0 && crt_initialization_slots.front().identifier == 0);
static_assert(crt_initialization_slots[1].operand == 4 && crt_initialization_slots[1].identifier == 0x434822c8u);
static_assert(crt_initialization_slots.back().operand == 0x2d4u && crt_initialization_slots.back().identifier == 0x43484f06u);
}

std::span<const CrtInitializationSlot, 182> crt_initialization_plan() noexcept {
    return crt_initialization_slots;
}
CrtInitializationSlot crt_initialization_precheck() noexcept {
    return {0x2DCu, 0};
}

void preflight_crt_initialization(const NativeStartupSources& sources) {
    const auto precheck = crt_initialization_precheck();
    if (precheck.identifier)
        sources.require_preflight(precheck.operand, precheck.identifier);
    for (const auto& slot : crt_initialization_slots)
        if (slot.identifier)
            sources.require_initializer(slot.operand, slot.identifier);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: crt_startup_bindings.cpp
// ============================================================================

namespace skeet::client {
namespace {
void first_services(void* context) {
    // Devirtualized from434822C8 ->434EA5D4; actual OS imports remain explicit.
    engine::initialize_first_startup_services(
        *static_cast<engine::FirstStartupServiceBindings*>(context));
}
void bit_masks(void* context) {
    auto& data = static_cast<ClientRuntimeStorage*>(context)->data();
    initialize_native_startup_bit_masks(
        {data.native_cell<std::array<std::uint32_t, 32 * 33>>(0x43477BE0u),
         data.native_cell<std::array<std::uint32_t, 32>>(0x43477B60u)});
}
void menu_key(void* context) {
    auto& storage = *static_cast<ClientRuntimeStorage*>(context);
    input::initialize_menu_hotkey(storage.configuration(), storage.menu_hotkey());
}
void infinite_duck(void* context) {
    auto& config = *static_cast<ConfigurationRuntimeOwner*>(context);
    initialize_infinite_duck_config(config.storage().configuration(),
                                    config.movement().infinite_duck);
}
void air_duck(void* context) {
    auto& config = *static_cast<ConfigurationRuntimeOwner*>(context);
    initialize_air_duck_config(config.storage().configuration(), config.movement().air_duck);
}
void edge_jump(void* context) {
    auto& config = *static_cast<ConfigurationRuntimeOwner*>(context);
    initialize_edge_jump_hotkey(config.storage().configuration(), config.movement().edge_jump);
}
void skin_filter(void* context) {
    auto& config = *static_cast<ConfigurationRuntimeOwner*>(context);
    config.skins_settings().initialize_shared(config.storage().configuration());
}
void keys_container(void* context) {
    auto& owner = *static_cast<StaticCrtRuntimeOwner*>(context);
    auto& data = owner.storage().data();
    data.native_cell<std::uint32_t>(0x4346DAACu) =
        owner.static_initializers().keys_constructor_word;
    data.native_cell<void*>(0x4346DAB0u) = &data.native_cell<std::byte>(0x4346DAB4u);
    (void)owner.storage().exit_callbacks().append(
        owner.native_vector_cleanup().source_finalizer(0x433FBE15u));
}
}

void bind_early_crt_sources(NativeStartupSources& sources, CrtRuntimeOwner& crt,
                            ConfigurationRuntimeOwner& configuration,
                            StaticCrtRuntimeOwner& containers,
                            script::SourceScriptRuntimeOwner& scripts,
                            animation::FixAnimationBackupCleanupContext& backups,
                            SourceTerminalCleanupBindings& terminal) {
    auto& storage = crt.storage();
    if (&sources.data() != &storage.data() || &configuration.storage() != &storage ||
        &containers.storage() != &storage || &scripts.storage() != &storage ||
        &backups.callbacks != &storage.exit_callbacks() ||
        &backups.allocator != &storage.data().native_cell<void*>(0x43467204u) ||
        &terminal.callbacks != &storage.exit_callbacks() || &terminal.bridge != &scripts.bridge() ||
        &terminal.browser != &scripts.browser_state() || &terminal.ui != &scripts.ui_bindings() ||
        &terminal.strings != &scripts.flags())
        throw std::invalid_argument(
            "Early CRT must borrow the actual process storage and cleanup owners");
    if (storage.exit_callbacks().count)
        throw std::logic_error("Early CRT bindings must be selected before initialization begins");
    crt.bind_initializers(sources);
    configuration.bind_initializers(sources);
    containers.bind_initializers(sources);
    script::bind_native_startup_initializers(sources, scripts.startup_initializers());
    animation::bind_fix_animation_backup_exit_initializer(sources, backups);
    sources.bind_initializer({0x434822C8u, &crt.first_startup_services(), first_services});
    sources.bind_initializer({0x433FB71Eu, &storage, bit_masks});
    sources.bind_initializer({0x434928A2u, &storage, menu_key});
    sources.bind_initializer({0x434933A6u, &configuration, infinite_duck});
    sources.bind_initializer({0x43490B39u, &configuration, air_duck});
    sources.bind_initializer({0x4349340Bu, &configuration, edge_jump});
    sources.bind_initializer({0x43492E21u, &configuration, skin_filter});
    sources.bind_initializer({0x433412F8u, &containers, keys_container});
    // Select actual source containers after native-header binders. This also
    // supplies4332BB3A for the source reference-string vector.
    bind_source_terminal_cleanup(sources, terminal);
    preflight_crt_initialization(sources);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: engine_runtime_bindings.cpp
// ============================================================================

namespace skeet::client {
EngineRuntimeBindings
ClientRuntimeStorage::initialize_engine_bindings(const engine::SourceInitializationImports& imports,
                                                 std::uint32_t timestamp_low) {
    if (engine_bindings_started_)
        throw std::logic_error("Engine bindings already started for this client storage");
    if (offsets_.words)
        throw std::logic_error("Source engine initialization cannot adopt existing offset storage");
    if (imports.memory.allocate != memory_.allocate || imports.memory.free != memory_.free ||
        imports.memory.process_heap != memory_.process_heap)
        throw std::invalid_argument("Engine bindings must use the canonical client process heap");
    engine_bindings_started_ = true;

    auto modules = engine::publish_build_module_bases(*data_, imports);
    engine::initialize_build_window_bindings(*data_, imports.modules);
    auto interfaces = engine::discover_build_interfaces(*data_, imports.images);
    engine::initialize_build_game_state_bindings(*data_);
    engine::initialize_build_client_state_aliases(*data_);
    engine::initialize_build_script_bindings(*data_);
    engine::initialize_build_loader_inspection_inputs(*data_);
    engine::initialize_build_client_factories(*data_);
    engine::initialize_build_visual_interface_bindings(*data_, imports.images);
    engine::initialize_build_esp_bindings(*data_, imports.images);
    engine::initialize_build_material_bindings(*data_, imports.modules, imports.images);
    engine::initialize_build_misc_engine_bindings(*data_);
    const auto command = engine::initialize_build_command_bindings(*data_);
    engine::initialize_build_legit_bindings(*data_);
    engine::initialize_build_world_bindings(*data_);
    engine::initialize_build_map_game_system_bindings(*data_);
    engine::initialize_build_packet_bindings(*data_);
    engine::initialize_build_shared_esp_bindings(*data_, imports.images);
    const auto core = interfaces.core();
    input_.interfaces.engine_client = core.engine_client;
    engine::initialize_build_offset_storage(offsets_, memory_, core.base_client,
                                            static_cast<std::byte*>(modules.at(u"client.dll")),
                                            timestamp_low);
    auto variables = engine::discover_build_startup_convars(*data_, core.cvar);
    const auto attachments = engine::initialize_build_animation_bindings(*data_, imports.modules);
    engine::initialize_build_animation_update_code(*data_, imports.modules);
    engine::initialize_build_animation_velocity_code(*data_, imports.modules);
    engine::initialize_animation_update_flag(*data_);
    engine::initialize_tier0_startup_exports(*data_, imports.modules, imports.images);
    engine::initialize_build_runtime_type_bindings(*data_, imports.images);
    engine::initialize_build_entity_animation_bindings(*data_, imports.modules, offsets_);
    engine::initialize_build_eye_adjustment_bindings(*data_);
    engine::initialize_build_scene_bindings(*data_);
    return {modules, interfaces, variables, attachments, command};
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: entity_command_finish.cpp
// ============================================================================

namespace skeet::client {
namespace {
using Vector = std::array<float, 3>;
/* deduplicated helper */
/* deduplicated helper */
void copy_vector_words(void* destination, std::ptrdiff_t output, const void* source,
                       std::ptrdiff_t input) noexcept {
    for (std::ptrdiff_t word = 0; word != 3; ++word)
        write(destination, output + word * 4, read<std::uint32_t>(source, input + word * 4));
}
std::int32_t previous(std::int32_t number) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(number) - 1u);
}
float recoil_scale(const void* variable) noexcept {
    for (;;) {
        const auto* parent = read<const void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    return std::bit_cast<float>(read<std::uint32_t>(variable, 0x2C) ^
                                std::uint32_t(reinterpret_cast<std::uintptr_t>(variable)));
}
float clamped_view_height(float height) noexcept {
    // Native MAXSS/MINSS choose the second operand for unordered inputs.
    const float lower = height > 46.0f ? height : 46.0f;
    return lower < 64.0f ? lower : 64.0f;
}
void* resolve_handle(const EntityCommandBindings& bindings, std::uint32_t handle) {
    return handle == UINT32_MAX ? nullptr : engine::invoke<void*>(bindings.entity_list, 4, handle);
}
// Complete43342DD7. Resolve the indexed view model first, then follow a live
// observer target only in mode four. The recursion retains the original index.
void* view_model(void* player, std::int32_t index, const EntityCommandBindings& bindings) {
    const auto& offsets = bindings.offsets;
    void* result =
        resolve_handle(bindings, read<std::uint32_t>(player, offsets.view_models_71dcee42 +
                                                                 std::ptrdiff_t(index) * 4));
    if (read<std::int32_t>(player, offsets.observer_mode_2f4e97c4) != 4)
        return result;
    auto* target =
        resolve_handle(bindings, read<std::uint32_t>(player, offsets.observer_target_3b704b41));
    if (target && target != player && read<std::uint8_t>(target, offsets.rejected_9d920e35) == 0 &&
        engine::invoke<std::int32_t>(target, 122) > 0 &&
        !engine::invoke<bool>(static_cast<std::byte*>(target) + 8, 9) &&
        read<std::uint8_t>(target, offsets.rejected_cde290d2) == 0)
        result = view_model(target, index, bindings);
    return result;
}
}

void* entity_command_view_model(EntityCommandOwner& owner, void* player, std::int32_t index) {
    return view_model(player, index, owner.bindings);
}
void capture_entity_prediction_history(EntityCommandOwner& owner, void* player,
                                       std::int32_t number) {
    const auto& bindings = owner.bindings;
    const auto& offsets = bindings.offsets;
    auto& entry = owner.state.history[number % 150];
    const bool first_capture = entry.command_number != number;
    if (first_capture)
        entry.state.flags &= 0xFFFFFFF1u;
    entry.command_number = number;
    write(&entry, 0x04, read<std::uint32_t>(player, offsets.tick_base));
    copy_vector_words(&entry, 0x08, player, offsets.vector_5ae32976);
    copy_vector_words(&entry, 0x14, player, offsets.vector_1bae1168);
    copy_vector_words(&entry, 0x20, player, offsets.vector_48442721);
    write(&entry, 0x2C, clamped_view_height(read<float>(player, offsets.view_offset + 8)));
    copy_vector_words(&entry, 0x30, player, offsets.network_origin);
    write(&entry, 0x3C, read<std::uint32_t>(player, offsets.scalar_2f33c681));
    const auto movement = read<std::uint32_t>(player, offsets.field_b5a66556);
    auto flags = (entry.state.flags & ~0x70u) | ((movement << 4u) & 0x70u);
    entry.state.flags = flags;
    write(&entry, 0x40, read<std::uint32_t>(player, offsets.shots_fired_8cfc633a));
    write(&entry, 0x44, read<std::uint32_t>(player, offsets.field_7f5ed2d0));
    write(&entry, 0x4C, read<std::uint8_t>(player, offsets.field_bcb263a5));
    write(&entry, 0x50, read<std::uint32_t>(player, offsets.field_66146802));
    flags &= ~1u;
    write(&entry, 0x54, read<std::uint32_t>(player, offsets.field_fb7e3404 + 4));
    const auto second_weapon_value = read<std::uint32_t>(player, offsets.field_fb7e3404 + 8);
    write(&entry, 0x5C, UINT32_MAX);
    write(&entry, 0x6C, std::uint32_t(0));
    write(&entry, 0x74, std::uint32_t(0));
    write(&entry, 0x58, second_weapon_value);
    write(&entry, 0x70, std::uint16_t(0));
    entry.state.flags = flags;

    auto* weapon = resolve_handle(bindings, read<std::uint32_t>(player, offsets.active_weapon));
    if (weapon) {
        // The lookup can change the player's handle; native rereads it for
        // publication instead of publishing the handle used by the lookup.
        write(&entry, 0x5C, read<std::uint32_t>(player, offsets.active_weapon));
        write(&entry, 0x60, read<std::uint32_t>(weapon, offsets.weapon_ammunition_ca5ec698));
        write(&entry, 0x64, read<std::uint32_t>(weapon, offsets.weapon_field_0176a381));
        write(&entry, 0x48, read<std::uint32_t>(weapon, offsets.weapon_field_6777c9ce));
        write(&entry, 0x68, read<std::uint32_t>(weapon, offsets.weapon_field_d2563274));
        write(&entry, 0x6C, read<std::uint32_t>(weapon, offsets.weapon_field_15335aac));
        if (engine::invoke<std::int32_t>(weapon, 455) == 9) {
            write(&entry, 0x70, read<std::uint8_t>(weapon, offsets.weapon_field_4013fc03));
            write(&entry, 0x71, read<std::uint8_t>(weapon, offsets.weapon_field_e865a877));
            const auto state = read<std::uint32_t>(weapon, offsets.weapon_field_3b93667b);
            entry.state.flags |= 1u;
            write(&entry, 0x74, state);
        }
        auto* model = view_model(
            player, read<std::int32_t>(weapon, offsets.weapon_viewmodel_index_c6c78c82), bindings);
        if (model)
            write(&entry, 0x78, read<std::uint32_t>(model, offsets.viewmodel_695ff325));
    }
    if (first_capture) {
        write(&entry, 0x7C, read<std::uint32_t>(player, offsets.player_field_6e9852d6));
        write(&entry, 0x80, read<std::uint32_t>(player, offsets.player_field_432b1f5b));
        write(&entry, 0x84, read<std::uint32_t>(player, offsets.player_field_0d560fac));
        auto* collision = engine::invoke<void*>(player, 3);
        const auto* maximum = engine::invoke<const Vector*>(collision, 2);
        write(&entry, 0x88, read<std::uint32_t>(maximum, 8));
    }
}

void finish_entity_prediction(EntityCommandOwner& owner, void* player, const Command& command) {
    const auto& bindings = owner.bindings;
    const auto& offsets = bindings.offsets;
    const float scale = recoil_scale(bindings.recoil_scale_convar);
    write(player, offsets.field_66146802, read<float>(player, offsets.vector_1bae1168) * scale);
    // Complete4340310B: this is the canonical434759B4 snapshot array, whose
    // unsigned modulo differs from the entity-history producer's signed one.
    const auto* animation_owner = prediction_history_shared(bindings.shared_animation_owner);
    if (animation_owner) {
        const auto& snapshot = animation_owner->snapshots[std::uint32_t(command.number) % 150u];
        if (snapshot.command_number == command.number)
            write(player, offsets.field_00a5bd68, read<std::uint8_t>(&snapshot, 4));
    }
    if (read<std::uint8_t>(player, offsets.rejected_9d920e35) == 0) {
        if (!(read<std::uint32_t>(player, offsets.ground_flags) & 2u) &&
            read<std::uint8_t>(player, offsets.field_6b1a59b5) == 0 &&
            read<std::uint8_t>(player, offsets.field_50f9ccc8) == 0) {
            if (read<float>(player, offsets.view_offset + 8) != 64.0f)
                write(player, offsets.view_offset + 8, 64.0f);
        } else if (read<std::uint8_t>(player, offsets.field_50f9ccc8) != 0 &&
                   read<std::uint8_t>(player, offsets.field_6b1a59b5) == 0) {
            if (read<float>(player, offsets.view_offset + 8) != 46.0f)
                write(player, offsets.view_offset + 8, 46.0f);
        }
    }
    const auto number = command.number;
    auto* weapon = resolve_handle(bindings, read<std::uint32_t>(player, offsets.active_weapon));
    if (weapon && read<std::uint16_t>(weapon, offsets.weapon_definition) == 64) {
        const auto preceding = previous(number);
        const auto& prior = owner.state.revolver[preceding % 150];
        const auto state = read<std::uint32_t>(weapon, offsets.weapon_state_9e2078a1);
        if (prior.command_number == preceding && read<std::uint32_t>(&prior, 4) != state &&
            state == 0xD0u) {
            const auto tick = previous(read<std::int32_t>(player, offsets.tick_base));
            const float postpone = float(tick) * *bindings.interval + owner.state.revolver_delay;
            write(weapon, offsets.postpone_fire, postpone);
        }
        auto& predicted = owner.state.revolver[number % 150];
        predicted.command_number = number;
        write(&predicted, 8, read<std::uint32_t>(weapon, offsets.postpone_fire));
        write(&predicted, 4, state);
    }
    capture_entity_prediction_history(owner, player, command.number);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: entity_command_lifecycle.cpp
// ============================================================================

namespace skeet::client {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define ENTITY_MEMBER __thiscall
#define ENTITY_FAST __fastcall
#define ENTITY_SCALAR __vectorcall
#else
#define ENTITY_MEMBER
#define ENTITY_FAST
#define ENTITY_SCALAR
#endif
using OwnerEntry = void(ENTITY_MEMBER*)(void*);
struct EntityCommandInstance;
struct MapEndpoint : esp::MapEntityParserEndpoint {
    EntityCommandInstance* owner;
};
void ENTITY_MEMBER entity_cmd_destruction_entry(void*);
void ENTITY_MEMBER entity_cmd_release_entry(void*);
void ENTITY_MEMBER entity_cmd_reset_entry(void*);
void ENTITY_MEMBER entity_cmd_refresh_entry(void*);
void ENTITY_MEMBER entity_cmd_unchanged(void*) {} //actual433CCE8E
void ENTITY_SCALAR map_scalar(void*, float) {} //actual433CCE8E
void ENTITY_FAST map_text(void*, const char*);
const std::array<OwnerEntry, 9> entity_cmd_primary_table{entity_cmd_destruction_entry, entity_cmd_unchanged,     entity_cmd_unchanged,
                                              entity_cmd_unchanged,         entity_cmd_release_entry, entity_cmd_reset_entry,
                                              entity_cmd_unchanged,         entity_cmd_refresh_entry, entity_cmd_unchanged};
const esp::MapEntityParserTable map_table{map_scalar, map_text};

struct EntityCommandInstance {
    const OwnerEntry* vtable = nullptr;
    NativeCommandPhaseReceiver command_receiver;
    NativeEntityObserverReceiver entity_receiver;
    MapEndpoint map{{nullptr}, this};
    EntityCommandFactoryBindings bindings;
    std::shared_ptr<engine::NativeDataImage> image;
    entity_command_detail::ModelCategories categories;
    std::array<misc::NativeText, 8> names;

    explicit EntityCommandInstance(const EntityCommandFactoryBindings& source)
        : bindings(source), image(source.data),
          categories(source.registries.lifecycle.memory, *image, source.ceiling) {
        auto& registries = bindings.registries;
        auto& command = bindings.command;
        registries.lifecycle.append(this); //4347F93C
        registries.commands.append(command_receiver, &command); //4347F944
        registries.prediction.append(command); //4347F94C
        registries.data_updates.append(command); //4347F954: register, not publish
        registries.entities.append_entity_commands(entity_receiver, command); //4347F95C
        esp::register_map_entity_parser(registries.map_entities, &map); //4347F964
        registries.recharge.append(command); //4347F96C

        registries.entities.publish_entity_commands(entity_receiver); //4347F973, table4346FBF0
        map.vtable = &map_table; //4347F97C, table4347294C
        registries.recharge.publish(command); //4347F983, table43470268
        vtable = entity_cmd_primary_table.data(); //4347F991, table434726E0
        registries.commands.publish(command_receiver, CommandOwnerId::owner_43470200); //4347F997
        registries.prediction.publish(command); //4347F99E, table43470924
        registries.data_updates.publish(command); //4347F9A5, table4346F814
        auto& context = bindings.command_bindings;
        context.engine_client = bindings.engine_client; //4347F9AC
        context.shot_recording_enabled = &bindings.shot_recording_enabled; //4347F9B4
        context.prediction = bindings.prediction_interface; //4347F9BB
        auto& state = command.state;
        state.revolver_delay = 0.23125f;
        auto* revolver = static_cast<RevolverCommandPrediction*>(
            entity_command_detail::allocate(registries.lifecycle.memory, 0x708));
        if (revolver)
            initialize_entity_revolver_storage(revolver);
        state.revolver = revolver;
        auto* history = static_cast<prediction::CommandHistoryEntry*>(
            entity_command_detail::allocate(registries.lifecycle.memory, 0x5460));
        if (history)
            initialize_entity_prediction_storage(history);
        state.history = history;
        categories.initialize();
        state.model_categories = &categories.view();

        constexpr std::array<const char*, 8> literals{
            "m_flVelocityModifier", "m_flThirdpersonRecoil", "m_bWaitForNoAttack",
            "m_bStrafing",          "m_fThrowTime",          "m_bIsHeldByPlayer",
            "m_iMoveState",         "m_flLowerBodyYawTarget"};
        for (std::size_t index = 0; index < names.size(); ++index)
            misc::construct_native_text(names[index], literals[index], registries.lifecycle.memory);
        command.prediction_names = names;
    }
    EntityCommandOwner*& active() const {
        return image->native_cell<EntityCommandOwner*>(0x43475898u);
    }
    void release_owned() {
        bindings.command.prediction_names = {};
        for (std::size_t index = names.size(); index; --index)
            misc::destroy_native_text(names[index - 1], bindings.registries.lifecycle.memory);
        categories.release();
        auto& state = bindings.command.state;
        if (state.history) {
            entity_command_detail::free(bindings.registries.lifecycle.memory, state.history);
            state.history = nullptr;
        }
        if (state.revolver) {
            entity_command_detail::free(bindings.registries.lifecycle.memory, state.revolver);
            state.revolver = nullptr;
        }
    }
};
void ENTITY_MEMBER entity_cmd_destruction_entry(void* object) {
    auto& owner = *static_cast<EntityCommandInstance*>(object);
    destroy_entity_command_owner(object, *owner.image);
}
void ENTITY_MEMBER entity_cmd_release_entry(void* object) {
    auto& owner = *static_cast<EntityCommandInstance*>(object);
    entity_command_detail::release_prediction_scratch(owner.active(), *owner.image);
}
void ENTITY_MEMBER entity_cmd_reset_entry(void* object) {
    reset_entity_command_owner(object);
}
void ENTITY_MEMBER entity_cmd_refresh_entry(void* object) {
    refresh_entity_command_owner(object);
}
void ENTITY_FAST map_text(void* endpoint, const char* text) {
    static_cast<MapEndpoint*>(static_cast<esp::MapEntityParserEndpoint*>(endpoint))
        ->owner->categories.parse(text);
}
void entity_cmd_factory_entry(void* source) {
    create_entity_command_owner(*static_cast<EntityCommandFactoryBindings*>(source));
}
void require_binding_identity(const EntityCommandFactoryBindings& bindings) {
    if (&bindings.command.bindings != &bindings.command_bindings)
        throw std::invalid_argument(
            "Entity command lifecycle requires its canonical command bindings");
    const auto& image = engine::require_native_data_image(bindings.data);
    if (&bindings.registries.prediction.image() != &image ||
        &bindings.registries.data_updates.image() != &image ||
        &bindings.registries.recharge.image() != &image)
        throw std::invalid_argument("Entity command registries require the same retained image");
}
#undef ENTITY_MEMBER
#undef ENTITY_FAST
#undef ENTITY_SCALAR
}

std::size_t entity_command_owner_size() noexcept {
    return sizeof(EntityCommandInstance);
}
void construct_entity_command_owner(void* storage, const EntityCommandFactoryBindings& bindings) {
    require_binding_identity(bindings);
    ::new (storage) EntityCommandInstance(bindings);
}
void* create_entity_command_owner(const EntityCommandFactoryBindings& bindings) {
    require_binding_identity(bindings);
    auto* storage = entity_command_detail::allocate(bindings.registries.lifecycle.memory,
                                                    entity_command_owner_size());
    EntityCommandOwner* owner = nullptr;
    if (storage) {
        construct_entity_command_owner(storage, bindings);
        owner = &bindings.command;
    }
    engine::require_native_data_image(bindings.data).native_cell<EntityCommandOwner*>(0x43475898u) =
        owner;
    return storage;
}
void destroy_entity_command_owner(void* storage, const engine::NativeDataImage& image) {
    image.native_cell<EntityCommandOwner*>(0x43475898u) = nullptr;
    if (!storage)
        return;
    auto* owner = static_cast<EntityCommandInstance*>(storage);
    const auto memory = owner->bindings.registries.lifecycle.memory;
    owner->release_owned(); //43401B38
    owner->~EntityCommandInstance();
    entity_command_detail::free(memory, storage);
}
void reset_entity_command_owner(void* storage) {
    auto& owner = *static_cast<EntityCommandInstance*>(storage);
    auto& command = owner.bindings.command;
    command.state.prediction_stamp = 0;
    command.bindings.angle_history.set_active(false);
    command.bindings.angle_confirmation->count = 0;
    command.bindings.angle_confirmation->index = 0;
    reset_entity_prediction_history(command);
}
void refresh_entity_command_owner(void* storage) {
    auto& owner = *static_cast<EntityCommandInstance*>(storage);
    owner.vtable[5](storage);
    if (!engine::invoke<bool>(owner.bindings.command.bindings.engine_client, 27))
        return;
    const auto* interval = owner.image->native_cell<const float*>(0x4346A338u);
    const float ticks = 1.0f / *interval + 0.5f;
    if (_mm_cvttss_si32(_mm_set_ss(ticks)) != 64)
        return;
    auto* convar = reinterpret_cast<const std::byte*>(
        std::uintptr_t(owner.image->native_cell<std::uint32_t>(0x43468EA4u) ^ 0xFCD2DA7Au));
    for (;;) {
        const std::byte* parent;
        std::memcpy(&parent, convar + 0x1C, sizeof parent);
        if (parent == convar)
            break;
        convar = parent;
    }
    std::uint32_t marker;
    std::memcpy(&marker, convar + 0x30, sizeof marker);
    if (marker == std::uint32_t(reinterpret_cast<std::uintptr_t>(convar)))
        owner.bindings.command.bindings.angle_history.set_active(true);
}
void bind_native_entity_command_factory(NativeStartupSources& sources,
                                        EntityCommandFactoryBindings& bindings) {
    sources.bind_initializer({0x43486705u, &bindings, entity_cmd_factory_entry});
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: entity_command_map.cpp
// ============================================================================

namespace skeet::client::entity_command_detail {
namespace {
// Exact256-word read-only growth table43473248 in the pinned original.
constexpr std::array<std::uint32_t, 256> native_model_category_primes{
    2u,          3u,          5u,          7u,          11u,         13u,         17u,
    19u,         23u,         29u,         31u,         37u,         41u,         43u,
    47u,         53u,         59u,         61u,         67u,         71u,         73u,
    79u,         83u,         89u,         97u,         103u,        109u,        113u,
    127u,        137u,        139u,        149u,        157u,        167u,        179u,
    193u,        199u,        211u,        227u,        241u,        257u,        277u,
    293u,        313u,        337u,        359u,        383u,        409u,        439u,
    467u,        503u,        541u,        577u,        619u,        661u,        709u,
    761u,        823u,        887u,        953u,        1031u,       1109u,       1193u,
    1289u,       1381u,       1493u,       1613u,       1741u,       1879u,       2029u,
    2179u,       2357u,       2549u,       2753u,       2971u,       3209u,       3469u,
    3739u,       4027u,       4349u,       4703u,       5087u,       5503u,       5953u,
    6427u,       6949u,       7517u,       8123u,       8783u,       9497u,       10273u,
    11113u,      12011u,      12983u,      14033u,      15173u,      16411u,      17749u,
    19183u,      20753u,      22447u,      24281u,      26267u,      28411u,      30727u,
    33223u,      35933u,      38873u,      42043u,      45481u,      49201u,      53201u,
    57557u,      62233u,      67307u,      72817u,      78779u,      85229u,      92203u,
    99733u,      107897u,     116731u,     126271u,     136607u,     147793u,     159871u,
    172933u,     187091u,     202409u,     218971u,     236897u,     256279u,     277261u,
    299951u,     324503u,     351061u,     379787u,     410857u,     444487u,     480881u,
    520241u,     562841u,     608903u,     658753u,     712697u,     771049u,     834181u,
    902483u,     976369u,     1056323u,    1142821u,    1236397u,    1337629u,    1447153u,
    1565659u,    1693859u,    1832561u,    1982627u,    2144977u,    2320627u,    2510653u,
    2716249u,    2938679u,    3179303u,    3439651u,    3721303u,    4026031u,    4355707u,
    4712381u,    5098259u,    5515729u,    5967347u,    6456007u,    6984629u,    7556579u,
    8175383u,    8844859u,    9569143u,    10352717u,   11200489u,   12117689u,   13109983u,
    14183539u,   15345007u,   16601593u,   17961079u,   19431899u,   21023161u,   22744717u,
    24607243u,   26622317u,   28802401u,   31160981u,   33712729u,   36473443u,   39460231u,
    42691603u,   46187573u,   49969847u,   54061849u,   58488943u,   63278561u,   68460391u,
    74066549u,   80131819u,   86693767u,   93793069u,   101473717u,  109783337u,  118773397u,
    128499677u,  139022417u,  150406843u,  162723577u,  176048909u,  190465427u,  206062531u,
    222936881u,  241193053u,  260944219u,  282312799u,  305431229u,  330442829u,  357502601u,
    386778277u,  418451333u,  452718089u,  489790921u,  529899637u,  573292817u,  620239453u,
    671030513u,  725980837u,  785430967u,  849749479u,  919334987u,  994618837u,  1076067617u,
    1164186217u, 1259520799u, 1362662261u, 1474249943u, 1594975441u, 1725587117u, 1866894511u,
    2019773507u, 2185171673u, 2364114217u, 2557710269u, 2767159799u, 2993761039u, 3238918481u,
    3504151727u, 3791104843u, 4101556399u, 4294967291u};
struct TokenInput {
    std::uint8_t& initialize;
    std::array<std::uint32_t, 8>& delimiters;

    //433EA7BC. Map entity text uses the native ASCII token contract. Quoted
    //and ordinary tokens retain the original 0x800-byte limit behavior.
    const char* read(const char* cursor, char* output) const {
        std::uint32_t length = 0;
        output[0] = 0;
        if (!cursor)
            return nullptr;
        if (initialize) {
            initialize = 0;
            delimiters.fill(0);
            constexpr char symbols[]{'{', '}', '(', ')', '\'', '"'};
            for (const unsigned char symbol : symbols)
                delimiters[symbol >> 5] |= 1u << (symbol & 31u);
        }
        std::int32_t value;
        for (;;) {
            value = static_cast<signed char>(*cursor);
            while (value <= 0x20) {
                if (!value)
                    return nullptr;
                value = static_cast<signed char>(*++cursor);
            }
            if (value != '/' || cursor[1] != '/')
                break;
            while (*cursor && *cursor != '\n')
                ++cursor;
        }
        if (value == '"') {
            ++cursor;
            for (;;) {
                const char byte = *cursor++;
                value = static_cast<signed char>(byte);
                if (value == '"' || !byte) {
                    output[length] = 0;
                    return cursor;
                }
                const auto previous = length;
                output[length++] = byte;
                if (length < 0x800u)
                    continue;
                length = previous;
                output[length] = 0;
                break;
            }
        }
        const auto delimiter = [&](std::int32_t byte) {
            const auto bits = std::uint32_t(byte);
            return (delimiters[bits >> 5] & (1u << (bits & 31u))) != 0;
        };
        if (delimiter(value)) {
            output[length] = char(value);
            output[length + 1] = 0;
            return cursor + 1;
        }
        do {
            ++cursor;
            output[length] = char(value);
            const auto previous = length++;
            value = static_cast<signed char>(*cursor);
            if (delimiter(value))
                break;
            if (length >= 0x800u) {
                length = previous;
                output[length] = 0;
            }
        } while (value > 0x20);
        output[length] = 0;
        return cursor;
    }
};
struct EntityText {
    const char* begin;
    std::int32_t field_04 = -1;
    const char* cursor;
};
using Token = std::array<char, 0x801>;

bool find_property(const TokenInput& input, const EntityText& entity, std::uint32_t hash,
                   char* output) { //433FBD9F
    Token token;
    auto* cursor = entity.begin;
    while (cursor) {
        cursor = input.read(cursor, token.data());
        if (token[0] == '}')
            return false;
        if (engine::netvar_name_hash(token.data()) == hash) {
            input.read(cursor, token.data());
            std::strncpy(output, token.data(), 0x800);
            return true;
        }
        cursor = input.read(cursor, token.data());
    }
    return false;
}
bool next_property(const TokenInput& input, EntityText& entity, char* key, char* value) { //4340C6B1
    Token token;
    const auto* previous = entity.cursor;
    entity.cursor = input.read(previous, token.data());
    if (token[0] == '}') {
        entity.cursor = previous;
        return false;
    }
    if (!entity.cursor)
        return false;
    std::strncpy(key, token.data(), 0x800);
    auto length = std::strlen(key);
    while (length && key[length - 1] == ' ')
        key[--length] = 0;
    entity.cursor = input.read(entity.cursor, token.data());
    if (!entity.cursor || token[0] == '}')
        return false;
    std::strncpy(value, token.data(), 0x800);
    return true;
}
}

void ModelCategories::initialize() noexcept {
    auto& initial = image_.native_cell<std::array<EntityModelCategoryNode*, 2>>(0x4346935Cu);
    view_.buckets = initial;
    count_ = 0;
    next_resize_ = 0;
    maximum_load_ = 1.0f;
    multiplier_ = 2.0f;
}
void ModelCategories::clear() {
    const auto bucket_count = view_.buckets.size() - 1;
    auto** buckets = const_cast<EntityModelCategoryNode**>(view_.buckets.data());
    for (std::size_t index = 0; index < bucket_count; ++index) {
        auto* node = buckets[index];
        while (node) {
            auto* next = node->next;
            free(memory_, node);
            node = next;
        }
        buckets[index] = nullptr;
    }
    count_ = 0;
}
void ModelCategories::rehash(std::uint32_t count) {
    auto** buckets = static_cast<EntityModelCategoryNode**>(
        allocate(memory_, (std::size_t(count) + 1) * sizeof(EntityModelCategoryNode*)));
    std::memset(buckets, 0, std::size_t(count) * sizeof(EntityModelCategoryNode*));
    buckets[count] = reinterpret_cast<EntityModelCategoryNode*>(~std::uintptr_t{});
    const auto old_count = view_.buckets.size() - 1;
    auto** old = const_cast<EntityModelCategoryNode**>(view_.buckets.data());
    for (std::size_t index = 0; index < old_count; ++index) {
        while (auto* node = old[index]) {
            const auto bucket = node->model_hash % count;
            old[index] = node->next;
            node->next = buckets[bucket];
            buckets[bucket] = node;
        }
    }
    if (old_count > 1)
        free(memory_, old);
    view_.buckets = {buckets, std::size_t(count) + 1};
}
EntityModelCategoryNode& ModelCategories::find_or_insert(const std::uint32_t* key) {
    const auto hash = *key;
    const auto bucket_count = std::uint32_t(view_.buckets.size() - 1);
    auto bucket = hash % bucket_count;
    for (auto* node = view_.buckets[bucket]; node; node = node->next)
        if (node->model_hash == hash)
            return *node;
    const auto growth =
        engine::plan_native_hash_growth(bucket_count, count_, maximum_load_, multiplier_,
                                        next_resize_, native_model_category_primes, ceiling_);
    auto* node = ::new (allocate(memory_, sizeof(EntityModelCategoryNode)))
        EntityModelCategoryNode{*key, 0, nullptr};
    if (growth.rehash) {
        bucket = hash % growth.bucket_count;
        rehash(growth.bucket_count);
    }
    auto** buckets = const_cast<EntityModelCategoryNode**>(view_.buckets.data());
    node->next = buckets[bucket];
    buckets[bucket] = node;
    ++count_;
    return *node;
}
void ModelCategories::parse(const char* text) {
    clear();
    const TokenInput input{image_.native_cell<std::uint8_t>(0x4346D678u),
                           image_.native_cell<std::array<std::uint32_t, 8>>(0x4347C620u)};
    Token token, key, value, temporary;
    auto* cursor = text;
    while ((cursor = input.read(cursor, token.data()))) {
        if (token[0] == '{') {
            EntityText entity{cursor, -1, cursor};
            if (find_property(input, entity, 0xE5EA520Du, temporary.data())) {
                const auto category = engine::netvar_name_hash(temporary.data());
                if (category == 0xA717EF32u || category == 0x96FE2D31u || category == 0x54770CA4u ||
                    category == 0x0A6154BFu) {
                    if (find_property(input, entity, 0xF952B07Eu, temporary.data()) &&
                        std::atoi(temporary.data()) != 0 &&
                        find_property(input, entity, 0x6EA9233Au, temporary.data()) &&
                        !(std::atoi(temporary.data()) & 1) &&
                        find_property(input, entity, 0x22413BABu, temporary.data())) {
                        auto model = engine::netvar_name_hash(temporary.data());
                        find_or_insert(&model).category = category;
                    }
                }
            }
            while (next_property(input, entity, key.data(), value.data())) {
            }
            cursor = entity.cursor;
        }
        if (!cursor)
            break;
        auto depth = 1;
        do {
            cursor = input.read(cursor, token.data());
            if (token[0] == '{' && !token[1])
                ++depth;
            else if (token[0] == '}' && !token[1] && !--depth)
                break;
        } while (cursor);
    }
}
void ModelCategories::release() {
    clear();
    if (view_.buckets.size() > 2)
        free(memory_, const_cast<EntityModelCategoryNode**>(view_.buckets.data()));
}
} // namespace skeet::client::entity_command_detail

// ============================================================================
// Subcomponent: entity_command_owner.cpp
// ============================================================================

namespace skeet::prediction {
CommandHistoryView
CommandHistoryView::from_owner(const client::EntityCommandOwner* owner) noexcept {
    CommandHistoryView result;
    result.owner_present_ = owner != nullptr;
    if (owner)
        result.entries_ = owner->state.history;
    return result;
}
}

namespace skeet::client {
void EntityAnimationAngleBindings::set_active(bool value) const noexcept {
    if (active_)
        *active_ = std::uint8_t(value);
    else
        standalone_->active = value;
}
bool EntityAnimationAngleBindings::select(std::int32_t frame, RageVector& angles) const noexcept {
    if (active_)
        return animation::select_animation_angles(
            animation::AnimationAngleHistoryView(*active_, *frame_, *samples_), frame, angles);
    return animation::select_animation_angles(*standalone_, frame, angles);
}
namespace {
/* deduplicated helper */
/* deduplicated helper */
using Vector = std::array<float, 3>;
std::int32_t previous_number(std::int32_t number) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(number) - 1u);
}
// Fixed433FD134 inputs ECX=14, low=0, high=100. Keep its scale retry and
// SSE conversion: a downward-rounded scale can avoid the retry entirely.
float quantized_prediction_scalar(float value) noexcept {
    if (value < 0.0f)
        return 0.0f;
    if (value > 100.0f)
        return 100.0f;
    const auto ratio = _mm_cvtpd_ps(_mm_div_sd(_mm_set_sd(16383.0), _mm_set_sd(100.0)));
    const auto initial = _mm_cvtss_f32(ratio);
    auto scale = initial;
    if (static_cast<double>(scale) * 100.0 > 16383.0) {
        constexpr float factors[]{0.9998999834060669f, 0.9900000095367432f, 0.8999999761581421f,
                                  0.800000011920929f, 0.699999988079071f};
        for (const auto factor : factors) {
            scale = _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(factor), _mm_set_ss(initial)));
            if (static_cast<double>(scale) * 100.0 <= 16383.0)
                break;
        }
    }
    const auto scaled = value * scale;
    // Native FISTP qword yields an indefinite value for NaN; its low word
    // is zero, which the unsigned conversion below observes as zero.
    const auto integer = std::isnan(scaled) ? 0.0 : std::nearbyint(static_cast<double>(scaled));
    return static_cast<float>(integer) / scale;
}
void restore_near_vector(void* player, std::ptrdiff_t offset, const Vector& desired) {
    const auto actual = read<Vector>(player, offset);
    for (unsigned i = 0; i < 3; ++i)
        if (!(std::fabs(actual[i] - desired[i]) <= 0.03125f))
            return;
    write(player, offset, desired);
}
void repair_predicted_fields(EntityCommandOwner& owner, void* player,
                             const prediction::CommandHistoryState& record) {
    const auto& b = owner.bindings;
    const auto& o = b.offsets;
    const auto tick = read<std::int32_t>(&record, 0);
    if (tick != read<std::int32_t>(player, o.tick_base) && !(record.flags & 8u) &&
        !(std::fabs(*b.prediction_time - std::bit_cast<float>(owner.state.prediction_stamp)) <=
          0.2f))
        return;
    restore_near_vector(player, o.vector_5ae32976, read<Vector>(&record, 4));
    for (unsigned i = 0; i < 3; ++i) {
        const auto desired = record.recoil_angles[i];
        if (std::fabs(read<float>(player, o.vector_1bae1168 + i * 4) - desired) <= 0.03125f)
            write(player, o.vector_1bae1168 + i * 4, desired);
    }
    restore_near_vector(player, o.vector_48442721, read<Vector>(&record, 0x1C));
    const auto height = read<float>(&record, 0x28);
    if (std::fabs(read<float>(player, o.view_offset + 8) - height) <= 0.25f)
        write(player, o.view_offset + 8, height);
    const auto scalar = read<float>(&record, 0x38);
    if (read<float>(player, o.scalar_2f33c681) == quantized_prediction_scalar(scalar))
        write(player, o.scalar_2f33c681, scalar);
    write(player, o.field_b5a66556, (record.flags >> 4) & 7u);
    write(player, o.field_fb7e3404 + 4, read<std::uint32_t>(&record, 0x50));
    write(player, o.field_fb7e3404 + 8, read<std::uint32_t>(&record, 0x54));
    for (unsigned i = 0; i < 3; ++i)
        if (read<float>(player, o.vector_0317eede + i * 4) == -0.0009765625f)
            write(player, o.vector_0317eede + i * 4, std::uint32_t(0));
}
bool convar_enabled(const void* variable) {
    for (;;) {
        const auto* parent = read<const void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    return read<std::uint32_t>(variable, 0x30) !=
           static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(variable));
}
float convar_float(const void* variable) {
    for (;;) {
        const auto* parent = read<const void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    return std::bit_cast<float>(
        read<std::uint32_t>(variable, 0x2C) ^
        static_cast<std::uint32_t>(reinterpret_cast<std::uintptr_t>(variable)));
}
void restore_revolver(EntityCommandOwner& owner, void* player, std::int32_t number) {
    const auto& b = owner.bindings;
    const auto& o = b.offsets;
    const auto handle = read<std::uint32_t>(player, o.active_weapon);
    if (handle != UINT32_MAX)
        if (auto* weapon = engine::invoke<void*>(b.entity_list, 4, handle))
            if (read<std::uint16_t>(weapon, o.weapon_definition) == 64) {
                const auto& record = owner.state.revolver[number % 150];
                if (record.command_number == number)
                    write(weapon, o.postpone_fire, record.postpone_fire);
            }
}
Vector bounds(void* entity, std::size_t slot) {
    return *engine::invoke<const Vector*>(engine::invoke<void*>(entity, 3), slot);
}
Vector vector_add(Vector first, const Vector& second) noexcept {
    for (unsigned i = 0; i < 3; ++i)
        first[i] += second[i];
    return first;
}
}

void initialize_entity_revolver_storage(RevolverCommandPrediction* storage) noexcept {
    std::memset(storage, 0, 0x708);
}
void initialize_entity_prediction_storage(prediction::CommandHistoryEntry* storage) noexcept {
    std::memset(storage, 0, 0x5460);
    for (unsigned index = 0; index < 150; ++index) {
        storage[index].state.weapon_identity = UINT32_MAX;
        write(&storage[index].state, 0x78, UINT32_MAX);
    }
}
void initialize_entity_command_storage(EntityCommandStorage& storage) noexcept {
    initialize_entity_revolver_storage(storage.revolver.data());
    initialize_entity_prediction_storage(storage.history.data());
}
void bind_entity_command_storage(EntityCommandState& state,
                                 EntityCommandStorage& storage) noexcept {
    state.revolver = storage.revolver.data();
    state.history = storage.history.data();
    state.revolver_delay = 0.23125f;
}
void process_entity_command_phase1(EntityCommandOwner& owner, void* player, Command& command) {
    const auto& b = owner.bindings;
    const auto& o = b.offsets;
    auto& state = owner.state;
    if (!*b.startup_guard_4347bdac && !*b.startup_guard_4347bd78 && !*b.connection_checked) {
        *b.connection_checked = 1;
        if (engine::invoke<bool>(b.engine_client, 27)) {
            if (b.connection_notification)
                b.connection_notification();
            else
                *b.deferred_connection = 1;
        }
    }
    const auto update_frame = *b.update_frame;
    if (update_frame <= 0)
        return;
    const auto previous = previous_number(command.number);
    const auto acknowledged = engine::invoke<std::int32_t>(b.engine_client, 205);
    const auto difference = std::uint32_t(previous) - std::uint32_t(acknowledged);
    if (difference >= 150u)
        return;
    const auto* commands = read<const Command*>(b.input, 0xF0);
    const auto& prior = commands[std::uint32_t(previous) % 150u];
    if (prior.number != previous)
        return;
    const auto slot = previous % 150;
    const bool can_restore = prior.predicted &&
                             read<std::int32_t>(player, o.prediction_context + 0x68) == previous &&
                             read<std::int32_t>(b.prediction, 0xC) == update_frame &&
                             state.history[slot].command_number == previous;
    if (can_restore) {
        if (b.restore_prediction_frame && difference > 0)
            b.restore_prediction_frame(b.prediction, 0, static_cast<std::int32_t>(difference) - 1);
    } else {
        const auto saved_player_angles = read<Vector>(player, o.prediction_angles);
        Vector saved_engine_angles;
        engine::invoke<void>(b.engine_client, 18, saved_engine_angles.data());
        engine::invoke<void>(b.engine_client, 19, b.sent_angles->data());
        write(player, o.prediction_angles, *b.sent_angles);
        engine::invoke<void>(b.prediction, 3, update_frame, true, acknowledged, previous);
        engine::invoke<void>(b.engine_client, 19, saved_engine_angles.data());
        write(player, o.prediction_angles, saved_player_angles);
    }
    if (state.history[slot].command_number == previous)
        repair_predicted_fields(owner, player, state.history[slot].state);
    restore_revolver(owner, player, previous);
    const auto tick = read<std::int32_t>(player, o.tick_base);
    write(player, o.tick_base, b.observers->adjust_tick(command.number, tick));
}

void process_entity_command_phase6(EntityCommandOwner& owner, const CommandFrame& frame,
                                   void* player, void* weapon, const Command& command,
                                   std::uint8_t send_packet) {
    const auto& b = owner.bindings;
    const auto& o = b.offsets;
    if (*b.shot_recording_enabled && frame.weapon_address &&
        network::classified_packet_weapon(read<std::uint16_t>(weapon, o.weapon_definition),
                                          b.weapon_classification) &&
        ((frame.flags & 0x100000u) || ((frame.flags & 0x400u) && (command.movement.buttons & 1u)) ||
         ((frame.flags & 2u) && (command.movement.buttons & 0x800u) &&
          (frame.weapon_id_d8 == 64 || frame.weapon_kind_124 == 0))) &&
        !send_packet)
        owner.state.last_choked_shot = command.number;
    const bool include_teammates = convar_enabled(b.teammates_are_enemies);
    const auto minimum = vector_add(bounds(player, 1), frame.origin_10c);
    const auto maximum = vector_add(bounds(player, 2), frame.origin_10c);
    for (std::int32_t index = 1; index <= 64; ++index) {
        auto* other = engine::invoke<void*>(b.entity_list, 3, index);
        if (!other || !engine::invoke<bool>(other, 158) ||
            read<std::uint8_t>(other, o.rejected_9d920e35) ||
            engine::invoke<std::int32_t>(other, 122) <= 0 ||
            engine::invoke<bool>(static_cast<std::byte*>(other) + 8, 9) ||
            read<std::uint8_t>(other, o.rejected_cde290d2) || other == player ||
            (!include_teammates &&
             read<std::int32_t>(player, o.team) == read<std::int32_t>(other, o.team)))
            continue;
        const auto origin = read<Vector>(other, o.network_origin);
        const auto other_minimum = vector_add(bounds(other, 1), origin);
        const auto other_maximum = vector_add(bounds(other, 2), origin);
        if (minimum[0] < other_maximum[0] && minimum[1] < other_maximum[1] &&
            minimum[2] < other_maximum[2] && other_minimum[0] < maximum[0] &&
            other_minimum[1] < maximum[1] && other_minimum[2] < maximum[2]) {
            invalidate_command_prediction(b.prediction);
            return;
        }
    }
}

void process_entity_prediction_entities(EntityCommandOwner& owner,
                                        const EntityPredictionEntities& records) {
    const auto& b = owner.bindings;
    const auto& o = b.offsets;
    if (!*b.physics_class)
        return;
    for (std::int32_t i = 0; i < records.count; ++i) {
        const auto index = records.entries[i].entity_index;
        if (index < 64 || (std::uint32_t(index) & 0xFFFF0000u))
            continue;
        auto* entity = engine::invoke<void*>(b.entity_list, 3, index);
        if (!entity)
            continue;
        auto* networkable = engine::invoke<void*>(entity, 4);
        if (!networkable)
            continue;
        auto* identity = engine::invoke<void*>(networkable, 2);
        if (!identity || identity != *b.physics_class)
            continue;
        if (engine::invoke<std::int32_t>(engine::invoke<void*>(entity, 3), 11) != 1 ||
            read<std::uint8_t>(entity, o.move_type) != 7)
            continue;
        if (!(engine::invoke<std::uint32_t>(engine::invoke<void*>(entity, 3), 12) & 0x800u) ||
            read<std::int32_t>(entity, o.owner_handle) != -1 ||
            (read<std::uint32_t>(entity, o.ground_flags) & 0x2000000u) ||
            (read<float>(entity, o.activity_8dbc9874) == 0.0f &&
             read<float>(entity, o.activity_68140cd8) == 0.0f))
            continue;
        auto* model = engine::invoke<void*>(static_cast<std::byte*>(entity) + 4, 8);
        if (!model)
            continue;
        const auto* name = engine::invoke<const char*>(b.model_info, 3, model);
        if (!name)
            continue;
        const auto hash = events::event_hash(name);
        const auto* categories = owner.state.model_categories;
        if (!categories)
            continue;
        const auto count = categories->buckets.size() - 1;
        auto* const end = categories->buckets[count];
        auto* node = categories->buckets[hash % count];
        while (node && node->model_hash != hash)
            node = node->next;
        if (!node)
            node = end;
        if (node != end && node->category == 0xA717EF32u)
            write(entity, o.collision_d5fd2e2c + 0x18, std::uint8_t(2));
    }
}
void observe_entity_command_recharge(EntityCommandOwner& owner, std::int32_t) noexcept {
    owner.state.prediction_stamp = read<std::uint32_t>(owner.bindings.prediction_time);
}
void repair_entity_prediction_fields(EntityCommandOwner& owner, void* player,
                                     const prediction::CommandHistoryState& record) {
    repair_predicted_fields(owner, player, record);
}
float quantize_entity_prediction_scalar(float value) noexcept {
    return quantized_prediction_scalar(value);
}
void begin_entity_prediction(EntityCommandOwner& owner, void* player, const Command& command) {
    const auto& b = owner.bindings;
    const auto& o = b.offsets;
    if (!(read<std::uint32_t>(player, o.player_flags_38ca5228) & 8u) &&
        read<std::uint8_t>(player, o.state_043fa575) ==
            read<std::uint8_t>(player, o.state_334b460c)) {
        const auto interpolated = read<Vector>(player, o.interpolated_origin_6610746e);
        const auto network = read<Vector>(player, o.network_origin);
        const auto x = interpolated[0] - network[0], y = interpolated[1] - network[1],
                   z = interpolated[2] - network[2];
        if (((y * y + x * x) + z * z) < 4096.0f)
            write(player, o.interpolated_origin_6610746e, network);
    }
    const auto previous = previous_number(command.number);
    restore_revolver(owner, player, previous);
    if (convar_float(b.interpolation_enabled) != 0.0f &&
        read<std::int32_t>(b.prediction, 0x1C) == 0 &&
        read<std::int32_t>(b.prediction, 0x20) == 0) {
        const auto& entry = owner.state.history[previous % 150];
        if (entry.command_number == previous)
            repair_predicted_fields(owner, player, entry.state);
    }
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: entity_command_reject.cpp
// ============================================================================

namespace skeet::client {
void reject_entity_prediction(EntityCommandOwner& owner, void* player,
                              const prediction::PlayerCommandContext& context) {
    // 433B2674 reads context +8: the embedded command's number, rather than
    // context +68 or the first word of a standalone Command.
    const auto previous = std::bit_cast<std::int32_t>(std::uint32_t(context.command.number) - 1U);
    const auto& prior = owner.state.history[previous % 150];
    if (prior.command_number == previous)
        repair_entity_prediction_fields(owner, player, prior.state);

    // 433B2A2B..433B2F30 is the same live entity/weapon producer as the finish
    // observer's tail at 43382AE1. Read the context number again after repair,
    // and let that producer preserve existing flags when the slot matches.
    capture_entity_prediction_history(owner, player, context.command.number);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: entity_command_runtime_owner.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::client {
namespace {
std::ptrdiff_t entity_cmd_field(ClientRuntimeStorage& storage, std::uint32_t salt) {
    const auto& globals = storage.offsets().globals;
    for (const auto& recipe : engine::build_receive_offset_recipes())
        if (recipe.initial_xor_key == salt)
            return engine::DecodedOffset(globals, recipe.pointer_identity, recipe.xor_key_identity,
                                         salt, 0);
    for (const auto& recipe : engine::build_data_map_offset_recipes())
        if (recipe.initial_xor_key == salt)
            return engine::DecodedOffset(globals, recipe.pointer_identity, recipe.xor_key_identity,
                                         salt, 0);
    struct Derived {
        std::uint32_t pointer, key, salt;
    };
    constexpr std::array derived{Derived{0x43468368u, 0x43467FACu, 0x3B1F6ECCu},
                                 Derived{0x4346757Cu, 0x4346707Cu, 0x043FA575u},
                                 Derived{0x43467EBCu, 0x43467038u, 0x432B1F5Bu},
                                 Derived{0x43467A88u, 0x43467160u, 0x0D560FACu},
                                 Derived{0x4346828Cu, 0x43468D2Cu, 0x9E2078A1u}};
    for (const auto& recipe : derived)
        if (recipe.salt == salt)
            return engine::DecodedOffset(globals, recipe.pointer, recipe.key, salt, 0);
    throw std::invalid_argument("Entity command field has no recovered receive/datamap recipe");
}
EntityCommandOffsets offsets(ClientRuntimeStorage& s) {
    const auto f = [&](std::uint32_t salt) { return entity_cmd_field(s, salt); };
    return {
        f(0x5DF1A4BC), f(0x3B1F6ECC), f(0x3D10B94C), f(0x5AE32976), f(0x1BAE1168), f(0x48442721),
        f(0xC246315E), f(0x2F33C681), f(0xB5A66556), f(0xFB7E3404), f(0x0317EEDE), f(0xCD812654),
        f(0x5C9F0FBB), f(0x43D0EFC4), f(0x9D920E35), f(0xCDE290D2), f(0x8B5586F6), f(0xA9EE6062),
        f(0x7501CB91), f(0x8D1BD8D0), f(0xE2FBE695), f(0x8DBC9874), f(0x68140CD8), f(0xD5FD2E2C),
        f(0x38CA5228), f(0x043FA575), f(0x334B460C), f(0x6610746E), f(0x66146802), f(0x00A5BD68),
        f(0x6B1A59B5), f(0x50F9CCC8), f(0x8CFC633A), f(0x7F5ED2D0), f(0xBCB263A5), f(0x9E2078A1),
        f(0xCA5EC698), f(0x0176A381), f(0x6777C9CE), f(0xD2563274), f(0x15335AAC), f(0x4013FC03),
        f(0xE865A877), f(0x3B93667B), f(0xC6C78C82), f(0x6E9852D6), f(0x432B1F5B), f(0x0D560FAC),
        f(0x71DCEE42), f(0x2F4E97C4), f(0x3B704B41), f(0x695FF325), f(0x47A6F50C), f(0x01AA449D),
        f(0xA116369A), f(0xA25D8271)};
}
engine::OwnerRegistryBinding registry(ClientRuntimeStorage& s, std::uint32_t cell) {
    return {s.owner_registries().at(cell), s.memory()};
}
ClientRuntimeStorage& checked(ClientRuntimeStorage& s, const EngineRuntimeBindings& engine,
                              ConfigurationRuntimeOwner& configuration,
                              HistoryRuntimeOwner& history, LocalAnimationRuntimeOwner& local,
                              CommandCaptureRuntimeOwner& capture,
                              EntityCommandRuntimeImports imports) {
    auto& data = s.data();
    if (&engine.variables.data() != &data || &configuration.storage() != &s ||
        &history.storage() != &s || &local.storage() != &s || &local.history() != &history ||
        &capture.storage() != &s ||
        &local.local_hooks().bindings.view_angles !=
            &data.native_cell<std::array<float, 3>>(0x4347BD30u))
        throw std::invalid_argument(
            "Entity command contexts require canonical history/local animation/capture/settings owners");
    if (!imports.ceiling || !imports.exponential || !s.offsets().words ||
        s.offsets().globals.pointers.empty() || !data.native_cell<void*>(0x4346A894u) ||
        !data.native_cell<void*>(0x43467B04u) || !data.native_cell<float*>(0x43467EB8u) ||
        !data.native_cell<float*>(0x4346A338u) || !data.native_cell<float*>(0x4346A334u) ||
        !data.native_cell<std::int32_t*>(0x43475B2Cu) ||
        !data.native_cell<std::int32_t*>(0x434757DCu))
        throw std::invalid_argument(
            "Entity command contexts require acquired offsets, clocks, input and CRT imports");
    for (const auto cell : {0x4346A514u, 0x4346A890u, 0x43469264u, 0x4346A614u}) {
        bool found = false;
        const auto manifest = engine::build_interface_manifest();
        for (std::size_t i = 0; i < manifest.size(); ++i)
            if (manifest[i].destination_cell == cell) {
                if (!data.native_cell<void*>(cell) ||
                    data.native_cell<void*>(cell) != engine.interfaces.objects[i])
                    throw std::invalid_argument(
                        "Entity command interface differs from its acquired engine object");
                found = true;
                break;
            }
        if (!found)
            throw std::logic_error(
                "Entity command interface is absent from its acquisition manifest");
    }
    return s;
}
}
EntityCommandRuntimeImports native_entity_command_runtime_imports() {
    const auto crt = GetModuleHandleW(L"msvcrt.dll");
    auto* ceiling = crt ? GetProcAddress(crt, "ceil") : nullptr;
    if (!ceiling)
        throw std::runtime_error("Entity command model categories require loaded msvcrt!ceil");
    return {reinterpret_cast<double (*)(double)>(ceiling), animation::native_sse_exponential};
}
struct EntityCommandRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    legit::RecoilDecayBindings recoil;
    EntityCommandState state{};
    EntityCommandBindings bindings;
    EntityCommandOwner command;
    EntityCommandFactoryBindings factory;
    Data(ClientRuntimeStorage& source, const EngineRuntimeBindings& engine,
         ConfigurationRuntimeOwner& config, HistoryRuntimeOwner& history,
         LocalAnimationRuntimeOwner& local, CommandCaptureRuntimeOwner& capture,
         EntityCommandRuntimeImports imports)
        : storage(checked(source, engine, config, history, local, capture, imports)),
          recoil{engine.variables.at(0x43468D8Cu), engine.variables.at(0x43467BB4u),
                 engine.variables.at(0x4346A480u), *source.data().native_cell<float*>(0x4346A338u),
                 imports.exponential},
          bindings{nullptr,
                   nullptr,
                   source.data().native_cell<void*>(0x4346A890u),
                   source.data().native_cell<void*>(0x4346A894u),
                   source.data().native_cell<void*>(0x4346A614u),
                   engine.variables.at(0x43468CACu),
                   engine.variables.at(0x4346A850u),
                   engine.variables.at(0x4346794Cu),
                   nullptr,
                   &source.data().native_cell<std::int32_t>(0x4347BDACu),
                   &source.data().native_cell<std::int32_t>(0x4347BD78u),
                   &source.data().native_cell<std::uint8_t>(0x434758B0u),
                   &source.data().native_cell<std::uint8_t>(0x43475BD2u),
                   std::ref(source.data().native_cell<void (*)()>(0x43468BF0u)),
                   source.data().native_cell<const std::int32_t*>(0x434758D8u),
                   source.data().native_cell<const std::int32_t*>(0x43475794u),
                   source.data().native_cell<const std::int32_t*>(0x43475B2Cu),
                   source.data().native_cell<const std::int32_t*>(0x434757DCu),
                   &source.data().native_cell<std::int32_t>(0x43468290u),
                   source.data().native_cell<void*>(0x43467B04u),
                   source.data().native_cell<float*>(0x4346A334u),
                   source.data().native_cell<float*>(0x4346A338u),
                   source.data().native_cell<float*>(0x43467EB8u),
                   &recoil,
                   &local.local_hooks().bindings.view_angles,
                   source.data().native_cell<RestorePredictionFrame>(0x4346D660u),
                   source.data().native_cell<engine::MemberFunction<std::int32_t, std::int32_t>>(
                       0x4346A364u),
                   &source.data().native_cell<std::int32_t>(0x4347C9ECu),
                   &source.data().native_cell<std::int32_t>(0x43467114u),
                   &source.data().native_cell<std::uint32_t>(0x434759C0u),
                   &source.data().native_cell<void*>(0x43467478u),
                   &source.prediction_observers(),
                   source.data().native_cell<PredictionHistoryOwner*>(0x434759B4u),
                   &history.players().cache(),
                   {source.data().native_cell<std::uint8_t>(0x43468D7Cu),
                    source.data().native_cell<const std::int32_t*>(0x43475B2Cu),
                    source.data().native_cell<std::array<animation::AnimationAngleSample, 2>>(
                        0x434759DCu)},
                   &source.data().native_cell<EntityAngleConfirmation>(0x43479794u),
                   source.data().native_cell<std::array<std::uint32_t, 3>>(0x43474580u),
                   offsets(source)},
          command{state, bindings},
          factory{{registry(source, 0x43475B78u), source.command_phases(),
                   source.prediction_observers(), source.data_update_observers(),
                   source.entity_observers(), registry(source, 0x43475770u),
                   source.recharge_observers()},
                  command,
                  bindings,
                  source.data_owner(),
                  source.data().native_cell<void*>(0x4346A514u),
                  source.data().native_cell<void*>(0x43469264u),
                  config.rage_settings().global.enabled.value,
                  imports.ceiling} {
        engine::initialize_build_prediction_scratch_bindings(storage.data());
    }
};
EntityCommandRuntimeOwner::EntityCommandRuntimeOwner(
    ClientRuntimeStorage& s, const EngineRuntimeBindings& e, ConfigurationRuntimeOwner& c,
    HistoryRuntimeOwner& h, LocalAnimationRuntimeOwner& l, CommandCaptureRuntimeOwner& capture)
    : EntityCommandRuntimeOwner(s, e, c, h, l, capture, native_entity_command_runtime_imports()) {}
EntityCommandRuntimeOwner::EntityCommandRuntimeOwner(
    ClientRuntimeStorage& s, const EngineRuntimeBindings& e, ConfigurationRuntimeOwner& c,
    HistoryRuntimeOwner& h, LocalAnimationRuntimeOwner& l, CommandCaptureRuntimeOwner& capture,
    EntityCommandRuntimeImports imports)
    : data_(std::make_unique<Data>(s, e, c, h, l, capture, imports)) {}
EntityCommandRuntimeOwner::~EntityCommandRuntimeOwner() = default;
ClientRuntimeStorage& EntityCommandRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
EntityCommandState& EntityCommandRuntimeOwner::state() noexcept {
    return data_->state;
}
EntityCommandOwner& EntityCommandRuntimeOwner::command() noexcept {
    return data_->command;
}
EntityCommandBindings& EntityCommandRuntimeOwner::bindings() noexcept {
    return data_->bindings;
}
EntityCommandFactoryBindings& EntityCommandRuntimeOwner::factory_bindings() noexcept {
    return data_->factory;
}
void EntityCommandRuntimeOwner::initialize_prediction_scratch(bool local_creation) {
    auto& storage = data_->storage;
    entity_command_detail::initialize_prediction_scratch(
        storage.data().native_cell<EntityCommandOwner*>(0x43475898u), storage.data(),
        storage.offsets().globals, local_creation);
}
void EntityCommandRuntimeOwner::bind_initializers(NativeStartupSources& sources) {
    if (&sources.data() != &storage().data())
        throw std::invalid_argument("Entity command startup requires canonical source data");
    bind_native_entity_command_factory(sources, data_->factory);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: entity_command_scratch.cpp
// ============================================================================

namespace skeet::client::entity_command_detail {
namespace {
/* deduplicated helper */
/* deduplicated helper */
void purge_memory(void* memory, const engine::NativeDataImage& image) { //4331E2CD
    if (read<std::int32_t>(memory, 8) < 0)
        return;
    if (auto* allocation = read<void*>(memory)) {
        engine::invoke<void>(image.native_cell<void*>(0x43467204u), 5, allocation);
        write(memory, 0, static_cast<void*>(nullptr));
    }
    write(memory, 4, std::uint32_t(0));
}
void purge_vector(void* vector, const engine::NativeDataImage& image) { //433CF41B
    write(vector, 0xC, std::uint32_t(0));
    purge_memory(vector, image);
    write(vector, 0x10, read<void*>(vector));
    purge_memory(vector, image);
}
void release_record(void* record, const engine::NativeDataImage& image) { //4336B845
    auto* block = static_cast<std::byte*>(record) + 0x60;
    for (unsigned count = 0; count < 2; ++count) {
        block -= 0x30;
        purge_vector(block + 0x1C, image);
        purge_vector(block, image);
    }
}
void release_group(std::uint32_t address, const engine::NativeDataImage& image) {
    auto*& destination = image.native_cell<void*>(address);
    if (destination) {
        const auto& backup = image.native_cell<std::array<std::byte, 0x18>>(address + 4);
        std::memcpy(destination, backup.data(), backup.size());
    }
    auto*& record = image.native_cell<void*>(address + 0x30);
    if (record) {
        release_record(record, image);
        auto* pool = image.native_cell<void*>(0x4346D658u);
        if (record) {
            auto* previous = read<void*>(pool, 0x18);
            write(pool, 0xC, std::uint32_t(read<std::uint32_t>(pool, 0xC) - 1u));
            write(record, 0, previous);
            write(pool, 0x18, record);
        }
        record = nullptr;
    }
    auto*& allocation = image.native_cell<void*>(address + 0x1C);
    if (allocation) {
        engine::invoke<void>(image.native_cell<void*>(0x43467204u), 5, allocation);
        allocation = nullptr;
    }
    image.native_cell<std::array<std::byte, 0x18>>(address + 0x1C).fill(std::byte{});
}
struct AdditionalField {
    std::uint32_t offset;
    engine::DataMapField field;
};
AdditionalField additional_field(const engine::NativeOffsetGlobals& globals, std::uint32_t pointer,
                                 std::uint32_t key, std::uint32_t salt, const char* name,
                                 std::int32_t type, std::uint32_t bytes, std::uint32_t tolerance) {
    AdditionalField result{static_cast<std::uint32_t>(std::ptrdiff_t(
                               engine::DecodedOffset(globals, pointer, key, salt, 0))),
                           {}};
    result.field.type = type;
    result.field.name = name;
    write(&result.field, 0xc, std::uint16_t(1));
    write(&result.field, 0xe, std::uint16_t(0x100));
    write(&result.field, 0x20, bytes);
    write(&result.field, 0x2c, tolerance);
    return result;
}
engine::DataMap* data_map(const engine::NativeDataImage& image, std::uint32_t vtable_cell) {
    auto** table = image.native_cell<void**>(vtable_cell);
    const auto* getter = static_cast<const std::byte*>(table[17]);
    // Native inspects this six-byte constant-return getter; it never calls it.
    if (!getter || read<std::uint8_t>(getter) != 0xb8 || read<std::uint8_t>(getter, 5) != 0xc3)
        return nullptr;
    return read<engine::DataMap*>(getter, 1);
}
void rebuild_map(engine::DataMap* map, const engine::NativeDataImage& image) {
    image.native_cell<engine::MemberFunction<void>>(0x4346D65Cu)(map);
}
void append_fields(engine::DataMap* map, std::uint32_t destination,
                   std::span<const AdditionalField> extra, const engine::NativeDataImage& image) {
    if (!image.native_cell<void*>(0x4346D658u) || !image.native_cell<void*>(0x4346D65Cu) || !map)
        return;
    image.native_cell<engine::DataMap*>(destination) = map;
    auto& backup = image.native_cell<std::array<std::byte, 0x18>>(destination + 4);
    std::memcpy(backup.data(), map, backup.size());
    const auto original_count = std::bit_cast<std::uint32_t>(map->field_count);
    const auto new_count = original_count + static_cast<std::uint32_t>(extra.size());
    std::uint32_t names_offset = new_count * 0x3cu;
    auto bytes = names_offset;
    for (const auto& field : extra)
        if (field.field.name)
            bytes += static_cast<std::uint32_t>(std::strlen(field.field.name) + 1u);
    auto* allocation = static_cast<std::byte*>(
        engine::invoke<void*>(image.native_cell<void*>(0x43467204u), 1, bytes));
    if (!allocation)
        return;
    std::memcpy(allocation, map->fields, original_count * 0x3cu);
    auto* appended = allocation + std::bit_cast<std::uint32_t>(map->field_count) * 0x3cu;
    for (const auto& field : extra) {
        std::memcpy(appended, &field.field, 0x3c);
        write(appended, 8, field.offset);
        if (field.field.name) {
            const auto length = static_cast<std::uint32_t>(std::strlen(field.field.name) + 1u);
            auto* const name = reinterpret_cast<char*>(allocation + names_offset);
            std::memcpy(name, field.field.name, length);
            names_offset += length;
            write(appended, 4, name);
        }
        appended += 0x3c;
    }
    auto& clone = image.native_cell<engine::DataMap>(destination + 0x1c);
    std::memcpy(&clone, map, 0x18);
    clone.optimized = nullptr;
    clone.fields = reinterpret_cast<engine::DataMapField*>(allocation);
    clone.field_count = std::bit_cast<std::int32_t>(new_count);
    rebuild_map(&clone, image);
    if (clone.optimized)
        std::memcpy(map, &clone, 0x18);
}
// Complete43417ED1 first-match traversal. Only the matching descriptor is
// needed here; native's accompanying accumulated offset is not consumed.
engine::DataMapField* find_field(engine::DataMap* map, std::uint32_t hash) {
    for (; map; map = map->base) {
        for (std::int32_t index = 0; index < map->field_count; ++index) {
            auto& field = map->fields[index];
            if (field.name && engine::netvar_name_hash(field.name) == hash)
                return &field;
            if (field.type == 10)
                if (auto* found = find_field(field.embedded, hash))
                    return found;
        }
    }
    return nullptr;
}
void update_weapon_fields(engine::DataMap* map, const engine::NativeDataImage& image) {
    if (!map)
        return;
    bool changed = false;
    for (const auto hash : {0x1860C50Du, 0x2489A29Du}) {
        auto* field = find_field(map, hash);
        if (!field)
            continue;
        auto flags = read<std::uint16_t>(field, 0xe);
        if (flags & 0x100u)
            continue;
        flags = std::uint16_t((flags | 0x100u) & ~0x400u);
        write(field, 0xe, flags);
        write(field, 0x2c, std::uint32_t(0x3a83126f));
        changed = true;
    }
    if (!changed || !map->optimized)
        return;
    auto* const previous_record = map->optimized;
    map->optimized = nullptr;
    rebuild_map(map, image);
    auto* const pool = image.native_cell<void*>(0x4346D658u);
    auto* const previous_free = read<void*>(pool, 0x18);
    write(pool, 0xc, std::uint32_t(read<std::uint32_t>(pool, 0xc) - 1u));
    write(previous_record, 0, previous_free);
    write(pool, 0x18, previous_record);
}
}
void release_prediction_scratch(EntityCommandOwner* const& active,
                                const engine::NativeDataImage& image) {
    if (!active)
        return;
    if (image.native_cell<void*>(0x4346D658u) && image.native_cell<std::uint32_t>(0x4346D65Cu))
        release_group(0x4347BD90u, image);
    // The first group calls actual allocator methods. Native reloads all
    // three gate inputs before considering the independent second group.
    if (active && image.native_cell<void*>(0x4346D658u) &&
        image.native_cell<std::uint32_t>(0x4346D65Cu))
        release_group(0x4347BD5Cu, image);
}
void initialize_prediction_scratch(EntityCommandOwner* const& active,
                                   const engine::NativeDataImage& image,
                                   const engine::NativeOffsetGlobals& offsets,
                                   bool local_creation) {
    // Devirtualized433E33D3. The owner and both guard words are consumed before
    // either branch, including the one-shot connection notification path.
    auto* const owner = active;
    if (!owner || image.native_cell<void*>(0x4347BDACu) || image.native_cell<void*>(0x4347BD78u))
        return;
    if (!local_creation) {
        auto& checked = image.native_cell<std::uint8_t>(0x434758B0u);
        if (checked)
            return;
        checked = 1;
        if (!engine::invoke<bool>(image.native_cell<void*>(0x4346A514u), 27))
            return;
        if (auto entry = image.native_cell<void (*)()>(0x43468BF0u))
            entry();
        else
            image.native_cell<std::uint8_t>(0x43475BD2u) = 1;
        return;
    }
    if (owner->prediction_names.size() != 8)
        throw std::logic_error(
            "Prediction scratch requires its eight constructed entity-owner strings");
    const auto name = [&](std::size_t index) { return owner->prediction_names[index].begin; };
    const std::array player{
        additional_field(offsets, 0x4346791c, 0x43467bd4, 0x7f5ed2d0, name(0), 1, 4, 0x3c23d70a),
        additional_field(offsets, 0x43468354, 0x4346852c, 0x66146802, name(1), 1, 4, 0x3a83126f),
        additional_field(offsets, 0x4346799c, 0x4346a2ec, 0xbcb263a5, name(2), 6, 1, 0),
        additional_field(offsets, 0x434679cc, 0x43467164, 0x00a5bd68, name(3), 6, 1, 0),
        additional_field(offsets, 0x43468ee4, 0x4346853c, 0x52899df3, name(6), 5, 4, 0)};
    const std::array grenade{
        additional_field(offsets, 0x43467aa4, 0x43467bf4, 0x3b93667b, name(4), 1, 4, 0),
        additional_field(offsets, 0x43467a6c, 0x4346a658, 0xe865a877, name(5), 6, 1, 0)};
    append_fields(data_map(image, 0x43467D54u), 0x4347BD90u, player, image);
    append_fields(data_map(image, 0x4346A43Cu), 0x4347BD5Cu, grenade, image);
    update_weapon_fields(data_map(image, 0x43467F84u), image);
    write(image.native_cell<void*>(0x43469264u), 0x8c, std::uint8_t(1));
    write(image.native_cell<void*>(0x43469264u), 0x9c, std::uint32_t(0));
}
} // namespace skeet::client::entity_command_detail

// ============================================================================
// Subcomponent: entity_data_update.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */
/* deduplicated helper */
using Vector = std::array<float, 3>;
const Command* command_at(const EntityCommandBindings& b, std::int32_t number) {
    const auto& command = read<const Command*>(b.input, 0xF0)[std::uint32_t(number) % 150u];
    return command.number == number ? &command : nullptr;
}
float quantized_angle(float value) noexcept { //full4337F9D3
    const auto integer =
        _mm_cvttss_si32(_mm_mul_ss(_mm_set_ss(value), _mm_set_ss(182.0444488525390625f)));
    return float(std::uint32_t(integer) & 0xFFFFu) * 0.0054931640625f;
}
bool near_vector(const Vector& actual, const Vector& expected, float tolerance) noexcept {
    for (unsigned i = 0; i < 3; ++i)
        if (!(std::fabs(actual[i] - expected[i]) <= tolerance))
            return false;
    return true;
}
// The two native diagnostic blocks43403B77..3FAF and43404CFD..5290
// compare the same history fields, with their supplied entry/tick identity.
std::uint32_t difference_flags(EntityCommandOwner& owner, void* player,
                               const prediction::CommandHistoryState& predicted,
                               std::int32_t number, bool compare_angles) {
    const auto& b = owner.bindings;
    const auto& o = b.offsets;
    std::uint32_t flags =
        read<std::int32_t>(&predicted) != read<std::int32_t>(player, o.tick_base) ? 1u : 0u;
    if (!near_vector(read<Vector>(player, o.vector_5ae32976), read<Vector>(&predicted, 4),
                     0.03125f))
        flags |= 2u;
    if (!near_vector(read<Vector>(player, o.vector_1bae1168), predicted.recoil_angles, 0.03125f))
        flags |= 4u;
    if (!near_vector(read<Vector>(player, o.vector_48442721), read<Vector>(&predicted, 0x1C),
                     0.03125f))
        flags |= 8u;
    if (!(std::fabs(read<float>(player, o.view_offset + 8) - read<float>(&predicted, 0x28)) <=
          0.25f))
        flags |= 0x10u;
    const auto position = read<Vector>(player, o.network_origin);
    const auto prior = read<Vector>(&predicted, 0x2C);
    const auto x = position[0] - prior[0], y = position[1] - prior[1], z = position[2] - prior[2];
    if ((y * y + x * x) + z * z >= 0.0625f)
        flags |= 0x20u;
    if (read<std::int32_t>(player, o.shots_fired_8cfc633a) != read<std::int32_t>(&predicted, 0x3C))
        flags |= 0x40u;
    if (!(std::fabs(read<float>(player, o.field_66146802) - read<float>(&predicted, 0x4C)) <=
          0.000099999997473787516f))
        flags |= 0x400u;
    const auto handle = read<std::uint32_t>(player, o.active_weapon);
    if (predicted.weapon_identity != handle)
        flags |= 0x80u;
    else if (handle != UINT32_MAX)
        if (auto* weapon = engine::invoke<void*>(b.entity_list, 4, handle)) {
            if (!(std::fabs(read<float>(weapon, o.weapon_field_d2563274) -
                            read<float>(&predicted, 0x64)) <= 0.000099999997473787516f))
                flags |= 0x100u;
            if (read<float>(&predicted, 0x60) != read<float>(weapon, o.weapon_field_0176a381))
                flags |= 0x200u;
            if (!(std::fabs(read<float>(weapon, o.weapon_field_6777c9ce) -
                            read<float>(&predicted, 0x44)) <= 0.000099999997473787516f))
                flags |= 0x1000u;
            auto* model = entity_command_view_model(
                owner, player, read<std::int32_t>(weapon, o.weapon_viewmodel_index_c6c78c82));
            if (model && read<std::uint32_t>(&predicted, 0x74) !=
                             read<std::uint32_t>(model, o.viewmodel_695ff325))
                flags |= 0x2000u;
        }
    if (const auto* command = command_at(b, number)) {
        const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
        auto* cached = b.players->get(index);
        if (compare_angles) {
            auto angles = command->angles;
            b.angle_history.select(number, angles);
            const auto yaw = quantized_angle(angles[1]);
            const auto pitch = quantized_angle(angles[0]);
            if (pitch != read<float>(&cached->native, 0x1A8) || yaw != cached->native.reference_yaw)
                flags |= 0x800u;
        }
    }
    return flags;
}
void repair_weapon_activity(EntityCommandOwner& owner, void* player) {
    const auto& b = owner.bindings;
    const auto& o = b.offsets;
    const auto handle = read<std::uint32_t>(player, o.active_weapon);
    if (handle == UINT32_MAX)
        return;
    auto* weapon = engine::invoke<void*>(b.entity_list, 4, handle);
    if (!weapon || read<std::int32_t>(weapon, o.weapon_field_01aa449d) != 2)
        return;
    auto* viewmodel = entity_command_view_model(
        owner, player, read<std::int32_t>(weapon, o.weapon_viewmodel_index_c6c78c82));
    if (!viewmodel)
        return;
    const auto model_weapon = read<std::uint32_t>(viewmodel, o.viewmodel_weapon_a116369a);
    if (model_weapon == UINT32_MAX)
        return;
    auto* resolved = engine::invoke<void*>(b.entity_list, 4, model_weapon);
    if (resolved != weapon)
        return;
    const auto sequence = read<std::int32_t>(viewmodel, o.viewmodel_sequence_a25d8271);
    if (sequence < 0)
        return;
    const auto activity = b.sequence_activity ? b.sequence_activity(viewmodel, sequence) : -1;
    if (activity == 0xB7 || activity == 0x1FA)
        write(weapon, o.weapon_state_9e2078a1, activity);
}
void reset_prediction_history(EntityCommandOwner& owner) noexcept {
    for (unsigned i = 0; i < 150; ++i)
        owner.state.revolver[i].command_number = -1;
    for (unsigned i = 0; i < 150; ++i)
        owner.state.history[i].command_number = -1;
    auto& saved = owner.state.data_update;
    saved.recoil_angles = {0, 0, 0};
    saved.scaled_recoil = 0;
    saved.field_7f5ed2d0 = 0x3F800000u;
    owner.state.last_choked_shot = -1;
    *owner.bindings.data_update_reset_marker = -1;
}
void record_recoil_mismatch(EntityCommandOwner& owner, void* player) {
    const auto& b = owner.bindings;
    const auto& o = b.offsets;
    *b.data_update_reset_marker = engine::invoke<std::int32_t>(b.engine_client, 205);
    if (!(read<std::uint8_t>(player, o.ground_flags) & 1u))
        return;
    const auto handle = read<std::uint32_t>(player, o.active_weapon);
    if (handle == UINT32_MAX)
        return;
    auto* weapon = engine::invoke<void*>(b.entity_list, 4, handle);
    if (weapon && network::classified_packet_weapon(
                      read<std::uint16_t>(weapon, o.weapon_definition), b.weapon_classification))
        *b.scheduler_flags = (*b.scheduler_flags & ~0x400u) | 0x200u;
}
void merge_acknowledged_record(EntityCommandOwner& owner, void* player,
                               prediction::CommandHistoryEntry& entry, std::uint32_t flags,
                               std::int32_t number) {
    const auto& b = owner.bindings;
    const auto& o = b.offsets;
    if (flags & 1u) {
        if (flags != 1u)
            return;
        if (!(std::fabs(*b.prediction_time - std::bit_cast<float>(owner.state.prediction_stamp)) <=
              0.2f) &&
            !(entry.state.flags & 8u))
            return;
    }
    const auto merge_vector = [&](std::ptrdiff_t record_offset, std::ptrdiff_t player_offset,
                                  std::uint32_t mismatch) {
        if (flags & mismatch)
            write(&entry.state, record_offset, read<Vector>(player, player_offset));
        else
            write(player, player_offset, read<Vector>(&entry.state, record_offset));
    };
    merge_vector(4, o.vector_5ae32976, 2u);
    merge_vector(0x10, o.vector_1bae1168, 4u);
    merge_vector(0x1C, o.vector_48442721, 8u);
    if (flags & 0x10u)
        write(&entry.state, 0x28, read<std::uint32_t>(player, o.view_offset + 8));
    else
        write(player, o.view_offset + 8, read<std::uint32_t>(&entry.state, 0x28));
    if (flags & 0x20u)
        write(&entry.state, 0x2C, read<Vector>(player, o.network_origin));
    else {
        write(player, o.field_fb7e3404 + 4, read<std::uint32_t>(&entry.state, 0x50));
        write(player, o.field_fb7e3404 + 8, read<std::uint32_t>(&entry.state, 0x54));
    }
    if (flags & 0x400u)
        write(&entry.state, 0x4C, read<std::uint32_t>(player, o.field_66146802));
    const auto scalar = read<float>(&entry.state, 0x38);
    if (read<float>(player, o.scalar_2f33c681) == quantize_entity_prediction_scalar(scalar))
        write(player, o.scalar_2f33c681, scalar);
    else
        write(&entry.state, 0x38, read<std::uint32_t>(player, o.scalar_2f33c681));
    if (flags & 0x34u) {
        const auto last = *b.last_outgoing;
        for (unsigned i = 0; i < 150 && number <= last; ++i) {
            auto& current = owner.state.history[number % 150];
            if (current.command_number != number)
                break;
            current.state.flags |= 2u;
            number = std::bit_cast<std::int32_t>(std::uint32_t(number) + 1u);
        }
        invalidate_command_prediction(b.prediction);
    }
}
}
void reset_entity_prediction_history(EntityCommandOwner& owner) noexcept {
    reset_prediction_history(owner);
}
void process_entity_data_update_capture(EntityCommandOwner& owner, void* player, std::int32_t) {
    const auto& offsets = owner.bindings.offsets;
    auto& saved = owner.state.data_update;
    saved.field_7f5ed2d0 = read<std::uint32_t>(player, offsets.field_7f5ed2d0);
    for (unsigned i = 0; i < 3; ++i)
        saved.recoil_angles[i] = read<std::uint32_t>(player, offsets.vector_1bae1168 + i * 4);
    saved.scaled_recoil = read<std::uint32_t>(player, offsets.field_66146802);
    saved.tick_base = read<std::int32_t>(player, offsets.tick_base);
    saved.health = engine::invoke<std::int32_t>(player, 122);
    saved.field_47a6f50c = read<std::uint32_t>(player, offsets.field_47a6f50c);
}
std::uint32_t entity_data_update_difference_flags(EntityCommandOwner& owner, void* player,
                                                  const prediction::CommandHistoryState& predicted,
                                                  std::int32_t number, bool compare_angles) {
    return difference_flags(owner, player, predicted, number, compare_angles);
}
float entity_data_update_quantized_angle(float angle) noexcept {
    return quantized_angle(angle);
}
void process_entity_data_update_finish(EntityCommandOwner& owner, void* player,
                                       std::int32_t update) {
    const auto& b = owner.bindings;
    const auto& o = b.offsets;
    repair_weapon_activity(owner, player);
    if (update == 0 && *b.data_update_backup_mode != 2) {
        reset_entity_prediction_history(owner);
        return;
    }
    if (read<std::uint8_t>(player, o.rejected_9d920e35))
        return;
    const auto acknowledged = engine::invoke<std::int32_t>(b.engine_client, 205);
    const auto delta = std::bit_cast<std::int32_t>(std::uint32_t(acknowledged) -
                                                   std::uint32_t(*b.old_acknowledged));
    const auto original_recoil = read<std::uint32_t>(player, o.vector_1bae1168);
    const auto captured_tick = owner.state.data_update.tick_base;
    const auto current_tick = read<std::int32_t>(player, o.tick_base);
    const bool changed = std::bit_cast<float>(owner.state.data_update.field_7f5ed2d0) !=
                         read<float>(player, o.field_7f5ed2d0);
    EntityDataUpdateWork work{acknowledged, delta,           captured_tick,
                              current_tick, original_recoil, changed};
    reconcile_entity_data_update_ack(owner, player, work);
    auto& entry = owner.state.history[acknowledged % 150];
    work.current_record = &entry;
    if (entry.command_number == acknowledged) {
        const auto flags = difference_flags(owner, player, entry.state, acknowledged, true);
        if (flags & 4u)
            record_recoil_mismatch(owner, player);
        merge_acknowledged_record(owner, player, entry, flags, acknowledged);
    }
    finish_entity_data_update_tail(owner, player, work);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: entity_data_update_ack.cpp
// ============================================================================

namespace skeet::client {
namespace {
using Vector = std::array<float, 3>;
/* deduplicated helper */
/* deduplicated helper */
/* deduplicated helper */
/* deduplicated subtract */
prediction::CommandHistoryState* history_at(EntityCommandOwner& owner,
                                            std::int32_t number) noexcept {
    auto& entry = owner.state.history[number % 150];
    return entry.command_number == number ? &entry.state : nullptr;
}
/* deduplicated command_at */
/* deduplicated convar_float */
std::int32_t processing_ticks(const EntityCommandBindings& bindings) noexcept {
    auto seconds = _mm_cvtepi32_ps(_mm_cvtsi32_si128(*bindings.process_milliseconds));
    seconds = _mm_div_ss(seconds, _mm_set_ss(1000.0f));
    seconds = _mm_min_ss(_mm_max_ss(seconds, _mm_setzero_ps()), _mm_set_ss(1.0f));
    seconds = _mm_div_ss(seconds, _mm_set_ss(*bindings.interval));
    return _mm_cvttss_si32(_mm_add_ss(seconds, _mm_set_ss(0.5f)));
}
std::int32_t receive_tick(const EntityCommandBindings& bindings) {
    const auto time = engine::invoke<long double>(bindings.engine_client, 14);
    const auto interval = static_cast<long double>(read<float>(bindings.engine_globals, 0x20));
    // FDIV/FADD retain the engine's ST0 return until one final FSTP float.
    volatile float rounded = static_cast<float>(time / interval + 0.5L);
    return _mm_cvttss_si32(_mm_set_ss(rounded));
}
void visit_cached_player(EntityCommandOwner& owner, void* player) {
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    static_cast<void>(owner.bindings.players->get(index));
}

void advance_short_acknowledgement(EntityCommandOwner& owner, void* player,
                                   prediction::CommandHistoryState& predicted, std::int32_t steps) {
    const auto& bindings = owner.bindings;
    const auto& offsets = bindings.offsets;
    if (predicted.weapon_identity != UINT32_MAX) {
        auto* weapon = engine::invoke<void*>(bindings.entity_list, 4, predicted.weapon_identity);
        if (weapon) {
            auto tick = read<std::int32_t>(&predicted);
            const auto saved_accuracy = read<std::uint32_t>(weapon, offsets.weapon_field_d2563274);
            const auto saved_recoil = read<std::uint32_t>(weapon, offsets.weapon_field_6777c9ce);
            const auto saved_time = read<std::uint32_t>(bindings.simulation_time);
            for (std::int32_t iteration = 0; iteration < steps; ++iteration) {
                *bindings.simulation_time = float(tick) * *bindings.interval;
                auto decay = *bindings.recoil_decay;
                decay.interval = *bindings.interval;
                legit::decay_recoil(predicted.recoil_velocity, predicted.recoil_angles, decay);
                engine::invoke<void>(weapon, 484); //actual virtual+790
                write(&predicted, 0x44, read<std::uint32_t>(weapon, offsets.weapon_field_6777c9ce));
                write(&predicted, 0x64, read<std::uint32_t>(weapon, offsets.weapon_field_d2563274));
                tick = add(tick, 1);
            }
            write(bindings.simulation_time, 0, saved_time);
            write(weapon, offsets.weapon_field_6777c9ce, saved_recoil);
            write(weapon, offsets.weapon_field_d2563274, saved_accuracy);
        }
    }
    write(&predicted, 0, read<std::uint32_t>(player, offsets.tick_base));
}

void observe_near_vector(const Vector& actual, const Vector& predicted) noexcept {
    //4340426B/42B1/42F7 only inspect components. They do not restore vectors;
    // keep these comparisons distinct from repair_entity_prediction_fields.
    for (unsigned component = 0; component != 3; ++component)
        if (!(std::fabs(actual[component] - predicted[component]) <= 0.03125f))
            break;
}
void visit_acknowledged_state(EntityCommandOwner& owner, void* player,
                              const prediction::CommandHistoryState& predicted,
                              std::int32_t acknowledged) {
    const auto& bindings = owner.bindings;
    const auto& offsets = bindings.offsets;
    observe_near_vector(read<Vector>(player, offsets.vector_5ae32976), read<Vector>(&predicted, 4));
    observe_near_vector(read<Vector>(player, offsets.vector_1bae1168), predicted.recoil_angles);
    observe_near_vector(read<Vector>(player, offsets.vector_48442721), predicted.recoil_velocity);
    const auto handle = read<std::uint32_t>(player, offsets.active_weapon);
    if (predicted.weapon_identity == handle && handle != UINT32_MAX) {
        if (auto* weapon = engine::invoke<void*>(bindings.entity_list, 4, handle))
            static_cast<void>(entity_command_view_model(
                owner, player,
                read<std::int32_t>(weapon, offsets.weapon_viewmodel_index_c6c78c82)));
    }
    if (command_at(bindings, acknowledged))
        visit_cached_player(owner, player);
}

void retime_acknowledged_history(EntityCommandOwner& owner, void* player,
                                 prediction::CommandHistoryState& predicted,
                                 const EntityDataUpdateWork& work) {
    const auto& bindings = owner.bindings;
    const auto acknowledged = engine::invoke<std::int32_t>(bindings.engine_client, 205);
    auto tick = read<std::int32_t>(player, bindings.offsets.tick_base);
    const auto delta = subtract(acknowledged, *bindings.old_acknowledged);
    if (read<std::int32_t>(bindings.engine_globals, 0x18) != 1 &&
        convar_float(bindings.interpolation_enabled) != 0.0f &&
        (std::uint32_t(delta) & 0xFFFFFFC0u) == 0u) {
        const auto processing = processing_ticks(bindings);
        const auto received = receive_tick(bindings);
        const auto center = add(received, processing);
        const auto captured = add(owner.state.data_update.tick_base, delta);
        if (captured <= add(center, processing) && captured >= received)
            return;
        const auto adjusted =
            add(subtract(read<std::int32_t>(bindings.engine_globals, 0x28), delta), center);
        if (adjusted == -1)
            return;
    }
    const auto received = receive_tick(bindings);
    auto expected = add(add(received, 1), processing_ticks(bindings));
    auto number = add(*bindings.old_acknowledged, 1);
    std::uint32_t inspected = 0;
    while (number <= acknowledged && inspected < 150u) {
        const auto* command = command_at(bindings, number);
        if (!command)
            break;
        if (command->tick == 0x7F7FFFFF)
            expected = subtract(expected, 1);
        ++inspected;
        number = add(number, 1);
    }
    if (expected != tick)
        return;
    const auto choked = *bindings.choked_commands;
    const auto outgoing = *bindings.last_outgoing;
    const auto last = add(choked, outgoing);
    for (number = acknowledged; number <= last; number = add(number, 1)) {
        auto* record = history_at(owner, number);
        if (!record)
            break;
        const auto* command = command_at(bindings, number);
        if (!command)
            break;
        write(record, 0, tick);
        if (command->tick != 0x7F7FFFFF)
            tick = add(tick, 1);
    }
    visit_acknowledged_state(owner, player, predicted, work.acknowledged_command);
}

bool copy_matching_history(EntityCommandOwner& owner, void* player,
                           prediction::CommandHistoryState& destination, std::int32_t number,
                           bool compare_angles) {
    auto* source = history_at(owner, number);
    if (!source)
        return false;
    const auto flags =
        entity_data_update_difference_flags(owner, player, *source, number, compare_angles);
    const auto allowed = compare_angles ? 0x201u : 0xA01u;
    if ((flags & ~allowed) != 0u)
        return false;
    std::memcpy(&destination, source, sizeof(destination)); //actual msvcrt memcpy774FD4A0
    write(&destination, 0, read<std::uint32_t>(player, owner.bindings.offsets.tick_base));
    invalidate_command_prediction(
        owner.bindings.prediction); //433D11F7, before the repeated inline invalidation
    return true;
}
}

void reconcile_entity_data_update_ack(EntityCommandOwner& owner, void* player,
                                      const EntityDataUpdateWork& work) {
    if (work.acknowledged_delta <= 0 && work.captured_tick_base == work.current_tick_base)
        return;
    auto* acknowledged = history_at(owner, work.acknowledged_command);
    if (!acknowledged)
        return;
    if (read<std::int32_t>(acknowledged) == work.current_tick_base)
        return;

    if (work.acknowledged_delta <= 0 && work.captured_tick_base != work.current_tick_base) {
        const auto steps = subtract(work.current_tick_base, work.captured_tick_base);
        if (std::uint32_t(subtract(steps, 1)) <= 14u)
            advance_short_acknowledgement(owner, player, *acknowledged, steps);
    } else {
        const auto flags = entity_data_update_difference_flags(owner, player, *acknowledged,
                                                               work.acknowledged_command);
        if ((flags & ~0x201u) == 0u) {
            retime_acknowledged_history(owner, player, *acknowledged, work);
        } else {
            const auto prior_number =
                subtract(engine::invoke<std::int32_t>(owner.bindings.engine_client, 205), 1);
            if (!copy_matching_history(owner, player, *acknowledged, prior_number, true) &&
                read<std::int32_t>(acknowledged) <
                    read<std::int32_t>(player, owner.bindings.offsets.tick_base)) {
                const auto next_number =
                    add(engine::invoke<std::int32_t>(owner.bindings.engine_client, 205), 1);
                static_cast<void>(
                    copy_matching_history(owner, player, *acknowledged, next_number, false));
            }
        }
    }
    //43404C7E..4CCC repeats the same three writes even after433D11F7.
    invalidate_command_prediction(owner.bindings.prediction);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: entity_data_update_tail.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */
/* deduplicated helper */
const Command* verified_command(const EntityCommandBindings& bindings, std::int32_t number) {
    // Full 4334116B uses CInput +F4 and stride 68, distinct from the +F0 ring.
    const auto* entries = read<const VerifiedCommand*>(bindings.input, 0xF4);
    const auto& command = entries[std::uint32_t(number) % 150U].command;
    return command.number == number ? &command : nullptr;
}
prediction::CommandHistoryState* acknowledged_history(EntityCommandOwner& owner,
                                                      std::int32_t number) {
    // Full 433F0944 uses signed remainder and returns entry +4.
    auto& entry = owner.state.history[number % 150];
    return entry.command_number == number ? &entry.state : nullptr;
}
/* deduplicated recoil_scale */
void confirm_animation_angles(EntityCommandOwner& owner, bool confirmed) {
    const auto& bindings = owner.bindings;
    auto& history = *bindings.angle_confirmation;
    bindings.angle_history.set_active(confirmed);
    auto count = history.count, index = history.index;
    for (std::uint32_t i = 0; i < count; ++i) {
        if (history.values[(index + i) % 3U] != std::uint8_t(confirmed)) {
            count = 0;
            index = 0;
            history.count = 0;
            break;
        }
    }
    if (count != 3U)
        history.count = ++count;
    history.index = (index + 2U) % 3U;
    history.values[history.index] = std::uint8_t(confirmed);
}
void check_choked_shot_angles(EntityCommandOwner& owner, void* player) {
    const auto& bindings = owner.bindings;
    const auto& offsets = bindings.offsets;
    if (bindings.angle_confirmation->count == 3U)
        return;
    if (owner.state.last_choked_shot <= *bindings.old_acknowledged)
        return;
    const auto first_outgoing = engine::invoke<std::int32_t>(bindings.engine_client, 205);
    if (owner.state.last_choked_shot >= first_outgoing ||
        read<std::uint8_t>(player, offsets.rejected_9d920e35))
        return;

    // The second virtual query and both exact-number verified lookups retain
    // their native order; the interface can change its answer between calls.
    const auto outgoing = engine::invoke<std::int32_t>(bindings.engine_client, 205);
    const auto* latest = verified_command(bindings, outgoing);
    if (!latest || std::bit_cast<std::uint32_t>(latest->tick) == 0x7F7FFFFFU)
        return;
    const auto latest_pitch = entity_data_update_quantized_angle(latest->angles[0]);
    const auto latest_yaw = entity_data_update_quantized_angle(latest->angles[1]);
    const auto* fired = verified_command(bindings, owner.state.last_choked_shot);
    if (!fired || std::bit_cast<std::uint32_t>(fired->tick) == 0x7F7FFFFFU)
        return;
    const auto fired_pitch = entity_data_update_quantized_angle(fired->angles[0]);
    const auto fired_yaw = entity_data_update_quantized_angle(fired->angles[1]);
    if (fired_pitch == latest_pitch && fired_yaw == latest_yaw)
        return;
    const auto index = engine::invoke<std::uint32_t>(bindings.engine_client, 12);
    const auto* cached = bindings.players->get(index);
    const auto cached_pitch = read<float>(&cached->native, 0x1A8);
    if (latest_pitch == cached_pitch && latest_yaw == cached->native.reference_yaw)
        confirm_animation_angles(owner, false);
    else if (fired_pitch == cached_pitch && fired_yaw == cached->native.reference_yaw)
        confirm_animation_angles(owner, true);
}
}

void finish_entity_data_update_tail(EntityCommandOwner& owner, void* player,
                                    const EntityDataUpdateWork& work) {
    const auto& bindings = owner.bindings;
    const auto& offsets = bindings.offsets;
    if (work.acknowledged_delta > 0) {
        if (work.acknowledged_delta > 1)
            check_choked_shot_angles(owner, player);
    } else if (*bindings.shot_recording_enabled) {
        const auto saved_pitch = std::bit_cast<float>(owner.state.data_update.recoil_angles[0]);
        if (work.captured_tick_base != work.current_tick_base ||
            saved_pitch != read<float>(player, offsets.vector_1bae1168))
            invalidate_command_prediction(bindings.prediction);
    }

    if (work.field_7f5ed2d0_changed) {
        if (auto* record = acknowledged_history(owner, work.acknowledged_command)) {
            write(record, 0x40, read<std::uint32_t>(player, offsets.field_7f5ed2d0));
            invalidate_command_prediction(bindings.prediction);
        }
    }
    if (work.acknowledged_delta <= 0 && work.captured_tick_base == work.current_tick_base)
        return;

    // Native 43405A2B reloads the live ring base and adds the signed slot
    // offset saved at 43404CED. Its first word is deliberately not checked.
    auto& current = owner.state.history[work.acknowledged_command % 150];
    write(player, offsets.field_b5a66556, (current.state.flags >> 4U) & 7U);
    const float scaled_pitch = read<float>(player, offsets.field_66146802);
    const float captured_scaled = std::bit_cast<float>(owner.state.data_update.scaled_recoil);
    if (captured_scaled != scaled_pitch || *bindings.data_update_reset_marker != -1) {
        *bindings.data_update_reset_marker = -1;
        const auto scale = recoil_scale(bindings.recoil_scale_convar);
        if (scale != 0.0f) {
            if (auto* record = acknowledged_history(owner, work.acknowledged_command)) {
                const float pitch = scaled_pitch / scale;
                const float difference = record->recoil_angles[0] - pitch;
                if (!(std::fabs(difference) <= 0.000009999999747378752f)) {
                    if (quantize_shot_coordinate(pitch) !=
                        std::bit_cast<float>(work.original_recoil_pitch))
                        *bindings.data_update_reset_marker = work.acknowledged_command;
                    else {
                        write(player, offsets.vector_1bae1168, pitch);
                        record->recoil_angles[0] = pitch;
                        invalidate_command_prediction(bindings.prediction);
                    }
                }
            }
        }
    }

    const auto handle = read<std::uint32_t>(player, offsets.active_weapon);
    if (handle != UINT32_MAX) {
        if (auto* weapon = engine::invoke<void*>(bindings.entity_list, 4, handle)) {
            const auto* identity = engine::invoke<const std::uint32_t*>(weapon, 2);
            current.state.weapon_identity = *identity;
            write(&current, 0x48, read<std::uint32_t>(weapon, offsets.weapon_field_6777c9ce));
        }
    }
    write(&current, 0x50, read<std::uint32_t>(player, offsets.field_66146802));
    write(&current, 0x4C, read<std::uint8_t>(player, offsets.field_bcb263a5));
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: entity_observer_registry.cpp
// ============================================================================

#if defined(_M_IX86) || defined(__i386__)
#define ENTITY_OBSERVER_FASTCALL __fastcall
#else
#define ENTITY_OBSERVER_FASTCALL
#endif

namespace skeet::client {
namespace {
constexpr std::array<EntityObserverTable, 18> entity_observer_tables{{
    {EntityObserverId::prediction_history, {{0x433CCE8Eu, 0x43397E0Du, 0x433CCE8Eu, 0x433C95B7u}}},
    {EntityObserverId::clan_tag, {{0x433CCE8Eu, 0x433C95B7u, 0x433CCE8Eu, 0x433414C7u}}},
    {EntityObserverId::history_recording, {{0x433CCE8Eu, 0x433C95B7u, 0x433CCE8Eu, 0x4340C971u}}},
    {EntityObserverId::mask, {{0x433CCE8Eu, 0x43408846u, 0x433CCE8Eu, 0x433C95B7u}}},
    {EntityObserverId::shot_feedback, {{0x433CCE8Eu, 0x433C95B7u, 0x4333D358u, 0x433240DDu}}},
    {EntityObserverId::entity_commands, {{0x433CCE8Eu, 0x433CFDA2u, 0x433CCE8Eu, 0x433C95B7u}}},
    {EntityObserverId::scripts, {{0x433CCE8Eu, 0x433998EBu, 0x433CCE8Eu, 0x4336A3FDu}}},
    {EntityObserverId::rage, {{0x433CCE8Eu, 0x43325DECu, 0x433CCE8Eu, 0x433C95B7u}}},
    {EntityObserverId::target_mask, {{0x4336AC2Eu, 0x4336526Du, 0x433CCE8Eu, 0x433C95B7u}}},
    {EntityObserverId::pickup_history, {{0x433CCE8Eu, 0x433C95B7u, 0x433CCE8Eu, 0x433E0F98u}}},
    {EntityObserverId::prediction_update, {{0x433CCE8Eu, 0x433C95B7u, 0x433CCE8Eu, 0x4337D273u}}},
    {EntityObserverId::shared_esp, {{0x433CCE8Eu, 0x433C95B7u, 0x433654AAu, 0x43382555u}}},
    {EntityObserverId::grenades, {{0x433CCE8Eu, 0x43337BC0u, 0x433CCE8Eu, 0x433C95B7u}}},
    {EntityObserverId::viewmodel_lag, {{0x433CCE8Eu, 0x433833B0u, 0x433CCE8Eu, 0x433C95B7u}}},
    {EntityObserverId::sequences, {{0x433CCE8Eu, 0x433E53A5u, 0x433CCE8Eu, 0x433C95B7u}}},
    {EntityObserverId::effects, {{0x433CCE8Eu, 0x43326129u, 0x433CCE8Eu, 0x433C95B7u}}},
    {EntityObserverId::inventory, {{0x433CCE8Eu, 0x4339D6B2u, 0x433CCE8Eu, 0x433C95B7u}}},
    {EntityObserverId::gloves, {{0x433CCE8Eu, 0x43342310u, 0x433CCE8Eu, 0x433C95B7u}}},
}};
const std::array<std::uint32_t, 4>& methods(EntityObserverId id) {
    for (const auto& table : entity_observer_tables)
        if (table.id == id)
            return table.methods;
    throw std::invalid_argument("Unknown native entity observer");
}
bool noop(std::uint32_t entry) noexcept {
    return entry == 0x433CCE8Eu || entry == 0x433C95B7u;
}
/* deduplicated helper */
template <class T> T& source(void* object) {
    auto* base = static_cast<NativeEntityObserver*>(object);
    return *static_cast<T*>(static_cast<NativeEntityObserverReceiver*>(base)->source);
}
template <class T> T& existing(void* object) {
    return *static_cast<T*>(static_cast<NativeEntityObserver*>(object));
}
void ENTITY_OBSERVER_FASTCALL empty_status(void*, const void*) {} //433CCE8E
void ENTITY_OBSERVER_FASTCALL empty_update(void*, void*, std::uintptr_t) {} //433C95B7
void ENTITY_OBSERVER_FASTCALL empty_reset(void*, std::int32_t) {} //433CCE8E
void ENTITY_OBSERVER_FASTCALL empty_incoming(void*, void*, std::int32_t) {} //433C95B7
void ENTITY_OBSERVER_FASTCALL update_prediction(void* object, void* player, std::uintptr_t) {
    observe_prediction_history(source<PredictionHistoryOwner>(object), player);
}
void ENTITY_OBSERVER_FASTCALL incoming_clan(void* object, void*, std::int32_t) {
    source<misc::ClanTagRuntime>(object).update();
}
void ENTITY_OBSERVER_FASTCALL incoming_history(void* object, void* player, std::int32_t) {
    auto& owner = existing<lagcomp::HistoryRecordingReceiver>(object);
    owner.record_players(player);
}
void ENTITY_OBSERVER_FASTCALL update_mask(void* object, void* player, std::uintptr_t) {
    source<skins::NativeEntityCosmetics>(object).update_mask(player);
}
void ENTITY_OBSERVER_FASTCALL reset_feedback(void* object, std::int32_t) {
    rage::reset_feedback_events(source<rage::FeedbackEntityOwner>(object));
}
void ENTITY_OBSERVER_FASTCALL incoming_feedback(void* object, void*, std::int32_t) {
    rage::reconcile_feedback_events(source<rage::FeedbackEntityOwner>(object));
}
void ENTITY_OBSERVER_FASTCALL update_entity_commands(void* object, void*, std::uintptr_t incoming) {
    process_entity_prediction_entities(
        source<EntityCommandOwner>(object),
        *reinterpret_cast<const EntityPredictionEntities*>(incoming));
}
void ENTITY_OBSERVER_FASTCALL update_scripts(void* object, void* player, std::uintptr_t incoming) {
    source<script::ScriptEntityObserver>(object).update_entities(player, incoming);
}
void ENTITY_OBSERVER_FASTCALL incoming_scripts(void* object, void* player, std::int32_t delta) {
    source<script::ScriptEntityObserver>(object).incoming_complete(player, delta);
}
void ENTITY_OBSERVER_FASTCALL update_rage(void* object, void* player, std::uintptr_t incoming) {
    rage::update_rage_entities(source<const rage::EntityUpdateBindings>(object), player,
                               *reinterpret_cast<const EntityPredictionEntities*>(incoming));
}
void ENTITY_OBSERVER_FASTCALL status_target_mask(void* object, const void* status) {
    observe_target_mask_update_status(source<TargetMaskOwner>(object), status);
}
void ENTITY_OBSERVER_FASTCALL update_target_mask(void* object, void*, std::uintptr_t incoming) {
    capture_target_mask_update(source<TargetMaskOwner>(object), incoming);
}
void ENTITY_OBSERVER_FASTCALL incoming_sound(void* object, void*, std::int32_t) {
    existing<esp::SoundHistoryReceiver>(object).process_incoming();
}
void ENTITY_OBSERVER_FASTCALL incoming_prediction(void* object, void*, std::int32_t) {
    prediction::update_prediction_entities(source<prediction::EngineEntityUpdateOwner>(object));
}
void ENTITY_OBSERVER_FASTCALL reset_shared(void* object, std::int32_t) {
    enter_shared_esp_update(source<SharedEspCommandOwner>(object).bindings);
}
void ENTITY_OBSERVER_FASTCALL incoming_shared(void* object, void*, std::int32_t) {
    leave_shared_esp_update(source<SharedEspCommandOwner>(object).bindings);
}
void ENTITY_OBSERVER_FASTCALL update_grenades(void* object, void* player, std::uintptr_t incoming) {
    source<misc::GrenadeCommandRuntime>(object).update_remote_warnings(player, incoming);
}
void ENTITY_OBSERVER_FASTCALL update_viewmodel(void* object, void* player, std::uintptr_t) {
    auto& owner = source<prediction::ViewmodelPredictionObserver>(object);
    misc::repair_viewmodel_lag(owner.state, owner.bindings, player);
}
void ENTITY_OBSERVER_FASTCALL update_sequences(void* object, void* player,
                                               std::uintptr_t incoming) {
    existing<skins::SequenceEntityObserverReceiver>(object).runtime->process(player, incoming);
}
void ENTITY_OBSERVER_FASTCALL update_effects(void* object, void* player, std::uintptr_t incoming) {
    effects::update_effects_entities(source<effects::EffectsCommandOwner>(object), player,
                                     incoming);
}
void ENTITY_OBSERVER_FASTCALL update_inventory(void* object, void* player,
                                               std::uintptr_t incoming) {
    source<skins::NativeInventoryRuntime>(object).process(player, incoming);
}
void ENTITY_OBSERVER_FASTCALL update_gloves(void* object, void* player, std::uintptr_t) {
    source<skins::NativeEntityCosmetics>(object).update_gloves(player);
}
const NativeEntityObserverVtable observer_prediction_table{empty_status, update_prediction, empty_reset,
                                                  empty_incoming};
const NativeEntityObserverVtable clan_table{empty_status, empty_update, empty_reset, incoming_clan};
const NativeEntityObserverVtable history_table{empty_status, empty_update, empty_reset,
                                               incoming_history};
const NativeEntityObserverVtable mask_table{empty_status, update_mask, empty_reset, empty_incoming};
const NativeEntityObserverVtable feedback_table{empty_status, empty_update, reset_feedback,
                                                incoming_feedback};
const NativeEntityObserverVtable entity_table{empty_status, update_entity_commands, empty_reset,
                                              empty_incoming};
const NativeEntityObserverVtable script_table{empty_status, update_scripts, empty_reset,
                                              incoming_scripts};
const NativeEntityObserverVtable rage_table{empty_status, update_rage, empty_reset, empty_incoming};
const NativeEntityObserverVtable target_table{status_target_mask, update_target_mask, empty_reset,
                                              empty_incoming};
const NativeEntityObserverVtable sound_table{empty_status, empty_update, empty_reset,
                                             incoming_sound};
const NativeEntityObserverVtable engine_prediction_table{empty_status, empty_update, empty_reset,
                                                         incoming_prediction};
const NativeEntityObserverVtable shared_table{empty_status, empty_update, reset_shared,
                                              incoming_shared};
const NativeEntityObserverVtable observer_grenade_table{empty_status, update_grenades, empty_reset,
                                               empty_incoming};
const NativeEntityObserverVtable viewmodel_table{empty_status, update_viewmodel, empty_reset,
                                                 empty_incoming};
const NativeEntityObserverVtable sequence_table{empty_status, update_sequences, empty_reset,
                                                empty_incoming};
const NativeEntityObserverVtable effects_table{empty_status, update_effects, empty_reset,
                                               empty_incoming};
const NativeEntityObserverVtable inventory_table{empty_status, update_inventory, empty_reset,
                                                 empty_incoming};
const NativeEntityObserverVtable gloves_table{empty_status, update_gloves, empty_reset,
                                              empty_incoming};
constexpr std::array<const NativeEntityObserverVtable*, 18> observer_source_tables{&observer_prediction_table,
                                                                          &clan_table,
                                                                          &history_table,
                                                                          &mask_table,
                                                                          &feedback_table,
                                                                          &entity_table,
                                                                          &script_table,
                                                                          &rage_table,
                                                                          &target_table,
                                                                          &sound_table,
                                                                          &engine_prediction_table,
                                                                          &shared_table,
                                                                          &observer_grenade_table,
                                                                          &viewmodel_table,
                                                                          &sequence_table,
                                                                          &effects_table,
                                                                          &inventory_table,
                                                                          &gloves_table};
NativeEntityObserver& observer_source_receiver(void* object) {
    auto* receiver = static_cast<NativeEntityObserver*>(object);
    if (receiver)
        for (const auto* table : observer_source_tables)
            if (receiver->vtable == table)
                return *receiver;
    throw std::logic_error("Native entity observer has no concrete source receiver binding");
}
}
std::span<const EntityObserverTable> native_entity_observer_tables() noexcept {
    return entity_observer_tables;
}
std::optional<EntityObserverId> identify_entity_observer(std::uint32_t address) noexcept {
    for (const auto& table : entity_observer_tables)
        if (static_cast<std::uint32_t>(table.id) == address)
            return table.id;
    return std::nullopt;
}
bool entity_observer_source_bound(EntityObserverId id, std::uint32_t slot) {
    if (slot >= 4)
        throw std::invalid_argument("Invalid entity observer slot");
    const auto entry = methods(id)[slot];
    if (noop(entry))
        return true;
    switch (entry) {
    case 0x43397E0D:
    case 0x43408846:
    case 0x433CFDA2:
    case 0x4336AC2E:
    case 0x4336526D:
    case 0x433833B0:
    case 0x43326129:
    case 0x4339D6B2:
    case 0x43342310:
    case 0x433E53A5:
    case 0x433414C7:
    case 0x4340C971:
    case 0x433E0F98:
        return true;
    case 0x433998EB:
    case 0x4336A3FD:
    case 0x433654AA:
    case 0x43382555:
        return true;
    case 0x4337D273:
    case 0x43325DEC:
    case 0x4333D358:
    case 0x433240DD:
        return true;
    case 0x43337BC0:
        return true;
    default:
        return false;
    }
}
EntityObserverRegistry::EntityObserverRegistry(engine::PreservedInitialization& initialization,
                                               engine::JobMemory memory)
    : EntityObserverRegistry(initialization.image_owner(), memory) {}
EntityObserverRegistry::EntityObserverRegistry(std::shared_ptr<engine::NativeDataImage> image,
                                               engine::JobMemory memory)
    : image_(image),
      owners_(engine::require_native_data_image(image_).native_cell<engine::OwnerRegistry>(
          0x43475BA8u)),
      memory_(memory) {
    if (!memory_.allocate || !memory_.free || !memory_.process_heap)
        throw std::invalid_argument("Native entity observers require the canonical process heap");
}
void EntityObserverRegistry::append_native(NativeEntityObserver& receiver) {
    engine::register_owner(owners_, &receiver, memory_); //4333BF32, including its -4 sentinel
}
void EntityObserverRegistry::append_receiver(NativeEntityObserverReceiver& receiver, void* owner) {
    receiver.source = owner;
    append_native(receiver);
}
void EntityObserverRegistry::append_prediction_history(NativeEntityObserverReceiver& receiver,
                                                       PredictionHistoryOwner& owner) {
    append_receiver(receiver, &owner);
}
void EntityObserverRegistry::append_clan_tag(NativeEntityObserverReceiver& receiver,
                                             misc::ClanTagRuntime& owner) {
    append_receiver(receiver, &owner);
}
void EntityObserverRegistry::append_mask(NativeEntityObserverReceiver& receiver,
                                         skins::NativeEntityCosmetics& owner) {
    append_receiver(receiver, &owner);
}
void EntityObserverRegistry::append_feedback(NativeEntityObserverReceiver& receiver,
                                             rage::FeedbackEntityOwner& owner) {
    append_receiver(receiver, &owner);
}
void EntityObserverRegistry::append_entity_commands(NativeEntityObserverReceiver& receiver,
                                                    EntityCommandOwner& owner) {
    append_receiver(receiver, &owner);
}
void EntityObserverRegistry::append_scripts(NativeEntityObserverReceiver& receiver,
                                            script::ScriptEntityObserver& owner) {
    append_receiver(receiver, &owner);
}
void EntityObserverRegistry::append_rage(NativeEntityObserverReceiver& receiver,
                                         const rage::EntityUpdateBindings& owner) {
    append_receiver(receiver, const_cast<rage::EntityUpdateBindings*>(&owner));
}
void EntityObserverRegistry::append_target_mask(NativeEntityObserverReceiver& receiver,
                                                TargetMaskOwner& owner) {
    append_receiver(receiver, &owner);
}
void EntityObserverRegistry::append_prediction_update(NativeEntityObserverReceiver& receiver,
                                                      prediction::EngineEntityUpdateOwner& owner) {
    append_receiver(receiver, &owner);
}
void EntityObserverRegistry::append_shared_esp(NativeEntityObserverReceiver& receiver,
                                               SharedEspCommandOwner& owner) {
    append_receiver(receiver, &owner);
}
void EntityObserverRegistry::append_grenades(NativeEntityObserverReceiver& receiver,
                                             misc::GrenadeCommandRuntime* owner) {
    append_receiver(receiver, owner);
}
void EntityObserverRegistry::append_viewmodel_lag(NativeEntityObserverReceiver& receiver,
                                                  prediction::ViewmodelPredictionObserver& owner) {
    append_receiver(receiver, &owner);
}
void EntityObserverRegistry::append_effects(NativeEntityObserverReceiver& receiver,
                                            effects::EffectsCommandOwner& owner) {
    append_receiver(receiver, &owner);
}
void EntityObserverRegistry::append_inventory(NativeEntityObserverReceiver& receiver,
                                              skins::NativeInventoryRuntime& owner) {
    append_receiver(receiver, &owner);
}
void EntityObserverRegistry::append_gloves(NativeEntityObserverReceiver& receiver,
                                           skins::NativeEntityCosmetics& owner) {
    append_receiver(receiver, &owner);
}
void EntityObserverRegistry::append(lagcomp::HistoryRecordingReceiver& owner) {
    append_native(owner);
    lagcomp::bind_history_recording_runtime(*owner.runtime);
}
void EntityObserverRegistry::append(esp::SoundHistoryReceiver& owner) {
    append_native(owner);
}
void EntityObserverRegistry::append(skins::SequenceEntityObserverReceiver& receiver,
                                    skins::NativeSequenceRuntime& owner) {
    receiver.runtime = &owner;
    append_native(receiver);
}
void EntityObserverRegistry::publish_prediction_history(
    NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &observer_prediction_table;
}
void EntityObserverRegistry::publish_clan_tag(
    NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &clan_table;
}
void EntityObserverRegistry::publish_mask(NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &mask_table;
}
void EntityObserverRegistry::publish_feedback(
    NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &feedback_table;
}
void EntityObserverRegistry::publish_entity_commands(
    NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &entity_table;
}
void EntityObserverRegistry::publish_scripts(
    NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &script_table;
}
void EntityObserverRegistry::publish_rage(NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &rage_table;
}
void EntityObserverRegistry::publish_target_mask(
    NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &target_table;
}
void EntityObserverRegistry::publish_prediction_update(
    NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &engine_prediction_table;
}
void EntityObserverRegistry::publish_shared_esp(
    NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &shared_table;
}
void EntityObserverRegistry::publish_grenades(
    NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &observer_grenade_table;
}
void EntityObserverRegistry::publish_viewmodel_lag(
    NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &viewmodel_table;
}
void EntityObserverRegistry::publish_effects(
    NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &effects_table;
}
void EntityObserverRegistry::publish_inventory(
    NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &inventory_table;
}
void EntityObserverRegistry::publish_gloves(NativeEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &gloves_table;
}
void EntityObserverRegistry::publish(lagcomp::HistoryRecordingReceiver& owner) const noexcept {
    owner.vtable = &history_table;
}
void EntityObserverRegistry::publish(esp::SoundHistoryReceiver& owner) const noexcept {
    owner.vtable = &sound_table;
}
void EntityObserverRegistry::publish(
    skins::SequenceEntityObserverReceiver& receiver) const noexcept {
    receiver.vtable = &sequence_table;
}
void EntityObserverRegistry::require_sources(std::uint32_t slot) const {
    if (slot >= 4)
        throw std::invalid_argument("Invalid entity observer slot");
    for (auto* object : owners())
        (void)observer_source_receiver(object);
}
void EntityObserverRegistry::update_status(const void* status) const {
    auto** const end = owners_.end; //4333CA25
    auto** cursor = owners_.begin; //4333CA2B
    for (; cursor != end; ++cursor)
        observer_source_receiver(*cursor).vtable->status(*cursor, status);
}
void EntityObserverRegistry::update_entities(void* player, std::uintptr_t incoming) const {
    auto** const end = owners_.end; //433206AD
    auto** cursor = owners_.begin; //433206B3
    for (; cursor != end; ++cursor)
        observer_source_receiver(*cursor).vtable->update(*cursor, player, incoming);
}
void EntityObserverRegistry::reset_update_state(std::int32_t selected_tick) const {
    auto** const end = owners_.end; //4341F019
    auto** cursor = owners_.begin; //4341F01F
    for (; cursor != end; ++cursor)
        observer_source_receiver(*cursor).vtable->reset(*cursor, selected_tick);
}
void EntityObserverRegistry::incoming_complete(void* player, std::int32_t delta) const {
    auto** const end = owners_.end; //433CFAEA
    auto** cursor = owners_.begin; //433CFAF0
    for (; cursor != end; ++cursor)
        observer_source_receiver(*cursor).vtable->incoming(*cursor, player, delta);
}
void EntityObserverRegistry::finalize_storage() const {
    engine::UnloadSystemFunctions memory;
    memory.free_heap = memory_.free;
    memory.process_heap = memory_.process_heap;
    engine::free_native_allocation(owners_.begin, memory); //433E4C98, no cell reset
}
} //namespace skeet::client

#undef ENTITY_OBSERVER_FASTCALL

// ============================================================================
// Subcomponent: entity_update_dispatch.cpp
// ============================================================================

namespace skeet::client {
namespace {
EntityUpdateRuntime* runtime = nullptr;
void apply_patches(EntityUpdatePatch* head, bool before,
                   EntityUpdateBindings::ExchangeWord exchange) {
    for (auto* patch = head; patch; patch = patch->next) {
        if (!patch->target)
            continue;
        const auto replacement = before ? patch->before : patch->after;
        if (replacement)
            exchange(patch->target, replacement);
    }
}
}
void* register_entity_update_receiver(engine::OwnerRegistry& registry, void* object,
                                      engine::JobMemory memory) {
    //4333BF32 shares433F1665 and4334DC99 with the owner pointer registry.
    return engine::register_owner(registry, object, memory);
}
void dispatch_entity_update(void* object, std::uintptr_t incoming, EntityUpdateGlobals& globals,
                            const EntityUpdateBindings& bindings, ui::SkinsMenu& skins_menu) {
    globals.phase = 1;
    apply_patches(globals.patches, true, bindings.exchange);
    //433205F9..4332061C rereads the installed metadata after the first patch
    // pass. A patch can change the receiver/table, so this is not cached.
    const auto* table = *static_cast<void***>(object);
    const auto* metadata = static_cast<const engine::VtableHookMetadata*>(table[-2]);
    reinterpret_cast<engine::MemberFunction<void, std::uintptr_t>>(metadata->original_table[64])(
        object, incoming);
    apply_patches(globals.patches, false, bindings.exchange);

    const auto index = engine::invoke<std::int32_t>(bindings.engine_client, 0x30 / 4);
    auto* local = engine::invoke<void*, std::int32_t>(bindings.entity_list, 0xC / 4, index);
    if (local && !engine::invoke<bool>(bindings.engine_client, 0x148 / 4) &&
        !engine::invoke<bool>(bindings.engine_client, 0x174 / 4)) {
        std::int32_t team;
        std::memcpy(&team, static_cast<const std::byte*>(local) + bindings.team_offset,
                    sizeof(team));
        if (team != globals.local_team) {
            globals.local_team = team;
            skins_menu.select_team(team); //complete existing4334A06E body
        }
    }
    // The concrete registry captures its range after the team-menu update and
    // directly invokes the recovered bodies for the registered native tables.
    globals.receivers.update_entities(local, incoming);
    globals.phase = 4;
}
void bind_entity_update_runtime(EntityUpdateRuntime& value) noexcept {
    runtime = &value;
}
#if defined(_M_IX86) || defined(__i386__)
void __fastcall entity_update_hook(void* object, void*, std::uintptr_t incoming) {
#else
void entity_update_hook(void* object, void*, std::uintptr_t incoming) {
#endif
    runtime->process(object, incoming);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: exit_registration.cpp
// ============================================================================

namespace skeet::client {
namespace {
struct ExitRegistration {
    std::uint32_t initializer, finalizer;
    void (*invoke)(void*);
};
template <std::uint32_t Finalizer> void append_exit_callback(void* owner) {
    auto& context = *static_cast<ExitRegistrationContext*>(owner);
    // source_callback selects an actual recovered C++ finalizer. The native
    // identifier is not converted into a host function pointer.
    (void)context.callbacks.append(context.cleanup.source_callback(Finalizer));
}

void initialize_spread_and_append(void* owner) { //4340B25A
    auto& context = *static_cast<ExitRegistrationContext*>(owner);
    context.cleanup.initialize_spread_storage(); //complete 43402E73
    append_exit_callback<0x4337D954u>(owner);
}
}

void bind_exit_initializers(NativeStartupSources& sources, ExitRegistrationContext& context) {
    // Ordinary wrappers load ECX with the finalizer, then tail-jump into the
    // existing 433837F5 append primitive. 4340B25A first initializes the two
    // shared spread-table allocations. Return values are unused by startup.
    const std::array<ExitRegistration, 19> entries{{
        {0x4339E627u, 0x433EB717u, append_exit_callback<0x433EB717u>}, //008
        {0x4341738Bu, 0x433E4A38u, append_exit_callback<0x433E4A38u>}, //00C
        {0x433B22B6u, 0x43349ECBu, append_exit_callback<0x43349ECBu>}, //010
        {0x4337DA78u, 0x43347D44u, append_exit_callback<0x43347D44u>}, //014
        {0x4340BF0Bu, 0x433F515Bu, append_exit_callback<0x433F515Bu>}, //018
        {0x433F9001u, 0x433E6194u, append_exit_callback<0x433E6194u>}, //01C
        {0x433B7724u, 0x433E4C98u, append_exit_callback<0x433E4C98u>}, //020
        {0x4337F0AFu, 0x433F4A1Du, append_exit_callback<0x433F4A1Du>}, //024
        {0x433C084Fu, 0x433F0746u, append_exit_callback<0x433F0746u>}, //028
        {0x4339FA2Bu, 0x433C9DB2u, append_exit_callback<0x433C9DB2u>}, //02C
        {0x433B261Du, 0x433F4C95u, append_exit_callback<0x433F4C95u>}, //030
        {0x4340B872u, 0x433D1E9Fu, append_exit_callback<0x433D1E9Fu>}, //034
        {0x433881D7u, 0x433C9173u, append_exit_callback<0x433C9173u>}, //038
        {0x433AB3C9u, 0x43336AD4u, append_exit_callback<0x43336AD4u>}, //03C
        {0x433A165Du, 0x43341B3Fu, append_exit_callback<0x43341B3Fu>}, //040
        {0x4332798Cu, 0x433B8C06u, append_exit_callback<0x433B8C06u>}, //044
        {0x4340B25Au, 0x4337D954u, initialize_spread_and_append}, //048
        {0x433E1038u, 0x43329A5Bu, append_exit_callback<0x43329A5Bu>}, //04C
        {0x433B4B44u, 0x4332EFADu, append_exit_callback<0x4332EFADu>}, //050
    }};
    for (const auto& entry : entries)
        if (context.cleanup.has_source_callback(entry.finalizer))
            sources.bind_initializer({entry.initializer, &context, entry.invoke});
}
}

// ============================================================================
// Subcomponent: exploit_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* [dedup offset_storage] */
ClientRuntimeStorage& checked(ClientRuntimeStorage& s, const EngineRuntimeBindings& e,
                              ConfigurationRuntimeOwner& c, HistoryRuntimeOwner& h,
                              MovementHudRuntimeOwner& m, CommandRuntimeOwner& command,
                              LocalAnimationRuntimeOwner& local,
                              engine::SceneQueryRuntimeOwner& scene, VisualRuntimeOwner& visual,
                              script::SourceScriptRuntimeOwner& scripts) {
    if (&e.variables.data() != &s.data() || &c.storage() != &s || &h.storage() != &s ||
        &m.storage() != &s || &command.storage() != &s || &command.history() != &h ||
        &local.storage() != &s || &local.history() != &h || &scene.storage() != &s ||
        &scene.players() != &h.players() || &visual.storage() != &s || &scripts.storage() != &s)
        throw std::invalid_argument(
            "Exploit requires canonical command, history, movement, local-animation, scene, HUD and Lua owners");
    auto& data = s.data();
    if (!e.command.seed_generator || !e.command.random_seed ||
        e.command.seed_generator != data.native_cell<prediction::SeedGenerator>(0x434679ACu) ||
        e.command.random_seed != data.native_cell<std::int32_t*>(0x43468328u))
        throw std::invalid_argument("Exploit requires the acquired genuine command seed inputs");
    for (const auto identity : {0x434757DCu, 0x43475B2Cu, 0x43475BA4u, 0x4346A338u, 0x43467EB8u,
                                0x4346A334u, 0x43475A60u, 0x4346A894u, 0x43469264u})
        if (!data.native_cell<void*>(identity))
            throw std::invalid_argument("Exploit command/clock producer is missing");
    if (!e.variables.at(0x4346A850u).get())
        throw std::invalid_argument("Exploit acknowledgement ConVar is not acquired");
    initialize_source_command_checksum(data);
    return s;
}
/* deduplicated registry helper */
}
struct ExploitRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    InputBitmap input;
    network::ExploitState state{};
    network::ExploitRuntimeState runtime;
    network::ExploitPhaseState phases{};
    network::ExploitControlView controls;
    network::ExploitRuntimeSettings settings;
    rage::TargetBindings targeting;
    GeneratedCommandOwnersBindings generated;
    RageStopBindings stopping;
    network::ExploitRuntimeBindings live;
    network::ExploitPhase1Bindings first;
    network::ExploitPhase7Bindings last;
    network::ExploitCommandOwner command;
    network::ExploitAcknowledgementBindings acknowledgement;
    network::ExploitMessageBindings messages;
    hud::ExploitIndicatorBindings indicator;
    network::ExploitOwnerHudBindings hud;
    ExploitRuntimeFactoryBindings factory;
    static void initialize(void* raw) {
        auto& self = *static_cast<Data*>(raw);
        auto* allocation = network::allocate_exploit_owner_storage(self.storage.memory());
        //4347FF44 reaches unresolved INT1/junk. This explicit source failure
        //policy neither constructs nor registers an invented null owner.
        if (!allocation)
            throw std::bad_alloc();
        network::construct_allocated_exploit_owner(
            allocation, self.factory.registries, self.storage.configuration(), self.factory.owner);
    }
    Data(ClientRuntimeStorage& s, const EngineRuntimeBindings& e, ConfigurationRuntimeOwner& c,
         HistoryRuntimeOwner& h, MovementHudRuntimeOwner& m, CommandRuntimeOwner& core,
         LocalAnimationRuntimeOwner& local, engine::SceneQueryRuntimeOwner& scene,
         VisualRuntimeOwner& visual, script::SourceScriptRuntimeOwner& scripts)
        : storage(checked(s, e, c, h, m, core, local, scene, visual, scripts)),
          input(s.input_runtime().bitmap()), runtime{state, 0, 0, 0, 0, 0, {}, {}, {}},
          controls{c.rage_settings().exploits, input},
          settings{controls,
                   c.rage_settings().global.duck_peek_assist.value.binding,
                   s.data().native_cell<std::uint8_t>(0x43467560u),
                   s.data().native_cell<std::int32_t>(0x43469274u),
                   s.data().native_cell<std::int32_t>(0x43468290u),
                   s.data().native_cell<std::int32_t>(0x4346DB30u)},
          targeting{s.data().native_cell<void*>(0x43475808u),
                    s.data().native_cell<void*>(0x4346A890u),
                    h.game_type(),
                    h.teammates_are_enemies(),
                    h.players().adjustments(),
                    h.target_offsets()},
          generated{s.data().native_cell<MovementCommandOwner*>(0x434759B0u),
                    s.data().native_cell<anti_aim::CommandOwner*>(0x43475940u),
                    m.movement_command().bindings.command, c.movement().infinite_duck.value,
                    std::uint32_t(offset(s, 0x4346A6DCu))},
          stopping{m.movement_environment()},
          live{s.data().native_cell<void*>(0x4346A514u),
               s.data().native_cell<void**>(0x4346A3A4u),
               s.data().native_cell<const void*>(0x43475B74u),
               s.data().native_cell<const void*>(0x43475808u),
               s.data().native_cell<std::int32_t*>(0x434757DCu),
               s.data().native_cell<const std::int32_t*>(0x43475B2Cu),
               s.data().native_cell<const std::int32_t*>(0x43475BA4u),
               s.data().native_cell<const float*>(0x4346A338u),
               s.data().native_cell<float*>(0x43467EB8u),
               {},
               core.capture().firing(),
               {offset(s, 0x43468378u), offset(s, 0x4346836Cu), offset(s, 0x43468BF4u),
                offset(s, 0x43468E78u), offset(s, 0x4346A438u), offset(s, 0x434693D8u),
                offset(s, 0x434670ECu), offset(s, 0x43467AA4u), offset(s, 0x43467A6Cu)},
               s.data().native_cell<std::array<std::uint32_t, 3>>(0x43474580u),
               {s.data().native_cell<void*>(0x4346A894u), 0xF0},
               {s.data().native_cell<void*>(0x4346A894u), 0xF4},
               s.data().native_cell<CommandChecksum>(0x43467C48u),
               s.data().native_cell<PredictionHistoryOwner*>(0x434759B4u),
               s.data().native_cell<EntityCommandOwner*>(0x43475898u),
               s.recharge_observers(),
               s.data().native_cell<const std::uint32_t*>(0x4346A334u),
               s.data().native_cell<void*>(0x43469264u)},
          first{c.rage_settings().global,
                s.data().native_cell<void*>(0x4346A890u),
                offset(s, 0x43467A74u),
                local.restricted().offsets.field_2a671b42,
                s.data().native_cell<void*>(0x43475AACu),
                *s.data().native_cell<std::int32_t*>(0x43475A60u),
                s.data().native_cell<std::int32_t>(0x4346DB1Cu),
                s.data().native_cell<std::int32_t>(0x4346DB2Cu),
                s.data().native_cell<std::array<float, 3>>(0x4346DB20u),
                s.recharge_observers()},
          last{core.capture(),
               core.capture().capture_offsets(),
               core.prediction(),
               core.prediction_player_offsets(),
               s.prediction_observers(),
               s.recharge_observers(),
               s.data().native_cell<prediction::SeedGenerator>(0x434679ACu),
               s.data().native_cell<std::int32_t*>(0x43468328u),
               scripts,
               generated,
               s.data().native_cell<std::array<std::uint32_t, 150>>(0x43476090u),
               c.anti_aim_other(),
               core.trigonometry(),
               core.capture().angle_environment(),
               m.movement_capture(),
               m.movement_environment(),
               stopping,
               scene.queries().movement(),
               targeting,
               s.data().native_cell<anti_aim::TargetState>(0x4346DB14u),
               c.rage_settings().global.quick_stop.value,
               c.rage_settings().global.activation.value.binding,
               input,
               reinterpret_cast<const std::uint32_t&>(
                   c.rage_settings().global.double_tap_quick_stop.value),
               s.data().native_cell<std::uint32_t>(0x43468488u),
               s.data().native_cell<std::int32_t>(0x434759F4u),
               s.data().native_cell<std::int32_t>(0x434759C4u),
               s.data().native_cell<PredictionHistoryOwner*>(0x434759B4u),
               local.restricted(),
               s.data().native_cell<std::uint8_t*>(0x434683D0u),
               {offset(s, 0x43468308u), offset(s, 0x43467554u), offset(s, 0x43467184u),
                offset(s, 0x4346A828u), offset(s, 0x434688ACu), offset(s, 0x4346720Cu)}},
          command{runtime,
                  settings,
                  live,
                  s.data().native_cell<std::uint32_t>(0x434759C0u),
                  {s.data().native_cell<network::LatencySamples>(0x43478C70u),
                   h.entity_runtime().command_tick,
                   s.data().native_cell<const std::int32_t*>(0x43468F9Cu),
                   s.data().native_cell<const std::int32_t*>(0x434670A8u)},
                  phases,
                  first,
                  last,
                  0,
                  controls},
          acknowledgement{s.data().native_cell<void*>(0x4346A514u),
                          s.data().native_cell<void*>(0x43467B04u),
                          s.data().native_cell<std::int32_t*>(0x43475BA4u),
                          s.data().native_cell<const std::int32_t*>(0x43475794u),
                          s.data().native_cell<float*>(0x4346A338u),
                          s.data().native_cell<std::int32_t>(0x43468290u),
                          e.variables.at(0x4346A850u),
                          std::uint32_t(offset(s, 0x43468E78u)),
                          std::uint32_t(offset(s, 0x434693D8u))},
          messages{s.data().native_cell<void*>(0x434683E0u),
                   s.data().native_cell<void*>(0x4346A894u),
                   s.data().native_cell<std::int32_t*>(0x434757DCu),
                   s.data().native_cell<std::int32_t*>(0x43475B2Cu),
                   s.data().native_cell<void**>(0x43475B0Cu)},
          indicator{s.data().native_cell<void*>(0x4346A890u), std::uint32_t(offset(s, 0x43468E78u)),
                    std::uint32_t(offset(s, 0x43467A74u)), std::uint32_t(offset(s, 0x4346908Cu))},
          hud{scripts.bridge(),
              s.data().native_cell<float>(0x43475890u),
              c.other_esp_settings().feature_indicators,
              indicator,
              *reinterpret_cast<float*>(s.data().native_cell<std::byte*>(0x43467B04u) + 0x14),
              visual.indicators()},
          factory{{registry(s, 0x43475B78u), registry(s, 0x43475A9Cu), registry(s, 0x43475B94u),
                   s.command_phases(), s.data_update_observers(), s.prediction_observers()},
                  {command, acknowledgement, messages, hud}} {}
};
ExploitRuntimeOwner::ExploitRuntimeOwner(ClientRuntimeStorage& s, const EngineRuntimeBindings& e,
                                         ConfigurationRuntimeOwner& c, HistoryRuntimeOwner& h,
                                         MovementHudRuntimeOwner& m, CommandRuntimeOwner& core,
                                         LocalAnimationRuntimeOwner& local,
                                         engine::SceneQueryRuntimeOwner& scene,
                                         VisualRuntimeOwner& visual,
                                         script::SourceScriptRuntimeOwner& scripts)
    : data_(std::make_unique<Data>(s, e, c, h, m, core, local, scene, visual, scripts)) {}
ExploitRuntimeOwner::~ExploitRuntimeOwner() = default;
ClientRuntimeStorage& ExploitRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
network::ExploitCommandOwner& ExploitRuntimeOwner::command() noexcept {
    return data_->command;
}
network::ExploitState& ExploitRuntimeOwner::state() noexcept {
    return data_->state;
}
ExploitRuntimeFactoryBindings& ExploitRuntimeOwner::factory_bindings() noexcept {
    return data_->factory;
}
void ExploitRuntimeOwner::bind_initializers(NativeStartupSources& sources) {
    if (&sources.data() != &storage().data())
        throw std::invalid_argument("Exploit factory requires the canonical startup dispatcher");
    sources.bind_initializer({exploit_runtime_factory, data_.get(), Data::initialize});
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: feature_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
std::uint32_t receive_offset(ClientRuntimeStorage& storage, std::uint32_t identity) {
    for (const auto& recipe : engine::build_receive_offset_recipes())
        if (recipe.pointer_identity == identity)
            return static_cast<std::uint32_t>(static_cast<std::ptrdiff_t>(
                engine::DecodedOffset(storage.offsets().globals, identity, recipe.xor_key_identity,
                                      recipe.initial_xor_key, 0)));
    throw std::invalid_argument("Feature factory has no proven receive offset");
}
/* deduplicated registry helper */
const float& frame_time(ClientRuntimeStorage& storage) {
    auto* const globals = storage.data().native_cell<const std::byte*>(0x43467B04u);
    if (!globals)
        throw std::invalid_argument("Feature factories require actual engine global variables");
    return *reinterpret_cast<const float*>(globals + 0x14);
}
void require_inputs(ClientRuntimeStorage& storage, ConfigurationRuntimeOwner& settings,
                    HistoryRuntimeOwner& history, skins::SkinsRuntimeOwner& skins,
                    rage::RuntimeOwner& rage, legit::RuntimeOwner& legit, esp::EspRuntimeOwner& esp,
                    script::ScriptApiRuntimeOwner& scripts, engine::HookRuntimeOwner& hooks) {
    if (&settings.storage() != &storage || &history.storage() != &storage ||
        &rage.storage() != &storage || &legit.storage() != &storage || &esp.storage() != &storage ||
        &scripts.storage() != &storage || &hooks.storage() != &storage ||
        &skins.sequence_bindings().settings != &settings.skins_settings() ||
        &skins.sequence_bindings().events != &storage.event_registry())
        throw std::invalid_argument("Feature factories must share the same process owners");
    auto& image = storage.data();
    if (!image.native_cell<void*>(0x4346A514u) || !image.native_cell<void*>(0x4346A890u) ||
        !image.native_cell<void*>(0x4346A4E0u))
        throw std::invalid_argument("Feature factories require acquired engine interfaces");
}
}

struct FeatureRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    misc::ZoomSensitivityBindings sensitivity;
    rage::LiveTargetRelationships event_targets;
    RuntimeOwnerFactoryBindings factories;

    Data(ClientRuntimeStorage& source, ConfigurationRuntimeOwner& settings,
         HistoryRuntimeOwner& history, skins::SkinsRuntimeOwner& skins, rage::RuntimeOwner& rage,
         legit::RuntimeOwner& legit, esp::EspRuntimeOwner& esp,
         script::ScriptApiRuntimeOwner& scripts, engine::HookRuntimeOwner& hooks)
        : storage(source), sensitivity{source.data().native_cell<void*>(0x43468994u),
                                       source.data().native_cell<void*>(0x43475808u),
                                       source.data().native_cell<std::uint32_t>(0x43467A48u),
                                       receive_offset(source, 0x43468248u)},
          event_targets{source.data().native_cell<void*>(0x4346A890u), history.game_type(),
                        history.teammates_are_enemies(), history.players().adjustments(),
                        history.target_offsets()},
          factories{source.configuration(),
                    registry(source, 0x43475B78u),
                    registry(source, 0x43475A9Cu),
                    source.entity_observers(),
                    registry(source, 0x43475A84u),
                    skins.sequence_bindings(),
                    rage.factory_bindings(),
                    source.data().native_cell<rage::LifecycleOwner*>(0x43475810u),
                    legit.factory_bindings(),
                    esp.factory_bindings(),
                    esp.active(),
                    {source.data().native_cell<void*>(0x4346A514u),
                     source.data().native_cell<void*>(0x4346A4E0u)},
                    source.data().native_cell<effects::BlurEffect*>(0x43475A20u),
                    settings.miscellaneous().console_output,
                    source.data().native_cell<misc::ColoredLogRing*>(0x434758CCu),
                    frame_time(source),
                    settings.miscellaneous().fov,
                    {sensitivity,
                     source.data().native_cell<void*>(0x4346A890u),
                     {static_cast<std::uint32_t>(history.target_offsets().rejection_bytes[0]),
                      static_cast<std::uint32_t>(history.target_offsets().movement_type),
                      receive_offset(source, 0x4346A6CCu), receive_offset(source, 0x43468F84u),
                      sensitivity.fov_offset}},
                    scripts.normal_log(),
                    event_targets,
                    hooks.console_input(),
                    source.data().native_cell<void*>(0x43467D84u)} {}
};

FeatureRuntimeOwner::FeatureRuntimeOwner(ClientRuntimeStorage& storage,
                                         ConfigurationRuntimeOwner& settings,
                                         HistoryRuntimeOwner& history,
                                         skins::SkinsRuntimeOwner& skins, rage::RuntimeOwner& rage,
                                         legit::RuntimeOwner& legit, esp::EspRuntimeOwner& esp,
                                         script::ScriptApiRuntimeOwner& scripts,
                                         engine::HookRuntimeOwner& hooks) {
    require_inputs(storage, settings, history, skins, rage, legit, esp, scripts, hooks);
    data_ =
        std::make_unique<Data>(storage, settings, history, skins, rage, legit, esp, scripts, hooks);
}
FeatureRuntimeOwner::~FeatureRuntimeOwner() = default;
ClientRuntimeStorage& FeatureRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
RuntimeOwnerFactoryBindings& FeatureRuntimeOwner::factory_bindings() noexcept {
    return data_->factories;
}
void FeatureRuntimeOwner::bind_initializers(NativeStartupSources& sources) {
    bind_runtime_owner_factories(sources, data_->factories);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: final_menu_startup.cpp
// ============================================================================

namespace skeet::client {
namespace {
void refresh_menu_key(ui::MenuWindow& window, RegisteredHotkey& binding) {
    ui::HotkeyWidget* found = nullptr;
    for (auto* tab : window.tabs)
        for (auto* category : tab->categories)
            for (auto* widget : category->elements)
                if (auto* key = dynamic_cast<ui::HotkeyWidget*>(widget);
                    key && key->value == &binding) {
                    if (found)
                        throw std::logic_error(
                            "Startup menu key has more than one canonical widget");
                    found = key;
                }
    if (!found)
        throw std::logic_error("Startup menu key widget has not been composed");
    found->config_loaded(); //actual433ADDF2, no settings-change notification
}
}
namespace {
struct RestoredSettings {
    std::array<bool, 10> records{};
};
void restore_saved_layout(ui::RootMenu& menu, const std::array<std::int32_t, 21>& values) {
    if (values[0] != 9)
        return;
    const auto targets = menu.layout_targets();
    constexpr std::array<std::size_t, 15> indices{3,  5,  4,  6,  7,  8,  9, 10,
                                                  11, 12, 13, 14, 15, 19, 20};
    for (std::size_t index = 0; index < indices.size(); ++index) {
        const auto packed = std::bit_cast<std::uint32_t>(values[indices[index]]);
        auto& category = *targets.categories[index + 2];
        category.column = std::bit_cast<std::int8_t>(std::uint8_t(packed));
        category.column_span = std::bit_cast<std::int8_t>(std::uint8_t(packed >> 8));
        category.row = std::bit_cast<std::int8_t>(std::uint8_t(packed >> 16));
        category.row_span = std::bit_cast<std::int8_t>(std::uint8_t(packed >> 24));
    }
    // Same ordinary43342FF1 category layout/reflow/resize consumer. The two
    // player-only built-in defaults are not persisted by433EEC8B.
    for (auto* tab : menu.window().tabs) {
        tab->layout_categories();
        tab->reflow_categories();
        tab->resize(menu.window().input.slider_metrics);
    }
}
}
void capture_startup_configuration_baseline(ClientRuntimeStorage& storage) {
    auto& cells = storage.config_storage().baseline_cells();
    if (cells.begin || cells.end || cells.capacity)
        throw std::logic_error("Startup configuration baseline has already been captured");
    ConfigBytes bytes;
    if (!serialize_config_registry(storage.configuration().descriptors, bytes))
        throw std::runtime_error(
            "Startup registry contains an unsupported configuration descriptor");
    auto* const allocation =
        static_cast<std::byte*>(engine::allocate_owner_storage(storage.memory(), bytes.size()));
    if (!allocation)
        throw std::bad_alloc();
    std::memcpy(allocation, bytes.data(), bytes.size());
    const auto begin = std::uint32_t(reinterpret_cast<std::uintptr_t>(allocation));
    cells = {begin, begin + std::uint32_t(bytes.size()), begin + std::uint32_t(bytes.size())};
}
FinalMenuStartupTail::FinalMenuStartupTail(ClientRuntimeStorage& storage,
                                           ui::MainMenuWindowOwner& window,
                                           misc::SettingsRuntimeOwner& settings,
                                           ConfigStorageOwner& configuration,
                                           ui::StartupRenderer& renderer,
                                           ui::StartupRegistryImports imports)
    : storage_(storage), window_(window), settings_(settings), configuration_(configuration),
      renderer_(renderer), imports_(imports) {
    if (&window.menu_data().data() != &storage.data() || &settings.storage() != &storage ||
        &settings.window() != &window || &settings.configuration() != &configuration ||
        &configuration.native_storage() != &storage.config_storage() ||
        &configuration.configuration() != &storage.configuration() ||
        &renderer.dpi() != &storage.menu_startup_data().dpi)
        throw std::invalid_argument("Final menu startup requires the canonical source owners");
}
void FinalMenuStartupTail::initialize() {
    if (started_)
        throw std::logic_error("Final menu startup has already started");
    auto& menu = window_.controller();
    if (!settings_.default_layout_captured() || !configuration_.menu_bound_to(menu) ||
        storage_.data().native_cell<ui::RootMenu*>(0x43476924u) != &menu)
        throw std::logic_error(
            "Final menu startup requires the actual RootMenu and captured default layout");
    started_ = true;
    auto& persistent = storage_.config_storage().initialize_settings(storage_.memory());
    const auto value = ui::read_startup_registry_value(window_.registry_name().text.data(),
                                                       imports_, storage_.memory());
    ui::unlink_startup_settings_records(persistent);
    RestoredSettings restored;
    if (value)
        restored.records = ui::restore_startup_settings_records(persistent, *value);
    auto values = storage_.menu_startup_data().settings_values();
    if (restored.records[0])
        values.menu_color = std::bit_cast<std::uint32_t>(persistent.global_value);
    if (restored.records[1]) {
        auto& binding = values.menu_key.binding;
        binding.packed = (binding.packed & 3u) | (persistent.menu_binding.packed & ~3u);
    }
    if (restored.records[2])
        values.lock_layout = std::uint8_t(persistent.input_mode);
    if (restored.records[3])
        values.dpi_scale = persistent.dpi_scale;
    if (restored.records[6])
        std::copy(persistent.script_hashes.begin(), persistent.script_hashes.end(),
                  window_.startup_scripts().begin());
    if (restored.records[9])
        storage_.menu_startup_data().script_values().allow_unsafe = persistent.flag;
    refresh_menu_key(window_.window(), storage_.menu_hotkey());
    // The window's first DPI pass precedes tab construction. Initialize the
    // completed control tree even when the saved scale is still 100%.
    window_.menu_data().dpi.refresh(true);
    auto& window = window_.window();
    if (restored.records[4] && restored.records[5])
        window.restore_size({persistent.menu_width, persistent.menu_height}, {});
    // Registry persistence overrides hardcoded layout defaults; bypass restoring saved grid layout.
    // if (restored.records[8])
    //     restore_saved_layout(menu, persistent.menu_controls);
    settings_.runtime().previous_dpi_scale = values.dpi_scale;
    settings_.runtime().previous_lock_layout = values.lock_layout;
    capture_startup_configuration_baseline(storage_);
    // Startup can run while the game is minimized or changing video mode.
    // Place the menu on the HUD thread once its actual client area is known.
    window.startup_placement_pending = true;
    menu.initialize_renderer_resources(renderer_.renderer());
    window.resize_finished = {&configuration_.persistence(), [](void* context) {
                                  static_cast<SettingsPersistence*>(context)->persist();
                              }};
    configuration_.presets().load_startup();
    complete_ = true;
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: final_patch_startup.cpp
// ============================================================================

namespace skeet::client {
void initialize_final_feature_callbacks(ClientRuntimeStorage& storage) {
    auto& owners = storage.owner_registries().at(0x43475B78u);
    auto entries = owners.entries();
    if (entries.size() != 57)
        throw std::logic_error("Final initialization requires all57 actual feature owners");
    for (auto* owner : entries)
        engine::invoke<void>(owner, 1);
    if (engine::invoke<bool>(storage.data().native_cell<void*>(0x4346A514u), 26)) {
        entries = owners.entries();
        for (auto* owner : entries)
            engine::invoke<void>(owner, 5);
    }
}
void capture_final_startup_patches(ClientRuntimeStorage& storage, PlayerReceiveOwner& players,
                                   effects::VisualReceiveOwner& visual,
                                   skins::SkinsRuntimeOwner& skins,
                                   engine::ClientClassFactoryRuntime& factories) {
    if (&players.storage() != &storage || &visual.storage() != &storage)
        throw std::invalid_argument("Final receive patches require canonical source runtimes");
    auto& image = storage.data();
    auto*& head = image.native_cell<EntityUpdatePatch*>(0x43476908u);
    if (head)
        throw std::logic_error("Final startup patch list is already constructed");
    auto* const base_client = image.native_cell<void*>(0x434683E0u);
    if (!base_client)
        throw std::logic_error("Final startup requires actual VClient018");
    struct Capture {
        EntityUpdatePatch* node;
        volatile std::int32_t* target;
        std::int32_t original, replacement;
    };
    std::vector<Capture> captures;
    captures.reserve(31);
    const auto capture = [&](std::uint32_t address, volatile std::int32_t* target,
                             std::uintptr_t replacement) {
        auto& node = image.native_cell<EntityUpdatePatch>(address);
        if (node.next || node.target || node.after || node.before)
            throw std::logic_error("A final startup patch is already initialized");
        if (!target || !*target || !replacement)
            throw std::runtime_error(
                "A final startup patch target or original callback is missing");
        captures.push_back(
            {&node, target, *target, std::bit_cast<std::int32_t>(std::uint32_t(replacement))});
    };
    for (const auto& row : engine::client_class_factory_descriptors()) {
        auto* const client = engine::find_client_class(base_client, row.class_hash);
        if (!client)
            throw std::runtime_error("Required final ClientClass factory is missing");
        capture(row.patch_address, reinterpret_cast<volatile std::int32_t*>(&client->create),
                reinterpret_cast<std::uintptr_t>(row.replacement));
    }
    const auto receive = [&](const auto& row) {
        auto* const property = engine::find_receive_property(base_client, row.class_hash,
                                                             row.table_hash, row.property_hash);
        if (!property)
            throw std::runtime_error("Required final receive property is missing");
        capture(row.patch_address, reinterpret_cast<volatile std::int32_t*>(&property->proxy),
                reinterpret_cast<std::uintptr_t>(row.replacement));
    };
    for (const auto& row : player_receive_descriptors())
        receive(row);
    const auto sequence = skins::sequence_receive_descriptors();
    for (const auto& row : sequence)
        receive(row);
    for (const auto& row : effects::visual_receive_descriptors())
        receive(row);
    const auto& agent = skins::agent_model_receive_descriptor();
    receive(agent);
    if (captures.size() != 31)
        throw std::logic_error("Final startup patch recipe count differs");
    // All mandatory source targets are validated before publication. This is
    // explicit source failure policy; it does not invent a native null branch.
    for (const auto& item : captures) {
        item.node->target = item.target;
        item.node->after = item.original;
        item.node->before = item.replacement;
        item.node->next = head;
        head = item.node;
    }
    engine::bind_client_class_factory_runtime(factories);
    players.bind_receive_patches();
    visual.runtime().bind_receive_patches();
    std::array<EntityUpdatePatch*, 4> sequence_nodes;
    for (std::size_t index = 0; index < sequence.size(); ++index)
        sequence_nodes[index] =
            &image.native_cell<EntityUpdatePatch>(sequence[index].patch_address);
    skins.bind_receive_patches(sequence_nodes,
                               image.native_cell<EntityUpdatePatch>(agent.patch_address));
}
void finish_final_engine_startup(ClientRuntimeStorage& storage, engine::HookRuntimeOwner& hooks) {
    if (&hooks.storage() != &storage)
        throw std::invalid_argument("Final engine startup requires its canonical hook owner");
    finish_final_engine_startup(storage, hooks.descriptor_storage(), hooks.unload_state());
}
void finish_final_engine_startup(ClientRuntimeStorage& storage,
                                 engine::HookDescriptorStorage& descriptors,
                                 engine::UnloadState& unload) {
    auto& data = storage.data();
    auto& owners = storage.owner_registries().at(0x43475B78u);
    if (&descriptors.image() != &data || &unload.owners != &owners ||
        &unload.clear_4347644c != &data.native_cell<std::uint32_t>(0x4347644Cu) ||
        &unload.clear_43476908 != &data.native_cell<std::uint32_t>(0x43476908u))
        throw std::invalid_argument(
            "Final engine startup views must borrow the canonical data and owner registry");
    auto entries = owners.entries();
    if (entries.size() != 57 || !data.native_cell<network::ClientStateScopedPatch*>(0x43476900u) ||
        !data.native_cell<EntityUpdatePatch*>(0x43476908u))
        throw std::logic_error(
            "Final engine startup requires all57 owners and both actual receive patch chains");
    auto& incoming = data.native_cell<network::IncomingUpdatePatch*>(0x4347644Cu);
    auto& fire = descriptors.at(0x43475A68u);
    auto& overlay = descriptors.at(0x4347591Cu);
    auto& state = descriptors.at(0x43475A3Cu);
    if (incoming || fire.target || fire.unknown_0 || overlay.target || state.target ||
        unload.final_patch.enabled || unload.second_patch.enabled || unload.cleared_patch.saved)
        throw std::logic_error("Final engine startup has already published its patches");
    const auto binding = engine::initialize_build_final_startup_bindings(data);
    auto* const client = data.native_cell<void*>(0x434683E0u);
    auto* const klass = engine::find_client_class(client, 0xD929B4F3u);
    if (!klass || !klass->create_event)
        throw std::runtime_error("The actual CTEFireBullets event factory is missing");
    using CreateEvent = void*(__cdecl*)();
    auto* const bullet = reinterpret_cast<CreateEvent>(klass->create_event)();
    auto* const stream = *binding.overlay_stream_slot;
    auto* const client_state = data.native_cell<void*>(0x43468E44u);
    const auto actual_receiver = [](void* receiver, const engine::VtableHookMetadata& metadata) {
        return receiver && metadata.original_table && metadata.replacement_table &&
               *static_cast<void***>(receiver) == metadata.original_table;
    };
    // The working dump can contain destroyed external UI/overlay objects.
    // Prevalidate live source receivers rather than publishing into stale
    // memory. This is explicit source failure policy, not a fabricated guard.
    if (!actual_receiver(bullet, fire) || !actual_receiver(stream, overlay) ||
        !actual_receiver(client_state, state) || !binding.float_proxy_slot ||
        !*binding.float_proxy_slot)
        throw std::runtime_error(
            "A final live engine receiver does not match its prepared RTTI descriptor");
    fire.target = bullet;
    fire.unknown_0 = reinterpret_cast<std::uintptr_t>(incoming);
    incoming = reinterpret_cast<network::IncomingUpdatePatch*>(&fire);
    overlay.target = stream;
    unload.final_patch = {static_cast<void**>(stream), overlay.original_table,
                          reinterpret_cast<std::uintptr_t>(overlay.replacement_table)};
    std::atomic_ref<void**>(*static_cast<void***>(stream)).exchange(overlay.replacement_table);
    state.target = client_state;
    unload.second_patch = {static_cast<void**>(client_state), state.original_table,
                           reinterpret_cast<std::uintptr_t>(state.replacement_table)};
    std::atomic_ref<void**>(*static_cast<void***>(client_state)).exchange(state.replacement_table);
    auto* const original = *binding.float_proxy_slot;
    data.native_cell<void*>(0x43476A44u) = original;
    network::bind_receive_float_proxy(data);
    auto* const replacement = reinterpret_cast<void*>(&network::receive_float_proxy);
    data.native_cell<void*>(0x43476A48u) = replacement;
    unload.cleared_patch = {binding.float_proxy_slot, original,
                            reinterpret_cast<std::uintptr_t>(replacement)};
    std::atomic_ref<void*>(*binding.float_proxy_slot).exchange(replacement);
    entries = owners.entries();
    for (auto* owner : entries)
        engine::invoke<void>(owner, 3);
    // Exact guest execution returns AL1 here to4355BB96. Subsequent protected
    // outer-caller checks are not part of43484F06's completed implementation.
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: game_window_proxy_factory.cpp
// ============================================================================

namespace skeet::client {
namespace {
void create(void* opaque) {
    create_game_window_proxy_owner(*static_cast<GameWindowProxyFactoryBindings*>(opaque));
}
}
engine::GameWindowProxyOwner*
create_game_window_proxy_owner(GameWindowProxyFactoryBindings& bindings) {
    return engine::create_game_window_proxy_owner(
        {{bindings.registries.at(0x43475B78u), bindings.memory},
         {bindings.registries.at(0x43475770u), bindings.memory}},
        bindings.proxy);
}
void bind_game_window_proxy_factory(NativeStartupSources& sources,
                                    GameWindowProxyFactoryBindings& bindings) {
    auto& data = engine::require_native_data_image(bindings.data);
    if (&data != &sources.data() || &data != &bindings.registries.image())
        throw std::invalid_argument(
            "Game-window proxy requires the scheduler's canonical native data and registries");
    if (&bindings.proxy.guarded_return_site != &data.native_cell<const std::byte*>(0x43467288u) ||
        &bindings.proxy.game_interface_slot != &data.native_cell<void**>(0x43468B2Cu) ||
        &bindings.proxy.original_rtti != &data.native_cell<std::uintptr_t>(0x43469288u))
        throw std::invalid_argument(
            "Game-window proxy requires its canonical live engine binding cells");
    if (!bindings.memory.allocate || !bindings.memory.free)
        throw std::invalid_argument(
            "Game-window proxy requires its genuine process-heap allocation imports");
    if (!bindings.proxy.imports.crc32 || !bindings.proxy.imports.exchange)
        throw std::invalid_argument(
            "Game-window proxy requires genuine CRC and atomic exchange imports");
    sources.bind_initializer({game_window_proxy_factory, &bindings, create});
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: generated_command_owners.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */

void generated_micro_movement(anti_aim::CommandOwner& owner, void* player,
                              Command& command) noexcept {
    const auto& offsets = owner.offsets;
    anti_aim::PrecalculationPlayer snapshot{};
    snapshot.duck_3309c0cf = field<float>(player, offsets.duck_3309c0cf);
    float interval = 0.0f;
    if (command.movement.buttons & 4u) {
        interval = *owner.globals.interval;
        snapshot.rate_fb7e3404 = field<float>(player, offsets.rate_fb7e3404);
    } else {
        if (snapshot.duck_3309c0cf != 1.0f ||
            snapshot.duck_3309c0cf != owner.precalculation.previous_duck_1a4) {
            snapshot.rate_fb7e3404 = field<float>(player, offsets.rate_fb7e3404);
            interval = *owner.globals.interval;
        }
        snapshot.field_6b1a59b5 = field<std::uint8_t>(player, offsets.ducking_6b1a59b5) != 0;
        if (!snapshot.field_6b1a59b5)
            snapshot.flags_e2fbe695 = field<std::uint32_t>(player, offsets.player_flags_e2fbe695);
    }
    anti_aim::apply_micro_movement(owner.precalculation, snapshot, interval, command);
}
}

void process_generated_movement_command(const GeneratedCommandOwnersBindings& bindings,
                                        CommandFrame& frame, void* player, Command& command,
                                        std::uint8_t& send_packet) {
    auto* const owner = bindings.movement_owner;
    if (!owner)
        return;
    owner->duck_peek.choked = frame.choked_15c;
    process_movement_phase_one(*owner, command, bindings.infinite_duck != 0);
    process_movement_command(*owner, player,
                             reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address)), command,
                             frame.angles_118, send_packet, owner->feedback, bindings.movement);
    owner->feedback.was_grounded = static_cast<std::uint8_t>((frame.flags >> 18) & 1u);
    update_quick_peek_shot_counter(owner->quick_peek,
                                   field<std::int32_t>(player, bindings.shot_counter_8cfc633a));
}

void process_generated_angle_command(const GeneratedCommandOwnersBindings& bindings,
                                     CommandFrame& frame, void* player, Command& command,
                                     std::uint8_t& send_packet) {
    auto* const owner = bindings.angle_owner;
    if (!owner)
        return;
    if (!(owner->globals.scheduler_flags & 0x800u)) {
        const auto& velocity = frame.velocity_150;
        const auto speed_squared =
            (velocity[0] * velocity[0] + velocity[1] * velocity[1]) + velocity[2] * velocity[2];
        if (4.0f > speed_squared)
            generated_micro_movement(*owner, player, command);
    }
    anti_aim::command_owner_phase4(*owner, frame, player, command, send_packet);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: global_config_initializers.cpp
// ============================================================================

namespace skeet::client {
namespace {
GlobalConfigInitializerBindings& global_bindings(void* context) noexcept {
    return *static_cast<GlobalConfigInitializerBindings*>(context);
}
void append(GlobalConfigInitializerBindings& source, ConfigDescriptor& descriptor) noexcept {
    append_config_link(source.configuration.descriptors, descriptor); //43385AE4
}
void hold_key(GlobalConfigInitializerBindings& source, HotkeyConfig& descriptor,
              std::uint32_t packed_key) noexcept {
    append(source, descriptor);
    auto& key = descriptor.value;
    append_hotkey(source.configuration.hotkeys, key); //4333CA03
    const auto packed = key.binding.packed;
    key.callback_object = nullptr;
    key.callback = nullptr;
    key.binding.mode = HotkeyMode::hold;
    key.binding.packed = (packed & 1u) | packed_key;
}
void always_key(GlobalConfigInitializerBindings& source, HotkeyConfig& descriptor) noexcept {
    append(source, descriptor);
    auto& key = descriptor.value;
    append_hotkey(source.configuration.hotkeys, key);
    const auto packed = key.binding.packed;
    key.callback_object = nullptr;
    key.callback = nullptr;
    key.binding.packed = (packed & 1u) | 0x6cu;
    key.binding.mode = HotkeyMode::always;
}

void anti_untrusted(void* context) { //434928E5,434693DC
    auto& source = global_bindings(context);
    append(source, source.anti_untrusted.enabled);
    source.anti_untrusted.enabled.value = 1;
}
void hide_from_obs(void* context) { //43492907,43468E48
    auto& source = global_bindings(context);
    append(source, source.hide_from_obs.enabled);
}
void multithreading(void* context) { //43492921,434683EC
    auto& source = global_bindings(context);
    append(source, source.settings_menu.multithreading_config);
    source.settings_menu.multithreading_config.value = 1;
}
void custom_events(void* context) { //43492943,434691A8
    auto& source = global_bindings(context);
    append(source, source.custom_events.enabled);
    source.custom_events.enabled.value = 1;
}
void free_look(void* context) { //43493241,43468264
    auto& source = global_bindings(context);
    hold_key(source, source.free_look.key, 0x6cu);
}
void duck_peek(void* context) { //434933C0,43467C7C
    auto& source = global_bindings(context);
    hold_key(source, source.rage.duck_peek_assist, 0x6cu);
}
void shared_esp(void* context) { //434934A1,43467F88
    auto& source = global_bindings(context);
    append(source, source.other_esp.shared.enabled);
}
void restrict_shared_esp(void* context) { //434934BB,43468588
    auto& source = global_bindings(context);
    append(source, source.other_esp.shared.restrict_updates);
}
void remove_flash(void* context) { //434934D5,43468FF4
    auto& source = global_bindings(context);
    append(source, source.effects.remove_flash);
}
void third_person_dead(void* context) { //434934EF,4346DAE0
    auto& source = global_bindings(context);
    append(source, source.effects.third_person_dead);
}
void third_person_key(void* context) { //43493509,43468438
    auto& source = global_bindings(context);
    always_key(source, source.effects.third_person_key);
}
void third_person_alive(void* context) { //43493552,43468A00
    auto& source = global_bindings(context);
    append(source, source.effects.third_person_alive);
}
void disable_ragdolls(void* context) { //4349356C,43469010
    auto& source = global_bindings(context);
    append(source, source.effects.disable_ragdolls);
}
void disable_model_occlusion(void* context) { //43493586,43467ADC
    auto& source = global_bindings(context);
    append(source, source.colored_models.disable_model_occlusion);
}
void disable_teammates(void* context) { //434935A0,43468544
    auto& source = global_bindings(context);
    append(source, source.effects.disable_teammates);
    source.effects.disable_teammates.value = 0;
}
void colored_ragdolls(void* context) { //434935C2,4346A680
    auto& source = global_bindings(context);
    append(source, source.colored_models.ragdolls);
}
void pattern_recoil(void* context) { //434935DC,43467CA4
    auto& source = global_bindings(context);
    append(source, source.rage.pattern_recoil_correction);
}
void automatic_penetration(void* context) { //434935F6,4346A3D0
    auto& source = global_bindings(context);
    append(source, source.rage.automatic_penetration);
}
void automatic_fire(void* context) { //43493610,43468400
    auto& source = global_bindings(context);
    append(source, source.rage.automatic_fire);
}
void target_hitboxes(void* context) { //4349362A,434691BC
    auto& source = global_bindings(context);
    append(source, source.rage.target_hitboxes);
    source.rage.target_hitboxes.value = 2;
}
void avoid_unsafe_hitboxes(void* context) { //4349364F,43467020
    auto& source = global_bindings(context);
    append(source, source.rage.avoid_unsafe_hitboxes);
    source.rage.avoid_unsafe_hitboxes.value = 0;
}
void prefer_safe_point(void* context) { //43493671,43468F68
    auto& source = global_bindings(context);
    append(source, source.rage.prefer_safe_point);
}
void rage_activation(void* context) { //4349368B,43467F24
    auto& source = global_bindings(context);
    hold_key(source, source.rage.activation, 0x18u);
}
void quick_stop_key(void* context) { //434936D6,43469308
    auto& source = global_bindings(context);
    always_key(source, source.rage.quick_stop_key);
}
void force_body_aim(void* context) { //4349371F,4346A2F0
    auto& source = global_bindings(context);
    hold_key(source, source.rage.force_body_aim, 0x6cu);
}
void multipoint_key(void* context) { //4349376A,43469400
    auto& source = global_bindings(context);
    always_key(source, source.rage.multipoint_key);
}
void rage_enabled(void* context) { //434937B3,43467214
    auto& source = global_bindings(context);
    append(source, source.rage.enabled);
}
void silent_aim(void* context) { //434937CD,434684D0
    auto& source = global_bindings(context);
    append(source, source.rage.silent_aim);
}
void remove_recoil(void* context) { //434937E7,43467134
    auto& source = global_bindings(context);
    append(source, source.rage.remove_recoil);
}
void quick_stop(void* context) { //43493801,434671D0
    auto& source = global_bindings(context);
    append(source, source.rage.quick_stop);
}
void quick_stop_options(void* context) { //4349381B,43467E30
    auto& source = global_bindings(context);
    append(source, source.rage.quick_stop_options);
    source.rage.quick_stop_options.value = 0;
}
void prefer_body_aim(void* context) { //4349383D,43467D60
    auto& source = global_bindings(context);
    append(source, source.rage.prefer_body_aim);
}
void body_aim_disablers(void* context) { //43493857,4346A8C0
    auto& source = global_bindings(context);
    append(source, source.rage.body_aim_disablers);
}
void force_safe_point(void* context) { //43493871,434678D0
    auto& source = global_bindings(context);
    hold_key(source, source.rage.force_safe_point, 0x6cu);
}
void double_tap_hit_chance(void* context) { //434938BC,4346A4A0
    auto& source = global_bindings(context);
    append(source, source.rage.double_tap_hit_chance);
    source.rage.double_tap_hit_chance.value = 50;
}
void automatic_scope(void* context) { //434938E1,434693C4
    auto& source = global_bindings(context);
    append(source, source.rage.automatic_scope);
}
}

void bind_global_config_initializers(NativeStartupSources& sources,
                                     GlobalConfigInitializerBindings& source) {
    sources.bind_initializer({0x434928e5u, &source, anti_untrusted});
    sources.bind_initializer({0x43492907u, &source, hide_from_obs});
    sources.bind_initializer({0x43492921u, &source, multithreading});
    sources.bind_initializer({0x43492943u, &source, custom_events});
    sources.bind_initializer({0x43493241u, &source, free_look});
    sources.bind_initializer({0x434933c0u, &source, duck_peek});
    sources.bind_initializer({0x434934a1u, &source, shared_esp});
    sources.bind_initializer({0x434934bbu, &source, restrict_shared_esp});
    sources.bind_initializer({0x434934d5u, &source, remove_flash});
    sources.bind_initializer({0x434934efu, &source, third_person_dead});
    sources.bind_initializer({0x43493509u, &source, third_person_key});
    sources.bind_initializer({0x43493552u, &source, third_person_alive});
    sources.bind_initializer({0x4349356cu, &source, disable_ragdolls});
    sources.bind_initializer({0x43493586u, &source, disable_model_occlusion});
    sources.bind_initializer({0x434935a0u, &source, disable_teammates});
    sources.bind_initializer({0x434935c2u, &source, colored_ragdolls});
    sources.bind_initializer({0x434935dcu, &source, pattern_recoil});
    sources.bind_initializer({0x434935f6u, &source, automatic_penetration});
    sources.bind_initializer({0x43493610u, &source, automatic_fire});
    sources.bind_initializer({0x4349362au, &source, target_hitboxes});
    sources.bind_initializer({0x4349364fu, &source, avoid_unsafe_hitboxes});
    sources.bind_initializer({0x43493671u, &source, prefer_safe_point});
    sources.bind_initializer({0x4349368bu, &source, rage_activation});
    sources.bind_initializer({0x434936d6u, &source, quick_stop_key});
    sources.bind_initializer({0x4349371fu, &source, force_body_aim});
    sources.bind_initializer({0x4349376au, &source, multipoint_key});
    sources.bind_initializer({0x434937b3u, &source, rage_enabled});
    sources.bind_initializer({0x434937cdu, &source, silent_aim});
    sources.bind_initializer({0x434937e7u, &source, remove_recoil});
    sources.bind_initializer({0x43493801u, &source, quick_stop});
    sources.bind_initializer({0x4349381bu, &source, quick_stop_options});
    sources.bind_initializer({0x4349383du, &source, prefer_body_aim});
    sources.bind_initializer({0x43493857u, &source, body_aim_disablers});
    sources.bind_initializer({0x43493871u, &source, force_safe_point});
    sources.bind_initializer({0x434938bcu, &source, double_tap_hit_chance});
    sources.bind_initializer({0x434938e1u, &source, automatic_scope});
}

} //namespace skeet::client

// ============================================================================
// Subcomponent: history_runtime_factories.cpp
// ============================================================================

namespace skeet::client {
namespace {

bool same_memory(engine::JobMemory first, engine::JobMemory second) noexcept {
    return first.allocate == second.allocate && first.free == second.free &&
           first.process_heap == second.process_heap;
}
void require_bindings(const HistoryRuntimeFactoryBindings& b) {
    const auto& image = engine::require_native_data_image(b.data);
    const auto& registries = b.history.registries;
    if (!b.lifecycle.memory.allocate || !b.lifecycle.memory.free ||
        !b.lifecycle.memory.process_heap)
        throw std::invalid_argument("History factories require their canonical process heap");
    struct Alias {
        const engine::OwnerRegistryBinding* binding;
        std::uint32_t address;
    };
    const std::array aliases{
        Alias{&b.lifecycle, 0x43475b78u}, Alias{&registries.lifecycle, 0x43475b78u},
        Alias{&registries.packets, 0x4347584cu}, Alias{&registries.frame_update, 0x43475ad8u}};
    for (const auto& alias : aliases)
        if (&alias.binding->registry != &image.native_cell<engine::OwnerRegistry>(alias.address) ||
            !same_memory(alias.binding->memory, b.lifecycle.memory))
            throw std::invalid_argument(
                "History factories must share their native registry cells and heap");
    if (&b.entities != &registries.entities || &b.entities.image() != &image ||
        &b.entities.native_storage() != &image.native_cell<engine::OwnerRegistry>(0x43475ba8u))
        throw std::invalid_argument(
            "History factories must share the canonical entity observer registry");
    if (&registries.configuration.descriptors != &image.native_cell<ConfigLink>(0x434798bcu) ||
        &registries.configuration.hotkeys != &image.native_cell<HotkeyLink>(0x434798c4u))
        throw std::invalid_argument(
            "History factories require the canonical configuration sentinels");
    if (&b.history_active != &image.native_cell<lagcomp::HistoryOwnerRuntime*>(0x43475ac0u) ||
        &b.history.local_player != &image.native_cell<void*>(0x43475808u) ||
        &b.prediction.engine_client != &image.native_cell<void*>(0x4346a514u) ||
        &b.prediction.entity_list != &image.native_cell<void*>(0x4346a890u) ||
        &b.prediction.pending_events != &image.native_cell<void*>(0x43475a50u) ||
        &b.prediction.base_client != &image.native_cell<void*>(0x434683e0u) ||
        &b.prediction.replacement_property != &image.native_cell<std::uint32_t*>(0x43469060u))
        throw std::invalid_argument("History factories require their canonical live global cells");
}

void initialize_prediction(void* context) {
    (void)create_prediction_entity_update_owner(*static_cast<HistoryRuntimeFactoryBindings*>(context));
}
void initialize_history(void* context) {
    (void)create_lagcomp_history_owner(*static_cast<HistoryRuntimeFactoryBindings*>(context));
}
}

void* create_prediction_entity_update_owner(HistoryRuntimeFactoryBindings& bindings) {
    require_bindings(bindings);
    auto* const storage = engine::allocate_owner_storage(
        bindings.lifecycle.memory,
        std::max(std::size_t{0x18}, prediction::engine_entity_update_owner_size()));
    if (!storage) {
        //The native null branch enters unresolved INT1/junk at4347E369.
        //Source failure policy is explicit; no null continuation is invented.
        throw std::bad_alloc();
    }
    (void)prediction::construct_engine_entity_update_owner(storage, bindings.lifecycle,
                                                           bindings.entities, bindings.prediction);
    return storage;
}

lagcomp::HistoryOwnerRuntime*
create_lagcomp_history_owner(HistoryRuntimeFactoryBindings& bindings) {
    require_bindings(bindings);
    auto* const storage = engine::allocate_owner_storage(
        bindings.lifecycle.memory, std::max(std::size_t{0x60}, lagcomp::history_owner_size()));
    lagcomp::HistoryOwnerRuntime* owner = nullptr;
    if (storage) {
        //HEAP_ZERO_MEMORY covered this descriptor inside the native owner.
        //Its source projection is borrowed, and is cleared only on success.
        bindings.history.field_ed048f3d = BooleanConfig{};
        owner =
            &lagcomp::construct_history_owner(storage, bindings.history_active, bindings.history);
    }
    //mem_int3.h records43484F7D MOV[43475AC0],EAX. The success constructor
    //returns before this store; the known null branch publishes the same cell.
    bindings.history_active = owner;
    return owner;
}

void bind_history_runtime_factories(NativeStartupSources& sources,
                                    HistoryRuntimeFactoryBindings& bindings) {
    if (&sources.data() != bindings.data.get())
        throw std::invalid_argument("History factories must share the startup native data owner");
    require_bindings(bindings);
    sources.bind_initializer({prediction_entity_update_factory, &bindings, initialize_prediction});
    sources.bind_initializer({lagcomp_history_factory, &bindings, initialize_history});
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: history_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
struct DerivedOffset {
    std::uint32_t pointer, key, salt;
};
// Native4348D65D..4348D9E6 derived encodings and the two collision-history
// keys sharing word0. All other fields use the verified receive/
// datamap recipes, by exact identity.
constexpr std::array history_derived{DerivedOffset{0x43467004u, 0x4346A6B4u, 0x1599541Fu},
                             DerivedOffset{0x43469064u, 0x4346A484u, 0x17978090u},
                             DerivedOffset{0x43468E7Cu, 0x434674A8u, 0x54783141u},
                             DerivedOffset{0x43467F4Cu, 0x4346A8B0u, 0x3D52A1A7u},
                             DerivedOffset{0x43467D78u, 0x43467B80u, 0x1CAAF0A8u},
                             DerivedOffset{0x4346757Cu, 0x4346707Cu, 0x043FA575u},
                             DerivedOffset{0x43468BF8u, 0x43467B8Cu, 0xD070684Cu},
                             DerivedOffset{0x43467634u, 0x43468F2Cu, 0xBB78DC43u},
                             DerivedOffset{0x43467EBCu, 0x43467038u, 0x432B1F5Bu},
                             DerivedOffset{0x43467A88u, 0x43467160u, 0x0D560FACu}};
std::uint32_t offset(const engine::NativeOffsetGlobals& globals, std::uint32_t pointer) {
    for (const auto& row : engine::build_receive_offset_recipes())
        if (row.pointer_identity == pointer)
            return std::uint32_t(std::ptrdiff_t(engine::DecodedOffset(
                globals, pointer, row.xor_key_identity, row.initial_xor_key, 0)));
    for (const auto& row : engine::build_data_map_offset_recipes())
        if (row.pointer_identity == pointer)
            return std::uint32_t(std::ptrdiff_t(engine::DecodedOffset(
                globals, pointer, row.xor_key_identity, row.initial_xor_key, 0)));
    for (const auto& row : history_derived)
        if (row.pointer == pointer)
            return std::uint32_t(
                std::ptrdiff_t(engine::DecodedOffset(globals, pointer, row.key, row.salt, 0)));
    throw std::logic_error("History field identity has no recovered offset recipe");
}
/* deduplicated registry helper */
const std::uint32_t& bits(const std::int32_t& value) noexcept {
    // Corresponding signed/unsigned types may alias the same integer object.
    return reinterpret_cast<const std::uint32_t&>(value);
}
ClientRuntimeStorage& checked(ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
                              ConfigurationRuntimeOwner& config, PlayerRuntimeOwner& players,
                              engine::SceneQueryRuntimeOwner& scene, animation::RuntimeOwner& bones,
                              script::SourceScriptRuntimeOwner& scripts) {
    auto& data = storage.data();
    if (&engine.variables.data() != &data || &config.storage() != &storage ||
        &players.storage() != &storage || &scene.storage() != &storage ||
        &scene.players() != &players || &bones.storage() != &storage ||
        &bones.players() != &players.cache() || &scripts.storage() != &storage)
        throw std::invalid_argument(
            "History must share the canonical engine, configuration, player, scene and script owners");
    if (engine.modules.at(u"client.dll") != data.native_cell<void*>(0x4346A55Cu) ||
        engine.interfaces.core().engine_client != data.native_cell<void*>(0x4346A514u) ||
        engine.interfaces.core().entity_list != data.native_cell<void*>(0x4346A890u))
        throw std::invalid_argument(
            "History requires the initialized canonical engine interface bundle");
    auto* const globals = data.native_cell<std::byte*>(0x43467B04u);
    if (!globals || reinterpret_cast<std::uintptr_t>(globals) % 4 ||
        data.native_cell<void*>(0x4346A338u) != globals + 0x20 ||
        data.native_cell<void*>(0x434678F8u) != globals + 4 ||
        data.native_cell<void*>(0x43467EB8u) != globals + 0x10 ||
        data.native_cell<void*>(0x43469300u) != globals + 0x18 ||
        !data.native_cell<float*>(0x4346A334u) || !data.native_cell<std::int32_t*>(0x43475A60u) ||
        !storage.offsets().words || storage.offsets().globals.pointers.empty())
        throw std::invalid_argument(
            "History requires live clock aliases and initialized entity offsets");
    for (const auto cell : {0x434688C8u, 0x43467E44u, 0x4346A7F0u, 0x434693F0u})
        if (!data.native_cell<void*>(cell))
            throw std::invalid_argument("History requires the recovered engine animation entries");
    for (const auto cell :
         {0x43467A8Cu, 0x4346A568u, 0x43467A54u, 0x43467990u, 0x43467124u, 0x4346A820u, 0x43467968u,
          0x43467E28u, 0x43468C2Cu, 0x43467FA0u, 0x4346794Cu, 0x4346A850u})
        if (!engine.variables.at(cell).get())
            throw std::invalid_argument("History ConVar acquisition is incomplete");
    return storage;
}
rage::TargetOffsets compose_target_offsets(const engine::NativeOffsetGlobals& g) {
    return {{offset(g, 0x43468E78u), offset(g, 0x4346A5DCu), offset(g, 0x43468BF8u),
             offset(g, 0x43467DD0u)},
            offset(g, 0x434688B0u),
            offset(g, 0x434678ACu),
            offset(g, 0x43468BD4u),
            offset(g, 0x434688D0u),
            offset(g, 0x434670CCu),
            offset(g, 0x434679F8u),
            offset(g, 0x43467A74u),
            offset(g, 0x4346908Cu)};
}
lagcomp::HistoryVelocityFields velocity_fields(const engine::NativeOffsetGlobals& g) {
    return {offset(g, 0x434670CCu), offset(g, 0x434679F8u), offset(g, 0x43468BF8u),
            offset(g, 0x43467230u), offset(g, 0x43469198u), offset(g, 0x434685CCu),
            offset(g, 0x434688D0u), offset(g, 0x43468EE4u), offset(g, 0x43468C40u),
            offset(g, 0x43467A74u)};
}
lagcomp::EntityStateRuntime compose_entity_runtime(ClientRuntimeStorage& storage) {
    auto& data = storage.data();
    return {native_history_entity_fields(storage.offsets().globals),
            data.native_cell<void*>(0x4346A514u),
            data.native_cell<void*>(0x4346A890u),
            *data.native_cell<float*>(0x4346A338u),
            *reinterpret_cast<float*>(data.native_cell<std::byte*>(0x43467B04u) + 0x20),
            *data.native_cell<std::int32_t*>(0x434678F8u),
            data.native_cell<std::int32_t*>(0x43475A60u)[1],
            data.native_cell<const std::int32_t*>(0x43468F9Cu),
            data.native_cell<const std::int32_t*>(0x434670A8u),
            data.native_cell<lagcomp::SetAbsoluteVector>(0x434688C8u),
            data.native_cell<lagcomp::SetAbsoluteVector>(0x43467E44u),
            data.native_cell<lagcomp::SetCollisionBounds>(0x4346A7F0u),
            data.native_cell<std::uint8_t*>(0x434683D0u)};
}
void trace(void* raw, const resolver_runtime::EnvironmentTraceRequest& source,
           BallisticTrace& result) {
    auto& storage = *static_cast<ClientRuntimeStorage*>(raw);
    engine::TraceRay ray;
    ray.start = {source.ray.ray.start[0], source.ray.ray.start[1], source.ray.ray.start[2], 0};
    ray.delta = {source.ray.ray.delta[0], source.ray.ray.delta[1], source.ray.ray.delta[2], 0};
    ray.start_offset = {source.ray.start_offset[0], source.ray.start_offset[1],
                        source.ray.start_offset[2], 0};
    ray.extents = {source.ray.extents[0], source.ray.extents[1], source.ray.extents[2], 0};
    ray.is_ray = source.ray.is_ray;
    ray.swept = source.ray.swept;
    auto* const engine = storage.data().native_cell<void*>(0x43468304u);
    if (source.trace_type == 0) {
        engine::TraceRejectEntitiesFilter filter;
        engine::invoke<void>(engine, 5, &ray, source.mask, &filter, &result);
    } else if (source.trace_type == 1) {
        engine::TraceWorldOnlyFilter filter;
        engine::invoke<void>(engine, 5, &ray, source.mask, &filter, &result);
    } else
        throw std::invalid_argument("History environment has an unproved trace-filter type");
}
std::uint32_t point_contents(void* raw, const RageVector& point, std::uint32_t mask) {
    auto& data = static_cast<ClientRuntimeStorage*>(raw)->data();
    //43348ECB/4340E0CB call the world-only contents virtual+4 with exactly
    //point and mask; the separate general contents entry takes a third arg.
    return engine::invoke<std::uint32_t>(data.native_cell<void*>(0x43468304u), 1, &point, mask);
}
void view_angles(void* raw, RageVector& angles) {
    auto& data = static_cast<ClientRuntimeStorage*>(raw)->data();
    engine::invoke<void>(data.native_cell<void*>(0x4346A514u), 18, &angles);
}
HistoryRuntimeImports require_imports(HistoryRuntimeImports imports) {
    const auto& n = imports.numeric;
    if (!imports.thread_pool || !n.fmod || !n.environment_atan2 || !n.atan2 || !n.power ||
        !n.arccosine || !n.contains_sequence_name)
        throw std::invalid_argument("History requires its actual thread-pool and numeric imports");
    return imports;
}
}

lagcomp::EntityNetvars native_history_entity_fields(const engine::NativeOffsetGlobals& g) {
    if (offset(g, 0x43467EBCu) < sizeof(void*) || offset(g, 0x43467A88u) < sizeof(void*))
        throw std::logic_error(
            "History collision offsets were not initialized before feature construction");
    //433FD690,43389C5D,4331E453 and4341D38E operand identities.
    return {offset(g, 0x43468EA0u), offset(g, 0x43469198u), offset(g, 0x43467230u),
            offset(g, 0x43468FECu), offset(g, 0x434670CCu), offset(g, 0x434688D0u),
            offset(g, 0x4346A8E4u), offset(g, 0x43467DD4u), offset(g, 0x434679F8u),
            offset(g, 0x43467A74u), offset(g, 0x43467B44u), offset(g, 0x43467F4Cu),
            offset(g, 0x4346A660u), offset(g, 0x43467D78u), offset(g, 0x43469064u),
            offset(g, 0x43468E7Cu), offset(g, 0x4346A808u), offset(g, 0x434675CCu),
            offset(g, 0x434688B4u), offset(g, 0x43467634u), offset(g, 0x43467EBCu),
            offset(g, 0x43467A88u), offset(g, 0x43469344u)};
}

struct HistoryRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    PlayerRuntimeOwner& players;
    const HistoryRuntimeImports imports;
    const engine::BuildHistoryBindings functions;
    const lagcomp::HistoryNumericBindings numeric;
    const lagcomp::EntityStateRuntime entity;
    const rage::TargetOffsets targets;
    engine::NativeConVarReference game_type, enemies;
    ShotFeedbackState feedback;
    ui::SettingsMenuValues menu;
    InputBitmap keys;
    lagcomp::HistoryRecordingConfiguration configuration;
    lagcomp::HistoryJobDispatcher jobs;
    resolver_runtime::EnvironmentEngine environment;
    lagcomp::HistoryEnvironmentEngine history_environment;
    animation::AnimationUpdateEngine animation;
    std::uint32_t seed_update_flags = 0;
    lagcomp::HistoryVelocityReconstructionBindings physics;
    lagcomp::HistoryVelocityCaptureBindings velocity;
    lagcomp::HistoryResolverUpdateBindings resolver;
    const float seed_exponent = std::bit_cast<float>(0x3EDC8172u);
    lagcomp::HistoryAnimationPublishBindings publication;
    lagcomp::HistoryRecordingBindings recording;
    BooleanConfig field_ed048f3d;
    HistoryRuntimeFactoryBindings factory;
    SourceTerminalCleanupBindings& terminal;

    Data(ClientRuntimeStorage& source, const EngineRuntimeBindings& engine,
         ConfigurationRuntimeOwner& config, PlayerRuntimeOwner& player,
         engine::SceneQueryRuntimeOwner& scene, animation::RuntimeOwner& bones,
         script::SourceScriptRuntimeOwner& scripts, lagcomp::HistoryRecordingScratch* scratch,
         HistoryRuntimeImports services)
        : storage(checked(source, engine, config, player, scene, bones, scripts)), players(player),
          imports(require_imports(services)),
          functions(engine::initialize_build_history_bindings(storage.data())),
          numeric(imports.numeric), entity(compose_entity_runtime(storage)),
          targets(compose_target_offsets(storage.offsets().globals)),
          game_type(engine.variables.at(0x43467A8Cu)), enemies(engine.variables.at(0x4346A568u)),
          feedback(scripts.bridge()), menu(config.settings_menu().values()),
          keys(storage.input_runtime().bitmap()),
          configuration{nullptr,
                        &config.rage_settings().global.enabled.value,
                        &config.rage_settings().global.anti_aim_correction.value,
                        0,
                        entity.engine},
          jobs(imports.thread_pool, storage.memory()), environment{&storage,
                                                                   trace,
                                                                   point_contents,
                                                                   view_angles,
                                                                   scene.queries().ballistics(),
                                                                   bones.trigonometry(),
                                                                   entity.transform_override},
          history_environment{environment, entity.offsets.origin, numeric.environment_atan2},
          animation(lagcomp::native_history_animation_engine(
              storage.data().native_cell<void*>(0x434693F0u))),
          physics{player.cache(),
                  scene.queries().movement(),
                  entity.entity_list,
                  entity.interval,
                  velocity_fields(storage.offsets().globals),
                  {engine.variables.at(0x43467A54u), engine.variables.at(0x43467990u),
                   engine.variables.at(0x43467124u), engine.variables.at(0x4346A820u),
                   engine.variables.at(0x43467968u), engine.variables.at(0x43467E28u)},
                  functions.activity,
                  functions.sequence_name,
                  numeric.contains_sequence_name,
                  "_crouch",
                  numeric.atan2,
                  bones.trigonometry().sine,
                  bones.trigonometry().cosine,
                  entity.transform_override,
                  seed_update_flags},
          velocity{entity, physics, engine.variables.at(0x43468C2Cu),
                   engine.variables.at(0x43467FA0u)},
          resolver{entity,
                   configuration,
                   player.adjustments().settings_base(),
                   storage.data().native_cell<std::int32_t>(0x4346DB1Cu),
                   storage.data().native_cell<void* const*>(0x4346A3A4u),
                   *storage.data().native_cell<float*>(0x4346A334u),
                   *storage.data().native_cell<float*>(0x43467EB8u),
                   entity.frame,
                   menu.hold_aim,
                   storage.data().native_cell<std::uint8_t*>(0x43467B24u),
                   storage.data().native_cell<RageVector>(0x43475BD4u),
                   feedback.pending,
                   resolver_fields(storage.offsets().globals),
                   engine.variables.at(0x4346794Cu),
                   physics,
                   animation,
                   environment},
          publication{entity,
                      publication_fields(storage.offsets().globals),
                      entity.engine,
                      resolver.local_time_entity,
                      &resolver.current_time,
                      entity.receive_interval,
                      resolver.animation_reset_byte,
                      configuration,
                      player.adjustments().settings_base(),
                      0,
                      storage.data().native_cell<std::int32_t>(0x4347C9ECu),
                      seed_exponent,
                      &storage.data().native_cell<std::uint8_t>(0x434745E0u),
                      lagcomp::native_history_movement_engine(functions.sequence_displacement),
                      numeric.power,
                      numeric.arccosine,
                      numeric.atan2},
          recording{player.cache(),
                    player.records(),
                    entity,
                    configuration,
                    {&storage.data().native_cell<std::uint8_t>(0x434693ECu),
                     &storage.data().native_cell<void**>(0x4346A3A4u),
                     offset(storage.offsets().globals, 0x43468378u),
                     &storage.data().native_cell<legit::LifecycleOwner*>(0x4347588Cu),
                     configuration.recording_enabled},
                    {game_type, enemies, targets},
                    prelude_fields(storage.offsets().globals),
                    recording_fields(storage.offsets().globals),
                    {*storage.data().native_cell<std::int32_t*>(0x43469300u),
                     *storage.data().native_cell<std::int32_t*>(0x43475A60u), resolver.current_time,
                     resolver.animation_time,
                     *reinterpret_cast<float*>(storage.data().native_cell<std::byte*>(0x43467B04u) +
                                               0x14),
                     entity.frame, menu, menu.hold_aim, *configuration.resolver_enabled,
                     resolver.animation_reset_byte,
                     storage.data().native_cell<RageVector>(0x43475BD4u), player.adjustments()},
                    {game_type, resolver.selected_entity,
                     config.rage_settings().global.force_safe_point.value.binding, keys,
                     bits(config.rage_settings().global.avoid_unsafe_hitboxes.value),
                     config.rage_settings().global.prefer_body_aim.value,
                     bits(config.rage_settings().global.body_aim_disablers.value)},
                    numeric.fmod,
                    bones.bones(),
                    jobs,
                    history_environment,
                    velocity,
                    resolver,
                    publication,
                    menu.maximum_unlag},
          factory{storage.data_owner(),
                  registry(storage, 0x43475B78u),
                  storage.entity_observers(),
                  {storage.data().native_cell<void*>(0x4346A514u),
                   storage.data().native_cell<void*>(0x4346A890u),
                   storage.data().native_cell<void*>(0x43475A50u),
                   storage.data().native_cell<void*>(0x434683E0u),
                   storage.data().native_cell<std::uint32_t*>(0x43469060u),
                   offset(storage.offsets().globals, 0x4346A808u)},
                  {{registry(storage, 0x43475B78u), storage.entity_observers(),
                    registry(storage, 0x4347584Cu), registry(storage, 0x43475AD8u),
                    storage.configuration(), storage.event_registry()},
                   recording,
                   scratch,
                   field_ed048f3d,
                   storage.data().native_cell<void*>(0x43475808u),
                   engine.variables.at(0x4346A850u),
                   feedback,
                   entity.command_tick,
                   owner_fields(storage.offsets().globals)},
                  storage.data().native_cell<lagcomp::HistoryOwnerRuntime*>(0x43475AC0u)},
          terminal(scripts.source_terminal_cleanup(player)) {}

    static lagcomp::HistoryResolverFields resolver_fields(const engine::NativeOffsetGlobals& g) {
        return {offset(g, 0x43468354u), offset(g, 0x434670E0u), offset(g, 0x434679CCu),
                offset(g, 0x43467D78u), offset(g, 0x43467E8Cu), offset(g, 0x43468240u),
                offset(g, 0x43468D6Cu)};
    }
    static lagcomp::HistoryAnimationPublishFields
    publication_fields(const engine::NativeOffsetGlobals& g) {
        return {offset(g, 0x43468DBCu), offset(g, 0x43467D78u), offset(g, 0x4346A8E4u),
                offset(g, 0x43467E8Cu), offset(g, 0x434670CCu), offset(g, 0x4346A594u),
                offset(g, 0x434688D0u), offset(g, 0x43467230u), offset(g, 0x43468D6Cu),
                offset(g, 0x4346A808u)};
    }
    static lagcomp::HistoryPreludeOffsets prelude_fields(const engine::NativeOffsetGlobals& g) {
        return {offset(g, 0x43468E78u), offset(g, 0x4346A5DCu), offset(g, 0x43467004u),
                offset(g, 0x4346908Cu)};
    }
    static lagcomp::HistoryRecordingFields recording_fields(const engine::NativeOffsetGlobals& g) {
        return {offset(g, 0x43468B50u), offset(g, 0x4346757Cu), offset(g, 0x4346934Cu),
                offset(g, 0x43467200u), offset(g, 0x43468DBCu), offset(g, 0x4346A6C0u)};
    }
    static lagcomp::HistoryOwnerFields owner_fields(const engine::NativeOffsetGlobals& g) {
        return {offset(g, 0x43467EC0u), offset(g, 0x43468FACu), offset(g, 0x43468E78u)};
    }
};

HistoryRuntimeOwner::HistoryRuntimeOwner(ClientRuntimeStorage& storage,
                                         const EngineRuntimeBindings& engine,
                                         ConfigurationRuntimeOwner& config,
                                         PlayerRuntimeOwner& players,
                                         engine::SceneQueryRuntimeOwner& scene,
                                         animation::RuntimeOwner& bones,
                                         script::SourceScriptRuntimeOwner& scripts)
    : HistoryRuntimeOwner(storage, engine, config, players, scene, bones, scripts,
                          native_history_runtime_imports()) {}
HistoryRuntimeOwner::HistoryRuntimeOwner(
    ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
    ConfigurationRuntimeOwner& config, PlayerRuntimeOwner& players,
    engine::SceneQueryRuntimeOwner& scene, animation::RuntimeOwner& bones,
    script::SourceScriptRuntimeOwner& scripts, HistoryRuntimeImports imports)
    : data_(std::make_unique<Data>(storage, engine, config, players, scene, bones, scripts, nullptr,
                                   imports)) {}
HistoryRuntimeOwner::HistoryRuntimeOwner(
    ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
    ConfigurationRuntimeOwner& config, PlayerRuntimeOwner& players,
    engine::SceneQueryRuntimeOwner& scene, animation::RuntimeOwner& bones,
    script::SourceScriptRuntimeOwner& scripts, lagcomp::HistoryRecordingScratch& scratch)
    : HistoryRuntimeOwner(storage, engine, config, players, scene, bones, scripts, scratch,
                          native_history_runtime_imports()) {}
HistoryRuntimeOwner::HistoryRuntimeOwner(
    ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
    ConfigurationRuntimeOwner& config, PlayerRuntimeOwner& players,
    engine::SceneQueryRuntimeOwner& scene, animation::RuntimeOwner& bones,
    script::SourceScriptRuntimeOwner& scripts, lagcomp::HistoryRecordingScratch& scratch,
    HistoryRuntimeImports imports)
    : data_(std::make_unique<Data>(storage, engine, config, players, scene, bones, scripts,
                                   &scratch, imports)) {}
HistoryRuntimeImports native_history_runtime_imports() {
    return {engine::native_thread_pool(), lagcomp::native_history_numeric_bindings()};
}
HistoryRuntimeOwner::~HistoryRuntimeOwner() = default;
ClientRuntimeStorage& HistoryRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
PlayerRuntimeOwner& HistoryRuntimeOwner::players() const noexcept {
    return data_->players;
}
const lagcomp::EntityStateRuntime& HistoryRuntimeOwner::entity_runtime() const noexcept {
    return data_->entity;
}
const rage::TargetOffsets& HistoryRuntimeOwner::target_offsets() const noexcept {
    return data_->targets;
}
engine::NativeConVarReference HistoryRuntimeOwner::game_type() const noexcept {
    return data_->game_type;
}
engine::NativeConVarReference HistoryRuntimeOwner::teammates_are_enemies() const noexcept {
    return data_->enemies;
}
ShotFeedbackState& HistoryRuntimeOwner::feedback_state() noexcept {
    return data_->feedback;
}
const lagcomp::HistoryRecordingBindings& HistoryRuntimeOwner::recording_bindings() const noexcept {
    return data_->recording;
}
HistoryRuntimeFactoryBindings& HistoryRuntimeOwner::factory_bindings() noexcept {
    return data_->factory;
}
SourceTerminalCleanupBindings& HistoryRuntimeOwner::source_terminal_cleanup() noexcept {
    return data_->terminal;
}
void HistoryRuntimeOwner::bind_initializers(NativeStartupSources& sources) {
    bind_history_runtime_factories(sources, data_->factory);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: hitboxes.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */
/* deduplicated helper */
struct Model {
    std::span<const std::byte> descriptors;
    std::array<std::uint32_t, 19> bones;
    std::int32_t count;
    animation::BoneModel bone_model() const noexcept {
        return {true,  0,  true, false,
                false, {}, {},   {bones.data(), static_cast<std::size_t>(count)}};
    }
};
bool model_for(const void* entity, const ClientHitboxOffsets& offsets, Model& model) noexcept {
    const auto wrapper = read<const void*>(entity, offsets.studio_wrapper);
    if (!wrapper)
        return false;
    const auto studio = read<const std::byte*>(wrapper, 0);
    if (!studio)
        return false;
    const auto* set = studio + read<std::int32_t>(studio, 0xB0) +
                      read<std::int32_t>(entity, offsets.hitbox_set) * 12;
    model.count = read<std::int32_t>(set, 4);
    const auto* descriptors = set + read<std::int32_t>(set, 8);
    model.descriptors = {descriptors, static_cast<std::size_t>(model.count) * 0x44};
    for (std::int32_t i = 0; i < model.count; ++i)
        model.bones[i] = read<std::uint32_t>(descriptors, i * 0x44);
    return true;
}
RageLocalHitbox local_hitbox(const std::byte* descriptor) noexcept {
    return {read<std::int32_t>(descriptor, 4),  read<RageVector>(descriptor, 8),
            read<RageVector>(descriptor, 0x14), read<RageVector>(descriptor, 0x24),
            read<float>(descriptor, 0x30),      descriptor};
}
// SSE keeps the destination operand's NaN payload. C++ permits commuting an
// add/multiply even without fast-math, so retain that observable native order.
bool nan(float value) noexcept {
    return (std::bit_cast<std::uint32_t>(value) & 0x7FFFFFFFu) > 0x7F800000u;
}
float quiet(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) | 0x00400000u);
}
/* [dedup add_float_plain] */
/* [dedup subtract_float_plain] */
/* [dedup multiply_float_plain] */
/* deduplicated negate helper */
Matrix3x4 overlay_matrix(const Matrix3x4& bone, const RageVector& angles,
                         const RageTrigonometry& trig) {
    constexpr float radians = 0.01745329238474369f;
    const auto yaw = multiply(angles[1], radians), pitch = multiply(angles[0], radians),
               roll = multiply(angles[2], radians);
    const auto sy = trig.sine(yaw), cy = trig.cosine(yaw), sp = trig.sine(pitch),
               cp = trig.cosine(pitch), sr = trig.sine(roll), cr = trig.cosine(roll);
    const auto crcy = multiply(cr, cy), crsy = multiply(cr, sy), srcy = multiply(sr, cy),
               srsy = multiply(sr, sy);
    const Matrix3x4 rotation{
        {{multiply(cp, cy), subtract(multiply(srcy, sp), crsy), add(multiply(crcy, sp), srsy), 0},
         {multiply(cp, sy), add(multiply(srsy, sp), crcy), subtract(multiply(crsy, sp), srcy), 0},
         {negate(sp), multiply(sr, cp), multiply(cr, cp), 0}}};
    Matrix3x4 result;
    for (unsigned row = 0; row < 3; ++row)
        for (unsigned column = 0; column < 4; ++column)
            result.rows[row][column] =
                add(add(add(multiply(bone.rows[row][2], rotation.rows[2][column]),
                            multiply(bone.rows[row][1], rotation.rows[1][column])),
                        multiply(bone.rows[row][0], rotation.rows[0][column])),
                    column == 3 ? bone.rows[row][3] : 0.0f);
    return result;
}
}
void draw_client_hitbox(const ClientHitboxRuntime& runtime, const RageLocalHitbox& hitbox,
                        const Matrix3x4& bone, float duration, std::uint32_t color) {
    const auto matrix = overlay_matrix(bone, hitbox.rotation, runtime.trigonometry);
    if (hitbox.radius > 0.0f) {
        RageVector minimum, maximum;
        for (unsigned row = 0; row < 3; ++row) {
            const auto& m = matrix.rows[row];
            minimum[row] =
                add(add(add(multiply(m[0], hitbox.minimum[0]), multiply(m[1], hitbox.minimum[1])),
                        multiply(m[2], hitbox.minimum[2])),
                    m[3]);
            maximum[row] =
                add(add(add(multiply(m[0], hitbox.maximum[0]), multiply(m[1], hitbox.maximum[1])),
                        multiply(m[2], hitbox.maximum[2])),
                    m[3]);
        }
        using Capsule =
            engine::MemberFunction<void, const RageVector&, const RageVector&, const float&,
                                   std::int32_t, std::int32_t, std::int32_t, std::int32_t, float,
                                   std::int32_t, std::int32_t>;
        const auto table = *static_cast<void***>(runtime.debug_overlay);
        reinterpret_cast<Capsule>(table[23])(runtime.debug_overlay, minimum, maximum, hitbox.radius,
                                             color & 255, (color >> 8) & 255, (color >> 16) & 255,
                                             color >> 24, duration, 1, 1);
        return;
    }
    constexpr float degrees = 57.2957763671875f;
    const float horizontal = std::sqrt(add(multiply(matrix.rows[0][0], matrix.rows[0][0]),
                                           multiply(matrix.rows[1][0], matrix.rows[1][0])));
    RageVector angles;
    if (horizontal > 0.001) {
        angles[1] = multiply(
            static_cast<float>(runtime.arc_tangent(matrix.rows[1][0], matrix.rows[0][0])), degrees);
        angles[0] =
            multiply(static_cast<float>(runtime.arc_tangent(negate(matrix.rows[2][0]), horizontal)),
                     degrees);
        angles[2] = multiply(
            static_cast<float>(runtime.arc_tangent(matrix.rows[2][1], matrix.rows[2][2])), degrees);
    } else {
        angles[1] = multiply(
            static_cast<float>(runtime.arc_tangent(negate(matrix.rows[0][1]), matrix.rows[1][1])),
            degrees);
        angles[0] =
            multiply(static_cast<float>(runtime.arc_tangent(negate(matrix.rows[2][0]), horizontal)),
                     degrees);
        angles[2] = 0.0f;
    }
    const RageVector origin{matrix.rows[0][3], matrix.rows[1][3], matrix.rows[2][3]};
    const std::uint32_t transparent = 0;
    using Box = engine::MemberFunction<void, const RageVector&, const RageVector&,
                                       const RageVector&, const RageVector&, const std::uint32_t&,
                                       const std::uint32_t&, float>;
    const auto table = *static_cast<void***>(runtime.debug_overlay);
    reinterpret_cast<Box>(table[21])(runtime.debug_overlay, origin, hitbox.minimum, hitbox.maximum,
                                     angles, transparent, color, duration);
}

void draw_client_hitboxes(const ClientHitboxRuntime& runtime, void* entity,
                          const ClientHitboxesRequest& request) {
    auto& player = *runtime.players.get(request.entity_index);
    auto& ring = player.history;
    auto tick = request.has_tick
                    ? request.tick
                    : lagcomp::time_to_tick(read<float>(entity, runtime.offsets.simulation_time),
                                            runtime.interval);
    if (ring.count) {
        const auto newest = ring.at(0).header.simulation_tick;
        const auto oldest = ring.at(ring.count - 1).header.simulation_tick;
        tick = newest < oldest ? newest : std::clamp(tick, oldest, newest);
    }
    Model model{};
    if (!model_for(entity, runtime.offsets, model))
        return;
    if (tick == -1)
        tick = runtime.current_tick;
    auto& selected_ring = runtime.players.get(request.entity_index)->history;
    const lagcomp::LagRecord* record = nullptr;
    for (std::uint32_t i = 0; i < selected_ring.count; ++i) {
        const auto* candidate = &selected_ring.at(i);
        if (candidate->header.simulation_tick <= tick && (candidate->header.flags & 4)) {
            record = candidate;
            break;
        }
    }
    HitboxMatrices matrices;
    if (!record) {
        const auto* descriptor = static_cast<const std::byte*>(entity) + runtime.offsets.bone_cache;
        const auto count = read<std::int32_t>(descriptor, 0xC);
        if (!count)
            return;
        animation::select_hitbox_bone_matrices(
            model.bone_model(),
            {read<const Matrix3x4*>(descriptor, 0), static_cast<std::size_t>(count)}, matrices);
    } else {
        matrices = record->matrices;
        const auto& offsets = runtime.offsets;
        const auto yaw = read<float>(entity, offsets.absolute_angles + 4);
        write(entity, offsets.absolute_angles + 4,
              request.flags & 1 ? request.yaw : player.native.reference_yaw);
        auto* collideable = engine::invoke<void*>(entity, 3);
        auto* maximum = engine::invoke<RageVector*>(collideable, 2);
        const auto max_z = (*maximum)[2];
        (*maximum)[2] = record->header.collision_max_z;
        const auto origin = read<RageVector>(entity, offsets.origin),
                   absolute_origin = read<RageVector>(entity, offsets.absolute_origin);
        write(entity, offsets.origin, record->header.origin);
        write(entity, offsets.absolute_origin, record->header.origin);
        const auto flags = read<std::uint32_t>(entity, offsets.entity_flags);
        write(entity, offsets.entity_flags, flags & 0xFFFF07FFu);
        const auto field_432 = read<float>(entity, offsets.field_432b1f5b),
                   field_0d = read<float>(entity, offsets.field_0d560fac);
        auto height = read<float>(&player.native, 0x12C),
             stamp = read<float>(&player.native, 0x148);
        const auto& newest = selected_ring.at(0).header;
        if (tick < newest.simulation_tick &&
            record->header.collision_max_z != newest.collision_max_z) {
            height = add(newest.origin[2], newest.collision_max_z);
            stamp = runtime.current_time;
        }
        write(entity, offsets.field_432b1f5b, height);
        write(entity, offsets.field_0d560fac, stamp);
        std::array<Matrix3x4, animation::maximum_bones> bones;
        lagcomp::scatter_record_hitbox_matrices(matrices, model.bone_model().hitbox_bones,
                                                bones.data());
        if (runtime.update_attachments) {
            const auto readable = read<std::uint32_t>(entity, offsets.bone_accessor + 8);
            const auto writable = read<std::uint32_t>(entity, offsets.bone_accessor + 12);
            write(entity, offsets.bone_accessor + 8, 0xFFF00u);
            write(entity, offsets.bone_accessor + 12, 0xFFF00u);
            runtime.update_attachments(entity, bones.data(), 0x300);
            write(entity, offsets.bone_accessor + 12, writable);
            write(entity, offsets.bone_accessor + 8, readable);
        }
        animation::select_hitbox_bone_matrices(model.bone_model(), bones, matrices);
        write(entity, offsets.field_432b1f5b, field_432);
        write(entity, offsets.field_0d560fac, field_0d);
        write(entity, offsets.entity_flags, flags);
        write(entity, offsets.origin, origin);
        write(entity, offsets.absolute_origin, absolute_origin);
        (*maximum)[2] = max_z;
        write(entity, offsets.absolute_angles + 4, yaw);
    }
    for (std::int32_t i = 0; i < model.count; ++i)
        if (request.hitbox_mask & (1u << (i & 31)))
            draw_client_hitbox(runtime, local_hitbox(model.descriptors.data() + i * 0x44),
                               matrices[i], request.duration, request.color);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: input_commands.cpp
// ============================================================================

namespace skeet::client {
Command& input_command_slot(void* input, std::uint32_t number) noexcept {
    Command* commands;
    std::memcpy(&commands, static_cast<std::byte*>(input) + 0xF0, sizeof commands);
    return commands[number % 150u];
}
const Command& input_command_slot(const void* input, std::uint32_t number) noexcept {
    return input_command_slot(const_cast<void*>(input), number);
}
const Command* find_input_command(const void* input, std::int32_t number) noexcept {
    const auto& command = input_command_slot(input, std::uint32_t(number));
    return command.number == number ? &command : nullptr;
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: local_animation_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
struct Derived {
    std::uint32_t pointer, key, salt;
};
constexpr std::array local_anim_derived{
    Derived{0x43468FD0u, 0x43467C70u, 0xF58E6118u}, Derived{0x43467F4Cu, 0x4346A8B0u, 0x3D52A1A7u},
    Derived{0x43468368u, 0x43467FACu, 0x3B1F6ECCu}, Derived{0x43468D30u, 0x43468ED4u, 0x2A671B42u},
    Derived{0x43468E7Cu, 0x434674A8u, 0x54783141u}, Derived{0x43467634u, 0x43468F2Cu, 0xBB78DC43u},
    Derived{0x43468BF8u, 0x43467B8Cu, 0xD070684Cu}};
std::ptrdiff_t local_anim_field(ClientRuntimeStorage& storage, std::uint32_t salt) {
    const auto& globals = storage.offsets().globals;
    // XOR salts are not field identities: CTEFireBullets::m_vecOrigin also
    // uses 2A671B42. These player fields must retain their exact publications.
    for (const auto& row : local_anim_derived)
        if (row.salt == salt)
            return engine::DecodedOffset(globals, row.pointer, row.key, salt, 0);
    for (const auto& row : engine::build_receive_offset_recipes())
        if (row.initial_xor_key == salt)
            return engine::DecodedOffset(globals, row.pointer_identity, row.xor_key_identity, salt,
                                         0);
    for (const auto& row : engine::build_data_map_offset_recipes())
        if (row.initial_xor_key == salt)
            return engine::DecodedOffset(globals, row.pointer_identity, row.xor_key_identity, salt,
                                         0);
    throw std::invalid_argument("Local animation field lacks a recovered canonical offset recipe");
}
/* [dedup pointed] */
RestrictedPlayerCommandOffsets restricted_fields(ClientRuntimeStorage& storage,
                                                 const lagcomp::EntityNetvars& common) {
    const auto f = [&](std::uint32_t salt) { return local_anim_field(storage, salt); };
    return {common,        f(0x66146802), f(0x00A5BD68), f(0x52899DF3), f(0x1BAE1168),
            f(0xF58E6118), f(0xCD812654), f(0x3D10B94C), f(0x36375C23), f(0x2A671B42),
            f(0x3309C0CF), f(0xE2FBE695), f(0xAC7F2913), f(0x7090D8DD), f(0xCACAEECC),
            f(0xA15C3105), f(0xAE838771), f(0xD4248210), f(0xA3ABBB09), f(0x7501CB91),
            f(0x7A2D60C1), f(0x4E66BB34), f(0xA9EE6062), f(0xF8205EBA)};
}
LocalPlayerAnimationHookOffsets local_fields(ClientRuntimeStorage& storage) {
    const auto f = [&](std::uint32_t salt) { return local_anim_field(storage, salt); };
    return {f(0xF58E6118), f(0x3D52A1A7), f(0x00A5BD68), f(0x9D920E35), f(0x2F4E97C4),
            f(0xC246315E), f(0xA25D8271), f(0x3B1F6ECC), f(0x8B5586F6), f(0xA15C3105),
            f(0x54783141), f(0x2B554AD0), f(0xC7E1ECD2), f(0x0471E13C), f(0xBB78DC43)};
}
animation::LocalAnimationOffsets helper_fields(ClientRuntimeStorage& storage) {
    const auto f = [&](std::uint32_t salt) { return local_anim_field(storage, salt); };
    return {f(0xB538BBA5), f(0x32E4DAF1), f(0x3D52A1A7), f(0xCD812654),
            f(0x5C9F0FBB), f(0xE2FBE695), f(0xAC7F2913), f(0x52899DF3)};
}
prediction::PlayerSimulationOffsets simulation_fields(ClientRuntimeStorage& storage) {
    const auto f = [&](std::uint32_t salt) { return local_anim_field(storage, salt); };
    return {f(0x5DF1A4BC), f(0xD070684C), f(0x9D920E35), f(0xA249061A),
            f(0x3D10B94C), f(0x53E7C7D2), f(0x1BAE1168), f(0x66146802)};
}
ClientRuntimeStorage& checked(ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
                              ConfigurationRuntimeOwner& config, HistoryRuntimeOwner& history,
                              CommandCaptureRuntimeOwner& capture,
                              effects::EffectsRuntimeOwner& effects) {
    auto& data = storage.data();
    auto* const clock = data.native_cell<std::byte*>(0x43467B04u);
    const auto& entities = history.entity_runtime();
    if (&engine.variables.data() != &data || &config.storage() != &storage ||
        &history.storage() != &storage || &capture.storage() != &storage ||
        &effects.storage() != &storage || &effects.view().settings != &config.effects_settings() ||
        entities.entity_list != data.native_cell<void*>(0x4346A890u) ||
        &entities.interval != &pointed<float>(data, 0x4346A338u) ||
        &entities.frame != &pointed<std::int32_t>(data, 0x434678F8u))
        throw std::invalid_argument(
            "Local animation requires the same canonical feature, history and capture owners");
    if (!clock || data.native_cell<void*>(0x43467EB8u) != clock + 0x10 ||
        data.native_cell<void*>(0x4346A338u) != clock + 0x20 ||
        data.native_cell<void*>(0x434678F8u) != clock + 4 ||
        !data.native_cell<void*>(0x43475B2Cu) || !data.native_cell<void*>(0x43468D4Cu) ||
        !data.native_cell<void*>(0x4346A894u) || !data.native_cell<void*>(0x434693F0u) ||
        !data.native_cell<void*>(0x4346A364u) || !data.native_cell<void*>(0x434678C8u) ||
        !engine.command.random_seed || !engine.command.prediction_player ||
        engine.command.random_seed != data.native_cell<std::int32_t*>(0x43468328u) ||
        engine.command.prediction_player != data.native_cell<void**>(0x434693A4u))
        throw std::invalid_argument(
            "Local animation requires genuine command, clock and history engine producers");
    for (const auto identity : {0x4346794Cu, 0x43467AA0u, 0x43468C2Cu, 0x4346A648u})
        if (!engine.variables.at(identity).get())
            throw std::invalid_argument("Local animation requires its actual acquired ConVars");
    return storage;
}
}
struct LocalAnimationRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    HistoryRuntimeOwner& history;
    effects::EffectsRuntimeOwner& effects;
    engine::BuildLocalAnimationBindings engine_entries;
    animation::AnimationAngleHistoryView angles;
    animation::AnimationUpdateEngine update_engine;
    animation::LocalAnimationBindings helpers;
    effects::ViewDuckBindings duck;
    misc::NetworkFixProxyRuntime proxy;
    misc::NetworkFixBindings network_fix;
    hud::CrosshairViewBindings crosshair;
    LocalPlayerAnimationHookBindings local_bindings;
    LocalPlayerAnimationHooks local;
    PlayerCommandQueryBindings query_bindings;
    PlayerCommandQueries queries;
    PlayerPhysicsHooks physics;
    PlayerCommandHookBindings command_bindings;
    PlayerCommandHooks command_hooks;
    prediction::PlayerSimulationBindings simulation;
    RestrictedPlayerCommandBindings restricted;
    Data(ClientRuntimeStorage& source, const EngineRuntimeBindings& engine,
         ConfigurationRuntimeOwner& config, HistoryRuntimeOwner& history_owner,
         CommandCaptureRuntimeOwner& capture, effects::EffectsRuntimeOwner& effect_owner)
        : storage(checked(source, engine, config, history_owner, capture, effect_owner)),
          history(history_owner), effects(effect_owner),
          engine_entries(engine::initialize_build_local_animation_bindings(source.data())),
          angles(source.data().native_cell<std::uint8_t>(0x43468D7Cu),
                 source.data().native_cell<const std::int32_t*>(0x43475B2Cu),
                 source.data().native_cell<std::array<animation::AnimationAngleSample, 2>>(
                     0x434759DCu)),
          update_engine(lagcomp::native_history_animation_engine(
              source.data().native_cell<void*>(0x434693F0u))),
          helpers{helper_fields(source),
                  source.data().native_cell<void*>(0x4346A890u),
                  engine_entries.symbol_table,
                  engine_entries.initialize_mapping,
                  engine_entries.select_symbol_sequence,
                  engine_entries.find_symbol,
                  engine_entries.add_symbol,
                  engine_entries.activity_vtable,
                  engine.command.random_seed,
                  engine.command.prediction_player,
                  source.data().native_cell<void*>(0x43475808u),
                  source.data().native_cell<PredictionHistoryOwner*>(0x434759B4u),
                  source.data().native_cell<engine::MemberFunction<std::int32_t, std::int32_t>>(
                      0x4346A364u),
                  engine_entries.layer_weight,
                  engine::native_local_layer_weight_engine(engine_entries.layer_weight),
                  source.data().native_cell<engine::MemberFunction<const char*, std::int32_t>>(
                      0x434678C8u),
                  {}},
          duck{effects::ViewDuckTarget(
                   source.data().native_cell<MovementCommandOwner*>(0x434759B0u)),
               config.movement().infinite_duck.value,
               source.data().native_cell<std::uint32_t>(0x434759C0u),
               config.rage_settings().global.duck_peek_assist.value.binding,
               source.input_runtime().bitmap(),
               source.data().native_cell<float>(0x4346D654u),
               std::uint32_t(local_anim_field(source, 0x3309C0CF)),
               std::uint32_t(local_anim_field(source, 0xC246315E))},
          proxy({source.data().native_cell<misc::NetworkFixEngineEntry>(0x4346A454u),
                 source.data().native_cell<std::uint32_t>(0x4346A648u),
                 source.data().native_cell<std::uintptr_t>(0x4346A64Cu)}),
          network_fix{config.settings_menu().fix_networking,
                      source.data().native_cell<std::uintptr_t>(0x434689A0u),
                      source.data().native_cell<void*>(0x4346A64Cu),
                      source.data().native_cell<std::uintptr_t>(0x434692F4u),
                      source.data().native_cell<void*>(0x4346A514u),
                      source.data().native_cell<PredictionHistoryOwner*>(0x434759B4u),
                      {local_anim_field(source, 0x9D920E35), local_anim_field(source, 0x7E9BA7BC),
                       local_anim_field(source, 0xAC7F2913), local_anim_field(source, 0xF58E6118)},
                      proxy},
          crosshair{
              source.data().native_cell<void*>(0x4346A890u),
              {std::uint32_t(local_anim_field(source, 0x9D920E35)), std::uint32_t(local_anim_field(source, 0xCACAEECC)),
               std::uint32_t(local_anim_field(source, 0xCD812654)), std::uint32_t(local_anim_field(source, 0xA1AF8FFC))}},
          local_bindings{local_fields(source),
                         source.data().native_cell<std::uint8_t>(0x43475BD3u),
                         source.data().native_cell<std::uintptr_t>(0x434688C4u),
                         source.data().native_cell<PredictionHistoryOwner*>(0x434759B4u),
                         config.effects_settings().third_person_alive.value,
                         source.data().native_cell<void*>(0x4346A894u),
                         engine.variables.at(0x43467AA0u),
                         source.data().native_cell<void*>(0x43468304u),
                         source.data().native_cell<std::uint8_t*>(0x434683D0u),
                         effect_owner.view(),
                         duck,
                         config.rage_settings().global.enabled.value,
                         config.anti_aim_global().enabled.value,
                         pointed<std::int32_t>(source.data(), 0x434678F8u),
                         pointed<float>(source.data(), 0x43467EB8u),
                         source.data().native_cell<std::array<float, 3>>(0x4347BD30u),
                         config.effects_settings().third_person_key.value.binding,
                         source.input_runtime().bitmap(),
                         source.data().native_cell<lagcomp::SetAbsoluteVector>(0x434688C8u),
                         network_fix,
                         config.other_esp_settings().overlays,
                         crosshair,
                         source.data().native_cell<void*>(0x4346A890u),
                         source.data().native_cell<std::uint8_t>(0x43468E58u)},
          local(local_bindings),
          query_bindings{source.data().native_cell<std::uint32_t>(0x43476904u),
                         source.data().native_cell<std::uintptr_t>(0x434674E0u),
                         source.data().native_cell<std::uint8_t>(0x4346907Cu),
                         source.data().native_cell<std::uint8_t>(0x43468E58u),
                         source.data().native_cell<std::uintptr_t>(0x4346756Cu),
                         local_anim_field(source, 0x8B5586F6),
                         source.data().native_cell<void*>(0x4346A514u),
                         source.data().native_cell<std::uint8_t*>(0x43468FA4u),
                         local_anim_field(source, 0x9D920E35),
                         source.data().native_cell<const std::int32_t*>(0x434757DCu),
                         source.data().native_cell<const std::int32_t*>(0x43475B2Cu),
                         source.data().native_cell<void*>(0x4346A894u),
                         source.data().native_cell<std::uintptr_t>(0x43468518u),
                         capture.sent_history()},
          queries(query_bindings),
          physics({history_owner.players().cache(), config.rage_settings().global.enabled.value,
                   config.anti_aim_global().enabled.value,
                   source.data().native_cell<std::uint32_t>(0x43468C2Cu),
                   source.data().native_cell<std::uint32_t>(0x43467F0Cu),
                   source.data().native_cell<std::uint32_t>(0x434757C4u),
                   source.data().native_cell<std::uint8_t*>(0x43468FA4u),
                   native_player_physics_hook_fields(source.offsets().globals)}),
          command_bindings{source.data().native_cell<void*>(0x43469264u),
                           config.effects_settings(),
                           source.data().native_cell<PredictionHistoryOwner*>(0x434759B4u),
                           {local_anim_field(source, 0xF58E6118), local_anim_field(source, 0x3D52A1A7)}},
          command_hooks(command_bindings),
          simulation{source.data().native_cell<void*>(0x4346A514u),
                     nullptr,
                     source.data().native_cell<const std::int32_t*>(0x43468D4Cu),
                     engine.variables.at(0x4346794Cu),
                     source.prediction_observers(),
                     simulation_fields(source)},
          restricted{
              &config.rage_settings().global.enabled.value,
              &config.anti_aim_global().enabled.value,
              angles,
              engine.variables.at(0x4346794Cu),
              source.data().native_cell<void*>(0x4346A890u),
              history_owner.players().cache(),
              RestrictedCommandRing(source.data().native_cell<void*>(0x4346A894u)),
              source.data().native_cell<EntityCommandOwner*>(0x43475898u),
              &pointed<float>(source.data(), 0x4346A338u),
              &pointed<std::int32_t>(source.data(), 0x434678F8u),
              &pointed<float>(source.data(), 0x43467EB8u),
              reinterpret_cast<float*>(source.data().native_cell<std::byte*>(0x43467B04u) + 0x10),
              &source.data().native_cell<float>(0x43476B3Cu),
              &source.data().native_cell<float>(0x43476B40u),
              &source.data().native_cell<std::array<float, 3>>(0x4347BD30u),
              &source.prediction_observers(),
              nullptr,
              &source.data().native_cell<std::uint8_t>(0x43475BD3u),
              {0, 0, 0, nullptr, &source.data().native_cell<std::uint8_t*>(0x43467B24u)},
              update_engine,
              helpers,
              restricted_fields(source, history_owner.entity_runtime().offsets),
              0,
              true,
              true} {}
};
LocalAnimationRuntimeOwner::LocalAnimationRuntimeOwner(ClientRuntimeStorage& storage,
                                                       const EngineRuntimeBindings& engine,
                                                       ConfigurationRuntimeOwner& config,
                                                       HistoryRuntimeOwner& history,
                                                       CommandCaptureRuntimeOwner& capture,
                                                       effects::EffectsRuntimeOwner& effects)
    : data_(std::make_unique<Data>(storage, engine, config, history, capture, effects)) {}
LocalAnimationRuntimeOwner::~LocalAnimationRuntimeOwner() = default;
ClientRuntimeStorage& LocalAnimationRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
HistoryRuntimeOwner& LocalAnimationRuntimeOwner::history() const noexcept {
    return data_->history;
}
const RestrictedPlayerCommandBindings& LocalAnimationRuntimeOwner::restricted() const noexcept {
    return data_->restricted;
}
const animation::LocalAnimationBindings& LocalAnimationRuntimeOwner::helpers() const noexcept {
    return data_->helpers;
}
LocalPlayerAnimationHooks& LocalAnimationRuntimeOwner::local_hooks() noexcept {
    return data_->local;
}
PlayerCommandQueries& LocalAnimationRuntimeOwner::queries() noexcept {
    return data_->queries;
}
PlayerPhysicsHooks& LocalAnimationRuntimeOwner::physics() noexcept {
    return data_->physics;
}
PlayerCommandHooks& LocalAnimationRuntimeOwner::command_hooks() noexcept {
    return data_->command_hooks;
}
const prediction::PlayerSimulationBindings&
LocalAnimationRuntimeOwner::simulation() const noexcept {
    return data_->simulation;
}
effects::ViewDuckBindings& LocalAnimationRuntimeOwner::duck() noexcept {
    return data_->duck;
}
effects::ViewRuntime& LocalAnimationRuntimeOwner::view() noexcept {
    return data_->effects.view();
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: local_animation_strafe.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */
void** original(void* player) noexcept {
    const auto* metadata =
        static_cast<const engine::VtableHookMetadata*>((*static_cast<void***>(player))[-2]);
    return metadata->original_table;
}
}
std::uint32_t LocalPlayerAnimationHooks::update_strafe(void* player, std::uint32_t first,
                                                       std::uint32_t second,
                                                       std::uintptr_t caller) {
    const auto& b = bindings;
    const auto& o = b.offsets;
    if (b.animation_update && caller == b.strafe_call_site) {
        const auto* command = field<const void*>(player, o.current_command_f58e6118);
        if (command) {
            const auto* state = field<const void*>(player, o.animation_state_3d52a1a7);
            if (state && b.command_owner) {
                const auto buttons = field<std::uint32_t>(command, 0x30);
                MovementVector forward, right;
                movement_angle_vectors({0.0f, field<float>(state, 0x80), 0.0f}, &forward, &right,
                                       nullptr);
                const auto length =
                    std::sqrt((right.x * right.x + right.y * right.y) + right.z * right.z);
                const auto inverse = 1.0f / (length + 0.00000011920928955078125f);
                right = {right.x * inverse, right.y * inverse, right.z * inverse};
                const auto velocity = field<MovementVector>(state, 0xE0);
                const auto forward_motion =
                    (velocity.x * forward.x + velocity.y * forward.y) + velocity.z * forward.z;
                const auto side_motion =
                    (velocity.x * right.x + velocity.y * right.y) + velocity.z * right.z;
                const auto fraction = field<float>(state, 0xF8);
                bool strafing = false;
                if (fraction >= 0.730000019073486328125f) {
                    if (buttons & 0x400U) {
                        if (!(buttons & 0x200U))
                            strafing = -0.62999999523162841796875f > side_motion;
                    } else if (buttons & 0x200U)
                        strafing = side_motion > 0.62999999523162841796875f;
                }
                if (fraction >= 0.64999997615814208984375f) {
                    if (buttons & 8U) {
                        if (!(buttons & 0x10U))
                            strafing |= -0.550000011920928955078125f > forward_motion;
                    } else if (buttons & 0x10U)
                        strafing |= forward_motion > 0.550000011920928955078125f;
                }
                const auto value = std::uint8_t(strafing);
                std::memcpy(static_cast<std::byte*>(player) + o.strafe_00a5bd68, &value,
                            sizeof value);
            }
        }
    }
    return reinterpret_cast<engine::MemberFunction<std::uint32_t, std::uint32_t, std::uint32_t>>(
        original(player)[222])(player, first, second);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: local_client_animation.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */
/* deduplicated helper */
/* deduplicated convar_float */
void call_original(void* player) {
    const auto* metadata =
        static_cast<const engine::VtableHookMetadata*>((*static_cast<void***>(player))[-2]);
    reinterpret_cast<engine::MemberFunction<void>>(metadata->original_table[224])(player);
}
void normalize_camera(void* input, std::ptrdiff_t offset) noexcept {
    auto angle = field<float>(input, offset);
    if (angle >= 180.0f)
        angle -= 360.0f;
    else if (-180.0f >= angle)
        angle += 360.0f;
    else
        return;
    write(input, offset, angle);
}
void update_camera(void* player, const LocalPlayerAnimationHookBindings& b) {
    const auto saved = b.view.begin_view_height(player, b.duck);
    {
        MovementTraceFlagGuard guard(b.trace_flag);
        auto* input = b.input;
        write(input, 0xB4, convar_float(b.camera_distance));
        MovementVector forward;
        movement_angle_vectors({field<float>(input, 0xAC), field<float>(input, 0xB0), 0.0f},
                               &forward, nullptr, nullptr);
        const auto absolute = *engine::invoke<const std::array<float, 3>*>(player, 10);
        const auto offset = field<std::array<float, 3>>(player, b.offsets.view_offset_c246315e);
        const auto distance = field<float>(input, 0xB4);
        const std::array<float, 3> eye{absolute[0] + offset[0], absolute[1] + offset[1],
                                       absolute[2] + offset[2]};
        engine::TraceRay ray;
        ray.start = {eye[0] + 0.0f, eye[1] + 0.0f, eye[2] + 0.0f, 0.0f};
        ray.delta = {(eye[0] - forward.x * distance) - eye[0],
                     (eye[1] - forward.y * distance) - eye[1],
                     (eye[2] - forward.z * distance) - eye[2], 0.0f};
        ray.start_offset = {-0.0f, -0.0f, -0.0f, 0.0f};
        ray.extents = {9.0f, 9.0f, 9.0f, 0.0f};
        ray.is_ray = 0;
        ray.swept = ((ray.delta[0] * ray.delta[0] + ray.delta[1] * ray.delta[1]) +
                     ray.delta[2] * ray.delta[2]) != 0.0f;
        std::array<std::byte, engine::trace_result_size> trace;
        write(trace.data(), 0x2C, 1.0f);
        write(trace.data(), 0x36, std::uint16_t(0));
        write(trace.data(), 0x4C, std::uint32_t(0));
        write(trace.data(), 0x44, UINT32_MAX);
        write(trace.data(), 0x50, UINT32_MAX);
        engine::TraceRejectEntitiesFilter filter;
        engine::invoke<void>(b.engine_trace, 5, &ray, std::uint32_t(0x200400B), &filter,
                             trace.data());
        const auto fraction = field<float>(trace.data(), 0x2C);
        if (1.0f > fraction)
            write(input, 0xB4, field<float>(input, 0xB4) * fraction);
        normalize_camera(input, 0xAC);
        normalize_camera(input, 0xB0);
    }
    b.view.restore_view_height(saved);
}
}
void LocalPlayerAnimationHooks::update_client_animation(void* player) {
    const auto& b = bindings;
    const auto& o = b.offsets;
    const bool camera_enabled = b.third_person != 0;
    const auto dead = field<std::uint8_t>(player, o.life_state_9d920e35);
    if (camera_enabled) {
        bool camera = !dead;
        if (dead) {
            const auto mode = field<std::int32_t>(player, o.observer_mode_2f4e97c4);
            camera = mode == 1 || mode == 2;
        }
        if (camera && engine::invoke<std::uint32_t>(b.input, 32, std::int32_t(-1)))
            update_camera(player, b);
    }
    if (!dead && (b.command_mode || b.alternate_command_mode)) {
        if (b.set_absolute_angles)
            b.set_absolute_angles(player, b.view_angles);
        const auto model = field<std::uint32_t>(player, o.model_index_a25d8271);
        write(player, o.model_index_a25d8271, UINT32_MAX);
        auto* state = field<void*>(player, o.animation_state_3d52a1a7);
        std::uint32_t saved_state;
        if (state) {
            saved_state = field<std::uint32_t>(state, 0x60);
            write(state, 0x60, std::uint32_t(0));
        }
        call_original(player);
        if (state)
            write(state, 0x60, saved_state);
        write(player, o.model_index_a25d8271, model);
        return;
    }
    const auto* state = field<const void*>(player, o.animation_state_3d52a1a7);
    const bool publish = !dead && state && field<std::uint32_t>(state, 0x60) &&
                         b.animation_frame != field<std::int32_t>(state, 0x70) &&
                         b.current_time != field<float>(state, 0x6C);
    call_original(player);
    if (publish) {
        auto angles = field<std::array<float, 3>>(player, o.angles_3b1f6ecc);
        for (auto& angle : angles)
            angle = normalize_movement_angle(angle);
        const bool invalidate = b.third_person != 0;
        b.view_angles = angles;
        if (invalidate && hotkey_active(b.third_person_key, b.keys) &&
            field<std::int32_t>(player, o.movement_mode_8b5586f6) == 3 &&
            field<std::uint8_t>(player, o.ducking_a15c3105)) {
            write(player, o.bone_accessor_54783141 + 8, std::uint32_t(0));
            write(player, o.bone_field_2b554ad0, std::uint32_t(0));
            write(player, o.bone_time_c7e1ecd2, std::uint32_t(0xFF7FFFFF));
            write(player, o.bone_counter_0471e13c, std::uint32_t(0));
            write(player, o.setup_flags_bb78dc43,
                  field<std::uint32_t>(player, o.setup_flags_bb78dc43) & ~2U);
        }
    } else if (!dead && b.set_absolute_angles)
        b.set_absolute_angles(player, b.view_angles);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: misc_runtime_factories.cpp
// ============================================================================

namespace skeet::client {
namespace {

/* [dedup allocate_two_sizes] */
void initialize_low_fps(void* owner) { //434903AD ->4347E6F2
    auto& b = *static_cast<MiscRuntimeFactoryBindings*>(owner);
    auto& c = b.low_fps;
    (void)misc::initialize_low_fps_owner(b.lifecycle, b.commands, b.hud, b.configuration,
                                         c.settings, c.rage, c.engine, c.scripts, c.input, c.layout,
                                         c.life_state_offset, c.active);
}
void initialize_quick_switch(void* owner) { //43481DC0 ->4347DC0A
    auto& b = *static_cast<MiscRuntimeFactoryBindings*>(owner);
    auto& c = b.quick_switch;
    auto* const storage = allocate(b.lifecycle.memory, 0x44, misc::quick_switch_owner_size());
    (void)misc::construct_quick_switch(storage, b.lifecycle, b.commands, b.recharge, b.prediction,
                                       b.configuration, c.settings, c.runtime);
}
void initialize_grenades(void* owner) { //43481EDA ->4348675E
    auto& b = *static_cast<MiscRuntimeFactoryBindings*>(owner);
    auto& c = b.grenades;
    const misc::GrenadeOwnerRegistries registries{b.lifecycle, b.commands, b.empty_ticks, b.hud,
                                                  b.entities,  b.updates,  b.recharge};
    //43481EF3 publishes this invocation's complete owner, including null.
    (void)misc::create_grenade_owner(registries, b.configuration, c.settings, c.visuals, c.engine);
}
void initialize_no_fall_damage(void* owner) { //4348401C ->4348DCB4
    auto& b = *static_cast<MiscRuntimeFactoryBindings*>(owner);
    auto& c = b.no_fall_damage;
    auto* const storage = allocate(b.lifecycle.memory, 0x40, misc::no_fall_damage_owner_size());
    (void)misc::construct_no_fall_damage(storage, b.lifecycle, b.commands, b.hud, b.configuration,
                                         c.settings, c.engine);
}
void initialize_automatic_weapons(void* owner) { //4348E51F ->4347D991
    auto& b = *static_cast<MiscRuntimeFactoryBindings*>(owner);
    auto& c = b.automatic_weapons;
    auto* const storage = allocate(b.lifecycle.memory, 0x64, misc::automatic_weapons_owner_size());
    (void)misc::construct_automatic_weapons(storage, b.lifecycle, b.commands, b.configuration,
                                            c.settings, c.state, c.engine);
}
void initialize_knife(void* owner) { //43483FF8 ->434802E2
    auto& b = *static_cast<MiscRuntimeFactoryBindings*>(owner);
    auto& c = b.knife;
    auto* const storage = allocate(b.lifecycle.memory, 0x34, misc::knife_owner_size());
    (void)misc::construct_knife(storage, b.lifecycle, b.commands, b.configuration, c.settings,
                                c.engine, c.scheduler_flags);
}
void initialize_zeus(void* owner) { //43490389 ->43485D4C
    auto& b = *static_cast<MiscRuntimeFactoryBindings*>(owner);
    auto& c = b.zeus;
    auto* const storage = allocate(b.lifecycle.memory, 0x24, misc::zeus_owner_size());
    (void)misc::construct_zeus(storage, b.lifecycle, b.commands, b.configuration, c.settings,
                               c.state, c.engine);
}
void initialize_clan_tag(void* owner) { //4348DA01 ->4347E092
    auto& b = *static_cast<MiscRuntimeFactoryBindings*>(owner);
    auto& c = b.clan_tag;
    auto* const storage = allocate(b.lifecycle.memory, 0x30, misc::clan_tag_owner_size());
    misc::construct_clan_tag_owner(storage, b.lifecycle, b.entities, b.messages, b.configuration,
                                   c.settings, c.runtime, c.environment);
}
void initialize_rebuy(void* owner) { //4347E2E3 ->4348217D
    (void)misc::create_rebuy_fix_owner(static_cast<MiscRuntimeFactoryBindings*>(owner)->rebuy.engine);
}

/* [dedup same_memory] */
bool same_input(InputBitmap first, InputBitmap second) noexcept {
    return first.words.data() == second.words.data() && first.words.size() == second.words.size() &&
           first.bit_count == second.bit_count;
}
void require_bindings(NativeStartupSources& sources, const MiscRuntimeFactoryBindings& b) {
    const auto& image = sources.data();
    if (!b.lifecycle.memory.allocate || !b.lifecycle.memory.free ||
        !b.lifecycle.memory.process_heap)
        throw std::invalid_argument("Misc factories require the canonical process heap");
    struct RegistryAlias {
        const engine::OwnerRegistryBinding* binding;
        std::uint32_t address;
    };
    const std::array aliases{RegistryAlias{&b.lifecycle, 0x43475B78u},
                             RegistryAlias{&b.hud, 0x43475A9Cu},
                             RegistryAlias{&b.updates, 0x43475988u},
                             RegistryAlias{&b.frames, 0x43475AD8u},
                             RegistryAlias{&b.messages, 0x43475B94u},
                             RegistryAlias{&b.rebuy.engine.owners, 0x43475B78u},
                             RegistryAlias{&b.rebuy.engine.frame_updates, 0x43475AD8u}};
    for (const auto& alias : aliases)
        if (&alias.binding->registry != &image.native_cell<engine::OwnerRegistry>(alias.address) ||
            !same_memory(alias.binding->memory, b.lifecycle.memory))
            throw std::invalid_argument(
                "Misc factories must share their native registry cells and heap");
    if (&b.configuration.descriptors != &image.native_cell<ConfigLink>(0x434798BCu) ||
        &b.configuration.hotkeys != &image.native_cell<HotkeyLink>(0x434798C4u) ||
        &b.rebuy.engine.configuration != &b.configuration)
        throw std::invalid_argument("Misc factories require the canonical configuration registry");
    if (&b.commands.image() != &image || &b.empty_ticks.image() != &image ||
        &b.entities.image() != &image || &b.recharge.image() != &image ||
        &b.prediction.image() != &image || &b.rebuy.engine.command_owners != &b.commands ||
        &b.commands.native_storage() != &image.native_cell<engine::OwnerRegistry>(0x434757A0u) ||
        &b.empty_ticks.native_storage() != &image.native_cell<engine::OwnerRegistry>(0x43475978u) ||
        &b.entities.native_storage() != &image.native_cell<engine::OwnerRegistry>(0x43475BA8u) ||
        &b.recharge.native_storage() != &image.native_cell<engine::OwnerRegistry>(0x43475784u) ||
        &b.prediction.native_storage() != &image.native_cell<engine::OwnerRegistry>(0x43475AB0u))
        throw std::invalid_argument(
            "Misc factories require the canonical command and observer registries");
    if (&b.knife.scheduler_flags != &b.commands.services().scheduler_flags ||
        !same_input(b.low_fps.input, b.commands.services().input))
        throw std::invalid_argument(
            "Misc factories must share the live command flags and input words");
    const auto& g = b.grenades.engine;
    const auto& grenade_visuals = b.grenades.visuals;
    if (&g.active != &image.native_cell<void*>(0x43475AACu) ||
        &g.input != &image.native_cell<void*>(0x4346A894u) ||
        &g.live.engine_client != &image.native_cell<void*>(0x4346A514u) ||
        &g.view_angles.engine_client != &g.live.engine_client ||
        &g.live.local_player != &image.native_cell<void*>(0x43475808u) ||
        &g.live.game_rules_slot != &image.native_cell<void*>(0x4346A3A4u) ||
        &g.live.command_buttons != &image.native_cell<std::uint32_t>(0x434759C0u) ||
        &g.release_choke_enabled != &image.native_cell<std::uint8_t>(0x43467560u) ||
        &g.observer_suppressed != &image.native_cell<std::uint8_t>(0x434684E0u) ||
        &g.ribbons_suppressed != &image.native_cell<std::uint8_t>(0x43468E58u) ||
        &g.ribbons_color != &grenade_visuals.entity_color.value ||
        &g.live.grenade_entities_enabled != &grenade_visuals.entities.value ||
        &g.live.queries.grenades().bindings().entity_list != &image.native_cell<void*>(0x4346A890u))
        throw std::invalid_argument(
            "Misc grenade factory requires its canonical owner and live engine cells");
}
}

void bind_misc_runtime_factories(NativeStartupSources& sources,
                                 MiscRuntimeFactoryBindings& bindings) {
    require_bindings(sources, bindings);
    const std::array calls{
        NativeInitializerCall{0x434903ADu, &bindings, initialize_low_fps},
        NativeInitializerCall{0x43481DC0u, &bindings, initialize_quick_switch},
        NativeInitializerCall{0x43481EDAu, &bindings, initialize_grenades},
        NativeInitializerCall{0x4348401Cu, &bindings, initialize_no_fall_damage},
        NativeInitializerCall{0x4348E51Fu, &bindings, initialize_automatic_weapons},
        NativeInitializerCall{0x43483FF8u, &bindings, initialize_knife},
        NativeInitializerCall{0x43490389u, &bindings, initialize_zeus},
        NativeInitializerCall{0x4348DA01u, &bindings, initialize_clan_tag},
        NativeInitializerCall{0x4347E2E3u, &bindings, initialize_rebuy}};
    for (const auto& call : calls)
        sources.bind_initializer(call);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: misc_runtime_owner.cpp
// ============================================================================
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif

namespace skeet::client {
namespace {
/* [dedup offset_storage] */
std::uint32_t offset32(ClientRuntimeStorage& storage, std::uint32_t identity) {
    return static_cast<std::uint32_t>(offset(storage, identity));
}
/* [dedup pointed] */
void* variable(const EngineRuntimeBindings& engine, std::uint32_t identity) {
    auto* const object = engine.variables.at(identity).get();
    if (!object)
        throw std::invalid_argument("Misc runtime requires its genuine acquired ConVar");
    return object;
}
/* deduplicated registry helper */
misc::RuntimeInterfaces& initialize_interfaces(misc::RuntimeInterfaces& result,
                                               ClientRuntimeStorage& storage,
                                               const EngineRuntimeBindings& engine) {
    auto& data = storage.data();
    const auto core = engine.interfaces.core();
    if (core.engine_client != data.native_cell<void*>(0x4346A514u) ||
        core.entity_list != data.native_cell<void*>(0x4346A890u) ||
        core.cvar != data.native_cell<void*>(0x4346A87Cu) || &engine.variables.data() != &data)
        throw std::invalid_argument(
            "Misc runtime must share the canonical engine interfaces and variables");
    result.engine = core.engine_client;
    result.entity_list = core.entity_list;
    result.cvar = core.cvar;
    result.allocator = data.native_cell<void*>(0x43467204u);
    result.game_rules = data.native_cell<void**>(0x4346A3A4u);
    result.player_resource = data.native_cell<void**>(0x43468FF0u);
    result.max_clients = &pointed<std::int32_t>(data, 0x43469300u);
    result.name_cvar = variable(engine, 0x43467A5Cu);
    result.drawing_suspended = &data.native_cell<std::uint8_t>(0x43468E58u);
    result.log_hud_slot = &data.native_cell<misc::ColoredLogRing*>(0x434758CCu);
    if (!result.engine || !result.entity_list || !result.cvar || !result.allocator ||
        !result.game_rules || !result.player_resource)
        throw std::invalid_argument(
            "Misc runtime requires the genuine engine objects and live global slots");
    return result;
}
misc::RuntimeOffsets runtime_offsets(ClientRuntimeStorage& storage) {
    return {offset32(storage, 0x434688B0u), offset32(storage, 0x43468378u),
            offset32(storage, 0x43467C54u), offset32(storage, 0x4346926Cu)};
}
misc::QuickSwitchOffsets quick_offsets(ClientRuntimeStorage& storage) {
    return {offset(storage, 0x43467A74u), offset(storage, 0x4346908Cu),
            offset(storage, 0x43467BE4u), offset(storage, 0x43467A6Cu),
            offset(storage, 0x43467AA4u), offset(storage, 0x4346A6DCu),
            offset(storage, 0x4346A60Cu)};
}
misc::AutomaticWeaponsOffsets automatic_offsets(ClientRuntimeStorage& storage) {
    return {offset(storage, 0x434688B0u), offset(storage, 0x4346A4F4u),
            offset(storage, 0x4346A6C4u), offset(storage, 0x4346834Cu),
            offset(storage, 0x4346799Cu), offset(storage, 0x4346A6DCu),
            offset(storage, 0x4346908Cu), offset(storage, 0x43467200u)};
}
misc::RebuyFixOffsets rebuy_offsets(ClientRuntimeStorage& storage) {
    return {offset32(storage, 0x434688B0u), offset32(storage, 0x4346A3B4u),
            offset32(storage, 0x43468D34u), offset32(storage, 0x434685D4u),
            offset32(storage, 0x43467908u), offset32(storage, 0x43468C04u)};
}
void require_shared(ClientRuntimeStorage& storage, ConfigurationRuntimeOwner& configuration,
                    engine::HookDescriptorStorage& descriptors,
                    const MiscRuntimeSharedBindings& shared, const MiscRuntimeImports& imports) {
    auto& data = storage.data();
    if (&configuration.storage() != &storage || &descriptors.image() != &data ||
        &shared.scene.storage() != &storage || &shared.animation.storage() != &storage ||
        &shared.scene.players().cache() != &shared.animation.players() ||
        &shared.scene.queries().offsets() != &storage.offsets().globals ||
        &shared.live_movement.capture.interval != &pointed<float>(data, 0x4346A338u) ||
        &shared.live_movement.environment != shared.live_movement.capture.environment ||
        &shared.ribbons.entity_list != &data.native_cell<void*>(0x4346A890u))
        throw std::invalid_argument(
            "Misc owners require the same canonical configuration, scene, movement and player contexts");
    std::visit(
        [&](const auto& source) {
            using Source = std::remove_cvref_t<decltype(source)>;
            if constexpr (std::is_same_v<Source, rage::RuntimeOwner*>) {
                if (!source || &source->storage() != &storage)
                    throw std::invalid_argument(
                        "Production misc knife requires the canonical RAGE runtime owner");
            } else if (&source.targeting.player_adjustments !=
                           &shared.scene.players().adjustments() ||
                       &source.records.players != &shared.scene.players().cache() ||
                       &source.extrapolation.players != &shared.scene.players().cache())
                throw std::invalid_argument(
                    "Standalone misc knife views must share their canonical player state");
        },
        shared.knife.source);
    if (!imports.random_integer || !imports.performance_clock.counter ||
        !imports.performance_clock.frequency || !imports.damage_math.trigonometry.sine ||
        !imports.damage_math.trigonometry.cosine || !imports.damage_math.exponential ||
        !imports.arc_tangent2)
        throw std::invalid_argument(
            "Misc owners require their actual imported RNG, performance clock and SSE math");
    if (!data.native_cell<misc::SetClanTag>(0x43467550u) ||
        !data.native_cell<const misc::AutomaticWeaponsBombList*>(0x434688C0u))
        throw std::invalid_argument(
            "Misc owners require the verified clan-tag and planted-bomb bindings");
}
ClientRuntimeStorage& validated_storage(ClientRuntimeStorage& storage,
                                        const EngineRuntimeBindings& engine,
                                        ConfigurationRuntimeOwner& configuration,
                                        engine::HookDescriptorStorage& descriptors,
                                        const MiscRuntimeSharedBindings& shared,
                                        const MiscRuntimeImports& imports) {
    auto& data = storage.data();
    const auto* globals = data.native_cell<const std::byte*>(0x43467B04u);
    const auto* state = data.native_cell<const std::byte*>(0x43468E44u);
    if (!globals || !state || &engine.variables.data() != &data ||
        reinterpret_cast<const std::byte*>(data.native_cell<float*>(0x4346A338u)) !=
            globals + 0x20 ||
        reinterpret_cast<const std::byte*>(data.native_cell<float*>(0x43467EB8u)) !=
            globals + 0x10 ||
        reinterpret_cast<const std::byte*>(data.native_cell<float*>(0x4346A334u)) != globals ||
        reinterpret_cast<const std::byte*>(data.native_cell<std::int32_t*>(0x43469300u)) !=
            globals + 0x18 ||
        reinterpret_cast<const std::byte*>(data.native_cell<std::int32_t*>(0x43468D4Cu)) !=
            globals + 0x1C ||
        reinterpret_cast<const std::byte*>(data.native_cell<std::int32_t*>(0x434678F8u)) !=
            globals + 4 ||
        reinterpret_cast<const std::byte*>(data.native_cell<std::int32_t*>(0x43475B2Cu)) !=
            state + 0x4D24 ||
        reinterpret_cast<const std::byte*>(data.native_cell<std::int32_t*>(0x434757DCu)) !=
            state + 0x4D28 ||
        reinterpret_cast<const std::byte*>(data.native_cell<std::int32_t*>(0x43475A60u)) !=
            state + 0x164)
        throw std::invalid_argument(
            "Misc owner requires the initialized canonical client-state/global aliases");
    require_shared(storage, configuration, descriptors, shared, imports);
    for (const auto identity : {0x43467C3Cu, 0x4346A4FCu, 0x4346A7ECu, 0x434685C8u, 0x4346794Cu,
                                0x43467A8Cu, 0x4346A568u})
        (void)variable(engine, identity);
    return storage;
}
}

MiscRuntimeImports native_misc_runtime_imports(const EngineRuntimeBindings& engine) {
#if defined(_WIN32) && (defined(_M_IX86) || defined(__i386__))
    const auto module = static_cast<HMODULE>(engine.modules.at(u"vstdlib.dll"));
    const auto random = module ? GetProcAddress(module, "RandomInt") : nullptr;
    if (!random)
        throw std::runtime_error("Misc runtime requires the loaded vstdlib RandomInt export");
    return {reinterpret_cast<misc::AutomaticWeaponsRandomInteger>(random),
            misc::native_low_fps_clock(),
            {animation::native_sse_trigonometry(), animation::native_sse_exponential},
            animation::native_sse_atan2};
#else
    (void)engine;
    throw std::logic_error("Native misc imports require Windows x86");
#endif
}

struct MiscRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    ConfigurationRuntimeOwner& configuration;
    MiscRuntimeSharedBindings shared;
    MiscRuntimeImports imports;
    ChannelHookState channel;
    misc::AutoAcceptPatchState patch;
    misc::RuntimeInterfaces interfaces;
    misc::Runtime miscellaneous;
    misc::QuickSwitchRuntime quick;
    misc::AutomaticWeaponsState automatic{};
    misc::ZeusState zeus{};
    misc::LowFpsRuntime* performance = nullptr;
    std::uint32_t life_state_offset;
    ScriptCommandEnvironment command_environment;
    misc::ClanTagClock clan_clock;
    misc::AutomaticWeaponsBindings automatic_bindings;
    misc::KnifeAimBindings aim;
    struct KnifeSource final : misc::KnifeCommandSource {
        Data& data;
        explicit KnifeSource(Data& owner) noexcept : data(owner) {}
        void process(const misc::KnifeConfig&, CommandFrame&, Command&, void*, std::uint8_t&,
                     std::uint32_t&) override;
    } knife;
    misc::ZeusBindings zeus_bindings;
    MiscRuntimeFactoryBindings factories;

    Data(ClientRuntimeStorage& source, const EngineRuntimeBindings& engine,
         ConfigurationRuntimeOwner& settings, engine::HookDescriptorStorage& descriptors,
         MiscRuntimeSharedBindings dependencies, MiscRuntimeImports external)
        : storage(source), configuration(settings), shared(dependencies), imports(external),
          channel(source.data().native_cell<void**>(0x43475B0Cu),
                  source.data().native_cell<void**>(0x43475B14u),
                  source.data().native_cell<void*>(0x43475B08u)),
          patch{descriptors.at(misc::auto_accept_hook_descriptor)},
          interfaces(source.data().native_cell<void*>(0x43475808u)),
          miscellaneous(initialize_interfaces(interfaces, source, engine), patch, channel,
                        runtime_offsets(source)),
          quick{nullptr,
                {interfaces.entity_list, interfaces.game_rules,
                 source.data().native_cell<EntityCommandOwner*>(0x43475898u),
                 &pointed<std::int32_t>(source.data(), 0x43475B2Cu),
                 &pointed<std::int32_t>(source.data(), 0x434757DCu), quick_offsets(source)},
                misc::QuickSwitchState{},
                prediction::NativePredictionObserverReceiver{}},
          life_state_offset(offset32(source, 0x43468E78u)),
          command_environment{&settings.anti_untrusted().enabled.value, interfaces.game_rules,
                              offset(source, 0x43468378u)},
          clan_clock{source.data().native_cell<lagcomp::TickTimingSamples>(0x43478C70u),
                     (&pointed<std::int32_t>(source.data(), 0x43475A60u))[1],
                     source.data().native_cell<const std::int32_t*>(0x43468F9Cu),
                     source.data().native_cell<const std::int32_t*>(0x434670A8u),
                     pointed<float>(source.data(), 0x4346A338u)},
          automatic_bindings{
              &source.data().native_cell<const misc::AutomaticWeaponsBombList*>(0x434688C0u),
              &pointed<float>(source.data(), 0x43467EB8u), automatic_offsets(source),
              imports.random_integer},
          aim{source.data().native_cell<void*>(0x4346A894u), offset(source, 0x43468B50u),
              engine::DecodedOffset(source.offsets().globals, 0x4346757Cu, 0x4346707Cu, 0x043FA575u,
                                    0),
              offset(source, 0x4346934Cu), imports.arc_tangent2},
          knife(*this), zeus_bindings{shared.scene.queries().ballistics(),
                                      source.data().native_cell<void* const*>(0x43475B90u),
                                      offset(source, 0x4346908Cu), offset(source, 0x43467DD0u)},
          factories{
              source.configuration(),
              registry(source, 0x43475B78u),
              registry(source, 0x43475A9Cu),
              registry(source, 0x43475988u),
              registry(source, 0x43475AD8u),
              registry(source, 0x43475B94u),
              source.command_phases(),
              source.empty_ticks(),
              source.entity_observers(),
              source.recharge_observers(),
              source.prediction_observers(),
              {settings.rage_settings().performance,
               settings.rage_settings().global,
               {pointed<std::int32_t>(source.data(), 0x434678F8u),
                pointed<float>(source.data(), 0x4346A338u),
                source.data().native_cell<void*>(0x4346D560u),
                source.data().native_cell<void*>(0x4346D55Cu), imports.performance_clock},
               shared.scripts,
               source.command_phases().services().input,
               shared.indicators,
               life_state_offset,
               performance},
              {settings.miscellaneous().quick_switch, quick},
              {settings.miscellaneous().grenades,
               settings.other_esp_settings().grenades,
               {{shared.scene.queries(),
                 shared.live_movement.capture,
                 shared.live_movement.environment,
                 shared.scene.players().adjustments(),
                 shared.scene.players().cache(),
                 source.data().native_cell<void*>(0x4346A514u),
                 source.data().native_cell<void*>(0x43475808u),
                 source.data().native_cell<void*>(0x4346A3A4u),
                 pointed<std::int32_t>(source.data(), 0x43469300u),
                 pointed<std::int32_t>(source.data(), 0x43468D4Cu),
                 pointed<std::int32_t>(source.data(), 0x434757DCu),
                 pointed<std::int32_t>(source.data(), 0x43475B2Cu),
                 pointed<float>(source.data(), 0x43467EB8u),
                 pointed<float>(source.data(), 0x4346A338u),
                 reinterpret_cast<float*>(source.data().native_cell<void*>(0x43467B04u))[9],
                 settings.other_esp_settings().grenades.entities.value,
                 shared.projection,
                 shared.renderer,
                 shared.explosive_texture,
                 shared.fire_texture,
                 imports.damage_math,
                 imports.arc_tangent2,
                 source.data().native_cell<std::uint8_t*>(0x434683D0u),
                 source.data().native_cell<std::uint32_t>(0x434759C0u),
                 {engine.variables.at(0x43467C3Cu), engine.variables.at(0x4346A4FCu),
                  engine.variables.at(0x4346A7ECu), engine.variables.at(0x434685C8u),
                  engine.variables.at(0x4346794Cu), engine.variables.at(0x43467A8Cu),
                  engine.variables.at(0x4346A568u)}},
                source.data().native_cell<void*>(0x4346A894u),
                (&pointed<std::int32_t>(source.data(), 0x43475A60u))[1],
                source.data().native_cell<std::uint8_t>(0x43467560u),
                source.data().native_cell<std::uint8_t>(0x434684E0u),
                {source.data().native_cell<void*>(0x4346A514u),
                 source.data().native_cell<void*>(0x43469264u), command_environment},
                shared.release_movement,
                shared.movement_feedback,
                shared.ribbons,
                source.data().native_cell<std::uint8_t>(0x43468E58u),
                settings.other_esp_settings().grenades.entity_color.value,
                source.data().native_cell<void*>(0x43475AACu)}},
              {settings.miscellaneous().no_fall_damage,
               {{offset(source, 0x43468BD4u), offset(source, 0x434688D0u),
                 offset(source, 0x4346A8E4u)},
                interfaces.engine,
                variable(engine, 0x43467A54u),
                variable(engine, 0x43467990u),
                variable(engine, 0x434674E4u),
                pointed<float>(source.data(), 0x4346A338u),
                shared.scene.queries().movement(),
                shared.live_movement.capture,
                shared.live_movement.environment,
                shared.projection}},
              {settings.miscellaneous().automatic_weapons, automatic, automatic_bindings},
              {settings.miscellaneous().knife, knife,
               source.command_phases().services().scheduler_flags},
              {settings.miscellaneous().zeus, zeus, zeus_bindings},
              {settings.miscellaneous().clan_tag, miscellaneous.clan_tag, command_environment},
              {{source.configuration(), settings.miscellaneous().rebuy, miscellaneous,
                source.event_registry(), registry(source, 0x43475B78u), source.command_phases(),
                registry(source, 0x43475AD8u), variable(engine, 0x434670E8u),
                &pointed<float>(source.data(), 0x4346A334u),
                &source.data().native_cell<double>(0x4347A500u),
                &source.data().native_cell<double>(0x4347B118u), rebuy_offsets(source)}}} {
        require_shared(source, settings, descriptors, shared, imports);
        miscellaneous.quick_switch = &quick;
        // Auto-accept's descriptor lives inside its separate complete owner.
        // MiscellaneousMenu binds that registered value before its callback.
        miscellaneous.upgrade_tablet_enabled =
            &source.data().native_cell<std::uint8_t>(0x43478C6Cu);
        miscellaneous.clan_tag.set_tag = source.data().native_cell<misc::SetClanTag>(0x43467550u);
        miscellaneous.clan_tag.clan_id_cvar = variable(engine, 0x43467128u);
        miscellaneous.clan_tag.clock = &clan_clock;
        auto& ping = miscellaneous.ping_spike;
        ping.enabled = &settings.ping_spike().enabled.value;
        ping.hotkey = &settings.ping_spike().key.value;
        ping.keys = source.input_runtime().bitmap();
        ping.milliseconds = &settings.ping_spike().milliseconds.value;
        ping.maximum_unlag_milliseconds = &source.data().native_cell<std::int32_t>(0x43468230u);
        ping.latency_4347a508 = &source.data().native_cell<double>(0x4347A508u);
        ping.latency_4347b120 = &source.data().native_cell<double>(0x4347B120u);
        ping.interval_per_tick = &pointed<float>(source.data(), 0x4346A338u);
    }
};

void MiscRuntimeOwner::Data::KnifeSource::process(const misc::KnifeConfig& settings,
                                                  CommandFrame& frame, Command& command,
                                                  void* player, std::uint8_t& packet,
                                                  std::uint32_t& flags) {
    const auto* enabled = &data.configuration.rage_settings().global.enabled.value;
    if (!misc::knife_command_enabled(settings, frame, command, enabled))
        return;
    const auto invoke = [&](const rage::RuntimeFrameBindings& views) {
        auto& storage = data.storage;
        auto& image = storage.data();
        const misc::KnifeMeleeBindings melee{image.native_cell<void*>(0x43468304u),
                                             image.native_cell<void* const*>(0x434758B4u),
                                             views.targeting,
                                             offset(storage, 0x434670CCu),
                                             offset(storage, 0x43467908u),
                                             offset(storage, 0x43467A50u),
                                             &pointed<float>(image, 0x43467EB8u),
                                             data.imports.damage_math.trigonometry,
                                             data.imports.arc_tangent2};
        const misc::KnifeRuntimeBindings bindings{enabled,
                                                  data.interfaces.max_clients,
                                                  views.targeting,
                                                  offset(storage, 0x43467908u),
                                                  views.candidates,
                                                  views.records,
                                                  views.extrapolation,
                                                  melee,
                                                  data.aim,
                                                  data.shared.animation.bones(),
                                                  data.shared.live_movement,
                                                  data.shared.budget};
        misc::process_enabled_knife_command(settings, frame, command, player, packet, flags,
                                            bindings);
    };
    std::visit(
        [&](const auto& source) {
            using Source = std::remove_cvref_t<decltype(source)>;
            if constexpr (std::is_same_v<Source, rage::RuntimeOwner*>)
                source->with_frame(frame, player, command, invoke);
            else
                invoke({source.targeting, source.candidates, source.records, source.extrapolation});
        },
        data.shared.knife.source);
}

MiscRuntimeOwner::MiscRuntimeOwner(ClientRuntimeStorage& storage,
                                   const EngineRuntimeBindings& engine,
                                   ConfigurationRuntimeOwner& configuration,
                                   engine::HookDescriptorStorage& descriptors,
                                   MiscRuntimeSharedBindings shared)
    : MiscRuntimeOwner(storage, engine, configuration, descriptors, shared,
                       native_misc_runtime_imports(engine)) {}
ClientRuntimeStorage& MiscRuntimeOwner::require_available(ClientRuntimeStorage& storage) {
    if (storage.active_misc_owner_)
        throw std::logic_error("The source Misc feature state already has a process owner");
    return storage;
}
MiscRuntimeOwner::MiscRuntimeOwner(ClientRuntimeStorage& storage,
                                   const EngineRuntimeBindings& engine,
                                   ConfigurationRuntimeOwner& configuration,
                                   engine::HookDescriptorStorage& descriptors,
                                   MiscRuntimeSharedBindings shared, MiscRuntimeImports imports)
    : data_(std::make_unique<Data>(validated_storage(require_available(storage), engine,
                                                     configuration, descriptors, shared, imports),
                                   engine, configuration, descriptors, shared, imports)) {
    storage.active_misc_owner_ = this;
}
MiscRuntimeOwner::~MiscRuntimeOwner() {
    data_->storage.active_misc_owner_ = nullptr;
}
ClientRuntimeStorage& MiscRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
misc::Runtime& MiscRuntimeOwner::runtime() noexcept {
    return data_->miscellaneous;
}
ChannelHookState& MiscRuntimeOwner::channel() noexcept {
    return data_->channel;
}
misc::AutoAcceptPatchState& MiscRuntimeOwner::auto_accept_patch() noexcept {
    return data_->patch;
}
misc::QuickSwitchRuntime& MiscRuntimeOwner::quick_switch() noexcept {
    return data_->quick;
}
misc::LowFpsRuntime* MiscRuntimeOwner::low_fps() const noexcept {
    return data_->performance;
}
MiscRuntimeFactoryBindings& MiscRuntimeOwner::factory_bindings() noexcept {
    return data_->factories;
}
void MiscRuntimeOwner::bind_initializers(NativeStartupSources& sources) {
    bind_misc_runtime_factories(sources, data_->factories);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: movement_hud_factories.cpp
// ============================================================================

namespace skeet::client {
namespace {

void initialize_movement(void* owner) {
    auto& b = *static_cast<MovementHudFactoryBindings*>(owner);
    auto* const storage =
        engine::allocate_owner_storage(b.movement_registries.lifecycle.memory,
                                       (std::max)(std::size_t(0x578), movement_owner_size()));
    if (!storage) {
        b.movement.active = nullptr; //434856ED/EF
        return;
    }
    // The native process-heap allocation zeroed the embedded state before
    // construction. Project that backing into the same borrowed source state.
    b.movement.command.state = MovementCommandOwner{};
    construct_movement_owner(storage, b.movement_registries, b.configuration, b.movement);
}

void initialize_blockbot(void* owner) {
    auto& b = *static_cast<MovementHudFactoryBindings*>(owner);
    const auto& registries = b.movement_registries;
    auto* const storage = engine::allocate_owner_storage(
        registries.lifecycle.memory, (std::max)(std::size_t(0x68), blockbot_owner_size()));
    //4348DA67's protected INT1 continuation is not recovered. Surface the
    //allocation failure instead of inventing its native control flow.
    if (!storage)
        throw std::bad_alloc{};
    b.blockbot_settings = BlockbotConfig{};
    b.blockbot_state = BlockbotState{};
    (void)construct_blockbot_owner(storage, registries.lifecycle, registries.commands,
                                   registries.hud, b.configuration, b.blockbot_settings,
                                   b.blockbot_state, b.blockbot, b.blockbot_projection,
                                   b.blockbot_life_state_offset);
}

void initialize_overlay(void* owner) {
    auto& b = *static_cast<MovementHudFactoryBindings*>(owner);
    auto* const storage =
        hud::allocate_screen_overlay_owner_storage(b.overlay_registries.lifecycle.memory);
    //43490971 is an unrecovered INT1 allocation-failure continuation.
    if (!storage)
        throw std::bad_alloc{};
    hud::construct_allocated_screen_overlay_owner(storage, b.overlay_registries, b.configuration,
                                                  b.screen_settings, b.overlay_command,
                                                  b.overlay_rendering);
}

void initialize_reticle(void* owner) {
    auto& b = *static_cast<MovementHudFactoryBindings*>(owner);
    auto* const storage =
        hud::allocate_penetration_reticle_owner_storage(b.reticle_registries.lifecycle.memory);
    //43481DF7 is an unrecovered INT1 allocation-failure continuation.
    if (!storage)
        throw std::bad_alloc{};
    hud::construct_allocated_penetration_reticle_owner(
        storage, b.reticle_registries, b.configuration, b.screen_settings, b.reticle_command,
        b.reticle_environment, b.engine_trace, b.reticle_rendering);
}

/* [dedup same_memory] */

void require_bindings(NativeStartupSources& sources, const MovementHudFactoryBindings& b) {
    const auto& image = sources.data();
    const auto memory = b.movement_registries.lifecycle.memory;
    if (!memory.allocate || !memory.free || !memory.process_heap)
        throw std::invalid_argument("Movement/HUD factories require the canonical process heap");
    struct RegistryAlias {
        const engine::OwnerRegistryBinding* binding;
        std::uint32_t address;
    };
    const std::array aliases{RegistryAlias{&b.movement_registries.lifecycle, 0x43475B78u},
                             RegistryAlias{&b.movement_registries.hud, 0x43475A9Cu},
                             RegistryAlias{&b.overlay_registries.lifecycle, 0x43475B78u},
                             RegistryAlias{&b.overlay_registries.hud, 0x43475A9Cu},
                             RegistryAlias{&b.reticle_registries.lifecycle, 0x43475B78u},
                             RegistryAlias{&b.reticle_registries.hud, 0x43475A9Cu}};
    for (const auto& alias : aliases)
        if (&alias.binding->registry != &image.native_cell<engine::OwnerRegistry>(alias.address) ||
            !same_memory(alias.binding->memory, memory))
            throw std::invalid_argument(
                "Movement/HUD factories must share their native registry cells and heap");
    const auto& commands = b.movement_registries.commands;
    if (&b.overlay_registries.commands != &commands ||
        &b.reticle_registries.commands != &commands || &commands.image() != &image ||
        &commands.native_storage() != &image.native_cell<engine::OwnerRegistry>(0x434757A0u))
        throw std::invalid_argument(
            "Movement/HUD factories must share their canonical command registry");
    if (&b.configuration.descriptors != &image.native_cell<ConfigLink>(0x434798BCu) ||
        &b.configuration.hotkeys != &image.native_cell<HotkeyLink>(0x434798C4u) ||
        &b.movement.active != &image.native_cell<MovementCommandOwner*>(0x434759B0u))
        throw std::invalid_argument(
            "Movement/HUD factories require canonical configuration and movement publication cells");
    if (&b.overlay_command.settings != &b.screen_settings ||
        &b.overlay_rendering.config != &b.screen_settings ||
        &b.overlay_rendering.state != &b.overlay_command.state ||
        &b.reticle_command.settings != &b.screen_settings ||
        &b.reticle_rendering.config != &b.screen_settings ||
        &b.reticle_rendering.state != &b.reticle_command.state ||
        &b.reticle_command.bindings != &b.reticle_environment)
        throw std::invalid_argument(
            "Movement/HUD factories must share canonical command and render state");
    if (&b.engine_trace != &image.native_cell<void*>(0x43468304u) ||
        &b.overlay_command.bindings.history_owner !=
            &image.native_cell<EntityCommandOwner*>(0x43475898u))
        throw std::invalid_argument(
            "Movement/HUD factories require live engine-trace and command-history cells");
}
}

void bind_movement_hud_factories(NativeStartupSources& sources,
                                 MovementHudFactoryBindings& bindings) {
    require_bindings(sources, bindings);
    const std::array calls{NativeInitializerCall{0x434856D6u, &bindings, initialize_movement},
                           NativeInitializerCall{0x4348DA54u, &bindings, initialize_blockbot},
                           NativeInitializerCall{0x4349095Eu, &bindings, initialize_overlay},
                           NativeInitializerCall{0x43481DE4u, &bindings, initialize_reticle}};
    for (const auto& call : calls)
        sources.bind_initializer(call);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: movement_hud_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
std::uint32_t movement_hud_field(ClientRuntimeStorage& storage, std::uint32_t identity) {
    const auto& globals = storage.offsets().globals;
    for (const auto& row : engine::build_receive_offset_recipes())
        if (row.pointer_identity == identity)
            return std::uint32_t(std::ptrdiff_t(engine::DecodedOffset(
                globals, identity, row.xor_key_identity, row.initial_xor_key, 0)));
    for (const auto& row : engine::build_data_map_offset_recipes())
        if (row.pointer_identity == identity)
            return std::uint32_t(std::ptrdiff_t(engine::DecodedOffset(
                globals, identity, row.xor_key_identity, row.initial_xor_key, 0)));
    throw std::invalid_argument("Movement/HUD field has no proven netvar or datamap recipe");
}
std::uint32_t derived(ClientRuntimeStorage& storage, std::uint32_t pointer, std::uint32_t key,
                      std::uint32_t salt) {
    return std::uint32_t(
        std::ptrdiff_t(engine::DecodedOffset(storage.offsets().globals, pointer, key, salt, 0)));
}
/* [dedup pointed] */
engine::NativeConVarReference hud_variable(const EngineRuntimeBindings& engine,
                                       std::uint32_t identity) {
    const auto result = engine.variables.at(identity);
    if (!result.get())
        throw std::invalid_argument("Movement/HUD requires its acquired engine ConVar");
    return result;
}
/* deduplicated registry helper */
/* deduplicated bits helper */
MovementEnvironmentFields environment_fields(ClientRuntimeStorage& s) {
    return {movement_hud_field(s, 0x434685CCu), movement_hud_field(s, 0x4346A398u), movement_hud_field(s, 0x4346A7E0u),
            movement_hud_field(s, 0x43467A74u), movement_hud_field(s, 0x434688D0u), movement_hud_field(s, 0x434679F8u),
            movement_hud_field(s, 0x43467DD4u)};
}
MovementCaptureOffsets capture_fields(ClientRuntimeStorage& s) {
    return {movement_hud_field(s, 0x4346791Cu), movement_hud_field(s, 0x43467DD4u), movement_hud_field(s, 0x434679F8u),
            movement_hud_field(s, 0x43468A34u), movement_hud_field(s, 0x43468B30u), movement_hud_field(s, 0x4346A398u),
            movement_hud_field(s, 0x4346A464u), movement_hud_field(s, 0x43469374u), movement_hud_field(s, 0x43468BD4u)};
}
MovementCommandFields command_fields(ClientRuntimeStorage& s) {
    return {movement_hud_field(s, 0x4346A8E4u), movement_hud_field(s, 0x43468BD4u),
            movement_hud_field(s, 0x434678A4u), movement_hud_field(s, 0x43468B30u),
            movement_hud_field(s, 0x43468558u), movement_hud_field(s, 0x43467200u),
            movement_hud_field(s, 0x4346A80Cu), movement_hud_field(s, 0x43467A50u),
            movement_hud_field(s, 0x434684CCu), derived(s, 0x43467430u, 0x4346A718u, 0xB8969256u),
            movement_hud_field(s, 0x4346908Cu), movement_hud_field(s, 0x434693D8u),
            movement_hud_field(s, 0x434670CCu), movement_hud_field(s, 0x4346A6DCu)};
}
BlockbotOffsets blockbot_fields(ClientRuntimeStorage& s) {
    return {movement_hud_field(s, 0x43468BD4u), movement_hud_field(s, 0x434688D0u),
            movement_hud_field(s, 0x434688B0u), movement_hud_field(s, 0x43468A34u),
            movement_hud_field(s, 0x43468434u), movement_hud_field(s, 0x434670CCu),
            movement_hud_field(s, 0x434685CCu), movement_hud_field(s, 0x43468E78u),
            movement_hud_field(s, 0x4346A5DCu), derived(s, 0x43468BF8u, 0x43467B8Cu, 0xD070684Cu),
            movement_hud_field(s, 0x43467910u)};
}
hud::ScreenOverlayFields overlay_fields(ClientRuntimeStorage& s) {
    return {movement_hud_field(s, 0x4346908Cu), movement_hud_field(s, 0x43467B44u), movement_hud_field(s, 0x434682B8u),
            movement_hud_field(s, 0x43467F74u), movement_hud_field(s, 0x43467A50u), movement_hud_field(s, 0x43467200u),
            movement_hud_field(s, 0x4346A80Cu), movement_hud_field(s, 0x4346A620u)};
}
ClientRuntimeStorage& require_shared(ClientRuntimeStorage& s, const EngineRuntimeBindings& engine,
                                     ConfigurationRuntimeOwner& configuration,
                                     engine::SceneQueryRuntimeOwner& scene,
                                     const StandaloneMovementHudViews& views,
                                     const MovementHudRuntimeImports& imports) {
    auto& data = s.data();
    auto* globals = data.native_cell<std::byte*>(0x43467B04u);
    auto* client_state = data.native_cell<std::byte*>(0x43468E44u);
    if (&configuration.storage() != &s || &scene.storage() != &s ||
        &scene.players().storage() != &s || &engine.variables.data() != &data ||
        &views.scripts.name_storage != &s.event_names() ||
        &views.budget.game_rules != &data.native_cell<void**>(0x4346A3A4u) ||
        &views.budget.configured != &data.native_cell<std::int32_t>(0x43469274u))
        throw std::invalid_argument(
            "Movement/HUD requires the same configuration, scene, script and command owners");
    const auto core = engine.interfaces.core();
    if (!globals || !client_state || !core.engine_client || !core.entity_list || !core.cvar ||
        core.engine_client != data.native_cell<void*>(0x4346A514u) ||
        core.entity_list != data.native_cell<void*>(0x4346A890u) ||
        core.cvar != data.native_cell<void*>(0x4346A87Cu) ||
        !data.native_cell<void*>(0x4346A894u) || !data.native_cell<void*>(0x4346A89Cu) ||
        !data.native_cell<void**>(0x4346A3A4u) ||
        reinterpret_cast<std::byte*>(data.native_cell<float*>(0x4346A338u)) != globals + 0x20 ||
        reinterpret_cast<std::byte*>(data.native_cell<float*>(0x43467EB8u)) != globals + 0x10 ||
        reinterpret_cast<std::byte*>(data.native_cell<float*>(0x4346A334u)) != globals ||
        reinterpret_cast<std::byte*>(data.native_cell<std::int32_t*>(0x43469300u)) !=
            globals + 0x18 ||
        reinterpret_cast<std::byte*>(data.native_cell<std::int32_t*>(0x434757DCu)) !=
            client_state + 0x4D28 ||
        reinterpret_cast<std::byte*>(data.native_cell<std::int32_t*>(0x43475B2Cu)) !=
            client_state + 0x4D24)
        throw std::invalid_argument(
            "Movement/HUD requires canonical engine objects and live clock/client-state aliases");
    if (!imports.trigonometry.sine || !imports.trigonometry.cosine || !imports.rotation.sine ||
        !imports.rotation.cosine || !imports.rotation.arc_cosine ||
        !imports.rotation.arc_tangent2 || !imports.exponential || !imports.tangent)
        throw std::invalid_argument("Movement/HUD requires genuine scalar math boundaries");
    // Native target adjustment state belongs to the one PlayerRuntimeOwner.
    // TargetOffsets are the history owner's already proven fields, not names
    // guessed from a second independent engine-property lookup.
    if (std::uint32_t(views.target_offsets.team) != movement_hud_field(s, 0x434688B0u) ||
        std::uint32_t(views.target_offsets.origin) != movement_hud_field(s, 0x434670CCu) ||
        std::uint32_t(views.target_offsets.weapon_identifier) != movement_hud_field(s, 0x4346908Cu))
        throw std::invalid_argument(
            "Movement/HUD target fields differ from canonical offset storage");
    return s;
}
StandaloneMovementHudViews
canonical_views(ClientRuntimeStorage& storage, engine::SceneQueryRuntimeOwner& scene,
                VisualRuntimeOwner& visual, script::SourceScriptRuntimeOwner& scripts,
                CommandCaptureRuntimeOwner& capture, HistoryRuntimeOwner& history) {
    if (&visual.storage() != &storage || &scripts.storage() != &storage ||
        &capture.storage() != &storage || &history.storage() != &storage ||
        &history.players() != &scene.players())
        throw std::invalid_argument(
            "Movement/HUD production views must belong to the same process owners");
    return {scripts.bridge(),    capture.budget(),    history.target_offsets(),
            visual.projection(), visual.indicators(), visual.screen_size()};
}
}
MovementHudRuntimeImports native_movement_hud_runtime_imports() {
    const auto& trig = animation::native_sse_trigonometry();
    const auto numeric = lagcomp::native_history_numeric_bindings();
    return {trig,
            {trig.sine, trig.cosine, numeric.arccosine, animation::native_sse_atan2},
            animation::native_sse_exponential,
            animation::native_sse_tangent};
}

struct MovementHudRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    ConfigurationRuntimeOwner& configuration;
    const StandaloneMovementHudViews views;
    const MovementHudRuntimeImports imports;
    const std::uint32_t life;
    MovementCommandOwner state;
    MovementCaptureOffsets capture_offsets;
    MovementCaptureConvars capture_variables;
    MovementEnvironmentBindings environment;
    MovementCaptureInput capture;
    MovementExtrapolationLiveBindings live;
    MovementCommandBindings command_bindings;
    MovementCommandPhaseBindings phase_bindings;
    MovementCommandPhaseOwner command;
    MovementReleaseRuntime release;
    MovementOwnerHudBindings movement_hud;
    BlockbotConfig blockbot_settings;
    BlockbotState blockbot_state;
    BlockbotBindings blockbot;
    hud::ScreenOverlayState overlay_state;
    hud::ScreenOverlayPublisher overlay_publisher;
    hud::ScreenOverlayFields overlay_offsets;
    hud::ScreenOverlayUpdateBindings overlay_bindings;
    ScreenOverlayCommandOwner overlay_command;
    hud::ScreenOverlayHudBinding overlay_render;
    rage::TargetBindings targeting;
    hud::PenetrationReticleState reticle_state;
    hud::PenetrationReticleEnvironment reticle_environment;
    PenetrationReticleCommandOwner reticle_command;
    hud::PenetrationReticleBindings reticle_fields;
    hud::PenetrationHudBinding reticle_render;
    MovementHudFactoryBindings factories;

    Data(ClientRuntimeStorage& source, const EngineRuntimeBindings& engine,
         ConfigurationRuntimeOwner& config, engine::SceneQueryRuntimeOwner& scene,
         StandaloneMovementHudViews dependencies, MovementHudRuntimeImports external)
        : storage(require_shared(source, engine, config, scene, dependencies, external)),
          configuration(config), views(dependencies), imports(external),
          life(movement_hud_field(source, 0x43468E78u)), capture_offsets(capture_fields(source)),
          capture_variables{hud_variable(engine, 0x43467A54u), hud_variable(engine, 0x43467990u),
                            hud_variable(engine, 0x43467638u), hud_variable(engine, 0x43467E4Cu),
                            hud_variable(engine, 0x43468F4Cu), hud_variable(engine, 0x43468FB4u),
                            hud_variable(engine, 0x4346A88Cu), hud_variable(engine, 0x43467124u),
                            hud_variable(engine, 0x4346A820u), hud_variable(engine, 0x43467968u),
                            hud_variable(engine, 0x434674E4u)},
          environment{environment_fields(source),
                      {hud_variable(engine, 0x43467E28u), hud_variable(engine, 0x43467FA0u),
                       hud_variable(engine, 0x43468F4Cu), hud_variable(engine, 0x43467124u),
                       hud_variable(engine, 0x4346A820u), hud_variable(engine, 0x43467968u)},
                      source.data().native_cell<void*>(0x4346A890u),
                      &pointed<float>(source.data(), 0x4346A338u),
                      source.data().native_cell<std::uint8_t*>(0x434683D0u)},
          capture{capture_offsets, capture_variables, pointed<float>(source.data(), 0x4346A338u),
                  &environment},
          live{capture, environment},
          command_bindings{environment,
                           capture,
                           scene.queries().movement(),
                           scene.queries().ballistics(),
                           source.data().native_cell<void*>(0x4346A514u),
                           source.data().native_cell<void*>(0x43469268u),
                           source.input_runtime().bitmap(),
                           command_fields(source),
                           views.budget,
                           config.rage_settings().global.duck_peek_assist.value.binding,
                           config.movement().infinite_duck.value,
                           config.anti_aim_other().slow_motion_key.value.binding,
                           config.rage_settings().global.activation.value.binding,
                           BunnyHopGuard(engine.variables.at(0x4346A4E4u)),
                           hud_variable(engine, 0x43467D7Cu),
                           engine.variables.at(0x43468EA4u),
                           pointed<float>(source.data(), 0x43467EB8u),
                           pointed<float>(source.data(), 0x4346A334u),
                           pointed<std::int32_t>(source.data(), 0x43475B2Cu),
                           source.data().native_cell<std::int32_t>(0x4346DB1Cu),
                           source.data().native_cell<std::int32_t>(0x4346DB2Cu),
                           source.data().native_cell<std::int32_t>(0x43468488u),
                           bits(config.rage_settings().global.quick_stop_options.value),
                           config.anti_aim_global().enabled.value,
                           config.anti_aim_global().pitch.value,
                           config.anti_aim_global().body_yaw.value,
                           source.data().native_cell<EntityCommandOwner*>(0x43475898u),
                           source.data().native_cell<std::array<std::uint32_t, 150>>(0x43476090u),
                           source.data().native_cell<std::uint32_t>(0x434759C0u)},
          phase_bindings{command_bindings, source.data().native_cell<std::int32_t*>(0x434757DCu),
                         config.movement().edge_jump.value.binding, movement_hud_field(source, 0x43468B50u)},
          command{state, phase_bindings},
          release{
              state.config,
              {state.limits.angle_scale, state.side_magnitude, state.limits.maximum_angle,
               state.brake_threshold, state.angle_step},
              source.input_runtime().bitmap(),
              hud_variable(engine, 0x4346A4E4u),
              hud_variable(engine, 0x43468EA4u),
              state.jump_speed_cap,
              state.air_accelerate,
              config.movement().air_duck.value,
              pointed<float>(source.data(), 0x4346A334u),
              source.data().native_cell<void*>(0x4346A514u),
              {movement_hud_field(source, 0x434678A4u), movement_hud_field(source, 0x4346908Cu), movement_hud_field(source, 0x434670CCu)},
              environment,
              capture,
              scene.queries().movement(),
              scene.queries().ballistics(),
              source.data().native_cell<void*>(0x43469268u)},
          movement_hud{views.scripts,
                       config.other_esp_settings().feature_indicators,
                       config.movement().infinite_duck.value,
                       config.rage_settings().global,
                       source.data().native_cell<std::uint32_t>(0x434759C0u),
                       views.indicators,
                       views.projection,
                       life},
          blockbot{source.data().native_cell<void*>(0x4346A514u),
                   source.data().native_cell<void*>(0x4346A890u),
                   source.data().native_cell<void**>(0x4346A3A4u),
                   &pointed<std::int32_t>(source.data(), 0x43469300u),
                   &source.data().native_cell<lagcomp::TickTimingSamples>(0x43478C70u),
                   &pointed<float>(source.data(), 0x4346A338u),
                   &scene.queries().movement(),
                   source.data().native_cell<std::uint8_t*>(0x434683D0u),
                   blockbot_fields(source)},
          overlay_publisher(source.data().native_cell<bool>(0x43475894u),
                            source.data().native_cell<void*>(0x43475A24u)),
          overlay_offsets(overlay_fields(source)),
          overlay_bindings{source.data().native_cell<void*>(0x4346A514u),
                           source.data().native_cell<void*>(0x4346A894u),
                           source.data().native_cell<void*>(0x4346A87Cu),
                           &pointed<float>(source.data(), 0x43467EB8u),
                           &pointed<float>(source.data(), 0x4346A338u),
                           &pointed<float>(source.data(), 0x4346A338u),
                           &source.data().native_cell<float>(0x43468E80u),
                           hud_variable(engine, 0x4346794Cu),
                           hud_variable(engine, 0x43467104u),
                           hud_variable(engine, 0x43468D8Cu),
                           hud_variable(engine, 0x43467BB4u),
                           hud_variable(engine, 0x4346A480u),
                           source.data().native_cell<EntityCommandOwner*>(0x43475898u),
                           imports.rotation,
                           imports.exponential,
                           imports.tangent},
          overlay_command{overlay_state, overlay_publisher, config.other_esp_settings().overlays,
                          overlay_offsets, overlay_bindings},
          overlay_render{config.other_esp_settings().overlays,
                         overlay_state,
                         life,
                         views.screen_size,
                         views.projection,
                         source.data().native_cell<RageVector>(0x43479CCCu),
                         imports.trigonometry},
          targeting{source.data().native_cell<void*>(0x43475808u),
                    source.data().native_cell<void*>(0x4346A890u),
                    hud_variable(engine, 0x43467A8Cu),
                    hud_variable(engine, 0x4346A568u),
                    scene.players().adjustments(),
                    views.target_offsets},
          reticle_environment{targeting,
                              &pointed<std::int32_t>(source.data(), 0x43469300u),
                              scene.queries().ballistics(),
                              movement_hud_field(source, 0x4346A808u),
                              source.data().native_cell<const std::int32_t*>(0x43469060u),
                              movement_hud_field(source, 0x434683C8u),
                              std::ptrdiff_t(movement_hud_field(source, 0x43469344u)) - 0x18,
                              source.data().native_cell<void*>(0x43468304u),
                              source.data().native_cell<void* const*>(0x43475B90u)},
          reticle_command{reticle_state, config.other_esp_settings().overlays, reticle_environment},
          reticle_fields{source.data().native_cell<void*>(0x4346A890u), life,
                         movement_hud_field(source, 0x43467A74u), movement_hud_field(source, 0x4346908Cu)},
          reticle_render{config.other_esp_settings().overlays, reticle_state, reticle_fields,
                         views.screen_size},
          factories{source.configuration(),
                    {registry(source, 0x43475B78u), source.command_phases(),
                     registry(source, 0x43475A9Cu)},
                    {command, movement_hud, source.data().native_cell<void*>(0x4346A514u),
                     source.data().native_cell<void*>(0x4346A89Cu),
                     engine.variables.at(0x434674E4u), engine.variables.at(0x43467FA0u),
                     source.data().native_cell<MovementCommandOwner*>(0x434759B0u)},
                    blockbot_settings,
                    blockbot_state,
                    blockbot,
                    views.projection,
                    life,
                    {registry(source, 0x43475B78u), source.command_phases(),
                     registry(source, 0x43475A9Cu)},
                    overlay_command,
                    overlay_render,
                    {registry(source, 0x43475B78u), source.command_phases(),
                     registry(source, 0x43475A9Cu)},
                    reticle_command,
                    reticle_environment,
                    source.data().native_cell<void*>(0x43468304u),
                    reticle_render,
                    config.other_esp_settings().overlays} {}
};
MovementHudRuntimeOwner::MovementHudRuntimeOwner(
    ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
    ConfigurationRuntimeOwner& configuration, engine::SceneQueryRuntimeOwner& scene,
    VisualRuntimeOwner& visual, script::SourceScriptRuntimeOwner& scripts,
    CommandCaptureRuntimeOwner& capture, HistoryRuntimeOwner& history)
    : MovementHudRuntimeOwner(storage, engine, configuration, scene,
                              canonical_views(storage, scene, visual, scripts, capture, history),
                              native_movement_hud_runtime_imports()) {}
MovementHudRuntimeOwner::MovementHudRuntimeOwner(ClientRuntimeStorage& storage,
                                                 const EngineRuntimeBindings& engine,
                                                 ConfigurationRuntimeOwner& configuration,
                                                 engine::SceneQueryRuntimeOwner& scene,
                                                 StandaloneMovementHudViews views,
                                                 MovementHudRuntimeImports imports)
    : data_(std::make_unique<Data>(storage, engine, configuration, scene, views, imports)) {}
MovementHudRuntimeOwner::~MovementHudRuntimeOwner() = default;
ClientRuntimeStorage& MovementHudRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
const MovementEnvironmentBindings& MovementHudRuntimeOwner::movement_environment() const noexcept {
    return data_->environment;
}
const MovementCaptureInput& MovementHudRuntimeOwner::movement_capture() const noexcept {
    return data_->capture;
}
const MovementExtrapolationLiveBindings& MovementHudRuntimeOwner::live_movement() const noexcept {
    return data_->live;
}
MovementCommandPhaseOwner& MovementHudRuntimeOwner::movement_command() noexcept {
    return data_->command;
}
MovementReleaseRuntime& MovementHudRuntimeOwner::release_movement() noexcept {
    return data_->release;
}
MovementHudFactoryBindings& MovementHudRuntimeOwner::factory_bindings() noexcept {
    return data_->factories;
}
void MovementHudRuntimeOwner::bind_initializers(NativeStartupSources& sources) {
    if (&sources.data() != &storage().data())
        throw std::invalid_argument(
            "Movement/HUD factories require the canonical source scheduler");
    bind_movement_hud_factories(sources, data_->factories);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: native_startup.cpp
// ============================================================================
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif

namespace skeet::client {
namespace {
StartupExitProcess active_exit_process = nullptr;
std::string unmapped_message(std::uint32_t operand, std::uint32_t address, bool preflight) {
    std::ostringstream message;
    message << "Unfinished native startup " << (preflight ? "preflight" : "initializer")
            << " at operand0x" << std::hex << operand << ":0x" << address;
    return message.str();
}
void initialize_masks(void* graph) {
    auto& image = static_cast<NativeStartupSourceGraph*>(graph)->data;
    auto& matrix = image.native_cell<std::array<std::uint32_t, 32 * 33>>(0x43477be0);
    auto& prefixes = image.native_cell<std::array<std::uint32_t, 32>>(0x43477b60);
    initialize_native_startup_bit_masks({matrix, prefixes});
}
void initialize_menu_key(void* graph) {
    auto& source = *static_cast<NativeStartupSourceGraph*>(graph);
    input::initialize_menu_hotkey(source.configuration, source.menu_key);
}
void initialize_infinite_duck(void* graph) {
    auto& source = *static_cast<NativeStartupSourceGraph*>(graph);
    initialize_infinite_duck_config(source.configuration, source.movement.infinite_duck);
}
void initialize_air_duck(void* graph) {
    auto& source = *static_cast<NativeStartupSourceGraph*>(graph);
    initialize_air_duck_config(source.configuration, source.movement.air_duck);
}
void initialize_edge_jump(void* graph) {
    auto& source = *static_cast<NativeStartupSourceGraph*>(graph);
    initialize_edge_jump_hotkey(source.configuration, source.movement.edge_jump);
}
void initialize_skin_filter(void* graph) {
    auto& source = *static_cast<NativeStartupSourceGraph*>(graph);
    source.skins.initialize_shared(source.configuration);
}
void initialize_ping_hud(void*) {
    hud::initialize_ping_hud_owner();
}
void initialize_crosshair_hud(void*) {
    hud::initialize_crosshair_hud_owner();
}
void initialize_flags_container(void* raw) { //4332BB3A
    auto& graph = *static_cast<NativeStartupSourceGraph*>(raw);
    const auto& image = graph.data;
    image.native_cell<std::uint32_t>(0x4346d72cu) = graph.static_crt.flags_constructor_word;
    image.native_cell<void*>(0x4346d730u) = &image.native_cell<std::byte>(0x4346d734u);
    (void)graph.exits.callbacks.append(graph.native_vector_cleanup.source_finalizer(0x433c956cu));
}
void initialize_keys_container(void* raw) { //433412F8
    auto& graph = *static_cast<NativeStartupSourceGraph*>(raw);
    const auto& image = graph.data;
    image.native_cell<std::uint32_t>(0x4346daacu) = graph.static_crt.keys_constructor_word;
    image.native_cell<void*>(0x4346dab0u) = &image.native_cell<std::byte>(0x4346dab4u);
    (void)graph.exits.callbacks.append(graph.native_vector_cleanup.source_finalizer(0x433fbe15u));
}
void first_startup_services(void* context) {
    // Devirtualized from434822C8 ->434EA5D4; only source code is called.
    engine::initialize_first_startup_services(
        *static_cast<engine::FirstStartupServiceBindings*>(context));
}
void last_vm_callback(void* vm) {
    static_cast<engine::PreservedInitialization*>(vm)->last_startup_callback();
}
NativeStartupImports require_imports(NativeStartupImports imports) {
    if (!imports.clear_unhandled_exception_filter || !imports.terminate_thread ||
        !imports.exit_process)
        throw std::invalid_argument("Startup runtime requires its actual Windows imports");
    return imports;
}
void begin_startup(NativeStartupState state, NativeStartupImports imports,
                   std::uintptr_t argument) {
    active_exit_process = imports.exit_process;
    state.initialization_argument = argument;
    imports.clear_unhandled_exception_filter();
    state.fatal_handler = native_startup_fatal;
}
#if defined(_WIN32)
void clear_exception_filter() {
    (void)SetUnhandledExceptionFilter(nullptr);
}
#endif
}

void initialize_native_startup_bit_masks(NativeStartupBitMasks masks) noexcept { //433FB71E
    for (std::uint32_t row = 0; row < 32; ++row) {
        const auto prefix = (std::uint32_t(1) << row) - 1;
        for (std::uint32_t column = 0; column < 33; ++column) {
            const auto shift = row + column;
            masks.matrix[row * 33 + column] = shift < 32 ? (0xffffffffu << shift) | prefix : prefix;
        }
    }
    for (std::uint32_t row = 0; row < 32; ++row)
        masks.prefixes[row] = (std::uint32_t(1) << row) - 1;
}

UnmappedNativeInitializer::UnmappedNativeInitializer(std::uint32_t operand, std::uint32_t address,
                                                     bool is_preflight)
    : std::runtime_error(unmapped_message(operand, address, is_preflight)), operand(operand),
      native_address(address), preflight(is_preflight) {}

NativeStartupSources::NativeStartupSources(engine::NativeDataImage& data,
                                           NativeInitializerCall final_initializer,
                                           std::span<const NativeInitializerCall> initializers,
                                           std::span<const NativePreflightCall> preflight)
    : data_(data), final_initializer_(final_initializer) {
    if (final_initializer.native_address != 0x43484F06u || !final_initializer.owner ||
        !final_initializer.invoke)
        throw std::invalid_argument(
            "Startup requires the exact nonempty final initializer43484F06 binding");
    bind_initializer(final_initializer);
    for (const auto& source : initializers)
        bind_initializer(source);
    for (const auto& source : preflight)
        bind_preflight(source);
}
NativeStartupSources::NativeStartupSources(NativeStartupSourceGraph& graph,
                                           std::span<const NativeInitializerCall> initializers,
                                           std::span<const NativePreflightCall> preflight)
    : NativeStartupSources(graph.data, graph.final_initializer) {
    if (&graph.first_startup_services.data != &data_)
        throw std::invalid_argument("First startup must share the canonical service-number cells");
    if (&graph.configuration.descriptors != &data_.native_cell<ConfigLink>(0x434798bcu) ||
        &graph.configuration.hotkeys != &data_.native_cell<HotkeyLink>(0x434798c4u))
        throw std::invalid_argument(
            "Native startup requires the canonical configuration sentinels");
    if (&graph.menu_key != &data_.native_cell<RegisteredHotkey>(0x43478DB8u) ||
        &graph.global_configuration.settings_menu.menu_key != &graph.menu_key ||
        &graph.global_configuration.settings_menu.multithreading_config !=
            &data_.native_cell<BooleanConfig>(0x434683ECu) ||
        &graph.global_configuration.settings_menu.dpi_scale !=
            &data_.native_cell<std::int32_t>(0x43475A94u))
        throw std::invalid_argument(
            "CRT, menu and runtime settings must share their canonical source cells");
    if (graph.static_crt.memory.free != graph.runtime_owners.lifecycle.memory.free ||
        graph.static_crt.memory.process_heap !=
            graph.runtime_owners.lifecycle.memory.process_heap ||
        graph.event_names.memory.allocate != graph.runtime_owners.lifecycle.memory.allocate ||
        graph.event_names.memory.free != graph.runtime_owners.lifecycle.memory.free ||
        graph.event_names.memory.process_heap !=
            graph.runtime_owners.lifecycle.memory.process_heap ||
        graph.native_vector_cleanup.memory.free != graph.runtime_owners.lifecycle.memory.free ||
        graph.native_vector_cleanup.memory.process_heap !=
            graph.runtime_owners.lifecycle.memory.process_heap ||
        graph.native_container_cleanup.bindings().memory.free !=
            graph.runtime_owners.lifecycle.memory.free ||
        graph.native_container_cleanup.bindings().memory.process_heap !=
            graph.runtime_owners.lifecycle.memory.process_heap)
        throw std::invalid_argument(
            "CRT storage and runtime owners must share the canonical process heap");
    if (&graph.anti_aim.configuration != &graph.configuration ||
        &graph.static_configuration.configuration != &graph.configuration ||
        &graph.rage_groups.configuration != &graph.configuration ||
        &graph.prediction_history.configuration != &graph.configuration ||
        &graph.shared_esp.configuration != &graph.configuration ||
        &graph.shot_feedback.configuration != &graph.configuration ||
        &graph.global_configuration.configuration != &graph.configuration ||
        &graph.global_configuration.rage != &graph.static_configuration.rage ||
        &graph.global_configuration.other_esp != &graph.static_configuration.other_esp ||
        &graph.global_configuration.effects != &graph.static_configuration.effects ||
        &graph.global_configuration.settings_menu.menu_key != &graph.menu_key ||
        &graph.simple_hud.feature_indicators !=
            &graph.global_configuration.other_esp.feature_indicators ||
        &graph.simple_hud.overlays != &graph.global_configuration.other_esp.overlays ||
        &graph.global_configuration.free_look.key.value.binding !=
            &graph.free_look.bindings.view.free_look_key ||
        &graph.static_configuration.target_mask != &graph.target_mask.settings.enabled)
        throw std::invalid_argument("Native startup must share the configuration registries");
    if (&graph.hook_descriptors.callbacks != &graph.exits.callbacks ||
        &graph.script_extensions.exit_callbacks != &graph.exits.callbacks ||
        &graph.animation_backups.callbacks != &graph.exits.callbacks ||
        &graph.rage_groups.callbacks != &graph.exits.callbacks ||
        &graph.esp_flags.callbacks != &graph.exits.callbacks ||
        &graph.esp_flags.flags.image() != &data_ ||
        &graph.ui_types.callbacks != &graph.exits.callbacks || &graph.ui_types.image() != &data_ ||
        graph.static_crt.data.get() != &data_ ||
        &graph.static_crt.callbacks != &graph.exits.callbacks ||
        &graph.event_names.callbacks != &graph.exits.callbacks ||
        &graph.event_names.registry != &graph.runtime_owners.sequences.events ||
        &graph.event_names.registry.name_storage.image() != &data_ ||
        graph.native_vector_cleanup.data.get() != &data_ ||
        &graph.native_vector_cleanup.callbacks != &graph.exits.callbacks ||
        &graph.native_container_cleanup.exit_callbacks() != &graph.exits.callbacks ||
        &graph.rage_groups.groups.image() != &data_ || graph.entity_commands.data.get() != &data_ ||
        graph.target_mask.data.get() != &data_ ||
        &graph.entity_commands.registries.data_updates.image() != &data_ ||
        &graph.entity_commands.registries.recharge.image() != &data_ ||
        &graph.entity_commands.registries.prediction.image() != &data_ ||
        &graph.prediction_history.registries.data_updates.image() != &data_ ||
        &graph.prediction_history.registries.recharge.image() != &data_ ||
        &graph.prediction_history.registries.prediction.image() != &data_ ||
        &graph.prediction_history.registries.empty_ticks.image() != &data_ ||
        &graph.shared_esp.registries.empty_ticks.image() != &data_ ||
        &graph.shot_feedback.recharge.image() != &data_ ||
        &graph.exits.cleanup.owner_registry_storage().image() != &data_ ||
        &graph.hook_descriptors.storage.image() != &data_)
        throw std::invalid_argument(
            "Native startup must share its canonical data and exit registry");
    if (&graph.free_look.bindings.active !=
            &data_.native_cell<effects::FreeLookOwner*>(0x43475a64u) ||
        &graph.shared_esp.bindings.active !=
            &data_.native_cell<RejectedPlayerUpdateOwner*>(0x434758d0u) ||
        &graph.shot_feedback.active != &data_.native_cell<rage::FeedbackOwner*>(0x43475a7cu) ||
        &graph.simple_hud.entity_list != &data_.native_cell<void*>(0x4346a890u) ||
        &graph.simple_hud.cvar != &data_.native_cell<void*>(0x4346a87cu) ||
        &graph.simple_hud.suspended != &data_.native_cell<std::uint8_t>(0x43468e58u) ||
        &graph.simple_hud.ping_channel.bindings.history !=
            &data_.native_cell<misc::PingSpikeNetworkHistory>(0x4347a500u))
        throw std::invalid_argument("Startup factories require their canonical native owner slots");
    const auto& owner_storage = graph.exits.cleanup.owner_registry_storage();
    struct RegistryAlias {
        const engine::OwnerRegistry* registry;
        std::uint32_t address;
    };
    const std::array registry_aliases{
        RegistryAlias{&graph.prediction_history.registries.primary.registry, 0x43475b78u},
        RegistryAlias{&graph.entity_commands.registries.lifecycle.registry, 0x43475b78u},
        RegistryAlias{&graph.entity_commands.registries.map_entities.registry, 0x43475770u},
        RegistryAlias{&graph.free_look.lifecycle.registry, 0x43475b78u},
        RegistryAlias{&graph.free_look.updates.registry, 0x43475988u},
        RegistryAlias{&graph.target_mask.registries.primary.registry, 0x43475b78u},
        RegistryAlias{&graph.target_mask.registries.messages.registry, 0x43475b94u},
        RegistryAlias{&graph.target_mask.registries.hud.registry, 0x43475a9cu},
        RegistryAlias{&graph.shared_esp.registries.primary.registry, 0x43475b78u},
        RegistryAlias{&graph.shared_esp.registries.messages.registry, 0x43475b94u},
        RegistryAlias{&graph.shot_feedback.lifecycle.registry, 0x43475b78u},
        RegistryAlias{&graph.shot_feedback.packets.registry, 0x4347584cu},
        RegistryAlias{&graph.shot_feedback.frames.registry, 0x43475ad8u},
        RegistryAlias{&graph.shot_feedback.positions.registry, 0x43475930u},
        RegistryAlias{&graph.simple_hud.lifecycle.registry, 0x43475b78u},
        RegistryAlias{&graph.simple_hud.hud.registry, 0x43475a9cu}};
    for (const auto& alias : registry_aliases)
        if (alias.registry != &owner_storage.at(alias.address))
            throw std::invalid_argument(
                "Startup factories must share the native owner registry cells");
    const auto* const commands = &graph.entity_commands.registries.commands;
    if (&graph.prediction_history.registries.commands != commands ||
        &graph.target_mask.registries.commands != commands ||
        &graph.shared_esp.registries.commands != commands ||
        &graph.shot_feedback.commands != commands ||
        &graph.buffered_shots.lifecycle.commands != commands ||
        &graph.viewmodel_lag.registries.commands != commands)
        throw std::invalid_argument("Startup factories must share the command registry");
    hud::bind_simple_hud_owners_context(graph.simple_hud);
    if (&graph.runtime_owners.configuration != &graph.configuration ||
        &graph.runtime_owners.sequences.settings != &graph.skins ||
        &graph.runtime_owners.rage.command_owners != commands ||
        &graph.runtime_owners.entity_updates != &graph.entity_commands.registries.entities ||
        &graph.runtime_owners.legit.empty_tick_owners !=
            &graph.prediction_history.registries.empty_ticks ||
        &graph.runtime_owners.rage.commands.prediction_masks() != &graph.prediction_masks.active)
        throw std::invalid_argument(
            "Runtime factories must share startup configuration, registries and prediction masks");
    if (&graph.prediction_masks.active !=
            &data_.native_cell<rage::PredictionMaskOwner*>(0x43475a58u) ||
        &graph.prediction_masks.lifecycle.registry != &owner_storage.at(0x43475b78u) ||
        &graph.prediction_masks.frames.registry != &owner_storage.at(0x43475ad8u))
        throw std::invalid_argument(
            "Prediction masks require their canonical owner and frame registries");
    if (&graph.misc_owners.configuration != &graph.configuration ||
        &graph.combat_owners.configuration != &graph.configuration ||
        &graph.skin_owners.configuration != &graph.configuration ||
        &graph.movement_hud.configuration != &graph.configuration ||
        &graph.world_owners.configuration != &graph.configuration ||
        &graph.skin_owners.settings != &graph.skins || &graph.misc_owners.commands != commands ||
        &graph.skin_owners.commands != commands ||
        &graph.combat_owners.packet.commands != commands ||
        &graph.movement_hud.movement_registries.commands != commands ||
        &graph.misc_owners.entities != &graph.runtime_owners.entity_updates ||
        &graph.skin_owners.entity_updates != &graph.runtime_owners.entity_updates ||
        &graph.history_owners.entities != &graph.runtime_owners.entity_updates ||
        &graph.combat_owners.scripting.modules.ui_types != &graph.ui_types.storage ||
        &graph.world_owners.command_owners != commands)
        throw std::invalid_argument(
            "Runtime factory groups must share startup configuration and receivers");
    bind_initializer({0x434822c8, &graph.first_startup_services, first_startup_services});
    bind_initializer({0x433fb71e, &graph, initialize_masks});
    bind_initializer({0x434928a2, &graph, initialize_menu_key});
    bind_initializer({0x434933a6, &graph, initialize_infinite_duck});
    bind_initializer({0x43490b39, &graph, initialize_air_duck});
    bind_initializer({0x4349340b, &graph, initialize_edge_jump});
    bind_initializer({0x43492e21, &graph, initialize_skin_filter});
    anti_aim::bind_native_startup_initializers(*this, graph.anti_aim);
    bind_exit_initializers(*this, graph.exits);
    bind_native_config_initializers(*this, graph.static_configuration);
    bind_global_config_initializers(*this, graph.global_configuration);
    bind_hook_descriptor_initializers(*this, graph.hook_descriptors);
    script::bind_native_startup_initializers(*this, graph.script_extensions);
    animation::bind_fix_animation_backup_exit_initializer(*this, graph.animation_backups);
    rage::bind_native_group_initializers(*this, graph.rage_groups);
    bind_prediction_history_initializer(*this, graph.prediction_history);
    bind_native_entity_command_factory(*this, graph.entity_commands);
    effects::bind_free_look_initializer(*this, graph.free_look);
    bind_native_target_mask_factory(*this, graph.target_mask);
    bind_shared_esp_initializer(*this, graph.shared_esp);
    rage::bind_feedback_owner_factory(*this, graph.shot_feedback);
    bind_buffered_shot_factory(*this, graph.buffered_shots);
    misc::bind_viewmodel_lag_factory(*this, graph.viewmodel_lag);
    bind_initializer({0x4347dd87u, &graph.simple_hud, initialize_ping_hud});
    bind_initializer({0x43486bf7u, &graph.simple_hud, initialize_crosshair_hud});
    esp::bind_flag_table_initializer(*this, graph.esp_flags);
    script::bind_ui_type_initializer(*this, graph.ui_types);
    bind_static_crt_initializers(*this, graph.static_crt);
    events::bind_event_name_initializer(*this, graph.event_names);
    engine::bind_startup_vector_cleanup(*this, graph.native_vector_cleanup);
    engine::bind_startup_container_cleanup(*this, graph.native_container_cleanup);
    bind_initializer({0x4332bb3au, &graph, initialize_flags_container});
    bind_initializer({0x433412f8u, &graph, initialize_keys_container});
    bind_runtime_owner_factories(*this, graph.runtime_owners);
    rage::bind_prediction_mask_factory(*this, graph.prediction_masks);
    engine::bind_map_game_system_factory(*this, graph.map_game_system);
    bind_misc_runtime_factories(*this, graph.misc_owners);
    bind_combat_runtime_factories(*this, graph.combat_owners);
    bind_skin_runtime_factories(*this, graph.skin_owners);
    bind_movement_hud_factories(*this, graph.movement_hud);
    bind_history_runtime_factories(*this, graph.history_owners);
    bind_world_runtime_factories(*this, graph.world_owners);
    bind_game_window_proxy_factory(*this, graph.game_window_proxy);
    for (const auto& source : initializers)
        bind_initializer(source);
    for (const auto& source : preflight)
        bind_preflight(source);
    if (&graph.source_terminal_cleanup.callbacks != &graph.exits.callbacks ||
        &graph.source_terminal_cleanup.bridge != &graph.shot_feedback.state.scripts ||
        &graph.shot_feedback.state != &graph.history_owners.history.feedback ||
        &graph.shot_feedback.runtime.players != &graph.source_terminal_cleanup.players)
        throw std::invalid_argument(
            "Source terminal cleanup requires the graph's canonical process owners");
    graph.source_terminal_cleanup.require_producers(graph.combat_owners.scripting.runtime,
                                                    graph.combat_owners.scripting.modules,
                                                    graph.history_owners.history.recording);
    // Select this last so no native-header registration can replace actual
    // source Bridge/UI/history cleanup, including a caller's extra bindings.
    bind_source_terminal_cleanup(*this, graph.source_terminal_cleanup);
    // Validate both known source phases before any initializer body runs.
    // These fixed build configurations do not implement the protected accessor.
    preflight_crt_initialization(*this);
    preflight_runtime_owner_initialization(*this);
}
void NativeStartupSources::bind_initializer(NativeInitializerCall source) {
    // The caller supplies the actual final implementation once. Additional
    // binders cannot silently replace that required operation or its owner.
    if (source.native_address == 0x43484f06 &&
        (source.owner != final_initializer_.owner || source.invoke != final_initializer_.invoke))
        throw std::invalid_argument("The final startup initializer binding cannot be replaced");
    initializers_.insert_or_assign(source.native_address, source);
}
void NativeStartupSources::bind_preflight(NativePreflightCall source) {
    preflight_.insert_or_assign(source.native_address, source);
}
void NativeStartupSources::require_initializer(std::uint32_t operand, std::uint32_t address) const {
    const auto source = initializers_.find(address);
    if (source == initializers_.end() || !source->second.invoke)
        throw UnmappedNativeInitializer(operand, address, false);
}
void NativeStartupSources::require_preflight(std::uint32_t operand, std::uint32_t address) const {
    const auto source = preflight_.find(address);
    if (source == preflight_.end() || !source->second.invoke)
        throw UnmappedNativeInitializer(operand, address, true);
}
void NativeStartupSources::initialize(std::uint32_t operand, std::uint32_t address) const {
    const auto source = initializers_.find(address);
    if (source == initializers_.end() || !source->second.invoke)
        throw UnmappedNativeInitializer(operand, address, false);
    source->second.invoke(source->second.owner);
}
std::uint32_t NativeStartupSources::check(std::uint32_t operand, std::uint32_t address) const {
    const auto source = preflight_.find(address);
    if (source == preflight_.end() || !source->second.invoke)
        throw UnmappedNativeInitializer(operand, address, true);
    return source->second.invoke(source->second.owner);
}

NativeStartupImports native_startup_runtime_imports() {
#if defined(_WIN32)
    const auto native = GetModuleHandleA("ntdll.dll");
    if (!native)
        throw std::runtime_error("ntdll.dll is not loaded");
    const auto terminate = GetProcAddress(native, "ZwTerminateThread");
    const auto exit = GetProcAddress(native, "RtlExitUserProcess");
    if (!terminate || !exit)
        throw std::runtime_error("Native startup service is not available");
    return {clear_exception_filter, reinterpret_cast<StartupTerminateThread>(terminate),
            reinterpret_cast<StartupExitProcess>(exit)};
#else
    throw std::runtime_error("Native startup services require Windows");
#endif
}

NativeStartupRuntime::NativeStartupRuntime(NativeStartupSources& sources,
                                           NativeStartupImports imports)
    : state_{sources.data().native_cell<std::uintptr_t>(0x43475AE8u),
             sources.data().native_cell<void (*)()>(0x43475BC4u)},
      sources_(sources), imports_(require_imports(imports)) {}

std::uint32_t NativeStartupRuntime::initialize(std::uintptr_t third_argument) {
    preflight_crt_initialization(sources_);
    begin_startup(state_, imports_, third_argument);
    const auto precheck = crt_initialization_precheck();
    if (precheck.identifier && sources_.check(precheck.operand, precheck.identifier))
        (void)imports_.terminate_thread(reinterpret_cast<void*>(~std::uintptr_t(1)), 5);
    for (const auto& slot : crt_initialization_plan())
        if (slot.identifier)
            sources_.initialize(slot.operand, slot.identifier);
    return 0;
}
void NativeStartupRuntime::fatal() {
    imports_.exit_process(0xFFFFFFF6u);
}

RetainedNativeStartupRuntime::RetainedNativeStartupRuntime(
    NativeStartupSources& sources, engine::PreservedInitialization& metadata,
    NativeStartupImports imports)
    : state_{sources.data().native_cell<std::uintptr_t>(0x43475AE8u),
             sources.data().native_cell<void (*)()>(0x43475BC4u)},
      sources_(sources), metadata_(metadata), imports_(require_imports(imports)) {
    if (&sources.data() != &metadata.image())
        throw std::invalid_argument(
            "Retained startup requires its executable environment's canonical data owner");
}
std::uint32_t RetainedNativeStartupRuntime::initialize(std::uintptr_t third_argument) {
    preflight_crt_initialization(sources_);
    begin_startup(state_, imports_, third_argument);
    for (std::uint32_t operand = 0x2dc; operand != 0x2e0; operand += 4) {
        const auto native_address = static_cast<std::uint32_t>(metadata_.operand(operand));
        if (native_address && sources_.check(operand, native_address)) {
            (void)imports_.terminate_thread(reinterpret_cast<void*>(~std::uintptr_t(1)), 5);
            break;
        }
    }
    for (std::uint32_t operand = 0; operand != 0x2d8; operand += 4) {
        const auto native_address = static_cast<std::uint32_t>(metadata_.operand(operand));
        if (native_address)
            sources_.initialize(operand, native_address);
    }
    // Native writesEAX0. Its trailing43481E98 INT1 return-transfer ABI remains
    // outside this source entry; no raw ABI thunk is invented here.
    return 0;
}
void RetainedNativeStartupRuntime::fatal() {
    imports_.exit_process(0xFFFFFFF6u);
}
NativeInitializerCall
retained_final_startup_initializer(engine::PreservedInitialization& metadata) {
    return {0x43484F06u, &metadata, last_vm_callback};
}
void native_startup_fatal() {
    if (!active_exit_process)
        throw std::logic_error("Startup fatal import is not bound");
    active_exit_process(0xFFFFFFF6u);
}
}

// ============================================================================
// Subcomponent: packet_runtime_owner.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::client {
namespace {
/* [dedup offset_storage] */
ClientRuntimeStorage& checked(ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
                              ConfigurationRuntimeOwner& config, HistoryRuntimeOwner& history,
                              MovementHudRuntimeOwner& movement, TargetRefreshRuntimeOwner& target,
                              PacketRuntimeImports imports) {
    if (&engine.variables.data() != &storage.data() || &config.storage() != &storage ||
        &history.storage() != &storage || &movement.storage() != &storage ||
        &target.storage() != &storage || !imports.seed || !imports.integer)
        throw std::invalid_argument(
            "Packet requires its canonical engine, configuration, history, movement, target and random boundaries");
    auto& data = storage.data();
    for (const auto identity : {0x434757DCu, 0x43475B2Cu, 0x4346A338u, 0x4346A334u, 0x43467B04u,
                                0x4346A514u, 0x4346A890u, 0x43469300u, 0x434678F8u, 0x43467E68u})
        if (!data.native_cell<void*>(identity))
            throw std::invalid_argument("Packet engine alias or class producer is missing");
    for (const auto identity : {0x434679D0u, 0x43468338u, 0x434671CCu, 0x43467A8Cu, 0x4346A42Cu})
        if (!engine.variables.at(identity).get())
            throw std::invalid_argument("Packet ConVar is not acquired");
    return storage;
}
}
PacketRuntimeImports native_packet_runtime_imports(ClientRuntimeStorage& storage,
                                                   const EngineRuntimeBindings& engine) {
    if (&engine.variables.data() != &storage.data())
        throw std::invalid_argument("Packet module inputs must share canonical source storage");
    const auto module = GetModuleHandleW(L"vstdlib.dll");
    if (!module || module != engine.modules.at(u"vstdlib.dll") ||
        module != storage.data().native_cell<void*>(0x43468A7Cu))
        throw std::invalid_argument("Packet requires its actual already loaded vstdlib module");
    const auto seed = GetProcAddress(module, "RandomSeed"),
               integer = GetProcAddress(module, "RandomInt");
    if (!seed || !integer)
        throw std::runtime_error("Packet requires vstdlib RandomSeed and RandomInt exports");
    engine::initialize_build_packet_bindings(storage.data());
    return {reinterpret_cast<PacketRuntimeImports::Seed>(seed),
            reinterpret_cast<PacketRuntimeImports::Integer>(integer)};
}
struct PacketRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    PacketRuntimeImports imports;
    InputBitmap input;
    network::FakeLagState state;
    engine::PacketBoxRuntime box;
    rage::TargetBindings targeting;
    network::PacketWorld world;
    network::PacketCommandOwner command;
    PacketRuntimeFactoryBindings factory;
    static void seed(void* raw, std::int32_t value) {
        static_cast<Data*>(raw)->imports.seed(value);
    }
    static std::int32_t integer(void* raw, std::int32_t first, std::int32_t last) {
        return static_cast<Data*>(raw)->imports.integer(first, last);
    }
    static void initialize(void* raw) {
        auto& self = *static_cast<Data*>(raw);
        create_packet_runtime_owner(self.factory, self.storage.configuration());
    }
    Data(ClientRuntimeStorage& s, const EngineRuntimeBindings& e, ConfigurationRuntimeOwner& c,
         HistoryRuntimeOwner& h, MovementHudRuntimeOwner& m, TargetRefreshRuntimeOwner& target,
         PacketRuntimeImports i)
        : storage(checked(s, e, c, h, m, target, i)), imports(i), input(s.input_runtime().bitmap()),
          state{c.fake_lag_settings().amount.value,
                c.fake_lag_settings().variance.value,
                c.fake_lag_settings().limit.value,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                0,
                {},
                0,
                0,
                0,
                {},
                0},
          box(s.data()), targeting{s.data().native_cell<void*>(0x43475808u),
                                   s.data().native_cell<void*>(0x4346A890u),
                                   h.game_type(),
                                   h.teammates_are_enemies(),
                                   h.players().adjustments(),
                                   h.target_offsets()},
          world{s.data().native_cell<void*>(0x4346A890u),
                s.data().native_cell<void*>(0x43475808u),
                h.target_offsets().rejection_bytes[0],
                offset(s, 0x434670CCu),
                offset(s, 0x434682A4u),
                e.variables.at(0x434679D0u),
                e.variables.at(0x43468338u),
                e.variables.at(0x434671CCu),
                e.variables.at(0x43467A8Cu),
                e.variables.at(0x4346A42Cu),
                s.data().native_cell<const void*>(0x43467E68u),
                network::PacketPlayerGeometry(h.players().cache()),
                box.binding()},
          command{
              state,
              s.data().native_cell<anti_aim::TargetState>(0x4346DB14u),
              c.fake_lag_settings(),
              {s.data().native_cell<std::int32_t*>(0x434757DCu),
               s.data().native_cell<std::int32_t*>(0x43475B2Cu),
               s.data().native_cell<float*>(0x4346A338u), s.data().native_cell<float*>(0x4346A334u),
               s.data().native_cell<void*>(0x43467B04u), s.data().native_cell<void*>(0x4346A514u),
               s.data().native_cell<void*>(0x4346A890u), s.data().native_cell<void**>(0x4346A3A4u),
               s.data().native_cell<std::int32_t*>(0x43469300u),
               s.data().native_cell<std::int32_t*>(0x434678F8u),
               s.data().native_cell<std::int32_t>(0x434759C4u),
               s.data().native_cell<std::uint8_t>(0x43468D7Cu), c.movement().air_duck.value,
               s.data().native_cell<std::int32_t>(0x43469274u), input,
               s.data().native_cell<std::uint8_t*>(0x434683D0u),
               s.data().native_cell<bool>(0x4347CA30u), s.data().native_cell<void**>(0x43475B90u)},
              {offset(s, 0x434670CCu), offset(s, 0x434688D0u), offset(s, 0x434679F8u),
               offset(s, 0x4346A8E4u), offset(s, 0x43468BD4u), offset(s, 0x434678A4u),
               offset(s, 0x43469374u), offset(s, 0x43467910u), offset(s, 0x43468378u),
               offset(s, 0x4346836Cu), offset(s, 0x43468BF4u)},
              {world,
               target,
               targeting,
               m.movement_capture(),
               m.movement_environment(),
               {s.data().native_cell<void*>(0x4346A890u), offset(s, 0x4346A60Cu),
                offset(s, 0x4346749Cu), offset(s, 0x434670A4u)},
               {this, seed, integer},
               s.data().native_cell<std::array<std::uint32_t, 3>>(0x43474580u)}},
          factory{{s.owner_registries().at(0x43475B78u), s.memory()},
                  s.command_phases(),
                  c.fake_lag_settings(),
                  command} {}
};
PacketRuntimeOwner::PacketRuntimeOwner(ClientRuntimeStorage& s, const EngineRuntimeBindings& e,
                                       ConfigurationRuntimeOwner& c, HistoryRuntimeOwner& h,
                                       MovementHudRuntimeOwner& m, TargetRefreshRuntimeOwner& t)
    : PacketRuntimeOwner(s, e, c, h, m, t, native_packet_runtime_imports(s, e)) {}
PacketRuntimeOwner::PacketRuntimeOwner(ClientRuntimeStorage& s, const EngineRuntimeBindings& e,
                                       ConfigurationRuntimeOwner& c, HistoryRuntimeOwner& h,
                                       MovementHudRuntimeOwner& m, TargetRefreshRuntimeOwner& t,
                                       PacketRuntimeImports i)
    : data_(std::make_unique<Data>(s, e, c, h, m, t, i)) {}
PacketRuntimeOwner::~PacketRuntimeOwner() = default;
ClientRuntimeStorage& PacketRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
network::PacketCommandOwner& PacketRuntimeOwner::command() noexcept {
    return data_->command;
}
network::FakeLagState& PacketRuntimeOwner::state() noexcept {
    return data_->state;
}
PacketRuntimeFactoryBindings& PacketRuntimeOwner::factory_bindings() noexcept {
    return data_->factory;
}
void PacketRuntimeOwner::bind_initializers(NativeStartupSources& sources) {
    if (&sources.data() != &storage().data())
        throw std::invalid_argument("Packet factory requires the canonical startup dispatcher");
    sources.bind_initializer({packet_runtime_factory, data_.get(), Data::initialize});
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: player_command_queries.cpp
// ============================================================================
#if defined(_MSC_VER)
#define SKEET_PLAYER_QUERY_NOINLINE __declspec(noinline)
#define SKEET_PLAYER_QUERY_CALLER _ReturnAddress()
#else
#define SKEET_PLAYER_QUERY_NOINLINE [[gnu::noinline]]
#define SKEET_PLAYER_QUERY_CALLER __builtin_return_address(0)
#endif
#if defined(__GNUC__) || defined(__clang__)
#define SKEET_PLAYER_QUERY_FORCEINLINE [[gnu::always_inline]] inline
#define SKEET_PLAYER_QUERY_ASM_BRIDGE [[gnu::used, gnu::noinline]]
#elif defined(_MSC_VER)
#define SKEET_PLAYER_QUERY_FORCEINLINE __forceinline
// The MSVC inline assembler retains its direct symbol reference; GNU's string
// assembler needs the explicit used attribute above for this same bridge.
#define SKEET_PLAYER_QUERY_ASM_BRIDGE __declspec(noinline)
#endif

namespace skeet::client {
namespace {
PlayerCommandQueries* active_queries;

/* deduplicated helper read */
template <class Result, class... Arguments>
Result original(void* receiver, std::size_t slot, Arguments... arguments) {
    auto** table = *static_cast<void***>(receiver);
    const auto* metadata = static_cast<const engine::VtableHookMetadata*>(table[-2]);
    return reinterpret_cast<engine::MemberFunction<Result, Arguments...>>(
        metadata->original_table[slot])(receiver, arguments...);
}
}

void PlayerCommandQueries::dispatch_update_call(void* player,
                                                std::array<std::uint32_t, 4> args) const {
    if (bindings.update_phase == 1u)
        return;
    original<void>(player, 78, args[0], args[1], args[2], args[3]);
}

std::uint32_t PlayerCommandQueries::query_callsite(void* player, std::uint32_t caller_esi,
                                                   std::uintptr_t return_address,
                                                   std::uintptr_t stack_word_08) const {
    if (stack_word_08 == bindings.forced_query_stack)
        return 1u;
    if (bindings.team_query_override && !bindings.update_mode &&
        return_address == bindings.team_query_return) {
        if (read<std::int32_t>(player, bindings.team) == 3 ||
            read<std::int32_t>(player, bindings.team) == 2)
            return caller_esi;
    }
    return original<std::uint32_t>(player, 88);
}

std::uint8_t PlayerCommandQueries::interpolate(void* player, std::uint32_t time_bits) const {
    if (bindings.interpolation_guard)
        *bindings.interpolation_guard = 0;
    if (!engine::invoke<std::uint8_t>(bindings.engine_client, 82) &&
        !engine::invoke<std::uint8_t>(bindings.engine_client, 93) &&
        !read<std::uint8_t>(player, bindings.rejection_byte)) {
        const auto number = std::bit_cast<std::int32_t>(std::uint32_t(*bindings.choked_commands) +
                                                        std::uint32_t(*bindings.last_outgoing));
        const auto* command = find_input_command(bindings.input, number);
        if (command && command->tick == 0x7F7FFFFF)
            return 1;
    }
    // Float occupies one stack word on the native ABI. Forwarding its bits
    // avoids an extra x87 load/store that could alter a signaling NaN.
    return original<std::uint8_t>(player, 113, time_bits);
}

std::array<float, 3>* PlayerCommandQueries::eye_angles(void* player,
                                                       std::uintptr_t return_address) const {
    if (return_address == bindings.sent_angles_return && !bindings.update_mode)
        return &bindings.sent_history.angles;
    return original<std::array<float, 3>*>(player, 170);
}

// Slot88 has no ordinary stack arguments. Its native thunk copies incoming
// ESI into EDX and its callee inspects [entryESP] and [entryESP+8]. Capture
// all three words before a C++ prologue can use ESI or change ESP.
extern "C" SKEET_PLAYER_QUERY_ASM_BRIDGE std::uint32_t __cdecl
skeet_player_query88_dispatch(void* player, std::uint32_t caller_esi, std::uintptr_t return_address,
                              std::uintptr_t stack_word_08) {
    return active_queries->query_callsite(player, caller_esi, return_address, stack_word_08);
}

#if defined(__i386__) && (defined(__GNUC__) || defined(__clang__))
#define SKEET_PLAYER_QUERY_STRING_IMPL(value) #value
#define SKEET_PLAYER_QUERY_STRING(value) SKEET_PLAYER_QUERY_STRING_IMPL(value)
extern "C" __attribute__((naked, used)) std::uint32_t skeet_player_query88_entry() {
    __asm__ __volatile__(
        "pushl %ebp\n\t"
        "movl %esp, %ebp\n\t"
        "andl $-16, %esp\n\t"
        "pushl 12(%ebp)\n\t"
        "pushl 4(%ebp)\n\t"
        "pushl %esi\n\t"
        "pushl %ecx\n\t"
        "call " SKEET_PLAYER_QUERY_STRING(__USER_LABEL_PREFIX__) "skeet_player_query88_dispatch\n\t"
                                                                 "movl %ebp, %esp\n\t"
                                                                 "popl %ebp\n\t"
                                                                 "ret\n\t");
}
#undef SKEET_PLAYER_QUERY_STRING
#undef SKEET_PLAYER_QUERY_STRING_IMPL
#elif defined(_M_IX86) && defined(_MSC_VER)
extern "C" __declspec(naked) std::uint32_t skeet_player_query88_entry() {
    __asm {
        push ebp
        mov ebp,esp
        and esp,-16
        push dword ptr [ebp+12]
        push dword ptr [ebp+4]
        push esi
        push ecx
        call skeet_player_query88_dispatch
        mov esp,ebp
        pop ebp
        ret
    }
}
#endif

namespace {
#if defined(_M_IX86) || defined(__i386__)
void __fastcall update_call_entry(void* player, void*, std::uint32_t a, std::uint32_t b,
                                  std::uint32_t c, std::uint32_t d) {
    active_queries->dispatch_update_call(player, {a, b, c, d});
}
std::uint8_t __fastcall interpolation_entry(void* player, void*, float time) {
    return active_queries->interpolate(player, std::bit_cast<std::uint32_t>(time));
}
SKEET_PLAYER_QUERY_NOINLINE std::array<float, 3>* __fastcall eye_angles_entry(void* player, void*) {
    const auto caller = reinterpret_cast<std::uintptr_t>(SKEET_PLAYER_QUERY_CALLER);
    return active_queries->eye_angles(player, caller);
}
#else
void update_call_entry(void* player, std::uint32_t a, std::uint32_t b, std::uint32_t c,
                       std::uint32_t d) {
    active_queries->dispatch_update_call(player, {a, b, c, d});
}
std::uint8_t interpolation_entry(void* player, float time) {
    return active_queries->interpolate(player, std::bit_cast<std::uint32_t>(time));
}
SKEET_PLAYER_QUERY_NOINLINE std::array<float, 3>* eye_angles_entry(void* player) {
    const auto caller = reinterpret_cast<std::uintptr_t>(SKEET_PLAYER_QUERY_CALLER);
    return active_queries->eye_angles(player, caller);
}
#endif
}

void* PlayerCommandQueries::native_replacement_entry(std::size_t slot, std::uint32_t entry) {
    void* replacement;
    switch (entry) {
    case 0x433842C1u:
        if (slot != 78)
            throw std::invalid_argument("Player update-call slot does not match native entry");
        replacement = reinterpret_cast<void*>(&update_call_entry);
        break;
    case 0x4334D3EBu:
        if (slot != 88)
            throw std::invalid_argument("Player callsite-query slot does not match native entry");
#if defined(_M_IX86) || defined(__i386__)
        replacement = reinterpret_cast<void*>(&skeet_player_query88_entry);
        break;
#else
        throw std::logic_error(
            "The player callsite-query entry requires the native x86 ESI/stack ABI");
#endif
    case 0x4340191Du:
        if (slot != 113)
            throw std::invalid_argument("Player interpolation slot does not match native entry");
        replacement = reinterpret_cast<void*>(&interpolation_entry);
        break;
    case 0x433E97C1u:
        if (slot != 170)
            throw std::invalid_argument("Player eye-angle slot does not match native entry");
        replacement = reinterpret_cast<void*>(&eye_angles_entry);
        break;
    default:
        throw std::invalid_argument("Unknown captured player command-query entry");
    }
    return replacement;
}
void PlayerCommandQueries::bind_native_replacement(engine::VtableHookMetadata& metadata,
                                                   std::size_t slot, std::uint32_t entry) {
    auto* replacement = native_replacement_entry(slot, entry);
    active_queries = this;
    metadata.replacement_table[slot] = replacement;
}
} // namespace skeet::client
#undef SKEET_PLAYER_QUERY_NOINLINE
#undef SKEET_PLAYER_QUERY_CALLER
#undef SKEET_PLAYER_QUERY_FORCEINLINE
#undef SKEET_PLAYER_QUERY_ASM_BRIDGE

// ============================================================================
// Subcomponent: player_data_update.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */
/* deduplicated helper store */
}
PlayerDataUpdateOffsets
native_player_data_update_offsets(const engine::NativeOffsetGlobals& globals) {
    const auto decode = [&](std::uint32_t pointer, std::uint32_t mask, std::uint32_t key) {
        return std::ptrdiff_t(*globals.pointers.at(pointer) ^ globals.keys.at(mask) ^ key);
    };
    return {decode(0x434679F8, 0x434675F8, 0x3309C0CF), decode(0x43468354, 0x4346852C, 0x66146802),
            decode(0x43467230, 0x43467100, 0xD3D4230D), decode(0x43468FEC, 0x4346A3C4, 0xA2A8247F),
            decode(0x4346A8E4, 0x434688CC, 0x36375C23), decode(0x43468BF8, 0x43467B8C, 0xD070684C)};
}
void capture_serialized_player_state(NativePlayerStateLayout& cache, void* player,
                                     const PlayerDataUpdateOffsets& offsets) {
    auto* bytes = reinterpret_cast<std::byte*>(&cache);
    std::memcpy(bytes + 0x2FC, bytes + 0x468, 0x16C);
    store(&cache, 0x104, field<std::uint32_t>(player, offsets.duck_3309c0cf));
    store(&cache, 0x150, field<std::uint32_t>(player, offsets.scaled_recoil_66146802));
    store(&cache, 0x188, field<std::uint32_t>(&cache, 0x1A8));
    store(&cache, 0x114, field<std::uint32_t>(&cache, 0x1AC));
    store(&cache, 0x154, field<std::uint32_t>(player, offsets.replicated_time_d3d4230d));
    store(&cache, 0x184, field<std::uint32_t>(player, offsets.body_yaw_a2a8247f));
    const auto* virtual77 = engine::invoke<const std::byte*>(player, 77);
    store(&cache, 0x11C, field<std::uint32_t>(virtual77, 8));
    // The velocity read follows virtual77; the engine may change the player.
    for (std::size_t index = 0; index < 3; ++index)
        store(&cache, 0x120 + index * 4,
              field<std::uint32_t>(player, offsets.velocity_36375c23 + index * 4));
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: player_eye_position.cpp
// ============================================================================

namespace skeet::client {
void PlayerPhysicsHooks::eye_position(void* player, std::array<float, 3>& output) {
    using namespace player_physics_detail;
    // Complete43367C8B and43382222, preserving the virtual-origin read and
    // the SSE component return's calculation order before publishing output.
    const auto* origin = engine::invoke<const std::uint32_t*>(player, 10);
    std::array<std::uint32_t, 3> captured;
    captured[0] = origin[0];
    captured[1] = origin[1];
    auto duck = _mm_load_ss(reinterpret_cast<const float*>(static_cast<const std::byte*>(player) +
                                                           bindings.fields.duck_amount_3309c0cf));
    duck = _mm_mul_ss(duck, duck);
    captured[2] = origin[2];
    auto smooth = duck;
    duck = _mm_add_ss(duck, duck);
    smooth = _mm_mul_ss(smooth, _mm_set_ss(3.0f));
    duck = _mm_mul_ss(
        duck, _mm_load_ss(reinterpret_cast<const float*>(static_cast<const std::byte*>(player) +
                                                         bindings.fields.duck_amount_3309c0cf)));
    smooth = _mm_sub_ss(smooth, duck);
    smooth = _mm_sub_ss(_mm_set_ss(1.0f), smooth);
    smooth = _mm_mul_ss(smooth, _mm_set_ss(18.0f));
    smooth = _mm_add_ss(smooth, _mm_set_ss(46.0f));
    const auto z = _mm_castsi128_ps(_mm_cvtsi32_si128(std::bit_cast<std::int32_t>(captured[2])));
    smooth = _mm_add_ss(smooth, z);
    store(output.data(), 0, captured[0]);
    store(output.data(), 4, captured[1]);
    store(output.data(), 8, std::uint32_t(_mm_cvtsi128_si32(_mm_castps_si128(smooth))));
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: player_receive.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* [stripped helper] */
/* [stripped helper] */
std::uint32_t index(void* entity) {
    return engine::invoke<std::uint32_t>(static_cast<std::byte*>(entity) + 8, 10);
}
NativePlayerStateLayout& state(PlayerStateCache& players, std::uint32_t id) {
    auto* result = players.get(id);
    if (!result)
        throw std::bad_alloc();
    return result->native;
}
}
void PlayerReceiveRuntime::original(std::uint32_t cell, const engine::RecvProxyData* packet,
                                    void* entity, void* output) const {
    data_.native_cell<engine::RecvVarProxy>(cell)(packet, entity, output);
}
void PlayerReceiveRuntime::simulation_time(const engine::RecvProxyData* packet, void* entity,
                                           void* output) {
    original(0x434769E4u, packet, entity, output);
    const auto id = index(entity);
    if (id - 1u > 63u || !engine::invoke<bool>(entity, 158))
        return;
    auto& owner = state(players_, id);
    const auto& fields = offsets_.globals;
    const auto offset =
        *fields.pointers.at(0x43469198u) ^ fields.keys.at(0x4346A538u) ^ 0xD5FD2E2Cu;
    write(&owner, 0x10C, read<std::uint32_t>(entity, offset));
}
void PlayerReceiveRuntime::timestamped(const engine::RecvProxyData* packet, void* entity,
                                       void* output, std::uint32_t saved, std::size_t value_offset,
                                       std::size_t stamp_offset) {
    original(saved, packet, entity, output);
    const auto id = index(entity);
    if (id - 1u > 63u || !engine::invoke<bool>(entity, 158))
        return;
    auto& owner = state(players_, id);
    write(&owner, value_offset, read<std::uint32_t>(packet, 8));
    const auto tick = lagcomp::sample_receive_tick(data_.native_cell<void*>(0x4346A514u),
                                                   data_.native_cell<std::byte*>(0x43467B04u));
    write(&owner, stamp_offset, tick);
}
void PlayerReceiveRuntime::tick_base(const engine::RecvProxyData* packet, void* entity,
                                     void* output) {
    timestamped(packet, entity, output, 0x43476AC4u, 0x1E0, 0x1E4);
}
void PlayerReceiveRuntime::velocity(const engine::RecvProxyData* packet, void* entity, void* output,
                                    unsigned component) {
    constexpr std::uint32_t saved[] = {0x43476954u, 0x43476A04u, 0x43476A64u};
    if (component >= 3)
        throw std::out_of_range("Receive velocity component");
    timestamped(packet, entity, output, saved[component], 0x1C8 + component * 4, 0x1DC);
}
void PlayerReceiveRuntime::eye_angle(const engine::RecvProxyData* packet, void* entity,
                                     void* output, unsigned component) {
    constexpr std::uint32_t saved[] = {0x43476994u, 0x43476A54u};
    if (component >= 2)
        throw std::out_of_range("Receive eye angle component");
    const auto id = index(entity);
    if (id - 1u <= 63u) {
        auto& owner = state(players_, id);
        write(&owner, 0x1A8 + component * 4, read<std::uint32_t>(packet, 8));
        // The engine packet is mutable; the saved proxy consumes this value.
        write(const_cast<engine::RecvProxyData*>(packet), 8,
              normalize_movement_angle(read<float>(packet, 8)));
    }
    original(saved[component], packet, entity, output);
}
void PlayerReceiveRuntime::client_animation(const engine::RecvProxyData* packet, void* entity,
                                            void* output) {
    const auto id = index(entity);
    if (id - 1u <= 63u) {
        const auto incoming = std::uint8_t(read<std::uint32_t>(packet, 8) != 0);
        auto& owner = state(players_, id);
        write(&owner, 0x17C, incoming);
    }
    original(0x434769D4u, packet, entity, output);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: player_receive_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
PlayerReceiveRuntime* active_players = nullptr;
animation::LayerReceiveRuntime* active_layers = nullptr;
template <unsigned Kind>
void receive(const engine::RecvProxyData* packet, void* entity, void* output) {
    if constexpr (Kind == 0)
        active_players->simulation_time(packet, entity, output);
    else if constexpr (Kind == 1)
        active_players->tick_base(packet, entity, output);
    else if constexpr (Kind < 5)
        active_players->velocity(packet, entity, output, Kind - 2);
    else if constexpr (Kind < 7)
        active_players->eye_angle(packet, entity, output, Kind - 5);
    else if constexpr (Kind == 7)
        active_players->client_animation(packet, entity, output);
    else
        active_layers->receive(static_cast<animation::ReceivedLayerField>(Kind - 8), packet, entity,
                               output);
}
constexpr std::uint32_t hash(const char* text) {
    std::uint32_t value = 0x811C9DC5u;
    for (; *text; ++text)
        value = (value ^ std::uint8_t(*text)) * 0x1000193u;
    return value ^ 0x92CA5DF1u;
}
constexpr std::array<PlayerReceiveDescriptor, 15> descriptors{
    {{0x91C, hash("CBaseEntity"), hash("DT_BaseEntity"), hash("m_flSimulationTime"), 0x433FCA81,
      0x434769DC, receive<0>},
     {0x930, hash("CBasePlayer"), hash("DT_LocalPlayerExclusive"), hash("m_nTickBase"), 0x43402A54,
      0x43476ABC, receive<1>},
     {0x944, hash("CBasePlayer"), hash("DT_LocalPlayerExclusive"), hash("m_vecVelocity[0]"),
      0x433C6496, 0x4347694C, receive<2>},
     {0x958, hash("CBasePlayer"), hash("DT_LocalPlayerExclusive"), hash("m_vecVelocity[1]"),
      0x43343176, 0x434769FC, receive<3>},
     {0x96C, hash("CBasePlayer"), hash("DT_LocalPlayerExclusive"), hash("m_vecVelocity[2]"),
      0x43417E35, 0x43476A5C, receive<4>},
     {0x980, hash("CCSPlayer"), hash("DT_CSPlayer"), hash("m_angEyeAngles[0]"), 0x4341DC67,
      0x4347698C, receive<5>},
     {0x994, hash("CCSPlayer"), hash("DT_CSPlayer"), hash("m_angEyeAngles[1]"), 0x4336B0E6,
      0x43476A4C, receive<6>},
     {0x9A8, hash("CBaseAnimating"), hash("DT_BaseAnimating"), hash("m_bClientSideAnimation"),
      0x4338A848, 0x434769CC, receive<7>},
     {0x9BC, hash("CBaseAnimatingOverlay"), hash("DT_Animationlayer"), hash("m_flCycle"),
      0x433898BD, 0x43476AAC, receive<8>},
     {0x9D0, hash("CBaseAnimatingOverlay"), hash("DT_Animationlayer"), hash("m_flPrevCycle"),
      0x4332A584, 0x43476A7C, receive<9>},
     {0x9E4, hash("CBaseAnimatingOverlay"), hash("DT_Animationlayer"), hash("m_nSequence"),
      0x433CE396, 0x4347696C, receive<10>},
     {0x9F8, hash("CBaseAnimatingOverlay"), hash("DT_Animationlayer"), hash("m_flWeight"),
      0x43337A05, 0x43476AFC, receive<11>},
     {0xA0C, hash("CBaseAnimatingOverlay"), hash("DT_Animationlayer"), hash("m_flPlaybackRate"),
      0x433FBAE8, 0x43476A1C, receive<12>},
     {0xA20, hash("CBaseAnimatingOverlay"), hash("DT_Animationlayer"), hash("m_flWeightDeltaRate"),
      0x4339768A, 0x4347697C, receive<13>},
     {0xA34, hash("CBaseAnimatingOverlay"), hash("DT_Animationlayer"), hash("m_nOrder"), 0x433D702B,
      0x4347699C, receive<14>}}};
ClientRuntimeStorage& require_owners(ClientRuntimeStorage& storage, PlayerRuntimeOwner& players,
                                     animation::FixRuntimeOwner& fix) {
    if (&players.storage() != &storage || &fix.storage() != &storage ||
        &fix.bones_owner().players() != &players.cache() ||
        &fix.animations().bindings.players != &players.cache())
        throw std::invalid_argument("Receive owners must share canonical storage and player cache");
    if (reinterpret_cast<void*>(&fix.animations().bindings.original_layer_proxy) !=
        reinterpret_cast<void*>(&storage.data().native_cell<engine::RecvVarProxy>(0x43476974u)))
        throw std::invalid_argument(
            "Sequence receiver must borrow the canonical saved original cell");
    if (!storage.data().native_cell<void*>(0x4346A514u) ||
        !storage.data().native_cell<void*>(0x43467B04u))
        throw std::invalid_argument(
            "Receive owners require initialized engine interfaces and gpGlobals");
    return storage;
}
}
std::span<const PlayerReceiveDescriptor, 15> player_receive_descriptors() noexcept {
    return descriptors;
}
PlayerReceiveOwner::PlayerReceiveOwner(ClientRuntimeStorage& storage, PlayerRuntimeOwner& players,
                                       animation::FixRuntimeOwner& fix)
    : storage_(require_owners(storage, players, fix)),
      players_(storage_.data(), storage_.offsets(), players.cache()),
      layers_(storage_.data(), storage_.offsets(), fix.animations()) {}
void bind_player_receive_patches(engine::NativeDataImage& data, PlayerReceiveRuntime& players,
                                 animation::LayerReceiveRuntime& layers) {
    if (&players.data() != &data || &layers.data() != &data ||
        &players.player_cache() != &layers.animations().bindings.players ||
        reinterpret_cast<void*>(&layers.animations().bindings.original_layer_proxy) !=
            reinterpret_cast<void*>(&data.native_cell<engine::RecvVarProxy>(0x43476974u)))
        throw std::invalid_argument(
            "Receive entries require the same data image, player cache and sequence original cell");
    for (const auto& row : descriptors) {
        const auto& node = data.native_cell<EntityUpdatePatch>(row.patch_address);
        if (!node.target || !node.after)
            throw std::invalid_argument(
                "Player receive patches require captured targets and originals");
    }
    active_players = &players;
    active_layers = &layers;
    for (const auto& row : descriptors)
        data.native_cell<EntityUpdatePatch>(row.patch_address).before = std::bit_cast<std::int32_t>(
            std::uint32_t(reinterpret_cast<std::uintptr_t>(row.replacement)));
}
void initialize_player_receive_patch_targets(engine::NativeDataImage& data) {
    auto* client = data.native_cell<void*>(0x434683E0u);
    if (!client)
        throw std::invalid_argument("Player receive patches require canonical VClient018");
    struct Capture {
        EntityUpdatePatch* node;
        engine::RecvProp* property;
        engine::RecvVarProxy original;
    };
    std::array<Capture, 15> captures;
    for (std::size_t i = 0; i < descriptors.size(); ++i) {
        const auto& row = descriptors[i];
        auto& node = data.native_cell<EntityUpdatePatch>(row.patch_address);
        if (node.target || node.after || node.before)
            throw std::logic_error("Player receive patch already initialized");
        auto* property = engine::find_receive_property(client, row.class_hash, row.table_hash,
                                                       row.property_hash);
        if (!property || !property->proxy)
            throw std::runtime_error("Required player receive property or original is missing");
        captures[i] = {&node, property, property->proxy};
    }
    for (const auto& capture : captures) {
        capture.node->target = reinterpret_cast<volatile std::int32_t*>(&capture.property->proxy);
        capture.node->after = std::bit_cast<std::int32_t>(
            std::uint32_t(reinterpret_cast<std::uintptr_t>(capture.original)));
    }
}
void PlayerReceiveOwner::bind_receive_patches() {
    bind_player_receive_patches(storage_.data(), players_, layers_);
}
void PlayerReceiveOwner::initialize_receive_patches() {
    initialize_player_receive_patch_targets(storage_.data());
    bind_receive_patches();
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: player_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
ClientRuntimeStorage& PlayerRuntimeOwner::require_available(ClientRuntimeStorage& storage) {
    if (storage.active_player_owner_)
        throw std::logic_error("The source player cache already has a process owner");
    return storage;
}
PlayerRuntimeOwner::PlayerRuntimeOwner(ClientRuntimeStorage& storage)
    : storage_(require_available(storage)),
      cache_(storage.memory(),
             storage.data().native_cell<std::array<NativePlayerStateLayout*, 64>>(0x434797BCu)),
      adjustments_(storage.data().native_cell<players::State>(0x4347BE04u)) {
    storage_.active_player_owner_ = this;
}
PlayerRuntimeOwner::~PlayerRuntimeOwner() {
    // The registered source terminal pass may already have drained these
    // exact slots. Repeating an empty drain releases no additional objects.
    cache_.finalize_history(records_);
    storage_.active_player_owner_ = nullptr;
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: player_state.cpp
// ============================================================================

namespace skeet::client {
PlayerState::PlayerState(NativePlayerStateLayout& data)
    : native(data), geometry(data.geometry), cached_target_height(data.cached_target_height),
      cached_definition(data.cached_definition), fallback(data.fallback) {
    history.first = data.history_first;
    history.count = data.history_count;
    for (std::size_t index = 0; index < 32; ++index)
        history.records[index] = reinterpret_cast<lagcomp::LagRecord*>(
            static_cast<std::uintptr_t>(data.native_history_links[index]));
}
resolver_runtime::EnvironmentOwner PlayerState::environment() const noexcept {
    resolver_runtime::EnvironmentOwner result{};
    for (std::size_t index = 0; index < 4; ++index)
        result.selection.attempt_by_mode[index] = native.resolver_slots[index].attempt;
    result.selection.status = native.resolver_status;
    result.probe_time = native.probe_time;
    result.probe_stamp = native.probe_stamp;
    result.probe_count = native.probe_count;
    result.reference_time = native.reference_time;
    result.reference_origin = native.reference_origin;
    result.last_update_tick = native.last_update_tick;
    result.reference_yaw = native.reference_yaw;
    return result;
}
void PlayerState::environment(const resolver_runtime::EnvironmentOwner& value) noexcept {
    for (std::size_t index = 0; index < 4; ++index)
        native.resolver_slots[index].attempt = value.selection.attempt_by_mode[index];
    native.resolver_status = value.selection.status;
    native.probe_time = value.probe_time;
    native.probe_stamp = value.probe_stamp;
    native.probe_count = value.probe_count;
    native.reference_time = value.reference_time;
    native.reference_origin = value.reference_origin;
    native.last_update_tick = value.last_update_tick;
    native.reference_yaw = value.reference_yaw;
}
lagcomp::PlayerCaptureState PlayerState::capture_state() const noexcept {
    lagcomp::PlayerCaptureState result{};
    result.field_180 = native.capture_field_180;
    result.field_1ac = std::bit_cast<std::uint32_t>(native.reference_yaw);
    result.resolver_index = native.resolver_index;
    result.resolver_state = native.resolver_status;
    // Capture consumes exactly the selected byte. The native owner guarantees
    // the resolver index's slot is readable, as at 433FD8F8.
    const auto bytes = reinterpret_cast<const std::uint8_t*>(&native);
    result.resolver_modes[result.resolver_index] =
        bytes[0x10 + std::uint32_t(result.resolver_index) * 0x24];
    return result;
}
void PlayerState::publish_history() noexcept {
    native.history_first = history.first;
    native.history_count = history.count;
    if constexpr (sizeof(void*) == 4)
        for (std::size_t index = 0; index < 32; ++index)
            native.native_history_links[index] = static_cast<std::uint32_t>(
                reinterpret_cast<std::uintptr_t>(history.records[index]));
}
lagcomp::HistoryRecordingState PlayerState::recording_state() const {
    return {history, fallback, native.flags, native.resolver_index};
}
void PlayerState::recording_state(const lagcomp::HistoryRecordingState& value) {
    // This publishes the complete owner projection, including padding cleared
    // by prepare_history_capture's native full-record memset.
    history = value.ring;
    std::memcpy(&fallback, &value.fallback, sizeof fallback);
    native.flags = value.flags;
    native.resolver_index = value.resolver_index;
    publish_history();
}

PlayerStateCache::PlayerStateCache(engine::JobMemory memory)
    : addresses(standalone_addresses_), memory_(memory) {
    if (!memory_.allocate || !memory_.free || !memory_.process_heap)
        throw std::invalid_argument("Player cache requires process heap exports");
}
PlayerStateCache::PlayerStateCache(engine::JobMemory memory,
                                   std::array<NativePlayerStateLayout*, 64>& shared)
    : addresses(shared), memory_(memory) {
    if (!memory_.allocate || !memory_.free || !memory_.process_heap)
        throw std::invalid_argument("Player cache requires process heap exports");
    for (auto* object : addresses)
        if (object)
            throw std::invalid_argument(
                "Source player cache cannot adopt pre-existing player allocations");
}
PlayerStateCache::~PlayerStateCache() {
    // The process terminal path uses finalize_history with its actual record
    //pool. An independently owned cache releases only still-present raw blocks
    //here; terminal cleanup has already nulled its freed slots.
    for (auto*& object : addresses) {
        free(object);
        object = nullptr;
    }
}
void PlayerStateCache::finalize_history(lagcomp::RecordPool& pool) {
    for (std::uint32_t index = 0; index < addresses.size(); ++index) {
        auto* const player = view(index);
        if (!player)
            continue;
        lagcomp::clear_history_records(player->history, pool);
        player->publish_history();
    }
    pool.finalize_free_records();
    for (std::size_t index = addresses.size(); index;) {
        const auto slot = --index;
        if (!addresses[slot])
            continue;
        free(addresses[slot]);
        addresses[slot] = nullptr;
        views_[slot].reset();
    }
}
NativePlayerStateLayout* PlayerStateCache::allocate() {
    constexpr std::uint32_t size = 0x1700, allocation_size = size + 0x20;
    auto* raw = memory_.allocate(memory_.process_heap, 8, allocation_size);
    if (!raw)
        return nullptr;
    const auto base = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (base + 15) & ~std::uintptr_t(15);
    const auto displacement = static_cast<std::uint32_t>(header - base);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    auto* object = ::new (reinterpret_cast<void*>(header + 16)) NativePlayerStateLayout;
    std::memset(object, 0, size);
    object->field_238 = 0;
    object->history_first = 0;
    object->history_count = 0;
    object->fallback.header.field_00 = 0;
    return object;
}
void PlayerStateCache::free(void* object) {
    if (!object)
        return;
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory_.free(memory_.process_heap, 0, bytes - 16 - displacement);
}
PlayerState* PlayerStateCache::view(std::uint32_t slot) {
    const auto pointer = addresses[slot];
    if (!pointer) {
        views_[slot].reset();
        return nullptr;
    }
    auto& existing = views_[slot];
    if (!existing || &existing->native != pointer)
        existing.emplace(*pointer);
    return &*existing;
}
PlayerState* PlayerStateCache::find(std::uint32_t index) {
    return view(index & 63);
}
PlayerState* PlayerStateCache::get(std::uint32_t index) {
    const auto slot = index & 63;
    if (!addresses[slot]) {
        const auto object = allocate();
        if (object != addresses[slot]) {
            free(addresses[slot]);
            addresses[slot] = object;
        }
    }
    return view(slot);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: prediction_history_lifecycle.cpp
// ============================================================================

namespace skeet::client {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define PREDICTION_HISTORY_MEMBER __thiscall
#else
#define PREDICTION_HISTORY_MEMBER
#endif
using PrimaryEntry = void(PREDICTION_HISTORY_MEMBER*)(void*);

void* allocate(engine::JobMemory memory, std::size_t size) {
    auto* const raw = memory.allocate(memory.process_heap, 8, size + 0x20);
    if (!raw)
        return nullptr;
    const auto base = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (base + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - base), total = std::uint32_t(size + 0x20);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &total, 4);
    return reinterpret_cast<void*>(header + 16);
}
void release(engine::JobMemory memory, void* object) {
    auto* const bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void construct_string(PredictionHistoryStorage& state, std::size_t index, const char* text,
                      engine::JobMemory memory) {
    //433B2213 ->433C1360. All22decoded native literals are nonempty.
    state.bind_sequence_string(index, {});
    const auto length = std::strlen(text);
    auto* const bytes = static_cast<char*>(allocate(memory, length + 1));
    PredictionHistoryString descriptor{bytes, bytes, bytes + length + 1};
    state.bind_sequence_string(index, descriptor); //433C1394..139C, before memmove
    // The native nonempty path does not establish an allocation-failure
    // continuation. Do not substitute a literal, empty text or another buffer.
    std::memmove(bytes, text, length);
    descriptor.end = bytes + length;
    state.bind_sequence_string(index, descriptor); //433C13B1, before terminator
    bytes[length] = 0;
}
void destroy_string(const PredictionHistoryString& text, engine::JobMemory memory) {
    const auto capacity =
        std::bit_cast<std::int32_t>(std::uint32_t(reinterpret_cast<std::uintptr_t>(text.limit)) -
                                    std::uint32_t(reinterpret_cast<std::uintptr_t>(text.begin)));
    if (capacity > 1 && text.begin)
        release(memory, const_cast<char*>(text.begin));
    //4336B1E9 preserves its descriptor words after release.
}

void PREDICTION_HISTORY_MEMBER pred_hist_destruction_entry(void*);
void PREDICTION_HISTORY_MEMBER pred_hist_settings_entry(void*);
void PREDICTION_HISTORY_MEMBER pred_hist_reset_entry(void*);
void PREDICTION_HISTORY_MEMBER pred_hist_refresh_entry(void*);
void PREDICTION_HISTORY_MEMBER pred_hist_unchanged_entry(void*) {} //actual433CCE8E
const std::array<PrimaryEntry, 9> pred_hist_primary_table{
    pred_hist_destruction_entry, pred_hist_settings_entry,  pred_hist_unchanged_entry, pred_hist_unchanged_entry, pred_hist_unchanged_entry,
    pred_hist_reset_entry,       pred_hist_unchanged_entry, pred_hist_refresh_entry,   pred_hist_unchanged_entry};

struct PredHistInstance {
    const PrimaryEntry* vtable = nullptr;
    NativeCommandPhaseReceiver command_receiver;
    NativeEntityObserverReceiver entity_receiver;
    PredictionHistoryLifecycleBindings bindings;
    ConfigRegistries& configuration;
    engine::JobMemory memory;

    PredHistInstance(const PredictionHistoryRegistries& registries, ConfigRegistries& config,
             PredictionHistoryLifecycleBindings context)
        : bindings(context), configuration(config), memory(registries.primary.memory) {
        auto& owner = bindings.owner;
        auto& state = bindings.storage;
        owner.canonical_storage = &state;
        registries.primary.append(this); //4347D024
        registries.commands.append(command_receiver, &owner); //02C
        registries.empty_ticks.append(owner); //034, table still unpublished
        registries.prediction.append(owner); //03C
        registries.entities.append_prediction_history(entity_receiver, owner); //044
        registries.data_updates.append(owner); //04C, table still unpublished
        registries.recharge.append(owner); //054, table still unpublished
        registries.entities.publish_prediction_history(entity_receiver); //05B
        registries.data_updates.publish(owner); //064
        registries.recharge.publish(owner); //06D
        vtable = pred_hist_primary_table.data(); //079
        registries.commands.publish(command_receiver, CommandOwnerId::owner_4346ff8c); //07F
        registries.empty_ticks.publish(owner); //086
        registries.prediction.publish(owner); //08D

        //4347D094's MOV is recorded in supplied mem_int3.h:462; raw is CCx5.
        state.initialize_header(bindings.acknowledged_command); //094..09C
        auto* const snapshot_allocation = static_cast<std::byte*>(allocate(memory, 0x3112C)); //09F
        network::ExploitCommandSnapshot* snapshots = nullptr;
        if (snapshot_allocation) {
            constexpr std::uint32_t count = 150;
            std::memcpy(snapshot_allocation, &count, 4);
            snapshots = ::new (snapshot_allocation + 4) network::ExploitCommandSnapshot[150];
            for (std::size_t index = 0; index < 150; ++index)
                animation::initialize_animation_state_vector(
                    reinterpret_cast<std::byte*>(snapshots + index) + 0x1F4);
        }
        state.bind_snapshots(snapshots, snapshots ? 150u : 0u); //0D1, including native null branch
        auto* const record_allocation = allocate(memory, 0x2C88); //0D4
        auto* const records = record_allocation ? ::new (record_allocation)
                                                      RestrictedCommandPrediction0x4C[150]
                                                : nullptr;
        state.bind_prediction_records(records, records ? 150u : 0u); //0D9
        state.redirect_from_44 = -1; //0DF
        state.redirect_to_48 = -1; //0E3
        const auto strings = prediction_history_sequence_strings();
        for (std::size_t index = 0; index < strings.size(); ++index)
            construct_string(state, index, strings[index], memory); //0DC..D868
    }
};
static_assert(sizeof(PredHistInstance) <= 0x65C);

void require_binding_identity(const PredictionHistoryRegistries& registries,
                              const PredictionHistoryLifecycleBindings& bindings) {
    if (&registries.recharge.image() != &registries.data_updates.image() ||
        &registries.prediction.image() != &registries.data_updates.image() ||
        &registries.empty_ticks.image() != &registries.data_updates.image())
        throw std::invalid_argument(
            "Prediction history observer registries must share their preserved image");
    if (&bindings.active !=
        &registries.data_updates.image().native_cell<PredictionHistoryOwner*>(0x434759B4u))
        throw std::invalid_argument(
            "Prediction history lifecycle requires the preserved434759B4 owner cell");
    auto& state = bindings.storage;
    const auto& owner = bindings.owner;
    if (&owner.shared != &state.shared() || &owner.redirect_from_44 != &state.redirect_from_44 ||
        &owner.redirect_to_48 != &state.redirect_to_48 ||
        (owner.canonical_storage && owner.canonical_storage != &state))
        throw std::invalid_argument(
            "Prediction history lifecycle requires its canonical owner and storage");
    if (state.has_standalone_backing() || state.shared().snapshots ||
        state.shared().prediction_records)
        throw std::invalid_argument(
            "Prediction history lifecycle requires deferred unbound native buffers");
    for (const auto& text : state.sequence_strings)
        if (text.begin || text.end || text.limit)
            throw std::invalid_argument(
                "Prediction history lifecycle requires unbound owned strings");
}

void PREDICTION_HISTORY_MEMBER pred_hist_destruction_entry(void* storage) {
    auto& instance = *static_cast<PredHistInstance*>(storage);
    destroy_prediction_history_owner(storage, instance.bindings.active);
}
void PREDICTION_HISTORY_MEMBER pred_hist_settings_entry(void* storage) {
    bind_prediction_history_owner_settings(storage);
}
void PREDICTION_HISTORY_MEMBER pred_hist_reset_entry(void* storage) {
    reset_prediction_history_owner(storage);
}
void PREDICTION_HISTORY_MEMBER pred_hist_refresh_entry(void* storage) {
    refresh_prediction_history_owner(storage);
}
void pred_hist_initialize_entry(void* context) {
    auto& source = *static_cast<PredictionHistoryInitialization*>(context);
    create_prediction_history_owner(source.registries, source.configuration, source.bindings);
}
#undef PREDICTION_HISTORY_MEMBER
}

std::size_t prediction_history_owner_size() noexcept {
    return 0x65C;
} //4349081E allocation extent
void* construct_prediction_history_owner(void* storage,
                                         const PredictionHistoryRegistries& registries,
                                         ConfigRegistries& configuration,
                                         PredictionHistoryLifecycleBindings bindings) {
    require_binding_identity(registries, bindings);
    return ::new (storage) PredHistInstance(registries, configuration, bindings);
}
void* create_prediction_history_owner(const PredictionHistoryRegistries& registries,
                                      ConfigRegistries& configuration,
                                      PredictionHistoryLifecycleBindings bindings) {
    require_binding_identity(registries, bindings);
    void* const storage = allocate(registries.primary.memory, prediction_history_owner_size());
    if (storage) {
        construct_prediction_history_owner(storage, registries, configuration, bindings);
        bindings.active = &bindings.owner; //43490837, mem_int3.h:654 MOV record
    } else
        bindings.active = nullptr; //ordinary raw XOR at0835, then same recorded store
    return storage;
}
void destroy_prediction_history_owner(void* storage, PredictionHistoryOwner*& active) {
    active = nullptr; //433B7312, before null test
    if (!storage)
        return;
    auto* const instance = static_cast<PredHistInstance*>(storage);
    auto& state = instance->bindings.storage;
    const auto memory = instance->memory;
    for (std::size_t index = state.sequence_strings.size(); index != 0; --index)
        destroy_string(state.sequence_strings[index - 1], memory);
    if (auto* records = state.shared().prediction_records) {
        release(memory, records);
        state.bind_prediction_records(nullptr, 0); //433AE465
    }
    if (auto* snapshots = state.shared().snapshots) {
        auto* const cookie = reinterpret_cast<std::byte*>(snapshots) - 4;
        std::uint32_t count;
        std::memcpy(&count, cookie, 4);
        //433FB67D visits the genuine count-cookie array in reverse.
        while (count) {
            --count;
            animation::release_animation_state_vector(
                reinterpret_cast<std::byte*>(snapshots + count) + 0x1F4,
                instance->bindings.animation_allocator);
        }
        release(memory, cookie);
        state.bind_snapshots(nullptr, 0); //433F925D
    }
    instance->~PredHistInstance();
    release(memory, storage);
}
void bind_prediction_history_owner_settings(void* storage) {
    auto& instance = *static_cast<PredHistInstance*>(storage);
    instance.bindings.storage.field_34 =
        &static_cast<IntegerConfig*>(find_config(instance.configuration.descriptors, 0x8F8A4366u))
             ->value;
}
void reset_prediction_history_owner(void* storage) {
    auto& state = static_cast<PredHistInstance*>(storage)->bindings.storage;
    for (std::size_t index = 0; index < 150; ++index)
        state.shared().snapshots[index].command_number = -1;
    for (std::size_t index = 0; index < 150; ++index)
        state.shared().prediction_records[index].command_number = -1;
    state.command_tags.fill(UINT32_MAX);
    state.landing_tags.fill(UINT32_MAX);
    state.redirect_from_44 = -1;
    state.redirect_to_48 = -1;
}
void refresh_prediction_history_owner(void* storage) {
    auto& instance = *static_cast<PredHistInstance*>(storage);
    instance.vtable[5](storage); //433CF2F5 calls the current primary slot5
}
void bind_prediction_history_initializer(NativeStartupSources& sources,
                                         PredictionHistoryInitialization& context) {
    sources.bind_initializer({0x4349081Eu, &context, pred_hist_initialize_entry});
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: prediction_history_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */
/* deduplicated helper */
bool enabled(const PredictionHistoryOwner& owner) noexcept { // complete4341ACE0
    const auto& b = owner.bindings.command;
    return *b.enabled_43467224 || *b.enabled_4346a740;
}
/* deduplicated previous helper */
void publish_player_angles(PredictionHistoryOwner& owner, void* player) {
    const auto& b = owner.bindings;
    auto angles = *engine::invoke<const std::array<float, 3>*>(player, 11);
    for (auto& angle : angles)
        angle = normalize_movement_angle(angle);
    *b.command.sent_angles = angles;
    const auto actual = *engine::invoke<const std::array<float, 3>*>(player, 11);
    write(player, b.restored_angles, actual);
}
void apply_sent_angles(PredictionHistoryOwner& owner, void* player) {
    const auto& b = owner.bindings;
    write(player, b.restored_angles, *b.command.sent_angles);
    const auto absolute = field<std::array<float, 3>>(player, b.absolute_angles);
    const auto& sent = *b.command.sent_angles;
    if (sent[0] == absolute[0] && sent[1] == absolute[1] && sent[2] == absolute[2])
        return;
    if (b.invalidate_angles)
        b.invalidate_angles(player, 2);
    // The engine invalidation is allowed to change the shared sent-angle
    // words. Native reloads them before writing the absolute-angle field.
    write(player, b.absolute_angles, *b.command.sent_angles);
}
// Complete433FD134 at the two fixed433F4628 call sites. The scale is exact
// for both8-bit[0,1] and15-bit[0,32767/32768], so the retry arm is not entered.
float quantize_cached_layer(float value, float maximum, float scale) noexcept {
    if (value < 0.0f)
        return 0.0f;
    if (value > maximum)
        return maximum;
    if (std::isnan(value))
        return 0.0f; // FISTP indefinite has a zero low dword.
    const float scaled = value * scale;
    const auto integer = static_cast<std::uint32_t>(std::nearbyint(static_cast<double>(scaled)));
    return static_cast<float>(integer) / scale;
}
}

std::span<const char* const, 22> prediction_history_sequence_strings() noexcept {
    static constexpr std::array<const char*, 22> strings{
        "_moving",        "_crouch", "moving", "crouch", "shield",  "land_", "heavy", "light",
        "reposition_to_", "idle",    "jump",   "knife",  "pistol",  "smg",   "rifle", "shotgun",
        "sniper",         "heavy",   "c4",     "pistol", "grenade", "knife"};
    return strings;
}
struct PredictionHistoryStorage::StandaloneBuffers {
    std::array<network::ExploitCommandSnapshot, 150> snapshots{};
    std::array<RestrictedCommandPrediction0x4C, 150> prediction_records{};
};
PredictionHistoryStorage::PredictionHistoryStorage(DeferredPredictionHistoryStorageTag) noexcept {
    view_.command_tags = command_tags.data();
    view_.landing_tags = landing_tags.data();
}
PredictionHistoryStorage::~PredictionHistoryStorage() = default;
void PredictionHistoryStorage::initialize_header(const std::int32_t* current_ack) noexcept {
    field_34 = nullptr;
    acknowledged_command = current_ack;
    view_.last_processed_command = current_ack;
}
void PredictionHistoryStorage::bind_snapshots(network::ExploitCommandSnapshot* first,
                                              std::size_t count) noexcept {
    snapshots = first ? std::span(first, count) : std::span<network::ExploitCommandSnapshot>{};
    view_.snapshots = first;
}
void PredictionHistoryStorage::bind_prediction_records(RestrictedCommandPrediction0x4C* first,
                                                       std::size_t count) noexcept {
    prediction_records =
        first ? std::span(first, count) : std::span<RestrictedCommandPrediction0x4C>{};
    view_.prediction_records = first;
}
void PredictionHistoryStorage::bind_sequence_string(std::size_t index,
                                                    PredictionHistoryString text) noexcept {
    sequence_strings[index] = text;
    switch (index) {
    case 0:
        view_.moving_modifier = text.begin;
        break;
    case 1:
        view_.crouching_modifier = text.begin;
        break;
    case 3:
        view_.layer_modifier = text.begin;
        break;
    case 8:
        view_.idle_activity = text.begin;
        break;
    case 9:
        view_.idle_modifier = text.begin;
        break;
    case 10:
        view_.jump_activity = text.begin;
        break;
    default:
        break;
    }
}
PredictionHistoryStorage::PredictionHistoryStorage(const std::int32_t* current_ack) noexcept
    : PredictionHistoryStorage(deferred_prediction_history_storage) {
    // Explicit semantic/fixture route. The lifecycle constructor below uses
    // the deferred route and the actual two native heap allocations instead.
    standalone_ = std::make_unique<StandaloneBuffers>();
    initialize_header(current_ack);
    bind_snapshots(standalone_->snapshots.data(), 150);
    bind_prediction_records(standalone_->prediction_records.data(), 150);
    for (auto& snapshot : snapshots)
        animation::initialize_animation_state(reinterpret_cast<std::byte*>(&snapshot) + 0x1F4);
    redirect_from_44 = redirect_to_48 = -1;
    const auto strings = prediction_history_sequence_strings();
    for (std::size_t index = 0; index < strings.size(); ++index)
        bind_sequence_string(index, {strings[index], nullptr, nullptr});
}

RestrictedPlayerCommandOwner* prediction_history_shared(PredictionHistoryOwner* owner) noexcept {
    return owner ? &owner->shared : nullptr;
}
const RestrictedPlayerCommandOwner*
prediction_history_shared(const PredictionHistoryOwner* owner) noexcept {
    return owner ? &owner->shared : nullptr;
}
network::ExploitCommandSnapshot*
prediction_history_snapshots(PredictionHistoryOwner* owner) noexcept {
    return owner ? owner->shared.snapshots : nullptr;
}
PredictionHistoryStorage& prediction_history_storage(PredictionHistoryOwner& owner) noexcept {
    return *owner.canonical_storage;
}
const PredictionHistoryStorage&
prediction_history_storage(const PredictionHistoryOwner& owner) noexcept {
    return *owner.canonical_storage;
}
const char* prediction_history_sequence_string(const PredictionHistoryOwner& owner,
                                               std::size_t index) noexcept {
    return prediction_history_storage(owner).sequence_strings[index].begin;
}

bool restore_prediction_snapshot(PredictionHistoryOwner& owner, void* player, std::int32_t number) {
    if (number == owner.redirect_from_44)
        number = owner.redirect_to_48;
    const auto& snapshot = owner.shared.snapshots[std::uint32_t(number) % 150U];
    if (snapshot.command_number != number)
        return false;
    const auto saved = field<lagcomp::AnimationState>(&snapshot, 0x14);
    animation::restore_entity_animation(saved, player, owner.bindings.command.offsets.animation);
    if (owner.bindings.set_absolute_angles) {
        const std::array<float, 3> angles{0.0f, field<float>(&snapshot, 0x1E4), 0.0f};
        owner.bindings.set_absolute_angles(player, angles);
    }
    return true;
}
bool copy_prediction_snapshot(PredictionHistoryOwner& owner, std::int32_t from,
                              std::int32_t to) noexcept {
    const auto& source = owner.shared.snapshots[std::uint32_t(from) % 150U];
    if (source.command_number != from)
        return false;
    auto& destination = owner.shared.snapshots[std::uint32_t(to) % 150U];
    std::memmove(&destination, &source, sizeof destination);
    destination.command_number = to;
    return true;
}
void prepare_prediction_history(PredictionHistoryOwner& owner, void* player,
                                const Command& command) {
    if (enabled(owner))
        restore_prediction_snapshot(owner, player, previous(command.number));
}
void finish_prediction_history(PredictionHistoryOwner& owner, CommandFrame& frame, void* player,
                               Command& command, std::uint8_t& send_packet) {
    process_restricted_player_command(&owner.shared, player, &command, &send_packet, true,
                                      owner.bindings.command);
    animation::publish_local_prediction_pose(frame, player, command, send_packet,
                                             owner.bindings.pose);
    if (!enabled(owner))
        return;
    if (!send_packet)
        restore_prediction_snapshot(owner, player, *owner.bindings.last_outgoing);
    publish_player_angles(owner, player);
}
void complete_prediction_history(PredictionHistoryOwner& owner, const CommandFrame& frame,
                                 void* player, const Command& command, std::uint8_t send_packet) {
    if (!send_packet || !(frame.flags & 0x10000000U) || !enabled(owner))
        return;
    owner.redirect_from_44 = command.number;
    owner.redirect_to_48 = *owner.bindings.last_outgoing;
    if (restore_prediction_snapshot(owner, player, *owner.bindings.last_outgoing))
        publish_player_angles(owner, player);
}
void apply_empty_prediction_history(PredictionHistoryOwner& owner, void* player) {
    if (enabled(owner))
        apply_sent_angles(owner, player);
}
void observe_prediction_history(PredictionHistoryOwner& owner, void* player) {
    if (enabled(owner) && player && !field<std::uint8_t>(player, owner.bindings.rejection_byte))
        apply_sent_angles(owner, player);
}
void begin_prediction_history(PredictionHistoryOwner& owner, void* player, const Command& command) {
    if (enabled(owner))
        owner.shared.prediction_records[std::uint32_t(command.number) % 150U].tick_base =
            field<std::int32_t>(player, owner.bindings.command.offsets.tick_base);
}
void capture_prediction_history(PredictionHistoryOwner& owner, void* player,
                                const Command& command) {
    const auto slot = std::uint32_t(command.number) % 150U;
    auto& record = owner.shared.prediction_records[slot];
    if (!enabled(owner)) {
        record.command_number = -1;
        return;
    }
    const auto& offsets = owner.bindings.command.offsets;
    record.command_number = command.number;
    record.field_04 =
        (record.field_04 & 0xFFFFFF00U) |
        std::uint32_t(owner.shared.command_tags[slot] == std::uint32_t(command.number));
    record.origin = field<std::array<float, 3>>(player, offsets.field_36375c23);
    record.velocity = field<std::array<float, 3>>(player, offsets.field_2a671b42);
    record.field_3309c0cf = field<float>(player, offsets.field_3309c0cf);
    record.field_fb7e3404 = field<float>(player, offsets.animation.field_fb7e3404);
    record.flags = field<std::uint32_t>(player, offsets.field_e2fbe695);
    const auto byte = field<std::uint8_t>(player, offsets.field_ac7f2913);
    write(&record, 0x24, std::uint32_t(byte));
    record.field_7090d8dd = field<std::uint8_t>(player, offsets.field_7090d8dd);
    record.field_cacaeecc = field<std::uint8_t>(player, offsets.field_cacaeecc);
    record.field_a15c3105 = field<std::uint8_t>(player, offsets.field_a15c3105);
    record.field_ae838771 = field<float>(player, offsets.field_ae838771);
    record.field_d4248210 = field<std::uint32_t>(player, offsets.field_d4248210);
    record.field_a3abbb09 = field<std::uint32_t>(player, offsets.field_a3abbb09);
    record.field_7501cb91 = field<std::uint8_t>(player, offsets.field_7501cb91);
}
void clone_previous_prediction_snapshot(PredictionHistoryOwner& owner,
                                        std::int32_t number) noexcept {
    copy_prediction_snapshot(owner, previous(number), number);
}
void finish_prediction_data_update(PredictionHistoryOwner& owner, void* player,
                                   std::int32_t update) {
    if (!update || field<std::uint8_t>(player, owner.bindings.rejection_byte) || !enabled(owner))
        return;
    const auto index = engine::invoke<std::uint32_t>(static_cast<std::byte*>(player) + 8, 10);
    auto& cache = owner.bindings.command.players.get(index)->native;
    bool changed = false;
    for (std::size_t word = 0; word < 91; ++word) {
        if (field<std::uint32_t>(&cache, 0x2FC + word * 4) !=
            field<std::uint32_t>(&cache, 0x468 + word * 4)) {
            changed = true;
            break;
        }
    }
    if (!changed)
        return;
    const auto number = *owner.shared.last_processed_command;
    const auto& snapshot = owner.shared.snapshots[std::uint32_t(number) % 150U];
    if (snapshot.command_number != number)
        return;
    for (std::size_t layer = 0; layer < 13; ++layer) {
        const auto source = 0x18 + layer * 0x1C;
        const auto destination = 0x468 + layer * 0x1C;
        const auto weight = field<float>(&snapshot, source + 12);
        const auto cycle = field<float>(&snapshot, source + 24);
        const float quantized_weight = quantize_cached_layer(weight, 1.0f, 255.0f);
        const float quantized_cycle = quantize_cached_layer(cycle, 0.999969482421875f, 32768.0f);
        if (field<std::uint32_t>(&snapshot, source + 4) !=
                field<std::uint32_t>(&cache, destination + 4) ||
            field<float>(&snapshot, source + 20) != field<float>(&cache, destination + 20) ||
            field<float>(&cache, destination + 12) != quantized_weight ||
            field<float>(&cache, destination + 24) != quantized_cycle)
            continue;
        write(&cache, destination + 12, field<std::uint32_t>(&snapshot, source + 12));
        write(&cache, destination + 24, field<std::uint32_t>(&snapshot, source + 24));
    }
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: prediction_history_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
std::uint32_t pred_hist_field(ClientRuntimeStorage& storage, std::uint32_t salt) {
    const auto& offsets = storage.offsets().globals;
    for (const auto& row : engine::build_receive_offset_recipes())
        if (row.initial_xor_key == salt)
            return std::uint32_t(std::ptrdiff_t(engine::DecodedOffset(
                offsets, row.pointer_identity, row.xor_key_identity, salt, 0)));
    for (const auto& row : engine::build_data_map_offset_recipes())
        if (row.initial_xor_key == salt)
            return std::uint32_t(std::ptrdiff_t(engine::DecodedOffset(
                offsets, row.pointer_identity, row.xor_key_identity, salt, 0)));
    if (salt == 0xF58E6118u)
        return std::uint32_t(
            std::ptrdiff_t(engine::DecodedOffset(offsets, 0x43468FD0u, 0x43467C70u, salt, 0)));
    throw std::invalid_argument(
        "Prediction history field has no recovered canonical offset recipe");
}
engine::OwnerRegistryBinding registry(ClientRuntimeStorage& storage) {
    return {storage.owner_registries().at(0x43475B78u), storage.memory()};
}
ClientRuntimeStorage& checked(ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
                              ConfigurationRuntimeOwner& config, LocalAnimationRuntimeOwner& local,
                              HistoryRuntimeOwner& history, MovementHudRuntimeOwner& movement,
                              animation::RuntimeOwner& animation,
                              CommandCaptureRuntimeOwner& capture) {
    auto& data = storage.data();
    const auto& command = local.restricted();
    const auto& entity = history.entity_runtime();
    if (&engine.variables.data() != &data || &config.storage() != &storage ||
        &local.storage() != &storage || &history.storage() != &storage ||
        &local.history() != &history || &movement.storage() != &storage ||
        &animation.storage() != &storage || &capture.storage() != &storage ||
        &animation.players() != &history.players().cache() ||
        &command.players != &history.players().cache() || &command.helpers != &local.helpers() ||
        command.enabled_43467224 != &config.rage_settings().global.enabled.value ||
        command.enabled_4346a740 != &config.anti_aim_global().enabled.value ||
        &command.history_owner != &data.native_cell<EntityCommandOwner*>(0x43475898u) ||
        &capture.globals().engine_client != &data.native_cell<void*>(0x4346A514u) ||
        command.offsets.animation.animation_state != entity.offsets.animation_state ||
        command.offsets.animation.pose_parameters != entity.offsets.pose_parameters ||
        entity.offsets.animation_state !=
            std::uint32_t(animation.bones().fields.animation_state_3d52a1a7))
        throw std::invalid_argument(
            "Prediction history requires the canonical local-animation, history, movement, capture and bone owners");
    const auto core = engine.interfaces.core();
    auto* const globals = data.native_cell<std::byte*>(0x43467B04u);
    if (!globals || !core.engine_client || !core.entity_list ||
        core.engine_client != data.native_cell<void*>(0x4346A514u) ||
        core.entity_list != data.native_cell<void*>(0x4346A890u) ||
        !data.native_cell<void*>(0x4346A894u) ||
        data.native_cell<void*>(0x434678F8u) != globals + 4 ||
        data.native_cell<void*>(0x43467EB8u) != globals + 0x10 ||
        data.native_cell<void*>(0x4346A338u) != globals + 0x20 ||
        !data.native_cell<void*>(0x4346A334u) || !data.native_cell<void*>(0x43475B2Cu) ||
        !data.native_cell<void*>(0x43475BA4u) ||
        command.interval != data.native_cell<const float*>(0x4346A338u) ||
        command.current_time != data.native_cell<float*>(0x43467EB8u) ||
        animation.bones().entity_list != core.entity_list || entity.entity_list != core.entity_list)
        throw std::invalid_argument(
            "Prediction history requires its actual published engine and live clock/command aliases");
    return storage;
}
animation::LocalPredictionPoseOffsets pose_fields(const RestrictedPlayerCommandBindings& command) {
    const auto& o = command.offsets;
    return {std::uint32_t(o.field_36375c23), std::uint32_t(o.field_3309c0cf),
            std::uint32_t(o.field_e2fbe695), std::uint32_t(o.field_f58e6118),
            std::uint32_t(o.field_4e66bb34), std::uint32_t(o.field_2a671b42),
            o.animation.field_a2a8247f};
}
}
struct PredictionHistoryRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    PredictionHistoryStorage history;
    animation::LocalPredictionPoseBindings pose;
    PredictionHistoryBindings bindings;
    PredictionHistoryOwner command;
    PredictionHistoryRegistries registries;
    PredictionHistoryInitialization factory;
    Data(ClientRuntimeStorage& s, const EngineRuntimeBindings& engine,
         ConfigurationRuntimeOwner& config, LocalAnimationRuntimeOwner& local,
         HistoryRuntimeOwner& remote_history, MovementHudRuntimeOwner& movement,
         animation::RuntimeOwner& animation, CommandCaptureRuntimeOwner& capture)
        : storage(checked(s, engine, config, local, remote_history, movement, animation, capture)),
          history(deferred_prediction_history_storage),
          pose{remote_history.entity_runtime(),
               pose_fields(local.restricted()),
               s.data().native_cell<lagcomp::TickTimingSamples>(0x43478C70u),
               movement.movement_capture(),
               movement.movement_environment(),
               movement.movement_command().bindings.command.collision,
               animation::LocalPredictionCommandRing(s.data().native_cell<void*>(0x4346A894u)),
               s.data().native_cell<void*>(0x434693F0u),
               s.data().native_cell<const std::int32_t*>(0x434678F8u),
               s.data().native_cell<float*>(0x43467EB8u),
               s.data().native_cell<const float*>(0x4346A338u),
               s.data().native_cell<std::uint8_t*>(0x43467B24u),
               s.data().native_cell<std::uint8_t>(0x43475BD3u),
               s.data().native_cell<void*>(0x43467204u),
               animation.bones(),
               s.data().native_cell<lagcomp::SetAbsoluteVector>(0x434688C8u),
               {s.data().native_cell<float>(0x4347C640u),
                s.data().native_cell<HitboxMatrices>(0x4347C650u),
                s.data().native_cell<RageVector>(0x4347C9E0u)},
               s.data().native_cell<const float*>(0x4346A334u)},
          bindings{local.restricted(),
                   pose,
                   s.data().native_cell<const std::int32_t*>(0x43475B2Cu),
                   std::ptrdiff_t(pred_hist_field(s, 0x9D920E35u)),
                   std::ptrdiff_t(pred_hist_field(s, 0xC865F10Eu)),
                   animation.bones().fields.angles_3b1f6ecc,
                   s.data().native_cell<lagcomp::SetAbsoluteVector>(0x434688C8u),
                   s.data().native_cell<engine::MemberFunction<void, std::int32_t>>(0x43468C10u)},
          command{history.shared(), history.redirect_from_44, history.redirect_to_48, bindings},
          registries{registry(s),           s.command_phases(),
                     s.empty_ticks(),       s.prediction_observers(),
                     s.entity_observers(),  s.data_update_observers(),
                     s.recharge_observers()},
          factory{registries,
                  s.configuration(),
                  {history, command, s.data().native_cell<const std::int32_t*>(0x43475BA4u),
                   s.data().native_cell<void*>(0x43467204u),
                   s.data().native_cell<PredictionHistoryOwner*>(0x434759B4u)}} {}
};
PredictionHistoryRuntimeOwner::PredictionHistoryRuntimeOwner(
    ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
    ConfigurationRuntimeOwner& config, LocalAnimationRuntimeOwner& local,
    HistoryRuntimeOwner& history, MovementHudRuntimeOwner& movement,
    animation::RuntimeOwner& animation, CommandCaptureRuntimeOwner& capture)
    : data_(std::make_unique<Data>(storage, engine, config, local, history, movement, animation,
                                   capture)) {}
PredictionHistoryRuntimeOwner::~PredictionHistoryRuntimeOwner() = default;
ClientRuntimeStorage& PredictionHistoryRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
PredictionHistoryStorage& PredictionHistoryRuntimeOwner::history_storage() noexcept {
    return data_->history;
}
PredictionHistoryOwner& PredictionHistoryRuntimeOwner::command() noexcept {
    return data_->command;
}
const animation::LocalPredictionPoseBindings& PredictionHistoryRuntimeOwner::pose() const noexcept {
    return data_->pose;
}
PredictionHistoryInitialization& PredictionHistoryRuntimeOwner::factory_bindings() noexcept {
    return data_->factory;
}
void PredictionHistoryRuntimeOwner::bind_initializers(NativeStartupSources& sources) {
    if (&sources.data() != &storage().data())
        throw std::invalid_argument(
            "Prediction history factory requires its canonical source scheduler");
    bind_prediction_history_initializer(sources, data_->factory);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: rejected_player_update.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */
/* deduplicated helper */
std::int32_t integer(float value) noexcept {
    return _mm_cvttss_si32(_mm_set_ss(value));
}
/* deduplicated absolute helper */
std::int32_t signed_bits(std::uint32_t value) noexcept {
    return std::bit_cast<std::int32_t>(value);
}
std::int32_t convar_integer(const void* object) noexcept {
    for (;;) {
        const auto parent = field<const void*>(object, 0x1c);
        if (parent == object)
            break;
        object = parent;
    }
    return signed_bits(field<std::uint32_t>(object, 0x30) ^
                       std::uint32_t(reinterpret_cast<std::uintptr_t>(object)));
}
std::uint32_t timestamp_mix() noexcept {
    const auto value = static_cast<std::uint32_t>(__rdtsc());
    return std::rotr(value, 3) ^ value;
}
std::uint32_t coordinate(float value) noexcept {
    const auto whole = integer(absolute(value));
    const auto scaled = integer(value * 32.0f);
    const auto magnitude = scaled < 0 ? 0u - std::uint32_t(scaled) : std::uint32_t(scaled);
    auto fraction = magnitude & 31u;
    std::uint32_t result = whole != 0 ? 1u : 0u;
    if (fraction)
        result |= 2u;
    if (-0.03125f >= value)
        result |= 4u;
    if (whole != 0)
        result |= ((std::uint32_t(whole) - 1u) & 0x3fffu) << 3;
    else
        result |= (timestamp_mix() * 0x8208u) & 0x1fff8u;
    if (!fraction)
        fraction = timestamp_mix();
    return result | (fraction << 17);
}

// Five chained 16-bit pairs start at packet+2. The fifth pair's right word
// aliases the saved player index at packet+20 in the native stack frame.
// Only the 20 wire bytes are published; the scan cursor remains an entity index.
void mix_packet(std::array<std::byte, 20>& packet, std::int32_t selected,
                std::uint32_t key) noexcept {
    std::array<std::uint16_t, 11> words{};
    std::memcpy(words.data(), packet.data(), packet.size());
    words[10] = static_cast<std::uint16_t>(selected);
    const auto rotated = std::rotr(key, 1);
    std::uint16_t previous_left = 0, previous_right = 0;
    for (std::size_t index = 1; index < 10; index += 2) {
        auto left = static_cast<std::uint16_t>((words[index] ^ previous_left) + key);
        auto right = static_cast<std::uint16_t>((words[index + 1] ^ previous_right) + rotated);
        auto round_key = std::rotr(rotated, 1);
        for (std::uint32_t round = 0; round < 15; ++round) {
            left = static_cast<std::uint16_t>(
                std::rotl(static_cast<std::uint16_t>(right ^ left), right & 15u) + round_key);
            round_key = std::rotr(round_key, 1);
            right = static_cast<std::uint16_t>(
                std::rotl(static_cast<std::uint16_t>(right ^ left), left & 15u) + round_key);
            round_key = std::rotr(round_key, 1);
        }
        words[index] = left;
        words[index + 1] = right;
        previous_left = left;
        previous_right = right;
    }
    std::memcpy(packet.data(), words.data(), packet.size());
}

RageVector shared_origin(PlayerState& state, const void* player,
                         const RejectedPlayerUpdateContext& context) noexcept {
    if (state.history.count) {
        const auto tick =
            lagcomp::time_to_tick(field<float>(&state.native, 0x110), context.tick_interval);
        const auto previous =
            signed_bits(std::uint32_t(field<std::int32_t>(&state.native, 0x168)) - 1u);
        const auto* newest = state.history.records[state.history.first & 31u];
        if (tick >= previous || tick >= newest->header.simulation_tick)
            return newest->header.origin;
    }
    return field<RageVector>(player, context.offsets.origin_a9ee6062);
}
std::uint32_t shared_yaw(PlayerState& state, const RejectedPlayerUpdateContext& context) noexcept {
    std::uint32_t result = 0x1ffu;
    if (field<std::int32_t>(&state.native, 0xc) == -1 || (state.native.flags & 0x80u))
        return result;
    state.native.flags |= 0x80u;
    float rule_time = 0.0f;
    if (context.game_rules && *context.game_rules)
        rule_time = field<float>(*context.game_rules, context.offsets.rule_time_9f8e7f71);
    if (field<float>(&state.native, 4) == rule_time) {
        const auto encoded =
            std::uint32_t(integer(field<float>(&state.native, 8) * 182.04444885253906f)) & 0xffffu;
        result = std::uint32_t(integer(static_cast<float>(encoded) * 0.0054931640625f));
    }
    return result;
}

void publish(RejectedPlayerUpdateOwner& owner, void* selected, std::int32_t index,
             const RejectedPlayerUpdateContext& context) {
    enter_shared_esp_update(context);
    auto& cache = *context.cache.get(std::uint32_t(index));
    const auto origin = shared_origin(cache, selected, context);
    const auto yaw = shared_yaw(cache, context);
    const auto z = coordinate(origin[2]);
    const auto y = coordinate(origin[1]);
    const auto x = coordinate(origin[0]);
    const auto clock = engine::invoke<float>(owner.engine_client, 14);
    const auto clock_interval = context.engine_tick_interval;
    const auto random = timestamp_mix() * 0x1041u;
    std::array<std::uint32_t, 5> words{};
    words[0] = (random & 0xffffu) | (std::uint32_t(owner.marker) << 16);
    words[1] = std::uint32_t(lagcomp::receive_time_to_tick(clock, clock_interval));
    words[2] = (((x << 9) | (yaw & 0x1ffu)) << 7) | ((std::uint32_t(index) - 1u) & 0x7fu);
    words[3] = (((z << 22) | (y & 0x3fffffu)) << 6) | ((x >> 16) & 0x3fu);
    words[4] = (z >> 4) & 0x3ffffu;
    const auto local_index = engine::invoke<std::int32_t>(owner.engine_client, 12);
    misc::PlayerInfo info;
    if (engine::invoke<bool>(owner.engine_client, 8, local_index, &info)) {
        words[0] ^= field<std::uint32_t>(info.bytes.data(), 8);
        words[1] ^= field<std::uint32_t>(info.bytes.data(), 0x90);
        words[2] ^= std::uint32_t(local_index);
    }
    std::array<std::byte, 20> packet{};
    std::memcpy(packet.data(), words.data(), packet.size());
    mix_packet(packet, index, owner.mixing_key);
    transform_shared_esp_packet(context.packet_cipher, owner, packet);
    std::memcpy(words.data(), packet.data(), packet.size());
    auto* channel = engine::invoke<void*>(owner.engine_client, 78);
    if (channel) {
        owner.message.fill(std::byte{});
        auto* message = owner.message.data();
        owner.construct_message(message);
        write<std::uint32_t>(message, 0x20, 0);
        write<std::uint32_t>(message, 0x34, field<std::uint32_t>(message, 0x34) | 0x3eu);
        write(message, 0x18, words[0]);
        write(message, 0x1c, words[1]);
        write(message, 0x28, words[2]);
        write(message, 0x24, words[3]);
        write(message, 0x2c, words[4]);
        engine::invoke<bool>(channel, 40, message, false, false);
        owner.destroy_message(message);
    }
    leave_shared_esp_update(context);
    owner.last_player = index;
    owner.last_update = context.current_time;
}
} // namespace

void transform_shared_esp_packet(SharedEspPacketCipher cipher,
                                 const RejectedPlayerUpdateOwner& owner,
                                 std::span<std::byte> packet) noexcept {
    if (cipher == SharedEspPacketCipher::legacy_xor) {
        auto seed = field<std::uint32_t>(packet.data(), 0);
        for (std::size_t index = 4; index < 20; ++index) {
            seed = seed * 0x41c64e6du + 0x3039u;
            packet[index] ^= std::byte(static_cast<std::uint8_t>(seed >> 16));
        }
        return;
    }
    std::array<std::uint8_t, 256> state{};
    for (std::size_t index = 0; index < 256; ++index)
        state[index] = static_cast<std::uint8_t>(index);
    std::uint8_t j = 0;
    for (std::size_t index = 0; index < 256; ++index) {
        j = static_cast<std::uint8_t>(j + state[index] + owner.packet_key[index & 15]);
        std::swap(state[index], state[j]);
    }
    std::uint8_t i = 0;
    j = 0;
    for (std::uint32_t skipped = 0; skipped < 128; ++skipped) {
        ++i;
        j = static_cast<std::uint8_t>(j + state[i]);
        std::swap(state[i], state[j]);
    }
    for (auto& byte : packet) {
        ++i;
        j = static_cast<std::uint8_t>(j + state[i]);
        std::swap(state[i], state[j]);
        byte ^= std::byte(state[static_cast<std::uint8_t>(state[i] + state[j])]);
    }
}

void update_rejected_player(void* player, const RejectedPlayerUpdateContext& context) {
    auto* owner = context.owner;
    if (!owner || !context.settings.enabled.value ||
        !hotkey_active(context.settings.activation.value.binding, context.input))
        return;
    if (0.0625f > absolute(context.current_time - owner->last_update))
        return;
    if (convar_integer(owner->game_type) == 6)
        return;
    if (!owner->construct_message || !owner->destroy_message || !owner->construct_receiver ||
        !owner->destroy_receiver)
        return;
    if (!convar_integer(owner->send_gate_a) && !convar_integer(owner->send_gate_b))
        return;
    if (!engine::invoke<void*>(owner->engine_client, 78) ||
        engine::invoke<bool>(owner->engine_client, 197) ||
        engine::invoke<bool>(owner->engine_client, 224) || *context.blocked)
        return;
    if (context.game_rules && *context.game_rules &&
        field<std::uint8_t>(*context.game_rules, context.offsets.rule_d720aea6))
        return;
    auto index = signed_bits(std::uint32_t(owner->last_player) + 1u);
    for (std::int32_t iteration = 1; iteration <= context.maximum_clients; ++iteration) {
        if (index > context.maximum_clients)
            index = 1;
        auto* candidate = engine::invoke<void*>(context.entity_list, 3, index);
        if (candidate && engine::invoke<bool>(candidate, 158) &&
            !field<std::uint8_t>(candidate, context.offsets.rejected_d070684c) &&
            !engine::invoke<bool>(static_cast<std::byte*>(candidate) + 8, 9) &&
            !field<std::uint8_t>(candidate, context.offsets.rejected_9d920e35) &&
            engine::invoke<std::int32_t>(candidate, 122) > 0 &&
            !context.players.records[std::size_t(index)].adjustments.whitelist &&
            (context.settings.other_team.value ||
             rage::target_is_enemy(context.targeting.capture(player), candidate))) {
            publish(*owner, candidate, index, context);
            return;
        }
        index = signed_bits(std::uint32_t(index) + 1u);
    }
}

void enter_shared_esp_update(const RejectedPlayerUpdateContext& context) noexcept {
    context.packet_cipher = SharedEspPacketCipher::rc4_drop128;
}
void leave_shared_esp_update(const RejectedPlayerUpdateContext& context) noexcept {
    context.packet_cipher = SharedEspPacketCipher::legacy_xor;
}

} // namespace skeet::client

// ============================================================================
// Subcomponent: restricted_player_command.cpp
// ============================================================================

namespace skeet::client {
using restricted_detail::update_animation_step;
namespace {
std::int32_t increment(std::int32_t value, int amount) noexcept {
    return std::bit_cast<std::int32_t>(std::uint32_t(value) + std::uint32_t(amount));
}
float recoil(void* player, const RestrictedPlayerCommandBindings& b) {
    const auto bits = lagcomp::read_prediction_convar(b.recoil_scale_convar);
    return std::bit_cast<float>(bits) * read<float>(player, b.offsets.field_1bae1168);
}
void* weapon(void* player, const RestrictedPlayerCommandBindings& b) {
    const auto handle = read<std::uint32_t>(player, b.offsets.weapon_handle);
    return handle == UINT32_MAX ? nullptr : engine::invoke<void*>(b.entity_list, 4, handle);
}
void transfer_prediction(void* player, const RestrictedCommandPrediction0x4C& r,
                         const RestrictedPlayerCommandBindings& b, bool restore_time = true) {
    const auto& o = b.offsets;
    if (restore_time)
        *b.globals_current_time = float(r.tick_base) * *b.interval;
    write(player, o.field_7a2d60c1, r.origin);
    write(player, o.field_36375c23, r.origin);
    write(player, o.field_4e66bb34, r.velocity);
    write(player, o.field_2a671b42, r.velocity);
    write(player, o.field_a9ee6062, r.velocity);
    write(player, o.field_3309c0cf, r.field_3309c0cf);
    write(player, o.animation.field_fb7e3404, r.field_fb7e3404);
    write(player, o.field_e2fbe695, r.flags);
    write(player, o.field_ac7f2913, r.field_24);
    write(player, o.field_7090d8dd, r.field_7090d8dd);
    write(player, o.field_cacaeecc, r.field_cacaeecc);
    write(player, o.field_a15c3105, r.field_a15c3105);
    write(player, o.field_ae838771, r.field_ae838771);
    write(player, o.field_d4248210, r.field_d4248210);
    write(player, o.field_a3abbb09, r.field_a3abbb09);
    write(player, o.field_7501cb91, r.field_7501cb91);
}
RestrictedCommandPrediction0x4C capture_prediction(void* player,
                                                   const RestrictedPlayerCommandBindings& b) {
    const auto& o = b.offsets;
    RestrictedCommandPrediction0x4C r{};
    r.tick_base = b.replay_restore_tick_scratch;
    r.origin = read<std::array<float, 3>>(player, o.field_36375c23);
    r.velocity = read<std::array<float, 3>>(player, o.field_2a671b42);
    r.field_3309c0cf = read<float>(player, o.field_3309c0cf);
    r.field_fb7e3404 = read<float>(player, o.animation.field_fb7e3404);
    r.flags = read<std::uint32_t>(player, o.field_e2fbe695);
    r.field_24 = read<std::uint8_t>(player, o.field_ac7f2913);
    r.field_7090d8dd = read<std::uint8_t>(player, o.field_7090d8dd);
    r.field_cacaeecc = read<std::uint8_t>(player, o.field_cacaeecc);
    r.field_a15c3105 = read<std::uint8_t>(player, o.field_a15c3105);
    r.field_ae838771 = read<float>(player, o.field_ae838771);
    r.field_d4248210 = read<std::uint32_t>(player, o.field_d4248210);
    r.field_a3abbb09 = read<std::uint32_t>(player, o.field_a3abbb09);
    r.field_7501cb91 = read<std::uint8_t>(player, o.field_7501cb91);
    return r;
}
bool can_replay(const RestrictedPlayerCommandOwner& owner, void* player, void* active_weapon,
                const Command& command, const RestrictedPlayerCommandBindings& b) {
    const auto first = increment(*owner.last_processed_command, 1);
    for (auto number = increment(command.number, -1); number >= first;
         number = increment(number, -1)) {
        const auto slot = std::uint32_t(number) % 150U;
        const auto& old = b.commands[slot];
        if (old.number != number || old.tick >= 0x7F7FFFFF)
            continue;
        if (owner.prediction_records[slot].command_number != number)
            return false;
        const auto& snapshot = owner.snapshots[slot];
        if (snapshot.command_number != number)
            continue;
        if (read<std::uint8_t>(&snapshot, 0x1F8) || read<void*>(&snapshot, 0x254) != player ||
            read<void*>(&snapshot, 0x258) != active_weapon)
            return false;
    }
    return true;
}
void replay_commands(RestrictedPlayerCommandOwner& owner, void* player, void* state,
                     const Command& current, PlayerState& cache, std::int32_t layer_count,
                     const RestrictedPlayerCommandBindings& b) {
    const auto& o = b.offsets;
    auto* descriptor = static_cast<std::byte*>(player) + o.animation.animation_layers;
    auto* layers = read<std::byte*>(descriptor, 0);
    if (layer_count >= 13) {
        for (std::ptrdiff_t i = 0; i < 13; ++i)
            for (std::ptrdiff_t word = 0; word < 7; ++word)
                write(layers, i * 0x38 + 0x14 + word * 4,
                      read<std::uint32_t>(&cache.native, 0x468 + i * 0x1C + word * 4));
    }
    const auto saved = capture_prediction(player, b);
    const auto saved_time = *b.current_time;
    auto restored_body_time = *b.body_yaw_time;
    const auto restored_body_yaw = read<float>(&cache.native, 0x118);
    bool restored = false;
    for (auto number = increment(*owner.last_processed_command, 1); number < current.number;
         number = increment(number, 1)) {
        const auto slot = std::uint32_t(number) % 150U;
        auto& command = b.commands[slot];
        if (command.tick == 0x7F7FFFFF)
            continue;
        auto& snapshot = owner.snapshots[slot];
        if (snapshot.command_number != number)
            continue;
        const auto& record = owner.prediction_records[slot];
        transfer_prediction(player, record, b);
        const auto* history =
            prediction::CommandHistoryView::from_owner(b.history_owner).find(number);
        const auto seed = increment(history ? read<std::int32_t>(history, 0) : -1, -1);
        if (!restored) {
            const auto previous_number = std::uint32_t(number) - 1U;
            const auto& previous = owner.snapshots[previous_number % 150U];
            if (std::uint32_t(previous.command_number) == previous_number)
                write(player, o.field_00a5bd68, read<std::uint8_t>(&previous, 4));
            std::memcpy(state, reinterpret_cast<const std::byte*>(&snapshot) + 0x1F4, 0x348);
            restored = true;
            if (read<std::int32_t>(descriptor, 0xC) > 7) {
                write(state, 0x19C, read<std::uint32_t>(layers, 0x1A0));
                write(state, 0x190, read<std::uint32_t>(layers, 0x1A8));
                write(state, 0x198, read<std::uint32_t>(layers, 0x1B4));
                write(state, 0x98, read<std::uint32_t>(layers, 0x17C));
            }
            const auto time = read<float>(&snapshot, 0x1F0);
            if (time != std::numeric_limits<float>::max())
                restored_body_time = time;
        } else {
            snapshot.command_number = number;
            write(&snapshot, 0x1F0, *b.body_yaw_time);
            std::memcpy(reinterpret_cast<std::byte*>(&snapshot) + 0x1F4, state, 0x348);
        }
        write(player, o.animation.field_a2a8247f, restored_body_yaw);
        *b.body_yaw = restored_body_yaw;
        const std::array<float, 3> angles{read<float>(&snapshot, 0x1E8),
                                          read<float>(&snapshot, 0x1EC), 0.0f};
        update_animation_step(owner, player, state, command, (record.field_04 & 0xFFU) != 0, seed,
                              layer_count, &angles, b);
        auto captured_animation = read<lagcomp::AnimationState>(&snapshot, 0x14);
        animation::capture_entity_animation(captured_animation, player, o.animation);
        write(&snapshot, 0x14, captured_animation);
        write(&snapshot, 0x1E4, read<std::uint32_t>(state, 0x80));
        write(&snapshot, 4, read<std::uint8_t>(player, o.field_00a5bd68));
    }
    // Native ESP+188 is uninitialized. In the canonical alias profile its
    // transient time write is overwritten below without an engine callback.
    // Elide that input only in this proven source profile. This does not
    // claim equivalence of the machine MXCSR exception-status flags.
    transfer_prediction(player, saved, b, !b.normalize_uninitialized_restore_time);
    *b.current_time = saved_time;
    *b.body_yaw_time = restored_body_time;
    *b.body_yaw = restored_body_yaw;
}
}
void process_restricted_player_command(RestrictedPlayerCommandOwner* owner, void* player,
                                       Command* command, std::uint8_t* send_packet,
                                       bool caller_flag, const RestrictedPlayerCommandBindings& b) {
    if (!player || !command || !send_packet || !owner)
        return;
    if (b.normalize_uninitialized_restore_time && b.current_time != b.globals_current_time)
        throw std::invalid_argument(
            "Restricted restore normalization requires the canonical current-time alias");
    const auto number = std::uint32_t(command->number), slot = number % 150U;
    const bool tagged = owner->command_tags[slot] == number;
    (void)tagged; //433F54D5 does not consume its fourth stack argument.
    auto& snapshot = owner->snapshots[slot];
    if (!*b.enabled_43467224 && !*b.enabled_4346a740) {
        snapshot.command_number = -1;
        return;
    }
    if (command->tick == 0x7F7FFFFF) {
        const auto previous_number = number - 1U;
        const auto& previous = owner->snapshots[previous_number % 150U];
        if (std::uint32_t(previous.command_number) == previous_number) {
            std::memcpy(&snapshot, &previous, sizeof snapshot);
            snapshot.command_number = command->number;
        }
        return;
    }
    const auto& o = b.offsets;
    const auto saved_recoil = read<std::uint32_t>(player, o.field_66146802);
    const auto saved_byte = read<std::uint8_t>(player, o.field_00a5bd68);
    const auto saved_movement = read<std::uint32_t>(player, o.field_52899df3);
    write(player, o.field_66146802, recoil(player, b));
    owner->prediction_records[slot].recoil_pitch = read<float>(player, o.field_66146802);
    auto angles = command->angles;
    animation::select_animation_angles(b.angle_history, command->number, angles);
    auto* state = read<void*>(player, o.animation.animation_state);
    write(&snapshot, 0x1F0, *b.body_yaw_time);
    if (state) {
        write(&snapshot, 0x1E8, angles[0]);
        write(&snapshot, 0x1EC, angles[1]);
        snapshot.command_number = command->number;
        std::memcpy(reinterpret_cast<std::byte*>(&snapshot) + 0x1F4, state, 0x348);
    }
    *engine::invoke<std::array<float, 3>*>(player, 170) = angles;
    if (state) {
        const auto index = engine::invoke<std::int32_t>(static_cast<std::byte*>(player) + 8, 10);
        auto& cache = *b.players.get(std::uint32_t(index));
        const auto layer_count = read<std::int32_t>(player, o.animation.animation_layers + 0xC);
        const auto saved_time = *b.current_time;
        const auto saved_command = read<std::uint32_t>(player, o.field_f58e6118);
        const auto saved_body = read<std::uint32_t>(player, o.animation.field_a2a8247f);
        if (caller_flag && can_replay(*owner, player, weapon(player, b), *command, b))
            replay_commands(*owner, player, state, *command, cache, layer_count, b);
        update_animation_step(*owner, player, state, *command, owner->command_tags[slot] == number,
                              read<std::int32_t>(player, o.tick_base), layer_count, nullptr, b);
        write(player, o.animation.field_0471e13c, std::uint32_t(0));
        write(player, o.animation.field_a2a8247f, saved_body);
        write(player, o.field_f58e6118, saved_command);
        *b.current_time = saved_time;
    }
    write(&snapshot, 4, read<std::uint8_t>(player, o.field_00a5bd68));
    if (state) {
        write(&snapshot, 8, read<std::uint32_t>(state, 0xEC));
        write(&snapshot, 0x1E4, read<std::uint32_t>(state, 0x80));
    }
    write(&snapshot, 0xC, *b.body_yaw_time);
    write(&snapshot, 0x10, *b.body_yaw);
    auto captured_animation = read<lagcomp::AnimationState>(&snapshot, 0x14);
    animation::capture_entity_animation(captured_animation, player, o.animation);
    write(&snapshot, 0x14, captured_animation);
    if (*send_packet) {
        auto sent = read<std::array<float, 3>>(player, o.field_f8205eba);
        for (auto& angle : sent)
            angle = normalize_movement_angle(angle);
        *b.sent_angles = sent;
    }
    write(player, o.field_52899df3, saved_movement);
    write(player, o.field_00a5bd68, saved_byte);
    write(player, o.field_66146802, saved_recoil);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: restricted_player_command_layers.cpp
// ============================================================================

namespace skeet::client::restricted_detail {
namespace {
float absolute(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) & 0x7FFFFFFFU);
}
float negated(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) ^ 0x80000000U);
}
float unit(float value) noexcept {
    return _mm_cvtss_f32(
        _mm_min_ss(_mm_max_ss(_mm_set_ss(value), _mm_setzero_ps()), _mm_set_ss(1.0f)));
}
float approach(float target, float value, float amount) noexcept {
    const auto difference = target - value;
    if (difference > amount)
        return value + amount;
    if (negated(amount) > difference)
        return value - amount;
    return target;
}
float angle_difference(float first, float second) noexcept {
    auto difference = float(std::fmod(double(first - second), 360.0));
    if (first > second) {
        if (difference >= 180.0f)
            difference -= 360.0f;
    } else if (difference <= -180.0f)
        difference += 360.0f;
    return difference;
}
float quantized_angle(float value) noexcept {
    const auto integer =
        _mm_cvttss_si32(_mm_mul_ss(_mm_set_ss(value), _mm_set_ss(182.04444885253906f)));
    return float(std::uint32_t(integer) & 0xFFFFU) * 0.0054931640625f;
}
float approach_angle(float target, float value, float amount) noexcept {
    target = quantized_angle(target);
    value = quantized_angle(value);
    const auto difference = normalize_movement_angle(target - value);
    if (difference > amount)
        return value + amount;
    if (difference < negated(amount))
        return value - amount;
    return target;
}
void advance_cycle(void* layer, float delta) noexcept {
    const auto rate = read<float>(layer, 0x28);
    if (absolute(rate) <= 0.0f)
        return;
    auto cycle = rate * delta + read<float>(layer, 0x2C);
    if (cycle >= 1.0f)
        cycle = 0.9990000128746033f;
    const auto integer = _mm_cvttss_si32(_mm_set_ss(cycle));
    cycle -= float(integer);
    if (cycle < 0.0f)
        cycle += 1.0f;
    else if (cycle > 1.0f)
        cycle -= 1.0f;
    write(layer, 0x2C, cycle);
}
void set_sequence(void* player, void* layer, std::int32_t sequence) {
    write(layer, 0x2C, std::uint32_t(0));
    write(layer, 0x20, std::uint32_t(0));
    write(layer, 0x18, sequence);
    write(layer, 0x28, engine::invoke<float>(player, 223, layer, sequence));
}
void jump_layer(RestrictedPlayerCommandOwner& owner, void* player, void* state, std::byte* layers,
                std::int32_t count, const animation::LocalAnimationBindings& helpers) {
    animation::mark_animation_modifiers(state);
    write(state, 0x10B, std::uint8_t(1));
    if (count <= 4)
        return;
    auto sequence = animation::select_symbol_weighted_sequence(player, 4, 0x3D9, helpers);
    if (sequence <= 1) {
        std::array<char, 64> text;
        ::strcpy_s(text.data(), text.size(), owner.jump_activity);
        const auto modifiers = read<std::uint8_t>(state, 0x32B);
        if (modifiers & 1U)
            ::strcat_s(text.data(), text.size(), owner.moving_modifier);
        if (modifiers & 2U)
            ::strcat_s(text.data(), text.size(), owner.crouching_modifier);
        sequence = animation::map_animation_activity(player, text.data(), helpers);
    }
    if (sequence > 1 && std::uintptr_t(layers) + 4U * 0x38U)
        set_sequence(player, layers + 4 * 0x38, sequence);
}
std::int32_t observer_tick(const RestrictedPlayerCommandBindings& b, const Command& command,
                           std::int32_t tick) {
    return b.tickbase_observers->adjust_tick(command.number, tick);
}
void* active_weapon(void* player, const RestrictedPlayerCommandBindings& b) {
    const auto handle = read<std::uint32_t>(player, b.offsets.weapon_handle);
    return handle == UINT32_MAX ? nullptr : engine::invoke<void*>(b.entity_list, 4, handle);
}
void prepare_idle_layer(RestrictedPlayerCommandOwner& owner, void* player, void* state,
                        Command& command, std::byte* layers,
                        const RestrictedPlayerCommandBindings& b) {
    auto* layer = layers + 3 * 0x38;
    const auto& h = b.helpers;
    const auto velocity = read<std::array<float, 3>>(player, b.offsets.field_36375c23);
    const auto speed =
        minimum_number(std::sqrt(velocity[0] * velocity[0] + velocity[1] * velocity[1]), 260.0f);
    const auto delta = maximum_number(0.0f, *b.globals_current_time - read<float>(state, 0x6C));
    float maximum_speed = 260.0f;
    if (auto* weapon = active_weapon(player, b)) {
        maximum_speed = 0.0010000000474974513f;
        if (engine::invoke<float>(weapon, 442) > maximum_speed)
            maximum_speed = engine::invoke<float>(weapon, 442);
    }
    const auto walk_speed = speed / (maximum_speed * 0.5199999809265137f);
    const auto duck_speed = speed / (maximum_speed * 0.3400000035762787f);
    const auto transition = read<float>(state, 0x11C);
    const auto duck_target =
        unit(read<float>(player, b.offsets.field_3309c0cf) + read<float>(state, 0xA8));
    const auto duck = unit(approach(duck_target, read<float>(state, 0xA4), delta * 6.0f));
    const bool still = !(speed > 0.0f) && 0.0f >= read<float>(state, 0x104);
    if (!read<std::uint8_t>(state, 0x138) && still && read<std::uint8_t>(state, 0x108) &&
        !read<std::uint8_t>(state, 0x128) && !read<std::uint8_t>(state, 0x109) &&
        50.0f > read<float>(state, 0x2AC)) {
        std::array<char, 64> text;
        ::strcpy_s(text.data(), text.size(), owner.idle_activity);
        if (read<std::uint8_t>(state, 0x32B) & 2U)
            ::strcat_s(text.data(), text.size(), owner.layer_modifier);
        ::strcat_s(text.data(), text.size(), owner.idle_modifier);
        const auto sequence = animation::map_animation_activity(player, text.data(), h);
        if (sequence > 1) {
            if (std::uintptr_t(layer))
                set_sequence(player, layer, sequence);
            write(state, 0x138, std::uint8_t(1));
        }
    }
    if (h.sequence_activity) {
        const auto activity = h.sequence_activity(player, read<std::int32_t>(layer, 0x18));
        if (activity == 0x3D4 || activity == 0x3D3) {
            if (read<std::uint8_t>(state, 0x138) && duck_speed <= 0.25f) {
                if (!(delta <= 0.0f)) {
                    advance_cycle(layer, delta);
                    if (h.layer_weight_entry) {
                        const auto previous = read<float>(layer, 0x20);
                        const auto weight = unit(animation::read_local_layer_weight(state, 3, h));
                        write(layer, 0x20, weight);
                        write(layer, 0x24, (weight - previous) / delta);
                    }
                }
                write(state, 0x138,
                      std::uint8_t(read<float>(layer, 0x28) * delta + read<float>(layer, 0x2C) <
                                   0.9990000128746033f));
            } else {
                write(state, 0x138, std::uint8_t(0));
                const auto previous = read<float>(layer, 0x20);
                const auto weight = unit(approach(0.0f, previous, delta * 5.0f));
                write(layer, 0x20, weight);
                if (delta != 0.0f)
                    write(layer, 0x24, (weight - previous) / delta);
            }
        }
    }
    if (!(read<float>(state, 0xEC) <= 1.0f) || !read<std::uint8_t>(state, 0x108) ||
        read<std::uint8_t>(state, 0x128) || read<std::uint8_t>(state, 0x109) || !(delta > 0.0f))
        return;
    const auto previous_goal = read<float>(state, 0x80);
    auto goal = _mm_cvtss_f32(
        _mm_min_ss(_mm_max_ss(_mm_set_ss(previous_goal), _mm_set_ss(-360.0f)), _mm_set_ss(360.0f)));
    const auto eye = normalize_movement_angle(command.angles[1]);
    const auto difference = angle_difference(eye, goal);
    auto modifier =
        ((0.800000011920929f - transition * 0.30000001192092896f) - 1.0f) * unit(walk_speed) + 1.0f;
    if (duck > 0.0f)
        modifier = unit(duck_speed) * duck * (0.5f - modifier) + modifier;
    const auto maximum = read<float>(state, 0x338) * modifier;
    const auto minimum = read<float>(state, 0x334) * modifier;
    if (difference > maximum)
        goal = eye - absolute(maximum);
    else if (difference < minimum)
        goal = absolute(minimum) + eye;
    goal = normalize_movement_angle(goal);
    if (speed > 0.1f || absolute(velocity[2]) > 100.0f)
        goal = approach_angle(eye, goal, absolute((transition * 20.0f + 30.0f) * delta));
    else
        goal = approach_angle(*b.body_yaw, goal, absolute(delta * 100.0f));
    if (absolute(angle_difference(previous_goal, goal)) / delta > 120.0f) {
        const auto sequence =
            animation::select_prediction_sequence(player, 0x3D3, command.random_seed, h);
        if (sequence > 1) {
            write(layer, 0x18, sequence);
            const auto rate = engine::invoke<float>(player, 223, layer, sequence);
            write(layer, 0x2C, std::uint32_t(0));
            write(layer, 0x20, std::uint32_t(0));
            write(layer, 0x28, rate);
            write(state, 0x138, std::uint8_t(1));
        }
    }
}
void prepare_landing_tag(RestrictedPlayerCommandOwner& owner, void* player, void* state,
                         std::byte* layers, const RestrictedPlayerCommandBindings& b) {
    auto* layer = layers + 10 * 0x38;
    const auto command_address = read<std::uint32_t>(player, b.offsets.field_f58e6118);
    if (command_address && b.helpers.sequence_activity) {
        const auto* command = reinterpret_cast<const Command*>(std::uintptr_t(command_address));
        const auto number = std::uint32_t(command->number);
        if (owner.landing_tags[number % 150U] == number) {
            const auto sequence = read<std::int32_t>(layer, 0x18);
            const auto activity = b.helpers.sequence_activity(player, sequence);
            if (activity == 0x3D7 || activity == 0x3D6) {
                write(layer, 0x2C, std::uint32_t(0));
                write(layer, 0x20, std::uint32_t(0));
                write(layer, 0x28, engine::invoke<float>(player, 223, layer, sequence));
            }
        }
    }
    if (read<std::int32_t>(layer, 0x18) <= 0)
        write(layer, 0x20, std::uint32_t(0));
    else if (b.helpers.layer_weight_entry)
        write(layer, 0x20, unit(animation::read_local_layer_weight(state, 10, b.helpers)));
}
void update_post_layers(RestrictedPlayerCommandOwner& owner, void* player, void* state,
                        std::byte* layers, std::int32_t count, bool replay,
                        const RestrictedPlayerCommandBindings& b) {
    if (count > 2) {
        auto* first = layers + 0x38;
        auto* second = layers + 2 * 0x38;
        const auto duck = read<float>(state, 0xA4);
        const auto delta =
            replay ? read<float>(state, 0x74) : *b.current_time - read<float>(state, 0x6C);
        if (read<std::int32_t>(first, 0x18) <= 0)
            write(first, 0x20, std::uint32_t(0));
        if (read<std::int32_t>(second, 0x18) <= 0)
            write(second, 0x20, std::uint32_t(0));
        const auto first_weight = read<float>(first, 0x20);
        float weight;
        if (first_weight > 0.0f) {
            const char* name = "";
            if (b.helpers.sequence_name_entry) {
                const auto function = b.helpers.sequence_name_entry;
                name = function(player, read<std::int32_t>(first, 0x18));
            }
            if (name && std::strstr(name, owner.layer_modifier))
                weight = duck < 1.0f ? (1.0f - duck) * first_weight : 0.0f;
            else
                weight = duck > 0.0f ? duck * first_weight : 0.0f;
        } else {
            const auto previous = read<float>(second, 0x20);
            weight = previous > 0.0f ? approach(0.0f, previous, delta * 4.0f) : 0.0f;
        }
        write(second, 0x20, unit(weight));
        if (!(delta <= 0.0f)) {
            advance_cycle(first, delta);
            if (b.helpers.layer_weight_entry) {
                const auto previous = read<float>(first, 0x20);
                const auto current = unit(animation::read_local_layer_weight(state, 1, b.helpers));
                write(first, 0x20, current);
                write(first, 0x24, (current - previous) / delta);
            }
        }
    }
    if (count > 11) {
        auto* layer = layers + 11 * 0x38;
        if (read<std::int32_t>(layer, 0x18) <= 0)
            write(layer, 0x20, std::uint32_t(0));
        else if (read<float>(layer, 0x28) * read<float>(state, 0x74) + read<float>(layer, 0x2C) <
                 0.9990000128746033f)
            write(layer, 0x20,
                  unit(1.0f - unit((read<float>(state, 0xF4) - 0.55000001192092896f) /
                                   0.34999996423721313f)));
    }
}
}
void update_animation_step(RestrictedPlayerCommandOwner& owner, void* player, void* state,
                           Command& command, bool jump_reset, std::int32_t seed, std::int32_t count,
                           const std::array<float, 3>* replay_angles,
                           const RestrictedPlayerCommandBindings& b) {
    auto* descriptor = static_cast<std::byte*>(player) + b.offsets.animation.animation_layers;
    auto* layers = read<std::byte*>(descriptor, 0);
    const bool replay = replay_angles != nullptr;
    if (replay && jump_reset)
        jump_layer(owner, player, state, layers, count, b.helpers);
    const auto tick = observer_tick(b, command, seed);
    *b.current_time = float(tick) * *b.interval;
    if (!replay) {
        layers = read<std::byte*>(descriptor, 0);
        if (jump_reset)
            jump_layer(owner, player, state, layers, count, b.helpers);
        if (read<std::int32_t>(state, 0x70) >= *b.frame_number)
            write(state, 0x70, std::bit_cast<std::int32_t>(std::uint32_t(*b.frame_number) - 1U));
    }
    animation::apply_movement_button_state(player, command, b.helpers);
    if (replay) {
        const auto slot = std::uint32_t(command.number) % 150U;
        write(player, b.offsets.field_66146802, owner.prediction_records[slot].recoil_pitch);
    } else {
        const auto scale =
            std::bit_cast<float>(lagcomp::read_prediction_convar(b.recoil_scale_convar));
        write(player, b.offsets.field_66146802,
              scale * read<float>(player, b.offsets.field_1bae1168));
    }
    write(player, b.offsets.field_f58e6118,
          std::uint32_t(reinterpret_cast<std::uintptr_t>(&command)));
    const auto previous_air = read<std::uint8_t>(state, 0x109) != 0;
    const auto previous_time = read<float>(state, 0x110);
    layers = read<std::byte*>(descriptor, 0);
    if (count > 3)
        prepare_idle_layer(owner, player, state, command, layers, b);
    animation::update_landing_layer(player, state, b.helpers);
    layers = read<std::byte*>(descriptor, 0);
    prepare_landing_tag(owner, player, state, layers, b);
    std::array<std::uint32_t, 6> saved;
    if (count > 2) {
        saved[0] = read<std::uint32_t>(layers, 0x90);
        saved[1] = read<std::uint32_t>(layers, 0x9C);
        saved[2] = read<std::uint32_t>(layers, 0x58);
        saved[3] = read<std::uint32_t>(layers, 0x64);
    }
    if (count > 3) {
        saved[4] = read<std::uint32_t>(layers, 0xC8);
        saved[5] = read<std::uint32_t>(layers, 0xD4);
    }
    const auto saved_command = read<std::uint32_t>(player, b.offsets.field_f58e6118);
    if (replay) {
        const auto saved_state = read<void*>(player, b.offsets.animation.animation_state);
        *b.animation_update_flag = 1;
        write(player, b.offsets.animation.animation_state, state);
        auto timing = b.animation_timing;
        timing.frame = *b.frame_number;
        timing.current_time = *b.current_time;
        timing.interval = *b.interval;
        animation::update_animation_state(state, *replay_angles, timing, b.animation_engine);
        write(player, b.offsets.animation.animation_state, saved_state);
        *b.animation_update_flag = 0;
        write(player, b.offsets.field_f58e6118, saved_command);
    } else {
        const auto previous_body = read<std::uint32_t>(player, b.offsets.animation.field_a2a8247f);
        write(player, b.offsets.animation.field_a2a8247f, *b.body_yaw);
        write(player, b.offsets.field_f58e6118,
              std::uint32_t(reinterpret_cast<std::uintptr_t>(&command)));
        if (auto* current_state = read<void*>(player, b.offsets.animation.animation_state))
            if (read<float>(current_state, 0x6C) == *b.current_time)
                write(current_state, 0x6C, *b.current_time - *b.interval);
        *b.animation_update_flag = 1;
        auto** table = b.original_player_table;
        if (b.resolve_original_player_table)
            table = static_cast<engine::VtableHookMetadata*>((*static_cast<void***>(player))[-2])
                        ->original_table;
        reinterpret_cast<engine::MemberFunction<void>>(table[224])(player);
        *b.animation_update_flag = 0;
        b.animation_timing.clear_reset_byte();
        write(player, b.offsets.field_f58e6118, saved_command);
        write(player, b.offsets.animation.field_a2a8247f, previous_body);
    }
    if (count > 3) {
        write(layers, 0xD4, saved[5]);
        write(layers, 0xC8, saved[4]);
    }
    if (count > 2) {
        write(layers, 0x64, saved[3]);
        write(layers, 0x58, saved[2]);
        write(layers, 0x9C, saved[1]);
        write(layers, 0x90, saved[0]);
    }
    update_post_layers(owner, player, state, read<std::byte*>(descriptor, 0), count, replay, b);
    const auto vertical = read<float>(player, b.offsets.field_36375c23 + 8);
    animation::update_lower_body_yaw_timer(command.angles[1], read<float>(state, 0xEC), vertical,
                                           *b.current_time, read<float>(state, 0x80),
                                           *b.body_yaw_time, *b.body_yaw);
    animation::update_post_animation_layer(player, state, previous_air, previous_time, b.helpers);
}
} // namespace skeet::client::restricted_detail

// ============================================================================
// Subcomponent: runtime_composition.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::client {
namespace {
struct RandomImports {
    void(__cdecl* seed)(std::int32_t);
    float(__cdecl* uniform)(float, float);
};
RandomImports required_random_imports() {
    const auto module = GetModuleHandleA("vstdlib.dll");
    if (!module)
        throw std::runtime_error("Spread RNG requires the already-loaded vstdlib.dll");
    const auto seed = GetProcAddress(module, "RandomSeed");
    const auto uniform = GetProcAddress(module, "RandomFloat");
    if (!seed || !uniform)
        throw std::runtime_error("vstdlib RandomSeed/RandomFloat exports are missing");
    return {reinterpret_cast<decltype(RandomImports::seed)>(seed),
            reinterpret_cast<decltype(RandomImports::uniform)>(uniform)};
}
void seed_random(void* context, std::int32_t seed) {
    static_cast<const RandomImports*>(context)->seed(seed);
}
float uniform_random(void* context, float minimum, float maximum) {
    return static_cast<const RandomImports*>(context)->uniform(minimum, maximum);
}
CrtRuntimeImports require_imports(ClientRuntimeStorage& storage, CrtRuntimeImports imports) {
    const auto memory = storage.memory();
    if (!memory.allocate || !memory.free || !memory.process_heap ||
        imports.startup.memory.allocate != memory.allocate ||
        imports.startup.memory.free != memory.free ||
        imports.startup.memory.process_heap != memory.process_heap ||
        imports.startup.memory.purecall != memory.purecall)
        throw std::invalid_argument("CRT runtime must share the canonical process-heap imports");
    if (!imports.random.seed || !imports.random.uniform || !imports.trigonometry.sine ||
        !imports.trigonometry.cosine)
        throw std::invalid_argument(
            "CRT runtime requires its actual RNG and SSE trigonometry imports");
    if (!imports.hook_queries.query_memory || !imports.startup.images.image_nt_header ||
        !imports.startup.modules.loader ||
        !imports.startup.forwarded_exports.image_directory_entry ||
        !imports.startup.terminate_process)
        throw std::invalid_argument(
            "CRT runtime requires its actual loader, image, query and termination imports");
    return imports;
}
}

const rage::SpreadRandom& native_spread_random() {
    static RandomImports imports = required_random_imports();
    static const rage::SpreadRandom random{&imports, seed_random, uniform_random};
    return random;
}
CrtRuntimeImports native_crt_runtime_imports() {
    return {engine::native_source_initialization_imports(), engine::native_hook_descriptor_system(),
            native_spread_random(), animation::native_sse_trigonometry()};
}

struct CrtRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    CrtRuntimeImports imports;
    rage::SpreadStorage spread;
    rage::SpreadStorageBindings spread_imports;
    StaticCleanupRuntime static_cleanup;
    ExitRegistrationContext exits;
    engine::HookDescriptorStorage descriptor_storage;
    engine::HookDescriptorRuntime descriptors;
    HookDescriptorCleanupContext descriptor_initializers;
    rage::GroupStorage groups;
    rage::GroupInitializerBindings group_initializers;
    esp::FlagTable flags;
    esp::FlagTableInitialization flag_initializers;
    script::UiTypeStorage ui_types;
    script::UiTypeInitialization ui_type_initializers;
    events::EventNameStartupBindings event_names;
    engine::FirstStartupServiceBindings first_startup;

    Data(ClientRuntimeStorage& source, CrtRuntimeImports services)
        : storage(source), imports(require_imports(source, services)), spread(source.data_owner()),
          spread_imports{source.memory(), imports.random, imports.trigonometry},
          static_cleanup({source.owner_registries(), source.data_update_observers(),
                          source.recharge_observers(), source.prediction_observers(),
                          source.empty_ticks(), source.command_phases(), source.entity_observers(),
                          spread, spread_imports, source.config_storage(), source.memory()}),
          exits(source.exit_callbacks(), static_cleanup), descriptor_storage(source.data_owner()),
          descriptors(source.memory(), imports.hook_queries, imports.startup.images),
          descriptor_initializers(source.exit_callbacks(), descriptors, descriptor_storage),
          groups(source.data_owner(), source.memory()),
          group_initializers{source.configuration(), source.exit_callbacks(), groups},
          flags(source.data_owner(), source.memory()),
          flag_initializers(flags, source.exit_callbacks()),
          ui_types(source.data_owner(), source.memory()),
          ui_type_initializers(ui_types, source.exit_callbacks()),
          event_names(source.event_registry(), source.exit_callbacks(), source.memory()),
          first_startup(imports.startup.first_startup_bindings(source.data())) {}
};

CrtRuntimeOwner::CrtRuntimeOwner(ClientRuntimeStorage& storage)
    : CrtRuntimeOwner(storage, native_crt_runtime_imports()) {}
CrtRuntimeOwner::CrtRuntimeOwner(ClientRuntimeStorage& storage, CrtRuntimeImports imports)
    : data_(std::make_unique<Data>(storage, imports)) {}
CrtRuntimeOwner::~CrtRuntimeOwner() = default;
ClientRuntimeStorage& CrtRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
const rage::SpreadRandom& CrtRuntimeOwner::random() const noexcept {
    return data_->imports.random;
}
const RageTrigonometry& CrtRuntimeOwner::trigonometry() const noexcept {
    return data_->imports.trigonometry;
}
rage::SpreadStorage& CrtRuntimeOwner::spread() noexcept {
    return data_->spread;
}
const rage::SpreadStorageBindings& CrtRuntimeOwner::spread_bindings() const noexcept {
    return data_->spread_imports;
}
StaticCleanupRuntime& CrtRuntimeOwner::static_cleanup() noexcept {
    return data_->static_cleanup;
}
ExitRegistrationContext& CrtRuntimeOwner::exit_registration() noexcept {
    return data_->exits;
}
engine::HookDescriptorStorage& CrtRuntimeOwner::descriptor_storage() noexcept {
    return data_->descriptor_storage;
}
engine::HookDescriptorRuntime& CrtRuntimeOwner::descriptors() noexcept {
    return data_->descriptors;
}
engine::HookDescriptorSystem CrtRuntimeOwner::hook_system() const noexcept {
    return data_->imports.hook_queries;
}
HookDescriptorCleanupContext& CrtRuntimeOwner::descriptor_initializers() noexcept {
    return data_->descriptor_initializers;
}
rage::GroupStorage& CrtRuntimeOwner::rage_groups() noexcept {
    return data_->groups;
}
rage::GroupInitializerBindings& CrtRuntimeOwner::group_initializers() noexcept {
    return data_->group_initializers;
}
esp::FlagTable& CrtRuntimeOwner::esp_flags() noexcept {
    return data_->flags;
}
esp::FlagTableInitialization& CrtRuntimeOwner::flag_initializers() noexcept {
    return data_->flag_initializers;
}
script::UiTypeStorage& CrtRuntimeOwner::ui_types() noexcept {
    return data_->ui_types;
}
script::UiTypeInitialization& CrtRuntimeOwner::ui_type_initializers() noexcept {
    return data_->ui_type_initializers;
}
events::EventNameStartupBindings& CrtRuntimeOwner::event_name_initializers() noexcept {
    return data_->event_names;
}
engine::FirstStartupServiceBindings& CrtRuntimeOwner::first_startup_services() noexcept {
    return data_->first_startup;
}

void CrtRuntimeOwner::bind_initializers(NativeStartupSources& sources) {
    if (&sources.data() != &storage().data())
        throw std::invalid_argument("CRT callbacks must share the canonical source data owner");
    bind_exit_initializers(sources, exit_registration());
    bind_hook_descriptor_initializers(sources, descriptor_initializers());
    rage::bind_native_group_initializers(sources, group_initializers());
    esp::bind_flag_table_initializer(sources, flag_initializers());
    script::bind_ui_type_initializer(sources, ui_type_initializers());
    events::bind_event_name_initializer(sources, event_name_initializers());
}

struct ConfigurationRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    MovementGlobalConfig movement;
    skins::Settings skins;
    rage::Settings rage;
    anti_aim::Config anti_aim;
    anti_aim::GlobalConfig anti_aim_global;
    anti_aim::FakeLagSettings fake_lag;
    anti_aim::OtherGlobalSettings anti_aim_other;
    esp::PlayerEspSettings player_esp;
    esp::OtherEspSettings other_esp;
    esp::EffectsSettings effects;
    esp::ColoredModelsSettings colored_models;
    misc::FovConfig fov;
    misc::KnifeConfig knife;
    misc::ZeusConfig zeus;
    misc::AutomaticWeaponsConfig automatic_weapons;
    misc::QuickSwitchConfig quick_switch;
    misc::RevealRanksConfig reveal_ranks;
    misc::RevealOverwatchConfig reveal_overwatch;
    misc::ClanTagConfig clan_tag;
    misc::EventLogConfig event_log;
    misc::GrenadePredictionConfig grenades;
    misc::PersistentKillFeedConfig persistent_kill_feed;
    misc::DisablePureConfig disable_pure;
    misc::RebuyConfig rebuy;
    misc::ConsoleOutputConfig console_output;
    misc::NoFallDamageConfig no_fall_damage;
    MiscFeatureSettings miscellaneous;
    misc::PingSpikeConfig ping_spike;
    misc::FreeLookConfig& free_look;
    misc::AntiUntrustedConfig& anti_untrusted;
    misc::HideFromObsConfig& hide_from_obs;
    misc::CustomEventsConfig& custom_events;
    TargetMaskSettings& target_mask;
    ui::WidgetIdentities identities;
    anti_aim::StartupInitializerBindings anti_aim_initializers;
    ConfigInitializerBindings static_initializers;
    GlobalConfigInitializerBindings global_initializers;

    explicit Data(ClientRuntimeStorage& source)
        : storage(source), miscellaneous{fov,
                                         knife,
                                         zeus,
                                         automatic_weapons,
                                         quick_switch,
                                         reveal_ranks,
                                         reveal_overwatch,
                                         clan_tag,
                                         event_log,
                                         grenades,
                                         persistent_kill_feed,
                                         disable_pure,
                                         rebuy,
                                         console_output,
                                         no_fall_damage},
          free_look(source.data().native_cell<misc::FreeLookConfig>(0x43468264u)),
          anti_untrusted(source.data().native_cell<misc::AntiUntrustedConfig>(0x434693DCu)),
          hide_from_obs(source.data().native_cell<misc::HideFromObsConfig>(0x43468E48u)),
          custom_events(source.data().native_cell<misc::CustomEventsConfig>(0x434691A8u)),
          target_mask(source.data().native_cell<TargetMaskSettings>(0x43467974u)),
          anti_aim_initializers{source.configuration(), fake_lag, anti_aim_other},
          static_initializers{source.configuration(),
                              other_esp,
                              player_esp,
                              effects,
                              rage.global,
                              ping_spike,
                              anti_aim_global,
                              target_mask.enabled},
          global_initializers{source.configuration(),
                              rage.global,
                              other_esp,
                              effects,
                              colored_models,
                              anti_untrusted,
                              hide_from_obs,
                              custom_events,
                              free_look,
                              source.settings_menu_storage()} {}
};

ConfigurationRuntimeOwner::ConfigurationRuntimeOwner(ClientRuntimeStorage& storage)
    : data_(std::make_unique<Data>(storage)) {}
ConfigurationRuntimeOwner::~ConfigurationRuntimeOwner() = default;
ClientRuntimeStorage& ConfigurationRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
MovementGlobalConfig& ConfigurationRuntimeOwner::movement() noexcept {
    return data_->movement;
}
skins::Settings& ConfigurationRuntimeOwner::skins_settings() noexcept {
    return data_->skins;
}
rage::Settings& ConfigurationRuntimeOwner::rage_settings() noexcept {
    return data_->rage;
}
anti_aim::Config& ConfigurationRuntimeOwner::anti_aim_settings() noexcept {
    return data_->anti_aim;
}
anti_aim::GlobalConfig& ConfigurationRuntimeOwner::anti_aim_global() noexcept {
    return data_->anti_aim_global;
}
anti_aim::FakeLagSettings& ConfigurationRuntimeOwner::fake_lag_settings() noexcept {
    return data_->fake_lag;
}
anti_aim::OtherGlobalSettings& ConfigurationRuntimeOwner::anti_aim_other() noexcept {
    return data_->anti_aim_other;
}
esp::PlayerEspSettings& ConfigurationRuntimeOwner::player_esp_settings() noexcept {
    return data_->player_esp;
}
esp::OtherEspSettings& ConfigurationRuntimeOwner::other_esp_settings() noexcept {
    return data_->other_esp;
}
esp::EffectsSettings& ConfigurationRuntimeOwner::effects_settings() noexcept {
    return data_->effects;
}
esp::ColoredModelsSettings& ConfigurationRuntimeOwner::colored_model_settings() noexcept {
    return data_->colored_models;
}
MiscFeatureSettings& ConfigurationRuntimeOwner::miscellaneous() noexcept {
    return data_->miscellaneous;
}
misc::PingSpikeConfig& ConfigurationRuntimeOwner::ping_spike() noexcept {
    return data_->ping_spike;
}
misc::FreeLookConfig& ConfigurationRuntimeOwner::free_look() noexcept {
    return data_->free_look;
}
misc::AntiUntrustedConfig& ConfigurationRuntimeOwner::anti_untrusted() noexcept {
    return data_->anti_untrusted;
}
misc::HideFromObsConfig& ConfigurationRuntimeOwner::hide_from_obs() noexcept {
    return data_->hide_from_obs;
}
misc::CustomEventsConfig& ConfigurationRuntimeOwner::custom_events() noexcept {
    return data_->custom_events;
}
TargetMaskSettings& ConfigurationRuntimeOwner::target_mask() noexcept {
    return data_->target_mask;
}
ui::WidgetIdentities& ConfigurationRuntimeOwner::widget_identities() noexcept {
    return data_->identities;
}
ui::SettingsMenuStorage& ConfigurationRuntimeOwner::settings_menu() noexcept {
    return storage().settings_menu_storage();
}
anti_aim::StartupInitializerBindings& ConfigurationRuntimeOwner::anti_aim_initializers() noexcept {
    return data_->anti_aim_initializers;
}
ConfigInitializerBindings& ConfigurationRuntimeOwner::static_initializers() noexcept {
    return data_->static_initializers;
}
GlobalConfigInitializerBindings& ConfigurationRuntimeOwner::global_initializers() noexcept {
    return data_->global_initializers;
}
void ConfigurationRuntimeOwner::bind_initializers(NativeStartupSources& sources) {
    if (&sources.data() != &storage().data())
        throw std::invalid_argument(
            "Configuration callbacks must share the canonical source data owner");
    anti_aim::bind_native_startup_initializers(sources, anti_aim_initializers());
    bind_native_config_initializers(sources, static_initializers());
    bind_global_config_initializers(sources, global_initializers());
}

WindowRuntimeImports
native_window_runtime_imports(const engine::SourceInitializationImports& imports) {
    return {imports, engine::native_unload_system_functions(imports.common_controls),
            input::native_window_hook_imports(imports.common_controls)};
}
namespace {
WindowRuntimeImports require_window_imports(ClientRuntimeStorage& storage,
                                            WindowRuntimeImports imports) {
    const auto memory = storage.memory();
    if (imports.startup.memory.allocate != memory.allocate ||
        imports.startup.memory.free != memory.free ||
        imports.startup.memory.process_heap != memory.process_heap ||
        imports.unload.free_heap != memory.free ||
        imports.unload.process_heap != memory.process_heap)
        throw std::invalid_argument("Window runtime must share the canonical process-heap imports");
    const auto& modules = imports.startup.modules;
    const auto& unload = imports.unload;
    const auto& window = imports.subclass;
    const auto& controls = imports.startup.common_controls;
    if (!modules.loader || !modules.acquire_peb_lock || !modules.release_peb_lock ||
        !unload.set_window_long_w || !unload.set_window_long_a || !unload.get_window_long_w ||
        !unload.get_window_long_a || !unload.remove_window_subclass ||
        !window.read_timestamp_counter || !window.current_thread_id_slot ||
        !window.set_window_subclass || !window.default_subclass_proc || !window.get_window_thread ||
        !window.pc_to_file_header)
        throw std::invalid_argument("Window runtime requires its actual loader and window imports");
    if (window.set_window_subclass !=
            reinterpret_cast<input::WindowHookImports::SetSubclass>(controls.set_window_subclass) ||
        window.default_subclass_proc !=
            reinterpret_cast<input::WindowHookImports::DefaultProcedure>(
                controls.default_subclass_proc) ||
        unload.remove_window_subclass !=
            reinterpret_cast<engine::UnloadSystemFunctions::RemoveWindowSubclass>(
                controls.remove_window_subclass))
        throw std::invalid_argument(
            "Window runtime must share the resolved common-controls exports");
    return imports;
}
}
struct WindowRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    WindowRuntimeImports imports;
    engine::UnloadState state;
    engine::UnloadRuntime unload;
    input::WindowSubclassRuntime subclass;
    Data(ClientRuntimeStorage& source, WindowRuntimeImports services)
        : storage(source), imports(require_window_imports(source, services)),
          state(source.owner_registries().at(0x43475B78u),
                source.data().native_cell<bool>(0x43475895u),
                source.data().native_cell<std::uint32_t>(0x43476908u),
                source.data().native_cell<std::uint32_t>(0x4347644Cu),
                source.data().native_cell<void*>(0x4346A514u),
                source.data().native_cell<void (*)()>(0x43468BF0u),
                source.data().native_cell<bool>(0x43468B80u),
                source.data().native_cell<void*>(0x4346A470u)),
          unload(state, imports.unload),
          subclass(source.input_runtime(), unload, imports.startup.modules, imports.subclass) {}
};
WindowRuntimeOwner::WindowRuntimeOwner(ClientRuntimeStorage& storage)
    : WindowRuntimeOwner(storage, engine::native_source_initialization_imports()) {}
WindowRuntimeOwner::WindowRuntimeOwner(ClientRuntimeStorage& storage,
                                       const engine::SourceInitializationImports& imports)
    : WindowRuntimeOwner(storage, native_window_runtime_imports(imports)) {}
WindowRuntimeOwner::WindowRuntimeOwner(ClientRuntimeStorage& storage, WindowRuntimeImports imports)
    : data_(std::make_unique<Data>(storage, imports)) {}
WindowRuntimeOwner::~WindowRuntimeOwner() = default;
ClientRuntimeStorage& WindowRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
engine::UnloadState& WindowRuntimeOwner::unload_state() noexcept {
    return data_->state;
}
engine::UnloadRuntime& WindowRuntimeOwner::unload_runtime() noexcept {
    return data_->unload;
}
input::WindowSubclassRuntime& WindowRuntimeOwner::subclass() noexcept {
    return data_->subclass;
}
const engine::SourceInitializationImports& WindowRuntimeOwner::startup_imports() const noexcept {
    return data_->imports.startup;
}
void WindowRuntimeOwner::bind_subclass_callback() {
    input::bind_window_subclass_runtime(subclass());
}

StaticCrtImports native_static_crt_imports() {
    return {script::bind_loaded_panorama_v8_api().dispose_global};
}
namespace {
ClientRuntimeStorage& require_static_owners(CrtRuntimeOwner& crt, WindowRuntimeOwner& window,
                                            ui::RendererAssets& assets, StaticCrtImports imports) {
    auto& storage = crt.storage();
    if (&window.storage() != &storage || &assets.storage() != &storage ||
        (window.unload_state().event_listener &&
         window.unload_state().event_listener != &assets.device_listener()))
        throw std::invalid_argument(
            "Static CRT contexts require the canonical early renderer assets and window owners");
    if (!imports.dispose_v8_global)
        throw std::invalid_argument("Static CRT cleanup requires the real V8 DisposeGlobal import");
    return storage;
}
}
struct StaticCrtRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    StaticCrtInitialization static_initializers;
    engine::StartupVectorCleanupBindings native_vectors;
    engine::StartupContainerCleanup native_containers;
    Data(CrtRuntimeOwner& crt, WindowRuntimeOwner& window, ui::RendererAssets& assets,
         StaticCrtConstructorInputs words, StaticCrtImports imports)
        : storage(require_static_owners(crt, window, assets, imports)),
          static_initializers{storage.data_owner(), storage.exit_callbacks(), storage.memory(),
                              assets.icon_cache(),  words.flags_word,         words.keys_word},
          native_vectors(storage.data_owner(), storage.exit_callbacks(), storage.memory(),
                         imports.dispose_v8_global),
          native_containers({storage.exit_callbacks(), storage.memory(),
                             storage.data().native_cell<engine::StartupOwnedPointers>(0x43478DA8u),
                             storage.data().native_cell<engine::StartupOwnedTexts>(0x43479088u),
                             storage.data().native_cell<engine::StartupOwnedTexts>(0x43479078u),
                             storage.data().native_cell<engine::StartupOwnedModules>(0x434792E0u),
                             storage.data().native_cell<engine::StartupOwnedModules>(0x434792D0u),
                             storage.data().native_cell<engine::StartupTextBuckets>(0x4346D700u),
                             storage.data().native_cell<engine::StartupPointerBuckets>(0x4346DA40u),
                             assets.device_listener(), window.unload_state()}) {}
};
StaticCrtRuntimeOwner::StaticCrtRuntimeOwner(CrtRuntimeOwner& crt, WindowRuntimeOwner& window,
                                             ui::RendererAssets& assets,
                                             StaticCrtConstructorInputs words)
    : StaticCrtRuntimeOwner(crt, window, assets, words, native_static_crt_imports()) {}
StaticCrtRuntimeOwner::StaticCrtRuntimeOwner(CrtRuntimeOwner& crt, WindowRuntimeOwner& window,
                                             ui::RendererAssets& assets,
                                             StaticCrtConstructorInputs words,
                                             StaticCrtImports imports)
    : data_(std::make_unique<Data>(crt, window, assets, words, imports)) {}
StaticCrtRuntimeOwner::~StaticCrtRuntimeOwner() = default;
ClientRuntimeStorage& StaticCrtRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
StaticCrtInitialization& StaticCrtRuntimeOwner::static_initializers() noexcept {
    return data_->static_initializers;
}
engine::StartupVectorCleanupBindings& StaticCrtRuntimeOwner::native_vector_cleanup() noexcept {
    return data_->native_vectors;
}
engine::StartupContainerCleanup& StaticCrtRuntimeOwner::native_container_cleanup() noexcept {
    return data_->native_containers;
}
void StaticCrtRuntimeOwner::bind_initializers(NativeStartupSources& sources) {
    if (&sources.data() != &storage().data())
        throw std::invalid_argument(
            "Static CRT callbacks must share the canonical source data owner");
    bind_static_crt_initializers(sources, static_initializers());
    engine::bind_startup_vector_cleanup(sources, native_vector_cleanup());
    engine::bind_startup_container_cleanup(sources, native_container_cleanup());
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: runtime_feature_bindings.cpp
// ============================================================================

namespace skeet::client {
namespace {
ClientRuntimeStorage& checked(ClientRuntimeStorage& storage, PacketRuntimeOwner& packet,
                              ExploitRuntimeOwner& exploit, anti_aim::RuntimeOwner& aa,
                              effects::EffectsRuntimeOwner& effects,
                              script::SourceScriptRuntimeOwner& scripts) {
    if (&packet.storage() != &storage || &exploit.storage() != &storage ||
        &aa.storage() != &storage || &effects.storage() != &storage ||
        &scripts.storage() != &storage)
        throw std::invalid_argument(
            "Combat factory composition requires the same physical process owners");
    const auto& factory = scripts.factory_bindings();
    if (&factory.runtime != &scripts.runtime() || &factory.runtime.bridge != &scripts.bridge() ||
        &factory.modules != &scripts.module_bindings())
        throw std::invalid_argument(
            "Combat Lua factory requires its actual source runtime and bound modules");
    return storage;
}
void initialize_ping(void*) {
    hud::initialize_ping_hud_owner();
}
void initialize_crosshair(void*) {
    hud::initialize_crosshair_hud_owner();
}

void require_graph(NativeStartupSources& sources, NativeStartupSourceGraph& g) {
    auto& image = sources.data();
    if (&g.data != &image || g.final_initializer.native_address != 0x43484F06u ||
        !g.final_initializer.owner || !g.final_initializer.invoke)
        throw std::invalid_argument(
            "Late feature composition requires the canonical data and genuine final initializer context");
    // This checks the existing required operation; it never replaces its
    // owner/function or binds an empty stand-in for the remaining final work.
    sources.require_initializer(0, 0x43484F06u);
    if (&g.configuration.descriptors != &image.native_cell<ConfigLink>(0x434798BCu) ||
        &g.configuration.hotkeys != &image.native_cell<HotkeyLink>(0x434798C4u) ||
        &g.menu_key != &image.native_cell<RegisteredHotkey>(0x43478DB8u))
        throw std::invalid_argument(
            "Late feature factories require canonical configuration and menu-key cells");
    if (&g.prediction_history.configuration != &g.configuration ||
        &g.shared_esp.configuration != &g.configuration ||
        &g.shot_feedback.configuration != &g.configuration ||
        &g.runtime_owners.configuration != &g.configuration ||
        &g.misc_owners.configuration != &g.configuration ||
        &g.combat_owners.configuration != &g.configuration ||
        &g.skin_owners.configuration != &g.configuration ||
        &g.movement_hud.configuration != &g.configuration ||
        &g.world_owners.configuration != &g.configuration ||
        &g.runtime_owners.sequences.settings != &g.skins || &g.skin_owners.settings != &g.skins)
        throw std::invalid_argument(
            "Late feature groups require their one canonical configuration owner");
    const auto& registries = g.exits.cleanup.owner_registry_storage();
    if (&registries.image() != &image || g.entity_commands.data.get() != &image ||
        g.target_mask.data.get() != &image || g.history_owners.data.get() != &image ||
        g.combat_owners.data.get() != &image || &g.combat_owners.registries != &registries)
        throw std::invalid_argument(
            "Late feature groups require the same native data and lifecycle registry storage");
    struct Alias {
        const engine::OwnerRegistry* registry;
        std::uint32_t identity;
    };
    const std::array aliases{
        Alias{&g.prediction_history.registries.primary.registry, 0x43475B78u},
        Alias{&g.entity_commands.registries.lifecycle.registry, 0x43475B78u},
        Alias{&g.entity_commands.registries.map_entities.registry, 0x43475770u},
        Alias{&g.free_look.lifecycle.registry, 0x43475B78u},
        Alias{&g.free_look.updates.registry, 0x43475988u},
        Alias{&g.target_mask.registries.primary.registry, 0x43475B78u},
        Alias{&g.target_mask.registries.messages.registry, 0x43475B94u},
        Alias{&g.target_mask.registries.hud.registry, 0x43475A9Cu},
        Alias{&g.shared_esp.registries.primary.registry, 0x43475B78u},
        Alias{&g.shared_esp.registries.messages.registry, 0x43475B94u},
        Alias{&g.shot_feedback.lifecycle.registry, 0x43475B78u},
        Alias{&g.shot_feedback.packets.registry, 0x4347584Cu},
        Alias{&g.shot_feedback.frames.registry, 0x43475AD8u},
        Alias{&g.shot_feedback.positions.registry, 0x43475930u},
        Alias{&g.simple_hud.lifecycle.registry, 0x43475B78u},
        Alias{&g.simple_hud.hud.registry, 0x43475A9Cu},
        Alias{&g.prediction_masks.lifecycle.registry, 0x43475B78u},
        Alias{&g.prediction_masks.frames.registry, 0x43475AD8u}};
    for (const auto& alias : aliases)
        if (alias.registry != &registries.at(alias.identity))
            throw std::invalid_argument(
                "Late feature factories must retain their exact native owner registry cells");
    const auto* commands = &g.entity_commands.registries.commands;
    const auto* entities = &g.entity_commands.registries.entities;
    if (&commands->image() != &image || &entities->image() != &image ||
        &g.prediction_history.registries.commands != commands ||
        &g.target_mask.registries.commands != commands ||
        &g.shared_esp.registries.commands != commands || &g.shot_feedback.commands != commands ||
        &g.buffered_shots.lifecycle.commands != commands ||
        &g.viewmodel_lag.registries.commands != commands ||
        &g.runtime_owners.rage.command_owners != commands || &g.misc_owners.commands != commands ||
        &g.skin_owners.commands != commands || &g.combat_owners.packet.commands != commands ||
        &g.movement_hud.movement_registries.commands != commands ||
        &g.world_owners.command_owners != commands ||
        &g.runtime_owners.entity_updates != entities || &g.misc_owners.entities != entities ||
        &g.skin_owners.entity_updates != entities || &g.history_owners.entities != entities ||
        &g.target_mask.registries.entities != entities)
        throw std::invalid_argument(
            "Late feature factories require their one command and entity receiver registries");
    if (&g.entity_commands.registries.data_updates.image() != &image ||
        &g.entity_commands.registries.recharge.image() != &image ||
        &g.entity_commands.registries.prediction.image() != &image ||
        &g.prediction_history.registries.data_updates.image() != &image ||
        &g.prediction_history.registries.recharge.image() != &image ||
        &g.prediction_history.registries.prediction.image() != &image ||
        &g.prediction_history.registries.empty_ticks.image() != &image ||
        &g.shared_esp.registries.empty_ticks.image() != &image ||
        &g.shot_feedback.recharge.image() != &image ||
        &g.runtime_owners.legit.empty_tick_owners != &g.prediction_history.registries.empty_ticks)
        throw std::invalid_argument(
            "Late feature observers require their canonical prediction, recharge and empty-tick cells");
    if (&g.free_look.bindings.active != &image.native_cell<effects::FreeLookOwner*>(0x43475A64u) ||
        &g.shared_esp.bindings.active !=
            &image.native_cell<RejectedPlayerUpdateOwner*>(0x434758D0u) ||
        &g.shot_feedback.active != &image.native_cell<rage::FeedbackOwner*>(0x43475A7Cu) ||
        &g.prediction_masks.active != &image.native_cell<rage::PredictionMaskOwner*>(0x43475A58u) ||
        &g.runtime_owners.rage.commands.prediction_masks() != &g.prediction_masks.active ||
        &g.simple_hud.entity_list != &image.native_cell<void*>(0x4346A890u) ||
        &g.simple_hud.cvar != &image.native_cell<void*>(0x4346A87Cu) ||
        &g.simple_hud.suspended != &image.native_cell<std::uint8_t>(0x43468E58u) ||
        &g.simple_hud.ping_channel.bindings.history !=
            &image.native_cell<misc::PingSpikeNetworkHistory>(0x4347A500u))
        throw std::invalid_argument(
            "Late feature factories require canonical source singleton and HUD cells");
    if (&g.simple_hud.feature_indicators != &g.global_configuration.other_esp.feature_indicators ||
        &g.simple_hud.overlays != &g.global_configuration.other_esp.overlays ||
        &g.combat_owners.scripting.modules.ui_types != &g.ui_types.storage ||
        &g.source_terminal_cleanup.callbacks != &g.exits.callbacks ||
        &g.source_terminal_cleanup.bridge != &g.combat_owners.scripting.runtime.bridge ||
        &g.source_terminal_cleanup.bridge != &g.shot_feedback.state.scripts ||
        &g.shot_feedback.state != &g.history_owners.history.feedback ||
        &g.shot_feedback.runtime.players != &g.source_terminal_cleanup.players)
        throw std::invalid_argument(
            "Late feature factories require the actual source HUD, Lua, history and terminal producers");
    g.source_terminal_cleanup.require_producers(g.combat_owners.scripting.runtime,
                                                g.combat_owners.scripting.modules,
                                                g.history_owners.history.recording);
}
}

struct CombatRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    CombatRuntimeFactoryBindings factory;
    Data(ClientRuntimeStorage& s, PacketRuntimeOwner& packet, ExploitRuntimeOwner& exploit,
         anti_aim::RuntimeOwner& aa, effects::EffectsRuntimeOwner& effects,
         script::SourceScriptRuntimeOwner& scripts)
        : storage(checked(s, packet, exploit, aa, effects, scripts)),
          factory{s.data_owner(),
                  s.owner_registries(),
                  s.configuration(),
                  packet.factory_bindings(),
                  exploit.factory_bindings(),
                  aa.factory_bindings(),
                  effects.factory_bindings(),
                  scripts.factory_bindings()} {}
};
CombatRuntimeOwner::CombatRuntimeOwner(ClientRuntimeStorage& s, PacketRuntimeOwner& packet,
                                       ExploitRuntimeOwner& exploit, anti_aim::RuntimeOwner& aa,
                                       effects::EffectsRuntimeOwner& effects,
                                       script::SourceScriptRuntimeOwner& scripts)
    : data_(std::make_unique<Data>(s, packet, exploit, aa, effects, scripts)) {}
CombatRuntimeOwner::CombatRuntimeOwner(ClientRuntimeStorage& s, PacketRuntimeOwner& packet,
                                       ExploitRuntimeOwner& exploit, anti_aim::RuntimeOwner& aa,
                                       effects::EffectsRuntimeOwner& effects,
                                       script::SourceScriptRuntimeOwner& scripts,
                                       script::SourceScriptModuleInputs inputs) {
    if (&scripts.storage() != &s)
        throw std::invalid_argument("Combat Lua modules must share this process storage");
    scripts.bind_modules(inputs);
    data_ = std::make_unique<Data>(s, packet, exploit, aa, effects, scripts);
}
CombatRuntimeOwner::~CombatRuntimeOwner() = default;
ClientRuntimeStorage& CombatRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
CombatRuntimeFactoryBindings& CombatRuntimeOwner::factory_bindings() noexcept {
    return data_->factory;
}
void CombatRuntimeOwner::bind_initializers(NativeStartupSources& sources) {
    bind_combat_runtime_factories(sources, data_->factory);
}

void bind_runtime_feature_sources(NativeStartupSources& sources, NativeStartupSourceGraph& graph) {
    require_graph(sources, graph);
    bind_prediction_history_initializer(sources, graph.prediction_history);
    bind_native_entity_command_factory(sources, graph.entity_commands);
    effects::bind_free_look_initializer(sources, graph.free_look);
    bind_native_target_mask_factory(sources, graph.target_mask);
    bind_shared_esp_initializer(sources, graph.shared_esp);
    rage::bind_feedback_owner_factory(sources, graph.shot_feedback);
    bind_buffered_shot_factory(sources, graph.buffered_shots);
    misc::bind_viewmodel_lag_factory(sources, graph.viewmodel_lag);
    hud::bind_simple_hud_owners_context(graph.simple_hud);
    sources.bind_initializer({0x4347DD87u, &graph.simple_hud, initialize_ping});
    sources.bind_initializer({0x43486BF7u, &graph.simple_hud, initialize_crosshair});
    bind_runtime_owner_factories(sources, graph.runtime_owners);
    rage::bind_prediction_mask_factory(sources, graph.prediction_masks);
    engine::bind_map_game_system_factory(sources, graph.map_game_system);
    bind_misc_runtime_factories(sources, graph.misc_owners);
    bind_combat_runtime_factories(sources, graph.combat_owners);
    bind_skin_runtime_factories(sources, graph.skin_owners);
    bind_movement_hud_factories(sources, graph.movement_hud);
    bind_history_runtime_factories(sources, graph.history_owners);
    bind_world_runtime_factories(sources, graph.world_owners);
    bind_game_window_proxy_factory(sources, graph.game_window_proxy);
    preflight_runtime_owner_initialization(sources);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: runtime_owner_bindings.cpp
// ============================================================================

namespace skeet::client {
namespace {

void initialize_sequences(void* owner) {
    auto& b = *static_cast<RuntimeOwnerFactoryBindings*>(owner);
    (void)skins::initialize_sequence_owner(b.lifecycle, b.entity_updates, b.configuration,
                                           b.sequences);
}
void initialize_rage(void* owner) {
    auto& b = *static_cast<RuntimeOwnerFactoryBindings*>(owner);
    (void)rage::construct_rage_owner(b.rage, b.rage_active);
}
void initialize_legit(void* owner) {
    (void)legit::create_legit_owner(static_cast<RuntimeOwnerFactoryBindings*>(owner)->legit);
}
void initialize_esp(void* owner) {
    auto& b = *static_cast<RuntimeOwnerFactoryBindings*>(owner);
    esp::initialize_esp_owner(b.esp_active, b.esp);
}
void initialize_blur(void* owner) {
    auto& b = *static_cast<RuntimeOwnerFactoryBindings*>(owner);
    effects::initialize_blur(b.lifecycle, b.effect_frames, b.blur_active, b.blur);
}
void initialize_console(void* owner) {
    auto& b = *static_cast<RuntimeOwnerFactoryBindings*>(owner);
    (void)misc::initialize_console_output_owner(b.lifecycle, b.hud, b.configuration,
                                                b.console_settings, b.console_active, b.frame_time);
}
void initialize_fov(void* owner) {
    auto& b = *static_cast<RuntimeOwnerFactoryBindings*>(owner);
    auto* storage = engine::allocate_owner_storage(
        b.lifecycle.memory, std::max(std::size_t{0x34}, misc::fov_owner_size()));
    //4347DA73 is an unresolved INT1 allocation-failure edge. This source
    //factory fails explicitly rather than inventing a null continuation.
    if (!storage)
        throw std::bad_alloc();
    std::memset(&b.fov_settings, 0, sizeof b.fov_settings);
    misc::construct_fov_owner(storage, b.lifecycle, b.esp.registries.updates, b.configuration,
                              b.fov_settings, b.fov);
}
void initialize_purchase_log(void* owner) {
    auto& b = *static_cast<RuntimeOwnerFactoryBindings*>(owner);
    auto* storage = engine::allocate_owner_storage(
        b.lifecycle.memory, std::max(std::size_t{0xE4}, misc::purchase_log_owner_size()));
    //43481F59 reaches unresolved INT1 on OOM; bad_alloc is source policy.
    if (!storage)
        throw std::bad_alloc();
    misc::construct_purchase_log(storage, b.lifecycle, b.esp.registries.frame_update,
                                 b.configuration, b.sequences.events, b.event_output,
                                 b.event_targets, b.fov.entity_list, b.purchase_console,
                                 b.console_container);
}

/* [dedup same_memory] */
void require_bindings(NativeStartupSources& sources, const RuntimeOwnerFactoryBindings& b) {
    const auto& image = sources.data();
    if (!b.lifecycle.memory.allocate || !b.lifecycle.memory.free ||
        !b.lifecycle.memory.process_heap)
        throw std::invalid_argument("Runtime owner factories require their canonical process heap");
    struct RegistryAlias {
        const engine::OwnerRegistryBinding* binding;
        std::uint32_t address;
    };
    const std::array aliases{RegistryAlias{&b.lifecycle, 0x43475b78u},
                             RegistryAlias{&b.hud, 0x43475a9cu},
                             RegistryAlias{&b.effect_frames, 0x43475a84u},
                             RegistryAlias{&b.rage.lifecycle, 0x43475b78u},
                             RegistryAlias{&b.rage.hud_owners, 0x43475a9cu},
                             RegistryAlias{&b.legit.lifecycle, 0x43475b78u},
                             RegistryAlias{&b.esp.registries.owners, 0x43475b78u},
                             RegistryAlias{&b.esp.registries.hud, 0x43475a9cu},
                             RegistryAlias{&b.esp.registries.updates, 0x43475988u},
                             RegistryAlias{&b.esp.registries.packets, 0x4347584cu},
                             RegistryAlias{&b.esp.registries.frame_update, 0x43475ad8u},
                             RegistryAlias{&b.esp.registries.map_parser, 0x43475770u}};
    for (const auto& alias : aliases)
        if (&alias.binding->registry != &image.native_cell<engine::OwnerRegistry>(alias.address) ||
            !same_memory(alias.binding->memory, b.lifecycle.memory))
            throw std::invalid_argument(
                "Runtime owner factories must share their native registry cells and heap");
    if (&b.configuration.descriptors != &image.native_cell<ConfigLink>(0x434798bcu) ||
        &b.configuration.hotkeys != &image.native_cell<HotkeyLink>(0x434798c4u) ||
        &b.rage.configuration != &b.configuration || &b.legit.configuration != &b.configuration ||
        &b.esp.registries.configuration != &b.configuration)
        throw std::invalid_argument(
            "Runtime owner factories must share the canonical configuration registry");
    if (&b.rage.command_owners != &b.legit.command_owners ||
        &b.rage.command_owners.image() != &image || &b.legit.empty_tick_owners.image() != &image ||
        &b.rage.entity_owners != &b.entity_updates ||
        &b.esp.registries.histories != &b.entity_updates || &b.entity_updates.image() != &image ||
        &b.entity_updates.native_storage() !=
            &image.native_cell<engine::OwnerRegistry>(0x43475ba8u))
        throw std::invalid_argument(
            "Runtime owner factories must share their command and entity observer registries");
    if (&b.rage_active != &image.native_cell<rage::LifecycleOwner*>(0x43475810u) ||
        &b.legit.active != &image.native_cell<legit::LifecycleOwner*>(0x4347588cu) ||
        &b.esp_active != &image.native_cell<esp::EspOwnerRuntime*>(0x434758dcu) ||
        &b.blur_active != &image.native_cell<effects::BlurEffect*>(0x43475a20u) ||
        &b.console_active != &image.native_cell<misc::ColoredLogRing*>(0x434758ccu))
        throw std::invalid_argument(
            "Runtime owner factories require their canonical published owner slots");
    if (&b.sequences.settings != &b.sequences.receive.bindings.settings ||
        &b.sequences.state != &b.sequences.receive.state ||
        &b.sequences.events != &b.esp.registries.events)
        throw std::invalid_argument(
            "Runtime owner factories must share their sequence state and event registry");
    if (&b.fov.entity_list != &image.native_cell<void*>(0x4346a890u) ||
        &b.fov.sensitivity.local_player != &image.native_cell<void*>(0x43475808u) ||
        &b.fov.sensitivity.override_context != &image.native_cell<void*>(0x43468994u) ||
        &b.fov.sensitivity.encoded_mouse_sensitivity_cvar !=
            &image.native_cell<std::uint32_t>(0x43467a48u) ||
        &b.console_container != &image.native_cell<void*>(0x43467d84u) ||
        b.event_output.hud_slot != &b.console_active ||
        b.event_output.script != &b.purchase_console.bridge ||
        &b.purchase_console.storage.image() != &image)
        throw std::invalid_argument(
            "FOV and event owners require canonical entity and hook storage");
}
}

void bind_runtime_owner_factories(NativeStartupSources& sources,
                                  RuntimeOwnerFactoryBindings& bindings) {
    require_bindings(sources, bindings);
    const std::array calls{NativeInitializerCall{0x43481f6au, &bindings, initialize_sequences},
                           NativeInitializerCall{0x4348da25u, &bindings, initialize_rage},
                           NativeInitializerCall{0x4347febcu, &bindings, initialize_legit},
                           NativeInitializerCall{0x43481eabu, &bindings, initialize_esp},
                           NativeInitializerCall{0x4347f6dbu, &bindings, initialize_blur},
                           NativeInitializerCall{0x43484e4eu, &bindings, initialize_console},
                           NativeInitializerCall{0x4347da60u, &bindings, initialize_fov},
                           NativeInitializerCall{0x43481f44u, &bindings, initialize_purchase_log}};
    for (const auto& call : calls)
        sources.bind_initializer(call);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: runtime_owner_initialization.cpp
// ============================================================================

namespace skeet::client {
namespace {
// Devirtualized factory dispatch at 0x434BE957. Every primary vtable in the
// working 57-owner registry matches a reachable store in its corresponding
// constructor. The retained addresses identify recovered source bindings.
constexpr std::array<std::uint32_t, 57> owner_initializers{
    {0x434903ad, 0x4347e356, 0x4348066a, 0x43481f6a, 0x4347e13e, 0x43486705, 0x43481dc0, 0x4347dd87,
     0x4347ff2f, 0x4349081e, 0x43484a2b, 0x4347da60, 0x4348e608, 0x43481f44, 0x4348da54, 0x43481eda,
     0x434856d6, 0x4347da84, 0x4348401c, 0x4348da25, 0x4347febc, 0x4348e51f, 0x43483ff8, 0x43490389,
     0x43485ccc, 0x43484f66, 0x43485c3a, 0x43484f93, 0x4347d8d0, 0x43486bf7, 0x4349095e, 0x4348e3f9,
     0x43481eab, 0x4347e81a, 0x43484ac4, 0x43480644, 0x4348574e, 0x4348da78, 0x4348e09f, 0x43485be3,
     0x43481d50, 0x4348038f, 0x4348da01, 0x43490411, 0x43484ed9, 0x4347ea07, 0x4348e6aa, 0x4347d94f,
     0x4347f6db, 0x4347e2e3, 0x4347da14, 0x43481e08, 0x43481de4, 0x43484954, 0x43484e4e, 0x4347f902,
     0x43490982}};
}

std::span<const std::uint32_t, 57> runtime_owner_initializers() noexcept {
    return owner_initializers;
}

void preflight_runtime_owner_initialization(const NativeStartupSources& sources) {
    // Validate the complete source binding set before constructing any owner.
    // Missing implementations remain errors, never calls into retained code.
    for (std::uint32_t i = 0; i != owner_initializers.size(); ++i)
        sources.require_initializer(0x1148 + i * 4, owner_initializers[i]);
}
void initialize_runtime_owners(const NativeStartupSources& sources) {
    preflight_runtime_owner_initialization(sources);
    for (std::uint32_t i = 0; i != owner_initializers.size(); ++i)
        sources.initialize(0x1148 + i * 4, owner_initializers[i]);
}
}

// ============================================================================
// Subcomponent: runtime_storage.cpp
// ============================================================================

namespace skeet::client {
namespace {
std::shared_ptr<engine::NativeDataImage>
require_data(std::shared_ptr<engine::NativeDataImage> data) {
    if (!data)
        throw std::invalid_argument("Client runtime storage requires one canonical data image");
    return data;
}
std::shared_ptr<engine::NativeDataImage> create_runtime_data() {
    auto data = engine::NativeDataImage::create_source();
    engine::initialize_source_native_data(*data);
    return data;
}
}

ClientRuntimeStorage::ClientRuntimeStorage() : ClientRuntimeStorage(create_runtime_data()) {}

ClientRuntimeStorage::ClientRuntimeStorage(std::shared_ptr<engine::NativeDataImage> data)
    : data_(require_data(std::move(data))), memory_(engine::native_job_memory()),
      configuration_(std::make_shared<PreservedConfigStorage>(data_)), settings_menu_(data_),
      menu_data_(data_), input_(data_->native_cell<std::int32_t>(0x4347C9FCu)),
      owner_registries_(data_),
      commands_(data_, memory_, {data_->native_cell<std::uint32_t>(0x434759C0u), input_.bitmap()}),
      empty_ticks_(data_, memory_, commands_.services().input), entities_(data_, memory_),
      predictions_(data_, memory_), data_updates_(data_, memory_), recharge_(data_, memory_),
      offsets_(data_->native_cell<std::uint32_t*>(0x434797B0u)), event_names_(data_),
      events_(event_names_, data_->native_cell<void*>(0x4346A514u),
              data_->native_cell<void*>(0x43468364u)),
      exits_{data_->native_cell<std::array<std::uintptr_t, 512>>(0x43479CF0u),
             data_->native_cell<std::uintptr_t>(0x4347CA08u),
             data_->native_cell<std::uint32_t>(0x4347CA0Cu)} {}

ClientRuntimeStorage::~ClientRuntimeStorage() {
    if (engine_bindings_started_ && offsets_.words) {
        engine::release_owner_storage(memory_, offsets_.words);
        offsets_.words = nullptr;
    }
}

RegisteredHotkey& ClientRuntimeStorage::menu_hotkey() const {
    return data_->native_cell<RegisteredHotkey>(0x43478DB8u);
}
engine::CoreEngineInterfaces
ClientRuntimeStorage::bind_core_interfaces(engine::FactoryProvider& provider) {
    const auto interfaces = engine::discover_core_interfaces(
        provider, {data_->native_cell<void*>(0x4346A514u), data_->native_cell<void*>(0x4346A890u),
                   data_->native_cell<void*>(0x434683E0u), data_->native_cell<void*>(0x43469264u),
                   data_->native_cell<void*>(0x434689C0u), data_->native_cell<void*>(0x4346A87Cu),
                   data_->native_cell<void*>(0x43468304u)});
    input_.interfaces.engine_client = interfaces.engine_client;
    return interfaces;
}
engine::CoreEngineInterfaces ClientRuntimeStorage::bind_core_interfaces() {
    engine::LoadedModuleFactories provider;
    return bind_core_interfaces(provider);
}
void ClientRuntimeStorage::bind_input_interfaces() {
    const input::EngineInterfaces bindings{
        data_->native_cell<void*>(0x4346A514u), data_->native_cell<void*>(0x43468994u),
        data_->native_cell<engine::MemberFunction<void*, const char*>>(0x4346A374u),
        data_->native_cell<void*>(0x43468D24u), data_->native_cell<void*>(0x4346A89Cu)};
    if (!bindings.engine_client || !bindings.hud || !bindings.find_hud_element ||
        !bindings.cursor_contexts || !bindings.input_system)
        throw std::logic_error(
            "Menu input requires the discovered engine, HUD, input stack and input system");
    // The original cursor helpers read these canonical cells on every call.
    // Publish the same objects to the source runtime before the window hook.
    input_.interfaces = bindings;
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: shared_esp_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define SHARED_ESP_MEMBER __thiscall
#define SHARED_ESP_FAST __fastcall
#else
#define SHARED_ESP_MEMBER
#define SHARED_ESP_FAST
#endif
using PrimaryEntry = void(SHARED_ESP_MEMBER*)(void*);
struct SharedEspInstance;
void SHARED_ESP_MEMBER destroy_entry(void*);
void SHARED_ESP_MEMBER shared_esp_forward_shared_esp_reset_entry(void*);
void SHARED_ESP_MEMBER shared_esp_reset_entry(void*);
void SHARED_ESP_MEMBER shared_esp_unchanged_entry(void*) {} //433CCE8E
void SHARED_ESP_FAST message_entry(void*, misc::NetworkMessageCall*);
const std::array<PrimaryEntry, 9> shared_esp_primary_table{
    destroy_entry,       shared_esp_unchanged_entry, shared_esp_unchanged_entry, shared_esp_unchanged_entry, shared_esp_unchanged_entry,
    shared_esp_forward_shared_esp_reset_entry, shared_esp_unchanged_entry, shared_esp_reset_entry,     shared_esp_unchanged_entry}; //43470354
const std::array<misc::NetworkMessageEntry, 1> message_table{message_entry}; //43472B40

/* deduplicated helper */
const void* decoded_pointer(std::uint32_t value, std::uint32_t mask) noexcept {
    return reinterpret_cast<const void*>(static_cast<std::uintptr_t>(value ^ mask));
}
engine::MemberFunction<void> call_target(const std::byte* call) noexcept {
    const auto address = reinterpret_cast<std::uintptr_t>(call) + 5 +
                         static_cast<std::intptr_t>(read<std::int32_t>(call, 1));
    return reinterpret_cast<engine::MemberFunction<void>>(address);
}
void* allocate(engine::JobMemory memory) {
    constexpr std::uint32_t size = 0x2b8, total = size + 0x20;
    auto* const raw = memory.allocate(memory.process_heap, 8, total);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &total, 4);
    return reinterpret_cast<void*>(header + 16);
}

void unchanged_event(void*, events::GameEventReader&, events::EventPlayerIndices&) {} //433C95B7
void reset_event(void*, events::GameEventReader&, events::EventPlayerIndices&);

struct SharedEspInstance {
    const PrimaryEntry* vtable = nullptr;
    NativeCommandPhaseReceiver command_receiver;
    NativeEntityObserverReceiver entity_receiver;
    engine::JobMemory memory;
    SharedEspOwnerBindings bindings;
    misc::NetworkMessageCallback message{nullptr};
    std::uint32_t auxiliary_identity = 0; //unregistered native full+14
    CommandEmptyTickTimer empty_tick;
    const void* field_b0 = nullptr;
    const void* field_b4 = nullptr;

    SharedEspInstance(const SharedEspOwnerRegistries& registries, ConfigRegistries& configuration,
             SharedEspOwnerBindings context)
        : memory(registries.primary.memory), bindings(context), empty_tick{context.state.timer_84} {
        registries.primary.append(this); //43485DDA
        registries.commands.append(command_receiver, &bindings.command); //5DE2
        registries.messages.append(&message); //5DEA, before table publication
        auxiliary_identity = 0x4346f3b4u; //43485DF2
        registries.entities.append_shared_esp(entity_receiver, bindings.command); //5DF9
        registries.empty_ticks.append(empty_tick); //43485E01
        registries.commands.publish(command_receiver, CommandOwnerId::owner_4346fe30); //43485E08
        message.vtable = message_table.data(); //43485E0F, network43472B40
        registries.entities.publish_shared_esp(entity_receiver); //43485E18, BA8 table4346F2D0
        registries.empty_ticks.publish(empty_tick); //43485E1F, after BA8 table43485E18
        vtable = shared_esp_primary_table.data(); //43485E33, primary43470354
        auxiliary_identity = 0x4346f3b4u; //43485E39, same unregistered identity
        auto& state = bindings.state;
        state.mixing_key = 0xc8eac742u;
        initialize_hotkey_config(configuration, bindings.settings.activation, 0x7174d684u, 27,
                                 HotkeyMode::always); //43485E47
        initialize_boolean_config(configuration.descriptors, bindings.settings.other_team,
                                  0x6834d5dau); //43485E60
        state.marker = 0x2424;
        //Full+78 points to command.bindings.packet_cipher; +7C/+80 identify
        //the already recovered legacy/RC4 source dispatch, without callbacks.
        const auto& globals = bindings.globals;
        state.engine_client = globals.engine_client; //mem_int3.h:564
        state.send_gate_a = decoded_pointer(globals.encoded_send_gate_a, 0x7b3564bbu);
        state.send_gate_b = decoded_pointer(globals.encoded_send_gate_b, 0x584c3c72u);
        state.game_type = decoded_pointer(globals.encoded_game_type, 0xac9299ccu);
        field_b0 = decoded_pointer(globals.encoded_field_b0, 0xfa2dd713u);
        const auto last = globals.encoded_field_b4;
        state.last_player = 1; //43485EDB
        field_b4 = decoded_pointer(last, 0xac34f836u);

        auto* call = globals.message_constructor_call; //mem_int3.h:570
        if (!call)
            return;
        globals.message_constructor_call = nullptr; //recorded43485EF9 MOV, before target read
        state.construct_message = call_target(call); //43485F07
        if (read<std::uint8_t>(call, 0xb2) != 0xe8)
            return;
        state.destroy_message = call_target(call + 0xb2); //43485F28
        call = globals.receiver_constructor_call; //recorded43485F2E MOV
        if (!call)
            return;
        globals.receiver_constructor_call = nullptr; //recorded43485F38 MOV
        state.construct_receiver = call_target(call); //43485F49
        call += 0x28;
        if (read<std::uint8_t>(call, 0) != 0xe8)
            return;
        state.destroy_receiver = call_target(call); //43485F5D
        registries.events.add({0x3d07d291u, this, unchanged_event}); //43485F74
        registries.events.add({0xf03e64e8u, this, reset_event}); //43485F8B
        registries.events.add({0xdd2bbbc2u, this, unchanged_event}); //43485FA2
    }
};
static_assert(sizeof(SharedEspInstance) <= 0x2b8);

void require_identity(const SharedEspOwnerBindings& bindings) {
    const auto& command = bindings.command.bindings;
    const auto& incoming = bindings.incoming;
    if (&bindings.active != &command.owner || &bindings.active != &incoming.owner ||
        &bindings.settings != &command.settings ||
        &bindings.settings.enabled.value != &incoming.receive_enabled ||
        &bindings.settings.restrict_updates.value != &incoming.permission_gate ||
        &command.cache != &incoming.cache || &command.players != &incoming.adjustments ||
        &command.entity_list != &incoming.entity_list ||
        &bindings.globals.engine_client != &incoming.engine)
        throw std::invalid_argument(
            "Shared ESP lifecycle requires its canonical command, incoming, configuration and owner slots");
}
void SHARED_ESP_MEMBER destroy_entry(void* storage) {
    auto& instance = *static_cast<SharedEspInstance*>(storage);
    destroy_shared_esp_owner(storage, instance.bindings.active);
}
void SHARED_ESP_MEMBER shared_esp_forward_shared_esp_reset_entry(void* storage) { //433B06E3
    auto& instance = *static_cast<SharedEspInstance*>(storage);
    instance.vtable[7](storage);
}
void SHARED_ESP_MEMBER shared_esp_reset_entry(void* storage) {
    reset_shared_esp_owner(storage);
}
void SHARED_ESP_FAST message_entry(void*, misc::NetworkMessageCall* call) { //4336F718
    if (engine::invoke<std::int32_t>(call->message, 7) == 10)
        *call->voice = 1;
}
void reset_event(void* storage, events::GameEventReader&,
                 events::EventPlayerIndices&) { //433B30BB ->433B531F
    auto& instance = *static_cast<SharedEspInstance*>(storage);
    for (std::uint32_t index = 1; index <= 64; ++index) {
        auto& geometry = instance.bindings.command.bindings.cache.get(index)->geometry;
        geometry.last_live_tick = 0;
        geometry.alpha = 0.0f;
    }
    instance.bindings.state.timer_84 = 2.0f;
}
void shared_esp_initialize_entry(void* context) {
    auto& source = *static_cast<SharedEspInitialization*>(context);
    create_shared_esp_owner(source.registries, source.configuration, source.bindings);
}
} // namespace

std::size_t shared_esp_owner_size() noexcept {
    return 0x2b8;
}
void* construct_shared_esp_owner(void* storage, const SharedEspOwnerRegistries& registries,
                                 ConfigRegistries& configuration, SharedEspOwnerBindings bindings) {
    require_identity(bindings);
    return ::new (storage) SharedEspInstance(registries, configuration, bindings);
}
void* create_shared_esp_owner(const SharedEspOwnerRegistries& registries,
                              ConfigRegistries& configuration, SharedEspOwnerBindings bindings) {
    require_identity(bindings);
    auto* const storage = allocate(registries.primary.memory);
    if (storage) {
        //43387448's zeroed native allocation covers these borrowed projections.
        //The separate static43467F98/43468598 descriptors are not in this block.
        bindings.state = RejectedPlayerUpdateOwner{};
        bindings.state.mixing_key = 0;
        bindings.state.marker = 0;
        bindings.state.last_player = 0;
        bindings.settings.activation = HotkeyConfig{};
        bindings.settings.other_team = BooleanConfig{};
        construct_shared_esp_owner(storage, registries, configuration, bindings);
        bindings.active = &bindings.state; //4348DA91, mem_int3.h:620
    } else
        bindings.active = nullptr; //raw4348DA8F XOR, same recorded publication
    return storage;
}
void destroy_shared_esp_owner(void* storage, RejectedPlayerUpdateOwner*& active) {
    active = nullptr; //433801D6, before433837D0's null guard
    if (!storage)
        return;
    auto* instance = static_cast<SharedEspInstance*>(storage);
    const auto memory = instance->memory;
    instance->~SharedEspInstance();
    auto* const bytes = static_cast<std::byte*>(storage);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
void reset_shared_esp_owner(void* storage) { //433C0954
    auto& instance = *static_cast<SharedEspInstance*>(storage);
    for (std::uint32_t index = 1; index <= 64; ++index)
        instance.bindings.command.bindings.cache.get(index)->geometry.last_live_tick = 0;
}
void bind_shared_esp_initializer(NativeStartupSources& sources, SharedEspInitialization& context) {
    sources.bind_initializer({0x4348da78u, &context, shared_esp_initialize_entry});
}
#undef SHARED_ESP_MEMBER
#undef SHARED_ESP_FAST
} // namespace skeet::client

// ============================================================================
// Subcomponent: shared_esp_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* [dedup pointed] */
ClientRuntimeStorage& checked(ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
                              ConfigurationRuntimeOwner& configuration, PlayerRuntimeOwner& players,
                              HistoryRuntimeOwner& history,
                              script::SourceScriptRuntimeOwner& scripts) {
    auto& data = storage.data();
    if (&engine.variables.data() != &data || &configuration.storage() != &storage ||
        &players.storage() != &storage || &history.storage() != &storage ||
        &history.players() != &players || &scripts.storage() != &storage)
        throw std::invalid_argument(
            "Shared ESP must borrow the canonical configuration, player, history and script owners");
    if (data.native_cell<RejectedPlayerUpdateOwner*>(0x434758D0))
        throw std::logic_error(
            "Shared ESP contexts must be composed before their actual feature factory");
    const auto* globals = data.native_cell<const std::byte*>(0x43467B04);
    if (!globals ||
        reinterpret_cast<const std::byte*>(data.native_cell<float*>(0x4346A334)) != globals ||
        reinterpret_cast<const std::byte*>(data.native_cell<float*>(0x4346A338)) !=
            globals + 0x20 ||
        reinterpret_cast<const std::byte*>(data.native_cell<std::int32_t*>(0x43469300)) !=
            globals + 0x18 ||
        !data.native_cell<const std::int32_t*>(0x434757DC) ||
        !data.native_cell<void**>(0x43468FF0) || !data.native_cell<void**>(0x4346A3A4))
        throw std::invalid_argument(
            "Shared ESP requires the actual clock, choked-command, resource and game-rules aliases");
    const auto core = engine.interfaces.core();
    if (!core.engine_client || core.engine_client != data.native_cell<void*>(0x4346A514) ||
        !core.entity_list || core.entity_list != data.native_cell<void*>(0x4346A890))
        throw std::invalid_argument(
            "Shared ESP interfaces do not belong to the canonical engine bundle");
    for (const auto cell :
         {0x43469068u, 0x43468D88u, 0x43467A8Cu, 0x43467A4Cu, 0x43468EA8u, 0x4346A568u})
        if (!engine.variables.at(cell).get())
            throw std::invalid_argument("Shared ESP requires its actual acquired ConVars");
    if (!data.native_cell<const std::byte*>(0x4346D668) ||
        !data.native_cell<const std::byte*>(0x4346D66C))
        throw std::invalid_argument(
            "Shared ESP requires the verified engine message CALL producers before construction");
    return storage;
}
/* deduplicated registry helper */
RejectedPlayerUpdateOffsets shared_esp_fields(ClientRuntimeStorage& storage) {
    const auto& g = storage.offsets().globals;
    return {engine::DecodedOffset(g, 0x43468BF8, 0x43467B8C, 0xD070684C, 0),
            engine::DecodedOffset(g, 0x43468E78, 0x434684E4, 0x9D920E35, 0),
            engine::DecodedOffset(g, 0x434670CC, 0x43468BCC, 0xA9EE6062, 0),
            engine::DecodedOffset(g, 0x43467910, 0x43467C08, 0xD720AEA6, 0),
            engine::DecodedOffset(g, 0x43468D6C, 0x43468428, 0x9F8E7F71, 0)};
}
}
struct SharedEspRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    RejectedPlayerUpdateOwner state;
    SharedEspPacketCipher cipher = SharedEspPacketCipher::legacy_xor;
    rage::LiveTargetRelationships relationships;
    RejectedPlayerUpdateContext rejected;
    SharedEspCommandOwner command;
    network::SharedEspRuntime incoming;
    SharedEspConstructorGlobals globals;
    SharedEspOwnerRegistries registries;
    SharedEspInitialization initialization;

    Data(ClientRuntimeStorage& source, const EngineRuntimeBindings& engine,
         ConfigurationRuntimeOwner& configuration, PlayerRuntimeOwner& players,
         HistoryRuntimeOwner& history, script::SourceScriptRuntimeOwner& scripts)
        : storage(checked(source, engine, configuration, players, history, scripts)),
          relationships{source.data().native_cell<void*>(0x4346A890),
                        engine.variables.at(0x43467A8C), engine.variables.at(0x4346A568),
                        players.adjustments(), history.target_offsets()},
          rejected{source.data().native_cell<RejectedPlayerUpdateOwner*>(0x434758D0),
                   configuration.other_esp_settings().shared,
                   source.input_runtime().bitmap(),
                   players.cache(),
                   players.adjustments(),
                   relationships,
                   source.data().native_cell<void*>(0x4346A890),
                   source.data().native_cell<void**>(0x4346A3A4),
                   source.data().native_cell<const std::int32_t*>(0x434757DC),
                   pointed<std::int32_t>(source.data(), 0x43469300),
                   pointed<float>(source.data(), 0x4346A334),
                   pointed<float>(source.data(), 0x4346A338),
                   *reinterpret_cast<const float*>(
                       source.data().native_cell<const std::byte*>(0x43467B04) + 0x20),
                   cipher,
                   shared_esp_fields(source)},
          command{rejected},
          incoming{source.data().native_cell<RejectedPlayerUpdateOwner*>(0x434758D0),
                   source.data().native_cell<void*>(0x43475808),
                   source.data().native_cell<void*>(0x4346A514),
                   source.data().native_cell<void*>(0x4346A890),
                   source.data().native_cell<void**>(0x43468FF0),
                   source.data().native_cell<void**>(0x4346A3A4),
                   engine.variables.at(0x43467A8C),
                   engine.variables.at(0x4346A568),
                   players.adjustments(),
                   players.cache(),
                   configuration.other_esp_settings().shared.enabled.value,
                   configuration.other_esp_settings().shared.restrict_updates.value,
                   pointed<float>(source.data(), 0x4346A338),
                   *reinterpret_cast<const float*>(
                       source.data().native_cell<const std::byte*>(0x43467B04) + 0x20),
                   scripts.bridge(),
                   source.event_names().alias(events::EventName::voice),
                   network::shared_esp_offsets(source.offsets())},
          globals{source.data().native_cell<void*>(0x4346A514),
                  source.data().native_cell<std::uint32_t>(0x43469068),
                  source.data().native_cell<std::uint32_t>(0x43468D88),
                  source.data().native_cell<std::uint32_t>(0x43467A8C),
                  source.data().native_cell<std::uint32_t>(0x43467A4C),
                  source.data().native_cell<std::uint32_t>(0x43468EA8),
                  source.data().native_cell<const std::byte*>(0x4346D668),
                  source.data().native_cell<const std::byte*>(0x4346D66C)},
          registries{registry(source, 0x43475B78), source.command_phases(),
                     registry(source, 0x43475B94), source.entity_observers(),
                     source.empty_ticks(),         source.event_registry()},
          initialization{registries,
                         source.configuration(),
                         {state, configuration.other_esp_settings().shared, command, incoming,
                          source.data().native_cell<RejectedPlayerUpdateOwner*>(0x434758D0),
                          globals}} {}
};
SharedEspRuntimeOwner::SharedEspRuntimeOwner(ClientRuntimeStorage& storage,
                                             const EngineRuntimeBindings& engine,
                                             ConfigurationRuntimeOwner& configuration,
                                             PlayerRuntimeOwner& players,
                                             HistoryRuntimeOwner& history,
                                             script::SourceScriptRuntimeOwner& scripts)
    : data_(std::make_unique<Data>(storage, engine, configuration, players, history, scripts)) {}
SharedEspRuntimeOwner::~SharedEspRuntimeOwner() = default;
ClientRuntimeStorage& SharedEspRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
SharedEspInitialization& SharedEspRuntimeOwner::initializers() noexcept {
    return data_->initialization;
}
const RejectedPlayerUpdateContext& SharedEspRuntimeOwner::rejected_player_update() const noexcept {
    return data_->rejected;
}
network::SharedEspRuntime& SharedEspRuntimeOwner::incoming() noexcept {
    return data_->incoming;
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: shutdown.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

extern "C" {
skeet::client::ShutdownImports::UnmapSection skeet_shutdown_unmap = nullptr;
}

namespace skeet::client {
namespace {
ShutdownRuntime* active_runtime = nullptr;
int WINAPI native_kill_timer(std::uintptr_t window, std::uintptr_t timer) {
    return KillTimer(reinterpret_cast<HWND>(window), static_cast<UINT_PTR>(timer));
}
}
ShutdownImports native_shutdown_imports() {
    const auto native = GetModuleHandleA("ntdll.dll");
    const auto query = native ? GetProcAddress(native, "ZwQueryVirtualMemory") : nullptr;
    const auto unmap = native ? GetProcAddress(native, "ZwUnmapViewOfSection") : nullptr;
    if (!query || !unmap)
        throw std::runtime_error("Required native finalization export is missing");
    return {&native_kill_timer, reinterpret_cast<ShutdownImports::QueryMemory>(query),
            reinterpret_cast<ShutdownImports::UnmapSection>(unmap)};
}
void ShutdownRuntime::destroy_gui() { // 0x43336C51
    input.windows = nullptr;
    if (input.main_menu) {
        if (source_main_window)
            source_main_window->release_gui(window_title, window_imports);
        else
            ui::destroy_main_menu_window(*input.main_menu, *main_callbacks, window_title,
                                         window_imports);
        input.main_menu = nullptr;
    }
    if (renderer) {
        renderer->destroy();
        if (source_renderer_cell && *source_renderer_cell == renderer)
            *source_renderer_cell = nullptr;
        renderer = nullptr;
    }
    input.destroy_cursor();
}
void* ShutdownRuntime::finalize() { // 0x433C87F6
    restore_engine_dispatch();
    script::notify_shutdown(scripts.bridge);
    scripting_enabled = false;
    script::destroy_runtime(scripts);
    if (source_main_window)
        source_main_window->release_controller();
    release_cleanup_objects();
    destroy_gui();
    exit_callbacks.run();
    MemoryRegion region{};
    (void)imports.query_memory(reinterpret_cast<void*>(~std::uintptr_t(0)),
                               reinterpret_cast<const void*>(&finalize_client), 0, &region,
                               sizeof(region), nullptr);
    return region.allocation_base;
}
void ShutdownRuntime::detach() { // 0x433EF91A
    (void)__atomic_exchange_n(&input.disabled, 1, __ATOMIC_SEQ_CST);
    hooks.state.unloading = true; // 433EF926, before cursor/engine/Lua teardown
    input.release_cursor();
    input.destroy_cursor();
    if (input.main_menu) {
        input.main_menu->transition_state = 0;
        input.main_menu->input.mouse_flags &= ~std::uint32_t(1);
    }
    hooks.cleanup();
    script::notify_shutdown(scripts.bridge);
    scripting_enabled = false;
    script::destroy_runtime(scripts);
    hooks.restore_window();
    restore_engine_dispatch();
    if (renderer && renderer->backend_if_alive())
        renderer->release_context();
    if (source_main_window)
        source_main_window->release_controller();
    release_cleanup_objects();
    destroy_gui();
    exit_callbacks.run();
}
void bind_shutdown_runtime(ShutdownRuntime& runtime) noexcept {
    active_runtime = &runtime;
    skeet_shutdown_unmap = runtime.imports.unmap_section;
}
ShutdownRuntime* bound_shutdown_runtime() noexcept {
    return active_runtime;
}
void unbind_shutdown_runtime(ShutdownRuntime& runtime) noexcept {
    if (active_runtime == &runtime) {
        active_runtime = nullptr;
        skeet_shutdown_unmap = nullptr;
    }
}
void* finalize_client() {
    return active_runtime->finalize();
}
void detach_client() {
    active_runtime->detach();
}
}

extern "C" void* skeet_shutdown_timer_finalize() {
    auto& runtime = *skeet::client::bound_shutdown_runtime();
    runtime.cancel_timer();
    return skeet::client::finalize_client();
}

namespace skeet::misc {
#if defined(__GNUC__) || defined(__clang__)
__attribute__((naked)) void __stdcall unload_timer_callback(void*, std::uint32_t, std::uintptr_t,
                                                            std::uint32_t) {
#if defined(__x86_64__)
    __asm__("subq $40, %rsp\n\t"
            "call skeet_shutdown_timer_finalize\n\t"
            "addq $40, %rsp\n\t"
            "movq %rax, %rdx\n\t"
            "movq $-1, %rcx\n\t"
            "jmp *skeet_shutdown_unmap(%rip)");
#elif defined(__i386__)
    __asm__("call _skeet_shutdown_timer_finalize\n\t"
            "popl %edx\n\t"
            "addl $16, %esp\n\t"
            "pushl %eax\n\t"
            "pushl $-1\n\t"
            "pushl %edx\n\t"
            "jmp *_skeet_shutdown_unmap");
#else
#error The unload tail jump requires an x86 or x64 target.
#endif
}
#elif defined(_MSC_VER) && defined(_M_IX86)
__declspec(naked) void __stdcall unload_timer_callback(void*, std::uint32_t, std::uintptr_t,
                                                       std::uint32_t) {
    __asm {
        call skeet_shutdown_timer_finalize
        pop edx
        add esp,16
        push eax
        push -1
        push edx
        jmp dword ptr [skeet_shutdown_unmap]
    }
}
#else
#error The unload callback needs a compiler supporting the verified x86/x64 tail-jump body.
#endif
}

// ============================================================================
// Subcomponent: shutdown_helpers.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::client {
namespace {
void terminate_exit_callback_overflow() {
    using NativeTerminate = std::int32_t(__stdcall*)(void*, std::int32_t);
    const auto native = GetModuleHandleA("ntdll.dll");
    const auto address = native ? GetProcAddress(native, "ZwTerminateProcess") : nullptr;
    if (!address)
        throw std::runtime_error("Required ntdll!ZwTerminateProcess export is missing");
    (void)reinterpret_cast<NativeTerminate>(address)(reinterpret_cast<void*>(~std::uintptr_t(0)),
                                                     -3);
}
}
int ExitCallbacks::append(void (*callback)()) { // 0x433837F5
    if (entries.size() != 0x200)
        throw std::invalid_argument("Exit registration requires its512-slot backing store");
    if (!count)
        xor_mask = static_cast<std::uint32_t>(__rdtsc());
    else if (count == 0x200) {
        terminate_exit_callback_overflow();
        return -1;
    }
    const auto encoded = reinterpret_cast<std::uintptr_t>(callback) ^ xor_mask;
    for (std::uint32_t index = 0; index < count; ++index)
        if (entries[index] == encoded)
            return 0;
    entries[count] = encoded;
    ++count;
    return 0;
}
void ExitCallbacks::run() { // 0x433B3987
    const auto mask = xor_mask;
    while (count != 0) {
        const auto index = --count;
        entries[index] ^= mask;
        reinterpret_cast<void (*)()>(entries[index])();
        entries[index] = 0;
    }
    count = UINT32_MAX;
}
void ShutdownRuntime::restore_engine_dispatch() { // 0x43365AA1
    if (source_channel_) {
        if (!source_channel_->replacement_table)
            return;
        auto* const channel = engine::invoke<void*>(input.interfaces.engine_client, 78);
        if (channel && *static_cast<void***>(channel) == source_channel_->replacement_table)
            InterlockedExchangePointer(reinterpret_cast<void* volatile*>(channel),
                                       source_channel_->expected_table);
        return;
    }
    if (!installed_engine_dispatch)
        return;
    auto* slot = engine::invoke<std::uint32_t*>(input.interfaces.engine_client, 78);
    if (slot && *slot == installed_engine_dispatch)
        InterlockedExchange(reinterpret_cast<volatile LONG*>(slot),
                            static_cast<LONG>(saved_engine_dispatch));
}
void ShutdownRuntime::release_cleanup_objects() { // 0x4339D451
    const auto objects = owners.entries();
    for (auto* object : objects)
        engine::invoke<void>(object, 0);
}
void ShutdownRuntime::cancel_timer() { // 0x433D7307
    if (!native_window)
        throw std::invalid_argument("Client shutdown main window is unresolved");
    (void)imports.kill_timer(native_window, 0x332);
}
}

// ============================================================================
// Subcomponent: shutdown_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
ClientShutdownImports native_client_shutdown_imports() {
    return {native_shutdown_imports(), ui::native_window_lifetime_imports(), {}};
}
namespace {
ClientRuntimeStorage& require_owners(ClientRuntimeStorage& storage, WindowRuntimeOwner& hooks,
                                     ui::MainMenuWindowOwner& window,
                                     ConfigStorageOwner& configuration,
                                     script::SourceScriptRuntimeOwner& scripts,
                                     ui::StartupRenderer& renderer,
                                     const ClientShutdownImports& imports) {
    auto& data = storage.data();
    if (&hooks.storage() != &storage || &window.menu_data() != &storage.menu_startup_data() ||
        &configuration.native_storage() != &storage.config_storage() ||
        &configuration.configuration() != &storage.configuration() ||
        &scripts.storage() != &storage || &scripts.unload_state() != &hooks.unload_state() ||
        &renderer.dpi() != &storage.menu_startup_data().dpi ||
        data.native_cell<ui::RendererContainer*>(0x4347690Cu) != &renderer.renderer() ||
        storage.input_runtime().main_menu != &window.window() ||
        scripts.shutdown_state().ui != &scripts.ui_bindings() ||
        &scripts.shutdown_state().bridge != &scripts.bridge())
        throw std::invalid_argument("Source shutdown requires the canonical process owners");
    if (!imports.finalization.kill_timer || !imports.finalization.query_memory ||
        !imports.window.free_unicode_string || !imports.request.set_timer)
        throw std::invalid_argument("Source shutdown requires its actual lifecycle imports");
    return storage;
}
}
struct ClientShutdownOwner::Data {
    ClientRuntimeStorage& storage;
    ChannelHookState channel;
    ShutdownRuntime finalizer;
    misc::UnloadRuntime request;
    std::unique_ptr<SourceDllShutdownProfile> profile;
    Data(ClientRuntimeStorage& source, WindowRuntimeOwner& hooks, ui::MainMenuWindowOwner& window,
         ConfigStorageOwner& configuration, script::SourceScriptRuntimeOwner& scripts,
         ui::StartupRenderer& renderer, ClientShutdownImports imports)
        : storage(require_owners(source, hooks, window, configuration, scripts, renderer, imports)),
          channel(storage.data().native_cell<void**>(0x43475B0Cu),
                  storage.data().native_cell<void**>(0x43475B14u),
                  storage.data().native_cell<void*>(0x43475B08u)),
          finalizer(scripts.shutdown_state(), storage.input_runtime(), imports.finalization,
                    scripts.runtime().enabled, hooks.unload_runtime(), storage.exit_callbacks(),
                    storage.config_storage().path(), window.window(), channel),
          request(window.window(), scripts.ui_bindings(), config_loading_flag()) {
        finalizer.main_callbacks = &window.callbacks();
        finalizer.window_imports = imports.window;
        finalizer.renderer = &renderer.renderer();
        finalizer.source_main_window = &window;
        finalizer.source_renderer_cell =
            &storage.data().native_cell<ui::RendererContainer*>(0x4347690Cu);
        request.config_registry = &storage.configuration().descriptors;
        request.baseline_config =
            ConfigStreamView::native_vector(storage.config_storage().baseline_cells());
        request.current_config = ConfigStreamView::script_state(scripts.shutdown_state());
        request.input = &storage.input_runtime();
        request.hooks = &hooks.unload_runtime();
        request.imports = imports.request;
        request.timer_callback = nullptr;
    }
    ~Data() {
        unbind_shutdown_runtime(finalizer);
    }
};
ClientShutdownOwner::ClientShutdownOwner(ClientRuntimeStorage& storage, WindowRuntimeOwner& hooks,
                                         ui::MainMenuWindowOwner& window,
                                         ConfigStorageOwner& configuration,
                                         script::SourceScriptRuntimeOwner& scripts,
                                         ui::StartupRenderer& renderer)
    : ClientShutdownOwner(storage, hooks, window, configuration, scripts, renderer,
                          native_client_shutdown_imports()) {}
ClientShutdownOwner::ClientShutdownOwner(ClientRuntimeStorage& storage, WindowRuntimeOwner& hooks,
                                         ui::MainMenuWindowOwner& window,
                                         ConfigStorageOwner& configuration,
                                         script::SourceScriptRuntimeOwner& scripts,
                                         ui::StartupRenderer& renderer,
                                         ClientShutdownImports imports)
    : data_(std::make_unique<Data>(storage, hooks, window, configuration, scripts, renderer,
                                   imports)) {}
ClientShutdownOwner::~ClientShutdownOwner() = default;
ClientRuntimeStorage& ClientShutdownOwner::storage() const noexcept {
    return data_->storage;
}
ShutdownRuntime& ClientShutdownOwner::finalizer() noexcept {
    return data_->finalizer;
}
misc::UnloadRuntime& ClientShutdownOwner::request() noexcept {
    return data_->request;
}
void ClientShutdownOwner::select_source_dll(void* module) {
    select_source_dll(module, native_source_dll_shutdown_imports(), {});
}
void ClientShutdownOwner::select_source_dll(void* module, SourceDllShutdownImports imports) {
    select_source_dll(module, imports, {});
}
void ClientShutdownOwner::select_source_dll(void* module, SourceDllShutdownCompletion completion) {
    select_source_dll(module, native_source_dll_shutdown_imports(), completion);
}
void ClientShutdownOwner::select_source_dll(void* module, SourceDllShutdownImports imports,
                                            SourceDllShutdownCompletion completion) {
    if (data_->profile)
        throw std::logic_error("Source DLL shutdown is already selected");
    auto profile =
        std::make_unique<SourceDllShutdownProfile>(data_->finalizer, module, imports, completion);
    bind_source_dll_shutdown(*profile, data_->request);
    data_->profile = std::move(profile);
}
void ClientShutdownOwner::cancel_deferred_request() noexcept {
    if (data_->profile)
        data_->profile->cancel_deferred_request();
}
}

// ============================================================================
// Subcomponent: skin_runtime_factories.cpp
// ============================================================================

namespace skeet::client {
namespace {

void require_bindings(const SkinRuntimeFactoryBindings& b) {
    const auto& image = b.entity_updates.image();
    if (!b.lifecycle.memory.allocate || !b.lifecycle.memory.free ||
        !b.lifecycle.memory.process_heap)
        throw std::invalid_argument("Skin factories require the canonical process heap");
    if (&b.lifecycle.registry != &image.native_cell<engine::OwnerRegistry>(0x43475B78u) ||
        &b.commands.image() != &image ||
        &b.commands.native_storage() != &image.native_cell<engine::OwnerRegistry>(0x434757A0u) ||
        &b.entity_updates.native_storage() !=
            &image.native_cell<engine::OwnerRegistry>(0x43475BA8u) ||
        &b.agent.observers.image() != &image ||
        &b.agent.observers.native_storage() !=
            &image.native_cell<engine::OwnerRegistry>(0x43475760u))
        throw std::invalid_argument(
            "Skin factories require their canonical owner and observer registries");
    if (&b.configuration.descriptors != &image.native_cell<ConfigLink>(0x434798BCu) ||
        &b.configuration.hotkeys != &image.native_cell<HotkeyLink>(0x434798C4u) ||
        &b.inventory.weapons != &b.settings.weapons || &b.cosmetics.settings != &b.settings ||
        &b.cosmetics.cosmetics.settings() != &b.settings || &b.agent.settings != &b.settings)
        throw std::invalid_argument(
            "Skin factories must share the canonical configuration and Settings");
    const auto& receive = b.agent.receiver.receive_bindings();
    const auto& capture = b.agent.receiver.capture_globals();
    if (&b.inventory.engine_client != &image.native_cell<void*>(0x4346A514u) ||
        &b.cosmetics.engine_client != &b.inventory.engine_client ||
        &receive.engine_client != &b.inventory.engine_client ||
        &b.inventory.entity_list != &image.native_cell<void*>(0x4346A890u) ||
        &b.agent.entity_list != &b.inventory.entity_list ||
        &b.cosmetics.local_player != &image.native_cell<void*>(0x43475808u) ||
        &b.agent.offsets != &b.inventory.offsets ||
        &capture.original_handle != &image.native_cell<std::uint32_t>(0x4346D648u) ||
        &capture.original_model != &image.native_cell<std::int32_t>(0x4346D650u))
        throw std::invalid_argument(
            "Skin factories must share their live engine and agent capture cells");
}

void require_fresh(std::initializer_list<const ConfigLink*> descriptors) {
    for (const auto* descriptor : descriptors)
        if (descriptor->next || descriptor->previous)
            throw std::invalid_argument("Skin factory descriptors must be fresh and unlinked");
}
template <class T> void zero_descriptor(ConfigValue<T>& descriptor) {
    descriptor.next = descriptor.previous = nullptr;
    descriptor.hash = 0;
    descriptor.serialized_size = 0;
    descriptor.type = static_cast<ConfigType>(0);
    descriptor.reserved = 0;
    descriptor.value = {};
}
void* allocate_owner(engine::JobMemory memory, std::size_t native_size, std::size_t source_size) {
    const auto size = std::max(native_size, source_size);
    auto* storage = engine::allocate_owner_storage(memory, size); //43387448
    // Each original null edge enters an unresolved INT1 continuation. The
    // source failure policy is bad_alloc; it does not emulate that transfer.
    if (!storage)
        throw std::bad_alloc{};
    return storage;
}
void inventory_entry(void* context) {
    (void)create_skin_inventory_owner(*static_cast<SkinRuntimeFactoryBindings*>(context));
}
void glove_entry(void* context) {
    (void)create_skin_glove_owner(*static_cast<SkinRuntimeFactoryBindings*>(context));
}
void mask_entry(void* context) {
    (void)create_skin_mask_owner(*static_cast<SkinRuntimeFactoryBindings*>(context));
}
void agent_entry(void* context) {
    (void)create_skin_agent_owner(*static_cast<SkinRuntimeFactoryBindings*>(context));
}
}

skins::InventoryLifecycleOwner* create_skin_inventory_owner(const SkinRuntimeFactoryBindings& b) {
    require_bindings(b);
    require_fresh({&b.settings.weapons});
    auto* storage = allocate_owner(b.lifecycle.memory, skin_inventory_native_size,
                                   skins::inventory_owner_size());
    // Native calloc zeros the inline records before the constructor appends
    // and initializes them. Project those zeros into the same borrowed view.
    zero_descriptor(b.settings.weapons);
    for (auto& record : b.settings.weapons.value)
        record = {0, 0, 0, 0, 0, 0};
    return &skins::construct_inventory_owner(storage, b.lifecycle, b.commands, b.entity_updates,
                                             b.configuration, b.inventory); //4347E82A ->434902DF
}
skins::GloveOwner* create_skin_glove_owner(const SkinRuntimeFactoryBindings& b) {
    require_bindings(b);
    auto& s = b.settings;
    require_fresh({&s.glove_t, &s.glove_ct, &s.glove_model_t, &s.glove_model_ct, &s.glove_paint_t,
                   &s.glove_paint_ct});
    auto* storage =
        allocate_owner(b.lifecycle.memory, skin_glove_native_size, skins::glove_owner_size());
    zero_descriptor(s.glove_t);
    zero_descriptor(s.glove_ct);
    zero_descriptor(s.glove_model_t);
    zero_descriptor(s.glove_model_ct);
    zero_descriptor(s.glove_paint_t);
    zero_descriptor(s.glove_paint_ct);
    return &skins::construct_glove_owner(storage, {b.lifecycle, b.entity_updates}, b.configuration,
                                         b.cosmetics);
}
skins::MaskOwner* create_skin_mask_owner(const SkinRuntimeFactoryBindings& b) {
    require_bindings(b);
    auto& s = b.settings;
    require_fresh({&s.mask_t, &s.mask_ct, &s.mask_model_t, &s.mask_model_ct});
    auto* storage =
        allocate_owner(b.lifecycle.memory, skin_mask_native_size, skins::mask_owner_size());
    zero_descriptor(s.mask_t);
    zero_descriptor(s.mask_ct);
    zero_descriptor(s.mask_model_t);
    zero_descriptor(s.mask_model_ct);
    return &skins::construct_mask_owner(storage, {b.lifecycle, b.entity_updates}, b.configuration,
                                        b.cosmetics);
}
void* create_skin_agent_owner(const SkinRuntimeFactoryBindings& b) {
    require_bindings(b);
    auto& s = b.settings;
    require_fresh({&s.agent_t, &s.agent_ct, &s.agent_model_t, &s.agent_model_ct});
    auto* storage =
        allocate_owner(b.lifecycle.memory, skin_agent_native_size, skins::agent_owner_size());
    zero_descriptor(s.agent_t);
    zero_descriptor(s.agent_ct);
    zero_descriptor(s.agent_model_t);
    zero_descriptor(s.agent_model_ct);
    skins::construct_agent_owner(storage, b.lifecycle, b.configuration, b.agent);
    return storage;
}
void bind_skin_runtime_factories(NativeStartupSources& sources,
                                 SkinRuntimeFactoryBindings& bindings) {
    require_bindings(bindings);
    if (&sources.data() != &bindings.entity_updates.image())
        throw std::invalid_argument("Skin factories must share the startup canonical data owner");
    const std::array calls{
        NativeInitializerCall{skin_inventory_factory, &bindings, inventory_entry},
        NativeInitializerCall{skin_glove_factory, &bindings, glove_entry},
        NativeInitializerCall{skin_mask_factory, &bindings, mask_entry},
        NativeInitializerCall{skin_agent_factory, &bindings, agent_entry}};
    for (const auto& call : calls)
        sources.bind_initializer(call);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: source_application.cpp
// ============================================================================

namespace skeet::client {
struct SourceApplication::Data {
    void* module;
    SourceDllShutdownCompletion completion;
    ClientRuntimeStorage storage;
    CrtRuntimeOwner crt;
    ConfigurationRuntimeOwner configuration;
    WindowRuntimeOwner window;
    PlayerRuntimeOwner players;
    script::RequiredModuleStorage script_modules;
    script::SourceScriptRuntimeOwner scripts;
    ui::RendererAssets assets;
    StaticCrtRuntimeOwner static_crt;
    animation::FixAnimationBackupCleanupContext animation_cleanup;
    NativeStartupSources sources;
    NativeStartupRuntime startup;
    std::optional<EngineRuntimeBindings> engine;
    std::unique_ptr<ui::StartupRenderer> renderer;
    std::unique_ptr<ui::MainMenuWindowOwner> main_window;
    std::unique_ptr<ConfigStorageOwner> presets;
    std::unique_ptr<ClientShutdownOwner> shutdown;
    std::unique_ptr<script::SourceScriptExecutableImage> executable_image;
    std::unique_ptr<SourceFeatureRuntime> features;
    std::unique_ptr<FinalMenuStartupTail> menu_tail;
    std::unique_ptr<PostInitializationOwner> post;
    bool started = false, complete = false, cleanup_started = false, cleanup_complete = false;
    bool constructing_features = false;
    bool focus_publication_started = false;
    const char* stage = "source CRT initialization";

    NativeInitializerCall final_initializer() noexcept {
        return {0x43484F06u, this, initialize_final};
    }
    static void initialize_final(void* context) {
        static_cast<Data*>(context)->initialize_engine_and_features();
    }

    Data(void* source_module, SourceDllShutdownCompletion final_completion)
        : module(source_module), completion(final_completion), crt(storage), configuration(storage),
          window(storage), players(storage),
          scripts(storage, window.unload_state(), crt.ui_types(), script_modules.builtins(),
                  script_modules.modules()),
          assets(storage),
          // The native inline containers retained unconsumed incoming ECX in
          // these two words. The source containers initialize unused storage
          // deterministically; neither word supplies an engine or feature input.
          static_crt(crt, window, assets, {0, 0}), animation_cleanup(storage),
          sources(storage.data(), final_initializer()),
          startup(sources, native_startup_runtime_imports()) {
        if (!module)
            throw std::invalid_argument("Source application requires its actual DLL module");
        script_modules.encrypt_for(scripts.runtime());
        bind_early_crt_sources(sources, crt, configuration, static_crt, scripts, animation_cleanup,
                               scripts.source_terminal_cleanup(players));
    }

    void initialize_engine_and_features() {
        stage = "engine interfaces, offsets and module bindings";
        const auto& imports = window.startup_imports();
        engine.emplace(
            storage.initialize_engine_bindings(imports, static_cast<std::uint32_t>(__rdtsc())));
        stage = "renderer and main window";
        renderer = std::make_unique<ui::StartupRenderer>(
            storage.data_owner(), storage.menu_startup_data().dpi, assets.device_listener());
        assets.bind_renderer(*renderer);
        main_window = std::make_unique<ui::MainMenuWindowOwner>(storage);
        presets = std::make_unique<ConfigStorageOwner>(
            storage.configuration(), storage.config_storage_owner(), main_window->window(),
            scripts.bridge(), scripts.ui_bindings(), storage.menu_startup_data().settings_values(),
            storage.menu_startup_data().script_values());
        shutdown = std::make_unique<ClientShutdownOwner>(storage, window, *main_window, *presets,
                                                         scripts, *renderer);
        shutdown->select_source_dll(module, completion);
        stage = "engine hook descriptors";
        engine::prepare_build_hook_descriptors(crt.descriptor_storage(), storage.memory(),
                                               imports.images, crt.hook_system());
        stage = "Lua image, material and skin bindings";
        executable_image =
            std::make_unique<script::SourceScriptExecutableImage>(storage.data_owner());
        const auto materials = engine::initialize_build_material_bindings(
            storage.data(), imports.modules, imports.images);
        const auto skin_engine =
            engine::initialize_build_skin_bindings(storage.data(), imports.images);
        stage = "menu cursor and input interfaces";
        storage.bind_input_interfaces();
        stage = "source feature contexts";
        features = std::make_unique<SourceFeatureRuntime>(
            SourceFeatureInputs{storage, *engine, crt, configuration, static_crt, window, players,
                                scripts, animation_cleanup, assets, *renderer, *main_window,
                                *presets, *shutdown, materials, skin_engine, final_initializer()});
        stage = "57 feature factories";
        constructing_features = true;
        features->initialize_features(sources);
        constructing_features = false;
        stage = "menu and runtime hook contexts";
        (void)ui::initialize_main_menu_config_path(storage.config_storage());
        auto& menu = features->compose_menu();
        features->bind_hook_contexts();
        features->menu_settings().capture_default_layout(menu);
        stage = "saved menu settings";
        menu_tail = std::make_unique<FinalMenuStartupTail>(
            storage, *main_window, features->menu_settings(), *presets, *renderer,
            ui::native_startup_registry_imports());
        menu_tail->initialize();
        stage = "post initialization contexts and feature callbacks";
        post = std::make_unique<PostInitializationOwner>(
            storage, *engine, *main_window, *renderer, assets.icon_cache(), assets.startup_icons(),
            window.subclass(), *presets, scripts.require_owner(), shutdown->finalizer());
        window.bind_subclass_callback();
        initialize_final_feature_callbacks(storage);
        stage = "client class and receive proxy patches";
        capture_final_startup_patches(storage, features->player_receives(),
                                      features->visual_receives(), features->skins(),
                                      features->class_factories());
        stage = "SDK user message bindings";
        network::prepare_startup_user_message_patches(storage.data());
        stage = "final engine and Steam overlay bindings";
        finish_final_engine_startup(storage, features->hooks());
        // Source concurrency policy: publish the focus callback last. Its
        // first engine invocation runs4341F57F's2/7 passes, startup scripts
        // and the window subclass. Those operations must not race the final
        // primary3 pass or expose a menu while final patch capture can fail.
        // The original initializer installed focus earlier; no final-tail
        // operation depends on that receiver already being hooked.
        stage = "focus overlay publication";
        focus_publication_started = true;
        features->hooks().initialize_focus(*post, features->visual());
        complete = true;
    }
};

SourceApplication::SourceApplication(void* module, SourceDllShutdownCompletion completion)
    : data_(std::make_unique<Data>(module, completion)) {}
SourceApplication::~SourceApplication() = default;
void SourceApplication::initialize(std::uintptr_t argument) {
    if (data_->started)
        throw std::logic_error("Source application initialization has already started");
    data_->started = true;
    try {
        (void)data_->startup.initialize(argument);
    } catch (const std::exception& error) {
        throw std::runtime_error(std::string(data_->stage) + ": " + error.what());
    }
    if (!data_->complete)
        throw std::logic_error("Source final initialization did not complete");
}
bool SourceApplication::initialized() const noexcept {
    return data_->complete;
}
bool SourceApplication::cancel_initialization() noexcept {
    auto& data = *data_;
    if (data.complete)
        return false;
    if (data.cleanup_started)
        return data.cleanup_complete;
    data.cleanup_started = true;
    try {
        if (data.shutdown) {
            data.shutdown->cancel_deferred_request();
            data.shutdown->finalizer().cancel_timer();
            // A factory can publish its vtable before a later field's C++
            // constructor throws. Executable callback addresses alone cannot
            // certify that receiver's destructor is safe to dispatch.
            if (data.constructing_features || data.focus_publication_started)
                return false;
            if (!source_startup_detach_ready(data.storage, data.window))
                return false;
            data.shutdown->finalizer().detach();
        } else {
            if (!source_exit_callbacks_ready(data.storage))
                return false;
            data.storage.exit_callbacks().run();
        }
        data.cleanup_complete = true;
    } catch (...) {
        // The registry can contain already-destroyed owners after a throwing
        // callback. Keeping the owning application resident prevents a second
        // cleanup attempt and dangling engine callbacks into an unloaded DLL.
    }
    return data.cleanup_complete;
}
ClientRuntimeStorage& SourceApplication::storage() noexcept {
    return data_->storage;
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: source_feature_runtime.cpp
// ============================================================================

namespace skeet::client {
namespace {
SourceFeatureInputs checked(SourceFeatureInputs inputs) {
    auto& s = inputs.storage;
    if (&inputs.engine.variables.data() != &s.data() || &inputs.crt.storage() != &s ||
        &inputs.configuration.storage() != &s || &inputs.static_crt.storage() != &s ||
        &inputs.window.storage() != &s || &inputs.players.storage() != &s ||
        &inputs.scripts.storage() != &s || &inputs.assets.storage() != &s ||
        &inputs.shutdown.storage() != &s || &inputs.materials.data != &s.data() ||
        &inputs.skin_engine.data != &s.data() ||
        &inputs.presets.native_storage() != &s.config_storage() ||
        &inputs.main_window.menu_data() != &s.menu_startup_data() ||
        &inputs.animation_cleanup.callbacks != &s.exit_callbacks() ||
        &inputs.scripts.unload_state() != &inputs.window.unload_state() ||
        &inputs.assets.startup_renderer() != &inputs.renderer)
        throw std::invalid_argument(
            "Feature assembly requires the one canonical process and engine lifetime");
    if (inputs.final_initializer.native_address != 0x43484F06u || !inputs.final_initializer.owner ||
        !inputs.final_initializer.invoke)
        throw std::invalid_argument(
            "Feature assembly requires its actual complete final startup operation");
    if (!s.exit_callbacks().count)
        throw std::logic_error("Feature assembly must follow the real process CRT initialization");
    return inputs;
}
/* deduplicated registry helper */
SourceFeatureRuntimeImports native_imports(const SourceFeatureInputs& in) {
    return {native_history_runtime_imports(), native_misc_runtime_imports(in.engine),
            native_packet_runtime_imports(in.storage, in.engine),
            anti_aim::native_runtime_imports(in.storage, in.engine),
            script::native_script_api_runtime_imports(in.engine)};
}
}

struct SourceFeatureRuntime::Data {
    SourceFeatureInputs inputs;
    engine::SceneQueryRuntimeOwner scene;
    animation::RuntimeOwner bones;
    HistoryRuntimeOwner history;
    VisualRuntimeOwner visual;
    effects::EffectsRuntimeOwner effects;
    effects::CameraRuntimeOwner camera;
    effects::PlayerOpacityOwner opacity;
    engine::HookRuntimeOwner hooks;
    animation::FixRuntimeOwner fix;
    CommandCaptureRuntimeOwner capture;
    LocalAnimationRuntimeOwner local;
    MovementHudRuntimeOwner movement;
    TargetRefreshRuntimeOwner target_refresh;
    SharedEspRuntimeOwner shared_esp;
    CommandRuntimeOwner commands;
    esp::EspRuntimeOwner esp;
    rage::RuntimeOwner rage;
    legit::RuntimeOwner legit;
    MiscRuntimeOwner miscellaneous;
    script::ScriptApiRuntimeOwner script_api;
    skins::SkinsRuntimeOwner skins;
    PacketRuntimeOwner packet;
    anti_aim::RuntimeOwner anti_aim;
    ExploitRuntimeOwner exploit;
    misc::SettingsRuntimeOwner settings;
    PredictionHistoryRuntimeOwner prediction_history;
    EntityCommandRuntimeOwner entity_commands;
    engine::ClientClassFactoryRuntime class_factories;
    misc::ViewmodelRuntimeOwner viewmodel;
    effects::FreeLookRuntimeOwner free_look;
    TargetMaskRuntimeOwner target_mask;
    rage::FeedbackRuntimeOwner feedback;
    BufferedShotRuntimeOwner buffered_shots;
    rage::PredictionMaskRuntimeOwner prediction_masks;
    PlayerReceiveOwner player_receives;
    effects::VisualReceiveOwner visual_receives;
    players::MenuBindings player_menu;
    FeatureRuntimeOwner features;
    engine::MapGameSystemRuntime map_system;
    network::PacketDispatchRuntime packets;
    network::UserMessageRuntime user_messages;
    std::unique_ptr<engine::PlayerHookFamily> player_hooks;
    std::unique_ptr<CombatRuntimeOwner> combat;
    std::unique_ptr<WorldRuntimeOwner> world;
    std::unique_ptr<hud::SimpleHudRuntimeOwner> simple_hud;
    std::unique_ptr<NativeStartupSourceGraph> graph;
    bool factories_started = false, hook_contexts_bound = false;

    Data(SourceFeatureInputs in, const SourceFeatureRuntimeImports& imports)
        : inputs(checked(in)), scene(in.storage, in.engine, in.players),
          bones(in.storage, in.engine, in.players.cache()),
          history(in.storage, in.engine, in.configuration, in.players, scene, bones, in.scripts,
                  imports.history),
          visual(in.storage, in.engine, in.configuration, in.renderer, in.assets),
          effects(in.storage, in.engine, in.configuration, visual, in.materials),
          camera(in.storage, in.engine, in.configuration, in.players),
          opacity(in.storage, in.engine, in.configuration, in.players),
          hooks(in.crt, in.scripts, in.renderer),
          fix(bones, in.configuration, hooks, opacity.bindings(), in.animation_cleanup),
          capture(in.storage, in.engine, in.configuration, in.players, bones,
                  history.entity_runtime()),
          local(in.storage, in.engine, in.configuration, history, capture, effects),
          movement(in.storage, in.engine, in.configuration, scene, visual, in.scripts, capture,
                   history),
          target_refresh(in.storage, in.engine, in.players, history, scene, bones, movement),
          shared_esp(in.storage, in.engine, in.configuration, in.players, history, in.scripts),
          commands(in.storage, in.engine, in.configuration, history, capture, bones),
          esp(in.storage, in.engine, in.configuration, in.players, scene, visual, in.crt,
              in.scripts, in.assets, {in.storage.data()}),
          rage(in.storage, in.engine, in.configuration, history, scene, bones, capture, movement,
               target_refresh, in.crt, esp, effects, visual, in.scripts,
               rage::RuntimeImports{imports.history.thread_pool}),
          legit(in.storage, in.engine, in.configuration, history, scene, bones, capture, in.crt,
                esp),
          miscellaneous(in.storage, in.engine, in.configuration, in.crt.descriptor_storage(),
                        {in.scripts.bridge(),
                         scene,
                         bones,
                         rage,
                         movement.live_movement(),
                         capture.budget(),
                         visual.projection(),
                         visual.renderer(),
                         visual.explosive_texture(),
                         visual.fire_texture(),
                         visual.ribbons(),
                         visual.indicators(),
                         {in.storage.data().native_cell<MovementCommandOwner*>(0x434759B0u),
                          movement.release_movement()},
                         MovementFeedbackBinding(
                             in.storage.data().native_cell<MovementCommandOwner*>(0x434759B0u))},
                        imports.miscellaneous),
          script_api(in.storage, in.engine, in.configuration, in.players, history, scene, bones,
                     visual, effects, hooks, in.assets, miscellaneous, in.scripts,
                     imports.script_api),
          skins(in.storage, in.configuration, in.skin_engine),
          packet(in.storage, in.engine, in.configuration, history, movement, target_refresh,
                 imports.packet),
          anti_aim(in.storage, in.engine, in.configuration, scene, bones, movement, capture,
                   history, target_refresh, visual, in.scripts, imports.anti_aim),
          exploit(in.storage, in.engine, in.configuration, history, movement, commands, local,
                  scene, visual, in.scripts),
          settings(in.storage, in.configuration, in.main_window, in.presets, in.shutdown.request()),
          prediction_history(in.storage, in.engine, in.configuration, local, history, movement,
                             bones, capture),
          entity_commands(in.storage, in.engine, in.configuration, history, local, capture),
          class_factories(in.storage, in.engine, in.players, entity_commands, hooks),
          viewmodel(in.storage, in.engine, in.configuration, skins, effects, hooks),
          free_look(in.storage, in.engine, in.configuration, in.crt),
          target_mask(in.storage, in.engine, in.configuration),
          feedback(in.storage, in.engine, in.configuration, history, scene, bones, in.crt, esp,
                   script_api),
          buffered_shots(in.storage, in.engine, in.configuration, history, capture, bones, in.crt,
                         esp, rage, feedback),
          prediction_masks(in.storage, in.engine, in.configuration, history, scene, esp),
          player_receives(in.storage, in.players, fix),
          visual_receives(in.storage, in.configuration, in.players),
          player_menu(in.storage, in.engine), features(in.storage, in.configuration, history, skins,
                                                       rage, legit, esp, script_api, hooks),
          map_system(registry(in.storage, 0x43475B78u),
                     {in.storage.data().native_cell<std::int32_t*>(0x43468D0Cu),
                      in.storage.data().native_cell<std::int32_t*>(0x43467A70u),
                      in.storage.data().native_cell<void*>(0x43467204u),
                      in.storage.data().native_cell<void*>(0x4346A514u),
                      in.storage.owner_registries().at(0x43475770u)}),
          packets{in.storage.owner_registries().at(0x4347584Cu),
                  in.storage.data().native_cell<void*>(0x4346A514u),
                  in.storage.data().native_cell<void*>(0x4346A890u),
                  in.storage.data().native_cell<EntityCommandOwner*>(0x43475898u),
                  network::packet_dispatch_offsets(in.storage.offsets())},
          user_messages(in.storage, in.players.adjustments(), in.scripts.bridge()) {
        // Resolve construction cycles by composing the real contexts, before
        // any registered receiver can observe them. No feature factory runs.
        effects.compose_command(camera, in.players, scene, in.crt, {in.storage.data()});
        effects.compose_primary(camera, history, bones, in.crt);
        hooks.initialize_network(miscellaneous);
        hooks.initialize_callbacks(fix);
        hooks.initialize_player_auxiliary(in.engine, in.configuration, history, bones);
        commands.complete(hooks, local, target_refresh, shared_esp);
        player_hooks = std::make_unique<engine::PlayerHookFamily>(
            hooks.descriptor_storage().at(engine::local_player_descriptor_identity),
            hooks.descriptor_storage().at(engine::remote_player_descriptor_identity),
            hooks.registry(),
            in.storage.data().native_cell<std::byte*>(engine::player_primary_module_identity),
            hooks.descriptor_images(), hooks.descriptor_system(),
            engine::PlayerPrimaryHookRuntimes{
                fix.animations(), commands.runtime(), local.simulation(),
                hooks.player_auxiliary().free_look_capture(), hooks.player_auxiliary().visibility(),
                local.queries(), local.physics(), local.local_hooks(), local.command_hooks()});
        visual.initialize_game_system(in.shutdown.finalizer());
        script_api.bind_modules(in.presets, in.main_window);
        combat = std::make_unique<CombatRuntimeOwner>(in.storage, packet, exploit, anti_aim,
                                                      effects, in.scripts);
        world =
            std::make_unique<WorldRuntimeOwner>(in.storage, in.engine, in.configuration, history,
                                                visual, effects, hooks, in.window, in.presets);
        simple_hud = std::make_unique<hud::SimpleHudRuntimeOwner>(in.storage, in.configuration,
                                                                  visual, hooks, in.scripts);
        graph = std::make_unique<NativeStartupSourceGraph>(
            NativeStartupSourceGraph{in.storage.configuration(),
                                     in.storage.menu_hotkey(),
                                     in.configuration.movement(),
                                     in.configuration.skins_settings(),
                                     in.configuration.anti_aim_initializers(),
                                     in.crt.exit_registration(),
                                     in.configuration.static_initializers(),
                                     in.storage.data(),
                                     in.final_initializer,
                                     in.crt.first_startup_services(),
                                     in.crt.descriptor_initializers(),
                                     in.scripts.startup_initializers(),
                                     in.animation_cleanup,
                                     in.crt.group_initializers(),
                                     prediction_history.factory_bindings(),
                                     entity_commands.factory_bindings(),
                                     free_look.initializers(),
                                     target_mask.factory_bindings(),
                                     shared_esp.initializers(),
                                     feedback.factory_bindings(),
                                     buffered_shots.factory_bindings(),
                                     viewmodel.factory_bindings(),
                                     in.configuration.global_initializers(),
                                     simple_hud->context(),
                                     in.crt.flag_initializers(),
                                     in.crt.ui_type_initializers(),
                                     features.factory_bindings(),
                                     prediction_masks.factory_bindings(),
                                     map_system,
                                     miscellaneous.factory_bindings(),
                                     combat->factory_bindings(),
                                     skins.factory_bindings(),
                                     movement.factory_bindings(),
                                     history.factory_bindings(),
                                     world->factory_bindings(),
                                     world->proxy_factory_bindings(),
                                     in.static_crt.static_initializers(),
                                     in.crt.event_name_initializers(),
                                     in.static_crt.native_vector_cleanup(),
                                     in.static_crt.native_container_cleanup(),
                                     history.source_terminal_cleanup()});
    }
};

SourceFeatureRuntime::SourceFeatureRuntime(SourceFeatureInputs inputs)
    : SourceFeatureRuntime(inputs, native_imports(inputs)) {}
SourceFeatureRuntime::SourceFeatureRuntime(SourceFeatureInputs inputs,
                                           const SourceFeatureRuntimeImports& imports)
    : data_(std::make_unique<Data>(inputs, imports)) {}
SourceFeatureRuntime::~SourceFeatureRuntime() = default;
NativeStartupSourceGraph& SourceFeatureRuntime::graph() noexcept {
    return *data_->graph;
}
void SourceFeatureRuntime::bind_factories(NativeStartupSources& sources) {
    bind_runtime_feature_sources(sources, graph());
}
void SourceFeatureRuntime::initialize_features(NativeStartupSources& sources) {
    auto& d = *data_;
    if (d.factories_started)
        throw std::logic_error("Source feature initialization has already started");
    bind_factories(sources);
    d.factories_started = true;
    initialize_runtime_owners(sources);
    d.inputs.scripts.bind_manager();
}
void SourceFeatureRuntime::bind_hook_contexts() {
    auto& d = *data_;
    if (d.hook_contexts_bound)
        throw std::logic_error("Source hook contexts have already been bound");
    if (d.inputs.storage.owner_registries().at(0x43475B78u).entries().size() != 57)
        throw std::logic_error("Source hook contexts require all57 actual feature owners");
    auto* particles =
        d.inputs.storage.data().native_cell<effects::HarmlessMolotovRuntime*>(0x43475828u);
    if (!particles || &particles->bindings != &d.world->molotov_bindings())
        throw std::logic_error(
            "Source particle hook context requires its real constructed feature owner");
    // Client-state handlers borrow the actual ScriptOwner and RootMenu.
    // Both are created after the physical feature contexts assembled above.
    auto& in = d.inputs;
    d.hooks.initialize_client_state(in.engine, in.configuration, d.history, d.capture, in.scripts,
                                    in.main_window);
    const engine::BuildHookFamilies families{*d.player_hooks,
                                             d.fix.animations(),
                                             d.bones.remote_setup_bones(),
                                             d.hooks.callbacks(),
                                             d.hooks.player_auxiliary().ragdolls(),
                                             *particles,
                                             d.packets,
                                             d.hooks.channel(),
                                             d.hooks.messages(),
                                             d.viewmodel.hooks(),
                                             d.hooks.client_state().family(),
                                             d.effects.model_hooks(),
                                             d.hooks.auto_accept_hooks(),
                                             d.hooks.console_input()};
    engine::bind_build_hook_runtime_contexts(d.hooks.descriptor_storage(), d.hooks.registry(),
                                             families);
    d.user_messages.bind();
    d.hook_contexts_bound = true;
}
ui::RootMenu& SourceFeatureRuntime::compose_menu() {
    auto& d = *data_;
    auto& in = d.inputs;
    return in.main_window.compose({in.configuration.widget_identities(), in.crt.rage_groups(),
                                   d.camera.runtime(), d.miscellaneous.runtime(),
                                   d.settings.runtime(), d.skins.catalog(), d.skins.refresh(),
                                   in.players.adjustments(), d.player_menu.bindings(), in.presets,
                                   in.scripts.require_owner(), in.assets.icon_cache()},
                                  in.renderer, in.engine.variables);
}
engine::HookRuntimeOwner& SourceFeatureRuntime::hooks() noexcept {
    return data_->hooks;
}
engine::ClientClassFactoryRuntime& SourceFeatureRuntime::class_factories() noexcept {
    return data_->class_factories;
}
HistoryRuntimeOwner& SourceFeatureRuntime::history() noexcept {
    return data_->history;
}
LocalAnimationRuntimeOwner& SourceFeatureRuntime::local_animation() noexcept {
    return data_->local;
}
EntityCommandRuntimeOwner& SourceFeatureRuntime::entity_commands() noexcept {
    return data_->entity_commands;
}
CommandRuntimeOwner& SourceFeatureRuntime::commands() noexcept {
    return data_->commands;
}
VisualRuntimeOwner& SourceFeatureRuntime::visual() noexcept {
    return data_->visual;
}
MiscRuntimeOwner& SourceFeatureRuntime::miscellaneous() noexcept {
    return data_->miscellaneous;
}
WorldRuntimeOwner& SourceFeatureRuntime::world() noexcept {
    return *data_->world;
}
skins::SkinsRuntimeOwner& SourceFeatureRuntime::skins() noexcept {
    return data_->skins;
}
misc::SettingsRuntimeOwner& SourceFeatureRuntime::menu_settings() noexcept {
    return data_->settings;
}
misc::ViewmodelRuntimeOwner& SourceFeatureRuntime::viewmodel() noexcept {
    return data_->viewmodel;
}
effects::CameraRuntimeOwner& SourceFeatureRuntime::camera() noexcept {
    return data_->camera;
}
PlayerReceiveOwner& SourceFeatureRuntime::player_receives() noexcept {
    return data_->player_receives;
}
effects::VisualReceiveOwner& SourceFeatureRuntime::visual_receives() noexcept {
    return data_->visual_receives;
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: source_startup_cleanup.cpp
// ============================================================================
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX

namespace skeet::client {
namespace {
enum class Access {
    read,
    write,
    execute
};
bool accessible(const void* pointer, std::size_t length, Access access) noexcept {
    auto position = reinterpret_cast<std::uintptr_t>(pointer);
    if (!position || !length || length > std::numeric_limits<std::uintptr_t>::max() - position)
        return false;
    const auto end = position + length;
    do {
        MEMORY_BASIC_INFORMATION region{};
        if (VirtualQuery(reinterpret_cast<const void*>(position), &region, sizeof region) !=
                sizeof region ||
            region.State != MEM_COMMIT || (region.Protect & (PAGE_GUARD | PAGE_NOACCESS)))
            return false;
        const auto protection = region.Protect & 0xFFu;
        const auto writable = protection == PAGE_READWRITE || protection == PAGE_WRITECOPY ||
                              protection == PAGE_EXECUTE_READWRITE ||
                              protection == PAGE_EXECUTE_WRITECOPY;
        const auto executable = protection == PAGE_EXECUTE || protection == PAGE_EXECUTE_READ ||
                                protection == PAGE_EXECUTE_READWRITE ||
                                protection == PAGE_EXECUTE_WRITECOPY;
        const auto readable =
            writable || protection == PAGE_READONLY || protection == PAGE_EXECUTE_READ;
        if ((access == Access::read && !readable) || (access == Access::write && !writable) ||
            (access == Access::execute && !executable))
            return false;
        const auto begin = reinterpret_cast<std::uintptr_t>(region.BaseAddress);
        if (region.RegionSize > std::numeric_limits<std::uintptr_t>::max() - begin)
            return false;
        const auto next = begin + region.RegionSize;
        if (next <= position)
            return false;
        position = next;
    } while (position < end);
    return true;
}
/* deduplicated helper */
bool executable(const void* pointer) noexcept {
    return accessible(pointer, 1, Access::execute);
}
template <class Function> bool callable(Function function) noexcept {
    return executable(reinterpret_cast<const void*>(function));
}
bool receiver_entry(void* receiver, std::size_t slot) noexcept {
    if (!accessible(receiver, sizeof(void*), Access::read))
        return false;
    auto** table = read<void**>(receiver);
    if (!accessible(table, (slot + 1) * sizeof(void*), Access::read))
        return false;
    return executable(read<void*>(table + slot));
}
bool primary_owners_ready(const engine::OwnerRegistry& owners) noexcept {
    const auto begin = reinterpret_cast<std::uintptr_t>(owners.begin);
    const auto end = reinterpret_cast<std::uintptr_t>(owners.end);
    const auto limit = reinterpret_cast<std::uintptr_t>(owners.limit);
    if (!begin)
        return !end && !limit;
    if (end < begin || limit < end || (end - begin) % sizeof(void*) ||
        (limit - begin) % sizeof(void*) || (end - begin) / sizeof(void*) > 57u)
        return false;
    if (end != begin && !accessible(owners.begin, end - begin, Access::read))
        return false;
    for (auto position = begin; position != end; position += sizeof(void*)) {
        auto* owner = read<void*>(reinterpret_cast<void*>(position));
        if (!receiver_entry(owner, 4) || !receiver_entry(owner, 0))
            return false;
    }
    return true;
}
bool hooks_ready(const engine::VtableHookNode& sentinel) noexcept {
    const auto* previous = &sentinel;
    auto* current = sentinel.next;
    // This is a bounded startup-cancellation check, not a map-lifetime list
    // walk. All57 factories plus their initial entity receivers fit this cut.
    constexpr std::size_t maximum_startup_hooks = 4096;
    std::size_t count = 0;
    while (current != &sentinel) {
        if (++count > maximum_startup_hooks ||
            !accessible(current, sizeof(*current), Access::write))
            return false;
        const auto node = read<engine::VtableHookNode>(current);
        if (node.previous != previous || !accessible(node.object, sizeof(void*), Access::write))
            return false;
        auto** installed = read<void**>(node.object);
        const auto address = reinterpret_cast<std::uintptr_t>(installed);
        if (address < 2 * sizeof(void*) ||
            !accessible(installed - 2, 3 * sizeof(void*), Access::read))
            return false;
        auto* metadata = read<engine::VtableHookMetadata*>(installed - 2);
        if (!accessible(metadata, sizeof(*metadata), Access::read))
            return false;
        const auto descriptor = read<engine::VtableHookMetadata>(metadata);
        if (descriptor.replacement_table != installed ||
            !accessible(descriptor.original_table, sizeof(void*), Access::read))
            return false;
        previous = current;
        current = node.next;
    }
    return sentinel.previous == previous;
}
bool pointer_patch_ready(const engine::SavedPointerPatch& patch, bool active) noexcept {
    return !active || (!patch.target) ||
           (accessible(patch.target, sizeof(void*), Access::write) &&
            accessible(patch.saved, sizeof(void*), Access::read));
}
}
bool source_exit_callbacks_ready(const ClientRuntimeStorage& storage) noexcept {
    try {
        const auto& data = storage.data();
        const auto count = data.native_cell<std::uint32_t>(0x4347CA0Cu);
        if (count > 512u)
            return false;
        const auto mask = data.native_cell<std::uintptr_t>(0x4347CA08u);
        const auto& entries = data.native_cell<std::array<std::uintptr_t, 512>>(0x43479CF0u);
        for (std::uint32_t index = 0; index < count; ++index)
            if (!executable(reinterpret_cast<const void*>(entries[index] ^ mask)))
                return false;
        return true;
    } catch (...) {
        return false;
    }
}
bool source_startup_detach_ready(ClientRuntimeStorage& storage,
                                 WindowRuntimeOwner& window) noexcept {
    try {
        if (&window.storage() != &storage || !source_exit_callbacks_ready(storage))
            return false;
        const auto& state = window.unload_state();
        const auto& system = window.unload_runtime().system;
        if (&state.owners != &storage.owner_registries().at(0x43475B78u) ||
            !primary_owners_ready(state.owners) || !hooks_ready(state.hooks) ||
            !callable(system.free_heap))
            return false;
        if (state.in_game_callback &&
            (!callable(state.in_game_callback) || !receiver_entry(state.engine, 27)))
            return false;
        if (!pointer_patch_ready(state.first_patch, state.first_patch.enabled != 0) ||
            !pointer_patch_ready(state.second_patch, state.second_patch.enabled != 0) ||
            !pointer_patch_ready(state.final_patch, state.final_patch.enabled != 0) ||
            !pointer_patch_ready(state.cleared_patch,
                                 state.cleared_patch.target && state.cleared_patch.saved))
            return false;
        if (state.listener_registered &&
            (!state.event_listener || !receiver_entry(state.event_manager, 21)))
            return false;
        if (state.window.window && state.window.subclass_id &&
            (!callable(state.window.subclass_procedure) || !callable(system.set_window_long_w) ||
             !callable(system.set_window_long_a) || !callable(system.get_window_long_a) ||
             !callable(system.get_window_long_w) || !callable(system.remove_window_subclass)))
            return false;
        return true;
    } catch (...) {
        return false;
    }
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: source_terminal_cleanup.cpp
// ============================================================================

namespace skeet::client {
namespace {
constexpr std::array<SourceTerminalCleanupSlot, 8> source_terminal_cleanup_slots{{
    {0x1CCu, 0x4332BB3Au, 0x433C956Cu},
    {0x1ECu, 0x433C7FAEu, 0x4332F1FDu},
    {0x1F0u, 0x4334C970u, 0x4334DA70u},
    {0x204u, 0x43325025u, 0x4339D4FEu},
    {0x20Cu, 0x433F47E6u, 0x433874D2u},
    {0x210u, 0x4340BCC1u, 0x4341EFD3u},
    {0x228u, 0x433412AFu, 0x433C8C11u},
    {0x2CCu, 0x4334115Au, 0x433AFCE8u},
}};
SourceTerminalCleanupBindings* terminal_active_cleanup = nullptr;

template <std::size_t Index> void finalize_entry() {
    if (!terminal_active_cleanup)
        throw std::logic_error("Source terminal callback has no process owner");
    terminal_active_cleanup->finalize(source_terminal_cleanup_slots[Index].finalizer);
}
template <std::size_t... Index> constexpr auto finalizer_functions(std::index_sequence<Index...>) {
    return std::array<void (*)(), sizeof...(Index)>{&finalize_entry<Index>...};
}
constexpr auto functions = finalizer_functions(std::make_index_sequence<source_terminal_cleanup_slots.size()>{});
template <std::size_t Index> void initialize_entry(void* owner) {
    static_cast<SourceTerminalCleanupBindings*>(owner)->initialize(source_terminal_cleanup_slots[Index].initializer);
}
template <std::size_t... Index>
void bind_entries(NativeStartupSources& sources, SourceTerminalCleanupBindings& binding,
                  std::index_sequence<Index...>) {
    (sources.bind_initializer({source_terminal_cleanup_slots[Index].initializer, &binding, &initialize_entry<Index>}), ...);
}
void require_plan() {
    const auto plan = crt_initialization_plan();
    for (const auto& slot : source_terminal_cleanup_slots) {
        bool found = false;
        for (const auto& entry : plan)
            if (entry.operand == slot.operand && entry.identifier == slot.initializer) {
                found = true;
                break;
            }
        if (!found)
            throw std::logic_error("Source terminal cleanup differs from the configured CRT plan");
    }
}
}

std::span<const SourceTerminalCleanupSlot, 8> source_terminal_cleanup_plan() noexcept {
    return source_terminal_cleanup_slots;
}

SourceTerminalCleanupBindings::SourceTerminalCleanupBindings(
    ExitCallbacks& exits, script::Bridge& callback_bridge, script::BrowserState& panorama,
    script::UiBindings& bindings, script::ReferenceStrings& flags, PlayerStateCache& cache,
    lagcomp::RecordPool& pool)
    : callbacks(exits), bridge(callback_bridge), browser(panorama), ui(bindings), strings(flags),
      players(cache), records(pool) {
    if (callbacks.entries.size() != 0x200)
        throw std::invalid_argument(
            "Source terminal cleanup requires the canonical512-slot exit registry");
    require_plan();
}
SourceTerminalCleanupBindings::SourceTerminalCleanupBindings(
    ClientRuntimeStorage& storage, script::SourceScriptRuntimeOwner& scripts,
    PlayerRuntimeOwner& player_owner)
    : SourceTerminalCleanupBindings(storage.exit_callbacks(), scripts.bridge(),
                                    scripts.browser_state(), scripts.ui_bindings(), scripts.flags(),
                                    player_owner.cache(), player_owner.records()) {
    if (&scripts.storage() != &storage || &player_owner.storage() != &storage ||
        &callbacks != &storage.exit_callbacks() || &bridge != &scripts.runtime().bridge ||
        &bridge != &scripts.extensions().process_bridge() ||
        &browser != &scripts.extensions().browser_state() ||
        &bridge.name_storage != &storage.event_names())
        throw std::invalid_argument(
            "Early source terminal cleanup requires the actual canonical script and player owners");
    source_owners_validated_ = true;
}
void SourceTerminalCleanupBindings::require_producers(
    const script::ScriptRuntime& runtime, const script::ScriptModulesBindings& modules,
    const lagcomp::HistoryRecordingBindings& history) {
    if (&bridge != &runtime.bridge || &bridge != &modules.extensions.process_bridge() ||
        &browser != &modules.extensions.browser_state() || &ui != &modules.ui ||
        &strings != &modules.flags || &players != &modules.player_cache ||
        &players != &history.players || &records != &history.records ||
        &modules.players != &history.globals.adjustments)
        throw std::invalid_argument(
            "Source terminal cleanup must borrow the actual script and history producers");
    producers_validated_ = true;
}
void SourceTerminalCleanupBindings::initialize(std::uint32_t address) {
    for (const auto& slot : source_terminal_cleanup_slots) {
        if (slot.initializer != address)
            continue;
        //4332BB3A writes only native inline-buffer metadata and an unused word.
        //The canonical C++ vector is constructed by its process owner already;
        //its source producer must not run before this CRT registration point.
        if (address == 0x4332BB3Au && !strings.entries.empty())
            throw std::logic_error(
                "Reference-string production precedes its source CRT initializer");
        (void)callbacks.append(source_finalizer(slot.finalizer));
        return;
    }
    throw std::out_of_range("Initializer is not a source-owned terminal cleanup registration");
}
void SourceTerminalCleanupBindings::finalize(std::uint32_t address) {
    switch (address) {
    case 0x4332F1FDu:
        bridge.finalize_active_callback_storage();
        break;
    case 0x4334DA70u:
        bridge.finalize_dirty_callback_storage();
        break;
    case 0x4339D4FEu:
        browser.finalize_object_handles();
        break;
    case 0x433874D2u:
        browser.finalize_script_handles();
        break;
    case 0x4341EFD3u:
        browser.finalize_global_handle();
        break;
    case 0x433C8C11u:
        ui.finalize_record_storage();
        break;
    case 0x433AFCE8u:
        players.finalize_history(records);
        break;
    case 0x433C956Cu:
        strings.finalize_storage();
        break;
    default:
        throw std::out_of_range("Finalizer is not a source-owned terminal cleanup body");
    }
}
void (*SourceTerminalCleanupBindings::source_finalizer(std::uint32_t address))() {
    for (std::size_t index = 0; index < source_terminal_cleanup_slots.size(); ++index) {
        if (source_terminal_cleanup_slots[index].finalizer != address)
            continue;
        if (terminal_active_cleanup && terminal_active_cleanup != this)
            throw std::logic_error("Source terminal callbacks require one stable process context");
        terminal_active_cleanup = this;
        return functions[index];
    }
    throw std::out_of_range("Finalizer is not a source-owned terminal cleanup body");
}

void bind_source_terminal_cleanup(NativeStartupSources& sources,
                                  SourceTerminalCleanupBindings& binding) {
    if (!binding.source_owners_validated_ && !binding.producers_validated_)
        throw std::logic_error(
            "Validate canonical source owners before selecting their terminal startup profile");
    if (binding.callbacks.count != 0)
        throw std::logic_error(
            "Select source terminal ownership before any CRT exits are registered");
    require_plan();
    bind_entries(sources, binding, std::make_index_sequence<source_terminal_cleanup_slots.size()>{});
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: startup.cpp
// ============================================================================
#if defined(_WIN32)
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#endif

namespace skeet::client {
namespace {
#if defined(_WIN32)
const char* __stdcall command_line_a() {
    return GetCommandLineA();
}
#endif
}

StartupImports native_startup_imports() {
#if defined(_WIN32)
    return {command_line_a};
#else
    throw std::runtime_error("Client startup imports require Windows");
#endif
}

void install_startup_window(ui::MenuWindow& menu, input::WindowSubclassRuntime& window) {
    if (!menu.native_window)
        throw std::invalid_argument("Client startup main window is unresolved");
    if (window.input.main_menu != &menu)
        throw std::invalid_argument("Client startup and input must share the main menu window");
    window.install(menu.native_window);
}

PostInitializationRuntime::PostInitializationRuntime(PostInitializationState state,
                                                     StartupConVars variables,
                                                     PostInitializationBindings bindings,
                                                     StartupImports imports)
    : state_(state), variables_(variables), bindings_(bindings), imports_(imports) {
    if (&bindings_.menu.script_owner() != &bindings_.scripts)
        throw std::invalid_argument("Client startup must use the main menu's script owner");
    if (&bindings_.scripts.modules().unload_state() != &bindings_.hooks)
        throw std::invalid_argument(
            "Client startup must use the script module owner's unload state");
    if (&bindings_.finalizer.window_title !=
        &bindings_.configuration_storage.native_storage().path())
        throw std::invalid_argument(
            "Client shutdown and CONFIG must share the native preset-store path cell");
    if (&bindings_.finalizer.native_window != &bindings_.menu.native_window())
        throw std::invalid_argument("Client startup and shutdown must share the main window cell");
    if (!bindings_.window.input.main_menu ||
        &bindings_.window.input.main_menu->native_window != &bindings_.menu.native_window())
        throw std::invalid_argument("Client startup and input must share the main menu window");
    if (!bindings_.menu.native_window())
        throw std::invalid_argument("Client startup requires the resolved main window");
    if (&bindings_.finalizer.input != &bindings_.window.input ||
        &bindings_.window.hooks.state != &bindings_.hooks ||
        &bindings_.finalizer.hooks.state != &bindings_.hooks)
        throw std::invalid_argument("Client window lifecycle must share input and hook state");
    if (!bindings_.configuration_storage.menu_bound_to(bindings_.menu))
        throw std::invalid_argument(
            "Client startup requires the composed CONFIG main menu binding");
    // Main-menu composition binds CONFIG once before this runtime exists.
    // The script finalizer attachment is separate from4341F57F call order.
    bindings_.scripts.bind_shutdown(bindings_.finalizer);
    // Bind the recovered static listener identity for both startup and cleanup.
    bindings_.hooks.event_listener = &bindings_.renderer_listener;
}

void PostInitializationRuntime::configure_variables() { // 4341F652..4341F740
    engine::invoke<void>(variables_.at_43467fbc, 16, 0xc0000);
    engine::invoke<void>(variables_.at_43467130, 16, 0x4b0);
    engine::invoke<void>(variables_.at_4346a5b4, 16, 0x40);
    engine::invoke<void>(variables_.at_43468ba8, 16, 0);
    if (engine::invoke<bool>(bindings_.hooks.engine, 27))
        return;

    engine::invoke<void>(variables_.at_434671ac, 16, 0x40);
    engine::invoke<void>(variables_.at_4346747c, 16, 0x40);
    auto* const callback_count = static_cast<std::byte*>(variables_.at_4346a748.get()) + 0x60;
    std::uint32_t saved;
    std::memcpy(&saved, callback_count, sizeof(saved));
    const std::uint32_t zero = 0;
    std::memcpy(callback_count, &zero, sizeof(zero));
    engine::invoke<void>(variables_.at_4346a748, 15, 1.0f / 64.0f);
    std::memcpy(callback_count, &saved, sizeof(saved));
    engine::invoke<void>(variables_.at_4346a330, 16, 2);

    auto* const source = state_.refresh_source;
    if (!source)
        return;
    auto* const vector = source->vector;
    if (!vector)
        return;
    for (std::int32_t i = 0; i < vector->count; ++i)
        engine::invoke<void>(vector->objects[i], 0);
}

void PostInitializationRuntime::initialize() { // 4341F57F..4341F7CD
    auto& hooks = bindings_.hooks;
    if (!hooks.listener_registered) {
        hooks.listener_registered = true;
        engine::invoke<void>(hooks.event_manager, 20, &bindings_.renderer_listener);
    }
    if (const auto factory = state_.first_factory) {
        state_.first_result = factory();
        state_.first_factory = nullptr;
    }
    if (const auto factory = state_.second_factory)
        state_.second_result = factory();

    configure_variables();
    bindings_.menu.skins_menu().initialize_default_paints();
    ui::initialize_startup_icons(bindings_.icons, bindings_.startup_icons);
    auto objects = hooks.owners.entries();
    for (auto* object : objects)
        engine::invoke<void>(object, 2);

    bindings_.menu.scripts_menu().initialize_startup(imports_.get_command_line_a(),
                                                     bindings_.startup_scripts);
    // Lua controls must exist before the preset restores their saved values.
    bindings_.menu.preset_manager().load_startup();
    if (engine::invoke<bool>(hooks.engine, 26)) {
        reset_startup_acknowledgements(bindings_.acknowledgements);
        reset_command_state(bindings_.command_reset);
        objects = hooks.owners.entries();
        for (auto* object : objects)
            engine::invoke<void>(object, 7);
        bindings_.window.input.enable_cursor();
        state_.connected = 1;
    }
    install_startup_window(bindings_.menu.window(), bindings_.window);
}
}

// ============================================================================
// Subcomponent: startup_bindings.cpp
// ============================================================================

namespace skeet::client {
PostInitializationData::PostInitializationData(std::shared_ptr<engine::NativeDataImage> data)
    : data_(std::move(data)) {
    (void)engine::require_native_data_image(data_);
}

PostInitializationState PostInitializationData::state() const {
    auto& d = *data_;
    return {d.native_cell<StartupFactory>(0x434678C4u),
            d.native_cell<void*>(0x43467460u),
            d.native_cell<StartupFactory>(0x43468B9Cu),
            d.native_cell<void*>(0x43476424u),
            d.native_cell<StartupRefreshSource*>(0x43467C04u),
            d.native_cell<std::uint8_t>(0x43475BD2u)};
}
CommandResetState PostInitializationData::command_reset() const {
    auto& d = *data_;
    return {d.native_cell<std::uint32_t>(0x434759C0u),
            d.native_cell<std::int32_t>(0x434759C4u),
            d.native_cell<std::int32_t>(0x434759C8u),
            d.native_cell<std::int32_t>(0x434759CCu),
            d.native_cell<std::int32_t>(0x434759DCu),
            d.native_cell<std::int32_t>(0x434759E8u),
            d.native_cell<std::int32_t>(0x434759F8u),
            d.native_cell<std::array<std::uint32_t, 150>>(0x43476090u),
            {d.native_cell<std::int32_t>(0x4346DB1Cu), d.native_cell<std::uint32_t>(0x4346DB2Cu),
             d.native_cell<std::uint32_t>(0x4346DB30u), d.native_cell<std::uint32_t>(0x4346DB38u)}};
}
std::span<StartupAcknowledgement, 150> PostInitializationData::acknowledgements() const {
    return data_->native_cell<std::array<StartupAcknowledgement, 150>>(0x43475BE0u);
}
PostInitializationImports native_post_initialization_imports() {
    return {native_startup_imports(), ui::native_window_lifetime_imports()};
}

namespace {
PostInitializationBindings
compose_bindings(PostInitializationData& data, ClientRuntimeStorage& storage,
                 const EngineRuntimeBindings& engine, ui::MainMenuWindowOwner& window,
                 ui::StartupRenderer& renderer, ui::IconCache& icons,
                 ui::StartupIcons& startup_icons, input::WindowSubclassRuntime& subclass,
                 ConfigStorageOwner& configuration, script::ScriptOwner& scripts,
                 ShutdownRuntime& finalizer, PostInitializationImports imports) {
    auto& d = storage.data();
    auto& menu = window.controller();
    auto& hooks = subclass.hooks.state;
    if (&data.data() != &d || &engine.variables.data() != &d ||
        &window.menu_data() != &storage.menu_startup_data() ||
        &renderer.dpi() != &storage.menu_startup_data().dpi ||
        d.native_cell<ui::RendererContainer*>(0x4347690Cu) != &renderer.renderer() ||
        d.native_cell<ui::RootMenu*>(0x43476924u) != &menu || &menu.window() != &window.window() ||
        &configuration.native_storage() != &storage.config_storage() ||
        &configuration.configuration() != &storage.configuration() ||
        !configuration.menu_bound_to(menu) ||
        d.native_cell<script::ScriptOwner*>(0x43475B00u) != &scripts ||
        &subclass.input != &storage.input_runtime() ||
        &finalizer.exit_callbacks != &storage.exit_callbacks() ||
        &finalizer.scripts != &scripts.modules().shutdown_state() ||
        hooks.engine != d.native_cell<void*>(0x4346A514u) || !hooks.engine ||
        hooks.event_manager != d.native_cell<void*>(0x4346A470u) || !hooks.event_manager)
        throw std::invalid_argument(
            "Post-initialization requires the canonical completed process owners");
    if ((finalizer.main_callbacks && finalizer.main_callbacks != &window.callbacks()) ||
        (finalizer.renderer && finalizer.renderer != &renderer.renderer()))
        throw std::invalid_argument(
            "Post-initialization cannot replace a different GUI shutdown owner");
    if (!imports.startup.get_command_line_a || !imports.window_lifetime.free_unicode_string)
        throw std::invalid_argument(
            "Post-initialization requires its actual startup and window imports");
    return {hooks,
            renderer.device_listener(),
            menu,
            icons,
            startup_icons,
            subclass,
            data.command_reset(),
            data.acknowledgements(),
            window.startup_scripts(),
            configuration,
            scripts,
            finalizer};
}
}

PostInitializationOwner::PostInitializationOwner(
    ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
    ui::MainMenuWindowOwner& window, ui::StartupRenderer& renderer, ui::IconCache& icons,
    ui::StartupIcons& startup_icons, input::WindowSubclassRuntime& subclass,
    ConfigStorageOwner& configuration, script::ScriptOwner& scripts, ShutdownRuntime& finalizer)
    : PostInitializationOwner(storage, engine, window, renderer, icons, startup_icons, subclass,
                              configuration, scripts, finalizer,
                              native_post_initialization_imports()) {}

PostInitializationOwner::PostInitializationOwner(
    ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
    ui::MainMenuWindowOwner& window, ui::StartupRenderer& renderer, ui::IconCache& icons,
    ui::StartupIcons& startup_icons, input::WindowSubclassRuntime& subclass,
    ConfigStorageOwner& configuration, script::ScriptOwner& scripts, ShutdownRuntime& finalizer,
    PostInitializationImports imports)
    : data_(storage.data_owner()), menu_(window.controller()),
      runtime_(data_.state(), engine.variables.startup_variables(),
               compose_bindings(data_, storage, engine, window, renderer, icons, startup_icons,
                                subclass, configuration, scripts, finalizer, imports),
               imports.startup) {
    finalizer.main_callbacks = &window.callbacks();
    finalizer.renderer = &renderer.renderer();
    finalizer.window_imports = imports.window_lifetime;
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: startup_reset.cpp
// ============================================================================

namespace skeet::client {
void reset_target_state(TargetResetState state) noexcept { // 4341D854
    state.handle = -1;
    state.value_2c = 0;
    state.value_30 = 0;
    state.value_38 = 0;
}

void reset_command_state(CommandResetState state) noexcept { // 433D6630
    state.flags &= 0xfffef9e7u;
    state.value_c4 = -1;
    std::fill(state.inverted_commands.begin(), state.inverted_commands.end(), 0xffffffffu);
    state.value_cc = -1;
    state.value_c8 = -1;
    state.value_dc = -1;
    state.value_e8 = -1;
    state.value_f8 = -1;
    reset_target_state(state.target);
}

void reset_startup_acknowledgements(std::span<StartupAcknowledgement, 150> records) noexcept {
    for (auto& record : records)
        record[0] = 0xffffffffu;
}
}

// ============================================================================
// Subcomponent: static_cleanup.cpp
// ============================================================================

namespace skeet::client {
namespace {
const StaticCleanupRuntime* static_active_cleanup = nullptr;
}

StaticCleanupRuntime::StaticCleanupRuntime(StaticCleanupBindings bindings) : bindings_(bindings) {
    if (!bindings_.memory.free || !bindings_.memory.process_heap)
        throw std::invalid_argument("Static cleanup requires the canonical process heap binding");
    const auto* const image = &bindings_.owner_registries.image();
    if (&bindings_.spread.image() != image || &bindings_.configuration.image() != image ||
        &bindings_.data_update_observers.image() != image ||
        &bindings_.recharge_observers.image() != image ||
        &bindings_.prediction_observers.image() != image ||
        &bindings_.empty_tick_observers.image() != image ||
        &bindings_.command_observers.image() != image ||
        &bindings_.entity_observers.image() != image)
        throw std::invalid_argument(
            "Static cleanup storage must retain the same initialization image");
}

bool StaticCleanupRuntime::has_source_callback(std::uint32_t target) const noexcept {
    switch (target) {
    case 0x433EB717u:
    case 0x43347D44u:
    case 0x433F0746u:
    case 0x433C9DB2u:
    case 0x433F4C95u:
    case 0x433D1E9Fu:
    case 0x433C9173u:
    case 0x43336AD4u:
    case 0x43341B3Fu:
    case 0x4337D954u:
    case 0x43329A5Bu:
    case 0x4332EFADu:
    case 0x433F4A1Du:
    case 0x433B8C06u:
    case 0x433E6194u:
    case 0x433F515Bu:
    case 0x43349ECBu:
    case 0x433E4A38u:
    case 0x433E4C98u:
        return true;
    default:
        return false;
    }
}

void (*StaticCleanupRuntime::source_callback(std::uint32_t target) const)() {
    if (!has_source_callback(target))
        throw std::out_of_range("Static finalizer has no concrete source storage owner");
    if (static_active_cleanup && static_active_cleanup != this)
        throw std::logic_error("Exit callbacks require one stable static cleanup owner");
    static_active_cleanup = this;
    switch (target) {
    case 0x433EB717u:
        return free_main_owners;
    case 0x43347D44u:
        return free_hud_callbacks;
    case 0x433F0746u:
        return free_network_callbacks;
    case 0x433C9DB2u:
        return free_update_callbacks;
    case 0x433F4C95u:
        return free_packet_callbacks;
    case 0x433D1E9Fu:
        return free_frame_callbacks;
    case 0x433C9173u:
        return free_map_callbacks;
    case 0x43336AD4u:
        return free_blur_callbacks;
    case 0x43341B3Fu:
        return free_event_callbacks;
    case 0x4337D954u:
        return free_spread_tables;
    case 0x43329A5Bu:
        return free_config_baseline;
    case 0x4332EFADu:
        return free_config_settings;
    case 0x433F4A1Du:
        return free_data_update_observers;
    case 0x433E6194u:
        return free_recharge_observers;
    case 0x433F515Bu:
        return free_prediction_observers;
    case 0x43349ECBu:
        return free_empty_tick_observers;
    case 0x433E4A38u:
        return free_command_observers;
    case 0x433E4C98u:
        return free_entity_observers;
    case 0x433B8C06u:
        return free_feedback_position_callbacks;
    default:
        throw std::out_of_range("Static finalizer has no concrete source storage owner");
    }
}

const StaticCleanupRuntime& StaticCleanupRuntime::bound() {
    if (!static_active_cleanup)
        throw std::logic_error("Static cleanup source binding is not installed");
    return *static_active_cleanup;
}

void StaticCleanupRuntime::free_allocation(void* allocation) const {
    engine::UnloadSystemFunctions memory;
    memory.free_heap = bindings_.memory.free;
    memory.process_heap = bindings_.memory.process_heap;
    // Native callbacks load their live begin slot once, null-test it, then
    // tail-call433837D0. That existing body restores the allocation base.
    // No registry pointer is reset and no registered object is freed here.
    engine::free_native_allocation(allocation, memory);
}
void StaticCleanupRuntime::free_registry_storage(engine::OwnerRegistry& registry) const {
    free_allocation(registry.begin);
}

void StaticCleanupRuntime::free_main_owners() {
    const auto& runtime = bound();
    runtime.free_registry_storage(runtime.bindings_.owner_registries.at(0x43475B78u));
}
void StaticCleanupRuntime::free_hud_callbacks() {
    const auto& runtime = bound();
    runtime.free_registry_storage(runtime.bindings_.owner_registries.at(0x43475A9Cu));
}
void StaticCleanupRuntime::free_network_callbacks() {
    const auto& runtime = bound();
    runtime.free_registry_storage(runtime.bindings_.owner_registries.at(0x43475B94u));
}
void StaticCleanupRuntime::free_update_callbacks() {
    const auto& runtime = bound();
    runtime.free_registry_storage(runtime.bindings_.owner_registries.at(0x43475988u));
}
void StaticCleanupRuntime::free_packet_callbacks() {
    const auto& runtime = bound();
    runtime.free_registry_storage(runtime.bindings_.owner_registries.at(0x4347584Cu));
}
void StaticCleanupRuntime::free_frame_callbacks() {
    const auto& runtime = bound();
    runtime.free_registry_storage(runtime.bindings_.owner_registries.at(0x43475AD8u));
}
void StaticCleanupRuntime::free_map_callbacks() {
    const auto& runtime = bound();
    runtime.free_registry_storage(runtime.bindings_.owner_registries.at(0x43475770u));
}
void StaticCleanupRuntime::free_blur_callbacks() {
    const auto& runtime = bound();
    runtime.free_registry_storage(runtime.bindings_.owner_registries.at(0x43475A84u));
}
void StaticCleanupRuntime::free_event_callbacks() {
    const auto& runtime = bound();
    runtime.free_registry_storage(runtime.bindings_.owner_registries.at(0x434758E0u));
}
void StaticCleanupRuntime::free_data_update_observers() {
    bound().bindings_.data_update_observers.finalize_storage();
}
void StaticCleanupRuntime::free_recharge_observers() {
    bound().bindings_.recharge_observers.finalize_storage();
}
void StaticCleanupRuntime::free_prediction_observers() {
    bound().bindings_.prediction_observers.finalize_storage();
}
void StaticCleanupRuntime::free_empty_tick_observers() {
    bound().bindings_.empty_tick_observers.finalize_storage();
}
void StaticCleanupRuntime::free_command_observers() {
    bound().bindings_.command_observers.finalize_storage();
}
void StaticCleanupRuntime::free_entity_observers() {
    bound().bindings_.entity_observers.finalize_storage();
}
void StaticCleanupRuntime::free_feedback_position_callbacks() {
    const auto& runtime = bound();
    runtime.free_registry_storage(runtime.bindings_.owner_registries.at(0x43475930u));
}
void StaticCleanupRuntime::initialize_spread_storage() const {
    bindings_.spread.initialize(bindings_.spread_bindings);
}
void StaticCleanupRuntime::free_spread_tables() {
    const auto& runtime = bound();
    runtime.bindings_.spread.finalize(runtime.bindings_.spread_bindings);
}
void StaticCleanupRuntime::free_config_baseline() {
    const auto& runtime = bound();
    runtime.bindings_.configuration.finalize_baseline(runtime.bindings_.memory);
}
void StaticCleanupRuntime::free_config_settings() {
    const auto& runtime = bound();
    runtime.bindings_.configuration.finalize_settings(runtime.bindings_.memory);
}
}

// ============================================================================
// Subcomponent: static_crt_initialization.cpp
// ============================================================================

namespace skeet::client {
namespace {
struct PointerFinalizer {
    std::uint32_t initializer, finalizer, cell;
    bool clear_after_free;
};
constexpr std::array<PointerFinalizer, 11> pointers{
    {{0x43393926u, 0x4339c54fu, 0x43478cc8u, false},
     {0x43320100u, 0x43382567u, 0x43478dd0u, false},
     {0x433b37a8u, 0x433ab14du, 0x43478de0u, false},
     {0x433848d6u, 0x4331e21au, 0x43475944u, false},
     {0x433800e5u, 0x433b32bfu, 0x43479300u, true},
     {0x433e1321u, 0x433e0032u, 0x4347962cu, false},
     {0x433e3de2u, 0x433d1129u, 0x4347961cu, false},
     {0x433eb701u, 0x433e3b6du, 0x43479784u, false},
     {0x43400269u, 0x433ab50cu, 0x43479770u, false},
     {0x433a12a4u, 0x433b76abu, 0x434797a0u, false},
     {0x43349e5au, 0x433b1fe9u, 0x434797b0u, true}}};
StaticCrtInitialization* active_context = nullptr;

template <std::size_t Index> void finalize_pointer() {
    active_context->finalize_pointer_storage(pointers[Index].finalizer);
}
template <std::size_t... Index> constexpr auto pointer_callbacks(std::index_sequence<Index...>) {
    return std::array<void (*)(), sizeof...(Index)>{finalize_pointer<Index>...};
}
constexpr auto finalizers = pointer_callbacks(std::make_index_sequence<pointers.size()>{});

template <std::size_t Index> void register_pointer_finalizer(void* raw) {
    auto& context = *static_cast<StaticCrtInitialization*>(raw);
    (void)context.callbacks.append(context.source_finalizer(pointers[Index].finalizer));
}
template <std::size_t... Index>
void bind_pointers(NativeStartupSources& sources, StaticCrtInitialization& context,
                   std::index_sequence<Index...>) {
    (sources.bind_initializer(
         {pointers[Index].initializer, &context, register_pointer_finalizer<Index>}),
     ...);
}
void initialize_icons(void* raw) {
    static_cast<StaticCrtInitialization*>(raw)->initialize_icon_cache();
}
void initialize_entities(void* raw) {
    static_cast<StaticCrtInitialization*>(raw)->initialize_entity_slots();
}
}

void StaticCrtInitialization::initialize_icon_cache() {
    icons.initialize_native_cache();
}
void StaticCrtInitialization::initialize_entity_slots() {
    engine::require_native_data_image(data)
        .native_cell<std::array<std::uint32_t, 65>>(0x43469094u)
        .fill(UINT32_MAX);
}
void StaticCrtInitialization::finalize_pointer_storage(std::uint32_t native_finalizer) const {
    for (const auto& entry : pointers) {
        if (entry.finalizer != native_finalizer)
            continue;
        auto& image = engine::require_native_data_image(data);
        auto& cell = image.native_cell<void*>(entry.cell);
        if (auto* const allocation = cell) {
            engine::release_owner_storage(memory, allocation);
            // Only 433B32BF/433B1FE9 clear their pointer in the native binary.
            if (entry.clear_after_free)
                cell = nullptr;
        }
        if (entry.cell == 0x434797A0u) {
            // The source application destroys ShotModelBoneStorage after the
            // CRT exit callbacks. Both share this vector, so revoke ownership
            // here to prevent its C++ destructor from freeing the block again.
            image.native_cell<std::array<void*, 3>>(entry.cell).fill(nullptr);
        }
        return;
    }
    throw std::out_of_range("Finalizer is not a recovered static pointer-buffer cleanup entry");
}
void (*StaticCrtInitialization::source_finalizer(std::uint32_t native_finalizer))() {
    for (std::size_t index = 0; index < pointers.size(); ++index) {
        if (pointers[index].finalizer != native_finalizer)
            continue;
        if (active_context && active_context != this)
            throw std::logic_error("Static CRT exit callbacks require one stable source context");
        active_context = this;
        return finalizers[index];
    }
    throw std::out_of_range("Finalizer is not a recovered static pointer-buffer cleanup entry");
}
void bind_static_crt_initializers(NativeStartupSources& sources, StaticCrtInitialization& context) {
    if (context.data.get() != &sources.data())
        throw std::invalid_argument("Static CRT storage must share the startup native data owner");
    if (!context.memory.free || !context.memory.process_heap)
        throw std::invalid_argument("Static CRT finalizers require the canonical process heap");
    bind_pointers(sources, context, std::make_index_sequence<pointers.size()>{});
    sources.bind_initializer({0x433689b6u, &context, initialize_icons});
    sources.bind_initializer({0x433b0bceu, &context, initialize_entities});
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: target_mask_lifecycle.cpp
// ============================================================================

namespace skeet::client {
namespace {
#if defined(_M_IX86) || defined(__i386__)
#define TARGET_MASK_MEMBER __thiscall
#define TARGET_MASK_FAST __fastcall
#else
#define TARGET_MASK_MEMBER
#define TARGET_MASK_FAST
#endif
using PrimaryEntry = void(TARGET_MASK_MEMBER*)(void*);
using HudSlotOne = bool(TARGET_MASK_FAST*)(void*, ui::RenderQueue*, void*, std::int32_t*);
using HudSlotTwo = std::int32_t(TARGET_MASK_MEMBER*)(void*);
struct TargetMaskHudTable {
    hud::HudFrameEntry draw;
    HudSlotOne slot_one;
    HudSlotTwo slot_two;
};
static_assert(offsetof(TargetMaskHudTable, slot_one) == sizeof(hud::HudFrameEntry));
static_assert(offsetof(TargetMaskHudTable, slot_two) == sizeof(hud::HudFrameEntry) * 2);
struct TargetMaskMessage : misc::NetworkMessageCallback {
    TargetMaskOwner* owner;
};

void TARGET_MASK_MEMBER target_mask_destruction_entry(void*);
void TARGET_MASK_MEMBER target_mask_reset_entry(void*);
void TARGET_MASK_MEMBER target_mask_unchanged_entry(void*) {} //433CCE8E
void TARGET_MASK_FAST target_mask_message_entry(void*, misc::NetworkMessageCall*);
void TARGET_MASK_FAST hud_draw(void*, ui::RenderQueue*, void*, std::int32_t*) {} //4334A269 RET8
bool TARGET_MASK_FAST hud_slot_one(void*, ui::RenderQueue*, void*, std::int32_t*) {
    return false;
} //43382057
std::int32_t TARGET_MASK_MEMBER hud_slot_two(void*) {
    return 1;
} //433686FC

const std::array<PrimaryEntry, 9> target_mask_primary_table{
    target_mask_destruction_entry, target_mask_unchanged_entry, target_mask_unchanged_entry, target_mask_unchanged_entry, target_mask_unchanged_entry,
    target_mask_unchanged_entry,   target_mask_reset_entry,     target_mask_unchanged_entry, target_mask_unchanged_entry}; //43470114
const std::array<misc::NetworkMessageEntry, 1> target_mask_message_table{target_mask_message_entry}; //4346F334
const TargetMaskHudTable hud_table{hud_draw, hud_slot_one, hud_slot_two}; //434701CC

void require_canonical_descriptor(const TargetMaskFactoryBindings& bindings) {
    const auto& data = engine::require_native_data_image(bindings.data);
    if (&bindings.registries.commands.image() != &data ||
        &bindings.registries.entities.image() != &data)
        throw std::invalid_argument(
            "Target-mask lifecycle requires the canonical native data owner");
    if (&bindings.command.enabled != &bindings.settings.enabled.value)
        throw std::invalid_argument(
            "Target-mask lifecycle requires the canonical CRT descriptor347A4FA6");
}
/* [dedup allocate_job_memory] */
void target_mask_release(engine::JobMemory memory, void* storage) {
    auto* const bytes = static_cast<std::byte*>(storage);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, sizeof displacement);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
}

class TargetMaskLifecycleOwner {
public:
    const PrimaryEntry* vtable = nullptr;
    NativeCommandPhaseReceiver command_receiver;
    NativeEntityObserverReceiver entity_receiver;
    TargetMaskMessage message{{nullptr}, nullptr};
    hud::HudFrameCallback hud;
    TargetMaskFactoryBindings bindings;

    explicit TargetMaskLifecycleOwner(const TargetMaskFactoryBindings& source) : bindings(source) {
        auto& registries = bindings.registries;
        auto& command = bindings.command;
        registries.primary.append(this); //434806D8 ->433676A0, primary still zero
        registries.commands.append(command_receiver, &command); //434806E0
        message.owner = &command;
        message.vtable = nullptr; //434806E8, before the actual message append
        registries.messages.append(&message); //434806ED ->4336F73C
        registries.entities.append_target_mask(entity_receiver, command); //434806F5
        hud::register_hud_frame(registries.hud, &hud); //434806FD ->433D20B5

        message.vtable = target_mask_message_table.data(); //43480704, native4346F334
        registries.commands.publish(command_receiver, CommandOwnerId::owner_43470478); //4348070C
        registries.entities.publish_target_mask(entity_receiver); //43480713
        hud.vtable = &hud_table.draw; //4348071F, all three native slots retained
        vtable = target_mask_primary_table.data(); //43480726

        auto& state = command.state;
        state.accepted_tick = std::bit_cast<std::int32_t>(std::uint32_t(state.accepted_tick) &
                                                          0xFFFFFF00u); //4348072C
        state.has_accepted_tick = 0; //4348072F
        state.attack_pending = 0; //43480732
        //mem_int3.h's explicit43480735 record is MOV EAX,[43467A78].
        // Read that live cell only; this constructor does not invoke a VM.
        const auto variable = engine::require_native_data_image(bindings.data)
                                  .native_cell<std::uint32_t>(0x43467A78u) ^
                              0x3AE06513u;
        state.first = 0; //4348073F
        command.bindings.lag_compensation =
            reinterpret_cast<const void*>(std::uintptr_t(variable)); //43480742
        state.count = 0; //43480748
        for (auto& record : state.records) { //4348074B..43480756
            record.targets[0] = 0;
            record.targets[1] = 0;
        }
    }
};
static_assert(sizeof(TargetMaskLifecycleOwner) <= target_mask_native_size);

namespace {
void TARGET_MASK_MEMBER target_mask_destruction_entry(void* object) {
    destroy_target_mask_lifecycle(static_cast<TargetMaskLifecycleOwner*>(object));
}
void TARGET_MASK_MEMBER target_mask_reset_entry(void* object) {
    reset_target_mask_lifecycle(*static_cast<TargetMaskLifecycleOwner*>(object));
}
void TARGET_MASK_FAST target_mask_message_entry(void* object, misc::NetworkMessageCall* call) {
    auto& endpoint =
        *static_cast<TargetMaskMessage*>(static_cast<misc::NetworkMessageCallback*>(object));
    process_target_mask_message(*endpoint.owner, call->message);
}
void target_mask_factory_entry(void* context) {
    create_target_mask_lifecycle(*static_cast<TargetMaskFactoryBindings*>(context));
}
}

std::size_t target_mask_lifecycle_size() noexcept {
    return sizeof(TargetMaskLifecycleOwner);
}
TargetMaskLifecycleOwner&
construct_target_mask_lifecycle(void* storage, const TargetMaskFactoryBindings& bindings) {
    require_canonical_descriptor(bindings);
    return *::new (storage) TargetMaskLifecycleOwner(bindings);
}
TargetMaskLifecycleOwner* create_target_mask_lifecycle(const TargetMaskFactoryBindings& bindings) {
    require_canonical_descriptor(bindings);
    auto* const storage = allocate(bindings.registries.primary.memory); //4348066A..43480674
    if (!storage)
        throw std::runtime_error("Unrecovered native target-mask allocation-failure edge4348067F");
    //43387448 zeroed the native owner, while the source ring is the existing
    //borrowed canonical state. Project the same zeros only after allocation.
    bindings.command.state = TargetMaskState{};
    return &construct_target_mask_lifecycle(storage, bindings); //43480678/7A tail-jump
}
void destroy_target_mask_lifecycle(TargetMaskLifecycleOwner* owner) {
    if (!owner)
        return; //433837D0's only null guard
    const auto memory = owner->bindings.registries.primary.memory;
    owner->~TargetMaskLifecycleOwner();
    target_mask_release(memory, owner);
}
void reset_target_mask_lifecycle(TargetMaskLifecycleOwner& owner) noexcept {
    reset_target_mask_owner(owner.bindings.command);
}
void* target_mask_lifecycle_object(TargetMaskLifecycleOwner& owner) noexcept {
    return &owner;
}
TargetMaskOwner& target_mask_command_owner(TargetMaskLifecycleOwner& owner) noexcept {
    return owner.bindings.command;
}
void bind_native_target_mask_factory(NativeStartupSources& sources,
                                     TargetMaskFactoryBindings& bindings) {
    require_canonical_descriptor(bindings);
    if (bindings.data.get() != &sources.data())
        throw std::invalid_argument("Target-mask factory must share the startup native data owner");
    sources.bind_initializer({target_mask_factory, &bindings, target_mask_factory_entry});
}

#undef TARGET_MASK_MEMBER
#undef TARGET_MASK_FAST
} // namespace skeet::client

// ============================================================================
// Subcomponent: target_mask_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* deduplicated helper */
/* [stripped helper] */
/* [dedup subtract_i32] */
const std::uint8_t& feature(ConfigRegistries& registries) {
    auto* link = find_config(registries.descriptors, target_mask_feature_hash);
    if (!link)
        throw std::invalid_argument("Target-mask owner requires native config347A4FA6");
    auto& descriptor = *static_cast<ConfigDescriptor*>(link);
    if (descriptor.type != ConfigType::boolean || descriptor.serialized_size != 1)
        throw std::invalid_argument("Target-mask config347A4FA6 must be native Boolean");
    return static_cast<BooleanConfig&>(descriptor).value;
}
bool lag_compensation_enabled(const void* variable) noexcept {
    for (;;) {
        const auto* parent = field<const void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    return field<std::uint32_t>(variable, 0x30) !=
           std::uint32_t(reinterpret_cast<std::uintptr_t>(variable));
}
bool low_latency(const TargetMaskBindings& bindings) noexcept {
    const auto total = _mm_add_sd(_mm_set_sd(*bindings.real_latency_outgoing),
                                  _mm_set_sd(*bindings.real_latency_incoming));
    const auto rounded = _mm_cvtss_f32(_mm_cvtsd_ss(_mm_setzero_ps(), total));
    return 0.5f > rounded;
}
void clear_history(TargetMaskState& state) noexcept {
    state.count = 0;
    state.first = 0;
    if (state.has_accepted_tick)
        state.has_accepted_tick = 0;
}
}

TargetMaskOwner::TargetMaskOwner(TargetMaskSettings& settings, TargetMaskBindings bindings) noexcept
    : enabled(settings.enabled.value), bindings(bindings) {}
TargetMaskOwner::TargetMaskOwner(ConfigRegistries& registries, TargetMaskBindings bindings)
    : enabled(feature(registries)), bindings(bindings) {}

void reset_target_mask_owner(TargetMaskOwner& owner) noexcept { // 4332F1D9
    clear_history(owner.state);
    owner.state.attack_pending = 0;
}
void observe_target_mask_update_status(TargetMaskOwner& owner,
                                       const void* status) noexcept { // 4336AC2E
    if ((field<std::uint32_t>(status, 0x24) & 4u) && !field<std::uint8_t>(status, 0x10))
        clear_history(owner.state);
}
void capture_target_mask_update(TargetMaskOwner& owner, std::uintptr_t incoming) { // 4336526D
    if (!owner.enabled)
        return;
    const auto* visibility = field<const void*>(reinterpret_cast<const void*>(incoming), 8);
    if (!visibility)
        return;
    auto& state = owner.state;
    if (state.count != 128)
        ++state.count;
    state.first = (state.first - 1u) & 127u;
    auto& record = state.records[state.first];
    const auto time = engine::invoke<float>(owner.bindings.engine_client, 14);
    record.receive_tick = lagcomp::receive_time_to_tick(time, *owner.bindings.receive_interval);
    // Native4334194D replaces every destination bit. Bit0 comes from entity64,
    // not the unused entity0 bit in the first source word.
    record.targets[0] = (field<std::uint32_t>(visibility, 0x10) & 0xFFFFFFFEu) |
                        (field<std::uint32_t>(visibility, 0x18) & 1u);
    record.targets[1] = field<std::uint32_t>(visibility, 0x14);
}
void process_target_mask_message(TargetMaskOwner& owner, void* message) { // 4332CC6A
    if (engine::invoke<std::int32_t>(message, 7) != 4)
        return;
    auto& state = owner.state;
    const auto& bindings = owner.bindings;
    if (owner.enabled && !engine::invoke<bool>(bindings.engine_client, 82) &&
        !engine::invoke<bool>(bindings.engine_client, 93) &&
        (field<std::uint8_t>(message, 0x24) & 1u)) {
        const auto incoming_tick = field<std::int32_t>(message, 0xC);
        if (incoming_tick == -1) {
            clear_history(state);
            return;
        }
        if (state.has_accepted_tick && incoming_tick >= state.accepted_tick &&
            *bindings.rage_enabled && lag_compensation_enabled(bindings.lag_compensation) &&
            low_latency(bindings)) {
            if (state.attack_pending) {
                state.attack_pending = 0;
            } else {
                const auto gap = subtract(incoming_tick, state.accepted_tick);
                if (gap >= 0 && std::uint32_t(gap) <= 80u) {
                    field(message, 0xC, state.accepted_tick);
                } else {
                    const auto oldest_allowed = subtract(incoming_tick, 80);
                    auto tick = incoming_tick;
                    for (std::uint32_t i = 0; i != state.count; ++i) {
                        const auto& record = state.records[(state.first + i) & 127u];
                        if (record.receive_tick < oldest_allowed)
                            break;
                        tick = record.receive_tick;
                    }
                    field(message, 0xC, tick);
                }
            }
        }
    }
    const auto tick = field<std::int32_t>(message, 0xC);
    if (tick == -1) {
        clear_history(state);
        return;
    }
    while (state.count) {
        const auto oldest = (state.first + state.count - 1u) & 127u;
        if (state.records[oldest].receive_tick >= tick)
            break;
        --state.count;
    }
    if (!state.count && state.has_accepted_tick)
        state.has_accepted_tick = 0;
    state.accepted_tick = tick;
    if (!state.has_accepted_tick)
        state.has_accepted_tick = 1;
}
void process_target_mask_message_context(TargetMaskOwner& owner, const void* context) {
    process_target_mask_message(owner, field<void*>(context, 4));
}
void build_command_target_mask(TargetMaskOwner& owner, CommandFrame& frame) noexcept { // 4340029E
    frame.field_1ec = 0;
    frame.field_1f0 = 0;
    const auto& state = owner.state;
    if (!owner.enabled || state.count < 2 || !state.has_accepted_tick)
        return;
    const auto& latest = state.records[state.first & 127u];
    for (std::uint32_t i = 0; i != state.count; ++i) {
        const auto& record = state.records[(state.first + i) & 127u];
        if (record.receive_tick != state.accepted_tick)
            continue;
        if (&record == &latest)
            return;
        frame.field_1ec = latest.targets[0] & ~record.targets[0];
        frame.field_1f0 = latest.targets[1] & ~record.targets[1];
        return;
    }
}
void mark_target_mask_attack(TargetMaskOwner& owner, const CommandFrame& frame,
                             const Command& command) noexcept { // 43386E3E
    if (!owner.enabled || !(frame.field_1ec || frame.field_1f0) || frame.weapon_kind_124 == 9 ||
        frame.weapon_kind_124 == 7)
        return;
    const auto flags = frame.flags;
    const auto buttons = command.movement.buttons;
    if (((flags & 0x100000u) || ((flags & 0x400u) && (buttons & 1u)) ||
         ((flags & 2u) && (buttons & 0x800u) &&
          (frame.weapon_id_d8 == 0x40 || frame.weapon_kind_124 == 0))) &&
        !(flags & 0x2000000u))
        owner.state.attack_pending = 1;
}

} // namespace skeet::client

// ============================================================================
// Subcomponent: target_mask_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
ClientRuntimeStorage& checked(ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
                              ConfigurationRuntimeOwner& config) {
    auto& data = storage.data();
    if (&engine.variables.data() != &data || &config.storage() != &storage ||
        engine.interfaces.core().engine_client != data.native_cell<void*>(0x4346A514u) ||
        !data.native_cell<void*>(0x4346A514u) || !data.native_cell<std::byte*>(0x43467B04u) ||
        !engine.variables.at(0x43467A78u).get())
        throw std::invalid_argument(
            "Target-mask requires the actual configuration, engine and latency state");
    return storage;
}
/* deduplicated registry helper */
}
TargetMaskRuntimeOwner::TargetMaskRuntimeOwner(ClientRuntimeStorage& storage,
                                               const EngineRuntimeBindings& engine,
                                               ConfigurationRuntimeOwner& config)
    : storage_(checked(storage, engine, config)),
      command_(config.target_mask(),
               {storage.data().native_cell<void*>(0x4346A514u),
                reinterpret_cast<const float*>(
                    storage.data().native_cell<const std::byte*>(0x43467B04u) + 0x20),
                &config.rage_settings().global.enabled.value, engine.variables.at(0x43467A78u),
                &storage.data().native_cell<double>(0x4347A500u),
                &storage.data().native_cell<double>(0x4347B118u)}),
      factories_{{registry(storage, 0x43475B78u), storage.command_phases(),
                  registry(storage, 0x43475B94u), storage.entity_observers(),
                  registry(storage, 0x43475A9Cu)},
                 command_,
                 config.target_mask(),
                 storage.data_owner()} {}
void TargetMaskRuntimeOwner::bind_initializers(NativeStartupSources& sources) {
    bind_native_target_mask_factory(sources, factories_);
}
}

// ============================================================================
// Subcomponent: target_refresh_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
std::uint32_t refresh_offset(ClientRuntimeStorage& storage, std::uint32_t identity) {
    const auto& g = storage.offsets().globals;
    for (const auto& row : engine::build_receive_offset_recipes())
        if (row.pointer_identity == identity)
            return std::uint32_t(std::ptrdiff_t(
                engine::DecodedOffset(g, identity, row.xor_key_identity, row.initial_xor_key, 0)));
    for (const auto& row : engine::build_data_map_offset_recipes())
        if (row.pointer_identity == identity)
            return std::uint32_t(std::ptrdiff_t(
                engine::DecodedOffset(g, identity, row.xor_key_identity, row.initial_xor_key, 0)));
    throw std::invalid_argument("Target refresh field has no recovered offset recipe");
}
/* [stripped helper] */
ClientRuntimeStorage& checked(ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
                              PlayerRuntimeOwner& players, HistoryRuntimeOwner& history,
                              engine::SceneQueryRuntimeOwner& scene,
                              animation::RuntimeOwner& animation,
                              MovementHudRuntimeOwner& movement) {
    if (&engine.variables.data() != &storage.data() || &players.storage() != &storage ||
        &history.storage() != &storage || &history.players() != &players ||
        &scene.storage() != &storage || &scene.players() != &players ||
        &animation.storage() != &storage || &animation.players() != &players.cache() ||
        &movement.storage() != &storage)
        throw std::invalid_argument(
            "Target refresh must share the actual engine, history, player, scene and movement owners");
    for (const auto identity : {0x43468EA4u, 0x43467A8Cu, 0x4346A568u})
        if (!engine.variables.at(identity).get())
            throw std::invalid_argument("Target refresh ConVar is not acquired");
    auto& data = storage.data();
    if (!data.native_cell<void*>(0x43475B90u) || !data.native_cell<void*>(0x4346A3A4u) ||
        !data.native_cell<void*>(0x434678F8u))
        throw std::invalid_argument(
            "Target refresh requires live frame, rules and genuine trace-filter publications");
    return storage;
}
}
struct TargetRefreshRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    PlayerRuntimeOwner& players;
    HistoryRuntimeOwner& history;
    engine::SceneQueryRuntimeOwner& scene;
    animation::RuntimeOwner& animation;
    MovementHudRuntimeOwner& movement;
    const EngineRuntimeBindings& engine;
    esp::EnginePlayers resources;
    anti_aim::TargetSelectorOffsets selector;
    anti_aim::TargetScoredOffsets scored;
    anti_aim::TargetGeometryOffsets geometry;
    std::uint32_t rule_first, rule_second, rule_mode, local_time;
    Data(ClientRuntimeStorage& s, const EngineRuntimeBindings& e, PlayerRuntimeOwner& p,
         HistoryRuntimeOwner& h, engine::SceneQueryRuntimeOwner& q, animation::RuntimeOwner& a,
         MovementHudRuntimeOwner& m)
        : storage(checked(s, e, p, h, q, a, m)), players(p), history(h), scene(q), animation(a),
          movement(m), engine(e), resources(esp::native_player_engine_bindings(s, e)),
          selector{refresh_offset(s, 0x434670CCu), refresh_offset(s, 0x4346A8E4u), refresh_offset(s, 0x43468A34u),
                   refresh_offset(s, 0x434685D4u)},
          scored{refresh_offset(s, 0x4346A60Cu), refresh_offset(s, 0x43467DD4u), refresh_offset(s, 0x43467908u),
                 refresh_offset(s, 0x43468C04u), refresh_offset(s, 0x434683C8u), refresh_offset(s, 0x43469344u) - 0x18u},
          geometry{a.bones().fields.studio_wrapper_32e4daf1, a.bones().fields.hitbox_set_c2b6fb10},
          rule_first(refresh_offset(s, 0x43468378u)), rule_second(refresh_offset(s, 0x4346836Cu)),
          rule_mode(refresh_offset(s, 0x43468BF4u)), local_time(refresh_offset(s, 0x43468D6Cu)) {}
};
TargetRefreshRuntimeOwner::TargetRefreshRuntimeOwner(ClientRuntimeStorage& s,
                                                     const EngineRuntimeBindings& e,
                                                     PlayerRuntimeOwner& p, HistoryRuntimeOwner& h,
                                                     engine::SceneQueryRuntimeOwner& q,
                                                     animation::RuntimeOwner& a,
                                                     MovementHudRuntimeOwner& m)
    : data_(std::make_unique<Data>(s, e, p, h, q, a, m)) {}
TargetRefreshRuntimeOwner::~TargetRefreshRuntimeOwner() = default;
ClientRuntimeStorage& TargetRefreshRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
void TargetRefreshRuntimeOwner::refresh(anti_aim::TargetState& target, void* player,
                                        const Command& command, std::uint32_t buttons,
                                        float yaw) const {
    auto& d = *data_;
    auto& data = d.storage.data();
    if (&target != &data.native_cell<anti_aim::TargetState>(0x4346DB14u))
        throw std::invalid_argument(
            "Target refresh must publish the canonical shared target state");
    //4335BA8E checks this before either selector or its engine/resource reads.
    if (*data.native_cell<const std::int32_t*>(0x434678F8u) == target.sequence)
        return;
    if (!player)
        throw std::invalid_argument("Target refresh requires its actual current player");
    const rage::TargetBindings targeting{player,
                                         data.native_cell<void*>(0x4346A890u),
                                         d.history.game_type(),
                                         d.history.teammates_are_enemies(),
                                         d.players.adjustments(),
                                         d.history.target_offsets()};
    const auto holder = data.native_cell<void**>(0x4346A3A4u);
    const auto rules = holder ? *holder : nullptr;
    const bool first = rules && field<std::uint8_t>(rules, d.rule_first) != 0;
    const bool second = first && field<std::uint8_t>(rules, d.rule_second) != 0;
    const anti_aim::TargetPredictionLimit prediction{
        rules != nullptr, first, second, second ? field<std::int32_t>(rules, d.rule_mode) : 0,
        data.native_cell<std::int32_t>(0x43469274u)};
    const anti_aim::TargetSelectorContext common{
        targeting,
        data.native_cell<void*>(0x4346A514u),
        data.native_cell<const std::int32_t*>(0x43469300u),
        anti_aim::TargetPlayerHistories(d.players.cache()),
        d.selector,
        data.native_cell<lagcomp::TickTimingSamples>(0x43478C70u),
        prediction,
        *data.native_cell<const float*>(0x4346A338u),
        field<float>(data.native_cell<void*>(0x43467B04u), 0x20),
        d.scene.queries().movement(),
        d.scene.queries().ballistics(),
        data.native_cell<std::uint8_t*>(0x434683D0u)};
    auto environment = d.movement.movement_environment();
    environment.interval = data.native_cell<const float*>(0x4346A338u);
    environment.entity_list = data.native_cell<void*>(0x4346A890u);
    environment.engine_trace_flag = data.native_cell<std::uint8_t*>(0x434683D0u);
    const auto& base = d.movement.movement_capture();
    const anti_aim::MovementCaptureInput movement{base.offsets, base.convars, *environment.interval,
                                                  &environment};
    const anti_aim::RawTargetTraceEntities entities{targeting.entity_list, d.scored};
    const anti_aim::LiveTargetScoredContext scored{
        common,
        command,
        buttons,
        yaw,
        d.resources,
        d.scored,
        d.geometry,
        data.native_cell<RageVector>(0x4347C9E0u),
        data.native_cell<HitboxMatrices>(0x4347C650u),
        d.animation.trigonometry(),
        data.native_cell<const std::int32_t*>(0x434678F8u),
        data.native_cell<const void* const*>(0x4346A3A4u),
        d.local_time,
        data.native_cell<void**>(0x43475B90u),
        entities,
        movement,
        environment};
    anti_aim::refresh_live_yaw_target(target, scored, d.engine.variables.at(0x43468EA4u).get());
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: verified_command.cpp
// ============================================================================

namespace skeet::client {
void publish_verified_command(VerifiedCommand& destination, const Command& source,
                              CommandChecksum checksum) {
    if (&destination.command != &source)
        std::memcpy(&destination.command, &source, sizeof source);
    destination.checksum = checksum ? checksum(&destination) : 0;
}
void invalidate_command_prediction(void* prediction) noexcept {
    auto* bytes = static_cast<std::byte*>(prediction);
    constexpr std::uint32_t invalid = UINT32_MAX, zero = 0, one = 1;
    std::memcpy(bytes + 0xC, &invalid, sizeof invalid);
    std::memcpy(bytes + 0x1C, &zero, sizeof zero);
    std::memcpy(bytes + 0x24, &one, sizeof one);
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: visual_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
/* [stripped helper] */
void require(bool condition, const char* message) {
    if (!condition)
        throw std::invalid_argument(message);
}
bool contains(const std::byte* module, const void* pointer, std::size_t size, std::uint32_t rva,
              std::uint32_t length) {
    const auto address = reinterpret_cast<std::uintptr_t>(pointer);
    const auto first = reinterpret_cast<std::uintptr_t>(module) + rva;
    return address >= first && size <= length && address - first <= length - size;
}
void require_interface(const ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
                       std::uint32_t cell) {
    const auto manifest = engine::build_interface_manifest();
    for (std::size_t index = 0; index < manifest.size(); ++index) {
        if (manifest[index].destination_cell != cell)
            continue;
        auto* object = storage.data().native_cell<void*>(cell);
        require(object && object == engine.interfaces.objects[index],
                "Visual runtime requires interfaces from the canonical engine bundle");
        return;
    }
    throw std::logic_error("Visual interface is absent from the build acquisition manifest");
}
void* acquire_key_values(const EngineRuntimeBindings& engine) {
    auto* module = static_cast<std::byte*>(engine.modules.at(u"vstdlib.dll"));
    require(module && read<std::uint16_t>(module) == 0x5A4D &&
                read<std::uint32_t>(module + 0x3C) == 0x108,
            "Projectile materials require the loaded vstdlib build");
    const auto* nt = module + 0x108;
    require(read<std::uint32_t>(nt) == 0x4550 && read<std::uint16_t>(nt + 4) == 0x14C &&
                read<std::uint16_t>(nt + 6) == 5 && read<std::uint16_t>(nt + 0x18) == 0x10B &&
                read<std::uint32_t>(nt + 8) == 0x63DC10BF &&
                read<std::uint32_t>(nt + 80) == 0x44000 && read<std::uint32_t>(nt + 88) == 0x4B1B5,
            "Unsupported projectile material vstdlib build");
    const auto* sections = nt + 24 + read<std::uint16_t>(nt + 20);
    constexpr std::array<std::array<std::uint32_t, 3>, 3> regions{{{0x1000, 0x277BA, 0x60000020},
                                                                   {0x29000, 0xF6F6, 0x40000040},
                                                                   {0x39000, 0x6B34, 0xC0000040}}};
    for (std::size_t index = 0; index < regions.size(); ++index) {
        const auto* section = sections + index * 40;
        require(read<std::uint32_t>(section + 12) == regions[index][0] &&
                    read<std::uint32_t>(section + 8) == regions[index][1] &&
                    read<std::uint32_t>(section + 36) == regions[index][2],
                "KeyValuesSystem module sections differ from the supported build");
    }
    // The PE export, not an old loader RVA, identifies this genuine getter.
    // Its complete build body is MOV EAX, singleton; RET. Reading the operand
    // has the same result and no skipped initializer or external side effect.
    const auto* getter =
        engine::resolve_pe_export(module, engine::netvar_name_hash("KeyValuesSystem"), 0, true);
    require(getter && contains(module, getter, 6, 0x1000, 0x277BA) &&
                getter[0] == std::byte{0xB8} && getter[5] == std::byte{0xC3},
            "KeyValuesSystem does not contain the proven singleton getter");
    auto* singleton = read<void*>(getter + 1);
    require(contains(module, singleton, 4, 0x39000, 0x6B34),
            "KeyValuesSystem singleton is outside the real module data");
    auto* table = read<const std::byte*>(static_cast<const std::byte*>(singleton));
    require(contains(module, table, 11 * 4, 0x29000, 0xF6F6),
            "KeyValuesSystem vtable is outside the real module read-only data");
    for (std::size_t slot = 0; slot < 11; ++slot)
        require(contains(module, read<const void*>(table + slot * 4), 1, 0x1000, 0x277BA),
                "KeyValuesSystem method is outside the real module executable section");
    return singleton;
}
void* checked_key_values(ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
                         ConfigurationRuntimeOwner& configuration, ui::StartupRenderer& renderer,
                         ui::RendererAssets& assets) {
    require(
        &engine.variables.data() == &storage.data() && &configuration.storage() == &storage &&
            &assets.storage() == &storage && &assets.startup_renderer() == &renderer &&
            &renderer.dpi() == &storage.menu_startup_data().dpi &&
            storage.data().native_cell<ui::RendererContainer*>(0x4347690C) == &renderer.renderer(),
        "Visual runtime must borrow one canonical storage, configuration, renderer and asset owner");
    for (const auto cell : {0x4346A514u, 0x43467F68u, 0x4346A890u, 0x4346A4E0u})
        require_interface(storage, engine, cell);
    require(storage.data().native_cell<void*>(0x4346A624) != nullptr,
            "Visual runtime requires the acquired genuine CCSViewRender inline object");
    require(storage.offsets().words && !storage.offsets().globals.pointers.empty(),
            "Projectile material bindings require the initialized canonical entity offsets");
    require(storage.data().native_cell<void*>(0x43468A7C) == engine.modules.at(u"vstdlib.dll"),
            "Projectile material singleton must belong to the canonical loaded vstdlib module");
    return acquire_key_values(engine);
}
esp::Projection initial_projection(engine::NativeDataImage& data) {
    // The native scales surround the matrix; Projection's semantic C++ layout
    // differs. Never reinterpret that whole BSS region as this source object.
    return {data.native_cell<std::array<float, 16>>(0x434798E0),
            data.native_cell<float>(0x43479920), data.native_cell<float>(0x434798DC)};
}
}
VisualRuntimeOwner::VisualRuntimeOwner(ClientRuntimeStorage& storage,
                                       const EngineRuntimeBindings& engine,
                                       ConfigurationRuntimeOwner& configuration,
                                       ui::StartupRenderer& renderer, ui::RendererAssets& assets)
    : storage_(storage), configuration_(configuration), renderer_(renderer), assets_(assets),
      key_values_(checked_key_values(storage, engine, configuration, renderer, assets)),
      ribbons_{storage.data().native_cell<void*>(0x4346A890),
               storage.data().native_cell<void*>(0x4346A4E0), key_values_,
               effects::ProjectileRibbonOffsets{storage.offsets().globals}},
      projection_(initial_projection(storage.data())) {
    refresh();
}
VisualRuntimeOwner::~VisualRuntimeOwner() = default;
ui::RendererBackend& VisualRuntimeOwner::renderer() const {
    auto* backend = renderer_.renderer().backend_if_alive();
    if (!backend)
        throw std::logic_error("The canonical visual renderer has released its context");
    return *backend;
}
engine::GameSystemCameraState VisualRuntimeOwner::camera_state() noexcept {
    auto& data = storage_.data();
    return {data.native_cell<std::array<float, 3>>(0x43479CCC),
            data.native_cell<std::array<float, 3>>(0x43479CD8), data.native_cell<float>(0x43468E80),
            data.native_cell<float>(0x4346D548), projection_};
}
void VisualRuntimeOwner::refresh() {
    auto& backend = renderer();
    screen_size_ = {backend.screen_width, backend.screen_height};
    indicators_.scale = renderer_.dpi().active_scale;
    indicators_.screen_height = backend.screen_height;
    indicators_.texture_source = &backend;
    const auto& icons = assets_.startup_icons();
    explosive_.texture = static_cast<std::uint32_t>(icons.grenade);
    backend.texture_size(explosive_.texture, &explosive_.width, &explosive_.height);
    fire_.texture = static_cast<std::uint32_t>(icons.inferno);
    backend.texture_size(fire_.texture, &fire_.width, &fire_.height);
}
void VisualRuntimeOwner::initialize_game_system(ShutdownRuntime& shutdown) {
    if (game_system_)
        throw std::logic_error("The canonical game-system runtime has already been composed");
    require(
        &shutdown.input == &storage_.input_runtime() &&
            &shutdown.exit_callbacks == &storage_.exit_callbacks() &&
            &shutdown.owners == &storage_.owner_registries().at(0x43475B78),
        "Game-system runtime requires shutdown over the canonical input, exits and owner registry");
    auto& data = storage_.data();
    game_system_ = std::make_unique<engine::GameSystemRuntime>(
        shutdown, storage_.owner_registries().at(0x43475988),
        storage_.owner_registries().at(0x43475AD8), data.native_cell<std::int32_t>(0x4346DB2C),
        data.native_cell<std::int32_t>(0x43476904),
        engine::GameSystemBindings{
            data.native_cell<void*>(0x4346A514), data.native_cell<void*>(0x4346A624),
            data.native_cell<void*>(0x43467F68), configuration_.miscellaneous().disable_pure,
            animation::native_sse_tangent, animation::native_sse_arctangent},
        camera_state());
}
engine::GameSystemRuntime& VisualRuntimeOwner::game_system() {
    if (!game_system_)
        throw std::logic_error(
            "Actual shutdown must exist before composing the game-system runtime");
    return *game_system_;
}
GameSystemFactoryContext VisualRuntimeOwner::game_system_factory() {
    return {storage_.data().native_cell<engine::GameSystemManager*>(0x43467FB0),
            &storage_.data().native_cell<void*>(0x43467204), game_system()};
}
} // namespace skeet::client

// ============================================================================
// Subcomponent: world_runtime_factories.cpp
// ============================================================================

namespace skeet::client {
namespace {
WorldRuntimeFactoryBindings& world_factory_bindings(void* opaque) {
    return *static_cast<WorldRuntimeFactoryBindings*>(opaque);
}
void* allocate(WorldRuntimeFactoryBindings& source, std::size_t native_size, std::size_t source_size) {
    return engine::allocate_owner_storage(source.lifecycle.memory,
                                          std::max(native_size, source_size));
}
void* required_storage(WorldRuntimeFactoryBindings& source, std::size_t native_size, std::size_t source_size) {
    auto* storage = allocate(source, native_size, source_size);
    // The five original OOM targets begin INT1 and remain unresolved. Throwing
    // bad_alloc is an explicit source failure policy, not a recovered VM edge.
    if (!storage)
        throw std::bad_alloc();
    return storage;
}
void smoke(void* opaque) { //4347D8D0 ->43480340
    auto& source = world_factory_bindings(opaque);
    auto* storage = allocate(source, 0x20, effects::smoke_owner_size());
    if (!storage) {
        source.smoke.active = nullptr;
        return;
    }
    effects::construct_smoke_owner(storage, {source.lifecycle, source.updates},
                                   source.configuration, source.smoke);
}
void scope(void* opaque) { //4348E3F9 ->43486002
    auto& source = world_factory_bindings(opaque);
    auto* storage = required_storage(source, 0x2C, effects::scope_owner_size());
    effects::construct_scope_owner(storage, {source.lifecycle, source.hud, source.updates},
                                   source.configuration, source.scope);
}
void game_system(void* opaque) { //4348E09F, including its nullable allocation path
    auto& source = world_factory_bindings(opaque);
    auto& context = source.game_system;
    engine::create_game_system_owner(source.lifecycle, context.manager, context.allocator,
                                     context.runtime);
}
void auto_accept(void* opaque) { //43481D50 ->4348E0E8
    auto& source = world_factory_bindings(opaque);
    auto* storage = allocate(source, 0x28, misc::auto_accept_owner_size());
    if (!storage) {
        source.auto_accept.active_owner = nullptr;
        return;
    }
    misc::construct_auto_accept(storage, source.lifecycle, source.configuration,
                                source.auto_accept);
}
void reveal_ranks(void* opaque) { //4348038F ->4347E8A6
    auto& source = world_factory_bindings(opaque);
    auto& context = source.reveal_ranks;
    // The existing whole factory allocates its larger source instance and
    // publishes null to both canonical consumers on allocation failure.
    misc::initialize_reveal_ranks(source.lifecycle, source.configuration, context.settings,
                                  context.active, context.engine);
}
void reveal_overwatch(void* opaque) { //43490411 ->4348E4CC
    auto& source = world_factory_bindings(opaque);
    auto& context = source.reveal_overwatch;
    auto* storage = required_storage(source, 0x24, misc::reveal_overwatch_owner_size());
    misc::construct_reveal_overwatch(storage, source.lifecycle, source.updates,
                                     source.configuration, context.settings, context.engine);
}
void disable_sv_pure(void* opaque) { //43484ED9 ->434864C4
    auto& source = world_factory_bindings(opaque);
    auto& context = source.disable_sv_pure;
    auto* storage = allocate(source, 0x20, misc::disable_sv_pure_owner_size());
    if (!storage) {
        context.active = nullptr;
        return;
    }
    auto& owner = misc::construct_disable_sv_pure_owner(storage, source.lifecycle, source.messages,
                                                        source.configuration, context.settings,
                                                        context.active);
    context.active = &owner;
}
void game_system_registration(void* opaque) { //4347DA14
    auto& source = world_factory_bindings(opaque);
    auto& context = source.game_system_registration;
    engine::create_game_system_registration(source.lifecycle, context.head_slot, context.callbacks,
                                            context.unload);
}
void persistent_kill_feed(void* opaque) { //43481E08 ->4347E910
    auto& source = world_factory_bindings(opaque);
    auto& context = source.persistent_kill_feed;
    auto* storage = required_storage(source, 0x3C, misc::persistent_kill_feed_owner_size());
    misc::construct_persistent_kill_feed_owner(storage, source.lifecycle, source.event_observers,
                                               source.frames, source.configuration,
                                               context.settings, context.engine);
}
void hit_markers(void* opaque) { //43484954 ->43486513
    auto& source = world_factory_bindings(opaque);
    auto& context = source.hit_markers;
    auto* storage = required_storage(source, 0x4C, events::hit_marker_owner_size());
    events::construct_hit_marker(storage, source.lifecycle, source.hud, source.configuration,
                                 context.events, context.bindings);
}
void window_lifecycle(void* opaque) { //4347F902 ->4347F555
    auto& source = world_factory_bindings(opaque);
    auto& context = source.window_lifecycle;
    auto* storage = required_storage(source, 0xC0, input::native_window_lifecycle_owner_size());
    input::construct_native_window_lifecycle(storage, source.lifecycle, source.command_owners,
                                             source.updates, context.persistence, context.imports,
                                             context.modules, context.images);
}
void harmless_molotov(void* opaque) { //43490982 ->43484AEA
    auto& source = world_factory_bindings(opaque);
    auto* storage = allocate(source, 0xA4, effects::harmless_molotov_owner_size());
    if (!storage) {
        source.harmless_molotov.effects.active = nullptr;
        return;
    }
    effects::construct_harmless_molotov_owner(storage, {source.lifecycle, source.updates},
                                              source.configuration, source.harmless_molotov);
}
} //namespace

void bind_world_runtime_factories(NativeStartupSources& sources,
                                  WorldRuntimeFactoryBindings& source) {
    if (&sources.data() != &source.command_owners.image())
        throw std::invalid_argument(
            "World factories require the scheduler's canonical command registry");
    if (&source.scope.runtime.settings != &source.scope.settings ||
        &source.smoke.runtime.settings != &source.scope.settings ||
        &source.harmless_molotov.settings != &source.scope.settings)
        throw std::invalid_argument("World factories require one canonical effects configuration");
    auto& system = source.game_system.runtime;
    if (&system.bindings.disable_pure != &source.disable_sv_pure.settings)
        throw std::invalid_argument("GameSystem and DisableSvPure require the same configuration");
    if (&system.updates != &source.updates.registry || &system.frames != &source.frames.registry ||
        &system.shutdown.owners != &source.lifecycle.registry ||
        &source.game_system_registration.unload.owners != &source.lifecycle.registry)
        throw std::invalid_argument(
            "World factories require the canonical dispatch and shutdown registries");
    sources.bind_initializer({0x4347D8D0u, &source, smoke});
    sources.bind_initializer({0x4348E3F9u, &source, scope});
    sources.bind_initializer({0x4348E09Fu, &source, game_system});
    sources.bind_initializer({0x43481D50u, &source, auto_accept});
    sources.bind_initializer({0x4348038Fu, &source, reveal_ranks});
    sources.bind_initializer({0x43490411u, &source, reveal_overwatch});
    sources.bind_initializer({0x43484ED9u, &source, disable_sv_pure});
    sources.bind_initializer({0x4347DA14u, &source, game_system_registration});
    sources.bind_initializer({0x43481E08u, &source, persistent_kill_feed});
    sources.bind_initializer({0x43484954u, &source, hit_markers});
    sources.bind_initializer({0x4347F902u, &source, window_lifecycle});
    sources.bind_initializer({0x43490982u, &source, harmless_molotov});
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: world_runtime_owner.cpp
// ============================================================================

namespace skeet::client {
namespace {
engine::DecodedOffset world_field(ClientRuntimeStorage& storage, std::uint32_t identity) {
    for (const auto& row : engine::build_receive_offset_recipes())
        if (row.pointer_identity == identity)
            return {storage.offsets().globals, identity, row.xor_key_identity, row.initial_xor_key,
                    0};
    for (const auto& row : engine::build_data_map_offset_recipes())
        if (row.pointer_identity == identity)
            return {storage.offsets().globals, identity, row.xor_key_identity, row.initial_xor_key,
                    0};
    throw std::logic_error("World field has no recovered receive/datamap recipe");
}
/* deduplicated registry helper */
ClientRuntimeStorage& checked(ClientRuntimeStorage& storage, const EngineRuntimeBindings& engine,
                              ConfigurationRuntimeOwner& configuration,
                              HistoryRuntimeOwner& history, VisualRuntimeOwner& visual,
                              effects::EffectsRuntimeOwner& effects,
                              engine::HookRuntimeOwner& hooks, WindowRuntimeOwner& window,
                              ConfigStorageOwner& presets, WorldRuntimeImports imports) {
    auto& data = storage.data();
    const auto core = engine.interfaces.core();
    if (&engine.variables.data() != &data || &configuration.storage() != &storage ||
        &history.storage() != &storage || &visual.storage() != &storage ||
        &effects.storage() != &storage || &hooks.storage() != &storage ||
        &window.storage() != &storage || &hooks.unload_state() != &window.unload_state() ||
        &presets.configuration() != &storage.configuration() ||
        &presets.native_storage() != &storage.config_storage() ||
        core.engine_client != data.native_cell<void*>(0x4346A514) ||
        core.entity_list != data.native_cell<void*>(0x4346A890) ||
        core.cvar != data.native_cell<void*>(0x4346A87C))
        throw std::invalid_argument(
            "World factories require the canonical engine, settings, history, effects, hook and window owners");
    if (&effects.world().settings != &configuration.effects_settings() ||
        &effects.view().settings != &configuration.effects_settings() ||
        &visual.game_system().shutdown.owners != &storage.owner_registries().at(0x43475B78))
        throw std::invalid_argument(
            "World factories require the same effects settings and initialized game-system context");
    if (!storage.offsets().words || storage.offsets().globals.pointers.empty() ||
        !data.native_cell<engine::MemberFunction<void*, const char*>>(0x4346A374) ||
        !data.native_cell<void*>(0x43467B04) || !core.cvar || !core.engine_client ||
        !core.entity_list)
        throw std::invalid_argument(
            "World factories require acquired netvars, HUD lookup, global clock and interfaces");
    if (!imports.proxy.crc32 || !imports.proxy.exchange || !imports.window.get_foreground_window ||
        !imports.window.open_mutex || !imports.window.close || !imports.window.delete_file ||
        !imports.window.time || !imports.window.default_window_procedure ||
        !imports.window.module_file_name || !imports.window.temp_path ||
        !imports.window.dos_to_nt_path || !imports.window.free_unicode || !imports.window.format ||
        !imports.window.append_wide || !imports.window.current_process_id)
        throw std::invalid_argument("World factories require their actual OS import boundaries");
    if (!data.native_cell<const std::byte*>(0x43467288) || !data.native_cell<void**>(0x43468B2C))
        throw std::invalid_argument(
            "World proxy requires the proven engine guard and live game-interface slot");
    return storage;
}
}
WorldRuntimeImports native_world_runtime_imports() {
    return {input::native_window_lifecycle_imports(), engine::native_game_window_proxy_imports()};
}

struct WorldRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    rage::LiveTargetRelationships relationships;
    effects::HarmlessMolotovBindings molotov;
    events::HitMarkerBindings marker_events;
    float marker_remaining = 0;
    misc::NativeText marker_sound{};
    misc::PersistentKillFeedBindings kill_feed;
    WorldRuntimeFactoryBindings factories;
    GameWindowProxyFactoryBindings proxy;

    Data(ClientRuntimeStorage& source, const EngineRuntimeBindings& engine,
         ConfigurationRuntimeOwner& configuration, HistoryRuntimeOwner& history,
         VisualRuntimeOwner& visual, effects::EffectsRuntimeOwner& effects,
         engine::HookRuntimeOwner& hooks, WindowRuntimeOwner& window, ConfigStorageOwner& presets,
         WorldRuntimeImports imports)
        : storage(checked(source, engine, configuration, history, visual, effects, hooks, window,
                          presets, imports)),
          relationships{source.data().native_cell<void*>(0x4346A890), history.game_type(),
                        history.teammates_are_enemies(), history.players().adjustments(),
                        history.target_offsets()},
          molotov{source.data().native_cell<void*>(0x4346A514),
                  source.data().native_cell<void*>(0x4346A890),
                  &source.data().native_cell<void*>(0x43475808),
                  std::uint32_t(std::ptrdiff_t(world_field(source, 0x43467FCC))),
                  source.data().native_cell<std::uint32_t>(0x43467C3C),
                  source.data().native_cell<std::uint32_t>(0x434685C8),
                  relationships,
                  hooks.registry(),
                  {&hooks.descriptor_storage().at(0x43475908),
                   &hooks.descriptor_storage().at(0x434758F4),
                   &hooks.descriptor_storage().at(0x434757F4)},
                  source.data().native_cell<effects::HarmlessMolotovRuntime*>(0x43475828)},
          marker_events{source.data().native_cell<void*>(0x4346A514),
                        source.data().native_cell<void*>(0x4346A890),
                        source.data().native_cell<void*>(0x434689C0),
                        relationships,
                        source.data().native_cell<std::uint8_t>(0x43468E58),
                        source.data().native_cell<std::int32_t*>(0x4346D558)},
          kill_feed{
              source.data().native_cell<void*>(0x4346A514),
              source.data().native_cell<void*>(0x43468994),
              source.data().native_cell<engine::MemberFunction<void*, const char*>>(0x4346A374),
              source.data().native_cell<void* const*>(0x4346A3A4),
              source.data().native_cell<void*>(0x43467B04),
              {world_field(source, 0x4346746C), world_field(source, 0x43468BE4), world_field(source, 0x43468D6C)}},
          factories{
              registry(source, 0x43475B78),
              registry(source, 0x43475988),
              registry(source, 0x43475A9C),
              registry(source, 0x43475AD8),
              registry(source, 0x434758E0),
              registry(source, 0x43475B94),
              source.command_phases(),
              source.configuration(),
              effects.smoke_bindings(),
              effects.scope_bindings(),
              visual.game_system_factory(),
              hooks.auto_accept_owner_bindings(),
              {configuration.miscellaneous().reveal_ranks,
               source.data().native_cell<misc::RevealRanksRuntime*>(0x434758F0),
               {source.data().native_cell<void*>(0x4346A87C),
                source.data().native_cell<void*>(0x4346D664),
                source.data().native_cell<void* const*>(0x43468250),
                source.data().native_cell<void* const*>(0x4346A3A4), world_field(source, 0x43468378)}},
              {configuration.miscellaneous().reveal_overwatch,
               source.data().native_cell<void*>(0x4346A514)},
              {configuration.miscellaneous().disable_pure,
               source.data().native_cell<misc::DisableSvPureOwner*>(0x43475974)},
              {&source.data().native_cell<engine::GameSystemRegistrationNode**>(0x43468D50),
               source.owner_registries().at(0x43475A84), window.unload_state()},
              {configuration.miscellaneous().persistent_kill_feed, kill_feed},
              {source.event_registry(),
               {configuration.player_esp_settings().hit_marker, marker_remaining, marker_sound,
                marker_events, source.data().native_cell<void*>(0x43467B04),
                world_field(source, 0x43468E78)}},
              {presets.persistence(), imports.window, window.startup_imports().modules,
               window.startup_imports().images},
              {configuration.effects_settings(), molotov}},
          proxy{source.data_owner(), source.owner_registries(), source.memory(),
                engine::native_game_window_proxy_bindings(source.data(), imports.proxy)} {}
};
WorldRuntimeOwner::WorldRuntimeOwner(ClientRuntimeStorage& storage,
                                     const EngineRuntimeBindings& engine,
                                     ConfigurationRuntimeOwner& configuration,
                                     HistoryRuntimeOwner& history, VisualRuntimeOwner& visual,
                                     effects::EffectsRuntimeOwner& effects,
                                     engine::HookRuntimeOwner& hooks, WindowRuntimeOwner& window,
                                     ConfigStorageOwner& presets)
    : WorldRuntimeOwner(storage, engine, configuration, history, visual, effects, hooks, window,
                        presets, native_world_runtime_imports()) {}
WorldRuntimeOwner::WorldRuntimeOwner(ClientRuntimeStorage& storage,
                                     const EngineRuntimeBindings& engine,
                                     ConfigurationRuntimeOwner& configuration,
                                     HistoryRuntimeOwner& history, VisualRuntimeOwner& visual,
                                     effects::EffectsRuntimeOwner& effects,
                                     engine::HookRuntimeOwner& hooks, WindowRuntimeOwner& window,
                                     ConfigStorageOwner& presets, WorldRuntimeImports imports)
    : data_(std::make_unique<Data>(storage, engine, configuration, history, visual, effects, hooks,
                                   window, presets, imports)) {}
WorldRuntimeOwner::~WorldRuntimeOwner() = default;
ClientRuntimeStorage& WorldRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
WorldRuntimeFactoryBindings& WorldRuntimeOwner::factory_bindings() noexcept {
    return data_->factories;
}
GameWindowProxyFactoryBindings& WorldRuntimeOwner::proxy_factory_bindings() noexcept {
    return data_->proxy;
}
const effects::HarmlessMolotovBindings& WorldRuntimeOwner::molotov_bindings() const noexcept {
    return data_->molotov;
}
events::HitMarkerBindings& WorldRuntimeOwner::hit_marker_bindings() noexcept {
    return data_->marker_events;
}
void WorldRuntimeOwner::bind_initializers(NativeStartupSources& sources) {
    bind_world_runtime_factories(sources, data_->factories);
    bind_game_window_proxy_factory(sources, data_->proxy);
}
} //namespace skeet::client

// ============================================================================
// Subcomponent: source_dll_shutdown.cpp
// ============================================================================

#include "skeet/client/source_dll_shutdown.hpp"
#include "skeet/client/shutdown.hpp"
#include "skeet/misc/unload.hpp"
#include <exception>
#include <stdexcept>

extern "C" {
skeet::client::SourceDllShutdownImports::FreeLibrary skeet_source_dll_free_library = nullptr;
}

namespace skeet::client {
namespace {
SourceDllShutdownProfile* shutdown_profile_active = nullptr;
SRWLOCK profile_lock = SRWLOCK_INIT;
struct ProfileGuard {
    ProfileGuard() {
        AcquireSRWLockExclusive(&profile_lock);
    }
    ~ProfileGuard() {
        ReleaseSRWLockExclusive(&profile_lock);
    }
};
}
SourceDllShutdownImports native_source_dll_shutdown_imports() {
    const auto kernel = GetModuleHandleW(L"kernel32.dll");
    const auto locate = kernel ? GetProcAddress(kernel, "GetModuleHandleExW") : nullptr;
    const auto release = kernel ? GetProcAddress(kernel, "FreeLibrary") : nullptr;
    if (!locate || !release)
        throw std::runtime_error("Loaded-DLL shutdown requires the genuine loader exports");
    return {reinterpret_cast<SourceDllShutdownImports::ModuleFromAddress>(locate),
            reinterpret_cast<SourceDllShutdownImports::FreeLibrary>(release)};
}
SourceDllShutdownProfile::SourceDllShutdownProfile(ShutdownRuntime& runtime, void* source_module)
    : SourceDllShutdownProfile(runtime, source_module, native_source_dll_shutdown_imports(), {}) {}
SourceDllShutdownProfile::SourceDllShutdownProfile(ShutdownRuntime& runtime, void* source_module,
                                                   SourceDllShutdownImports imports)
    : SourceDllShutdownProfile(runtime, source_module, imports, {}) {}
SourceDllShutdownProfile::SourceDllShutdownProfile(ShutdownRuntime& runtime, void* source_module,
                                                   SourceDllShutdownCompletion completion)
    : SourceDllShutdownProfile(runtime, source_module, native_source_dll_shutdown_imports(),
                               completion) {}
SourceDllShutdownProfile::SourceDllShutdownProfile(ShutdownRuntime& runtime, void* source_module,
                                                   SourceDllShutdownImports imports,
                                                   SourceDllShutdownCompletion completion)
    : runtime_(runtime), module_(source_module), imports_(imports), completion_(completion) {
    if (!module_ || !imports_.module_from_address || !imports_.free_library ||
        !runtime.imports.kill_timer || !runtime.imports.query_memory)
        throw std::invalid_argument(
            "Loaded-DLL shutdown requires its real module and complete finalization imports");
    if (completion_.context && !completion_.complete)
        throw std::invalid_argument(
            "Loaded-DLL application completion has no destruction callback");
    void* actual = nullptr;
    constexpr std::uint32_t from_address_unchanged_refcount = 4u | 2u;
    if (!imports_.module_from_address(
            from_address_unchanged_refcount,
            reinterpret_cast<const void*>(&source_dll_unload_timer_callback), &actual) ||
        actual != module_)
        throw std::invalid_argument(
            "Loaded-DLL shutdown module does not contain its actual timer entry");
}
SourceDllShutdownProfile::~SourceDllShutdownProfile() {
    ProfileGuard guard;
    cancel_deferred_request_locked();
    if (shutdown_profile_active == this) {
        shutdown_profile_active = nullptr;
        unbind_shutdown_runtime(runtime_);
    }
}
void SourceDllShutdownProfile::cancel_deferred_request_locked() noexcept {
    const auto window = deferred_window_;
    deferred_window_ = 0;
    if (window)
        (void)runtime_.imports.kill_timer(window, 0x332);
}
void SourceDllShutdownProfile::cancel_deferred_request() noexcept {
    ProfileGuard guard;
    cancel_deferred_request_locked();
}
bool SourceDllShutdownProfile::allow_request_locked() {
    switch (completion_.readiness(completion_.context)) {
    case SourceDllShutdownReadiness::ready:
        cancel_deferred_request_locked();
        return true;
    case SourceDllShutdownReadiness::failed:
        cancel_deferred_request_locked();
        return false;
    case SourceDllShutdownReadiness::waiting:
        if (!deferred_window_) {
            deferred_window_ = request_->main_menu.native_window;
            if (!request_->imports.set_timer(deferred_window_, 0x332, 250,
                                             &source_dll_deferred_request_timer_callback)) {
                deferred_window_ = 0;
                throw std::runtime_error(
                    "Source startup unload deferral could not create its actual window timer");
            }
        }
        return false;
    }
    std::terminate();
}
void bind_source_dll_shutdown(SourceDllShutdownProfile& profile, misc::UnloadRuntime& request) {
    ProfileGuard guard;
    auto& runtime = profile.runtime();
    if (shutdown_profile_active && shutdown_profile_active != &profile)
        throw std::logic_error("Another loaded-DLL shutdown profile is already bound");
    if (request.input != &runtime.input || request.hooks != &runtime.hooks ||
        &request.main_menu.native_window != &runtime.native_window)
        throw std::invalid_argument(
            "Loaded-DLL shutdown and unload must share the actual input, hooks and main window");
    if (profile.request_ && profile.request_ != &request)
        throw std::invalid_argument("Loaded-DLL shutdown already borrows another unload request");
    bind_shutdown_runtime(runtime);
    shutdown_profile_active = &profile;
    skeet_source_dll_free_library = profile.imports().free_library;
    request.timer_callback = &source_dll_unload_timer_callback;
    profile.request_ = &request;
    if (profile.completion_.readiness)
        request.request_gate = {&profile, [](void* value) {
                                    ProfileGuard guard;
                                    return shutdown_profile_active == value && shutdown_profile_active->allow_request_locked();
                                }};
}
void __stdcall source_dll_deferred_request_timer_callback(void*, std::uint32_t, std::uintptr_t,
                                                          std::uint32_t) {
    misc::UnloadRuntime* request = nullptr;
    {
        ProfileGuard guard;
        if (!shutdown_profile_active || !shutdown_profile_active->deferred_window_)
            return;
        if (shutdown_profile_active->allow_request_locked())
            request = shutdown_profile_active->request_;
    }
    if (request)
        request->request();
}
}

extern "C" void* skeet_source_dll_timer_finalize() {
    void* module;
    skeet::client::SourceDllShutdownImports::FreeLibrary release;
    skeet::client::SourceDllShutdownCompletion completion;
    skeet::client::ShutdownRuntime* runtime;
    {
        skeet::client::ProfileGuard guard;
        auto* profile = skeet::client::shutdown_profile_active;
        if (!profile)
            std::terminate();
        module = profile->module();
        release = profile->imports().free_library;
        completion = profile->completion();
        runtime = &profile->runtime();
        skeet::client::shutdown_profile_active = nullptr;
    }
    runtime->cancel_timer();
    (void)runtime->finalize();
    skeet::client::unbind_shutdown_runtime(*runtime);
    if (completion.complete)
        completion.complete(completion.context);
    skeet_source_dll_free_library = release;
    return module;
}

namespace skeet::client {
#if defined(__GNUC__) || defined(__clang__)
__attribute__((naked)) void __stdcall
source_dll_unload_timer_callback(void*, std::uint32_t, std::uintptr_t, std::uint32_t) {
#if defined(__i386__)
    __asm__("call _skeet_source_dll_timer_finalize\n\t"
            "popl %edx\n\t"
            "addl $16,%esp\n\t"
            "pushl %eax\n\t"
            "pushl %edx\n\t"
            "jmp *_skeet_source_dll_free_library");
#elif defined(__x86_64__)
    __asm__("subq $40,%rsp\n\t"
            "call skeet_source_dll_timer_finalize\n\t"
            "addq $40,%rsp\n\t"
            "movq %rax,%rcx\n\t"
            "jmp *skeet_source_dll_free_library(%rip)");
#endif
}
#elif defined(_MSC_VER) && defined(_M_IX86)
__declspec(naked) void __stdcall source_dll_unload_timer_callback(void*, std::uint32_t,
                                                                  std::uintptr_t, std::uint32_t) {
    __asm {
        call skeet_source_dll_timer_finalize
        pop edx
        add esp,16
        push eax
        push edx
        jmp dword ptr [skeet_source_dll_free_library]
    }
}
#endif
}
