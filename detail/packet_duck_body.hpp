#pragma once
#include "skeet/network/packet_control.hpp"
#include <bit>

namespace skeet::network::packet_duck_detail {
inline float absolute(float value) noexcept {
    return std::bit_cast<float>(std::bit_cast<std::uint32_t>(value) & 0x7fffffffU);
}
inline void force_send(std::uint32_t& flags) noexcept {
    flags = (flags & ~0x400U) | 0x200U;
}

// Shared 433EA9DA..433EADAB. Input readers preserve the native branches: only
// interval/velocity are retained across player virtual10; all later settings,
// player flags, activation and choked-command reads occur at their consumers.
template <class Input>
void update(FakeLagState& state, const Input& input, std::uint32_t& buttons, std::uint32_t& flags) {
    if (input.movement_type() == 2 && input.player_state() < 2) {
        if (!(input.player_flags() & 1U) && input.mode() == 1 && !(buttons & 0x10U))
            buttons |= 4U;
        if (input.active() && (!(input.player_flags() & 1U) || !(state.flags & 1U)) &&
            input.lag_mode(state) == 0) {
            const float interval = input.interval();
            const auto velocity = input.velocity();
            const auto& origin = input.absolute_origin();
            const float predicted_x = velocity.x * interval + origin.x;
            float predicted_z = interval * 0.0F + origin.z;
            const float dx = state.last_origin.x - predicted_x;
            const float predicted_y = velocity.y * interval + origin.y;
            const float dy = state.last_origin.y - predicted_y;
            const float horizontal_squared = dy * dy + dx * dx;
            float dz = state.last_origin.z - predicted_z;
            if (dz * dz + horizontal_squared > 4096.0F)
                force_send(flags);
            else if (!(input.player_flags() & 1U)) {
                if (input.mode() < 2)
                    goto publish_ground_state;
                const auto ducked = input.player_flags() & 2U;
                if (ducked)
                    predicted_z -= 9.0F;
                else
                    predicted_z += 9.0F;
                dz = state.last_origin.z - predicted_z;
                if (dz * dz + horizontal_squared > 4096.0F) {
                    if (ducked)
                        buttons &= ~4U;
                    else
                        buttons |= 4U;
                    force_send(flags);
                }
            }
        }
        const auto mode = input.mode();
        if (mode >= 2) {
            if (mode == 4) {
                // 433EAC16 reloads velocity after the origin getter. Z stays
                // in XMM2 through the following current-ground-state read.
                const auto velocity = input.velocity();
                const float speed_squared =
                    (velocity.x * velocity.x + velocity.y * velocity.y) + velocity.z * velocity.z;
                if (speed_squared != 0.0F && !(input.player_flags() & 1U) &&
                    absolute(velocity.z) > 120.0F)
                    buttons |= 4U;
            } else if (input.mode() == 3 && input.active()) {
                if (input.player_flags() & 1U)
                    state.value_58 = 0;
                else if (input.duck_state() == 0) {
                    if (state.value_58 == 0) {
                        if (state.target != 0 && input.engine_choked() == state.target) {
                            buttons |= 4U;
                            state.value_58 = 1;
                        }
                    } else if (state.value_58 == 1) {
                        if (state.target == 0 || input.engine_choked() != state.target)
                            buttons |= 4U;
                        else
                            state.value_58 = 2;
                    }
                }
            }
        }
    }
publish_ground_state:
    state.flags = (state.flags & ~1U) | (input.player_flags() & 1U);
}
} // namespace skeet::network::packet_duck_detail
