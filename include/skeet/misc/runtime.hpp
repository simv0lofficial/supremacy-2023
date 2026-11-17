#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <vector>
#include "skeet/misc/logs.hpp"
#include "skeet/misc/clan_tag.hpp"
#include "skeet/misc/ping_spike.hpp"
#include "skeet/misc/auto_accept_owner.hpp"
#include "skeet/client/commands.hpp"

namespace skeet::misc {
class RebuyFixRuntime;
class QuickSwitchRuntime;
class RevealRanksRuntime;

struct PlayerInfo {
    std::array<std::byte, 0x158> bytes{};
    const char* name() const noexcept {
        return reinterpret_cast<const char*>(bytes.data() + 0x10);
    }
    bool is_hltv() const noexcept {
        return bytes[0x13D] != std::byte{};
    }
};

struct RuntimeInterfaces {
private:
    void* standalone_local_player_ = nullptr;

public:
    RuntimeInterfaces() noexcept : local_player(standalone_local_player_) {}
    explicit RuntimeInterfaces(void*& canonical_local_player) noexcept
        : local_player(canonical_local_player) {}
    RuntimeInterfaces(const RuntimeInterfaces&) = delete;
    RuntimeInterfaces& operator=(const RuntimeInterfaces&) = delete;
    RuntimeInterfaces(RuntimeInterfaces&&) = delete;
    RuntimeInterfaces& operator=(RuntimeInterfaces&&) = delete;
    void* engine = nullptr; // 0x4346A514
    void* entity_list = nullptr; // 0x4346A890
    void* cvar = nullptr; // 0x4346A87C
    void* allocator = nullptr; // 0x43467204
    void*& local_player; // live0x43475808; standalone storage for isolated callers
    void** game_rules = nullptr; // 0x4346A3A4
    void** player_resource = nullptr; // 0x43468FF0
    std::int32_t* max_clients = nullptr; // *0x43469300
    void* name_cvar = nullptr; // 0x43467A5C ^ 0xBA5BE8A8
    const std::uint8_t* drawing_suspended = nullptr; //canonical643364E6.value
    ColoredLogRing* const* log_hud_slot = nullptr; //canonical434758CC
};

struct RuntimeOffsets {
    std::uint32_t team = 0; // *0x434688B0 ^ *0x4346A518 ^ 0x8B5586F6
    std::uint32_t valve_matchmaking = 0; // *0x43468378 ^ *0x43468244 ^ 0xBF567619
    std::uint32_t competitive_wins = 0; // *0x43467C54 ^ *0x43467BF0 ^ 0x8C7BA015
    std::uint32_t game_phase = 0; // *0x4346926C ^ *0x43468300 ^ 0xA5085C33
};

// The three imported operations retain their original boundaries. The defaults
// call GetCommandLineA, sprintf_s, and WideCharToMultiByte(CP_UTF8) respectively.
// Verification replaces only these OS/CRT imports, never native game helpers.
struct RuntimeImports {
    const char* (*command_line)();
    int (*format_name_command)(char*, std::size_t, const char*);
    int (*append_name_suffix)(char*, int);
    RuntimeImports();
};

struct TabletSavedFlags {
    void* variable;
    std::uint32_t flags;
};

class Runtime {
public:
    Runtime(RuntimeInterfaces& interfaces, AutoAcceptPatchState&, client::ChannelHookState&,
            RuntimeOffsets offsets = {});
    Runtime(const Runtime&) = delete;
    Runtime& operator=(const Runtime&) = delete;
    Runtime(Runtime&&) = delete;
    Runtime& operator=(Runtime&&) = delete;
    RuntimeInterfaces& interfaces;
    RuntimeOffsets offsets;
    RuntimeImports imports;
    AutoAcceptPatchState& auto_accept_patch; //same live cells as owner/preserved initializer
    client::ChannelHookState& channel; //canonical43475B08/0C/14 shared by CreateMove/network
    const std::uint8_t* auto_accept_enabled = nullptr; // *(0x43475970)+0x14, configuration-owned
    const std::uint8_t* upgrade_tablet_enabled = nullptr; // 0x43478C6C, configuration-owned
    std::int32_t last_name_index = 0; // 0x4346D4B0
    std::vector<TabletSavedFlags> tablet_flags; // 0x43478DE0..0x43478DE8
    ErrorLog error_log; // 0x43416C1C, used for the non-Valve-server message
    ClanTagRuntime clan_tag;
    PingSpikeRuntime ping_spike;
    RebuyFixRuntime* rebuy_fix = nullptr; // independent owner 4348217D
    QuickSwitchRuntime* quick_switch = nullptr; // independent owner4347DC0A
    RevealRanksRuntime* reveal_ranks = nullptr; // independent owner4347E8A5

    void auto_accept_changed(); // complete 0x433ADE97
    void ping_spike_changed(bool enabled); // 0x4336F7B8 and complete 0x433CBDFD
    void restore_ping_channel(); // complete 0x43365AA1
    void steal_player_name(); // 0x4337D6CE and complete 0x433CECA3
    void dump_matchmaking_wins(); // 0x4334C4E0 and complete 0x433CB313
    void upgrade_tablet_changed(); // 0x4338028B and complete 0x433E8612
};

} // namespace skeet::misc
