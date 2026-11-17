#pragma once

// Chams & Visual Effects Subsystem Declarations
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string>
#include <vector>
#include "skeet/effects/blur.hpp"
#include "skeet/engine/owner_registry.hpp"
#include "skeet/client/command_empty_tick.hpp"
#include "skeet/client/command_phase_registry.hpp"
#include "skeet/client/command_frame.hpp"
#include "skeet/rage/spread.hpp"
#include "skeet/rage/targeting_commit.hpp"
#include "skeet/config.hpp"
#include "skeet/effects/camera.hpp"
#include "skeet/esp/effects_config.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/lagcomp/entity_state.hpp"
#include "skeet/engine/startup_convars.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/engine/hooks.hpp"
#include "skeet/effects/entity_render_state.hpp"
#include "skeet/esp/model_color_config.hpp"
#include "skeet/client/commands.hpp"
#include "skeet/network/update_observers.hpp"
#include "skeet/rage_hitboxes.hpp"
#include "skeet/esp/config.hpp"
#include "skeet/esp/other_config.hpp"
#include "skeet/players/state.hpp"
#include "skeet/engine/offset_storage.hpp"
#include "skeet/rage/target_relationships.hpp"
#include "skeet/animation/bones.hpp"
#include "skeet/animation/bone_runtime.hpp"
#include "skeet/lagcomp/interpolation.hpp"
#include "skeet/events/server_impacts.hpp"
#include "skeet/events/registry.hpp"
#include "skeet/engine/frame_update.hpp"
#include "skeet/hud/composite_runtime.hpp"
#include "skeet/client/entity_observer_registry.hpp"
#include <memory>
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/esp/grenade_prediction.hpp"
#include "skeet/engine/native_data_image.hpp"
#include "skeet/engine/netvars.hpp"
#include <stdexcept>
#include "skeet/render_queue.hpp"
#include "skeet/misc/fov.hpp"
#include "skeet/misc/config.hpp"


// Declarations from blur.hpp

namespace skeet::effects {
struct BlurRectangle {
    std::int16_t x, y, width, height;
};
static_assert(sizeof(BlurRectangle) == 8);
struct BlurBindings {
    void* engine = nullptr; // 4346A514
    void* material_system = nullptr; // 4346A4E0
};
// IMesh::LockMesh output. Layout/data entries 0,3,6,14,15 are position,
// normal, first UV channel, tangent S and tangent T. The other engine fields
// pass back unchanged to UnlockMesh. Native x86 MeshDesc_t is 0xB4 bytes.
struct BlurMeshDescriptor {
    std::array<std::int32_t, 21> vertex_layout{};
    std::array<void*, 18> vertex_data{};
    std::int32_t first_vertex = 0;
    std::uint32_t vertex_offset = 0;
    std::uint16_t* indices = nullptr;
    std::uint32_t index_offset = 0, first_index = 0;
    std::int32_t index_size = 0;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(BlurMeshDescriptor) == 0xB4);
static_assert(offsetof(BlurMeshDescriptor, vertex_data) == 0x54);
static_assert(offsetof(BlurMeshDescriptor, first_vertex) == 0x9C);
static_assert(offsetof(BlurMeshDescriptor, indices) == 0xA4);
#endif

class BlurEffect {
public:
    explicit BlurEffect(BlurBindings bindings = {}) : bindings(bindings) {}
    BlurBindings bindings;
    // Material fields +0C,+14,+18,+10, then texture fields +20,+24,+1C.
    std::array<void*, 4> materials{};
    std::array<void*, 3> textures{};
    std::array<std::uint32_t, 3> material_hashes{0x2EA680DFu, 0x3141F4D7u, 0x3641F748u};
    std::string texture_group = "RenderTargets";
    std::array<std::string, 3> texture_names{"_rt_SmallFB0", "_rt_SmallFB1", "_rt_FullFrameFB"};

    // Each input keeps its low 16 bits. Full batches ignore further entries.
    void draw(std::int32_t x, std::int32_t y, std::int32_t width, std::int32_t height); // 433F2AE2
    std::span<const BlurRectangle> pending() const noexcept {
        return rectangles_;
    }
    bool initialize_resources(); // 433C0291
    void release_resources(); // 433E2BC0
    void render(); // 433C8896, including clearing failed batches
private:
    void apply(); // 433B1183
    std::vector<BlurRectangle> rectangles_;
};
} // namespace skeet::effects

// Declarations from blur_runtime.hpp

namespace skeet::effects {
#if defined(_M_IX86) || defined(__i386__)
using EffectFrameEntry = void(__fastcall*)(void*, std::uintptr_t, std::uintptr_t, std::uintptr_t,
                                           std::uintptr_t);
#else
using EffectFrameEntry = void (*)(void*, std::uintptr_t, std::uintptr_t, std::uintptr_t,
                                  std::uintptr_t);
#endif
// Native registered callback objects have one frame entry. ECX receives the
// object, EDX receives the first forwarded word, and three words stay on stack.
struct EffectFrameCallback {
    const EffectFrameEntry* vtable;
};

// Complete433B8C46. The registry is43475A84/88/8C, using the same pointer-vector
// implementation as the canonical owner registry. Its begin/end are captured
// once; callback registration during dispatch does not extend that pass.
void dispatch_effect_frame(const engine::OwnerRegistry& callbacks, std::uintptr_t first,
                           std::uintptr_t second, std::uintptr_t third, std::uintptr_t fourth);

// Complete4347F6DB/4347E37A composition. active is the canonical43475A20 slot.
// Registration runs in native constructor order: lifecycle owner first, frame
// endpoint second, then vtables and effect state. The owner registry's slot0
// destroys the instance and clears active; slot4 releases render resources.
// Other lifecycle entries are the actual native433CCE8E return-only body.
void initialize_blur(engine::OwnerRegistryBinding owners, engine::OwnerRegistryBinding callbacks,
                     BlurEffect*& active, BlurBindings bindings);
} // namespace skeet::effects

// Declarations from bullet_impact_command_owner.hpp

namespace skeet::effects {
struct BulletImpactCommandBindings {
    void** const& game_rules; //4346A3A4, then the actual rules object
    std::ptrdiff_t game_rule_override; //D720AEA6, byte; complete433DF22C gate
    std::ptrdiff_t weapon_definition; //5C9F0FBB, read at both native consumers
    void* const& debug_overlay; //43468260, actual virtual1 AddBoxOverlay
    engine::NativeConVarReference shotgun_patterns; //live43468520 XOR394BFF56, read during spread
    BallisticEngine& ballistics;
    void* const* trace_filter_vtable; //genuine external43475B90, slots0..4
    const rage::SpreadTables& spread_tables;
    const rage::SpreadRandom& random;
    const rage::ShotgunPattern& shotgun_pattern;
    const RageTrigonometry& trigonometry;
};
// One native effects/model owner4348E6E2 supplies normal434700E4,
// HUD434704B8 and empty-tick4347270C. These are references to its existing
// canonical state; this adapter does not construct another camera/world owner.
struct EffectsCommandOwner {
    CameraRuntime& camera;
    WorldRuntime& world;
    ModelRenderRuntime& models;
    client::PlayerStateCache& players;
    const EntityRenderBindings& entities;
    const BulletImpactCommandBindings& bindings;
    client::NativeCommandEmptyTickReceiver empty_tick_receiver{};
};
// Complete normal phase6 body4339EC1B. Uses live weapon-definition getters,
// command angles/seed, frame recoil/spread/origin, concrete direction ballistics
// and an engine debug box for every recorded penetration/impact point.
void process_bullet_impacts(EffectsCommandOwner&, const client::CommandFrame&, void* player,
                            const client::Command&);
// Whole4346FAFC slot1, consuming the actual incoming packet only after the
// camera/ragdoll gates in the existing43326129 implementation have run.
void update_effects_entities(EffectsCommandOwner&, void* player, std::uintptr_t incoming);
} // namespace skeet::effects

// Declarations from camera.hpp

namespace skeet::client {
class PlayerStateCache;
}
namespace skeet::effects {

struct CameraNetvars {
    std::uint32_t life_state;
    std::uint32_t observer_mode;
    std::uint32_t observer_target;
    std::uint32_t fov;
    // The second cleared dword has no independently recovered field name.
    // Native resolver: *0x43468D10 ^ *0x43468DCC ^ 0x3F51AA0A.
    std::uint32_t reset_field_3f51aa0a;
};

struct SavedCameraState {
    std::int32_t fov = 0; // native player record +0xFC
    std::int32_t observer_mode = 0; // native player record +0x164
};
struct SavedCameraStateView {
    std::int32_t& fov;
    std::int32_t& observer_mode;
    SavedCameraStateView& operator=(SavedCameraState value) noexcept {
        fov = value.fov;
        observer_mode = value.observer_mode;
        return *this;
    }
};
// The original record lookup masks the requested index with 63 and initializes
// an absent record to zero. The owning player-state capture supplies these two
// fields before observer overrides; this camera code does not invent a capture.
struct CameraHistory {
private:
    client::PlayerStateCache* source_ = nullptr;

public:
    CameraHistory() = default;
    explicit CameraHistory(client::PlayerStateCache& source) noexcept : source_(&source) {}
    std::array<SavedCameraState, 64> players{};
    SavedCameraStateView at(std::uint32_t index);
};

// Native observer-property proxy argument layout. At 0x43394077..0x433940A2
// and 0x43394228..0x43394269 the integer is at +8 and tag 1 is at +0x14.
// The two unused dwords at +0xC/+0x10 are initialized here; native leaves them
// unspecified. The remaining eight dwords reproduce its explicit writes.
struct ObserverProxyData {
    std::array<std::uint32_t, 10> words{};
};
using ObserverProxy = void (*)(const ObserverProxyData*, void* entity, void* destination);

struct CameraInterfaces {
private:
    void* standalone_engine_ = nullptr;
    void* standalone_entities_ = nullptr;
    void* standalone_input_ = nullptr;
    void* standalone_local_ = nullptr;
    ObserverProxy standalone_proxy_ = nullptr;

public:
    CameraInterfaces() noexcept
        : engine(standalone_engine_), entity_list(standalone_entities_), input(standalone_input_),
          local_player(standalone_local_), observer_proxy(standalone_proxy_) {}
    explicit CameraInterfaces(void* engine_object) noexcept : CameraInterfaces() {
        engine = engine_object;
    }
    CameraInterfaces(void*& engine_cell, void*& entities_cell, void*& input_cell, void*& local_cell,
                     ObserverProxy& proxy_cell) noexcept
        : engine(engine_cell), entity_list(entities_cell), input(input_cell),
          local_player(local_cell), observer_proxy(proxy_cell) {}
    CameraInterfaces(const CameraInterfaces&) = delete;
    CameraInterfaces& operator=(const CameraInterfaces&) = delete;
    CameraInterfaces(CameraInterfaces&&) = delete;
    CameraInterfaces& operator=(CameraInterfaces&&) = delete;
    void*& engine; // native global 0x4346A514
    void*& entity_list; // 0x4346A890
    void*& input; // 0x4346A894
    void*& local_player; // 0x43475808
    ObserverProxy& observer_proxy; // 0x4347BDE4, optional
};

// Uses actual engine vtable entries, not caller-supplied substitute actions:
// engine[26], entity_list[4], player[158]/[312], networkable(+8)[10], and
// input[32]/[36]. Their byte offsets are the recovered x86 offsets / 4.
class CameraRuntime {
    bool standalone_forced_observer_ = false;

public:
    CameraRuntime(CameraInterfaces& interfaces, CameraNetvars offsets, CameraHistory& history,
                  const std::uint8_t& force_dead, const RegisteredHotkey& alive_hotkey)
        : interfaces(interfaces), offsets(offsets), history(history), force_dead(force_dead),
          alive_hotkey(alive_hotkey), forced_observer(standalone_forced_observer_) {}
    CameraRuntime(CameraInterfaces& interfaces, CameraNetvars offsets, CameraHistory& history,
                  const std::uint8_t& force_dead, const RegisteredHotkey& alive_hotkey,
                  bool& forced_observer)
        : interfaces(interfaces), offsets(offsets), history(history), force_dead(force_dead),
          alive_hotkey(alive_hotkey), forced_observer(forced_observer) {}
    CameraRuntime(const CameraRuntime&) = delete;
    CameraRuntime& operator=(const CameraRuntime&) = delete;
    CameraInterfaces& interfaces;
    CameraNetvars offsets;
    CameraHistory& history;
    const std::uint8_t& force_dead;
    const RegisteredHotkey& alive_hotkey;
    InputBitmap keys{};
    bool& forced_observer; // native global 0x4347CA59

