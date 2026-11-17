#pragma once

// CS:GO Interfaces & Netvars Declarations
#include <span>
#include <string>
#include <string_view>
#include <vector>
#include "skeet/engine/netvars.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/prediction/command.hpp"
#include "skeet/lagcomp/entity_state.hpp"
#include "skeet/effects/glow.hpp"
#include "skeet/rage/spread.hpp"
#include "skeet/rage_hitchance.hpp"
#include "skeet/legit/trigger_trace.hpp"
#include "skeet/animation/local_helpers.hpp"
#include "skeet/misc/network_fix.hpp"
#include "skeet/effects/model_render.hpp"
#include "skeet/misc/automatic_weapons.hpp"
#include "skeet/misc/clan_tag.hpp"
#include <array>
#include "skeet/network/packet_world.hpp"
#include <stdexcept>
#include <cstdint>
#include "skeet/client/entity_update_dispatch.hpp"
#include "skeet/script/panorama_api.hpp"
#include <cstddef>
#include <bit>
#include "skeet/engine/loaded_modules.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/esp/projection.hpp"
#include <algorithm>
#include <xmmintrin.h>
#include "skeet/engine/preserved_initialization.hpp"
#include "skeet/engine/unload.hpp"
#include <memory>
#include <type_traits>
#include <unordered_map>
#include <cstdio>
#include <cstring>
#include "skeet/engine/bindings.hpp"
#include "skeet/movement/engine_collision.hpp"
#include "skeet/rage/engine_ballistics.hpp"
#include "skeet/esp/grenade_engine.hpp"
#include "skeet/esp/grenade_environment.hpp"
#include "skeet/misc/native_text.hpp"
#include <variant>
#include "skeet/engine/jobs.hpp"


// Declarations from animation_startup_patterns.hpp

namespace skeet::engine {
// Recovered last-startup operation: search the actual animation Update code
// cell434693F0 for C605??????????F30F5F05 within512 bytes, then publish its
// decoded MOV byte-store address to43467B24. Null code or a search miss preserves
// the previous output. The flag's current byte is never changed by startup.
// Its existing animation-update/channel consumers perform their own clears.
void initialize_animation_update_flag(NativeDataImage&);
} // namespace skeet::engine

// Declarations from bindings.hpp

namespace skeet::engine {

#if defined(_M_IX86) || defined(__i386__)
using InterfaceFactory = void*(__cdecl*)(const char*, int*);
#else
using InterfaceFactory = void* (*)(const char*, int*);
#endif

struct InterfaceRequest {
    std::string_view name;
    std::string_view module;
    std::string_view version;
};

struct BoundInterface {
    std::string name;
    std::string module;
    std::string version;
    void* object = nullptr;
};

class FactoryProvider {
public:
    virtual ~FactoryProvider() = default;
    virtual InterfaceFactory factory(std::string_view module) = 0;
};

// Resolves CreateInterface only in an already loaded, explicitly named module.
// There is no module loading, version probing, or alternate-name search.
class LoadedModuleFactories final : public FactoryProvider {
public:
    InterfaceFactory factory(std::string_view module) override;
};

// Integration adapter. The protected native factory manifest and its complete
// initialization sequence have not been recovered by this unit.
class InterfaceBindings {
public:
    // Every request is mandatory. A failed bind preserves the previous set.
    // Interface objects are borrowed from the engine and are never released.
    void bind(std::span<const InterfaceRequest> requests, FactoryProvider& provider);
    void clear() noexcept;
    void* at(std::string_view name) const;
    std::span<const BoundInterface> entries() const noexcept {
        return bindings_;
    }

private:
    std::vector<BoundInterface> bindings_;
};

// Known embedded interface consumers, with the base-client module corrected
// to client.dll by the working dump's registry and creator/object identity.
// This catalog is not the unrecovered native initialization manifest.
std::span<const InterfaceRequest> embedded_interface_consumers() noexcept;

// The existing protected operand accessor0x43493903 is a real external
// dependency. This interface exposes its values without replacing its logic.
class InitializationMetadata {
public:
    virtual ~InitializationMetadata() = default;
    virtual std::uintptr_t operand(std::uint32_t address) = 0;
};

struct DiscoveredDataMaps {
    DataMap* c69c5c29 = nullptr; // native global43477B50
    DataMap* a9cd7264 = nullptr; // native global43477B4C
};

// Complete native scheduler0x43407CDE. Recipe reads retain their native order;
// the operand accessor and loaded client module are supplied by initialization.
void initialize_native_netvars(void* base_client, std::byte* client_module,
                               InitializationMetadata& metadata, std::uint32_t*& cursor,
                               DiscoveredDataMaps& maps);

} // namespace skeet::engine

// Declarations from build_animation_bindings.hpp

namespace skeet::engine {
// Observed client 65272025 / tier0 63DC10BA build binding policy. It finds actual
// loaded modules, verifies their identities/sections/code and follows native
// animation call operands. ThreadInMainThread is obtained from tier0's named
// export and checked against the client's actual import slot.
//
// Publishes 4346A38C,4346A6B0,43469060,43476920,43468F80 and returns that same typed
// attachment entry for subsequent BoneRuntime construction. Other consumers
// borrow the cells directly. Every target exists in this build: unsupported
// or missing recovery is an error, rather than an invented optional null.
// Runs no client/tier0 function and loads no module. Call before bone consumers;
// both genuine modules must remain loaded throughout their use.
animation::BoneAttachmentUpdate initialize_build_animation_bindings(NativeDataImage&,
                                                                    const LoadedModuleEnvironment&);

// Narrow prerequisite for the startup search over the native animation Update
// body. Two real client call sites independently supply the animation-state
// receiver and XMM1/XMM2 angles to this same RET4 entry. Publishes only434693F0;
// returns readable genuine client code without adapting or calling its SIMD ABI.
// Subsequent startup pattern-search outputs belong to their own operation.
const std::byte* initialize_build_animation_update_code(NativeDataImage&,
                                                        const LoadedModuleEnvironment&);

// Next startup code input43467F0C. Follows Update's real direct call to its
// vector helper, then verifies the interior block and branch to the32-byte
// hash window at block+4C. The result is inspected code, not a callable entry.
// Publishes only43467F0C and performs no hash-result or later VM publication.
const std::byte* initialize_build_animation_velocity_code(NativeDataImage&,
                                                          const LoadedModuleEnvironment&);
} // namespace skeet::engine

// Declarations from build_client_state_aliases.hpp

namespace skeet::engine {
struct BuildClientStateAliases {
    std::int32_t* choked_commands;
    std::int32_t* last_outgoing_command;
    std::int32_t* server_tick;
    std::int32_t* command_tick;
    float* real_time;
    std::int32_t* tick_count;
    std::int32_t* maximum_clients;
    std::int32_t* acknowledged_tick;
    std::int32_t* outgoing_tick;
};

// Run after initialize_build_game_state_bindings. Verify the actual engine
// build and its current-record/member operands before publishing the live
// client-state/global aliases. No pointed tick, command or time is copied.
// The acknowledged/outgoing cells name engine-owned counters, not commands.
// Also publishes434758D8 as the current record's delta-tick member, shared by
// SendNetMsg, incoming updates, prediction repair and local-player creation.
// Publishes43475A50 as the engine's pending event queue used by FireEvents.
BuildClientStateAliases initialize_build_client_state_aliases(NativeDataImage&);
} // namespace skeet::engine

// Declarations from build_command_bindings.hpp

namespace skeet::engine {
struct BuildCommandBindings {
    prediction::SeedGenerator seed_generator;
    void* move_helper;
    std::int32_t* random_seed;
    void** prediction_player;
    prediction::PredictionFlagNode** flag_head;
};
// Supported client65272025 only. Actual ordinary engine code independently
// supplies the command seed ABI and four static aliases; the working dump's
// globals match these operands. Validates PE/code/RET/RTTI provenance,
// then publishes434679AC/43468EC8/43468328/434693A4/43467C58/4346D660 together.
//4346D660 is the genuine two-word/RET8 predicted-frame restore operation.
// No engine function, constructor or captured client entry executes here.
BuildCommandBindings initialize_build_command_bindings(NativeDataImage&);
} //namespace skeet::engine

// Declarations from build_entity_animation_bindings.hpp

namespace skeet::engine {
struct NativeOffsetStorage;
struct BuildEntityAnimationBindings {
    MemberFunction<void, std::int32_t> invalidate_physics;
    lagcomp::SetAbsoluteVector set_absolute_angles, set_absolute_origin;
    lagcomp::SetCollisionBounds set_collision_bounds;
    std::uint8_t* transform_override;
    double* animation_timestamp;
    void** player_resource;
    void* smoke_registry;
    MemberFunction<void> update_render_visibility;
};
// Supported client.dll65272025 only. Verifies PE identity, sections, actual
// function instructions/RET ABI, setter calls and live data operands before
// publishing43468C10/434688C8/43467E44/4346A7F0/434683D0/4346A7E4/434679C0/43468320/43468FA4. Independently
// checks the player-resource slot already published by misc engine bindings.
// This explicit source build policy runs no engine code and loads no module.
// Also resolves the collision-height/time encodings from the engine's paired
// stores and readers. Their shared zero word is not a zero field offset.
// These targets all exist in this build; incomplete recovery is an error.
BuildEntityAnimationBindings
initialize_build_entity_animation_bindings(NativeDataImage&, const LoadedModuleEnvironment&,
                                           NativeOffsetStorage&);
} //namespace skeet::engine

// Declarations from build_esp_bindings.hpp

namespace skeet::engine {
// Genuine five-stack-word engine entry. Its two output pointers are ordinary
// arguments; the function preserves the native RET14 callee cleanup.
using ShotgunPatternEntry = void(__stdcall*)(std::uint32_t, std::int32_t, std::int32_t, float*,
                                             float*);
struct BuildEspBindings {
    NativeDataImage& data;
    effects::GlowEngine glow_engine() const;
    // The adapter borrows the canonical43467DDC cell, not this result object.
    // Keeping the shared NativeDataImage alive preserves its actual context.
    rage::ShotgunPattern shotgun_pattern() const;
};
// Validate the supported loaded client image, unique complete entry anchors,
// helper calls, lookup-data operands, RandomFloat imports and return ABIs.
// Publishes4346832C/43467DDC; no game function or factory executes here.
BuildEspBindings initialize_build_esp_bindings(NativeDataImage&, NativeImageEnvironment);
BuildEspBindings initialize_build_esp_bindings(NativeDataImage&);
} // namespace skeet::engine

// Declarations from build_eye_adjustment_bindings.hpp

namespace skeet::engine {
// Locate the supported client65272025 Studio_SetPoseParameter implementation
// from its PE32 identity, unique instruction body and descriptor-helper edge.
// Publishes only canonical43467CF0; no game function is called here.
void initialize_build_eye_adjustment_bindings(NativeDataImage&);
// The adjacent genuine Studio_GetPoseParameter entry, called by433A0EC6.
// Its descriptor helper and both RET tails are verified independently before
// publishing43468CA4. The normalized input arrives inXMM2, result inXMM0.
void initialize_build_pose_range_bindings(NativeDataImage&);
class PoseRangeRuntime {
public:
    explicit PoseRangeRuntime(const NativeDataImage&);
    float convert(const void* studio_wrapper, std::uint32_t index, float normalized) const;

private:
    const std::uintptr_t& entry_;
};

class EyePoseParameterRuntime {
public:
    explicit EyePoseParameterRuntime(const NativeDataImage&);
    // Genuine engine ABI: ECX=studio wrapper, EDX=index, XMM2=angle,
    // caller-cleaned normalized-output pointer on the stack; result XMM0.
    // The source bridge converts only the float return to ordinary C++ ABI.
    float convert(const void* studio_wrapper, std::uint32_t index, float angle,
                  float& normalized) const;

private:
    const std::uintptr_t& entry_;
};
namespace detail {
// Explicit ABI bridge, exposed internally for authored/Unicorn contract tests.
float invoke_eye_pose_parameter(std::uintptr_t entry, const void* studio_wrapper,
                                std::uint32_t index, float angle, float& normalized);
float invoke_pose_range(std::uintptr_t, const void*, std::uint32_t, float);
}
} //namespace skeet::engine

// Declarations from build_final_startup_bindings.hpp

namespace skeet::engine {
struct BuildFinalStartupBindings {
    const std::byte* player_feedback_instruction;
    std::uint32_t player_feedback_offset;
    void** float_proxy_slot;
    const std::byte* overlay_return;
    void** overlay_stream_slot;
};
// Resolve the final ordinary client instruction, engine decoder table slot
// and overlay instruction operands from their supported loaded PE32 builds.
// Publishes only canonical data cells; no game method or target executes.
BuildFinalStartupBindings initialize_build_final_startup_bindings(NativeDataImage&);
} //namespace skeet::engine

// Declarations from build_game_state_bindings.hpp

namespace skeet::engine {
// Source acquisition for client 65272025 and engine 64EE7E86. The canonical
// module and VClient018/VEngineClient014 interface cells must already be bound.
// Reads the actual loaded PE images and virtual-method instruction operands;
// calls no game function and installs no pointer from a captured process.
//
// Publishes gpGlobals 43467B04, CInput 4346A894, CClientState 43468E44 and the
// gpGlobals member aliases 43467EB8/+10, 434678F8/+4, 4346A338/+20. CClientState
// is the engine's current split-screen record plus eight, stored without XOR.
// All identities, signatures and pointer provenance are checked before the
// first publication. Unsupported builds or missing initialized objects fail.
void initialize_build_game_state_bindings(NativeDataImage&);
// The two genuine client factories consumed by4341F57F: GlowObjectManager
// (434678C4, consumed once) and ItemSystem (43468B9C, retained for skins/icons).
// Verify their actual singleton/constructor code and publish both entries;
// also binds43467C04 to the real lazy refresh-vector pointer slot after
// verifying its static reset, getter and indexed virtual0 notifier loop;
// this operation never calls a factory or copies a captured singleton object.
void initialize_build_client_factories(NativeDataImage&);
} // namespace skeet::engine

// Declarations from build_history_bindings.hpp

namespace skeet::engine {
struct BuildHistoryBindings {
    MemberFunction<std::int32_t, std::int32_t> activity;
    MemberFunction<const char*, std::int32_t> sequence_name;
    void* sequence_displacement; //custom ECX/EDX + caller-cleaned stack ABI
};
// Actual client.dll65272025 functions independently matched to the three
// working-dump globals. Requires the canonical bound client module, verifies
// PE sections, function instructions, direct callees and every return ABI,
// then publishes4346A364/434678C8/43468DC0 together. Runs no client function.
BuildHistoryBindings initialize_build_history_bindings(NativeDataImage&);
} //namespace skeet::engine

// Declarations from build_hitbox_clip_bindings.hpp

namespace skeet::engine {
// Supported client65272025. Verifies the ordinary loaded client function and
// publishes43468C3C. No module load or engine function call is performed.
void* initialize_build_hitbox_clip_bindings(NativeDataImage&);
// Source packing for4339FA8E's genuine external boundary. The entry is read
// from the canonical cell on every call. Its x86 ABI uses ECX=Ray_t,
// EDX=mstudiobbox_t, stack(matrix,trace) and a plain RET with caller cleanup.
RageBoxClipper native_hitbox_clipper(NativeDataImage&);
} //namespace skeet::engine

// Declarations from build_hook_initialization.hpp

namespace skeet::animation {
class FixAnimationsRuntime;
class FixAnimationBackupRuntime;
class RemoteSetupBonesRuntime;
}
namespace skeet::effects {
class EntityNetworkableRuntime;
class HarmlessMolotovRuntime;
class ModelRenderHooks;
}
namespace skeet::misc {
class PingSpikeChannelRuntime;
struct NetworkMessageRuntime;
class ViewmodelHooksRuntime;
class AutoAcceptHookFamily;
}
namespace skeet::network {
class ClientStateHookFamily;
struct PacketDispatchRuntime;
}
namespace skeet::script {
struct ConsoleInputHook;
}
namespace skeet::ui {
class FrameRuntime;
}

namespace skeet::engine {
class PlayerHookFamily;
struct NativeOffsetStorage;
enum class BuildHookDescriptor {
    channel,
    local_player,
    local_thinkable,
    local_renderable,
    local_networkable,
    remote_player,
    remote_thinkable,
    remote_renderable,
    remote_networkable,
    game_event_listener,
    ragdoll_networkable,
    fire_bullets_networkable,
    molotov,
    incendiary,
    viewmodel,
    inferno,
    particle_effect,
    particle_unknown,
    particle_renderable,
    focus_overlay,
    client_state,
    studio,
    leaf_system,
    auto_accept,
    console,
    overlay_stream
};
struct BuildHookDescriptorRecipe {
    BuildHookDescriptor kind;
    std::uint32_t descriptor, module_cell, type_hash, base_hash;
    std::uint32_t descriptor_operand, replacement_operand, replacement_count;
};
struct BuildHookNativeReplacement {
    std::uint32_t slot, native_entry, entry_operand;
};
// Fixed-build metadata, not an implementation of the protected operand reader.
// Existing family manifests remain the sole source of their replacement rows.
std::span<const BuildHookDescriptorRecipe, 26> build_hook_descriptors() noexcept;
std::vector<BuildHookNativeReplacement> build_hook_native_replacements(BuildHookDescriptor);

// Canonical state for the short hooks which previously had no source entry.
// FocusOverlayHooks separately owns the canonical post-init/HUD callbacks.
class BuildHookCallbacks {
public:
    BuildHookCallbacks(HookDescriptorStorage&, VtableHookRegistry&, const NativeOffsetStorage&,
                       animation::FixAnimationsRuntime&, animation::FixAnimationBackupRuntime&,
                       misc::PingSpikeChannelRuntime&, ui::FrameRuntime&);
    void bind(std::uint32_t native_entry, VtableHookMetadata&, std::uint32_t slot);
    static void bind_static(std::uint32_t native_entry, VtableHookMetadata&, std::uint32_t slot);
    std::uintptr_t channel_reset(void*, std::uintptr_t caller);
    void channel_packet(void*, std::uintptr_t, std::uintptr_t);
    void detach_thinkable(void*);
    void detach_networkable(void*, bool local);
    void record_local_frame(void*);
    void before_remote_data_change(void*, std::int32_t);
    std::uintptr_t renderable_shadow(void*);
    std::uintptr_t game_event(void*, void*);
    std::uintptr_t weapon_query(void*);
    std::uintptr_t overlay_stream(void*, std::uintptr_t caller);

private:
    const NativeDataImage& data_;
    const NativeOffsetStorage& offsets_;
    animation::FixAnimationsRuntime& animations_;
    animation::FixAnimationBackupRuntime& backups_;
    misc::PingSpikeChannelRuntime& channel_;
    ui::FrameRuntime& frame_;
};

struct BuildHookFamilies {
    PlayerHookFamily& players;
    animation::FixAnimationsRuntime& animations;
    animation::RemoteSetupBonesRuntime& remote_bones;
    BuildHookCallbacks& callbacks;
    effects::EntityNetworkableRuntime& ragdolls;
    effects::HarmlessMolotovRuntime& particles;
    network::PacketDispatchRuntime& packets;
    misc::PingSpikeChannelRuntime& channel;
    misc::NetworkMessageRuntime& messages;
    misc::ViewmodelHooksRuntime& viewmodels;
    network::ClientStateHookFamily& client_state;
    effects::ModelRenderHooks& models;
    misc::AutoAcceptHookFamily& auto_accept;
    script::ConsoleInputHook& console;
};
// Actual early phase: twenty-six genuine RTTI clones and94 source entry
// addresses, before all57 feature factories and RootMenu. It publishes no
// runtime context, installs no receiver and needs no feature/menu object.
void prepare_build_hook_descriptors(HookDescriptorStorage&, JobMemory, NativeImageEnvironment,
                                    HookDescriptorSystem);
// Later context publication after the actual owners exist. Reuses those same
// descriptors without allocating, querying memory or resolving RTTI again.
// Complete this before installing any prepared table on a real receiver.
void bind_build_hook_runtime_contexts(HookDescriptorStorage&, VtableHookRegistry&,
                                      const BuildHookFamilies&);
// Whole descriptor-production phase. Resolve actual engine RTTI and clone its
// executable prefix, then bind source entries in the native26-group order.
// Actual receiver installation remains in the existing object lifecycles.
// Source startup uses the two phases above at their separate native cuts.
void initialize_build_hooks(HookDescriptorStorage&, VtableHookRegistry&, NativeImageEnvironment,
                            HookDescriptorSystem, const BuildHookFamilies&);
}

