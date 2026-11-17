#pragma once

// Penetration & Autowall Declarations
#include "skeet/rage_damage.hpp"
#include <cstddef>
#include "skeet/engine/offset_storage.hpp"
#include "skeet/engine/startup_convars.hpp"
#include "skeet/players/state.hpp"
#include "skeet/rage/ballistics.hpp"


// Declarations from ballistics.hpp

namespace skeet {
using BallisticVector = std::array<float, 3>;
using BallisticEntity = std::uint32_t;

// The x86 engine trace is retained verbatim; entity and surface-name values are
// engine handles/addresses, never host pointers. Offsets follow 0x433AF7D5.
struct BallisticTrace {
    BallisticVector start{}, end{}, plane_normal{};
    float plane_distance = 0;
    std::uint8_t plane_type = 0, plane_signs = 0;
    std::uint16_t plane_padding = 0;
    float fraction = 1;
    std::uint32_t contents = 0;
    std::uint16_t displacement_flags = 0;
    std::uint8_t all_solid = 0, start_solid = 0;
    float fraction_left_solid = 0;
    std::uint32_t surface_name = 0;
    std::int16_t surface_property = 0;
    std::uint16_t surface_flags = 0;
    std::int32_t hitgroup = -1;
    std::int16_t physics_bone = 0;
    std::uint16_t physics_padding = 0;
    BallisticEntity entity = 0;
    std::int32_t hitbox = -1;
};
static_assert(sizeof(BallisticTrace) == 0x54);
static_assert(offsetof(BallisticTrace, fraction) == 0x2c);
static_assert(offsetof(BallisticTrace, entity) == 0x4c);

struct BallisticQuery {
    std::uint8_t allow_penetration = 0;
    std::array<std::uint8_t, 3> padding{};
    std::uint32_t flags = 0;
    float damage = 0;
    std::uint16_t weapon_id = 0, weapon_padding = 0;
    float penetration = 0, range = 0, range_modifier = 0, armor_ratio = 0;
    float base_damage = 0;
    std::uint32_t reserved = 0;
    std::int32_t remaining_penetrations = 0;
    BallisticTrace trace{};
    std::uint32_t point_count = 0;
    std::array<BallisticVector, 8> points{};
};
static_assert(offsetof(BallisticQuery, trace) == 0x2c);
static_assert(offsetof(BallisticQuery, point_count) == 0x80);

enum class BallisticFilterKind {
    supplied,
    skip_source,
    skip_source_and_hit,
    world_only,
    reject_entities,
    reject_players
};
struct BallisticFilter {
    void* object = nullptr;
    BallisticEntity preferred_entity = 0; // native filter +0x14
    std::uint32_t mask = 0, exit_mask = 0; // +0x18/+0x1c
    float minimum_damage = 0; // +0x20
    BallisticFilterKind kind = BallisticFilterKind::supplied;
    // Built-in4346EEBC/43470404 retain the genuine43475B90 setters' writes
    //between traces. Supplied engine ABI objects keep their own prefix.
    mutable std::array<std::uint32_t, 5> native_fields_04{};
};
struct BallisticRay {
    BallisticVector start{}, delta{};
    bool swept = false;
};
struct BallisticTraceRequest {
    BallisticRay ray;
    std::uint32_t mask = 0;
    BallisticFilterKind filter_kind = BallisticFilterKind::supplied;
    const BallisticFilter* filter = nullptr;
    BallisticEntity source = 0, skip = 0;
};

// Snapshots of engine reads. The oddly named virtual predicates retain their
// proven vtable offsets rather than assigning speculative class names.
struct BallisticEntityInfo {
    bool virtual_278 = false, virtual_280 = false, dormant = false;
    bool target_index_blocked = false;
    std::uint8_t target_state = 0, helmet = 0;
    std::int32_t armor = 0, team = 0, survival_team = -1;
    BallisticVector origin{};
    float minimum_z = 0, maximum_z = 0;
};
struct BallisticBreakableInfo {
    bool primary_class_matches = false, alternate_class_matches = false;
    std::uint8_t primary_class_state = 0, collision_group = 0;
    std::int32_t solid_type = 0, health = 0;
    std::uint32_t solid_flags = 0, effects = 0;
    float first_scale = 0, second_scale = 0;
};
struct BallisticModelInfo {
    std::int32_t entity_type = 0, model_type = 0;
    bool model_present = false, studio_model_present = false;
    std::uint16_t activity_flags = 0;
};
struct BallisticRules {
    std::int32_t game_type = 0;
    bool teammates_are_enemies = false;
};

// Only actual engine access is bound: trace/contents/surface, entity virtual
// getters and the engine's IsBreakableEntity callback. No ballistic decisions
// or exit-search/penetration callbacks are delegated to the host.
class BallisticEngine {
public:
    virtual ~BallisticEngine() = default;
    virtual std::int32_t filter_type(const BallisticFilter&) = 0;
    virtual void bind_source(const BallisticFilter&, BallisticEntity) = 0;
    virtual void bind_last_hit(const BallisticFilter&, BallisticEntity) = 0;
    virtual bool accepts(const BallisticFilter&, BallisticEntity, std::uint32_t mask) = 0;
    virtual std::uint32_t point_contents(const BallisticVector&, std::uint32_t mask,
                                         bool world_only) = 0;
    virtual void trace_ray(const BallisticTraceRequest&, BallisticTrace&) = 0;
    virtual void clip_ray_to_entity(const BallisticRay&, std::uint32_t mask, BallisticEntity,
                                    BallisticTrace&) = 0;
    virtual PenetrationSurface surface(std::int16_t property) = 0;
    virtual BallisticEntity entity(std::int32_t index) = 0;
    virtual std::int32_t maximum_clients() = 0;
    virtual BallisticEntityInfo entity_info(BallisticEntity) = 0;
    //433F1FA6 reads9D920E35;43329D2B reads the distinct CDE290D2 byte.
    virtual BallisticEntityInfo clip_entity_info(BallisticEntity) = 0;
    virtual BallisticBreakableInfo breakable_info(BallisticEntity) = 0;
    virtual BallisticModelInfo model_info(std::int32_t model_index) = 0;
    virtual std::uint8_t damage_mode(BallisticEntity) = 0;
    virtual void set_damage_mode(BallisticEntity, std::uint8_t) = 0;
    virtual bool has_engine_breakable_callback() = 0;
    virtual bool engine_breakable(BallisticEntity) = 0;
    virtual BallisticRules rules() = 0;
};

// Native owners: 433FCFD4, 433D7353, 4339EF8C, 433AF7D5, 4339E047,
// 433F1FA6/43408020 and 43329D2B respectively.
void reset_ballistic_query(BallisticQuery&) noexcept;
bool ballistic_breakable(BallisticEngine&, BallisticEntity);
bool ballistic_enemy(BallisticEngine&, BallisticEntity source, BallisticEntity target);
bool trace_ballistic_exit(BallisticEngine&, const BallisticVector& direction,
                          const BallisticTrace& entry, const BallisticFilter&, BallisticTrace& exit,
                          BallisticEntity source, float step, float maximum_distance);
float ballistic_exit_budget(float damage, float penetration, const PenetrationSurface&,
                            const BallisticTrace&, float filter_minimum) noexcept;
// Native return sense: true stops the shot, false continues after an exit.
bool handle_ballistic_penetration(BallisticEngine&, const BallisticVector& direction,
                                  BallisticVector& position, BallisticEntity source,
                                  const BallisticFilter&, BallisticQuery&,
                                  const PenetrationSurface&, float& damage);
void clip_ballistic_entities(BallisticEngine&, const BallisticVector& start,
                             const BallisticVector& end, float minimum_distance,
                             float fraction_limit, const BallisticFilter&, BallisticTrace&);
std::int32_t trace_ballistic_bullet(BallisticEngine&, BallisticEntity source,
                                    const BallisticVector& start, const BallisticVector& target,
                                    const BallisticFilter&, BallisticQuery&);
} // namespace skeet

// Declarations from engine_ballistics.hpp

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

// Declarations from point_penetration.hpp

namespace skeet {
// Complete 433C0AE9 world-only penetration-to-point loop. minimum_damage is
// the native signed stack integer; the effective threshold is max(1,value-5).
// query.allow_penetration controls penetration independently of that threshold.
// The optional byte is the engine-state pointer at 434683D0, temporarily zeroed
// through 433AC5BD and restored on every return, including nested calls.
std::int32_t trace_point_penetration(BallisticEngine&, const BallisticVector& start,
                                     const BallisticVector& target, std::int32_t minimum_damage,
                                     BallisticQuery&, std::uint8_t* suppression_flag = nullptr);
} // namespace skeet
