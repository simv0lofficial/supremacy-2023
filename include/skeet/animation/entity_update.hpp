#pragma once
#include "skeet/lagcomp/entity_state.hpp"
#include "skeet/rage_hitboxes.hpp"

namespace skeet::animation {
// Complete433F526C: clear five dwords at13C; every other byte is preserved.
void initialize_animation_state(void* animation_state) noexcept;

struct AnimationAngleSample {
    std::int32_t frame;
    std::array<float, 2> angles; // pitch,yaw
};
static_assert(sizeof(AnimationAngleSample) == 12);
struct AnimationAngleHistory {
    bool active; //43468D7C
    std::int32_t current_frame; // pointed global43475B2C
    std::array<AnimationAngleSample, 2> samples; //434759DC,434759E8
};
// Production history has independent data cells and an indirect frame cell.
// The standalone aggregate remains useful for explicit isolated inputs.
class AnimationAngleHistoryView {
public:
    AnimationAngleHistoryView(const AnimationAngleHistory& value) noexcept
        : active_(&value.active), frame_(&value.current_frame), frame_slot_(nullptr),
          samples_(value.samples) {}
    AnimationAngleHistoryView(AnimationAngleHistory&&) = delete;
    AnimationAngleHistoryView(const std::uint8_t& active, const std::int32_t* const& frame,
                              const std::array<AnimationAngleSample, 2>& samples) noexcept
        : active_(&active), frame_(nullptr), frame_slot_(&frame), samples_(samples) {}
    bool active() const noexcept;
    std::int32_t frame() const noexcept {
        return frame_slot_ ? **frame_slot_ : *frame_;
    }
    std::span<const AnimationAngleSample, 2> samples() const noexcept {
        return samples_;
    }

private:
    const void* active_;
    const std::int32_t* frame_;
    const std::int32_t* const* frame_slot_;
    std::span<const AnimationAngleSample, 2> samples_;
};
// Complete43384C9E. First eligible sample wins; roll and failed output retain
// their original bits. The comparisons use signed frame numbers.
bool select_animation_angles(const AnimationAngleHistory&, std::int32_t requested_frame,
                             RageVector& angles) noexcept;
bool select_animation_angles(AnimationAngleHistoryView, std::int32_t requested_frame,
                             RageVector&) noexcept;

struct AnimationUpdateEngine {
    void* context;
    // Genuine optional engine entry434693F0. Its native x86 entry receives
    // ECX=state,EDX=0,XMM0=zero,XMM1=yaw,XMM2=pitch and stack int32(force).
    // The binding adapts this external engine ABI, not an internal helper.
    void (*update_state)(void* context, void* state, float zero, float yaw, float pitch,
                         std::int32_t force);
};
struct AnimationUpdateTiming {
    std::int32_t frame; // pointed global434678F8
    float current_time; // pointed global43467EB8
    float interval; // pointed global4346A338
    std::uint8_t* reset_byte; // optional43467B24, cleared after the engine call
    std::uint8_t* const* reset_byte_slot = nullptr; // production rereads43467B24 after the call
    void clear_reset_byte() const noexcept {
        auto* const reset = reset_byte_slot ? *reset_byte_slot : reset_byte;
        if (reset)
            *reset = 0;
    }
};
// Complete433A0F80. Optional engine entry absence leaves every field intact.
// The caller supplies the actual0x348-byte animation state (or its saved copy).
void update_animation_state(void* animation_state, const RageVector& angles,
                            const AnimationUpdateTiming&, const AnimationUpdateEngine&);

// Entity-facing4341D38E/4331ED0B use the canonical1D0-byte lagcomp snapshot.
// Layer descriptor stores data at0/count atC; each engine layer has stride38.
void capture_entity_animation(lagcomp::AnimationState&, const void* entity,
                              const lagcomp::EntityNetvars&) noexcept;
void restore_entity_animation(const lagcomp::AnimationState&, void* entity,
                              const lagcomp::EntityNetvars&) noexcept;
} // namespace skeet::animation