// Declarations from build_legit_bindings.hpp

namespace skeet::engine {
//433E7C6B spills XMM0..5 in order, then passes an additional integer1.
// The genuine client function returns AL and leaves all seven words to caller.
using LineSmokeEntry = bool(__cdecl*)(float, float, float, float, float, float, std::int32_t);
struct BuildLegitBindings {
    NativeDataImage& data;
    legit::TriggerTraceBindings trigger_trace() const noexcept;
};
// Supported client65272025 only. Verifies the complete smoke entry's two RET
// tails, smoke-vector/handle-array operands and geometric helper call, then
// publishes canonical4346A82C. No engine/client function executes here.
BuildLegitBindings initialize_build_legit_bindings(NativeDataImage&);
// Complete ordinary433E7C6B. Its missing-entry false result is native behavior;
// a production source bootstrap still acquires the actual supported entry.
bool line_passes_through_smoke(const NativeDataImage&, const legit::Vector&, const legit::Vector&);
} //namespace skeet::engine

// Declarations from build_loader_inspection_bindings.hpp

namespace skeet::engine {
// The supplied loader's four inputs consumed by the last startup VM. Resolve
// against actual canonical engine/material modules and VEngineClient014 cells.
// Supported builds: engine64EE7E86, materialsystem63DC3DE4. All code/section
// checks precede publication; no captured client pointer or RVA-only bind is used.
//
// 4346D4D0: engine virtual222 code (its LEA displacement is inspected).
// 4346D4C4: interior engine counter/arithmetic code.
// 4346D4C8: interior material-array call instruction.
// 4346D4C0: address of the engine tree header, whose value can legitimately0.
// These are readable instruction/data inputs, never executable callbacks.
void initialize_build_loader_inspection_inputs(NativeDataImage&);
} // namespace skeet::engine

// Declarations from build_local_animation_bindings.hpp

namespace skeet::engine {
struct BuildLocalAnimationBindings {
    animation::InitializeSequenceMapping initialize_mapping;
    animation::SelectSymbolSequence select_symbol_sequence;
    animation::LookupAnimationSymbol find_symbol, add_symbol;
    void* symbol_table;
    void* activity_vtable;
    void* layer_weight;
    misc::NetworkFixEngineEntry network_process;
    std::uintptr_t strafe_call_site, proxy_call_site, sequence_call_site, proxy_context,
        velocity_continuation;
};
// Exact supported client65272025/engine64EE7E86 build. Resolves genuine
// functions and call-site identities from checked module code/operands; it
// neither executes engine code nor loads a module. All publications occur
// after validation, including the independently resolved animation RTTI table.
// Publishes the three actual local-query caller identities434674E0/4346756C/
//43468518 after proving their virtual88/170 engine call scopes.
BuildLocalAnimationBindings initialize_build_local_animation_bindings(NativeDataImage&);
// External engine ABI only: ECX=state, EDX=0, stack(layer), XMM0 result, RET4.
animation::LocalLayerWeightEngine native_local_layer_weight_engine(void* entry);
} //namespace skeet::engine

// Declarations from build_material_bindings.hpp

namespace skeet::engine {
struct BuildMaterialBindings {
    NativeDataImage& data;
    effects::CreateMaterialVariable create_variable;
    void* localization;
};
// Acquire only already loaded modules. The material-variable fastcall is an
// internal materialsystem function, identified by its unique supported-build
// instruction body. Localize_001 is the actual localize.dll registration;
// its proven MOV EAX,object;RET creator is evaluated from its live operand.
// All provenance checks finish before43467A68/4346A31C are published. No game
// constructor, material allocation or captured function is executed here.
BuildMaterialBindings initialize_build_material_bindings(NativeDataImage&,
                                                         const LoadedModuleEnvironment&,
                                                         NativeImageEnvironment);
BuildMaterialBindings initialize_build_material_bindings(NativeDataImage&);
} //namespace skeet::engine

// Declarations from build_misc_engine_bindings.hpp

namespace skeet::engine {
struct BuildMiscEngineBindings {
    misc::SetClanTag set_clan_tag;
    void** game_rules;
    void** player_resource;
    const misc::AutomaticWeaponsBombList* planted_bombs;
};

// Verify actual loaded client/engine builds and instruction/data operands,
// then publish43467550/4346A3A4/43468FF0/434688C0/4346D558/4346D554 together.
// The four data aliases borrow live engine-owned storage. Auto-accept follows
// the genuine SetLocalPlayerReady registration and its stdcall string bridge.
//4346D560/4346D55C retain the actual two performance-owner instruction records.
// No game helper, constructor or interface method executes during discovery.
BuildMiscEngineBindings initialize_build_misc_engine_bindings(NativeDataImage&);
} // namespace skeet::engine

// Declarations from build_module_bindings.hpp

namespace skeet::engine {
struct SourceInitializationImports;
struct BuildModuleRecipe {
    std::uint32_t destination_cell;
    std::uint32_t salted_name_hash;
    std::u16string_view module_name;
};

// These17 loaded PE32 modules supply all41 build interface acquisitions. The
//working dump independently matches each canonical cell to its module base.
std::span<const BuildModuleRecipe, 17> build_module_manifest() noexcept;
// The Steam overlay has no interface request in the41-row acquisition table.
// Its base is resolved separately and may be null at this publication cut.
// The later hook-descriptor phase requires it: a missing overlay reaches
// native termination12 after25 descriptors/93 slots, before operands898/89C.
const BuildModuleRecipe& build_overlay_module_recipe() noexcept;

struct BuildModuleBases {
    std::array<void*, 17> modules{};
    void* overlay = nullptr;
    void* at(std::u16string_view exact_module_name) const;
};

// Resolve only the existing PEB loader list, with the real loader-lock imports.
// Verify its exact case-folded basename and bounded MZ/PE32/I386 headers.
// Stage all17 mandatory results before changing any canonical cell. This
//publishes module bases alone: no code RVA, interface, offset or decoded helper
//pointer is obtained from a captured process. Repeated calls refresh the bases.
BuildModuleBases publish_build_module_bases(NativeDataImage&, const LoadedModuleEnvironment&);
BuildModuleBases publish_build_module_bases(NativeDataImage&, const SourceInitializationImports&);
BuildModuleBases publish_build_module_bases(NativeDataImage&);
} //namespace skeet::engine

// Declarations from build_packet_bindings.hpp

namespace skeet::engine {
// The build's4346A7CC external primitive is engine.dll's occlusion query.
// Reads loaded module bytes only; no original engine function is called here.
void initialize_build_packet_bindings(NativeDataImage&);
class PacketBoxRuntime final {
public:
    explicit PacketBoxRuntime(const NativeDataImage&);
    network::PacketBoxQuery binding() noexcept;
    bool query(const network::Bounds&, const network::Bounds&, network::Vector&, float) const;

private:
    const std::uintptr_t& entry_;
    static bool invoke(void*, const network::Bounds&, const network::Bounds&, network::Vector&,
                       float);
};
namespace detail {
bool invoke_packet_box(std::uintptr_t, const network::Bounds&, const network::Bounds&,
                       network::Vector&, float);
}
} //namespace skeet::engine

// Declarations from build_prediction_scratch_bindings.hpp

namespace skeet::engine {
// Acquire the genuine client.dll prediction-map pool and rebuild member from
// the supported PE32 build. This inspects bytes and publishes engine addresses;
// it neither runs an instruction nor creates a prediction map or pool record.
void initialize_build_prediction_scratch_bindings(NativeDataImage&);
} //namespace skeet::engine

// Declarations from build_runtime_type_bindings.hpp

namespace skeet::engine {
enum class BuildRuntimeType : std::size_t {
    trace_filter_simple,
    trace_filter_skip_two_entities,
    base_player_animation_state,
    game_rules_proxy,
    cs_ragdoll,
    cs_ragdoll_renderable,
    econ_item_attribute,
    viewmodel_attachment_renderable,
    predicted_viewmodel_renderable,
    weapon_world_model_renderable,
    econ_wearable_renderable,
    cs_player,
    base_cs_grenade,
    player_addon_model_renderable,
    weapon_cs_base,
    count
};
struct BuildRuntimeTypeRecipe {
    BuildRuntimeType identity;
    std::uint32_t operand, type_hash, destination_cell, object_offset;
    const char* decorated_name;
};
// The fifteen orderedFC0..1070 triples are explicit fixed-build data. They
// neither implement nor substitute for the protected metadata accessor.
std::span<const BuildRuntimeTypeRecipe, 15> build_runtime_type_manifest() noexcept;

enum class BuildRuntimeTypeFailureReason {
    missing_type,
    unexpected_type_name,
    missing_vtable,
    invalid_vtable
};
class BuildRuntimeTypeFailure final : public std::runtime_error {
public:
    static constexpr std::int32_t native_exit_code = 10;
    BuildRuntimeTypeFailure(std::size_t, const BuildRuntimeTypeRecipe&,
                            BuildRuntimeTypeFailureReason);
    const std::size_t index;
    const BuildRuntimeType identity;
    const std::uint32_t operand, type_hash, destination_cell, object_offset;
    const char* const decorated_name;
    const BuildRuntimeTypeFailureReason reason;
};

class BuildRuntimeTypeBindings {
public:
    NativeDataImage& data() const noexcept {
        return data_;
    }
    RttiTypeDescriptor* type(BuildRuntimeType) const;
    // Reload each actual source cell. Later native/source stores, including a
    // cleared viewmodel-attachment table, remain observable to its consumers.
    void** vtable(BuildRuntimeType) const;

private:
    BuildRuntimeTypeBindings(NativeDataImage&, const std::array<RttiTypeDescriptor*, 15>&);
    NativeDataImage& data_;
    std::array<RttiTypeDescriptor*, 15> types_;
    friend BuildRuntimeTypeBindings initialize_build_runtime_type_bindings(NativeDataImage&,
                                                                           NativeImageEnvironment);
};

// Read the already published client.dll module cell4346A55C and resolve its
// real MSVC RTTI through433B305B/433EC4BD. No game function is called. The exact
// supported PE32 build/sections and each decorated name/table are checked.
// Successful rows publish in manifest order. Native missing-type/table paths
// reach4337F065 and ZwTerminateThread(-2,10) before the failed row's store.
// This boundary reports a typed failure, retaining earlier rows; the startup
// owner performs that terminal sequence using its actual state/imports.
BuildRuntimeTypeBindings initialize_build_runtime_type_bindings(NativeDataImage&,
                                                                NativeImageEnvironment);
BuildRuntimeTypeBindings initialize_build_runtime_type_bindings(NativeDataImage&);
} //namespace skeet::engine

// Declarations from build_scene_bindings.hpp

namespace skeet::engine {
struct BuildSceneClassRecipe {
    std::uint32_t operand, class_hash, destination_cell;
    const char* network_name;
};
std::span<const BuildSceneClassRecipe, 20> build_scene_class_manifest() noexcept;
class BuildSceneClassFailure final : public std::runtime_error {
public:
    static constexpr std::int32_t native_exit_code = 10;
    BuildSceneClassFailure(std::size_t, const BuildSceneClassRecipe&);
    const std::size_t index;
    const std::uint32_t operand, class_hash, destination_cell;
    const char* const network_name;
};

// Source scene bindings for the supported client65272025 build. Requires its
// canonical module4346A55C and VClient018434683E0. Reads the genuine virtual8
// GetAllClasses list, preserving ClientClass pointer identity independently of
// assigned class IDs. These twenty cells are not RTTI descriptors or vtables.
// Also discovers the real optional IsBreakableEntity entry43468F30 through
// verified code/RTTI operands. It calls no breakability/entity factory method.
//
// The supported build has289 registered classes. The whole390..42C class loop
// calls GetAllClasses once per row and publishes successful rows in native
// order. A missing class reaches native ZwTerminateThread(-2,10) before that
// row's destination read/store; the source failure exposes this boundary and
// preserves prior stores. The startup owner performs the terminal OS call.
// Invalid image/list provenance is an unsupported source binding. Keep the
// module loaded and run before constructing source scene views.
void initialize_build_scene_bindings(NativeDataImage&);
} // namespace skeet::engine

// Declarations from build_script_bindings.hpp

namespace skeet::engine {
struct BuildScriptBindings {
    NativeDataImage& data;
    const std::int32_t* old_command_ack;
    const std::int32_t* command_ack;
    void* class_size_interface;
};
// Verify the actual loaded engine/client instruction operands, client-state
// publications and CClassMap implementation before publishing the three cells.
// This inspects passive module bytes; it calls no game method or constructor.
BuildScriptBindings initialize_build_script_bindings(NativeDataImage&);
}

// Declarations from build_shared_esp_bindings.hpp

namespace skeet::engine {
// Source inputs consumed once by43485DC9. Recover the real engine CALL sites,
// their constructor/destructor pairs and exact protobuf class identities.
// This publishes4346D668/4346D66C without executing an engine instruction.
void initialize_build_shared_esp_bindings(NativeDataImage&, NativeImageEnvironment);
void initialize_build_shared_esp_bindings(NativeDataImage&);
} // namespace skeet::engine

// Declarations from build_skin_bindings.hpp

namespace skeet::engine {
// Genuine external engine boundaries and class objects used by the skin
// owners. All source views below share this canonical data image. Fixtures may
// supply authored engine boundaries; production uses the acquisition function.
struct BuildSkinBindings {
    using RandomSeed = void(__cdecl*)(std::int32_t);
    using RandomInteger = std::int32_t(__cdecl*)(std::int32_t, std::int32_t);
    NativeDataImage& data;
    RandomSeed seed_random;
    RandomInteger random_integer;
    ClientClass* viewmodel_class;
    ClientClass* knife_class;
};

// Supported client65272025. Locate unique code signatures and read their real
// static data operands before publication. The returned function addresses are
// in the actual loaded client/vstdlib images; no captured singleton is copied.
// Acquires CEconWearable, CPredictedViewModel and CKnife through VClient018's
// actual GetAllClasses boundary. ItemSystem43468B9C must already be produced
// by initialize_build_client_factories; it is not called here.
BuildSkinBindings initialize_build_skin_bindings(NativeDataImage&, NativeImageEnvironment);
BuildSkinBindings initialize_build_skin_bindings(NativeDataImage&);
} //namespace skeet::engine

// Declarations from build_skin_receive_patches.hpp

namespace skeet::engine {
struct BuildSkinReceivePatchRecipe {
    std::uint32_t operand, class_hash, table_hash, property_hash, patch_address;
    const char* class_name;
    const char* table_name;
    const char* property_name;
};
// Four ordered A48..A84 recipes and the later AFC agent recipe. These are
// explicit fixed-build metadata, not an implementation of its protected getter.
std::span<const BuildSkinReceivePatchRecipe, 5> build_skin_receive_patch_manifest() noexcept;
enum class BuildSkinReceivePatchFailureReason {
    missing_property,
    missing_original,
    already_initialized
};
class BuildSkinReceivePatchFailure final : public std::runtime_error {
public:
    BuildSkinReceivePatchFailure(std::size_t, const BuildSkinReceivePatchRecipe&,
                                 BuildSkinReceivePatchFailureReason);
    const std::size_t index;
    const BuildSkinReceivePatchRecipe recipe;
    const BuildSkinReceivePatchFailureReason reason;
};
class BuildSkinReceivePatches {
public:
    NativeDataImage& data() const noexcept {
        return data_;
    }
    std::span<client::EntityUpdatePatch* const, 4> sequence() const noexcept {
        return sequence_;
    }
    client::EntityUpdatePatch& agent() const noexcept {
        return agent_;
    }

private:
    explicit BuildSkinReceivePatches(NativeDataImage&);
    NativeDataImage& data_;
    std::array<client::EntityUpdatePatch*, 4> sequence_;
    client::EntityUpdatePatch& agent_;
    friend BuildSkinReceivePatches initialize_build_skin_receive_patches(NativeDataImage&);
};
// Resolve genuine current RecvProp.proxy fields through VClient018 slot8 and
// the ordinary native lookup helpers. Capture their current callbacks in the
// five canonical nodes without changing a property, node.next, or list head.
// Bind these nodes to the same SkinsRuntimeOwner before publishing any list.
// All-five validation and typed rejection are explicit source policy while
// the native missing-property continuation and list-write order are unresolved.
BuildSkinReceivePatches initialize_build_skin_receive_patches(NativeDataImage&);
} //namespace skeet::engine

// Declarations from build_startup_convars.hpp

namespace skeet::engine {
struct BuildStartupConVarSlot {
    std::uint32_t operand, address, name_hash, xor_key;
    const char* name;
};
// Explicit fixed-build data from the105 ordered B70..EB0 operand pairs.
// This manifest does not implement or claim the protected operand getter.
std::span<const BuildStartupConVarSlot, 105> build_startup_convar_slots() noexcept;

enum class BuildStartupConVarFailureReason {
    missing_candidate,
    candidate_is_command
};
class BuildStartupConVarFailure final : public std::runtime_error {
public:
    // Proven only for missing_candidate. The command-candidate continuation
    // reaches erased code4353F942 and has no recovered terminal policy.
    static constexpr std::int32_t native_missing_exit_code = 10;
    BuildStartupConVarFailure(std::size_t, const BuildStartupConVarSlot&,
                              BuildStartupConVarFailureReason);
    const std::size_t index;
    const std::uint32_t operand, address, name_hash;
    const char* const name;
    const BuildStartupConVarFailureReason reason;
};

class BuildStartupConVarProvider : public StartupConVarProvider {
public:
    virtual bool is_command(void* candidate) = 0;
};
class BuildStartupConVarBindings {
public:
    NativeDataImage& data() const noexcept {
        return data_;
    }
    NativeConVarReference at(std::uint32_t native_cell) const;
    StartupConVarBindings startup_bindings() const noexcept;
    client::StartupConVars startup_variables() const;
    NativeConVarReference accuracy_variable() const;

private:
    explicit BuildStartupConVarBindings(NativeDataImage& data) noexcept : data_(data) {}
    NativeDataImage& data_;
    friend BuildStartupConVarBindings discover_build_startup_convars(NativeDataImage&, void*,
                                                                     BuildStartupConVarProvider&);
};

// Source bootstrap requires all105 genuine ConVars and publishes only after
// complete acquisition. The native success loop mutates each row separately.
// A missing candidate reaches4337F065/ZwTerminateThread(-2,10), retaining the
// prior stores; its failed row has no publication. The source transaction and
// typed failures remain explicit source policy. A command candidate's erased
// continuation4353F942 remains unresolved. Canonical XOR encodings are preserved.
BuildStartupConVarBindings discover_build_startup_convars(NativeDataImage&, void* cvar,
                                                          BuildStartupConVarProvider&);
BuildStartupConVarBindings discover_build_startup_convars(NativeDataImage&, void* cvar);
} //namespace skeet::engine

// Declarations from build_visual_interface_bindings.hpp