    void alive_changed(bool enabled); // complete 0x43322683
    void dead_changed(); // complete 0x433662E6
    void update_observer(void* player); // complete 0x43393F51, valid entity-index contract
private:
    bool alive_or_death_camera(void* player) const; // 0x43386EB3
    void apply_observer_mode(void* player, std::int32_t mode);
    void reset_camera(void* player);
};

} // namespace skeet::effects

// Declarations from camera_runtime_owner.hpp

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::effects {
// Camera callbacks and receive proxies use the same player cache and live
// source globals. Construction neither reads a player nor changes the camera.
class CameraRuntimeOwner {
public:
    CameraRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                       client::ConfigurationRuntimeOwner&, client::PlayerRuntimeOwner&);
    CameraRuntimeOwner(const CameraRuntimeOwner&) = delete;
    CameraRuntimeOwner& operator=(const CameraRuntimeOwner&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    CameraRuntime& runtime() noexcept {
        return runtime_;
    }
    CameraInterfaces& interfaces() noexcept {
        return interfaces_;
    }
    CameraHistory& history() noexcept {
        return history_;
    }

private:
    client::ClientRuntimeStorage& storage_;
    CameraInterfaces interfaces_;
    CameraHistory history_;
    CameraRuntime runtime_;
};
}

// Declarations from entity_render_state.hpp

namespace skeet::effects {

// Native list entries occupy eight bytes at argument+0x44. Only the first
// dword is consumed; the signed count is stored at argument+0x4044.
struct EntityRenderListEntry {
    std::uint32_t index;
    std::uint32_t untouched;
};
static_assert(sizeof(EntityRenderListEntry) == 8);

struct EntityRenderOffsets {
    std::uint32_t ragdoll_yaw; // 73DBBB49
    std::uint32_t render_flags; // 38CA5228
    std::uint32_t ragdoll_owner; // 9C14F5A7
    std::uint32_t hidden_byte_1b5f47d5;
    std::uint32_t render_mode; // FC2723F7; hidden mode is 10
    std::uint32_t reset_byte_8481ea04;
    std::uint32_t origin_source; // 258BF82B
    std::uint32_t origin_copy; // A9EE6062
    std::uint32_t origin_invalidated; // 2A671B42
    std::uint32_t angles_invalidated; // 3B1F6ECC
    std::uint32_t angles_copy; // C865F10E
};

using InvalidateEntityPhysics = engine::MemberFunction<void, std::int32_t>;
struct EntityRenderBindings {
    void* const& entity_list; //live4346A890
    void* const& ragdoll_vtable; //live43467DC0, entity's primary vtable
    engine::NativeConVarReference ragdoll_convar; //live4346A67C XOR3155EA46
    InvalidateEntityPhysics const& invalidate_physics; //live optional43468C10
    EntityRenderOffsets offsets;
    const lagcomp::EntityNetvars& animation_offsets;
};

// Complete 43326129. The original observer update is performed first through
// its restored owner. Ragdoll animation uses the same PlayerStateCache and
// newest LagRecord as capture/resolver; no second history or entity model is
// created. weapon_models_enabled references ColoredModelsSettings::weapons
// enabled.value directly: receiver=full owner+0x18, receiver+0x394=full+0x3AC,
// descriptor hash 92EBAD93. It gates the actual entity slot 159 predicate
// followed by slot 238(false).
// Each supplied entry belongs to the original positive-count list; entries
// with index<=64 or any high sixteen bits are skipped exactly as native.
void update_entity_render_state(CameraRuntime&, client::PlayerStateCache&,
                                const EntityRenderBindings&, void* observer_player,
                                std::span<const EntityRenderListEntry>, const esp::EffectsSettings&,
                                const std::uint8_t& weapon_models_enabled);

// Live native receiver form. Packet count and entry storage are inspected
// after the observer update and the ConVar/configuration gates, at 43326184.
// A nonpositive signed count reads no entries.
void update_entity_render_state_packet(CameraRuntime&, client::PlayerStateCache&,
                                       const EntityRenderBindings&, void* observer_player,
                                       const void* packet, const esp::EffectsSettings&,
                                       const std::uint8_t& weapon_models_enabled);

} // namespace skeet::effects

// Declarations from entity_visibility.hpp

namespace skeet::effects {
// Relationship inputs carry no selected player. Native callbacks choose their
// actual saved entity/local argument at their own read sites; both ConVar
// pointer cells remain live until that per-call binding is formed.
struct EntityVisibilityRelationships {
    void* const& entity_list;
    engine::NativeConVarReference game_type, teammates_are_enemies;
    const players::State& adjustments;
    const rage::TargetOffsets& offsets;
    rage::TargetBindings for_player(void* actual_saved_player) const noexcept;
};
struct EntityVisibilityBindings {
    void* const& engine; //live4346A514,slots82/93 demo/HLTV bypasses
    void* const* local_player; //43475808,read after both actual engine calls
    const EntityVisibilityRelationships& relationships;
};
// Complete4336EE16. Original player slot194 receives both original stack
// words; only the proven teammate branch returns zero without forwarding.
std::uint32_t filter_teammate_render(const esp::EffectsSettings&, const EntityVisibilityBindings&,
                                     void* entity, std::uintptr_t first, std::uintptr_t second);
struct EntityVisibilityRuntime {
    const esp::EffectsSettings& settings;
    const EntityVisibilityBindings& bindings;
    // Adds only slot194 to the already prepared canonical player table. The
    // existing producer remains responsible for its single installation.
    void bind_player_table(engine::VtableHookMetadata&);
    static void* source_entry() noexcept;
};

struct EntityNetworkableOffsets {
    EntityRenderOffsets render;
    std::uint32_t render_fx; //688E0292
    std::uint32_t local_player_flag; //D070684C, bit0
    std::uint32_t observer_mode; //2F4E97C4
    std::uint32_t observer_target; //3B704B41
};
struct EntityNetworkableBindings {
    const EntityVisibilityBindings& visibility;
    void* const& entity_list; //live4346A890
    const std::uint32_t& encoded_ragdoll_convar; //live4346A67C^3155EA46
    const lagcomp::EntityNetvars& animation_offsets;
    EntityNetworkableOffsets offsets;
    const InvalidateEntityPhysics& invalidate_physics; //optional live43468C10
    engine::MemberFunction<void> const& invalidate_render_flags; //optional live43468320
    void* const& globals; //live43467B04; time+10 read after child mode dispatch
    const std::uint32_t& encoded_shadow_history_convar; //live43467A78^3AE06513
    engine::VtableHookRegistry& hook_registry; //canonical43476B2C, outlives owner
};
// Complete4340C3A8, paired slot1 of descriptor43475AEC. The restored slot1
// is entered only after the actual per-object registry node is removed.
void release_hooked_networkable(engine::VtableHookRegistry&, void* receiver);
// Complete433E622F. FLT_MAX means sample the live clock; an already-set
// effect15 with that sentinel returns before any engine interface call.
void set_entity_fade(void*, float start, float duration, const EntityNetworkableBindings&);
class EntityNetworkableRuntime {
public:
    const esp::EffectsSettings& settings;
    const esp::ColoredModelsSettings& models;
    client::PlayerStateCache& players;
    const EntityNetworkableBindings& bindings;
    // Complete433990E9. receiver is the actual entity+8 networkable, stage is
    // forwarded unchanged to its saved original slot5 between both mutations.
    void post_data_update(void* receiver, std::int32_t stage);
    void bind_networkable_table(engine::VtableHookMetadata&);
    static void bind_static_networkable_table(engine::VtableHookMetadata&);

private:
    bool visible_ragdoll_pass(void*);
};
} // namespace skeet::effects

// Declarations from free_look_capture.hpp

namespace skeet::effects {
class FreeLookOwner;

struct FreeLookCaptureBindings {
    void* const& input; //4346A894; actual slots32/34, camera distance+B4
    void* const& engine_trace; //43468304; actual TraceRay slot5
    void* const& engine_info; //43467014; actual slot3 trace-entity predicate
    void* const& entity_list; //4346A890; actual handle lookup slot4
    void* const* const& trace_filter_vtable; //genuine external table434758B4
    std::uint32_t owner_handle; //decoded4E36DFD7
    FreeLookOwner* const& free_look; //actual43475A64 owner, required at capture time
    std::int32_t* const& frame; //434678F8, pointed engine frame
    RageTrigonometry trigonometry; //actual imported SSE sine/cosine
};

// Complete4337EE59, including4333D32B. receiver is the native-compatible
// filter prefix passed to the genuine engine filter's first virtual method.
// Resolved entities and every owner are rejected by actual slots162/158.
bool free_look_capture_accepts(void* receiver, void* candidate, std::uint32_t mask,
                               const FreeLookCaptureBindings&);

// Complete433CCEA1..433CD183. The saved original player vtable slot337
// (byte offset0x544) runs first. The input camera, the special owner-chain trace
// filter, distance adjustment and unconditional origin/frame capture stay here.
// setup is the engine view setup with origin at+B8 and height component+C0.
void capture_free_look_view_hook(void* player, void** original_table, void* setup,
                                 const FreeLookCaptureBindings&);

} //namespace skeet::effects

// Declarations from free_look_owner.hpp

namespace skeet::client {
class NativeStartupSources;
}
namespace skeet::effects {

class FreeLookOwner;
struct FreeLookOwnerBindings {
    FreeLookOwnerState& state; //one canonical command/view-capture state
    FreeLookViewBindings& view;
    FreeLookOwner*& active; //actual43475A64 owner slot, shared by all consumers
};

inline constexpr std::size_t native_free_look_owner_allocation_size = 0x38;
std::size_t free_look_owner_size() noexcept;

// Entire ordinary inline constructor43484A3A..43484A65. The factory
// prefix43484A2B requests0x38 from43387448; caller supplies its aligned storage
// with at least free_look_owner_size() bytes. Lifecycle registration precedes
// update registration; both precede their concrete table assignments. Only the
// three delta words are initialized. active/base/capture and the primary's
// second word retain their incoming values. Configuration has a separate CRT.
// Publication belongs to the enclosing factory after this constructor.
FreeLookOwner& construct_free_look_owner(void* storage, engine::OwnerRegistryBinding lifecycle,
                                         engine::OwnerRegistryBinding updates,
                                         FreeLookOwnerBindings);

// Whole43484A2B. The x86 allocator requests0x38 with HEAP_ZERO_MEMORY; the
// borrowed canonical state receives that zero-initialization only on success.
// mem_int3.h:516 records43484A69 MOV[43475A64],EBX: publish the new source
// owner after construction, or null from the ordinary43484A67 failure branch.
FreeLookOwner* create_free_look_owner(engine::OwnerRegistryBinding lifecycle,
                                      engine::OwnerRegistryBinding updates, FreeLookOwnerBindings);

// Whole43406BFF ->433837D0, including clearing43475A64 for a null object.
// The memory boundary is the genuine process-heap/free binding; the storage
// carries43387448's alignment displacement at object-8.
void destroy_free_look_owner(FreeLookOwner*, FreeLookOwner*& active, engine::JobMemory);

FreeLookOwnerState& free_look_owner_state(FreeLookOwner&) noexcept;
FreeLookViewRuntime& free_look_view_runtime(FreeLookOwner&) noexcept;
network::UpdateObserver& free_look_view_endpoint(FreeLookOwner&) noexcept;
void* free_look_lifecycle_object(FreeLookOwner&) noexcept;

struct FreeLookInitialization {
    engine::OwnerRegistryBinding lifecycle;
    engine::OwnerRegistryBinding updates;
    FreeLookOwnerBindings bindings;
};
// The original factory is a parameterless initializer, not an owner-returning
// engine method. This source adapter discards the convenience helper result;
// the existing startup graph supplies the invocation order.
void bind_free_look_initializer(client::NativeStartupSources&, FreeLookInitialization&);

} //namespace skeet::effects

// Declarations from free_look_runtime_owner.hpp

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class CrtRuntimeOwner;
struct EngineRuntimeBindings;
}
namespace skeet::effects {
// Shared command/view state before43484A2B allocates its physical receivers.
class FreeLookRuntimeOwner final {
public:
    FreeLookRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                         client::ConfigurationRuntimeOwner&, client::CrtRuntimeOwner&);
    FreeLookRuntimeOwner(const FreeLookRuntimeOwner&) = delete;
    FreeLookRuntimeOwner& operator=(const FreeLookRuntimeOwner&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    FreeLookInitialization& initializers() noexcept {
        return initialization_;
    }
    void bind_initializers(client::NativeStartupSources&);

private:
    client::ClientRuntimeStorage& storage_;
    FreeLookOwnerState state_{};
    FreeLookViewBindings view_;
    FreeLookInitialization initialization_;
};
}

