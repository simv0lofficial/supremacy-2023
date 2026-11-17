#pragma once
#include "player_draw.hpp"
#include "skeet/engine/offset_storage.hpp"
#include "skeet/engine/startup_convars.hpp"
#include "skeet/hud/observer_panel.hpp"
#include <unordered_map>

namespace skeet::esp {
// Borrow the actual pointer cells: engine callbacks may replace their values.
struct PlayerEngineBindings {
    void* const& entity_list; //4346A890
    void* const& engine_client; //4346A514
    void* const& local_player; //43475808
    void** const& player_resource; //43468FF0
    void** const& game_rules; //4346A3A4
    engine::NativeConVarReference game_type; //43467A8C^AC9299CC
    engine::NativeConVarReference game_mode; //4346A694^D9328889, distinct from game_type
    engine::NativeConVarReference teammates_are_enemies; //4346A568^9190E2E6
    const std::int32_t* const& bone_counter; //43469060
    const float* const& current_time; //43467EB8
    const void* const& engine_globals; //43467B04, absframe+8/frame+14/tick+20
    const float* const& simulation_tick_interval; //4346A338
    const Vector3& view_origin; //43479CCC/CD0/CD4
    const float& view_pitch; //43479CD8
    const float& view_yaw; //43479CDC
    const float* const& animation_time; //4346A334
    const std::uint8_t& suspended; //43468E58
    const std::uint8_t& rage_enabled; //43467224
    const std::int32_t& on_shot_target; //4346DB1C
    const std::uint8_t& on_shot_active; //4347CA30
    const PlayerSequenceActivity& sequence_activity; //4346A364, optional genuine engine entry
    const std::int32_t* const& maximum_players; //43469300
    const engine::NativeOffsetStorage& offsets;
};

// Identity views contain no decoded player snapshots. Every field below is
// read from the live entity at its native use site. The only shared player
// history/fade/definition storage remains client::PlayerStateCache.
class EnginePlayers {
public:
    explicit EnginePlayers(PlayerEngineBindings);
    EnginePlayers(const EnginePlayers&) = delete;
    EnginePlayers& operator=(const EnginePlayers&) = delete;
    RuntimePlayer* local();
    RuntimePlayer* identity(void* entity) {
        return view(entity);
    }
    RuntimePlayer* by_index(std::int32_t);
    RuntimePlayer* by_handle(std::int32_t);
    RuntimeWeapon* weapon(std::int32_t);
    bool is_player(const RuntimePlayer&) const;
    bool is_weapon(const RuntimePlayer&) const;
    std::uint16_t item_definition(const RuntimePlayer&) const;
    std::int32_t owner_handle(const RuntimePlayer&) const;
    std::int32_t local_index() const;
    bool dormant(const RuntimePlayer&) const;
    std::int32_t index(const RuntimePlayer&) const;
    Vector3 origin(const RuntimePlayer&) const;
    Vector3 minimum(const RuntimePlayer&) const;
    Vector3 maximum(const RuntimePlayer&) const;
    float time() const;
    std::uint8_t life(const RuntimePlayer&) const;
    std::uint8_t movement(const RuntimePlayer&) const;
    std::int32_t observer_mode(const RuntimePlayer&) const;
    std::int32_t observer_target(const RuntimePlayer&) const;
    std::int32_t team(const RuntimePlayer&) const;
    std::int32_t squad(const RuntimePlayer&) const;
    std::uint8_t eligibility(const RuntimePlayer&) const;
    std::uint8_t suppressed(const RuntimePlayer&) const;
    float pose(const RuntimePlayer&, unsigned) const;
    Vector3 network_origin(const RuntimePlayer&) const;
    Vector3 view_offset(const RuntimePlayer&) const;
    std::int32_t armor(const RuntimePlayer&) const;
    bool helmet(const RuntimePlayer&) const;
    bool scoped(const RuntimePlayer&) const;
    float flash(const RuntimePlayer&) const;
    bool vip(const RuntimePlayer&) const;
    bool defusing(const RuntimePlayer&) const;
    float duck_amount(const RuntimePlayer&) const;
    float duck_speed(const RuntimePlayer&) const;
    float simulation_time(const RuntimePlayer&) const;
    std::int32_t active_weapon(const RuntimePlayer&) const;
    std::int32_t inventory_weapon(const RuntimePlayer&, unsigned) const;
    std::int32_t money(const RuntimePlayer&) const;
    std::int32_t health(const RuntimePlayer&) const;
    std::uint16_t definition(const RuntimeWeapon&) const;
    std::int32_t ammunition(const RuntimeWeapon&) const;
    bool pin_pulled(const RuntimeWeapon&) const;
    float reload_cycle(const RuntimePlayer&) const;
    bool is_weapon(const RuntimeWeapon&) const;
    WeaponInformation information(const RuntimeWeapon&) const;
    const void* information_pointer(const RuntimeWeapon&) const;
    float armor_ratio(const RuntimeWeapon&) const;
    std::int32_t damage(const RuntimeWeapon&) const;
    std::int32_t capacity(const RuntimeWeapon&) const;
    std::int32_t kind(const RuntimeWeapon&) const;
    bool player_info(std::int32_t, std::string_view&);
    bool player_info(std::int32_t, hud::ObserverPlayerInfo&);
    std::int32_t game_type() const;
    std::int32_t game_mode() const;
    bool teammates_are_enemies() const;
    void* resource() const;
    bool connected(const void*, std::int32_t) const;
    bool alive(const void*, std::int32_t) const;
    std::int32_t resource_team(const void*, std::int32_t) const;
    std::int32_t resource_health(const void*, std::int32_t) const;
    bool retain_dormant_alpha() const;
    float rule_time() const; //433FC87C
    std::int32_t bomb_carrier() const;
    bool rage_enabled() const noexcept {
        return bindings_.rage_enabled != 0;
    }
    std::int32_t on_shot_target() const noexcept {
        return bindings_.on_shot_target;
    }
    bool on_shot_active() const noexcept {
        return bindings_.on_shot_active != 0;
    }
    float tick_interval() const noexcept;
    float frame_time() const noexcept;
    float absolute_frame_time() const noexcept;
    std::int32_t maximum_players() const noexcept {
        return *bindings_.maximum_players;
    }
    float simulation_tick_interval() const noexcept {
        return *bindings_.simulation_tick_interval;
    }
    const Vector3& view_origin() const noexcept {
        return bindings_.view_origin;
    }
    float view_pitch() const noexcept {
        return bindings_.view_pitch;
    }
    float view_yaw() const noexcept {
        return bindings_.view_yaw;
    }
    float animation_time() const noexcept {
        return *bindings_.animation_time;
    }
    bool suspended() const noexcept {
        return bindings_.suspended != 0;
    }
    void set_spotted(const RuntimePlayer&, SpottedProxy) const;
    void set_spotted(const RuntimePlayer&) const;
    bool spotted(const RuntimePlayer&) const;
    void skeleton(ui::RenderQueue&, const Projection&, const RuntimePlayer&, std::uint32_t) const;
    // SDK query adapter for existing event callbacks; decoded fields remain
    // accessible only through this live owner, never through identity views.
    PlayerRuntimeEngine& runtime_engine();

private:
    std::ptrdiff_t offset(std::uint32_t pointer, std::uint32_t key, std::uint32_t mask) const;
    RuntimePlayer* view(void*);
    PlayerEngineBindings bindings_;
    std::unordered_map<void*, RuntimePlayer> players_;
    std::unordered_map<void*, RuntimeWeapon> weapons_;
    std::array<std::byte, 0x158> player_info_{};
    PlayerRuntimeEngine runtime_{};
};

// Complete43328CA6 using live engine reads, shared by ESP and incoming-fire probes.
PlayerWeaponMetadata player_weapon_metadata(const RuntimePlayer&, EnginePlayers&);

RuntimePlayer* effective_view_player(EnginePlayers&);
bool live_enemy(const RuntimePlayer*, const RuntimePlayer*, EnginePlayers&);
bool dormant_enemy(const RuntimePlayer*, const RuntimePlayer*, EnginePlayers&);
bool dormant_alive(const RuntimePlayer*, EnginePlayers&);
void update_player_geometry(PlayerGeometry&, const RuntimePlayer&, EnginePlayers&, Vector3);
PreparedPlayer prepare_live_player(ui::RenderQueue&, PlayerRuntimeState&, const PlayerEspConfig&,
                                   const WorldEspConfig&, EnginePlayers&, RuntimePlayer*,
                                   std::int32_t, bool, bool);
std::uint32_t player_overlay_flags(const RuntimePlayer&, const PlayerDrawContext&, EnginePlayers&);
void draw_live_prepared_player(ui::RenderQueue&, const PlayerEspConfig&, const WorldEspConfig&,
                               const PreparedPlayer&, const PlayerDrawContext&, EnginePlayers&);
void draw_live_observer(ui::RenderQueue&, PlayerFrameState&, EnginePlayers&, RuntimePlayer*,
                        const PreparedPlayer&);
} //namespace skeet::esp
