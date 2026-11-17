#pragma once

// Client Core Declarations
#include "skeet/animation/entity_update.hpp"
#include "skeet/network/packet_control.hpp"
#include "skeet/prediction/history.hpp"
#include "skeet/rage/pending_shot_publication.hpp"
#include "skeet/rage/shot_diagnostics.hpp"
#include "skeet/rage/spread.hpp"
#include "skeet/rage/targeting_commit.hpp"
#include <variant>
#include "skeet/engine/owner_registry.hpp"
#include <vector>
#include <memory>
#include "skeet/anti_aim/owner_lifecycle.hpp"
#include "skeet/effects/owner_lifecycle.hpp"
#include "skeet/engine/native_owner_registry_storage.hpp"
#include "skeet/network/exploit_owner.hpp"
#include "skeet/network/packet_owner.hpp"
#include "skeet/script/owner.hpp"
#include "skeet/core.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/script/bridge.hpp"
#include <array>
#include "skeet/anti_aim/targets.hpp"
#include "skeet/engine/native_data_image.hpp"
#include "skeet/engine/preserved_initialization.hpp"
#include <cstddef>
#include <cstdint>
#include "skeet/animation/bones.hpp"
#include "skeet/lagcomp/entity_state.hpp"
#include "skeet/anti_aim/config.hpp"
#include "skeet/legit/command.hpp"
#include "skeet/legit/command_source.hpp"
#include "skeet/hud/penetration_runtime.hpp"
#include "skeet/hud/screen_overlay_update.hpp"
#include <span>
#include "skeet/client/startup_reset.hpp"
#include <optional>
#include "skeet/prediction/command.hpp"
#include "skeet/weapon/fire_state.hpp"
#include "skeet/rage_hitboxes.hpp"
#include "skeet/engine/startup_convars.hpp"
#include "skeet/config.hpp"
#include "skeet/network/data_update_observers.hpp"
#include "skeet/network/exploit_observers.hpp"
#include "skeet/prediction/observer_registry.hpp"
#include "skeet/network/exploit_recharge_receiver.hpp"
#include "skeet/prediction/observer_receiver.hpp"
#include "skeet/network/data_update_receiver.hpp"
#include <functional>
#include "skeet/client/entity_observer_registry.hpp"
#include "skeet/menu/startup_registry.hpp"
#include <stdexcept>
#include "skeet/engine/game_window_proxy.hpp"
#include "skeet/prediction/entity_update.hpp"
#include "skeet/lagcomp/history_owner.hpp"
#include "skeet/lagcomp/native_history_boundaries.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/misc/automatic_weapons_owner.hpp"
#include "skeet/misc/clan_tag_owner.hpp"
#include "skeet/misc/grenade_owner.hpp"
#include "skeet/misc/knife_owner.hpp"
#include "skeet/misc/low_fps_runtime.hpp"
#include "skeet/misc/no_fall_damage.hpp"
#include "skeet/misc/quick_switch_owner.hpp"
#include "skeet/misc/rebuy_fix_owner.hpp"
#include "skeet/misc/zeus_owner.hpp"
#include "skeet/movement/owner_lifecycle.hpp"
#include "skeet/movement/blockbot_owner.hpp"
#include "skeet/hud/screen_overlay_owner.hpp"
#include "skeet/hud/penetration_reticle_owner.hpp"
#include "skeet/movement/extrapolation.hpp"
#include "skeet/movement/release_command.hpp"
#include "skeet/engine/bindings.hpp"
#include "skeet/movement/global_config.hpp"
#include "skeet/skins/config.hpp"
#include <unordered_map>
#include "skeet/engine/unload.hpp"
#include "skeet/engine/offset_storage.hpp"
#include "skeet/animation/layer_receive.hpp"
#include "skeet/players/state.hpp"
#include "skeet/engine/jobs.hpp"
#include "skeet/esp/player_geometry.hpp"
#include "skeet/lagcomp/history.hpp"
#include "skeet/resolver_runtime/environment_update.hpp"
#include "skeet/animation/local_prediction_pose_inputs.hpp"
#include "skeet/esp/other_config.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/rage/target_relationships.hpp"
#include "skeet/network/exploit_runtime.hpp"
#include "skeet/animation/local_helpers.hpp"
#include "skeet/engine/source_initialization_imports.hpp"
#include "skeet/engine/hook_descriptors.hpp"
#include "skeet/input/window_hook.hpp"
#include "skeet/skins/sequence_owner.hpp"
#include "skeet/rage/owner.hpp"
#include "skeet/legit/owner.hpp"
#include "skeet/esp/owner_runtime.hpp"
#include "skeet/effects/blur_runtime.hpp"
#include "skeet/misc/console_output_owner.hpp"
#include "skeet/misc/fov_owner.hpp"
#include "skeet/misc/purchase_log_owner.hpp"
#include "skeet/config_storage/preserved.hpp"
#include "skeet/engine/interface_discovery.hpp"
#include "skeet/engine/build_module_bindings.hpp"
#include "skeet/engine/build_startup_convars.hpp"
#include "skeet/engine/build_animation_bindings.hpp"
#include "skeet/engine/build_command_bindings.hpp"
#include "skeet/input/runtime.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/client/command_empty_tick.hpp"
#include "skeet/client/shutdown.hpp"
#include "skeet/events/registry.hpp"
#include "skeet/menu/settings_menu_storage.hpp"
#include "skeet/menu/startup_data.hpp"
#include "skeet/network/shared_esp.hpp"
#include "skeet/menu/window_lifetime.hpp"
#include "skeet/script/shutdown.hpp"
#include "skeet/misc/unload.hpp"
#include "skeet/skins/agent_owner.hpp"
#include "skeet/skins/cosmetic_owners.hpp"
#include "skeet/skins/inventory_owner.hpp"
#include "skeet/engine/build_material_bindings.hpp"
#include "skeet/engine/build_skin_bindings.hpp"
#include "skeet/anti_aim/runtime_owner.hpp"
#include "skeet/script/api_runtime_owner.hpp"
#include "skeet/effects/projectile_ribbons.hpp"
#include "skeet/engine/game_system.hpp"
#include "skeet/esp/indicators.hpp"
#include "skeet/effects/harmless_molotov_owner.hpp"
#include "skeet/effects/scope_owner.hpp"
#include "skeet/effects/smoke_owner.hpp"
#include "skeet/events/hit_marker_owner.hpp"
#include "skeet/input/native_window_lifecycle.hpp"
#include "skeet/misc/auto_accept_owner.hpp"
#include "skeet/misc/disable_sv_pure_owner.hpp"
#include "skeet/misc/persistent_kill_feed.hpp"
#include "skeet/misc/reveal_overwatch.hpp"
#include "skeet/misc/reveal_ranks.hpp"


// Declarations from buffered_shot_command.hpp

namespace skeet::rage {
struct BufferedShotRepairBindings;
}
namespace skeet::client {

// One physical owner, native 43475968. The existing diagnostic producer writes
// diagnostics directly; phase6 consumes it alongside the retained frame ring.
struct BufferedShotCommandStorage {
    rage::ShotDiagnosticHistory diagnostics;
    std::array<CommandFrame, 150> frames;
    BufferedShotCommandStorage() noexcept; // true constructor 4347FE52
};
static_assert(offsetof(BufferedShotCommandStorage, frames) == 0x1C0);

struct BufferedShotCommandOffsets {
    std::ptrdiff_t simulation_time; // D5FD2E2C
    std::ptrdiff_t local_field_7f5ed2d0;
    std::ptrdiff_t target_rejection; // 9D920E35
    std::ptrdiff_t target_flags; // E2FBE695
    std::ptrdiff_t target_duck; // 3309C0CF
};
struct BufferedShotCommandConvars {
    void* interpolation_rate; // 4346747C XOR 03B4819C
    void* interpolation_minimum; // 4346A748 XOR 7D9DC86F
    void* interpolation_ratio; // 4346A330 XOR BE64D223
    void* recoil_scale; // 4346794C XOR C429E1BF
    void* command_tick_offset; // 43468528 XOR F75367FE
    void* shotgun_patterns; // 43468520 XOR 394BFF56
};
struct BufferedShotCommandTiming {
    void* command_clock; // actual 43475A60, base tick +4
    const std::int32_t* acknowledged; // nullable 43468F9C
    const std::int32_t* sent; // nullable 434670A8
};
struct BufferedShotCommandBindings {
    const std::uint8_t& enabled; // 43467224
    const std::uint8_t& firing_mode_enabled; // 434684E0
    const std::uint8_t& send_mode; // 43468D7C
    const std::int32_t& choked; // *434757DC
    const std::int32_t& last_outgoing; // *43475B2C
    const std::int32_t& maximum_unlag_ms; // 43468230
    const float& clock_time; // *4346A334
    const float& interval; // *4346A338
    const float& interpolation_floor; // 43469254
    float& prediction_time; // *43467EB8
    CommandChecksum const& checksum; // optional actual engine 43467C48
    Command* const& commands; // genuine CInput +F0
    VerifiedCommand* const& verified; // genuine CInput +F4
    EntityCommandOwner* const& history_owner; //live43475898,4336B29A/433952F0
    void* engine_client; // 4346A514, command getter slot205
    void* entity_list; // 4346A890, handle-to-networkable slot1
    PlayerStateCache& players;
    BufferedShotCommandOffsets offsets;
    BufferedShotCommandConvars convars;
    ScriptCommandEnvironment command_environment;
    const network::LatencySamples& latency;
    const BufferedShotCommandTiming& timing;
    const CommandButtonPolicy& button_policy;
    const CommandTrigonometry& trigonometry;
    const rage::SpreadTables& spread_tables;
    const rage::SpreadRandom& random;
    const rage::ShotgunPattern& shotgun_pattern;
    const RageTrigonometry& spread_trigonometry;
    rage::TargetAngleMath angle_math; // genuine CRT atan2
    float (*arctangent)(float); // genuine CRT __libm_sse2_atanf
    std::array<animation::AnimationAngleSample, 2>& animation_angles; // 434759DC/E8
    const rage::PendingShotPublicationBindings& pending_shots;
    const rage::BufferedShotRepairBindings& repair;
};
class BufferedShotCommandSource {
public:
    virtual ~BufferedShotCommandSource() = default;
    virtual void prepare(BufferedShotCommandStorage&) noexcept = 0;
    virtual void repair(BufferedShotCommandStorage&, CommandFrame&, void* player,
                        std::uint8_t send) = 0;
};
struct BufferedShotCommandOwner {
private:
    std::variant<const BufferedShotCommandBindings*, BufferedShotCommandSource*> source_;

public:
    BufferedShotCommandStorage& state;
    BufferedShotCommandOwner(BufferedShotCommandStorage& state,
                             const BufferedShotCommandBindings& bindings) noexcept
        : source_(&bindings), state(state) {}
    BufferedShotCommandOwner(BufferedShotCommandStorage& state,
                             BufferedShotCommandSource& source) noexcept
        : source_(&source), state(state) {}
    const BufferedShotCommandBindings* decoded() const noexcept {
        if (auto* value = std::get_if<const BufferedShotCommandBindings*>(&source_))
            return *value;
        return nullptr;
    }
    BufferedShotCommandSource* live() const noexcept {
        if (auto* value = std::get_if<BufferedShotCommandSource*>(&source_))
            return *value;
        return nullptr;
    }
};

inline constexpr std::uint32_t buffered_shot_command_vtable = 0x4346F01Cu;
// Complete phase2 entry 433E4CB6. Other phase2 arguments are unused.
void prepare_buffered_shot_commands(BufferedShotCommandOwner&) noexcept;
// Complete phase6 entry 43323284. Current command is not a native input;
// historical commands come from the genuine raw/verified CInput rings.
void repair_buffered_shot_commands(BufferedShotCommandOwner&, CommandFrame&, void* player,
                                   std::uint8_t send_packet);

} // namespace skeet::client

// Declarations from buffered_shot_lifecycle.hpp

namespace skeet::engine {
class NativeOwnerRegistryStorage;
}
namespace skeet::client {
class NativeStartupSources;
class BufferedShotLifecycleOwner;
struct BufferedShotLifecycleBindings {
    engine::OwnerRegistryBinding primary; //canonical 43475B78/7C/80
    CommandPhaseRegistry& commands; //canonical normal-command order
    BufferedShotCommandOwner& command;
    // The one full-owner slot43475968. The factory publishes it after
    // construction; the constructor itself performs no global publication.
    BufferedShotLifecycleOwner*& active;
};

inline constexpr std::uint32_t buffered_shot_constructor = 0x4347FE52u;
inline constexpr std::uint32_t buffered_shot_primary_vtable = 0x4346FC00u;
inline constexpr std::uint32_t buffered_shot_factory = 0x43485BE3u;
inline constexpr std::size_t buffered_shot_native_size = 0x126B8u;

std::size_t buffered_shot_lifecycle_size() noexcept;
// Complete ordinary4347FE52 on caller storage carrying the43387448 header.
// Primary registration precedes F01C command registration, primary table
// publication, then sparse construction of the ONE borrowed state owner.
// No configuration, empty-tick, data-update or other registry is added.
// The F01C interface uses the existing typed phase-registry representation.
// The borrowed command/state/registries/active cell must outlive this object.
BufferedShotLifecycleOwner& construct_buffered_shot_lifecycle(void* storage,
                                                              BufferedShotLifecycleBindings);

// Complete43485BE3..43485C01: allocate126B8/HEAP_ZERO_MEMORY and construct on
// success. Both branches publish the returned owner/null to the same active
// cell. The five trapped bytes at43485BFC encode the MOV[43475968],EAX
// recorded in mem_int3.h:560; the ordinary suffix at43485C01 is RET.
BufferedShotLifecycleOwner* create_buffered_shot_owner(BufferedShotLifecycleBindings);

// Complete primary slot0: clear active first, then433837D0 header-free.
// Native destruction leaves the primary and command registry entries intact.
void destroy_buffered_shot_lifecycle(BufferedShotLifecycleOwner*,
                                     BufferedShotLifecycleOwner*& active);
// Complete primary slot7,433D6C8F: count0 and all150 command tags -1 only.
void reset_buffered_shot_lifecycle(BufferedShotLifecycleOwner&) noexcept;
// Complete primary slot5,433B06E3: call the current primary slot7.
void refresh_buffered_shot_lifecycle(BufferedShotLifecycleOwner&);
void* buffered_shot_lifecycle_object(BufferedShotLifecycleOwner&) noexcept;
// Resolve views from the one live full-owner slot. No command/state/history
// pointer is published in another canonical cell.
BufferedShotCommandOwner* buffered_shot_command_owner(BufferedShotLifecycleOwner*) noexcept;
BufferedShotCommandStorage* buffered_shot_command_storage(BufferedShotLifecycleOwner*) noexcept;
rage::ShotDiagnosticHistory* buffered_shot_diagnostics(BufferedShotLifecycleOwner*) noexcept;

// Stable source binding for the actual parameterless43485BE3 initializer.
// The image-backed registry and active pointer cell are checked by the binder
// before any factory call. Retain this context through startup invocation.
struct BufferedShotFactoryBindings {
    engine::NativeOwnerRegistryStorage& registries;
    const BufferedShotLifecycleBindings lifecycle;
    BufferedShotFactoryBindings(engine::NativeOwnerRegistryStorage& registries,
                                BufferedShotLifecycleBindings lifecycle) noexcept
        : registries(registries), lifecycle(lifecycle) {}
    BufferedShotFactoryBindings(const BufferedShotFactoryBindings&) = delete;
    BufferedShotFactoryBindings& operator=(const BufferedShotFactoryBindings&) = delete;
    BufferedShotFactoryBindings(BufferedShotFactoryBindings&&) = delete;
    BufferedShotFactoryBindings& operator=(BufferedShotFactoryBindings&&) = delete;
};
// Register the confirmed factory identity only. The existing live startup
// scheduler selects when it runs; binding does not construct or publish an owner.
void bind_buffered_shot_factory(NativeStartupSources&, BufferedShotFactoryBindings&);
}

