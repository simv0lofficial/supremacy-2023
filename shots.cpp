// Shot Tracking & Hit Registration Implementation
#include "util/address.h"
#include "skeet/legit/runtime.hpp"
#include "skeet/engine/interface_call.hpp"
#include <cmath>
#include <limits>
#include "skeet/rage/aim_fire_event.hpp"
#include "skeet/rage/acquisition.hpp"
#include "skeet/script/bridge.hpp"
#include "skeet/events/event_name_storage.hpp"
#include <string_view>
#include "skeet/rage/buffered_shot_repair.hpp"
#include "skeet/rage/acquisition_frame.hpp"
#include "skeet/rage/final_target_context.hpp"
#include "skeet/rage/targeting_acquired.hpp"
#include "skeet/rage/runtime_owner.hpp"
#include <bit>
#include "skeet/rage/feedback_command_owner.hpp"
#include <cstring>
#include "skeet/rage/feedback_entity_owner.hpp"
#include "skeet/rage/feedback_owner.hpp"
#include "skeet/client/input_commands.hpp"
#include "skeet/client/native_startup.hpp"
#include "skeet/legit/accuracy.hpp"
#include "skeet/misc/native_text.hpp"
#include <algorithm>
#include <array>
#include <new>
#include <stdexcept>
#include <xmmintrin.h>
#include "skeet/rage/feedback_runtime_owner.hpp"
#include "skeet/rage/pending_shot_publication.hpp"
#include "skeet/rage/spread_storage.hpp"
#include "skeet/client/runtime_storage.hpp"
#include "skeet/client/runtime_composition.hpp"
#include "skeet/client/history_runtime_owner.hpp"
#include "skeet/client/player_runtime_owner.hpp"
#include "skeet/animation/runtime_owner.hpp"
#include "skeet/engine/scene_runtime_owner.hpp"
#include "skeet/engine/decoded_offset.hpp"
#include "skeet/engine/netvar_initialization.hpp"
#include "skeet/engine/build_hitbox_clip_bindings.hpp"
#include "skeet/esp/runtime_owner.hpp"
#include "skeet/script/api_runtime_owner.hpp"
#include "skeet/misc/logs.hpp"
#include <emmintrin.h>
#include "skeet/rage/shot_diagnostics.hpp"
#include "skeet/engine/owner_allocation.hpp"
#include "skeet/engine/native_data_image.hpp"
#include "skeet/client/buffered_shot_lifecycle.hpp"
#include "skeet/rage/shot_feedback.hpp"
#include "skeet/rage/spread_miss_log.hpp"
#include "skeet/rage/spread.hpp"
#include "skeet/rage/settings.hpp"
#include "skeet/engine/unload.hpp"
#include "skeet/rage_shot.hpp"


// ============================================================================
// Subcomponent: first_shot.cpp
// ============================================================================

namespace skeet::legit {
void apply_first_shot_delay(const Settings& settings, ProfileState& state,
                            const FirstShotFrame& frame, const rage::TargetBindings& bindings,
                            InputBitmap input, std::uint32_t& buttons) {
    const auto threshold = state.first_shot_range;
    if (threshold == 0.0f || !(buttons & 1u) || !state.enabled ||
        !hotkey_active(settings.aim_keys[std::size_t(state.profile)].value.binding, input)) {
        state.secondary_flags &= 0xF7u;
        state.previous_buttons = frame.prediction_buttons;
        return;
    }
    if (state.shots != 0)
        state.secondary_flags &= 0xF7u;
    else if (!(state.previous_buttons & 1u)) {
        if (frame.prediction_buttons & 1u)
            state.secondary_flags |= 8u;
        else
            state.secondary_flags &= 0xF7u;
    }
    state.previous_buttons = frame.prediction_buttons;
    if (!(state.secondary_flags & 8u))
        return;
    for (std::int32_t index = 1; index <= frame.maximum_clients; ++index) {
        auto* target = engine::invoke<void*>(bindings.entity_list, 3, index);
        if (!rage::target_is_eligible(bindings, target))
            continue;
        const auto origin = *engine::invoke<const Vector*>(target, 10);
        const auto lower = *engine::invoke<const Vector*>(target, 76);
        const auto upper = *engine::invoke<const Vector*>(target, 77);
        const auto height = upper[2] + lower[2];
        const auto& eye = frame.eye_position;
        const auto& forward = frame.forward;
        const auto horizontal =
            (origin[1] - eye[1]) * forward[1] + (origin[0] - eye[0]) * forward[0];
        for (const auto fraction : {0.55f, 0.75f, 0.9f}) {
            const auto z = fraction * height + origin[2];
            const auto projection = (z - eye[2]) * forward[2] + horizontal;
            float distance;
            if (projection < 0.0f || projection > 10000.0f)
                distance = std::numeric_limits<float>::max();
            else {
                const auto x_delta = origin[0] - (forward[0] * projection + eye[0]);
                const auto y_delta = origin[1] - (forward[1] * projection + eye[1]);
                const auto z_delta = z - (forward[2] * projection + eye[2]);
                distance = std::sqrt((x_delta * x_delta + y_delta * y_delta) + z_delta * z_delta);
            }
            if (threshold > distance)
                return;
        }
    }
    buttons &= 0xFFFFFFFEu;
}
} // namespace skeet::legit

// ============================================================================
// Subcomponent: aim_fire_event.cpp
// ============================================================================

