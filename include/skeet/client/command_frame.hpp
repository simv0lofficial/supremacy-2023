#pragma once
#include "commands.hpp"

namespace skeet::client {
struct CommandBullet {
    std::uint16_t weapon_id, padding;
    float penetration, range, range_modifier, armor_ratio, damage;
    std::uint32_t pellet_count;
};
static_assert(sizeof(CommandBullet) == 0x1C);
// Shared 0x1F4 frame passed to the CreateMove phase interfaces. Pointer fields
// retain the original x86 address width, as in Command's vtable word.
struct CommandFrame {
    std::uint32_t flags, secondary_flags;
    std::uint32_t send_packet_address, weapon_address, weapon_data_address;
    Command previous_command;
    float input_forward_78;
    std::uint32_t player_flags_7c;
    float input_side_80, view_height_84;
    std::array<std::byte, 0x10> fields_88;
    float duck_amount_98;
    std::array<float, 3> recoil_angles_9c;
    float player_field_a8;
    std::int32_t command_number_ac;
    std::uint32_t field_b0, field_b4;
    float angle_b8, field_bc, scalar_c0;
    union {
        std::uint32_t field_c4;
        float inaccuracy_c4;
    };
    std::int32_t tick_base_c8;
    std::uint32_t field_cc;
    float spread_d0;
    std::uint32_t field_d4;
    std::int32_t weapon_id_d8;
    std::array<float, 3> origin_dc;
    std::array<std::byte, 0x10> fields_e8;
    std::uint32_t player_flags_f8;
    std::int32_t choke_limit_fc;
    union {
        std::uint32_t field_100;
        float weapon_field_100;
    };
    std::uint8_t state_104;
    std::array<std::byte, 3> padding_105;
    std::uint32_t field_108;
    std::array<float, 3> origin_10c;
    std::array<float, 3> angles_118;
    std::uint8_t weapon_kind_124;
    std::array<std::byte, 3> padding_125;
    CommandBullet bullet_128;
    std::array<std::byte, 0xC> fields_144;
    std::array<float, 3> velocity_150;
    std::int32_t choked_15c;
    union {
        std::uint32_t field_160;
        float adjusted_spread_160;
    };
    float height_164;
    std::array<float, 3> origin_168;
    std::array<float, 3> forward_174;
    std::array<float, 3> previous_velocity_180;
    std::uint32_t field_18c;
    float previous_speed_190;
    std::array<std::byte, 8> fields_194;
    std::int32_t pellet_count_19c;
    std::uint32_t field_1a0;
    std::uint32_t field_1a4;
    std::array<float, 3> direction_1a8;
    std::uint32_t field_1b4;
    std::array<float, 3> hull_minimum_1b8, hull_maximum_1c4;
    std::uint32_t field_1d0;
    std::uint32_t field_1d4, field_1d8, field_1dc;
    std::uint32_t field_1e0;
    union {
        std::uint32_t field_1e4;
        float postpone_fire_1e4;
    };
    float maximum_angle_step_1e8;
    std::uint32_t field_1ec, field_1f0;
};
static_assert(sizeof(CommandFrame) == 0x1F4);
static_assert(offsetof(CommandFrame, previous_command) == 0x14);
static_assert(offsetof(CommandFrame, command_number_ac) == 0xAC);
static_assert(offsetof(CommandFrame, player_flags_7c) == 0x7C);
static_assert(offsetof(CommandFrame, recoil_angles_9c) == 0x9C);
static_assert(offsetof(CommandFrame, weapon_id_d8) == 0xD8);
static_assert(offsetof(CommandFrame, tick_base_c8) == 0xC8);
static_assert(offsetof(CommandFrame, choke_limit_fc) == 0xFC);
static_assert(offsetof(CommandFrame, player_flags_f8) == 0xF8);
static_assert(offsetof(CommandFrame, velocity_150) == 0x150);
static_assert(offsetof(CommandFrame, bullet_128) == 0x128);
static_assert(offsetof(CommandFrame, angles_118) == 0x118);
static_assert(offsetof(CommandFrame, hull_minimum_1b8) == 0x1B8);
static_assert(offsetof(CommandFrame, direction_1a8) == 0x1A8);
static_assert(offsetof(CommandFrame, maximum_angle_step_1e8) == 0x1E8);
static_assert(offsetof(CommandFrame, choked_15c) == 0x15C);
static_assert(offsetof(CommandFrame, previous_speed_190) == 0x190);

// Exact sparse constructor 43366497. All other frame bytes are untouched.
void initialize_command_frame(CommandFrame&) noexcept;
// Exact inlined frame prefix 433D34BB..433D3522, before query-limit phase 9.
void prepare_command_frame(CommandFrame&, const Command& saved_command,
                           std::uint32_t send_packet_address, std::int32_t choked,
                           std::int32_t command_number) noexcept;
} // namespace skeet::client
