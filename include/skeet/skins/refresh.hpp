#pragma once
#include "skeet/skins/catalog.hpp"
#include "skeet/engine/interface_call.hpp"
#include <cstddef>
#include <cstdint>

namespace skeet::skins {
struct RefreshGlobals {
    std::uint8_t& full_update; // 43475BD2, shared with network transition
    const float*& current_time; // 4346A334
    float& hud_refresh_at; // 4346A8A0
    const std::uint8_t& hud_refresh; // 4346838B
};
struct RefreshBindings {
    using FindHudElement = engine::MemberFunction<void*, const char*>;
    void* const& engine_client; // live 4346A514
    void* const& entity_list; // live 4346A890
    std::ptrdiff_t life_state; // 43468E78 ^ 434684E4 ^ 9D920E35
    std::ptrdiff_t team; // 434688B0 ^ 4346A518 ^ 8B5586F6
    void* const& local_player; // 43475808
    void* const& hud; // 43468994
    FindHudElement const& find_hud; // 4346A374, actual engine HUD lookup
    void* const& event_manager; // live 43468364, reread after HUD notification
};

// Concrete internal refresh bodies used by the recovered menu callbacks.
// Engine calls remain external; no feature-level invalidation callback is used.
class WeaponRefresh final : public Refresh {
public:
    WeaponRefresh(RefreshGlobals globals, RefreshBindings bindings)
        : globals_(globals), bindings_(bindings) {}
    void invalidate_weapon_models() override; // complete 4334D9E5
    float current_time() const override;
    void schedule_full_update(float time) override;
    std::int32_t agent_team() const override; // complete 433821B6 team query
    void clear_hud_weapon_selection(); // complete 433F11C8
    void process_deferred_hud_refresh(); // 43344A6C..43344A9C
private:
    RefreshGlobals globals_;
    RefreshBindings bindings_;
};
} // namespace skeet::skins
