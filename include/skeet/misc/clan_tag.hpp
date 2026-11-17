#pragma once
#include <array>
#include <cstdint>
#include "skeet/lagcomp/history.hpp"

namespace skeet::misc {
#if defined(_M_IX86) || defined(__i386__)
using SetClanTag = void(__fastcall*)(const char*, const char*);
#else
using SetClanTag = void (*)(const char*, const char*);
#endif
struct ClanTagClock {
    const lagcomp::TickTimingSamples& latency; //canonical43478C70/74/78
    const std::int32_t& client_tick; // *(0x43475A60)+4
    const std::int32_t* const& acknowledged_tick; //live0x43468F9C
    const std::int32_t* const& outgoing_tick; //live0x434670A8
    const float& interval_per_tick; // *0x4346A338
    std::int32_t mean_latency_ticks() const; // complete 0x43385D57
    std::int32_t corrected_client_tick() const; // complete 0x4340C3FA
};
class ClanTagRuntime {
public:
    const std::uint8_t* enabled = nullptr; // class +0x24
    SetClanTag set_tag = nullptr; // 0x43467550
    void* engine = nullptr;
    void** game_rules = nullptr;
    std::uint32_t game_phase_offset = 0; // *0x4346926C ^ *0x43468300 ^ 0xA5085C33
    void* clan_id_cvar = nullptr; // 0x43467128 ^ 0xE78AB46A
    ClanTagClock* clock = nullptr;
    std::uint8_t was_enabled = 0; // class +0x28
    std::uint8_t changed = 0; // class +0x29
    std::int32_t last_frame = 0; // class +0x2C
    void update(); // complete 0x433414C7, receiver is class +8
    void reset() noexcept; // 0x433FC9C4 and 0x433CF2F5
};
} // namespace skeet::misc
