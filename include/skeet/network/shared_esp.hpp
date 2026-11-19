#pragma once
#include "skeet/client/rejected_player_update.hpp"
#include "skeet/esp/player_geometry.hpp"
#include "skeet/script/bridge.hpp"
#include "skeet/events/event_name_storage.hpp"
#include "skeet/engine/startup_convars.hpp"
#include <array>

namespace skeet::engine {
struct NativeOffsetStorage;
}
namespace skeet::network {

struct SharedEspPacket {
    std::array<std::byte, 20> bytes;
    // The sender belongs to the voice envelope, not the encrypted payload.
    std::int32_t sender_index;
};
struct SharedEspFields {
    std::uint16_t marker;
    std::int32_t tick;
    std::uint32_t entity_index, yaw;
    esp::Vector3 origin;
};
bool decode_shared_esp_packet(SharedEspPacket, const client::RejectedPlayerUpdateOwner&,
                             std::uint32_t sender_account, std::uint32_t sender_user_id,
                             SharedEspFields&) noexcept;

struct SharedEspOffsets {
    std::ptrdiff_t life_state;
    std::ptrdiff_t connected;
    std::ptrdiff_t alive;
    std::ptrdiff_t resource_team;
    std::ptrdiff_t survival_team;
    std::ptrdiff_t pose_parameters;
    std::ptrdiff_t rule_time;
};
SharedEspOffsets shared_esp_offsets(const engine::NativeOffsetStorage&);

// All changing outer slots and permission bytes are borrowed from the same
// owners as outgoing shared ESP, PLAYERS, lag history and the ESP frame.
struct SharedEspRuntime {
    client::RejectedPlayerUpdateOwner* const& owner; //434758D0
    void* const& local_player; //43475808
    void* const& engine; //4346A514
    void* const& entity_list; //4346A890
    void** const& player_resource; //43468FF0
    void** const& game_rules; //4346A3A4
    engine::NativeConVarReference game_type; //live encoded43467A8C
    engine::NativeConVarReference teammates_are_enemies; //live encoded4346A568
    const players::State& adjustments;
    client::PlayerStateCache& cache;
    const std::uint8_t& receive_enabled; //43467F98
    const std::uint8_t& permission_gate; //43468598
    const float& tick_interval; // *4346A338
    const float& engine_interval; //43467B04+20
    script::Bridge& script;
    events::EventNameRef voice_event_name; //434795F0
    SharedEspOffsets offsets;
};

// Complete433D5BCF. True suppresses the original engine voice handler,
// including recognized packets rejected by permission/validation checks.
bool receive_shared_esp(SharedEspRuntime&, void* voice_message);
//433E90B7 ->433BA24E: publishes the actual unrecognized message pointer.
void dispatch_voice_message(SharedEspRuntime&, void* voice_message);
// Complete433FD2C4: shared decoder first, otherwise real original slot24.
bool process_voice_message(SharedEspRuntime&, void* receiver, void* voice_message);
void bind_shared_esp_runtime(SharedEspRuntime&) noexcept;
#if defined(_M_IX86) || defined(__i386__)
bool __fastcall process_voice_message_hook(void* receiver, void*, void* voice_message);
#else
bool process_voice_message_hook(void* receiver, void*, void* voice_message);
#endif
} // namespace skeet::network