namespace skeet::engine {
struct BuildVisualInterfaceBindings {
    void* view_provider;
    void* temp_entities;
    void* beams;
    void* weapon_system;
};
// These four genuine inline client objects are not CreateInterface entries.
// Validate the client build, exact RTTI, unique live instances and their real
// instruction operands/ABIs, then publish4346A624/434679EC/4346A898/43467B54. No game code runs
// during acquisition and no constructor is replayed over a loaded object.
BuildVisualInterfaceBindings initialize_build_visual_interface_bindings(NativeDataImage&,
                                                                        NativeImageEnvironment);
BuildVisualInterfaceBindings initialize_build_visual_interface_bindings(NativeDataImage&);
} // namespace skeet::engine

// Declarations from build_window_bindings.hpp

namespace skeet::engine {
// Source integration for the observed engine64EE7E86/studiorender63DC118B
// builds. The PE identities and instruction fingerprints are build assumptions,
// separate from the recovered game-window and range-initializer algorithms.
// Reads already loaded module data/code only; calls no engine/studio function.
// Rejects unsupported identities, sections, signatures or pointer provenance
// before publishing any cell. Keep both modules loaded while consumers run.
//
// Writes43467288/43468B2C/43469288 and the two active range rows. Rows0/2 have
// their observed literal initializer1; their unused companion cells are left
// untouched. Call before owner4347D94F and initialize_module_ranges. This does
// not initialize the separately produced client-module cell4346A55C.
void initialize_build_window_bindings(NativeDataImage&, const LoadedModuleEnvironment&);
} // namespace skeet::engine

// Declarations from build_world_bindings.hpp

namespace skeet::engine {
struct GameSystemRegistrationNode;
struct BuildWorldBindings {
    NativeDataImage& data;
    void* rank_context;
    void** rank_data;
    GameSystemRegistrationNode** registry_head;
    script::PanoramaBeginEngineCall begin_engine_call;
    script::PanoramaEndEngineCall end_engine_call;
};
// Supported live client/panorama instruction and data provenance. The lazy
// rank slot and registration head remain engine-owned live cells; no object
// is allocated or copied from a captured process and no game code is called.
BuildWorldBindings initialize_build_world_bindings(NativeDataImage&);
// The two live CUtlVector size members used by MapGameSystemRuntime. Verify
// the real client CRT stores and independent append consumers before binding
//43468D0C/43467A70; no list or captured endpoint is copied into source memory.
void initialize_build_map_game_system_bindings(NativeDataImage&);
} //namespace skeet::engine

// Declarations from client_class_factory_hooks.hpp

namespace skeet::client {
class ClientRuntimeStorage;
class PlayerRuntimeOwner;
class EntityCommandRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::engine {
class HookRuntimeOwner;
class HookDescriptorStorage;
struct VtableHookRegistry;
enum class ClientClassFactoryKind {
    player,
    ragdoll,
    incendiary,
    molotov,
    viewmodel,
    inferno
};
using ClientClassCreate = void* (*)(std::int32_t, std::int32_t);
struct ClientClassFactoryDescriptor {
    std::uint32_t operand;
    const char* class_name;
    std::uint32_t class_hash, native_entry, patch_address;
    ClientClassCreate replacement;
};
// The six native480..4BC recipes. The final startup scheduler captures each
// actual ClientClass.create in patch.after and installs this source entry.
std::span<const ClientClassFactoryDescriptor, 6> client_class_factory_descriptors() noexcept;

// Creation callbacks borrow the same cache, record pool, prediction owner,
// descriptors and installation registry as the existing feature lifecycles.
// This context does not create an entity or install a ClientClass patch.
class ClientClassFactoryRuntime final {
public:
    ClientClassFactoryRuntime(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                              client::PlayerRuntimeOwner&, client::EntityCommandRuntimeOwner&,
                              HookRuntimeOwner&);
    ClientClassFactoryRuntime(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                              client::PlayerRuntimeOwner&, client::EntityCommandRuntimeOwner&,
                              VtableHookRegistry&, HookDescriptorStorage&);
    ~ClientClassFactoryRuntime();
    ClientClassFactoryRuntime(const ClientClassFactoryRuntime&) = delete;
    ClientClassFactoryRuntime& operator=(const ClientClassFactoryRuntime&) = delete;
    void* create(ClientClassFactoryKind, std::int32_t entity_index, std::int32_t serial);

private:
    void install(void*, std::uint32_t descriptor);
    void initialize_player(void* networkable, std::int32_t entity_index);
    void restore_local_movement_flags(void* player);
    client::ClientRuntimeStorage& storage_;
    client::PlayerRuntimeOwner& players_;
    client::EntityCommandRuntimeOwner& entity_;
    VtableHookRegistry& registry_;
    HookDescriptorStorage& descriptors_;
};
// Publish only after the actual family contexts are bound. Keep this runtime
// alive through restoration of the six ClientClass.create patch targets.
void bind_client_class_factory_runtime(ClientClassFactoryRuntime&);
} //namespace skeet::engine

// Declarations from common_controls_initialization.hpp

namespace skeet::engine {
struct ForwardedExportEnvironment;

struct CommonControlsExports {
    std::byte* default_subclass_proc = nullptr; // Native 0x43476910, ordinal 413.
    std::byte* remove_window_subclass = nullptr; // Native 0x43476914, ordinal 412.
    std::byte* set_window_subclass = nullptr; // Native 0x43476918, ordinal 410.
};

// Devirtualized startup imports. The fixed ordinal/destination pairs are
// independently confirmed by the working client's dump and PE export table.
// No protected metadata accessor or captured process address is called.
void initialize_startup_common_controls(CommonControlsExports& imports,
                                        const ForwardedExportEnvironment& environment);
}

// Declarations from decoded_offset.hpp

namespace skeet::engine {
// A live operand from the protected offset globals. Copying this object keeps
// its binding; only conversion reads the current pointer, pointee and key.
class DecodedOffset {
public:
    DecodedOffset(const NativeOffsetGlobals& globals, std::uint32_t pointer, std::uint32_t key,
                  std::uint32_t salt, std::uint32_t displacement)
        : globals_(&globals), pointer_(pointer), key_(key), salt_(salt),
          displacement_(displacement) {}
    operator std::ptrdiff_t() const {
        const auto value = static_cast<std::uint32_t>(
            (*globals_->pointers.at(pointer_) ^ globals_->keys.at(key_) ^ salt_) + displacement_);
        return static_cast<std::ptrdiff_t>(std::bit_cast<std::int32_t>(value));
    }

private:
    const NativeOffsetGlobals* globals_;
    std::uint32_t pointer_, key_, salt_, displacement_;
};
} //namespace skeet::engine

// Declarations from executable_pattern.hpp

namespace skeet::engine {
// Complete433C018C over loaded PE32 executable, nondiscardable sections.
// Matches within a section skip by pattern length; occurrence restarts for
// each section. Both native length subtractions are retained.
std::byte* executable_pattern(std::byte* module, const std::uint8_t* pattern, std::uint32_t length,
                              std::uint32_t occurrence = 0, std::uint8_t wildcard = 0xCC);
} //namespace skeet::engine

// Declarations from first_startup_services.hpp

namespace skeet::engine {
struct FirstStartupServiceSlot {
    std::uint32_t address;
    std::uint32_t initial_hash;
    const char* name;
};
std::span<const FirstStartupServiceSlot, 9> first_startup_service_slots() noexcept;

// All imports are genuine already-loaded Windows entry points. This context
// borrows source-owned data; no executable input image or VM is required.
struct FirstStartupServiceBindings {
    NativeDataImage& data;
    LoadedModuleEnvironment modules;
    NativeImageEnvironment images;
    ImageDirectoryEntry image_directory_entry;
    NativeServiceTerminateProcess terminate_process;
};

// Source operation434822C8 ->434EA5D4. The second load-order module's base
// selects the original77BF0000 branch. It creates the temporary Nt-service
// table and converts the nine current hash cells in their actual write order.
// The startup scheduler ignores the native scalar return. This conversion is
// one-shot: a repeated call consumes the then-current cells, just as native.
void initialize_first_startup_services(const FirstStartupServiceBindings&);
} // namespace skeet::engine

// Declarations from focus_overlay_hooks.hpp

namespace skeet::client {
class CrtRuntimeOwner;
class ClientRuntimeStorage;
class PostInitializationOwner;
class VisualRuntimeOwner;
}
namespace skeet::ui {
class StartupRenderer;
}
namespace skeet::script {
class ScriptModulesOwner;
}

namespace skeet::engine {
class NativeDataImage;
// Decode the proven VEngineVGui001 field use and validate the receiver's
// genuine CFocusOverlayPanel RTTI table. No captured object pointer is used.
void* find_build_focus_overlay_receiver(const NativeDataImage&, const VtableHookMetadata&);
// This source address can be bound by the earlier descriptor phase. Its real
// runtime is published after the menu/post-initialization owners exist.
void bind_focus_overlay_first_entry(VtableHookMetadata&) noexcept;
// Complete43416A0F over the actual canonical callable cells and receiver.
void dispatch_focus_first_invocation(NativeDataImage&, void* receiver);

class FocusOverlayHooks {
public:
    FocusOverlayHooks(client::CrtRuntimeOwner&, client::PostInitializationOwner&,
                      ui::StartupRenderer&, client::VisualRuntimeOwner&);
    ~FocusOverlayHooks();
    FocusOverlayHooks(const FocusOverlayHooks&) = delete;
    FocusOverlayHooks& operator=(const FocusOverlayHooks&) = delete;
    FocusOverlayHooks(FocusOverlayHooks&&) = delete;
    FocusOverlayHooks& operator=(FocusOverlayHooks&&) = delete;
    void prepare();
    void install();
    void restore();
    void first_invocation(void* receiver); //whole43416A0F
    void draw(void* receiver); //whole4338A69A
    void initialize(); //whole4341F57F via the actual PostInitializationOwner
    VtableHookMetadata& metadata() const noexcept {
        return metadata_;
    }

private:
    client::CrtRuntimeOwner& crt_;
    client::ClientRuntimeStorage& storage_;
    client::PostInitializationOwner& post_;
    ui::StartupRenderer& renderer_;
    client::VisualRuntimeOwner& visual_;
    script::ScriptModulesOwner& scripts_;
    VtableHookMetadata& metadata_;
    UnloadState& unload_;
    const std::uint32_t& accent_;
    bool installed_ = false;
};
}

// Declarations from forwarded_exports.hpp

namespace skeet::engine {
struct ExportAnsiString {
    std::uint16_t length;
    std::uint16_t maximum_length;
    const char* buffer;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(ExportAnsiString) == 8);
#endif

#if defined(_M_IX86) || defined(__i386__)
using ImageDirectoryEntry = std::byte*(__stdcall*)(std::byte*, std::uint8_t, std::uint16_t,
                                                   std::uint32_t*);
using InitializeExportString = void(__stdcall*)(ExportAnsiString*, const char*);
using ExportOrdinalFromString = std::int32_t(__stdcall*)(const char*, std::uint32_t,
                                                         std::uint32_t*);
#else
using ImageDirectoryEntry = std::byte* (*)(std::byte*, std::uint8_t, std::uint16_t, std::uint32_t*);
using InitializeExportString = void (*)(ExportAnsiString*, const char*);
using ExportOrdinalFromString = std::int32_t (*)(const char*, std::uint32_t, std::uint32_t*);
#endif

struct ForwardedExportEnvironment {
    LoadedModuleEnvironment modules;
    ImageDirectoryEntry image_directory_entry;
    InitializeExportString initialize_string;
    ExportOrdinalFromString ordinal_from_string;
};
ForwardedExportEnvironment native_forwarded_export_environment();

// Native 0x43320B0A and 0x433EC666, including named and ordinal forwarders.
// The caller supplies valid mapped PE data. Forwarded modules must already
// be loaded; this resolver does not load them or change their names.
std::byte* resolve_forwarded_export(std::byte* module, std::uint32_t hash, std::uint32_t ordinal,
                                    bool salted_hash, bool lock_loader,
                                    const ForwardedExportEnvironment& environment);
}

// Declarations from frame_update.hpp

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
using FrameUpdateEntry = void(__fastcall*)(void*, std::uint8_t);
#else
using FrameUpdateEntry = void (*)(void*, std::uint8_t);
#endif
// Actual one-entry interface of registry43475AD8/ADC/AE0. Unlike the HUD
// registry it receives only the forwarded byte in DL, with no stack arguments.
struct FrameUpdateEndpoint {
    const FrameUpdateEntry* vtable = nullptr;
};
FrameUpdateEndpoint* register_frame_update(OwnerRegistryBinding, FrameUpdateEndpoint*); //433FCB12
// Complete4334820E..4334824B, including the wrapping counter4346DB2C update.
void dispatch_frame_update(const OwnerRegistry&, std::int32_t& counter, std::uint8_t update);
struct FrameUpdateRuntime {
    const OwnerRegistry& callbacks; //canonical43475AD8/ADC/AE0
    std::int32_t& counter; //canonical4346DB2C
};
void bind_frame_update_runtime(FrameUpdateRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
void __stdcall frame_update_hook(std::uint8_t update);
#else
void frame_update_hook(std::uint8_t update);
#endif
} //namespace skeet::engine

// Declarations from game_system.hpp

namespace skeet::client {
class ShutdownRuntime;
}
namespace skeet::misc {
struct DisablePureConfig;
}
namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_SYSTEM_MEMBER __thiscall
#define SKEET_SYSTEM_FAST __fastcall
#else
#define SKEET_SYSTEM_MEMBER
#define SKEET_SYSTEM_FAST
#endif

// The argument counts are part of the actual4347058C interface. In particular,
// its false/true/empty entries have different RET immediates.
template <class R, class... A> using GameSystemEntry = R(SKEET_SYSTEM_MEMBER*)(void*, A...);
struct GameSystemVtable {
    GameSystemEntry<const char*> name; //0 4332938A
    GameSystemEntry<bool> initialize; //1 43408189: false
    GameSystemEntry<void> shutdown; //2 433EF91A
    GameSystemEntry<bool, std::uintptr_t> slot3, slot4;
    GameSystemEntry<void> slot5, slot6;
    GameSystemEntry<bool, std::uintptr_t> slot7;
    GameSystemEntry<void, std::uintptr_t, std::uintptr_t> slot8;
    GameSystemEntry<void, std::uint8_t> frame; //9 4334820E
    std::array<GameSystemEntry<void>, 8> slots10_17;
    GameSystemEntry<std::uintptr_t, std::uintptr_t> slot18, slot19;
    GameSystemEntry<void> owner_phase6, slot21, owner_phase8, slot23;
    GameSystemEntry<void> update_phase2, update_phase3; //24,25
    GameSystemEntry<void, std::uintptr_t, std::uintptr_t, const std::int32_t*, const std::int32_t*>
        capture_camera; //26 4334331D
    GameSystemEntry<bool, float*, float*, float*> view; //27 433B91CB
    GameSystemEntry<bool, std::uintptr_t> slot28;
    GameSystemEntry<bool> slot29, slot30, slot31, slot32;
    GameSystemEntry<std::uintptr_t, std::uintptr_t> slot33;
    GameSystemEntry<void> slot34, slot35;
    GameSystemEntry<bool, std::uintptr_t, std::uintptr_t> slot36;
    GameSystemEntry<bool, std::uintptr_t, std::uintptr_t, std::uintptr_t> slot37;
    GameSystemEntry<void> update_phase0, update_phase1, slot40, slot41;
    GameSystemEntry<void, std::int32_t> before_stage, after_stage; //42,43
    GameSystemEntry<void> pure_file_override, update_phase4; //44,45
    GameSystemEntry<std::uint32_t> slot46;
};
struct GameSystemEndpoint {
    const GameSystemVtable* vtable = nullptr;
};
struct GameSystemVector {
    GameSystemEndpoint** data = nullptr;
    std::int32_t capacity = 0, grow_size = 0, size = 0;
    GameSystemEndpoint** elements = nullptr;
};
struct GameSystemManager {
    void* vtable = nullptr;
    GameSystemVector systems;
    std::array<std::uint32_t, 5> reserved{};
    std::int32_t current = -1; // native manager+2C
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(GameSystemVtable) == 47 * 4);
static_assert(offsetof(GameSystemVtable, frame) == 9 * 4);
static_assert(offsetof(GameSystemVtable, before_stage) == 42 * 4);
static_assert(sizeof(GameSystemVector) == 0x14);
static_assert(offsetof(GameSystemManager, systems) == 4);
static_assert(offsetof(GameSystemManager, current) == 0x2C);
#endif

struct GameSystemBindings {
    void* const& engine; //4346A514, slots5/37/101
    void* const& view_provider; //4346A624, slot12(-1)
    void* const& filesystem; //43467F68, slot128's actual instruction
    const misc::DisablePureConfig& disable_pure; //43475974 owner+1C,177AB88C
    float (*tangent)(float); //genuine CRT7750CA90
    float (*arctangent)(float); //genuine CRT7750A400
};
struct GameSystemCameraState {
    std::array<float, 3>& origin; //43479CCC
    std::array<float, 3>& angles; //43479CD8
    float& field_of_view; //43468E80
    float& aspect_field_of_view; //4346D548
    esp::Projection& projection; //matrix434798E0, halves43479920/434798DC
};

class GameSystemRuntime {
public:
    GameSystemRuntime(client::ShutdownRuntime&, const OwnerRegistry& updates,
                      const OwnerRegistry& frames, std::int32_t& frame_counter,
                      std::int32_t& update_stage, GameSystemBindings, GameSystemCameraState);
    client::ShutdownRuntime& shutdown;
    const OwnerRegistry& updates;
    const OwnerRegistry& frames;
    std::int32_t& frame_counter;
    std::int32_t& update_stage;
    GameSystemBindings bindings;
    GameSystemCameraState view_state;
    void capture_camera(const std::int32_t* width, const std::int32_t* height); //4334331D
    void capture_projection(); //43417FE8
    void before_stage(std::int32_t); //43419ADC
    void after_stage(std::int32_t); //4337E9DE
    void apply_pure_file_override(); //4332922E, same config as4335C233
};

// Full4348E09F owner, lifecycle43472A2C. The manager pointer is captured at
// construction; allocator is the live43467204 engine allocation interface.
struct GameSystemOwner {
    const GameSystemEntry<void>* vtable = nullptr;
    GameSystemManager* manager = nullptr;
    std::int32_t saved_index = -1;
    GameSystemEndpoint endpoint;
    GameSystemOwner* installed = nullptr;
    GameSystemRuntime* runtime = nullptr;
    void* const* allocator = nullptr;
    JobMemory memory{};
    void install(); //4347F4E5 ->433F4786/4337CF0C
    void remove(); //43331AAF ->433B9059
};
GameSystemOwner* create_game_system_owner(OwnerRegistryBinding, GameSystemManager*,
                                          void* const* allocator, GameSystemRuntime&);

// Separate constructor4347DA14: this six-entry secondary interface is linked
// through43468D50. It is not the47-entry manager interface above.
struct GameSystemRegistrationVtable {
    GameSystemEntry<void> slot0, slot1;
    GameSystemEntry<void, std::uintptr_t> slot2;
    GameSystemEntry<void, std::uintptr_t, std::uintptr_t, std::uintptr_t, std::uintptr_t> dispatch;
    GameSystemEntry<void, std::uintptr_t> slot4;
    GameSystemEntry<bool> slot5;
};
struct GameSystemRegistrationEndpoint {
    const GameSystemRegistrationVtable* vtable = nullptr;
};
struct GameSystemRegistrationNode {
    const char* name = nullptr;
    GameSystemRegistrationEndpoint* endpoint = nullptr;
    GameSystemRegistrationNode* next = nullptr;
};
using RegisteredClientEntry = void(SKEET_SYSTEM_FAST*)(void*, std::uintptr_t, std::uintptr_t,
                                                       std::uintptr_t, std::uintptr_t);
struct RegisteredClientEndpoint {
    const RegisteredClientEntry* vtable = nullptr;
};
void dispatch_registered_client_callbacks(const OwnerRegistry&, std::uintptr_t, std::uintptr_t,
                                          std::uintptr_t, std::uintptr_t); //433B8C46
struct GameSystemRegistrationOwner {
    const GameSystemEntry<void>* vtable = nullptr;
    GameSystemRegistrationEndpoint endpoint;
    GameSystemRegistrationNode node;
    std::uint8_t registered = 0;
    char empty_name = 0;
    GameSystemRegistrationNode** const* head_slot = nullptr;
    const OwnerRegistry* callbacks = nullptr;
    UnloadState* unload = nullptr;
    JobMemory memory{};
    void install(); //43417897
    void remove(); //433AE857; hard detach leaves all link state untouched
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(GameSystemRegistrationVtable) == 24);
static_assert(offsetof(GameSystemOwner, endpoint) == 0xC);
static_assert(offsetof(GameSystemOwner, installed) == 0x10);
static_assert(offsetof(GameSystemRegistrationOwner, endpoint) == 4);
static_assert(offsetof(GameSystemRegistrationOwner, node) == 8);
static_assert(offsetof(GameSystemRegistrationOwner, registered) == 0x14);
static_assert(offsetof(GameSystemRegistrationOwner, empty_name) == 0x15);
#endif
GameSystemRegistrationOwner*
create_game_system_registration(OwnerRegistryBinding, GameSystemRegistrationNode** const* head_slot,
                                const OwnerRegistry& callbacks, UnloadState&);

#undef SKEET_SYSTEM_MEMBER
#undef SKEET_SYSTEM_FAST
} //namespace skeet::engine

