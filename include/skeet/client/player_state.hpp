#pragma once
#include "skeet/engine/jobs.hpp"
#include "skeet/esp/player_geometry.hpp"
#include "skeet/lagcomp/history.hpp"
#include "skeet/resolver_runtime/environment_update.hpp"
#include <array>
#include <cstddef>
#include <cstdint>
#include <optional>

namespace skeet::client {
// Four 0x24-byte native resolver selection slots begin at owner+0x10.
struct PlayerResolverSlot {
    std::uint8_t attempt;
    std::array<std::byte, 0x23> other;
};
static_assert(sizeof(PlayerResolverSlot) == 0x24);

// The original 0x1700-byte owner returned by 433B8A2B. Unknown intervals are
// retained verbatim; proven shared members have one physical home.
struct NativePlayerStateLayout {
    std::uint32_t flags; // 000
    float shared_rule_time; // 004
    float shared_yaw; // 008
    std::int32_t shared_yaw_tick; // 00C
    std::array<PlayerResolverSlot, 4> resolver_slots; // 010
    std::uint8_t resolver_status, resolver_index; // 0A0 / 0A1
    std::array<std::byte, 0xE> field_0a2;
    float probe_time, probe_stamp; // 0B0 / 0B4
    std::int32_t probe_count; // 0B8
    float reference_time; // 0BC
    RageVector reference_origin; // 0C0
    std::array<std::byte, 0x24> field_0cc;
    float environment_yaw; // 0F0
    float environment_yaw_change_time; // 0F4
    std::array<std::byte, 4> field_0f8;
    std::int32_t captured_camera_fov; // 0FC
    std::int32_t last_target_damage_tick; // 100
    std::array<std::byte, 0x2C> field_104;
    float cached_target_height; // 130
    float cached_duck_amount; // 134
    std::array<std::byte, 4> field_138;
    RageVector cached_origin; // 13C
    std::array<std::byte, 0x14> field_148;
    std::int32_t last_update_tick; // 15C
    std::array<std::byte, 4> field_160;
    std::int32_t captured_observer_mode; // 164
    std::array<std::byte, 0x18> field_168;
    std::uint32_t capture_field_180; // 180
    std::array<std::byte, 0x28> field_184;
    float reference_yaw; // 1AC
    std::array<std::byte, 0x38> field_1b0;
    esp::PlayerGeometry geometry; // 1E8
    std::uint32_t cached_esp_flags, cached_esp_frame; // 228 / 22C
    std::uint16_t cached_definition; // 230
    std::array<std::byte, 6> field_232;
    std::uint32_t field_238; // 238
    std::array<std::byte, 0x2E0> field_23c;
    float field_51c; // 51C
    std::array<std::byte, 4> field_520;
    float target_prediction_524; // 524
    std::array<std::byte, 0xAC> field_528;
    std::uint32_t history_first, history_count; // 5D4 / 5D8
    // 32-bit ABI projection of lagcomp::RecordRing, not another ring owner.
    std::array<std::uint32_t, 32> native_history_links; // 5DC
    std::array<std::byte, 4> field_65c;
    lagcomp::LagRecord fallback; // 660
};
static_assert(sizeof(NativePlayerStateLayout) == 0x1700);
static_assert(offsetof(NativePlayerStateLayout, shared_rule_time) == 0x4);
static_assert(offsetof(NativePlayerStateLayout, shared_yaw) == 0x8);
static_assert(offsetof(NativePlayerStateLayout, shared_yaw_tick) == 0xC);
static_assert(offsetof(NativePlayerStateLayout, resolver_status) == 0xA0);
static_assert(offsetof(NativePlayerStateLayout, environment_yaw) == 0xF0);
static_assert(offsetof(NativePlayerStateLayout, environment_yaw_change_time) == 0xF4);
static_assert(offsetof(NativePlayerStateLayout, captured_camera_fov) == 0xFC);
static_assert(offsetof(NativePlayerStateLayout, last_target_damage_tick) == 0x100);
static_assert(offsetof(NativePlayerStateLayout, target_prediction_524) == 0x524);
static_assert(offsetof(NativePlayerStateLayout, captured_observer_mode) == 0x164);
static_assert(offsetof(NativePlayerStateLayout, cached_target_height) == 0x130);
static_assert(offsetof(NativePlayerStateLayout, cached_duck_amount) == 0x134);
static_assert(offsetof(NativePlayerStateLayout, cached_origin) == 0x13C);
static_assert(offsetof(NativePlayerStateLayout, reference_yaw) == 0x1AC);
static_assert(offsetof(NativePlayerStateLayout, geometry) == 0x1E8);
static_assert(offsetof(NativePlayerStateLayout, cached_esp_flags) == 0x228);
static_assert(offsetof(NativePlayerStateLayout, cached_definition) == 0x230);
static_assert(offsetof(NativePlayerStateLayout, field_238) == 0x238);
static_assert(offsetof(NativePlayerStateLayout, field_51c) == 0x51C);
static_assert(offsetof(NativePlayerStateLayout, history_first) == 0x5D4);
static_assert(offsetof(NativePlayerStateLayout, native_history_links) == 0x5DC);
static_assert(offsetof(NativePlayerStateLayout, fallback) == 0x660);

struct PlayerState {
    explicit PlayerState(NativePlayerStateLayout&);
    NativePlayerStateLayout& native;
    esp::PlayerGeometry& geometry;
    float& cached_target_height;
    std::uint16_t& cached_definition;
    lagcomp::LagRecord& fallback;
    // Reuse the existing portable ring, once per player. On x86 its pointers
    // publish directly into native_history_links; host-width links stay here.
    lagcomp::RecordRing history;
    resolver_runtime::EnvironmentOwner environment() const noexcept;
    void environment(const resolver_runtime::EnvironmentOwner&) noexcept;
    lagcomp::PlayerCaptureState capture_state() const noexcept;
    lagcomp::HistoryRecordingState recording_state() const;
    void recording_state(const lagcomp::HistoryRecordingState&);
    void publish_history() noexcept;
};

class PlayerStateCache {
    std::array<NativePlayerStateLayout*, 64> standalone_addresses_{};

public:
    // Only actual RtlAllocateHeap/RtlFreeHeap exports are boundaries. The
    // alignment header, zero-initialization and replacement are recovered code.
    explicit PlayerStateCache(engine::JobMemory memory = engine::native_job_memory());
    // Production source storage is shared with the canonical native-layout
    // terminal views. This constructor accepts only an unowned empty cache.
    PlayerStateCache(engine::JobMemory, std::array<NativePlayerStateLayout*, 64>&);
    ~PlayerStateCache();
    PlayerStateCache(const PlayerStateCache&) = delete;
    PlayerStateCache& operator=(const PlayerStateCache&) = delete;
    PlayerState* get(std::uint32_t entity_index); // complete 433B8A2B, nullable
    PlayerState* find(std::uint32_t entity_index);
    //433AFCE8: drain live rings ascending, release their shared record pool,
    //then free cache owners descending and null each slot after its free.
    void finalize_history(lagcomp::RecordPool&);
    esp::PlayerGeometry& geometry(std::uint32_t entity_index) {
        return get(entity_index)->geometry;
    }
    // Native global 434797BC: index aliases use the low six bits. Public
    // storage also preserves native reentrant replacement during heap calls.
    std::array<NativePlayerStateLayout*, 64>& addresses;

private:
    engine::JobMemory memory_;
    std::array<std::optional<PlayerState>, 64> views_;
    PlayerState* view(std::uint32_t slot);
    NativePlayerStateLayout* allocate(); // 43387448 + inline constructor
    void free(void* object); // complete 433837D0
};
} // namespace skeet::client
