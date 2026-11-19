#pragma once
#include "skeet/rage/shot_feedback.hpp"

namespace skeet::rage {
// Required live producer bindings. Pointer-global references preserve native
// rebinding; encrypted ConVar references are decoded only at consumption.
struct FeedbackEntityBindings {
    const std::uint8_t& recording_enabled; //4346754C
    const float* const& current_time; //4346A334 -> float
    const float* const& tick_interval; //4346A338 -> float
    const void* const& receive_clock; //43467B04 -> object, interval+20
    const std::uint32_t& update_rate_convar; //4346747C XOR03B4819C
    const std::uint32_t& interpolation_convar; //4346A748 XOR7D9DC86F
    const std::uint32_t& interpolation_ratio_convar; //4346A330 XORBE64D223
    const std::uint32_t& disable_ray_tests_convar; //43468EA4 XORFCD2DA7A
    const float& correction_floor; //43469254
    const float& correction_addend; //43476928
    const std::int32_t& maximum_unlag; //43468230, milliseconds
    const std::uint8_t* resolver_rework = nullptr;

    bool enabled(const ShotFeedbackState&) const noexcept;
    float now() const noexcept;
    float interval() const noexcept;
    float receive_interval() const noexcept;
    float lerp() const noexcept; // complete4331E237, update/interpolation/ratio order
    float correction_minimum() const noexcept;
    float correction_extra() const noexcept;
    std::int32_t maximum_unlag_milliseconds() const noexcept;
    bool ray_tests_disabled() const noexcept;
    bool reworked() const noexcept {
        return resolver_rework && *resolver_rework;
    }
};
struct FeedbackEntityOwner {
    ShotFeedbackState& feedback;
    ShotFeedbackRuntime& runtime;
    const FeedbackEntityBindings& bindings;
};
// BA8 interface4346F68C, full feedback owner+10. Slot2 clears only the event
// queue and recent-position count; pending shots/fire snapshots stay owned.
void reset_feedback_events(FeedbackEntityOwner&) noexcept; //complete4333D358
// Slot3 enters the existing complete reconciliation body with required live
// reads at its original consumption sites, after the same native gates.
void reconcile_feedback_events(FeedbackEntityOwner&); //complete433240DD
} //namespace skeet::rage