// Declarations from game_system_patch.hpp

namespace skeet::engine {
// Devirtualized from VM entry 0x4351CF96. The caller supplies the same
// target slot (native 0x434757CC) later consumed by GameSystemRuntime and unload.
// A null candidate leaves the previous slot untouched. No pointee is read.
// The native entry receives the candidate in EAX and returns success in AL;
// this ordinary C++ entry is for recovered callers, not a raw VM ABI detour.
bool capture_game_system_patch_target(void**& target_slot, void** candidate) noexcept;
}

// Declarations from game_window_proxy.hpp

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
using GameWindowCrc32 = std::uint32_t(__stdcall*)(std::uint32_t, const void*, std::uint32_t);
using GameWindowExchange = std::int32_t(__stdcall*)(volatile std::int32_t*, std::int32_t);
#else
using GameWindowCrc32 = std::uint32_t (*)(std::uint32_t, const void*, std::uint32_t);
using GameWindowExchange = std::int32_t (*)(volatile std::int32_t*, std::int32_t);
#endif
struct GameWindowProxyImports {
    GameWindowCrc32 crc32;
    GameWindowExchange exchange;
};
// Resolves only the genuine exports in already loaded ntdll/kernel32 modules.
GameWindowProxyImports native_game_window_proxy_imports();

struct GameWindowProxyBindings {
    const std::byte* const& guarded_return_site; //43467288, verified engine caller
    void** const& game_interface_slot; //43468B2C, address of the live IGame pointer
    std::uintptr_t& original_rtti; //43469288, original vtable[-1]
    GameWindowProxyImports imports;
};
GameWindowProxyBindings native_game_window_proxy_bindings(NativeDataImage&, GameWindowProxyImports);

struct GameWindowProxyRegistries {
    OwnerRegistryBinding lifecycle; //43475B78
    OwnerRegistryBinding maps; //43475770
};
struct GameWindowProxyOwner;
std::size_t game_window_proxy_owner_size() noexcept;
// Complete4347D94F: zeroed native2C-byte prefix, lifecycle registration, map
// endpoint registration, then primary43470530 and secondary4346EF00 publication.
// Storage follows allocate_owner_storage's aligned heap-header contract.
GameWindowProxyOwner& construct_game_window_proxy_owner(void*, const GameWindowProxyRegistries&,
                                                        GameWindowProxyBindings);
// The original null allocation branch returns without registering an owner.
GameWindowProxyOwner* create_game_window_proxy_owner(const GameWindowProxyRegistries&,
                                                     GameWindowProxyBindings);

// Complete primary1, map0 and primary4. Initialization scans four-byte CRC
// windows until B94D4C7E; its native CMP200 is followed by an unconditional jump
// and does not impose a search bound. The verified loaded-build guard must have
// a readable match. The proxy preserves all25 native virtual tail transfers;
// slot9 restores the real interface and changes only the matched caller return.
void initialize_game_window_proxy(GameWindowProxyOwner&);
void install_game_window_proxy(GameWindowProxyOwner&);
void restore_game_window_proxy(GameWindowProxyOwner&);
// Perform primary4 before lifecycle destruction while a proxy can still be used.
} // namespace skeet::engine

// Declarations from hash_growth.hpp

namespace skeet::engine {
namespace hash_growth_detail {
inline float unsigned_float(std::uint32_t value) noexcept {
    double converted = double(std::bit_cast<std::int32_t>(value));
    if (value & 0x80000000u)
        converted += 4294967296.0;
    return float(converted);
}
// Low result word of433EC12E's float-to-signed64 truncation. The native hash
// policies supply finite positive values; preserve the low word past2^32.
inline std::uint32_t integer_low_word(float value) noexcept {
    const auto bits = std::bit_cast<std::uint32_t>(value);
    const auto exponent = std::int32_t((bits >> 23) & 0xFFu) - 127;
    if (exponent < 0)
        return 0;
    const auto fraction = bits & 0x7FFFFFu;
    const bool negative = (bits >> 31) != 0;
    if (exponent > 63 || (!negative && exponent == 63) || (negative && exponent == 63 && fraction))
        return 0;
    std::uint64_t magnitude = 0x800000u | fraction;
    if (exponent >= 23)
        magnitude <<= unsigned(exponent - 23);
    else
        magnitude >>= unsigned(23 - exponent);
    return std::uint32_t(negative ? std::uint64_t(0) - magnitude : magnitude);
}
}
struct NativeHashGrowthDecision {
    bool rehash;
    std::uint32_t bucket_count;
};

// Shared433B20BA policy body. maximum_load/multiplier/next_resize are the
// actual policy words; ceiling is the recorded external CRT ceil import.
inline NativeHashGrowthDecision
plan_native_hash_growth(std::uint32_t buckets, std::uint32_t size, const float& maximum_load,
                        const float& multiplier, std::uint32_t& next_resize,
                        std::span<const std::uint32_t> bucket_primes, double (*ceiling)(double)) {
    using namespace hash_growth_detail;
    const auto wanted = size + 1u;
    if (wanted <= next_resize)
        return {false, 0};
    const auto current = unsigned_float(buckets == 1 ? 0u : buckets);
    const auto minimum = unsigned_float(wanted) / maximum_load;
    if (minimum > current) {
        const auto grown = current * multiplier;
        const auto maximum = _mm_cvtss_f32(_mm_max_ss(_mm_set_ss(grown), _mm_set_ss(minimum)));
        const auto requested = integer_low_word(maximum);
        const auto count = *std::lower_bound(bucket_primes.begin(), bucket_primes.end(), requested);
        const float threshold = unsigned_float(count) * maximum_load;
        next_resize = integer_low_word(float(ceiling(double(threshold))));
        return {true, count};
    }
    const float threshold = current * maximum_load;
    next_resize = integer_low_word(float(ceiling(double(threshold))));
    return {false, 0};
}
} // namespace skeet::engine

// Declarations from hooks.hpp

namespace skeet::engine {
struct VtableHookRegistry {
    VtableHookNode& sentinel; // same sentinel owned by UnloadState
    JobMemory memory; // actual native heap exports
};

// Complete4331E834. The supplied descriptor holds original+8/replacement+0x10;
// the actual installed-vtable allocation points back to it at table[-2]. The
// atomic swap precedes node allocation; native allocation failure is preserved.
VtableHookNode* install_vtable_hook(VtableHookRegistry& registry, void* object,
                                    const VtableHookMetadata& metadata);
// Complete43369692. Searches oldest registration first, unlinks/frees that
// node, then atomically restores installed[-2]->original_table. No match is a
// no-op. This ordering differs from whole-unload restoration and is retained.
void uninstall_vtable_hook(VtableHookRegistry& registry, void* object);
} // namespace skeet::engine

// Declarations from hook_descriptors.hpp

namespace skeet::engine {
// MEMORY_BASIC_INFORMATION prefix consumed by native433AB42B. The query is
// the actual ZwQueryVirtualMemory service, reached through4335BD9A in native.
struct HookMemoryRegion {
    void* base_address;
    void* allocation_base;
    std::uint32_t allocation_protection;
    std::size_t region_size;
    std::uint32_t state, protection, type;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(HookMemoryRegion) == 0x1c);
static_assert(offsetof(HookMemoryRegion, protection) == 0x14);
#endif
#if defined(_M_IX86) || defined(__i386__)
using HookQueryMemory = std::int32_t(__stdcall*)(void*, const void*, std::uint32_t,
                                                 HookMemoryRegion*, std::size_t, std::size_t*);
#else
using HookQueryMemory = std::int32_t (*)(void*, const void*, std::uint32_t, HookMemoryRegion*,
                                         std::size_t, std::size_t*);
#endif
struct HookDescriptorSystem {
    HookQueryMemory query_memory;
};
HookDescriptorSystem native_hook_descriptor_system();

// Complete433AB42B..433AB4AA. Probe the executable prefix, allocate the native
// descriptor backlink/RTTI/table shape, then publish original+8/replacement+10.
// Failed probes with no prefix and allocation failure preserve both pointers.
// Existing replacement storage is not freed by this producer.
void prepare_vtable_hook(VtableHookMetadata& metadata, void** original_table, JobMemory memory,
                         HookDescriptorSystem system);
// Complete433D2DC1. Atomically clear only replacement_table, then free the
// captured allocation beginning two pointers before it; original_table stays.
void clear_vtable_hook(VtableHookMetadata& metadata, JobMemory memory);
// Source composition of the captured module/type/base recipe and the actual
// native producer. Replacement binding is performed after this whole call.
void prepare_rtti_vtable_hook(VtableHookMetadata& metadata, std::byte* module,
                              std::uint32_t type_hash, std::uint32_t base_hash,
                              NativeImageEnvironment images, JobMemory memory,
                              HookDescriptorSystem system);

// Production import owner for the native RTTI and descriptor entries. Binding
// their source addresses does not select VM ordering or rewrite its operands.
class HookDescriptorRuntime {
public:
    HookDescriptorRuntime(JobMemory memory, HookDescriptorSystem system,
                          NativeImageEnvironment images)
        : memory_(memory), system_(system), images_(images) {}
    HookDescriptorRuntime(const HookDescriptorRuntime&) = delete;
    HookDescriptorRuntime& operator=(const HookDescriptorRuntime&) = delete;
    void prepare(VtableHookMetadata& metadata, void** original_table) const {
        prepare_vtable_hook(metadata, original_table, memory_, system_);
    }
    void clear(VtableHookMetadata& metadata) const {
        clear_vtable_hook(metadata, memory_);
    }
    NativeImageEnvironment images() const noexcept {
        return images_;
    }

private:
    JobMemory memory_;
    HookDescriptorSystem system_;
    NativeImageEnvironment images_;
};
void bind_hook_descriptor_runtime(HookDescriptorRuntime& runtime) noexcept;
#if defined(_M_IX86) || defined(__i386__)
void __fastcall prepare_vtable_hook_native(VtableHookMetadata*, void* ignored_edx,
                                           void** original_table);
void __fastcall clear_vtable_hook_native(VtableHookMetadata*, void* ignored_edx);
const std::byte* __fastcall find_pe_section_native(const std::byte* nt_header, std::uint32_t hash);
RttiTypeDescriptor* __fastcall find_rtti_type_native(std::byte* module, std::uint32_t hash);
void** __fastcall find_rtti_vtable_native(std::byte* module, RttiTypeDescriptor*,
                                          std::uint32_t base_hash);
void** __fastcall find_rtti_vtable_at_offset_native(std::byte* module, RttiTypeDescriptor*,
                                                    std::uint32_t offset);
#else
void prepare_vtable_hook_native(VtableHookMetadata*, void* ignored_edx, void** original_table);
void clear_vtable_hook_native(VtableHookMetadata*, void* ignored_edx);
const std::byte* find_pe_section_native(const std::byte* nt_header, std::uint32_t hash);
RttiTypeDescriptor* find_rtti_type_native(std::byte* module, std::uint32_t hash);
void** find_rtti_vtable_native(std::byte* module, RttiTypeDescriptor*, std::uint32_t base_hash);
void** find_rtti_vtable_at_offset_native(std::byte* module, RttiTypeDescriptor*,
                                         std::uint32_t offset);
#endif
}

// Declarations from hook_descriptor_storage.hpp

namespace skeet::engine {
// Exact cells named by the26 ordinary CRT finalizers. This is a view of the
// canonical native data storage, not another set of hook metadata objects.
inline constexpr std::array<std::uint32_t, 26> hook_descriptor_cells{
    0x43475954, 0x43475b48, 0x43475878, 0x43475998, 0x43475830, 0x4347589c, 0x43475a28,
    0x43475b60, 0x43475864, 0x43475aec, 0x43475b18, 0x43475a08, 0x43475b34, 0x43475ac4,
    0x43475908, 0x434758f4, 0x434757f4, 0x434757c8, 0x43475a3c, 0x43475b04, 0x43475a68,
    0x434757b0, 0x434757e0, 0x43475814, 0x434758b8, 0x4347591c};

class HookDescriptorStorage {
public:
    explicit HookDescriptorStorage(PreservedInitialization&);
    explicit HookDescriptorStorage(std::shared_ptr<NativeDataImage>);
    HookDescriptorStorage(const HookDescriptorStorage&) = delete;
    HookDescriptorStorage& operator=(const HookDescriptorStorage&) = delete;
    HookDescriptorStorage(HookDescriptorStorage&&) = delete;
    HookDescriptorStorage& operator=(HookDescriptorStorage&&) = delete;

    VtableHookMetadata& at(std::uint32_t native_address) const;
    const NativeDataImage& image() const noexcept {
        return *image_;
    }

private:
    std::shared_ptr<NativeDataImage> image_;
};
}

// Declarations from hook_runtime_owner.hpp

namespace skeet::client {
class CrtRuntimeOwner;
class ClientRuntimeStorage;
class PostInitializationOwner;
class MiscRuntimeOwner;
class VisualRuntimeOwner;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
class CommandCaptureRuntimeOwner;
struct EngineRuntimeBindings;
struct ChannelHookState;
}
namespace skeet::misc {
struct AutoAcceptPatchState;
struct AutoAcceptOwnerBindings;
class AutoAcceptHookFamily;
class PingSpikeChannelRuntime;
struct NetworkMessageRuntime;
}
namespace skeet::animation {
class FixRuntimeOwner;
class RuntimeOwner;
}
namespace skeet::effects {
class PlayerAuxiliaryHooks;
}
namespace skeet::script {
class ScriptOwner;
class SourceScriptRuntimeOwner;
struct ConsoleInputHook;
}
namespace skeet::ui {
class StartupRenderer;
class MainMenuWindowOwner;
}
namespace skeet::network {
class ClientStateRuntimeOwner;
}
namespace skeet::engine {
class HookDescriptorStorage;
struct HookDescriptorSystem;
struct NativeImageEnvironment;
class FocusOverlayHooks;
class BuildHookCallbacks;

// Process-lifetime hook composition shared by animation/effects owners and
// the later menu/post-initialization stage. It creates no second hook list,
// data image, script runtime, renderer, configuration or descriptor storage.
class HookRuntimeOwner {
public:
    HookRuntimeOwner(client::CrtRuntimeOwner&, script::SourceScriptRuntimeOwner&,
                     ui::StartupRenderer&);
    ~HookRuntimeOwner();
    HookRuntimeOwner(const HookRuntimeOwner&) = delete;
    HookRuntimeOwner& operator=(const HookRuntimeOwner&) = delete;
    HookRuntimeOwner(HookRuntimeOwner&&) = delete;
    HookRuntimeOwner& operator=(HookRuntimeOwner&&) = delete;
    VtableHookRegistry& registry() noexcept;
    client::ClientRuntimeStorage& storage() const noexcept;
    UnloadState& unload_state() const noexcept;
    HookDescriptorStorage& descriptor_storage() noexcept;
    HookDescriptorSystem descriptor_system() const noexcept;
    NativeImageEnvironment descriptor_images() const noexcept;
    misc::AutoAcceptPatchState& auto_accept_patch() noexcept;
    misc::AutoAcceptHookFamily& auto_accept_hooks() noexcept;
    misc::AutoAcceptOwnerBindings auto_accept_owner_bindings() const;
    script::ConsoleInputHook& console_input() noexcept;
    script::ScriptOwner& script_owner() const;
    // Compose the two channel handlers from the actual misc feature owner.
    // These contexts borrow the canonical history/acknowledgement cells and
    // live clock/message-transition aliases; no hook is installed here.
    void initialize_network(client::MiscRuntimeOwner&);
    misc::PingSpikeChannelRuntime& channel();
    client::ChannelHookState& channel_state();
    misc::NetworkMessageRuntime& messages();
    // FixRuntimeOwner itself borrows this hook registry. Construct it first,
    // then bind the shared short-hook context to its actual animation state.
    void initialize_callbacks(animation::FixRuntimeOwner&);
    BuildHookCallbacks& callbacks();
    void initialize_player_auxiliary(const client::EngineRuntimeBindings&,
                                     client::ConfigurationRuntimeOwner&,
                                     client::HistoryRuntimeOwner&, animation::RuntimeOwner&);
    effects::PlayerAuxiliaryHooks& player_auxiliary();
    void initialize_client_state(const client::EngineRuntimeBindings&,
                                 client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                                 client::CommandCaptureRuntimeOwner&,
                                 script::SourceScriptRuntimeOwner&, ui::MainMenuWindowOwner&);
    network::ClientStateRuntimeOwner& client_state();
    // This actual phase runs after RootMenu/PostInitializationOwner exist.
    // It publishes the source callbacks, resolves the actual engine panel,
    // and installs the one-shot entry. Its first real invocation initializes
    // startup, then publishes/enters the permanent HUD focus entry.
    void initialize_focus(client::PostInitializationOwner&, client::VisualRuntimeOwner&);
    FocusOverlayHooks& focus();

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
}

// Declarations from initialization_failure.hpp

namespace skeet::engine {
struct InitializationFailureState {
    std::uintptr_t& image_base; // Native 0x43467E70.
    std::uint32_t& image_size; // Native 0x434679DC.
    std::int32_t& requested_mode; // Native 0x43475A80.
};

#if defined(_M_IX86) || defined(__i386__)
using TerminateInitializationThread = std::int32_t(__stdcall*)(void*, std::int32_t);
#else
using TerminateInitializationThread = std::int32_t (*)(void*, std::int32_t);
#endif

// Recovered native 0x4337F065, reached by protected startup failure paths.
// The imported call can fail and return; its status is propagated unchanged.
std::int32_t fail_initialization(InitializationFailureState state,
                                 TerminateInitializationThread terminate_thread,
                                 std::int32_t exit_code);
}

// Declarations from interface_call.hpp

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
template <class Result, class... Arguments>
using MemberFunction = Result(__thiscall*)(void*, Arguments...);
#else
template <class Result, class... Arguments> using MemberFunction = Result (*)(void*, Arguments...);
#endif
// Source engine interfaces use the Microsoft x86 member-function ABI. The
// explicit receiver also permits host-side verification with interface spies.
template <class Result, class... Arguments>
Result invoke(void* object, std::size_t slot, Arguments... arguments) {
    if (!object)
        throw std::logic_error("Required engine interface is not bound");
    const auto table = *static_cast<void***>(object);
    return reinterpret_cast<MemberFunction<Result, Arguments...>>(table[slot])(object,
                                                                               arguments...);
}
}

