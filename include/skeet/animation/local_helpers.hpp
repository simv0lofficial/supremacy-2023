#pragma once
#include "skeet/client/commands.hpp"
#include "skeet/engine/interface_call.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace skeet::client {
struct PredictionHistoryOwner;
}
namespace skeet::animation {
struct LocalAnimationOffsets {
    std::ptrdiff_t layers; //B538BBA5, descriptor.data/count+C
    std::ptrdiff_t studio_header; //32E4DAF1
    std::ptrdiff_t animation_state; //3D52A1A7
    std::ptrdiff_t active_weapon; //CD812654
    std::ptrdiff_t weapon_identifier; //5C9F0FBB
    std::ptrdiff_t player_flags; //E2FBE695
    std::ptrdiff_t additional_ground; //AC7F2913
    std::ptrdiff_t movement_state; //52899DF3
};
using InitializeSequenceMapping = engine::MemberFunction<void*>;
using SelectSymbolSequence =
    engine::MemberFunction<std::int32_t, void*, std::int32_t, const std::uint16_t*, std::int32_t>;
using LookupAnimationSymbol = engine::MemberFunction<void, std::uint16_t*, const char*>;
struct LocalLayerWeightEngine {
    void* context;
    // Genuine4346A598 receives ECX=state,EDX=0,stack(index),returnsXMM0.
    // The runtime adapter handles that external ABI only.
    float (*read)(void* context, void* animation_state, std::int32_t layer);
};
struct LocalAnimationBindings {
    LocalAnimationOffsets offsets;
    void* entity_list;
    void* symbol_table; //4346830C
    InitializeSequenceMapping initialize_mapping; //43469348
    SelectSymbolSequence select_symbol_sequence; //4346A4F8
    LookupAnimationSymbol find_symbol; //434691DC
    LookupAnimationSymbol add_symbol; //4346728C
    void* activity_interface; //4346A460; genuine entries+5C/+64
    std::int32_t* prediction_seed; //43468328
    void** prediction_player; //434693A4
    void* const& local_player; //live43475808, read at prediction publication
    client::PredictionHistoryOwner* const& sequence_owner; //live434759B4,canonical owned strings
    engine::MemberFunction<std::int32_t, std::int32_t> sequence_activity; //4346A364
    void* layer_weight_entry; //4346A598; genuine engine ABI adapted by main
    LocalLayerWeightEngine layer_weight;
    engine::MemberFunction<const char*, std::int32_t> sequence_name_entry; //434678C8
    // Explicit standalone context bytes. The supported engine methods at
    // vtable+5C/+64 consume only word7 (player); production reserves these.
    std::array<std::uint32_t, 6> activity_scratch;
};
// Complete433EAE1C, including weapon slot455 and the native prefix table.
const char* weapon_sequence_prefix(void* weapon, const LocalAnimationBindings&);
// Complete433B1A8F, including modifier symbol lookup/add and mapping cache.
std::int32_t select_symbol_weighted_sequence(void* player, std::int32_t layer,
                                             std::int32_t activity, const LocalAnimationBindings&);
// Complete433DF1A9->433C0554. The latter is solely a cdecl return-spoof
// trampoline to the genuine animation-interface entry+64.
std::int32_t map_animation_activity(void* player, const char* name, const LocalAnimationBindings&);
// Complete43387742, with paired prediction seed/player publication/restoration.
std::int32_t select_prediction_sequence(void* player, std::int32_t activity, std::int32_t seed,
                                        const LocalAnimationBindings&);
// Complete433800F5. Valid player and command contract; the full state dword
// is cleared first even when no directional combination is admitted.
std::uint32_t movement_button_state(std::uint32_t buttons) noexcept;
void apply_movement_button_state(void* player, const client::Command&,
                                 const LocalAnimationBindings&) noexcept;
// Complete4340BD50; scalar arguments are XMM0..4, references are ECX/EDX.
void update_lower_body_yaw_timer(float yaw, float horizontal_speed, float vertical_speed, float now,
                                 float target_yaw, float& next_update,
                                 float& previous_yaw) noexcept;
// Complete434011C8; only modifier byte32B is assigned.
void mark_animation_modifiers(void* animation_state) noexcept;
// Called under the native nonnull4346A598 gate; read is its actual ABI adapter.
float read_local_layer_weight(void* animation_state, std::int32_t layer,
                              const LocalAnimationBindings&);
// Complete433EBF17 and43381E04. The animation-state pointer is the caller's
// saved/live0x348-byte state. Both reset the actual layer5 through player slot223.
void update_landing_layer(void* player, void* animation_state, const LocalAnimationBindings&);
// Fourth native argument (previous+110) is passed and purged but never read.
void update_post_animation_layer(void* player, void* animation_state, bool previous_in_air,
                                 float unused_previous_time, const LocalAnimationBindings&);
} // namespace skeet::animation
