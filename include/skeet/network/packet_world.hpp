#pragma once
#include "packet_control.hpp"
#include "skeet/esp/player_geometry.hpp"
#include "skeet/engine/startup_convars.hpp"
#include "skeet/client/player_state.hpp"
#include <span>

namespace skeet::network {
struct Bounds {
    Vector minimum, maximum;
};
struct PacketBoxQuery {
    void* context;
    // Resolved external engine geometry function stored at 4346A7CC. The
    // native adapter 4332B611 passes moving bounds in ECX, target bounds in
    // EDX, movement by stack and expansion in XMM3. Both calls share the target.
    bool (*function)(void*, const Bounds& moving, const Bounds& target, Vector& movement,
                     float expansion);
};
class PacketPlayerGeometry {
public:
    PacketPlayerGeometry(std::span<const esp::PlayerGeometry, 64> value) : decoded_(value.data()) {}
    PacketPlayerGeometry(std::array<esp::PlayerGeometry, 64>& value) : decoded_(value.data()) {}
    explicit PacketPlayerGeometry(client::PlayerStateCache& value) : source_(&value) {}
    const esp::PlayerGeometry& operator[](std::uint32_t index) const {
        return source_ ? source_->geometry(index) : decoded_[index & 63u];
    }

private:
    const esp::PlayerGeometry* decoded_ = nullptr;
    client::PlayerStateCache* source_ = nullptr;
};
struct PacketWorld {
    void* entity_list;
    void* local_player;
    std::ptrdiff_t rejection_byte; // 9D920E35
    std::ptrdiff_t origin; // A9EE6062
    std::ptrdiff_t wind_velocity; // 31042F87
    engine::ConVarBinding hull_radius; // ConVar 434679D0
    engine::ConVarBinding standing_height; // ConVar 43468338
    engine::ConVarBinding wind_scale; // ConVar 434671CC
    engine::ConVarBinding game_type; // ConVar 43467A8C
    engine::ConVarBinding game_mode; // ConVar 4346A42C
    const void* wind_class; // actual ClientClass identity 43467E68
    // Shared cache segment at record +1E8; indexed by network index &63.
    PacketPlayerGeometry player_geometry;
    PacketBoxQuery box_query;
};
Bounds transformed_bounds(const Matrix3x4&, Vector minimum, Vector maximum) noexcept; // 433F0EC2
Bounds entity_bounds(void* entity); // complete 43327EF7, actual collideable virtuals
Bounds standing_bounds(Vector origin, float radius, float height) noexcept; // 433ECBBE
// Complete 4332C670: target/dormant gates, real collideable bounds, wind-entity
// lookup/cache, game-mode expansion and both calls of the external geometry primitive.
bool target_collision_trigger(FakeLagState&, const PacketFrame&, Vector predicted_origin,
                              std::int32_t target_handle, PacketWorld&, bool& collision_result);
} // namespace skeet::network
