#pragma once
#include "config.hpp"
#include "other_config.hpp"
#include "indicators.hpp"
#include "projection.hpp"
#include "player_geometry.hpp"
#include "skeet/render_queue.hpp"
#include "skeet/client/player_state.hpp"
#include <array>

namespace skeet::esp {
struct PlayerDrawState;

struct RuntimePlayer {
    void* engine_entity = nullptr;
    std::uint8_t life_state = 0, move_type = 0;
    std::int32_t observer_mode = 0, observer_handle = -1;
    std::int32_t team = 0, squad = -1;
    // Raw bytes from decoders D070684C and CDE290D2 respectively.
    std::uint8_t eligibility_state = 0, suppressed = 0;
    float pose_parameter_1 = 0, pose_parameter_12 = 0;
    // Concrete decoded netvars/cache bound to this engine entity. Required by
    // the complete frame renderer; preparation alone does not consume it.
    PlayerDrawState* draw = nullptr;
};
struct RuntimeRulePlayer {
    bool connected = false, alive = false;
    std::int32_t team = 0;
};
struct PlayerRuntimeRules {
    std::int32_t game_mode = 0;
    bool teammates_are_enemies = false;
    bool present = false;
    // The scalar flag read by 0x433DF22C; bypasses dormant alpha decay.
    bool retain_dormant_alpha = false;
    std::span<const RuntimeRulePlayer> players;
};
struct PlayerRuntimeEngine {
    void* context = nullptr;
    RuntimePlayer* local_player = nullptr;
    RuntimePlayer* (*by_index)(void*, std::int32_t) = nullptr;
    RuntimePlayer* (*by_handle)(void*, std::int32_t) = nullptr;
    bool (*is_player)(void*, const RuntimePlayer&) = nullptr; // entity slot 158
    std::int32_t (*index)(void*, const RuntimePlayer&) = nullptr; // network slot 10
    bool (*dormant)(void*, const RuntimePlayer&) = nullptr; // network slot 9
    Vector3 (*absolute_origin)(void*, const RuntimePlayer&) = nullptr; // entity slot 10
    Vector3 (*minimum_bounds)(void*, const RuntimePlayer&) = nullptr; // entity slot 76
    Vector3 (*maximum_bounds)(void*, const RuntimePlayer&) = nullptr; // entity slot 77
    float (*time)(void*) = nullptr; // engine slot 14
};
struct PlayerRuntimeState {
    // Native 0x433B8A2B chooses records with entity_index & 63. This is the
    // geometry/fade segment at each record +0x1E8, not its other subsystems.
    client::PlayerStateCache& cache;
    explicit PlayerRuntimeState(client::PlayerStateCache& shared) : cache(shared) {}
    PlayerGeometry& geometry(std::uint32_t index) {
        return cache.geometry(index);
    }
};
struct PlayerRuntimeTiming {
    float frame_time = 0, tick_interval = 0;
};
enum class PreparedPlayerStage : std::uint32_t {
    skipped,
    overlay,
    observer_panel
};
struct PreparedPlayer {
    PreparedPlayerStage stage = PreparedPlayerStage::skipped;
    RuntimePlayer* player = nullptr;
    RuntimePlayer* view_player = nullptr;
    PlayerGeometry* geometry = nullptr;
    std::int32_t entity_index = 0;
    bool dormant = false, teammate = false;
    client::PlayerState* cache_state = nullptr;
};

// Complete helpers 43359AFA, 4339EF8C, 433E4E4E and 433D7563.
RuntimePlayer* effective_view_player(const PlayerRuntimeEngine& engine);
bool live_enemy(const RuntimePlayer* first, const RuntimePlayer* second,
                const PlayerRuntimeRules& rules) noexcept;
bool dormant_enemy(const RuntimePlayer* first, const RuntimePlayer* second,
                   const PlayerRuntimeEngine& engine, const PlayerRuntimeRules& rules);
bool dormant_alive(const RuntimePlayer* player, const PlayerRuntimeEngine& engine,
                   const PlayerRuntimeRules& rules);
// 4341AF4A, including the repeated maximum-bounds engine query.
void update_player_geometry(PlayerGeometry& geometry, const RuntimePlayer& player,
                            const PlayerRuntimeEngine& engine, Vector3 origin);

// Per-player owner chain 4340983F..43409D3D with rejection exits AE82..AE9C.
// A returned overlay stage hands the selected entity and modified renderer
// state to the existing offscreen/projection/overlay blocks. Network index is
// nonzero for an accepted player, matching the native engine contract.
PreparedPlayer prepare_player(ui::RenderQueue& queue, PlayerRuntimeState& state,
                              const PlayerEspConfig& config, const WorldEspConfig& world,
                              const PlayerRuntimeEngine& engine, const PlayerRuntimeRules& rules,
                              PlayerRuntimeTiming timing, RuntimePlayer* initial_view_player,
                              std::int32_t slot, bool activation, bool blocked);
// Common post-draw renderer restoration 4340AE66..4340AE80.
void finish_player_overlay(ui::RenderQueue& queue);

// The source frame and glow/sound callbacks share4346A65C. Authored standalone
// helper states keep their explicit local value; production binds its byte.
class PlayerFrameActivation {
public:
    PlayerFrameActivation(bool value = false) noexcept : local_(value) {}
    explicit PlayerFrameActivation(std::uint8_t& cell) noexcept : native_(&cell) {}
    operator bool() const noexcept {
        return native_ ? *native_ != 0 : local_;
    }
    PlayerFrameActivation& operator=(bool value) noexcept {
        if (native_)
            *native_ = static_cast<std::uint8_t>(value);
        else
            local_ = value;
        return *this;
    }

private:
    bool local_ = false;
    std::uint8_t* native_ = nullptr;
};
struct PlayerFrameState {
    std::int32_t transient = 0; // owner +0x6A0
    float timer = 0; // owner +0x6A4
    PlayerFrameActivation activation; //global4346A65C in source production
};
// 43408EE3..43408F34, before the optional timed bomb-site indicator.
void begin_player_frame(PlayerFrameState& state, const HotkeyBinding& binding, InputBitmap input,
                        float absolute_frame_time);
using RuntimeBombSites = std::array<Vector3, 2>;
// Complete 433AC842: site nearest the entity's bounds midpoint, default A.
char runtime_bomb_site(const PlayerRuntimeEngine& engine, std::int32_t entity_index,
                       const RuntimeBombSites* sites);
// Remaining frame prefix 43408F34..43408FB6, including its three-second ring.
void draw_player_frame_notice(ui::RenderQueue& queue, const PlayerFrameState& state,
                              const WorldEspConfig& world, const PlayerRuntimeEngine& engine,
                              std::int32_t entity_index, const RuntimeBombSites* sites,
                              std::int32_t& indicator_cursor, const IndicatorLayout& layout,
                              std::int32_t texture);

} // namespace skeet::esp