// Declarations from buffered_shot_runtime_owner.hpp

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::esp {
class EspRuntimeOwner;
}
namespace skeet::rage {
class RuntimeOwner;
class FeedbackRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class CommandCaptureRuntimeOwner;
class CrtRuntimeOwner;
struct EngineRuntimeBindings;
//43485BE3's one canonical diagnostic/frame store. Each phase6 invocation
// composes timing, input rings, movement-button policy and actual RAGE repair
// from its live arguments. Construction captures no player or command.
class BufferedShotRuntimeOwner final {
public:
    BufferedShotRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                             ConfigurationRuntimeOwner&, HistoryRuntimeOwner&,
                             CommandCaptureRuntimeOwner&, animation::RuntimeOwner&,
                             CrtRuntimeOwner&, esp::EspRuntimeOwner&, rage::RuntimeOwner&,
                             rage::FeedbackRuntimeOwner&);
    ~BufferedShotRuntimeOwner();
    BufferedShotRuntimeOwner(const BufferedShotRuntimeOwner&) = delete;
    BufferedShotRuntimeOwner& operator=(const BufferedShotRuntimeOwner&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    BufferedShotCommandOwner& command() noexcept;
    BufferedShotCommandStorage& state() noexcept;
    BufferedShotFactoryBindings& factory_bindings() noexcept;
    void bind_initializer(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from combat_runtime_factories.hpp

namespace skeet::client {
class NativeStartupSources;

struct PacketRuntimeFactoryBindings {
    engine::OwnerRegistryBinding lifecycle;
    CommandPhaseRegistry& commands;
    anti_aim::FakeLagSettings& settings;
    network::PacketCommandOwner& command;
};
struct ExploitRuntimeFactoryBindings {
    network::ExploitOwnerRegistries registries;
    network::ExploitOwnerBindings owner;
};
struct AntiAimRuntimeFactoryBindings {
    anti_aim::OwnerRegistries registries;
    anti_aim::OwnerBindings owner;
};
struct EffectsRuntimeFactoryBindings {
    effects::EffectsOwnerRegistries registries;
    effects::EffectsOwnerBindings owner;
};
struct ScriptRuntimeFactoryBindings {
    script::ScriptRuntime& runtime;
    script::ScriptOwnerBindings owner;
    script::ScriptModulesBindings& modules;
};

// All views borrow their existing configuration, command state and registries.
// Keep this context and its borrowed objects at stable addresses through startup
// and through the final registered callbacks. Binding creates no owner.
struct CombatRuntimeFactoryBindings {
    std::shared_ptr<engine::NativeDataImage> data;
    engine::NativeOwnerRegistryStorage& registries;
    ConfigRegistries& configuration;
    PacketRuntimeFactoryBindings packet;
    ExploitRuntimeFactoryBindings exploit;
    AntiAimRuntimeFactoryBindings anti_aim;
    EffectsRuntimeFactoryBindings effects;
    ScriptRuntimeFactoryBindings scripting;
};

inline constexpr std::uint32_t packet_runtime_factory = 0x4347DA84u;
inline constexpr std::uint32_t exploit_runtime_factory = 0x4347FF2Fu;
inline constexpr std::uint32_t anti_aim_runtime_factory = 0x43485CCCu;
inline constexpr std::uint32_t effects_runtime_factory = 0x43484F93u;
inline constexpr std::uint32_t script_runtime_factory = 0x4348E608u;

// These are source allocations: retain the native minimum and aligned heap
// header while reserving room for additional typed source binding metadata.
// Packet/exploit allocation failure reaches unresolved native INT1/junk; the
// source failure policy throws bad_alloc before construction or registration.
// AA/effects/script preserve their actual null singleton publication branches.
void* create_packet_runtime_owner(CombatRuntimeFactoryBindings&);
void* create_packet_runtime_owner(PacketRuntimeFactoryBindings&, ConfigRegistries&);
void* create_exploit_runtime_owner(CombatRuntimeFactoryBindings&);
void* create_anti_aim_runtime_owner(CombatRuntimeFactoryBindings&);
void* create_anti_aim_runtime_owner(AntiAimRuntimeFactoryBindings&, ConfigRegistries&);
void* create_effects_runtime_owner(CombatRuntimeFactoryBindings&);
void* create_effects_runtime_owner(EffectsRuntimeFactoryBindings&, ConfigRegistries&);
script::ScriptOwner* create_script_runtime_owner(CombatRuntimeFactoryBindings&);
// Same4348E608 factory, independently usable once its concrete Lua/module
// dependencies exist. No other combat owner is constructed by this overload.
script::ScriptOwner* create_script_runtime_owner(ScriptRuntimeFactoryBindings&);

// Validates the common image-backed cells and the canonical shared state views,
// then binds these five real factory identities to the startup dispatcher.
void bind_combat_runtime_factories(NativeStartupSources&, CombatRuntimeFactoryBindings&);
} // namespace skeet::client

// Declarations from commands.hpp

namespace skeet::client {

// The complete 0x64-byte command copied by 433D333B. The leading word is an
// x86 vtable address, even when the recovered algorithms are built for x64.
struct Command {
    std::uint32_t vtable;
    std::int32_t number;
    std::int32_t tick;
    std::array<float, 3> angles;
    std::array<float, 3> aim_direction;
    UserCommand movement;
    std::uint8_t impulse;
    std::array<std::uint8_t, 3> impulse_padding;
    std::int32_t weapon_select;
    std::int32_t weapon_subtype;
    std::int32_t random_seed;
    std::int16_t mouse_x;
    std::int16_t mouse_y;
    std::uint8_t predicted;
    std::array<std::uint8_t, 3> predicted_padding;
    std::array<float, 3> saved_angles;
    std::uint32_t saved_buttons;
    std::array<std::uint32_t, 2> trailing;
};
static_assert(sizeof(Command) == 0x64);
static_assert(offsetof(Command, tick) == 8);
static_assert(offsetof(Command, angles) == 0xC);
static_assert(offsetof(Command, movement) == 0x24);
static_assert(offsetof(Command, saved_angles) == 0x4C);
static_assert(offsetof(Command, saved_buttons) == 0x58);

struct ChannelHookState {
private:
    void** standalone_expected_ = nullptr;
    void** standalone_replacement_ = nullptr;
    void* standalone_channel_ = nullptr;

public:
    ChannelHookState() noexcept
        : expected_table(standalone_expected_), replacement_table(standalone_replacement_),
          installed_channel(standalone_channel_) {}
    ChannelHookState(void**& expected, void**& replacement, void*& channel) noexcept
        : expected_table(expected), replacement_table(replacement), installed_channel(channel) {}
    ChannelHookState(const ChannelHookState&) = delete;
    ChannelHookState& operator=(const ChannelHookState&) = delete;
    ChannelHookState(ChannelHookState&&) = delete;
    ChannelHookState& operator=(ChannelHookState&&) = delete;
    void**& expected_table; // live43475B0C
    void**& replacement_table; // live43475B14
    void*& installed_channel; // live43475B08
};
// Complete 433CBDFD. Engine-client slot 78 obtains the channel. A replacement
// is installed only while its table still equals the recorded expected table.
void restore_channel_hook(void* engine_client, ChannelHookState&);

struct CommandEntryBindings {
    void* const& engine_client; // live4346A514
    void** original_player_table; // hook metadata's original vtable
    ChannelHookState* channel_hook;
    std::array<std::ptrdiff_t, 2> rejection_bytes; // 43468E78, 4346A5DC
};
enum class CommandPath : std::uint32_t {
    engine_return,
    empty_tick,
    active_command,
};
struct CommandEntry {
    std::uint8_t original_result;
    CommandPath path;
};
// Exact framing/gates 433D2E40..433D2F05, with the active path continuing at
// 433D2F95. Calls the original player CreateMove slot 289 before testing
// engine slots 82/93. The empty-tick path alone tests both rejection bytes.
// The later internal feature phases are separate recovered runtime units.
CommandEntry enter_create_move(const CommandEntryBindings&, void* player, float sample_time,
                               Command&);
//Production433D2E40 framing reads the current receiver's hook metadata AFTER
//channel restoration, then calls its genuine original player virtual289.
CommandEntry enter_hooked_create_move(const CommandEntryBindings&, void* player, float sample_time,
                                      Command&);

struct CommandEntityBindings {
    void* const& entity_list; // live4346A890
    std::ptrdiff_t overridden_property; // decoded 4346A808
    std::uint32_t* const& replacement_value; // live43469060
};
using CommandEntitySnapshot = std::array<std::uint32_t, 64>;
// Complete temporary entity override at 433D30CC..433D3174 and matching
// restoration 433D5953..433D59DB. Indices are 1..64, indexed by (index & 63).
// Native restoration intentionally skips saved zero values and rechecks both
// lookup and player virtual slot 158, retaining those observable side effects.
void save_command_entity_properties(const CommandEntityBindings&, CommandEntitySnapshot&);
void restore_command_entity_properties(const CommandEntityBindings&, const CommandEntitySnapshot&);

// Exact final command snapshot 433D5868..433D587A; copies the angle words
// sequentially, preserving all bit patterns, including NaNs.
void save_final_command(Command&) noexcept;

struct ScriptCommand {
    std::int32_t choked_commands;
    std::int32_t command_number;
    std::array<float, 3> angles;
    float forward_move, side_move;
    std::uint8_t send_packet, force_defensive;
    std::array<std::uint8_t, 2> padding;
    std::uint32_t buttons;
    std::int32_t weapon_select, weapon_subtype;
    float move_yaw;
    std::uint8_t exploit_state, state_bit_3, state_bit_18, state_bit_20;
};
static_assert(sizeof(ScriptCommand) == 0x34);
static_assert(offsetof(ScriptCommand, buttons) == 0x20);

// Exact snapshot and apply blocks of 43347D5C. The callback sees the same
// ScriptCommand object, which the native runtime wraps using registry slot 1.
struct ScriptCommandBaseline {
    std::uint8_t send_packet, exploit_state, state_bit_18;
};
ScriptCommandBaseline capture_script_command(ScriptCommand&, const Command&, float move_yaw,
                                             std::uint8_t send_packet, std::int32_t choked_commands,
                                             std::uint32_t scheduler_flags) noexcept;
void apply_script_command(ScriptCommand&, Command&, float& move_yaw, std::uint8_t& send_packet,
                          std::uint32_t& scheduler_flags, ScriptCommandBaseline,
                          bool unrestricted_angles, bool block_roll) noexcept;

struct ScriptCommandEnvironment {
    const std::uint8_t* angle_guard; // 434693EC
    void* const* shared_player; // 4346A3A4; may itself be null
    std::ptrdiff_t player_guard; // decoded 43468378
};
bool command_angles_unrestricted(const ScriptCommandEnvironment&) noexcept; // 433C0714
bool command_roll_blocked(const ScriptCommandEnvironment&) noexcept; // 433240A6
struct ScriptCommandBindings {
    script::Bridge* bridge;
    ScriptCommand* snapshot; // 43479300
    const char* event_name; // initialized 434795E0
    const std::int32_t* choked_commands; // *434757DC
    std::uint8_t* marshalling; // 43475A5C
    // Actual Lua 5.1 lua_pushlightuserdata, native entry 43429132.
    void (*push_light_userdata)(void*, void*);
    ScriptCommandEnvironment environment;
};
// Complete 43347D5C -> 433BA445 -> 4334AD41 dispatch/apply chain. Reuses the
// recovered callback-list dispatcher; the runtime Lua table's own constructor
// is called through its real registry reference, with no feature callbacks.
int dispatch_command_script(const ScriptCommandBindings&, Command&, float& move_yaw,
                            std::uint8_t& send_packet, std::uint32_t& scheduler_flags);

struct CommandAngleBindings {
    void* const& engine_client;
    void* const& prediction;
    ScriptCommandEnvironment environment;
};
// Complete 433B8974: sanitize NaNs, normalize under the recovered trust guard,
// clear roll, and call actual engine-client SetViewAngles (slot 19).
void set_command_view_angles(const CommandAngleBindings&, std::array<float, 3>&);
struct FreeLookState {
    std::array<std::uint32_t, 3> prefix;
    std::array<float, 3> delta;
    std::uint8_t active;
    std::array<std::uint8_t, 3> padding;
    std::array<float, 3> base_angles;
};
static_assert(sizeof(FreeLookState) == 0x28);
// Complete 4341B096; the unused first native stack argument is omitted.
// Uses the real hotkey evaluator, then engine slot 19 and, for tick zero,
// prediction SetLocalViewAngles (slot 13), in native order.
void process_command_free_look(FreeLookState&, Command&, const HotkeyBinding&, InputBitmap,
                               const CommandAngleBindings&);

} // namespace skeet::client

// Declarations from command_angle_retention.hpp

namespace skeet::client {
// Inline 433D415B..433D4377. The previous command comes directly from
// CInput +F0, with an exact unsigned command-number check. This phase changes
// the command only; native global 434759C0 is separate from frame.flags.
void retain_command_attack_angles(const CommandFrame&, Command&, const void* player,
                                  const CommandMouseOffsets&, const void* input) noexcept;

// Inline gate 433D4377..433D4396, including its branch at 433D4600. variable
// is the resolved 43468E94 XOR 2DC3CC20 ConVar, which may be null. Its terminal
// +30 encoded word is tested by sign, before the command's sentinel tick.
bool can_predict_command_copy(const Command&, const void* variable) noexcept;

// Exact 433F27C3/4341873D optional 434683D0 byte scope. Create it immediately
// before prepare_prediction_command_copy and retain it through native 4DE5;
// CommandPrediction ends earlier, at 46F4..4709.
class CommandCopyOverrideScope {
public:
    explicit CommandCopyOverrideScope(std::uint8_t* target) noexcept;
    ~CommandCopyOverrideScope() {
        restore();
    }
    CommandCopyOverrideScope(const CommandCopyOverrideScope&) = delete;
    CommandCopyOverrideScope& operator=(const CommandCopyOverrideScope&) = delete;
    void restore() noexcept;

private:
    std::uint8_t* target;
    std::uint8_t saved;
};

// Inline 433D43A2..433D4404, after a successful gate and override entry.
// Copies all 0x64 command bytes, reorients from frame+118 with the captured
// movement type, and sanitizes using frame.flags bit 3. Native always supplies
// true for the grounded argument here; force buttons is send_packet==0.
void prepare_prediction_command_copy(const CommandFrame&, const Command&, Command& copy,
                                     std::uint8_t send_packet, const CommandButtonPolicy&,
                                     const CommandTrigonometry&, const ScriptCommandEnvironment&);
} // namespace skeet::client

// Declarations from command_capture.hpp

namespace skeet::client {
struct CommandCaptureOffsets {
    CommandMouseOffsets mouse;
    std::ptrdiff_t rules_guard; // D000A932
    std::ptrdiff_t tick_base; // 3D10B94C
    std::ptrdiff_t vector_48442721;
    std::ptrdiff_t weapon_field_d2563274;
    std::ptrdiff_t team; // 8B5586F6
};
struct CommandCaptureConvars {
    engine::NativeConVarReference target_selector; //43468EA4^FCD2DA7A,+30
    engine::NativeConVarReference optional_angle_override; //43468254^CF54AEB3,+30;nullable
    engine::NativeConVarReference angle_mode; //43467A8C^AC9299CC,+30
    engine::NativeConVarReference angle_secondary; //4346A42C^672DFE69,+30
    engine::NativeConVarReference command_tick_offset; //43468528^F75367FE,+30
    engine::NativeConVarReference interpolation_rate; //4346747C^03B4819C,+2C
    engine::NativeConVarReference interpolation_minimum; //4346A748^7D9DC86F,+2C
    engine::NativeConVarReference interpolation_ratio; //4346A330^BE64D223,+2C
};
struct CommandTargetValidation {
    anti_aim::TargetState& target; // shared 4346DB14..4346DB38
    PlayerStateCache& players; // concrete 433B8A2B cache
    void* const& entity_list; // live4346A890
    const std::int32_t* sequence; // *434678F8
    void* const* clock_owner; // 4346A3A4; nullable
    std::ptrdiff_t clock_offset; // 9F8E7F71
    std::ptrdiff_t rejection_byte; // 9D920E35
    std::ptrdiff_t network_origin; // A9EE6062
    void* const* dormant_owner; // 43468FF0; nullable
    std::ptrdiff_t dormant_present; // 9571D5F5, indexed by handle low word
    std::ptrdiff_t dormant_valid; // D266B537, indexed by handle low word
};
// Complete 433ED5A8. The valid player-handle contract excludes its deliberate
// invalid-instruction branch for a zero low-word entity index.
bool command_target_valid(const CommandTargetValidation&);

struct CommandCaptureContext {
    const CommandRecaptureContext& physical;
    ScriptCommandEnvironment angle_environment;
    CommandCaptureOffsets offsets;
    CommandCaptureConvars convars;
    void* engine_client; // 4346A514; GetViewAngles slot18
    const HotkeyBinding& movement_key; //live RAGE duck_peek_assist, D23ED2A7
    InputBitmap input;
    const std::uint8_t* angle_override; // 434670C0
    const std::int32_t* maximum_angle; // 4346709C
    const float* interpolation_floor; // 43469254
    const network::LatencySamples& latency;
    const SentCommandTiming& timing;
    const CommandTargetValidation& target_validation;
};
struct CommandCapturedState {
    CommandBoneBackup bones;
    float planar_speed; // master stack +29C
    float speed; // master stack +2A0
};
// Complete initial capture 433D355E..433D415B. saved_angles is the master's
// pre-phase2 +318 snapshot. bone_storage is the retained-capacity native
// 43478CC8 vector; its storage must survive prediction and later restoration.
// global_flags refers to 434759C0, separately from frame.flags.
CommandCapturedState capture_initial_command_frame(CommandFrame&, Command&,
                                                   std::uint8_t& send_packet,
                                                   std::uint32_t& global_flags, float sample_time,
                                                   const std::array<float, 3>& saved_angles,
                                                   std::vector<Matrix3x4>& bone_storage,
                                                   const CommandCaptureContext&);
} // namespace skeet::client

// Declarations from command_capture_runtime_owner.hpp

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
struct EngineRuntimeBindings;

// Concrete canonical field production for the existing CreateMove capture
// bodies. Construct after engine/offset publication and the shared player and
// history owners. This owns no duplicate EntityStateRuntime or player cache.
// It performs no registration, engine callback, prediction or command phase.
// All borrowed owners and their engine module objects must outlive this owner.
class CommandCaptureRuntimeOwner final {
public:
    CommandCaptureRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                               ConfigurationRuntimeOwner&, PlayerRuntimeOwner&,
                               animation::RuntimeOwner&, const lagcomp::EntityStateRuntime&);
    ~CommandCaptureRuntimeOwner();
    CommandCaptureRuntimeOwner(const CommandCaptureRuntimeOwner&) = delete;
    CommandCaptureRuntimeOwner& operator=(const CommandCaptureRuntimeOwner&) = delete;
    CommandCaptureRuntimeOwner(CommandCaptureRuntimeOwner&&) = delete;
    CommandCaptureRuntimeOwner& operator=(CommandCaptureRuntimeOwner&&) = delete;

    ClientRuntimeStorage& storage() const noexcept;
    const CommandMasterGlobals& globals() const noexcept;
    const CommandPlayerGates& player_gates() const noexcept;
    const CommandMouseOffsets& mouse_offsets() const noexcept;
    const CommandRecaptureOffsets& physical_offsets() const noexcept;
    const CommandCaptureOffsets& capture_offsets() const noexcept;
    const weapon::FireStateBindings& firing() const noexcept;
    const CommandEntityBindings& entities() const noexcept;
    const ScriptCommandEnvironment& angle_environment() const noexcept;
    const CommandAngleBindings& view_angles() const noexcept;
    const CommandPacketCatchupBindings& packet_catchup() const noexcept;
    const CommandBudgetBindings& budget() const noexcept;
    const CommandTargetValidation& target_validation() const noexcept;
    const CommandSentTimingBindings& sent_timing_bindings() const noexcept;
    const CommandEyeAdjustmentSourceContext& eye_adjustment() const noexcept;
    engine::NativeConVarReference copy_gate() const noexcept;
    std::vector<Matrix3x4>& bone_storage() noexcept;
    SentCommandHistoryView& sent_history() noexcept;

    // Call at the master's existing native capture position, after its fresh
    // SentCommandTiming local has been formed. The returned views borrow the
    // actual player, physical context and timing; none is retained as a fake
    // initial frame. The unchanged capture/recapture bodies consume them.
    CommandRecaptureContext recapture(void* current_player) const;
    CommandCaptureContext capture(const CommandRecaptureContext&, const SentCommandTiming&) const;
    CommandCaptureContext capture(CommandRecaptureContext&&,
                                  const SentCommandTiming&) const = delete;
    CommandCaptureContext capture(const CommandRecaptureContext&&,
                                  const SentCommandTiming&) const = delete;
    CommandCaptureContext capture(const CommandRecaptureContext&,
                                  SentCommandTiming&&) const = delete;
    CommandCaptureContext capture(const CommandRecaptureContext&,
                                  const SentCommandTiming&&) const = delete;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from command_checksum.hpp

namespace skeet::client {
// Complete client307260 CUserCmd checksum. The14 field ranges are part of
// this source Command ABI; padding, vtable and trailing prediction state are
// deliberately outside the native checksum transaction.
std::uint32_t source_command_checksum(const Command&) noexcept;
CommandChecksum source_command_checksum_entry() noexcept;
// Publish the actual compiled source function, never a captured module address.
// A conflicting preexisting publication is rejected before any change.
void initialize_source_command_checksum(engine::NativeDataImage&);
} //namespace skeet::client

// Declarations from command_empty_tick.hpp

namespace skeet::misc {
class GrenadeCommandRuntime;
}
namespace skeet::effects {
struct EffectsCommandOwner;
}

namespace skeet::client {
//Separate native registry43475978/7C. These five interfaces are not command
//phase owners or prediction observers. Runtime construction order is supplied.
enum class CommandEmptyTickId : std::uint32_t {
    prediction_history = 0x43470470u, //PredictionHistoryOwner*
    legit = 0x434704B4u, //LegitCommandOwner*
    timer = 0x4346F3B0u, //CommandEmptyTickTimer*
    grenade = 0x434704BCu, //misc::GrenadeCommandRuntime*
    camera = 0x4347270Cu, //effects::EffectsCommandOwner*
};
struct PredictionHistoryOwner;
struct LegitCommandOwner;
//43398A4D's receiver is fullowner+20 and its only field is receiver+64.
struct CommandEmptyTickTimer {
    float& remaining;
    NativeCommandEmptyTickReceiver empty_tick_receiver{};
};

// Borrows actual43475978/7C/80. The buffer contains physical receiver pointers;
// construction neither initializes its cells nor allocates a parallel vector.
class CommandEmptyTickRegistry {
public:
    // input is the same address-stable CommandOwnerDispatch.input view of the
    // genuine key words and must outlive this registry and its receivers.
    CommandEmptyTickRegistry(engine::PreservedInitialization&, engine::JobMemory,
                             const InputBitmap&);
    CommandEmptyTickRegistry(std::shared_ptr<engine::NativeDataImage>, engine::JobMemory,
                             const InputBitmap&);
    CommandEmptyTickRegistry(const CommandEmptyTickRegistry&) = delete;
    CommandEmptyTickRegistry& operator=(const CommandEmptyTickRegistry&) = delete;
    CommandEmptyTickRegistry(CommandEmptyTickRegistry&&) = delete;
    CommandEmptyTickRegistry& operator=(CommandEmptyTickRegistry&&) = delete;
    void append(PredictionHistoryOwner&);
    void append(LegitCommandOwner&);
    void append(CommandEmptyTickTimer&);
    // Grenade registers before constructing its runtime. Its physical endpoint
    // belongs to the already constructed whole owner, not that runtime buffer.
    void append_grenade(NativeCommandEmptyTickReceiver&, misc::GrenadeCommandRuntime*);
    void append(effects::EffectsCommandOwner&);
    // Final table writes are separate from their earlier registrar calls.
    void publish(PredictionHistoryOwner&) const noexcept; //4347D086
    void publish(LegitCommandOwner&) const noexcept; //43484601
    void publish(CommandEmptyTickTimer&) const noexcept; //43485E1F
    void publish_grenade(NativeCommandEmptyTickReceiver&) const noexcept; //434867CD
    void publish(effects::EffectsCommandOwner&) const noexcept; //4348E766
    void require_sources() const;
    void dispatch(void* player, float sample_time, Command&, std::uint8_t& original_result) const;
    engine::OwnerRegistry& native_storage() const noexcept {
        return owners_;
    }
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }
    // Exact43349ECB, registered by433B22B6. Sole terminal native-buffer free;
    // the C++ destructor does not release it and all three cells stay intact.
    void finalize_storage() const;

private:
    std::shared_ptr<engine::NativeDataImage> image_;
    engine::OwnerRegistry& owners_;
    engine::JobMemory memory_;
    const InputBitmap& input_; // same live keyboard bitmap used by command dispatch
    void append_receiver(NativeCommandEmptyTickReceiver&, void* source);
};
void require_empty_tick_sources(const CommandEmptyTickRegistry&);
void dispatch_empty_tick_command(const CommandEmptyTickRegistry&, void* player, float sample_time,
                                 Command&, std::uint8_t& original_result);
} //namespace skeet::client

// Declarations from command_empty_tick_receiver.hpp

namespace skeet {
struct InputBitmap;
}
namespace skeet::client {
struct Command;
struct NativeCommandEmptyTickReceiver;
#if defined(_M_IX86) || defined(__i386__)
using NativeCommandEmptyTickEntry = void(__vectorcall*)(NativeCommandEmptyTickReceiver*,
                                                        void* player, float sample_time, Command*,
                                                        std::uint8_t* original_result);
#else
using NativeCommandEmptyTickEntry = void (*)(NativeCommandEmptyTickReceiver*, void* player,
                                             float sample_time, Command*,
                                             std::uint8_t* original_result);
#endif
//433D2F6C: ECX=receiver, EDX=player, XMM0=sample time, stack Command* then
//original-result byte*. The single entry returns8; result is not an EAX value.
struct NativeCommandEmptyTickVtable {
    NativeCommandEmptyTickEntry invoke;
};
struct NativeCommandEmptyTickReceiver {
    const NativeCommandEmptyTickVtable* vtable = nullptr;
    void* source = nullptr;
    const InputBitmap* input = nullptr; // genuine global-key boundary for grenade
};
static_assert(offsetof(NativeCommandEmptyTickReceiver, vtable) == 0);
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(NativeCommandEmptyTickVtable) == 4);
static_assert(offsetof(NativeCommandEmptyTickReceiver, source) == 4);
static_assert(sizeof(NativeCommandEmptyTickReceiver) == 0xC);
#endif
} // namespace skeet::client

// Declarations from command_eye_adjustment.hpp

namespace skeet::animation {
struct BoneRuntime;
}
namespace skeet::engine {
class EyePoseParameterRuntime;
}
namespace skeet::client {
struct CommandEyeAdjustmentOffsets {
    lagcomp::EntityNetvars common;
    std::ptrdiff_t view_offset; // C246315E, vector
    std::ptrdiff_t overridden_property; // 2B554AD0, uint32
    std::ptrdiff_t animation_guard; // E2FBE695, byte
    std::ptrdiff_t weapon_identifier; // 5C9F0FBB, uint16
    std::ptrdiff_t studio_header; // 32E4DAF1, studio wrapper pointer
    std::ptrdiff_t hitbox_set; // C2B6FB10, int32
    std::ptrdiff_t player_pitch_adjustment; // 5AE32976, float
    std::ptrdiff_t bone_origin; // A9EE6062, vector
};
struct CommandEyeAdjustmentContext {
    CommandEyeAdjustmentOffsets offsets;
    const std::uint8_t* enabled; // 43467224
    const std::uint8_t* alternate_enabled; // 4346A740
    const std::uint32_t* pitch_enabled; // 43467F60
    const std::int32_t* selected_command; // 434759CC
    const std::array<float, 3>* selected_point; // 434759D0
    const std::uint32_t* replacement_property; // optional 43469060
    std::span<const std::uint32_t, 3> weapon_classification; // 43474580
    animation::BoneEntityState& bone_state;
    const animation::BoneModel& bone_model;
    const animation::BoneEngine& bone_engine;
    const RageTrigonometry& trigonometry;
    // Genuine CRT __libm_sse2_atan2 boundary 77509EE0.
    double (*atan2)(double y, double x);
    // Genuine resolved engine entry 43467CF0, as in FiringPostAngleBindings.
    // 4331F306 supplies studio wrapper, index 12, angle, normalized output.
    float (*convert_pose_parameter)(const void* studio_wrapper, std::uint32_t index, float angle,
                                    float& normalized);
};
// Production uses the actual live-player bone builder and the verified engine
// pose ABI, without manufacturing semantic BoneEntityState/BoneModel copies.
// The ordinary geometry body and its read/build/write order are shared with
// the explicit standalone context above.
struct CommandEyeAdjustmentSourceContext {
    CommandEyeAdjustmentOffsets offsets;
    const std::uint8_t* enabled;
    const std::uint8_t* alternate_enabled;
    const std::uint32_t* pitch_enabled;
    const std::int32_t* selected_command;
    const std::array<float, 3>* selected_point;
    const std::uint32_t* const& replacement_property;
    std::span<const std::uint32_t, 3> weapon_classification;
    const animation::BoneRuntime& bones;
    const RageTrigonometry& trigonometry;
    double (*atan2)(double, double);
    const engine::EyePoseParameterRuntime& pose;
};

// Complete 433B5BC4, including its 433F8EEA bone wrapper. Uses the player's
// actual live matrix cache and restores pose parameter 12 on every exit after
// capture. Command is the unused native stack argument, retained in this API.
void adjust_command_eye(CommandFrame&, void* player, Command&, const CommandEyeAdjustmentContext&);
void adjust_command_eye(CommandFrame&, void* player, Command&,
                        const CommandEyeAdjustmentSourceContext&);
} // namespace skeet::client

// Declarations from command_frame.hpp

namespace skeet::client {
struct CommandBullet {
    std::uint16_t weapon_id, padding;
    float penetration, range, range_modifier, armor_ratio, damage;
    std::uint32_t pellet_count;
};
static_assert(sizeof(CommandBullet) == 0x1C);
// Shared 0x1F4 frame passed to the CreateMove phase interfaces. Pointer fields
// retain the original x86 address width, as in Command's vtable word.
struct CommandFrame {
    std::uint32_t flags, secondary_flags;
    std::uint32_t send_packet_address, weapon_address, weapon_data_address;
    Command previous_command;
    float input_forward_78;
    std::uint32_t player_flags_7c;
    float input_side_80, view_height_84;
    std::array<std::byte, 0x10> fields_88;
    float duck_amount_98;
    std::array<float, 3> recoil_angles_9c;
    float player_field_a8;
    std::int32_t command_number_ac;
    std::uint32_t field_b0, field_b4;
    float angle_b8, field_bc, scalar_c0;
    union {
        std::uint32_t field_c4;
        float inaccuracy_c4;
    };
    std::int32_t tick_base_c8;
    std::uint32_t field_cc;
    float spread_d0;
    std::uint32_t field_d4;
    std::int32_t weapon_id_d8;
    std::array<float, 3> origin_dc;
    std::array<std::byte, 0x10> fields_e8;
    std::uint32_t player_flags_f8;
    std::int32_t choke_limit_fc;
    union {
        std::uint32_t field_100;
        float weapon_field_100;
    };
    std::uint8_t state_104;
    std::array<std::byte, 3> padding_105;
    std::uint32_t field_108;
    std::array<float, 3> origin_10c;
    std::array<float, 3> angles_118;
    std::uint8_t weapon_kind_124;
    std::array<std::byte, 3> padding_125;
    CommandBullet bullet_128;
    std::array<std::byte, 0xC> fields_144;
    std::array<float, 3> velocity_150;
    std::int32_t choked_15c;
    union {
        std::uint32_t field_160;
        float adjusted_spread_160;
    };
    float height_164;
    std::array<float, 3> origin_168;
    std::array<float, 3> forward_174;
    std::array<float, 3> previous_velocity_180;
    std::uint32_t field_18c;
    float previous_speed_190;
    std::array<std::byte, 8> fields_194;
    std::int32_t pellet_count_19c;
    std::uint32_t field_1a0;
    std::uint32_t field_1a4;
    std::array<float, 3> direction_1a8;
    std::uint32_t field_1b4;
    std::array<float, 3> hull_minimum_1b8, hull_maximum_1c4;
    std::uint32_t field_1d0;
    std::uint32_t field_1d4, field_1d8, field_1dc;
    std::uint32_t field_1e0;
    union {
        std::uint32_t field_1e4;
        float postpone_fire_1e4;
    };
    float maximum_angle_step_1e8;
    std::uint32_t field_1ec, field_1f0;
};
static_assert(sizeof(CommandFrame) == 0x1F4);
static_assert(offsetof(CommandFrame, previous_command) == 0x14);
static_assert(offsetof(CommandFrame, command_number_ac) == 0xAC);
static_assert(offsetof(CommandFrame, player_flags_7c) == 0x7C);
static_assert(offsetof(CommandFrame, recoil_angles_9c) == 0x9C);
static_assert(offsetof(CommandFrame, weapon_id_d8) == 0xD8);
static_assert(offsetof(CommandFrame, tick_base_c8) == 0xC8);
static_assert(offsetof(CommandFrame, choke_limit_fc) == 0xFC);
static_assert(offsetof(CommandFrame, player_flags_f8) == 0xF8);
static_assert(offsetof(CommandFrame, velocity_150) == 0x150);
static_assert(offsetof(CommandFrame, bullet_128) == 0x128);
static_assert(offsetof(CommandFrame, angles_118) == 0x118);
static_assert(offsetof(CommandFrame, hull_minimum_1b8) == 0x1B8);
static_assert(offsetof(CommandFrame, direction_1a8) == 0x1A8);
static_assert(offsetof(CommandFrame, maximum_angle_step_1e8) == 0x1E8);
static_assert(offsetof(CommandFrame, choked_15c) == 0x15C);
static_assert(offsetof(CommandFrame, previous_speed_190) == 0x190);

// Exact sparse constructor 43366497. All other frame bytes are untouched.
void initialize_command_frame(CommandFrame&) noexcept;
// Exact inlined frame prefix 433D34BB..433D3522, before query-limit phase 9.
void prepare_command_frame(CommandFrame&, const Command& saved_command,
                           std::uint32_t send_packet_address, std::int32_t choked,
                           std::int32_t command_number) noexcept;
} // namespace skeet::client

// Declarations from command_orchestrator.hpp

namespace skeet::effects {
class FreeLookOwner;
}
namespace skeet::client {
struct PredictionHistoryOwner;
class CommandCaptureRuntimeOwner;
class TargetRefreshRuntimeOwner;
struct CommandPlayerGates {
    std::array<std::ptrdiff_t, 2> rejection_bytes; //9D920E35,CDE290D2
    std::ptrdiff_t restricted_state; //2F4E97C4, int32
    std::ptrdiff_t movement_type; //7501CB91, byte
    std::ptrdiff_t player_flags; //E2FBE695, uint32
    std::ptrdiff_t tick_base; //3D10B94C, int32
    std::ptrdiff_t active_weapon; //CD812654, handle
};
bool command_player_rejected(const void*, const CommandPlayerGates&) noexcept;
bool command_player_restricted(const void*, const CommandPlayerGates&) noexcept;

struct CommandPacketCatchupBindings {
    void* const& engine_client; //4346A514
    void* const& network_state; //43468E44; double clock at+108
    const float& interval; //*4346A338
};
//433D300D..433D3075. Only the channel clock is an engine boundary.
void catch_up_command_packet(std::uint8_t&, const std::uint8_t& packet_context,
                             const CommandPacketCatchupBindings&);

struct CommandBudgetBindings {
    void** const& game_rules; //4346A3A4
    std::ptrdiff_t rule_bf567619, rule_dda99ab5, rule_b7ea8fda;
    const std::int32_t& configured; //43469274
};
//Complete433A13ED as read by the master before ordered phase9 reductions.
std::int32_t query_command_budget(const CommandBudgetBindings&) noexcept;

//433D2F15..2F63/433D308D..30A1: forward the original CreateMove result's
//angles directly. The empty-tick path additionally updates prediction slot13.
void forward_original_command_angles(void* const& engine_client, void* const& prediction, Command&,
                                     bool empty_tick);

//433D524E..433D5368. Phase6 follows this concrete movement/angle/shot block.
void finalize_command_movement(CommandFrame&, Command&, std::uint8_t& send_packet,
                               const CommandButtonPolicy&, const CommandTrigonometry&,
                               const ScriptCommandEnvironment&, std::int32_t last_outgoing,
                               std::int32_t& last_shot);

//433D5868..433D59DB, after all active time scopes have been restored.
void finish_command_transaction(Command&, std::uint8_t& engine_send_packet,
                                std::uint8_t requested_packet,
                                const CommandPacketTransitionBindings&,
                                std::uint32_t& scheduler_flags, const CommandEntityBindings&,
                                const CommandEntitySnapshot&);

struct CommandMasterGlobals {
    void* const& engine_client;
    void* const& entity_list;
    void* const& prediction;
    void* const& engine_globals;
    float* const& interval;
    float* const& current_time;
    std::int32_t* const& choked;
    std::int32_t* const& last_outgoing;
    std::uint32_t& scheduler_flags;
    std::int32_t& last_shot;
};
struct CommandSentTimingBindings {
    void* const& command_clock; //43475A60, base tick+4
    std::int32_t* const& acknowledged; //43468F9C
    std::int32_t* const& sent; //434670A8
};
struct CommandAcceptedBindings {
    CommandMasterGlobals globals;
    const CommandPlayerGates& player_gates;
    const CommandEntityBindings& entities;
    const CommandPacketCatchupBindings& catchup;
    effects::FreeLookOwner* const& free_look; //same live43475A64 owner used by view capture
    const HotkeyBinding& free_look_key;
    const CommandAngleBindings& view_angles;
    const CommandBudgetBindings& budget;
    //Each phase forms a fresh span, as the native owner rereads begin/end.
    const CommandPhaseRegistry& order;
    const CommandCaptureRuntimeOwner& capture;
    const CommandEyeAdjustmentSourceContext& eye;
    std::vector<Matrix3x4>& bone_storage;
    const CommandMouseOffsets& mouse;
    void* const& input; //live4346A894
    engine::NativeConVarReference copy_gate_convar; //43468E94^2DC3CC20
    std::uint8_t* const& transform_override;
    std::span<const std::uint32_t, 150> inverted_commands;
    const anti_aim::OtherGlobalSettings& anti_aim_other;
    const CommandTrigonometry& trigonometry;
    const ScriptCommandEnvironment& environment;
    const prediction::CommandBindings& prediction_bindings;
    const prediction::CommandPlayerOffsets& prediction_player_offsets;
    prediction::SeedGenerator seed_generator;
    const CommandPacketAngleBindings& packet_angles;
    SentCommandHistoryView sent_history;
    CommandSentTimingBindings sent_timing;
    const CommandWeaponPostprocessBindings& postprocess;
    const CommandPacketTransitionBindings& packet_transition;
    PredictionHistoryOwner* const& restricted_owner; //live434759B4, shared view resolved at call
    const RestrictedPlayerCommandBindings& restricted;
    const TargetRefreshRuntimeOwner& sentinel_refresh;
};
//433D2FFA..433D59DB, after phase0 and the two active-player rejection gates.
//All phases1..9 use the supplied actual registration order. Each custom owner
//must have a concrete source binding; unsupported owners are rejected up front.
//The caller retains frame and bone capacity; send_packet addresses the outer
//saved EBX word, including its adjacent context byte. No payload callback runs.
std::uint8_t run_accepted_command_pipeline(const CommandAcceptedBindings&, CommandFrame&,
                                           void* player, float sample_time, Command&,
                                           std::uint8_t& send_packet,
                                           std::uint8_t original_create_move_result);
struct RejectedPlayerUpdateContext;
struct CommandCreateMoveBindings {
    const CommandEntryBindings& entry;
    const CommandAcceptedBindings& accepted;
    const CommandEmptyTickRegistry& empty_tick_order;
    const RejectedPlayerUpdateContext& rejected_update;
};
//Complete native433D2E40 framing and branches, followed by the concrete
//accepted pipeline. Both independently registered owner orders are explicit.
std::uint8_t process_create_move_command(const CommandCreateMoveBindings&, CommandFrame&,
                                         void* player, float sample_time, Command&,
                                         std::uint8_t& send_packet);
struct CommandCreateMoveRuntime {
    const CommandCreateMoveBindings& bindings;
    std::uint8_t process(void* player, std::uint8_t* send_packet, float sample_time, Command*);
};
void bind_create_move_runtime(CommandCreateMoveRuntime&) noexcept;
void unbind_create_move_runtime(CommandCreateMoveRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
std::uint8_t __fastcall create_move_command_hook(void* player, std::uint8_t* send_packet,
                                                 float sample_time, Command*);
//Actual player virtual289 replacement4334F53D. EDX at engine entry is not
//the packet pointer; this wrapper passes the saved EBX word to the inner master
//and pops the modified word back into EBX before returning RET8.
std::uint8_t __fastcall player_create_move_hook(void* player, void* register_edx, float sample_time,
                                                Command*);
#else
std::uint8_t create_move_command_hook(void* player, std::uint8_t* send_packet, float sample_time,
                                      Command*);
#endif
} //namespace skeet::client

// Declarations from command_owner_dispatch.hpp

namespace skeet::client {
struct LegitCommandOwner {
private:
    struct Standalone {
        legit::RuntimeBindings& bindings;
        void* engine;
    };
    std::variant<Standalone, legit::CommandSource*> source_;

public:
    legit::RuntimeState& state;
    const legit::RuntimeSettings& settings;
    NativeCommandEmptyTickReceiver empty_tick_receiver{};
    LegitCommandOwner(legit::RuntimeState& s, const legit::RuntimeSettings& configuration,
                      legit::RuntimeBindings& bindings, void* engine) noexcept
        : source_(Standalone{bindings, engine}), state(s), settings(configuration) {}
    LegitCommandOwner(legit::RuntimeState& s, const legit::RuntimeSettings& configuration,
                      legit::CommandSource& source) noexcept
        : source_(&source), state(s), settings(configuration) {}
    void capture_view();
    void clear_allocated_storage();
    void process(CommandFrame&, void* player, Command&);
    void reset();
    void pending(Command&, float);
};
struct PenetrationReticleCommandOwner {
    hud::PenetrationReticleState& state;
    const esp::ScreenOverlayConfig& settings;
    const hud::PenetrationReticleEnvironment& bindings;
};
struct ScreenOverlayCommandOwner {
    hud::ScreenOverlayState& state;
    hud::ScreenOverlayPublisher& publisher;
    const esp::ScreenOverlayConfig& settings;
    const hud::ScreenOverlayFields& offsets;
    const hud::ScreenOverlayUpdateBindings& bindings;
};
struct SharedEspCommandOwner {
    const RejectedPlayerUpdateContext& bindings;
};
//A missing custom source body is rejected before a command mutates engine
//state. Return-only native slots are recognized from the exact phase manifest.
bool command_phase_source_bound(CommandOwnerId, std::uint32_t phase);
const NativeCommandPhaseVtable& source_command_phase_table(CommandOwnerId);
bool is_source_command_phase_table(const NativeCommandPhaseVtable*) noexcept;
} //namespace skeet::client

// Declarations from command_packet_angle_guard.hpp

namespace skeet::client {
struct CommandPacketAngleBindings {
    const std::uint8_t* enabled; // 43467224
    const std::uint8_t* firing_mode_enabled; // 434684E0
    const std::uint8_t* send_mode; // 43468D7C
    std::ptrdiff_t weapon_identifier; // decoded 5C9F0FBB
    std::ptrdiff_t player_flags; // decoded 38CA5228
    std::ptrdiff_t player_first_byte; // decoded 043FA575
    std::ptrdiff_t player_second_byte; // decoded 334B460C
    std::span<const std::uint32_t, 3> weapon_classification; // 43474580
    void* const& input; // live4346A894, CInput GetUserCmd slot8
};

// Inline 433D4E53..433D4F40. Chokes a changed shot angle for classified
// weapons, then enforces the signed frame choke limit. Phase 5 follows.
void guard_changed_shot_packet(CommandFrame&, const Command&, std::uint8_t& send_packet,
                               const void* weapon, const CommandPacketAngleBindings&) noexcept;

// Inline 433D4F71..433D524E, after phase 5. Enforces the choke limit again,
// applies the choked-shot pitch offset, then limits the pitch/yaw angle step.
// Its maximum step is the native float at frame +1E8. The reference is the
// saved command or actual CInput GetUserCmd(0,number-1), in native gate order.
void guard_packet_angles(CommandFrame&, Command&, std::uint8_t& send_packet, void* player,
                         const void* weapon, const CommandPacketAngleBindings&);
} // namespace skeet::client

// Declarations from command_packet_transition.hpp

namespace skeet::client {

// References retain the shared globals rather than capturing their values
// before the engine queries in this cut. The per-call packet_context argument
// is send_packet+1 in the outer wrapper's saved EBX word.
struct CommandPacketTransitionBindings {
    const std::uint8_t& enabled; // 43467224
    const std::uint8_t& transition_enabled; // 4346754C
    void* const& engine_client; // 4346A514
    void** const& shared_player; // 4346A3A4, nullable holder
    std::uint32_t* const& player_guard_word; // 43468378
    const std::uint32_t& player_guard_key; // 43468244, XOR BF567619
    std::uint32_t* const& last_outgoing; // 43475B2C
    std::span<StartupAcknowledgement, 150> acknowledgements; // 43475BE0
};

// Inline433D5888..433D594F compares the mutable ESP+1A decision with the
// saved engine EBX byte. A requested choke while the engine wants to send,
// under all native gates, records the sequence and calls SendDatagram(nullptr).
// The decoded channel virtual byte offset is B8 (slot 46). 433E533F changes
// its return path only; the real thiscall receiver and argument are retained.
// The live channel +2C/+18 words are decremented after that call, then the
// engine byte receives the requested decision. Flag/entity restoration follows.
void apply_command_packet_transition(std::uint8_t requested_packet,
                                     std::uint8_t& engine_send_packet,
                                     const std::uint8_t& packet_context,
                                     const CommandPacketTransitionBindings&);

} // namespace skeet::client

// Declarations from command_phase_receiver.hpp

namespace skeet::client {
struct Command;
struct CommandFrame;
struct NativeCommandPhaseReceiver;
struct CommandOwnerDispatch {
    std::uint32_t& scheduler_flags;
    InputBitmap input;
};
#if defined(_M_IX86) || defined(__i386__)
using CommandPhase0Entry = void(__fastcall*)(NativeCommandPhaseReceiver*, void*, Command*);
using CommandPhase1Entry = void(__fastcall*)(NativeCommandPhaseReceiver*, void*, void*, Command*,
                                             std::uint8_t*);
using CommandPhase2Entry = void(__fastcall*)(NativeCommandPhaseReceiver*, void*, void*, Command*,
                                             std::array<float, 3>*, std::uint8_t*);
using CommandFramePhaseEntry = void(__fastcall*)(NativeCommandPhaseReceiver*, CommandFrame*, void*,
                                                 Command*);
using CommandLimitEntry = std::int32_t(__fastcall*)(NativeCommandPhaseReceiver*, std::int32_t);
#else
using CommandPhase0Entry = void (*)(NativeCommandPhaseReceiver*, void*, Command*);
using CommandPhase1Entry = void (*)(NativeCommandPhaseReceiver*, void*, void*, Command*,
                                    std::uint8_t*);
using CommandPhase2Entry = void (*)(NativeCommandPhaseReceiver*, void*, void*, Command*,
                                    std::array<float, 3>*, std::uint8_t*);
using CommandFramePhaseEntry = void (*)(NativeCommandPhaseReceiver*, CommandFrame*, void*,
                                        Command*);
using CommandLimitEntry = std::int32_t (*)(NativeCommandPhaseReceiver*, std::int32_t);
#endif
//433D2E40 supplies ECX=secondaryreceiver throughout. Slots0/1/2 take the
//player in EDX and consume1/3/4 stack words. Slots3..8 take the frame in EDX
//and player,command on the stack. Slot9 chains EDX through the returned EAX.
struct NativeCommandPhaseVtable {
    CommandPhase0Entry phase0;
    CommandPhase1Entry phase1;
    CommandPhase2Entry phase2;
    CommandFramePhaseEntry phase3, phase4, phase5, phase6, phase7, phase8;
    CommandLimitEntry query_limit;
};
// This endpoint lives in its actual whole source owner. The native pointer
//buffer contains its address alone; the backreference borrows the existing
//command state/view and services borrow the one canonical dispatch binding.
struct NativeCommandPhaseReceiver {
    const NativeCommandPhaseVtable* vtable = nullptr;
    void* source = nullptr;
    const CommandOwnerDispatch* services = nullptr;
};
static_assert(offsetof(NativeCommandPhaseReceiver, vtable) == 0);
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(NativeCommandPhaseVtable) == 0x28);
static_assert(offsetof(NativeCommandPhaseVtable, phase3) == 0xC);
static_assert(offsetof(NativeCommandPhaseVtable, query_limit) == 0x24);
static_assert(sizeof(NativeCommandPhaseReceiver) == 0xC);
#endif
} //namespace skeet::client

// Declarations from command_phase_registry.hpp

namespace skeet::client {
//Exact secondary vtables written after the28 proven43345EBC registrations.
//This enumeration supplies identities, never an assumed construction order.
enum class CommandOwnerId : std::uint32_t {
    owner_4346ff8c = 0x4346FF8Cu,
    automatic_weapons = 0x4346F244u,
    quick_switch = 0x4346FC24u,
    exploit = 0x4346EDD4u,
    owner_43472148 = 0x43472148u,
    owner_4346eed8 = 0x4346EED8u,
    owner_4346dc2c = 0x4346DC2Cu,
    owner_43470200 = 0x43470200u,
    owner_4346f01c = 0x4346F01Cu,
    owner_43470834 = 0x43470834u,
    blockbot = 0x43471E50u,
    knife = 0x43471EB8u,
    rage = 0x43472A00u,
    owner_43470478 = 0x43470478u,
    rebuy = 0x43471F0Cu,
    penetration_reticle = 0x4346F798u,
    legit = 0x4346F5B0u,
    zeus = 0x434701A0u,
    owner_4346fe30 = 0x4346FE30u,
    owner_4346f0ac = 0x4346F0ACu,
    owner_4346f048 = 0x4346F048u,
    screen_overlay = 0x4346DCC0u,
    owner_43472998 = 0x43472998u,
    owner_434700e4 = 0x434700E4u,
    movement = 0x43470770u,
    fake_lag = 0x43470390u,
    owner_4346f1f4 = 0x4346F1F4u,
    anti_aim = 0x4346F2E0u,
};
struct CommandPhaseTable {
    CommandOwnerId id;
    std::array<std::uint32_t, 10> entries;
};
std::span<const CommandPhaseTable> native_command_phase_tables() noexcept;
std::optional<CommandOwnerId> identify_command_phase_owner(std::uint32_t vtable) noexcept;
const std::array<std::uint32_t, 10>& command_phase_entries(CommandOwnerId);
bool command_phase_is_noop(std::uint32_t entry) noexcept;

//43345EBC appends physical source secondaryreceivers into the existing
//434757A0/A4/A8 triplet. Actual constructor calls establish the order; no
//semantic registration vector or duplicate pointer buffer is constructed.
class CommandPhaseRegistry {
public:
    CommandPhaseRegistry(engine::PreservedInitialization&, engine::JobMemory, CommandOwnerDispatch);
    CommandPhaseRegistry(std::shared_ptr<engine::NativeDataImage>, engine::JobMemory,
                         CommandOwnerDispatch);
    CommandPhaseRegistry(const CommandPhaseRegistry&) = delete;
    CommandPhaseRegistry& operator=(const CommandPhaseRegistry&) = delete;
    CommandPhaseRegistry(CommandPhaseRegistry&&) = delete;
    CommandPhaseRegistry& operator=(CommandPhaseRegistry&&) = delete;
    void append(NativeCommandPhaseReceiver&, void* canonical_source);
    // Publication is separate from registration and remains at the actual
    //constructor's final secondarytable write recorded in the28row manifest.
    void publish(NativeCommandPhaseReceiver&, CommandOwnerId) const;
    void require_sources() const;
    void phase0(void* player, Command&) const;
    void phase1(void* player, void* weapon, Command&, std::uint8_t& send_packet) const;
    void phase2(void* player, void* weapon, Command&, std::array<float, 3>& incoming_angles,
                std::uint8_t& send_packet) const;
    void frame_phase(std::uint32_t phase, CommandFrame&, void* player, Command&) const;
    std::int32_t query_limit(std::int32_t running_limit) const;
    std::span<void* const> owners() const noexcept {
        return owners_.entries();
    }
    engine::OwnerRegistry& native_storage() const noexcept {
        return owners_;
    }
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }
    const CommandOwnerDispatch& services() const noexcept {
        return services_;
    }
    //433E4A38 alone frees the current begin through433837D0. It preserves
    //all three BSS cells; C++ destruction frees neither buffer nor receivers.
    void finalize_storage() const;

private:
    std::shared_ptr<engine::NativeDataImage> image_;
    engine::OwnerRegistry& owners_;
    engine::JobMemory memory_;
    CommandOwnerDispatch services_;
};
} //namespace skeet::client

