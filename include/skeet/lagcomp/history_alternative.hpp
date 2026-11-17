#pragma once
#include "records.hpp"
#include "skeet/client/command_frame.hpp"
#include "skeet/client/player_state.hpp"
#include "skeet/animation/entity_update.hpp"
#include "skeet/animation/state_copy.hpp"
#include "skeet/movement/collision.hpp"
#include <span>

namespace skeet::lagcomp {
struct HistoryAlternativeFields {
    std::uint32_t velocity_36375c23;
    std::uint32_t origin_4e66bb34, origin_2a671b42;
    std::uint32_t tick_3d10b94c, player_flags_e2fbe695, angles_e23a9be1;
    std::uint32_t maximum_speed_3eb71843;
    std::uint32_t weapon_time_0176a381;
};
struct HistoryAlternativePredictionLimit {
    void* const* local_player; // dereferenced global4346A3A4, nullable
    std::uint32_t field_bf567619, field_dda99ab5, field_b7ea8fda;
    const std::int32_t* configured; //43469274
};
struct HistoryAlternativeVariables {
    // Decoded engine ConVars: follow parent+1C, XOR float+2C/int+30 with
    // the terminal object's native address. Only recovery may be absent.
    const void* stamina_recovery; //43467E4C ^FC23F1E7
    const void* gravity; //43467A54 ^B882FC15
    const void* bunnyhop; //43467FA0 ^4B52A9BF
    const void* jump_impulse; //43467990 ^AF1F2C93
    const void* stamina_maximum; //4346A88C ^D9D937D8
    const void* stamina_jump; //43468FB4 ^C9B16EBB
    const void* stamina_land; //43468F4C ^B0CF25E8
    const void* air_acceleration; //434674E4 ^DCEAEAC8
};
struct HistoryAlternativeBindings {
    EntityNetvars offsets;
    HistoryAlternativeFields fields;
    HistoryAlternativePredictionLimit prediction;
    HistoryAlternativeVariables variables;
    void* engine; // actual engine-client virtual12/14
    void* entity_list; // actual entity-list virtual4
    float* interval;
    const float* receive_interval;
    float* current_time;
    const std::int32_t* animation_frame;
    std::uint8_t* animation_reset;
    std::uint8_t& alternate_strafe; //4347CA58, shared across players/calls
    MovementCollisionEngine& collision;
    std::uint8_t* engine_trace_flag;
    animation::AnimationUpdateEngine animation_engine;
    RageTrigonometry trigonometry;
    double (*atan2_import)(double y, double x); // actual CRT77509EE0
    SetAbsoluteVector set_absolute_angles; // optional434688C8
    void* allocator; //43467204
    // Native local348-byte workspace. Its initial bytes are retained when
    // the player's animation pointer is null, except constructor+13C..14C.
    // Each invocation requires independent storage; it must not alias player.
    std::span<std::byte, animation::copied_animation_state_size> animation_scratch;
};

// Native4333D6D9: context, nonnull player, canonical per-player cache,
// destination record and requested extrapolation ticks, in that order.
std::int32_t model_alternative_history_record(client::CommandFrame& context, void* player,
                                              client::PlayerState& state, LagRecord& destination,
                                              std::int32_t ticks,
                                              const HistoryAlternativeBindings& bindings);
} // namespace skeet::lagcomp
