#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

namespace skeet {

struct Point {
    std::int32_t x, y;
};
struct Rectangle {
    std::int32_t x, y, width, height;
};
// Native hit testing includes the right and bottom edges and uses wrapping x86 sums.
bool contains(const Rectangle& bounds, Point cursor) noexcept;

struct MenuTab {
    void* object;
    void (*reset)(void*);
    void (*activate)(void*);
};
struct Menu {
    std::span<MenuTab> tabs;
    std::uint32_t active_tab;
};
void select_tab(Menu& menu, std::uint32_t requested_tab);

struct Matrix3x4 {
    float rows[3][4];
};
using HitboxMatrices = std::array<Matrix3x4, 19>;
// Preserve the original sequential 16-byte load/store ordering, including aliases.
void copy_hitbox_matrices(const HitboxMatrices& source, HitboxMatrices& destination) noexcept;

// Both native x86 helpers take XMM0/XMM1 and return XMM0. An ordinary x86
// float return passes through x87 and quiets a selected signaling NaN.
#if defined(_M_IX86) || defined(__i386__)
#define SKEET_NATIVE_FLOAT_ABI __vectorcall
#else
#define SKEET_NATIVE_FLOAT_ABI
#endif
float SKEET_NATIVE_FLOAT_ABI minimum_number(float first, float second) noexcept;
float SKEET_NATIVE_FLOAT_ABI maximum_number(float first, float second) noexcept;
#undef SKEET_NATIVE_FLOAT_ABI

enum class HotkeyMode : std::uint32_t {
    always = 0,
    hold = 1,
    toggle = 2,
    inverse_hold = 3,
};
struct HotkeyBinding {
    // bit 1: toggle state; bits 2..31: bit index in the input-state bitmap.
    std::uint32_t packed;
    HotkeyMode mode;
};
struct InputBitmap {
    std::span<const std::uint32_t> words;
    std::uint32_t bit_count;
};
bool hotkey_active(const HotkeyBinding& binding, InputBitmap input);

inline constexpr std::uint32_t on_ground = 1;
inline constexpr std::uint32_t jump_button = 2;
inline constexpr std::uint32_t frame_on_ground = 0x40000;
inline constexpr std::uint32_t frame_shared_flag = 0x80;

struct MovementFrame {
    std::uint32_t packed_flags;
    std::uint32_t saved_player_flags;
};
struct UserCommand {
    float forward_move, side_move, up_move;
    std::uint32_t buttons;
};
struct MovementSettings {
    bool jump_at_edge;
    // The bit's effect is proven; its higher-level owner is not identified yet.
    std::uint32_t shared_callback_flags;
};
void post_prediction(const MovementSettings& settings, MovementFrame& frame, UserCommand& command,
                     const HotkeyBinding& binding, InputBitmap input);

} // namespace skeet