// Declarations from interface_discovery.hpp

namespace skeet::engine {
// Each role is backed by an exact captured CreateInterface registration. The
// interface objects remain borrowed engine objects; this bundle owns none.
struct CoreEngineInterfaces {
    void* engine_client;
    void* entity_list;
    void* base_client;
    void* prediction;
    void* surface;
    void* cvar;
    void* engine_trace;
};
struct CoreInterfaceCells {
    void*& engine_client; //4346A514
    void*& entity_list; //4346A890
    void*& base_client; //434683E0
    void*& prediction; //43469264
    void*& surface; //434689C0
    void*& cvar; //4346A87C
    void*& engine_trace; //43468304
};

// The working dump's real PE exports, linked registration names and creator
// results prove these pairs. This is a source acquisition manifest, not a
// claim about the unrecovered protected initializer's invocation order.
std::span<const InterfaceRequest, 7> core_interface_requests() noexcept;
CoreEngineInterfaces discover_core_interfaces(FactoryProvider&);
// All seven mandatory acquisitions finish before any canonical cell changes.
CoreEngineInterfaces discover_core_interfaces(FactoryProvider&, CoreInterfaceCells);
// Uses the existing provider, which resolves exports only in loaded modules.
CoreEngineInterfaces discover_core_interfaces(CoreInterfaceCells);

struct BuildInterfaceRecipe {
    std::uint32_t operand, module_cell, interface_hash, destination_cell, destination_xor;
    InterfaceRequest request;
};
enum class BuildInterfaceFailureStage {
    module_registry,
    registration,
    creator,
    factory
};
class BuildInterfaceDiscoveryFailure final : public std::runtime_error {
public:
    static constexpr std::int32_t native_exit_code = 23;
    BuildInterfaceDiscoveryFailure(const BuildInterfaceRecipe&, BuildInterfaceFailureStage);
    const std::uint32_t operand, destination_cell;
    const BuildInterfaceFailureStage stage;
};
struct BuildEngineInterfaces {
    std::array<void*, 41> objects{};
    void* at(std::string_view role) const;
    CoreEngineInterfaces core() const noexcept;
};

// Explicit data for this build's 41 acquisition triples. Their starting
// operands are 4C8..6A8; the final destination field is operand 6B0.
// ctx.h supplies the observed module-cell/hash/destination identifiers; exact
// names, creator results and plain-pointer destination representation are
// independently checked against the working module registration lists. No
// captured process pointer or erased operand-accessor implementation is used.
std::span<const BuildInterfaceRecipe, 41> build_interface_manifest() noexcept;

// Acquire in manifest order and publish each real result into its canonical
// source cell. Earlier published results survive an explicit source failure.
// Missing acquisitions raise BuildInterfaceDiscoveryFailure; the startup
// owner routes code 23 through its actual fail_initialization state/imports.
// The module-cell route uses the recovered registry/hash helpers and genuine
// external creators. FactoryProvider supplies a separate exact-name route.
BuildEngineInterfaces discover_build_interfaces(NativeDataImage&, NativeImageEnvironment);
BuildEngineInterfaces discover_build_interfaces(NativeDataImage&);
BuildEngineInterfaces discover_build_interfaces(FactoryProvider&, NativeDataImage&);
} //namespace skeet::engine

// Declarations from jobs.hpp

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_JOB_FASTCALL __fastcall
#define SKEET_JOB_STDCALL __stdcall
#define SKEET_JOB_MEMBER __thiscall
#else
#define SKEET_JOB_FASTCALL
#define SKEET_JOB_STDCALL
#define SKEET_JOB_MEMBER
#endif

// The callback is the actual recovered item worker. Native433EDC66/433C131E
// pass the claimed item pointer in ECX; no dispatcher-owned replacement action.
using JobWork = void(SKEET_JOB_FASTCALL*)(void* item);

struct JobTask {
    void** vtable;
    std::uint32_t unknown_4;
    std::uint32_t references;
    JobWork work;
    std::uintptr_t cursor;
    std::uintptr_t end;
    std::uint32_t completed;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(JobTask) == 0x1c);
static_assert(offsetof(JobTask, references) == 8);
static_assert(offsetof(JobTask, work) == 0xc);
static_assert(offsetof(JobTask, cursor) == 0x10);
static_assert(offsetof(JobTask, end) == 0x14);
static_assert(offsetof(JobTask, completed) == 0x18);
#endif

struct JobMemory {
    using AllocateHeap = void*(SKEET_JOB_STDCALL*)(void*, std::uint32_t, std::size_t);
    using FreeHeap = unsigned char(SKEET_JOB_STDCALL*)(void*, std::uint32_t, void*);
    AllocateHeap allocate = nullptr;
    FreeHeap free = nullptr;
    void* process_heap = nullptr;
    void* purecall = nullptr; // actual linked CRT _purecall for the destroyed task table
};
JobMemory native_job_memory();
void* native_thread_pool(); // already loaded vstdlib!g_pThreadPool, dereferenced

// Complete43418461/43346F24. release calls actual task virtual[2](1) at zero.
std::int32_t retain_job(JobTask& task);
std::int32_t release_job(JobTask& task);
void consume_eight_byte_jobs(JobTask& task); // complete433EDC66 / thunk433E128F
void consume_thirty_six_byte_jobs(JobTask& task); // complete433C131E / thunk433E0A68

// The dispatcher and its vtables must outlive their tasks. Each task starts
// with the native caller reference1; caller releases it after dispatch. Tasks
// are single-use because native dispatch does not reset the completed counter.
class JobDispatcher {
public:
    JobDispatcher(void* thread_pool, JobMemory memory);
    JobDispatcher(const JobDispatcher&) = delete;
    JobDispatcher& operator=(const JobDispatcher&) = delete;
    JobTask* create_eight(JobWork work);
    JobTask* create_thirty_six(JobWork work);
    void dispatch_eight(JobTask& task, void* items, std::uint32_t count); // complete4338714F
    void dispatch_thirty_six(JobTask& task, void* items, std::uint32_t count); // complete4334B70B
    void* thread_pool() const noexcept {
        return pool_;
    }

private:
    struct TaskTable {
        std::array<void*, 4> methods;
        JobDispatcher* owner;
    } eight_, thirty_six_;
    std::array<void*, 4> destroyed_;
    void* pool_;
    JobMemory memory_;
    JobTask* create(JobWork work, TaskTable& table);
    void dispatch(JobTask& task, void* items, std::uint32_t count, std::uint32_t stride);
    static JobTask* destroy(JobTask& task, std::uint8_t flags);
    static JobTask* SKEET_JOB_MEMBER destruction_entry(void* object, std::uint8_t flags);
};

#undef SKEET_JOB_FASTCALL
#undef SKEET_JOB_STDCALL
#undef SKEET_JOB_MEMBER
} // namespace skeet::engine

// Declarations from loaded_modules.hpp

namespace skeet::engine {
struct LoaderList {
    LoaderList* next;
    LoaderList* previous;
};
struct LoaderUnicodeName {
    std::uint16_t length;
    std::uint16_t maximum_length;
    const char16_t* buffer;
};
// Exact native LDR_DATA_TABLE_ENTRY prefix accessed by433F29A5.
struct LoaderModule {
    LoaderList load_order;
    LoaderList memory_order;
    LoaderList initialization_order;
    void* base;
    void* entry;
    std::uint32_t image_size;
    LoaderUnicodeName full_name;
    LoaderUnicodeName base_name;
};
struct LoaderData {
    std::uint32_t length;
    std::uint8_t initialized;
    std::uint8_t reserved[3];
    void* ss_handle;
    LoaderList modules;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(LoaderData, modules) == 0xc);
static_assert(offsetof(LoaderModule, base) == 0x18);
static_assert(offsetof(LoaderModule, base_name) == 0x2c);
static_assert(offsetof(LoaderModule, base_name) + offsetof(LoaderUnicodeName, buffer) == 0x30);
#endif

#if defined(_M_IX86) || defined(__i386__)
using PebLockFunction = void(__stdcall*)();
#else
using PebLockFunction = void (*)();
#endif
struct LoadedModuleEnvironment {
    LoaderData* loader;
    PebLockFunction acquire_peb_lock;
    PebLockFunction release_peb_lock;
};

// Complete43381FE9: UTF16 code units, ASCII A..Z folding, FNV1a andsalt.
std::uint32_t loaded_module_name_hash(const char16_t* name, std::int32_t length) noexcept;
// Complete433F29A5. The boolean selects salted or raw FNV input; loader-lock
// imports are called only when requested and loader is nonnull.
void* find_loaded_module(const LoadedModuleEnvironment& environment, std::uint32_t hash,
                         bool salted_hash, bool lock_loader);
LoadedModuleEnvironment native_loaded_module_environment();
void* find_loaded_module(std::uint32_t hash, bool salted_hash, bool lock_loader);
} // namespace skeet::engine

// Declarations from map_game_system_owner.hpp

namespace skeet::client {
class NativeStartupSources;
}

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_MAP_SYSTEM_MEMBER __thiscall
#else
#define SKEET_MAP_SYSTEM_MEMBER
#endif

// The abstract base has13 entries, including its scalar-deleting destructor.
struct MapGameSystemBaseVtable {
    GameSystemEntry<const char*> name;
    GameSystemEntry<bool> initialize;
    GameSystemEntry<void> slot2, slot3, slot4;
    GameSystemEntry<void> map_entity_text; //43325578
    GameSystemEntry<void> slot6, slot7, slot8, slot9, slot10;
    GameSystemEntry<bool> slot11;
    GameSystemEntry<void*, std::uint8_t> destroy;
};
//4346F3B8 also includes the three IGameSystemPerFrame methods. The engine's
//client+2F2D85 calls slot14 with its frame-time argument on the stack.
struct MapGameSystemVtable {
    MapGameSystemBaseVtable base;
    GameSystemEntry<void> pre_render; //433CCE8E
    GameSystemEntry<void, float> update; //43328352
    GameSystemEntry<void> post_render; //433CCE8E
};
struct MapGameSystemEndpoint {
    const MapGameSystemBaseVtable* vtable;
};
struct MapGameSystemVector {
    MapGameSystemEndpoint** data;
    std::int32_t capacity, grow_size, size;
    MapGameSystemEndpoint** elements;
};
struct MapGameSystemOwner {
    const GameSystemEntry<void>* vtable;
    MapGameSystemVector* first;
    MapGameSystemVector* second;
    MapGameSystemEndpoint endpoint;
    MapGameSystemOwner* initialized;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(MapGameSystemBaseVtable) == 13 * 4);
static_assert(sizeof(MapGameSystemVtable) == 16 * 4);
static_assert(offsetof(MapGameSystemVtable, update) == 14 * 4);
static_assert(sizeof(MapGameSystemVector) == 0x14);
static_assert(offsetof(MapGameSystemVector, size) == 0xC);
static_assert(sizeof(MapGameSystemOwner) == 0x14);
static_assert(offsetof(MapGameSystemOwner, endpoint) == 0xC);
static_assert(offsetof(MapGameSystemOwner, initialized) == 0x10);
#endif

struct MapGameSystemBindings {
    // Actual globals contain pointers to each list's size member. They are
    // read by primary slot1; the resulting list pointers remain cached.
    std::int32_t* const& first_size; //43468D0C
    std::int32_t* const& second_size; //43467A70
    void* const& allocator; //43467204, slots1/3
    void* const& engine; //4346A514, slot99
    const OwnerRegistry& map_entity_parsers; //43475770/74/78
};

// Keep this runtime alive until all its registered owners have been destroyed.
// Source context precedes each source-owned endpoint vtable, preserving the exact
// native0x14 owner allocation and the constructor's sparse writes.
class MapGameSystemRuntime {
public:
    MapGameSystemRuntime(OwnerRegistryBinding, MapGameSystemBindings);
    MapGameSystemRuntime(const MapGameSystemRuntime&) = delete;
    MapGameSystemRuntime& operator=(const MapGameSystemRuntime&) = delete;
    MapGameSystemOwner* create(); //4348E6AA
    void initialize(MapGameSystemOwner&); //4348DAA7
    void append(MapGameSystemOwner&); //4331FED0
    void remove(MapGameSystemOwner&); //4341799E
    void destroy(MapGameSystemOwner*); //4334B488
private:
    struct PrimaryTable {
        std::array<GameSystemEntry<void>, 9> entries;
        MapGameSystemRuntime* runtime;
    } primary_;
    struct EndpointTable {
        MapGameSystemRuntime* runtime;
        MapGameSystemVtable entries;
    } endpoint_;
    struct DestroyedEndpointTable {
        MapGameSystemRuntime* runtime;
        MapGameSystemBaseVtable entries;
    } destroyed_;
    OwnerRegistryBinding owners_;
    MapGameSystemBindings bindings_;
    static MapGameSystemRuntime& runtime(MapGameSystemOwner&);
    static MapGameSystemRuntime& runtime(MapGameSystemEndpoint&);
    static void SKEET_MAP_SYSTEM_MEMBER destroy_entry(void*);
    static void SKEET_MAP_SYSTEM_MEMBER initialize_entry(void*);
    static void SKEET_MAP_SYSTEM_MEMBER append_entry(void*);
    static void SKEET_MAP_SYSTEM_MEMBER remove_entry(void*);
    static void SKEET_MAP_SYSTEM_MEMBER map_entity_text_entry(void*);
    static void SKEET_MAP_SYSTEM_MEMBER frame_update_entry(void*, float);
    static void* SKEET_MAP_SYSTEM_MEMBER destroy_endpoint_entry(void*, std::uint8_t);
};
void bind_map_game_system_factory(client::NativeStartupSources&, MapGameSystemRuntime&);

#undef SKEET_MAP_SYSTEM_MEMBER
} //namespace skeet::engine

// Declarations from module_range_initialization.hpp

namespace skeet::engine {
struct ModuleAddressRange {
    std::uint32_t low;
    std::uint32_t high;
};
static_assert(sizeof(ModuleAddressRange) == 8);

#if defined(_M_IX86) || defined(__i386__)
using ModuleRangeInitialize = void(__cdecl*)();
using ModuleRangeGet = ModuleAddressRange*(__cdecl*)();
#else
using ModuleRangeInitialize = void (*)();
using ModuleRangeGet = ModuleAddressRange* (*)();
#endif

struct ModuleRangeRowCells {
    std::uint32_t initialize;
    std::uint32_t get;
    std::uint32_t indirect;
};
// Exact static rows at43467A10. Each nonzero entry identifies a data cell,
// rather than a function or the address of a range object.
std::span<const ModuleRangeRowCells, 4> module_range_row_cells() noexcept;

struct ModuleRangeInitializationRow {
    ModuleRangeInitialize const* initialize;
    ModuleRangeGet const* get;
    ModuleAddressRange** const* indirect;
};
struct ModuleRangeInitializationBindings {
    std::array<ModuleRangeInitializationRow, 4> rows;
    const std::byte* const* client_module;
};

// Borrows the exact current cells, including their sentinel values. Active
// function cells must have been populated with genuine functions from verified
// bindings for the loaded engine build; this helper resolves no code address.
ModuleRangeInitializationBindings native_module_range_bindings(NativeDataImage&);

// Complete4334DA95. For each active row, initialize its external range table,
// then obtain its object through the getter or the indirect holder. Widen only
// the first pair whose low endpoint still equals the current client base+1000.
// Sentinel1 skips a row. Active zero callbacks/holders are invalid native input
// and are reported as invalid bindings instead of being silently skipped.
void initialize_module_ranges(const ModuleRangeInitializationBindings&);
} // namespace skeet::engine

// Declarations from native_data_image.hpp

namespace skeet::engine {
// Stable storage for the recovered x86 data layouts. Native addresses identify
// byte offsets; source-owned storage need not occupy the original address.
// This owner supplies no instructions, operand decoder or exception runtime.
class NativeDataImage {
public:
    static constexpr std::uintptr_t preferred_address = 0x43310000;
    static constexpr std::size_t image_size = 0x2FC000;
    static std::shared_ptr<NativeDataImage> create_source();
    virtual ~NativeDataImage() = default;
    NativeDataImage(const NativeDataImage&) = delete;
    NativeDataImage& operator=(const NativeDataImage&) = delete;
    NativeDataImage(NativeDataImage&&) = delete;
    NativeDataImage& operator=(NativeDataImage&&) = delete;

    std::byte* data() const noexcept {
        return mapping_.get();
    }
    std::size_t size() const noexcept {
        return mapped_size_;
    }
    template <class T> T& native_cell(std::uint32_t address) const {
        static_assert(std::is_trivially_copyable_v<T>);
        if (sizeof(void*) != 4)
            throw std::logic_error("Native data cells require their original x86 pointer layout");
        if (address < preferred_address)
            throw std::out_of_range("Native cell precedes its logical data image");
        const auto offset = std::uintptr_t(address) - preferred_address;
        if (offset > mapped_size_ || mapped_size_ - offset < sizeof(T))
            throw std::out_of_range("Native cell is outside its logical data image");
        auto* const bytes = mapping_.get() + offset;
        if (reinterpret_cast<std::uintptr_t>(bytes) % alignof(T))
            throw std::invalid_argument(
                "Native data cell alignment does not match its recovered layout");
        return *reinterpret_cast<T*>(bytes);
    }

protected:
    NativeDataImage(std::shared_ptr<std::byte>, std::size_t);

private:
    std::shared_ptr<std::byte> mapping_;
    std::size_t mapped_size_;
};

// Validate ownership before a view binds reference members into its cells.
NativeDataImage& require_native_data_image(const std::shared_ptr<NativeDataImage>&);
} //namespace skeet::engine

// Declarations from native_owner_registry_storage.hpp

namespace skeet::engine {
// These ten native begin/end/limit triplets are consumed by the ordinary
// registry-buffer finalizers. All construction, dispatch and cleanup users
// borrow the same native data bytes; this view creates no registry.
inline constexpr std::array<std::uint32_t, 10> native_owner_registry_cells{
    0x43475b78, // main lifecycle owners
    0x43475a9c, // HUD callbacks
    0x43475b94, // network callbacks
    0x43475988, // update callbacks
    0x4347584c, // packet callbacks
    0x43475ad8, // frame callbacks
    0x43475770, // map callbacks
    0x43475a84, // blur callbacks
    0x434758e0, // event callbacks
    0x43475930 // feedback position callbacks, constructor4347F752 ->433F1665
};

class NativeOwnerRegistryStorage {
public:
    explicit NativeOwnerRegistryStorage(PreservedInitialization&);
    explicit NativeOwnerRegistryStorage(std::shared_ptr<NativeDataImage>);
    NativeOwnerRegistryStorage(const NativeOwnerRegistryStorage&) = delete;
    NativeOwnerRegistryStorage& operator=(const NativeOwnerRegistryStorage&) = delete;
    NativeOwnerRegistryStorage(NativeOwnerRegistryStorage&&) = delete;
    NativeOwnerRegistryStorage& operator=(NativeOwnerRegistryStorage&&) = delete;

    OwnerRegistry& at(std::uint32_t native_begin_address) const;
    const NativeDataImage& image() const noexcept {
        return *image_;
    }

private:
    std::shared_ptr<NativeDataImage> image_;
};
}

// Declarations from native_service_table.hpp

