#pragma once
#include "game_events.hpp"
#include "skeet/misc/logs.hpp"
#include "skeet/misc/config.hpp"
#include "skeet/misc/native_text.hpp"
#include <array>
#include <cstddef>

namespace skeet::events {
struct HitLogPlayerInfo : EventPlayerIndices {
    // Engine vtable +0x20, the native 0x158-byte player_info_t output.
    virtual bool player_info(std::int32_t index, std::array<std::byte, 0x158>& output) = 0;
};
// Complete 4339FF69: sanitize only bell/tab/LF/CR, truncate successful engine
// names at byte 20, or format the player index when the engine query fails.
std::array<char, 128> player_log_name(std::int32_t index, HitLogPlayerInfo&);

struct HitLog {
    HitLog(const misc::EventLogConfig& settings, const misc::NativeEncryptedText& format,
           const std::array<misc::NativeText, 9>& hitgroups)
        : settings(settings), format(format), hitgroups(hitgroups) {}
    const misc::EventLogConfig& settings; // live descriptor0CE14F5A, owner+30
    const misc::NativeEncryptedText& format; // same native owner+34/+38 backing
    const std::array<misc::NativeText, 9>& hitgroups; // same native owner+54..D4
    // 433E0D36, registered for player_hurt. Defaults recovered from 434819D2.
    void player_hurt(GameEventReader&, HitLogPlayerInfo&, misc::NormalLog&);
};
} // namespace skeet::events
