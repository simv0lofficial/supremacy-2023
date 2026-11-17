#pragma once
#include "native_data_image.hpp"

namespace skeet::engine {
// Recovered last-startup operation: search the actual animation Update code
// cell434693F0 for C605??????????F30F5F05 within512 bytes, then publish its
// decoded MOV byte-store address to43467B24. Null code or a search miss preserves
// the previous output. The flag's current byte is never changed by startup.
// Its existing animation-update/channel consumers perform their own clears.
void initialize_animation_update_flag(NativeDataImage&);
} // namespace skeet::engine
