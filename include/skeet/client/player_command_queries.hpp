#pragma once
#include "command_pipeline.hpp"
#include "skeet/engine/unload.hpp"

namespace skeet::client {
struct PlayerCommandQueryBindings {
    const std::uint32_t& update_phase; // canonical43476904
    const std::uintptr_t& forced_query_stack; //434674E0, caller stack+8 word
    const std::uint8_t& team_query_override; //4346907C
    const std::uint8_t& update_mode; //43468E58
    const std::uintptr_t& team_query_return; //4346756C
    std::ptrdiff_t team; //8B5586F6
    void* const& engine_client; //4346A514
    std::uint8_t* const& interpolation_guard; //optional43468FA4
    std::ptrdiff_t rejection_byte; //9D920E35
    const std::int32_t* const& choked_commands; //434757DC
    const std::int32_t* const& last_outgoing; //43475B2C
    void* const& input; //4346A894, CInput+F0
    const std::uintptr_t& sent_angles_return; //43468518
    SentCommandHistoryView sent_history; //43476414 angles/43476450 records
};

class PlayerCommandQueries {
public:
    explicit PlayerCommandQueries(const PlayerCommandQueryBindings& source) noexcept
        : bindings(source) {}
    PlayerCommandQueryBindings bindings;

    // Whole433842C1, local slot78. Four argument words retain their exact
    // stack bits; phase1 suppresses the original four-word engine call.
    void dispatch_update_call(void* player, std::array<std::uint32_t, 4> arguments) const;
    // Whole4334D3EB ->4338257F, local slot88. Native entry copies incoming
    // ESI to EDX; stack_word_08 and return_address are the caller's raw words.
    std::uint32_t query_callsite(void* player, std::uint32_t caller_esi,
                                 std::uintptr_t return_address, std::uintptr_t stack_word_08) const;
    // Whole4340191D, slot113. Always clears the optional guard first; a
    // matching sentinel command returns AL1 without calling the original.
    // time_bits retains the original float stack word without conversion.
    std::uint8_t interpolate(void* player, std::uint32_t time_bits) const;
    // Whole433E97C1, slot170. The matching callsite obtains canonical sent
    // angles directly; every other caller reaches the original engine method.
    std::array<float, 3>* eye_angles(void* player, std::uintptr_t return_address) const;

    // The family installer owns installation. This binds only the four exact
    // captured native operands to their actual source ABI entry points.
    void bind_native_replacement(engine::VtableHookMetadata&, std::size_t slot,
                                 std::uint32_t native_entry);
    static void* native_replacement_entry(std::size_t slot, std::uint32_t native_entry);
};
} // namespace skeet::client
