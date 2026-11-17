#pragma once
#include "skeet/config.hpp"
#include "skeet/esp/other_config.hpp"

namespace skeet::misc {
struct FovConfig {
    IntegerConfig fov, zoom_fov;
}; // owner 0x4347DDCD: +0xC/+0x20
struct KnifeConfig {
    BooleanConfig enabled;
    IntegerConfig mode;
}; // 0x434802E2: +0xC/+0x20
struct ZeusConfig {
    BooleanConfig enabled;
}; // 0x43485D4C: +0xC
struct AutomaticWeaponsConfig { // owner 0x4347D991
    BooleanConfig enabled; // +0xC
    IntegerConfig delay; // +0x20: constructor preserves its existing value
    HotkeyConfig last_second_defuse; // +0x34: R / hold
};
struct QuickSwitchConfig {
    BooleanConfig enabled;
}; // 0x4347DC0A: +0x1C
struct RevealRanksConfig {
    BooleanConfig enabled;
}; // 0x4347E8A5: +4
struct RevealOverwatchConfig {
    BooleanConfig enabled;
}; // 0x4348E4CC: +0xC
struct AutoAcceptConfig {
    BooleanConfig enabled;
}; // 0x4348E0E8: +4, explicitly true
struct ClanTagConfig {
    BooleanConfig enabled;
}; // 0x4347E092: +0x14
struct EventLogConfig {
    BooleanConfig purchases, damage;
}; // 0x434819DE: +0xC/+0x20
struct GrenadePredictionConfig { // owner 0x4348675E, shared with ESP trajectory settings
    BooleanConfig super_toss; // +0x58
    BooleanConfig automatic_release; // +0x128
    HotkeyConfig release_key; // +0x13C: Escape / always
    IntegerConfig minimum_damage; // +0x164: 50
};
struct PersistentKillFeedConfig {
    BooleanConfig enabled;
}; // 0x4347E910: +0x14
struct DisablePureConfig {
    BooleanConfig enabled;
}; // 0x434864C4: +0xC
struct RebuyConfig {
    BooleanConfig enabled;
}; // 0x4348217D: +0x14, explicitly false
struct ConsoleOutputConfig {
    BooleanConfig enabled;
}; // 0x43481F99: +0xC
struct PingSpikeConfig {
    BooleanConfig enabled{{{nullptr, nullptr, 0x916F47DAu}, 1, ConfigType::boolean, 0}, 0};
    HotkeyConfig key{{{nullptr, nullptr, 0x3C4135FFu}, 8, ConfigType::hotkey, 0}, {}};
    IntegerConfig milliseconds{{{nullptr, nullptr, 0xC5290E81u}, 4, ConfigType::integer, 0}, 0};
};
struct FreeLookConfig {
    HotkeyConfig key{{{nullptr, nullptr, 0x77D31850u}, 8, ConfigType::hotkey, 0}, {}}; //43468264
};
struct AntiUntrustedConfig {
    BooleanConfig enabled{{{nullptr, nullptr, 0x851E6ABEu}, 1, ConfigType::boolean, 0},
                          0}; //434693DC
};
struct CustomEventsConfig {
    BooleanConfig enabled{{{nullptr, nullptr, 0x526A2BF4u}, 1, ConfigType::boolean, 0},
                          0}; //434691A8
};
struct HideFromObsConfig {
    BooleanConfig enabled{{{nullptr, nullptr, 0x643364E6u}, 1, ConfigType::boolean, 0},
                          0}; //43468E48
};

// Every entry below follows one actual owner or one CRT initializer. There is
// deliberately no invented ordering between independent feature owners.
void initialize_fov_config(ConfigRegistries&, FovConfig&) noexcept;
void initialize_knife_config(ConfigRegistries&, KnifeConfig&) noexcept;
void initialize_zeus_config(ConfigRegistries&, ZeusConfig&) noexcept;
void initialize_automatic_weapons_config(ConfigRegistries&, AutomaticWeaponsConfig&) noexcept;
void initialize_quick_switch_config(ConfigRegistries&, QuickSwitchConfig&) noexcept;
void initialize_reveal_ranks_config(ConfigRegistries&, RevealRanksConfig&) noexcept;
void initialize_reveal_overwatch_config(ConfigRegistries&, RevealOverwatchConfig&) noexcept;
void initialize_auto_accept_config(ConfigRegistries&, AutoAcceptConfig&) noexcept;
void initialize_clan_tag_config(ConfigRegistries&, ClanTagConfig&) noexcept;
void initialize_event_log_config(ConfigRegistries&, EventLogConfig&) noexcept;
void initialize_grenade_prediction_config(ConfigRegistries&, GrenadePredictionConfig&,
                                          esp::GrenadeEspConfig&) noexcept;
void initialize_persistent_kill_feed_config(ConfigRegistries&, PersistentKillFeedConfig&) noexcept;
void initialize_disable_pure_config(ConfigRegistries&, DisablePureConfig&) noexcept;
void initialize_rebuy_config(ConfigRegistries&, RebuyConfig&) noexcept;
void initialize_console_output_config(ConfigRegistries&, ConsoleOutputConfig&) noexcept;
void initialize_ping_spike_enabled(ConfigRegistries&, PingSpikeConfig&) noexcept; // 0x43490AAF
void initialize_ping_spike_key(ConfigRegistries&, PingSpikeConfig&) noexcept; // 0x43490AC9
void initialize_ping_spike_milliseconds(ConfigRegistries&, PingSpikeConfig&) noexcept; // 0x43490B14
void initialize_free_look_config(ConfigRegistries&, FreeLookConfig&) noexcept; // 0x43493241
void initialize_anti_untrusted_config(ConfigRegistries&,
                                      AntiUntrustedConfig&) noexcept; // 0x434928E5
void initialize_custom_events_config(ConfigRegistries&, CustomEventsConfig&) noexcept; // 0x43492943
void initialize_hide_from_obs_config(ConfigRegistries&, HideFromObsConfig&) noexcept; // 0x43492907
} // namespace skeet::misc