// Declarations from free_look_view.hpp

namespace skeet::effects {

// The view half follows the existing command state in the same native owner.
// 433CD16F captures the origin; 433CD17C stamps the current engine frame.
struct FreeLookViewCapture {
    std::array<float, 3> origin;
    std::int32_t frame;
};
struct FreeLookOwnerState {
    client::FreeLookState command;
    FreeLookViewCapture view;
};
static_assert(offsetof(FreeLookOwnerState, view) == 0x28);
static_assert(offsetof(FreeLookOwnerState, view) + offsetof(FreeLookViewCapture, frame) == 0x34);
static_assert(sizeof(FreeLookOwnerState) == 0x38);

struct FreeLookViewBindings {
    void* const& input; //4346A894; slots32/34
    void* const& prediction; //43469264; temporary byte+8
    void* const& local_player; //43475808; captured once after the key gate
    std::uint32_t life_state; //decoded9D920E35
    const std::int32_t& frame; //*434678F8
    engine::NativeConVarReference camera_distance; //43467AA0 ^8985EFCE, live encoded cell
    engine::NativeConVarReference camera_mode; //4346A7DC ^B2005B9C, live encoded cell
    void* const& engine_trace; //43468304; actual TraceRay slot5
    void* const* trace_filter_vtable; //genuine external434758B4 table
    const HotkeyBinding& free_look_key; //43468274/43468288
    const std::uint8_t& third_person; //43468A10
    const HotkeyBinding& third_person_key; //43468448/4346845C
    InputBitmap keys;
    RageTrigonometry trigonometry; //actual imported SSE sine/cosine
};

class FreeLookViewRuntime {
public:
    client::FreeLookState& state;
    FreeLookViewCapture& capture;
    FreeLookViewBindings& bindings;

    FreeLookViewRuntime(client::FreeLookState& state, FreeLookViewCapture& capture,
                        FreeLookViewBindings& bindings)
        : state(state), capture(capture), bindings(bindings) {}
    void reset() noexcept; //complete4341781B; also the failed43336F85 gate
    bool third_person_active(); //complete433FBBCD, including the second hotkey
    // Complete43336F85. Both camera queries and the repeated third-person test
    // retain their native order; player slots212/278 and TraceRay are real calls.
    void update(float* origin, float* angles);
};

// Actual secondary interface4347269C. Slot6 is the concrete433B1E73 wrapper;
// all remaining entries are its original no-op methods, with their real ABIs.
struct FreeLookViewObserver {
    network::UpdateObserver endpoint;
    FreeLookViewRuntime* runtime;
    explicit FreeLookViewObserver(FreeLookViewRuntime&, bool initialize_table = true);
    void initialize_vtable() noexcept;
};

} //namespace skeet::effects

// Declarations from glow.hpp

namespace skeet::effects {
class GlowEntityEngine;

// Engine CGlowObjectManager layout. Pointer fields follow host width in the
// independent verification build; the x86 runtime has the native 0x38 stride.
struct GlowObject {
    std::int32_t next_free = -2;
    void* entity = nullptr;
    std::array<float, 4> rgba{};
    std::array<std::byte, 16> reserved_18{};
    std::uint8_t render_occluded = 0;
    std::array<std::byte, 15> reserved_29{};
};
struct GlowRegistry {
    GlowObject* objects = nullptr;
    std::int32_t allocation_count = 0, grow_size = 0, count = 0;
    GlowObject* elements = nullptr;
    std::int32_t first_free = -1;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(GlowObject) == 0x38 && offsetof(GlowObject, entity) == 4 &&
              offsetof(GlowObject, rgba) == 8 && offsetof(GlowObject, render_occluded) == 0x28);
static_assert(offsetof(GlowRegistry, count) == 0xC && offsetof(GlowRegistry, first_free) == 0x14);
#endif
struct GlowEngine {
    GlowRegistry* const& registry; // live native 0x43467460 cell
    void* const& registration_function; // live native external engine 0x4346832C cell
    // Non-x86 verification ABI. The runtime's x86 call reproduces the native
    // ECX/EDX, XMM0..3, and five stack arguments inside its explicit adapter.
    using HostRegistration = std::int32_t (*)(GlowRegistry*, void* entity, const float* rgb,
                                              float alpha, bool occluded, bool unoccluded,
                                              std::int32_t split_screen);
};
// Complete 0x43403018 and 0x4340BF38. Native updates temporarily select the
// current slot as the free-list head, then call the engine's actual allocator.
void update_glow(GlowEngine&, void* entity, std::uint32_t index, const std::array<float, 4>& rgba);
void clear_glow(GlowEngine&, void* entity, std::uint32_t index);

class GlowRuntime {
public:
    GlowRuntime(const esp::PlayerEspConfig& players, const esp::WorldEspConfig& world,
                GlowEngine& engine, GlowEntityEngine& entities)
        : players(players), world(world), engine(engine), entities(entities) {}
    const esp::PlayerEspConfig& players;
    const esp::WorldEspConfig& world;
    GlowEngine& engine;
    GlowEntityEngine& entities;
    bool weapons_active = false; // owner +0x6B8 (glow subobject +0x6A0)
    std::int32_t bomb_handle = -1; // owner +0x6F4 (subobject +0x6DC)
    void update(); // complete 0x43385D9A
    void clear_bomb(); // complete 0x433B5B76
};

} // namespace skeet::effects

// Declarations from glow_engine.hpp

namespace skeet::effects {
struct GlowEntityBindings {
    void* const& entity_list; //4346A890
    void* const& local_player; //43475808
    std::int32_t* const& max_clients; //43469300
    void* const& planted_bombs; //434688C0, native vector data+0/count+C
    const std::uint8_t& suspended; //43468E58
    const std::uint8_t& activation; //4346A65C
    engine::NativeConVarReference game_type; //43467A8C^AC9299CC ConVar
    engine::NativeConVarReference teammates_are_enemies; //4346A568^9190E2E6 ConVar
    const players::State& player_adjustments; //4347BE09, native index*20
    const engine::NativeOffsetStorage& offsets;
};

// Raw engine side of43385D9A/433B5B76. Every global cell and encoded offset
// is consumed at its native use; no frame/configuration snapshot is retained.
class GlowEntityEngine {
public:
    explicit GlowEntityEngine(GlowEntityBindings bindings) : bindings_(bindings) {}
    void* local_player() const noexcept {
        return bindings_.local_player;
    }
    bool suspended() const noexcept {
        return bindings_.suspended != 0;
    }
    bool activation() const noexcept {
        return bindings_.activation != 0;
    }
    std::int32_t max_clients() const noexcept {
        return *bindings_.max_clients;
    }
    void* planted_bombs() const noexcept {
        return bindings_.planted_bombs;
    }
    void* by_index(std::int32_t) const;
    void* by_handle(std::int32_t) const;
    std::int32_t entity_limit() const;
    bool life_state(void*) const;
    bool suppressed(void*) const;
    bool eligibility_state(void*) const;
    std::uint32_t player_glow_index(void*) const;
    std::uint32_t weapon_glow_index(void*) const;
    bool bomb_active(void*) const;
    std::uint32_t bomb_glow_offset() const;
    bool enemy(void* local, void* target) const;
    bool disable_visuals(std::uint32_t index) const noexcept;

private:
    std::uint32_t offset(std::uint32_t pointer, std::uint32_t key, std::uint32_t consumer) const;
    GlowEntityBindings bindings_;
};
} //namespace skeet::effects

// Declarations from harmless_molotov.hpp

namespace skeet::effects {
enum class HarmlessMolotovHook {
    creation,
    primary,
    secondary,
    renderable
};
class HarmlessMolotovRuntime;
struct ParticleColorBackup {
    float* address;
    std::array<std::uint32_t, 3> values;
};
struct ParticleAlphaBackup {
    float* address;
    std::uint32_t value;
};
struct HarmlessMolotovState {
    explicit HarmlessMolotovState(bool initialize_storage = true);
    void initialize_native_storage(); //43484B67..43484CDD, after config registration
    void release_native_storage() noexcept; //whole43326010: alpha, color, hashes
    std::array<float, 3> color{}, hsv{};
    float alpha = 1;
    std::vector<std::uint32_t> definition_hashes;
    std::vector<ParticleColorBackup> colors;
    std::vector<ParticleAlphaBackup> alphas;
};
struct HarmlessMolotovBindings {
    void* const& engine; //live4346A514,actual slots82/93
    void* const& entity_list; //live4346A890
    void* const* local_player; //43475808
    std::uint32_t owner_handle; //8D1BD8D0
    const std::uint32_t& encoded_friendly_fire; //live43467C3C XOR211963A6
    const std::uint32_t& encoded_grenade_damage_reduction; //live434685C8 XORD5F0540C
    rage::TargetRelationshipBindings relationships;
    engine::VtableHookRegistry& hook_registry;
    std::array<engine::VtableHookMetadata*, 3> entity_hooks; //43475908/434758F4/434757F4
    HarmlessMolotovRuntime*& active; //one canonical43475828 slot, shared with factory
};
// Whole433C8A72 and433CEAAB. Particle traversal, HSV interpolation, backup
// and restoration execute in source; only real engine virtuals are bound.
class HarmlessMolotovRuntime {
public:
    const esp::EffectsSettings& settings;
    HarmlessMolotovState& state;
    const HarmlessMolotovBindings& bindings;
    void apply_collection(void* collection); //complete433AD7B0, recursive children
    std::uint32_t render(void* receiver, std::uintptr_t first, std::uintptr_t second);
    std::uint32_t created(void* receiver, const char* name, void* created);
    std::uint32_t destroy_primary(void* receiver, std::uintptr_t flags); //433E7C16
    std::uint32_t destroy_secondary(void* receiver, std::uintptr_t flags); //4340B2C6
    // The RTTI producer supplies descriptor43475AC4 (creation110) plus the
    // canonical three entity descriptors above. No second table is allocated.
    void bind_tables(engine::VtableHookMetadata& creation);
    // Independent descriptor preparation must not publish the feature owner.
    void bind_table(HarmlessMolotovHook, engine::VtableHookMetadata&);
    static void bind_static_table(HarmlessMolotovHook, engine::VtableHookMetadata&);
    void publish() noexcept; //canonical43475828 publication, separate from RTTI preparation
    void unbind() noexcept;
};
void unbind_harmless_molotov_runtime() noexcept;
} // namespace skeet::effects

// Declarations from harmless_molotov_owner.hpp

namespace skeet::effects {
struct HarmlessMolotovOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78, primary first
    engine::OwnerRegistryBinding updates; //43475988, owner+8 second
};
struct HarmlessMolotovOwnerBindings {
    esp::EffectsSettings& settings;
    const HarmlessMolotovBindings& effects;
};
// Complete ordinary ctor43484AEA..43484CEC. Storage carries the canonical
// 43387448 alignment header; primary slot0 frees it through lifecycle.memory.
// Only the three C82CBD3B/F9200DEF/921CAA0A descriptors are initialized here.
std::size_t harmless_molotov_owner_size() noexcept;
void construct_harmless_molotov_owner(void* storage, const HarmlessMolotovOwnerRegistries&,
                                      ConfigRegistries&, HarmlessMolotovOwnerBindings);
} // namespace skeet::effects

// Declarations from model_entity_relationships.hpp

namespace skeet::effects {
struct ModelEntityBindings;
struct ModelRelationshipOffsets {
    std::uint32_t life_state, move_type, observer_mode, observer_target, team, survival_team;
};
// The teammate and camera helpers need no render frame or timing snapshot.
// Read local_player after the genuine engine93 call; read each ConVar only
// when its native branch reaches it, after observer resolution if applicable.
struct ModelEntityRelationships {
    void* engine;
    void* entity_list;
    void* const& local_player;
    ModelRelationshipOffsets offsets;
    engine::NativeConVarReference game_type;
    engine::NativeConVarReference teammates_are_enemies;
};
bool model_teammate(void*, const ModelEntityRelationships&);
bool model_teammate(void*, const ModelEntityBindings&);
} //namespace skeet::effects

// Declarations from model_hooks.hpp

