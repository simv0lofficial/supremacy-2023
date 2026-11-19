#pragma once
#include "ballistics.hpp"
#include "skeet/engine/offset_storage.hpp"
#include "skeet/engine/startup_convars.hpp"
#include "skeet/players/state.hpp"

namespace skeet {
#if defined(_M_IX86) || defined(__i386__)
using EngineBreakableFunction = bool(__thiscall*)(void*);
#else
using EngineBreakableFunction = bool (*)(void*);
#endif

// References are the actual live interface/function cells. The adapter does
// not retain interface snapshots across reconnects or initialization changes.
struct EngineBallisticBindings {
    void* const& engine_trace; //43468304
    void* const& surface_properties; //43469268
    void* const& entity_list; //4346A890
    void* const& model_collision; //43467014, slot4(index)
    void* const& model_info; //4346A614
    void* const& model_cache; //43469330
    const std::int32_t* const& maximum_clients; //43469300
    void* const* const& skip_filter_vtable; //434758B4
    void* const* const& extended_filter_vtable; //43475B90
    const engine::NativeOffsetGlobals& offsets;
    const players::State& player_adjustments; //4347BE08, stride20
    void* const& primary_breakable_class; //4346A440
    void* const& alternate_breakable_class; //43467478
    EngineBreakableFunction const& breakable; //43468F30, optional native callback
    engine::NativeConVarReference game_type; //43467A8C^AC9299CC
    engine::NativeConVarReference teammates_are_enemies; //4346A568^9190E2E6
};

// The concrete production boundary for the recovered bullet/exit/penetration
// owners. Bullet decisions remain in ballistics.cpp; this class packs the
// native ABI, implements the source filter predicates and reads engine fields.
class EngineBallistics final : public BallisticEngine {
    EngineBallisticBindings bindings_;
    BallisticEntityInfo read_entity(BallisticEntity, bool clipping);

public:
    explicit EngineBallistics(const EngineBallisticBindings& bindings) : bindings_(bindings) {}
    // Own the complete engine filter for the whole shot, including source and
    // last-hit setters. A skip_source ray filter cannot be used for a bullet.
    std::int32_t trace_bullet(BallisticEntity source, const BallisticVector& start,
                              const BallisticVector& target, BallisticQuery& query);
    std::int32_t filter_type(const BallisticFilter&) override;
    void bind_source(const BallisticFilter&, BallisticEntity) override;
    void bind_last_hit(const BallisticFilter&, BallisticEntity) override;
    bool accepts(const BallisticFilter&, BallisticEntity, std::uint32_t) override;
    std::uint32_t point_contents(const BallisticVector&, std::uint32_t, bool) override;
    void trace_ray(const BallisticTraceRequest&, BallisticTrace&) override;
    void clip_ray_to_entity(const BallisticRay&, std::uint32_t, BallisticEntity,
                            BallisticTrace&) override;
    PenetrationSurface surface(std::int16_t) override;
    BallisticEntity entity(std::int32_t) override;
    std::int32_t maximum_clients() override;
    BallisticEntityInfo entity_info(BallisticEntity) override;
    BallisticEntityInfo clip_entity_info(BallisticEntity) override;
    BallisticBreakableInfo breakable_info(BallisticEntity) override;
    BallisticModelInfo model_info(std::int32_t) override;
    std::uint8_t damage_mode(BallisticEntity) override;
    void set_damage_mode(BallisticEntity, std::uint8_t) override;
    bool has_engine_breakable_callback() override;
    bool engine_breakable(BallisticEntity) override;
    BallisticRules rules() override;
};
} //namespace skeet
