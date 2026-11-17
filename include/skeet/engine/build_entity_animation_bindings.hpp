#pragma once
#include "loaded_modules.hpp"
#include "native_data_image.hpp"
#include "skeet/lagcomp/entity_state.hpp"

namespace skeet::engine {
struct NativeOffsetStorage;
struct BuildEntityAnimationBindings {
    MemberFunction<void, std::int32_t> invalidate_physics;
    lagcomp::SetAbsoluteVector set_absolute_angles, set_absolute_origin;
    lagcomp::SetCollisionBounds set_collision_bounds;
    std::uint8_t* transform_override;
    double* animation_timestamp;
    void** player_resource;
    void* smoke_registry;
    MemberFunction<void> update_render_visibility;
};
// Supported client.dll65272025 only. Verifies PE identity, sections, actual
// function instructions/RET ABI, setter calls and live data operands before
// publishing43468C10/434688C8/43467E44/4346A7F0/434683D0/4346A7E4/434679C0/43468320/43468FA4. Independently
// checks the player-resource slot already published by misc engine bindings.
// This explicit source build policy runs no engine code and loads no module.
// Also resolves the collision-height/time encodings from the engine's paired
// stores and readers. Their shared zero word is not a zero field offset.
// These targets all exist in this build; incomplete recovery is an error.
BuildEntityAnimationBindings
initialize_build_entity_animation_bindings(NativeDataImage&, const LoadedModuleEnvironment&,
                                           NativeOffsetStorage&);
} //namespace skeet::engine