namespace skeet::anti_aim {
struct CommandOwner;
}
namespace skeet::effects {
struct EffectsCommandOwner;
struct ModelHookReplacement {
    std::uint32_t slot, native_entry;
    void* source_entry;
};
inline constexpr std::uint32_t studio_model_hook_descriptor = 0x434757B0;
inline constexpr std::uint32_t model_occlusion_hook_descriptor = 0x434757E0;
// Exact contexts2086..2109, captured recipes21/22. The original protected
// scheduler supplies the real tables and selects the descriptor-install phase.
std::span<const ModelHookReplacement, 5> studio_model_hook_replacements();
std::span<const ModelHookReplacement, 1> model_occlusion_hook_replacements();
struct ModelMaterialProviderEntry {
    std::uint32_t native_entry;
    void* (*source_entry)();
};
// Same nullable active-owner slot published/cleared by4348E6E2/433279E3.
void bind_model_material_owner(EffectsCommandOwner* const& active) noexcept;
std::span<const ModelMaterialProviderEntry, 3> model_material_provider_entries();
void* player_model_material(); //whole43336B64, cache0
void* hands_model_material(); //whole434019FB, cache5
void* weapon_viewmodel_material(); //whole43417FC3, cache6

struct ModelFakePoseBindings {
    anti_aim::CommandOwner* const& record; //live source43475940, one semantic AA owner
    const std::int32_t* const& current_tick; //live43475B2C, dereferenced for each draw
    const float& blend; //43475890
    const RageVector& observed_angles; //43476414
    const RageVector& reference_angles; //4347A4F0
    const float& rotation_radians; //43472534
};
FakeModelPose capture_fake_model_pose(const ModelFakePoseBindings&);

// Bind these to the existing owners and genuine globals. The draw hook reads
// them for each invocation; no constructor-time ModelDrawFrame is retained.
struct ModelRenderHookInputs {
    const ModelClassification& classification;
    const ModelEntityBindings& entities; //engine/interfaces and decoded offsets
    const RageTrigonometry& trigonometry;
    const lagcomp::RotationMath& rotation_math;
    const players::State& player_adjustments;
    ModelShadowSource shadows;
    const animation::BoneRuntime& bone_runtime;
    std::uint32_t local_player_flag, immunity, ragdoll_owner, attachment_owner;
    void* const& local_player; //43475808
    const std::uint8_t& player_effects; //4346A65C
    engine::NativeConVarReference game_type; //live43467A8C ConVar
    engine::NativeConVarReference teammates_are_enemies; //live4346A568 ConVar
    engine::NativeConVarReference shadow_history; //live43467A78 ConVar
    void* const& fake_controller; //434685B8
    ModelFakePoseBindings fake_pose;
    const std::uint32_t& shot_entity_handle; //4346D674
    const ModelShotState& shot; //same live state consumed by WorldCallbacks
    const ModelOcclusionInterfaces& occlusion;
};

class ModelRenderHooks {
public:
    ModelRenderHooks(ModelRenderRuntime&, WorldRuntime&, const ModelRenderHookInputs&,
                     ModelHookDescriptor& studio, ModelHookDescriptor& occlusion);
    ~ModelRenderHooks();
    ModelRenderHooks(const ModelRenderHooks&) = delete;
    ModelRenderHooks& operator=(const ModelRenderHooks&) = delete;
    // Call after the actual table producer has prepared replacement storage,
    // before the existing lifecycle installs either table on its real object.
    void bind_replacements();
    void bind_studio_replacements();
    void bind_occlusion_replacements();
    ModelHookDescriptor& studio_descriptor() const noexcept {
        return studio_;
    }
    ModelHookDescriptor& occlusion_descriptor() const noexcept {
        return occlusion_;
    }
    void draw(void* studio, const ModelDrawArguments&);
    void unoccluded(void* list, void* view);
    void prop50(void*, std::uintptr_t, std::int32_t, void*, std::uintptr_t);
    void prop48(void*, std::uintptr_t, std::int32_t, void*, std::int32_t, std::uintptr_t);
    void prop60(void*, std::uintptr_t, std::int32_t, ModelPropBatch*, std::int32_t, std::uint32_t);
    void prop_studio(void*, std::uintptr_t, std::uintptr_t, std::uintptr_t);

private:
    ModelRenderRuntime& models_;
    WorldRuntime& world_;
    const ModelRenderHookInputs& inputs_;
    ModelHookDescriptor& studio_;
    ModelHookDescriptor& occlusion_;
};

#if defined(_M_IX86) || defined(__i386__)
#define SKEET_MODEL_HOOK_ABI __fastcall
#else
#define SKEET_MODEL_HOOK_ABI
#endif
void SKEET_MODEL_HOOK_ABI studio_model_draw_hook(void*, void*, void*, void*, void*, void*, void*,
                                                 void*, std::uint32_t);
void SKEET_MODEL_HOOK_ABI studio_prop_hook(void*, void*, std::uintptr_t, std::uintptr_t,
                                           std::uintptr_t);
void SKEET_MODEL_HOOK_ABI model_prop50_hook(void*, void*, std::uintptr_t, std::int32_t, void*,
                                            std::uintptr_t);
void SKEET_MODEL_HOOK_ABI model_prop48_hook(void*, void*, std::uintptr_t, std::int32_t, void*,
                                            std::int32_t, std::uintptr_t);
void SKEET_MODEL_HOOK_ABI model_prop60_hook(void*, void*, std::uintptr_t, std::int32_t,
                                            ModelPropBatch*, std::int32_t, std::uint32_t);
void SKEET_MODEL_HOOK_ABI model_unoccluded_hook(void*, void*, void*);
#undef SKEET_MODEL_HOOK_ABI
} // namespace skeet::effects

// Declarations from model_render.hpp

namespace skeet::engine {
class NativeDataImage;
}
namespace skeet::client {
class PlayerStateCache;
}
namespace skeet::effects {
class ModelRenderHooks;
class ModelShadowTimingSource;

// Genuine materialsystem internal fastcall recorded in43467A68. The supported
// body consumes material/name in ECX/EDX; it is not a PE export.
#if defined(_M_IX86) || defined(__i386__)
using CreateMaterialVariable = void*(__fastcall*)(void*, const char*);
#else
using CreateMaterialVariable = void* (*)(void*, const char*);
#endif
struct ModelMaterialInterfaces {
private:
    void* standalone_material_ = nullptr;
    CreateMaterialVariable standalone_create_ = nullptr;
    void* standalone_clock_ = nullptr;

public:
    ModelMaterialInterfaces() noexcept
        : material_system(standalone_material_), create_variable(standalone_create_),
          clock(standalone_clock_) {}
    ModelMaterialInterfaces(void* material, void* key_values, CreateMaterialVariable create,
                            void* timer = nullptr) noexcept
        : ModelMaterialInterfaces() {
        material_system = material;
        key_values_system = key_values;
        create_variable = create;
        clock = timer;
    }
    ModelMaterialInterfaces(engine::NativeDataImage&, void* key_values);
    ModelMaterialInterfaces(const ModelMaterialInterfaces&) = delete;
    ModelMaterialInterfaces& operator=(const ModelMaterialInterfaces&) = delete;
    void*& material_system; //live4346A4E0
    void* key_values_system = nullptr; // vstdlib!KeyValuesSystem()
    CreateMaterialVariable& create_variable; //live43467A68, optional native entry
    void*& clock; //live43467118, clock[5] returns double
};

// 0x44-byte native record, constructed by 0x433A0E96. Tokens belong to
// FindVarFast; these are not IMaterialVar pointers.
struct ModelMaterial {
    void* material = nullptr;
    std::array<std::uint32_t, 11> variable_tokens{};
    const IntegerConfig* kind = nullptr;
    const ColorConfig* secondary = nullptr;
    std::int32_t applied_kind = -1;
    std::uint32_t applied_color = UINT32_MAX;
    bool on_shot = false;
};

struct ModelDrawState {
    std::array<std::byte, 0x38> bytes{}; // studio render +0x240..+0x277
};
struct ModelPassSettings {
    // Native packet bits: local=1, special model=2, visible=4, occluded=8,
    // shadow/history=16. 0x433E655C selects these from the owner descriptors.
    std::uint8_t flags = 0;
    std::uint32_t visible_color = 0, occluded_color = 0;
    ModelMaterial* material = nullptr;
};
struct PlayerModelColors {
    std::uint8_t flags;
    std::uint32_t visible_color, occluded_color;
};
// Shared descriptor selection for drawing and opacity. Material resources are
// deliberately outside this helper: its three outputs contain only settings.
PlayerModelColors player_model_colors(const esp::ColoredModelsSettings&, bool local, bool teammate,
                                      bool history_available, bool ragdoll = false) noexcept;
struct ModelDrawArguments {
    void* results;
    void* info;
    void* bones;
    void* flex_weights;
    void* delayed_flex_weights;
    void* origin;
    std::uint32_t flags;
};
enum class RenderedModel : std::uint32_t {
    player = 0,
    viewmodel = 1,
    hands = 2,
    flagged_viewmodel = 3,
    other = 4,
    ragdoll = 5,
    player_attachment = 6,
    owned_weapon = 7,
    unowned_weapon = 8,
    world_weapon_model = 9,
};
class ModelRenderableTables {
public:
    ModelRenderableTables() noexcept = default;
    explicit ModelRenderableTables(engine::NativeDataImage& data) noexcept : data_(&data) {}
    void*& operator[](std::size_t);
    void* const& operator[](std::size_t) const;

private:
    engine::NativeDataImage* data_ = nullptr;
    std::array<void*, 9> standalone_{};
};
struct ModelClassification {
    // Recovered renderable vtables, in native comparison order. The first two
    // are 0x434758AC/0x434759A8; the others are 0x43468908, 0x43467190,
    // owner+0x73C, 0x43475B44, owner+0x738, 0x4346A444, 0x43469284.
    ModelRenderableTables renderable_vtables;
    void* entity_list = nullptr;
    std::uint32_t entity_flags; // *0x43468B50 ^ *0x43467B48 ^ 0x38CA5228
    std::uint32_t owner_handle; // *0x43468C9C ^ *0x43469280 ^ 0x4E36DFD7
};
struct ModelShotState {
private:
    double standalone_expires_ = 0;
    void* standalone_begin_ = nullptr;
    void* standalone_end_ = nullptr;
    std::array<float, 3> standalone_origin_{};

public:
    ModelShotState() noexcept
        : expires(standalone_expires_), bones_begin(standalone_begin_), bones_end(standalone_end_),
          origin(standalone_origin_) {}
    explicit ModelShotState(engine::NativeDataImage&);
    ModelShotState(const ModelShotState&) = delete;
    ModelShotState& operator=(const ModelShotState&) = delete;
    double& expires; //physical4347BDE8
    void*& bones_begin; //physical434797A0
    void*& bones_end; //physical434797A4
    std::array<float, 3>& origin; //physical4347BDF0
};
struct ModelEntityBindings {
    void* engine;
    void* entity_list;
    void* local_player;
    std::uint32_t life_state, move_type, observer_mode, observer_target;
    std::uint32_t team, survival_team;
    std::uint32_t move_parent; // resolved 0x49D93E62 property minus 0x18
    std::int32_t game_type;
    bool teammates_are_enemies;
};
struct FakeModelPose {
    bool available; // native 0x43475940
    std::int32_t capture_tick, current_tick;
    float blend; // 0x43475890
    float observed_yaw, reference_yaw; // record +0x258/+0x194
    RageVector observed_angles, reference_angles; // 0x43476414/0x4347A4F0
    RageVector transform_angles; // record +0x1B8
    float rotation_radians; // 0x43472534
    std::span<const Matrix3x4> bones; // record +0x520
};
// Complete 0x43369C23, with engine-captured fake pose inputs and recorded CRT
// sine/cosine semantics. Output capacity follows the captured bone-count contract.
bool build_fake_model_matrices(const FakeModelPose&, const RageVector& origin,
                               std::span<Matrix3x4> output, const RageTrigonometry&);
struct ModelShadowTiming {
    float interval, current_time;
    float update_rate, interpolation, interpolation_ratio;
    bool local_alive, prediction_enabled;
    std::int32_t tick_base;
    float prediction_counter; // native globals +0x24 is loaded with MOVSS
    float prediction_correction;
    std::array<float, 16> latency_samples;
    std::int32_t latency_sample_head, latency_sample_count;
    std::int32_t command_tick;
    const std::int32_t* previous_command;
    const std::int32_t* current_command;
    std::int32_t maximum_unlag_ms;
};
struct ModelShadowPose {
    const lagcomp::LagRecord* animation_record;
    float time;
    RageVector origin, angles;
};
// 43406265..43406674, including native timing helpers4331E237/43385D57.
// Ring records are nonnull, ordered from newest to oldest, with at most32.
bool select_model_shadow_pose(const lagcomp::LagTimeline&, const ModelShadowTiming&,
                              ModelShadowPose&, const lagcomp::RotationMath&);
// Explicit input profiles: production borrows the canonical cache and samples
// live timing; standalone algorithm fixtures borrow their supplied timeline.
class ModelShadowSource {
public:
    ModelShadowSource(client::PlayerStateCache&, const ModelShadowTimingSource&) noexcept;
    ModelShadowSource(const std::array<const lagcomp::LagTimeline*, 64>&,
                      const ModelShadowTiming&) noexcept;
    bool select(std::uint32_t entity_index, ModelShadowPose&, const lagcomp::RotationMath&) const;

private:
    client::PlayerStateCache* cache_ = nullptr;
    const ModelShadowTimingSource* live_ = nullptr;
    const std::array<const lagcomp::LagTimeline*, 64>* timelines_ = nullptr;
    const ModelShadowTiming* timing_ = nullptr;
};
struct ModelShadowBones {
    animation::BoneEntityState& state;
    const animation::BoneModel& model;
    const animation::BoneEngine& engine;
    const RageTrigonometry& trigonometry;
    void* layers;
    std::int32_t layer_count;
    float* pose_parameters;
};
struct ModelDrawFrame {
    const ModelClassification& classification;
    const ModelEntityBindings& entities;
    const RageTrigonometry& trigonometry;
    const lagcomp::RotationMath& rotation_math;
    std::uint32_t local_player_flag, immunity;
    std::uint32_t ragdoll_owner, attachment_owner;
    bool suppress_effects; // 0x43468E58
    bool player_effects; // 0x4346A65C
    const players::State& player_adjustments; // live4347BE09 + native index*32
    bool history_available; // resolved native history cvar condition
    bool fake_controller; // 0x434685B8 !=0
    const FakeModelPose* fake_pose; // absent native 0x43475940 is null
    ModelShadowSource shadows;
    const animation::BoneRuntime& bone_runtime;
    std::uint32_t shot_entity_handle; // 0x4346D674; -1 means absent
    const ModelShotState* shot; // null means the native shot vector is empty
    std::int32_t wall_transparency; // shared effects descriptor 0x9B4090D9
};
struct LocalTransparencyInterfaces {
    void* input;
    void* entity_list;
    void* model_cache;
};
struct LocalTransparencyNetvars {
    std::uint32_t field_af649dac;
    std::uint32_t field_cacaeecc; // native special guard also reads this minus0xC
    std::uint32_t field_c07d1150;
    std::uint32_t active_weapon;
};
struct ModelOcclusionRecord {
    void* renderable;
    std::uint16_t field_04;
    std::uint8_t field_06, flags, model_type;
    std::array<std::byte, 3> untouched;
};
struct ModelOcclusionStorage {
    std::array<std::byte, 0x14> untouched;
    std::array<std::array<ModelOcclusionRecord, 4096>, 3> buckets;
    std::array<std::int32_t, 3> counts;
};
struct ModelOcclusionItem {
    void* renderable;
    void* metadata;
    std::array<std::byte, 0x18 - 2 * sizeof(void*)> untouched;
    std::uint16_t flags;
    std::array<std::byte, 0x4E - 0x1A> remaining;
    std::uint16_t next;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(ModelOcclusionRecord) == 0xC);
static_assert(offsetof(ModelOcclusionStorage, buckets) == 0x14);
static_assert(offsetof(ModelOcclusionStorage, counts) == 0x24014);
#endif
static_assert(sizeof(ModelOcclusionItem) == 0x50);
struct ModelOcclusionInterfaces {
    void* entity_list; // 0x4346A890
    void* model_info; // 0x4346A614
    void* model_type_registry; // 0x43469330
    void* player_renderable_vtable; // 0x434758AC
    std::uint32_t shot_entity_handle;
};
struct ModelPropBatch {
    std::array<std::byte, 8> untouched;
    void* vertices;
    std::int32_t count;
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(sizeof(ModelPropBatch) == 0x10);
static_assert(offsetof(ModelPropBatch, vertices) == 8);
static_assert(offsetof(ModelPropBatch, count) == 0xC);
#endif

class ModelRenderRuntime {
public:
    ModelRenderHooks* hooks = nullptr;
    ModelRenderRuntime(ModelMaterialInterfaces& interfaces,
                       const esp::ColoredModelsSettings& settings);
    ModelMaterialInterfaces& interfaces;
    const esp::ColoredModelsSettings& settings;
    std::array<ModelMaterial, 8> materials;
    void* solid_material = nullptr; // owner +0x678; located by hash 0xF4A7A78A
    void* shadow_material = nullptr; // +0x680; hash 0x27DB215F
    void* base_texture = nullptr; // +0x694
    void* metallic_environment = nullptr; // +0x6A8
    void* glow_environment = nullptr; // +0x6BC
    void* fullscreen_texture = nullptr; // +0x6D0
    std::array<std::uint32_t, 2> material_name_hashes{0xF4A7A78Au, 0x27DB215Fu};
    std::array<std::string, 4> texture_names{"vgui/white", "editor/cube_vertigo_blurred_hdr",
                                             "models/effects/cube_white", "_rt_Fullscreen"};
    float local_alpha = 1.0f; // native owner+0x38
    bool local_alpha_overridden = false; // owner+0x3C

