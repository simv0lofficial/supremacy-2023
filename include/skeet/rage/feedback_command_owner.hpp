#pragma once
#include "skeet/client/command_frame.hpp"
#include "skeet/rage/shot_feedback.hpp"

namespace skeet::rage {
// Native4346DC2C is the command interface at feedback owner+8, registered
// by4347F721/43345EBC. Its+44/+48 vector is the same full owner's+4C/+50
// pending vector used by shot publication, reconciliation and RAGE selection.
// This interface view owns no second queue and must not outlive feedback.
struct FeedbackCommandOwner {
    ShotFeedbackState& feedback;
    const std::int32_t& last_outgoing_command; // dereferenced43475B2C
    const std::uint8_t& send_gate; // global4346754C; distinct from command send byte
};

// Complete4340C927. A choked frame extends the last command of each pending
// shot whose first command lies strictly inside(last outgoing,current).
void process_feedback_command_phase4(FeedbackCommandOwner&, const client::CommandFrame&) noexcept;
// Complete4331E7AC. Under the process send gate, only the newest pending shot
// with a matching first command receives the current raw three-word origin.
void process_feedback_command_phase6(FeedbackCommandOwner&, const client::CommandFrame&) noexcept;
} // namespace skeet::rage