namespace skeet::engine {
struct NativeServiceRecord {
    std::uint32_t name_hash;
    std::uint32_t function_rva;
    std::uint32_t service_number;
};
struct NativeServiceTable {
    std::uint32_t count;
    std::array<NativeServiceRecord, 1024> records;
};
static_assert(sizeof(NativeServiceRecord) == 12);
static_assert(sizeof(NativeServiceTable) == 0x3004);

#if defined(_M_IX86) || defined(__i386__)
using NativeServiceTerminateProcess = std::int32_t(__stdcall*)(void*, std::int32_t);
#else
using NativeServiceTerminateProcess = std::int32_t (*)(void*, std::int32_t);
#endif

// Complete ordinary433E9D60, called by the first startup VM on its matched
// ntdll-base branch. The valid loader contains its executable and ntdll as the
// first two InLoadOrder entries. Mapped export data must remain readable.
// Writes only count and the populated records; unused records are preserved.
// Resolves no syscall target and executes no system-service instruction.
void initialize_native_service_table(NativeServiceTable&, const LoaderData&, NativeImageEnvironment,
                                     ImageDirectoryEntry);
// Complete4331F3B6: first matching hash wins; a missing/FFFFFFFF entry calls
// the genuine ZwTerminateProcess(current process,-10). Only if that import
// returns does the original returnFFFFFFFF. Successful results retain16 bits.
std::uint32_t resolve_native_service_number(const NativeServiceTable&, std::uint32_t hash,
                                            NativeServiceTerminateProcess);
} // namespace skeet::engine

// Declarations from netvars.hpp

namespace skeet::engine {

struct RecvTable;
struct RecvProp;
struct RecvProxyData;
using RecvVarProxy = void (*)(const RecvProxyData*, void*, void*);
using ArrayLengthProxy = void (*)(void*, int, int);
using DataTableProxy = void (*)(const RecvProp*, void**, void*, int);

// Field order is copied from the ntv_* declarations embedded in
// skeet/Binary/initializer.h, byte offsets 0x4B074..0x4B8BD. The native
// 0x433ECB60 walker independently confirms prop stride, type, child and offset.
// Host-width pointers are used by private tests; the target ABI is x86.
#pragma pack(push, 4)
struct RecvProp {
    const char* name;
    std::int32_t type;
    std::int32_t flags;
    std::int32_t string_buffer_size;
    bool inside_array;
    std::uint8_t reserved[3];
    const void* extra_data;
    RecvProp* array_property;
    ArrayLengthProxy array_length_proxy;
    RecvVarProxy proxy;
    DataTableProxy data_table_proxy;
    RecvTable* data_table;
    std::int32_t offset;
    std::int32_t element_stride;
    std::int32_t element_count;
    const char* parent_array_property_name;
};

struct RecvTable {
    RecvProp* properties;
    std::int32_t property_count;
    void* decoder;
    const char* name;
    bool initialized;
    bool in_main_list;
    std::uint8_t reserved[2];
};

struct ClientClass {
    void* create;
    void* create_event;
    const char* network_name;
    RecvTable* table;
    ClientClass* next;
    std::int32_t class_id;
};

struct DataMap;
struct DataMapField {
    std::int32_t type;
    const char* name;
    std::uint32_t offset;
    std::uint8_t unknown_0c[0x10];
    DataMap* embedded;
    std::uint8_t unknown_20[0x1c];
};
struct DataMap {
    DataMapField* fields;
    std::int32_t field_count;
    const char* class_name;
    DataMap* base;
    std::uint32_t opaque_10 = 0;
    void* optimized = nullptr; // native+14, consumed by433C046C
};
#pragma pack(pop)

#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(RecvProp) == 0x3c);
static_assert(offsetof(RecvProp, type) == 4);
static_assert(offsetof(RecvProp, data_table) == 0x28);
static_assert(offsetof(RecvProp, offset) == 0x2c);
static_assert(sizeof(RecvTable) == 0x14);
static_assert(offsetof(RecvTable, property_count) == 4);
static_assert(offsetof(RecvTable, name) == 0xc);
static_assert(sizeof(ClientClass) == 0x18);
static_assert(offsetof(ClientClass, table) == 0xc);
static_assert(offsetof(ClientClass, next) == 0x10);
static_assert(sizeof(DataMapField) == 0x3c);
static_assert(offsetof(DataMapField, name) == 4);
static_assert(offsetof(DataMapField, offset) == 8);
static_assert(offsetof(DataMapField, embedded) == 0x1c);
static_assert(offsetof(DataMap, class_name) == 8);
static_assert(offsetof(DataMap, base) == 0xc);
static_assert(offsetof(DataMap, optimized) == 0x14);
#endif

inline constexpr std::int32_t recv_data_table = 6;

// Complete 0x433ECB60 on a valid acyclic receive-table graph. Offsets are
// unsigned dwords, including wraparound, and the result is an offset, not size.
std::uint32_t maximum_property_offset(const RecvTable& table, std::uint32_t base = 0,
                                      std::uint32_t minimum = 0);

// Complete native lookup helpers. Inputs are valid engine-owned lists/trees;
// these preserve first-match traversal, including duplicate hash behavior.
std::uint32_t netvar_name_hash(const char* name) noexcept; // 0x43359C70
ClientClass* find_client_class(void* base_client, std::uint32_t hash); // 0x433412C1
RecvTable* find_receive_table(RecvTable& table, std::uint32_t hash); // 0x4336AF41
RecvProp* find_receive_property(RecvTable& table, std::uint32_t hash); // 0x433F1534
// 0x43381D8D ignores properties whose name starts with the four bytes "base".
// A missing property is UINT32_MAX, and zero is a valid discovered offset.
std::uint32_t find_receive_offset(RecvTable& table, std::uint32_t hash, std::uint32_t base = 0);
// Complete 0x4340C2A3: engine class list -> recursive table -> recursive property.
RecvProp* find_receive_property(void* base_client, std::uint32_t class_hash,
                                std::uint32_t table_hash, std::uint32_t property_hash);

// Metadata operands returned by the protected 0x43493903 accessor. Resolving
// that accessor is outside this native unit; no table/property hash is invented.
struct OffsetBinding {
    std::uint32_t class_hash;
    std::uint32_t property_hash;
    std::uint32_t* xor_key;
    std::uint32_t** pointer_output;
    std::uint32_t addend = 0; // fifth metadata operand in 0x43486F71
};

// Native bodies 0x4348F24A / 0x43486F71 after their metadata reads. A missing
// class performs no writes. A missing property is encoded as the native -1.
// cursor is an existing dword-key stream, not an output buffer of offsets.
void bind_receive_offset(void* base_client, const OffsetBinding& binding, std::uint32_t*& cursor);
// Native consumers decode *pointer_output ^ xor_key ^ their compile-time key.
std::uint32_t decode_receive_offset(const std::uint32_t* pointer, std::uint32_t xor_key,
                                    std::uint32_t consumer_key) noexcept;

// Complete 0x43322A25. The table hash belongs to DataMap::class_name. The
// native lookup skips unnamed/short fields and does not add embedded offsets.
std::uint32_t find_data_map_offset(DataMap& map, std::uint32_t table_hash,
                                   std::uint32_t property_hash);

// Complete native search 0x433C1154 with its wildcard skip-table helpers.
std::byte* find_byte_pattern(std::byte* first, std::uint32_t size, const std::uint8_t* pattern,
                             std::uint32_t length, std::uint8_t wildcard);
// Complete 0x4337F95A. The 29-byte constructor pattern is at native434729E0;
// module SizeOfImage is read at e_lfanew+0x50. At most43 matches are inspected.
DataMap* find_data_map(std::byte* client_module, std::uint32_t class_hash);

struct DataMapOffsetBinding {
    DataMap* map;
    std::uint32_t table_hash;
    std::uint32_t property_hash;
    std::uint32_t* xor_key;
    std::uint32_t** pointer_output;
};
// Complete native body0x434823F9 after the protected metadata reads.
void bind_data_map_offset(const DataMapOffsetBinding& binding, std::uint32_t*& cursor);

} // namespace skeet::engine

// Declarations from netvar_initialization.hpp

namespace skeet::engine {
// Fixed-build data recovered from the supplied observed operand records.
// Names were matched independently against the working dump's engine trees;
// initial_xor_key is the pinned input image's static key, not a saved offset.
struct ReceiveOffsetRecipe {
    std::uint32_t operand;
    const char* client_class;
    const char* property;
    std::uint32_t xor_key_identity, pointer_identity, addend, initial_xor_key;
};
enum class DataMapRoot {
    player,
    weapon_base_gun
};
struct DataMapOffsetRecipe {
    std::uint32_t operand;
    DataMapRoot root;
    const char* table;
    const char* property;
    std::uint32_t xor_key_identity, pointer_identity, initial_xor_key;
};
std::span<const ReceiveOffsetRecipe, 268> build_receive_offset_recipes() noexcept;
std::span<const DataMapOffsetRecipe, 14> build_data_map_offset_recipes() noexcept;

// Source43407CDE schedule:231 receive rows,37 adjusted rows, two genuine
// datamap discoveries, then14 datamap rows. The caller supplies its seeded
// key-stream cursor in the same NativeOffsetStorage consumed by all clients.
// Missing classes/maps preserve outputs; missing properties retain native
// UINT32_MAX encoding. No decoded VM accessor or saved return is executed.
void initialize_build_netvars(void* base_client, std::byte* client_module, NativeOffsetStorage&);
} //namespace skeet::engine

// Declarations from offset_storage.hpp

namespace skeet::engine {
// Concrete source storage for the original dword globals. Keys are their
// recovered identities, not addresses to execute/dereference in the host.
struct NativeOffsetGlobals {
    std::unordered_map<std::uint32_t, std::uint32_t> keys;
    std::unordered_map<std::uint32_t, std::uint32_t*> pointers;
};
struct NativeOffsetStorage {
private:
    std::uint32_t* standalone_words_ = nullptr;

public:
    NativeOffsetStorage() noexcept : words(standalone_words_) {}
    explicit NativeOffsetStorage(std::uint32_t*& shared_words) noexcept : words(shared_words) {}
    NativeOffsetStorage(const NativeOffsetStorage&) = delete;
    NativeOffsetStorage& operator=(const NativeOffsetStorage&) = delete;
    NativeOffsetStorage(NativeOffsetStorage&&) = delete;
    NativeOffsetStorage& operator=(NativeOffsetStorage&&) = delete;
    // The source owner and terminal cleanup share the native434797B0 cell.
    // A view neither adopts nor resets an existing allocation.
    std::uint32_t*& words;
    std::uint32_t* cursor = nullptr;
    NativeOffsetGlobals globals;
    DiscoveredDataMaps maps;
    // Native43477B54,43477B58,43477B48,43476B44; actual proxy-field slots.
    std::array<RecvVarProxy*, 4> property_proxy_slots{};
};

// Complete4336F520 with its RDTSC low-dword supplied as a hardware input.
// Word0 is untouched; words1..1023 use the original32bit LCG.
void seed_native_offset_words(std::span<std::uint32_t, 0x400> words,
                              std::uint32_t timestamp_low) noexcept;
// Complete43400DD4: publish current key-word pointer, XOR the destination key,
// then advance the cursor by one dword.
void publish_native_offset(std::uint32_t& key, std::uint32_t*& pointer,
                           std::uint32_t*& cursor) noexcept;

// Complete native4348D41D..4348D9F0: replace native key allocation, seed it,
// bind receive/datamap offsets and actual proxy slots, derive29 additional
// offsets and publish their key-word pointers. The metadata accessor and
// already decoded globals remain real initialization inputs.
void initialize_native_offset_storage(NativeOffsetStorage& storage, JobMemory memory,
                                      void* base_client, std::byte* client_module,
                                      InitializationMetadata& metadata,
                                      std::uint32_t timestamp_low);
// Same complete ordinary allocation/LCG/proxy/derived-offset phase, using the
// typed fixed-build netvar manifest instead of the protected operand accessor.
void initialize_build_offset_storage(NativeOffsetStorage& storage, JobMemory memory,
                                     void* base_client, std::byte* client_module,
                                     std::uint32_t timestamp_low);
} // namespace skeet::engine

// Declarations from opaque_native_word.hpp

namespace skeet::engine {
#if defined(_MSC_VER)
#define SKEET_OPAQUE_INLINE __forceinline
#else
#define SKEET_OPAQUE_INLINE [[gnu::always_inline]] inline
#endif
// Some recovered native branches observe their own still-unwritten stack
// slots. Read the actual invocation's bytes through a machine load, rather
// than evaluating an indeterminate C++ float/pointer or inventing initial
// values. These are ordinary current-process loads, never captured bytes.
SKEET_OPAQUE_INLINE std::uint32_t read_opaque_native_word(const void* address) noexcept {
    std::uint32_t value;
#if defined(_MSC_VER) && defined(_M_IX86)
    __asm {mov eax,address mov eax,[eax] mov value,eax}
#elif defined(__i386__) || defined(__x86_64__)
    __asm__ __volatile__("movl (%1),%0" : "=r"(value) : "r"(address) : "memory");
#else
#error Opaque native stack loads require the supported x86 instruction set.
#endif
    return value;
}
SKEET_OPAQUE_INLINE float read_opaque_native_float(const void* address) noexcept {
    return std::bit_cast<float>(read_opaque_native_word(address));
}
SKEET_OPAQUE_INLINE void* read_opaque_native_pointer(const void* address) noexcept {
    std::uintptr_t bits = read_opaque_native_word(address);
    if constexpr (sizeof(std::uintptr_t) == 8)
        bits |= std::uintptr_t(read_opaque_native_word(static_cast<const std::byte*>(address) + 4))
                << 32;
    return reinterpret_cast<void*>(bits);
}
#undef SKEET_OPAQUE_INLINE
} //namespace skeet::engine

// Declarations from overlay_build.hpp

namespace skeet::engine {
struct OverlayBuildIdentity {
    std::uint32_t timestamp, checksum;
};
inline OverlayBuildIdentity require_overlay_build_identity(const std::byte* base) {
    const auto read = [&](std::size_t offset) {
        std::uint32_t value;
        std::memcpy(&value, base + offset, sizeof value);
        return value;
    };
    if (!base || static_cast<unsigned>(base[0]) != 0x4D || static_cast<unsigned>(base[1]) != 0x5A ||
        read(0x3C) != 0x130 || read(0x130) != 0x4550)
        throw std::runtime_error("Unsupported Steam overlay PE32 header layout");
    const auto timestamp = read(0x138);
    // Both installed builds retain the verified renderer producer, stream
    // callback patterns and CSharedMemStream RTTI. Each consumer still checks
    // its exact instruction/data relationships after this identity selection.
    switch (timestamp) {
    case 0x6AF24B4E:
        return {0x6AF24B4E, 0x16A236};
    case 0x6AF66EF3:
        return {0x6AF66EF3, 0x169F3A};
    default:
        char message[160];
        std::snprintf(message, sizeof message,
                      "Unsupported Steam overlay build: timestamp 0x%08X, checksum 0x%08X",
                      timestamp, read(0x188));
        throw std::runtime_error(message);
    }
}
}

// Declarations from owner_allocation.hpp

namespace skeet::engine {
//43387448: zeroed process-heap storage with a16-byte aligned owner and the
//displacement/total-size words consumed by433837D0. Returns null on failure.
void* allocate_owner_storage(JobMemory, std::size_t owner_size);
//Destroy the C++ object before releasing its native allocation header.
void release_owner_storage(JobMemory, void*) noexcept;
}

// Declarations from owner_registry.hpp

namespace skeet::engine {
// The three native pointer globals43475B78/7C/80. This vector borrows the
// registered owners; its allocation contains only their pointers.
struct OwnerRegistry {
    void** begin = nullptr;
    void** end = nullptr;
    void** limit = nullptr;
    std::span<void* const> entries() const noexcept {
        return {begin, begin == end ? 0u : static_cast<std::size_t>(end - begin)};
    }
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(OwnerRegistry) == 0xc);
#endif

// Complete433F1665 and its4334DC99 growth path. The input pointer can alias
// existing vector storage; its value is read after allocation/copy, beforefree.
void append_owner(OwnerRegistry& registry, void* const* value, JobMemory memory);
// Complete433676A0. Constructors invoke this before their vtables/configuration
// are initialized. Registration order follows those actual constructor calls.
void* register_owner(OwnerRegistry& registry, void* object, JobMemory memory);
// Standalone native dispatchers; each captures its own begin/end once.
void dispatch_owner_slot_six(OwnerRegistry& registry); // 43328BEA
void dispatch_owner_slot_eight(OwnerRegistry& registry); // 433DF17B

struct OwnerRegistryBinding {
    OwnerRegistry& registry;
    JobMemory memory;
    void* append(void* object) const {
        return register_owner(registry, object, memory);
    }
    std::span<void* const> entries() const noexcept {
        return registry.entries();
    }
};
}

// Declarations from pe_exports.hpp

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
using ImageNtHeader = const std::byte*(__stdcall*)(const std::byte* module);
using InterfaceConstructor = void*(__cdecl*)();
#else
using ImageNtHeader = const std::byte* (*)(const std::byte* module);
using InterfaceConstructor = void* (*)();
#endif
struct NativeImageEnvironment {
    ImageNtHeader image_nt_header;
};
NativeImageEnvironment native_image_environment();

// Complete4334AC0B. PE32 only; hash0 selects the ordinal argument. Forwarded
// exports are rejected as in the native function instead of being followed.
std::byte* resolve_pe_export(std::byte* module, std::uint32_t hash, std::uint32_t ordinal,
                             bool salted_hash, NativeImageEnvironment environment);
std::byte* resolve_pe_export(std::byte* module, std::uint32_t hash, std::uint32_t ordinal,
                             bool salted_hash);

struct InterfaceRegistration {
    InterfaceConstructor create;
    const char* name;
    InterfaceRegistration* next;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(InterfaceRegistration) == 0xc);
#endif
// Complete43328E9C. Resolves CreateInterface by its native hash and recovers
// the actual InterfaceReg list from either observed x86 compiler prologue.
InterfaceRegistration* find_interface_registry(std::byte* module,
                                               NativeImageEnvironment environment);
InterfaceRegistration* find_interface_registry(std::byte* module);

// Native registry loop43329359..43329374, also used byLua create_interface.
// This exact-name hash lookup never probes versions or alternate modules.
InterfaceRegistration* find_interface_registration(InterfaceRegistration* first,
                                                   std::uint32_t hash) noexcept;
} // namespace skeet::engine

// Declarations from player_hook_family.hpp

namespace skeet::animation {
class FixAnimationsRuntime;
}
namespace skeet::client {
struct CommandCreateMoveRuntime;
class PlayerCommandQueries;
class PlayerPhysicsHooks;
class LocalPlayerAnimationHooks;
class PlayerCommandHooks;
}
namespace skeet::effects {
struct EntityVisibilityRuntime;
struct FreeLookCaptureBindings;
}
namespace skeet::prediction {
struct PlayerSimulationBindings;
}

