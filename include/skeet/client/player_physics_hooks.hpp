#pragma once
#include "player_state.hpp"
#include "skeet/engine/hooks.hpp"
#include <array>

namespace skeet::engine {
struct NativeOffsetGlobals;
}
namespace skeet::client {
struct PlayerPhysicsHookFields {
    std::ptrdiff_t absolute_velocity_36375c23;
    std::ptrdiff_t duck_amount_3309c0cf;
};
PlayerPhysicsHookFields native_player_physics_hook_fields(const engine::NativeOffsetGlobals&);
struct PlayerPhysicsHookBindings {
    PlayerStateCache& players;
    const std::uint8_t& command_mode_43467224;
    const std::uint8_t& alternate_mode_4346a740;
    const std::uint32_t& maximum_velocity_convar_43468c2c; //encoded^3FE3AE0C
    const std::uint32_t& velocity_return_site_43467f0c;
    const std::uint32_t& velocity_continuation_434757c4;
    std::uint8_t* const& interpolation_guard_43468fa4;
    PlayerPhysicsHookFields fields;
};

// Shared109/145/173 and remote78/113/169 from the actual1B1A0D29 player
// hook families. Original calls use each receiver's installed[-2] metadata.
class PlayerPhysicsHooks {
public:
    explicit PlayerPhysicsHooks(PlayerPhysicsHookBindings bindings) noexcept : bindings(bindings) {}
    const PlayerPhysicsHookBindings bindings;
    void bind_native_replacement(engine::VtableHookMetadata&, std::size_t slot,
                                 std::uint32_t native_entry);
    static void* native_replacement_entry(std::size_t slot, std::uint32_t native_entry);
    void* studio_header(void* player); //43402307
    void estimate_absolute_velocity(void* player, std::array<float, 3>&,
                                    std::uintptr_t& caller_return); //4333181E
    bool should_collide(void* player, std::int32_t group, std::uint32_t mask); //4333C562
    // The engine's four stack words retain their bits; the canonical cache
    // field+1D4 alone controls whether that original update is forwarded.
    void update_interpolated_state(void* player, std::array<std::uint32_t, 4>); //433ABBF0
    std::uint8_t interpolate(void* player,
                             std::uint32_t time_bits); //433875BE, raw float stack word
    void eye_position(void* player, std::array<float, 3>& output); //43382222
};
} //namespace skeet::client
