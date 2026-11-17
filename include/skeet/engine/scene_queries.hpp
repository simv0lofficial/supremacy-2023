#pragma once
#include "skeet/movement/engine_collision.hpp"
#include "skeet/rage/engine_ballistics.hpp"
#include "skeet/esp/grenade_engine.hpp"
#include "skeet/esp/grenade_environment.hpp"

namespace skeet::engine {

// One set of actual engine pointer cells is shared by all three adapters.
// The cells and canonical state outlive this owner; interface replacements are
// observed by the adapters at each recovered call site.
struct SceneQueryBindings {
    void* const& engine_trace; //43468304
    void* const& surface_properties; //43469268
    void* const& entity_list; //4346A890
    void* const& static_props; //43467014
    void* const& model_info; //4346A614
    void* const& model_cache; //43469330
    void* const& spatial_partition; //43467F10
    void* const& player_resource_slot; //43468FF0
    const std::int32_t* const& maximum_clients; //43469300
    void* const* const& skip_filter_vtable; //434758B4
    void* const* const& entity_filter_vtable; //43475B90
    const NativeOffsetGlobals& offsets;
    const players::State& player_adjustments;
    void* const& primary_breakable_class; //4346A440
    void* const& alternate_breakable_class; //43467478
    const void* const& smoke_projectile_class; //43468238
    const void* const& base_projectile_class; //43468E30
    const void* const& molotov_projectile_class; //43467E88
    const void* const& inferno_class; //434674E8
    EngineBreakableFunction const& breakable; //43468F30
    NativeConVarReference game_type; //43467A8C^AC9299CC
    NativeConVarReference teammates_are_enemies; //4346A568^9190E2E6
    void* const& grenade_alternate_breakable_class; //43468CA0
    void* const& grenade_slope_exempt_class; //4346719C
    const float& tick_interval; //*4346A338
    const float& frame_interval; //43467B04 pointee+20
    NativeConVarReference gravity_convar; //43467A54^B882FC15
    NativeConVarReference incendiary_duration_convar; //43467048^983081BF
    NativeConVarReference incendiary_max_slope_convar; //43467088^8FD87158
};

// Source composition after the engine cells have been bound. Construction
// retains dependencies only; native initialization and owner registration
// must still execute in their recovered order. No client logic runs here.
class SceneQueries {
public:
    explicit SceneQueries(const SceneQueryBindings&);
    SceneQueries(const SceneQueries&) = delete;
    SceneQueries& operator=(const SceneQueries&) = delete;
    SceneQueries(SceneQueries&&) = delete;
    SceneQueries& operator=(SceneQueries&&) = delete;

    EngineMovementCollision& movement() noexcept {
        return movement_;
    }
    EngineBallistics& ballistics() noexcept {
        return ballistics_;
    }
    esp::EngineGrenades& grenades() noexcept {
        return grenades_;
    }
    const NativeOffsetGlobals& offsets() const noexcept {
        return bindings_.offsets;
    }
    const esp::GrenadeEnvironmentBindings& environment() const noexcept {
        return grenade_environment_;
    }

private:
    SceneQueryBindings bindings_;
    MovementEngineCollisionBindings movement_bindings_;
    EngineBallisticBindings ballistic_bindings_;
    esp::GrenadeEngineBindings grenade_bindings_;
    esp::GrenadeEnvironmentBindings grenade_environment_;
    EngineMovementCollision movement_;
    EngineBallistics ballistics_;
    esp::EngineGrenades grenades_;
};

} // namespace skeet::engine