    // Full resource setup 0x433F0977, texture refs 0x433378F3 and material
    // enumeration 0x433468AF. Successful lookup consumes its native name/hash.
    void initialize_resources();

    // Complete lazy material/settings chain, 0x433C6694 and 0x4336BB4B.
    void* prepare_material(ModelMaterial* material);
    void invalidate_material(void* material); // complete4333CDB7, Lua material refresh
    // Complete native color/alpha helpers 0x433F4C0E / 0x433ADE29.
    static void modulate(void* studio_render, std::uint32_t color);
    static void limit_alpha(void* studio_render, std::uint32_t color);
    static ModelDrawState save_state(void* studio_render);
    static void restore_state(void* studio_render, const ModelDrawState& saved);
    // Complete 0x4332CFC4; studio header name resolver 0x433C0864 is included.
    static RenderedModel classify(const void* draw_info, const ModelClassification& bindings);
    // Selects the exact packet for player/teammate/local/ragdoll branches.
    // The entity classification and teammate predicate are supplied as their
    // results, without replacing either engine/entity implementation.
    ModelPassSettings player_pass(bool local, bool teammate, bool history_available,
                                  bool ragdoll = false) noexcept;
    // Visible/occluded body 0x43406770..0x43406A56. History bone interpolation
    // precedes this body in the original and has its own owning subsystem.
    void apply_player_passes(void* studio_render, const ModelDrawState& saved,
                             const ModelDrawArguments& args, const ModelPassSettings& pass,
                             std::int32_t wall_transparency = 100);
    void apply_single_pass(void* studio_render, std::size_t group);
    // Complete hands/viewmodel branches of 0x433E655C, including the two
    // material-index overrides and save/draw/restore owner sequence.
    bool draw_viewmodel(void* studio_render, const ModelDrawArguments&, RenderedModel category);
    // 0x434181CD and dependencies 0x43359AFA / 0x4339EF8C, valid entity contract.
    static bool teammate(void* entity, const ModelEntityBindings&);
    // Complete owned/world-weapon branches of 0x433E655C, including walking
    // the move-parent chain and using the viewmodel settings for local owners.
    bool draw_weapon(void* studio_render, const ModelDrawArguments&, RenderedModel,
                     const ModelEntityBindings&);
    // Complete 0x43327FF7. Capture of the shot's matrices/origin/expiry belongs
    // to the event owner; the native draw helper only consumes this state.
    void draw_on_shot(void* studio_render, const ModelDrawState& saved,
                      const ModelDrawArguments& args, const ModelShotState& shot);
    // 0x433E6E13..0x433E6F05; caller has passed native player eligibility,
    // active fake-controller and local-entity gates. Matrix generation is the
    // complete 43369C23 above, rather than a captured-output replacement.
    void draw_local_fake(void* studio_render, const ModelDrawState& saved,
                         const ModelDrawArguments&, const FakeModelPose&, const RageTrigonometry&);
    // 43406499..43406770 composed with restored animation and bone owners.
    // Returns false when engine/model readiness rejects the native bone build.
    bool draw_shadow(void* studio_render, const ModelDrawArguments&, const ModelShadowPose&,
                     ModelShadowBones&);
    // Live production join: studio, model, hierarchy and temporary entity state
    // are sampled by43388720 for this entity/call, under selected animation.
    bool draw_shadow(void* studio_render, const ModelDrawArguments&, const ModelShadowPose&,
                     void* player, const animation::BoneRuntime&);
    // Complete top-level 433E655C selection, using the concrete restored
    // material/fake/history owners above and actual engine entity interfaces.
    void draw_model(void* studio_render, const ModelDrawArguments&, const ModelDrawFrame&);
    // Complete433CA178 and4336AC57/4337E85B. renderable is the actual
    // player+4 interface, provided explicitly for host pointer-width verification.
    void update_local_transparency(void* player, void* renderable,
                                   const LocalTransparencyInterfaces&,
                                   const LocalTransparencyNetvars&, bool suppressed,
                                   bool player_effects, float frame_time);
    // Complete43368F23: original list builder, native linked-item traversal,
    // classification, duplicate/capacity handling, and model-type queries.
    void build_unoccluded_list(void* renderable_list, void* view, const ModelOcclusionInterfaces&);
    // Complete prop-lighting hooks433B0A83/433C930C/4337D6DC/43401A33.
    // The original engine methods remain in their hook descriptor tables.
    void draw_prop_records_50(void* renderer, std::uintptr_t argument, std::int32_t count,
                              void* records, std::uintptr_t last, const esp::EffectsSettings&,
                              bool suppressed);
    void draw_prop_records_48(void* renderer, std::uintptr_t argument, std::int32_t count,
                              void* records, std::int32_t stride, std::uintptr_t last,
                              const esp::EffectsSettings&, bool suppressed);
    void draw_prop_batches_60(void* renderer, std::uintptr_t argument, std::int32_t count,
                              ModelPropBatch* batches, std::int32_t stride, std::uint32_t flags,
                              void* ragdoll_renderable_vtable, const esp::EffectsSettings&,
                              bool suppressed);
    void draw_prop_studio(void* studio_render, std::uintptr_t first, std::uintptr_t second,
                          std::uintptr_t third, const esp::EffectsSettings&, bool suppressed);

private:
    void draw_shadow_matrices(void* studio_render, const ModelDrawArguments&,
                              const ModelShadowPose&, std::span<Matrix3x4>);
    void* create_material();
    void resolve_variable(void* material, const char* name, std::uint32_t& token);
    void apply_gated_player(void* studio_render, const ModelDrawState&, const ModelDrawArguments&,
                            void* entity, const ModelPassSettings&, const ModelDrawFrame&);
};

} // namespace skeet::effects

// Declarations from model_render_lifecycle.hpp

namespace skeet::effects {
// Native hook descriptors434757B0/434757E0. The installed vtable's[-2]
// points to this descriptor; original table and owner are at+8/+C on x86.
struct ModelHookDescriptor {
    std::uint32_t untouched;
    void*** object;
    void** original_table;
    void* owner;
    void** replacement_table;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(ModelHookDescriptor, object) == 4);
static_assert(offsetof(ModelHookDescriptor, original_table) == 8);
static_assert(offsetof(ModelHookDescriptor, owner) == 0xC);
static_assert(offsetof(ModelHookDescriptor, replacement_table) == 0x10);
#endif
struct ModelRenderLifecycleState {
    std::uint8_t field_450;
    void* allocation_760;
};
struct ModelRenderLifecycleBindings {
    ModelHookDescriptor& first_hook;
    ModelHookDescriptor& second_hook;
    CameraInterfaces& camera;
    ObserverProxy const& initial_observer_proxy; //live43476A34, read by lifecycle slot3
    void* const& first_object; //live43468A4C, actual object/vptr slot
    void* const& second_object; //live4346A3A0, actual object/vptr slot
    void* const& allocator; //live actual IMemAlloc43467204, Free slot5
    std::uint32_t& shot_entity_handle; //same4346D674 used by ModelDrawFrame
};
struct ShotEntityBoneOffsets {
    std::ptrdiff_t bone_accessor; //54783141
    std::ptrdiff_t recent_counter; //2B554AD0
};
// Complete43349F65, secondary owner vtable43472768 slot6. The two native
// callback arguments are unused; only the shared handle and engine entity are read.
void update_shot_entity_bones(void* entity_list, std::uint32_t shot_entity_handle,
                              ShotEntityBoneOffsets, const std::uint32_t* current_counter);
// Shared write block for433F9D3D after that caller's existing entity lookup.
void update_shot_entity_bones_for_entity(void* entity, ShotEntityBoneOffsets,
                                         const std::uint32_t* current_counter);

// Actual lifecycle entries in owner vtable4346F0E4. Drawing and world state
// remain with their existing owners; reset/shutdown mutate those same objects.
class ModelRenderLifecycle {
public:
    ModelRenderLifecycle(ModelRenderRuntime& models, esp::ColoredModelsSettings& colors,
                         WorldRuntime& world, ModelRenderLifecycleState& state,
                         ModelRenderLifecycleBindings& bindings)
        : models(models), colors(colors), world(world), state(state), bindings(bindings) {}
    ModelRenderRuntime& models;
    esp::ColoredModelsSettings& colors;
    WorldRuntime& world;
    ModelRenderLifecycleState& state;
    ModelRenderLifecycleBindings& bindings;

