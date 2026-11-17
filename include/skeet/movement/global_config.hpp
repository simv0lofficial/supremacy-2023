#pragma once
#include "skeet/config.hpp"

namespace skeet {
// Separate native global descriptors, outside the4348FA90 movement owner.
// Their CRT entries are independent; callers retain the actual CRT order.
struct MovementGlobalConfig {
    BooleanConfig infinite_duck{{{nullptr, nullptr, 0xD0CA5B87u}, 1, ConfigType::boolean, 0},
                                0}; //43467CDC,value43467CEC
    IntegerConfig air_duck{{{nullptr, nullptr, 0x5B1A5C7Fu}, 4, ConfigType::integer, 0},
                           0}; //4346A570,value4346A580
    HotkeyConfig edge_jump{{{nullptr, nullptr, 0x5BBB224Eu}, 8, ConfigType::hotkey, 0},
                           {}}; //434675FC,value4346760C
};
// Metadata above belongs to the loaded image. These CRT callbacks only link
// the existing descriptors and perform their explicit runtime-value writes.
void initialize_infinite_duck_config(ConfigRegistries&, BooleanConfig&) noexcept; //434933A6
void initialize_air_duck_config(ConfigRegistries&, IntegerConfig&) noexcept; //43490B39
void initialize_edge_jump_hotkey(ConfigRegistries&, HotkeyConfig&) noexcept; //4349340B
} //namespace skeet
