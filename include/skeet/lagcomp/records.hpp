#pragma once
#include "skeet/core.hpp"
#include <array>
#include <cstddef>
#include <cstdint>

namespace skeet::lagcomp {
using Vector = std::array<float, 3>;

// 4341D38E/4331ED0B transfer seven dwords from each 0x38-byte engine layer,
// starting at engine layer +0x14. The other engine-layer fields are untouched.
struct AnimationLayerState {
    std::array<std::uint32_t, 7> words;
};
struct AnimationState {
    std::int32_t layer_count;
    std::array<AnimationLayerState, 13> layers;
    std::array<float, 24> pose_parameters;
};
static_assert(sizeof(AnimationState) == 0x1D0);

struct LagRecordHeader {
    std::uint32_t field_00; // 433FD690 leaves this dword untouched.
    std::uint32_t player_history_180;
    std::int32_t receive_tick;
    Vector velocity;
    float collision_max_z;
    std::uint32_t player_flags;
    std::uint32_t field_fb7e3404;
    float field_a2a8247f;
    float animation_yaw;
    std::uint32_t player_history_1ac;
    Vector origin;
    float duck_amount;
    float animation_speed_factor;
    float eye_pitch;
    std::int32_t command_tick;
    std::int32_t simulation_tick;
    float field_50; // Not assigned by the capture entry.
    float weapon_field_6777c9ce;
    std::uint8_t resolver_index;
    std::uint8_t resolver_mode;
    std::uint8_t resolver_state;
    std::uint8_t padding_5b;
    std::uint32_t flags;
};
static_assert(sizeof(LagRecordHeader) == 0x60);
static_assert(offsetof(LagRecordHeader, simulation_tick) == 0x4C);
struct LagRecordHypothesis {
    std::array<std::uint32_t, 2> state;
    std::array<std::byte, 8> padding;
    HitboxMatrices matrices;
};
static_assert(sizeof(LagRecordHypothesis) == 0x3A0);
struct LagRecord {
    LagRecordHeader header;
    AnimationState animation;
    HitboxMatrices matrices;
    std::array<LagRecordHypothesis, 3> alternatives;
};
static_assert(sizeof(LagRecord) == 0x10A0);
static_assert(offsetof(LagRecord, matrices) == 0x230);
static_assert(offsetof(LagRecord, alternatives) == 0x5C0);

// Complete assignment entries 43393CBC,4336A43E,433B7333 and copy constructor
// 433D68DA. Assignment preserves the proven padding holes in the destination.
void copy_lag_record(LagRecord& destination, const LagRecord& source) noexcept;
struct EvaluationState {
    std::array<std::uint32_t, 0x3B> words;
    std::uint32_t padding_ec;
    LagRecord record;
};
static_assert(sizeof(EvaluationState) == 0x1190);
struct HistoryEvaluation {
    std::array<std::uint32_t, 0x22> words;
    std::array<std::byte, 8> padding_88;
    EvaluationState state;
};
static_assert(sizeof(HistoryEvaluation) == 0x1220);
void assign_evaluation(HistoryEvaluation&, const HistoryEvaluation&) noexcept;
void copy_construct_evaluation(HistoryEvaluation&, const HistoryEvaluation&) noexcept;

// 4341D38E and 4331ED0B. Engine layer stride is 0x38, pose count is 24.
void capture_animation(AnimationState&, const void* layers, std::int32_t layer_count,
                       const float* pose_parameters) noexcept;
void restore_animation(const AnimationState&, void* layers, std::int32_t layer_count,
                       float* pose_parameters) noexcept;
std::int32_t time_to_tick(float time, float interval) noexcept;
// Engine-time callers retain the x87 quotient until one final float store.
std::int32_t receive_time_to_tick(float time, float interval) noexcept;
std::int32_t corrected_command_tick(std::int32_t tick, const std::int32_t* previous,
                                    const std::int32_t* current) noexcept; // 4340C3FA
} // namespace skeet::lagcomp