    void initialize_bindings(); //complete43484A81, owner vtable slot3
    void shutdown(); //complete43402097, slot4
    void reset(); //complete433CA4BD, slot5
    void reset_thunk(); //complete433CF2F5, slot7 ->same reset
    void clear_shot_marker(); //complete433AB8A9, slot8
};
} // namespace skeet::effects

// Declarations from model_render_sources.hpp

namespace skeet::client {
class PlayerStateCache;
}
namespace skeet::engine {
struct NativeOffsetGlobals;
}
namespace skeet::effects {
// The shadow branch samples these actual cells only after its record-count
// and movement gates. No draw-time clock or history projection is retained.
class ModelShadowTimingSource {
public:
    ModelShadowTimingSource(engine::NativeDataImage&, const engine::NativeOffsetGlobals&);
    ModelShadowTiming capture() const;

private:
    engine::NativeDataImage& data_;
    const engine::NativeOffsetGlobals& offsets_;
};
} //namespace skeet::effects

// Declarations from owner_lifecycle.hpp

namespace skeet::effects {
class ModelRenderHooks;
struct EffectsOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    client::CommandPhaseRegistry& commands; //434757A0
    engine::OwnerRegistryBinding hud; //43475A9C
    client::EntityObserverRegistry& entities; //43475BA8, concrete effects ID
    engine::OwnerRegistryBinding updates; //43475988
    engine::OwnerRegistryBinding frames; //43475AD8
    client::CommandEmptyTickRegistry& empty_ticks; //43475978
};
struct EffectsOwnerBindings {
    EffectsCommandOwner& command;
    ModelRenderLifecycle& lifecycle;
    ModelRenderHooks& hooks;
    WorldCallbacks& updates;
    events::EventRegistry& event_registry;
    EffectsCommandOwner*& active; //canonical43475A1C slot
    engine::NativeDataImage& data; //same canonical constructor ConVars/RTTI cells
    ModelClassification& classification; //constructor captures consumed renderable tables here
};
// Storage is genuine43387448-style aligned storage; lifecycle slot0 releases
// its original allocation header through lifecycle.memory. All semantic owners
// are canonical borrowed objects and must outlive their registered endpoints.
std::size_t model_effects_owner_size() noexcept;
// Complete seven-interface registration/configuration composition4348E6E2.
// Pass fresh canonical model/world state and its unregistered52 owned config
// descriptors. Independent static settings use their own initializers.
void construct_model_effects_owner(void* storage, const EffectsOwnerRegistries&, ConfigRegistries&,
                                   EffectsOwnerBindings);
} // namespace skeet::effects

// Declarations from owner_settings.hpp

namespace skeet::effects {
// Exact52 owned descriptors4348E73C..4348EC94. Pass the unregistered canonical
// objects. Static descriptors and other effects owners are initialized by their
// own constructors, rather than being registered a second time here.
void initialize_model_effects_owner_settings(ConfigRegistries&, esp::EffectsSettings&,
                                             esp::ColoredModelsSettings&) noexcept;
} // namespace skeet::effects

// Declarations from player_auxiliary_hooks.hpp

namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
class ConfigurationRuntimeOwner;
class HistoryRuntimeOwner;
}
namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
class HookRuntimeOwner;
}
namespace skeet::effects {
struct EntityVisibilityRuntime;
class EntityNetworkableRuntime;
struct FreeLookCaptureBindings;

// Actual per-player hook inputs shared by the primary and ragdoll families.
// Offsets and player state come from the canonical history owner; callbacks
// resolve their saved local/entity at the original call sites. This owner
// creates no second free-look state, player cache, configuration or hook list.
// Keep these contexts and all borrowed owners alive until hooks are restored.
class PlayerAuxiliaryHooks {
public:
    PlayerAuxiliaryHooks(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                         client::ConfigurationRuntimeOwner&, client::HistoryRuntimeOwner&,
                         animation::RuntimeOwner&, engine::HookRuntimeOwner&);
    ~PlayerAuxiliaryHooks();
    PlayerAuxiliaryHooks(const PlayerAuxiliaryHooks&) = delete;
    PlayerAuxiliaryHooks& operator=(const PlayerAuxiliaryHooks&) = delete;
    PlayerAuxiliaryHooks(PlayerAuxiliaryHooks&&) = delete;
    PlayerAuxiliaryHooks& operator=(PlayerAuxiliaryHooks&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    client::HistoryRuntimeOwner& history() const noexcept;
    EntityVisibilityRuntime& visibility() noexcept;
    EntityNetworkableRuntime& ragdolls() noexcept;
    const FreeLookCaptureBindings& free_look_capture() const noexcept;

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
}

// Declarations from player_opacity.hpp

namespace skeet::effects {
struct EffectsCommandOwner;
struct PlayerOpacityOffsets {
    std::ptrdiff_t field_af649dac;
    std::ptrdiff_t local_player_d070684c;
    std::ptrdiff_t view_offset_c246315e;
    std::ptrdiff_t duck_amount_3309c0cf;
    std::ptrdiff_t smoke_effect_58ae2423;
    std::ptrdiff_t smoke_tick_f267e956;
};
struct PlayerOpacityBindings {
    EffectsCommandOwner* const& active_effects; //actual nullable43475A1C owner
    const esp::ColoredModelsSettings& settings; //same source descriptors as model rendering
    const ModelEntityRelationships& entities;
    const players::State& player_adjustments; //canonical4347BE08, no copy
    void* input; //actual4346A894, virtual32(-1)
    void* const& smoke_registry; //actual434679C0, handles data0/countC
    const std::int32_t* const& smoke_clock; //pointed43468D4C
    const float& interval; //pointed4346A338
    const RageVector& camera_position; //43479CCC
    const std::uint8_t& suspended; //43468E58
    engine::NativeConVarReference lag_compensation; //live43467A78 XOR3AE06513
    PlayerOpacityOffsets offsets;
};
// Complete433CC653: target origin within149 units of an active smoke entity
// whose elapsed effect time is strictly between0 and18 seconds.
bool player_inside_active_smoke(void* player, const PlayerOpacityBindings&);
// Complete4341D06C plus43367C8B: target eye within36 units of the finite
// local-eye/camera segment. A dead/absent local player returnsfalse.
bool player_near_camera_segment(void* player, const PlayerOpacityBindings&);
// Complete4336E8B8. Uses the native live-entity/nonzero-index contract and the
// same colored-model descriptors/PLAYERS table as rendering. No ESPfade copy.
std::uint8_t evaluate_player_opacity(void* player, const PlayerOpacityBindings&);
// Complete43340082 including4339EBB5's genuine entity alpha virtual395.
// Owner absence skips all calls; mode396 receives0/4 and the originaltrue byte.
void apply_player_opacity(void* player, const PlayerOpacityBindings&);
} // namespace skeet::effects

// Declarations from player_opacity_owner.hpp

namespace skeet::client {
class ClientRuntimeStorage;
struct EngineRuntimeBindings;
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
}
namespace skeet::effects {
// No material, D3D or renderer resources are needed to compute opacity. The
// actual nullable model-owner cell still gates43340082 on each invocation;
// descriptors come from the one source configuration owner. No placeholder
// renderer is constructed or published by this owner.
class PlayerOpacityOwner {
public:
    PlayerOpacityOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                       client::ConfigurationRuntimeOwner&, client::PlayerRuntimeOwner&);
    PlayerOpacityOwner(const PlayerOpacityOwner&) = delete;
    PlayerOpacityOwner& operator=(const PlayerOpacityOwner&) = delete;
    PlayerOpacityOwner(PlayerOpacityOwner&&) = delete;
    PlayerOpacityOwner& operator=(PlayerOpacityOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    const PlayerOpacityBindings& bindings() const noexcept {
        return bindings_;
    }
    void apply(void* player) const {
        apply_player_opacity(player, bindings_);
    }

private:
    client::ClientRuntimeStorage& storage_;
    ModelEntityRelationships entities_;
    PlayerOpacityBindings bindings_;
};
} //namespace skeet::effects

// Declarations from projectile_ribbons.hpp

namespace skeet::effects {

// Actual owner +94..98 entries. Fields not read by 43380299 are preserved.
struct ProjectileRibbonEntry {
    std::uint32_t handle;
    std::uint32_t reserved_04[4];
    std::int32_t state; // only 0 and 1 are eligible
    esp::GrenadeKind kind;
    std::uint8_t rejected;
    std::uint8_t reserved_1b;
    float creation_time;
    std::uint32_t reserved_20;
};
static_assert(sizeof(ProjectileRibbonEntry) == 0x24);
static_assert(offsetof(ProjectileRibbonEntry, kind) == 0x18);
static_assert(offsetof(ProjectileRibbonEntry, creation_time) == 0x1c);

struct ProjectileRibbonOffsets {
    explicit ProjectileRibbonOffsets(const engine::NativeOffsetGlobals&);
    engine::DecodedOffset inactive; // 01E80986, int32
    engine::DecodedOffset effects; // 38CA5228, uint32; bit 5 suppresses prediction
    engine::DecodedOffset thrower_handle; // DC743513, handle
    engine::DecodedOffset velocity; // 36375C23, Vector3
    engine::DecodedOffset previous_velocity; // 434E9E95, Vector3
    engine::DecodedOffset simulation_time; // D5FD2E2C, float
    engine::DecodedOffset origin; // A9EE6062, Vector3
    engine::DecodedOffset collision_group; // E4E2D1A7, int32
    engine::DecodedOffset projectile_flags; // 7418868F, low uint16
};
struct ProjectileRibbonBindings {
    void* const& entity_list; // live4346A890, entity handle lookup at virtual slot4
    void* const& material_system; // live4346A4E0
    void* const& key_values_system; // actual vstdlib!KeyValuesSystem() result cell
    ProjectileRibbonOffsets offsets;
};
struct ProjectileRibbonFrame {
    bool suppressed; // 43468E58
    bool active; // 43467CD4
    std::int32_t current_tick; // *43468D4C
    std::uint32_t color; // 43468F60, native RGBA bytes
};
// Production bindings borrow the actual global storage. Tick and color are
// read at their native sites after preceding entity/simulation engine calls.
struct ProjectileRibbonLiveFrame {
    const std::uint8_t& suppressed; // 43468E58
    const std::uint8_t& active; // 43467CD4
    const std::int32_t& current_tick; // *43468D4C
    const std::uint32_t& color; // 43468F60
};

// These are the owner's actual decoded strings, not replacement asset names.
struct ProjectileRibbonMaterialNames {
    std::string render_mode; // owner +240, FindVarFast token +220
    std::string texture; // owner +250
    std::string base_texture; // owner +260, FindVarFast token +224
    std::string shader; // owner +270
};

class ProjectileRibbonOwner {
public:
    ProjectileRibbonMaterialNames material_names;
    void* material = nullptr; // owner +A4, lazily created
    std::uint32_t render_mode_token = 0, base_texture_token = 0;

