#pragma once
#include "model_render_lifecycle.hpp"
#include "view.hpp"
#include "skeet/network/update_observers.hpp"
#include <string>

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
