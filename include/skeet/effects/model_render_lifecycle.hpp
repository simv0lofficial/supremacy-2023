#pragma once
#include "model_render.hpp"
#include "world.hpp"
#include "camera.hpp"

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
