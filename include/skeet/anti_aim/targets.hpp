#pragma once
#include "skeet/rage_geometry.hpp"
#include "skeet/rage/targeting.hpp"
#include "skeet/rage/ballistics.hpp"
#include "skeet/movement/prediction.hpp"
#include "skeet/lagcomp/history.hpp"
#include "skeet/esp/player_runtime.hpp"
#include "skeet/anti_aim/target_geometry.hpp"
#include "skeet/anti_aim/movement_capture.hpp"
#include <array>
#include <cstdint>
#include <span>

namespace skeet::esp {
class EnginePlayers;
}
namespace skeet::anti_aim {
// Exact ten-dword publication at native 4346DB14..4346DB38. fields_18 are
// flags (4 means positive damage), maximum damage, shots-to-kill, weapon ID.
struct TargetState {
    std::int32_t sequence = -1;
    float timestamp = 0;
    std::int32_t handle = -1;
    RageVector position{};
    std::array<std::uint32_t, 4> fields_18{0, 0, 0x7FFFFFFFu, 0};
};
static_assert(sizeof(TargetState) == 0x28);

// Complete absent-target publication from 433269C4 / 4335BA8E. Native leaves
// the prior position and timestamp intact while resetting the handle/tail.
void clear_target_selection(TargetState&) noexcept;

struct TargetCandidateOrder {
    float score;
    std::uint32_t next = UINT32_MAX;
};
// The candidate insertion block of433269C4 stops only at an ordered greater
// prior score. Equal and unordered comparisons retain enumeration order.
void insert_target_candidate(std::span<TargetCandidateOrder>, std::uint32_t candidate,
                             std::uint32_t& head) noexcept;

struct TargetPlayerHistory {
    lagcomp::LagTimeline timeline;
    esp::PlayerGeometry geometry;
    float field_524 = 0; // native player record +0x524
    std::int32_t last_damage_tick = 0; // native player record +0x100
};
// The decoded fixture and the actual source cache are distinct stores. A
// source view resolves433B8A2B at each selector lookup and borrows the same
// geometry, ring and publications used by history/ESP; it copies no records.
class TargetPlayerHistoryView {
public:
    explicit TargetPlayerHistoryView(TargetPlayerHistory& value) : decoded_(&value) {}
    explicit TargetPlayerHistoryView(client::PlayerState& value) : source_(&value) {}
    std::uint32_t record_count() const noexcept;
    const lagcomp::LagRecord& at(std::uint32_t age) const noexcept;
    const esp::PlayerGeometry& geometry() const noexcept;
    float prediction_field() const noexcept;
    void last_damage_tick(std::int32_t) const noexcept;

private:
    TargetPlayerHistory* decoded_ = nullptr;
    client::PlayerState* source_ = nullptr;
};
class TargetPlayerHistories {
public:
    TargetPlayerHistories(std::span<TargetPlayerHistory, 64> value) : decoded_(value.data()) {}
    TargetPlayerHistories(std::array<TargetPlayerHistory, 64>& value) : decoded_(value.data()) {}
    explicit TargetPlayerHistories(client::PlayerStateCache& value) : source_(&value) {}
    TargetPlayerHistoryView operator[](std::uint32_t index) const;

private:
    TargetPlayerHistory* decoded_ = nullptr;
    client::PlayerStateCache* source_ = nullptr;
};
struct TargetSelectorOffsets {
    std::uint32_t network_origin; // 0xA9EE6062
    std::uint32_t velocity; // 0x36375C23
    std::uint32_t view_offset; // 0xC246315E
    std::uint32_t inventory; // 0xFC3297F7, first two handles
};
struct TargetPredictionLimit {
    bool local_available, field_bf567619, field_dda99ab5;
    std::int32_t field_b7ea8fda, configured;
};
struct TargetSelectorContext {
    const rage::TargetBindings& targeting;
    void* engine_client; // actual engine-client vtable 14 time, 8 player info
    const std::int32_t* max_clients;
    TargetPlayerHistories histories;
    TargetSelectorOffsets offsets;
    const lagcomp::TickTimingSamples& timing;
    TargetPredictionLimit prediction_limit;
    float interval, receive_interval;
    MovementCollisionEngine& collision;
    BallisticEngine& ballistics;
    std::uint8_t* trace_flag;
};
// Complete native selector433CC1A8. Existing targets survive a scan with no
// penetrable candidates, exactly as in the native owner; no RAGE ranking is used.
void select_penetrable_yaw_target(TargetState&, const TargetSelectorContext&);

struct TargetScoredOffsets {
    std::uint32_t weapon_ammunition; // 0xCA5EC698
    std::uint32_t field_fb7e3404;
    std::uint32_t local_armor, local_helmet;
    std::uint32_t collision_group; // 0xE4E2D1A7
    std::uint32_t move_parent_handle; // decoder0x49D93E62 minus0x18
};
// Actual engine/netvar reads used by the native43470404 trace filter. The
// player/collision-owner traversal and accept/reject decision stay in source.
class TargetTraceEntities {
public:
    virtual ~TargetTraceEntities() = default;
    virtual BallisticEntity base_entity(BallisticEntity) const = 0; // virtual7
    virtual std::int32_t collision_group(BallisticEntity) const = 0;
    virtual bool is_player(BallisticEntity) const = 0; // virtual158
    virtual std::int32_t parent_handle(BallisticEntity) const = 0;
    virtual BallisticEntity by_handle(std::int32_t) const = 0; // entity-list4
};
class RawTargetTraceEntities final : public TargetTraceEntities {
    void* list_;
    TargetScoredOffsets offsets_;

public:
    RawTargetTraceEntities(void* list, TargetScoredOffsets offsets)
        : list_(list), offsets_(offsets) {}
    RawTargetTraceEntities(void* list, std::uint32_t collision_group,
                           std::uint32_t move_parent_handle)
        : list_(list), offsets_{0, 0, 0, 0, collision_group, move_parent_handle} {}
    BallisticEntity base_entity(BallisticEntity) const override;
    std::int32_t collision_group(BallisticEntity) const override;
    bool is_player(BallisticEntity) const override;
    std::int32_t parent_handle(BallisticEntity) const override;
    BallisticEntity by_handle(std::int32_t) const override;
};
// Actual engine ABI object for native43470404. It must outlive each engine
// trace using binding(). The first0x24 bytes retain the native x86 layout.
// engine_vtable is the genuine external table43475B90; its slots1..4 are
// forwarded directly. Slot0 runs433B4EB6's source predicate, then slot0.
struct TargetTraceFilter {
    void** vtable;
    std::array<std::uint32_t, 5> fields_04{};
    std::uint32_t mask, exit_mask;
    float minimum_damage;
    void* const* engine_vtable;
    const TargetTraceEntities* entities;
    std::array<void*, 5> methods;
    TargetTraceFilter(void* const* engine_vtable, const TargetTraceEntities&,
                      std::uint32_t mask = 0x600400b, std::uint32_t exit_mask = 0x600400b,
                      float minimum_damage = 1.0f);
    TargetTraceFilter(const TargetTraceFilter&) = delete;
    TargetTraceFilter& operator=(const TargetTraceFilter&) = delete;
    BallisticFilter binding() noexcept;
#if defined(_M_IX86) || defined(__i386__)
    static bool __thiscall should_hit(void*, BallisticEntity, std::uint32_t);
#else
    static bool should_hit(void*, BallisticEntity, std::uint32_t);
#endif
};
#if defined(_M_IX86) || defined(__i386__)
static_assert(offsetof(TargetTraceFilter, mask) == 0x18);
static_assert(offsetof(TargetTraceFilter, minimum_damage) == 0x20);
#endif
struct TargetScoredInputs {
    const TargetSelectorContext& common;
    const client::Command& command;
    std::uint32_t input_buttons; // AA frame+0x44 (argument vector+0x30)
    float yaw; // AA frame+0x11C, incoming XMM0
    const esp::PlayerRuntimeRules& player_rules;
    std::span<const std::int32_t> dormant_health;
    TargetScoredOffsets offsets;
    TargetGeometryOffsets geometry_offsets;
    const RageVector& local_pose_origin; // native4347C9E0
    const HitboxMatrices& local_pose; // native4347C650, per hitbox
    const RageTrigonometry& trigonometry;
    const std::int32_t* sequence; // pointed global434678F8
    const void* const* clock_owner; // native4346A3A4 -> first entity pointer
    std::uint32_t local_time_offset; // 0x9F8E7F71
    void* const* trace_filter_vtable; // actual external table43475B90
    const TargetTraceEntities& trace_entities;
};
struct TargetScoredContext : TargetScoredInputs {
    const MovementCaptureInput& movement;
    const MovementEnvironmentBindings& movement_environment;
};
// Production uses EnginePlayers' live resource/ConVar reads, without forming
// a detached PlayerRuntimeRules array or a retained Command reference.
struct LiveTargetScoredContext {
    const TargetSelectorContext& common;
    const client::Command& command;
    std::uint32_t input_buttons;
    float yaw;
    esp::EnginePlayers& players;
    TargetScoredOffsets offsets;
    TargetGeometryOffsets geometry_offsets;
    const RageVector& local_pose_origin;
    const HitboxMatrices& local_pose;
    const RageTrigonometry& trigonometry;
    const std::int32_t* sequence;
    const void* const* clock_owner;
    std::uint32_t local_time_offset;
    void* const* trace_filter_vtable;
    const TargetTraceEntities& trace_entities;
    const MovementCaptureInput& movement;
    const MovementEnvironmentBindings& movement_environment;
};
// Explicit decoded semantic input, used independently of raw engine owners.
// Production refresh always takes the mandatory live context above.
struct DecodedTargetScoredContext : TargetScoredInputs {
    const MovementSimulationPlayer& movement_player;
    const MovementSimulationRules& movement_rules;
    const MovementSimulationEnvironment& movement_environment;
};
void select_scored_yaw_target(TargetState&, const TargetScoredContext&); // complete433269C4
void select_decoded_scored_yaw_target(TargetState&, const DecodedTargetScoredContext&);
struct TargetRefreshContext {
    const TargetScoredContext& scored;
    const void* selector_convar; // native43468EA4 XOR0xFCD2DA7A
};
// Shared actual owner4335BA8E. AA and packet consumers must share this state;
// the same sequence skips both native selectors and leaves every field intact.
void refresh_yaw_target(TargetState&, const TargetRefreshContext&);
void refresh_live_yaw_target(TargetState&, const LiveTargetScoredContext&,
                             const void* selector_convar);
void refresh_decoded_yaw_target(TargetState&, const DecodedTargetScoredContext&,
                                const void* selector_convar);

} // namespace skeet::anti_aim