    // Complete 43380299. Uses the real entity/material/trace boundaries;
    // prediction, filtering, endpoint rules and beam geometry remain source.
    void draw(const ProjectileRibbonFrame&, std::span<const ProjectileRibbonEntry>,
              const ProjectileRibbonBindings&, const esp::GrenadeEnvironment&,
              esp::GrenadeTraceProvider);
    // The production owner passes its canonical warning vector and the shared
    // full-owner+D0 world-point scratch used by warning refresh directly.
    void draw(const ProjectileRibbonLiveFrame&, std::span<const esp::GrenadeWarningRecord>,
              const ProjectileRibbonBindings&, const esp::GrenadeEnvironmentBindings&,
              esp::GrenadeTraceProvider, std::vector<esp::Vector3>& world_path);
    void release_material(); // complete4336843F: virtual13, then clear +A4
    std::span<const esp::Vector3> path() const noexcept {
        return path_;
    }

private:
    template <class Frame, class Entry, class Environment>
    void draw_impl(const Frame&, std::span<const Entry>, const ProjectileRibbonBindings&,
                   const Environment&, esp::GrenadeTraceProvider,
                   std::vector<esp::Vector3>& world_path);
    void prepare_material(const ProjectileRibbonBindings&);
    std::vector<esp::Vector3> path_; // fixture storage; production borrows full-owner+D0
};

} // namespace skeet::effects

// Declarations from receive_proxies.hpp

namespace skeet::client {
class PlayerStateCache;
}
namespace skeet::esp {
struct EffectsSettings;
}
namespace skeet::effects {
struct VisualReceiveDescriptor {
    std::uint32_t operand, class_hash, table_hash, property_hash, native_entry, patch_address;
    engine::RecvVarProxy replacement;
};
std::span<const VisualReceiveDescriptor, 5> visual_receive_descriptors() noexcept;
enum class VisualReceivePatchFailureReason {
    missing_property,
    missing_original,
    already_initialized
};
class VisualReceivePatchFailure final : public std::runtime_error {
public:
    VisualReceivePatchFailure(std::size_t, const VisualReceiveDescriptor&,
                              VisualReceivePatchFailureReason);
    const std::size_t index;
    const std::uint32_t native_entry, patch_address;
    const VisualReceivePatchFailureReason reason;
};
// Keep this lifetime through receive-patch restoration. All settings, offsets,
// player records, interfaces and saved callbacks are the shared source owners.
class VisualReceiveRuntime {
public:
    VisualReceiveRuntime(engine::NativeDataImage&, const esp::EffectsSettings&,
                         const engine::NativeOffsetStorage&, client::PlayerStateCache&);
    engine::NativeDataImage& data() const noexcept {
        return data_;
    }
    void receive_smoke(const engine::RecvProxyData*, void*, void*); //4337EDF0
    void receive_flash(const engine::RecvProxyData*, void*, void*); //4338A0C3
    void receive_observer(const engine::RecvProxyData*, void*, void*); //4340C7B4
    void receive_fov(const engine::RecvProxyData*, void*, void*); //4339EBD4
    void receive_sun(const engine::RecvProxyData*, void*, void*); //433E978B
    // The source producer must already have captured actual target/original
    // fields in these exact five canonical nodes. This publishes source entry
    // words only, leaving every link, target and original callback untouched.
    void bind_receive_patches();
    // Resolve/capture the five genuine current RecvProp slots first, then bind
    // these source receivers. No property or list is installed. All-five
    // validation is explicit source policy while the native missing-property
    // branch and list-write order remain under original-code verification.
    void initialize_receive_patches();

private:
    engine::NativeDataImage& data_;
    const esp::EffectsSettings& settings_;
    const engine::NativeOffsetStorage& offsets_;
    client::PlayerStateCache& players_;
    std::uint32_t offset(std::uint32_t, std::uint32_t, std::uint32_t) const;
    void original(std::uint32_t, const engine::RecvProxyData*, void*, void*) const;
    void capture(const engine::RecvProxyData*, void*, bool);
};
} //namespace skeet::effects

// Declarations from runtime_owner.hpp

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class VisualRuntimeOwner;
class PlayerRuntimeOwner;
class CrtRuntimeOwner;
class HistoryRuntimeOwner;
class NativeStartupSources;
struct EffectsRuntimeFactoryBindings;
struct EngineRuntimeBindings;
}
namespace skeet::animation {
class RuntimeOwner;
}
namespace skeet::engine {
struct BuildMaterialBindings;
struct BuildEspBindings;
class SceneQueryRuntimeOwner;
}
namespace skeet::rage {
class ShotModelBoneStorage;
}
namespace skeet::effects {
class ModelRenderRuntime;
class WorldRuntime;
class ViewRuntime;
class CameraRuntimeOwner;
struct SmokeOwnerBindings;
struct ScopeOwnerBindings;
struct EffectsCommandOwner;
struct ModelShotState;
class ModelRenderHooks;
class ModelRenderLifecycle;
class WorldCallbacks;
// Stable material/world/view lifetime shared by Lua, World factories and the
// later full effects owner. Phase one binds genuine acquired engine cells and
// canonical settings only; no resource, feature or hook is initialized.
class EffectsRuntimeOwner {
public:
    EffectsRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                        client::ConfigurationRuntimeOwner&, client::VisualRuntimeOwner&,
                        const engine::BuildMaterialBindings&);
    // Explicit genuine engine boundary for independent source composition and
    // authored ABI fixtures. The production overload above obtains this exact
    // singleton from its already verified VisualRuntimeOwner.
    EffectsRuntimeOwner(client::ClientRuntimeStorage&, const client::EngineRuntimeBindings&,
                        client::ConfigurationRuntimeOwner&, const engine::BuildMaterialBindings&,
                        void* key_values_system);
    ~EffectsRuntimeOwner();
    EffectsRuntimeOwner(const EffectsRuntimeOwner&) = delete;
    EffectsRuntimeOwner& operator=(const EffectsRuntimeOwner&) = delete;
    EffectsRuntimeOwner(EffectsRuntimeOwner&&) = delete;
    EffectsRuntimeOwner& operator=(EffectsRuntimeOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept;
    ModelRenderRuntime& models() noexcept;
    WorldRuntime& world() noexcept;
    ViewRuntime& view() noexcept;
    SmokeOwnerBindings& smoke_bindings() noexcept;
    ScopeOwnerBindings& scope_bindings() noexcept;
    ModelShotState& shot_state() noexcept;
    rage::ShotModelBoneStorage& shot_bones() noexcept;
    // Phase two borrows the real camera, player cache, SceneQueries ballistics,
    // CRT spread/RNG/math and the one genuine shotgun producer. It creates no
    // primary owner and does not publish43475A1C or install a hook.
    void compose_command(CameraRuntimeOwner&, client::PlayerRuntimeOwner&,
                         engine::SceneQueryRuntimeOwner&, client::CrtRuntimeOwner&,
                         const engine::BuildEspBindings&);
    EffectsCommandOwner& command();
    // The primary phase joins the same command, history, bone and hook-table
    // owners. Factory registration/publication retains its startup position.
    void compose_primary(CameraRuntimeOwner&, client::HistoryRuntimeOwner&,
                         animation::RuntimeOwner&, client::CrtRuntimeOwner&);
    client::EffectsRuntimeFactoryBindings& factory_bindings();
    ModelRenderHooks& model_hooks();
    ModelRenderLifecycle& lifecycle();
    WorldCallbacks& updates();
    void bind_initializers(client::NativeStartupSources&);

private:
    struct Data;
    std::unique_ptr<Data> data_;
};
} //namespace skeet::effects

// Declarations from scope_owner.hpp

namespace skeet::effects {
struct ScopeOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    engine::OwnerRegistryBinding hud; //43475A9C
    engine::OwnerRegistryBinding updates; //43475988
};
struct ScopeOwnerBindings {
    ViewRuntime& runtime;
    esp::EffectsSettings& settings; //same canonical object referenced by runtime
};
std::size_t scope_owner_size() noexcept;
// Whole ctor43486002..43486055 and primary434727D8. Storage is the real
// aligned43387448 allocation; primary0 releases its header, primary7 discovers
// HudZoom. The HUD and update endpoints share one runtime.scope_panel.
void construct_scope_owner(void* storage, const ScopeOwnerRegistries&, ConfigRegistries&,
                           ScopeOwnerBindings);
} // namespace skeet::effects

// Declarations from smoke_owner.hpp

namespace skeet::effects {
struct SmokeOwnerRegistries {
    engine::OwnerRegistryBinding lifecycle; //43475B78
    engine::OwnerRegistryBinding updates; //43475988
};
struct SmokeOwnerBindings {
    WorldRuntime& runtime;
    const SmokeRuntimeBindings& engine;
    WorldRuntime*& active; //canonical4347585C slot
};
std::size_t smoke_owner_size() noexcept;
// Whole ctor43480340..4348037E. One canonical623BD4F2 descriptor; primary
// 43470980 and update4347005C are published after both actual registrations.
void construct_smoke_owner(void* storage, const SmokeOwnerRegistries&, ConfigRegistries&,
                           SmokeOwnerBindings);
} // namespace skeet::effects

// Declarations from view.hpp

namespace skeet {
struct MovementCommandOwner;
}
namespace skeet::effects {
struct ViewOffsets {
    std::uint32_t life_state; // 0x9D920E35
    std::uint32_t scoped; // 0xCACAEECC
    std::uint32_t active_weapon; // 0xCD812654
    std::uint32_t fov; // 0xFBE8C5EA
    std::uint32_t scope_transition_35b5b2ec;
    std::uint32_t shake_angles_5ae32976;
    std::uint32_t recoil_angles_1bae1168;
};
struct ViewBindings {
private:
    void* standalone_entities_ = nullptr;
    void* standalone_panels_ = nullptr;
    void* standalone_input_ = nullptr;
    void* standalone_prediction_ = nullptr;
    void* standalone_recoil_ = nullptr;
    void* standalone_surface_ = nullptr;

public:
    ViewBindings() noexcept
        : entity_list(standalone_entities_), hud_panels(standalone_panels_),
          input(standalone_input_), prediction(standalone_prediction_),
          recoil_scale(standalone_recoil_), panel_surface(standalone_surface_) {}
    ViewBindings(void* entities, void* panels, void* input_object, void* prediction_object,
                 void* recoil, const std::uint8_t* disabled, const std::uint8_t* calculating,
                 void* surface = nullptr) noexcept
        : ViewBindings() {
        entity_list = entities;
        hud_panels = panels;
        input = input_object;
        prediction = prediction_object;
        standalone_recoil_ = recoil;
        suspended = disabled;
        calculating_view = calculating;
        panel_surface = surface;
    }
    explicit ViewBindings(engine::NativeDataImage&);
    ViewBindings(const ViewBindings&) = delete;
    ViewBindings& operator=(const ViewBindings&) = delete;
    void*& entity_list; //live4346A890
    void*& hud_panels; //live4346A44C
    void*& input; //live4346A894
    void*& prediction; //live43469264; byte+8
    engine::NativeConVarReference recoil_scale; //live4346A534 XOR B739EB42
    const std::uint8_t* suspended = nullptr; // 0x43468E58
    const std::uint8_t* calculating_view = nullptr; // 0x43475BD1
    void*& panel_surface; //live4346712C; GetPanel(2) through slot1
};
struct SavedRecoilAngles {
    void* shake = nullptr;
    void* all = nullptr;
    std::array<std::uint32_t, 3> previous_shake{}, previous_all{};
};
struct SavedViewHeight {
    void* address = nullptr;
    std::uint32_t value = 0;
};
class ViewDuckTarget {
public:
    ViewDuckTarget(const float* target) noexcept : target_(target), owner_(nullptr) {}
    explicit ViewDuckTarget(MovementCommandOwner* const& owner) noexcept
        : target_(nullptr), owner_(&owner) {}
    const float* get() const noexcept;

private:
    const float* target_;
    MovementCommandOwner* const* owner_;
};
struct ViewDuckBindings {
    ViewDuckTarget duck_target; // live movement owner434759B0, null if absent
    const std::uint8_t& enabled; //43467CEC
    const std::uint32_t& command_flags; //434759C0
    const HotkeyBinding& duck_key; //43467C8C, activation mode43467CA0
    InputBitmap input;
    float& previous_duck; //4346D654
    std::uint32_t duck_amount, view_offset; //3309C0CF /C246315E
};

class ViewRuntime {
public:
    ViewRuntime(const esp::EffectsSettings& settings, ViewBindings& interfaces, ViewOffsets offsets)
        : settings(settings), interfaces(interfaces), offsets(offsets) {}
    const esp::EffectsSettings& settings;
    ViewBindings& interfaces;
    ViewOffsets offsets;
    std::int32_t scope_panel = -1; // scope owner's +0x28

    void initialize_scope_panel(); // complete 0x433970BF, including 0x43368A74 tree search
    void hide_scope_panel(void* player); // complete 0x4337F0C1
    // Complete 0x433E273D with the renderer's screen-size query supplied by
    // the frame's viewport. Its two rectangles use the recovered queue writer.
    void draw_scope(ui::RenderQueue&, Point viewport, void* player);
    // Production HUD entry queries the supplied queue after weapon455,
    // preserving the original queue's late virtual18 context association.
    void draw_scope_live(ui::RenderQueue&, void* player);
    void instant_scope(void* player); // 0x433FA01F..0x433FA062, after local/suspended gates

    // Recoil transaction inside calculate_view_hook:433C82F7..433C842A and
    // restoration433C84C3..433C84DD. FOV has its own slot6 view callback.
    SavedRecoilAngles begin_recoil_angles(void* player);
    void restore_recoil_angles(const SavedRecoilAngles&);
    SavedViewHeight begin_view_height(void* player, ViewDuckBindings&); // complete43384DE0
    void restore_view_height(const SavedViewHeight&);
    // Complete433C82DC, with the actual registered secondary interfaces.
    void calculate_view_hook(void* player, void** original_table, ViewDuckBindings&,
                             const engine::OwnerRegistry&, std::uintptr_t first,
                             std::uintptr_t second, std::uintptr_t third);
    // Live player366 entry. Metadata is resolved after observer/duck dispatch,
    // at native433C8480; those real callbacks may replace the current table.
    void calculate_hooked_view(void* player, ViewDuckBindings&, const engine::OwnerRegistry&,
                               std::uintptr_t first, std::uintptr_t second, std::uintptr_t third);

