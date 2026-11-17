#pragma once
#include "entity_state.hpp"
#include "history_configuration.hpp"
#include <array>

namespace skeet::client {
struct PlayerState;
}
namespace skeet::lagcomp {
struct HistoryAnimationPublishFields {
    std::uint32_t animation_enabled; //8481EA04, byte
    std::uint32_t poses; //1CAAF0A8; pose vector and its interpolation watchers
    std::uint32_t absolute_velocity; //36375C23
    std::uint32_t studio_header; //32E4DAF1
    std::uint32_t player_origin; //A9EE6062
    std::uint32_t render_origin; //C865F10E
    std::uint32_t player_flags; //E2FBE695
    std::uint32_t old_simulation_time; //D3D4230D
    std::uint32_t local_time; //9F8E7F71, local player
    std::uint32_t invalidation_counter; //2B554AD0
};
struct HistoryAnimationPublishSaved {
    float* current_time; //43467EB8 retained before the animation transaction
    float saved_current_time;
    float* frame_time; //43467B04+14
    float saved_frame_time;
    Vector absolute_origin; //original player slot10, saved before animation
};
struct HistoryAnimationMovementEngine {
    void* context;
    // Genuine43468DC0: ECX=studio wrapper,EDX=sequence,stack(poses,out).
    // The engine entry writes the displacement; its length is recovered code.
    void (*displacement)(void* context, void* studio, std::int32_t sequence, const float* poses,
                         Vector& output);
};
struct HistoryAnimationPublishBindings {
    const EntityStateRuntime& entity;
    HistoryAnimationPublishFields fields;
    void* clock; //4346A514, actual engine virtual14
    void* const* const& local_time_entity; //live outer pointer cell4346A3A4; nullable
    const float* now; //4346A334
    const float& receive_interval; //live43467B04+20
    std::uint8_t* reset_byte; //optional43467B24
    const HistoryRecordingConfiguration& configuration;
    const std::uint8_t* player_table; //4347BE08, stride20
    std::uint32_t player_table_offset; //retained networkable index*20
    const std::int32_t& allow_zero_sequence; //4347C9EC
    const float& movement_exponent; //43475AC0+5C
    const std::uint8_t* candidate_attempts; //434745E0, three native bytes
    HistoryAnimationMovementEngine movement;
    // Actual imports7750BE50,775090B0,77509EE0. They are the only numeric
    // boundaries here; candidate generation and selection remain concrete.
    float (*power)(float, float);
    float (*arccosine)(float);
    double (*atan2)(double, double);
};
//43414F3C..43415F6F, including the acceptance cold tail434161F4..43416370.
// animation_state is the same nonnull state retained by CachedHistoryFrame.
// layer_weight and speed_fraction are the resolver worker's outgoing carries.
void publish_history_animation(client::PlayerState&, void* player, void* animation_state,
                               std::int32_t simulation_tick, std::int32_t simulation_ticks,
                               float speed_fraction, float layer_weight,
                               std::uint32_t& update_flags, const HistoryAnimationPublishSaved&,
                               const HistoryAnimationPublishBindings&);
//43415F6F..43415FF9, also reached by the native missing-animation-state arm.
void publish_history_animation_origin(client::PlayerState&, void* player,
                                      const Vector& saved_absolute_origin,
                                      const HistoryAnimationPublishBindings&);
} //namespace skeet::lagcomp