namespace skeet::engine {
enum class PlayerPrimaryKind {
    local,
    remote
};

inline constexpr std::uint32_t player_primary_module_identity = 0x4346a55c;
inline constexpr std::uint32_t player_primary_rtti_hash = 0x1b1a0d29; //.?AVC_CSPlayer@@
inline constexpr std::uint32_t player_primary_rtti_base_hash = 0;
inline constexpr std::uint32_t local_player_descriptor_identity = 0x43475954;
inline constexpr std::uint32_t remote_player_descriptor_identity = 0x43475830;
inline constexpr std::uint32_t local_player_replacement_stream = 0x6d0;
inline constexpr std::uint32_t remote_player_replacement_stream = 0xf00;

struct PlayerPrimaryNativeReplacement {
    std::uint32_t slot, native_entry, entry_operand;
};
// Captured contexts1812..1855, in the actual22-entry replacement-loop order.
inline constexpr std::array<PlayerPrimaryNativeReplacement, 22> local_player_native_replacements{
    {{78, 0x433842c1, 0x6d4},  {88, 0x4334d3eb, 0x6dc},  {109, 0x43402307, 0x6e4},
     {113, 0x4340191d, 0x6ec}, {145, 0x4333181e, 0x6f4}, {155, 0x433eb3f4, 0x6fc},
     {170, 0x433e97c1, 0x704}, {173, 0x4333c562, 0x70c}, {207, 0x4335c639, 0x714},
     {190, 0x43330f73, 0x71c}, {198, 0x43388201, 0x724}, {222, 0x43341fa7, 0x72c},
     {224, 0x43329436, 0x734}, {268, 0x433c6689, 0x73c}, {277, 0x4335b798, 0x744},
     {289, 0x4334f53d, 0x74c}, {366, 0x433c82dc, 0x754}, {294, 0x433e009a, 0x75c},
     {230, 0x4331e412, 0x764}, {337, 0x433ccea1, 0x76c}, {298, 0x433ab4b6, 0x774},
     {350, 0x433d18b0, 0x77c}}};
// Captured contexts1894..1913, in the actual10-entry replacement-loop order.
inline constexpr std::array<PlayerPrimaryNativeReplacement, 10> remote_player_native_replacements{
    {{78, 0x433abbf0, 0xf04},
     {109, 0x43402307, 0xf0c},
     {113, 0x433875be, 0xf14},
     {145, 0x4333181e, 0xf1c},
     {173, 0x4333c562, 0xf24},
     {190, 0x43330f73, 0xf2c},
     {198, 0x43388201, 0xf34},
     {194, 0x4336ee16, 0xf3c},
     {169, 0x43382222, 0xf44},
     {207, 0x4335c639, 0xf4c}}};

std::span<const PlayerPrimaryNativeReplacement>
    player_primary_native_replacements(PlayerPrimaryKind);
// Early descriptor phase: write only source entry addresses. Actual runtime
// publication belongs to PlayerHookFamily after its feature owners exist.
void bind_player_primary_static_entries(VtableHookMetadata&, PlayerPrimaryKind);

struct PlayerPrimaryHookRuntimes {
    animation::FixAnimationsRuntime& animations;
    client::CommandCreateMoveRuntime& commands;
    const prediction::PlayerSimulationBindings& simulation;
    const effects::FreeLookCaptureBindings& free_look;
    effects::EntityVisibilityRuntime& visibility;
    client::PlayerCommandQueries& queries;
    client::PlayerPhysicsHooks& physics;
    client::LocalPlayerAnimationHooks& animation_updates;
    client::PlayerCommandHooks& command_hooks;
};

// Both primary-player recipes require all concrete source owners. Per-base
// renderable/networkable/thinkable descriptors retain their separate recipes.
// All bindings, metadata and this family must remain alive until the canonical
// hook-list restoration has finished. No runtime or global state is copied.
class PlayerHookFamily {
public:
    PlayerHookFamily(VtableHookMetadata& local, VtableHookMetadata& remote,
                     VtableHookRegistry& hooks, std::byte* const& client_module,
                     NativeImageEnvironment images, HookDescriptorSystem system,
                     PlayerPrimaryHookRuntimes runtimes) noexcept;
    PlayerHookFamily(const PlayerHookFamily&) = delete;
    PlayerHookFamily& operator=(const PlayerHookFamily&) = delete;

    // Full source recipe: genuine RTTI resolution and descriptor cloning,
    // followed by every captured22/10 source replacement in its native order.
    // Do not run a preserved native replacement loop after this operation.
    void prepare(PlayerPrimaryKind);
    // Also used after an externally completed descriptor preparation. The
    // whole mapping is repeated immediately before the single install below.
    void bind_replacements(PlayerPrimaryKind);
    VtableHookNode* install(PlayerPrimaryKind, void* actual_player);
    VtableHookMetadata& metadata(PlayerPrimaryKind) const;
    // The receiver must already have been restored by the canonical hook list.
    void clear(PlayerPrimaryKind);

private:
    VtableHookMetadata& local_;
    VtableHookMetadata& remote_;
    VtableHookRegistry& hooks_;
    std::byte* const& client_module_;
    NativeImageEnvironment images_;
    HookDescriptorSystem system_;
    PlayerPrimaryHookRuntimes runtimes_;
};
} // namespace skeet::engine

// Declarations from preserved_initialization.hpp

namespace skeet::engine {
// An already mapped input image and the lifetime of its installed live
// VM/exception environment. Mapping, imports, globals and exception dispatch
// are supplied by that environment, not constructed by this adapter.
//
// In the supplied bytes43493903 begins CC040051CC1000, so the image alone is
// not a callable operand decoder. The environment must implement its actual
// live transfer; the old ctx.h register/return-value replay is not that input.
class PreservedInitializationImage final : public NativeDataImage {
public:
    PreservedInitializationImage(std::shared_ptr<std::byte> mapped_image, std::size_t mapped_size,
                                 std::shared_ptr<const void> live_exception_environment);
    PreservedInitializationImage(const PreservedInitializationImage&) = delete;
    PreservedInitializationImage& operator=(const PreservedInitializationImage&) = delete;
    PreservedInitializationImage(PreservedInitializationImage&&) = delete;
    PreservedInitializationImage& operator=(PreservedInitializationImage&&) = delete;

private:
    std::shared_ptr<const void> exception_environment_;
};

// Required live protected-code boundary for ordinary source initialization.
// This class never decodes VM bytecode or supplies captured operand values.
// Entry-prefix checks do not establish that the remaining VM/environment is
// complete; its loader, exception handler and VM-to-source translation remain
// separate required work.
class PreservedInitialization final : public InitializationMetadata {
public:
    explicit PreservedInitialization(std::shared_ptr<PreservedInitializationImage>);
    PreservedInitialization(const PreservedInitialization&) = delete;
    PreservedInitialization& operator=(const PreservedInitialization&) = delete;
    PreservedInitialization(PreservedInitialization&&) = delete;
    PreservedInitialization& operator=(PreservedInitialization&&) = delete;

    //43493903: the ordinary caller passes one stack word and resumes without
    //adjusting ESP. The live environment must supply that effective return/
    //cleanup contract. No actual RET4 instruction has been proved at this
    //INT3 entry, and the old captured-return handler is not used here.
    std::uintptr_t operand(std::uint32_t address) override;
    void last_startup_callback(); //operand2D4,43484F06 ->4354DB7E
    const PreservedInitializationImage& image() const noexcept {
        return *image_;
    }
    std::shared_ptr<PreservedInitializationImage> image_owner() const noexcept {
        return image_;
    }

private:
    std::shared_ptr<PreservedInitializationImage> image_;
};
}

// Declarations from rtti.hpp

namespace skeet::engine {
struct RttiClassHierarchy;
struct RttiTypeDescriptor {
    void* vtable;
    void* spare;
    char name[1];
};
struct RttiBaseClass {
    RttiTypeDescriptor* type;
    std::uint32_t contained_bases;
    std::int32_t member_displacement, virtual_base_displacement, virtual_displacement;
    std::uint32_t attributes;
    RttiClassHierarchy* hierarchy;
};
struct RttiClassHierarchy {
    std::uint32_t signature, attributes, count;
    RttiBaseClass** bases;
};
struct RttiObjectLocator {
    std::uint32_t signature, offset, constructor_displacement;
    RttiTypeDescriptor* type;
    RttiClassHierarchy* hierarchy;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(RttiTypeDescriptor, name) == 8);
static_assert(offsetof(RttiObjectLocator, type) == 0xc);
static_assert(offsetof(RttiObjectLocator, hierarchy) == 0x10);
static_assert(offsetof(RttiBaseClass, member_displacement) == 8);
static_assert(offsetof(RttiBaseClass, hierarchy) == 0x18);
static_assert(offsetof(RttiClassHierarchy, bases) == 0xc);
#endif

// Complete433A1085: section names are copied from their eight-byte PE field
// and compared with the native ASCII-lowercase salted FNV hash4334DEFF.
const std::byte* find_pe_section(const std::byte* nt_header, std::uint32_t name_hash);
// Complete433B305B: scan .data for aligned .?AV names, returning name-8.
RttiTypeDescriptor* find_rtti_type(std::byte* module, std::uint32_t type_hash,
                                   NativeImageEnvironment images);
// Complete4332CE3A. A nonzero base hash selects its real member displacement
// through the native hierarchy/32-locator collection; zero selects offset0.
void** find_rtti_vtable(std::byte* module, RttiTypeDescriptor* type, std::uint32_t base_hash,
                        NativeImageEnvironment images);
// Complete433EC4BD: direct locator-offset lookup, requiring cdOffset0.
void** find_rtti_vtable_at_offset(std::byte* module, RttiTypeDescriptor* type, std::uint32_t offset,
                                  NativeImageEnvironment images);
// Composition of the recovered type and vtable finders. The module, type hash
// and base hash are the actual first three captured descriptor recipe values.
void** resolve_rtti_vtable(std::byte* module, std::uint32_t type_hash, std::uint32_t base_hash,
                           NativeImageEnvironment images);
}

// Declarations from scene_queries.hpp

namespace skeet::engine {

// One set of actual engine pointer cells is shared by all three adapters.
// The cells and canonical state outlive this owner; interface replacements are
// observed by the adapters at each recovered call site.
struct SceneQueryBindings {
    void* const& engine_trace; //43468304
    void* const& surface_properties; //43469268
    void* const& entity_list; //4346A890
    void* const& static_props; //43467014
    void* const& model_info; //4346A614
    void* const& model_cache; //43469330
    void* const& spatial_partition; //43467F10
    void* const& player_resource_slot; //43468FF0
    const std::int32_t* const& maximum_clients; //43469300
    void* const* const& skip_filter_vtable; //434758B4
    void* const* const& entity_filter_vtable; //43475B90
    const NativeOffsetGlobals& offsets;
    const players::State& player_adjustments;
    void* const& primary_breakable_class; //4346A440
    void* const& alternate_breakable_class; //43467478
    const void* const& smoke_projectile_class; //43468238
    const void* const& base_projectile_class; //43468E30
    const void* const& molotov_projectile_class; //43467E88
    const void* const& inferno_class; //434674E8
    EngineBreakableFunction const& breakable; //43468F30
    NativeConVarReference game_type; //43467A8C^AC9299CC
    NativeConVarReference teammates_are_enemies; //4346A568^9190E2E6
    void* const& grenade_alternate_breakable_class; //43468CA0
    void* const& grenade_slope_exempt_class; //4346719C
    const float& tick_interval; //*4346A338
    const float& frame_interval; //43467B04 pointee+20
    NativeConVarReference gravity_convar; //43467A54^B882FC15
    NativeConVarReference incendiary_duration_convar; //43467048^983081BF
    NativeConVarReference incendiary_max_slope_convar; //43467088^8FD87158
};

// Source composition after the engine cells have been bound. Construction
// retains dependencies only; native initialization and owner registration
// must still execute in their recovered order. No client logic runs here.
class SceneQueries {
public:
    explicit SceneQueries(const SceneQueryBindings&);
    SceneQueries(const SceneQueries&) = delete;
    SceneQueries& operator=(const SceneQueries&) = delete;
    SceneQueries(SceneQueries&&) = delete;
    SceneQueries& operator=(SceneQueries&&) = delete;

    EngineMovementCollision& movement() noexcept {
        return movement_;
    }
    EngineBallistics& ballistics() noexcept {
        return ballistics_;
    }
    esp::EngineGrenades& grenades() noexcept {
        return grenades_;
    }
    const NativeOffsetGlobals& offsets() const noexcept {
        return bindings_.offsets;
    }
    const esp::GrenadeEnvironmentBindings& environment() const noexcept {
        return grenade_environment_;
    }

private:
    SceneQueryBindings bindings_;
    MovementEngineCollisionBindings movement_bindings_;
    EngineBallisticBindings ballistic_bindings_;
    esp::GrenadeEngineBindings grenade_bindings_;
    esp::GrenadeEnvironmentBindings grenade_environment_;
    EngineMovementCollision movement_;
    EngineBallistics ballistics_;
    esp::EngineGrenades grenades_;
};

} // namespace skeet::engine

// Declarations from scene_runtime_owner.hpp

namespace skeet::client {
class ClientRuntimeStorage;
class PlayerRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::engine {
// Real source adapters used by movement, bullet penetration and grenade
// prediction. All interface, class, clock and ConVar cells stay live.
class SceneQueryRuntimeOwner {
public:
    SceneQueryRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                           client::PlayerRuntimeOwner&);
    SceneQueryRuntimeOwner(const SceneQueryRuntimeOwner&) = delete;
    SceneQueryRuntimeOwner& operator=(const SceneQueryRuntimeOwner&) = delete;
    SceneQueryRuntimeOwner(SceneQueryRuntimeOwner&&) = delete;
    SceneQueryRuntimeOwner& operator=(SceneQueryRuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    client::PlayerRuntimeOwner& players() const noexcept {
        return players_;
    }
    SceneQueries& queries() noexcept {
        return queries_;
    }

private:
    client::ClientRuntimeStorage& storage_;
    client::PlayerRuntimeOwner& players_;
    SceneQueryBindings bindings_;
    SceneQueries queries_;
};
} //namespace skeet::engine

// Declarations from source_data_initialization.hpp

namespace skeet::engine {
// Reconstruct only the proven static defaults needed by these data consumers.
// Call once on newly-created source storage, before any producer or parser.
// This does not discover interfaces, initialize owners or complete startup.
void initialize_source_native_data(NativeDataImage&);
// Proven static metadata for the five typed configuration views borrowed by
// source runtime owners. Existing-data views never invoke this initializer.
void initialize_source_configuration_data(NativeDataImage&);
} //namespace skeet::engine

// Declarations from source_initialization_imports.hpp

namespace skeet::engine {
// Genuine process imports borrowed from already-loaded Windows modules. The
// bundle resolves dependencies only; it constructs no owner, installs no hook
// and performs no startup callback or native data publication.
struct SourceInitializationImports {
    JobMemory memory;
    LoadedModuleEnvironment modules;
    NativeImageEnvironment images;
    ForwardedExportEnvironment forwarded_exports;
    NativeServiceTerminateProcess terminate_process;
    CommonControlsExports common_controls;