// Declarations from command_pipeline.hpp

namespace skeet::client {
// Native constructor-registration primitive 43345EBC. Registration order is
// insertion order; the -4 x86 receiver appends a null entry and is still returned.
void* register_command_phase_owner(std::vector<void*>&, void* owner);
// Inline CreateMove state transitions, shared by its concrete feature phases.
void advance_command_cycle_flags(std::uint32_t& flags, std::int32_t choked) noexcept;
void enforce_command_choke_limit(std::uint8_t& send_packet, std::int32_t choked,
                                 std::int32_t limit) noexcept;
// If any angle contains an infinity or NaN, native clears all three angles.
void sanitize_final_command_angles(Command&) noexcept;
void record_command_shot(std::uint32_t frame_flags, const Command&, std::int32_t last_outgoing,
                         std::int32_t& last_shot) noexcept;

struct SentCommandRecord {
    std::int32_t command_number, adjusted_tick;
};
struct SentCommandHistory {
    std::array<float, 3> angles; // 43476414, normalized only when sending
    std::array<SentCommandRecord, 150> records; // 43476450, stride 8
};
// Production arrays are separated by0x3C, unlike the legacy standalone
// aggregate above. These views retain the two actual source arrays separately.
struct SentCommandHistoryView {
    std::array<float, 3>& angles;
    std::array<SentCommandRecord, 150>& records;
    SentCommandHistoryView(std::array<float, 3>& a, std::array<SentCommandRecord, 150>& r) noexcept
        : angles(a), records(r) {}
    SentCommandHistoryView(SentCommandHistory& history) noexcept
        : angles(history.angles), records(history.records) {}
};
struct ConstSentCommandHistoryView {
    const std::array<float, 3>& angles;
    const std::array<SentCommandRecord, 150>& records;
    ConstSentCommandHistoryView(SentCommandHistoryView history) noexcept
        : angles(history.angles), records(history.records) {}
    ConstSentCommandHistoryView(const SentCommandHistory& history) noexcept
        : angles(history.angles), records(history.records) {}
    ConstSentCommandHistoryView(SentCommandHistory&&) = delete;
    ConstSentCommandHistoryView(const SentCommandHistory&&) = delete;
};
struct SentCommandTiming {
    std::int32_t last_outgoing;
    std::int32_t base_tick; // *43475A60 +4
    bool have_ack;
    std::int32_t acknowledged; // *43468F9C
    bool have_sent;
    std::int32_t sent; // *434670A8
};
// Complete inline 433D53A5..433D5406, including the real recovered
// 433E9160 angle normalization and 4340C3FA adjusted-tick calculation.
void record_sent_command_history(SentCommandHistory&, const Command&, std::uint8_t send_packet,
                                 const SentCommandTiming&) noexcept;
void record_sent_command_history(SentCommandHistoryView, const Command&, std::uint8_t send_packet,
                                 const SentCommandTiming&) noexcept;
} // namespace skeet::client

// Declarations from command_prediction_cache.hpp

namespace skeet::client {
struct CommandBoneBackup {
    const Matrix3x4* storage;
    std::int32_t count;
};
// Inline433D569F..433D575F. Native copies the live cache count when it is at
// least the captured count. The backing storage must cover that live count,
// including retained capacity after a smaller captured logical size.
void restore_command_prediction_cache(std::uint32_t frame_flags, void* player,
                                      const lagcomp::EntityNetvars&, const CommandBoneBackup&);
} // namespace skeet::client

// Declarations from command_recapture.hpp

namespace skeet::client {
class CommandCopyOverrideScope;
struct CommandEyeAdjustmentContext;
struct CommandEyeAdjustmentSourceContext;
struct CommandRecaptureOffsets {
    lagcomp::EntityNetvars common;
    std::ptrdiff_t active_weapon; // CD812654
    std::ptrdiff_t byte_7501cb91;
    std::ptrdiff_t view_offset; // C246315E
    std::ptrdiff_t recoil_angles; // 1BAE1168
    std::ptrdiff_t field_66146802;
    std::ptrdiff_t field_5ae32976;
    std::ptrdiff_t weapon_field_572aa59f;
};
struct CommandRecaptureContext {
    void* player;
    void* entity_list;
    CommandRecaptureOffsets offsets;
    const weapon::FireStateBindings& fire;
    engine::NativeConVarReference recoil_scale_convar; //4346794C^C429E1BF; terminal+2C
    const std::uint8_t* secondary_override; // actual434693D4
    const RageTrigonometry& trigonometry;
    std::array<float, 3>& published_origin; //43475BD4
};
// Complete ordinary-player433AE483. Reads the terminal ConVar on every call.
std::array<float, 3> capture_command_recoil(void* player, std::ptrdiff_t recoil_offset,
                                            void* recoil_scale_convar);
//433D4558..433D46F4, before the second prediction transaction is restored.
void update_predicted_command_weapon(const CommandRecaptureContext&);
//433D4709..433D4DBE, after the actual CommandPrediction::restore().
void capture_predicted_command_frame(CommandFrame&, Command&, const CommandRecaptureContext&);
// Shared physical state capture, excluding the recapture-only flags|=1.
// Initial433D3Axx..405x and second prediction retain their own surrounding gates.
enum class CommandCapturePass {
    initial,
    predicted
};
void capture_command_physical_frame(CommandFrame&, Command&, const CommandRecaptureContext&,
                                    CommandCapturePass);
// Complete second-prediction tail433D4558..433D4DEA. The active prediction
// and outer override were constructed by the master at4404 and439D.
void recapture_command_prediction(CommandFrame&, Command&, prediction::CommandPrediction&,
                                  CommandCopyOverrideScope&, const CommandRecaptureContext&,
                                  const CommandEyeAdjustmentContext&);
void recapture_command_prediction(CommandFrame&, Command&, prediction::CommandPrediction&,
                                  CommandCopyOverrideScope&, const CommandRecaptureContext&,
                                  const CommandEyeAdjustmentSourceContext&);
} // namespace skeet::client

// Declarations from command_runtime_owner.hpp

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
class HookRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class SharedEspRuntimeOwner;
class LocalAnimationRuntimeOwner;
struct EngineRuntimeBindings;

// Process ownership for the complete recovered CreateMove framing and its
// accepted-command transaction. Early construction composes prediction,
// packet-angle, mouse and packet-transition contexts from canonical owners;
// it invokes no engine method, feature factory or command callback.
//
// The three late contexts are mandatory. complete() retains their real source
// owners and rejects mismatched canonical cells before publishing a runtime.
// Hook installation and factory registration remain separate startup phases.
// Every borrowed owner must outlive this object and all installed hooks.
class CommandRuntimeOwner final {
public:
    CommandRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                        ConfigurationRuntimeOwner&, HistoryRuntimeOwner&,
                        CommandCaptureRuntimeOwner&, animation::RuntimeOwner&);
    ~CommandRuntimeOwner();
    CommandRuntimeOwner(const CommandRuntimeOwner&) = delete;
    CommandRuntimeOwner& operator=(const CommandRuntimeOwner&) = delete;
    CommandRuntimeOwner(CommandRuntimeOwner&&) = delete;
    CommandRuntimeOwner& operator=(CommandRuntimeOwner&&) = delete;

    ClientRuntimeStorage& storage() const noexcept;
    CommandCaptureRuntimeOwner& capture() const noexcept;
    HistoryRuntimeOwner& history() const noexcept;
    const prediction::CommandBindings& prediction() const noexcept;
    const prediction::CommandPlayerOffsets& prediction_player_offsets() const noexcept;
    const CommandPacketAngleBindings& packet_angles() const noexcept;
    const CommandWeaponPostprocessBindings& postprocess() const noexcept;
    const CommandPacketTransitionBindings& packet_transition() const noexcept;
    const CommandTrigonometry& trigonometry() const noexcept;

    void complete(engine::HookRuntimeOwner&, LocalAnimationRuntimeOwner&,
                  TargetRefreshRuntimeOwner&, SharedEspRuntimeOwner&);
    bool complete() const noexcept;
    // Both operations fail explicitly while any required late context is
    // absent. bind_callback() publishes the concrete source callback context;
    // it installs no engine hook and executes no command.
    CommandCreateMoveRuntime& runtime() const;
    void bind_callback();

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from command_transform.hpp

namespace skeet::client {
struct CommandTrigonometry {
    float (*sine)(float);
    float (*cosine)(float);
};
struct CommandButtonPolicy {
    std::span<const std::uint32_t, 150> inverted_commands; // 43476090
    const std::int32_t& mode; // canonical Leg movement434685F8, read at433F1594
    std::int32_t choked; // *434757DC
};
// Complete 433F1594. The final native return value is a scratch pointer and
// is not consumed by the caller; observable command-button writes are retained.
void rebuild_command_direction_buttons(Command&, bool force_refresh,
                                       const CommandButtonPolicy&) noexcept;
// Complete 433B7FEA, including its original early exits, horizontal-basis
// projection, roll-dependent solve, speed scaling and direction-button update.
void reorient_command_movement(Command&, const std::array<float, 3>& original_angles,
                               std::uint8_t movement_type, bool on_ground,
                               bool force_button_refresh, const CommandButtonPolicy&,
                               const CommandTrigonometry&);
// Complete 4332B266, including its real 433240A6 roll guard.
void sanitize_command(Command&, bool unrestricted_angles, const ScriptCommandEnvironment&) noexcept;
} // namespace skeet::client

// Declarations from command_weapon_postprocess.hpp

namespace skeet::client {
struct CommandMouseOffsets {
    std::ptrdiff_t player_flags; // 43468B50 / 38CA5228, uint32
    std::ptrdiff_t player_state_043fa575; // 4346757C, byte
    std::ptrdiff_t player_state_334b460c; // 4346934C, byte
};
struct CommandWeaponPostprocessBindings {
    void* const& input; // live4346A894; Command[150] pointer at +F0
    engine::NativeConVarReference yaw_scale; //43467950 XOR66BFCA30
    engine::NativeConVarReference pitch_scale; //43467988 XOR01EAAD2F
    std::array<float, 3>* sent_angles; // 4347A4F0
    CommandMouseOffsets offsets;
};
// Complete 433D543B..433D566A, including 4341999F and the exact-number
// command lookup 433B9184 -> 43331EFC. Publishes a sent command's pitch/yaw
// with zero roll, then reconstructs mouse counts only for a changed command
// while the two player states match and player flag bit 3 is clear.
// The previous command is selected with unsigned command-number modulo 150.
void postprocess_command_after_phase7(const CommandWeaponPostprocessBindings&, const CommandFrame&,
                                      void* player, Command&, std::uint8_t send_packet) noexcept;
} // namespace skeet::client

// Declarations from config_initializers.hpp

namespace skeet::anti_aim {
struct GlobalConfig;
}
namespace skeet::esp {
struct OtherEspSettings;
struct PlayerEspSettings;
struct EffectsSettings;
}
namespace skeet::misc {
struct PingSpikeConfig;
}
namespace skeet::rage {
struct GlobalSettings;
}

namespace skeet::client {
class NativeStartupSources;

// Borrow the canonical menu/runtime descriptors. Their declarations retain the
// native image's initial hash/type/size; attaching these references writes no
// metadata or values. Keep this context alive throughout startup dispatch.
struct ConfigInitializerBindings {
    ConfigRegistries& configuration;
    esp::OtherEspSettings& other_esp;
    esp::PlayerEspSettings& player_esp;
    esp::EffectsSettings& effects;
    rage::GlobalSettings& rage;
    misc::PingSpikeConfig& ping_spike;
    anti_aim::GlobalConfig& anti_aim;
    BooleanConfig& target_mask; // descriptor43467974, hash347A4FA6
};

// Register eighteen independent ordinary CRT source bodies. This does not run
// them or select their order; NativeStartupRuntime follows the live operands.
void bind_native_config_initializers(NativeStartupSources&, ConfigInitializerBindings&);

} // namespace skeet::client

// Declarations from crt_initialization.hpp

namespace skeet::client {
class NativeStartupSources;
struct CrtInitializationSlot {
    std::uint32_t operand;
    std::uint32_t identifier;
};

// Explicit CRT configuration for the supplied build, projected from ctx.h.
// Native43481E78..43481E92 establishes traversal order, while the identifiers
// are observed constants. This is not a reconstruction of43493903 or proof
// that its outputs remain invariant in another execution environment.
// NativeStartupRuntime consumes this fixed source configuration. The explicitly
// separate RetainedNativeStartupRuntime still uses the real live accessor.
std::span<const CrtInitializationSlot, 182> crt_initialization_plan() noexcept;
// The supplied build's observed precheck at operand2DC is zero. Keeping this
// explicit avoids pretending to implement a live protected operand accessor.
CrtInitializationSlot crt_initialization_precheck() noexcept;

// Check every nonempty slot before any CRT execution. This validates source
// binding availability only; callback43484F06 requires an explicit nonempty
// binding to its actual implementation/context. This invokes no body.
void preflight_crt_initialization(const NativeStartupSources&);
} //namespace skeet::client

// Declarations from crt_startup_bindings.hpp

namespace skeet::animation {
class FixAnimationBackupCleanupContext;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::client {
class NativeStartupSources;
class CrtRuntimeOwner;
class ConfigurationRuntimeOwner;
class StaticCrtRuntimeOwner;
class SourceTerminalCleanupBindings;

// Bind the complete182-slot CRT plan using only process storage. Engine and
// feature owners are acquired later by the supplied final43484F06 callback.
// All contexts must outlive the exit callbacks. This performs no initializer,
// engine acquisition or allocation of a feature owner; it checks the whole
// CRT plan before the scheduler is allowed to begin.
void bind_early_crt_sources(NativeStartupSources&, CrtRuntimeOwner&, ConfigurationRuntimeOwner&,
                            StaticCrtRuntimeOwner&, script::SourceScriptRuntimeOwner&,
                            animation::FixAnimationBackupCleanupContext&,
                            SourceTerminalCleanupBindings&);
} //namespace skeet::client

// Declarations from entity_command_lifecycle.hpp

namespace skeet::client {
class NativeStartupSources;

struct EntityCommandOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //433676A0
    CommandPhaseRegistry& commands; //43345EBC
    prediction::ObserverRegistry& prediction; //4341CEE4
    network::DataUpdateObserverRegistry& data_updates; //4337EFD9
    EntityObserverRegistry& entities; //4333BF32
    engine::OwnerRegistryBinding map_entities; //433817AA
    network::ExploitRechargeObserverRegistry& recharge; //4334E132
};
struct EntityCommandFactoryBindings {
    EntityCommandOwnerRegistries registries;
    EntityCommandOwner& command;
    // The mutable object referenced by command.bindings, not another copy.
    EntityCommandBindings& command_bindings;
    std::shared_ptr<engine::NativeDataImage> data;
    void* const& engine_client; //4346A514
    void* const& prediction_interface; //43469264
    const std::uint8_t& shot_recording_enabled; //43467224
    double (*ceiling)(double); //actual recorded CRT ceil77512A30
};

std::size_t entity_command_owner_size() noexcept;
// Whole constructor4347F928. The same caller-supplied command/state remains
// the receiver of all six registered secondary interfaces; neither is copied.
void construct_entity_command_owner(void* storage, const EntityCommandFactoryBindings&);
// Whole factory43486705. Publishes the actual43475898 source-owner slot only
// after construction; allocation failure publishes null, as its proven branch.
void* create_entity_command_owner(const EntityCommandFactoryBindings&);
void destroy_entity_command_owner(void* storage, const engine::NativeDataImage&);
void reset_entity_command_owner(void* storage); //43388375
void refresh_entity_command_owner(void* storage); //43384729
// Bind the actual factory identity without selecting another factory's order.
void bind_native_entity_command_factory(NativeStartupSources&, EntityCommandFactoryBindings&);
} // namespace skeet::client

// Declarations from entity_command_owner.hpp

namespace skeet::prediction {
struct ObserverRegistry;
struct PlayerCommandContext;
}
namespace skeet::animation {
struct AnimationAngleHistory;
}
namespace skeet::legit {
struct RecoilDecayBindings;
}
namespace skeet::misc {
struct NativeText;
}
namespace skeet::client {
struct RestrictedPlayerCommandOwner;
struct PredictionHistoryOwner;
class PlayerStateCache;
// Actual CFDA2 stack argument, not a command number. Native reloads count
// after every item; the second word in each entry is not read by this owner.
struct EntityPredictionEntry {
    std::int32_t entity_index;
    std::uint32_t field_04;
};
struct EntityPredictionEntities {
    std::array<std::byte, 0x44> prefix;
    std::array<EntityPredictionEntry, 2048> entries;
    std::int32_t count;
};
static_assert(offsetof(EntityPredictionEntities, entries) == 0x44);
static_assert(offsetof(EntityPredictionEntities, count) == 0x4044);
struct RevolverCommandPrediction {
    std::int32_t command_number;
    std::uint32_t weapon_state;
    float postpone_fire;
};
static_assert(sizeof(RevolverCommandPrediction) == 0xC);
struct EntityModelCategoryNode {
    std::uint32_t model_hash, category;
    EntityModelCategoryNode* next;
};
struct EntityModelCategories {
    // One extra entry is the native end marker at buckets[bucket_count].
    std::span<EntityModelCategoryNode* const> buckets;
};
struct EntityDataUpdateSnapshot {
    std::uint32_t field_7f5ed2d0; //full owner+40
    std::array<std::uint32_t, 3> recoil_angles; //+44..4C, raw float words
    std::uint32_t scaled_recoil; //+50
    std::int32_t tick_base, health; //+54/+58
    std::uint32_t field_47a6f50c; //+5C
};
struct EntityAngleConfirmation {
    std::uint32_t index, count; //43479794/43479798
    std::array<std::uint8_t, 3> values; //4347979C..9E
};
static_assert(offsetof(EntityAngleConfirmation, values) == 8);
static_assert(sizeof(EntityDataUpdateSnapshot) == 0x20);
static_assert(offsetof(EntityDataUpdateSnapshot, tick_base) == 0x14);
struct EntityCommandState {
    EntityDataUpdateSnapshot data_update; //one canonical full owner+40..5C snapshot
    std::int32_t last_choked_shot = 0; // full owner+60
    float revolver_delay = 0.23125f; // full owner+64, constructor3E6CCCCD
    RevolverCommandPrediction* revolver; // full owner+68, same150-entry producer
    prediction::CommandHistoryEntry* history; // full owner+6C, canonical history
    std::uint32_t prediction_stamp = 0; // full owner+70, copied as raw time bits
    std::uint32_t data_update_flags = 0; //full owner+74
    const EntityModelCategories* model_categories; // full owner+78/+7C table
};
static_assert(offsetof(EntityCommandState, last_choked_shot) == 0x20);
static_assert(offsetof(EntityCommandState, revolver_delay) == 0x24);
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(EntityCommandState, revolver) == 0x28);
static_assert(offsetof(EntityCommandState, history) == 0x2C);
static_assert(offsetof(EntityCommandState, prediction_stamp) == 0x30);
static_assert(offsetof(EntityCommandState, data_update_flags) == 0x34);
#endif
struct EntityCommandStorage {
    std::array<RevolverCommandPrediction, 150> revolver;
    std::array<prediction::CommandHistoryEntry, 150> history;
};
static_assert(sizeof(std::array<RevolverCommandPrediction, 150>) == 0x708);
static_assert(sizeof(std::array<prediction::CommandHistoryEntry, 150>) == 0x5460);
// Constructor4347F9CA..4347FA24 buffer initialization. Consumers bind views
// of these same arrays; the canonical producer never maintains a mirror.
void initialize_entity_revolver_storage(RevolverCommandPrediction*) noexcept;
void initialize_entity_prediction_storage(prediction::CommandHistoryEntry*) noexcept;
void initialize_entity_command_storage(EntityCommandStorage&) noexcept;
void bind_entity_command_storage(EntityCommandState&, EntityCommandStorage&) noexcept;
struct EntityCommandOffsets {
    std::ptrdiff_t prediction_context; //5DF1A4BC, last command is this+68
    std::ptrdiff_t prediction_angles; //3B1F6ECC, vector
    std::ptrdiff_t tick_base; //3D10B94C
    std::ptrdiff_t vector_5ae32976, vector_1bae1168, vector_48442721;
    std::ptrdiff_t view_offset; //C246315E
    std::ptrdiff_t scalar_2f33c681, field_b5a66556, field_fb7e3404, vector_0317eede;
    std::ptrdiff_t active_weapon; //CD812654
    std::ptrdiff_t weapon_definition, postpone_fire; //5C9F0FBB,43D0EFC4
    std::ptrdiff_t rejected_9d920e35, rejected_cde290d2, team, network_origin;
    std::ptrdiff_t move_type, owner_handle, ground_flags; //7501CB91,8D1BD8D0,E2FBE695
    std::ptrdiff_t activity_8dbc9874, activity_68140cd8, collision_d5fd2e2c;
    std::ptrdiff_t player_flags_38ca5228, state_043fa575, state_334b460c,
        interpolated_origin_6610746e;
    std::ptrdiff_t field_66146802, field_00a5bd68, field_6b1a59b5, field_50f9ccc8;
    std::ptrdiff_t shots_fired_8cfc633a, field_7f5ed2d0, field_bcb263a5;
    std::ptrdiff_t weapon_state_9e2078a1, weapon_ammunition_ca5ec698, weapon_field_0176a381;
    std::ptrdiff_t weapon_field_6777c9ce, weapon_field_d2563274, weapon_field_15335aac;
    std::ptrdiff_t weapon_field_4013fc03, weapon_field_e865a877, weapon_field_3b93667b;
    std::ptrdiff_t weapon_viewmodel_index_c6c78c82;
    std::ptrdiff_t player_field_6e9852d6, player_field_432b1f5b, player_field_0d560fac;
    std::ptrdiff_t view_models_71dcee42, observer_mode_2f4e97c4, observer_target_3b704b41,
        viewmodel_695ff325;
    std::ptrdiff_t field_47a6f50c;
    std::ptrdiff_t weapon_field_01aa449d, viewmodel_weapon_a116369a, viewmodel_sequence_a25d8271;
};
using RestorePredictionFrame = engine::MemberFunction<void, std::int32_t, std::int32_t>;
class EntityAnimationAngleBindings {
public:
    EntityAnimationAngleBindings(animation::AnimationAngleHistory* standalone) noexcept
        : standalone_(standalone) {}
    EntityAnimationAngleBindings(std::uint8_t& active, const std::int32_t* const& frame,
                                 std::array<animation::AnimationAngleSample, 2>& samples) noexcept
        : active_(&active), frame_(&frame), samples_(&samples) {}
    void set_active(bool) const noexcept;
    bool select(std::int32_t frame, RageVector&) const noexcept;

private:
    animation::AnimationAngleHistory* standalone_ = nullptr;
    std::uint8_t* active_ = nullptr;
    const std::int32_t* const* frame_ = nullptr;
    std::array<animation::AnimationAngleSample, 2>* samples_ = nullptr;
};
class EntityConnectionNotification {
public:
    using Entry = void (*)();
    EntityConnectionNotification(Entry value) noexcept : value_(value) {}
    EntityConnectionNotification(std::reference_wrapper<Entry> cell) noexcept
        : value_(&cell.get()) {}
    Entry get() const noexcept {
        if (const auto* cell = std::get_if<Entry const*>(&value_))
            return **cell;
        return std::get<Entry>(value_);
    }
    explicit operator bool() const noexcept {
        return get() != nullptr;
    }
    void operator()() const {
        get()();
    }

private:
    std::variant<Entry, Entry const*> value_;
};
struct EntityCommandBindings {
    void* engine_client; //full owner+34; view18/19 and command query205
    void* prediction; //full owner+3C; actual Update slot3
    void* entity_list; //4346A890
    void* input; //4346A894; actual command ring+F0
    void* model_info; //4346A614; model-name slot3
    engine::ConVarBinding teammates_are_enemies; //live43468CAC XOR D8ED8A94, ConVar
    engine::ConVarBinding interpolation_enabled; //live4346A850 XOR478720C2, ConVar
    engine::ConVarBinding recoil_scale_convar; //live4346794C XORC429E1BF, ConVar
    const std::uint8_t* shot_recording_enabled; //full owner+38 ->43467224
    const std::int32_t* startup_guard_4347bdac;
    const std::int32_t* startup_guard_4347bd78;
    std::uint8_t* connection_checked; //434758B0
    std::uint8_t* deferred_connection; //43475BD2
    EntityConnectionNotification connection_notification; //live optional43468BF0
    const std::int32_t* const& update_frame; //live pointer434758D8
    const std::int32_t* const& old_acknowledged; //live pointer43475794
    const std::int32_t* last_outgoing; //43475B2C
    const std::int32_t* choked_commands; //434757DC
    const std::int32_t* process_milliseconds; //43468290
    void* engine_globals; //43467B04; maxclients+18,receiveinterval+20,tick+28
    const float* prediction_time; //4346A334
    const float* interval; //4346A338
    float* simulation_time; //43467EB8, saved/restored by short-gap correction
    const legit::RecoilDecayBindings* recoil_decay; //three actualConVars/CRTexpf
    std::array<float, 3>* sent_angles; //4347BD30
    RestorePredictionFrame restore_prediction_frame; //actual optional4346D660
    engine::MemberFunction<std::int32_t, std::int32_t> sequence_activity; //actual optional4346A364
    const std::int32_t* data_update_backup_mode; //4347C9EC
    std::int32_t* data_update_reset_marker; //43467114
    std::uint32_t* scheduler_flags; //434759C0
    void* const* physics_class; //43467478; actual client-class identity
    prediction::ObserverRegistry* observers; //canonical43475AB0/AB4 registry
    PredictionHistoryOwner* const& shared_animation_owner; //live canonical434759B4 source owner
    PlayerStateCache* players; //canonical433B8A2B cache
    EntityAnimationAngleBindings angle_history; //canonical434759DC/E8,active43468D7C
    EntityAngleConfirmation* angle_confirmation; //canonical43479794/98/9C queue
    std::span<const std::uint32_t, 3> weapon_classification; //43474580
    EntityCommandOffsets offsets;
};
struct EntityCommandOwner {
    EntityCommandState& state;
    const EntityCommandBindings& bindings;
    network::NativeDataUpdateReceiver data_update_receiver{};
    network::NativeExploitRechargeReceiver recharge_receiver{};
    prediction::NativePredictionObserverReceiver prediction_receiver{};
    // Constructed by the actual lifecycle owner, after its eight NativeTexts.
    // This source view never interprets native owner offsets as C++ members.
    std::span<const misc::NativeText> prediction_names{};
};
// Typed carries from43403836..434038F2, shared by the concrete correction
// and finalization cuts. current_record is selected afterward at43404CCF.
struct EntityDataUpdateWork {
    std::int32_t acknowledged_command, acknowledged_delta;
    std::int32_t captured_tick_base, current_tick_base;
    std::uint32_t original_recoil_pitch;
    bool field_7f5ed2d0_changed;
    prediction::CommandHistoryEntry* current_record = nullptr;
};
// Full4334BB84. Quantization repair and prediction Update use the same shared
// command/revolver history as the producer; no second history is allocated.
void process_entity_command_phase1(EntityCommandOwner&, void* player, Command&);
// Full433E2400: remember choked classified shots and invalidate prediction on
// a strict overlap with an eligible player's collision box.
void process_entity_command_phase6(EntityCommandOwner&, const CommandFrame&, void* player,
                                   void* weapon, const Command&, std::uint8_t send_packet);
// Full433CFDA2, typed pointer ABI; ignores its native receiver.
void process_entity_prediction_entities(EntityCommandOwner&, const EntityPredictionEntities&);
// Full433BFDDB, the distinct43475784 slot1 observer. Number is ignored.
void observe_entity_command_recharge(EntityCommandOwner&, std::int32_t command_number) noexcept;
// Actual43475AB0 source observer endpoints, receiver full owner+10.
void begin_entity_prediction(EntityCommandOwner&, void* player, const Command&); //4332BBA4
void finish_entity_prediction(EntityCommandOwner&, void* player, const Command&); //43382703
void reject_entity_prediction(EntityCommandOwner&, void* player,
                              const prediction::PlayerCommandContext&); //433B265C
// Shared concrete field repair in BB84/BBA4/B265C. Includes the native
// tick/flag/time gate, vector tolerances and fixed14-bit scalar quantizer.
void repair_entity_prediction_fields(EntityCommandOwner&, void* player,
                                     const prediction::CommandHistoryState&);
float quantize_entity_prediction_scalar(float) noexcept; //433FD134 fixed14bits,range0..100
// Shared canonical history producer tail in82703/B265C; writes the existing
//150-entry0x90 ring and retains its native per-slot first-capture fields.
void capture_entity_prediction_history(EntityCommandOwner&, void* player,
                                       std::int32_t command_number);
// Concrete43342DD7 indexed viewmodel lookup with live observer-target recursion.
void* entity_command_view_model(EntityCommandOwner&, void* player, std::int32_t index);
// Data-update table4346F814, receiver full owner+18. This is the distinct
//43475760 observer family; capture takes its snapshot before other updates.
void process_entity_data_update_capture(EntityCommandOwner&, void* player,
                                        std::int32_t update); //433D67DF
void process_entity_data_update_finish(EntityCommandOwner&, void* player,
                                       std::int32_t update); //43403681
// Shared reset tail used by43388375 and43403681; only the original ring tags,
// recoil snapshot, last shot and global reset marker are overwritten.
void reset_entity_prediction_history(EntityCommandOwner&) noexcept;
// Concrete inlined43403681 stages, split into readable source units.
std::uint32_t entity_data_update_difference_flags(EntityCommandOwner&, void* player,
                                                  const prediction::CommandHistoryState&,
                                                  std::int32_t command_number,
                                                  bool compare_angles = true);
float entity_data_update_quantized_angle(float) noexcept; //full4337F9D3
void reconcile_entity_data_update_ack(EntityCommandOwner&, void* player,
                                      const EntityDataUpdateWork&);
void finish_entity_data_update_tail(EntityCommandOwner&, void* player, const EntityDataUpdateWork&);
} //namespace skeet::client

// Declarations from entity_command_runtime_owner.hpp

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class LocalAnimationRuntimeOwner;
class CommandCaptureRuntimeOwner;
class NativeStartupSources;
struct EngineRuntimeBindings;
struct EntityCommandState;
struct EntityCommandBindings;
struct EntityCommandOwner;
struct EntityCommandFactoryBindings;
struct EntityCommandRuntimeImports {
    double (*ceiling)(double);
    float (*exponential)(float);
};
EntityCommandRuntimeImports native_entity_command_runtime_imports();

