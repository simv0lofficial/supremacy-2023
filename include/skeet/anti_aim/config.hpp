#pragma once
#include "skeet/config.hpp"

namespace skeet::anti_aim {
using BodyYawOffsetsConfig = ConfigValue<std::array<std::int32_t, 4>>;
using YawOffsetsConfig = ConfigValue<std::array<std::int32_t, 6>>;

struct GlobalConfig {
    BooleanConfig enabled{{{nullptr, nullptr, 0x96C0EF29u}, 1, ConfigType::boolean, 0},
                          0}; // 0x4346A730
    IntegerConfig pitch{{{nullptr, nullptr, 0x6D5F1172u}, 4, ConfigType::integer, 0},
                        0}; // 0x43467F50
    IntegerConfig yaw{{{nullptr, nullptr, 0xF2D408F3u}, 4, ConfigType::integer, 0},
                      0}; // 0x434682EC
    IntegerConfig body_yaw{{{nullptr, nullptr, 0x4D6D88C6u}, 4, ConfigType::integer, 0},
                           0}; // 0x434685A8
};

// Offsets are relative to the owner constructed at 0x43490435.
struct Config {
    IntegerConfig yaw_base; // 0x01C
    IntegerConfig yaw_jitter; // 0x030
    IntegerConfig yaw_jitter_offset; // 0x044
    BooleanConfig edge_yaw; // 0x058
    BooleanConfig freestanding; // 0x06C
    BodyYawOffsetsConfig body_yaw_offsets; // 0x080
    YawOffsetsConfig yaw_offsets; // 0x0A0
    HotkeyConfig freestanding_hotkey; // 0x0C8
    BooleanConfig freestanding_body_yaw; // 0x0F0
    IntegerConfig roll; // 0x104
    IntegerConfig custom_pitch; // 0x118
};

// Static descriptor registration preserves their stored values. The owner
// constructor registers booleans without overwriting their stored bytes and
// clears integer/array values at 0x43490482..0x43490599, then sets the final
// yaw defaults (-12 for 180, 80 for Spin) at 0x434906DF..0x434906F3.
void register_global_config(ConfigRegistries&, GlobalConfig&) noexcept;
//43490482..43490599. The late constructor yaw defaults are separate so the
// whole owner retains the intervening string/animation initialization order.
void initialize_owner_config(ConfigRegistries&, Config&) noexcept;
void finish_owner_config(Config&) noexcept;
void initialize_config(ConfigRegistries&, Config&) noexcept;
struct FakeLagSettings {
    BooleanConfig enabled{{{nullptr, nullptr, 0xF9BBEC7BU}, 1, ConfigType::boolean, 0},
                          0}; //43468D54
    HotkeyConfig hotkey{{{nullptr, nullptr, 0x27D21BD4U}, 8, ConfigType::hotkey, 0}, {}}; //43467E00
    IntegerConfig amount, variance, limit; //owner434901DA +0C/+20/+34
};
struct OtherGlobalSettings {
    HotkeyConfig slow_motion_key{{{nullptr, nullptr, 0x7293DE2AU}, 8, ConfigType::hotkey, 0},
                                 {}}; //4346849C
    IntegerConfig leg_movement{{{nullptr, nullptr, 0x8F8A4366U}, 4, ConfigType::integer, 0},
                               0}; //434685E8
};
// Independent static constructors and the complete configuration portion of
//434901DA. Runtime fields andRDTSCseeding belong to network::FakeLagState.
void initialize_fake_lag_settings(ConfigRegistries&, FakeLagSettings&) noexcept;
void initialize_fake_lag_enabled(ConfigRegistries&, FakeLagSettings&) noexcept;
void initialize_fake_lag_hotkey(ConfigRegistries&, FakeLagSettings&) noexcept;
void initialize_other_global_settings(ConfigRegistries&, OtherGlobalSettings&) noexcept;
} // namespace skeet::anti_aim
