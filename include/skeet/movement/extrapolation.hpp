#pragma once
#include "simulation_tick.hpp"
#include "simulation_capture.hpp"
#include "simulation_environment.hpp"
#include "skeet/animation/entity_update.hpp"
#include "skeet/client/player_state.hpp"
#include <optional>

namespace skeet {

struct MovementExtrapolationBindings {
    lagcomp::EntityNetvars offsets;
    std::uint32_t velocity_7a2d60c1;
    std::uint32_t origin_4e66bb34;
    std::uint32_t origin_2a671b42;
    std::uint32_t field_7090d8dd;
    std::uint32_t field_52899df3;
    std::uint32_t field_d3d4230d;
    // Decoded fixture inputs are absent in the mandatory live entry. Its
    // actual target is captured at43385AFD and each simulation construction.
    std::optional<std::uint32_t> player_handle;
    client::PlayerStateCache* players; //actual433B8A2B lookup in43385AFD
    std::optional<MovementSimulationPlayer> snapshot;
    std::optional<MovementSimulationRules> rules;
    std::optional<MovementSimulationEnvironment> environment;
    MovementCollisionEngine* collision;
    animation::AnimationUpdateEngine animation_engine;
    std::int32_t* animation_frame; //434678F8
    float* current_time; //43467EB8
    float* interval; //4346A338
    std::uint8_t* animation_reset; //optional43467B24
    lagcomp::SetAbsoluteVector set_absolute_angles; //optional434688C8
    void* allocator; //actual43467204 interface, Free virtual slot5
};
struct MovementExtrapolationLiveBindings {
    const MovementCaptureInput& capture;
    const MovementEnvironmentBindings& environment;
};
// Complete43322B09 valid-player path. The unused native ECX argument is
// omitted. More than32 ticks or a null player animation state returns0.
// The animation state copy, entity-field overrides, clock restoration and
// animation-vector cleanup are owned here; simulation calls the full tick.
std::int32_t extrapolate_lag_record(void* player, client::PlayerState&, lagcomp::LagRecord&,
                                    std::int32_t ticks, const MovementExtrapolationBindings&);
// Raw43322B09 entry. Constructor and every tick read this actual target at
// their native sites. Frozen semantic snapshots above are not consumed.
std::int32_t extrapolate_lag_record(void* player, client::PlayerState&, lagcomp::LagRecord&,
                                    std::int32_t ticks, const MovementExtrapolationBindings&,
                                    const MovementExtrapolationLiveBindings&, bool rebuild_pose = false);

} // namespace skeet
