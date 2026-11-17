#pragma once
#include "bone_runtime.hpp"
#include "fix_animation_entity.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/client/player_data_update.hpp"
#include "skeet/engine/hooks.hpp"
#include "skeet/network/data_update_observers.hpp"
#include "skeet/network/update_observers.hpp"
#include <array>

namespace skeet::effects {
struct PlayerOpacityBindings;
}
namespace skeet::animation {
struct FixAnimationFields {
    std::ptrdiff_t life_state_9d920e35;
    std::ptrdiff_t simulation_time_d5fd2e2c;
    std::ptrdiff_t previous_time_1599541f;
    std::ptrdiff_t replicated_time_d3d4230d;
    std::ptrdiff_t body_yaw_a2a8247f;
    std::ptrdiff_t client_animation_8481ea04;
    std::ptrdiff_t suppressed_cde290d2;
    std::ptrdiff_t move_type_7501cb91;
    std::ptrdiff_t observer_mode_2f4e97c4;
    std::ptrdiff_t observer_target_3b704b41;
    std::ptrdiff_t view_offset_c246315e;
    std::ptrdiff_t animation_frame_74019d87;
    std::ptrdiff_t transient_6e9852d6;
    std::ptrdiff_t eye_angles_c1b66ee8;
    std::ptrdiff_t resource_alive_d266b537;
};
// The actual4346D6E4 bucket chains consumed by433C7FC5. The final bucket is
// the sentinel. This is borrowed native/source-owned storage, not a new cache.
struct FixAnimationBackupNode {
    std::uint32_t entity_index;
    std::int32_t acknowledged_command, frame;
    std::array<std::byte, 0x348> animation;
    RageVector angles;
    std::array<float, 24> poses;
    float collision_time;
    FixAnimationBackupNode* next;
};
#if UINTPTR_MAX == UINT32_MAX
static_assert(offsetof(FixAnimationBackupNode, animation) == 0xC);
static_assert(offsetof(FixAnimationBackupNode, angles) == 0x354);
static_assert(offsetof(FixAnimationBackupNode, poses) == 0x360);
static_assert(offsetof(FixAnimationBackupNode, next) == 0x3C4);
#endif
struct FixAnimationGlobals {
    const std::uint8_t& enabled; //SettingsMenuValues.fix_animations,4346A527
    const std::uint8_t& command_mode; //43467224
    const std::uint8_t& alternate_command_mode; //4346A740
    const std::uint8_t& clear_animation_state; //43467CEC
    const std::uint8_t& invalidate_bones; //43468A10
    const std::uint32_t& command_flags; //434759C0
    const std::int32_t& animation_frame; //*434678F8
    const std::int32_t& prediction_frame; //43467B04+4
    const std::int32_t& clock_window; //43467B04+38
    const std::int32_t& clock_offset; //43467B04+3C
    const float& interval; //*4346A338
    const float& receive_interval; //43467B04+20
    float& current_time; //*43467EB8
    const std::int32_t& last_outgoing; //*43475B2C
    client::EntityCommandOwner* const& command_history_owner; //43475898
    void* const& local_player; //43475808
    const RageVector& setup_angles; //43476414
    const RageVector& view_angles; //4347BD30
    std::uint8_t& thinking; //43475BD1
    double* const& engine_timestamp; //optional4346A7E4
    double (*timestamp)(); //genuine engine entry43468C84
    FixAnimationBackupNode** const& backup_buckets; //4346D6E4
    const std::uint32_t& backup_bucket_count; //4346D6E8
    std::uint32_t& backup_count; //4346D6EC
    const std::int32_t& backup_mode; //4347C9EC
};
#if defined(_M_IX86) || defined(__i386__)
using AnimationLayerProxy = void(__cdecl*)(const void*, void*, void*);
#else
using AnimationLayerProxy = void (*)(const void*, void*, void*);
#endif
struct FixAnimationsBindings {
    const BoneRuntime& bones;
    FixAnimationEntityView entity;
    client::PlayerStateCache& players;
    FixAnimationFields fields;
    FixAnimationGlobals globals;
    void* input; //4346A894, actual third-person virtual32
    const void* interpolation_cvar; //4346A850 xor478720C2
    void* const* resource; //optional43468FF0, then its live first pointer
    std::byte* player_settings; //4347BE00, stride20, id at0/settings at8
    network::DataUpdateObserverRegistry& data_update_observers; //43475760/64/68
    const client::PlayerDataUpdateOffsets& data_update_fields;
    engine::OwnerRegistry& update_observers; //43475988/8C/90
    engine::MemberFunction<void, std::int32_t> invalidate_physics; //optional43468C10
    void* allocator; //43467204, Free virtual5
    engine::JobMemory memory;
    AnimationLayerProxy& original_layer_proxy; //saved engine proxy43476974
    const effects::PlayerOpacityBindings& opacity;
};

enum class FixAnimationHook {
    player,
    renderable,
    networkable_history,
    networkable_view,
    thinkable_observers,
    thinkable_opacity
};
// Hook bodies read the canonical enabled byte and the actual receiver on each
// invocation. Original methods always come from installed vtable[-2].
class FixAnimationsRuntime {
public:
    explicit FixAnimationsRuntime(const FixAnimationsBindings& bindings) noexcept
        : bindings(bindings) {}
    const FixAnimationsBindings& bindings;
    void build_transformations(void*, std::array<std::uintptr_t, 6>); //43330F73,slot190
    void standard_blending_rules(void*, void*, RageVector*, BoneQuaternion*,
                                 float); //4335C639,slot207
    void extra_bone_processing(void*, std::array<std::uintptr_t, 6>); //43388201,slot198
    void calculate_view(void*, RageVector*, RageVector*, float*, float*, float*); //4335B798,slot277
    bool setup_bones(void*, Matrix3x4*, std::int32_t, std::uint32_t, float); //433E80F7,slot13
    void networkable_history(void*, std::int32_t); //4337DE0B,slot7
    void networkable_pre_update(void*, std::int32_t); //433E129A ->433B7613,slot6
    void networkable_view(void*, std::int32_t); //433B7093,slot7
    void thinkable_observers(void*); //433FD3BF,slot1
    void thinkable_opacity(void*); //43383CA2,slot1
    void receive_layer(const void*, void*, void*); //433CE396
    void restore_animation_backup(void* player); //433C7FC5

    // The existing initialization scheduler supplies each actual prepared
    // descriptor and receiver. Source entries replace only the recovered slots;
    // installation and whole-unload restoration use the canonical hook list.
    void prepare(FixAnimationHook, engine::VtableHookMetadata&) noexcept;
    engine::VtableHookNode* install(FixAnimationHook, engine::VtableHookRegistry&, void* receiver,
                                    engine::VtableHookMetadata&);
    // Exact captured operand binding for the existing protected scheduler.
    // Invoke after its raw replacement loop and before4331E834. The native
    // entry identity selects the correct networkable/thinkable variant; no
    // class/local-player guess or replacement of unrelated entries is needed.
    void bind_native_replacement(engine::VtableHookMetadata&, std::size_t slot,
                                 std::uint32_t native_entry);
    static void* native_replacement_entry(std::size_t slot, std::uint32_t native_entry);
    void uninstall(engine::VtableHookRegistry&, void* receiver);
    void install_layer_proxy(void** actual_proxy_slot);
    void restore_layer_proxy();

private:
    bool animation_owner_changed(void* player) const; //complete4332FB3C
    void** proxy_slot_ = nullptr;
    void* saved_proxy_ = nullptr;
};
} // namespace skeet::animation
