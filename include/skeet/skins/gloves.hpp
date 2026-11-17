#pragma once
#include "skeet/skins/config.hpp"
#include "skeet/skins/model_resources.hpp"
#include "skeet/skins/native_catalog.hpp"
#include <span>

namespace skeet::skins {
inline constexpr std::uint32_t invalid_glove_handle = 0xFFFFFFFF;
struct GloveEntity;
class GloveFactory;
class GloveUnknown {
public:
    virtual ~GloveUnknown() = default;
    virtual GloveEntity* base_entity() = 0; // native unknown +0x1C
};
class GloveNetworkable {
public:
    virtual ~GloveNetworkable() = default;
    virtual GloveUnknown* unknown() = 0; // +0
    virtual void release() = 0; // +4
    virtual GloveFactory* client_class() = 0; // +8
    virtual void notify_transmission(std::int32_t) = 0; // +0xC
    virtual void on_data_changed(std::int32_t) = 0; // +0x14
    virtual void mark_destroyed_on_recreate() = 0; // +0x34
};
class GloveFactory {
public:
    virtual ~GloveFactory() = default;
    // The actual ClientClass create function, not a reconstructed lifecycle callback.
    virtual GloveNetworkable* create(std::int32_t index, std::int32_t serial) = 0;
};
struct GloveAttribute {
    std::uint32_t definition = 0;
    float value = 0, initial_value = 0;
};
// These fields are engine-owned, resolved storage. Their encrypted native field
// identities are retained in gloves.cpp; no Source SDK byte layout is assumed.
struct GloveEntity {
    std::uint32_t handle = invalid_glove_handle, flags = 0, account_id = 0;
    std::int32_t item_id_high = 0, item_id_low = 0, paintkit = 0, quality = 0;
    float wear = 0;
    std::uint16_t definition = 0;
    std::int16_t model_index = -1;
    std::uint32_t owner_handle = invalid_glove_handle;
    std::int32_t creation_index = 0;
    bool initialized = false;
    std::span<GloveAttribute> attributes;
    void* attribute_cache = nullptr;
    GloveNetworkable* networkable = nullptr;

    // Player / held-weapon fields used by the same native traversal.
    std::int32_t team = 0, observer_mode = 0, transmission_state = 0, view_model_index = 0;
    bool dormant = false, dead = false, observer_excluded = false;
    std::uint32_t wearable_handle = invalid_glove_handle,
                  active_weapon_handle = invalid_glove_handle;
    std::uint32_t model_update_handle = invalid_glove_handle; // native field 0x43467FCC
    std::uint32_t observer_target = invalid_glove_handle;
    std::span<const std::uint32_t> view_models;
    std::uint32_t wearable_change_counter = 0;
};
struct GloveOwnerState {
    std::uint32_t handle = invalid_glove_handle; // native owner +0x7C
    GloveFactory* factory = nullptr; // native owner +0x80
};
class GloveEngine {
public:
    virtual ~GloveEngine() = default;
    virtual bool demo_playback() = 0; // engine +0x148
    virtual bool hltv() = 0; // engine +0x174
    virtual GloveEntity* resolve(std::uint32_t handle) = 0; // entity list +0x10
    virtual bool entity_registration_available() = 0; // both 0x4346A884 / 0x43468E88 resolved
    virtual std::int32_t& highest_entity_index() = 0; // native entity list +0x24
    virtual void register_entity(std::uint32_t index) = 0; // 0x4346A884
    virtual void unregister_entity(std::uint32_t& handle, GloveEntity&) = 0; // 0x43468E88
    virtual GloveEntity*& local_override() = 0; // dereferenced 0x43468C94
    virtual std::uint32_t local_account_id() = 0; // engine +0x30 / +0x20 player info
    virtual void begin_model_cache() = 0; // model cache +0x84
    virtual void end_model_cache() = 0; // model cache +0x88
    virtual bool initialize_model(GloveEntity&, const char* path, bool) = 0; // entity +0x184
    virtual void set_model_index(GloveEntity&, std::int32_t) = 0; // entity +0x12C
    virtual std::uint32_t model_instance(GloveEntity&) = 0; // renderable +0x98
    virtual void set_model_instance(GloveEntity&, std::uint32_t) = 0; // entity +0x488
    virtual void attach(GloveEntity&, GloveEntity& player) = 0; // entity +0x468
    virtual void equip(GloveEntity&, GloveEntity& player, bool) = 0; // entity +0x454
    virtual void unequip(GloveEntity&, GloveEntity& player) = 0; // entity +0x478
    virtual void post_equip(GloveEntity&) = 0; // entity +0x80
    virtual std::int32_t health(GloveEntity&) = 0; // entity +0x1E8
    virtual bool network_dormant(GloveEntity&) = 0; // networkable +0x24
    virtual void invalidate_bone_cache(GloveEntity&) = 0; // optional resolved 0x434688A4
    virtual void invalidate_physics(GloveEntity&, std::int32_t) = 0; // entity +0x3F8
    virtual void release_attribute_cache(void* cache, bool) = 0; // cache +0
};
// Complete 0x43342310. Internal helpers 0x43364942, 0x433E320A,
// 0x433E2266 and 0x43342DD7 are implemented here, not engine callbacks.
void apply_gloves(const Settings&, GloveOwnerState&, GloveEntity* local, GloveEngine&,
                  SchemaEngine&, ModelInfo&);
} // namespace skeet::skins
