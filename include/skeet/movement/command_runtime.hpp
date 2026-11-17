#pragma once
#include "command_state.hpp"
#include "duck_peek.hpp"
#include "quick_peek.hpp"
#include "pre_speed.hpp"
#include "skeet/client/commands.hpp"

namespace skeet {

// Portable owner of the proven434759B0 movement state. The source projection
// keeps full+53C in duck_peek.previous_buttons, shared by both command phases.
struct MovementCommandOwner {
    MovementConfig config;
    QuickPeekConfig quick_peek_config;
    MovementFeedback feedback;
    DuckPeekState duck_peek;
    QuickPeekState quick_peek;
    std::uint32_t previous_buttons_1e0 = 0;
    std::uint32_t suppressed_buttons_1e4 = 0;
    const void* air_accelerate = nullptr; //native+14
    const void* jump_speed_cap = nullptr; //native+18
    PreSpeedLimits limits{845.5f, 90.0f};
    float side_magnitude = 450.0f;
    float brake_reduction_limit = 15.0f;
    float brake_threshold = 30.0f;
    float angle_step = 60.0f;
    float field_1d8 = 286.0f;
    std::uint32_t callback_flags_1dc = 0;
    MovementVector previous_packet_origin; //full+56C, captured by4334659B
};

// Concrete live engine/global bindings are defined with the enclosing native
// scheduler implementation. Wrappers retain this typed reference while the
// individual source stages own their actual engine-boundary reads.
struct MovementCommandBindings;

// Complete secondary-interface phase1 callback4333F746. Infinite Duck is the
// current global43467CEC value. This also publishes the shared full+53C word.
void process_movement_phase_one(MovementCommandOwner&, client::Command&,
                                bool infinite_duck) noexcept;

// Fullowner entry4336F912, with the native unused/interface framing removed.
// feedback is explicit because the native main accepts a separate20-byte
// feedback record. Standard command wrappers pass owner.feedback.
void process_movement_command(MovementCommandOwner&, void* player, void* weapon, client::Command&,
                              MovementAngles& movement_angles, std::uint8_t& send_packet,
                              MovementFeedback&, const MovementCommandBindings&);
void process_movement_command(MovementCommandOwner&, void* player, void* weapon, client::Command&,
                              std::array<float, 3>& movement_angles, std::uint8_t& send_packet,
                              MovementFeedback&, const MovementCommandBindings&);

} // namespace skeet