namespace skeet::rage {
namespace {
struct AimFireEvent {
    const TargetCandidate& candidate;
    const AcquisitionEvaluation& evaluation;
    std::int32_t index;
};

void field_key(script::Bridge& bridge, std::string_view name) {
    bridge.api->pushlstring(bridge.state, name.data(), name.size());
}
void field_key(script::Bridge& bridge, events::EventNameRef name, std::size_t length) {
    bridge.api->pushlstring(bridge.state, name.get(), length);
}
void integer_field(script::Bridge& bridge, std::string_view name, std::int32_t value) {
    field_key(bridge, name);
    bridge.api->pushinteger(bridge.state, value);
    bridge.api->settable(bridge.state, -3);
}
void integer_field(script::Bridge& bridge, events::EventNameRef name, std::size_t length,
                   std::int32_t value) {
    field_key(bridge, name, length);
    bridge.api->pushinteger(bridge.state, value);
    bridge.api->settable(bridge.state, -3);
}
void number_field(script::Bridge& bridge, std::string_view name, float value) {
    field_key(bridge, name);
    bridge.api->pushnumber(bridge.state, static_cast<double>(value));
    bridge.api->settable(bridge.state, -3);
}
void number_field(script::Bridge& bridge, events::EventNameRef name, std::size_t length,
                  float value) {
    field_key(bridge, name, length);
    bridge.api->pushnumber(bridge.state, static_cast<double>(value));
    bridge.api->settable(bridge.state, -3);
}
void boolean_field(script::Bridge& bridge, std::string_view name, bool value) {
    field_key(bridge, name);
    bridge.api->pushboolean(bridge.state, value);
    bridge.api->settable(bridge.state, -3);
}
void boolean_field(script::Bridge& bridge, events::EventNameRef name, std::size_t length,
                   bool value) {
    field_key(bridge, name, length);
    bridge.api->pushboolean(bridge.state, value);
    bridge.api->settable(bridge.state, -3);
}

int marshal_aim_fire(script::Bridge& bridge, void* context) { //4332A6B4 /4336A85F
    const auto& event = *static_cast<const AimFireEvent*>(context);
    const auto& candidate = event.candidate;
    const auto& point = candidate.primary;
    bridge.api->createtable(bridge.state, 0, 18);
    integer_field(bridge, "id", event.index);
    integer_field(bridge, "target", static_cast<std::int32_t>(candidate.entity_index));
    number_field(bridge, "x", point.position[0]);
    number_field(bridge, "y", point.position[1]);
    number_field(bridge, "z", point.position[2]);
    integer_field(bridge, bridge.name(events::EventName::hitgroup), 8, point.hitgroup);
    integer_field(bridge, bridge.name(events::EventName::damage), 6, point.damage);
    // Native MULSS rounds to a float before the Lua double conversion.
    const float percent = point.chance * 100.0f;
    number_field(bridge, bridge.name(events::EventName::hit_chance), 10, percent);
    boolean_field(bridge, bridge.name(events::EventName::boosted), 7, (point.flags & 4U) != 0);
    boolean_field(bridge, bridge.name(events::EventName::high_priority), 13, false);
    integer_field(bridge, "tick", event.evaluation.requested_tick);
    boolean_field(bridge, "teleported", false);
    boolean_field(bridge, "interpolated", false);
    boolean_field(bridge, "extrapolated", event.evaluation.extrapolated);
    number_field(bridge, "backtrack", 0.0f);
    return 1;
}
}

int dispatch_aim_fire_event(script::Bridge& bridge, client::PlayerStateCache& players,
                            const client::CommandFrame&, const TargetCandidate& candidate,
                            const AcquisitionEvaluation& evaluation, std::int32_t shot_index) {
    if (!bridge.state)
        return 0;
    (void)players.get(candidate.entity_index); //4340C8ED: result is discarded
    AimFireEvent event{candidate, evaluation, shot_index};
    return script::dispatch_callbacks(bridge, bridge.name(events::EventName::aim_fire).get(),
                                      marshal_aim_fire, &event);
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: buffered_shot_repair.cpp
// ============================================================================

namespace skeet::rage {
namespace {
struct ReconstructionFlag {
    PreambleState& state;
    explicit ReconstructionFlag(PreambleState& owner) : state(owner) {
        state.flags |= 1U;
    }
    ~ReconstructionFlag() {
        state.flags &= ~1U;
    }
};
void initialize_historical_bullet(BallisticQuery& query, const client::CommandBullet& source,
                                  bool penetrate) noexcept {
    // Complete433D0015. Seven raw words are copied after433FCFD4's reset;
    // damage itself remains zero until a concrete ballistic trace starts.
    query.allow_penetration = std::uint8_t(penetrate);
    reset_ballistic_query(query);
    query.weapon_id = source.weapon_id;
    query.weapon_padding = source.padding;
    query.penetration = source.penetration;
    query.range = source.range;
    query.range_modifier = source.range_modifier;
    query.armor_ratio = source.armor_ratio;
    query.base_damage = source.damage;
    query.reserved = source.pellet_count;
}
void clear_mask(std::array<std::uint32_t, 2>& mask, std::uint32_t entity) noexcept {
    const auto index = entity & 63U;
    mask[index >> 5] &= ~(1U << (index & 31U));
}
void group_completed(PreambleState& state, AcquisitionStorage& storage,
                     BufferedShotRepairScratch& scratch) {
    std::array<std::int8_t, 64> groups;
    groups.fill(-1);
    auto& batch = scratch.candidates;
    for (std::uint32_t ordinal = 0; ordinal < 80; ++ordinal) {
        if (!(state.completed[ordinal >> 5] & (1U << (ordinal & 31U))))
            continue;
        auto& candidate = storage.candidates[ordinal];
        const auto entity = candidate.entity_index & 63U;
        if (!(state.flags & 1U))
            state.target_mask[entity >> 5] |= 1U << (entity & 31U);
        const auto group = groups[entity];
        if (group == -1) {
            if (batch.count >= 16 || !scratch.inline_storage)
                continue;
            groups[entity] = std::int8_t(batch.count);
            batch.candidates[batch.count++] = &candidate;
        } else if (candidate.priority > batch.candidates[std::uint32_t(group)]->priority)
            batch.candidates[std::uint32_t(group)] = &candidate;
    }
}
void acquire_repair_target(PreambleState& state, PreambleResult& preamble,
                           AcquisitionStorage& storage, void* target, client::PlayerState& cached,
                           std::uint32_t index, const players::State& adjustments,
                           AcquisitionBranchFrameBindings& current, const client::Command& command,
                           bool body_probe) {
    auto& frame = *preamble.frame;
    AcquisitionTargetSettings settings{preamble.scan_template};
    prepare_acquisition_target_settings(frame, target, cached, index,
                                        adjustments.records[index].adjustments, settings,
                                        current.preparation);
    auto trace = current.body_probe;
    trace.context_flags = state.flags;
    trace.weapon_flags = state.weapon_flags;
    trace.weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
    if (body_probe) {
        if (!(state.flags & 9U))
            clear_mask(state.previous_targets, index);
        probe_body_aim(frame, settings, target, trace);
    }
    const TargetPredictionBindings prediction{trace,
                                              current.prediction.record_selection,
                                              current.prediction.engine_trace,
                                              current.prediction.local_maximum_speed,
                                              current.prediction.scheduler_flags,
                                              &command};
    refine_acquisition_minimum_damage(frame, target, cached, settings, prediction,
                                      current.records.bone_runtime);
    auto ticks = acquisition_target_ticks(frame, target, cached, settings, current.preparation,
                                          current.record_candidates);
    auto records = current.records;
    records.clear_safe_neck = true;
    prepare_and_submit_target_records(state, preamble, storage, target, cached, ticks.values,
                                      settings, ticks.descriptor_flags, records);
}
}
const lagcomp::EvaluationState* repair_buffered_shot(client::CommandFrame& frame, void* local,
                                                     client::Command& command,
                                                     const ShotDiagnosticEntry& diagnostic,
                                                     BufferedShotRepairScratch& scratch,
                                                     const BufferedShotRepairBindings& bindings) {
    if (const auto* source = std::get_if<RuntimeOwner*>(&bindings.source))
        return (*source)->repair_buffered(frame, local, command, diagnostic, scratch);
    auto* owner = std::get<CommandOwner*>(bindings.source);
    if (!owner || (frame.flags & 0x20040U))
        return nullptr;
    auto& state = owner->state;
    auto& storage = owner->acquisition;
    ReconstructionFlag active(state);
    PreambleResult preamble;
    preamble.frame = &frame;
    preamble.command = &command;
    preamble.scan_template = prepare_rage_scan_template(state, frame, owner->preamble);
    preamble.point_scale_is_automatic =
        owner->preamble.weapon_settings.multipoint_scale.value == 24;
    const SpreadParameters parameters{std::uint32_t(frame.weapon_id_d8),
                                      std::int32_t(frame.field_b0),
                                      frame.spread_d0,
                                      frame.inaccuracy_c4,
                                      frame.weapon_field_100,
                                      frame.pellet_count_19c,
                                      owner->preamble.shotgun_patterns,
                                      frame.weapon_kind_124};
    prepare_spread_cache(state.spread, parameters, frame.adjusted_spread_160,
                         std::uint32_t(frame.command_number_ac), (state.flags & 0x10U) != 0,
                         owner->preamble.spread_tables, owner->preamble.random,
                         owner->preamble.pattern, owner->preamble.trigonometry);
    initialize_historical_bullet(scratch.historical_bullet, diagnostic.bullet,
                                 owner->preamble.settings.automatic_penetration.value != 0);
    storage.queued.clear();
    state.completed.fill(0);
    state.evaluation_count = 0;
    AcquisitionFrameBindings current(state, preamble, owner->preamble.prediction_time,
                                     owner->preamble.yaw_target.handle,
                                     owner->preamble.scheduler_flags, owner->single_target,
                                     owner->multiple_targets, owner->settings_menu);
    const bool single = owner->preamble.weapon_settings.target_selection.value == 1;
    bool selected = true;
    if (single) {
        if (!(state.flags & 9U)) {
            state.target_mask.fill(0);
            state.previous_targets.fill(0);
        }
        auto& branch = current.single_frame;
        void* target = select_single_target(
            {branch.preparation.targeting, current.single.maximum_clients, frame.origin_dc,
             frame.direction_1a8, std::bit_cast<float>(preamble.scan_template[7])});
        selected = target != nullptr;
        if (target) {
            const auto index =
                engine::invoke<std::uint32_t>(static_cast<std::byte*>(target) + 8, 10);
            auto& cached = *branch.records.players.get(index);
            acquire_repair_target(state, preamble, storage, target, cached, index,
                                  current.single.adjustments, branch, command, false);
            finish_target_acquisition(state, storage, current.single.jobs, branch.records.players,
                                      current.single.maximum_clients, branch.records.settings_menu);
        }
    } else {
        auto& branch = current.multiple_frame;
        const TargetView view{frame.origin_dc, frame.direction_1a8,
                              branch.preparation.weapon_settings.maximum_fov.value,
                              std::bit_cast<float>(preamble.scan_template[7])};
        for (std::int32_t entity = 1; entity <= current.multiple.maximum_clients; ++entity) {
            auto& cached = *branch.records.players.get(std::uint32_t(entity));
            auto* target =
                engine::invoke<void*>(branch.preparation.targeting.entity_list, 3, entity);
            if (!(state.flags & 9U))
                clear_mask(state.target_mask, std::uint32_t(entity));
            if (!target_is_eligible(branch.preparation.targeting, target) ||
                !target_passes_scan_filter(branch.preparation.targeting, target, view))
                continue;
            const auto index =
                engine::invoke<std::uint32_t>(static_cast<std::byte*>(target) + 8, 10);
            acquire_repair_target(state, preamble, storage, target, cached, index,
                                  current.multiple.adjustments, branch, command, true);
        }
        finish_target_acquisition(state, storage, current.multiple.jobs, branch.records.players,
                                  current.multiple.maximum_clients, branch.records.settings_menu);
    }
    if (selected)
        group_completed(state, storage, scratch);
    auto& scanner = single ? current.single_frame.scanner : current.multiple_frame.scanner;
    recalculate_target_priorities(scratch.candidates, scanner.targeting);
    if (scratch.candidates.count > 1)
        sort_target_candidates(scratch.candidates);
    const FinalTargetContextBindings final{scanner, owner->final_target.current_bone_counter,
                                           owner->settings_menu, owner->final_target.jobs};
    auto choice = select_acquired_target_for_shot(scratch.candidates, storage, frame, state, final);
    for (std::uint32_t ordinal = 0; ordinal < state.evaluation_count; ++ordinal)
        synchronize_evaluation_state(storage.candidates[ordinal], storage.evaluations[ordinal]);
    if (!choice.candidate)
        return nullptr;
    auto& evaluation =
        storage.evaluations[std::size_t(choice.candidate - storage.candidates.data())];
    (void)local; // Native EDX is overwritten at433C1599 before its first read.
    return &synchronize_evaluation_state(*choice.candidate, evaluation);
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: feedback_command_owner.cpp
// ============================================================================

namespace skeet::rage {
void process_feedback_command_phase4(FeedbackCommandOwner& owner,
                                     const client::CommandFrame& frame) noexcept {
    if (!frame.choked_15c)
        return;
    for (auto& record : owner.feedback.pending) {
        const auto first = std::bit_cast<std::int32_t>(record.words[1]);
        if (first > owner.last_outgoing_command && first < frame.command_number_ac)
            record.words[3] = std::uint32_t(frame.command_number_ac);
    }
}

void process_feedback_command_phase6(FeedbackCommandOwner& owner,
                                     const client::CommandFrame& frame) noexcept {
    if (!owner.send_gate || owner.feedback.pending.empty())
        return;
    auto& record = owner.feedback.pending.back();
    if (record.words[1] != std::uint32_t(frame.command_number_ac))
        return;
    std::memcpy(record.words.data() + 9, frame.origin_dc.data(), sizeof(frame.origin_dc));
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: feedback_entity_owner.cpp
// ============================================================================

namespace skeet::rage {
namespace {
const void* convar_parent(std::uint32_t encrypted, std::uint32_t mask) noexcept {
    auto* object = reinterpret_cast<const void*>(std::uintptr_t(encrypted ^ mask));
    for (;;) {
        auto* parent = field<const void*>(object, 0x1C);
        if (parent == object)
            return object;
        object = parent;
    }
}
float convar_float(std::uint32_t encrypted, std::uint32_t mask) noexcept {
    const auto* object = convar_parent(encrypted, mask);
    return std::bit_cast<float>(field<std::uint32_t>(object, 0x2C) ^
                                std::uint32_t(reinterpret_cast<std::uintptr_t>(object)));
}
}
bool FeedbackEntityBindings::enabled(const ShotFeedbackState&) const noexcept {
    return recording_enabled != 0;
}
float FeedbackEntityBindings::now() const noexcept {
    return *current_time;
}
float FeedbackEntityBindings::interval() const noexcept {
    return *tick_interval;
}
float FeedbackEntityBindings::receive_interval() const noexcept {
    return field<float>(receive_clock, 0x20);
}
float FeedbackEntityBindings::lerp() const noexcept {
    const auto update_rate = convar_float(update_rate_convar, 0x03B4819CU);
    const auto interpolation = convar_float(interpolation_convar, 0x7D9DC86FU);
    const auto ratio = convar_float(interpolation_ratio_convar, 0xBE64D223U);
    return maximum_number(interpolation, ratio / update_rate);
}
float FeedbackEntityBindings::correction_minimum() const noexcept {
    return correction_floor;
}
float FeedbackEntityBindings::correction_extra() const noexcept {
    return correction_addend;
}
std::int32_t FeedbackEntityBindings::maximum_unlag_milliseconds() const noexcept {
    return maximum_unlag;
}
bool FeedbackEntityBindings::ray_tests_disabled() const noexcept {
    const auto* object = convar_parent(disable_ray_tests_convar, 0xFCD2DA7AU);
    return field<std::uint32_t>(object, 0x30) !=
           std::uint32_t(reinterpret_cast<std::uintptr_t>(object));
}
void reset_feedback_events(FeedbackEntityOwner& owner) noexcept {
    owner.feedback.recent_position_count = 0;
    owner.feedback.events.feedback.clear();
}
void reconcile_feedback_events(FeedbackEntityOwner& owner) {
    reconcile_shot_feedback(owner.feedback, owner.runtime, owner.bindings);
}
} //namespace skeet::rage

// ============================================================================
// Subcomponent: feedback_owner.cpp
// ============================================================================

namespace skeet::rage {
namespace {
FeedbackPositionRuntime* position_runtime;
/* deduplicated helper */
#if defined(_M_IX86) || defined(__i386__)
#define FEEDBACK_MEMBER __thiscall
#define FEEDBACK_FAST __fastcall
#define FEEDBACK_VECTOR __vectorcall
#else
#define FEEDBACK_MEMBER
#define FEEDBACK_FAST
#define FEEDBACK_VECTOR
#endif
using PrimaryEntry = void(FEEDBACK_MEMBER*)(void*);
using PacketEntry = void(FEEDBACK_FAST*)(void*, const void*);
using PositionEntry = void(FEEDBACK_VECTOR*)(void*, float, float, float);
void FEEDBACK_MEMBER destroy_entry(void*);
void FEEDBACK_MEMBER forward_reset_entry(void*);
void FEEDBACK_MEMBER reset_entry(void*);
void FEEDBACK_MEMBER unchanged_entry(void*) {} //433CCE8E
void FEEDBACK_FAST packet_entry(void*, const void*);
void FEEDBACK_FAST frame_entry(void*, std::uint8_t);
void FEEDBACK_VECTOR position_entry(void*, float, float, float);
const std::array<PrimaryEntry, 9> primary_table{
    destroy_entry,       unchanged_entry, unchanged_entry, unchanged_entry, unchanged_entry,
    forward_reset_entry, unchanged_entry, reset_entry,     unchanged_entry};
const PacketEntry packet_table[]{packet_entry};
const engine::FrameUpdateEntry frame_table[]{frame_entry};
const PositionEntry position_table[]{position_entry};
struct PacketEndpoint {
    const PacketEntry* vtable;
    FeedbackOwner* owner;
};
struct FrameEndpoint : engine::FrameUpdateEndpoint {
    FeedbackOwner* owner;
};
struct PositionEndpoint {
    const PositionEntry* vtable;
    FeedbackOwner* owner;
};

void* allocate(engine::JobMemory memory, std::uint32_t size) { //43387448
    const auto bytes = size + 0x20u;
    auto* raw = memory.allocate(memory.process_heap, 8, bytes);
    if (!raw)
        return nullptr;
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = std::uint32_t(header - address);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &bytes, 4);
    return reinterpret_cast<void*>(header + 16);
}
void free(engine::JobMemory memory, void* object) { //433837D0
    auto* bytes = static_cast<std::byte*>(object);
    std::uint32_t displacement;
    std::memcpy(&displacement, bytes - 8, 4);
    memory.free(memory.process_heap, 0, bytes - 16 - displacement);
}
bool shotgun_patterns(const void* variable) {
    for (;;) {
        const auto* parent = field<const void*>(variable, 0x1C);
        if (parent == variable)
            break;
        variable = parent;
    }
    return field<std::uint32_t>(variable, 0x30) !=
           std::uint32_t(reinterpret_cast<std::uintptr_t>(variable));
}
std::uint32_t packet_round_integer(float value) noexcept {
    std::int64_t integer;
#if (defined(__i386__) || defined(__x86_64__)) && (defined(__GNUC__) || defined(__clang__))
    __asm__ __volatile__("flds %1\n\tfistpq %0" : "=m"(integer) : "m"(value) : "st");
#elif defined(_M_IX86) && defined(_MSC_VER)
    __asm {
        fld dword ptr value
        fistp qword ptr integer
    }
#else
#error Feedback packet quantization requires the native x87 FISTP boundary
#endif
    return std::uint32_t(std::bit_cast<std::uint64_t>(integer));
}
//433FD134 at this packet body's four fixed0..maximum call sites. SSE controls
// scale conversion/retry; the final x87 FISTP uses the current rounding mode.
float packet_quantize(float value, unsigned bits, float maximum) noexcept {
    const auto upper = (std::uint32_t(1) << bits) - 1u;
    const double extent = static_cast<double>(maximum);
    const auto initial =
        _mm_cvtss_f32(_mm_cvtpd_ps(_mm_div_sd(_mm_set_sd(double(upper)), _mm_set_sd(extent))));
    auto scale = initial;
    if (static_cast<double>(scale) * extent > double(upper)) {
        constexpr float factors[]{0.9998999834060669f, 0.9900000095367432f, 0.8999999761581421f,
                                  0.800000011920929f, 0.699999988079071f};
        bool selected = false;
        for (const auto factor : factors) {
            scale = _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(factor), _mm_set_ss(initial)));
            if (static_cast<double>(scale) * extent <= double(upper)) {
                selected = true;
                break;
            }
        }
        if (!selected)
            scale = 0.0f;
    }
    if (value < 0.0f)
        return 0.0f;
    if (value > maximum)
        return maximum;
    const auto scaled = _mm_cvtss_f32(_mm_mul_ss(_mm_set_ss(value), _mm_set_ss(scale)));
    // Indefinite qword's low dword is zero for NaN. The native unsigned
    // low-dword conversion also turns the rounded signed zero into+0.
    const auto integer = packet_round_integer(scaled);
    const auto converted = _mm_cvtss_f32(_mm_cvtpd_ps(_mm_set_sd(double(integer))));
    return _mm_cvtss_f32(_mm_div_ss(_mm_set_ss(converted), _mm_set_ss(scale)));
}
float packet_angle(float value) noexcept {
    const auto scaled = _mm_mul_ss(_mm_set_ss(value), _mm_set_ss(182.04444885253906f));
    const auto wrapped = std::uint32_t(_mm_cvttss_si32(scaled)) & 0xFFFFu;
    const auto degrees = _mm_cvtss_f32(_mm_mul_ss(
        _mm_cvtsi32_ss(_mm_setzero_ps(), std::int32_t(wrapped)), _mm_set_ss(0.0054931640625f)));
    return packet_quantize(degrees, 13, 359.9560546875f);
}
void fire_listener(void*, events::GameEventReader&, events::EventPlayerIndices&);
void hurt_listener(void*, events::GameEventReader&, events::EventPlayerIndices&);
void death_listener(void*, events::GameEventReader&, events::EventPlayerIndices&);
void impact_listener(void*, events::GameEventReader&, events::EventPlayerIndices&);
void require_canonical_bindings(const FeedbackOwnerBindings& bindings) {
    if (&bindings.runtime.spread_log_settings != &bindings.settings ||
        &bindings.packet.recording_enabled != &bindings.live.recording_enabled)
        throw std::invalid_argument(
            "Feedback owner requires its canonical settings and recording gate");
}
}

class FeedbackOwner {
public:
    const PrimaryEntry* vtable = nullptr;
    const FeedbackOwnerBindings& bindings;
    FeedbackCommandOwner commands;
    client::NativeCommandPhaseReceiver command_receiver;
    FeedbackEntityOwner entities;
    client::NativeEntityObserverReceiver entity_receiver;
    network::ExploitShotFeedbackObserver recharge;
    PacketEndpoint packet;
    FrameEndpoint frame;
    PositionEndpoint position;
    void* engine_client; //full native+34, captured at the recorded MOV
    misc::NativeEncryptedText spread_message;
    events::EventListener impact;
    std::uint8_t impact_subscribed;

    explicit FeedbackOwner(const FeedbackOwnerBindings& b)
        : bindings(b), commands{b.state, b.last_outgoing_command, b.live.recording_enabled},
          entities{b.state, b.runtime, b.live}, recharge{b.state, b.last_outgoing_command} {
        b.lifecycle.append(this); //4347F719, primary pointer is still null
        b.commands.append(command_receiver, &commands); //4347F721, native+8
        b.entities.append_feedback(entity_receiver, entities); //4347F729, native+10
        packet.owner = this;
        b.packets.append(&packet); //+18,4347F731
        frame.owner = this;
        engine::register_frame_update(b.frames, &frame); //+20
        position.owner = this;
        b.positions.append(&position); //+28,4347F758
        b.recharge.append(recharge); //4347F760, source table still unpublished
        packet.vtable = packet_table;
        frame.vtable = frame_table; //4347F767/770
        b.recharge.publish(recharge); //4347F777, before primary4347F77E
        vtable = primary_table.data();
        b.commands.publish(command_receiver, client::CommandOwnerId::owner_4346dc2c); //4347F784
        b.entities.publish_feedback(entity_receiver); //4347F78E
        position.vtable = position_table;
        //mem_int3.h:497 records4347F79C MOVEAX,[4346A514].
        engine_client = b.engine_client;
        initialize_shot_log_settings(b.configuration, b.settings);
        b.state.pending.clear();
        b.state.extrapolation_shots.clear();
        b.state.events.has_pending_shots = false;
        constexpr char message[] = "Missed shot due to spread";
        static_assert(sizeof(message) == 26);
        misc::construct_native_encrypted_text(spread_message, message, 25, b.lifecycle.memory);
        b.state.recent_position_count = 0;
        b.state.events.feedback.clear();
        b.state.fire_snapshots.clear();
        impact = {events::bullet_impact_hash, this, impact_listener};
        impact_subscribed = 0;
        b.events.add({events::weapon_fire_hash, this, fire_listener});
        b.events.add({events::player_hurt_hash, this, hurt_listener});
        b.events.add({events::player_death_hash, this, death_listener});
    }
    void toggle_impact(std::uint8_t desired) { //complete4336B697
        if (desired == impact_subscribed)
            return;
        impact_subscribed = desired;
        if (desired)
            bindings.events.add(impact);
        else
            bindings.events.remove(impact);
    }
    void prepare_event() noexcept {
        auto& state = bindings.state;
        state.events.enabled = bindings.live.recording_enabled != 0;
        state.events.has_pending_shots = !state.pending.empty();
    }
    ~FeedbackOwner() { //complete433CDB60; permanent listener lifetime is external
        toggle_impact(0);
        std::deque<ShotFireSnapshot>{}.swap(bindings.state.fire_snapshots);
        std::vector<events::ShotEvent>{}.swap(bindings.state.events.feedback);
        misc::destroy_native_encrypted_text(spread_message, bindings.lifecycle.memory);
        std::vector<ShotFeedbackRecord>{}.swap(bindings.state.pending);
        bindings.state.events.has_pending_shots = false;
    }
};
static_assert(sizeof(FeedbackOwner) <= 0x110);

namespace {
void FEEDBACK_MEMBER destroy_entry(void* object) {
    auto* owner = static_cast<FeedbackOwner*>(object);
    destroy_feedback_owner(owner, owner->bindings.active);
}
void FEEDBACK_MEMBER forward_reset_entry(void* object) { //433B06E3 forwards to7
    auto* owner = static_cast<FeedbackOwner*>(object);
    owner->vtable[7](object);
}
void FEEDBACK_MEMBER reset_entry(void* object) {
    reset_feedback_owner(*static_cast<FeedbackOwner*>(object));
}
void FEEDBACK_FAST packet_entry(void* object, const void* packet) {
    capture_feedback_packet(*static_cast<PacketEndpoint*>(object)->owner, packet);
}
void FEEDBACK_FAST frame_entry(void* object, std::uint8_t update) {
    update_feedback_subscription(*static_cast<FrameEndpoint*>(object)->owner, update);
}
void FEEDBACK_VECTOR position_entry(void* object, float x, float y, float z) {
    capture_feedback_position(*static_cast<PositionEndpoint*>(object)->owner, {x, y, z});
}
void fire_listener(void* object, events::GameEventReader& event,
                   events::EventPlayerIndices& players) {
    auto& owner = *static_cast<FeedbackOwner*>(object);
    owner.prepare_event();
    events::weapon_fire(owner.bindings.state.events, event, players);
}
void hurt_listener(void* object, events::GameEventReader& event,
                   events::EventPlayerIndices& players) {
    auto& owner = *static_cast<FeedbackOwner*>(object);
    owner.prepare_event();
    events::player_hurt(owner.bindings.state.events, event, players);
}
void death_listener(void* object, events::GameEventReader& event,
                    events::EventPlayerIndices& players) {
    auto& owner = *static_cast<FeedbackOwner*>(object);
    owner.prepare_event();
    events::player_death(owner.bindings.state.events, event, players);
}
void impact_listener(void* object, events::GameEventReader& event,
                     events::EventPlayerIndices& players) {
    auto& owner = *static_cast<FeedbackOwner*>(object);
    owner.prepare_event();
    events::bullet_impact(owner.bindings.state.events, event, players);
}
void initialize_feedback_owner(void* context) {
    create_feedback_owner(*static_cast<FeedbackOwnerBindings*>(context));
}
}

FeedbackPacketOffsets feedback_packet_offsets(const engine::NativeOffsetStorage& storage) {
    const auto& g = storage.globals;
    return {{g, 0x43467EC0, 0x434678BC, 0xB6F85C08, 0}, {g, 0x43468910, 0x43468418, 0x2A671B42, 0},
            {g, 0x43468E90, 0x43468F20, 0xE9392423, 0}, {g, 0x43467480, 0x4346850C, 0xC6C62EBA, 0},
            {g, 0x4346A45C, 0x4346A324, 0x6777C9CE, 0}, {g, 0x43468FAC, 0x43467BEC, 0xF62F1430, 0},
            {g, 0x434693FC, 0x43468B6C, 0xF300F6B0, 0}, {g, 0x43467034, 0x434670D8, 0xEC1739AA, 0},
            {g, 0x43468B28, 0x4346A61C, 0x571B7D9A, 0}};
}
void reset_feedback_owner(FeedbackOwner& owner) noexcept {
    auto& state = owner.bindings.state;
    state.pending.clear();
    state.extrapolation_shots.clear();
    state.events.feedback.clear();
    state.fire_snapshots.clear();
    state.events.has_pending_shots = false;
}
void update_feedback_subscription(FeedbackOwner& owner, std::uint8_t update) {
    if (update)
        owner.toggle_impact(owner.bindings.live.recording_enabled);
}
void capture_feedback_position(FeedbackOwner& owner, const RageVector& position) noexcept {
    const RageVector captured = position; //XMM0/1/2 are saved before the native shift
    auto& state = owner.bindings.state;
    const auto incremented = state.recent_position_count + 1u;
    const auto count = std::min(incremented, 10u);
    state.recent_position_count = count;
    if (count != 1u)
        std::memmove(state.recent_positions.data() + 2, state.recent_positions.data() + 1,
                     std::size_t(count - 1u) * sizeof(RageVector));
    std::memcpy(&state.recent_positions[1], &captured, sizeof captured);
}
void capture_feedback_packet(FeedbackOwner& owner, const void* packet) {
    const auto& b = owner.bindings.packet;
    auto& state = owner.bindings.state;
    if (!b.capture_enabled || !b.recording_enabled)
        return;
    const auto player = field<std::uint32_t>(packet, b.offsets.player_index);
    if (player > 63u)
        return;
    if (player + 1u != std::uint32_t(engine::invoke<std::int32_t>(owner.engine_client, 12)))
        return;
    if (state.pending.empty())
        return;
    state.fire_snapshots.emplace_back();
    auto& snapshot = state.fire_snapshots.back();
    snapshot.marker =
        std::uint32_t(engine::invoke<std::int32_t>(owner.bindings.engine_client, 205));
    snapshot.origin = field<RageVector>(packet, b.offsets.origin);
    ShotFeedbackRecord* candidate = nullptr;
    std::uint32_t count = 0;
    for (auto& record : state.pending) {
        const auto current =
            std::uint32_t(engine::invoke<std::int32_t>(owner.bindings.engine_client, 205));
        if (record.words[3] == current) {
            ++count;
            candidate = &record;
        }
    }
    auto spread = field<float>(packet, b.offsets.spread);
    auto inaccuracy = field<float>(packet, b.offsets.inaccuracy);
    auto recoil = field<float>(packet, b.offsets.recoil_index);
    auto angles = field<RageVector>(packet, b.offsets.angles);
    angles[2] = 0.0f;
    if (candidate && count == 1u) {
        const auto first = std::bit_cast<std::int32_t>(candidate->words[1]);
        if (const auto* command = client::find_input_command(b.input, first)) {
            const auto pitch =
                packet_angle(command->angles[0] + std::bit_cast<float>(candidate->words[24]));
            const auto yaw =
                packet_angle(command->angles[1] + std::bit_cast<float>(candidate->words[25]));
            if (std::fabs(pitch - angles[0]) < 0.001f && std::fabs(yaw - angles[1]) < 0.001f) {
                angles[0] = pitch;
                angles[1] = yaw;
            }
        }
        const auto raw_inaccuracy = std::bit_cast<float>(candidate->words[22]);
        if (std::fabs(packet_quantize(raw_inaccuracy, 10, 1.0f) - inaccuracy) < 0.001f)
            inaccuracy = raw_inaccuracy;
        const auto raw_recoil = std::bit_cast<float>(candidate->words[21]);
        if (std::fabs(packet_quantize(raw_recoil, 10, 1000.0f) - recoil) < 0.001f)
            recoil = raw_recoil;
        const auto raw_spread = std::bit_cast<float>(candidate->words[23]);
        if (std::fabs(packet_quantize(raw_spread, 8, 0.10000000149011612f) - spread) < 0.001f)
            spread = raw_spread;
    }
    const auto seed = field<std::uint32_t>(packet, b.offsets.seed);
    const auto mode = field<std::int32_t>(packet, b.offsets.fire_mode);
    const auto weapon = field<std::uint16_t>(packet, b.offsets.weapon_id);
    const SpreadParameters parameters{weapon,
                                      mode,
                                      spread,
                                      minimum_number(1.0f, inaccuracy),
                                      recoil,
                                      1,
                                      shotgun_patterns(b.shotgun_patterns)};
    const auto sample = generate_spread_sample(parameters, seed, b.spread_tables, b.random,
                                               b.shotgun_pattern, b.trigonometry);
    const auto basis = legit::spread_basis(angles, b.trigonometry);
    RageVector direction;
    for (std::size_t i = 0; i < 3; ++i)
        direction[i] =
            (basis.first[i] * sample.first + basis.center[i]) + basis.second[i] * sample.second;
    const auto square =
        (direction[0] * direction[0] + direction[1] * direction[1]) + direction[2] * direction[2];
    const auto inverse = 1.0f / (std::sqrt(square) + 0x1p-23f);
    for (std::size_t i = 0; i < 3; ++i)
        snapshot.direction[i] = direction[i] * inverse;
}
void dispatch_feedback_position_packet(const engine::OwnerRegistry& registry, const void* packet) {
    if ((field<std::uint32_t>(packet, 0x1C) & 0xBu) != 0xBu)
        return;
    const auto x = field<float>(packet, 8), y = field<float>(packet, 0xC);
    auto* cursor = registry.begin;
    const auto z = field<float>(packet, 0x14);
    auto* const end = registry.end;
    for (; cursor != end; ++cursor) {
        auto* endpoint = *cursor;
        (*static_cast<const PositionEntry* const*>(endpoint))[0](endpoint, x, y, z);
    }
}
void bind_feedback_position_runtime(FeedbackPositionRuntime& runtime) noexcept {
    position_runtime = &runtime;
}
void unbind_feedback_position_runtime(FeedbackPositionRuntime& runtime) noexcept {
    if (position_runtime == &runtime)
        position_runtime = nullptr;
}
#if defined(_M_IX86) || defined(__i386__)
void __cdecl feedback_position_hook(const void* packet) {
#else
void feedback_position_hook(const void* packet) {
#endif
    dispatch_feedback_position_packet(position_runtime->positions, packet);
}
std::size_t feedback_owner_size() noexcept {
    return sizeof(FeedbackOwner);
}
FeedbackOwner& construct_feedback_owner(void* storage, const FeedbackOwnerBindings& bindings) {
    require_canonical_bindings(bindings);
    return *::new (storage) FeedbackOwner(bindings);
}
FeedbackOwner* create_feedback_owner(const FeedbackOwnerBindings& bindings) {
    require_canonical_bindings(bindings);
    auto* storage = allocate(bindings.lifecycle.memory, 0x110);
    if (!storage) {
        bindings.active = nullptr;
        return nullptr;
    }
    // These borrowed cells belong to the native HeapZeroMemory allocation.
    // The ordinary constructor below still preserves unwritten position slots.
    bindings.settings = ShotLogSettings{};
    bindings.state.recent_positions = {};
    auto& owner = construct_feedback_owner(storage, bindings);
    bindings.active = &owner; //mem_int3.h:561,43485C53 MOV[43475A7C],EAX
    return &owner;
}
void bind_feedback_owner_factory(client::NativeStartupSources& sources,
                                 FeedbackOwnerBindings& bindings) {
    sources.bind_initializer({0x43485C3A, &bindings, initialize_feedback_owner});
}
void destroy_feedback_owner(FeedbackOwner* owner, FeedbackOwner*& active) {
    active = nullptr;
    if (!owner)
        return;
    owner->bindings.active = nullptr;
    const auto memory = owner->bindings.lifecycle.memory;
    owner->~FeedbackOwner();
    free(memory, owner);
}
void* feedback_lifecycle_object(FeedbackOwner& owner) noexcept {
    return &owner;
}
ShotFeedbackState* feedback_owner_state(FeedbackOwner* owner) noexcept {
    return owner ? &owner->bindings.state : nullptr;
}
const ShotFeedbackState* feedback_owner_state(const FeedbackOwner* owner) noexcept {
    return owner ? &owner->bindings.state : nullptr;
}
const ShotLogSettings* feedback_owner_settings(const FeedbackOwner* owner) noexcept {
    return owner ? &owner->bindings.settings : nullptr;
}
const ShotFeedbackRuntime* feedback_owner_runtime(const FeedbackOwner* owner) noexcept {
    return owner ? &owner->bindings.runtime : nullptr;
}

#undef FEEDBACK_MEMBER
#undef FEEDBACK_FAST
#undef FEEDBACK_VECTOR
} //namespace skeet::rage

// ============================================================================
// Subcomponent: feedback_runtime_owner.cpp
// ============================================================================

namespace skeet::rage {
namespace {
using client::ClientRuntimeStorage;
std::ptrdiff_t storage_field(ClientRuntimeStorage& s, std::uint32_t hash) {
    const auto& globals = s.offsets().globals;
    for (const auto& recipe : engine::build_receive_offset_recipes())
        if (recipe.initial_xor_key == hash)
            return engine::DecodedOffset(globals, recipe.pointer_identity, recipe.xor_key_identity,
                                         recipe.initial_xor_key, 0);
    throw std::invalid_argument("Feedback field has no recovered receive-offset recipe");
}
engine::OwnerRegistryBinding registry(ClientRuntimeStorage& s, std::uint32_t identity) {
    return {s.owner_registries().at(identity), s.memory()};
}
ClientRuntimeStorage& checked(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
                              client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
                              engine::SceneQueryRuntimeOwner& q, animation::RuntimeOwner& b,
                              client::CrtRuntimeOwner& crt, esp::EspRuntimeOwner& esp,
                              script::ScriptApiRuntimeOwner& scripts) {
    if (&e.variables.data() != &s.data() || &c.storage() != &s || &h.storage() != &s ||
        &q.storage() != &s || &q.players() != &h.players() || &b.storage() != &s ||
        &b.players() != &h.players().cache() || &crt.storage() != &s || &esp.storage() != &s ||
        &scripts.storage() != &s)
        throw std::invalid_argument(
            "Feedback requires the canonical engine, configuration, history, scene, animation, CRT, ESP and Lua API owners");
    for (const auto identity : {0x4346A514u, 0x4346A890u, 0x4346A614u, 0x4346A334u, 0x4346A338u,
                                0x43467B04u, 0x43475B2Cu, 0x4346A894u})
        if (!s.data().native_cell<void*>(identity))
            throw std::invalid_argument("Feedback engine publication is incomplete");
    for (const auto identity :
         {0x43468520u, 0x434670D0u, 0x4346747Cu, 0x4346A748u, 0x4346A330u, 0x43468EA4u})
        if (!e.variables.at(identity).get())
            throw std::invalid_argument("Feedback requires its acquired live ConVars");
    if (s.data().native_cell<FeedbackOwner*>(0x43475A7Cu))
        throw std::logic_error("Feedback runtime must precede its actual feature factory");
    engine::initialize_build_hitbox_clip_bindings(s.data());
    engine::initialize_build_pose_range_bindings(s.data());
    return s;
}
class LiveFeedbackEngine final : public ShotFeedbackEngine {
    ClientRuntimeStorage& storage_;
    RageBoxClipper clipper_;
    std::ptrdiff_t hidden_, weapon_handle_, ammunition_, weapon_shot_, player_injury_, player_time_;
    std::vector<RageLocalHitbox> boxes_;
    template <class T> static T read_field(void* object, std::ptrdiff_t at) {
        return read<T>(object, at);
    }
    static bool read_hidden(void* object, std::ptrdiff_t at) {
        return read<std::uint8_t>(object, at) != 0;
    }
    static std::int32_t read_health(void* object, std::ptrdiff_t) {
        return engine::invoke<std::int32_t>(object, 122);
    }

public:
    LiveFeedbackEngine(ClientRuntimeStorage& s, RageBoxClipper clipper)
        : storage_(s), clipper_(clipper), hidden_(storage_field(s, 0x9D920E35u)),
          weapon_handle_(storage_field(s, 0xCD812654u)), ammunition_(storage_field(s, 0xCA5EC698u)),
          weapon_shot_(storage_field(s, 0x0176A381u)), player_injury_(storage_field(s, 0xAE838771u)),
          player_time_(storage_field(s, 0xD5FD2E2Cu)) {}
    std::int32_t current_command() override {
        return engine::invoke<std::int32_t>(storage_.data().native_cell<void*>(0x4346A514u), 205);
    }
    std::int32_t local_player_index() override {
        auto* local = storage_.data().native_cell<std::byte*>(0x43475808u);
        return engine::invoke<std::int32_t>(local + 8, 10);
    }
    float engine_time() override {
        return engine::invoke<float>(storage_.data().native_cell<void*>(0x4346A514u), 14);
    }
    ShotFeedbackEntity local_player() override {
        ShotFeedbackEntity result;
        auto* local = storage_.data().native_cell<void*>(0x43475808u);
        result.exists = local != nullptr;
        if (local) {
            result.hidden = ShotFeedbackEntityField<bool>::live(local, hidden_, read_hidden);
            result.weapon_handle = ShotFeedbackEntityField<std::uint32_t>::live(
                local, weapon_handle_, read_field<std::uint32_t>);
        }
        return result;
    }
    ShotFeedbackEntity entity_by_index(std::int32_t index) override {
        ShotFeedbackEntity result;
        auto* player =
            engine::invoke<void*>(storage_.data().native_cell<void*>(0x4346A890u), 3, index);
        result.exists = player != nullptr;
        if (player) {
            result.field_d5fd2e2c =
                ShotFeedbackEntityField<float>::live(player, player_time_, read_field<float>);
            result.health = ShotFeedbackEntityField<std::int32_t>::live(player, 0, read_health);
            result.last_shot_time =
                ShotFeedbackEntityField<float>::live(player, player_injury_, read_field<float>);
        }
        return result;
    }
    ShotFeedbackEntity entity_by_handle(std::uint32_t handle) override {
        ShotFeedbackEntity result;
        auto* weapon =
            engine::invoke<void*>(storage_.data().native_cell<void*>(0x4346A890u), 4, handle);
        result.exists = weapon != nullptr;
        if (weapon) {
            result.life_id = ShotFeedbackEntityField<std::int32_t>::live(weapon, ammunition_,
                                                                         read_field<std::int32_t>);
            result.simulation_time =
                ShotFeedbackEntityField<float>::live(weapon, weapon_shot_, read_field<float>);
        }
        return result;
    }
    std::span<const RageLocalHitbox> hitboxes(std::int32_t index, std::int32_t set_index) override {
        boxes_.clear();
        auto* info = storage_.data().native_cell<void*>(0x4346A614u);
        const auto* model = engine::invoke<const void*>(info, 1, index);
        if (!model)
            return boxes_;
        const auto* header = engine::invoke<const std::byte*>(info, 32, model);
        if (!header)
            return boxes_;
        const auto* set =
            header + read<std::int32_t>(header, 0xB0) + std::ptrdiff_t(set_index) * 12;
        const auto count = (std::min)(read<std::uint32_t>(set, 4), 19u);
        const auto* first = set + read<std::int32_t>(set, 8);
        boxes_.reserve(count);
        for (std::uint32_t i = 0; i < count; ++i) {
            const auto* descriptor = first + i * 0x44;
            boxes_.push_back({read<std::int32_t>(descriptor, 4), read<RageVector>(descriptor, 8),
                              read<RageVector>(descriptor, 0x14),
                              read<RageVector>(descriptor, 0x24), read<float>(descriptor, 0x30),
                              descriptor});
        }
        return boxes_;
    }
    int clip_hitbox(const RageRay& ray, const RageLocalHitbox& hitbox, const Matrix3x4& matrix,
                    RageTrace& trace) override {
        RageScanHitbox selected{hitbox.hitgroup, {}, hitbox.engine_hitbox, matrix};
        return clipper_.clip(clipper_.context, ray, selected, trace);
    }
};
}
struct FeedbackRuntimeOwner::Data {
    ClientRuntimeStorage& storage;
    engine::PoseRangeRuntime pose;
    RageBoxClipper clipper;
    resolver_runtime::RawAttemptEngine attempts;
    LiveFeedbackEngine engine;
    ShotFeedbackRuntime runtime;
    FeedbackEntityBindings live;
    FeedbackPacketBindings packet;
    PendingShotPublicationBindings pending;
    FeedbackOwnerBindings factory;
    Data(ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
         client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
         engine::SceneQueryRuntimeOwner& q, animation::RuntimeOwner& b,
         client::CrtRuntimeOwner& crt, esp::EspRuntimeOwner& esp,
         script::ScriptApiRuntimeOwner& scripts)
        : storage(checked(s, e, c, h, q, b, crt, esp, scripts)), pose(s.data()),
          clipper(engine::native_hitbox_clipper(s.data())),
          attempts{s.data().native_cell<void*>(0x4346A890u),
                   s.data().native_cell<float*>(0x4346A334u),
                   h.target_offsets().rejection_bytes[0],
                   std::ptrdiff_t(h.entity_runtime().offsets.animation_state),
                   b.bones().fields.studio_wrapper_32e4daf1,
                   b.bones().fields.pose_parameters,
                   b.bones().fields.hitbox_set_c2b6fb10,
                   pose,
                   clipper},
          engine(s, clipper), runtime{q.queries().ballistics(),
                                      engine,
                                      attempts,
                                      b.trigonometry(),
                                      h.players().cache(),
                                      h.players().adjustments(),
                                      c.rage_settings().shot_log,
                                      scripts.normal_log()},
          live{c.rage_settings().global.anti_aim_correction.value,
               s.data().native_cell<const float*>(0x4346A334u),
               s.data().native_cell<const float*>(0x4346A338u),
               s.data().native_cell<const void*>(0x43467B04u),
               s.data().native_cell<std::uint32_t>(0x4346747Cu),
               s.data().native_cell<std::uint32_t>(0x4346A748u),
               s.data().native_cell<std::uint32_t>(0x4346A330u),
               s.data().native_cell<std::uint32_t>(0x43468EA4u),
               s.data().native_cell<float>(0x43469254u),
               s.data().native_cell<float>(0x43476928u),
               c.settings_menu().maximum_unlag,
               nullptr},
          packet{c.rage_settings().global.enabled.value,
                 c.rage_settings().global.anti_aim_correction.value,
                 s.data().native_cell<void*>(0x4346A894u),
                 e.variables.at(0x43468520u),
                 feedback_packet_offsets(s.offsets()),
                 crt.spread().tables(),
                 crt.random(),
                 esp.shotgun_pattern(),
                 b.trigonometry()},
          pending{
              s.data().native_cell<FeedbackOwner*>(0x43475A7Cu),
              c.rage_settings().global.anti_aim_correction.value,
              h.players().cache(),
              s.data().native_cell<void*>(0x4346A514u),
              s.data().native_cell<void*>(0x4346A890u),
              s.data().native_cell<void*>(0x43467B54u),
              e.variables.at(0x434670D0u),
              {storage_field(s, 0xCA5EC698u), storage_field(s, 0x47877DEBu), b.bones().fields.hitbox_set_c2b6fb10},
              s.data().native_cell<float*>(0x4346A334u),
              s.data().native_cell<float*>(0x43467EB8u),
              s.data().native_cell<float*>(0x4346A338u),
              &s.data().native_cell<std::uint32_t>(0x434759C0u),
              attempts,
              nullptr},
          factory{registry(s, 0x43475B78u),
                  s.command_phases(),
                  s.entity_observers(),
                  registry(s, 0x4347584Cu),
                  registry(s, 0x43475AD8u),
                  registry(s, 0x43475930u),
                  s.recharge_observers(),
                  s.configuration(),
                  s.event_registry(),
                  c.rage_settings().shot_log,
                  h.feedback_state(),
                  runtime,
                  live,
                  packet,
                  *s.data().native_cell<std::int32_t*>(0x43475B2Cu),
                  s.data().native_cell<void*>(0x4346A514u),
                  s.data().native_cell<FeedbackOwner*>(0x43475A7Cu)} {}
};
FeedbackRuntimeOwner::FeedbackRuntimeOwner(
    ClientRuntimeStorage& s, const client::EngineRuntimeBindings& e,
    client::ConfigurationRuntimeOwner& c, client::HistoryRuntimeOwner& h,
    engine::SceneQueryRuntimeOwner& q, animation::RuntimeOwner& b, client::CrtRuntimeOwner& crt,
    esp::EspRuntimeOwner& esp, script::ScriptApiRuntimeOwner& scripts)
    : data_(std::make_unique<Data>(s, e, c, h, q, b, crt, esp, scripts)) {}
FeedbackRuntimeOwner::~FeedbackRuntimeOwner() = default;
ClientRuntimeStorage& FeedbackRuntimeOwner::storage() const noexcept {
    return data_->storage;
}
ShotFeedbackRuntime& FeedbackRuntimeOwner::runtime() noexcept {
    return data_->runtime;
}
FeedbackOwnerBindings& FeedbackRuntimeOwner::factory_bindings() noexcept {
    return data_->factory;
}
const PendingShotPublicationBindings& FeedbackRuntimeOwner::pending_shots() const noexcept {
    return data_->pending;
}
void FeedbackRuntimeOwner::bind_initializer(client::NativeStartupSources& sources) {
    bind_feedback_owner_factory(sources, data_->factory);
}
} //namespace skeet::rage

// ============================================================================
// Subcomponent: pending_shot_publication.cpp
// ============================================================================

namespace skeet::rage {
namespace {
using skeet::field;
using skeet::bits;
bool weapon_prefix(const char* name) noexcept {
    constexpr char prefix[] = "weapon_";
    // Native774FB8A0 is msvcrt!_strnicmp, not the case-sensitive strncmp.
    for (std::size_t index = 0; index < 7; ++index) {
        auto value = static_cast<unsigned char>(name[index]);
        if (value >= 'A' && value <= 'Z')
            value = static_cast<unsigned char>(value + ('a' - 'A'));
        if (value != static_cast<unsigned char>(prefix[index]))
            return false;
    }
    return true;
}
bool convar_nonzero(const void* object) noexcept {
    for (;;) {
        const auto parent = field<const void*>(object, 0x1c);
        if (parent == object)
            break;
        object = parent;
    }
    return field<std::uint32_t>(object, 0x30) !=
           std::uint32_t(reinterpret_cast<std::uintptr_t>(object));
}
ShotFeedbackRecord& append_pending_shot(ShotFeedbackState& state) {
    const auto count = state.pending.size();
    if (count == state.pending.capacity()) {
        //43406E18 grows by1 for an empty queue and2*size otherwise. Its copy
        // constructor433D68DA copies34 prefix words plus the complete1190
        // state, leaving the allocation's zeroed eight-byte prefix hole.
        std::vector<ShotFeedbackRecord> grown;
        grown.reserve(count ? count * 2 : 1);
        grown.resize(count + 1);
        for (std::size_t index = 0; index < count; ++index)
            lagcomp::copy_construct_evaluation(grown[index], state.pending[index]);
        state.pending.swap(grown);
    } else
        state.pending.emplace_back();
    auto& result = state.pending.back();
    std::memset(&result, 0, sizeof result);
    state.events.has_pending_shots = true;
    return result;
}
ShotFeedbackRecord* append_current_pending_shot(const PendingShotPublicationBindings& bindings) {
    //4341D459 reads the full owner once, after the caller's aim_fire dispatch.
    // Its EDI record is retained after append; no state/owner view is carried
    // into subsequent engine callbacks or used for the later recording read.
    auto* owner = bindings.active;
    if (!owner || !bindings.recording_enabled)
        return nullptr;
    return &append_pending_shot(*feedback_owner_state(owner));
}
void capture_weapon_bullet(void* system, std::uint16_t identifier, void* destination) {
    if (!system) {
        // Exact immutable4346F1B8 table, used by the original null-system path.
        constexpr std::array<std::uint32_t, 7> native_default{
            0x26U, 0x40200000U, 0x46000000U, 0x3f7ae148U, 0x3fd33333U, 0x42a00000U, 1U};
        std::memcpy(destination, native_default.data(), sizeof native_default);
        return;
    }
    const auto* data = engine::invoke<const void*>(system, 2, std::uint32_t(identifier));
    field(destination, 0, identifier);
    field(destination, 4, field<std::uint32_t>(data, 0x100));
    field(destination, 8, field<std::uint32_t>(data, 0x10c));
    field(destination, 12, field<std::uint32_t>(data, 0x110));
    field(destination, 16, field<std::uint32_t>(data, 0xf8));
    const auto damage = field<std::uint32_t>(data, 0xf0);
    auto converted = _mm_cvtsi32_sd(_mm_setzero_pd(), integer(damage));
    converted = _mm_add_sd(converted, _mm_set_sd((damage >> 31) ? 4294967296.0 : 0.0));
    field(destination, 20, bits(_mm_cvtss_f32(_mm_cvtsd_ss(_mm_setzero_ps(), converted))));
    field(destination, 24, field<std::uint32_t>(data, 0xfc));
}
void write_mode_yaw(client::NativePlayerStateLayout& owner,
                    const lagcomp::EvaluationState& source) noexcept {
    const auto mode = source.record.header.resolver_index;
    const auto yaw = source.record.header.player_history_1ac;
    if (mode == 1) {
        field(&owner, 0x3c, yaw);
        field(&owner, 0x60, real(yaw) + 180.0f);
    } else if (mode == 2) {
        field(&owner, 0x60, yaw);
        field(&owner, 0x3c, real(yaw) + 180.0f);
    }
}
}

void capture_shot_weapon_bullet(void* system, std::uint16_t identifier,
                                client::CommandBullet& destination) {
    capture_weapon_bullet(system, identifier, &destination);
}

void shot_capture_target(ShotFeedbackRecord& destination, const lagcomp::EvaluationState& source,
                         const RageVector& origin, std::uint8_t low_flags,
                         const PendingShotPublicationBindings& bindings) {
    const auto index = source.words[1];
    bindings.players.get(index);
    const auto* target = engine::invoke<void*>(bindings.entity_list, 3, index);
    if (!target)
        return;
    destination.words[0] = (destination.words[0] & ~1U) | low_flags;
    destination.words[6] =
        std::uint32_t(lagcomp::time_to_tick(*bindings.prediction_time, *bindings.interval));
    for (std::size_t axis = 0; axis < 3; ++axis)
        destination.words[9 + axis] = bits(origin[axis]);
    destination.words[12] =
        std::uint32_t(std::int32_t(field<std::int16_t>(target, bindings.offsets.model_index)));
    destination.words[13] = field<std::uint32_t>(target, bindings.offsets.hitbox_set);
    std::memcpy(&destination.state, &source, sizeof source);
}

bool publish_pending_shot(const client::CommandFrame& frame, const lagcomp::EvaluationState& source,
                          std::int32_t shot_index, const PendingShotPublicationBindings& bindings,
                          std::int32_t extrapolation_ticks) {
    auto* appended = append_current_pending_shot(bindings);
    if (!appended)
        return false;
    auto& shot = *appended;
    shot.words[1] = std::uint32_t(frame.command_number_ac);
    shot.words[3] = std::uint32_t(frame.command_number_ac);
    shot.words[4] = bits(*bindings.current_time);
    shot.words[2] = std::uint32_t(engine::invoke<std::int32_t>(bindings.engine_client, 205));
    shot.words[5] = std::uint32_t(shot_index);
    shot.words[14] = 0x4dc70629U;
    shot.words[15] = 0x09bfc248U;
    if (frame.weapon_address) {
        auto* weapon = reinterpret_cast<void*>(std::uintptr_t(frame.weapon_address));
        if (const auto* name = engine::invoke<const char*>(weapon, 459))
            shot.words[14] = events::event_hash(name);
        if (const auto* name = engine::invoke<const char*>(weapon, 143))
            shot.words[15] = events::event_hash(weapon_prefix(name) ? name + 7 : name);
        shot.words[7] = *engine::invoke<const std::uint32_t*>(weapon, 2);
        shot.words[8] = convar_nonzero(bindings.life_count_convar)
                            ? UINT32_MAX
                            : field<std::uint32_t>(weapon, bindings.offsets.life_count) - 1U;
        capture_weapon_bullet(bindings.weapon_system, std::uint16_t(frame.weapon_id_d8),
                              shot.words.data() + 27);
        shot.words[21] = frame.field_100;
        shot.words[22] = bits(minimum_number(1.0f, frame.inaccuracy_c4));
        shot.words[23] = bits(frame.spread_d0);
        for (std::size_t axis = 0; axis < 3; ++axis)
            shot.words[24 + axis] = bits(frame.recoil_angles_9c[axis]);
    } else {
        shot.words[7] = UINT32_MAX;
        shot.words[8] = UINT32_MAX;
    }
    shot_capture_target(shot, source, frame.origin_dc, 0, bindings);
    if (extrapolation_ticks > 0) {
        feedback_owner_state(bindings.active)
            ->extrapolation_shots.push_back({frame.command_number_ac, shot_index,
                                             extrapolation_ticks,
                                             source.record.header.simulation_tick});
        feedback_owner_runtime(bindings.active)
            ->normal_log.console_format(
                "[extrapolation] FIRE shot=%d target=%u +%d ticks record=%d", shot_index,
                source.words[1], extrapolation_ticks, source.record.header.simulation_tick);
    }
    auto& player = *bindings.players.get(source.words[1]);
    const auto mode = source.record.header.resolver_index;
    field(&player.native, 0x14 + std::uint32_t(mode) * 0x24, bits(*bindings.current_time));
    write_mode_yaw(player.native, source);
    // Publication is not feedback. Wait for the actual shot ray before
    // rejecting a pose; a pending high-chance shot has not missed yet.
    if (bindings.resolver_rework && *bindings.resolver_rework)
        return true;
    if (!(shot.state.words[13] & 4U) || !shot.state.record.header.resolver_index ||
        !(real(shot.state.words[14]) > 0.9f) ||
        integer(shot.state.words[10]) < integer(shot.state.words[2]))
        return true;
    shot.words[0] |= 0x80U;
    if (*bindings.scheduler_flags & 0x40U)
        return true;
    RageVector direction;
    const RageVector delta{real(shot.state.words[3]) - real(shot.words[9]),
                           real(shot.state.words[4]) - real(shot.words[10]),
                           real(shot.state.words[5]) - real(shot.words[11])};
    const auto length =
        std::sqrt((delta[0] * delta[0] + delta[1] * delta[1]) + delta[2] * delta[2]);
    const auto inverse = 1.0f / (length + std::numeric_limits<float>::epsilon());
    for (std::size_t axis = 0; axis < 3; ++axis)
        direction[axis] = inverse * delta[axis];
    const auto previous = shot.state.record.header.resolver_mode;
    const resolver_runtime::AttemptShot attempt_shot{shot.state.record, shot.words[0],
                                                     shot.state.words[13], shot.state.words[1],
                                                     real(shot.words[29])};
    const RageVector origin{real(shot.words[9]), real(shot.words[10]), real(shot.words[11])};
    const auto next = resolver_runtime::choose_next_attempt(
        {field<float>(&player.native, 0x88), field<float>(&player.native, 0x1a0)}, attempt_shot,
        *bindings.current_time, bindings.recording_enabled != 0, &origin, &direction,
        bindings.attempts);
    player.native.resolver_slots[mode].attempt = next;
    if (previous == next)
        return true;
    player.native.resolver_status = 0;
    shot.words[0] |= 0x40U;
    for (std::uint32_t age = 0; age < player.history.count; ++age) {
        auto& record = *player.history.records[(player.history.first + age) & 31U];
        if (record.header.resolver_index == shot.state.record.header.resolver_index &&
            record.header.resolver_mode == previous)
            record.header.flags |= 0x40U;
        if (record.header.simulation_tick < integer(shot.state.words[45]))
            break;
    }
    return true;
}
}

// ============================================================================
// Subcomponent: shot_diagnostics.cpp
// ============================================================================

namespace skeet::rage {
namespace {
/* template <class T> void write(void* object, std::ptrdiff_t offset, const T& value) noexcept {
    std::memcpy(static_cast<std::byte*>(object) + offset, &value, sizeof value);
} */
std::int32_t diagnostic_tick(float time, float interval) noexcept {
    const auto quotient = _mm_div_ss(_mm_set_ss(time), _mm_set_ss(interval));
    return _mm_cvtt_ss2si(_mm_add_ss(quotient, _mm_set_ss(0.5f)));
}
std::uint32_t handle(void* entity) {
    return *engine::invoke<const std::uint32_t*>(entity, 2);
}
std::uint32_t handle_index(std::uint32_t value) noexcept {
    return value == UINT32_MAX ? 0x1FFFu : value & 0xFFFFu;
}
void* networkable(void* entity) noexcept {
    return static_cast<std::byte*>(entity) + 8;
}
// Complete43364942 with the actual ClientClass creation function and engine
// registration calls. No reconstructed entity lifecycle is supplied as a callback.
void* create_shot_entity(const char* path, const ShotModelPublicationBindings& b) {
    const auto highest = read<std::int32_t>(b.entity_list, 0x24);
    if (!b.create_entity)
        throw std::logic_error("Shot render entity requires the native ClientClass factory");
    auto* created = b.create_entity(0x7FE, 0x7FF);
    if (!created)
        return nullptr;
    if (b.register_entity && b.unregister_entity) {
        auto* unknown = engine::invoke<void*>(created, 0);
        auto* entity = engine::invoke<void*>(unknown, 7);
        b.register_entity(b.registration_receiver, handle_index(handle(entity)));
        write(b.entity_list, 0x24, highest);
        if (engine::invoke<std::int32_t>(b.model_info, 2, path) != -1 &&
            engine::invoke<bool>(entity, 0x184 / 4, path, false))
            write(entity, 0x64, handle_index(handle(entity)));
        else {
            auto invalid = UINT32_MAX;
            b.unregister_entity(b.registration_receiver, &invalid, entity);
        }
    }
    engine::invoke<void>(created, 3, std::int32_t(0));
    return engine::invoke<void*>(engine::invoke<void*>(created, 0), 7);
}
void publish_vector(void* entity, std::ptrdiff_t offset, const RageVector& value,
                    std::int32_t change, const ShotModelPublicationBindings& b) {
    const auto previous = read<RageVector>(entity, offset);
    // Native ordered comparisons treat every NaN as a changed component.
    if (previous[0] == value[0] && previous[1] == value[1] && previous[2] == value[2])
        return;
    if (b.invalidate_transform)
        b.invalidate_transform(entity, change);
    write(entity, offset, value);
}
void force_opaque(void* entity, const ShotModelPublicationBindings& b) {
    const auto color = read<std::uint32_t>(entity, b.offsets.color);
    if ((color >> 24) == 0xFFu || !b.color_proxy || !*b.color_proxy)
        return;
    // Complete4341AE71. Unwritten proxy-packet bytes retain their stack values.
    std::array<std::byte, 0x30> packet;
    write(packet.data(), 8, color | 0xFF000000u);
    write(packet.data(), 0x14, std::uint32_t(1));
    (*b.color_proxy)(reinterpret_cast<const engine::RecvProxyData*>(packet.data()), entity,
                     static_cast<std::byte*>(entity) + b.offsets.color);
}
struct PreparationFields {
    void* target;
    const lagcomp::EntityNetvars& offsets;
    std::uint32_t collision_first, collision_second;
    std::array<std::uint32_t, 2> angles;
    PreparationFields(void* entity, const lagcomp::EntityNetvars& o) noexcept
        : target(entity), offsets(o),
          collision_first(read<std::uint32_t>(entity, o.field_432b1f5b)),
          collision_second(read<std::uint32_t>(entity, o.field_0d560fac)),
          angles(read<std::array<std::uint32_t, 2>>(entity, o.eye_angles)) {}
    ~PreparationFields() {
        write(target, offsets.eye_angles + 4, angles[1]);
        write(target, offsets.eye_angles, angles[0]);
        write(target, offsets.field_0d560fac, collision_second);
        write(target, offsets.field_432b1f5b, collision_first);
    }
};
}

void capture_shot_diagnostics(const client::CommandFrame& frame, void* local, void* target,
                              const ShotDiagnosticBindings& b) {
    //43329B33 reads43475968 once. This runs after the caller's real entity-list
    // lookup; a binding retains the pointer cell, never an earlier owner/view.
    auto* diagnostics = client::buffered_shot_diagnostics(b.buffered_shot_owner);
    if (!diagnostics)
        return;
    auto& history = *diagnostics;
    const auto incremented = history.count + 1u;
    history.count = incremented < 6u ? incremented : 6u;
    if (history.count != 1u)
        std::memmove(history.entries.data() + 1, history.entries.data(),
                     std::size_t(history.count - 1u) * sizeof(ShotDiagnosticEntry));
    auto& entry = history.entries[0];
    std::memset(&entry, 0, sizeof entry);
    entry.clock = *b.clock;
    entry.command = frame.command_number_ac;
    entry.target_tick = diagnostic_tick(read<float>(target, b.offsets.simulation_time), b.interval);
    const auto index = engine::invoke<std::int32_t>(networkable(target), 10);
    auto& cached = b.players.get(std::uint32_t(index))->native;
    entry.target_field_168 = read<std::uint32_t>(&cached, 0x168);
    entry.target_flags = read<std::uint32_t>(target, b.offsets.player_flags);
    entry.target_height = cached.cached_target_height;
    entry.target_duck = read<float>(target, b.offsets.duck_amount);
    entry.target_reference_yaw = cached.reference_yaw;
    entry.local_field_7f5ed2d0 = read<std::uint32_t>(local, b.offsets.local_field_7f5ed2d0);
    entry.local_tick = diagnostic_tick(read<float>(local, b.offsets.simulation_time), b.interval);
    entry.target_handle = handle(target);
    capture_shot_weapon_bullet(b.weapon_system, std::uint16_t(frame.weapon_id_d8), entry.bullet);
}

ShotModelBoneStorage::ShotModelBoneStorage(engine::JobMemory memory)
    : begin_(standalone_begin_), end_(standalone_end_), limit_(standalone_limit_), memory_(memory) {
    if (!memory_.allocate || !memory_.free || !memory_.process_heap)
        throw std::invalid_argument("Shot matrices require genuine process-heap services");
}
ShotModelBoneStorage::ShotModelBoneStorage(engine::NativeDataImage& data, engine::JobMemory memory)
    : begin_(data.native_cell<Matrix3x4*>(0x434797A0u)),
      end_(data.native_cell<Matrix3x4*>(0x434797A4u)),
      limit_(data.native_cell<Matrix3x4*>(0x434797A8u)), memory_(memory) {
    if (!memory_.allocate || !memory_.free || !memory_.process_heap || begin_ || end_ || limit_)
        throw std::invalid_argument(
            "Source shot matrices require an unowned canonical vector and genuine heap services");
}
ShotModelBoneStorage::~ShotModelBoneStorage() {
    finalize();
}
std::span<Matrix3x4> ShotModelBoneStorage::matrices() const noexcept {
    return begin_ ? std::span<Matrix3x4>(begin_, static_cast<std::size_t>(end_ - begin_))
                  : std::span<Matrix3x4>{};
}
void ShotModelBoneStorage::finalize() {
    if (begin_)
        engine::release_owner_storage(memory_, begin_);
    begin_ = end_ = limit_ = nullptr;
}
void ShotModelBoneStorage::resize(std::size_t count) {
    const auto current = begin_ ? static_cast<std::size_t>(end_ - begin_) : 0u;
    if (count <= current) {
        end_ = begin_ ? begin_ + count : nullptr;
        return;
    }
    const auto additional = count - current;
    const auto available = begin_ ? static_cast<std::size_t>(limit_ - end_) : 0u;
    if (additional > available) {
        const auto capacity = (std::max)(count, current ? current * 2u : std::size_t(1));
        if (capacity > (UINT32_MAX - 0x20u) / sizeof(Matrix3x4))
            throw std::length_error(
                "Shot matrix allocation exceeds the native header representation");
        auto* replacement = static_cast<Matrix3x4*>(
            engine::allocate_owner_storage(memory_, capacity * sizeof(Matrix3x4)));
        // The successful native resize assumes allocation succeeds. Report an
        // explicit source OOM failure instead of inventing its null-copy edge.
        if (!replacement)
            throw std::bad_alloc();
        //4336EB90/95 reload the live range after the real heap boundary.
        auto* first = begin_;
        auto* last = end_;
        const auto copied = first ? static_cast<std::size_t>(last - first) : 0u;
        if (copied)
            std::memcpy(replacement, first, copied * sizeof(Matrix3x4));
        std::memset(replacement + copied, 0, additional * sizeof(Matrix3x4));
        auto* next_end = replacement + copied + additional;
        if (begin_)
            engine::release_owner_storage(memory_, begin_);
        // Native publication order is first, capacity, then end.
        begin_ = replacement;
        limit_ = replacement + capacity;
        end_ = next_end;
    } else {
        std::memset(end_, 0, additional * sizeof(Matrix3x4));
        end_ += additional;
    }
}

void publish_accepted_shot_model(const client::CommandFrame& frame,
                                 const TargetCandidate& candidate, const lagcomp::LagRecord& record,
                                 const ShotModelPublicationBindings& b) {
    if (!b.settings.on_shot.enabled.value)
        return;
    auto* target = engine::invoke<void*>(b.entity_list, 3, candidate.entity_index);
    const auto* wrapper = read<const std::byte*>(target, b.offsets.studio_header);
    if (!wrapper)
        return;
    if (!read<const void*>(wrapper, 0))
        return;
    const auto& common = b.preparation.entity.offsets;
    std::array<Matrix3x4, animation::maximum_bones> saved_bones;
    lagcomp::EntityRecordScope scope(b.preparation.entity, target, saved_bones);
    const lagcomp::EntityHitboxModelOffsets model_offsets{
        b.bone_runtime.fields.studio_wrapper_32e4daf1, b.bone_runtime.fields.hitbox_set_c2b6fb10};
    scope.apply_live_model(record, model_offsets, b.current_bone_counter);
    PreparationFields saved_fields(target, common);
    auto preparation = b.preparation;
    auto timing = b.timing;
    timing.tick_d4 = std::bit_cast<std::int32_t>(frame.field_d4);
    timing.tick_18c = std::bit_cast<std::int32_t>(frame.field_18c);
    preparation.current_timing = &timing;
    lagcomp::prepare_entity_angles(b.players, target, record, true, preparation);
    //4331F545 retains the original wrapper and rereads its header after the
    // record application and preparation callbacks, before resizing matrices.
    const auto* studio = read<const std::byte*>(wrapper, 0);
    const auto count = read<std::int32_t>(studio, 0x9C);
    if (count < 0 || count > std::int32_t(animation::maximum_bones))
        throw std::length_error("Shot studio bone count must fit the native256-bone workspace");
    b.bones.resize(std::size_t(count));
    const auto output = b.bones.matrices();
    b.published.bones_begin = output.data();
    b.published.bones_end = output.empty() ? output.data() : output.data() + output.size();
    void* clone = b.shot_entity_handle == UINT32_MAX
                      ? nullptr
                      : engine::invoke<void*>(b.entity_list, 4, b.shot_entity_handle);
    if (!clone) {
        const auto model = read<std::int16_t>(target, b.offsets.model_index);
        const auto* model_pointer =
            engine::invoke<const void*>(b.model_info, 1, std::int32_t(model));
        const auto* path = engine::invoke<const char*>(b.model_info, 3, model_pointer);
        if (auto* created = create_shot_entity(path, b))
            b.shot_entity_handle = handle(created);
        if (b.shot_entity_handle != UINT32_MAX)
            clone = engine::invoke<void*>(b.entity_list, 4, b.shot_entity_handle);
        if (!clone)
            return;
        write(clone, b.offsets.entity_flags,
              read<std::uint32_t>(clone, b.offsets.entity_flags) | 0x4858u);
        write(clone, b.offsets.byte_8481ea04, std::uint8_t(0));
        engine::invoke<void>(networkable(clone), 3, std::int32_t(0));
        write(clone, common.simulation_time - 8, std::numeric_limits<float>::max());
        write(clone, b.offsets.interpolation_flags,
              std::uint8_t(read<std::uint8_t>(clone, b.offsets.interpolation_flags) | 2u));
        write(clone, b.offsets.hitbox_set - 0xC, std::uint8_t(0));
    }
    if (engine::invoke<bool>(networkable(clone), 9))
        engine::invoke<void>(networkable(clone), 3, std::int32_t(0));
    const auto model_index = read<std::int16_t>(target, b.offsets.model_index);
    if (read<std::int16_t>(clone, b.offsets.model_index) != model_index) {
        engine::invoke<void>(b.model_cache, 33);
        engine::invoke<void>(clone, 75, std::int32_t(model_index));
        engine::invoke<void>(b.model_cache, 34);
        engine::invoke<void>(clone, 112);
    }
    engine::invoke<void>(clone, 54);
    engine::invoke<void>(clone, 238, false);
    write(clone, b.offsets.hitbox_set, read<std::uint32_t>(target, b.offsets.hitbox_set));
    write(clone, b.offsets.field_950f7b20, read<std::uint32_t>(target, b.offsets.field_950f7b20));
    write(clone, b.offsets.model_instance,
          engine::invoke<std::uint32_t>(static_cast<std::byte*>(target) + 4, 38));
    write(clone, b.offsets.field_a25d8271, std::uint32_t(0));
    publish_vector(clone, b.offsets.local_origin, record.header.origin, 1, b);
    publish_vector(clone, b.offsets.local_angles, {0.0f, record.header.animation_yaw, 0.0f}, 2, b);
    auto* layers = static_cast<std::byte*>(clone) + common.animation_layers;
    lagcomp::restore_animation(
        record.animation, read<void*>(layers, 0), read<std::int32_t>(layers, 0xC),
        reinterpret_cast<float*>(static_cast<std::byte*>(clone) + common.pose_parameters));
    write(clone, common.bone_accessor + 8, 0xFFF00u);
    write(clone, common.bone_accessor + 12, 0xFFF00u);
    if (b.current_bone_counter)
        write(clone, common.recent_bone_counter, *b.current_bone_counter);
    write(clone, b.offsets.entity_flags,
          read<std::uint32_t>(clone, b.offsets.entity_flags) & ~0x20u);
    write(clone, common.effects_bb78dc43, std::uint32_t(4));
    if (read<std::uint8_t>(clone, b.offsets.render_mode) != 3u) {
        write(clone, b.offsets.render_mode, std::uint8_t(3));
        auto* property = engine::invoke<void*>(clone, 9);
        engine::invoke<void>(property, 2,
                             std::uint32_t(read<std::uint8_t>(clone, b.offsets.render_effect)),
                             std::int32_t(3), std::numeric_limits<float>::max(), 0.0f);
    }
    force_opaque(clone, b);
    engine::invoke<void>(clone, 55, false);
    const animation::BoneRequest request{{0.0f, record.header.animation_yaw, 0.0f},
                                         record.header.origin,
                                         read<float>(target, common.simulation_time),
                                         0xFFF00u,
                                         -1,
                                         1u,
                                         false};
    std::int32_t built_count = 0;
    if (b.bone_runtime.build_bone_matrices(target, request, output, &built_count)) {
        b.published.origin = record.header.origin;
        b.published.expires =
            engine::invoke<double>(b.clock, 5) + double(b.settings.on_shot_duration.value) * 0.1;
    }
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: shot_feedback.cpp
// ============================================================================

namespace skeet {
template <class Values>
bool shot_record_expired_body(const ShotRecordView&, const client::PlayerState&,
                              ShotFeedbackRuntime&, Values&);
namespace {
struct SnapshotFeedbackValues {
    const ShotFeedbackRuntime& runtime;
    bool enabled(const ShotFeedbackState& state) const noexcept {
        return state.events.enabled;
    }
    float now() const noexcept {
        return runtime.now;
    }
    float interval() const noexcept {
        return runtime.interval;
    }
    float receive_interval() const noexcept {
        return runtime.receive_interval;
    }
    float lerp() const noexcept {
        return maximum_number(runtime.interpolation,
                              runtime.interpolation_ratio / runtime.update_rate);
    }
    float correction_minimum() const noexcept {
        return runtime.correction_minimum;
    }
    float correction_extra() const noexcept {
        return runtime.correction_extra;
    }
    std::int32_t maximum_unlag_milliseconds() const noexcept {
        return runtime.maximum_unlag_milliseconds;
    }
    bool ray_tests_disabled() const noexcept {
        return runtime.ray_tests_disabled;
    }
    bool reworked() const noexcept {
        return runtime.resolver_rework && *runtime.resolver_rework;
    }
};
static_assert(offsetof(client::NativePlayerStateLayout, resolver_slots) == 0x10);
static_assert(offsetof(players::Record, adjustments) +
                  offsetof(players::Adjustments, correction_active) ==
              0xa);
/* float real(std::uint32_t value) noexcept {
    return std::bit_cast<float>(value);
} */
using skeet::field;
using skeet::bits;
RageVector vector_words(const std::uint32_t* words) noexcept {
    return {real(words[0]), real(words[1]), real(words[2])};
}
RageVector difference(const RageVector& a, const RageVector& b) noexcept {
    return {a[0] - b[0], a[1] - b[1], a[2] - b[2]};
}
float length(const RageVector& v) noexcept {
    return std::sqrt((v[0] * v[0] + v[1] * v[1]) + v[2] * v[2]);
}
RageVector normalized(const RageVector& delta) noexcept {
    const float inverse = 1.0f / (length(delta) + std::numeric_limits<float>::epsilon());
    return {inverse * delta[0], inverse * delta[1], inverse * delta[2]};
}
float distance_squared_yxz(const RageVector& v) noexcept {
    return (v[1] * v[1] + v[0] * v[0]) + v[2] * v[2];
}
float distance_squared_zyx(const RageVector& v) noexcept {
    return (v[2] * v[2] + v[1] * v[1]) + v[0] * v[0];
}
template <class T>
T target_field(const client::PlayerState& target, std::ptrdiff_t offset) noexcept {
    T value;
    std::memcpy(&value, reinterpret_cast<const std::byte*>(&target.native) + offset, sizeof value);
    return value;
}
template <class T>
void target_field(client::PlayerState& target, std::ptrdiff_t offset, const T& value) noexcept {
    std::memcpy(reinterpret_cast<std::byte*>(&target.native) + offset, &value, sizeof value);
}
client::PlayerState& target_state(ShotFeedbackRuntime& runtime, std::int32_t index) {
    if (index < 1 || index > 64)
        throw std::out_of_range("Native pending shot requires target index 1..64");
    return *runtime.players.get(std::uint32_t(index));
}
void log_extrapolation(ShotFeedbackState& state, const ShotRecordView& shot,
                       ShotFeedbackRuntime& runtime, const char* outcome, std::uint32_t reason,
                       int damage = 0, int hitgroup = 0) {
    // Choke and generated commands rewrite word3. Word1 remains the shot's
    // publication identity throughout matching, logging and retirement.
    for (auto& tag : state.extrapolation_shots)
        if (tag.original_command == integer(shot.record.words[1]) &&
            tag.id == integer(shot.record.words[5])) {
            if (tag.reported)
                return;
            tag.reported = true;
            constexpr const char* reasons[]{"unconfirmed/resolver", "spread or obstruction",
                                            "prediction mismatch", "death", "weapon state changed"};
            runtime.normal_log.console_format(
                "[extrapolation] %s shot=%d target=%d +%d ticks record=%d damage=%d hitgroup=%d reason=%s",
                outcome, tag.id, shot.target(), tag.ticks, tag.simulation_tick, damage, hitgroup,
                reason < 5 ? reasons[reason] : "none");
            return;
        }
}
void notify_miss(ShotFeedbackState& state, const ShotRecordView& shot, std::uint32_t reason,
                 ShotFeedbackRuntime& runtime) {
    log_extrapolation(state, shot, runtime, "MISS", reason);
    const auto event = shot.script_shot();
    script::dispatch_aim_miss(state.scripts, event, reason);
    state.notices.push_back({ShotFeedbackNoticeKind::miss, event, reason, 0, 0, 0});
}
void notify_hit(ShotFeedbackState& state, const ShotRecordView& shot, const events::ShotEvent& hurt,
                ShotFeedbackRuntime& runtime) {
    const auto event = shot.script_shot();
    const auto target = integer(hurt.payload[0]), group = integer(hurt.payload[1]),
               damage = integer(hurt.payload[2]);
    log_extrapolation(state, shot, runtime, target == shot.target() ? "HIT" : "HIT OTHER TARGET",
                      UINT32_MAX, damage, group);
    script::dispatch_aim_hit(state.scripts, event, target, group, damage);
    state.notices.push_back({ShotFeedbackNoticeKind::hit, event, 0, target, group, damage});
}
void erase_shot(ShotFeedbackState& state, std::size_t index) {
    const ShotRecordView shot{state.pending[index]};
    const auto command = integer(shot.record.words[1]), id = integer(shot.record.words[5]);
    std::erase_if(state.extrapolation_shots, [&](const auto& value) {
        return value.original_command == command && value.id == id;
    });
    for (std::size_t next = index + 1; next < state.pending.size(); ++next)
        lagcomp::assign_evaluation(state.pending[next - 1], state.pending[next]);
    state.pending.pop_back();
}
lagcomp::LagRecord& history_record(client::PlayerState& target, std::uint32_t age) {
    return *target.history.records[(target.history.first + age) & 31U];
}
const lagcomp::LagRecord& history_record(const client::PlayerState& target, std::uint32_t age) {
    return *target.history.records[(target.history.first + age) & 31U];
}
unsigned attempt_slot(std::uint8_t mode, std::uint8_t attempt) noexcept {
    return mode ? unsigned(attempt) % 3U : 0U;
}
std::ptrdiff_t attempt_offset(std::uint8_t mode, std::uint8_t attempt) noexcept {
    return 0x1c + std::uint32_t(mode) * 0x24 + attempt_slot(mode, attempt) * 8;
}
lagcomp::PacketTimingSample packet_sample(const client::PlayerState& target,
                                          std::uint32_t index) noexcept {
    return target_field<lagcomp::PacketTimingSample>(target, 0x23c + index * 0x20);
}
void confirm_target_hit(client::PlayerState& target, ShotFeedbackRecord& record) {
    for (std::uint32_t i = 0; i < target.history.count; ++i) {
        auto& current = history_record(target, i);
        if (current.header.simulation_tick <= ShotRecordView{record}.record_tick()) {
            current.header.flags |= 1U;
            break;
        }
    }
    const auto mode = record.state.record.header.resolver_index;
    const auto attempt = record.state.record.header.resolver_mode;
    target_field(target, attempt_offset(mode, attempt), 0.0f);
    target.native.resolver_slots[mode].attempt = attempt;
    target_field(target, 0xa4, std::uint8_t(target_field<std::uint8_t>(target, 0xa4) + 1U));
    target_field(target, 0x1a0, 0.0f);
    target_field(target, 0x108, bits(record.state.record.header.eye_pitch));
}
Matrix3x4 hitbox_rotation(const RageVector& angles, const RageTrigonometry& trig) {
    constexpr float radians = 0.01745329238474369f;
    const float sy = trig.sine(angles[1] * radians), cy = trig.cosine(angles[1] * radians);
    const float sp = trig.sine(angles[0] * radians), cp = trig.cosine(angles[0] * radians);
    const float sr = trig.sine(angles[2] * radians), cr = trig.cosine(angles[2] * radians);
    const float crcy = cr * cy, crsy = cr * sy, srcy = sr * cy, srsy = sr * sy;
    return {{{cp * cy, srcy * sp - crsy, crcy * sp + srsy, 0.0f},
             {cp * sy, srsy * sp + crcy, crsy * sp - srcy, 0.0f},
             {-sp, sr * cp, cr * cp, 0.0f}}};
}
Matrix3x4 compose(const Matrix3x4& matrix, const Matrix3x4& rotation) {
    Matrix3x4 result;
    for (unsigned row = 0; row < 3; ++row)
        for (unsigned column = 0; column < 4; ++column) {
            const auto& m = matrix.rows[row];
            const float value =
                (m[2] * rotation.rows[2][column] + m[1] * rotation.rows[1][column]) +
                m[0] * rotation.rows[0][column];
            result.rows[row][column] = value + (column == 3 ? m[3] : 0.0f);
        }
    return result;
}
// Complete nearest-point part of 4338590E, retaining its signed endpoint result.
float segment_distance(const RageVector& point, const RageVector& first, const RageVector& second,
                       RageVector& nearest) {
    const auto delta = difference(second, first), offset = difference(point, first);
    const float extent = length(delta),
                inverse = 1.0f / (extent + std::numeric_limits<float>::epsilon());
    const RageVector unit{delta[0] * inverse, delta[1] * inverse, delta[2] * inverse};
    const float projection = (unit[1] * offset[1] + unit[0] * offset[0]) + unit[2] * offset[2];
    if (projection < 0.0f) {
        nearest = first;
        return -length(offset);
    }
    if (projection > extent) {
        nearest = second;
        return -length(difference(point, second));
    }
    nearest = {unit[0] * projection + first[0], unit[1] * projection + first[1],
               unit[2] * projection + first[2]};
    return length(difference(point, nearest));
}
template <class Values>
void validate_shot_record(ShotFeedbackRecord& record, ShotFeedbackRuntime& runtime,
                          Values& values) {
    ShotRecordView shot{record};
    auto& target = target_state(runtime, shot.target());
    if (shot.record_tick() <= 0)
        return;
    if (shot_record_expired_body(shot, target, runtime, values))
        shot.flags() |= 2U;
    if (shot.target_handle() != 0xffffffffU) {
        const auto entity = runtime.engine.entity_by_handle(shot.target_handle());
        if (entity.exists) {
            const float lerp = values.lerp();
            float correct = maximum_number(values.correction_minimum(), lerp) +
                            (values.correction_extra() + 0.0f);
            correct = _mm_cvtss_f32(
                _mm_min_ss(_mm_max_ss(_mm_set_ss(correct), _mm_setzero_ps()),
                           _mm_set_ss(float(values.maximum_unlag_milliseconds()) * 0.001f)));
            const float interval = values.interval();
            const float rounded =
                float(lagcomp::time_to_tick(entity.simulation_time, interval)) * interval;
            const float recorded = float(shot.record_tick()) * interval;
            if (std::fabs(correct - (rounded - recorded)) > 0.2f)
                shot.flags() |= 4U;
        }
    }
    if (target.history.count >= 2) {
        const lagcomp::LagRecord* previous = nullptr;
        for (std::uint32_t age = 0; age < target.history.count; ++age) {
            const auto& current = history_record(target, age);
            if (previous && distance_squared_yxz(difference(current.header.origin,
                                                            previous->header.origin)) > 4096.0f) {
                shot.flags() |= 8U;
                break;
            }
            if (current.header.simulation_tick <= shot.record_tick()) {
                if (current.header.simulation_tick < shot.record_tick())
                    shot.flags() |= 0x10U;
                break;
            }
            previous = &current;
        }
    }
}
}

std::int32_t ShotRecordView::command() const noexcept {
    return integer(record.words[3]);
}
float ShotRecordView::time() const noexcept {
    return real(record.words[4]);
}
std::int32_t ShotRecordView::shot_tick() const noexcept {
    return integer(record.words[6]);
}
std::int32_t ShotRecordView::target_life_id() const noexcept {
    return integer(record.words[8]);
}
RageVector ShotRecordView::origin() const noexcept {
    return vector_words(record.words.data() + 9);
}
std::int32_t ShotRecordView::model() const noexcept {
    return integer(record.words[12]);
}
std::int32_t ShotRecordView::hitbox_set() const noexcept {
    return integer(record.words[13]);
}
std::int32_t ShotRecordView::target() const noexcept {
    return integer(record.state.words[1]);
}
std::int32_t ShotRecordView::target_health() const noexcept {
    return integer(record.state.words[2]);
}
std::int32_t ShotRecordView::record_tick() const noexcept {
    return integer(record.state.words[45]);
}
RageVector ShotRecordView::target_point() const noexcept {
    return vector_words(record.state.words.data() + 3);
}
script::AimShot ShotRecordView::script_shot() const noexcept {
    return {integer(record.words[5]), target(), integer(record.state.words[12]),
            real(record.state.words[14]), record_tick()};
}

float quantize_shot_coordinate(float value) noexcept {
    const auto whole = _mm_cvtt_ss2si(_mm_set_ss(std::fabs(value)));
    const auto scaled = _mm_cvtt_ss2si(_mm_set_ss(value * 32.0f));
    const auto sign = scaled < 0 ? 0xffffffffU : 0U;
    const auto fraction = ((std::uint32_t(scaled) ^ sign) - sign) & 31U;
    float result = 0.0f;
    if (whole || fraction) {
        result = float(double(fraction) * 0.03125 + double(whole));
        if (value <= -0.03125f)
            result = std::bit_cast<float>(bits(result) ^ 0x80000000U);
    }
    return result;
}
RageVector quantize_shot_origin(const RageVector& value) noexcept {
    return {quantize_shot_coordinate(value[0]), quantize_shot_coordinate(value[1]),
            quantize_shot_coordinate(value[2])};
}

template <class Values>
bool shot_record_expired_body(const ShotRecordView& shot, const client::PlayerState& target,
                              ShotFeedbackRuntime& runtime, Values& values) {
    const auto engine_time = runtime.engine.engine_time();
    const auto maximum_unlag = values.maximum_unlag_milliseconds();
    const auto received = lagcomp::receive_time_to_tick(engine_time, values.receive_interval());
    const float threshold = float(received) * values.interval() - float(maximum_unlag) * 0.001f;
    const auto lower = _mm_cvtt_ss2si(_mm_set_ss(threshold));
    for (std::uint32_t age = 0; age < target.history.count; ++age) {
        const auto tick = history_record(target, age).header.simulation_tick;
        if (tick > shot.record_tick())
            continue;
        const float time = float(tick) * values.interval();
        if (!(float(lower) > time))
            return false;
        const float newest_time =
            float(history_record(target, 0).header.simulation_tick) * values.interval();
        const auto entity = runtime.engine.entity_by_index(shot.target());
        return newest_time > time || (entity.exists && entity.field_d5fd2e2c > time);
    }
    return false;
}

template <class Values>
void classify_shot_ray_body(ShotFeedbackState& state, ShotFeedbackRecord& record,
                            const RageVector* origin, const RageVector* direction,
                            ShotFeedbackRuntime& runtime, Values& values) {
    ShotRecordView shot{record};
    std::uint32_t reason = 0;
    if (values.reworked() && (shot.flags() & 0x20u))
        return;
    const auto finite = [](const RageVector& value) {
        return std::all_of(value.begin(), value.end(),
                           [](float axis) { return std::isfinite(axis); });
    };
    if (values.reworked() &&
        ((origin && !finite(*origin)) ||
         (direction && (!finite(*direction) || distance_squared_zyx(*direction) < 1e-8f)))) {
        origin = nullptr;
        direction = nullptr;
    }
    bool verified_intersection = false;
    if (origin && std::sqrt(distance_squared_zyx(difference(*origin, shot.origin()))) > 0.5f)
        reason = 2;
    else if (origin && direction && !values.ray_tests_disabled()) {
        BallisticQuery query;
        query.allow_penetration = 1;
        query.weapon_id = std::uint16_t(record.words[27]);
        query.weapon_padding = std::uint16_t(record.words[27] >> 16);
        query.penetration = real(record.words[28]);
        query.range = real(record.words[29]);
        query.range_modifier = real(record.words[30]);
        query.armor_ratio = real(record.words[31]);
        query.base_damage = real(record.words[32]);
        query.reserved = record.words[33];
        const RageVector end{(*direction)[0] * query.range + (*origin)[0],
                             (*direction)[1] * query.range + (*origin)[1],
                             (*direction)[2] * query.range + (*origin)[2]};
        const BallisticFilter filter{nullptr,    0,    0x600400bU,
                                     0x600400bU, 1.0f, BallisticFilterKind::reject_players};
        trace_ballistic_bullet(runtime.ballistics, 0, shot.origin(), end, filter, query);
        const RageRay ray{*origin, difference(query.trace.end, *origin)};
        const auto hitboxes = runtime.engine.hitboxes(shot.model(), shot.hitbox_set());
        const auto count = std::min<std::size_t>(19, hitboxes.size());
        reason = 1;
        for (std::size_t index = 0; index < count; ++index) {
            const auto& box = hitboxes[index];
            auto matrix = record.state.record.matrices[index];
            if (!(box.radius > 0.0f))
                matrix = compose(matrix, hitbox_rotation(box.rotation, runtime.trigonometry));
            RageTrace trace;
            if (runtime.engine.clip_hitbox(ray, box, matrix, trace) >= 0) {
                reason = 0;
                verified_intersection = true;
                break;
            }
        }
        if (reason == 1)
            rage::log_spread_miss(runtime.spread_log_settings, runtime.normal_log);
    }
    if (!(shot.flags() & 0x20U)) {
        notify_miss(state, shot, reason, runtime);
        shot.flags() |= 0x20U;
    }
    if (reason)
        return;
    // Missing impact data is an unknown miss, not evidence against a pose.
    if (values.reworked() && (!verified_intersection || !(record.state.record.header.flags & 4u) ||
                              (shot.flags() & 0x1Eu)))
        return;
    auto& target = target_state(runtime, shot.target());
    runtime.player_adjustments.records[std::size_t(shot.target())].adjustments.correction_active =
        1;
    target_field(target, 0xa8, std::uint16_t(target_field<std::uint16_t>(target, 0xa8) + 1U));
    target_field(target, 0x1a0, values.now());
    target_field(target, 0xa4, std::uint8_t(0));
    const auto mode = record.state.record.header.resolver_index;
    const auto attempt = record.state.record.header.resolver_mode;
    const auto slot = attempt_offset(mode, attempt);
    target_field(target, slot, values.now());
    target_field(target, slot + 4, target_field<std::uint32_t>(target, slot + 4) + 1U);
    if (!(shot.flags() & 0x40U)) {
        const resolver_runtime::AttemptShot attempt_shot{
            record.state.record, shot.flags(), shot.shot_flags(), std::uint32_t(shot.target()),
            real(record.words[29])};
        const auto next = resolver_runtime::choose_next_attempt(
            {target_field<float>(target, 0x88), target_field<float>(target, 0x1a0)}, attempt_shot,
            values.now(), values.enabled(state), origin, direction, runtime.attempt_engine);
        target.native.resolver_slots[mode].attempt = next;
        if (next != attempt)
            target.native.resolver_status = 0;
    }
    if (shot.shot_flags() & 0x10U)
        target_field(target, 0xd0, values.now());
    if (record.state.record.header.resolver_state == 1) {
        target_field(target, 0xb8, target_field<std::uint32_t>(target, 0xb8) + 1U);
        target_field(target, 0xb4, values.now());
    }
    if (record.state.record.header.resolver_state == 2)
        target_field(target, 0xcc, values.now());
    if ((shot.shot_flags() & 5U) == 4U && mode) {
        for (std::uint32_t age = 0; age < target.history.count; ++age) {
            auto& current = history_record(target, age);
            if (current.header.resolver_index == mode && current.header.resolver_mode == attempt)
                current.header.flags |= 0x40U;
            if (current.header.flags & 0x200U)
                break;
        }
    }
}

template <class Values>
void reconcile_shot_feedback_body(ShotFeedbackState& state, ShotFeedbackRuntime& runtime,
                                  Values& values) {
    auto finish = [&] {
        state.events.feedback.clear();
        state.recent_position_count = 0;
        state.events.has_pending_shots = !state.pending.empty();
    };
    const auto enabled = values.enabled(state);
    state.events.enabled = enabled;
    if (!enabled) {
        state.fire_snapshots.clear();
        finish();
        return;
    }
    while (!state.pending.empty()) {
        ShotRecordView shot{state.pending.front()};
        if (std::fabs(shot.time() - values.now()) < 0.5f)
            break;
        classify_shot_ray_body(state, state.pending.front(), nullptr, nullptr, runtime, values);
        erase_shot(state, 0);
    }
    if (state.pending.empty()) {
        state.fire_snapshots.clear();
        finish();
        return;
    }
    const auto command = runtime.engine.current_command();
    std::size_t index = 0;
    while (index < state.pending.size()) {
        auto& record = state.pending[index];
        ShotRecordView shot{record};
        if (command < shot.command())
            break;
        if (!(shot.flags() & 0x100U)) {
            // The life-id check occurs after timing capture and before validity.
            auto& target = target_state(runtime, shot.target());
            shot.flags() |= 0x100U;
            record.words[16] = std::uint32_t(
                lagcomp::time_to_tick(target_field<float>(target, 0x110), values.interval()));
            record.words[17] = target_field<std::uint32_t>(target, 0x1ac);
            for (unsigned axis = 0; axis < 3; ++axis)
                record.words[18 + axis] = bits(target.native.cached_origin[axis]);
            const auto local_index = runtime.engine.local_player_index();
            if (local_index < shot.target()) {
                auto& local = target_state(runtime, runtime.engine.local_player_index());
                const auto target_head = target_field<std::uint32_t>(target, 0x234);
                const auto local_head = target_field<std::uint32_t>(local, 0x234);
                if (target.native.field_238 >= 2 && local.native.field_238 &&
                    packet_sample(target, target_head % 6U).field_18 ==
                        packet_sample(local, local_head % 6U).field_18) {
                    const auto sample = packet_sample(target, (target_head + 1U) % 6U);
                    record.words[16] = std::uint32_t(sample.field_00);
                    record.words[17] = sample.field_1c;
                    for (unsigned axis = 0; axis < 3; ++axis)
                        record.words[18 + axis] = sample.fields_04[axis];
                }
            }
            const auto local = runtime.engine.local_player();
            if (!local.hidden && shot.target_handle() != 0xffffffffU &&
                shot.target_life_id() != -1) {
                const auto entity = runtime.engine.entity_by_handle(shot.target_handle());
                if (entity.exists && entity.life_id != -1 &&
                    entity.life_id > shot.target_life_id()) {
                    if (!(shot.flags() & 0x20U)) {
                        notify_miss(state, shot, 4, runtime);
                        shot.flags() |= 0x20U;
                    }
                    erase_shot(state, index);
                    continue;
                }
            }
            validate_shot_record(record, runtime, values);
        }
        ++index;
    }
    if (state.pending.empty()) {
        state.fire_snapshots.clear();
        finish();
        return;
    }
    const auto& queue = state.events.feedback;
    for (std::size_t event_index = 0; event_index < queue.size();) {
        const auto& event = queue[event_index];
        if (event.kind == events::ShotEventKind::death) {
            const auto victim = integer(event.payload[0]);
            if (victim == runtime.engine.local_player_index()) {
                for (auto& record : state.pending) {
                    ShotRecordView shot{record};
                    if (!(shot.flags() & 0x20U)) {
                        notify_miss(state, shot, 3, runtime);
                        shot.flags() |= 0x20U;
                    }
                }
                state.pending.clear();
                state.extrapolation_shots.clear();
                state.fire_snapshots.clear();
                finish();
                return;
            }
            for (auto& record : state.pending) {
                ShotRecordView shot{record};
                if (shot.target() == victim && !(shot.flags() & 0x20U)) {
                    notify_miss(state, shot, 3, runtime);
                    shot.flags() |= 0x20U;
                }
            }
            ++event_index;
            continue;
        }
        if (event.kind != events::ShotEventKind::fire) {
            ++event_index;
            continue;
        }
        ShotFireSnapshot snapshot;
        const bool has_snapshot = !state.fire_snapshots.empty();
        if (has_snapshot) {
            snapshot = state.fire_snapshots.front();
            state.fire_snapshots.pop_front();
        }
        ++event_index;
        if (state.pending.empty())
            continue;
        auto& record = state.pending.front();
        ShotRecordView shot{record};
        if (event.payload[0] != shot.fire_weapon())
            continue;
        const events::ShotEvent* first_impact = nullptr;
        const events::ShotEvent* last_impact = nullptr;
        const events::ShotEvent* hurt = nullptr;
        while (event_index < queue.size()) {
            const auto& following = queue[event_index];
            if (following.kind == events::ShotEventKind::impact) {
                if (!first_impact)
                    first_impact = &following;
                last_impact = &following;
            } else if (following.kind == events::ShotEventKind::hurt) {
                if (following.payload[3] == shot.hurt_weapon())
                    hurt = &following;
            } else
                break;
            ++event_index;
        }
        RageVector origin = shot.origin(), direction{};
        const RageVector* origin_argument = &origin;
        const RageVector* direction_argument = nullptr;
        if (has_snapshot) {
            const auto quantized = quantize_shot_origin(origin);
            if (length(difference(quantized, snapshot.origin)) > 0.5f)
                origin = snapshot.origin;
            direction = snapshot.direction;
            direction_argument = &direction;
        }
        if (first_impact && last_impact) {
            const auto first = vector_words(first_impact->payload.data());
            const auto last = vector_words(last_impact->payload.data());
            if (first_impact != last_impact) {
                direction = normalized(difference(last, first));
                const RageVector backward{first[0] - direction[0] * 1000.0f,
                                          first[1] - direction[1] * 1000.0f,
                                          first[2] - direction[2] * 1000.0f};
                RageVector nearest;
                if (segment_distance(origin, first, backward, nearest) > 0.0f)
                    origin = nearest;
            } else
                direction = normalized(difference(first, origin));
            direction_argument = &direction;
        }
        if (hurt) {
            notify_hit(state, shot, *hurt, runtime);
            if (!(shot.flags() & 0x20U) && runtime.engine.entity_by_index(shot.target()).exists)
                confirm_target_hit(target_state(runtime, shot.target()), record);
        } else if (!(shot.flags() & 0x20U))
            classify_shot_ray_body(state, record, origin_argument, direction_argument, runtime,
                                   values);
        erase_shot(state, 0);
    }
    if (state.pending.empty())
        state.fire_snapshots.clear();
    else if (state.recent_position_count == 1) {
        ShotFeedbackRecord* candidate = nullptr;
        unsigned count = 0;
        for (auto& record : state.pending)
            if (ShotRecordView{record}.command() == runtime.engine.current_command()) {
                if (++count == 2)
                    break;
                candidate = &record;
            }
        if (count == 1 && candidate) {
            ShotRecordView shot{*candidate};
            if (!(shot.flags() & 0x20U) &&
                distance_squared_yxz(difference(state.recent_positions[1], shot.target_point())) <
                    3600.0f) {
                const auto entity = runtime.engine.entity_by_index(shot.target());
                if (entity.exists && entity.health < shot.target_health()) {
                    const auto local = runtime.engine.local_player();
                    if (local.weapon_handle != 0xffffffffU) {
                        const auto weapon = runtime.engine.entity_by_handle(local.weapon_handle);
                        if (weapon.exists &&
                            lagcomp::time_to_tick(weapon.simulation_time, values.interval()) >=
                                shot.shot_tick() &&
                            lagcomp::time_to_tick(entity.last_shot_time, values.interval()) ==
                                shot.shot_tick()) {
                            log_extrapolation(state, shot, runtime, "HIT INFERRED", UINT32_MAX,
                                              shot.target_health() - int(entity.health));
                            confirm_target_hit(target_state(runtime, shot.target()), *candidate);
                            shot.flags() |= 0x20U;
                        }
                    }
                }
            }
        }
    }
    finish();
}

bool shot_record_expired(const ShotRecordView& shot, const client::PlayerState& target,
                         ShotFeedbackRuntime& runtime) {
    SnapshotFeedbackValues values{runtime};
    return shot_record_expired_body(shot, target, runtime, values);
}
bool shot_record_expired(const ShotRecordView& shot, const client::PlayerState& target,
                         ShotFeedbackRuntime& runtime, const rage::FeedbackEntityBindings& values) {
    return shot_record_expired_body(shot, target, runtime, values);
}
void classify_shot_ray(ShotFeedbackState& state, ShotFeedbackRecord& record,
                       const RageVector* origin, const RageVector* direction,
                       ShotFeedbackRuntime& runtime) {
    SnapshotFeedbackValues values{runtime};
    classify_shot_ray_body(state, record, origin, direction, runtime, values);
}
void classify_shot_ray(ShotFeedbackState& state, ShotFeedbackRecord& record,
                       const RageVector* origin, const RageVector* direction,
                       ShotFeedbackRuntime& runtime, const rage::FeedbackEntityBindings& values) {
    classify_shot_ray_body(state, record, origin, direction, runtime, values);
}
void reconcile_shot_feedback(ShotFeedbackState& state, ShotFeedbackRuntime& runtime) {
    SnapshotFeedbackValues values{runtime};
    reconcile_shot_feedback_body(state, runtime, values);
}
void reconcile_shot_feedback(ShotFeedbackState& state, ShotFeedbackRuntime& runtime,
                             const rage::FeedbackEntityBindings& values) {
    reconcile_shot_feedback_body(state, runtime, values);
}
} // namespace skeet

// ============================================================================
// Subcomponent: spread.cpp
// ============================================================================

namespace skeet::rage {
namespace {
constexpr float full_turn = 6.283185482025146484375f;
constexpr std::array<std::uint8_t, 128> cache_seeds{
    2,   3,   4,   10,  13,  15,  27,  29, 37, 38,  41,  46,  47,  49,  50,  52,  56,  57,  58,
    62,  66,  68,  70,  72,  76,  83,  87, 97, 108, 109, 110, 111, 113, 121, 122, 125, 135, 1,
    5,   6,   8,   17,  25,  26,  28,  31, 32, 36,  39,  44,  51,  61,  67,  71,  74,  77,  78,
    79,  80,  81,  84,  89,  90,  91,  92, 96, 98,  101, 102, 104, 107, 114, 115, 119, 9,   11,
    16,  19,  20,  22,  30,  48,  54,  59, 60, 65,  69,  73,  75,  82,  85,  86,  88,  94,  99,
    103, 105, 112, 116, 117, 118, 124, 0,  7,  12,  14,  18,  21,  23,  24,  33,  34,  35,  40,
    42,  43,  45,  53,  55,  63,  64,  93, 95, 100, 106, 120, 133, 137};
float limit_inaccuracy(float value) noexcept {
    return _mm_cvtss_f32(_mm_min_ss(_mm_set_ss(1.0f), _mm_set_ss(value)));
}
RageSpreadSample combine(const SpreadSeed& sample, const SpreadParameters& parameters,
                         float inaccuracy) noexcept {
    const float first = transform_spread_radius(parameters.weapon_id, parameters.fire_mode,
                                                parameters.recoil_index, sample.first_radius) *
                        inaccuracy;
    const float second = transform_spread_radius(parameters.weapon_id, parameters.fire_mode,
                                                 parameters.recoil_index, sample.second_radius) *
                         parameters.spread;
    return {second * sample.second_cosine + first * sample.first_cosine,
            second * sample.second_sine + first * sample.first_sine};
}
}
void fill_spread_plane(std::span<SpreadSeed, 256> table, bool shotgun_patterns,
                       const SpreadRandom& random, const RageTrigonometry& trig) {
    for (unsigned seed = 0; seed < 256; ++seed) {
        auto& sample = table[seed];
        random.seed(random.context, std::int32_t(seed + 1));
        sample.first_radius = random.uniform(random.context, 0.0f, 1.0f);
        const float first_angle = random.uniform(random.context, 0.0f, full_turn);
        sample.first_sine = trig.sine(first_angle);
        sample.first_cosine = trig.cosine(first_angle);
        float second_angle;
        if (!shotgun_patterns) {
            sample.second_radius = random.uniform(random.context, 0.0f, 1.0f);
            second_angle = random.uniform(random.context, 0.0f, full_turn);
        } else {
            second_angle = random.uniform(random.context, 0.0f, full_turn);
            sample.second_radius = random.uniform(random.context, 0.0f, 1.0f);
        }
        sample.second_sine = trig.sine(second_angle);
        sample.second_cosine = trig.cosine(second_angle);
    }
}
float transform_spread_radius(std::uint32_t weapon, std::int32_t mode, float recoil,
                              float radius) noexcept {
    if (weapon == 0x40) {
        if (mode == 1)
            return 1.0f - radius * radius;
    } else if (weapon == 0x1c && recoil < 3.0f) {
        int iteration = 3;
        do {
            --iteration;
            radius *= radius;
        } while (float(iteration) > recoil);
        return 1.0f - radius;
    }
    return radius;
}
RageSpreadSample generate_spread_sample(const SpreadParameters& parameters, std::uint32_t seed,
                                        const SpreadTables& tables, const SpreadRandom& random,
                                        const ShotgunPattern& pattern,
                                        const RageTrigonometry& trig) {
    const float inaccuracy = limit_inaccuracy(parameters.inaccuracy);
    if (!parameters.shotgun_patterns)
        return combine(tables.standard[seed], parameters, inaccuracy);
    if (!pattern.get || parameters.pellet_count <= 1)
        return combine(tables.shotgun_patterns[seed], parameters, inaccuracy);
    random.seed(random.context, std::bit_cast<std::int32_t>(seed + 1));
    const auto recoil = _mm_cvttss_si32(_mm_set_ss(parameters.recoil_index));
    const auto first_pellet = std::uint32_t(recoil) * std::uint32_t(parameters.pellet_count);
    RageSpreadSample sum{0.0f, 0.0f};
    for (std::int32_t i = 0; i < parameters.pellet_count; ++i) {
        SpreadSeed sample;
        sample.first_radius = random.uniform(random.context, 0.0f, 1.0f);
        const float angle = random.uniform(random.context, 0.0f, full_turn);
        sample.first_sine = trig.sine(angle);
        sample.first_cosine = trig.cosine(angle);
        float second_angle = 0.0f;
        sample.second_radius = 0.0f;
        pattern.get(pattern.context, parameters.weapon_id, parameters.fire_mode,
                    std::bit_cast<std::int32_t>(first_pellet + std::uint32_t(i)), second_angle,
                    sample.second_radius);
        sample.second_sine = trig.sine(second_angle);
        sample.second_cosine = trig.cosine(second_angle);
        const auto point = combine(sample, parameters, inaccuracy);
        sum.first = point.first + sum.first;
        sum.second = point.second + sum.second;
    }
    const float inverse = 1.0f / float(parameters.pellet_count);
    return {sum.first * inverse, sum.second * inverse};
}
bool prepare_spread_cache(SpreadCache& cache, const SpreadParameters& parameters,
                          float combined_spread, std::uint32_t generation, bool enabled,
                          const SpreadTables& tables, const SpreadRandom& random,
                          const ShotgunPattern& pattern, const RageTrigonometry& trig) {
    if (!enabled || cache.generation == generation)
        return false;
    auto compensated = parameters;
    compensated.inaccuracy = combined_spread - parameters.spread;
    // 433C1B49 forces one pellet outside the class-4 shotgun cache branch.
    if (parameters.weapon_type != 4 || parameters.pellet_count <= 1)
        compensated.pellet_count = 1;
    for (unsigned i = 0; i < 128; ++i) {
        cache.primary[i] =
            generate_spread_sample(parameters, cache_seeds[i], tables, random, pattern, trig);
        cache.compensated[i] =
            generate_spread_sample(compensated, cache_seeds[i], tables, random, pattern, trig);
    }
    cache.generation = generation;
    return true;
}
std::array<RageSpreadSample, 32> hitchance_spread_samples(const SpreadCache& cache) noexcept {
    std::array<RageSpreadSample, 32> result;
    for (unsigned i = 0; i < 32; ++i)
        result[i] = cache.compensated[i * 4];
    return result;
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: spread_miss_log.cpp
// ============================================================================

namespace skeet::rage {
void log_spread_miss(const ShotLogSettings& settings, misc::NormalLog& output) {
    if (!settings.spread_misses.value)
        return;
    //4347F7CC..4347F856 recovers these25 bytes for the owner+5C string.
    output.format("%s", "Missed shot due to spread");
}
}

// ============================================================================
// Subcomponent: spread_storage.cpp
// ============================================================================

namespace skeet::rage {
namespace {
SpreadSeed* allocate_plane(const engine::JobMemory& memory) { //43387448(size1800)
    constexpr std::uint32_t allocation_size = 0x1820;
    auto* raw = memory.allocate(memory.process_heap, 8, allocation_size);
    if (!raw)
        return nullptr; //the actual allocation helper's null return
    const auto address = reinterpret_cast<std::uintptr_t>(raw);
    const auto header = (address + 15) & ~std::uintptr_t(15);
    const auto displacement = static_cast<std::uint32_t>(header - address);
    std::memcpy(reinterpret_cast<void*>(header + 8), &displacement, 4);
    std::memcpy(reinterpret_cast<void*>(header + 12), &allocation_size, 4);
    return reinterpret_cast<SpreadSeed*>(header + 16);
}
void release_plane(SpreadSeed* plane, const engine::JobMemory& bindings) {
    engine::UnloadSystemFunctions memory;
    memory.free_heap = bindings.free;
    memory.process_heap = bindings.process_heap;
    engine::free_native_allocation(plane, memory);
}
}
SpreadStorage::SpreadStorage(engine::PreservedInitialization& initialization)
    : SpreadStorage(initialization.image_owner()) {}
SpreadStorage::SpreadStorage(std::shared_ptr<engine::NativeDataImage> image)
    : image_(image),
      standard_(engine::require_native_data_image(image_).native_cell<SpreadSeed*>(0x43478C60u)),
      shotgun_patterns_(image_->native_cell<SpreadSeed*>(0x43478C64u)),
      tables_(standard_, shotgun_patterns_) {}
const SpreadSeed* const* SpreadStorage::initialize(const SpreadStorageBindings& bindings) {
    standard_ = allocate_plane(bindings.memory);
    auto* const second = allocate_plane(bindings.memory);
    // Native reloads the first global only after the second heap call. It
    // caches that pointer for the first loop, then reloads the second global
    // after all first-plane RNG/trigonometry calls have completed.
    auto* const first = standard_;
    shotgun_patterns_ = second;
    fill_spread_plane(std::span<SpreadSeed, 256>(first, 256), false, bindings.random,
                      bindings.trigonometry);
    auto* const patterns = shotgun_patterns_;
    fill_spread_plane(std::span<SpreadSeed, 256>(patterns, 256), true, bindings.random,
                      bindings.trigonometry);
    return &standard_;
}
void SpreadStorage::finalize(const SpreadStorageBindings& bindings) {
    if (shotgun_patterns_) {
        release_plane(shotgun_patterns_, bindings.memory);
        shotgun_patterns_ = nullptr;
    }
    if (standard_) {
        release_plane(standard_, bindings.memory);
        standard_ = nullptr;
    }
}
} // namespace skeet::rage

// ============================================================================
// Subcomponent: rage_shot.cpp
// ============================================================================

namespace skeet {
RageShotCommit commit_accepted_rage_shot(RageShotState& state, RageShotCommitState& shared,
                                         UserCommand& command, const RageShotCandidate& candidate,
                                         float special_gate_value, bool alternate_path,
                                         bool send_mode) noexcept {
    command.buttons |= state.flags & 0x800U ? 0x800U : 1U;
    if (state.weapon_id == 0x40 && !(state.flags & 0x800U) &&
        !(special_gate_value > state.special_gate_threshold))
        return RageShotCommit::attack_button_only;
    if (alternate_path)
        state.flags |= 0x20000000U;
    if (candidate.flags & 4U)
        state.flags |= 0x02000000U;
    shared.context_flags |= 0x800U;
    state.flags |= 0x1000U;
    state.selected_aim_point = candidate.aim_point;
    shared.last_committed_marker = state.marker;
    if (shared.send_packet == 0 && send_mode)
        shared.send_packet = 1;
    shared.command_flags = (shared.command_flags & ~0x400U) | 0x200U;
    return RageShotCommit::recorded;
}
} // namespace skeet
