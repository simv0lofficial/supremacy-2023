#pragma once
#include "types.hpp"

namespace skeet {

// Command feedback at full movement owner +0x1AC. The secondary callback
// interface receives owner+8 and passes this as its +0x1A4 subobject.
struct MovementFeedback {
    std::uint8_t side_flip;
    std::uint8_t was_grounded;
    std::uint8_t prediction_blocked;
    std::uint8_t reserved;
    std::uint32_t original_buttons;
    float primary_heading;
    float turn_direction;
    float secondary_heading;
};
static_assert(sizeof(MovementFeedback) == 20);

struct MovementFeedbackHistory {
    float primary_heading;
    float turn_direction;
    float secondary_heading;
};

// Complete command entry writes 0x4336F936..0x4336F9AA. Flags and the feedback
// record are reset after preserving the three values consumed by the solver.
MovementFeedbackHistory begin_movement_command(MovementFeedback& feedback,
                                               const UserCommand& command,
                                               std::uint32_t& original_buttons,
                                               std::uint32_t& scheduler_flags) noexcept;

} // namespace skeet
