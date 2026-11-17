#pragma once
#include "skeet/engine/owner_registry.hpp"
#include "skeet/misc/config.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace skeet::misc {
#if defined(_M_IX86) || defined(__i386__)
using RevealRanksCommand = void(__cdecl*)(const void* command_arguments);
#else
using RevealRanksCommand = void (*)(const void* command_arguments);
#endif

struct RevealRanksBindings {
    void* cvar; //4346A87C; command-list head at +30, node link at +4
    void* const& context; //live4346D664; optional engine owner, not a replacement callback
    void* const* const& rank_data; //live43468250; either pointer may be null
    void* const* const& game_rules; //live4346A3A4; either pointer may be null
    std::ptrdiff_t valve_matchmaking; //43468378 ^43468244 ^BF567619, byte
};

struct RevealRanksHook {
    RevealRanksCommand* target = nullptr; //4347BDC8 /4347BDD8
    RevealRanksCommand saved = nullptr; //4347BDCC /4347BDDC
    RevealRanksCommand replacement = nullptr; //4347BDD0 /4347BDE0
};

// Native owner4347E8A5 hooks +showscores and +score. Both installed entries
// deliberately call the first saved command callback, as43331FD3 does.
class RevealRanksRuntime {
public:
    RevealRanksRuntime(RevealRanksConfig&, RevealRanksBindings);
    const std::uint8_t* enabled; //canonical30E0CFE7 descriptor value
    RevealRanksBindings bindings;
    std::array<void*, 2> commands{}; //native owner+18/+1C
    std::array<RevealRanksHook, 2> hooks{};

    void install_hooks(); //4347DE21, primary lifecycle slot3
    void restore_hooks(); //433291A8, primary lifecycle slot4
    void on_score_command(const void* command_arguments);
};

// Actual cdecl ConCommand callback installed into both command nodes at+18.
void reveal_ranks_score_command(const void* command_arguments);

// Complete4332A793/43359C70: traverse ICvar's real list, query virtual6,
// and compare the bytewise FNV-1a hash with the native92CA5DF1 final xor.
void* find_reveal_ranks_command(void* cvar, std::uint32_t name_hash);

// Native4347E8A5 composition. configuration must be the unregistered canonical
// owner configuration. Registers the primary owner before its descriptor and
// command lookup. Primary4346DB50 slots0/3/4 destroy/install/restore; the other
// entries are the original return-only433CCE8E. active is434758F0's owner slot.
// Unload's existing slot4 pass restores both commands; final slot0 frees owner.
void initialize_reveal_ranks(engine::OwnerRegistryBinding owners, ConfigRegistries&,
                             RevealRanksConfig& configuration, RevealRanksRuntime*& active,
                             RevealRanksBindings);
} //namespace skeet::misc
