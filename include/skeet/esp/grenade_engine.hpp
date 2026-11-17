#pragma once
#include "grenade_warning_owner.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/engine/trace.hpp"

namespace skeet::esp {
struct GrenadeEngineOffsets {
    explicit GrenadeEngineOffsets(const engine::NativeOffsetGlobals&);
    engine::DecodedOffset origin, velocity, initial_velocity, simulation_time;
    engine::DecodedOffset effects, explosion_effect_tick, thrower_handle, owner_handle;
    engine::DecodedOffset damage, collision_group, projectile_flags, inferno_start_tick;
    engine::DecodedOffset team, survival_team, cached_team_valid, cached_team;
    engine::DecodedOffset physics_object; // decodedA249061A minus0C
    engine::DecodedOffset fire_count, fire_active, fire_x, fire_y, fire_z;
    engine::DecodedOffset smoke_active;
};
// Constructs live operands, without sampling any decoded value.
GrenadeEngineOffsets capture_grenade_engine_offsets(const engine::NativeOffsetGlobals&);
struct GrenadeEngineBindings {
    void* const& entity_list; //4346A890
    void* const& engine_trace; //43468304
    void* const& surface_properties; //43469268
    void* const& spatial_partition; //43467F10
    void* const& static_props; //43467014, virtual3 from4333D32B
    void* const* const& skip_filter_vtable; //genuine434758B4
    void* const* const& entity_filter_vtable; //genuine43475B90
    void* const& player_resource_slot; //43468FF0, optional pointer-to-resource
    const void* const& smoke_projectile_class; //43468238
    GrenadeEngineOffsets offsets;
    const void* const& base_projectile_class; //43468E30
    const void* const& molotov_projectile_class; //43467E88
    const void* const& inferno_class; //434674E8
};

// Only genuine engine calls and raw property reads cross this boundary.
// Prediction, damage, collector acceptance and owner policies remain source.
class EngineGrenades final : public GrenadeWarningEngine {
public:
    explicit EngineGrenades(const GrenadeEngineBindings& bindings) : bindings_(bindings) {}
    const GrenadeEngineBindings& bindings() const noexcept {
        return bindings_;
    }
    GrenadeTraceProvider provider(std::uint8_t* trace_flag) noexcept;
    GrenadeDamageTrace trace(const GrenadeRay&, std::uint32_t, const GrenadeDamageFilter&) override;
    std::uint32_t world_entity() override;
    std::uint32_t owner_handle(std::uint32_t) override;
    std::uint32_t entity_for_handle(std::uint32_t) override;
    std::uint32_t physics_object(std::uint32_t) override;
    std::int32_t physics_material(std::uint32_t) override;
    float surface_density(std::int32_t) override;
    bool snapshot_for_handle(std::uint32_t, GrenadeWarningSnapshot&) override;
    bool snapshot_for_index(std::uint32_t, GrenadeWarningSnapshot&) override;
    std::span<const GrenadeWarningPartitionEntity> partition_entities(Vector3, float, std::uint32_t,
                                                                      bool) override;
    // Native433E92B5 collector: class filtering and sixteen accepted pointers.
    // The owner reads their origins/active bytes only after the query returns.
    std::span<void* const> collect_smokes(Vector3, float, std::uint32_t, bool);
    void* by_index(std::int32_t) const;
    void* by_handle(std::uint32_t) const;
    const void* client_class(void*) const;
    std::int32_t index(void*) const;
    bool player(void*) const;
    bool dormant(void*) const;
    Vector3 live_origin(void*) const;
    std::span<const Vector3> fire_cells(void*);

private:
    // Copy the reference-bearing aggregate; callers need not retain a temporary
    // aggregate, while every engine/class/filter cell remains live.
    GrenadeEngineBindings bindings_;
    std::array<void*, 16> smoke_entities_{};
    std::array<GrenadeWarningPartitionEntity, 16> smoke_views_{};
    const void* collected_smoke_class_ = nullptr;
    std::vector<Vector3> cells_;
    bool snapshot(void*, GrenadeWarningSnapshot&);
    static GrenadeTrace trace_projectile(void*, const GrenadeRay&, std::uint32_t,
                                         const GrenadeTraceFilter&);
};
} // namespace skeet::esp
