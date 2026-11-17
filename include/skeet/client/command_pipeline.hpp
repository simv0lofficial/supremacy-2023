#pragma once
#include "commands.hpp"
#include "command_frame.hpp"
#include <vector>

namespace skeet::client {
// Native constructor-registration primitive 43345EBC. Registration order is
// insertion order; the -4 x86 receiver appends a null entry and is still returned.
void* register_command_phase_owner(std::vector<void*>&, void* owner);
// Inline CreateMove state transitions, shared by its concrete feature phases.
void advance_command_cycle_flags(std::uint32_t& flags, std::int32_t choked) noexcept;
void enforce_command_choke_limit(std::uint8_t& send_packet, std::int32_t choked,
                                 std::int32_t limit) noexcept;
// If any angle contains an infinity or NaN, native clears all three angles.
void sanitize_final_command_angles(Command&) noexcept;
void record_command_shot(std::uint32_t frame_flags, const Command&, std::int32_t last_outgoing,
                         std::int32_t& last_shot) noexcept;

struct SentCommandRecord {
    std::int32_t command_number, adjusted_tick;
};
struct SentCommandHistory {
    std::array<float, 3> angles; // 43476414, normalized only when sending
    std::array<SentCommandRecord, 150> records; // 43476450, stride 8
};
// Production arrays are separated by0x3C, unlike the legacy standalone
// aggregate above. These views retain the two actual source arrays separately.
struct SentCommandHistoryView {
    std::array<float, 3>& angles;
    std::array<SentCommandRecord, 150>& records;
    SentCommandHistoryView(std::array<float, 3>& a, std::array<SentCommandRecord, 150>& r) noexcept
        : angles(a), records(r) {}
    SentCommandHistoryView(SentCommandHistory& history) noexcept
        : angles(history.angles), records(history.records) {}
};
struct ConstSentCommandHistoryView {
    const std::array<float, 3>& angles;
    const std::array<SentCommandRecord, 150>& records;
    ConstSentCommandHistoryView(SentCommandHistoryView history) noexcept
        : angles(history.angles), records(history.records) {}
    ConstSentCommandHistoryView(const SentCommandHistory& history) noexcept
        : angles(history.angles), records(history.records) {}
    ConstSentCommandHistoryView(SentCommandHistory&&) = delete;
    ConstSentCommandHistoryView(const SentCommandHistory&&) = delete;
};
struct SentCommandTiming {
    std::int32_t last_outgoing;
    std::int32_t base_tick; // *43475A60 +4
    bool have_ack;
    std::int32_t acknowledged; // *43468F9C
    bool have_sent;
    std::int32_t sent; // *434670A8
};
// Complete inline 433D53A5..433D5406, including the real recovered
// 433E9160 angle normalization and 4340C3FA adjusted-tick calculation.
void record_sent_command_history(SentCommandHistory&, const Command&, std::uint8_t send_packet,
                                 const SentCommandTiming&) noexcept;
void record_sent_command_history(SentCommandHistoryView, const Command&, std::uint8_t send_packet,
                                 const SentCommandTiming&) noexcept;
} // namespace skeet::client
