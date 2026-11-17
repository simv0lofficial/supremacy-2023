#pragma once
#include "skeet/rage_geometry.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace skeet::client {
struct EntityCommandOwner;
}
namespace skeet::prediction {
// Exact returned payload of 433952F0. This is a view of the existing 150-slot
// command-history owner, not a second producer or replacement history store.
struct CommandHistoryState {
    std::array<std::byte, 0x10> field_00;
    RageVector recoil_angles; // 10
    RageVector recoil_velocity; // 1C
    std::array<std::byte, 0x30> field_28;
    std::uint32_t weapon_identity; // 58
    std::array<std::byte, 4> field_5c;
    float timestamp; // 60
    std::array<std::byte, 9> field_64;
    std::uint8_t fired; // 6D
    std::array<std::byte, 2> field_6e;
    float shot_time; // 70
    std::array<std::byte, 0x14> field_74;
    std::uint32_t flags; // 88
};
struct CommandHistoryEntry {
    std::int32_t command_number;
    CommandHistoryState state;
};
static_assert(sizeof(CommandHistoryState) == 0x8C);
static_assert(sizeof(CommandHistoryEntry) == 0x90);
static_assert(offsetof(CommandHistoryState, recoil_angles) == 0x10);
static_assert(offsetof(CommandHistoryState, recoil_velocity) == 0x1C);
static_assert(offsetof(CommandHistoryState, weapon_identity) == 0x58);
static_assert(offsetof(CommandHistoryState, timestamp) == 0x60);
static_assert(offsetof(CommandHistoryState, fired) == 0x6D);
static_assert(offsetof(CommandHistoryState, shot_time) == 0x70);
static_assert(offsetof(CommandHistoryState, flags) == 0x88);
struct CommandHistoryView {
    //43475898 contains the canonical source EntityCommandOwner projection.
    //Read its current state.history; the semantic owner has no raw+6C prefix.
    static CommandHistoryView from_owner(const client::EntityCommandOwner* owner) noexcept;
    // Native signed remainder and first-dword equality; actual command
    // numbers select readable slots in their engine-owned storage.
    const CommandHistoryState* find(std::int32_t number) const noexcept {
        if (!owner_present_)
            return nullptr;
        const auto& entry = entries_[number % 150];
        return entry.command_number == number ? &entry.state : nullptr;
    }

private:
    const CommandHistoryEntry* entries_ = nullptr;
    //433952F0 tests the owner, not its allocation. An existing owner with a
    //null ring must not be silently treated as an absent owner.
    bool owner_present_ = false;
};
} // namespace skeet::prediction
