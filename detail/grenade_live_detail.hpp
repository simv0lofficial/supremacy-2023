#pragma once
#include "skeet/esp/grenade_live.hpp"
#include "skeet/esp/grenade_live_bindings.hpp"
#include "skeet/esp/grenade_engine.hpp"
#include "skeet/engine/scene_queries.hpp"
#include "skeet/engine/interface_call.hpp"
#include <bit>
#include <cmath>
#include <cstring>
#include <immintrin.h>

namespace skeet::esp::grenade_live_detail {
}
namespace skeet::esp {
struct GrenadeLiveScene::Storage {
    GrenadeLiveBindings bindings;
    EngineGrenades& raw;
    GrenadeLiveOffsets offsets;
    GrenadeAuxiliaryLiveContext auxiliary_context;
    explicit Storage(const GrenadeLiveBindings&);
    bool enemy(void*) const;
    bool game_flag(std::ptrdiff_t) const;
    std::int32_t team(void*);
    bool admitted(std::uint32_t, void* thrower, void* selected);
    void* thrower(void* projectile, std::uint32_t type);
    float damage_scale(std::uint32_t handle, std::uint32_t type);
    GrenadeDamageResult damage(void*, Vector3 explosion, Vector3 distance_origin);
    GrenadeDamageSelection select_damage(Vector3, const IntegerConfig&);
    void assess(GrenadeWarningRecord&, void*, std::uint32_t&);
    void update(GrenadeWarningOwner&, const GrenadeEspConfig&, void*, std::uintptr_t);
    void draw(ui::RenderQueue&, const misc::GrenadePredictionConfig&, const GrenadeEspConfig&,
              GrenadeTrajectoryOwner&, misc::GrenadeCommandState&, GrenadeWarningOwner&);
    void draw_warnings(ui::RenderQueue&, const GrenadeEspConfig&, GrenadeWarningOwner&);
};
}
