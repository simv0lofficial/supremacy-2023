#pragma once
#include "skeet/skins/config.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/engine/netvars.hpp"
#include "skeet/engine/offset_storage.hpp"
#include "skeet/misc/native_text.hpp"
#include <array>
#include <cstddef>

namespace skeet::skins {
// All fields come from the recovered encrypted offset globals. There are no
// SDK-layout defaults. The decoder below requires actual initialized storage.
struct NativeCosmeticOffsets {
    std::ptrdiff_t team, dormant, life_state, wearable, paintkit, definition, model_index;
    std::ptrdiff_t transmission_state, wearable_counter, entity_flags, quality;
    std::ptrdiff_t model_update_handle, owner_handle, active_weapon, view_model_index;
    std::ptrdiff_t initialized, item_id_high, item_id_low, account_id, wear;
    std::ptrdiff_t attributes, attribute_cache, view_models, observer_mode, observer_target,
        observer_excluded;
    std::ptrdiff_t model_header, parent_handle, local_origin, local_angles;
    std::ptrdiff_t mask_byte_43468bd4, mask_bits_43467044, mask_byte_43467964, mask_flags_43469224;
};
NativeCosmeticOffsets native_cosmetic_offsets(const engine::NativeOffsetGlobals&);
struct NativeCosmeticGlobals {
    void*& item_system; //cached43476424
    void*& local_override; //dereferenced43468C94
    engine::ClientClass* const& glove_class; //4346841C, copied by glove constructor
};
struct NativeCosmeticBindings {
    using AcquireItemSystem = void*(__cdecl*)(); //resolved43468B9C
    using RegisterEntity = engine::MemberFunction<void, std::uint32_t>; //4346A884
    using UnregisterEntity = engine::MemberFunction<void, std::uint32_t*, void*>; //43468E88
    using SetParent = engine::MemberFunction<void, void*, std::int32_t>; //4346D644
    using InvalidateTransform = engine::MemberFunction<void, std::int32_t>; //43468C10
    using InvalidateBones = engine::MemberFunction<void>; //434688A4
    void* base_client = nullptr;
    void* engine_client = nullptr;
    void* entity_list = nullptr;
    void* model_info = nullptr;
    void* model_cache = nullptr;
    void* string_tables = nullptr;
    AcquireItemSystem acquire_item_system = nullptr;
    RegisterEntity register_entity = nullptr;
    UnregisterEntity unregister_entity = nullptr;
    SetParent set_parent = nullptr;
    InvalidateTransform invalidate_transform = nullptr;
    InvalidateBones invalidate_bones = nullptr;
};
struct NativeMaskResources {
    misc::NativeText attachment, directory;
    std::array<misc::NativeText, 22> filenames;
};
// Concrete raw engine owners for the two real BA8 slot1 bodies. These execute
// the lifecycle directly against engine fields/interfaces; they neither build
// semantic entity copies nor delegate an internal feature to a callback.
class NativeEntityCosmetics final {
public:
    NativeEntityCosmetics(const Settings&, NativeCosmeticOffsets, NativeCosmeticGlobals,
                          NativeCosmeticBindings);
    // These are called at the actual primary-owner constructor boundaries,
    // after registration/configuration and before the corresponding updates.
    void initialize_glove_owner() noexcept; //434907F5..43490807
    void initialize_mask_owner(NativeMaskResources&) noexcept; //4347EC33..3A
    void resolve_mask_owner_class(); //4347ED32..3C, after empty text construction
    void release_glove_owner(void* player); //4331EB65, caller has applied the connected gate
    void release_mask_owner(); //43386F30
    void update_gloves(void* player); //complete43342310; incoming stack argument unused
    void update_mask(void* player); //complete43408846; incoming stack argument unused
    std::uint32_t glove_handle() const noexcept {
        return glove_handle_;
    }
    std::uint32_t mask_handle() const noexcept {
        return mask_handle_;
    }
    const Settings& settings() const noexcept {
        return settings_;
    }

private:
    void* resolve(std::uint32_t);
    std::uint32_t handle(void*);
    void* create(engine::ClientClass*, const char*); //43364942
    void destroy(void*, bool recreating);
    void* item_definition(std::uint32_t);
    std::int32_t model_index(const char*); //43331B0C/433AE8B7
    void set_glove(void*, std::int32_t, std::uint32_t, std::int32_t); //433E320A
    void attribute(void*, std::int32_t, float); //433E2266
    bool glove_matches(void*, std::int32_t, std::uint32_t, std::int32_t);
    void detach(void*, void*);
    void* view_model(void*, std::int32_t); //43342DD7
    void teardown_glove(void*);
    void teardown_mask();
    void zero_transform(void*, std::ptrdiff_t, std::int32_t);
    const Settings& settings_;
    NativeCosmeticOffsets fields_;
    NativeCosmeticGlobals globals_;
    NativeCosmeticBindings bindings_;
    std::uint32_t glove_handle_ = 0xFFFFFFFF, mask_handle_ = 0xFFFFFFFF;
    engine::ClientClass* glove_factory_ = nullptr;
    engine::ClientClass* mask_factory_ = nullptr;
    NativeMaskResources* mask_resources_ = nullptr;
};
} //namespace skeet::skins
