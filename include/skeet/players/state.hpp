#pragma once
#include <array>
#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/engine/startup_convars.hpp"

namespace skeet::players {

struct Adjustments {
    std::uint8_t whitelist = 0;
    std::uint8_t disable_visuals = 0;
    std::uint8_t allow_shared_esp = 0;
    std::uint8_t high_priority = 0;
    std::uint8_t force_pitch = 0;
    std::uint8_t force_body_yaw = 0;
    std::uint8_t correction_active = 0;
    std::uint8_t reserved = 0;
    std::int32_t body_yaw = 0;
    std::int32_t pitch = 0;
    std::uint32_t prefer_body_aim = 0;
    std::uint32_t safe_point = 0;
};
static_assert(sizeof(Adjustments) == 24);
static_assert(offsetof(Adjustments, whitelist) == 0);
static_assert(offsetof(Adjustments, disable_visuals) == 1);
static_assert(offsetof(Adjustments, allow_shared_esp) == 2);
static_assert(offsetof(Adjustments, high_priority) == 3);
static_assert(offsetof(Adjustments, force_pitch) == 4);
static_assert(offsetof(Adjustments, force_body_yaw) == 5);
static_assert(offsetof(Adjustments, correction_active) == 6);
static_assert(offsetof(Adjustments, body_yaw) == 8);
static_assert(offsetof(Adjustments, pitch) == 12);
static_assert(offsetof(Adjustments, prefer_body_aim) == 16);
static_assert(offsetof(Adjustments, safe_point) == 20);

// Native index i has list_row at 4347BE04 + i*32 and adjustments at
// 4347BE08 + i*32. The final word is outside the native reset/copy range.
struct Record {
    std::int32_t list_row = 0;
    Adjustments adjustments;
    std::uint32_t trailing = 0;
};
static_assert(sizeof(Record) == 32 && offsetof(Record, adjustments) == 4);
struct State {
    std::array<Record, 65> records{};
    std::uint8_t* settings_base() noexcept {
        return reinterpret_cast<std::uint8_t*>(&records[0].adjustments);
    }
    const std::uint8_t* settings_base() const noexcept {
        return reinterpret_cast<const std::uint8_t*>(&records[0].adjustments);
    }
};

struct Offsets {
    std::ptrdiff_t connected; // decoded 43467CB8 / 4346A814
    std::ptrdiff_t team; // decoded 434684F0 / 4346A558
    std::ptrdiff_t local_team; // decoded 434688B0 / 4346A518
};
struct LiveBindings {
    void* const& engine;
    void* const& entity_list;
    void* const& local_player;
    void** const& player_resource;
    engine::NativeConVarReference teammates_are_enemies;
    const std::int32_t* const& maximum_clients;
    const std::uint8_t& shared_esp_gate_a;
    const std::uint8_t& shared_esp_gate_b;
    engine::DecodedOffset connected, team, local_team;
};
struct Bindings {
    void* engine = nullptr; // 4346A514, IVEngineClient
    void* entity_list = nullptr; // 4346A890
    void* local_player = nullptr; // 43475808
    const void* player_resource = nullptr; // *43468FF0
    const void* teammates_are_enemies = nullptr; // decoded 4346A568
    std::int32_t maximum_clients = 0; // *43469300
    std::uint8_t shared_esp_gate_a = 0; // 43467F98
    std::uint8_t shared_esp_gate_b = 0; // 43468598
    Offsets offsets{};
    // Production reads the canonical cells at the native use sites. The
    // decoded fields above remain the explicit standalone fixture profile.
    const LiveBindings* live = nullptr;
    void* engine_instance() const noexcept {
        return live ? live->engine : engine;
    }
    void* entity_list_instance() const noexcept {
        return live ? live->entity_list : entity_list;
    }
    void* local_player_instance() const noexcept {
        return live ? live->local_player : local_player;
    }
    const void* resource_instance() const noexcept {
        if (!live)
            return player_resource;
        const auto holder = live->player_resource;
        return holder ? *holder : nullptr;
    }
    const void* teammates_variable() const {
        return live ? live->teammates_are_enemies.get() : teammates_are_enemies;
    }
    std::int32_t client_limit() const noexcept {
        return live ? *live->maximum_clients : maximum_clients;
    }
    bool shared_esp_enabled() const noexcept {
        return live ? (live->shared_esp_gate_a && live->shared_esp_gate_b)
                    : (shared_esp_gate_a && shared_esp_gate_b);
    }
    std::ptrdiff_t connected_offset() const {
        return live ? std::ptrdiff_t(live->connected) : offsets.connected;
    }
    std::ptrdiff_t team_offset() const {
        return live ? std::ptrdiff_t(live->team) : offsets.team;
    }
    std::ptrdiff_t local_team_offset() const {
        return live ? std::ptrdiff_t(live->local_team) : offsets.local_team;
    }
};
struct ListEntry {
    std::int32_t player_index;
    std::u16string name;
};
struct PlayerList {
    std::vector<ListEntry> entries;
    std::uint32_t selected_row = 0;
    bool active = false;
    bool shared_esp_enabled = false;
};

// Complete enumeration/filter/name/mapping part of 4338936B. Names use the
// first 32 engine bytes, replace CR/LF with spaces, then decode CP_UTF8.
PlayerList build_list(State&, const Bindings&, std::int32_t selected_player);
// 433303C1 resets exactly the 24 adjustment bytes of listed, existing players.
bool reset_all(State&, const Bindings&);
// 4334EB4A preserves each recipient's high-priority byte while copying 24 bytes.
bool apply_to_all(State&, const Bindings&, std::uint32_t selected_player);

} // namespace skeet::players
