#pragma once
#include "command_runtime.hpp"

namespace skeet {

//4348FA90 through the scalar portion of4347DCB2. Both configuration groups
// must still be unregistered. ConVar pointers are the decoded actual globals.
void initialize_movement_state(MovementCommandOwner&, ConfigRegistries&, const void* air_accelerate,
                               const void* jump_speed_cap);
//4348FD15..4348FD3F, after the owner's two native strings are constructed.
void finish_movement_state_initialization(MovementCommandOwner&) noexcept;
void reset_movement_state(MovementCommandOwner&) noexcept; //433F4938
void bind_movement_settings(MovementCommandOwner&, ConfigRegistries&); //43490711

} //namespace skeet
