#pragma once
#include "player_state.hpp"
#include "skeet/engine/interface_call.hpp"
#include "skeet/esp/other_config.hpp"
#include "skeet/players/state.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/rage/target_relationships.hpp"

namespace skeet::client {

enum class SharedEspPacketCipher {
    legacy_xor,
    rc4_drop128
};

// Semantic source owner of 434758D0. The four lifetime functions are actual
// engine message constructors/destructors discovered by 43485EF1..43485F5C.
// No recovered payload entrypoint is called through these bindings.
struct RejectedPlayerUpdateOwner {
    // Initialized native +24..33, verified against original-client wire captures.
    std::array<std::uint8_t, 16> packet_key{
        0x68, 0x33, 0x05, 0x97, 0x36, 0x06, 0xd4, 0xea,
        0x4f, 0xc4, 0xa4, 0x3e, 0x85, 0xb2, 0xac, 0x0f};
    std::uint32_t mixing_key = 0xc8eac742u; // +34
    std::uint16_t marker = 0x2424; // +74
    float timer_84 = 0; // +84, empty-tick timer owner
    engine::MemberFunction<void> construct_message = nullptr; // +88
    engine::MemberFunction<void> destroy_message = nullptr; // +8C
    engine::MemberFunction<void> construct_receiver = nullptr; // +90, presence gate
    engine::MemberFunction<void> destroy_receiver = nullptr; // +94, presence gate
    std::int32_t last_player = 1; // +98
    float last_update = 0; // +9C
    void* engine_client = nullptr; // +A0
    const void* send_gate_a = nullptr; // +A4, terminal ConVar integer
    const void* send_gate_b = nullptr; // +A8, terminal ConVar integer
    const void* game_type = nullptr; // +AC, terminal ConVar integer
    alignas(8) std::array<std::byte, 0x200> message{}; // +B8
};

struct RejectedPlayerUpdateOffsets {
    std::ptrdiff_t rejected_d070684c, rejected_9d920e35;
    std::ptrdiff_t origin_a9ee6062;
    std::ptrdiff_t rule_d720aea6, rule_time_9f8e7f71;
};
struct RejectedPlayerUpdateContext {
    RejectedPlayerUpdateOwner* const& owner;
    const esp::SharedEspConfig& settings;
    InputBitmap input;
    PlayerStateCache& cache;
    const players::State& players;
    rage::TargetRelationshipBindings targeting;
    void* const& entity_list; // 4346A890
    void** const& game_rules; // 4346A3A4, nullable holder
    const std::int32_t* const& blocked; // *434757DC, required live pointer
    const std::int32_t& maximum_clients; // *43469300
    const float& current_time; // *4346A334
    const float& tick_interval; // *4346A338, history conversion
    const float& engine_tick_interval; // 43467B04 +20, wire timestamp
    SharedEspPacketCipher& packet_cipher; // fullowner+78 dispatch target
    RejectedPlayerUpdateOffsets offsets;
};

// Concrete children 433D7328 and 43400CA0. The live dispatch is switched to
// rc4_drop128 during an update and restored to legacy_xor afterward.
void transform_shared_esp_packet(SharedEspPacketCipher, const RejectedPlayerUpdateOwner&,
                                 std::span<std::byte> packet) noexcept;

// Complete 43384FD6..43385794. The rejected CreateMove path invokes it once
// per three commands, passing the local player in ECX. This sends one shared
// ESP metadata message when the native gates and player scan succeed.
void update_rejected_player(void* player, const RejectedPlayerUpdateContext&);

// Complete BA8 table4346F2D0 slot2=433654AA / slot3=43382555.
// Native ECX is fullowner+18. These switch the same dispatch target used by
// the sender; slot3 consumes no player/delta data and decodes no messages.
void enter_shared_esp_update(const RejectedPlayerUpdateContext&) noexcept;
void leave_shared_esp_update(const RejectedPlayerUpdateContext&) noexcept;

} // namespace skeet::client
