#pragma once
#include "grenade_prediction.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet::esp {
struct GrenadeEnvironmentOffsets {
    explicit GrenadeEnvironmentOffsets(const engine::NativeOffsetGlobals&);
    engine::DecodedOffset fire_origin_a9ee6062, fire_count_3584e3a5, fire_active_fc1ef3a1;
    engine::DecodedOffset fire_x_24a16ff6, fire_y_94e8c7f7, fire_z_d7600aac;
    engine::DecodedOffset collision_flags_b7e9c94e, movement_type_7501cb91, state_f0374295;
    engine::DecodedOffset owner_8d1bd8d0, activity_8dbc9874, activity_68140cd8;
};
// Constructs bound operands only; no pointer/key/pointee is sampled here.
GrenadeEnvironmentOffsets capture_grenade_environment_offsets(const engine::NativeOffsetGlobals&);
// Actual engine storage, read again at each native physics/trace boundary.
// No scalar physics snapshot is accepted by the production overloads.
struct GrenadeEnvironmentBindings {
    const float& tick_interval; // *4346A338
    const float& frame_interval; //43467B04 pointee+20
    engine::NativeConVarReference gravity_convar; //43467A54 xorB882FC15
    engine::NativeConVarReference incendiary_duration_convar; //43467048 xor983081BF
    engine::NativeConVarReference incendiary_max_slope_convar; //43467088 xor8FD87158
    void* const& entity_list; //4346A890, index virtual3/highest field24
    void* const& breakable_class; //4346A440
    void* const& physics_class; //43467478
    void* const& alternate_breakable_class; //43468CA0
    void* const& slope_exempt_class; //4346719C
    const void* const& inferno_class; //434674E8
    GrenadeEnvironmentOffsets offsets;
    float gravity() const noexcept;
    float incendiary_duration() const noexcept;
    float incendiary_max_slope() const noexcept;
    std::uint32_t world_entity() const;
};
} // namespace skeet::esp
