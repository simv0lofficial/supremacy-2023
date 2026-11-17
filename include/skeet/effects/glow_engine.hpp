#pragma once
#include "skeet/engine/offset_storage.hpp"
#include "skeet/players/state.hpp"
#include "skeet/engine/startup_convars.hpp"
#include <cstdint>

namespace skeet::effects {
struct GlowEntityBindings {
    void* const& entity_list; //4346A890
    void* const& local_player; //43475808
    std::int32_t* const& max_clients; //43469300
    void* const& planted_bombs; //434688C0, native vector data+0/count+C
    const std::uint8_t& suspended; //43468E58
    const std::uint8_t& activation; //4346A65C
    engine::NativeConVarReference game_type; //43467A8C^AC9299CC ConVar
    engine::NativeConVarReference teammates_are_enemies; //4346A568^9190E2E6 ConVar
    const players::State& player_adjustments; //4347BE09, native index*20
    const engine::NativeOffsetStorage& offsets;
};

// Raw engine side of43385D9A/433B5B76. Every global cell and encoded offset
// is consumed at its native use; no frame/configuration snapshot is retained.
class GlowEntityEngine {
public:
    explicit GlowEntityEngine(GlowEntityBindings bindings) : bindings_(bindings) {}
    void* local_player() const noexcept {
        return bindings_.local_player;
    }
    bool suspended() const noexcept {
        return bindings_.suspended != 0;
    }
    bool activation() const noexcept {
        return bindings_.activation != 0;
    }
    std::int32_t max_clients() const noexcept {
        return *bindings_.max_clients;
    }
    void* planted_bombs() const noexcept {
        return bindings_.planted_bombs;
    }
    void* by_index(std::int32_t) const;
    void* by_handle(std::int32_t) const;
    std::int32_t entity_limit() const;
    bool life_state(void*) const;
    bool suppressed(void*) const;
    bool eligibility_state(void*) const;
    std::uint32_t player_glow_index(void*) const;
    std::uint32_t weapon_glow_index(void*) const;
    bool bomb_active(void*) const;
    std::uint32_t bomb_glow_offset() const;
    bool enemy(void* local, void* target) const;
    bool disable_visuals(std::uint32_t index) const noexcept;

private:
    std::uint32_t offset(std::uint32_t pointer, std::uint32_t key, std::uint32_t consumer) const;
    GlowEntityBindings bindings_;
};
} //namespace skeet::effects