    // Complete 0x433B0541. original_table is the engine's saved vtable;
    // slot 250 receives the three opaque original arguments unchanged.
    void recoil_scale_hook(void* player, void** original_table, std::uintptr_t first,
                           std::uintptr_t second, std::uintptr_t third);
    void recoil_scale_hooked(void* player, const std::uint32_t& encoded_recoil_scale,
                             std::uintptr_t first, std::uintptr_t second, std::uintptr_t third);

private:
    bool scope_visible(void* player);
    template <bool hooked>
    void calculate_view_transaction(void*, void**, ViewDuckBindings&, const engine::OwnerRegistry&,
                                    std::uintptr_t, std::uintptr_t, std::uintptr_t);
    template <bool hooked>
    void recoil_scale_transaction(void*, void**, const std::uint32_t&, std::uintptr_t,
                                  std::uintptr_t, std::uintptr_t);
};
} // namespace skeet::effects

// Declarations from view_dispatch.hpp

namespace skeet::effects {
// Actual secondary-interface owners. These endpoints bind concrete recovered
// methods; their empty slots are the native433CCE8E/4337D653/4334A269 entries.
struct ScopeViewObserver {
    network::UpdateObserver endpoint;
    ViewRuntime* runtime;
    explicit ScopeViewObserver(ViewRuntime&);
    void initialize_vtable() noexcept;
    ScopeViewObserver(engine::OwnerRegistryBinding, ViewRuntime&);
};
struct FovViewObserver {
    network::UpdateObserver endpoint;
    misc::ZoomSensitivityRuntime* sensitivity;
    ViewFovRuntime* runtime;
    // Unregistered constructor; the full4347DDCD owner assigns both vtables
    // only after its lifecycle/update registrations.
    FovViewObserver(misc::ZoomSensitivityRuntime&, ViewFovRuntime&);
    void initialize_vtable() noexcept;
    FovViewObserver(engine::OwnerRegistryBinding, misc::ZoomSensitivityRuntime&, ViewFovRuntime&);
};
// Slot5 loop from433C843E..8471; the end pointer is captured before calls.
void dispatch_player_view(const engine::OwnerRegistry&, void* player, std::uintptr_t first,
                          std::uintptr_t second, std::uintptr_t third);
// Complete433B91CB, including its false AL result. Origin, angles and FOV are
// live mutable engine arguments; every endpoint comes from the native owner graph.
bool dispatch_view(const engine::OwnerRegistry&, float* origin, float* angles, float* fov);
} // namespace skeet::effects

// Declarations from view_fov.hpp

namespace skeet::effects {
struct ViewFovOffsets {
    std::uint32_t life_state, movement_type, observer_mode, observer_target, fov;
};
// Canonical owner4347DDCD, secondary slot6. Its origin/angles parameters are
// unused; effective observer selection and actual player slot333 stay here.
class ViewFovRuntime {
public:
    const misc::FovConfig& settings;
    void* const& entity_list; //live4346A890
    void* const* local_player;
    ViewFovOffsets offsets;
    void update(float& field_of_view); // complete4333FD39
};
} // namespace skeet::effects

// Declarations from visual_receive_owner.hpp

namespace skeet::client {
class ClientRuntimeStorage;
class ConfigurationRuntimeOwner;
class PlayerRuntimeOwner;
}
namespace skeet::effects {
// Source lifetime for the five camera/world receive callbacks. Construction
// borrows canonical owners only; explicit initialization belongs to the later
// receive-patch startup cut, after the real feature owners/classes exist.
class VisualReceiveOwner {
public:
    VisualReceiveOwner(client::ClientRuntimeStorage&, client::ConfigurationRuntimeOwner&,
                       client::PlayerRuntimeOwner&);
    VisualReceiveOwner(const VisualReceiveOwner&) = delete;
    VisualReceiveOwner& operator=(const VisualReceiveOwner&) = delete;
    VisualReceiveOwner(VisualReceiveOwner&&) = delete;
    VisualReceiveOwner& operator=(VisualReceiveOwner&&) = delete;
    client::ClientRuntimeStorage& storage() const noexcept {
        return storage_;
    }
    VisualReceiveRuntime& runtime() noexcept {
        return runtime_;
    }

private:
    client::ClientRuntimeStorage& storage_;
    VisualReceiveRuntime runtime_;
};
} //namespace skeet::effects

// Declarations from world.hpp

namespace skeet::engine {
class NativeDataImage;
}
namespace skeet::effects {

// 0x4334ED58. Bits select wall alpha, fullbright's $phong check,
// translucent fullbright, and night color, respectively.
std::uint8_t classify_world_material(void* material, std::uint32_t texture_group_hash, bool dust2);

struct WorldMaterialChange {
    bool dust2 = false;
    bool refresh = false;
    bool alpha = false;
    float wall_alpha = 1.0f;
    bool fullbright = false;
    bool night = false;
    std::array<float, 3> color{};
};
// Complete 0x433D63B7, including excluded groups and the nine owned materials.
void apply_world_material(void* material, const WorldMaterialChange& change,
                          const std::array<void*, 9>& excluded_materials);

struct WorldInterfaces {
private:
    void* standalone_engine_ = nullptr;
    void* standalone_material_ = nullptr;
    void* standalone_static_props_ = nullptr;
    void* standalone_entities_ = nullptr;
    const std::int32_t* standalone_maximum_ = nullptr;
    void* standalone_class_ = nullptr;
    void* standalone_smoke_ = nullptr;

public:
    WorldInterfaces() noexcept
        : engine(standalone_engine_), material_system(standalone_material_),
          static_props(standalone_static_props_), entity_list(standalone_entities_),
          max_clients(standalone_maximum_), tonemap_client_class(standalone_class_),
          smoke_registry(standalone_smoke_) {}
    explicit WorldInterfaces(engine::NativeDataImage&);
    WorldInterfaces(const WorldInterfaces&) = delete;
    WorldInterfaces& operator=(const WorldInterfaces&) = delete;
    void*& engine; //live4346A514
    void*& material_system; //live4346A4E0
    void*& static_props; //live43467014
    void*& entity_list; //live4346A890; highest index at native byte+24
    const std::int32_t*& max_clients; //live43469300
    // Native owner+730 captures CSun, not a CEnvTonemapController class.
    void*& tonemap_client_class; //canonical CSun434684C4
    void*& smoke_registry; //live434679C0, count at+0C
};

struct WorldNetvars {
    std::uint32_t tonemap_disabled; // resolver 0x37482375
    std::uint32_t fog_controller; // resolver 0xDC91E6CF
    std::uint32_t fog_enabled; // resolver 0x60816092
};
struct SmokeRuntimeBindings {
    void* const& engine; //live4346A514
    void* const& entity_list; //live4346A890,read after engine12
    void* const& smoke_registry; //live434679C0,count+0C
};

// The captured engine uses a parent-linked ConVar and pointer-xored integer
// storage at +0x30/+0x40. No command-string substitutes are used.
struct WorldConvarOverride {
    void* variable = nullptr;
    std::uint32_t saved = 0;
    bool active = false;
    void set(std::uint32_t value); // 0x433CEF55
    void restore(); // 0x433AB872
};

class WorldRuntime {
public:
    WorldRuntime(esp::EffectsSettings& settings, WorldInterfaces& interfaces, WorldNetvars offsets,
                 const std::uint8_t& suspended)
        : settings(settings), interfaces(interfaces), offsets(offsets), suspended(suspended) {}
    esp::EffectsSettings& settings;
    WorldInterfaces& interfaces;
    WorldNetvars offsets;
    const std::uint8_t& suspended; // native 0x43468E58
    std::array<void*, 9> excluded_materials{};
    std::uint8_t material_flags = 0; // owner +0x452
    float previous_wall_alpha = 1.0f; // +0x6D4
    std::int32_t previous_prop_transparency = 100; // +0x6D8
    std::int32_t previous_prop_transparency_2 = 100; // +0x6DC
    std::uint32_t previous_night_rgb = 0; // +0x6E0
    bool fog_overridden() const noexcept {
        return (material_flags & 8) != 0;
    }
    std::uint8_t saved_fog_enabled = 0;
    WorldConvarOverride post_processing; // owner +0x6E8
    WorldConvarOverride
        cascaded_shadows; //constructor+6F4, cl_csm_shadows; no current render override
    std::array<WorldConvarOverride, 4> sky; // +0x700/+0x70C/+0x718/+0x724

    void update(); // complete 0x433B9D00
    void
    begin_render(); // world-effect range 0x433FCE36..0x433FCFC1; camera belongs to CameraRuntime
    void begin_render_for_player(void* local); //same effect block after the caller's local lookup
    void end_render(); // the five original 0x433AB872 restores
    void smoke_before_render(); // complete 0x433C08F4
    void smoke_after_render(); // complete 0x433E6305
    void smoke_before_render(const SmokeRuntimeBindings&);
    void smoke_after_render(const SmokeRuntimeBindings&);
};

// Complete 0x43359CA0. The supplied table is the engine's saved original
// vtable, whose slot 13 receives the unchanged argument when not suppressed.
bool filter_flash_effect(const esp::EffectsSettings& settings, void* receiver,
                         void** original_vtable, void* effect_argument);
void bind_flash_effect_runtime(const esp::EffectsSettings&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
bool __fastcall flash_effect_hook(void* receiver, void* ignored_edx, void* effect_argument);
#else
bool flash_effect_hook(void* receiver, void* ignored_edx, void* effect_argument);
#endif

} // namespace skeet::effects

// Declarations from world_callbacks.hpp

namespace skeet::effects {
struct WorldCallbackState {
    std::uint8_t second_hook_installed = 0; // fullowner+0x34
    std::uint8_t camera_pending = 0; // fullowner+0x451
    void* saved_sky_name = nullptr; // fullowner+0x764; allocation+0x760 is shared below
    std::string sky_name{"sky_csgo_night02"}; //fullowner+0x750; constructor4348EFEB..4348F03C
};
struct WorldCallbackOffsets {
    ShotEntityBoneOffsets bones;
    std::uint32_t render_flags; //38CA5228
    std::uint32_t inventory; //FC3297F7,48 handles
    std::uint32_t weapon_world_model; //CA2B5FFB
};
struct WorldCallbackBindings {
    ModelHookDescriptor& first_hook;
    ModelHookDescriptor& second_hook;
    const std::uint8_t& install_second_hook; //43467AEC
    engine::NativeConVarReference sky_name_variable; //live4346A39C XOR738FE7C5, string+24
    engine::NativeConVarReference clear_convar; //live43467080 XOR62444FD4
    void* const& allocator; //liveIMemAlloc43467204
    void* const& model_manager; //live4346A3A0,slot3(modelhandle,1)
    engine::NativeConVarReference camera_snap_to; //livecam_snapto43468DFC XOR18D4EC2E,slot16(bool)
    CameraInterfaces& camera;
    CameraNetvars camera_offsets;
    std::uint32_t& shot_entity_handle; //shared4346D674
    const std::uint32_t* const& bone_counter; //live43469060
    const ModelShotState& shot;
    engine::MemberFunction<void> invalidate_render_flags; //optional actual43468320
    WorldCallbackOffsets offsets;
    InputBitmap input;
    const LocalTransparencyInterfaces& transparency_interfaces;
    const LocalTransparencyNetvars& transparency_offsets;
    const std::uint8_t& player_effects; //4346A65C
    const float& frame_time; //globals+0x14
};
// Whole secondarycallbacks at vtable43472768. All effects/model settings are
// the existing canonical objects. Sky allocation is shared with primaryshutdown.
class WorldCallbacks {
public:
    WorldRuntime& world;
    ViewRuntime& view;
    ModelRenderRuntime& models;
    ModelRenderLifecycleState& lifecycle;
    WorldCallbackState& state;
    WorldCallbackBindings& bindings;
    void begin(); //complete433FCDDD
    void end(); //complete43418089
    void clear(); //complete4338429F
    void prepare(); //complete433F9D3D
};
struct WorldUpdateObserver {
    network::UpdateObserver endpoint;
    WorldCallbacks* runtime;
    explicit WorldUpdateObserver(WorldCallbacks&);
    void initialize_vtable() noexcept;
    WorldUpdateObserver(engine::OwnerRegistryBinding, WorldCallbacks&);
};
} // namespace skeet::effects
