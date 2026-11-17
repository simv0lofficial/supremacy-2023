#pragma once
#include "acceleration.hpp"
#include "collision.hpp"
#include "skeet/movement_config.hpp"
#include <array>

namespace skeet::ui {
struct RenderQueue;
}
namespace skeet::esp {
struct Projection;
}
namespace skeet {
struct MovementCommandBindings;
namespace client {
struct Command;
}

struct QuickPeekPosition {
    MovementVector value{};
    bool valid = false;
};
struct QuickPeekHistory {
    std::int32_t command_number = 0;
    MovementVector position{};
    bool unavailable = false;
    bool grounded = false;
};
struct QuickPeekState {
    std::uint8_t phase = 0; // native quick-peek subobject +78, states 0..3
    std::uint32_t saved_buttons = 0;
    bool released = false, shot = false;
    QuickPeekPosition origin; // +84 vector, +90 validity
    MovementVector retreat_start{}; // +94
    float start_time = 0, stop_time = 0; // +A0, +A4
    std::uint32_t retreat_buttons = 0; // +A8
    std::int32_t shot_counter = 0; // +AC
    float distance = 0; // +B0
    std::array<QuickPeekHistory, 32> history{}; // +B4, native stride14
};

// These are decoded records returned by the native command-history lookup
// 433952F0, whose 150 slots are keyed by command number.
struct QuickPeekShotRecord {
    std::int32_t command_number;
    std::uint32_t flags;
    float shot_time;
    bool fired;
};
struct QuickPeekFrame {
    MovementPhysics physics;
    MovementVector position, velocity, hull_minimum, hull_maximum;
    float current_time, player_next_attack, weapon_next_attack;
    std::int32_t command_number, last_acknowledged_command, shot_counter;
    std::uint32_t original_buttons, trace_mask;
    std::uint32_t player_entity;
    bool history_unavailable;
    bool mark_command;
    std::span<const QuickPeekShotRecord> shot_records;
};

enum class QuickPeekContinuation {
    movement_features,
    complete
};

// Constructor writes at 434029C6..434029E0 and reset at 433F496E..433F4988.
void initialize_quick_peek_state(QuickPeekState& state) noexcept;
void reset_quick_peek_state(QuickPeekState& state) noexcept;
// Prediction wrappers 4334659B and 43328EE7 commit this counter after the
// command stage, so the next command can observe a newly fired shot.
void update_quick_peek_shot_counter(QuickPeekState& state, std::int32_t counter) noexcept;

// Native scheduler feature 4337019B..43371D4A, including its 32-command
// origin history, three retreat triggers, collision steering and stop window.
// The caller has already established a local player and active weapon.
// marked_commands is native 43476090's shared 150-slot command-number table.
QuickPeekContinuation process_quick_peek(const QuickPeekConfig& config, InputBitmap input,
                                         QuickPeekState& state, const QuickPeekFrame& frame,
                                         UserCommand& command, MovementAngleView movement_angles,
                                         std::uint32_t& scheduler_flags,
                                         std::span<std::uint32_t, 150> marked_commands,
                                         MovementCollisionEngine& collision,
                                         std::uint8_t* engine_trace_flag);
QuickPeekContinuation process_live_quick_peek(const QuickPeekConfig&, InputBitmap, QuickPeekState&,
                                              void* player, void* weapon, client::Command&,
                                              std::uint32_t original_buttons, MovementAngleView,
                                              std::uint32_t& scheduler_flags,
                                              const MovementCommandBindings&);

// Render-interface block 43384C27..43384C78; complete disc geometry helpers
// 4337DCA0 and 43397246. Uses the same color descriptor shown by QuickPeekMenu.
void draw_quick_peek(const QuickPeekConfig& config, InputBitmap input, const QuickPeekState& state,
                     const esp::Projection& projection, ui::RenderQueue& queue);

} // namespace skeet
