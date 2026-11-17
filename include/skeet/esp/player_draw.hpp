#pragma once
#include "player_runtime.hpp"
#include "player_overlay.hpp"
#include "skeleton.hpp"
#include "weapon_icon.hpp"
#include "weapon_text.hpp"
#include "offscreen.hpp"
#include "planted_bomb.hpp"
#include "skeet/engine/interface_call.hpp"
#include <cstddef>

namespace skeet::script {
struct Bridge;
struct ReferenceStrings;
}
namespace skeet::esp {

struct RuntimeWeapon {
    void* engine_entity = nullptr;
    std::uint16_t definition = 0;
    std::int32_t ammunition = -1;
    bool pin_pulled = false;
};
struct WeaponInformation {
    float penetration, range, range_modifier;
    std::int32_t kind;
};
struct PlayerWeaponMetadata {
    std::uint16_t definition = 38, padding = 0;
    float penetration = 2.5f, range = 8192.0f, range_modifier = 0.98f;
    float armor_ratio = 1.65f, damage = 80.0f;
    std::int32_t kind = 1;
};
static_assert(sizeof(PlayerWeaponMetadata) == 28);
struct PlayerSkeletonState {
    bool model_present = false, studio_present = false;
    bool counter_present = false;
    std::int32_t entity_counter = 0, global_counter = 0;
    std::span<const SkeletonBone> bones;
    std::span<const Matrix3x4> transforms;
};
struct PlayerDrawState {
    std::int32_t active_weapon_handle = -1;
    std::array<std::int32_t, 2> inventory_handles{-1, -1};
    std::int32_t money = 0;
    Vector3 view_offset{};
    Vector3 network_origin{}; // decoder 2A671B42, distinct from interpolated origin
    PlayerFlagState flags;
    PlayerSkeletonState skeleton;
    // Decoded damage/health binding for this same engine player. The world
    // dispatcher selects the actual effective view player's binding itself.
    PlantedBombPlayer bomb_target{};
    // Decoded FB1E8DAB spotted-byte snapshot.
    std::uint8_t spotted = 0;
    void* spotted_destination = nullptr; // actual entity netvar address, required by a bound proxy
};
struct SpottedProxyData {
    // Native explicitly initializes the integer +8 and type tag +0x14 to 1.
    // Remaining receive-proxy packet storage is unspecified in the owner.
    std::array<std::uint32_t, 11> words{};
};
using SpottedProxy = void (*)(const SpottedProxyData*, void* entity, void* destination);
struct PlayerDrawEngine {
    const PlayerRuntimeEngine& players;
    void* context = nullptr;
    RuntimeWeapon* (*weapon_by_handle)(void*, std::int32_t) = nullptr; // entity-list slot 4
    bool (*weapon_is_weapon)(void*, const RuntimeWeapon&) = nullptr; // weapon slot 166
    WeaponInformation (*weapon_information)(void*, const RuntimeWeapon&) = nullptr; // slot 461
    float (*weapon_armor_ratio)(void*, const RuntimeWeapon&) = nullptr; // slot 434
    std::int32_t (*weapon_damage)(void*, const RuntimeWeapon&) = nullptr; // slot 431
    std::int32_t (*weapon_capacity)(void*, const RuntimeWeapon&) = nullptr; // slot 375
    std::int32_t (*player_health)(void*, const RuntimePlayer&) = nullptr; // slot 122
    // IVEngineClient::GetPlayerInfo. Native consumes its first 20 name bytes.
    bool (*player_info)(void*, std::int32_t, std::string_view&) = nullptr;
    bool (*setup_bones)(void*, const RuntimePlayer&, std::int32_t maximum, std::uint32_t mask,
                        float time) = nullptr; // renderable slot 13
    WeaponIconProvider icons{};
    WeaponTextProvider names{};
    SpottedProxy spotted_proxy = nullptr; // *0x43477B48, optional actual RecvProxyFn
    std::int32_t (*weapon_kind)(void*, const RuntimeWeapon&) = nullptr; // slot 455
};
struct PlayerDrawRules {
    const PlayerRuntimeRules& players;
    // Integer array decoded by 9BB80163, indexed by networkable index.
    std::span<const std::int32_t> dormant_health;
};
struct PlayerFlagGlobals {
    // Actual pointer-to-pointer globals. The native helper checks both levels
    // before reading the decoded game-rules and player-resource members.
    void** const& game_rules; // borrowed global4346A3A4
    void** const& player_resource; // borrowed global43468FF0
    std::ptrdiff_t rules_guard; // D000A932, byte
    std::ptrdiff_t rules_state; // 82F5AD08, byte
    std::ptrdiff_t carrier_index; // 23B532EF, int32
    const std::uint8_t* rage_enabled; // 43467224
    const std::int32_t* on_shot_target; // 4346DB1C
    const std::uint8_t* on_shot_active; // 4347CA30
};
using PlayerSequenceActivity = engine::MemberFunction<std::int32_t, std::int32_t>;
struct PlayerReloadBindings {
    std::ptrdiff_t layers; //B538BBA5, data pointer+0/count+C
    const PlayerSequenceActivity& sequence_activity; //borrowed optional4346A364
};
// Complete433CEFB3/433880F9. The actual engine callback may change cycle;
// the saved layer's +64 cycle is read only after that callback returns.
float player_reload_cycle(const RuntimePlayer&, const PlayerReloadBindings&);
// Registry/rules gate embedded in 43417468, before the second index query.
std::int32_t bomb_carrier_index(const PlayerFlagGlobals&) noexcept;
// Full native helpers 433C652F, 43328CA6 and 433C8C4D.
std::int32_t overlay_player_health(const RuntimePlayer*, const PlayerDrawEngine&,
                                   const PlayerDrawRules&);
PlayerWeaponMetadata player_weapon_metadata(const RuntimePlayer&, const PlayerDrawState&,
                                            const PlayerDrawEngine&);
// 433FC78E, including x86 shift masking for the hitgroup argument.
bool armored_hitgroup(std::int32_t armor, bool helmet, std::int32_t hitgroup) noexcept;
void set_player_spotted(const RuntimePlayer&, PlayerDrawState&, SpottedProxy); // 433C11EB
// 4340A270..4340A44D. Other weapons retain the native number threshold 92.
std::int32_t
player_health_number_threshold(const RuntimePlayer& target, const PlayerDrawState& target_state,
                               const RuntimePlayer& view, const PlayerDrawState& view_state,
                               const PlayerDrawEngine& engine, client::PlayerStateCache& cache);

// Input projection retained for separately recovered pure leaf helpers. The
// ordinary ESP owner uses PlayerDrawContext and EnginePlayers below.
struct PlayerDrawSnapshotContext {
    const Projection& projection;
    const PlayerDrawRules& rules;
    const PlayerDrawEngine& engine;
    client::PlayerStateCache& cache;
    const PlayerFlagGlobals& flag_globals;
    const float& camera_fov; //actual4346D548, borrowed across frame callbacks
    const PlayerReloadBindings& reload;
    Vector3 view_origin{};
    Point screen{};
    float view_pitch = 0, view_yaw = 0, animation_time = 0, current_time = 0;
    const std::uint8_t& money;
    bool suspended = false;
    std::span<const FlagEntry, 18> flag_entries;
    script::Bridge* scripts = nullptr;
    script::ReferenceStrings* script_flags = nullptr;
};
struct PlayerDrawContext {
    const Projection& projection;
    client::PlayerStateCache& cache;
    const float& camera_fov;
    const std::uint8_t& money;
    FlagTable& flags;
    const WeaponIconProvider& icons;
    const WeaponTextProvider& names;
    script::Bridge* scripts = nullptr;
    script::ReferenceStrings* script_flags = nullptr;
};
// Full 43417468 binding: actual index/dormancy/weapon/time calls and shared
// ring/correction storage, followed by the independently recovered bit rules.
std::uint32_t player_overlay_flags(const RuntimePlayer&, const PlayerDrawState&,
                                   const PlayerDrawSnapshotContext&);
// The caller has applied prepare_player. This consumes every overlay setting
// in native order, then skeleton/radar and renderer restoration. Snapshot
// fields above are decoded netvars/cache data, not substituted decisions.
void draw_prepared_player(ui::RenderQueue&, const PlayerEspConfig&, const WorldEspConfig&,
                          const PreparedPlayer&, PlayerDrawState& target_state,
                          const PlayerDrawState& view_state, const PlayerDrawSnapshotContext&);

} // namespace skeet::esp