    FirstStartupServiceBindings first_startup_bindings(NativeDataImage&) const noexcept;
};

// Reuses the existing native process-heap, PE and loaded-module factories.
// Required modules/exports must already exist; this never loads a library.
SourceInitializationImports native_source_initialization_imports();
} //namespace skeet::engine

// Declarations from startup_container_cleanup.hpp

namespace skeet::client {
class NativeStartupSources;
struct ExitCallbacks;
}
namespace skeet::ui {
class RendererDeviceListener;
class RendererContainer;
}

namespace skeet::engine {
struct UnloadState;

// These are borrowed native allocation headers, not std::vector layouts. The
// terminal helpers preserve begin/end/limit and the fourth word after freeing.
// They do not destroy the separate STL-backed preset/catalog/script objects;
// their native allocation producers require an explicit ownership migration.
template <class T> struct StartupOwnedVector {
    T* begin;
    T* end;
    T* limit;
    std::uint32_t preserved;
};
using StartupOwnedPointers = StartupOwnedVector<void*>;
using StartupOwnedTexts = StartupOwnedVector<misc::NativeText>;

// Only the prefix touched by4334D2C1 is interpreted. Source bytes are freed
// first, then the name. Neither field is cleared before freeing the object.
struct StartupModulePrefix {
    std::array<std::byte, 0xC> preserved;
    misc::NativeText name;
    void* source;
};
using StartupOwnedModules = StartupOwnedVector<StartupModulePrefix*>;
struct StartupTextBucketNode {
    misc::NativeText text;
    std::uint32_t preserved;
    StartupTextBucketNode* next;
};
struct StartupPointerBucketNode {
    std::array<std::uint32_t, 3> preserved;
    StartupPointerBucketNode* next;
};
template <class Node> struct StartupBucketTable {
    std::uintptr_t preserved;
    Node** buckets;
    std::uint32_t bucket_count;
    std::uint32_t size;
};
using StartupTextBuckets = StartupBucketTable<StartupTextBucketNode>;
using StartupPointerBuckets = StartupBucketTable<StartupPointerBucketNode>;
#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(StartupOwnedPointers) == 0x10);
static_assert(sizeof(StartupOwnedTexts) == 0x10);
static_assert(sizeof(StartupOwnedModules) == 0x10);
static_assert(offsetof(StartupModulePrefix, name) == 0xC);
static_assert(offsetof(StartupModulePrefix, source) == 0x1C);
static_assert(offsetof(StartupTextBucketNode, next) == 0x14);
static_assert(offsetof(StartupPointerBucketNode, next) == 0xC);
static_assert(offsetof(StartupTextBuckets, buckets) == 4);
static_assert(offsetof(StartupTextBuckets, bucket_count) == 8);
static_assert(offsetof(StartupTextBuckets, size) == 0xC);
static_assert(sizeof(StartupTextBuckets) == 0x10);
static_assert(sizeof(StartupPointerBuckets) == 0x10);
#endif

void finalize_startup_owned_pointers(StartupOwnedPointers&, JobMemory); //43393934
void finalize_startup_owned_texts(StartupOwnedTexts&, JobMemory); //43383DAD
void finalize_startup_owned_modules(StartupOwnedModules&, JobMemory); //433C8710
void finalize_startup_text_buckets(StartupTextBuckets&, JobMemory); //4334320D
void finalize_startup_pointer_buckets(StartupPointerBuckets&, JobMemory); //433F0A02

struct StartupContainerCleanupBindings {
    client::ExitCallbacks& callbacks;
    JobMemory memory;
    StartupOwnedPointers& preset_hotkeys; //43478DA8
    StartupOwnedTexts& agent_paths_ct; //43479088
    StartupOwnedTexts& agent_paths_t; //43479078
    StartupOwnedModules& builtin_modules; //434792E0
    StartupOwnedModules& required_modules; //434792D0
    StartupTextBuckets& text_buckets; //4346D700
    StartupPointerBuckets& pointer_buckets; //4346DA40
    // This exact listener exists before renderer discovery and follows the
    // canonical nullable renderer slot. Registration shares UnloadState.
    ui::RendererDeviceListener& renderer_listener;
    UnloadState& unload;
};

// Keep this owner and all borrowed storage alive through ExitCallbacks::run().
// No-argument finalizers select this one process-lifetime source context.
class StartupContainerCleanup {
public:
    explicit StartupContainerCleanup(StartupContainerCleanupBindings);
    StartupContainerCleanup(const StartupContainerCleanup&) = delete;
    StartupContainerCleanup& operator=(const StartupContainerCleanup&) = delete;
    client::ExitCallbacks& exit_callbacks() const noexcept {
        return bindings_.callbacks;
    }
    void append_finalizer(std::uint32_t native_finalizer) const;
    void (*source_callback(std::uint32_t native_finalizer) const)();
    const StartupContainerCleanupBindings& bindings() const noexcept {
        return bindings_;
    }

private:
    StartupContainerCleanupBindings bindings_;
    static const StartupContainerCleanup& bound();
    static void preset_hotkeys(); //4333087C
    static void renderer_listener(); //433D2E2B ->433D0304
    static void agent_paths_ct(); //4331ECD4
    static void agent_paths_t(); //433F4BCB
    static void builtin_modules(); //43336C3C
    static void required_modules(); //4334D2AB
    static void text_buckets(); //4335851B
    static void pointer_buckets(); //433ACF65
};
void bind_startup_container_cleanup(client::NativeStartupSources&, StartupContainerCleanup&);
} //namespace skeet::engine

// Declarations from startup_convars.hpp

namespace skeet::client {
struct StartupConVars;
}
namespace skeet::engine {
// A live view of one pointer cell. Production binds the canonical encoded
// word and its key. Standalone decoded cells are borrowed by address without
// aliasing their pointer type or retaining a snapshot of the pointed object.
class NativeConVarReference {
public:
    NativeConVarReference(const std::uint32_t& encoded, std::uint32_t key) noexcept
        : storage_(&encoded), key_(key), representation_(Representation::encoded_word) {}
    NativeConVarReference(std::uint32_t&&, std::uint32_t) = delete;
    NativeConVarReference(const std::uint32_t&&, std::uint32_t) = delete;
    template <class Pointer>
        requires(std::is_pointer_v<Pointer> && !std::is_volatile_v<Pointer> &&
                 (std::is_object_v<std::remove_pointer_t<Pointer>> ||
                  std::is_void_v<std::remove_pointer_t<Pointer>>))
    NativeConVarReference(Pointer& decoded) noexcept
        : storage_(&decoded), key_(0), representation_(Representation::decoded_pointer) {
        static_assert(sizeof(Pointer) == sizeof(void*));
    }
    void* get() const noexcept {
        if (representation_ == Representation::encoded_word) {
            std::uint32_t encoded;
            std::memcpy(&encoded, storage_, sizeof encoded);
            return reinterpret_cast<void*>(std::uintptr_t(encoded ^ key_));
        }
        void* decoded;
        std::memcpy(&decoded, storage_, sizeof decoded);
        return decoded;
    }
    operator void*() const noexcept {
        return get();
    }

private:
    enum class Representation {
        encoded_word,
        decoded_pointer
    };
    const void* storage_;
    std::uint32_t key_;
    Representation representation_;
};
// Explicit decoded semantic objects and canonical live pointer-cell views are
// different input profiles. Production passes NativeConVarReference directly.
class ConVarBinding {
public:
    ConVarBinding(const void* decoded) noexcept : value_(decoded) {}
    ConVarBinding(NativeConVarReference live) noexcept : value_(live) {}
    const void* get() const noexcept {
        if (const auto* live = std::get_if<NativeConVarReference>(&value_))
            return live->get();
        return std::get<const void*>(value_);
    }
    operator const void*() const noexcept {
        return get();
    }

private:
    std::variant<const void*, NativeConVarReference> value_;
};
struct StartupConVarSlot {
    std::uint32_t address, xor_key;
    const char* name;
};
std::span<const StartupConVarSlot, 9> startup_convar_slots() noexcept;

// Only the genuine ICvar lookup boundary is replaceable by an isolated test.
class StartupConVarProvider {
public:
    virtual ~StartupConVarProvider() = default;
    virtual void* find_variable(void* cvar, const char* exact_name) = 0;
};
class StartupConVarBindings {
public:
    NativeDataImage& data() const noexcept {
        return data_;
    }
    client::StartupConVars startup_variables() const;
    NativeConVarReference accuracy_variable() const;

private:
    explicit StartupConVarBindings(NativeDataImage& data) noexcept : data_(data) {}
    NativeDataImage& data_;
    friend class BuildStartupConVarBindings;
    friend StartupConVarBindings discover_startup_convars(NativeDataImage&, void*,
                                                          StartupConVarProvider&);
};

// Resolve all eight post-initialization variables and the menu accuracy
// variable before publishing any canonical word. The default calls ICvar15.
StartupConVarBindings discover_startup_convars(NativeDataImage&, void* cvar,
                                               StartupConVarProvider&);
StartupConVarBindings discover_startup_convars(NativeDataImage&, void* cvar);
} //namespace skeet::engine

// Declarations from startup_operations.hpp

namespace skeet::engine {
struct VtableHookNode;
struct LoadedModuleEnvironment;

// Recovered portions of VM entry 0x4354DB7E. The enclosing callback has further
// operations; these helpers do not replace its complete initialization order.
void clear_startup_process_flag(volatile std::uint32_t& process_flags) noexcept;
std::byte* find_startup_jump_target(std::byte* client_module, std::uint32_t timestamp_low);
void initialize_startup_hook_list(VtableHookNode& sentinel) noexcept;
std::int64_t startup_unix_seconds(std::uint64_t file_time) noexcept;
std::byte* take_startup_common_controls(volatile std::uint32_t& module_hash,
                                        const LoadedModuleEnvironment& modules);
}

// Declarations from startup_vector_cleanup.hpp

namespace skeet::client {
struct ExitCallbacks;
class NativeStartupSources;
struct NativePlayerStateLayout;
}
namespace skeet::lagcomp {
struct LagRecord;
}

namespace skeet::engine {
// Borrowed native layouts, not additional containers. Producers and these
// terminal destructors must address the same canonical x86 data cells.
struct StartupCallbackNode {
    std::uint32_t key;
    std::uint8_t value;
    std::array<std::byte, 3> padding;
    StartupCallbackNode* next;
};
struct StartupCallbackMap {
    StartupCallbackNode* first_bucket;
    StartupCallbackNode** buckets;
    std::uint32_t bucket_count, size;
};
template <class T> struct StartupVector {
    T* begin;
    T* end;
    T* limit;
};
template <class T> struct StartupInlineVector {
    T* begin;
    T* end;
    T* limit;
    std::uint32_t preserved;
    T* inline_buffer;
};
using StartupV8Slots = StartupVector<void**>;
struct StartupV8Handle {
    void** handle;
    std::uint8_t owns_handle;
    std::array<std::byte, 3> padding;
};
struct StartupUiBinding {
    void* widget;
    void* configuration;
    StartupVector<std::int32_t> references;
    std::uint32_t preserved;
    std::uint8_t borrowed;
    std::array<std::byte, 3> padding;
};
struct StartupWideText {
    char16_t* begin;
    char16_t* end;
    char16_t* limit;
    std::uint32_t preserved;
};
struct StartupReferenceString {
    std::int32_t reference;
    StartupWideText text;
    std::uint32_t color;
};
using DisposeStartupV8Global = void (*)(void**);

#if UINTPTR_MAX == UINT32_MAX
static_assert(sizeof(StartupCallbackNode) == 12 && offsetof(StartupCallbackNode, next) == 8);
static_assert(sizeof(StartupCallbackMap) == 16 && offsetof(StartupCallbackMap, buckets) == 4);
static_assert(sizeof(StartupV8Slots) == 12 && sizeof(StartupV8Handle) == 8);
static_assert(sizeof(StartupUiBinding) == 28 && offsetof(StartupUiBinding, references) == 8);
static_assert(sizeof(StartupWideText) == 16 && sizeof(StartupReferenceString) == 24);
static_assert(offsetof(StartupReferenceString, text) == 4);
static_assert(sizeof(StartupInlineVector<std::uint32_t>) == 20);
#endif

// Complete ordinary helpers, including their deliberate dangling header
// fields. These are terminal operations and are not idempotent resets.
void cleanup_startup_callback_map(StartupCallbackMap&, JobMemory); //433CF5BD + bucket release
void cleanup_startup_v8_slots(StartupV8Slots&, DisposeStartupV8Global,
                              JobMemory); //433E223E + vector release
void cleanup_startup_v8_handle(StartupV8Handle&, DisposeStartupV8Global); //433F4D88
void cleanup_startup_ui_bindings(StartupVector<StartupUiBinding>&, JobMemory); //4337EA0A
void cleanup_startup_history(lagcomp::LagRecord*&, std::span<client::NativePlayerStateLayout*, 64>,
                             JobMemory); //4335BB22
void cleanup_startup_reference_strings(StartupInlineVector<StartupReferenceString>&,
                                       JobMemory); //43400D97
void cleanup_startup_inline_callbacks(StartupInlineVector<std::uint32_t>&, JobMemory); //433FBE15

class StartupVectorCleanupBindings {
public:
    StartupVectorCleanupBindings(PreservedInitialization&, client::ExitCallbacks&, JobMemory,
                                 DisposeStartupV8Global);
    StartupVectorCleanupBindings(std::shared_ptr<NativeDataImage>, client::ExitCallbacks&,
                                 JobMemory, DisposeStartupV8Global);
    StartupVectorCleanupBindings(const StartupVectorCleanupBindings&) = delete;
    StartupVectorCleanupBindings& operator=(const StartupVectorCleanupBindings&) = delete;
    StartupVectorCleanupBindings(StartupVectorCleanupBindings&&) = delete;
    StartupVectorCleanupBindings& operator=(StartupVectorCleanupBindings&&) = delete;

    std::shared_ptr<NativeDataImage> data;
    client::ExitCallbacks& callbacks;
    JobMemory memory;
    DisposeStartupV8Global dispose_global;
    void finalize(std::uint32_t native_address);
    void (*source_finalizer(std::uint32_t native_address))();
};

// The seven wrappers only append a finalizer through the existing exit
// registry. The two inline-vector constructors have separate field stores
// and remain the responsibility of the CRT initializer bindings.
void bind_startup_vector_cleanup(client::NativeStartupSources&, StartupVectorCleanupBindings&);
} // namespace skeet::engine

// Declarations from tier0_startup_exports.hpp

namespace skeet::engine {
#if defined(_M_IX86) || defined(__i386__)
using Tier0FloatTime = double(__cdecl*)(); //double in ST0
using Tier0MainThreadPredicate = bool(__cdecl*)(); //boolean in AL
#else
using Tier0FloatTime = double (*)();
using Tier0MainThreadPredicate = bool (*)();
#endif
struct Tier0StartupExports {
    Tier0FloatTime clock;
    void* allocator;
    Tier0MainThreadPredicate on_main_thread;
};

// Recovered three-row last-startup loop after tier0 lookup BBFFA7E0. Each real
// export lookup is followed immediately by its canonical store, in this order:
// Plat_FloatTime ->43468C84, g_pMemAlloc ->43467204, ThreadInMainThread ->43476920.
// After all three stores, dereference only the g_pMemAlloc export variable.
// The returned typed views contain those same final canonical values.
//
// Uses the original non-forwarding PE resolver4334AC0B, not an injected resolver
// callback. NativeImageEnvironment supplies its genuine RtlImageNtHeader import.
// An absent allocator export is an invalid native input (unguarded dereference);
// source reports that failure after the same preceding stores, instead of
// silently installing an empty allocator. No clock/allocator method is called.
Tier0StartupExports initialize_tier0_startup_exports(NativeDataImage&,
                                                     const LoadedModuleEnvironment&,
                                                     NativeImageEnvironment);
} // namespace skeet::engine

// Declarations from trace.hpp

namespace skeet::engine {

// Engine Ray_t passed to TraceRay slot5. Each VectorAligned occupies16 bytes;
// the transform and flags follow at40/44 in the native x86 interface.
struct alignas(16) TraceRay {
    std::array<float, 4> start{}, delta{}, start_offset{}, extents{};
    const void* world_axis_transform = nullptr;
    std::uint8_t is_ray = 0, swept = 0;
};

// Common4346EDC0 engine-filter prefix. Its four client entries only jump into
// the current genuine table at434758B4. Source adapters keep that table live.
struct TraceSkipFilterPrefix {
    void* const* vtable;
    void* skip;
    std::int32_t collision_group;
    std::uint32_t extra;
};

inline constexpr std::size_t trace_result_size = 0x54;
inline constexpr std::size_t trace_fraction_offset = 0x2c;
inline constexpr std::size_t trace_entity_offset = 0x4c;
static_assert(offsetof(TraceRay, delta) == 0x10);
static_assert(offsetof(TraceRay, start_offset) == 0x20);
static_assert(offsetof(TraceRay, extents) == 0x30);
static_assert(offsetof(TraceRay, world_axis_transform) == 0x40);
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(TraceRay, is_ray) == 0x44);
static_assert(offsetof(TraceRay, swept) == 0x45);
static_assert(sizeof(TraceRay) == 0x50);
static_assert(offsetof(TraceSkipFilterPrefix, skip) == 4);
static_assert(offsetof(TraceSkipFilterPrefix, collision_group) == 8);
static_assert(offsetof(TraceSkipFilterPrefix, extra) == 0xc);
static_assert(sizeof(TraceSkipFilterPrefix) == 0x10);
#endif

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_TRACE_MEMBER __thiscall
#else
#define SKEET_TRACE_MEMBER
#endif
namespace trace_detail {
inline bool SKEET_TRACE_MEMBER reject_entity(void*, void*, std::uint32_t) noexcept {
    return false; //43382057, RET8
}
inline std::int32_t SKEET_TRACE_MEMBER everything(void*) noexcept {
    return 0; //43336942
}
inline std::int32_t SKEET_TRACE_MEMBER world_only(void*) noexcept {
    return 1; //433686FC
}
inline void* const* reject_table() noexcept {
    static void* const table[]{reinterpret_cast<void*>(reject_entity),
                               reinterpret_cast<void*>(everything)};
    return table;
}
inline void* const* world_table() noexcept {
    static void* const table[]{reinterpret_cast<void*>(reject_entity),
                               reinterpret_cast<void*>(world_only)};
    return table;
}
} //namespace trace_detail
#undef SKEET_TRACE_MEMBER

// These native filters share ShouldHitEntity=false but have different trace
// types. They remain separate because GetTraceType changes engine traversal.
struct TraceRejectEntitiesFilter {
    void* const* vtable = trace_detail::reject_table(); //434727C0
};
struct TraceWorldOnlyFilter {
    void* const* vtable = trace_detail::world_table(); //434701D0
};

} //namespace skeet::engine

// Declarations from unload.hpp

namespace skeet::engine {
struct CommonControlsExports;

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_UNLOAD_STDCALL __stdcall
#else
#define SKEET_UNLOAD_STDCALL
#endif

struct WindowHookState {
    std::uintptr_t window = 0; // 0x434797B4
    std::uintptr_t subclass_id = 0; // 0x434798D4
    void* subclass_procedure = nullptr; // original 0x4341F1DF
    std::int32_t expected_ansi_procedure = 0; // 0x43475780
    std::int32_t restored_ansi_procedure = 0; // 0x43475848
    std::int32_t expected_wide_procedure = 0; // 0x43475B5C; also written to -21
    std::int32_t restored_wide_procedure = 0; // 0x4347580C
};

struct UnloadSystemFunctions {
    using SetLongFunction = std::int32_t(SKEET_UNLOAD_STDCALL*)(std::uintptr_t, int, std::int32_t);
    using GetLongFunction = std::int32_t(SKEET_UNLOAD_STDCALL*)(std::uintptr_t, int);
    using RemoveWindowSubclass = int(SKEET_UNLOAD_STDCALL*)(std::uintptr_t, void*, std::uintptr_t);
    using FreeHeap = unsigned char(SKEET_UNLOAD_STDCALL*)(void*, std::uint32_t, void*);
    SetLongFunction set_window_long_w = nullptr;
    SetLongFunction set_window_long_a = nullptr;
    GetLongFunction get_window_long_w = nullptr;
    GetLongFunction get_window_long_a = nullptr;
    RemoveWindowSubclass remove_window_subclass = nullptr;
    FreeHeap free_heap = nullptr;
    void* process_heap = nullptr;
};

// Binds the actual Windows APIs; a missing required loaded export is an error.
UnloadSystemFunctions native_unload_system_functions(const CommonControlsExports&);

struct VtableHookNode {
    VtableHookNode* previous;
    VtableHookNode* next;
    void* object;
};

struct VtableHookMetadata {
    std::uintptr_t unknown_0;
    void* target; // native+4, captured receiver used by manual patch owners
    void** original_table; // native metadata +8, installed vtable[-2]
    void* owner; // native+C, captured callback owner (AutoAccept/ConsoleInput)
    void** replacement_table; // native metadata +0x10, used by4331E834
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(VtableHookMetadata, target) == 4);
static_assert(offsetof(VtableHookMetadata, original_table) == 8);
static_assert(offsetof(VtableHookMetadata, owner) == 0xc);
static_assert(offsetof(VtableHookMetadata, replacement_table) == 0x10);
static_assert(sizeof(VtableHookMetadata) == 0x14);
#endif

struct SavedPointerPatch {
    void** target = nullptr;
    void* saved = nullptr;
    std::uintptr_t enabled = 0;
};

struct UnloadState {
private:
    struct StandaloneGlobals {
        bool unloading = false;
        std::uint32_t clear_43476908 = 0, clear_4347644c = 0;
        void* engine = nullptr;
        void (*in_game_callback)() = nullptr;
        bool listener_registered = false;
        void* event_manager = nullptr;
    } standalone_;

public:
    explicit UnloadState(OwnerRegistry& registry)
        : owners(registry), unloading(standalone_.unloading),
          clear_43476908(standalone_.clear_43476908), clear_4347644c(standalone_.clear_4347644c),
          engine(standalone_.engine), in_game_callback(standalone_.in_game_callback),
          listener_registered(standalone_.listener_registered),
          event_manager(standalone_.event_manager) {}
    UnloadState(OwnerRegistry& registry, bool& actual_unloading,
                std::uint32_t& actual_first_patches, std::uint32_t& actual_second_patches,
                void*& actual_engine, void (*&actual_in_game_callback)(),
                bool& actual_listener_registered, void*& actual_event_manager)
        : owners(registry), unloading(actual_unloading), clear_43476908(actual_first_patches),
          clear_4347644c(actual_second_patches), engine(actual_engine),
          in_game_callback(actual_in_game_callback),
          listener_registered(actual_listener_registered), event_manager(actual_event_manager) {}
    UnloadState(const UnloadState&) = delete;
    UnloadState& operator=(const UnloadState&) = delete;
    UnloadState(UnloadState&&) = delete;
    UnloadState& operator=(UnloadState&&) = delete;
    OwnerRegistry& owners; // canonical43475B78/7C/80 storage
    // Native BSS byte43475895. Only hard detach433EF91A sets it; the normal
    // menu request/timer finalization leaves browser disposal enabled.
    bool& unloading;
    WindowHookState window;
    std::uint32_t& clear_43476908; //native DWORD representation of the entity patch-list head
    std::uint32_t& clear_4347644c; //native DWORD representation of the incoming patch-list head
    VtableHookNode hooks{&hooks, &hooks, nullptr}; // native empty sentinel 0x43476B2C
    void*& engine; // live0x4346A514; virtual entry27
    void (*&in_game_callback)(); //live0x43468BF0
    SavedPointerPatch first_patch; // target 0x434757CC, saved +4, enabled +12
    SavedPointerPatch second_patch; // target 0x43475A40, saved +4, enabled +12
    SavedPointerPatch cleared_patch; // 0x43476A40/+4/+8, all cleared unconditionally
    SavedPointerPatch final_patch; // target 0x43475920, saved +4, enabled +12
    bool& listener_registered; // live0x43468B80
    // These legacy field names identify renderer-device notifications, not the
    // game's event registry. Startup registers the same listener at slot20.
    void*& event_manager; // live0x4346A470; virtual entry21
    void* event_listener = nullptr; // original address 0x43468B7C
};

// Restored native hook lifecycle. Engine virtual calls perform their actual
// operations; only OS-export dependencies are explicit for offline verification.
class UnloadRuntime {
public:
    UnloadRuntime(UnloadState& state, UnloadSystemFunctions system)
        : state(state), system(system) {}
    UnloadState& state;
    UnloadSystemFunctions system;
    void restore_window(); // complete 0x4335B9C9
    void cleanup(); // complete 0x433E445B, includes restore_window
};

// Complete 0x433837D0. Allocations have the native alignment displacement dword
// at object-8 and are freed from object-16-displacement on the process heap.
void free_native_allocation(void* object, const UnloadSystemFunctions& system);

#undef SKEET_UNLOAD_STDCALL
} // namespace skeet::engine