// Stable contexts for43486705. Its real constructor allocates the only
// revolver/prediction rings and model-category table. No player/model/history
// buffer or registration is manufactured while composing these contexts.
class EntityCommandRuntimeOwner final {
public:
    EntityCommandRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                              ConfigurationRuntimeOwner&, HistoryRuntimeOwner&,
                              LocalAnimationRuntimeOwner&, CommandCaptureRuntimeOwner&);
    EntityCommandRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                              ConfigurationRuntimeOwner&, HistoryRuntimeOwner&,
                              LocalAnimationRuntimeOwner&, CommandCaptureRuntimeOwner&,
                              EntityCommandRuntimeImports);
    ~EntityCommandRuntimeOwner();
    EntityCommandRuntimeOwner(const EntityCommandRuntimeOwner&) = delete;
    EntityCommandRuntimeOwner& operator=(const EntityCommandRuntimeOwner&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    EntityCommandState& state() noexcept;
    EntityCommandOwner& command() noexcept;
    EntityCommandBindings& bindings() noexcept;
    EntityCommandFactoryBindings& factory_bindings() noexcept;
    void initialize_prediction_scratch(bool local_creation);
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from entity_observer_receiver.hpp

namespace skeet::client {
#if defined(_M_IX86) || defined(__i386__)
using EntityStatusEntry = void(__fastcall*)(void*, const void*);
using EntityUpdateEntry = void(__fastcall*)(void*, void*, std::uintptr_t);
using EntityResetEntry = void(__fastcall*)(void*, std::int32_t);
using EntityIncomingEntry = void(__fastcall*)(void*, void*, std::int32_t);
#else
using EntityStatusEntry = void (*)(void*, const void*);
using EntityUpdateEntry = void (*)(void*, void*, std::uintptr_t);
using EntityResetEntry = void (*)(void*, std::int32_t);
using EntityIncomingEntry = void (*)(void*, void*, std::int32_t);
#endif
// ECX is always the physical secondary receiver. Slots0/2 consume EDX only
// and return0; slots1/3 also consume one stack word and return4.
struct NativeEntityObserverVtable {
    EntityStatusEntry status;
    EntityUpdateEntry update;
    EntityResetEntry reset;
    EntityIncomingEntry incoming;
};
struct NativeEntityObserver {
    const NativeEntityObserverVtable* vtable = nullptr;
};
// This metadata belongs inside the actual whole owner. Its source points to
// that owner's existing state/view; the native buffer stores only the prefix.
struct NativeEntityObserverReceiver : NativeEntityObserver {
    void* source = nullptr;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(NativeEntityObserverVtable) == 0x10);
static_assert(offsetof(NativeEntityObserverVtable, update) == 4);
static_assert(offsetof(NativeEntityObserverVtable, reset) == 8);
static_assert(offsetof(NativeEntityObserverVtable, incoming) == 0xC);
static_assert(sizeof(NativeEntityObserver) == 4);
static_assert(sizeof(NativeEntityObserverReceiver) == 8);
#endif
} // namespace skeet::client

// Declarations from entity_observer_registry.hpp

namespace skeet::esp {
class SoundHistoryReceiver;
}
namespace skeet::lagcomp {
struct HistoryRecordingReceiver;
}
namespace skeet::misc {
class ClanTagRuntime;
class GrenadeCommandRuntime;
}
namespace skeet::prediction {
struct EngineEntityUpdateOwner;
struct ViewmodelPredictionObserver;
}
namespace skeet::rage {
struct FeedbackEntityOwner;
struct EntityUpdateBindings;
}
namespace skeet::script {
class ScriptEntityObserver;
}
namespace skeet::skins {
class NativeEntityCosmetics;
class NativeInventoryRuntime;
class NativeSequenceRuntime;
struct SequenceEntityObserverReceiver;
}
namespace skeet::effects {
struct EffectsCommandOwner;
}
namespace skeet::client {
struct PredictionHistoryOwner;
struct EntityCommandOwner;
struct SharedEspCommandOwner;
class TargetMaskOwner;
//Registrar4333BF32, vector43475BA8/BAC. This is separate from the command,
//empty-tick, recharge and engine-prediction registries.
enum class EntityObserverId : std::uint32_t {
    prediction_history = 0x43471DD4u,
    clan_tag = 0x4346EEA0u,
    history_recording = 0x43471D6Cu,
    mask = 0x43470560u,
    shot_feedback = 0x4346F68Cu,
    entity_commands = 0x4346FBF0u,
    scripts = 0x434729C0u,
    rage = 0x43472480u,
    target_mask = 0x4346F98Cu,
    pickup_history = 0x43472670u,
    prediction_update = 0x43472388u,
    shared_esp = 0x4346F2D0u,
    grenades = 0x4346DC78u,
    viewmodel_lag = 0x43472644u,
    sequences = 0x4346DC68u,
    effects = 0x4346FAFCu,
    inventory = 0x4347030Cu,
    gloves = 0x4346FEBCu,
};
struct EntityObserverTable {
    EntityObserverId id;
    std::array<std::uint32_t, 4> methods;
};
std::span<const EntityObserverTable> native_entity_observer_tables() noexcept;
std::optional<EntityObserverId> identify_entity_observer(std::uint32_t) noexcept;
bool entity_observer_source_bound(EntityObserverId, std::uint32_t slot);

struct EntityObserverRegistry {
    EntityObserverRegistry(engine::PreservedInitialization&, engine::JobMemory);
    EntityObserverRegistry(std::shared_ptr<engine::NativeDataImage>, engine::JobMemory);
    EntityObserverRegistry(const EntityObserverRegistry&) = delete;
    EntityObserverRegistry& operator=(const EntityObserverRegistry&) = delete;
    EntityObserverRegistry(EntityObserverRegistry&&) = delete;
    EntityObserverRegistry& operator=(EntityObserverRegistry&&) = delete;
    // Actual constructors supply both their physical endpoint and their same
    // canonical source view. Publication follows at each recorded table write.
    void append_prediction_history(NativeEntityObserverReceiver&, PredictionHistoryOwner&);
    void append_clan_tag(NativeEntityObserverReceiver&, misc::ClanTagRuntime&);
    void append_mask(NativeEntityObserverReceiver&, skins::NativeEntityCosmetics&);
    void append_feedback(NativeEntityObserverReceiver&, rage::FeedbackEntityOwner&);
    void append_entity_commands(NativeEntityObserverReceiver&, EntityCommandOwner&);
    void append_scripts(NativeEntityObserverReceiver&, script::ScriptEntityObserver&);
    void append_rage(NativeEntityObserverReceiver&, const rage::EntityUpdateBindings&);
    void append_target_mask(NativeEntityObserverReceiver&, TargetMaskOwner&);
    void append_prediction_update(NativeEntityObserverReceiver&,
                                  prediction::EngineEntityUpdateOwner&);
    void append_shared_esp(NativeEntityObserverReceiver&, SharedEspCommandOwner&);
    void append_grenades(NativeEntityObserverReceiver&, misc::GrenadeCommandRuntime*);
    void append_viewmodel_lag(NativeEntityObserverReceiver&,
                              prediction::ViewmodelPredictionObserver&);
    void append_effects(NativeEntityObserverReceiver&, effects::EffectsCommandOwner&);
    void append_inventory(NativeEntityObserverReceiver&, skins::NativeInventoryRuntime&);
    void append_gloves(NativeEntityObserverReceiver&, skins::NativeEntityCosmetics&);
    void append(lagcomp::HistoryRecordingReceiver&);
    void append(esp::SoundHistoryReceiver&);
    void append(skins::SequenceEntityObserverReceiver&, skins::NativeSequenceRuntime&);
    void publish_prediction_history(NativeEntityObserverReceiver&) const noexcept;
    void publish_clan_tag(NativeEntityObserverReceiver&) const noexcept;
    void publish_mask(NativeEntityObserverReceiver&) const noexcept;
    void publish_feedback(NativeEntityObserverReceiver&) const noexcept;
    void publish_entity_commands(NativeEntityObserverReceiver&) const noexcept;
    void publish_scripts(NativeEntityObserverReceiver&) const noexcept;
    void publish_rage(NativeEntityObserverReceiver&) const noexcept;
    void publish_target_mask(NativeEntityObserverReceiver&) const noexcept;
    void publish_prediction_update(NativeEntityObserverReceiver&) const noexcept;
    void publish_shared_esp(NativeEntityObserverReceiver&) const noexcept;
    void publish_grenades(NativeEntityObserverReceiver&) const noexcept;
    void publish_viewmodel_lag(NativeEntityObserverReceiver&) const noexcept;
    void publish_effects(NativeEntityObserverReceiver&) const noexcept;
    void publish_inventory(NativeEntityObserverReceiver&) const noexcept;
    void publish_gloves(NativeEntityObserverReceiver&) const noexcept;
    void publish(lagcomp::HistoryRecordingReceiver&) const noexcept;
    void publish(esp::SoundHistoryReceiver&) const noexcept;
    void publish(skins::SequenceEntityObserverReceiver&) const noexcept;
    std::span<void* const> owners() const noexcept {
        return owners_.entries();
    }
    engine::OwnerRegistry& native_storage() const noexcept {
        return owners_;
    }
    const engine::NativeDataImage& image() const noexcept {
        return *image_;
    }
    //433E4C98 alone releases the current native buffer, without resetting its
    //three BSS cells. Ordinary C++ destruction releases no buffer or receiver.
    void finalize_storage() const;
    void require_sources(std::uint32_t slot) const;
    void update_status(const void* status) const; //slot0
    void update_entities(void* player, std::uintptr_t incoming) const; //slot1
    void reset_update_state(std::int32_t selected_tick) const; //slot2, EDX preserved
    void incoming_complete(void* saved_player, std::int32_t acknowledged_delta) const; //slot3
private:
    std::shared_ptr<engine::NativeDataImage> image_;
    engine::OwnerRegistry& owners_;
    engine::JobMemory memory_;
    void append_receiver(NativeEntityObserverReceiver&, void* source);
    void append_native(NativeEntityObserver&);
};
} //namespace skeet::client

// Declarations from entity_update_dispatch.hpp

namespace skeet::ui {
class SkinsMenu;
}
namespace skeet::client {
// Native43476908 list. Exchange targets and replacement words are borrowed;
// the engine's imported InterlockedExchange performs the actual atomic write.
struct EntityUpdatePatch {
    EntityUpdatePatch* next = nullptr;
    volatile std::int32_t* target = nullptr;
    std::int32_t after = 0, before = 0;
};
struct EntityUpdateGlobals {
    std::uint32_t& phase; //43476904, set1 before and4 after
    EntityUpdatePatch*& patches; //43476908, reread for the second pass
    std::int32_t& local_team; //4346D4BC
    EntityObserverRegistry& receivers; //43475BA8/AC/B0, concrete source owners
};
struct EntityUpdateBindings {
    using ExchangeWord = std::int32_t(__stdcall*)(volatile std::int32_t*, std::int32_t);
    void* engine_client = nullptr;
    void* entity_list = nullptr;
    std::uint32_t team_offset = 0; //resolved434688B0^4346A518^8B5586F6
    ExchangeWord exchange = nullptr; //recorded kernel32!InterlockedExchange
};
#if defined(_M_IX86) || defined(__i386__)
using EntityUpdateReceiver = void(__fastcall*)(void*, void*, std::uintptr_t);
#else
using EntityUpdateReceiver = void (*)(void*, void*, std::uintptr_t);
#endif
// Legacy native-shaped constructor registration used by owners while their
// bindings migrate. The live dispatcher below uses EntityObserverRegistry.
void* register_entity_update_receiver(engine::OwnerRegistry&, void* object, engine::JobMemory);

// Complete433205A0. No speculative engine name is assigned to original slot64.
// The local player and incoming engine argument reach every captured receiver,
// including the glove43342310/mask43408846 entries, even when local is null.
void dispatch_entity_update(void* hooked_object, std::uintptr_t incoming, EntityUpdateGlobals&,
                            const EntityUpdateBindings&, ui::SkinsMenu&);
struct EntityUpdateRuntime {
    EntityUpdateGlobals globals;
    EntityUpdateBindings bindings;
    ui::SkinsMenu& skins_menu;
    void process(void* object, std::uintptr_t incoming) {
        dispatch_entity_update(object, incoming, globals, bindings, skins_menu);
    }
};
void bind_entity_update_runtime(EntityUpdateRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
void __fastcall entity_update_hook(void* object, void* register_edx, std::uintptr_t incoming);
#else
void entity_update_hook(void* object, void* register_edx, std::uintptr_t incoming);
#endif
} //namespace skeet::client

// Declarations from exit_registration.hpp

namespace skeet::client {
struct ExitCallbacks;
class NativeStartupSources;
class StaticCleanupRuntime;

// Borrow the same 512-slot exit registry used by ShutdownRuntime and the
// concrete static-storage cleanup owner. Both owners and this binding object
// must remain at stable addresses through startup and ExitCallbacks::run().
struct ExitRegistrationContext {
    ExitCallbacks& callbacks;
    StaticCleanupRuntime& cleanup;
    ExitRegistrationContext(ExitCallbacks& callbacks, StaticCleanupRuntime& cleanup) noexcept
        : callbacks(callbacks), cleanup(cleanup) {}
    ExitRegistrationContext(const ExitRegistrationContext&) = delete;
    ExitRegistrationContext& operator=(const ExitRegistrationContext&) = delete;
    ExitRegistrationContext(ExitRegistrationContext&&) = delete;
    ExitRegistrationContext& operator=(ExitRegistrationContext&&) = delete;
};

// The fixed family contains nineteen ordinary entries at operands 008..050.
// Bind an entry only when its concrete finalizer's terminal storage owner is
// available; other entries remain explicitly unmapped in NativeStartupSources.
// Execution order remains the live metadata scheduler's order.
// Operand 004 / protected entry 434822C8 receives no source mapping here.
void bind_exit_initializers(NativeStartupSources&, ExitRegistrationContext&);
}

// Declarations from exploit_runtime_owner.hpp

namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::network {
struct ExploitCommandOwner;
struct ExploitState;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class MovementHudRuntimeOwner;
class CommandRuntimeOwner;
class LocalAnimationRuntimeOwner;
class VisualRuntimeOwner;
class NativeStartupSources;
struct EngineRuntimeBindings;
struct ExploitRuntimeFactoryBindings;
// One network/tickbase state shared by all six concrete exploit endpoints.
// Construction composes live inputs only; the4347FF2F factory still allocates,
// registers and initializes in the scheduler's recovered order.
class ExploitRuntimeOwner final {
public:
    ExploitRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                        ConfigurationRuntimeOwner&, HistoryRuntimeOwner&, MovementHudRuntimeOwner&,
                        CommandRuntimeOwner&, LocalAnimationRuntimeOwner&,
                        engine::SceneQueryRuntimeOwner&, VisualRuntimeOwner&,
                        script::SourceScriptRuntimeOwner&);
    ~ExploitRuntimeOwner();
    ExploitRuntimeOwner(const ExploitRuntimeOwner&) = delete;
    ExploitRuntimeOwner& operator=(const ExploitRuntimeOwner&) = delete;
    ExploitRuntimeOwner(ExploitRuntimeOwner&&) = delete;
    ExploitRuntimeOwner& operator=(ExploitRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    network::ExploitCommandOwner& command() noexcept;
    network::ExploitState& state() noexcept;
    ExploitRuntimeFactoryBindings& factory_bindings() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from feature_runtime_owner.hpp

namespace skeet::engine {
class HookRuntimeOwner;
}
namespace skeet::skins {
class SkinsRuntimeOwner;
}
namespace skeet::rage {
class RuntimeOwner;
}
namespace skeet::legit {
class RuntimeOwner;
}
namespace skeet::esp {
class EspRuntimeOwner;
}
namespace skeet::script {
class ScriptApiRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;

// Process lifetime for the eight factories shared by combat, rendering and
// events. Each domain supplies its existing physical state. Construction only
// composes references; the native factory order still controls registration.
class FeatureRuntimeOwner final {
public:
    FeatureRuntimeOwner(ClientRuntimeStorage&, ConfigurationRuntimeOwner&, HistoryRuntimeOwner&,
                        skins::SkinsRuntimeOwner&, rage::RuntimeOwner&, legit::RuntimeOwner&,
                        esp::EspRuntimeOwner&, script::ScriptApiRuntimeOwner&,
                        engine::HookRuntimeOwner&);
    ~FeatureRuntimeOwner();
    FeatureRuntimeOwner(const FeatureRuntimeOwner&) = delete;
    FeatureRuntimeOwner& operator=(const FeatureRuntimeOwner&) = delete;
    FeatureRuntimeOwner(FeatureRuntimeOwner&&) = delete;
    FeatureRuntimeOwner& operator=(FeatureRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    RuntimeOwnerFactoryBindings& factory_bindings() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from final_menu_startup.hpp

namespace skeet::misc {
class SettingsRuntimeOwner;
}
namespace skeet::ui {
class MainMenuWindowOwner;
class StartupRenderer;
}
namespace skeet::client {
class ClientRuntimeStorage;

// Publish the full feature-registry baseline after the ten private settings
// descriptors have been removed. The canonical NativeConfigVector owns the
// same aligned process-heap allocation used by its terminal finalizer.
void capture_startup_configuration_baseline(ClientRuntimeStorage&);

// The main initializer calls this after actual RootMenu/default-layout
// construction, before lifecycle owner slot1 and static patch installation.
// This phase allocates no second menu, settings registry or configuration.
class FinalMenuStartupTail {
public:
    FinalMenuStartupTail(ClientRuntimeStorage&, ui::MainMenuWindowOwner&,
                         misc::SettingsRuntimeOwner&, ConfigStorageOwner&, ui::StartupRenderer&,
                         ui::StartupRegistryImports);
    void initialize();
    bool complete() const noexcept {
        return complete_;
    }

private:
    ClientRuntimeStorage& storage_;
    ui::MainMenuWindowOwner& window_;
    misc::SettingsRuntimeOwner& settings_;
    ConfigStorageOwner& configuration_;
    ui::StartupRenderer& renderer_;
    ui::StartupRegistryImports imports_;
    bool started_ = false, complete_ = false;
};
} //namespace skeet::client

// Declarations from final_patch_startup.hpp

namespace skeet::engine {
class ClientClassFactoryRuntime;
class HookRuntimeOwner;
class HookDescriptorStorage;
struct UnloadState;
}
namespace skeet::effects {
class VisualReceiveOwner;
}
namespace skeet::skins {
class SkinsRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class PlayerReceiveOwner;

// The final initializer's first complete lifecycle pass calls primary1 on
// the registered57 owners. Its connected pass calls primary5, independently
// of the later4341F57F primary2/primary7 passes.
void initialize_final_feature_callbacks(ClientRuntimeStorage&);

// Devirtualized native final capture order: six ClientClass.create slots,
// fifteen player/layer proxies, four viewmodel proxies, five visual proxies,
// one agent-model proxy. Each node is prepended to canonical43476908. Slots
// keep their originals until the existing entity-update dispatcher exchanges
// them around the actual engine call. No vtable or target is installed here.
void capture_final_startup_patches(ClientRuntimeStorage&, PlayerReceiveOwner&,
                                   effects::VisualReceiveOwner&, skins::SkinsRuntimeOwner&,
                                   engine::ClientClassFactoryRuntime&);

// Actual final phase after the four user-message recipes: publish the client
// feedback fields, capture/link CTEFireBullets into4347644C, install overlay,
// ClientState and the source float decoder, then call all57 primary3 entries.
// CTEFireBullets is exchanged by incoming-tick/update, never left installed.
// The original final initializer then returns AL1 to its outer caller.
void finish_final_engine_startup(ClientRuntimeStorage&, engine::HookRuntimeOwner&);
// Same operation on its three canonical views, also usable by isolated source
// contracts without constructing unrelated animation/render context owners.
void finish_final_engine_startup(ClientRuntimeStorage&, engine::HookDescriptorStorage&,
                                 engine::UnloadState&);
} //namespace skeet::client

// Declarations from game_window_proxy_factory.hpp

namespace skeet::client {
class NativeStartupSources;
struct GameWindowProxyFactoryBindings {
    std::shared_ptr<engine::NativeDataImage> data;
    engine::NativeOwnerRegistryStorage& registries;
    engine::JobMemory memory;
    engine::GameWindowProxyBindings proxy;
};
inline constexpr std::uint32_t game_window_proxy_factory = 0x4347D94Fu;
engine::GameWindowProxyOwner* create_game_window_proxy_owner(GameWindowProxyFactoryBindings&);
// Requires canonical cells/registries and real loaded-build guard/interface
// bindings. Binding this factory allocates nothing and publishes no proxy.
void bind_game_window_proxy_factory(NativeStartupSources&, GameWindowProxyFactoryBindings&);
} // namespace skeet::client

// Declarations from generated_command_owners.hpp

namespace skeet {
struct MovementCommandOwner;
struct MovementCommandBindings;
namespace anti_aim {
struct CommandOwner;
}
namespace client {

// The generated-command path reads these two nullable source-owner slots at
// each entry, just as native434759B0 and43475940 do. Movement uses the same
// state and concrete engine bindings as the normal command scheduler.
struct GeneratedCommandOwnersBindings {
    MovementCommandOwner* const& movement_owner; //434759B0
    anti_aim::CommandOwner* const& angle_owner; //43475940
    const MovementCommandBindings& movement;
    const std::uint8_t& infinite_duck; //43467CEC
    std::uint32_t shot_counter_8cfc633a;
};

// Complete43328EE7: phase1, movement scheduler, grounded feedback and the
// live shot-counter publication. send_packet is frame+8's separate byte.
void process_generated_movement_command(const GeneratedCommandOwnersBindings&, CommandFrame&,
                                        void* player, Command&, std::uint8_t& send_packet);

// Complete43320B61: low-speed alternating movement followed by the actual
// AA phase4 owner. Its micro-movement does not publish previous_duck_1A4.
void process_generated_angle_command(const GeneratedCommandOwnersBindings&, CommandFrame&,
                                     void* player, Command&, std::uint8_t& send_packet);

} //namespace client
} //namespace skeet

// Declarations from global_config_initializers.hpp

namespace skeet::esp {
struct OtherEspSettings;
struct EffectsSettings;
struct ColoredModelsSettings;
}
namespace skeet::misc {
struct AntiUntrustedConfig;
struct HideFromObsConfig;
struct CustomEventsConfig;
struct FreeLookConfig;
}
namespace skeet::rage {
struct GlobalSettings;
}
namespace skeet::ui {
class SettingsMenuStorage;
}

namespace skeet::client {
class NativeStartupSources;

// Every reference is the existing descriptor used by its menu/runtime owner.
// The image-data metadata belongs to those declarations; the CRT bodies below
// only append their links and perform their own explicit value/key writes.
struct GlobalConfigInitializerBindings {
    ConfigRegistries& configuration;
    rage::GlobalSettings& rage;
    esp::OtherEspSettings& other_esp;
    esp::EffectsSettings& effects;
    esp::ColoredModelsSettings& colored_models;
    misc::AntiUntrustedConfig& anti_untrusted;
    misc::HideFromObsConfig& hide_from_obs;
    misc::CustomEventsConfig& custom_events;
    misc::FreeLookConfig& free_look;
    ui::SettingsMenuStorage& settings_menu;
};

// Thirty-six independent callbacks at operands0B8/0BC/130/134/1B8/240,
// 24C..270 and278..2C4. Binding does not invoke them or choose their order.
void bind_global_config_initializers(NativeStartupSources&, GlobalConfigInitializerBindings&);

} //namespace skeet::client

// Declarations from history_runtime_factories.hpp

namespace skeet::client {
class NativeStartupSources;

// Both factories append to the same source lifecycle/entity vectors. All
// referenced contexts and the factory binding record retain stable addresses
// through initialization and the registered owners' terminal cleanup.
struct HistoryRuntimeFactoryBindings {
    std::shared_ptr<engine::NativeDataImage> data;
    engine::OwnerRegistryBinding lifecycle; //43475B78
    EntityObserverRegistry& entities; //43475BA8
    prediction::EngineEntityUpdateBindings prediction;
    lagcomp::HistoryOwnerBindings history;
    lagcomp::HistoryOwnerRuntime*& history_active; //43475AC0
};

inline constexpr std::uint32_t prediction_entity_update_factory = 0x4347E356u;
inline constexpr std::uint32_t lagcomp_history_factory = 0x43484F66u;

//4347E356 allocates native0x18 storage and tail-calls434849D3. The returned
// source allocation owns its physical primary/entity receivers, with no
// singleton publication. The unresolved native INT1 failure edge4347E369 is
// an explicit std::bad_alloc failure in this source entry.
void* create_prediction_entity_update_owner(HistoryRuntimeFactoryBindings&);

//43484F66 allocates native0x60 storage, constructs4347E5DE on success and
// publishes owner/null only afterward. The canonical43475AC0 cell is also
// cleared by the existing433B5B46 source destructor before allocation release.
lagcomp::HistoryOwnerRuntime* create_lagcomp_history_owner(HistoryRuntimeFactoryBindings&);

// Bind concrete source bodies without invoking either constructor or choosing
// a cross-owner order. initialize_runtime_owners supplies the recovered order.
void bind_history_runtime_factories(NativeStartupSources&, HistoryRuntimeFactoryBindings&);
} //namespace skeet::client

// Declarations from history_runtime_owner.hpp

namespace skeet {
struct ShotFeedbackState;
}
namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
struct NativeOffsetGlobals;
}
namespace skeet::lagcomp {
struct HistoryRecordingBindings;
struct HistoryRecordingScratch;
}
namespace skeet::rage {
struct TargetOffsets;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
struct HistoryRuntimeFactoryBindings;
class SourceTerminalCleanupBindings;
class NativeStartupSources;
// Genuine external engine/CRT boundary inputs; the explicit overload permits
// authored standalone interface/ABI spies without replacing recovered bodies.
struct HistoryRuntimeImports {
    void* thread_pool;
    lagcomp::HistoryNumericBindings numeric;
};
HistoryRuntimeImports native_history_runtime_imports();

lagcomp::EntityNetvars native_history_entity_fields(const engine::NativeOffsetGlobals&);
// Stable complete history/prediction factory contexts, composed after genuine
// engine discovery. Production recording owns both native automatic scratch
// directions per invocation; explicit-input overloads serve differential fixtures.
// No owner is registered and no engine function is called by construction.
// All borrowed process owners must remain alive through workers and exit calls.
class HistoryRuntimeOwner {
public:
    HistoryRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                        ConfigurationRuntimeOwner&, PlayerRuntimeOwner&,
                        engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                        script::SourceScriptRuntimeOwner&);
    HistoryRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                        ConfigurationRuntimeOwner&, PlayerRuntimeOwner&,
                        engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                        script::SourceScriptRuntimeOwner&, HistoryRuntimeImports);
    HistoryRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                        ConfigurationRuntimeOwner&, PlayerRuntimeOwner&,
                        engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                        script::SourceScriptRuntimeOwner&, lagcomp::HistoryRecordingScratch&);
    HistoryRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                        ConfigurationRuntimeOwner&, PlayerRuntimeOwner&,
                        engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                        script::SourceScriptRuntimeOwner&, lagcomp::HistoryRecordingScratch&,
                        HistoryRuntimeImports);
    ~HistoryRuntimeOwner();
    HistoryRuntimeOwner(const HistoryRuntimeOwner&) = delete;
    HistoryRuntimeOwner& operator=(const HistoryRuntimeOwner&) = delete;
    HistoryRuntimeOwner(HistoryRuntimeOwner&&) = delete;
    HistoryRuntimeOwner& operator=(HistoryRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    PlayerRuntimeOwner& players() const noexcept;
    const lagcomp::EntityStateRuntime& entity_runtime() const noexcept;
    const rage::TargetOffsets& target_offsets() const noexcept;
    engine::NativeConVarReference game_type() const noexcept;
    engine::NativeConVarReference teammates_are_enemies() const noexcept;
    ShotFeedbackState& feedback_state() noexcept;
    const lagcomp::HistoryRecordingBindings& recording_bindings() const noexcept;
    HistoryRuntimeFactoryBindings& factory_bindings() noexcept;
    SourceTerminalCleanupBindings& source_terminal_cleanup() noexcept;
    // Bind only the two actual factories. Startup order and source terminal
    // profile selection remain responsibilities of NativeStartupSourceGraph.
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from hitboxes.hpp

namespace skeet::client {
// Parsed arguments of Lua client.draw_hitboxes. Only yaw/alternative set native
// option bits; omitted tick retains the simulation-time default.
struct ClientHitboxesRequest {
    std::int32_t entity_index;
    float duration;
    std::uint32_t hitbox_mask;
    std::uint32_t color;
    bool has_tick;
    std::int32_t tick;
    float yaw;
    std::int32_t alternative;
    std::uint32_t flags;
};
struct ClientHitboxOffsets {
    std::uint32_t simulation_time;
    std::uint32_t studio_wrapper;
    std::uint32_t hitbox_set;
    std::uint32_t bone_cache;
    std::uint32_t absolute_angles;
    std::uint32_t origin;
    std::uint32_t absolute_origin;
    std::uint32_t entity_flags;
    std::uint32_t field_432b1f5b;
    std::uint32_t field_0d560fac;
    std::uint32_t bone_accessor;
};
struct ClientHitboxRuntime {
    PlayerStateCache& players;
    ClientHitboxOffsets offsets;
    void* debug_overlay;
    RageTrigonometry trigonometry;
    double (*arc_tangent)(double, double); // binding for recorded __libm_sse2_atan2
    // Actual optional engine target43468F80, called by43341E47 with mask300.
    engine::MemberFunction<void, Matrix3x4*, std::uint32_t> update_attachments;
    const float& interval;
    const std::int32_t& current_tick;
    const float& current_time;
};
// Complete433B60DD. The two debug-overlay virtuals are the only draw boundary.
void draw_client_hitbox(const ClientHitboxRuntime&, const RageLocalHitbox&, const Matrix3x4&,
                        float duration, std::uint32_t color);
// Complete post-parse4334C981, including433E004F history search and4338AC35
// scatter/attachments/select transaction. Entity was resolved by the Lua entry.
// Valid studio models have at most19 hitboxes and bone indices below256.
void draw_client_hitboxes(const ClientHitboxRuntime&, void* entity, const ClientHitboxesRequest&);
} // namespace skeet::client

// Declarations from input_commands.hpp

namespace skeet::client {
// Complete43331EFC: CInput+F0 owns150 commands, indexed by unsigned remainder.
Command& input_command_slot(void* input, std::uint32_t command_number) noexcept;
const Command& input_command_slot(const void* input, std::uint32_t command_number) noexcept;
// Complete433B9184 exact-number check following the same actual ring lookup.
const Command* find_input_command(const void* input, std::int32_t command_number) noexcept;
} // namespace skeet::client

// Declarations from local_animation_runtime_owner.hpp

namespace skeet::animation {
struct LocalAnimationBindings;
}
namespace skeet::effects {
class EffectsRuntimeOwner;
class ViewRuntime;
struct ViewDuckBindings;
}
namespace skeet::prediction {
struct PlayerSimulationBindings;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class CommandCaptureRuntimeOwner;
class LocalPlayerAnimationHooks;
class PlayerCommandQueries;
class PlayerPhysicsHooks;
class PlayerCommandHooks;
struct RestrictedPlayerCommandBindings;
struct EngineRuntimeBindings;
// Canonical local-player animation/command contexts, available before the
// prediction-history factory. All nullable owner gates borrow their actual
// cells and see later factory publications. This creates no feature owner,
// second ViewRuntime, hook installation or engine call.
class LocalAnimationRuntimeOwner final {
public:
    LocalAnimationRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                               ConfigurationRuntimeOwner&, HistoryRuntimeOwner&,
                               CommandCaptureRuntimeOwner&, effects::EffectsRuntimeOwner&);
    ~LocalAnimationRuntimeOwner();
    LocalAnimationRuntimeOwner(const LocalAnimationRuntimeOwner&) = delete;
    LocalAnimationRuntimeOwner& operator=(const LocalAnimationRuntimeOwner&) = delete;
    LocalAnimationRuntimeOwner(LocalAnimationRuntimeOwner&&) = delete;
    LocalAnimationRuntimeOwner& operator=(LocalAnimationRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    HistoryRuntimeOwner& history() const noexcept;
    const RestrictedPlayerCommandBindings& restricted() const noexcept;
    const animation::LocalAnimationBindings& helpers() const noexcept;
    LocalPlayerAnimationHooks& local_hooks() noexcept;
    PlayerCommandQueries& queries() noexcept;
    PlayerPhysicsHooks& physics() noexcept;
    PlayerCommandHooks& command_hooks() noexcept;
    const prediction::PlayerSimulationBindings& simulation() const noexcept;
    effects::ViewDuckBindings& duck() noexcept;
    effects::ViewRuntime& view() noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from misc_runtime_factories.hpp

namespace skeet::client {
class NativeStartupSources;

struct MiscLowFpsFactoryContext {
    rage::PerformanceSettings& settings;
    const rage::GlobalSettings& rage;
    misc::LowFpsBindings engine;
    script::Bridge& scripts;
    const InputBitmap& input;
    const esp::IndicatorLayout& layout;
    const std::uint32_t& life_state_offset;
    misc::LowFpsRuntime*& active; // borrowed source projection, no native global store
};
struct MiscQuickSwitchFactoryContext {
    misc::QuickSwitchConfig& settings;
    misc::QuickSwitchRuntime& runtime;
};
struct MiscGrenadeFactoryContext {
    misc::GrenadePredictionConfig& settings;
    esp::GrenadeEspConfig& visuals;
    misc::GrenadeOwnerBindings engine;
};
struct MiscNoFallDamageFactoryContext {
    misc::NoFallDamageConfig& settings;
    misc::NoFallDamageBindings engine;
};
struct MiscAutomaticWeaponsFactoryContext {
    misc::AutomaticWeaponsConfig& settings;
    misc::AutomaticWeaponsState& state;
    const misc::AutomaticWeaponsBindings& engine;
};
struct MiscKnifeFactoryContext {
    misc::KnifeConfig& settings;
    misc::KnifeCommandBindings engine;
    std::uint32_t& scheduler_flags;
};
struct MiscZeusFactoryContext {
    misc::ZeusConfig& settings;
    misc::ZeusState& state;
    const misc::ZeusBindings& engine;
};
struct MiscClanTagFactoryContext {
    misc::ClanTagConfig& settings;
    misc::ClanTagRuntime& runtime;
    const ScriptCommandEnvironment& environment;
};
struct MiscRebuyFactoryContext {
    misc::RebuyFixOwnerBindings engine;
};

// These contexts borrow the same feature state/settings used by command,
// entity, HUD and menu consumers. Keep every reference and this record alive
// through owner teardown. Binding supplies bodies, not a factory order.
struct MiscRuntimeFactoryBindings {
    ConfigRegistries& configuration;
    engine::OwnerRegistryBinding lifecycle; //43475B78
    engine::OwnerRegistryBinding hud; //43475A9C
    engine::OwnerRegistryBinding updates; //43475988
    engine::OwnerRegistryBinding frames; //43475AD8
    engine::OwnerRegistryBinding messages; //43475B94
    CommandPhaseRegistry& commands;
    CommandEmptyTickRegistry& empty_ticks;
    EntityObserverRegistry& entities;
    network::ExploitRechargeObserverRegistry& recharge;
    prediction::ObserverRegistry& prediction;
    MiscLowFpsFactoryContext low_fps;
    MiscQuickSwitchFactoryContext quick_switch;
    MiscGrenadeFactoryContext grenades;
    MiscNoFallDamageFactoryContext no_fall_damage;
    MiscAutomaticWeaponsFactoryContext automatic_weapons;
    MiscKnifeFactoryContext knife;
    MiscZeusFactoryContext zeus;
    MiscClanTagFactoryContext clan_tag;
    MiscRebuyFactoryContext rebuy;
};

//434903AD,43481DC0,43481EDA,4348401C,4348E51F,43483FF8,43490389,
//4348DA01,4347E2E3. Native INT1 allocation-failure transfers remain unrecovered;
// the eight affected source factories use an explicit bad_alloc failure policy.
// Grenade preserves its proved allocation-failure null publication instead.
void bind_misc_runtime_factories(NativeStartupSources&, MiscRuntimeFactoryBindings&);
} //namespace skeet::client

// Declarations from misc_runtime_owner.hpp

namespace skeet::engine {
class SceneQueryRuntimeOwner;
class HookDescriptorStorage;
}
namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::rage {
class RuntimeOwner;
}
namespace skeet::misc {
class Runtime;
struct AutoAcceptPatchState;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
struct EngineRuntimeBindings;

// Explicit supplied frame views for authored standalone boundary fixtures.
// Production uses the actual RAGE owner and never retains these frame views.
struct StandaloneMiscKnifeViews {
    const rage::TargetBindings& targeting;
    const rage::RecordCandidateBindings& candidates;
    const lagcomp::RecordSelectionContext& records;
    const lagcomp::ExtrapolatedRecordBindings& extrapolation;
};
struct MiscKnifeFrameSource {
    MiscKnifeFrameSource(rage::RuntimeOwner& owner) noexcept : source(&owner) {}
    MiscKnifeFrameSource(StandaloneMiscKnifeViews views) noexcept : source(views) {}
    std::variant<rage::RuntimeOwner*, StandaloneMiscKnifeViews> source;
};
// Shared physical contexts already used by combat/history, movement and HUD.
// RAGE supplies invocation-local knife views only after native entry gates.
struct MiscRuntimeSharedBindings {
    script::Bridge& scripts;
    engine::SceneQueryRuntimeOwner& scene;
    animation::RuntimeOwner& animation;
    MiscKnifeFrameSource knife;
    const MovementExtrapolationLiveBindings& live_movement;
    const CommandBudgetBindings& budget;
    const esp::Projection& projection;
    ui::RendererBackend& renderer;
    const esp::GrenadeWarningTexture& explosive_texture;
    const esp::GrenadeWarningTexture& fire_texture;
    const effects::ProjectileRibbonBindings& ribbons;
    const esp::IndicatorLayout& indicators;
    MovementReleaseBinding release_movement;
    MovementFeedbackBinding movement_feedback;
};

struct MiscRuntimeImports {
    misc::AutomaticWeaponsRandomInteger random_integer;
    misc::LowFpsClock performance_clock;
    esp::GrenadeDamageMath damage_math;
    double (*arc_tangent2)(double, double);
};
MiscRuntimeImports native_misc_runtime_imports(const EngineRuntimeBindings&);

// One source owner for the nine misc factory contexts, shared runtime/channel,
// feature state and concrete engine bindings. Construction registers nothing;
// bind_initializers only supplies the existing native factory bodies. Their
// real startup order still allocates/registers the nine complete owners.
// Storage, engine/configuration/descriptor and shared-context owners must
// outlive this object, and this object must outlive the lifecycle owners.
class MiscRuntimeOwner {
public:
    MiscRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                     ConfigurationRuntimeOwner&, engine::HookDescriptorStorage&,
                     MiscRuntimeSharedBindings);
    MiscRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                     ConfigurationRuntimeOwner&, engine::HookDescriptorStorage&,
                     MiscRuntimeSharedBindings, MiscRuntimeImports);
    ~MiscRuntimeOwner();
    MiscRuntimeOwner(const MiscRuntimeOwner&) = delete;
    MiscRuntimeOwner& operator=(const MiscRuntimeOwner&) = delete;
    MiscRuntimeOwner(MiscRuntimeOwner&&) = delete;
    MiscRuntimeOwner& operator=(MiscRuntimeOwner&&) = delete;

    ClientRuntimeStorage& storage() const noexcept;
    misc::Runtime& runtime() noexcept;
    ChannelHookState& channel() noexcept;
    misc::AutoAcceptPatchState& auto_accept_patch() noexcept;
    misc::QuickSwitchRuntime& quick_switch() noexcept;
    misc::LowFpsRuntime* low_fps() const noexcept;
    MiscRuntimeFactoryBindings& factory_bindings() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    static ClientRuntimeStorage& require_available(ClientRuntimeStorage&);
    struct Data;
    std::unique_ptr<Data> data_;
};
} // namespace skeet::client

// Declarations from movement_hud_factories.hpp

namespace skeet::client {
class NativeStartupSources;

// Borrowed contexts for the four proven movement/HUD factory identities.
// Registry records, command views, settings and render bindings remain at
// stable addresses through destruction of their registered whole owners.
struct MovementHudFactoryBindings {
    ConfigRegistries& configuration;
    MovementOwnerRegistries movement_registries;
    MovementOwnerBindings movement;
    BlockbotConfig& blockbot_settings;
    BlockbotState& blockbot_state;
    const BlockbotBindings& blockbot;
    const esp::Projection& blockbot_projection;
    const std::uint32_t& blockbot_life_state_offset;
    hud::ScreenOverlayOwnerRegistries overlay_registries;
    ScreenOverlayCommandOwner& overlay_command;
    const hud::ScreenOverlayHudBinding& overlay_rendering;
    hud::PenetrationReticleOwnerRegistries reticle_registries;
    PenetrationReticleCommandOwner& reticle_command;
    hud::PenetrationReticleEnvironment& reticle_environment;
    void* const& engine_trace; //43468304, captured by the reticle constructor
    const hud::PenetrationHudBinding& reticle_rendering;
    esp::ScreenOverlayConfig& screen_settings;
};

//434856D6 publishes movement's canonical434759B0 slot, including null.
//4348DA54/4349095E/43481DE4 have proven nonnull constructor tails and no
//published owner globals. Their unrecovered INT1 allocation-failure paths
//explicitly throw bad_alloc; this does not claim those protected tails.
void bind_movement_hud_factories(NativeStartupSources&, MovementHudFactoryBindings&);
} //namespace skeet::client

// Declarations from movement_hud_runtime_owner.hpp

namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class CommandCaptureRuntimeOwner;
class HistoryRuntimeOwner;
class VisualRuntimeOwner;
struct EngineRuntimeBindings;
struct CommandBudgetBindings;

// Explicit borrowed source views for standalone authored fixtures. Production
// obtains these from the canonical owners in the first constructor below.
// No entity, player, command, camera or clock snapshot is stored here.
struct StandaloneMovementHudViews {
    script::Bridge& scripts;
    const CommandBudgetBindings& budget;
    const rage::TargetOffsets& target_offsets;
    const esp::Projection& projection;
    const esp::IndicatorLayout& indicators;
    const Point& screen_size;
};
struct MovementHudRuntimeImports {
    const RageTrigonometry& trigonometry;
    lagcomp::RotationMath rotation;
    float (*exponential)(float);
    float (*tangent)(float);
};
MovementHudRuntimeImports native_movement_hud_runtime_imports();

// Stable assembly for434856D6/4348DA54/4349095E/43481DE4. Construction does
// not allocate/register their owners or publish434759B0. Their real factories
// initialize the same state/settings later in the startup schedule.
class MovementHudRuntimeOwner {
public:
    MovementHudRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                            ConfigurationRuntimeOwner&, engine::SceneQueryRuntimeOwner&,
                            VisualRuntimeOwner&, script::SourceScriptRuntimeOwner&,
                            CommandCaptureRuntimeOwner&, HistoryRuntimeOwner&);
    MovementHudRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                            ConfigurationRuntimeOwner&, engine::SceneQueryRuntimeOwner&,
                            StandaloneMovementHudViews, MovementHudRuntimeImports);
    ~MovementHudRuntimeOwner();
    MovementHudRuntimeOwner(const MovementHudRuntimeOwner&) = delete;
    MovementHudRuntimeOwner& operator=(const MovementHudRuntimeOwner&) = delete;
    MovementHudRuntimeOwner(MovementHudRuntimeOwner&&) = delete;
    MovementHudRuntimeOwner& operator=(MovementHudRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    const MovementEnvironmentBindings& movement_environment() const noexcept;
    const MovementCaptureInput& movement_capture() const noexcept;
    const MovementExtrapolationLiveBindings& live_movement() const noexcept;
    MovementCommandPhaseOwner& movement_command() noexcept;
    MovementReleaseRuntime& release_movement() noexcept;
    MovementHudFactoryBindings& factory_bindings() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from native_startup.hpp

namespace skeet::anti_aim {
struct StartupInitializerBindings;
}
namespace skeet::engine {
class PreservedInitialization;
class MapGameSystemRuntime;
class StartupVectorCleanupBindings;
class StartupContainerCleanup;
struct FirstStartupServiceBindings;
}
namespace skeet::script {
struct StartupInitializerBindings;
struct UiTypeInitialization;
}
namespace skeet::animation {
class FixAnimationBackupCleanupContext;
}
namespace skeet::rage {
struct GroupInitializerBindings;
struct FeedbackOwnerBindings;
struct PredictionMaskFactoryBindings;
}
namespace skeet::effects {
struct FreeLookInitialization;
}
namespace skeet::misc {
struct ViewmodelLagFactoryContext;
}
namespace skeet::hud {
struct SimpleHudOwnersContext;
}
namespace skeet::esp {
class FlagTableInitialization;
}
namespace skeet::events {
class EventNameStartupBindings;
}

namespace skeet::client {
struct ExitRegistrationContext;
struct ConfigInitializerBindings;
struct GlobalConfigInitializerBindings;
struct PredictionHistoryInitialization;
struct EntityCommandFactoryBindings;
struct TargetMaskFactoryBindings;
struct SharedEspInitialization;
struct BufferedShotFactoryBindings;
struct RuntimeOwnerFactoryBindings;
struct MiscRuntimeFactoryBindings;
struct CombatRuntimeFactoryBindings;
struct SkinRuntimeFactoryBindings;
struct MovementHudFactoryBindings;
struct HistoryRuntimeFactoryBindings;
struct WorldRuntimeFactoryBindings;
struct GameWindowProxyFactoryBindings;
class SourceTerminalCleanupBindings;
class HookDescriptorCleanupContext;
struct StaticCrtInitialization;
struct NativeStartupBitMasks {
    std::span<std::uint32_t, 32 * 33> matrix; //43477BE0
    std::span<std::uint32_t, 32> prefixes; //43477B60
};
void initialize_native_startup_bit_masks(NativeStartupBitMasks) noexcept; //433FB71E
struct NativeInitializerCall {
    std::uint32_t native_address;
    void* owner;
    void (*invoke)(void*);
};
struct NativePreflightCall {
    std::uint32_t native_address;
    void* owner;
    std::uint32_t (*invoke)(void*);
};

// The graph binds the same configuration/hotkey and mask storage consumed by
// the rest of the source. A source initializer entry names its actual body and
// borrowed owner; it is never a captured native-address function-pointer cast.
// Keep this graph and every binding context at stable addresses until all
// owner callbacks and registered terminal cleanup calls have finished.
struct NativeStartupSourceGraph {
    ConfigRegistries& configuration;
    RegisteredHotkey& menu_key;
    MovementGlobalConfig& movement;
    skins::Settings& skins;
    anti_aim::StartupInitializerBindings& anti_aim;
    ExitRegistrationContext& exits;
    ConfigInitializerBindings& static_configuration;
    engine::NativeDataImage& data;
    // Required exact43484F06 source binding. An unfinished final operation
    // must be reported by an absent binding, never a default empty callback.
    NativeInitializerCall final_initializer;
    engine::FirstStartupServiceBindings& first_startup_services;
    HookDescriptorCleanupContext& hook_descriptors;
    script::StartupInitializerBindings& script_extensions;
    animation::FixAnimationBackupCleanupContext& animation_backups;
    rage::GroupInitializerBindings& rage_groups;
    PredictionHistoryInitialization& prediction_history;
    EntityCommandFactoryBindings& entity_commands;
    effects::FreeLookInitialization& free_look;
    TargetMaskFactoryBindings& target_mask;
    SharedEspInitialization& shared_esp;
    rage::FeedbackOwnerBindings& shot_feedback;
    BufferedShotFactoryBindings& buffered_shots;
    misc::ViewmodelLagFactoryContext& viewmodel_lag;
    GlobalConfigInitializerBindings& global_configuration;
    hud::SimpleHudOwnersContext& simple_hud;
    esp::FlagTableInitialization& esp_flags;
    script::UiTypeInitialization& ui_types;
    RuntimeOwnerFactoryBindings& runtime_owners;
    rage::PredictionMaskFactoryBindings& prediction_masks;
    engine::MapGameSystemRuntime& map_game_system;
    MiscRuntimeFactoryBindings& misc_owners;
    CombatRuntimeFactoryBindings& combat_owners;
    SkinRuntimeFactoryBindings& skin_owners;
    MovementHudFactoryBindings& movement_hud;
    HistoryRuntimeFactoryBindings& history_owners;
    WorldRuntimeFactoryBindings& world_owners;
    GameWindowProxyFactoryBindings& game_window_proxy;
    StaticCrtInitialization& static_crt;
    events::EventNameStartupBindings& event_names;
    // These exact native cells have separate ownership from the semantic
    // Bridge/UiBindings/RecordPool STL allocations, which retain their own
    // source cleanup. This context does not claim to release those owners.
    engine::StartupVectorCleanupBindings& native_vector_cleanup;
    // Retained native container headers and the actual source renderer
    // listener share this terminal context; separate STL owners retain their
    // own destructors and are not released through the native cell views.
    engine::StartupContainerCleanup& native_container_cleanup;
    // The complete source graph releases the containers used by its actual
    // producers. Retained header cleanup cannot substitute for this profile.
    SourceTerminalCleanupBindings& source_terminal_cleanup;
};
class UnmappedNativeInitializer final : public std::runtime_error {
public:
    UnmappedNativeInitializer(std::uint32_t operand, std::uint32_t native_address, bool preflight);
    const std::uint32_t operand, native_address;
    const bool preflight;
};

class NativeStartupSources {
public:
    // Low-level source callback table. This constructor runs no initializer
    // and does not claim that the complete client graph has been composed.
    // The runtime preflights the fixed build plan before any startup effects.
    NativeStartupSources(engine::NativeDataImage&, NativeInitializerCall final_initializer,
                         std::span<const NativeInitializerCall> initializers = {},
                         std::span<const NativePreflightCall> preflight = {});
    NativeStartupSources(NativeStartupSourceGraph& graph,
                         std::span<const NativeInitializerCall> initializers,
                         std::span<const NativePreflightCall> preflight);
    NativeStartupSources(const NativeStartupSources&) = delete;
    NativeStartupSources& operator=(const NativeStartupSources&) = delete;
    void bind_initializer(NativeInitializerCall);
    void bind_preflight(NativePreflightCall);
    void require_initializer(std::uint32_t operand, std::uint32_t native_address) const;
    void require_preflight(std::uint32_t operand, std::uint32_t native_address) const;
    void initialize(std::uint32_t operand, std::uint32_t native_address) const;
    std::uint32_t check(std::uint32_t operand, std::uint32_t native_address) const;
    engine::NativeDataImage& data() const noexcept {
        return data_;
    }

private:
    engine::NativeDataImage& data_;
    NativeInitializerCall final_initializer_;
    std::unordered_map<std::uint32_t, NativeInitializerCall> initializers_;
    std::unordered_map<std::uint32_t, NativePreflightCall> preflight_;
};

#if defined(_M_IX86) || defined(__i386__)
using StartupTerminateThread = std::int32_t(__stdcall*)(void*, std::int32_t);
using StartupExitProcess = void(__stdcall*)(std::uint32_t);
#else
using StartupTerminateThread = std::int32_t (*)(void*, std::int32_t);
using StartupExitProcess = void (*)(std::uint32_t);
#endif
struct NativeStartupImports {
    void (*clear_unhandled_exception_filter)(); //SetUnhandledExceptionFilter(NULL)
    StartupTerminateThread terminate_thread; //ntdll!ZwTerminateThread
    StartupExitProcess exit_process; //ntdll!RtlExitUserProcess
};
NativeStartupImports native_startup_runtime_imports();
struct NativeStartupState {
    std::uintptr_t& initialization_argument; //43475AE8, third native entry argument
    void (*&fatal_handler)(); //43475BC4, native433FB70F
};

// Source scheduler for this build's explicit crt_initialization_plan. It
// preflights the whole plan before changing data or invoking an import/body.
// Observed identifiers select source bindings;43493903 is not executed or
// claimed recovered. The separately required final binding owns its actual
// remaining work; no successful bootstrap is manufactured here.
class NativeStartupRuntime {
public:
    NativeStartupRuntime(NativeStartupSources& sources, NativeStartupImports imports);
    std::uint32_t initialize(std::uintptr_t third_argument);
    void fatal(); //433FB70F -> RtlExitUserProcess(-10)
private:
    NativeStartupState state_;
    NativeStartupSources& sources_;
    NativeStartupImports imports_;
};
// Explicit legacy route only. Its caller supplies the real executable image
// and live operand-accessor environment, with the same canonical data owner.
// This remains separate from ordinary source data/storage construction.
class RetainedNativeStartupRuntime {
public:
    RetainedNativeStartupRuntime(NativeStartupSources&, engine::PreservedInitialization&,
                                 NativeStartupImports);
    std::uint32_t initialize(std::uintptr_t third_argument);
    void fatal();

private:
    NativeStartupState state_;
    NativeStartupSources& sources_;
    engine::PreservedInitialization& metadata_;
    NativeStartupImports imports_;
};
// Opt-in adapter for the still-protected final operation. Source composition
// does not select it automatically and requires its actual retained lifetime.
NativeInitializerCall retained_final_startup_initializer(engine::PreservedInitialization&);
// Source counterpart of the no-argument433FB70F callback. The scheduler owner
// must remain alive while this published callback can be invoked.
void native_startup_fatal();
}

// Declarations from packet_runtime_owner.hpp

namespace skeet::network {
struct PacketCommandOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class MovementHudRuntimeOwner;
class TargetRefreshRuntimeOwner;
class NativeStartupSources;
struct EngineRuntimeBindings;
struct PacketRuntimeFactoryBindings;
// Genuine external vstdlib boundaries. The explicit overload permits authored
// standalone spies; production resolves the already loaded vstdlib exports.
struct PacketRuntimeImports {
    using Seed = void(__cdecl*)(std::int32_t);
    using Integer = std::int32_t(__cdecl*)(std::int32_t, std::int32_t);
    Seed seed;
    Integer integer;
};
PacketRuntimeImports native_packet_runtime_imports(ClientRuntimeStorage&,
                                                   const EngineRuntimeBindings&);
class PacketRuntimeOwner final {
public:
    PacketRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                       ConfigurationRuntimeOwner&, HistoryRuntimeOwner&, MovementHudRuntimeOwner&,
                       TargetRefreshRuntimeOwner&);
    // Requires an already published genuine/authored4346A7CC boundary. This
    // explicit surface performs no module discovery and executes no query.
    PacketRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                       ConfigurationRuntimeOwner&, HistoryRuntimeOwner&, MovementHudRuntimeOwner&,
                       TargetRefreshRuntimeOwner&, PacketRuntimeImports);
    ~PacketRuntimeOwner();
    PacketRuntimeOwner(const PacketRuntimeOwner&) = delete;
    PacketRuntimeOwner& operator=(const PacketRuntimeOwner&) = delete;
    PacketRuntimeOwner(PacketRuntimeOwner&&) = delete;
    PacketRuntimeOwner& operator=(PacketRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    network::PacketCommandOwner& command() noexcept;
    network::FakeLagState& state() noexcept;
    PacketRuntimeFactoryBindings& factory_bindings() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from player_command_queries.hpp

namespace skeet::client {
struct PlayerCommandQueryBindings {
    const std::uint32_t& update_phase; // canonical43476904
    const std::uintptr_t& forced_query_stack; //434674E0, caller stack+8 word
    const std::uint8_t& team_query_override; //4346907C
    const std::uint8_t& update_mode; //43468E58
    const std::uintptr_t& team_query_return; //4346756C
    std::ptrdiff_t team; //8B5586F6
    void* const& engine_client; //4346A514
    std::uint8_t* const& interpolation_guard; //optional43468FA4
    std::ptrdiff_t rejection_byte; //9D920E35
    const std::int32_t* const& choked_commands; //434757DC
    const std::int32_t* const& last_outgoing; //43475B2C
    void* const& input; //4346A894, CInput+F0
    const std::uintptr_t& sent_angles_return; //43468518
    SentCommandHistoryView sent_history; //43476414 angles/43476450 records
};

class PlayerCommandQueries {
public:
    explicit PlayerCommandQueries(const PlayerCommandQueryBindings& source) noexcept
        : bindings(source) {}
    PlayerCommandQueryBindings bindings;

    // Whole433842C1, local slot78. Four argument words retain their exact
    // stack bits; phase1 suppresses the original four-word engine call.
    void dispatch_update_call(void* player, std::array<std::uint32_t, 4> arguments) const;
    // Whole4334D3EB ->4338257F, local slot88. Native entry copies incoming
    // ESI to EDX; stack_word_08 and return_address are the caller's raw words.
    std::uint32_t query_callsite(void* player, std::uint32_t caller_esi,
                                 std::uintptr_t return_address, std::uintptr_t stack_word_08) const;
    // Whole4340191D, slot113. Always clears the optional guard first; a
    // matching sentinel command returns AL1 without calling the original.
    // time_bits retains the original float stack word without conversion.
    std::uint8_t interpolate(void* player, std::uint32_t time_bits) const;
    // Whole433E97C1, slot170. The matching callsite obtains canonical sent
    // angles directly; every other caller reaches the original engine method.
    std::array<float, 3>* eye_angles(void* player, std::uintptr_t return_address) const;

    // The family installer owns installation. This binds only the four exact
    // captured native operands to their actual source ABI entry points.
    void bind_native_replacement(engine::VtableHookMetadata&, std::size_t slot,
                                 std::uint32_t native_entry);
    static void* native_replacement_entry(std::size_t slot, std::uint32_t native_entry);
};
} // namespace skeet::client

// Declarations from player_data_update.hpp

namespace skeet::engine {
struct NativeOffsetGlobals;
}
namespace skeet::client {
struct PlayerDataUpdateOffsets {
    std::ptrdiff_t duck_3309c0cf, scaled_recoil_66146802;
    std::ptrdiff_t replicated_time_d3d4230d, body_yaw_a2a8247f;
    std::ptrdiff_t velocity_36375c23, local_serialized_d070684c;
};
PlayerDataUpdateOffsets native_player_data_update_offsets(const engine::NativeOffsetGlobals&);
// Complete4333027B on its valid-player contract. It shifts the canonical
//0x16C cache block and captures the live fields before the engine data update.
void capture_serialized_player_state(NativePlayerStateLayout&, void* player,
                                     const PlayerDataUpdateOffsets&);
} //namespace skeet::client

// Declarations from player_receive.hpp

namespace skeet::client {
class PlayerStateCache;
// Eight complete entity receive callbacks. Saved engine callbacks, engine
// interfaces and encoded fields are reread at their original use sites.
class PlayerReceiveRuntime {
public:
    PlayerReceiveRuntime(engine::NativeDataImage& data, const engine::NativeOffsetStorage& offsets,
                         PlayerStateCache& players) noexcept
        : data_(data), offsets_(offsets), players_(players) {}
    engine::NativeDataImage& data() const noexcept {
        return data_;
    }
    PlayerStateCache& player_cache() const noexcept {
        return players_;
    }
    void simulation_time(const engine::RecvProxyData*, void*, void*); //433FCA81
    void tick_base(const engine::RecvProxyData*, void*, void*); //43402A54
    void velocity(const engine::RecvProxyData*, void*, void*, unsigned component);
    void eye_angle(const engine::RecvProxyData*, void*, void*, unsigned component);
    void client_animation(const engine::RecvProxyData*, void*, void*); //4338A848
private:
    engine::NativeDataImage& data_;
    const engine::NativeOffsetStorage& offsets_;
    PlayerStateCache& players_;
    void timestamped(const engine::RecvProxyData*, void*, void*, std::uint32_t, std::size_t,
                     std::size_t);
    void original(std::uint32_t, const engine::RecvProxyData*, void*, void*) const;
};
} //namespace skeet::client

// Declarations from player_receive_owner.hpp

namespace skeet::animation {
class FixRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class PlayerRuntimeOwner;
struct PlayerReceiveDescriptor {
    std::uint32_t operand, class_hash, table_hash, property_hash, native_entry, patch_address;
    engine::RecvVarProxy replacement;
};
std::span<const PlayerReceiveDescriptor, 15> player_receive_descriptors() noexcept;
// Separate capture and publication preserve startup's prepare/install split.
// Neither function installs a property replacement or mutates patch links.
void initialize_player_receive_patch_targets(engine::NativeDataImage&);
void bind_player_receive_patches(engine::NativeDataImage&, PlayerReceiveRuntime&,
                                 animation::LayerReceiveRuntime&);
// Shared runtime assembly for the fifteen player/layer receive properties.
// Keep this owner through patch restoration. It creates no second player cache
// and uses the existing FixAnimations sequence receiver and configuration.
class PlayerReceiveOwner {
public:
    PlayerReceiveOwner(ClientRuntimeStorage&, PlayerRuntimeOwner&, animation::FixRuntimeOwner&);
    PlayerReceiveOwner(const PlayerReceiveOwner&) = delete;
    PlayerReceiveOwner& operator=(const PlayerReceiveOwner&) = delete;
    ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    PlayerReceiveRuntime& players() noexcept {
        return players_;
    }
    animation::LayerReceiveRuntime& layers() noexcept {
        return layers_;
    }
    void bind_receive_patches();
    // Capture the actual VClient018 RecvProp targets and current originals.
    // All-fifteen validation is explicit source policy, not a claim about the
    // protected initializer's missing-property path. No next/head/slot writes.
    void initialize_receive_patches();

private:
    ClientRuntimeStorage& storage_;
    PlayerReceiveRuntime players_;
    animation::LayerReceiveRuntime layers_;
};
} //namespace skeet::client

// Declarations from player_runtime_owner.hpp

namespace skeet::client {
class ClientRuntimeStorage;
// One cache, record pool and player-adjustment view shared by menu, RAGE,
// animation, Lua and history consumers. Source terminal cleanup receives the
// same cache/pool; neither construction nor destruction runs CRT callbacks.
class PlayerRuntimeOwner {
public:
    explicit PlayerRuntimeOwner(ClientRuntimeStorage&);
    ~PlayerRuntimeOwner();
    PlayerRuntimeOwner(const PlayerRuntimeOwner&) = delete;
    PlayerRuntimeOwner& operator=(const PlayerRuntimeOwner&) = delete;
    PlayerRuntimeOwner(PlayerRuntimeOwner&&) = delete;
    PlayerRuntimeOwner& operator=(PlayerRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    PlayerStateCache& cache() noexcept {
        return cache_;
    }
    players::State& adjustments() const noexcept {
        return adjustments_;
    }
    lagcomp::RecordPool& records() noexcept {
        return records_;
    }

private:
    static ClientRuntimeStorage& require_available(ClientRuntimeStorage&);
    ClientRuntimeStorage& storage_;
    lagcomp::RecordPool records_;
    PlayerStateCache cache_;
    players::State& adjustments_;
};
} //namespace skeet::client

// Declarations from player_state.hpp

namespace skeet::client {
// Four 0x24-byte native resolver selection slots begin at owner+0x10.
struct PlayerResolverSlot {
    std::uint8_t attempt;
    std::array<std::byte, 0x23> other;
};
static_assert(sizeof(PlayerResolverSlot) == 0x24);

// The original 0x1700-byte owner returned by 433B8A2B. Unknown intervals are
// retained verbatim; proven shared members have one physical home.
struct NativePlayerStateLayout {
    std::uint32_t flags; // 000
    float shared_rule_time; // 004
    float shared_yaw; // 008
    std::int32_t shared_yaw_tick; // 00C
    std::array<PlayerResolverSlot, 4> resolver_slots; // 010
    std::uint8_t resolver_status, resolver_index; // 0A0 / 0A1
    std::array<std::byte, 0xE> field_0a2;
    float probe_time, probe_stamp; // 0B0 / 0B4
    std::int32_t probe_count; // 0B8
    float reference_time; // 0BC
    RageVector reference_origin; // 0C0
    std::array<std::byte, 0x24> field_0cc;
    float environment_yaw; // 0F0
    float environment_yaw_change_time; // 0F4
    std::array<std::byte, 4> field_0f8;
    std::int32_t captured_camera_fov; // 0FC
    std::int32_t last_target_damage_tick; // 100
    std::array<std::byte, 0x2C> field_104;
    float cached_target_height; // 130
    float cached_duck_amount; // 134
    std::array<std::byte, 4> field_138;
    RageVector cached_origin; // 13C
    std::array<std::byte, 0x14> field_148;
    std::int32_t last_update_tick; // 15C
    std::array<std::byte, 4> field_160;
    std::int32_t captured_observer_mode; // 164
    std::array<std::byte, 0x18> field_168;
    std::uint32_t capture_field_180; // 180
    std::array<std::byte, 0x28> field_184;
    float reference_yaw; // 1AC
    std::array<std::byte, 0x38> field_1b0;
    esp::PlayerGeometry geometry; // 1E8
    std::uint32_t cached_esp_flags, cached_esp_frame; // 228 / 22C
    std::uint16_t cached_definition; // 230
    std::array<std::byte, 6> field_232;
    std::uint32_t field_238; // 238
    std::array<std::byte, 0x2E0> field_23c;
    float field_51c; // 51C
    std::array<std::byte, 4> field_520;
    float target_prediction_524; // 524
    std::array<std::byte, 0xAC> field_528;
    std::uint32_t history_first, history_count; // 5D4 / 5D8
    // 32-bit ABI projection of lagcomp::RecordRing, not another ring owner.
    std::array<std::uint32_t, 32> native_history_links; // 5DC
    std::array<std::byte, 4> field_65c;
    lagcomp::LagRecord fallback; // 660
};
static_assert(sizeof(NativePlayerStateLayout) == 0x1700);
static_assert(offsetof(NativePlayerStateLayout, shared_rule_time) == 0x4);
static_assert(offsetof(NativePlayerStateLayout, shared_yaw) == 0x8);
static_assert(offsetof(NativePlayerStateLayout, shared_yaw_tick) == 0xC);
static_assert(offsetof(NativePlayerStateLayout, resolver_status) == 0xA0);
static_assert(offsetof(NativePlayerStateLayout, environment_yaw) == 0xF0);
static_assert(offsetof(NativePlayerStateLayout, environment_yaw_change_time) == 0xF4);
static_assert(offsetof(NativePlayerStateLayout, captured_camera_fov) == 0xFC);
static_assert(offsetof(NativePlayerStateLayout, last_target_damage_tick) == 0x100);
static_assert(offsetof(NativePlayerStateLayout, target_prediction_524) == 0x524);
static_assert(offsetof(NativePlayerStateLayout, captured_observer_mode) == 0x164);
static_assert(offsetof(NativePlayerStateLayout, cached_target_height) == 0x130);
static_assert(offsetof(NativePlayerStateLayout, cached_duck_amount) == 0x134);
static_assert(offsetof(NativePlayerStateLayout, cached_origin) == 0x13C);
static_assert(offsetof(NativePlayerStateLayout, reference_yaw) == 0x1AC);
static_assert(offsetof(NativePlayerStateLayout, geometry) == 0x1E8);
static_assert(offsetof(NativePlayerStateLayout, cached_esp_flags) == 0x228);
static_assert(offsetof(NativePlayerStateLayout, cached_definition) == 0x230);
static_assert(offsetof(NativePlayerStateLayout, field_238) == 0x238);
static_assert(offsetof(NativePlayerStateLayout, field_51c) == 0x51C);
static_assert(offsetof(NativePlayerStateLayout, history_first) == 0x5D4);
static_assert(offsetof(NativePlayerStateLayout, native_history_links) == 0x5DC);
static_assert(offsetof(NativePlayerStateLayout, fallback) == 0x660);

struct PlayerState {
    explicit PlayerState(NativePlayerStateLayout&);
    NativePlayerStateLayout& native;
    esp::PlayerGeometry& geometry;
    float& cached_target_height;
    std::uint16_t& cached_definition;
    lagcomp::LagRecord& fallback;
    // Reuse the existing portable ring, once per player. On x86 its pointers
    // publish directly into native_history_links; host-width links stay here.
    lagcomp::RecordRing history;
    resolver_runtime::EnvironmentOwner environment() const noexcept;
    void environment(const resolver_runtime::EnvironmentOwner&) noexcept;
    lagcomp::PlayerCaptureState capture_state() const noexcept;
    lagcomp::HistoryRecordingState recording_state() const;
    void recording_state(const lagcomp::HistoryRecordingState&);
    void publish_history() noexcept;
};

class PlayerStateCache {
    std::array<NativePlayerStateLayout*, 64> standalone_addresses_{};

public:
    // Only actual RtlAllocateHeap/RtlFreeHeap exports are boundaries. The
    // alignment header, zero-initialization and replacement are recovered code.
    explicit PlayerStateCache(engine::JobMemory memory = engine::native_job_memory());
    // Production source storage is shared with the canonical native-layout
    // terminal views. This constructor accepts only an unowned empty cache.
    PlayerStateCache(engine::JobMemory, std::array<NativePlayerStateLayout*, 64>&);
    ~PlayerStateCache();
    PlayerStateCache(const PlayerStateCache&) = delete;
    PlayerStateCache& operator=(const PlayerStateCache&) = delete;
    PlayerState* get(std::uint32_t entity_index); // complete 433B8A2B, nullable
    PlayerState* find(std::uint32_t entity_index);
    //433AFCE8: drain live rings ascending, release their shared record pool,
    //then free cache owners descending and null each slot after its free.
    void finalize_history(lagcomp::RecordPool&);
    esp::PlayerGeometry& geometry(std::uint32_t entity_index) {
        return get(entity_index)->geometry;
    }
    // Native global 434797BC: index aliases use the low six bits. Public
    // storage also preserves native reentrant replacement during heap calls.
    std::array<NativePlayerStateLayout*, 64>& addresses;

private:
    engine::JobMemory memory_;
    std::array<std::optional<PlayerState>, 64> views_;
    PlayerState* view(std::uint32_t slot);
    NativePlayerStateLayout* allocate(); // 43387448 + inline constructor
    void free(void* object); // complete 433837D0
};
} // namespace skeet::client

// Declarations from prediction_history_lifecycle.hpp

namespace skeet::client {
class NativeStartupSources;
struct PredictionHistoryRegistries {
    engine::OwnerRegistryBinding primary; //43475B78
    CommandPhaseRegistry& commands; //434757A0
    CommandEmptyTickRegistry& empty_ticks; //actual43475978 buffer
    prediction::ObserverRegistry& prediction; //43475AB0
    EntityObserverRegistry& entities; //43475BA8
    network::DataUpdateObserverRegistry& data_updates; //43475760
    network::ExploitRechargeObserverRegistry& recharge; //actual43475784 buffer
};
struct PredictionHistoryLifecycleBindings {
    // The same canonical state/views consumed by commands, prediction, RAGE
    // and DataUpdate. Construct storage with deferred_prediction_history_storage;
    // owner.shared and redirect references must refer to this storage.
    PredictionHistoryStorage& storage;
    PredictionHistoryOwner& owner;
    const std::int32_t* const& acknowledged_command; //live43475BA4 cell; recorded MOV at4347D094
    void* const& animation_allocator; //live43467204, genuine Free virtual5
    PredictionHistoryOwner*& active; //must alias DataUpdate image's actual434759B4 cell
};

std::size_t prediction_history_owner_size() noexcept;
// Whole4347D010. Registers primary/command/empty/prediction/entity/DataUpdate/
// recharge in that order. DataUpdate table is published only at4347D064,
// recharge at06D; primary follows at079, empty tick086, prediction08D.
// Both native buffers and22owned strings are
// initialized afterward, without a semantic-buffer substitute on failure.
void* construct_prediction_history_owner(void* storage, const PredictionHistoryRegistries&,
                                         ConfigRegistries&, PredictionHistoryLifecycleBindings);
// Whole4349081E. A failed outer allocation publishes null; success publishes
// the canonical owner only after construction. No cross-owner order is chosen.
// The publication at43490837 is the explicit mem_int3.h instruction record;
// its original five bytes are traps, not an ordinary raw MOV instruction.
void* create_prediction_history_owner(const PredictionHistoryRegistries&, ConfigRegistries&,
                                      PredictionHistoryLifecycleBindings);
// Whole433B7312 ->433AE3CF ->433F9238. Clears active before the null test,
// destroys strings in reverse, releases records then snapshot vectors/cookie,
// and finally releases the primary allocation. Registry buffers remain owned
// by their actual later finalizers; no receiver is removed here.
void destroy_prediction_history_owner(void* storage, PredictionHistoryOwner*& active);
void bind_prediction_history_owner_settings(void* storage); //4348015A, leg movement
void reset_prediction_history_owner(void* storage); //433F2757, exact sparse reset
void refresh_prediction_history_owner(void* storage); //433CF2F5 -> current primary slot5

struct PredictionHistoryInitialization {
    const PredictionHistoryRegistries& registries;
    ConfigRegistries& configuration;
    PredictionHistoryLifecycleBindings bindings;
};
// Binds only the actual4349081E initializer identity. The existing startup
// graph supplies invocation order; registering this source does not invoke it.
void bind_prediction_history_initializer(NativeStartupSources&, PredictionHistoryInitialization&);
} // namespace skeet::client

// Declarations from prediction_history_owner.hpp

namespace skeet::animation {
struct LocalPredictionPoseBindings;
}
namespace skeet::client {
struct DeferredPredictionHistoryStorageTag {
    explicit constexpr DeferredPredictionHistoryStorageTag() = default;
};
inline constexpr DeferredPredictionHistoryStorageTag deferred_prediction_history_storage{};
struct PredictionHistoryString {
    const char* begin = nullptr;
    const char* end = nullptr;
    const char* limit = nullptr;
};
std::span<const char* const, 22> prediction_history_sequence_strings() noexcept;
// Canonical storage for the single434759B4 object. The bound arrays are those
// consumed by RestrictedPlayerCommandOwner and exploit clones. Sequence
// consumers borrow the same22owned descriptors through typed accessors.
class PredictionHistoryStorage {
    struct StandaloneBuffers;
    std::unique_ptr<StandaloneBuffers> standalone_;

public:
    // Restored4347D094 reads the pointer43475BA4 into full-owner+38. Preserve
    // that live alias rather than caching its current acknowledged number.
    explicit PredictionHistoryStorage(const std::int32_t* acknowledged_command) noexcept;
    // Production4347D010 starts with zeroed storage, registers all seven
    // interfaces, then binds its two genuine allocations and owned strings.
    // This route never constructs or falls back to standalone buffers.
    explicit PredictionHistoryStorage(DeferredPredictionHistoryStorageTag) noexcept;
    // Only the standalone backing is automatic. Production native buffers
    // and strings belong to the actual primary lifecycle destructor.
    ~PredictionHistoryStorage();
    PredictionHistoryStorage(const PredictionHistoryStorage&) = delete;
    PredictionHistoryStorage& operator=(const PredictionHistoryStorage&) = delete;
    RestrictedPlayerCommandOwner& shared() noexcept {
        return view_;
    }
    bool has_standalone_backing() const noexcept {
        return bool(standalone_);
    }
    void initialize_header(const std::int32_t*) noexcept;
    void bind_snapshots(network::ExploitCommandSnapshot*, std::size_t count) noexcept;
    void bind_prediction_records(RestrictedCommandPrediction0x4C*, std::size_t count) noexcept;
    void bind_sequence_string(std::size_t index, PredictionHistoryString) noexcept;
    const std::int32_t* field_34 = nullptr; // canonical leg_movement payload
    const std::int32_t* acknowledged_command = nullptr;
    std::span<network::ExploitCommandSnapshot> snapshots;
    std::span<RestrictedCommandPrediction0x4C> prediction_records;
    std::array<PredictionHistoryString, 22> sequence_strings{};
    std::int32_t redirect_from_44 = 0, redirect_to_48 = 0;
    std::array<std::uint32_t, 150> command_tags{}, landing_tags{};

private:
    RestrictedPlayerCommandOwner view_{};
};

struct PredictionHistoryBindings {
    const RestrictedPlayerCommandBindings& command;
    const animation::LocalPredictionPoseBindings& pose;
    const std::int32_t* const& last_outgoing; // live43475B2C pointer cell
    std::ptrdiff_t rejection_byte; // decoded9D920E35
    std::ptrdiff_t restored_angles; // decodedC865F10E
    std::ptrdiff_t absolute_angles; // decoded3B1F6ECC
    lagcomp::SetAbsoluteVector const& set_absolute_angles; // optional434688C8
    engine::MemberFunction<void, std::int32_t> const& invalidate_angles; // optional43468C10
};
// All six secondary interfaces produced by4347D010 refer to this same canonical
// owner. The redirect words are full-owner+44/+48, not the command subobject.
// Snapshot and prediction arrays are borrowed from shared, never duplicated.
struct PredictionHistoryOwner {
    RestrictedPlayerCommandOwner& shared;
    std::int32_t& redirect_from_44;
    std::int32_t& redirect_to_48;
    const PredictionHistoryBindings& bindings;
    network::NativeDataUpdateReceiver data_update_receiver{};
    // Non-owning source linkage, established by the actual lifecycle before
    // registration. This is the same canonical storage, not another history.
    PredictionHistoryStorage* canonical_storage = nullptr;
    network::NativeExploitRechargeReceiver recharge_receiver{};
    NativeCommandEmptyTickReceiver empty_tick_receiver{};
    prediction::NativePredictionObserverReceiver prediction_receiver{};
};
// Nullable native owner-slot consumers borrow the actual shared view. No
// independent RestrictedPlayerCommandOwner* cell is maintained.
RestrictedPlayerCommandOwner* prediction_history_shared(PredictionHistoryOwner*) noexcept;
const RestrictedPlayerCommandOwner*
prediction_history_shared(const PredictionHistoryOwner*) noexcept;
network::ExploitCommandSnapshot* prediction_history_snapshots(PredictionHistoryOwner*) noexcept;
// A nonnull published production owner has its lifecycle's storage linkage.
PredictionHistoryStorage& prediction_history_storage(PredictionHistoryOwner&) noexcept;
const PredictionHistoryStorage& prediction_history_storage(const PredictionHistoryOwner&) noexcept;
const char* prediction_history_sequence_string(const PredictionHistoryOwner&,
                                               std::size_t index) noexcept;

// Complete433CAE9B and4336B342. Snapshot indexing is unsigned modulo150.
bool restore_prediction_snapshot(PredictionHistoryOwner&, void* player, std::int32_t number);
bool copy_prediction_snapshot(PredictionHistoryOwner&, std::int32_t from, std::int32_t to) noexcept;
// Commandtable4346FF8C phases2,6,7 respectively.
void prepare_prediction_history(PredictionHistoryOwner&, void* player, const Command&);
void finish_prediction_history(PredictionHistoryOwner&, CommandFrame&, void* player, Command&,
                               std::uint8_t& send_packet);
void complete_prediction_history(PredictionHistoryOwner&, const CommandFrame&, void* player,
                                 const Command&, std::uint8_t send_packet);
// Complete4337D962, empty-ticktable43470470 slot0. Remaining native arguments
// are unused. A valid player is a native precondition while either mode is on.
void apply_empty_prediction_history(PredictionHistoryOwner&, void* player);
// Complete43397E0D, table43471DD4 slot1 in the separate43475BA8 registry.
void observe_prediction_history(PredictionHistoryOwner&, void* player);
// Complete4332C3C9/4340B612: the canonical0x4C before/after prediction capture.
void begin_prediction_history(PredictionHistoryOwner&, void* player, const Command&);
void capture_prediction_history(PredictionHistoryOwner&, void* player, const Command&);
// Complete433B6972, table434723B8 slot1 in43475784/88. Its slot2 is a real no-op.
void clone_previous_prediction_snapshot(PredictionHistoryOwner&, std::int32_t number) noexcept;
// Complete433F4628, table43470720 slot1 in43475760. A nonzero update
// requires a valid player. Repairs only the canonical cached layer words.
void finish_prediction_data_update(PredictionHistoryOwner&, void* player, std::int32_t update);
} // namespace skeet::client

// Declarations from prediction_history_runtime_owner.hpp

namespace skeet::animation {
class RuntimeOwner;
struct LocalPredictionPoseBindings;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class LocalAnimationRuntimeOwner;
class HistoryRuntimeOwner;
class MovementHudRuntimeOwner;
class CommandCaptureRuntimeOwner;
class PredictionHistoryStorage;
struct PredictionHistoryOwner;
struct PredictionHistoryInitialization;
struct EngineRuntimeBindings;
class NativeStartupSources;
// Stable source assembly for4349081E. Construction borrows the real local
// restricted-command, movement, history, animation and capture owners, without
// reading a player, command-ring pointer or scratch byte. Only the factory
// allocates/registers the canonical history arrays and22sequence strings.
class PredictionHistoryRuntimeOwner final {
public:
    PredictionHistoryRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                                  ConfigurationRuntimeOwner&, LocalAnimationRuntimeOwner&,
                                  HistoryRuntimeOwner&, MovementHudRuntimeOwner&,
                                  animation::RuntimeOwner&, CommandCaptureRuntimeOwner&);
    ~PredictionHistoryRuntimeOwner();
    PredictionHistoryRuntimeOwner(const PredictionHistoryRuntimeOwner&) = delete;
    PredictionHistoryRuntimeOwner& operator=(const PredictionHistoryRuntimeOwner&) = delete;
    PredictionHistoryRuntimeOwner(PredictionHistoryRuntimeOwner&&) = delete;
    PredictionHistoryRuntimeOwner& operator=(PredictionHistoryRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    PredictionHistoryStorage& history_storage() noexcept;
    PredictionHistoryOwner& command() noexcept;
    const animation::LocalPredictionPoseBindings& pose() const noexcept;
    PredictionHistoryInitialization& factory_bindings() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from rejected_player_update.hpp

namespace skeet::client {

enum class SharedEspPacketCipher {
    legacy_xor,
    rc4_drop128
};

// Semantic source owner of 434758D0. The four lifetime functions are actual
// engine message constructors/destructors discovered by 43485EF1..43485F5C.
// No recovered payload entrypoint is called through these bindings.
struct RejectedPlayerUpdateOwner {
    std::array<std::uint8_t, 16> packet_key{}; // native +24..33
    std::uint32_t mixing_key = 0xc8eac742u; // +34
    std::uint16_t marker = 0x2424; // +74
    float timer_84 = 0; // +84, empty-tick timer owner
    engine::MemberFunction<void> construct_message = nullptr; // +88
    engine::MemberFunction<void> destroy_message = nullptr; // +8C
    engine::MemberFunction<void> construct_receiver = nullptr; // +90, presence gate
    engine::MemberFunction<void> destroy_receiver = nullptr; // +94, presence gate
    std::int32_t last_player = 1; // +98
    float last_update = 0; // +9C
    void* engine_client = nullptr; // +A0
    const void* send_gate_a = nullptr; // +A4, terminal ConVar integer
    const void* send_gate_b = nullptr; // +A8, terminal ConVar integer
    const void* game_type = nullptr; // +AC, terminal ConVar integer
    alignas(8) std::array<std::byte, 0x200> message{}; // +B8
};

struct RejectedPlayerUpdateOffsets {
    std::ptrdiff_t rejected_d070684c, rejected_9d920e35;
    std::ptrdiff_t origin_a9ee6062;
    std::ptrdiff_t rule_d720aea6, rule_time_9f8e7f71;
};
struct RejectedPlayerUpdateContext {
    RejectedPlayerUpdateOwner* const& owner;
    const esp::SharedEspConfig& settings;
    InputBitmap input;
    PlayerStateCache& cache;
    const players::State& players;
    rage::TargetRelationshipBindings targeting;
    void* const& entity_list; // 4346A890
    void** const& game_rules; // 4346A3A4, nullable holder
    const std::int32_t* const& blocked; // *434757DC, required live pointer
    const std::int32_t& maximum_clients; // *43469300
    const float& current_time; // *4346A334
    const float& tick_interval; // *4346A338, history conversion
    const float& engine_tick_interval; // 43467B04 +20, wire timestamp
    SharedEspPacketCipher& packet_cipher; // fullowner+78 dispatch target
    RejectedPlayerUpdateOffsets offsets;
};

// Concrete children 433D7328 and 43400CA0. The live dispatch is switched to
// rc4_drop128 during an update and restored to legacy_xor afterward.
void transform_shared_esp_packet(SharedEspPacketCipher, const RejectedPlayerUpdateOwner&,
                                 std::span<std::byte> packet) noexcept;

// Complete 43384FD6..43385794. The rejected CreateMove path invokes it once
// per three commands, passing the local player in ECX. This sends one shared
// ESP metadata message when the native gates and player scan succeed.
void update_rejected_player(void* player, const RejectedPlayerUpdateContext&);

// Complete BA8 table4346F2D0 slot2=433654AA / slot3=43382555.
// Native ECX is fullowner+18. These switch the same dispatch target used by
// the sender; slot3 consumes no player/delta data and decodes no messages.
void enter_shared_esp_update(const RejectedPlayerUpdateContext&) noexcept;
void leave_shared_esp_update(const RejectedPlayerUpdateContext&) noexcept;

} // namespace skeet::client

// Declarations from restricted_player_command.hpp

namespace skeet::prediction {
struct ObserverRegistry;
}

namespace skeet::client {

struct RestrictedCommandPrediction0x4C {
    std::int32_t command_number;
    std::uint32_t field_04;
    std::int32_t tick_base;
    float recoil_pitch;
    std::uint32_t field_10;
    std::array<float, 3> origin;
    std::uint32_t flags;
    std::uint8_t field_24;
    std::array<std::byte, 3> padding_25;
    float field_fb7e3404, field_3309c0cf;
    std::array<float, 3> velocity;
    float field_ae838771;
    std::uint32_t field_d4248210, field_a3abbb09;
    std::uint8_t field_7501cb91, field_7090d8dd, field_cacaeecc, field_a15c3105;
};
static_assert(sizeof(RestrictedCommandPrediction0x4C) == 0x4C);
static_assert(offsetof(RestrictedCommandPrediction0x4C, recoil_pitch) == 0xC);
static_assert(offsetof(RestrictedCommandPrediction0x4C, origin) == 0x14);
static_assert(offsetof(RestrictedCommandPrediction0x4C, velocity) == 0x30);

// Borrowed views of the single owner behind434759B4. The producer and its
// native storage remain shared with exploit cloning and RAGE history reads.
struct RestrictedPlayerCommandOwner {
    const std::int32_t* last_processed_command; // owner+38
    network::ExploitCommandSnapshot* snapshots; // owner+3C,150 entries
    RestrictedCommandPrediction0x4C* prediction_records; // owner+40,150 entries
    const std::uint32_t* command_tags; // owner+1AC,150 entries
    const std::uint32_t* landing_tags; // owner+404,150 entries
    const char* moving_modifier; // owner+4C
    const char* crouching_modifier; // owner+5C
    const char* layer_modifier; // owner+7C
    const char* idle_activity; // owner+CC
    const char* idle_modifier; // owner+DC
    const char* jump_activity; // owner+EC
};

struct RestrictedPlayerCommandOffsets {
    lagcomp::EntityNetvars animation;
    std::ptrdiff_t field_66146802, field_00a5bd68, field_52899df3;
    std::ptrdiff_t field_1bae1168, field_f58e6118, weapon_handle, tick_base;
    std::ptrdiff_t field_36375c23, field_2a671b42, field_3309c0cf;
    std::ptrdiff_t field_e2fbe695, field_ac7f2913, field_7090d8dd;
    std::ptrdiff_t field_cacaeecc, field_a15c3105, field_ae838771;
    std::ptrdiff_t field_d4248210, field_a3abbb09, field_7501cb91;
    std::ptrdiff_t field_7a2d60c1, field_4e66bb34, field_a9ee6062;
    std::ptrdiff_t field_f8205eba;
};
class RestrictedCommandRing {
public:
    RestrictedCommandRing(std::span<Command, 150> commands) noexcept
        : commands_(commands.data()), input_(nullptr) {}
    explicit RestrictedCommandRing(void* const& input) noexcept
        : commands_(nullptr), input_(&input) {}
    Command& operator[](std::size_t slot) const noexcept {
        return input_ ? input_command_slot(*input_, std::uint32_t(slot)) : commands_[slot];
    }

private:
    Command* commands_;
    void* const* input_;
};
struct RestrictedPlayerCommandBindings {
    const std::uint8_t* enabled_43467224;
    const std::uint8_t* enabled_4346a740;
    animation::AnimationAngleHistoryView angle_history;
    engine::NativeConVarReference recoil_scale_convar;
    void* const& entity_list;
    PlayerStateCache& players;
    RestrictedCommandRing commands;
    EntityCommandOwner* const& history_owner; //live43475898, getter433952F0
    const float* interval;
    const std::int32_t* frame_number;
    float* current_time;
    float* globals_current_time; //43467B04+10; retains its real aliasing
    float* body_yaw_time; //43476B3C
    float* body_yaw; //43476B40
    std::array<float, 3>* sent_angles; //4347BD30
    prediction::ObserverRegistry* tickbase_observers; //43475AB0..43475AB4
    void** original_player_table; //original virtual224, recovered hook metadata
    std::uint8_t* animation_update_flag; //43475BD3
    animation::AnimationUpdateTiming animation_timing;
    const animation::AnimationUpdateEngine& animation_engine;
    const animation::LocalAnimationBindings& helpers;
    RestrictedPlayerCommandOffsets offsets;
    std::int32_t replay_restore_tick_scratch; //original incoming stack+188
    bool normalize_uninitialized_restore_time = false;
    bool resolve_original_player_table = false;
};
//43326621 ->433F54D5. Null inputs retain the wrapper's four early exits.
// caller_flag is the final native argument; the tag match is computed here
// from the same owner and command number immediately before the internal call.
void process_restricted_player_command(RestrictedPlayerCommandOwner*, void* player, Command*,
                                       std::uint8_t* send_packet, bool caller_flag,
                                       const RestrictedPlayerCommandBindings&);

} // namespace skeet::client

// Declarations from runtime_composition.hpp

namespace skeet::engine {
class HookDescriptorStorage;
class StartupVectorCleanupBindings;
class StartupContainerCleanup;
}
namespace skeet::rage {
class GroupStorage;
class SpreadStorage;
struct SpreadStorageBindings;
struct GroupInitializerBindings;
}
namespace skeet::esp {
class FlagTable;
class FlagTableInitialization;
}
namespace skeet::script {
class UiTypeStorage;
struct UiTypeInitialization;
}
namespace skeet::events {
class EventNameStartupBindings;
}
namespace skeet {
struct MovementGlobalConfig;
}
namespace skeet::rage {
struct Settings;
}
namespace skeet::skins {
struct Settings;
}
namespace skeet::anti_aim {
struct Config;
struct GlobalConfig;
struct FakeLagSettings;
struct OtherGlobalSettings;
struct StartupInitializerBindings;
}
namespace skeet::esp {
struct PlayerEspSettings;
struct OtherEspSettings;
struct EffectsSettings;
struct ColoredModelsSettings;
}
namespace skeet::misc {
struct FovConfig;
struct KnifeConfig;
struct ZeusConfig;
struct AutomaticWeaponsConfig;
struct QuickSwitchConfig;
struct RevealRanksConfig;
struct RevealOverwatchConfig;
struct AutoAcceptConfig;
struct ClanTagConfig;
struct EventLogConfig;
struct GrenadePredictionConfig;
struct PersistentKillFeedConfig;
struct DisablePureConfig;
struct RebuyConfig;
struct ConsoleOutputConfig;
struct NoFallDamageConfig;
struct PingSpikeConfig;
struct FreeLookConfig;
struct AntiUntrustedConfig;
struct HideFromObsConfig;
struct CustomEventsConfig;
}
namespace skeet::ui {
struct WidgetIdentities;
class SettingsMenuStorage;
class RendererAssets;
}

namespace skeet::client {
class ClientRuntimeStorage;
class NativeStartupSources;
class StaticCleanupRuntime;
class HookDescriptorCleanupContext;
struct ExitRegistrationContext;
struct ConfigInitializerBindings;
struct GlobalConfigInitializerBindings;
struct TargetMaskSettings;
struct StaticCrtInitialization;

// These references name process-lifetime imported RNG/math services. The
// explicit overload also supports authored external-boundary fixtures.
struct CrtRuntimeImports {
    engine::SourceInitializationImports startup;
    engine::HookDescriptorSystem hook_queries;
    const rage::SpreadRandom& random;
    const RageTrigonometry& trigonometry;
};
const rage::SpreadRandom& native_spread_random();
CrtRuntimeImports native_crt_runtime_imports();

// Concrete ownership for the data/CRT portion of NativeStartupSourceGraph.
// Construction creates canonical views and their stable binding contexts;
// it runs no CRT callback, feature constructor, hook installation or menu.
// The full feature graph and exact final initializer remain required before
// NativeStartupRuntime can pass its whole-plan preflight.
// Storage and imported RNG/math referents must outlive this owner. Keep this
// owner alive until all callbacks registered in storage.exit_callbacks finish.
class CrtRuntimeOwner {
public:
    explicit CrtRuntimeOwner(ClientRuntimeStorage&);
    CrtRuntimeOwner(ClientRuntimeStorage&, CrtRuntimeImports);
    ~CrtRuntimeOwner();
    CrtRuntimeOwner(const CrtRuntimeOwner&) = delete;
    CrtRuntimeOwner& operator=(const CrtRuntimeOwner&) = delete;
    CrtRuntimeOwner(CrtRuntimeOwner&&) = delete;
    CrtRuntimeOwner& operator=(CrtRuntimeOwner&&) = delete;

    ClientRuntimeStorage& storage() const noexcept;
    const rage::SpreadRandom& random() const noexcept;
    const RageTrigonometry& trigonometry() const noexcept;
    rage::SpreadStorage& spread() noexcept;
    const rage::SpreadStorageBindings& spread_bindings() const noexcept;
    StaticCleanupRuntime& static_cleanup() noexcept;
    ExitRegistrationContext& exit_registration() noexcept;
    engine::HookDescriptorStorage& descriptor_storage() noexcept;
    engine::HookDescriptorRuntime& descriptors() noexcept;
    engine::HookDescriptorSystem hook_system() const noexcept;
    HookDescriptorCleanupContext& descriptor_initializers() noexcept;
    rage::GroupStorage& rage_groups() noexcept;
    rage::GroupInitializerBindings& group_initializers() noexcept;
    esp::FlagTable& esp_flags() noexcept;
    esp::FlagTableInitialization& flag_initializers() noexcept;
    script::UiTypeStorage& ui_types() noexcept;
    script::UiTypeInitialization& ui_type_initializers() noexcept;
    events::EventNameStartupBindings& event_name_initializers() noexcept;
    engine::FirstStartupServiceBindings& first_startup_services() noexcept;

    // Bind only the72 complete CRT bodies owned here. This supplies no final
    // initializer and never executes an initializer or no-argument finalizer.
    // Full graph construction normally invokes the same individual binders.
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};

// A non-owning view of the one set of source feature settings. Constructors
// of their actual feature owners retain registration/default-write order.
struct MiscFeatureSettings {
    misc::FovConfig& fov;
    misc::KnifeConfig& knife;
    misc::ZeusConfig& zeus;
    misc::AutomaticWeaponsConfig& automatic_weapons;
    misc::QuickSwitchConfig& quick_switch;
    misc::RevealRanksConfig& reveal_ranks;
    misc::RevealOverwatchConfig& reveal_overwatch;
    misc::ClanTagConfig& clan_tag;
    misc::EventLogConfig& event_log;
    misc::GrenadePredictionConfig& grenades;
    misc::PersistentKillFeedConfig& persistent_kill_feed;
    misc::DisablePureConfig& disable_pure;
    misc::RebuyConfig& rebuy;
    misc::ConsoleOutputConfig& console_output;
    misc::NoFallDamageConfig& no_fall_damage;
};

// Stable canonical source descriptors for CRT, feature constructors and menu
// lookup. Composite source bundles are owned once here; consumers borrow
// these fields rather than reading independent native-address placeholders.
// The five single-descriptor global views and raw GUI settings borrow their
// exact data cells, including the HUD's live43468E58 hide flag. Construction
// preserves those cells and performs no descriptor/hotkey registration.
class ConfigurationRuntimeOwner {
public:
    explicit ConfigurationRuntimeOwner(ClientRuntimeStorage&);
    ~ConfigurationRuntimeOwner();
    ConfigurationRuntimeOwner(const ConfigurationRuntimeOwner&) = delete;
    ConfigurationRuntimeOwner& operator=(const ConfigurationRuntimeOwner&) = delete;
    ConfigurationRuntimeOwner(ConfigurationRuntimeOwner&&) = delete;
    ConfigurationRuntimeOwner& operator=(ConfigurationRuntimeOwner&&) = delete;

    ClientRuntimeStorage& storage() const noexcept;
    MovementGlobalConfig& movement() noexcept;
    skins::Settings& skins_settings() noexcept;
    rage::Settings& rage_settings() noexcept;
    anti_aim::Config& anti_aim_settings() noexcept;
    anti_aim::GlobalConfig& anti_aim_global() noexcept;
    anti_aim::FakeLagSettings& fake_lag_settings() noexcept;
    anti_aim::OtherGlobalSettings& anti_aim_other() noexcept;
    esp::PlayerEspSettings& player_esp_settings() noexcept;
    esp::OtherEspSettings& other_esp_settings() noexcept;
    esp::EffectsSettings& effects_settings() noexcept;
    esp::ColoredModelsSettings& colored_model_settings() noexcept;
    MiscFeatureSettings& miscellaneous() noexcept;
    misc::PingSpikeConfig& ping_spike() noexcept;
    misc::FreeLookConfig& free_look() noexcept;
    misc::AntiUntrustedConfig& anti_untrusted() noexcept;
    misc::HideFromObsConfig& hide_from_obs() noexcept;
    misc::CustomEventsConfig& custom_events() noexcept;
    TargetMaskSettings& target_mask() noexcept;
    ui::WidgetIdentities& widget_identities() noexcept;
    ui::SettingsMenuStorage& settings_menu() noexcept;
    anti_aim::StartupInitializerBindings& anti_aim_initializers() noexcept;
    ConfigInitializerBindings& static_initializers() noexcept;
    GlobalConfigInitializerBindings& global_initializers() noexcept;
    // Bind only the58 existing independent descriptor bodies. Movement,
    // shared skins and the menu hotkey are composed by the full source graph.
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};

struct WindowRuntimeImports {
    engine::SourceInitializationImports startup;
    engine::UnloadSystemFunctions unload;
    input::WindowHookImports subclass;
};
WindowRuntimeImports native_window_runtime_imports(const engine::SourceInitializationImports&);

// One process hook/unload state, available before ScriptOwner and MainMenu.
// Engine/device-manager pointers, lifecycle flags, both patch-list heads and
// the optional connection callback borrow the actual source data cells;
// later publication is observed without refresh.
// Construction installs no subclass, calls no window procedure and registers
// no callback. The explicit bind operation only publishes source callback
// context; PostInitializationRuntime retains the actual install position.
class WindowRuntimeOwner {
public:
    explicit WindowRuntimeOwner(ClientRuntimeStorage&);
    WindowRuntimeOwner(ClientRuntimeStorage&, const engine::SourceInitializationImports&);
    WindowRuntimeOwner(ClientRuntimeStorage&, WindowRuntimeImports);
    ~WindowRuntimeOwner();
    WindowRuntimeOwner(const WindowRuntimeOwner&) = delete;
    WindowRuntimeOwner& operator=(const WindowRuntimeOwner&) = delete;
    WindowRuntimeOwner(WindowRuntimeOwner&&) = delete;
    WindowRuntimeOwner& operator=(WindowRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    engine::UnloadState& unload_state() noexcept;
    engine::UnloadRuntime& unload_runtime() noexcept;
    input::WindowSubclassRuntime& subclass() noexcept;
    const engine::SourceInitializationImports& startup_imports() const noexcept;
    void bind_subclass_callback();

private:
    struct Data;
    std::unique_ptr<Data> data_;
};

// Explicit source inputs for the two otherwise unconsumed incoming-ECX words
// retained by the native inline-container constructors. No value is inferred
// from a captured context or supplied by a default argument.
struct StaticCrtConstructorInputs {
    std::uint32_t flags_word;
    std::uint32_t keys_word;
};
struct StaticCrtImports {
    void (*dispose_v8_global)(void**);
};
StaticCrtImports native_static_crt_imports();

// The remaining CRT contexts borrow early icon/listener storage and the same
// process unload state. No device, renderer, codecs or engine interfaces are
// required here. Native container headers remain native header views;
// they do not own or release the separate source Lua/catalog/preset vectors.
// A source bootstrap must select SourceTerminalCleanupBindings for its actual
// source container producers before executing any CRT registration callback.
class StaticCrtRuntimeOwner {
public:
    StaticCrtRuntimeOwner(CrtRuntimeOwner&, WindowRuntimeOwner&, ui::RendererAssets&,
                          StaticCrtConstructorInputs);
    StaticCrtRuntimeOwner(CrtRuntimeOwner&, WindowRuntimeOwner&, ui::RendererAssets&,
                          StaticCrtConstructorInputs, StaticCrtImports);
    ~StaticCrtRuntimeOwner();
    StaticCrtRuntimeOwner(const StaticCrtRuntimeOwner&) = delete;
    StaticCrtRuntimeOwner& operator=(const StaticCrtRuntimeOwner&) = delete;
    StaticCrtRuntimeOwner(StaticCrtRuntimeOwner&&) = delete;
    StaticCrtRuntimeOwner& operator=(StaticCrtRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    StaticCrtInitialization& static_initializers() noexcept;
    engine::StartupVectorCleanupBindings& native_vector_cleanup() noexcept;
    engine::StartupContainerCleanup& native_container_cleanup() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from runtime_feature_bindings.hpp

namespace skeet::anti_aim {
class RuntimeOwner;
}
namespace skeet::effects {
class EffectsRuntimeOwner;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
struct SourceScriptModuleInputs;
}
namespace skeet::client {
class ClientRuntimeStorage;
class PacketRuntimeOwner;
class ExploitRuntimeOwner;
struct NativeStartupSourceGraph;

// The five combat factories borrow their already composed physical domain
// owners. This aggregate creates no feature owner, Lua state, callback entry
// or registry receiver. All source referents outlive factory execution.
class CombatRuntimeOwner final {
public:
    // SourceScriptRuntimeOwner's genuine module inputs must already be bound.
    CombatRuntimeOwner(ClientRuntimeStorage&, PacketRuntimeOwner&, ExploitRuntimeOwner&,
                       anti_aim::RuntimeOwner&, effects::EffectsRuntimeOwner&,
                       script::SourceScriptRuntimeOwner&);
    // The late production phase may bind those actual module inputs here,
    // after CONFIG/RootMenu contexts exist and before ScriptOwner is created.
    CombatRuntimeOwner(ClientRuntimeStorage&, PacketRuntimeOwner&, ExploitRuntimeOwner&,
                       anti_aim::RuntimeOwner&, effects::EffectsRuntimeOwner&,
                       script::SourceScriptRuntimeOwner&, script::SourceScriptModuleInputs);
    ~CombatRuntimeOwner();
    CombatRuntimeOwner(const CombatRuntimeOwner&) = delete;
    CombatRuntimeOwner& operator=(const CombatRuntimeOwner&) = delete;
    CombatRuntimeOwner(CombatRuntimeOwner&&) = delete;
    CombatRuntimeOwner& operator=(CombatRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    CombatRuntimeFactoryBindings& factory_bindings() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};

// Late source composition after genuine engine acquisition. Validates the
// graph's shared native cells and all existing factory contexts, binds the57
// recovered factory identities and preflights that exact plan. It neither
// constructs features nor executes/replaces/reselects any CRT, terminal
// cleanup or final43484F06 binding. The graph and its referents stay alive.
void bind_runtime_feature_sources(NativeStartupSources&, NativeStartupSourceGraph&);
} //namespace skeet::client

// Declarations from runtime_owner_bindings.hpp

namespace skeet::client {
class NativeStartupSources;

// Borrowed canonical contexts for eight allocation/constructor wrappers.
// Keep this record and its referenced state alive through owner destruction.
// Registration below selects bodies only; runtime_owner_initializers supplies
// their order together with the remaining recovered owner families.
struct RuntimeOwnerFactoryBindings {
    ConfigRegistries& configuration;
    engine::OwnerRegistryBinding lifecycle; //43475B78
    engine::OwnerRegistryBinding hud; //43475A9C
    EntityObserverRegistry& entity_updates; //43475BA8
    engine::OwnerRegistryBinding effect_frames; //43475A84
    skins::SequenceOwnerBindings sequences;
    rage::OwnerBindings rage;
    rage::LifecycleOwner*& rage_active; //43475810
    legit::OwnerBindings legit;
    esp::EspOwnerBindings esp;
    esp::EspOwnerRuntime*& esp_active; //434758DC
    effects::BlurBindings blur;
    effects::BlurEffect*& blur_active; //43475A20
    misc::ConsoleOutputConfig& console_settings;
    misc::ColoredLogRing*& console_active; //434758CC
    const float& frame_time;
    misc::FovConfig& fov_settings;
    misc::FovOwnerBindings fov;
    misc::NormalLog& event_output;
    rage::TargetRelationshipBindings event_targets;
    script::ConsoleInputHook& purchase_console;
    void* const& console_container;
};

//43481F6A,4348DA25,4347FEBC,43481EAB,4347F6DB,43484E4E,4347DA60,43481F44.
// FOV/purchase OOM tails reach unresolved INT1; source throws bad_alloc.
// No captured native
// code is called. All registry/image/configuration aliases are checked before
// any binding is published or any factory is allowed to allocate an owner.
void bind_runtime_owner_factories(NativeStartupSources&, RuntimeOwnerFactoryBindings&);
} //namespace skeet::client

// Declarations from runtime_owner_initialization.hpp

namespace skeet::client {
class NativeStartupSources;

// Source identifiers in the constructor order observed in the working client.
// These integers are never converted into executable addresses.
std::span<const std::uint32_t, 57> runtime_owner_initializers() noexcept;
void preflight_runtime_owner_initialization(const NativeStartupSources&);
void initialize_runtime_owners(const NativeStartupSources& sources);
}

// Declarations from runtime_storage.hpp

namespace skeet::client {
class PlayerRuntimeOwner;
class MiscRuntimeOwner;
struct EngineRuntimeBindings {
    engine::BuildModuleBases modules;
    engine::BuildEngineInterfaces interfaces;
    engine::BuildStartupConVarBindings variables;
    animation::BoneAttachmentUpdate update_bone_attachments;
    engine::BuildCommandBindings command{};
};
// Concrete storage shared by startup factories, dispatchers and shutdown.
// The default constructor creates one zeroed source data image and obtains
// the actual Windows process-heap services. No executable image or VM is
// required to construct these canonical source instances.
class ClientRuntimeStorage {
public:
    ClientRuntimeStorage();
    explicit ClientRuntimeStorage(std::shared_ptr<engine::NativeDataImage>);
    ~ClientRuntimeStorage();
    ClientRuntimeStorage(const ClientRuntimeStorage&) = delete;
    ClientRuntimeStorage& operator=(const ClientRuntimeStorage&) = delete;
    ClientRuntimeStorage(ClientRuntimeStorage&&) = delete;
    ClientRuntimeStorage& operator=(ClientRuntimeStorage&&) = delete;

    engine::NativeDataImage& data() const noexcept {
        return *data_;
    }
    std::shared_ptr<engine::NativeDataImage> data_owner() const noexcept {
        return data_;
    }
    engine::JobMemory memory() const noexcept {
        return memory_;
    }
    PreservedConfigStorage& config_storage() noexcept {
        return *configuration_;
    }
    std::shared_ptr<PreservedConfigStorage> config_storage_owner() const noexcept {
        return configuration_;
    }
    ConfigRegistries& configuration() noexcept {
        return configuration_->configuration();
    }
    RegisteredHotkey& menu_hotkey() const;
    ui::SettingsMenuStorage& settings_menu_storage() noexcept {
        return settings_menu_;
    }
    ui::MenuStartupData& menu_startup_data() noexcept {
        return menu_data_;
    }
    input::Runtime& input_runtime() noexcept {
        return input_;
    }
    engine::NativeOwnerRegistryStorage& owner_registries() noexcept {
        return owner_registries_;
    }
    CommandPhaseRegistry& command_phases() noexcept {
        return commands_;
    }
    CommandEmptyTickRegistry& empty_ticks() noexcept {
        return empty_ticks_;
    }
    EntityObserverRegistry& entity_observers() noexcept {
        return entities_;
    }
    prediction::ObserverRegistry& prediction_observers() noexcept {
        return predictions_;
    }
    network::DataUpdateObserverRegistry& data_update_observers() noexcept {
        return data_updates_;
    }
    network::ExploitRechargeObserverRegistry& recharge_observers() noexcept {
        return recharge_;
    }
    engine::NativeOffsetStorage& offsets() noexcept {
        return offsets_;
    }
    events::EventNameStorage& event_names() noexcept {
        return event_names_;
    }
    events::EventRegistry& event_registry() noexcept {
        return events_;
    }
    ExitCallbacks& exit_callbacks() noexcept {
        return exits_;
    }
    // Acquire the seven proven engine interfaces before constructing owners
    // that borrow them. A failed acquisition publishes no partial bundle.
    engine::CoreEngineInterfaces bind_core_interfaces(engine::FactoryProvider&);
    engine::CoreEngineInterfaces bind_core_interfaces();
    // Call after interface discovery and HUD lookup publication, before input hooks.
    void bind_input_interfaces();

    // Construct the concrete engine inputs consumed by source owners: loaded
    // modules, interfaces, receive/datamap offsets and live ConVar references.
    // This phase runs once per storage instance. A failed attempt requires a
    // new instance; partially initialized offset keys must not be XORed twice.
    // CRT registration, feature owners, hooks and menu startup are separate.
    EngineRuntimeBindings initialize_engine_bindings(const engine::SourceInitializationImports&,
                                                     std::uint32_t timestamp_low);

    // Storage construction performs no CRT initializers, owner registration,
    // engine-interface binding or hook installation. Those operations retain
    // their actual source startup order and borrow these same instances.
    // Keep this aggregate alive until owners and terminal callbacks finish;
    // Its C++ destruction releases source offset storage if terminal cleanup
    // has not already cleared it; it does not run other native finalizers.
private:
    friend class PlayerRuntimeOwner;
    friend class MiscRuntimeOwner;
    PlayerRuntimeOwner* active_player_owner_ = nullptr;
    MiscRuntimeOwner* active_misc_owner_ = nullptr;
    std::shared_ptr<engine::NativeDataImage> data_;
    engine::JobMemory memory_;
    std::shared_ptr<PreservedConfigStorage> configuration_;
    ui::SettingsMenuStorage settings_menu_;
    ui::MenuStartupData menu_data_;
    input::Runtime input_;
    engine::NativeOwnerRegistryStorage owner_registries_;
    CommandPhaseRegistry commands_;
    CommandEmptyTickRegistry empty_ticks_;
    EntityObserverRegistry entities_;
    prediction::ObserverRegistry predictions_;
    network::DataUpdateObserverRegistry data_updates_;
    network::ExploitRechargeObserverRegistry recharge_;
    engine::NativeOffsetStorage offsets_;
    events::EventNameStorage event_names_;
    events::EventRegistry events_;
    ExitCallbacks exits_;
    bool engine_bindings_started_ = false;
};
} //namespace skeet::client

// Declarations from shared_esp_owner.hpp

namespace skeet::client {
class NativeStartupSources;

struct SharedEspOwnerRegistries {
    engine::OwnerRegistryBinding primary; //43475B78,433676A0
    CommandPhaseRegistry& commands; //434757A0,43345EBC
    engine::OwnerRegistryBinding messages; //43475B94,4336F73C
    EntityObserverRegistry& entities; //43475BA8,4333BF32
    CommandEmptyTickRegistry& empty_ticks; //43475978,433F10DB
    events::EventRegistry& events;
};
struct SharedEspConstructorGlobals {
    void* const& engine_client; //4346A514, recorded MOV43485E86
    const std::uint32_t& encoded_send_gate_a; //43469068 xor7B3564BB
    const std::uint32_t& encoded_send_gate_b; //43468D88 xor584C3C72
    const std::uint32_t& encoded_game_type; //43467A8C xorAC9299CC
    const std::uint32_t& encoded_field_b0; //43467A4C xorFA2DD713
    const std::uint32_t& encoded_field_b4; //43468EA8 xorAC34F836
    // Actual loaded-engine CALL locations. The constructor consumes each
    // nonnull slot before reading its relative target, exactly as recorded.
    const std::byte*& message_constructor_call; //4346D668
    const std::byte*& receiver_constructor_call; //4346D66C
};
struct SharedEspOwnerBindings {
    RejectedPlayerUpdateOwner& state;
    esp::SharedEspConfig& settings;
    SharedEspCommandOwner& command;
    network::SharedEspRuntime& incoming;
    RejectedPlayerUpdateOwner*& active; //canonical434758D0 cell
    const SharedEspConstructorGlobals& globals;
};

std::size_t shared_esp_owner_size() noexcept;
// Whole43485DC9. Registrations retain native primary -> command -> network
// message -> BA8 -> empty-tick order. Only the two owner-local descriptors
// (activation7174D684, other-team6834D5DA) are registered here. The enabled
// and restrict-updates descriptors remain the same separate static globals.
// Placement construction preserves fields not assigned by the native body.
void* construct_shared_esp_owner(void* storage, const SharedEspOwnerRegistries&, ConfigRegistries&,
                                 SharedEspOwnerBindings);
// Whole4348DA78: allocate0x2B8 with43387448's zeroed/aligned heap protocol,
// construct if nonnull, then publish state/null at the recorded4348DA91 MOV.
// Registration in NativeStartupSources does not assert VM invocation order.
void* create_shared_esp_owner(const SharedEspOwnerRegistries&, ConfigRegistries&,
                              SharedEspOwnerBindings);
// Whole433801D6 ->433837D0. Clears the active cell before the null test and
// releases this allocation; registered receiver ranges have their own cleanup.
void destroy_shared_esp_owner(void* storage, RejectedPlayerUpdateOwner*& active);
// Primary slot7=433C0954 clears only geometry.last_live_tick for indices1..64.
void reset_shared_esp_owner(void* storage);

struct SharedEspInitialization {
    const SharedEspOwnerRegistries& registries;
    ConfigRegistries& configuration;
    SharedEspOwnerBindings bindings;
};
void bind_shared_esp_initializer(NativeStartupSources&, SharedEspInitialization&);
} // namespace skeet::client

// Declarations from shared_esp_runtime_owner.hpp

namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
class HistoryRuntimeOwner;
// Stable production contexts for the separate4348DA78 shared-ESP owner.
// Main ESP, history, menu, Lua and the command master borrow one player cache
// and adjustment table. This context runs no factory or callback on creation.
class SharedEspRuntimeOwner {
public:
    SharedEspRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                          ConfigurationRuntimeOwner&, PlayerRuntimeOwner&, HistoryRuntimeOwner&,
                          script::SourceScriptRuntimeOwner&);
    ~SharedEspRuntimeOwner();
    SharedEspRuntimeOwner(const SharedEspRuntimeOwner&) = delete;
    SharedEspRuntimeOwner& operator=(const SharedEspRuntimeOwner&) = delete;
    SharedEspRuntimeOwner(SharedEspRuntimeOwner&&) = delete;
    SharedEspRuntimeOwner& operator=(SharedEspRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    SharedEspInitialization& initializers() noexcept;
    const RejectedPlayerUpdateContext& rejected_player_update() const noexcept;
    network::SharedEspRuntime& incoming() noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} // namespace skeet::client

// Declarations from shutdown.hpp

namespace skeet::ui {
class RendererContainer;
class MainMenuWindowOwner;
}
namespace skeet::client {
struct ChannelHookState;
struct MemoryRegion {
    void* base_address = nullptr;
    void* allocation_base = nullptr;
    std::uint32_t allocation_protection = 0;
    std::size_t region_size = 0;
    std::uint32_t state = 0, protection = 0, type = 0;
};
struct ShutdownImports {
    using KillTimer = int(__stdcall*)(std::uintptr_t, std::uintptr_t);
    using QueryMemory = std::int32_t(__stdcall*)(void*, const void*, std::uint32_t, MemoryRegion*,
                                                 std::size_t, std::size_t*);
    using UnmapSection = std::int32_t(__stdcall*)(void*, void*);
    KillTimer kill_timer = nullptr;
    QueryMemory query_memory = nullptr;
    UnmapSection unmap_section = nullptr;
};
ShutdownImports native_shutdown_imports();

struct ExitCallbacks {
    std::span<std::uintptr_t> entries;
    // Registration and teardown borrow the same cells; callbacks can observe
    // and change the live count while the terminal pass is running.
    std::uintptr_t& xor_mask; //4347CA08
    std::uint32_t& count; //4347CA0C
    // 433837F5 requires the actual512-slot backing store. Registration is
    // rejected only at count512; duplicates do not increment the count.
    int append(void (*callback)());
    void run(); // 0x433B3987, leaves count=UINT32_MAX after completion.
};

class ShutdownRuntime {
    ChannelHookState* source_channel_ = nullptr;

public:
    ShutdownRuntime(script::ShutdownState& scripts, input::Runtime& input, ShutdownImports imports,
                    bool& scripting_enabled, engine::UnloadRuntime& hooks,
                    ExitCallbacks& exit_callbacks, NativeConfigPath& config_path,
                    ui::MenuWindow& main_window)
        : scripts(scripts), input(input), imports(imports), hooks(hooks),
          native_window(main_window.native_window), scripting_enabled(scripting_enabled),
          owners(hooks.state.owners), window_title(config_path), exit_callbacks(exit_callbacks) {
        if (!native_window)
            throw std::invalid_argument("Client shutdown requires the resolved main window");
    }
    ShutdownRuntime(script::ShutdownState& scripts, input::Runtime& input, ShutdownImports imports,
                    bool& scripting_enabled, engine::UnloadRuntime& hooks,
                    ExitCallbacks& exit_callbacks, NativeConfigPath& config_path,
                    ui::MenuWindow& main_window, ChannelHookState& channel)
        : ShutdownRuntime(scripts, input, imports, scripting_enabled, hooks, exit_callbacks,
                          config_path, main_window) {
        source_channel_ = &channel;
    }
    script::ShutdownState& scripts;
    input::Runtime& input;
    ShutdownImports imports;
    engine::UnloadRuntime& hooks;
    // Borrow the menu foundation's HWND cell. Hook restoration clears its own
    // installation state before the unload timer is created and cancelled.
    const std::uintptr_t& native_window;
    std::uint32_t installed_engine_dispatch = 0, saved_engine_dispatch = 0;
    bool& scripting_enabled; // Native 0x434688BD; bind ScriptRuntime.enabled.
    engine::OwnerRegistry& owners; // same43475B78/7C/80 storage as startup/unload
    ui::WindowCallbackStorage* main_callbacks = nullptr;
    ui::WindowTitleString& window_title; //same native434798CC cell as CONFIG file backend
    ui::WindowLifetimeImports window_imports{};
    ui::RendererContainer* renderer = nullptr;
    ExitCallbacks& exit_callbacks; // same512slots/count/mask as process initialization
    // Source wrappers have a longer outer C++ lifetime than native slot0.
    // Detach RootMenu while its ScriptOwner references are still alive, and
    // publish the native GUI release before later outer-owner destruction.
    ui::MainMenuWindowOwner* source_main_window = nullptr;
    ui::RendererContainer** source_renderer_cell = nullptr;

    void restore_engine_dispatch(); // 0x43365AA1
    void release_cleanup_objects(); // 0x4339D451, virtual slot 0.
    void destroy_gui(); // 0x43336C51, logical ownership release for typed objects.
    void cancel_timer(); // 0x433D7307, shared resolved HWND and timer ID0x332.
    void* finalize(); // 0x433C87F6, returns this code section's allocation base.
    void detach(); // 0x433EF91A, hard teardown; no timer/query/unmap tail.
};

// Bind before installing the unload timer. The callback performs a machine tail
// jump to ZwUnmapViewOfSection, so it never returns through unmapped client code.
void bind_shutdown_runtime(ShutdownRuntime& runtime) noexcept;
ShutdownRuntime* bound_shutdown_runtime() noexcept;
void unbind_shutdown_runtime(ShutdownRuntime&) noexcept;
void* finalize_client(); // Native no-argument entry 0x433C87F6.
void detach_client(); // Native no-argument entry 0x433EF91A.
}

// Declarations from shutdown_runtime_owner.hpp

namespace skeet {
class ConfigStorageOwner;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::ui {
class MainMenuWindowOwner;
class StartupRenderer;
}
namespace skeet::client {
class ClientRuntimeStorage;
class WindowRuntimeOwner;
struct ClientShutdownImports {
    ShutdownImports finalization;
    ui::WindowLifetimeImports window;
    misc::UnloadImports request;
};
ClientShutdownImports native_client_shutdown_imports();

// Source shutdown/request composition before or after ScriptOwner. All menu,
// Lua, heap, hook and configuration contexts are borrowed from their actual
// owners. Constructing this object neither unloads nor schedules a timer.
class ClientShutdownOwner {
public:
    ClientShutdownOwner(ClientRuntimeStorage&, WindowRuntimeOwner&, ui::MainMenuWindowOwner&,
                        ConfigStorageOwner&, script::SourceScriptRuntimeOwner&,
                        ui::StartupRenderer&);
    ClientShutdownOwner(ClientRuntimeStorage&, WindowRuntimeOwner&, ui::MainMenuWindowOwner&,
                        ConfigStorageOwner&, script::SourceScriptRuntimeOwner&,
                        ui::StartupRenderer&, ClientShutdownImports);
    ~ClientShutdownOwner();
    ClientShutdownOwner(const ClientShutdownOwner&) = delete;
    ClientShutdownOwner& operator=(const ClientShutdownOwner&) = delete;
    ClientShutdownOwner(ClientShutdownOwner&&) = delete;
    ClientShutdownOwner& operator=(ClientShutdownOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    ShutdownRuntime& finalizer() noexcept;
    misc::UnloadRuntime& request() noexcept;
    // Required explicit profile selection before the unload button is usable.
    // The module must contain the source callback; no mapped-image profile
    // is selected when this validation fails or has not yet run.
    void select_source_dll(void* module);
    void select_source_dll(void* module, SourceDllShutdownImports);
    // An owning application supplies its destruction cut explicitly. The
    // callback runs after finalize and may destroy this owner itself.
    void select_source_dll(void* module, SourceDllShutdownCompletion);
    void select_source_dll(void* module, SourceDllShutdownImports, SourceDllShutdownCompletion);
    void cancel_deferred_request() noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
}

// Declarations from skin_runtime_factories.hpp

namespace skeet::client {
class NativeStartupSources;

struct SkinRuntimeFactoryBindings {
    ConfigRegistries& configuration;
    engine::OwnerRegistryBinding lifecycle; //43475B78
    CommandPhaseRegistry& commands; //434757A0
    EntityObserverRegistry& entity_updates; //43475BA8
    skins::Settings& settings;
    skins::InventoryOwnerBindings inventory;
    skins::CosmeticOwnerBindings cosmetics;
    skins::AgentOwnerBindings agent;
};

inline constexpr std::uint32_t skin_inventory_factory = 0x4347E81Au;
inline constexpr std::uint32_t skin_glove_factory = 0x43484AC4u;
inline constexpr std::uint32_t skin_mask_factory = 0x43480644u;
inline constexpr std::uint32_t skin_agent_factory = 0x4348574Eu;
inline constexpr std::size_t skin_inventory_native_size = 0x43C;
inline constexpr std::size_t skin_glove_native_size = 0x8C;
inline constexpr std::size_t skin_mask_native_size = 0x1E4;
inline constexpr std::size_t skin_agent_native_size = 0x64;

// The four native factories allocate with43387448 and tail to their complete
// constructors. They publish no singleton: their physical owners enter the
// existing registries. Source metadata may require a larger representation.
// Native OOM branches4347E82F/43484AD9/43480659/43485761 enter unresolved
// INT1 paths. Source policy throws bad_alloc there without claiming recovery.
skins::InventoryLifecycleOwner* create_skin_inventory_owner(const SkinRuntimeFactoryBindings&);
skins::GloveOwner* create_skin_glove_owner(const SkinRuntimeFactoryBindings&);
skins::MaskOwner* create_skin_mask_owner(const SkinRuntimeFactoryBindings&);
void* create_skin_agent_owner(const SkinRuntimeFactoryBindings&);

// Bind only these four identities; the live startup operand order chooses
// their construction order. The separate43481F6A sequence factory is unchanged.
void bind_skin_runtime_factories(NativeStartupSources&, SkinRuntimeFactoryBindings&);
} //namespace skeet::client

// Declarations from source_application.hpp

namespace skeet::client {
class ClientRuntimeStorage;
// Complete process lifetime for the source DLL. Construction prepares the
// early owners; initialize() executes their CRT schedule and the actual final
// engine/menu/hook operation. The caller retains this object until the source
// shutdown completion callback, after engine hooks and callbacks are removed.
class SourceApplication final {
public:
    SourceApplication(void* source_module, SourceDllShutdownCompletion);
    ~SourceApplication();
    SourceApplication(const SourceApplication&) = delete;
    SourceApplication& operator=(const SourceApplication&) = delete;
    SourceApplication(SourceApplication&&) = delete;
    SourceApplication& operator=(SourceApplication&&) = delete;
    void initialize(std::uintptr_t initialization_argument);
    bool initialized() const noexcept;
    // Called only by the initialization thread after publishing a failed
    // startup state. Failure retains this application and its DLL: cleanup
    // must never dispatch an incomplete owner or repeat partial destruction.
    bool cancel_initialization() noexcept;
    ClientRuntimeStorage& storage() noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from source_dll.hpp

namespace skeet::client {
enum class SourceDllStatus : std::uint32_t {
    initializing = 1,
    ready = 2,
    initialization_failed = 3,
    cleanup_failed = 4,
    stopping = 5,
    stopped = 6
};
}

#if defined(SKEET_SOURCE_DLL_BUILD)
#define SKEET_SOURCE_DLL_API __declspec(dllexport)
#else
#define SKEET_SOURCE_DLL_API __declspec(dllimport)
#endif

// Both functions are read-only. The DLL starts its source application on a
// worker after normal Windows loading; unloading belongs to its menu runtime.
extern "C" SKEET_SOURCE_DLL_API std::uint32_t __cdecl SkeetGetStatus() noexcept;
// Returns the required byte count including the terminator. A nonzero
// capacity requires a valid buffer; a short buffer receives a terminated copy.
extern "C" SKEET_SOURCE_DLL_API std::uint32_t __cdecl
SkeetCopyStartupError(char* buffer, std::uint32_t capacity) noexcept;

#undef SKEET_SOURCE_DLL_API

// Declarations from source_dll_shutdown.hpp

namespace skeet::misc {
class UnloadRuntime;
}
namespace skeet::client {
class ShutdownRuntime;

struct SourceDllShutdownImports {
    // The second GetModuleHandleExW operand is a code address when flag4 is
    // set. Flag2 preserves the existing loader reference count.
    using ModuleFromAddress = int(__stdcall*)(std::uint32_t, const void*, void**);
    using FreeLibrary = int(__stdcall*)(void*);
    ModuleFromAddress module_from_address;
    FreeLibrary free_library;
};
SourceDllShutdownImports native_source_dll_shutdown_imports();

enum class SourceDllShutdownReadiness {
    waiting,
    ready,
    failed
};
struct SourceDllShutdownCompletion {
    void* context = nullptr;
    // Runs after all native finalization, before the loader tail jump. It may
    // delete the application, including this profile and its ShutdownRuntime.
    void (*complete)(void*) noexcept = nullptr;
    // The application publishes ready after its last initialization access,
    // or failed before rollback. Once ready, it cannot transition to failed.
    SourceDllShutdownReadiness (*readiness)(void*) noexcept = nullptr;
};

// Explicit profile for this source DLL when its owner acquired a LoadLibrary
// reference. The supplied HMODULE must contain the actual timer entry below.
// Construction neither binds startup nor schedules any timer. Keep the shared
// shutdown runtime alive through its complete finalization and keep this
// profile alive until the scheduled timer begins finalization.
class SourceDllShutdownProfile {
public:
    SourceDllShutdownProfile(ShutdownRuntime&, void* source_module);
    SourceDllShutdownProfile(ShutdownRuntime&, void* source_module, SourceDllShutdownImports);
    SourceDllShutdownProfile(ShutdownRuntime&, void* source_module, SourceDllShutdownCompletion);
    SourceDllShutdownProfile(ShutdownRuntime&, void* source_module, SourceDllShutdownImports,
                             SourceDllShutdownCompletion);
    ~SourceDllShutdownProfile();
    SourceDllShutdownProfile(const SourceDllShutdownProfile&) = delete;
    SourceDllShutdownProfile& operator=(const SourceDllShutdownProfile&) = delete;
    SourceDllShutdownProfile(SourceDllShutdownProfile&&) = delete;
    SourceDllShutdownProfile& operator=(SourceDllShutdownProfile&&) = delete;
    ShutdownRuntime& runtime() const noexcept {
        return runtime_;
    }
    void* module() const noexcept {
        return module_;
    }
    SourceDllShutdownImports imports() const noexcept {
        return imports_;
    }
    SourceDllShutdownCompletion completion() const noexcept {
        return completion_;
    }
    void cancel_deferred_request() noexcept;

private:
    friend void bind_source_dll_shutdown(SourceDllShutdownProfile&, misc::UnloadRuntime&);
    friend void __stdcall source_dll_deferred_request_timer_callback(void*, std::uint32_t,
                                                                     std::uintptr_t, std::uint32_t);
    bool allow_request_locked();
    void cancel_deferred_request_locked() noexcept;
    ShutdownRuntime& runtime_;
    void* module_;
    SourceDllShutdownImports imports_;
    SourceDllShutdownCompletion completion_;
    misc::UnloadRuntime* request_ = nullptr;
    std::uintptr_t deferred_window_ = 0;
};

// Select only after the source startup graph and its actual shutdown runtime
// exist. This changes this request owner's timer entry explicitly; the retained
// mapped-image profile and its NtUnmapViewOfSection tail are unchanged.
void bind_source_dll_shutdown(SourceDllShutdownProfile&, misc::UnloadRuntime&);
void __stdcall source_dll_unload_timer_callback(void*, std::uint32_t, std::uintptr_t,
                                                std::uint32_t);
void __stdcall source_dll_deferred_request_timer_callback(void*, std::uint32_t, std::uintptr_t,
                                                          std::uint32_t);
}

// Declarations from source_feature_runtime.hpp

namespace skeet {
class ConfigStorageOwner;
}
namespace skeet::animation {
class FixAnimationBackupCleanupContext;
}
namespace skeet::script {
class SourceScriptRuntimeOwner;
}
namespace skeet::ui {
class RendererAssets;
class StartupRenderer;
class MainMenuWindowOwner;
class RootMenu;
}
namespace skeet::engine {
class HookRuntimeOwner;
class ClientClassFactoryRuntime;
}
namespace skeet::effects {
class VisualReceiveOwner;
class CameraRuntimeOwner;
}
namespace skeet::skins {
class SkinsRuntimeOwner;
}
namespace skeet::misc {
class SettingsRuntimeOwner;
class ViewmodelRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class CrtRuntimeOwner;
class ConfigurationRuntimeOwner;
class StaticCrtRuntimeOwner;
class WindowRuntimeOwner;
class PlayerRuntimeOwner;
class ClientShutdownOwner;
class HistoryRuntimeOwner;
class LocalAnimationRuntimeOwner;
class EntityCommandRuntimeOwner;
class CommandRuntimeOwner;
class VisualRuntimeOwner;
class MiscRuntimeOwner;
class PlayerReceiveOwner;
class WorldRuntimeOwner;
struct EngineRuntimeBindings;
struct SourceFeatureRuntimeImports;

// Already established process and engine lifetimes at the feature-factory cut.
// No retained executable client, captured callback or alternate data image is
// an input. The exact final initializer remains owned by the startup caller.
struct SourceFeatureInputs {
    ClientRuntimeStorage& storage;
    const EngineRuntimeBindings& engine;
    CrtRuntimeOwner& crt;
    ConfigurationRuntimeOwner& configuration;
    StaticCrtRuntimeOwner& static_crt;
    WindowRuntimeOwner& window;
    PlayerRuntimeOwner& players;
    script::SourceScriptRuntimeOwner& scripts;
    animation::FixAnimationBackupCleanupContext& animation_cleanup;
    ui::RendererAssets& assets;
    ui::StartupRenderer& renderer;
    ui::MainMenuWindowOwner& main_window;
    ConfigStorageOwner& presets;
    ClientShutdownOwner& shutdown;
    engine::BuildMaterialBindings materials;
    engine::BuildSkinBindings skin_engine;
    NativeInitializerCall final_initializer;
};
// One complete, stable assembly of all57 feature factory contexts and the
// shared command/hook adapters. Construction runs none of those factories.
// It is kept alive until hooks and lifecycle owners have been destroyed and
// source terminal cleanup has completed; borrowed early owners outlive it.
class SourceFeatureRuntime final {
public:
    explicit SourceFeatureRuntime(SourceFeatureInputs);
    SourceFeatureRuntime(SourceFeatureInputs, const SourceFeatureRuntimeImports&);
    ~SourceFeatureRuntime();
    SourceFeatureRuntime(const SourceFeatureRuntime&) = delete;
    SourceFeatureRuntime& operator=(const SourceFeatureRuntime&) = delete;
    SourceFeatureRuntime(SourceFeatureRuntime&&) = delete;
    SourceFeatureRuntime& operator=(SourceFeatureRuntime&&) = delete;
    NativeStartupSourceGraph& graph() noexcept;
    void bind_factories(NativeStartupSources&);
    void initialize_features(NativeStartupSources&);
    // After initialize_features and compose_menu, bind the94 entries to their
    // actual ScriptOwner/RootMenu and physical feature contexts.
    void bind_hook_contexts();
    ui::RootMenu& compose_menu();
    engine::HookRuntimeOwner& hooks() noexcept;
    engine::ClientClassFactoryRuntime& class_factories() noexcept;
    HistoryRuntimeOwner& history() noexcept;
    LocalAnimationRuntimeOwner& local_animation() noexcept;
    EntityCommandRuntimeOwner& entity_commands() noexcept;
    CommandRuntimeOwner& commands() noexcept;
    VisualRuntimeOwner& visual() noexcept;
    MiscRuntimeOwner& miscellaneous() noexcept;
    WorldRuntimeOwner& world() noexcept;
    skins::SkinsRuntimeOwner& skins() noexcept;
    misc::SettingsRuntimeOwner& menu_settings() noexcept;
    misc::ViewmodelRuntimeOwner& viewmodel() noexcept;
    effects::CameraRuntimeOwner& camera() noexcept;
    PlayerReceiveOwner& player_receives() noexcept;
    effects::VisualReceiveOwner& visual_receives() noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from source_feature_runtime_imports.hpp

namespace skeet::client {
// Explicit external services for an authored integrated boundary fixture.
// Every recovered domain owner remains the production implementation. The
// history and RAGE dispatchers receive this same actual external thread pool.
struct SourceFeatureRuntimeImports {
    HistoryRuntimeImports history;
    MiscRuntimeImports miscellaneous;
    PacketRuntimeImports packet;
    anti_aim::RuntimeImports anti_aim;
    script::ScriptApiRuntimeImports script_api;
};
} //namespace skeet::client

// Declarations from source_startup_cleanup.hpp

namespace skeet::client {
class ClientRuntimeStorage;
class WindowRuntimeOwner;
// Read-only preflight for cancellation of source initialization. Failure
// requires retaining the application; these checks perform no teardown.
bool source_exit_callbacks_ready(const ClientRuntimeStorage&) noexcept;
bool source_startup_detach_ready(ClientRuntimeStorage&, WindowRuntimeOwner&) noexcept;
} //namespace skeet::client

// Declarations from source_terminal_cleanup.hpp

namespace skeet::script {
struct Bridge;
struct BrowserState;
struct ReferenceStrings;
class UiBindings;
class ScriptRuntime;
struct ScriptModulesBindings;
class SourceScriptRuntimeOwner;
}
namespace skeet::lagcomp {
class RecordPool;
struct HistoryRecordingBindings;
}
namespace skeet::client {
class NativeStartupSources;
class PlayerStateCache;
class PlayerRuntimeOwner;
class ClientRuntimeStorage;
struct ExitCallbacks;

struct SourceTerminalCleanupSlot {
    std::uint32_t operand, initializer, finalizer;
};
// Relative positions in the existing182-slot CRT plan. Execution remains
//interleaved with every other initializer; this is not a bulk cleanup pass.
std::span<const SourceTerminalCleanupSlot, 8> source_terminal_cleanup_plan() noexcept;

// Explicit source-owner profile. These are the containers used by the source
//producers, never casts of retained-image headers. Keep this context and all
//borrowed process owners alive through the same ExitCallbacks::run invocation.
class SourceTerminalCleanupBindings {
public:
    SourceTerminalCleanupBindings(ClientRuntimeStorage&, script::SourceScriptRuntimeOwner&,
                                  PlayerRuntimeOwner&);
    SourceTerminalCleanupBindings(ExitCallbacks&, script::Bridge&, script::BrowserState&,
                                  script::UiBindings&, script::ReferenceStrings&, PlayerStateCache&,
                                  lagcomp::RecordPool&);
    SourceTerminalCleanupBindings(const SourceTerminalCleanupBindings&) = delete;
    SourceTerminalCleanupBindings& operator=(const SourceTerminalCleanupBindings&) = delete;
    SourceTerminalCleanupBindings(SourceTerminalCleanupBindings&&) = delete;
    SourceTerminalCleanupBindings& operator=(SourceTerminalCleanupBindings&&) = delete;

    ExitCallbacks& callbacks;
    script::Bridge& bridge;
    script::BrowserState& browser;
    script::UiBindings& ui;
    script::ReferenceStrings& strings;
    PlayerStateCache& players;
    lagcomp::RecordPool& records;

    // Validate late factory aliases once engine/history/modules are composed.
    // The canonical owner constructor permits earlier CRT exit registration;
    // the explicit standalone constructor requires this provenance check.
    void require_producers(const script::ScriptRuntime&, const script::ScriptModulesBindings&,
                           const lagcomp::HistoryRecordingBindings&);
    void initialize(std::uint32_t native_initializer);
    void finalize(std::uint32_t native_finalizer);
    void (*source_finalizer(std::uint32_t native_finalizer))();

private:
    friend void bind_source_terminal_cleanup(NativeStartupSources&, SourceTerminalCleanupBindings&);
    bool producers_validated_ = false;
    bool source_owners_validated_ = false;
};

// Source bootstrap selects this profile before any CRT exit registration.
// It replaces only the eight source-owned CRT bodies; it must not be combined
//with executed native-image cleanup registrations for those same owners.
// NativeStartupSourceGraph selects it after its other binders. A staged source
// startup can select the already validated process-owner context before engine
// acquisition, then validate its late producer aliases after composition.
void bind_source_terminal_cleanup(NativeStartupSources&, SourceTerminalCleanupBindings&);
} //namespace skeet::client

// Declarations from startup.hpp

namespace skeet::engine {
struct UnloadState;
}
namespace skeet::input {
class WindowSubclassRuntime;
}
namespace skeet {
class ConfigStorageOwner;
}
namespace skeet::script {
class ScriptOwner;
}
namespace skeet::ui {
class IconCache;
class RendererDeviceListener;
class RootMenu;
class MenuWindow;
struct StartupIcons;
}

namespace skeet::client {
class ShutdownRuntime;
#if defined(_M_IX86) || defined(__i386__)
using StartupFactory = void*(__cdecl*)();
using StartupCommandLine = const char*(__stdcall*)();
#else
using StartupFactory = void* (*)();
using StartupCommandLine = const char* (*)();
#endif

struct StartupImports {
    StartupCommandLine get_command_line_a;
};
StartupImports native_startup_imports();
// The startup, unload and timer finalization paths share this MenuWindow's
// resolved HWND cell; the separate subclass installation state is transient.
void install_startup_window(ui::MenuWindow&, input::WindowSubclassRuntime&);

// The optional owner43467C04 points to this native vector through its first
// field. Its count and storage are reread after each virtual0 notification.
struct StartupRefreshVector {
    void** objects;
    std::int32_t unknown_4, unknown_8, count;
};
struct StartupRefreshSource {
    StartupRefreshVector* vector;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(StartupRefreshVector, count) == 0xc);
#endif

struct PostInitializationState {
    StartupFactory& first_factory; // 434678C4, actual resolved engine function
    void*& first_result; // 43467460
    StartupFactory& second_factory; // 43468B9C, actual resolved engine function
    void*& second_result; // 43476424
    StartupRefreshSource*& refresh_source; // 43467C04
    std::uint8_t& connected; // 43475BD2
};

// Live decoded views of the canonical encoded engine ConVar words.
struct StartupConVars {
    engine::NativeConVarReference at_43467fbc; // ^96BEB183
    engine::NativeConVarReference at_43467130; // ^99854835
    engine::NativeConVarReference at_4346a5b4; // ^7CBE81A1
    engine::NativeConVarReference at_43468ba8; // ^30911EAB
    engine::NativeConVarReference at_434671ac; // ^D8BFAEFF
    engine::NativeConVarReference at_4346747c; // ^03B4819C
    engine::NativeConVarReference at_4346a748; // ^7D9DC86F, saved dword+60 around float setter
    engine::NativeConVarReference at_4346a330; // ^BE64D223
};

struct PostInitializationBindings {
    engine::UnloadState& hooks;
    ui::RendererDeviceListener& renderer_listener;
    ui::RootMenu& menu;
    ui::IconCache& icons;
    ui::StartupIcons& startup_icons;
    input::WindowSubclassRuntime& window;
    CommandResetState command_reset;
    std::span<StartupAcknowledgement, 150> acknowledgements; // 43475BE0
    std::span<const std::uint32_t, 128> startup_scripts; // mainmenu+120
    ConfigStorageOwner& configuration_storage;
    script::ScriptOwner& scripts;
    ShutdownRuntime& finalizer;
};

// Native4341F57F..4341F7CD, after the recovered menu and engine bindings exist.
// This entry does not stand in for the earlier protected binding scheduler.
class PostInitializationRuntime {
public:
    PostInitializationRuntime(PostInitializationState state, StartupConVars variables,
                              PostInitializationBindings bindings, StartupImports imports);
    void configure_variables(); // 4341F652..4341F740
    void initialize(); // complete valid-code-pointer path4341F57F
private:
    PostInitializationState state_;
    StartupConVars variables_;
    PostInitializationBindings bindings_;
    StartupImports imports_;
};
}

// Declarations from startup_bindings.hpp

namespace skeet::ui {
class MainMenuWindowOwner;
class StartupRenderer;
}
namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;

// The reset paths, packet history and post-initialization callbacks borrow
// these same process cells. Constructing the view does not reset live state.
class PostInitializationData {
public:
    explicit PostInitializationData(std::shared_ptr<engine::NativeDataImage>);
    engine::NativeDataImage& data() const noexcept {
        return *data_;
    }
    PostInitializationState state() const;
    CommandResetState command_reset() const;
    std::span<StartupAcknowledgement, 150> acknowledgements() const;

private:
    std::shared_ptr<engine::NativeDataImage> data_;
};

struct PostInitializationImports {
    StartupImports startup;
    ui::WindowLifetimeImports window_lifetime;
};
PostInitializationImports native_post_initialization_imports();

// Compose the real4341F57F source callback after the menu and script owners
// exist. Construction binds dependencies only; the focus callback invokes
// runtime().initialize() at its actual position in engine startup.
class PostInitializationOwner {
public:
    PostInitializationOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                            ui::MainMenuWindowOwner&, ui::StartupRenderer&, ui::IconCache&,
                            ui::StartupIcons&, input::WindowSubclassRuntime&, ConfigStorageOwner&,
                            script::ScriptOwner&, ShutdownRuntime&);
    PostInitializationOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                            ui::MainMenuWindowOwner&, ui::StartupRenderer&, ui::IconCache&,
                            ui::StartupIcons&, input::WindowSubclassRuntime&, ConfigStorageOwner&,
                            script::ScriptOwner&, ShutdownRuntime&, PostInitializationImports);
    PostInitializationOwner(const PostInitializationOwner&) = delete;
    PostInitializationOwner& operator=(const PostInitializationOwner&) = delete;
    PostInitializationOwner(PostInitializationOwner&&) = delete;
    PostInitializationOwner& operator=(PostInitializationOwner&&) = delete;
    PostInitializationRuntime& runtime() noexcept {
        return runtime_;
    }
    ui::RootMenu& menu() const noexcept {
        return menu_;
    }
    engine::NativeDataImage& data() const noexcept {
        return data_.data();
    }

private:
    PostInitializationData data_;
    ui::RootMenu& menu_;
    PostInitializationRuntime runtime_;
};
} //namespace skeet::client

// Declarations from startup_reset.hpp

namespace skeet::client {

// Views of shared globals; command, targeting and networking consumers retain
// their original storage. The startup/reset paths do not own separate copies.
struct TargetResetState {
    std::int32_t& handle; // 4346DB1C
    std::uint32_t& value_2c; // 4346DB2C
    std::uint32_t& value_30; // 4346DB30
    std::uint32_t& value_38; // 4346DB38
};
struct CommandResetState {
    std::uint32_t& flags; // 434759C0
    std::int32_t& value_c4; // 434759C4
    std::int32_t& value_c8; // 434759C8
    std::int32_t& value_cc; // 434759CC
    std::int32_t& value_dc; // 434759DC
    std::int32_t& value_e8; // 434759E8
    std::int32_t& value_f8; // 434759F8
    std::span<std::uint32_t, 150> inverted_commands; // 43476090
    TargetResetState target;
};

void reset_target_state(TargetResetState state) noexcept; // 4341D854
void reset_command_state(CommandResetState state) noexcept; // 433D6630

// 4341F780..4341F790 clears only the first dword of each eight-byte record.
using StartupAcknowledgement = std::array<std::uint32_t, 2>;
void reset_startup_acknowledgements(std::span<StartupAcknowledgement, 150> records) noexcept;
}

// Declarations from static_cleanup.hpp

namespace skeet::rage {
class SpreadStorage;
struct SpreadStorageBindings;
}
namespace skeet::network {
class DataUpdateObserverRegistry;
class ExploitRechargeObserverRegistry;
}
namespace skeet::prediction {
struct ObserverRegistry;
}
namespace skeet {
class PreservedConfigStorage;
}
namespace skeet::client {
class CommandEmptyTickRegistry;
class CommandPhaseRegistry;
struct EntityObserverRegistry;

// The retained-image registry views are the same cells used by construction,
// dispatch, unload and ShutdownRuntime. Finalizers release their pointer
// buffers; they do not destroy borrowed owners or clear begin/end/limit.
struct StaticCleanupBindings {
    engine::NativeOwnerRegistryStorage& owner_registries;
    network::DataUpdateObserverRegistry& data_update_observers; //43475760/64/68
    network::ExploitRechargeObserverRegistry& recharge_observers; //43475784/88/8C
    prediction::ObserverRegistry& prediction_observers; //43475AB0/B4/B8
    CommandEmptyTickRegistry& empty_tick_observers; //43475978/7C/80
    CommandPhaseRegistry& command_observers; //434757A0/A4/A8
    EntityObserverRegistry& entity_observers; //43475BA8/AC/B0
    rage::SpreadStorage& spread; //43478C60/43478C64, also consumed by aiming
    const rage::SpreadStorageBindings& spread_bindings;
    PreservedConfigStorage& configuration; //live43478CB8 baseline /43478C68 owner
    engine::JobMemory memory;
};

// This binding and every borrowed owner must outlive ExitCallbacks::run().
// Its no-argument callbacks share one process-lifetime source binding; no
// captured payload address is ever converted into an executable callback.
class StaticCleanupRuntime {
public:
    explicit StaticCleanupRuntime(StaticCleanupBindings);
    StaticCleanupRuntime(const StaticCleanupRuntime&) = delete;
    StaticCleanupRuntime& operator=(const StaticCleanupRuntime&) = delete;
    StaticCleanupRuntime(StaticCleanupRuntime&&) = delete;
    StaticCleanupRuntime& operator=(StaticCleanupRuntime&&) = delete;

    // Bind an initializer only when this reports an implemented finalizer
    // for its actual canonical storage. Other targets remain unmapped.
    bool has_source_callback(std::uint32_t native_target) const noexcept;
    void (*source_callback(std::uint32_t native_target) const)();
    void initialize_spread_storage() const; //43402E73, before registering4337D954
    engine::NativeOwnerRegistryStorage& owner_registry_storage() const noexcept {
        return bindings_.owner_registries;
    }

private:
    StaticCleanupBindings bindings_;
    void free_allocation(void*) const;
    void free_registry_storage(engine::OwnerRegistry&) const;
    static const StaticCleanupRuntime& bound();
    static void free_main_owners(); //433EB717
    static void free_hud_callbacks(); //43347D44
    static void free_network_callbacks(); //433F0746
    static void free_update_callbacks(); //433C9DB2
    static void free_packet_callbacks(); //433F4C95
    static void free_frame_callbacks(); //433D1E9F
    static void free_map_callbacks(); //433C9173
    static void free_blur_callbacks(); //43336AD4
    static void free_event_callbacks(); //43341B3F
    static void free_data_update_observers(); //433F4A1D
    static void free_recharge_observers(); //433E6194
    static void free_prediction_observers(); //433F515B
    static void free_empty_tick_observers(); //43349ECB
    static void free_command_observers(); //433E4A38
    static void free_entity_observers(); //433E4C98
    static void free_feedback_position_callbacks(); //433B8C06, buffer43475930
    static void free_spread_tables(); //4337D954 ->433AD1CD
    static void free_config_baseline(); //43329A5B
    static void free_config_settings(); //4332EFAD
};
}

// Declarations from static_crt_initialization.hpp

namespace skeet::engine {
class NativeDataImage;
}
namespace skeet::ui {
class IconCache;
}
namespace skeet::client {
class NativeStartupSources;
struct ExitCallbacks;

// Canonical image storage for eleven terminal pointer-buffer finalizers and
// the two ordinary icon/entity-slot initializers. This context and its owners
// remain alive until the common ExitCallbacks pass has completed.
struct StaticCrtInitialization {
    std::shared_ptr<engine::NativeDataImage> data;
    ExitCallbacks& callbacks;
    engine::JobMemory memory;
    ui::IconCache& icons;
    // The native constructors store volatile incoming ECX in these otherwise
    // unconsumed words. Callers supply explicit source initialization inputs;
    // these fields are not decoded values or captured-register substitutes.
    std::uint32_t flags_constructor_word;
    std::uint32_t keys_constructor_word;

    void initialize_icon_cache(); //433689B6
    void initialize_entity_slots(); //433B0BCE
    void finalize_pointer_storage(std::uint32_t native_finalizer) const;
    void (*source_finalizer(std::uint32_t native_finalizer))();
};

// These registrations invoke no finalizer and retain the scheduler's order.
// Every callback names a concrete source body; no native identifier is called.
void bind_static_crt_initializers(NativeStartupSources&, StaticCrtInitialization&);
} //namespace skeet::client

// Declarations from target_mask_lifecycle.hpp

namespace skeet::client {
class NativeStartupSources;
class TargetMaskLifecycleOwner;

struct TargetMaskOwnerRegistries {
    engine::OwnerRegistryBinding primary; //433676A0, 43475B78
    CommandPhaseRegistry& commands; //43345EBC, 434757A0
    engine::OwnerRegistryBinding messages; //4336F73C, 43475B94
    EntityObserverRegistry& entities; //4333BF32, 43475BA8
    engine::OwnerRegistryBinding hud; //433D20B5, 43475A9C
};
struct TargetMaskFactoryBindings {
    TargetMaskOwnerRegistries registries;
    TargetMaskOwner& command;
    TargetMaskSettings& settings; //same descriptor borrowed by CRT43490C68
    std::shared_ptr<engine::NativeDataImage> data;
};

inline constexpr std::uint32_t target_mask_factory = 0x4348066Au;
inline constexpr std::uint32_t target_mask_constructor = 0x434806CEu;
inline constexpr std::uint32_t target_mask_primary_vtable = 0x43470114u;
inline constexpr std::size_t target_mask_native_size = 0x63Cu;

std::size_t target_mask_lifecycle_size() noexcept;
// Whole ordinary434806CE..4348075F. All five interfaces borrow command.state;
// primary, command, SendNetMsg, BA8 and HUD registration retain that order.
// Constructor writes only its proven scalar/mask fields, preserving record
// ticks and the high three bytes of accepted_tick on caller-provided storage.
TargetMaskLifecycleOwner& construct_target_mask_lifecycle(void* storage,
                                                          const TargetMaskFactoryBindings&);

//4348066A successful allocation/tail-jump path. Uses the exact0x63C native
// zeroed allocation and projects those zeros into the one borrowed state.
// The null edge enters original INT1/junk at4348067F; no ordinary continuation
// or publication is proved. Source stops explicitly at that unfinished edge.
TargetMaskLifecycleOwner* create_target_mask_lifecycle(const TargetMaskFactoryBindings&);

// Primary43470114 slot0 is433B1A7A ->433837D0: header-free only, with no
// owner publication clear, state reset or registry removal. Slot6 is the
// existing complete4332F1D9 reset of the same canonical command owner.
void destroy_target_mask_lifecycle(TargetMaskLifecycleOwner*);
void reset_target_mask_lifecycle(TargetMaskLifecycleOwner&) noexcept;
void* target_mask_lifecycle_object(TargetMaskLifecycleOwner&) noexcept;
TargetMaskOwner& target_mask_command_owner(TargetMaskLifecycleOwner&) noexcept;

// Register only the actual4348066A factory identity. Startup's live operand
// order selects when to invoke it; this binding does not construct an owner.
void bind_native_target_mask_factory(NativeStartupSources&, TargetMaskFactoryBindings&);
} // namespace skeet::client

// Declarations from target_mask_owner.hpp

namespace skeet::client {

inline constexpr std::uint32_t target_mask_feature_hash = 0x347A4FA6u;

// Canonical static descriptor43467974, value43467984. CRT43490C68 appends
// this field without changing metadata or its stored value. Keep one settings
// object alive through startup and every TargetMaskOwner that borrows it.
struct TargetMaskSettings {
    BooleanConfig enabled{{{nullptr, nullptr, target_mask_feature_hash}, 1, ConfigType::boolean, 0},
                          0};
};

struct TargetMaskRecord {
    std::int32_t receive_tick = 0;
    std::array<std::uint32_t, 2> targets{};
};
static_assert(sizeof(TargetMaskRecord) == 12);

// One canonical owner shared by command table43470478, message observer
// table4346F334, entity-update table4346F98C and reset table43470114.
// Offsets below refer to its command interface, which is full-owner+8.
struct TargetMaskState {
    std::int32_t accepted_tick = 0; // +1C
    std::uint8_t has_accepted_tick = 0; // +20
    std::uint8_t attack_pending = 0; // +24
    std::uint32_t first = 0, count = 0; // +2C/+30
    std::array<TargetMaskRecord, 128> records{}; // +34
};

struct TargetMaskBindings {
    void* engine_client; // 4346A514, slots14/82/93
    const float* receive_interval; // [43467B04]+20
    const std::uint8_t* rage_enabled; // 43467224
    engine::ConVarBinding lag_compensation; //live [43467A78] XOR3AE06513
    const double* real_latency_outgoing; // 4347A500
    const double* real_latency_incoming; // 4347B118
};

class TargetMaskOwner {
public:
    // Startup bindings exist before CRT43490C68 appends this descriptor.
    // Borrow its canonical value directly without registering or resetting it.
    TargetMaskOwner(TargetMaskSettings&, TargetMaskBindings) noexcept;
    // The native feature is Boolean descriptor43467974, value43467984.
    // Bind its registered hash/type rather than manufacturing another switch.
    TargetMaskOwner(ConfigRegistries&, TargetMaskBindings);
    TargetMaskState state;
    const std::uint8_t& enabled;
    TargetMaskBindings bindings;
};

// Complete owner reset4332F1D9. Recorded payloads and accepted_tick survive;
// their head/count and the two validity/attack bytes are cleared.
void reset_target_mask_owner(TargetMaskOwner&) noexcept;

// Complete4336AC2E, entity-update interface slot0. Incoming status uses its
// actual flags+24 and byte+10; this reset preserves the attack bypass byte.
void observe_target_mask_update_status(TargetMaskOwner&, const void* native_status) noexcept;

// Complete4336526D, entity-update interface slot1. The unchanged incoming
// stack argument points to an object whose +8 is the visibility payload.
// Entity indices1..64 map to bit(index&63), including64 -> bit0.
void capture_target_mask_update(TargetMaskOwner&, std::uintptr_t incoming);

// Complete4332CC6A, observer interface slot0. The real message's virtual7
// identifies type4; its tick+0C is updated in place and retained in this owner.
void process_target_mask_message(TargetMaskOwner&, void* native_message);
// Native observer EDX holds a context whose +4 is that message pointer.
void process_target_mask_message_context(TargetMaskOwner&, const void* native_context);

// Command table43470478 phases4/7, complete4340029E and43386E3E.
void build_command_target_mask(TargetMaskOwner&, CommandFrame&) noexcept;
void mark_target_mask_attack(TargetMaskOwner&, const CommandFrame&, const Command&) noexcept;

} // namespace skeet::client

// Declarations from target_mask_runtime_owner.hpp

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
struct EngineRuntimeBindings;
class TargetMaskRuntimeOwner final {
public:
    TargetMaskRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                           ConfigurationRuntimeOwner&);
    TargetMaskRuntimeOwner(const TargetMaskRuntimeOwner&) = delete;
    TargetMaskRuntimeOwner& operator=(const TargetMaskRuntimeOwner&) = delete;
    ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    TargetMaskOwner& command() noexcept {
        return command_;
    }
    TargetMaskFactoryBindings& factory_bindings() noexcept {
        return factories_;
    }
    void bind_initializers(NativeStartupSources&);

private:
    ClientRuntimeStorage& storage_;
    TargetMaskOwner command_;
    TargetMaskFactoryBindings factories_;
};
}

// Declarations from target_refresh_runtime_owner.hpp

namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::anti_aim {
struct TargetState;
}
namespace skeet::engine {
class SceneQueryRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class PlayerRuntimeOwner;
class HistoryRuntimeOwner;
class MovementHudRuntimeOwner;
struct EngineRuntimeBindings;
struct Command;

//4335BA8E's shared production selector. All phase inputs are supplied at the
// actual call; canonical history, resource, clock and ConVar cells remain live.
// Construction calls no selector, engine method, owner factory or registration.
class TargetRefreshRuntimeOwner final {
public:
    TargetRefreshRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                              PlayerRuntimeOwner&, HistoryRuntimeOwner&,
                              engine::SceneQueryRuntimeOwner&, animation::RuntimeOwner&,
                              MovementHudRuntimeOwner&);
    ~TargetRefreshRuntimeOwner();
    TargetRefreshRuntimeOwner(const TargetRefreshRuntimeOwner&) = delete;
    TargetRefreshRuntimeOwner& operator=(const TargetRefreshRuntimeOwner&) = delete;
    TargetRefreshRuntimeOwner(TargetRefreshRuntimeOwner&&) = delete;
    TargetRefreshRuntimeOwner& operator=(TargetRefreshRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    void refresh(anti_aim::TargetState&, void* current_player, const Command&,
                 std::uint32_t incoming_buttons, float incoming_yaw) const;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client

// Declarations from verified_command.hpp

namespace skeet::client {
struct VerifiedCommand {
    Command command;
    std::uint32_t checksum;
};
static_assert(sizeof(VerifiedCommand) == 0x68);
using CommandChecksum = engine::MemberFunction<std::uint32_t>;
//4335849B..433584DD: snapshot is prepared by save_final_command first;
// the optional actual engine43467C48 getter receives the verified entry.
void publish_verified_command(VerifiedCommand&, const Command&, CommandChecksum);
// Complete433D11F7. The supplied object is the genuine prediction interface.
void invalidate_command_prediction(void*) noexcept;
} // namespace skeet::client

// Declarations from visual_runtime_owner.hpp

namespace skeet::ui {
class StartupRenderer;
class RendererAssets;
class RendererBackend;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class ShutdownRuntime;
struct EngineRuntimeBindings;
struct GameSystemFactoryContext;

// Shared source views of the game-system projection and the real renderer.
// Feature factories borrow these stable objects rather than making independent
// camera, viewport, icon or indicator snapshots. Construction installs no hook
// and runs no feature constructor or game-system callback.
class VisualRuntimeOwner {
public:
    VisualRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                       ConfigurationRuntimeOwner&, ui::StartupRenderer&, ui::RendererAssets&);
    ~VisualRuntimeOwner();
    VisualRuntimeOwner(const VisualRuntimeOwner&) = delete;
    VisualRuntimeOwner& operator=(const VisualRuntimeOwner&) = delete;
    VisualRuntimeOwner(VisualRuntimeOwner&&) = delete;
    VisualRuntimeOwner& operator=(VisualRuntimeOwner&&) = delete;

    ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    ui::StartupRenderer& startup_renderer() const noexcept {
        return renderer_;
    }
    ui::RendererBackend& renderer() const;
    esp::Projection& projection() noexcept {
        return projection_;
    }
    const Point& screen_size() const noexcept {
        return screen_size_;
    }
    const esp::IndicatorLayout& indicators() const noexcept {
        return indicators_;
    }
    const esp::GrenadeWarningTexture& explosive_texture() const noexcept {
        return explosive_;
    }
    const esp::GrenadeWarningTexture& fire_texture() const noexcept {
        return fire_;
    }
    const effects::ProjectileRibbonBindings& ribbons() const noexcept {
        return ribbons_;
    }
    void* key_values_system() const noexcept {
        return key_values_;
    }
    engine::GameSystemCameraState camera_state() noexcept;

    // Refresh the source HUD views at its real renderer capture cut. Texture
    // dimensions used by indicators are still queried from the backend at each
    // native metric operation, including textures added by a Lua callback.
    void refresh();

    // Shutdown borrows the already constructed menu/window. Compose this part
    // after that real owner exists and before binding WorldRuntime factories;
    // no substitute shutdown context is created to break the lifetime cycle.
    void initialize_game_system(ShutdownRuntime&);
    engine::GameSystemRuntime& game_system();
    GameSystemFactoryContext game_system_factory();

private:
    ClientRuntimeStorage& storage_;
    ConfigurationRuntimeOwner& configuration_;
    ui::StartupRenderer& renderer_;
    ui::RendererAssets& assets_;
    void* key_values_;
    effects::ProjectileRibbonBindings ribbons_;
    esp::Projection projection_;
    Point screen_size_{};
    esp::IndicatorLayout indicators_;
    esp::GrenadeWarningTexture explosive_, fire_;
    std::unique_ptr<engine::GameSystemRuntime> game_system_;
};
} // namespace skeet::client

// Declarations from world_runtime_factories.hpp

namespace skeet::client {
class NativeStartupSources;
class CommandPhaseRegistry;

struct GameSystemFactoryContext {
    engine::GameSystemManager* const& manager; //live43467FB0, captured by the constructor
    void* const* allocator; //address of live43467204
    engine::GameSystemRuntime& runtime;
};
struct RevealRanksFactoryContext {
    misc::RevealRanksConfig& settings;
    misc::RevealRanksRuntime*& active; //canonical434758F0 source projection
    misc::RevealRanksBindings engine;
};
struct RevealOverwatchFactoryContext {
    misc::RevealOverwatchConfig& settings;
    void* const& engine; //4346A514
};
struct DisableSvPureFactoryContext {
    misc::DisablePureConfig& settings; //same177AB88C value as GameSystemRuntime
    misc::DisableSvPureOwner*& active; //43475974
};
struct GameSystemRegistrationFactoryContext {
    engine::GameSystemRegistrationNode** const* head_slot; //address of43468D50
    const engine::OwnerRegistry& callbacks; //43475A84/88/8C
    engine::UnloadState& unload;
};
struct PersistentKillFeedFactoryContext {
    misc::PersistentKillFeedConfig& settings;
    misc::PersistentKillFeedBindings& engine;
};
struct HitMarkerFactoryContext {
    events::EventRegistry& events; //the existing43478DD0 permanent-listener registry
    events::HitMarkerOwnerBindings bindings;
};
struct NativeWindowFactoryContext {
    SettingsPersistence& persistence;
    input::NativeWindowLifecycleImports imports;
    engine::LoadedModuleEnvironment modules;
    engine::NativeImageEnvironment images;
};

// These are borrowed canonical registries/settings and existing complete
// constructor contexts. Keep this aggregate and its referents alive until
// lifecycle destruction has finished. Descriptor initialization belongs to
// each whole constructor; do not initialize a second feature configuration.
struct WorldRuntimeFactoryBindings {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    engine::OwnerRegistryBinding updates; //43475988
    engine::OwnerRegistryBinding hud; //43475A9C
    engine::OwnerRegistryBinding frames; //43475AD8
    engine::OwnerRegistryBinding event_observers; //434758E0
    engine::OwnerRegistryBinding messages; //43475B94
    CommandPhaseRegistry& command_owners; //canonical434757A0/A4/A8
    ConfigRegistries& configuration;
    effects::SmokeOwnerBindings smoke;
    effects::ScopeOwnerBindings scope;
    GameSystemFactoryContext game_system;
    misc::AutoAcceptOwnerBindings auto_accept;
    RevealRanksFactoryContext reveal_ranks;
    RevealOverwatchFactoryContext reveal_overwatch;
    DisableSvPureFactoryContext disable_sv_pure;
    GameSystemRegistrationFactoryContext game_system_registration;
    PersistentKillFeedFactoryContext persistent_kill_feed;
    HitMarkerFactoryContext hit_markers;
    NativeWindowFactoryContext window_lifecycle;
    effects::HarmlessMolotovOwnerBindings harmless_molotov;
};

// Twelve exact factory identities. Runtime-owner initialization supplies their
// order; this only binds ordinary source implementations. The game-window
// proxy at4347D94F has its separate concrete GameWindowProxyFactoryBindings.
void bind_world_runtime_factories(NativeStartupSources&, WorldRuntimeFactoryBindings&);
} //namespace skeet::client

// Declarations from world_runtime_owner.hpp

namespace skeet {
class ConfigStorageOwner;
}
namespace skeet::effects {
class EffectsRuntimeOwner;
}
namespace skeet::engine {
class HookRuntimeOwner;
}
namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class VisualRuntimeOwner;
class WindowRuntimeOwner;
struct EngineRuntimeBindings;
struct WorldRuntimeImports {
    input::NativeWindowLifecycleImports window;
    engine::GameWindowProxyImports proxy;
};
WorldRuntimeImports native_world_runtime_imports();

// Stable contexts for the twelve complete world factories and the genuine
// game-window proxy. Construction binds existing process owners only; their
// factories retain the startup scheduler's allocation/registration order.
class WorldRuntimeOwner {
public:
    WorldRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                      ConfigurationRuntimeOwner&, HistoryRuntimeOwner&, VisualRuntimeOwner&,
                      effects::EffectsRuntimeOwner&, engine::HookRuntimeOwner&, WindowRuntimeOwner&,
                      ConfigStorageOwner&);
    WorldRuntimeOwner(ClientRuntimeStorage&, const EngineRuntimeBindings&,
                      ConfigurationRuntimeOwner&, HistoryRuntimeOwner&, VisualRuntimeOwner&,
                      effects::EffectsRuntimeOwner&, engine::HookRuntimeOwner&, WindowRuntimeOwner&,
                      ConfigStorageOwner&, WorldRuntimeImports);
    ~WorldRuntimeOwner();
    WorldRuntimeOwner(const WorldRuntimeOwner&) = delete;
    WorldRuntimeOwner& operator=(const WorldRuntimeOwner&) = delete;
    WorldRuntimeOwner(WorldRuntimeOwner&&) = delete;
    WorldRuntimeOwner& operator=(WorldRuntimeOwner&&) = delete;
    ClientRuntimeStorage& storage() const noexcept;
    WorldRuntimeFactoryBindings& factory_bindings() noexcept;
    GameWindowProxyFactoryBindings& proxy_factory_bindings() noexcept;
    const effects::HarmlessMolotovBindings& molotov_bindings() const noexcept;
    events::HitMarkerBindings& hit_marker_bindings() noexcept;
    void bind_initializers(NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::client
