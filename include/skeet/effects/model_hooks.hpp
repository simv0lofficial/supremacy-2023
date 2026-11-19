#pragma once
#include "model_render_lifecycle.hpp"
#include "skeet/engine/startup_convars.hpp"
#include <span>

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
    const std::int32_t* const& current_tick; //live43475B2C, last outgoing command
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
