#pragma once
#include "skeet/skins/inventory.hpp"
#include "skeet/skins/models.hpp"
#include <array>
#include <cstddef>
#include <vector>

namespace skeet::skins {
struct SequenceDescription {
    // Engine studio sequence names are NUL terminated; source names fit 63 bytes.
    std::string_view name;
    std::int32_t activity = -1;
};
struct StudioSequences {
    // For a virtual studio model the adapter resolves the engine virtual-model
    // sequence table, as 0x433CCB8F / 0x433E2727 do for studiohdr_t +0x150.
    std::span<const SequenceDescription> descriptions;
};
struct KnifeSequenceModels {
    std::int32_t view_model = -1, world_model = -1, entity_model = -1; // owner row +8C/+90/+94
};
struct SequenceEntity {
    std::uint32_t class_id = 0;
    PlayerIdentity original_owner;
    std::uint32_t weapon_handle = 0xFFFFFFFF, owner_handle = 0xFFFFFFFF;
    std::int16_t model_index = -1;
    std::uint16_t definition = 0;
    std::int32_t entity_model = -1, view_model = -1, world_model = -1, model_kind = 0;
    std::int32_t sequence = -1;
    // Resolved fields 0x43468DA0 and 0x43467B4C; their history comparison is exact.
    std::int32_t animation_token_08 = 0, animation_token_0c = 0;
    float sequence_time = 0; // resolved field 0x43467A50
    std::int32_t team = 2;
    bool dead = false, weighted_activity = false; // 0x43468E78 / 0x43467EC4
};
struct SequenceHistory {
    std::uint32_t command = 0, random_seed = 0, weapon_handle = 0;
    std::int32_t sequence = 0, animation_token_08 = 0, animation_token_0c = 0;
    std::uint32_t field_18 = 0;
    std::uint8_t player_byte_1c = 0, padding_1d = 0;
    std::int16_t selected_model = 0;
};
static_assert(sizeof(SequenceHistory) == 0x20);
static_assert(offsetof(SequenceHistory, field_18) == 0x18);
static_assert(offsetof(SequenceHistory, selected_model) == 0x1E);
struct PendingViewModel {
    std::uint32_t handle;
    std::int16_t model = -1, sequence = -1;
};
struct PendingWeaponHandle {
    std::uint32_t handle, value;
};
struct SequenceRuntime {
    void* inventory_resolver = nullptr; //4348DE49 captured4346A890, owner+5C
    std::uint32_t knife_class = 0, weapon_world_model_class = 0;
    std::int32_t original_model = -1, original_definition = 0;
    std::string_view inspection_sequence = "lookat01"; //4348DF7D..DFC6 ->owner+358
    std::array<KnifeSequenceModels, 19> models{};
    std::array<KnifeSequenceModels, 2> default_models{}; //native42/59 rows+2E0/+300
    std::array<SequenceHistory, 150> history{};
    std::vector<PendingViewModel> viewmodels;
    std::vector<PendingWeaponHandle> weapon_handles;
};
class SequenceEngine : public CounterRandom {
public:
    virtual SequenceEntity* local_player() = 0; // 0x43475808
    virtual SequenceEntity* inventory_player() = 0; // EDX at 0x433E53A5
    virtual bool demo_playback() = 0; // engine +0x148
    virtual bool hltv() = 0; // engine +0x174
    virtual std::int32_t local_player_index() = 0;
    virtual bool local_identity(PlayerIdentity&) = 0;
    virtual std::uint32_t acknowledged_command() = 0; // engine +0x334
    virtual SequenceEntity* entity_for_handle(std::uint32_t) = 0;
    virtual SequenceEntity* inventory_entity(std::uint32_t) = 0;
    virtual const StudioSequences* model_sequences(std::int32_t model) = 0;
    virtual bool animation_interface_available() = 0; // resolved engine interface 0x4346A460
    virtual std::int32_t* prediction_random_seed() = 0; // 0x43468328, optional engine global
    virtual SequenceEntity** prediction_player() = 0; // 0x434693A4, optional engine global
    // Actual external engine targets at animation interface +0x5C / +0x64.
    virtual std::int32_t select_weighted_sequence(SequenceEntity&, std::int32_t activity) = 0;
    virtual std::int32_t lookup_sequence(SequenceEntity&, std::string_view name) = 0;
    virtual void random_seed(std::int32_t) = 0; // vstdlib!RandomSeed
    virtual float interval_per_tick() = 0;
    // Each method calls the original engine receive proxy with an integer
    // payload at +8 and DVariant type 1 at +0x14. Native header words +0/+4
    // and trailing words +0x18..+0x24 are zero; +0xC/+0x10 are unspecified.
    // The remaining arguments are the entity and indicated output field.
    virtual void receive_model_index(SequenceEntity&, std::int32_t) = 0;
    virtual void receive_sequence(SequenceEntity&, std::int32_t) = 0;
    virtual bool has_viewmodel_model_proxy() = 0;
    virtual void receive_viewmodel_model(SequenceEntity&, std::int32_t) = 0;
    virtual void receive_weapon_handle(SequenceEntity&, std::uint32_t) = 0;
    virtual void begin_model_cache_lock() = 0; // model cache +0x84
    virtual void end_model_cache_lock() = 0; // model cache +0x88
    virtual void set_model_index(SequenceEntity&, std::int32_t) = 0; // entity +0x12C
    virtual float current_time() = 0;
    virtual void schedule_full_update(float) = 0;
};
// Complete 0x433E53A5: replay queued proxies, map actual studio sequences,
// update owned knife/viewmodel inventory entities, then replay weapon handles.
void update_sequences(const Settings&, SequenceRuntime&, std::span<const InventoryEntry>,
                      SequenceEngine&);
} // namespace skeet::skins
